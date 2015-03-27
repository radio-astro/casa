#ifndef TABLE_STREAM_READER_H
#define TABLE_STREAM_READER_H
#include "Misc.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "ASDM.h"
#include "Entity.h"
#include "EndianStream.h"
#include "ConversionException.h"
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define READBUFFERSIZE ( 50 * 1024 * 1024 )
namespace asdm {
  /**
   * A generic class to read a file containing an ASDM table as a stream.
   *
   * The purpose of this class is to provide the programmer with a way to read a file containing an ASDM table by successive
   * slices. Each slice is transformed into the in memory representation of the rows which have been read. A slice can be specified
   * in two ways :
   * <ul>
   * <li> a number of rows. See methods nextNRows.</li>
   * <li> a number of bytes. See methods untilNBytes.</li>
   * </ul>
   *
   * <b>Remarks</b>
   * <ul>
   *<li> This class has been meant as an alternative to the historical ASDM methods <code>getXXXX</code> (e.g. <code>getAntenna</code> of 
   * <code>getPointing</code>) which load entirely the tables into memory. </li>
   *<li> This class allows to read a table completely independantly of the ASDM it belongs to and of the other tables of the ASDM. </li>
   *</ul>
   *
   * @param T The parameter T must be the type of one of the tables which can be found in an ASDM (e.g. AntennaTable)
   * @param R The parameter R must be the type of the rows which make the content of a table of type T (e.g. AntennaRow is T is AntennaTable).
   */
  
  template<class T, class R> class TableStreamReader {
  public:
    /**
     * An empty constructor.
     */
    TableStreamReader(){currentState = S_CLOSED; readBuffer = (char *) malloc (READBUFFERSIZE);  boundary_1 = "" ;}

    /**
     * The destructor.
     */
    virtual ~TableStreamReader(){;}

 

    /**
     * Opens a file expected to contain an ASDM table of type T with rows of type R.
     *
     * @param directory the path to the directory containing the file.
     */
    void open(const std::string& directory){
      checkState(T_OPEN, "TableStreamReader::open");
      // Open the file.
      tablePath = directory + "/"+ T::name() + ".bin";
      tableFile.open(tablePath.c_str(), ios::in|ios::binary);
      if (!tableFile.is_open())
	throw asdm::ConversionException("Could not open file " + tablePath, T::name());

      // Determine the size of the file.
      struct stat filestatus;
      stat( tablePath.c_str(), &filestatus);
      fileSizeInBytes = filestatus.st_size;

      // And start parsing the content.

      boundary_1 = requireMIMEHeader();
      //cout << "boundary_1 = " << boundary_1 << endl;

      requireBoundary(boundary_1, 0);
      
      skipUntilEmptyLine(10);
      std::string xmlHeader = accumulateUntilBoundary(boundary_1, 100);

      //
      // We have the xmlHeader , let's parse it.
      //
      xmlDoc *doc;
      doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
      if ( doc == NULL ) 
	throw asdm::ConversionException("Failed to parse the xmlHeader into a DOM structure.", T::name());
          
      xmlNode* root_element = xmlDocGetRootElement(doc);
      if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
	throw asdm::ConversionException("Failed to parse the xmlHeader into a DOM structure.", T::name());
    
      const ByteOrder* byteOrder = NULL;
      if ( std::string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
	// Then it's an "old fashioned" MIME file for tables.
	// Just try to deserialize it with Big_Endian for the bytes ordering.
	byteOrder = asdm::ByteOrder::Big_Endian;       
	attributesSeq = T::defaultAttributesNamesInBin();
      }
      else if (std::string(T::name()+"Table").compare((const char*) root_element->name) == 0) {
	// It's a new (and correct) MIME file for tables.
	//
	// 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
	//
	xmlNode* bulkStoreRef = 0;
	xmlNode* child = root_element->children;
      
	// Skip the two first children (Entity and ContainerEntity).
	bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
	if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (std::string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
	  throw asdm::ConversionException ("Could not find the element '/"+T::name()+"Table/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", T::name());
      	
	// We found BulkStoreRef, now look for its attribute byteOrder.
	_xmlAttr* byteOrderAttr = 0;
	for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	    byteOrderAttr = attr;
	    break;
	  }
      
	if (byteOrderAttr == 0) 
	  throw asdm::ConversionException("Could not find the element '/"+T::name()+"Table/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", T::name());
      
	string byteOrderValue = std::string((const char*) byteOrderAttr->children->content);
	if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
	  throw asdm::ConversionException("No valid value retrieved for the element '/"+T::name()+"Table/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", T::name());
		
	//
	// 2nd) Look for the Attributes element and grab the names of the elements it contains.
	//
	xmlNode* attributes = bulkStoreRef->next;
	if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
	  throw asdm::ConversionException ("Could not find the element '/"+T::name()+"Table/Attributes'. Invalid XML header '"+ xmlHeader + "'.", T::name());
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
	  attributesSeq.push_back(string((const char*) childOfAttributes->name));
	  childOfAttributes = childOfAttributes->next;
	}
      }
      
      skipUntilEmptyLine(10);

      // Create an EndianIFStream from the substring containing the binary part.
      eifs = asdm::EndianIFStream (&tableFile, byteOrder);
    
      asdm::Entity entity = Entity::fromBin((EndianIStream &)eifs);
    
      // We do nothing with that but we have to read it.
      asdm::Entity containerEntity = Entity::fromBin((EndianIStream &)eifs);

      // Let's read numRows but ignore it and rely on the value specified in the ASDM.xml file.    
      int numRows = ((EndianIStream &)eifs).readInt();
      
      // Memorize the starting point of rows.
      whereRowsStart = tableFile.tellg();

      // Update the state
      currentState = S_OPENED;
    }

    /**
     * Repositions the read head to the beginning of the table.
     * The internal storage containing the result of the last read operation is cleared.
     */
    void reset() {
      checkState(T_RESET, "TableStreamReader::reset");
      clear();
      tableFile.seekg(whereRowsStart);
    }

    /**
     * Reads at most n rows in the file, creates as many memory representations of these rows and return the reference of a vector of pointers on these representations.
     *
     * @param nRows the maximum number of rows to read.
     * @return a constant reference to a vector of pointers on instances of R.
     */
    const std::vector<R*>& nextNRows(unsigned int nRows) {
      checkState(T_READ, "TableStreamReader::nextNRows"); 
      clear();
      unsigned int nread = 0;
      T& tableRef =  (T&) asdm.getTable(T::name());
      while ( hasRows() && nread < nRows ) {
	rows.push_back(R::fromBin((EndianIStream&) eifs, tableRef, attributesSeq));
	nread++;
      }
      return rows;
    }

    /**
     * Reads as many rows as possible in the file, keeps their in memory representation until the number of read bytes is greater or equal
     * a number of bytes, n,  passed as a parameter to the method
     *
     * @param n the number of read bytes which is used as an upper limit to the read operation.
     * @return a constant reference to a vector of pointers on instances of R.
     */
    
    const std::vector<R*>& untilNBytes(unsigned int nBytes) {
      checkState(T_READ, "TableStreamReader::untilNBytes"); 
      clear();
      off_t whereAmI  = tableFile.tellg();
      if (!hasRows()) return rows;

      T& tableRef = (T&) asdm.getTable(T::name());
      do {
	rows.push_back(R::fromBin((EndianIStream&) eifs, tableRef , attributesSeq));
      }
      while (((tableFile.tellg() - whereAmI) < nBytes) && hasRows());
      return rows;
    }

    /**
     * Returns true if the end of the file has not been reached.
     */
    bool hasRows() {
      checkState(T_CHECK, "TableStreamReader::hasRows");
      return tableFile.tellg() < (fileSizeInBytes - 19);
    }

    /**
     * Releases all the resources allocated to the instance which can be reused with another file.
     */
    void close() {
      checkState(T_CLOSE, "TableStreamReader::close"); 
      clear();
      if (tableFile.is_open()) tableFile.close();
      free(readBuffer);
      // Update the state.
      currentState = S_CLOSED;
    }
    
  private:
    std::string			tablePath;
    std::ifstream		tableFile;
    std::string	                currentLine;
    std::string	                boundary_1;

    off_t                       fileSizeInBytes;
    asdm::EndianIFStream	eifs;
    std::vector<std::string>	attributesSeq;
    asdm::ASDM                  asdm;
    std::vector<R*>		rows;

    char*                       readBuffer;

    streampos whereRowsStart;

    enum State {S_CLOSED, S_OPENED};
    enum Transition {T_OPEN, T_CHECK, T_RESET, T_READ, T_CLOSE};
    State currentState;

    void checkState(Transition t, const std::string& methodName) const {
      switch (currentState) {
      case S_CLOSED:
	if (t == T_OPEN) return;

      case S_OPENED:
	if (t == T_CHECK || t == T_RESET || t == T_READ || t == T_CLOSE) return;
      }
      throw asdm::ConversionException("Invalid call of method '" + methodName + "' in the current context.", T::name());
    }
    /**
     * Empty the local storage containing the rows obtained during the last read operation.
     */
    void clear() {
      for (unsigned int i = 0; i < rows.size(); i++)
	if (rows[i]) delete rows[i];
      rows.clear();
    }

    void skipUntilEmptyLine(int maxSkips) {
      // cout << "Entering skipUntilEmptyLine" << endl;
      int numSkips = 0;
      std::string line;
      do {
	line = trim(nextLine());
	numSkips++;
      } 
      while (line.size() != 0 && numSkips <= maxSkips);
      
      if (numSkips > maxSkips) {
	ostringstream oss;
	oss << "could not find an empty line is less than " << maxSkips + 1 << " lines." << endl;
	throw asdm::ConversionException(oss.str(), T::name());
      } 
      // cout << "Exiting skipUntilEmptyLine" << endl;
    }

    std::string nextLine() {
      unsigned long long whereAmI = tableFile.tellg();
      getline(tableFile, currentLine);
      if (tableFile.fail()) {
	std::ostringstream oss ;
	oss << "TableStreamReader::nextLine() : I could not read a line in '" << tablePath <<  "' at position " << whereAmI << ".";
	throw asdm::ConversionException(oss.str(), T::name());
      }
      // cout << "nextLine has read '" << currentLine << "'" << endl;
      return currentLine;
    }  

    pair<std::string, std::string> headerField2Pair(const std::string& hf){
    std::string name, value;
    size_t colonIndex = hf.find(":");
    if (colonIndex == std::string::npos)
      throw asdm::ConversionException(" could not detect a well formed MIME header field in '"+hf+"'", T::name());

    if (colonIndex > 0) {
      name = hf.substr(0, colonIndex);
      trim(name);
    }

    if (colonIndex < hf.size()) {
      value = hf.substr(colonIndex+1);
      trim(value);
    }

    return make_pair(name, value);
}

std::string requireMIMEHeader() {
  // MIME-Version
  pair<std::string, std::string>name_value(headerField2Pair(nextLine()));
// cout << name_value.first << "=" << name_value.second << endl;
// if (currentLine != "MIME-Version: 1.0") // a work around for the case when the very first character is not the expected "M" (happened with some corrupted data).
if (! boost::algorithm::iends_with(currentLine, "IME-Version: 1.0"))
  throw asdm::ConversionException("'MIME-Version: 1.0' missing at the very beginning of the file '"+ tablePath +"'.", T::name());

// Content-Type
boundary_1 = requireBoundaryInCT(requireHeaderField("CONTENT-TYPE").second);

// cout << "boundary_1 =" << boundary_1 << endl;

// Content-Description
//name_value = requireHeaderField("CONTENT-DESCRIPTION");

// Content-Location
//name_value = requireHeaderField("CONTENT-LOCATION");

// Look for an empty line in the at most 10 subsequent lines.
skipUntilEmptyLine(20);

return boundary_1;
}    

pair<std::string, std::string> requireHeaderField(const std::string & hf) {
  std::string s = trim_copy(nextLine());
  while (boost::algorithm::iends_with(s, ";")) {
    s += trim_copy(nextLine());
  }
  pair<std::string, std::string> hf2pair(headerField2Pair(s));
if (to_upper_copy(hf2pair.first) != hf)
  throw asdm::ConversionException("read '" + currentLine + "'. Was expecting '" + hf + "'...", T::name());
return hf2pair;
}

void requireBoundary(const std::string& boundary, int maxLines) {
  // cout << "Entering require boundary with boundary == '" << boundary << "' and maxLines = " << maxLines << endl; 
  int numLines = 0;
  std::string dashdashBoundary = "--"+boundary;
  std::string line = nextLine();
  while ((numLines <= maxLines) && (line.compare(dashdashBoundary) != 0)) {
    numLines++;
    line = nextLine();
  }

  if (numLines > maxLines) {
    ostringstream oss;
    oss << "could not find the boundary std::string '"<< boundary << "' in less than " << maxLines + 1 << " lines." << endl;
    throw asdm::ConversionException(oss.str(), T::name());
  }
}

std::string accumulateUntilBoundary(const std::string& boundary, int maxLines) {
  // cout << "Entering accumulateUntilBoundary with maxLines = " << maxLines << endl;
  int numLines = 0;
  std::string line ;
  std::string result;
  line=trim(nextLine());
  while ( numLines <= maxLines && line.find("--"+boundary) == std::string::npos ) {
    result += line;
    line=trim(nextLine());
    numLines++;
  }
  
  if (numLines > maxLines) {
    ostringstream oss;
    oss << "could not find the boundary std::string '"<< boundary << "' in less than " << maxLines + 1 << " lines." << endl;
    throw asdm::ConversionException(oss.str(), T::name());    
  }
  return result;
}

std::string requireBoundaryInCT(const std::string& ctValue) {
  vector<std::string> cvValueItems;
 
split (cvValueItems, ctValue, is_any_of(";"));
vector<std::string> cvValueItemsNameValue;
for ( vector<std::string>::const_iterator iter = cvValueItems.begin(); iter != cvValueItems.end() ; iter++ ) {
  cvValueItemsNameValue.clear();
  split(cvValueItemsNameValue, *iter, is_any_of("="));
  string boundary;
  if ((cvValueItemsNameValue.size() > 1) && (to_upper_copy(trim_copy(cvValueItemsNameValue[0])) == "BOUNDARY") && (unquote(cvValueItemsNameValue[1], boundary).size() > 0))
    return boundary;
													     }
throw asdm::ConversionException("could not find a boundary definition in '" + ctValue + "'.", T::name());
}
string unquote(const string& s, string& unquoted) {
  if (s.size() >= 2) 
    if (((s.at(0) == '"') && (s.at(s.size()-1) == '"')) || ((s.at(0) == '\'') && (s.at(s.size()-1) == '\''))) {
      if (s.size() == 2)
	unquoted = "";
      else
	unquoted = s.substr(1, s.size() - 2);
    }
    else
      unquoted = s;
  else
    unquoted = s;
  return unquoted;
}

};
} // end namespace asdm
#endif

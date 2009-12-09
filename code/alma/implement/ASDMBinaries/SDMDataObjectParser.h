#ifndef SDMDataObjectParser_CLASS
#define SDMDataObjectParser_CLASS

#include <iostream>
#include <string>
#include <vector>
#include <libxml/parser.h>
#include <libxml/tree.h>



#ifdef REG_BASIC
#undef REG_BASIC
#endif

#ifdef REG_EXTENDED 
#undef REG_EXTENDED
#endif 

#ifdef REG_ICASE
#undef REG_ICASE
#endif

#ifdef REG_NOSUB
#undef REG_NOSUB
#endif

#ifdef REG_NEWLINE
#undef REG_NEWLINE
#endif

#ifdef REG_NOTBOL
#undef REG_NOTBOL
#endif

#ifdef REG_NOTEOL
#undef REG_NOTEOL
#endif

#ifdef REG_STARTEND
#undef REG_STARTEND
#endif

#include <boost/regex.hpp> 
using namespace boost;

#include "SDMDataObject.h"


#ifndef WITHOUT_ACS
#include "almaEnumerations_IFC.h"
#endif
#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;

#include "CPrimitiveDataType.h"
using namespace PrimitiveDataTypeMod; 

#include "CCorrelatorType.h"
using namespace CorrelatorTypeMod;

using namespace std;

namespace asdmbinaries {

  /**
   * A class to represent an exception thrown during the parsing of an XML header 
   * in a MIME message containing ALMA binary data.
   */ 
  class SDMDataObjectParserException {
    
  public:
    /**
     * An empty contructor.
     */
    SDMDataObjectParserException();

    /**
     * A constructor with a message associated with the exception.
     * @param m a string containing the message.
     */
    SDMDataObjectParserException(string m);
    
    /**
     * The destructor.
     */
    virtual ~SDMDataObjectParserException();

    /**
     * Returns the message associated to this exception.
     * @return a string.
     */
    string getMessage() const;
    
  protected:
    string message;
    
  };
  
  inline SDMDataObjectParserException::SDMDataObjectParserException() : message ("SDMDataObjectParserException") {}
  inline SDMDataObjectParserException::SDMDataObjectParserException(string m) : message(m) {}
  inline SDMDataObjectParserException::~SDMDataObjectParserException() {}
  inline string SDMDataObjectParserException::getMessage() const {
    return "SDMDataObjectParserException : " + message;
  }


  // class HeaderParser
  class HeaderParser {
    friend class SDMDataObjectParser;
    friend class SDMDataObject;
    
  public:
    HeaderParser();
    virtual ~HeaderParser();
    
    void parseFile(const string& filename, SDMDataObject& sdmDataObject);
    void parseMemory(const string& buffer, SDMDataObject& sdmDataObject);
    
    void reset();
    
    
    
  private:
    void parseSDMDataHeader(xmlNode* a_node, SDMDataObject& sdmDataObject);
    
    //    void parseProjectPath(xmlNode* a_node, SDMDataObject& sdmDataObject);
    long long parseStartTime(xmlNode* a_node);
    string parseDataOID(xmlNode* a_node);
    int  parseDimensionality(xmlNode* a_node);
    int  parseNumTime(xmlNode* a_node);
    void parseExecBlock(xmlNode* a_node, SDMDataObject& sdmDataObject);
    int  parseExecBlockNum(xmlNode* a_node);
    int  parseScanNum(xmlNode* a_node);
    int  parseSubscanNum(xmlNode* a_node);
    
    int  parseNumAntenna(xmlNode* a_node);
    
    void parseCorrelationMode(xmlNode* a_node, SDMDataObject& sdmDataObject);
    void parseSpectralResolution(xmlNode* a_node, SDMDataObject& sdmDataObject);
    void parseProcessorType(xmlNode* a_node, SDMDataObject& sdmDataObject);
    void parseDataStruct (xmlNode* a_node, SDMDataObject& sdmDataObject);
    
    SDMDataObject::Baseband  parseBaseband(xmlNode* a_node, SDMDataObject& sdmDataObject);
    
    void  parseSpectralWindow(xmlNode* a_node, SDMDataObject& sdmDataObject, vector<SDMDataObject::SpectralWindow>& spectralWindow);
    
    SDMDataObject::BinaryPart parseBinaryPart(xmlNode* a_node, const string& attachmentName);
    SDMDataObject::AutoDataBinaryPart parseAutoDataBinaryPart(xmlNode* a_node, const string& attachmentName);
    SDMDataObject::ZeroLagsBinaryPart parseZeroLagsBinaryPart(xmlNode* a_node, const string& attachmentName);

    //      SDMDataObject::TypedBinaryPart  parseTypedBinaryPart(xmlNode* a_node, const string& attachmentName);
    
    xmlDoc *doc;
    
    static const regex  PROJECTPATH3;
    const static string SDMDATAHEADER;
    const static string SCHEMAVERSION;
    const static string BYTEORDER;
    const static string PROJECTPATH;
    const static string STARTTIME;
    const static string DATAOID;
    const static string XLINKHREF;
    const static string XLINKTITLE;
    
    const static string DIMENSIONALITY;
    const static string NUMTIME;
    
    const static string EXECBLOCK;
    const static string EXECBLOCKNUM;
    const static string SCANNUM;
    const static string SUBSCANNUM;
    
    const static string NUMANTENNA;
    
    const static string CORRELATIONMODE;
    const static string SPECTRALRESOLUTION;
    const static string PROCESSORTYPE;
    const static string DATASTRUCT;
    const static string APC;
    const static string REF;
    
    const static string BASEBAND;
    const static string NAME;
    
    const static string SPECTRALWINDOW;
    const static string SW;
    const static string SWBB;
    const static string CROSSPOLPRODUCTS;
    const static string SDPOLPRODUCTS;
    const static string SCALEFACTOR;
    const static string NUMSPECTRALPOINT;
    const static string NUMBIN;
    const static string SIDEBAND;
    const static string IMAGE;
    
    const static string FLAGS;
    const static string ACTUALTIMES;
    const static string ACTUALDURATIONS;
    const static string ZEROLAGS;
    const static string CORRELATORTYPE;
    const static string CROSSDATA;
    const static string AUTODATA;
    const static string NORMALIZED;
    
    const static string SIZE;
    const static string AXES;
    const static string TYPE;
  }; // class HeaderParser

  // class CorrSubsetHeaderParser
  class CorrSubsetHeaderParser {
    friend class SDMDataSubset;
    
  public:
    CorrSubsetHeaderParser();
    virtual ~CorrSubsetHeaderParser();
    void parseFile(const string& filename, SDMDataSubset& sdmCorrDataSubset);
    void parseMemory(const string& buffer, SDMDataSubset& sdmCorrDataSubset);
    void parseCrossDataType(xmlNode* a_node, SDMDataSubset& sdmCorrDataSubset);
    void reset();
    
  private:
    // Regular expressions used to decipher the content of projectPath attribute.
    static const regex PROJECTPATH4;
    static const regex PROJECTPATH5;
    
    void parseSDMDataSubsetHeader(xmlNode* a_node, SDMDataSubset& sdmCorrDataSubset);
    //    void parseProjectPath(xmlNode* a_node, SDMDataSubset& sdmCorrDataSubset);
    void parseSchedulePeriodTime(xmlNode* a_node, SDMDataSubset& sdmCorrDataSubset);
    long long parseTime(xmlNode* a_node);
    long long parseInterval(xmlNode* a_node);
    void parseAbortObservation(xmlNode* a_node, SDMDataSubset& sdmCorrDataSubset);
    
    xmlDoc* doc;
    
    const static string SDMDATASUBSETHEADER;
    const static string PROJECTPATH;
    const static string SCHEDULEPERIODTIME;
    const static string TIME;
    const static string INTERVAL;
    const static string DATASTRUCT;
    const static string REF;
    const static string ABORTOBSERVATION;
    const static string ABORTTIME;
    const static string ABORTREASON;
    const static string XLINKHREF;
    const static string DATAREF;
    const static string FLAGSREF;
    const static string ACTUALTIMESREF;
    const static string ACTUALDURATIONSREF;
    const static string ZEROLAGSREF;
    const static string CROSSDATAREF;
    const static string TYPE;
    const static string AUTODATAREF;
  };  // class CorrSubsetHeaderParser


  // class TPSubsetHeaderParser
  class TPSubsetHeaderParser {
    friend class SDMDataSubset;
  public:
    TPSubsetHeaderParser();
    virtual ~TPSubsetHeaderParser();
    void parseFile(const string& filename,SDMDataSubset& sdmTPDataSubset );
    void parseMemory(const string& buffer,SDMDataSubset& sdmTPDataSubset );
    void reset();
    
  private:
    // Regular expressions used to decipher the content of projectPath attribute.
    static const regex PROJECTPATH3;

    void parseSDMDataSubsetHeader(xmlNode* a_node,SDMDataSubset& sdmTPDataSubset);
    //    void parseProjectPath(xmlNode* a_node, SDMDataSubset& sdmTPDataSubset);
    void parseSchedulePeriodTime(xmlNode* a_node, SDMDataSubset& sdmCorrDataSubset);
    long long parseTime(xmlNode* a_node);
    long long parseInterval(xmlNode* a_node);
    string parseDataStructureDesc(xmlNode* a_node);
    void parseBinaryData(xmlNode* a_node, SDMDataSubset& sdmTPDataSubset);
    
    xmlDoc* doc;
    
    const static string SDMDATASUBSETHEADER;
    const static string PROJECTPATH;
    const static string SCHEDULEPERIODTIME;
    const static string TIME;
    const static string INTERVAL;
    const static string DATASTRUCT;
    const static string REF;
    const static string DATAREF;
    const static string XLINKHREF;
    const static string FLAGSREF;
    const static string ACTUALTIMESREF;
    const static string ACTUALDURATIONSREF;
    const static string AUTODATAREF;
  };  // class TPSubsetHeaderParser
  
  
  class SDMDataObjectParser {
    friend class SDMDataObject;
    friend class SDMDataSubset;
    friend class HeaderParser;

  public:
    SDMDataObjectParser();
    virtual ~SDMDataObjectParser();
    void parseFileHeader(const string& filename, SDMDataObject& sdmDataObject);
    void parseMemoryHeader(const string& buffer, SDMDataObject& sdmDataObject);

    void parseFileCorrSubsetHeader(const string& filename, SDMDataSubset& sdmCorrSubset);
    void parseMemoryCorrSubsetHeader(const string& buffer, SDMDataSubset& sdmCorrSubset);

    void parseFileTPSubsetHeader(const string& filename, SDMDataSubset& sdmCorrDataSubset);
    void parseMemoryTPSubsetHeader(const string& filename, SDMDataSubset& sdmCorrDataSubset);
    
    static void isElement(xmlNode* a_node, const string& elementName);
    static bool testElement(xmlNode* a_node, const string& elementName);
    static void inElements(xmlNode* a_node, const vector<string>& elementNames);

    static xmlAttr* hasAttr(xmlNode* a_node, const string& attrName);

    static void tokenize(const string& str,
			 vector<string>& tokens,
			 const string& delimiters = " ");

    static void tokenize(const string& str,
			 set<string>& tokens,
			 const string& delimiters = " ");
    static string substring(const string &s, int a, int b);
    static string trim(const string &s);

    static string parseString(xmlNode* a_node);
    static long long parseLongLong(xmlNode* a_node);
    static int   parseInt(xmlNode* a_node);
    static bool  parseBool(xmlNode* a_node);
    static float parseFloat(xmlNode* a_node);
    static int   parseIntAttr(xmlNode* a_node, const string& attrName);
    static bool  parseBoolAttr(xmlNode* a_node, const string& attrName);
    static float parseFloatAttr(xmlNode* a_node, const string& attrName);
    static string parseStringAttr(xmlNode* a_node, const string& attrName);
    static const ByteOrder* parseByteOrderAttr(xmlNode* a_node, const string& attrName);

    template<class Enum, class EnumHelper> static Enum parseStringAttr(xmlNode* a_node, const string& attrName) {
      xmlAttr* attr = 0;
      
      if ((attr = hasAttr(a_node, attrName))) {
	string s = string((const char*)attr->children->content);
	try {
	  Enum result = EnumHelper::literal(SDMDataObjectParser::trim(s));
	  return result;
	}
	catch(string m) {
	  throw  SDMDataObjectParserException(m);
	}
      }
      else 
	throw SDMDataObjectParserException("could not find attribute '" + attrName + "' in " + string((const char*)a_node->name));    
    }
    
    template<class Enum, class EnumHelper> static Enum parseLiteral(xmlNode* a_node) {
      if ((a_node != NULL) && (a_node->next == NULL)) {      
	try {
	  Enum result = EnumHelper::literal(SDMDataObjectParser::trim(string((const char*) a_node->content)));
	  return result;
	}
	catch (string m) {
	  throw SDMDataObjectParserException(m);
	}
      }
      else 
	throw SDMDataObjectParserException("The content of an element could not parsed into a literal");
    }


    static vector<string> parseStringsAttr(xmlNode* a_node, const string& attrName);
    static set<string> parseStringSetAttr(xmlNode* a_node, const string& attrName);

    template<class Enum, class EnumHelper> static vector<Enum> parseStringsAttr(xmlNode* a_node, const string& attrName) {
      xmlAttr* attr = 0;
      
      if ((attr = hasAttr(a_node, attrName))) {
	vector<string> v_s;
	tokenize((const char*)attr->children->content, v_s);

	vector<Enum> result; 
	unsigned int i = 0;
	try {
	  for (i = 0; i < v_s.size(); i++)
	    result.push_back(EnumHelper::literal(v_s.at(i)));
	  return result;
	}
	catch (string m) {
	  throw  SDMDataObjectParserException(m);
	}
      }
      else 
	throw SDMDataObjectParserException("could not find attribute '" + attrName + "' in " + string((const char*)a_node->name));
    }

    static vector<unsigned int> parseProjectPath(xmlNode* a_node, unsigned int len);

    static vector<unsigned int> parseProjectPath(xmlNode* a_node);

    private:    

    static const regex PROJECTPATH3;
    static const regex PROJECTPATH4;
    static const regex PROJECTPATH5;
    static const regex PROJECTPATH4OR5;

    HeaderParser headerParser;
    CorrSubsetHeaderParser corrSubsetHeaderParser;
    TPSubsetHeaderParser tpSubsetHeaderParser;
    
  }; // class SDMDataObjectParser
  
  
} // namespace asdmbinaries
#endif // HeaderParser_CLASS

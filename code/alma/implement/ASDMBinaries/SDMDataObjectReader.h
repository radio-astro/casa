#ifndef SDMDataObjectReader_CLASS
#define SDMDataObjectReader_CLASS

#include <string>
#include <map>
#include <set>
#include <vector>
#include <bitset>

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

#include "SDMDataObjectParser.h"
#include "SDMDataObject.h"

using namespace std;
using namespace boost;


/*
typedef int64_t          ACTUALDURATIONSTYPE;
typedef int64_t          ACTUALTIMESTYPE;     
typedef float              AUTODATATYPE;        
typedef int                LONGCROSSDATATYPE;   // mcaillat 
typedef short int          SHORTCROSSDATATYPE;
typedef float              FLOATCROSSDATATYPE;  
typedef unsigned int       FLAGSTYPE; // mcaillat
typedef float              ZEROLAGSTYPE;   
*/

namespace asdmbinaries {
  /**
   * A class to represent an exception thrown while reading a MIME message containing ALMA binary data.
   */ 
  class SDMDataObjectReaderException {
    
  public:
    /**
     * An empty contructor.
     */
    SDMDataObjectReaderException();

    /**
     * A constructor with a message associated with the exception.
     * @param m a string containing the message.
     */
    SDMDataObjectReaderException(string m);
    
    /**
     * The destructor.
     */
    virtual ~SDMDataObjectReaderException();

    /**
     * Returns the message associated to this exception.
     * @return a string.
     */
    string getMessage() const;
    
  protected:
    string message;
    
  };

  inline SDMDataObjectReaderException::SDMDataObjectReaderException() : message ("SDMDataObjectReaderException") {}
  inline SDMDataObjectReaderException::SDMDataObjectReaderException(string m) : message(m) {}
  inline SDMDataObjectReaderException::~SDMDataObjectReaderException() {}
  inline string SDMDataObjectReaderException::getMessage() const {
    return "SDMDataObjectReaderException : " + message;
  }

  /**
   * A class to read a MIME message containing ALMA binary data and provide a view on these binary data through an instance of SDMDataObject.
   * An instance of a SDMDataObjectReader can read MIME messages stored in a file or in a memory buffer, via two different versions of
   * the read() method.
   * The binary data read can be Total Power or Correlator (integration and subintegration) data.
   * Once the data obtained by a call to a read method are not needed any more the done() method must be called to release the resources allocated to 
   * access the data.
   */
  class SDMDataObjectReader {
  public:
    /**
     * An empty constructor.
     */
    SDMDataObjectReader();

    /**
     * The destructor.
     */
    virtual ~SDMDataObjectReader();

    /**
     * This method reads a MIME message contained in a disk file and returns a reference to an SDMDataObject.
     * @param filename the name of the file to be read.
     * @return a reference to an SDMDataObject built during the read operation.
     * @throw SDMDataObjectReaderException
     */

    const SDMDataObject & read(string filename);

    /**
     * This method reads a MIME message contained in a memory buffer and returns a reference to an SDMDataObject.
     * @param buffer the adress of the buffer containing the MIME message.
     * @param size the size in *bytes* of that buffer.
     * @param fromFile a bool to indicate if the buffer is actually the result of a mapping of a file to virtual memory (false by default).
     * @return a reference to an SDMDataObject built during the read operation.
     * @throw SDMDataObjectReaderException
     */
    const SDMDataObject & read(const char * buffer, unsigned long int size, bool fromFile=false);

    
    /**
     * This method returns a reference to the SDMDataObject instance built during the execution of 
     * the read method.
     * @return a reference to an SDMDataObject.
     * @throw SDMDataObjectReaderException.
     */
    const SDMDataObject & ref() const;
    
    /**
     * This method returns a pointer to the SDMDataObject instance built during the execution of 
     * the read method.
     * @return a reference to an SDMDataObject.
     * @throw SDMDataObjectReaderException.
     */
    const SDMDataObject * ptr() const;
    
    /**
     * A method to release the resources allocated by the read operation. 
     * This method must be called once the SDMDataObject built by the read method is no more needed.
     */
    void done();

  private:
    enum READ {UNKNOWN_, MEMORY_, FILE_};
    READ read_;

    int filedes_;
    unsigned int dataSize_;
    char* data_;
    string::size_type position_;
    string::size_type lastPosition_;
    string::size_type endPosition_;

    unsigned int integrationNum_;
    unsigned int scanNum_;

    static const bool initClass_;
    static bool initClass();
    void init() ;
    string::size_type find(const string& s);
    bool compare(const string& s);
    bool EOD();

    // Two strings used as MIME boundaries
    static const string MIMEBOUNDARY_1;
    static const string MIMEBOUNDARY_2;
    
    // Regular expressions used to identify a Content-ID field in a MIME header
    static const regex CONTENTIDDATASTRUCTUREREGEXP;
    static const regex CONTENTIDSUBSETREGEXP;
    static const regex CONTENTIDBINREGEXP;
    static const regex CONTENTIDBINREGEXP1;
    static const regex CONTENTIDBINREGEXP2;

    // Set of valid binary attachment names
    static set<string> BINATTACHNAMES;
    enum BINATTACHCODES {ACTUALDURATIONS=0, ACTUALTIMES=1, AUTODATA=2, FLAGS=3, CROSSDATA=4, ZEROLAGS=5};
    static map<string, BINATTACHCODES> name2code;

    string extractXMLHeader(const string& boundary);
    void tokenize(const string& str, vector<string>& tokens, const string& delimiters);
    void getFields(const string& header, map<string, string>& fields);
    string getContentID();
    string getContentLocation();

    void processMIME();
    void processMIMESDMDataHeader();
    void processMIMESDMDataSubsetHeader(SDMDataSubset& sdmDataSubset);
    void processMIMEIntegrations();
    void processMIMEIntegration();
    void processMIMESubscan();
    void processBinaryAttachment(const string& boundary, const SDMDataSubset& sdmDataSubset);


    // SDMDataObject related stuff
    SDMDataObjectParser parser_;
    SDMDataObject sdmDataObject_;
    const SDMDataObject& sdmDataObject();

    bitset<6> attachmentFlags;

    const ACTUALTIMESTYPE * actualTimes_;
    unsigned long int nActualTimes_;
    const ACTUALDURATIONSTYPE * actualDurations_;
    unsigned long int nActualDurations_;
    const ZEROLAGSTYPE * zeroLags_;
    unsigned long int nZeroLags_;
    const FLAGSTYPE * flags_;
    unsigned long int nFlags_;
    const INTCROSSDATATYPE  * longCrossData_;
    const SHORTCROSSDATATYPE * shortCrossData_;
    const FLOATCROSSDATATYPE * floatCrossData_;
    unsigned long int nCrossData_;
    const AUTODATATYPE * autoData_;
    unsigned long int nAutoData_;   

  };
} // namespace asdmbinaries
#endif // SDMDataObjectReader_CLASS

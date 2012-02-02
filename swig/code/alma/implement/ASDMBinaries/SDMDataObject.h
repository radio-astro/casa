#ifndef SDMDataObject_CLASS
#define SDMDataObject_CLASS

#include <string>
#include <set>
#include <vector>
#include <map>
#include <sstream>

#ifdef WITHOUT_ACS
#else
#include "almaEnumerations_IFC.h"
#endif

#include "SDMDataObjectPartTypes.h"

#include "CAtmPhaseCorrection.h"
#include "CAxisName.h"
#include "CBasebandName.h"
#include "CCorrelationMode.h"
#include "CPrimitiveDataType.h"
#include "CSpectralResolutionType.h"
#include "CProcessorType.h"
#include "CCorrelatorType.h"
#include "CStokesParameter.h"
#include "CNetSideband.h"

using namespace AtmPhaseCorrectionMod;
using namespace AxisNameMod;
using namespace BasebandNameMod;
using namespace CorrelationModeMod;
using namespace PrimitiveDataTypeMod;
using namespace SpectralResolutionTypeMod;
using namespace ProcessorTypeMod;
using namespace CorrelatorTypeMod;
using namespace StokesParameterMod;
using namespace NetSidebandMod;

using namespace std;



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

#ifdef REG_NOERROR
#undef REG_NOERROR
#endif

#ifdef REG_NOMATCH
#undef REG_NOMATCH
#endif

#ifdef REG_BADPAT
#undef REG_BADPAT
#endif

#ifdef REG_ECOLLATE
#undef REG_ECOLLATE
#endif

#include <boost/regex.hpp> 
using namespace boost;

#if defined(__APPLE__)
#include <machine/endian.h>
#else 
#include <endian.h>
#endif

/*
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <boost/date_time/gregorian/gregorian.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;
*/

/**
 * @mainpage
 *
 * This is the documentation of a set of C++ classes dedicated to the processing of ALMA binary data. 
 * 
 * @section low-level Low level classes.
 * All those classess are grouped under the same namespace asdmbinaries.
 * ALMA binary data are kept on storage media in files whose content is formally
 * described by <b>the Science Data Model Binary Data Format</b> (BDF). The so called low level classes documented here
 * provide their users with an in-memory representation of those ALMA binary data and tools to write BDF files from
 * such representations and conversely to build in-memory representations from BDF files in a given number of use cases.
 *
 * <ul>
 * <li> asdmbinaries::SDMDataObject is a class to represent in-memory ALMA binary data.</li>
 * <li> asdmbinaries::SDMDataObjectWriter is a class to build an SDMDataObject and to write it as a BDF document. </li>
 * <li> asdmbinaries::SDMDataObjectReader is a class to read a BDF document and to parse it into an SDMDataObject. </li>
 * </ul>
 * 
 * 
 */

/**
 * The asdmbinaries namespace contains all the classes dedicated to the processing of ALMA binary data.
 */
namespace asdmbinaries {

  /**
   * The XML schema version of the XML instances that those classes claim to be compatible with.
   */
  const int SCHEMAVERSION=2;


  /**
   * A class to represent an exception thrown during an access to an SDMDataObject. 
   */ 
  class SDMDataObjectException {
    
  public:
    /**
     * An empty contructor.
     */
    SDMDataObjectException();

    /**
     * A constructor with a message associated with the exception.
     * @param m a string containing the message.
     */
    SDMDataObjectException(const string& m);
    
    /**
     * The destructor.
     */
    virtual ~SDMDataObjectException();

    /**
     * Returns the message associated to this exception.
     * @return a string.
     */
    string getMessage() const;
    
  protected:
    string message;
    
  };

  inline SDMDataObjectException::SDMDataObjectException() : message ("SDMDataObjectReaderException") {}
  inline SDMDataObjectException::SDMDataObjectException(const string& m) : message(m) {}
  inline SDMDataObjectException::~SDMDataObjectException() {}
  inline string SDMDataObjectException::getMessage() const {
    return "SDMDataObjectException : " + message;
  }
  
  /**
   * \defgroup optional Material related to optional values.
   */

  /**
   * \ingroup optional
   * A class to embed optional information.
   *
   */
  template<class Enum, class EnumHelper> 
    class Optional {
    private:
    bool present_;
    Enum literal_;
    
    public:
    /**
     * The empty constructor.
     * 
     * To be used whenever an optional value has to be declared as absent.
     */
    Optional() {
      present_=false;
    }
    
    /**
     * The full constructor.
     *
     * To be used whenever an optional value has to be declared as present.
     *
     * @param literal the value of type Enum to embed in the Optional class.
     */
    Optional(Enum literal) {
      literal_ = literal;
      present_ = true;
    }
    
    /**
     * Test of presence.
     *
     * @return true (resp. false)  if this represents a present (resp. false) optional value.
     */
    bool present() const { return present_; }

    /**
     * Returns the optional value 
     *
     * @return an Enum
     *
     * @note the returned value is meaningful if and only if present() == true !
     *
     */
    Enum literal() const { return literal_; }
  };

  /**
   * \ingroup optional
   * A typedef definition for an optional spectral resolution type.
   *
   */
  typedef Optional<SpectralResolutionType, CSpectralResolutionType> OptionalSpectralResolutionType;


  /**
   * A class to represent byte order information.
   *
   */
  class ByteOrder {
  public:
    static const ByteOrder* Little_Endian; /*< A unique object to represent a little endian byte order. */
    static const ByteOrder* Big_Endian;    /*< A unique object to represent a big endian byte order. */
    static const ByteOrder* Machine_Endianity; /*< A unique object storing the endianity of the machine. */

    /**
     * Returns a string representation of this.
     *
     * <ul>
     * <li> Little_Endian is returned as "Little_Endian", </li>
     * <li> Big_Endian is returned as "Big_Endian", </li>
     * </ul>
     */
    string toString() const ;

  private:
    string name_;
    int endianity_;

    ByteOrder(const string & name, int endianity);
    virtual ~ByteOrder();
    static const ByteOrder* machineEndianity(); 
  };

  // forward declarations.
  class SDMDataSubset;
  class SDMDataObject;


  
  class SDMDataObjectReader;

  // SDMDataObject:: declarations
  //

  /**
   * A class to represent ALMA binary data.
   * Two situations can be handled by this class:
   * <ul>
   * <li> Correlator data </li>
   * <li> Total power data </li>
   * </ul>
   *
   * In the former case (correlator data), an SDMDataObject is organized as a header containing descriptive informations
   * valid for all the recorded binary data, followed by a sequence of (local header, binary data) pairs. Each (local header, binary data) pair
   * corresponds to one integration and the full SDMDataObject corresponds to one subscan.
   *
   * In the latter case (total power data), an SDMDataObject is organized as a header containing descriptive informations,
   * followed by one (local header, binary data) pair. This pair corresponds to the data recorded during one subscan.
   *
   * An instance of an SDMDataObject is never created explicitely by calling some constructors or setters methods. Instances
   * of SDMDataObject are rather created implicitely :
   * <ul>
   * <li> on output, while creating a MIME message containing ALMA binary data by using the class SDMDataObjectWriter; actual parameters passed to methods of this class are assembled together to
form an SDMDataObject which is in turn converted into a MIME message, </li>
   * <li> on input, while reading a MIME message containing ALMA binary data by using the class SDMDataObjectReader; the result of the parsing of the MIME message is returned in an SDMDataObject. </li>
   * </ul>
   * 
   * @section content-access Accessing the different parts of an SDMDataObject.
   * We give here a quick list of the method which allows to retrieve the different parts of an SDMDataObject :
   * <ul>
   * <li>title() : the general title for these binary data.</li>
   * <li>startTime() : Epoch when started the observation for the data collected in this SDMDataObject. This must be equal to the mid-point interval minus half the interval of the first data dump.
   * <li>numTime() : number of (sub)integrations stored in this SDMDataObject. </li>
   * <li>dataOID() : Data object identifier given to this SDMDataObject once it's been converted into a MIME message and archived in the bulkstore.
   * <li>execBlockUID() : the archive UID of the exec block.
   * <li>execBlockNum() : the execblock number (one-based). </li>
   * <li>scanNum() : the scan number (one-based). </li>
   * <li>subscanNum() : the subscan number (one-based). </li>
   * <li>projectPath() : a string built from the string representations of execBlockNum, scanNum and subscanNum prefixed with '/' characters. E.g. if execBlockNum==10 && scanNum==1 &&  subscanNum==1
   * then projectPath=="/10/1/1".</li>
   * <li>numAntenna() : Number of antenna scheduled to produce the data.</li>
   * <li>correlationMode() : the correlation mode encoded as a value of the enumeration CorrelationMode.</li>
   * <li>spectralResolutionType() : the type of spectral resolution defined as a value of the enumeration SpectralResolutionType.</li>
   * <li>dataStruct() : a instance of DataStruct describing the structure of the binary data.</li>
   * <li>corrDataSubsets() : a vector of SDMDataSubset containing the collection of (sub) integrations in the case of correlator data. </li>
   * <li>sdmDataSubset() : an SDMDataSubset given its project path.
   * <li>tpDataSubset() : a single SDMDataSubset containing all the binary data of a subscan in the case of total power data. </li>
   * </ul>
   *
   */ 

  class SDMDataObject {
    friend class SDMDataObjectReader;
    friend class SDMDataObjectWriter;
    friend class HeaderParser;
    friend class SDMDataSubset;
    friend class CorrSubsetHeaderParser;

  public:

    // SDMDataObject::SpectralWindow:: declarations
    //
    /**
     * A class to describe a spectral window in use during an observation.
     * An instance of this class collects the following informations :
     * <ul>
     * <li> The description of polarization products for the interferometric data , if any (crossPolProducts()). </li>
     * <li> The description of polarization products for the single dish data , if any (sdPolProducts()). </li>
     * <li> The scale factor used in the representation of spectral visibilities, if any (scaleFactor()).</li>
     * <li> The number of spectral points (numSpectralPoint()).</li>
     * <li> The number of bins (numBin()). </li>
     * </ul>
     *
     */
    class SpectralWindow {
      friend class SDMDataObject;
      friend class DataStruct;
      friend class Baseband;
      friend class HeaderParser;

    private:
      vector<StokesParameter> crossPolProducts_;
      vector<StokesParameter> sdPolProducts_;
      float scaleFactor_;
      unsigned int numSpectralPoint_;
      unsigned int numBin_;
      NetSideband sideband_;
      string strSw_;
      string strImage_;
      void strSw(const string& s);
      const string & strSw() const;
      void strImage(const string& s);
      const string & strImage() const;

      const SDMDataObject* owner_;

      void owner(const SDMDataObject* o);

    public:
      /**
       * An empty constructor.
       * 
       * @note This constructor should never be used.
       */
      SpectralWindow();


      /**
       * The destructor.
       */
      virtual ~SpectralWindow();

      /**
       * A constructor of SpectralWindow to use when there are only interferometric data (correlationMode == CROSS_ONLY).
       */
      SpectralWindow(const vector<StokesParameter>& crossPolProducts,
		     float scaleFactor,
		     unsigned int numSpectralPoint,
		     unsigned int numBin,
		     NetSideband sideband);

      /**
       * A constructor of SpectralWindow to use when there are only single dish data (correlationMode == AUTO_ONLY).
       */
      SpectralWindow(const vector<StokesParameter>& sdPolProducts,
		     unsigned int numSpectralPoint,
		     unsigned numBin,
		     NetSideband sideband);

      /**
       * A constructor of SpectralWindow to use when there are both single dish and  interferometric data (correlationMode == CROSS_AND_AUTO).
       */
      SpectralWindow(const vector<StokesParameter>& crossPolProducts,
		     const vector<StokesParameter>& sdPolProduct,
		     float scaleFactor,
		     unsigned int numSpectralPoint,
		     unsigned int numBin,
		     NetSideband sideband);

      /**
       * Returns the vector of polarization products (for the interferometric data).
       * @return a reference to a vector of StokesParameter.
       *
       * @throw SDMDataObjectException when correlationMode() == AUTO_ONLY.
       */
      const vector<StokesParameter>& crossPolProducts() const;
      //void crossPolProducts(const vector<StokesParameter>& value);

      /**
       * Returns the vector of polarization products (for the single dish data).
       * @return a reference to a vector of StokesParameter.
       *
       * @throw SDMDataObjectException when correlationMode() == CROSS_ONLY.
       */
      const vector<StokesParameter>& sdPolProducts() const;
      //void sdPolProducts(const vector<StokesParameter>& value);

      /**
       * Returns the scale factor.
       * @return a float.
       *
       * @throw SDMDataObjectException when correlationMode() == AUTO_ONLY.
       * 
       */
      float scaleFactor() const;
      //void scaleFactor(float value);

      /**
       * Returns the number of spectral points.
       * @return an unsigned int.
       */
      unsigned int numSpectralPoint() const;
      //void numSpectralPoint(unsigned int value);

      /**
       * Returns the number of bins.
       * For ALMA this is the number of steps in a switch-cycle and a value of 1 means no switching cycle.
       * @return an unsigned int.
       *
       */
      unsigned int numBin() const;
      //void numBin(unsigned int value);

      /**
       * Returns the netsideband.
       *
       * @return a NetSidebandMod::NetSideband
       */ 
      NetSidebandMod::NetSideband sideband() const;

    }; // SpectralWindow::


    // SDMDataObject::Baseband:: declarations
    //
    /**
     * A class to describe a baseband in use during an observation.
     * 
     * An instance of this class collects the following informations:
     * <ul>
     * <li> possibly a name (ref()).
     * <li> the description of one or many spectral windows (spectralWindows()).
     * </ul>
     *
     */
    class Baseband {
      friend class SDMDataObject;
      friend class DataStruct;
      friend class HeaderParser;

    private:
      BasebandName name_;
      vector<SpectralWindow> spectralWindows_;

      const SDMDataObject* owner_;

      void owner(const SDMDataObject* o);

    public:
      /**
       * An empty constructor.
       *
       * @note This constructor should never be used.
       */
      Baseband();

      /**
       * The destructor.
       */
      ~Baseband();

      /**
       * The constructor of Baseband.
       */
      Baseband(BasebandName name, const vector<SpectralWindow>& spectralWindows);

      /**
       * Returns the name of the baseband.
       * @return a BasebandName value.
       */
      BasebandName name() const;
      //void ref(BasebandName value);

      /**
       * Returns the spectral windows of this baseband.
       * @return a reference to a vector of SpectralWindow.
       */
      const vector<SpectralWindow>& spectralWindows() const;
      void spectralWindows(const vector<SpectralWindow>& value);
    }; // Baseband::


    // SDMDataObject::BinaryPart:: declarations
    //
    /**
     * A class to describe binary data, i.e. :
     * <ul>
     * <li> flags. </li>
     * <li> actualTimes. </li>
     * <li> actualDurations. </li>
     * <li> zeroLags. </li>
     * <li> crossData. </li>
     * <li> autoData. </li>
     * </ul>
     *
     * An instance of this class collects information about :
     * <ul>
     * <li> The size of the binary part i.e. the number of values that it contains. (e.g. the number of float numbers in the case of autoData). (size()) </li>
     * <li> The ordered list of the names of axis of the hierarchical structure which contains these binary data. (axes())</li>
     * </ul>
     */
    class BinaryPart {
      friend class DataStruct;
      friend class SDMDataObject;
      friend class HeaderParser;
      friend class SDMDataObjectWriter;

    protected:
      unsigned int size_;
      vector<AxisName> axes_;

      const SDMDataObject* owner_;
    
      void owner(const SDMDataObject* o);
    public:

      /**
       * An empty constructor.
       */
      BinaryPart();

      /**
       * The destructor.
       */
      virtual ~BinaryPart();

      /**
       * The full constructor.
       */
      BinaryPart( unsigned int size,
		  const vector<AxisName>& axes);
      
      /**
       * Returns the size of a binary attachment as a <b>number of values</b> (e.g. a number of long long for the actualDurations, or 
       * a number of short int for crossData when they are encoded as short ints).
       */
      virtual unsigned int size() const; 
      //      virtual void size (unsigned int value); 
      
      /**
       * Returns a vector of axis names.
       * The vector contains the list of axis names relevant for this binary part ordered from the less (1st) to the more (last)
       * rapidly varying.
       * @return a vector of AxisName.
       */
      virtual const vector<AxisName>& axes() const ; 
      //      virtual void axes (const vector<AxisName>& axes); 
    }; // BinaryPart::

    /**
     * A subclass of binaryPart to describe the autodata.
     *
     * The description of autodata contains one extra information stored in a boolean which indicates
     * if the auto data are normalized or not. 
     *
     */
    class AutoDataBinaryPart : public BinaryPart {
      friend class DataStruct;
      friend class SDMDataObject;
      friend class HeaderParser;
      friend class SDMDataObjectWriter;
      
    protected:
      bool normalized_;

    public:
      /**
       * An empty constructor.
       */
      AutoDataBinaryPart();
      
      /**
       * The destructor.
       */
      ~AutoDataBinaryPart();
      
      /**
       * The full constructor.
       */
      AutoDataBinaryPart(unsigned int size,
			 const vector<AxisName>& axes,
			 bool normalized);
      
      /**
       * Returns true (resp.false) if auto data are normalized (resp. not normalized).
       * @return a boolean.
       */
      virtual bool normalized() const;
      
    }; // AutoDataBinaryPart
    
    /**
     * A subclass of binaryPart to describe the zeroLags.
     *
     * The description of zeroLags contains one extra information stored in a CorrelatorType field which indicates
     * the type of correlator which has produced them.
     *
     */
    class ZeroLagsBinaryPart : public BinaryPart {
      friend class DataStruct;
      friend class SDMDataObject;
      friend class HeaderParser;
      friend class SDMDataObjectWriter;

    protected:
      CorrelatorType correlatorType_;
      
    public:
      
      /**
       * An empty constructor.
       */
      ZeroLagsBinaryPart() ;

      /**
       * The destructor.
       */
      ~ZeroLagsBinaryPart();

      /**
       * The full constructor.
       */
      ZeroLagsBinaryPart(unsigned int size,
			 const vector<AxisName>& axes,
			 CorrelatorType correlatorType);

      /**
       * Returns the correlator type.
       * @return a value of the enumeration CorrelatorType.
       */
      virtual CorrelatorType correlatorType() const;
    };

    // SDMDataObject::DataStruct:: declarations
    //
    /**
     * A class to represent the structure of binary data.
     * An instance of this class collects informations :
     * <ul>
     * <li> about atmospheric phase correction when relevant (correlationMode != AUTO_ONLY) (apc()).</li>
     * <li> about basebands (basebands()).</li>
     * <li> about binary data (flags(), actualTimes(), actualDurations(), zeroLags(), crossData(), autoData()) .</li>
     * </ul>
     */
    class DataStruct {
      friend class SDMDataObject;
      friend class HeaderParser;
      friend class SDMDataObjectWriter;

    public:
      /**
       * An empty constructor.
       *
       * @note This constructor should never be used.
       */
      DataStruct();

      /**
       * The destructor.
       */
      virtual ~DataStruct();

      /**
       * A full constructor.
       *
       * @param apc a vector of AtmPhaseCorrection. If apc is not relevant pass an empty vector.
       * @param basebands a vector of Baseband.
       * @param flags a BinaryPart object describing the flags. If flags is not relevant 
       * pass an empty BinaryPart object.
       * @param actualTimes  a BinaryPart object describing the actual times. If actualTimes is not relevant 
       * pass an empty BinaryPart object.
       * @param actualDurations  a BinaryPart object describing the actual durations. If actualDurations is not relevant 
       * pass an empty BinaryPart object.
       * @param zeroLags  a ZeroLagsBinaryPart object describing the zero lags. If zeroLags is not relevant pass an empty 
       * ZeroLagsBinaryPart object.
       * @param crossData a BinaryPart object describing the cross data. If crossData is not relevant pass an empty 
       * BinaryPart object.
       * @param autoData an AutoDataBinaryPart object describing the auto data. If autoData is not relevant pass an empty 
       * AutoDataBinaryPart object.
       *
       */
      DataStruct(const vector<AtmPhaseCorrection>& apc,
		 const vector<Baseband>& basebands,
		 const BinaryPart& flags,
		 const BinaryPart& actualTimes,
		 const BinaryPart& actualDurations,
		 const ZeroLagsBinaryPart& zeroLags,
		 const BinaryPart& crossData,
		 const AutoDataBinaryPart& autoData);
      
      
      /**
       * Returns the radiometric atmospheric phase correction codes.
       * @return a vector of AtmPhaseCorrectionMod::AtmPhaseCorrection.
       */       
      const vector<AtmPhaseCorrection>& apc() const;
      //      void apc(const vector<AtmPhaseCorrection>& value);

      /**
       * Returns the vector of basebands.
       * @return a reference to a vector of Baseband.
       */
      const vector<Baseband>& basebands() const;
      //      void basebands(const vector<Baseband>& value);
      
      /**
       * Returns the description the flags attachment.
       * @return a reference to a BinaryPart value.
       */
      const BinaryPart& flags() const;
      //      void flags(const BinaryPart& value);

      /**
       * Returns the description the actualTimes attachment.
       * @return a reference to a BinaryPart value.
       */
      const BinaryPart& actualTimes() const;
      //      void actualTimes(const BinaryPart& value);
 

      /**
       * Returns the description the actualDurations attachment.
       * @return a reference to a BinaryPart value.
       */
      const BinaryPart& actualDurations() const;
      //      void actualDurations(const BinaryPart& value);


      /**
       * Returns the description the zeroLags attachment.
       * @return a reference to a ZeroLagsBinaryPart value.
       */
      const ZeroLagsBinaryPart& zeroLags() const;
      //      void zeroLags(const BinaryPart& value);


      /**
       * Returns the description the crossData attachment.
       * @return a reference to a BinaryPart value.
       */
      const BinaryPart& crossData() const;
      //      void crossData(const BinaryPart& value);


      /**
       * Returns the description the autoData attachment.
       * @return a reference to an AutoDataBinaryPart value.
       */
      const AutoDataBinaryPart& autoData() const;
      //      void autoData(const BinaryPart& value);

      /**
       * Defines an association "spw1 has image spw2" between two spectral windows spw1 and spw2.
       * Assuming that a spectral window can be located by a pair (ibb, ispw) of integer, where ibb denotes the index of the
       * baseband this spectral window belongs to, and ispw its index in that baseband, this method declares that
       * the spectral window with coordinates (ibb, ispw1) has the spectral window with coordinates (ibb, ispw2) as an image.
       *
       * @param ibb  1st common coordinate of spw1 and spw2
       * @param ispw1 2nd coordinate of spw1.
       * @param ispw2 2nd coordinate of spw2.
       *
       * @throw SDMDataObjectException
       */
      void imageSPW(unsigned int ibb,
		    unsigned int ispw1,
		    unsigned int ispw2);

      /**
       * Returns a pointer to the spectral window image of a spectral window.
       * Assuming that a spectral window spw can be located by a pair (ibb, ispw) of integer, where ibb denotes the index of the
       * baseband this spectral window belongs to, and ispw its index in that baseband, this method returns
       * a pointer to the spectral window defined as the image of spw. If spw has no image defined then null is returned.
       *
       * @param ibb  1st coordinate of spw
       * @param ispw 2nd coordinate of spw
       *
       * @throw SDMDataObjectException
       */
      const SpectralWindow* imageSPW(unsigned int ibb,
				     unsigned int ispw) const;

      /**
       * Defines an association "spw1 is image of spw2" between two spectral windows spw1 and spw2.
       * Assuming that a spectral window can be located by a pair (ibb, ispw) of integer, where ibb denotes the index of the
       * baseband this spectral window belongs to, and ispw its index in that baseband, this method declares that
       * the spectral window with coordinates (ibb1, ispw1) is the image of the spectral window with coordinates (ibb2, ispw2).
       *
       * @param ibb  1st common coordinate of spw1
       * @param ispw1 2nd coordinate of spw1.
       * @param ispw2 2nd coordinate of spw2.
       *
       * @throw SDMDataObjectException
       */
      void imageOfSPW(unsigned int ibb,
		      unsigned int ispw1,
		      unsigned int ispw2);
      

      /**
       * Returns a pointer to the spectral window whom the spectral window passed in argument is the image.
       * Assuming that a spectral window spw can be located by a pair (ibb, ispw) of integer, where ibb denotes the index of the
       * baseband this spectral window belongs to, and ispw its index in that baseband, this method returns
       * a pointer to the spectral window defined as the image of spw. If spw is not an image then null is returned.
       *
       * @param ibb  1st coordinate of spw
       * @param ispw 2nd coordinate of spw
       *
       * @throw SDMDataObjectException
       */
      const SpectralWindow* imageOfSPW(unsigned int ibb,
				       unsigned int ispw) const;
      

    private:
      vector<AtmPhaseCorrection> apc_;
      vector<Baseband> basebands_;
      BinaryPart flags_;
      BinaryPart actualTimes_;
      BinaryPart actualDurations_;
      ZeroLagsBinaryPart zeroLags_;
      BinaryPart crossData_;
      AutoDataBinaryPart autoData_;

      const SDMDataObject* owner_;
      void owner(const SDMDataObject* o);

      map<string, string> imageSPW_;
      map<string, string> imageOfSPW_;

      void checkCoordinate(unsigned int ibb, unsigned int ispw) const;
      bool associatedSPW(unsigned int ibb,
			 unsigned int ispw1,
			 unsigned int ispw2);

      void divorceSPW(unsigned int ibb, unsigned int ispw)   ;

    }; //DataStruct::

    SDMDataObject();
    SDMDataObject(unsigned long long startTime,
		  const string& dataOID,
		  unsigned int dimensionality,
		  const string& execBlockUID,
		  unsigned int execBlockNum,
		  unsigned int scanNum,
		  unsigned int subscanNum,
		  unsigned int numAntenna,
		  CorrelationMode correlatorMode,
		  const SDMDataObject::DataStruct& dataStruct);

    SDMDataObject(unsigned long long startTime,
		  const string& dataOID,
		  unsigned int dimensionality,
		  unsigned int numTime,
		  const string& execBlockUID,
		  unsigned int execBlockNum,
		  unsigned int scanNum,
		  unsigned int subscanNum,
		  unsigned int numAntenna,
		  const SDMDataObject::DataStruct& dataStruct);

    /**
     * Returns the title of the SDMDataObject.
     * @return a string.
     */
    string title() const;
    void title(const string& value) ;

    /**
     * Returns the start time.
     * @return a long long.
     */
    unsigned long long startTime() const;
    void startTime(unsigned long long value);    

    /**
     * Returns the number of (sub) integrations.
     * @return an unsigned int.
     *
     */
    unsigned int numTime() const;
    void numTime(unsigned int value);

    /**
     * Returns the dataOID.
     * @return a string.
     */
    string dataOID() const;
    void dataOID(const string& value);


    /**
     * Returns the UID of the ExecBlock.
     * @return a string.
     */
    string execBlockUID() const;
    void execBlockUID(const string& value);

    /**
     * Returns the number of the ExecBlock.
     * @return an unsigned int.
     */
    unsigned int execBlockNum() const;
    void execBlockNum (unsigned int value );

    /**
     * Returns the number of the scan.
     * @return an unsigned int.
     */
    unsigned int scanNum() const;
    void scanNum( unsigned int value);

    /**
     * Returns the number of the subscan.
     * @return an unsigned int.
     */
    unsigned int subscanNum() const;
    void subscanNum(int value);

    /**
     * Returns the project path.
     * The project path is a string of the form "/<s>execBlockNum</s>/<s>scanNum</s>/<s>subscanNum</s>"
     */
    string projectPath() const;

    /**
     * Returns the projects paths of all the data subsets present in this SDMDataObject
     *
     * @return a vector of string.
     */
    vector<string> projectPaths() const;

    /**
     * Returns the number of antenna.
     * @return an unsigned int.
     */
    unsigned int numAntenna() const;
    void numAntenna (unsigned int value);

    /**
     * Returns the correlation mode.
     * @return a value from enumeration CorrelationMode.
     */
    CorrelationMode correlationMode() const;


    /**
     * Returns the spectral resolution.
     * Due to this optional nature, the spectral resolution type is not returned directly as a literal of the enumeration
     * SpectralResolutionType, but as an instance of the class OptionalSpectralResolutionType. This instance can be queried to
     * check if the spectral resolution type information is present and if it is its value as an SpectralResolutionType literal.
     *
     * @return a value from enumeration SpectralResolutionType.
     */
    OptionalSpectralResolutionType spectralResolutionType() const;

    /**
     * Returns the processor type.
     * @return a value from the enumeration ProcessorType.
     */
    ProcessorType processorType() const;


    /**
     * Returns the correlator type.
     *
     * @return a value from the enumeration CorrelatorType if processorType == CORRELATOR else an SDMDataObjectException is thrown.
     * @throw SDMDataException
     */
    CorrelatorType correlatorType() const;

    /**
     * Returns true if the data are total power data and false otherwise.
     * @return a bool.
     *
     * @note data are considered as total power data if CorrelationMode == AUTO_ONLY && SpectralResolutionType == BASEBAND_WIDE.
     */
    bool isTP() const ;


    /**
     * Returns true if the data are correlator data and false otherwise.
     * @return a bool.
     *
     * @note data are considered as correlator data if SpectralResolutionType != BASEBAND_WIDE.
     */
    bool isCorrelation() const;



    /**
     * Returns the structure of the data.
     * @returns a reference to a DataStruct.
     */
    const DataStruct& dataStruct() const;
    void dataStruct(const DataStruct& dataStruct);


    /**
     * Returns the binary data as a sequence of integrations.
     * This method must be used only when the SDMDataObject contains correlator data (i.e. isCorrelation() == true)
     * @return a reference to a vector of SDMDataSubset.
     */
    const vector<SDMDataSubset>& corrDataSubsets() const;
    void corrDataSubsets(const vector<SDMDataSubset>& value);

    /**
     * Returns a reference to a SDMDataSubset given its projectPath.
     * 
     * @param projectPath a string containing the project path of the SDMDataSubset.
     * @return a pointer to an SDMDataSubset.
     *
     * @throw SDMDataObjectException.
     */
    const SDMDataSubset& sdmDataSubset(const string& projectPath) const;


    /**
     * Returns true if the observation has been aborted.
     * This method must be used on an SDMDataObject containing correlator data,otherwise a SDMDataObjectException is thrown.
     *
     * @return a bool.
     * 
     * @throw SDMDataObjectException.
     */
    bool aborted();

    /**
     * Returns the time, as an unsigned long long, at which the observation has been aborted.
     * The returned value is significant only if the observation has been aborted, therefore the method must always be used in 
     * conjuction with the aborted method. This method must be used on an SDMDataObject containing correlator data,
     * otherwise a SDMDataObjectException is thrown.
     *
     * @return an unsigned long long.
     *
     * @throw SDMDataObjectException
     */
    unsigned long long abortTime();



    /**
     * Returns the reason, as a string, why the observation has been aborted.
     * The returned value is significant only if the observation has been aborted, therefore the method must always be used in 
     * conjuction with the aborted method. This method must be used on an SDMDataObject containing correlator data,
     * otherwise a SDMDataObjectException is thrown.
     *
     * @return a string.
     *
     * @throw SDMDataObjectException
     */
    string abortReason();


    /**
     * Returns the binary data for a subscan.
     * This method must be used only when the SDMDataObject contains total power data (i.e. isTP() == true)
     * @return a reference to an SDMDataSubset.
     */
    const SDMDataSubset& tpDataSubset() const;
    void tpDataSubset(const SDMDataSubset& value);

    /**
     * Returns a string representation of the global header of this SDMDataObject.
     * @return a string.
     */
    string toString() const;

    /**
     * Makes this SDMDataObject unusable.
     * After a call to this method any request to this instance will generate an exception.
     */
    void done();

  private:

    static vector<string> correlationModeRefs;
    static vector<string> axes;
    static vector<string> types;
    static vector<string> apcs;

    const static bool _init;
    static bool init();

    // Is the SDMDataObject actually properly filled with valid data.
    bool valid_; 

    // Global header variables.
    string title_;

    const ByteOrder* byteOrder_;

    int schemaVersion_;
    
    long long startTime_;

    string dataOID_;

    unsigned int dimensionality_;

    unsigned int numTime_;

    string execBlockUID_;

    unsigned int execBlockNum_;

    unsigned int scanNum_;

    unsigned int subscanNum_;

    unsigned int numAntenna_;

    CorrelationMode correlationMode_;

    OptionalSpectralResolutionType spectralResolutionType_;

    ProcessorType processorType_;

    DataStruct dataStruct_;

    map<string, unsigned int> str2index_;
    
    vector<SDMDataSubset> dataSubsets_;

    bool aborted_;

    unsigned long long int abortTime_;
    string   abortReason_;

    void append(const SDMDataSubset& value);


    /**
     * Returns the "dimensionality" of this SDMDataObject.
     * A value of 
     * <ul>
     * <li> 0 corresponds to total power data.</li>
     * <li> 1 corresponds to correlator data.</li>
     * </ul>
     */
    unsigned int dimensionality() const;
    void dimensionality( unsigned int value );


    /**
     * Declares itself as the owner of all its parts.
     */
    void owns();

 
    /**
     * Returns an XML representation of the global header of this SDMDataObject.
     * @return a string.
     */
    string toXML() ;
    void toXML(const BinaryPart& binaryPart, const string& elementName,  ostringstream& oss) const;
    void toXML(const AutoDataBinaryPart& autoDataBinaryPart, const string& elementName,  ostringstream& oss) const;
    void toXML(const ZeroLagsBinaryPart& zeroLagsBinaryPart, const string& elementName,  ostringstream& oss) const;
    void spectralWindowsToXML(const vector<Baseband>& basebands, unsigned int ibb,  ostringstream& oss) const;
    void basebandsToXML(ostringstream& oss) const; 
    void dataStructToXML(ostringstream& oss) ;

    void updateIdImageSPW();

    const static regex  SPWID;

  };
  // SDMDataObject::

  // SDMDataSubset:: declarations
  //
  /**
   * An SDMDataSubset is a class to represent the binary data stored during :
   * <ul>
   * <li> one integration in the case of Correlator data. </li>
   * <li> one subscan in the case of Total Power Data. </li>
   * </ul>
   */
  class SDMDataSubset {
    friend class SDMDataObject;
    friend class SDMDataObjectReader;
    friend class SDMDataObjectWriter;
    friend class CorrSubsetHeaderParser;
    friend class TPSubsetHeaderParser;

  public:
  
    /*
     * An empty constructor.
     */
    SDMDataSubset(SDMDataObject* owner= 0);

    SDMDataSubset(SDMDataObject* owner,
		  unsigned long long time,
		  unsigned long long interval,
		  const vector<float>& autoData);

    virtual ~SDMDataSubset();

    const SDMDataObject* owner() const;

    unsigned int integrationNum() const;
    
    unsigned int subintegrationNum() const;    

    /**
     * Returns the project path of this SDMDataSubset.
     * the project path is a string of the form :
     * <ul>
     * <li>/<execblock-number>/<scan-number>/<subscan-number>/<integration-number> in the case of correlator data. </li>
     * <li>/<execblock-number>/<scan-number>/<subscan-number> in the case of total power data. </li>
     * </ul>
     */
    string projectPath() const;
  
    /**
     * Returns the midpoint of :
     * <ul>
     * <li> the integration in the case of correlator data. </li>
     * <li> the subscan in the case of total power data. </li>
     * </ul>
     */
    unsigned long long time() const;
  
    /**
     * Returns the duration of the subscan in the case of total power data.
     * @note not relevant in the case of correlator data.
     */
    unsigned long long interval() const;


    /**
     * Returns a description of this SDMDataSubset.
     *
     */
    string toString(unsigned int N = 10 )const ;

    void binAttachToXML(const string& name, ostringstream& oss);
    void tpBinAttachToXML(ostringstream& oss);
    void corrBinAttachToXML(ostringstream& oss);

    void toXML(ostringstream& oss) const;

    /**
     * Returns an XML representation of this SDMDataSubset.
     *
     */
    string toXML();


    /**
     *
     * Set ptr to the adress of the array of ActualDurations and returns the number of actualDurations.
     * @param ptr a reference to a pointer on long long.
     * @return the number of ActualDurations.
     *
     */
    unsigned long int actualDurations(const ACTUALDURATIONSTYPE* & ptr) const;

    /**
     *
     * Set ptr to the adress of the array of ActualTimes and returns the number of ActualTimes.
     * @param ptr a reference to a pointer on long long.
     * @return the number of ActualTimes.
     *
     */
    unsigned long int actualTimes(const ACTUALTIMESTYPE* & ptr) const;

    /**
     *
     * Set ptr to the adress of the array of AutoData and returns the number of AutoData.
     * @param ptr a reference to a pointer on float.
     * @return the number of AutoData.
     *
     */
    unsigned long int autoData(const AUTODATATYPE* & ptr) const ;

    /**
     *
     * Set ptr to the adress of the array of short int CrossData and returns the number of short int CrossData 
     * @param ptr a reference to a pointer on short int.
     * @return the number of short int CrossData values.
     *
     */
    unsigned long int crossData(const SHORTCROSSDATATYPE* & ptr) const;


    /**
     *
     * Set ptr to the adress of the array of int CrossData and Returns the number of long int CrossData.
     * @param ptr a reference to a pointer on long int.
     * @return the number of long int CrossData values.
     *
     */
    unsigned long int crossData(const INTCROSSDATATYPE* & ptr) const;

    /**
     *
     * Set ptr to the adress of the array of float CrossData and Returns the number of long int CrossData.
     * @param ptr a reference to a pointer on float.
     * @return the number of float CrossData values.
     *
     */
    unsigned long int crossData(const FLOATCROSSDATATYPE* & ptr) const;

    /**
     * Return the type of cross data values.
     * @return a value of the enumeration PrimitiveDataTypeMod::PrimitiveDataType.
     *
     */
    PrimitiveDataType crossDataType() const;

    void crossDataType(PrimitiveDataType value);

    /**
     *
     * Set ptr to the adress of the array of flags and returns the number of flags. 
     * @param ptr a reference to a pointer on unsigned long int.
     * @return the number of flags.
     *
     */
    unsigned long int flags(const FLAGSTYPE* &ptr) const;

    /**
     *
     * Set ptr to the adress of the array of ZeroLags and returns the number of ZeroLags.
     * @param ptr a reference to a pointer on float.
     * @return the number of autoData.
     *
     */
    unsigned long int zeroLags(const ZEROLAGSTYPE* & ptr) const;

    /**
     * Returns true if and only if this corresponds to an aborted [sub]integration.
     * @return a bool.
     */
    bool aborted() const ; 

    /**
     * Returns the time when the [sub]integration has been declared aborted.
     * @return an unsigned long long.
     * @note Of course, the returned value is not meaningful if aborted() returns false.
     */
    unsigned long long abortTime() const; 

    /**
     * Returns the reason why the [sub]integration was aborted.
     * @return a string.
     * @note Of course, the returned value is not meaningful if aborted() returns false.
     */
    string abortReason() const;



  private:
    SDMDataObject* owner_;
    unsigned int integrationNum_;
    unsigned int subintegrationNum_;
    CorrelationMode ref_;
    unsigned long long time_;
    unsigned long long interval_;
    string dataStruct_;
    string flagsREF_;
    string actualTimesREF_;
    string actualDurationsREF_;
    string zeroLagsREF_;
    string crossDataREF_;
    PrimitiveDataType crossDataType_;
    string autoDataREF_;

    const ACTUALTIMESTYPE * actualTimes_;
    unsigned long int nActualTimes_;
    const ACTUALDURATIONSTYPE * actualDurations_;
    unsigned long int nActualDurations_;
    const ZEROLAGSTYPE* zeroLags_;
    unsigned long int nZeroLags_;
    const FLAGSTYPE* flags_;    
    unsigned long int nFlags_;
    const INTCROSSDATATYPE* longCrossData_;
    const SHORTCROSSDATATYPE* shortCrossData_;
    const FLOATCROSSDATATYPE* floatCrossData_;
    unsigned long int nCrossData_;
    const AUTODATATYPE* autoData_;
    unsigned long int nAutoData_;

    string xsiType() const;

    bool aborted_;
    unsigned long long int abortTime_;
    string   abortReason_;

  };
  // SDMDataSubset:: declarations


  // Utils:: declarations
  //
  class Utils {
  public:
    static void invalidCall(const string & methodName, const SDMDataObject* sdmDataObject);
    static string quote(const string& s);
    static string quote(bool b);
    static string quote(int i);
    static string quote(unsigned int i);
    static string quote(long long l);
    static string quote(float f);
    static string quote(const set<string>& s);
    static string quote(const vector<string>& s);
    template<class Enum, class EnumHelper> static string quote(Enum l) {
      ostringstream oss;
      oss << "\"";
      oss << EnumHelper::name(l);
      oss << "\"";
      return oss.str();
    }

    template<class Enum, class EnumHelper> static string quote(const vector<Enum>& v_l) {
      ostringstream oss;
      oss << "\"";

      if (v_l.size() > 0)
	oss << EnumHelper::name(v_l.at(0));

      for (unsigned int i = 1; i < v_l.size(); i++)
	oss << " " << EnumHelper::name(v_l.at(i));
      oss << "\"";
      return oss.str();
    }

    template<class Enum, class EnumHelper> static string toString(Enum l) {
      ostringstream oss;
      oss << EnumHelper::name(l);
      return oss.str();
    }

    template<class Enum, class EnumHelper> static string toString(const vector<Enum>& v_l) {
      ostringstream oss;

      if (v_l.size() > 0)
	oss << EnumHelper::name(v_l.at(0));

      for (unsigned int i = 1; i < v_l.size(); i++)
	oss << " " << EnumHelper::name(v_l.at(i));
      return oss.str();
    }

    /**
     * A generic utility to return a vector of <Enum> out of a string of <Enum> literals separated by space characters.
     */
    template<class Enum, class EnumHelper> static vector<Enum> enumvec(const string& strliterals) {
      vector<Enum> result;
      
      string strliteral;
      istringstream iss(strliterals);
      
      while (iss >> strliteral)
	result.push_back(EnumHelper::literal(strliteral));
      
      return result;
    }


    static void toXML(const string& elementName, int value, ostringstream& oss);
    static void toXML(const string& elementName, unsigned int value, ostringstream& oss);
    static void toXML(const string& elementName, long long value, ostringstream& oss);
    static void toXML(const string& elementName, unsigned long long value, ostringstream& oss);
    static void oXML(const string& elementName, ostringstream& oss);
    static void cXML(const string& elementName, ostringstream& oss);
    
    /**
     * A generic utility to XML'ize a literal as the content of an XML element.
     *
     * @param elementName the name of the XML element to contain the Enum value.
     * @param literal the value of Enum to output.
     * @param oss a reference to the output string stream where the XML is written.
     *
     */
    template<class Enum, class EnumHelper> static void toXML(const string& elementName, Enum value, ostringstream& oss) {
      oss << "<" << elementName << ">" << EnumHelper::name(value) << "</" << elementName << ">" << endl;      
    }

#define QUOTE    Utils::quote
#define TOSTRING Utils::toString
#define TOXML    Utils::toXML
#define OXML     Utils::oXML
#define CXML     Utils::cXML
  };
}
#endif // SDMDataObject_CLASS

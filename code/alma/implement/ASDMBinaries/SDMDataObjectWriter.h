#ifndef SDMDataObjectWriter_CLASS
#define SDMDataObjectWriter_CLASS

#include <string>
#include <map>
#include <set>
#include <vector>
#include <bitset>
//#include <boost/regex.hpp> 

#include <iostream>
#include <fstream>
#include <sstream>

#include "SDMDataObject.h"

#include "CAtmPhaseCorrection.h"
#include "CCorrelationMode.h"
#include "CCorrelatorType.h"
#include "CNetSideband.h"
#include "CProcessorType.h"


using namespace std;

using namespace AtmPhaseCorrectionMod;
using namespace CorrelationModeMod;
using namespace CorrelatorTypeMod;
using namespace NetSidebandMod;
using namespace ProcessorTypeMod;

namespace asdmbinaries {
  /**
   * A class to represent an exception thrown while writing a MIME message containing ALMA binary data.
   */ 
  class SDMDataObjectWriterException {
    
  public:
    /**
     * An empty contructor.
     */
    SDMDataObjectWriterException();

    /**
     * A constructor with a message associated with the exception.
     * @param m a string containing the message.
     */
    SDMDataObjectWriterException(const string& m);
    
    /**
     * The destructor.
     */
    virtual ~SDMDataObjectWriterException();

    /**
     * Returns the message associated to this exception.
     * @return a string.
     */
    string getMessage() const;
    
  protected:
    string message;
    
  };

  inline SDMDataObjectWriterException::SDMDataObjectWriterException() : message ("SDMDataObjectWritererException") {}
  inline SDMDataObjectWriterException::SDMDataObjectWriterException(const string& m) : message(m) {}
  inline SDMDataObjectWriterException::~SDMDataObjectWriterException() {}
  inline string SDMDataObjectWriterException::getMessage() const {
    return "SDMDataObjectWriterException : " + message;
  }

  /** 
   * A general class to write MIME messages containing ALMA binary data.
   *
   * An instance of this class can be used to write :
   * <ul>
   * <li> Correlator data :
   * <ul>
   * <li>Integration : full resolution data. </li>
   * <li>Subintegration : channel average data. </li>
   * </ul>
   * </li>
   * <li> Total Power data.
   * <li> Water Vapor Radiometer (WVR) data.
   * </ul>
   *
   * The MIME message is written on standard output, a disk file or a char buffer depending on the constructor
   * used to create an instance of SDMDataObjectWriter.
   *
   * @section how-to-use How to use an instance of SDMDataObjectWriter.
   * Whatever is the type of binary data (total power, WVR, integration, subintegration) to write, the following 
   * scheme must be respected when using an SDMDataObjectWriter.
   *
   * <ol>
   * <li> Instantiate an SDMDataObjectWriter by using the appropriate constructor depending on the kind of desired output 
   * (standard output, disk file, memory).</li>
   * <li> Write the binary data by using the appropriate methods depending on the kind of binary data. </li>
   * <li> Conclude the usage of this instance of SDMDataObjectWriter by calling the done method (done()).
   * </ol>
   *
   * Example:
   * 
   * @code
   * // open a disk file
   * ofstream osf("myCorrData.dat");
   *
   * // builds an SDMDataObjectWriter which will write the data in the file "myCorrData.dat".  
   * SDMDataObjectWriter sdmdow(&osf, "uid://X1/X2/X3", "ALMA Binary Data");
   * 
   * // Here produce the binary data by using one the sequences detailed below.
   * .
   * .
   * // done with sdmdow.
   * sdmdow.done();
   * 
   * // Do not forget to close the file !!
   * osf.close()
   * .
   * .
   * @endcode
   *
   * Example of a MIME message output in an ostringstream :
   *
   * @code
   * // create an osstringstream
   * ostringstream oss;
   *
   * // builds an SDMDataObjectWriter which will write the data in the string attached to oss.  
   * SDMDataObjectWriter sdmdow(&oss, "uid://X1/X2/X3", "ALMA Binary Data");
   * 
   * // Here produce the binary data by using one the sequences detailed below.
   * .
   * .
   * // done with sdmdow.
   * sdmdow.done();
   * 
   * // Do whatever you want with oss.
   * .
   * .
   * // And perhaps a good thing to erase the content of oss.
   * oss.str("");
   * .
   * .
   * @endcode
   *
   * We detail now the different valid sequences for writing binary data depending on their kind.
   * @subsection how-to-tpData How to write Total Power data.
   * One single call to the tpData() method.
   * @code
   *  sdmdow.tpData(123450000,          // startTime
   *                "uid://X123/X4/X5", // execBlockUID
   *                1,                  // execBlockNum
   *                10,                 // scanNum
   *                3,                  // subscanNum
   *                100,                // number of integrations
   *                2,                  // number of antennas
   *                basebands,          // vector of basebands.
   *                171450000,          // time 
   *                96000000,           // interval
   *                axis,               // names of axis
   *                autoData);          // total power data values.
   * @endcode
   * One may also consider to use in that order the methods tpDataHeader and addTPSubscan.
   *
   * @subsection how-to-wvrData How to write WVR data.
   * One single call to the wvrData() method.
   * @code
   *  sdmdow.wvrData("uid://X123/X4/X5", // execBlockUID,
   *                 1,                  // execBlockNum,
   *                 10,                 // scanNum,
   *                 3,                  // subscanNum,
   *                 100,                // number of time stamps (i.e. size along the TIM axis),
   *                 8,                  // number of antennas,
   *                 4,                  // number of channels,
   *                 DSB,                // NetSideband characteristic,
   *                 171450000,          // time, 
   *                 96000000,           // interval,
   *                 wvrData,            // the WVR data values,
   *                 flags);             // flags associated to the WVR data.

   * 
   * @endcode
   *
   * @subsection how-to-intData How to write integrations.
   * One single call to corrDataHeader() followed by one or more calls to addIntegration().
   * @code
   *  // Write the global header.
   *  sdmdow.corrDataHeader(123450000,          // startTime
   *                        "uid://X123/X4/X5", // execBlockUID
   *                        1,                  // execBlockNum
   *                        10,                 // scanNum
   *                        3,                  // subscanNum
   *                        2,                  // numAntenna        
   *                        correlationMode,    // the correlation mode.
   *                        spectralResolution, // the spectral resolution.
   *                        correlatorType,     // the processor (correlator) type.
   *                        dataStruct);        // the description of the structure of the binary data.
   *
   * // And write the integrations (3 in that example).
   * for (unsigned int i = 0; i < 3; i++) {
   *  .
   *  .
   *  .
   *  sdmdow.addIntegration(i+1,               // integration's index.
   *                        time,              // midpoint
   *                        interval,          // time interval
   *                        flags,             // flags binary data 
   *                        actualTimes,       // actual times binary data 	    
   *                        actualDurations,   // actual durations binary data	    
   *                        zeroLags,          // zero lags binary data		    
   *                        shortCrossData,    // cross data (can be short or int)  
   *                        autoData);         // single dish data.                 
   *  .
   *  .
   *  .
   * }  
   * @endcode
   * @subsection how-to-subintData How to write subintegrations.
   * One single call to corrDataHeader() followed by one or more calls to addSubintegration().
   * @code
   *  // Write the global header.
   *  sdmdow.corrDataHeader(123450000,          // startTime
   *                        "uid://X123/X4/X5", // execBlockUID
   *                        1,                  // execBlockNum
   *                        10,                 // scanNum
   *                        3,                  // subscanNum
   *                        2,                  // numAntenna        
   *                        correlationMode,    // the correlation mode.
   *                        spectralResolution, // the spectral resolution.
   *                        processorType,      // the processor type.
   *                        dataStruct);        // the description of the structure of the binary data.
   *
   * // And write the subintegrations (6 in that example). 
   * for (unsigned int i = 0; i < 3; i++) {
   *   for (unsigned int j = 0; j < 2; j++) {
   *    .
   *    .
   *    .
   *    sdmdow.addSubintegration(i+1,            // integration's index.
   *                             j+1,            // subintegration's index
   *                             time,           // midpoint
   *                             interval,       // time interval
   *                             flags,          // flags binary data 
   *                             actualTimes,    // actual times binary data 	    
   *                             actualDurations,// actual durations binary data	    
   *                             zeroLags,	 // zero lags binary data		    
   *                             shortCrossData, // cross data (can be short or int)  
   *                             autoData);	 // single dish data.                 
   *    .
   *    .
   *    .
   *   }
   * }  
   * @endcode
   * @subsection valid-calls Valid calls sequences.
   * The table below summarizes the valid call sequences when using an SDMDataObjectWriter. Any deviation from these
   * rules will result in an SDMDataObjectWriterException thrown.
   * <table>
   * <caption> Valid sequences of methods calls </caption>
   * <tr>
   * <th> Total Power data </th> <th> WVR data </th> <th> Integration </th> <th> subIntegration </th>
   * </tr>
   * <tr>
   * <td> ctor, tpData, done </td>
   * <td rowspan=2> ctor, wvrData, done </td> 
   * <td rowspan=2> ctor, corrDataHeader, addIntegration (one or more times), done </td>
   * <td rowspan=2> ctor, corrDataHeader, addSubintegration (one or more times), done </td>
   * </tr>
   * <tr>
   * <td> ctor, tpDataHeader, addTPSubscan, done </td>
   * </tr>
   * </table>
   * 
   */
  class SDMDataObjectWriter {
  public:
    
    /**
     * A constructor to write on standard output.
     * The MIME message will be written to the standard output.
     * @param uid a string containing the ALMA uid of the MIME message.
     * @param title a string defining the title for the binary data to be written. 
     */
    SDMDataObjectWriter(const string& uid="uid://X0/X0/X0", const string& title="ALMA Binary Data");


    /**
     * A constructor to write in a file.
     * The MIME message will be written into the file attached to the ofstream argument.
     * @param ofs an pointer to an ofstream object.
     * @param uid a string containing the ALMA uid of the MIME message.
     * @param title a string defining the title for the binary data to be written. 
     */
    SDMDataObjectWriter(ofstream* ofs, const string& uid="uid://X0/X0/X0", const string& title="ALMA Binary Data");

    /**
     * A constructor to write in memory.
     * The MIME message will be written in an ostringstream.
     * @param oss  a pointer to an ostringstream.
     * @param uid a string containing the ALMA uid of the MIME message.
     * @param title a string defining the title for the binary data to be written. 
     * @note *oss will be systematically cleared before the first write operation.
     */
    SDMDataObjectWriter(ostringstream* oss, const string& uid="uid://X0/X0/X0", const string& title="ALMA Binary Data");


    /**
     * The destructor.
     * 
     */
    virtual ~SDMDataObjectWriter();
    

    /**
     * This method must be called to conclude the activity of this SDMDataObjectWriter.
     * It completes the MIME message.
     * @note Do not forget to call it when you have finished to write your binary data !
     * @note It <b>does not</b> close the file attached to the output stream if any, this operation is left to the user.
     */
    void done();
    
    /**
     * Writes the XML global header on the MIME message stream, when binary data are Total Power data.
     * @param startime start time.
     * @param execBlockUID the UID of the exec block.
     * @param execBlockNum the index of the exec block.
     * @param scanNum the index of the scan.
     * @param subscanNum the index of the subscan.
     * @param numOfIntegrations the number of integrations in that Subscan.
     * @param numAntenna the number of antenna.
     * @param dataStruct the description of the binary data structure.
     *
     * @throws SDMDataObjectWriterException
     */
    void tpDataHeader(unsigned long long int startTime,
		      const string& execBlockUID,
		      unsigned int execBlockNum,
		      unsigned int scanNum,
		      unsigned int subscanNum,
		      unsigned int numOfIntegrations,
		      unsigned int numAntenna,
		      SDMDataObject::DataStruct& dataStruct);

    /**
     * Writes a data subset of Total Power binary data.
     * @param time time of the subscan.
     * @param interval duration of the subscan.
     * @param flags the values of flags (see note).
     * @param actualTimes the values of actualTimes (see note).
     * @param actualDurations the values of actualDurations (see note).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException 
     *
     * @note
     * This method must called only once after a call to tpDataHeader.
     **/
    void addTPSubscan(unsigned long long time,
		      unsigned long long interval,
		      const vector<unsigned long>& flags,
		      const vector<long long>& actualTimes,
		      const vector<long long>& actualDurations,
		      const vector<float>& autoData);

    /**
     * Writes the full content of Total Power data in their respective attachments (global XML header, local XML header and binary attachments)
     * on the MIME message stream.
     * @param startTime start time.
     * @param execBlockUID the UID of the exec block.
     * @param execBlockNum the index of the exec block.
     * @param scanNum the index of the scan.
     * @param subscanNum the index of the subscan.
     * @param numOfIntegrations the number of integrations in that Subscan.
     * @param numAntenna the number of antenna.
     * @param basebands a vector of Baseband describing the structure of the binary data.
     * @param time. 
     * @param interval.
     * @param flags the values of flags (see note).
     * @param actualTimes the values of actualTimes (see note).
     * @param actualDurations the values of actualDurations (see note).
     * @param autoDataAxes the ordered set of axes names for autoData.
     * @param autoData the values of autoData.
     * @param autoDataNormalized 
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * A vector with a null size can be passed when the (optional) attachment is absent.
     *
     * @note this method allows to write Total Power data in a "one-call" way. An alternate solution consists
     * in calling tpDataHeader and then addTPSubscan.
     */
    void tpData(unsigned long long int startTime,
		const string& execBlockUID,
		unsigned int execBlockNum,
		unsigned int scanNum,
		unsigned int subscanNum,
		unsigned int numOfIntegrations,
		unsigned int numAntenna,
		const vector<SDMDataObject::Baseband>& basebands,
		unsigned long long time,
		unsigned long long interval,
		const vector<AxisName>& flagsAxes,
		const vector<unsigned long>& flags,
		const vector<AxisName>& actualTimesAxes,
		const vector<long long>& actualTimes,
		const vector<AxisName>& actualDurationsAxes,
		const vector<long long>& actualDurations,
		const vector<AxisName>& autoDataAxes,
		const vector<float>& autoData);

    /**
     * Writes the full content of Total Power data in their respective attachments (global XML header, local XML header and binary attachments)
     * on the MIME message stream.
     * @param startTime start time.
     * @param execBlockUID the UID of the exec block.
     * @param execBlockNum the index of the exec block.
     * @param scanNum the index of the scan.
     * @param subscanNum the index of the subscan.
     * @param numOfIntegrations the number of integrations in that Subscan.
     * @param numAntenna the number of antenna.
     * @param basebands a vector of Baseband describing the structure of the binary data.
     * @param time 
     * @param interval
     * @param autoDataAxes the ordered set of axes names for autoData.
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     * 
     * @note 
     * This method is kept for backward compatibility reasons. It's recommanded to use the "long" version of tpData which
     * gives a full control of the optional attachments to be written.
     */
    void tpData(unsigned long long int startTime,
		const string& execBlockUID,
		unsigned int execBlockNum,
		unsigned int scanNum,
		unsigned int subscanNum,
		unsigned int numOfIntegrations,
		unsigned int numAntenna,
		const vector<SDMDataObject::Baseband>& basebands,
		unsigned long long time,
		unsigned long long interval,
		const vector<AxisName>& autoDataAxes,
		const vector<float>& autoData);

    /**
     * Writes water vapour radiometer (WVR) data in a  MIME message conform 
     * with the BDF V2 format.
     *
     * @param execBlockUID the archive uid of the exec Block,
     * @param execBlockNum the index of the exec Block,
     * @param scanNum the number of the scan,
     * @param subscanNum the number of the subscan,
     * @param numTimes the number of time stamps (i.e. size along the TIM axis), 
     * @param numAntennas the number of antennas producing WVR data,
     * @param numChannels the number of channels in WVR data,
     * @param netSideband the NetSideband characteristic attached to WVR data,
     * @param time the mid-point of the time range containing all the WVR data,
     * @param interval the duration of the time range containing all the WVR data,
     * @param wvrData the WVR data,
     * @param flags the flags associated to the WVR data.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note
     * 
     * <ul>
     * <li>see the constructor of the class and the done method for opening the 
     * output stream where the MIME message is actually written (file, memory...) 
     * and for closing it</li>
     * <li>the "startTime" element of the global header in the resulting MIME document
     * will be filled with a value equal to 'time' - 'interval'/2, </li>
     * <li> 'time' and 'interval' express a number of nanoseconds. 'time' is an MJD, </li>
     * <li> 'wvrData' and 'flags' are both 1D arrays. Nonetheless they are expected to
     * be the linearized versions of multi dimensional arrays whose axes are defined 
     * by the sequence TIM ANT SPP for the WVR data and TIM ANT for their flags, 
     * (SPP varying before ANT varying itself before TIM),</li>
     * <li> a vector of null size for the argument 'flags' will be interpreted as 'flags not available'.</li>
     * <li> a null value in at least one of the arguments 'numTimes', 'numAntennas' or 'numChannels'
     * will trigger an SDMDataObjectWriterException. </li>
     * <li> a argument 'wvrData' with a size different from 'numTimes' * 'numAntennas' * 'numChannels' 
     * will trigger an SDMDataObjectWriterException. </li>
     * <li> an argument 'flags' with a size different from 0 and different from 'numTimes' * 'numAntennas'
     * will trigger an SDMDataObjectWriterException. </li>
     * </ul>
     *
     */
    
    void wvrData (const string & execBlockUID,
		  unsigned int execBlockNum,
		  unsigned int scanNum,
		  unsigned int subscanNum,
		  unsigned int numTimes,
		  unsigned int numAntennas,
		  unsigned int numChannels,
		  NetSideband  netSideband,
		  unsigned long long time,
		  unsigned long long interval,
		  const vector<float>& wvrData,
		  const vector<unsigned long>& flags);
  
  
    /**
     * Writes the XML global header on the MIME message stream, when binary data are (sub)integrations
     * produced by the correlator.
     * @param startime start time.
     * @param execBlockUID the UID of the exec block.
     * @param execBlockNum the index of the exec block.
     * @param scanNum the index of the scan.
     * @param subscanNum the index of the subscan.
     * @param numAntenna the number of antenna.
     * @param correlationMode the correlation mode code.
     * @param spectralResolution the spectral resolution code.
     * @param dataStruct the description of the binary data structure.
     *
     * @throws SDMDataObjectWriterException
     */
    void corrDataHeader(unsigned long long startime,
			const string& execBlockUID,
			unsigned int execBlockNum,
			unsigned int scanNum,
			unsigned int subscanNum,
			unsigned int numAntenna,
			CorrelationMode correlationMode,
			const OptionalSpectralResolutionType& spectralResolutionType,
			SDMDataObject::DataStruct& dataStruct);


    /**
     * Writes an integration (local header + binary attachment) on the MIME message stream.
     * @param integrationNum the index (1 based) of the integration.
     * @param time time of the integration.
     * @param interval interval of the integration.
     * @param flags the values of flags.
     * @param actualTimes the values of actualTimes.
     * @param actualDurations the values of actualDurations.
     * @param zeroLags the values of zeroLags.
     * @param crossData the values of crossData (encoded in int).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * This method is to be used when cross data are coded with "int" values.
     *
     * @par
     * If this integration contains only cross data (CROSS_ONLY) , the autoData
     * parameter is ignored. A empty vector can be passed as an actual parameter.
     * 
     */
    void addIntegration(unsigned int integrationNum,
			unsigned long long time,
			unsigned long long interval,
			const vector<unsigned long>& flags,
			const vector<long long>& actualTimes,
			const vector<long long>& actualDurations,
			const vector<float>& zeroLags,
			const vector<int>& crossData,
			const vector<float>& autoData);
    

    /**
     * Writes an integration (local header + binary attachment) on the MIME message stream.
     * @param integrationNum the index (1 based) of the integration.
     * @param time time of the integration.
     * @param interval interval of the integration.
     * @param flags the values of flags.
     * @param actualTimes the values of actualTimes.
     * @param actualDurations the values of actualDurations.
     * @param zeroLags the values of zeroLags.
     * @param crossData the values of crossData (encoded in short).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * This method is to be used when cross data are coded with "short" values.
     *
     * @par
     * If this integration contains only cross data (CROSS_ONLY) , the autoData
     * parameter is ignored. A empty vector can be passed as an actual parameter.
     * 
     */
    void addIntegration(unsigned int integrationNum,
			unsigned long long time,
			unsigned long long interval,
			const vector<unsigned long>& flags,
			const vector<long long>& actualTimes,
			const vector<long long>& actualDurations,
			const vector<float>& zeroLags,
			const vector<short>& crossData,
			const vector<float>& autoData);

    /**
     * Writes an integration (local header + binary attachment) on the MIME message stream.
     * @param integrationNum the index (1 based) of the integration.
     * @param time time of the integration.
     * @param interval interval of the integration.
     * @param flags the values of flags.
     * @param actualTimes the values of actualTimes.
     * @param actualDurations the values of actualDurations.
     * @param zeroLags the values of zeroLags.
     * @param crossData the values of crossData (encoded in float).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * This method is to be used when cross data are coded with "float" values.
     *
     * @par
     * If this integration contains only cross data (CROSS_ONLY) , the autoData
     * parameter is ignored. A empty vector can be passed as an actual parameter.
     * 
     */
    void addIntegration(unsigned int integrationNum,
			unsigned long long time,
			unsigned long long interval,
			const vector<unsigned long>& flags,
			const vector<long long>& actualTimes,
			const vector<long long>& actualDurations,
			const vector<float>& zeroLags,
			const vector<float>& crossData,
			const vector<float>& autoData);


    /**
     * Writes an subintegration (local header + binary attachment) on the MIME message stream.
     * @param integrationNum the index (1 based) of the integration.
     * @param subintegrationNum the index(1 based) of the subintegration.
     * @param time time of the integration.
     * @param interval interval of the integration.
     * @param flags the values of flags.
     * @param actualTimes the values of actualTimes.
     * @param actualDurations the values of actualDurations.
     * @param zeroLags the values of zeroLags.
     * @param crossData the values of crossData (encoded in int).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * This method is to be used when cross data are coded with "int" values.
     *
     * @par
     * If this integration contains only cross data (CROSS_ONLY) , the autoData
     * parameter is ignored. A empty vector can be passed as an actual parameter.
     * 
     */
    void addSubintegration(unsigned int integrationNum,
			   unsigned int subintegrationNum,
			   unsigned long long time,
			   unsigned long long interval,
			   const vector<unsigned long>& flags,
			   const vector<long long>& actualTimes,
			   const vector<long long>& actualDurations,
			   const vector<float>& zeroLags,
			   const vector<int>& crossData,
			   const vector<float>& autoData);
    

    /**
     * Writes an subintegration (local header + binary attachment) on the MIME message stream.
     * @param integrationNum the index (1 based) of the integration.
     * @param subintegrationNum the index(1 based) of the subintegration.
     * @param time time of the integration.
     * @param interval interval of the integration.
     * @param flags the values of flags.
     * @param actualTimes the values of actualTimes.
     * @param actualDurations the values of actualDurations.
     * @param zeroLags the values of zeroLags.
     * @param crossData the values of crossData (encoded in short).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * This method is to be used when cross data are coded with "short" values.
     *
     * @par
     * If this integration contains only cross data (CROSS_ONLY) , the autoData
     * parameter is ignored. A empty vector can be passed as an actual parameter.
     * 
     */
    void addSubintegration(unsigned int integrationNum,
			   unsigned int subintegrationNum,
			   unsigned long long time,
			   unsigned long long interval,
			   const vector<unsigned long>& flags,
			   const vector<long long>& actualTimes,
			   const vector<long long>& actualDurations,
			   const vector<float>& zeroLags,
			   const vector<short>& crossData,
			   const vector<float>& autoData);

    /**
     * Writes an subintegration (local header + binary attachment) on the MIME message stream.
     * @param integrationNum the index (1 based) of the integration.
     * @param subintegrationNum the index(1 based) of the subintegration.
     * @param time time of the integration.
     * @param interval interval of the integration.
     * @param flags the values of flags.
     * @param actualTimes the values of actualTimes.
     * @param actualDurations the values of actualDurations.
     * @param zeroLags the values of zeroLags.
     * @param crossData the values of crossData (encoded in float).
     * @param autoData the values of autoData.
     *
     * @throws SDMDataObjectWriterException
     *
     * @note 
     * This method is to be used when cross data are coded with "float" values.
     *
     * @par
     * If this integration contains only cross data (CROSS_ONLY) , the autoData
     * parameter is ignored. A empty vector can be passed as an actual parameter.
     * 
     */
    void addSubintegration(unsigned int integrationNum,
			   unsigned int subintegrationNum,
			   unsigned long long time,
			   unsigned long long interval,
			   const vector<unsigned long>& flags,
			   const vector<long long>& actualTimes,
			   const vector<long long>& actualDurations,
			   const vector<float>& zeroLags,
			   const vector<float>& crossData,
			   const vector<float>& autoData);

    /**
     * Returns the number of bytes written so far.
     * This method can be used at any time during the life of an instance of SDMDataObjectWriter.
     * It returns the number of bytes emitted on the output (memory, standard output, disk file...)
     * as the methods of this class, except done, are called.
     * <ul>
     * <li>This number is set to 0 at the creation of an SDMDataObjectWriter,</li>
     * <li>it is incremented accordingly with the number of bytes emitted by the different methods, except done, </li>
     * <li>it is reset to 0 by a call to the method done.</li>
     * </ul>
     *
     * @return an unsigned long long.
     */
    unsigned long long numBytes(); 
      
    
    void output   (const string& s);    
    void outputln (const string& s);
    void output   (const float* data, unsigned int numData);
    void outputln (const float* data, unsigned int numData);
    void outputln (const long long* data, unsigned int numData);

    template <class T> void output(const vector<T>& data) {
      numBytes_ += data.size()*sizeof(T);
      switch (otype_) {

      case STDOUT:
	cout.write((const char*)&data.at(0), data.size()*sizeof(T));
	break;
	
      case MEMORY:
	oss_->write((const char*)&data.at(0), data.size()*sizeof(T));
	break;
	
      case FILE:
	ofs_->write((const char*)&data.at(0), data.size()*sizeof(T));
	break;
      }       
    }

    template <class T> void outputln (const vector<T>& data) {
      output<T>(data);
      outputln();
    }
    
    void outputln ();

    void outputlnLocation(const string& name, const SDMDataSubset& sdmDataSubset);
    
  private:
    enum OUTDEST {STDOUT, MEMORY, FILE};
    OUTDEST otype_;
    ofstream*       ofs_;
    ostringstream*  oss_;

    // The ALMA uid of the MIME message.
    string uid_;

    // The title of the binary data.
    string title_;

    // The subscan path.
    string subscanPath_;

    // An SDMDataObject
    SDMDataObject sdmDataObject_;

    // The number of the SDMDataSubset being written
    unsigned int sdmDataSubsetNum_;

    // Two strings used as MIME boundaries
    static const string MIMEBOUNDARY_1;
    static const string MIMEBOUNDARY_2;


    // Class initialization stuff
    static const bool initClass_;
    static bool initClass();

    // The axes names definitions for WVR data and their related flags.
    static vector<AxisName> WVRDATAAXES, WVRDATAFLAGSAXES;

    // A utility to fill a vector of <Enum> from a an array of c-strings.
    template <class Enum, class EnumHelper> static vector<Enum> enumvec(const string& strliterals) {
      vector<Enum> result;
      
      string strliteral;
      stringstream ss(strliterals);
      
      vector<string> tokens;   
      while (ss >> strliteral)
	result.push_back(EnumHelper::literal(strliteral));
      
      return result;     
    }

    // Writes the very first part of the MIME message.
    void preamble();

    // Write the very end of the MIME message.
    void postamble();


    void addData(unsigned int integrationNum,
		 unsigned int subintegrationNum,
		 unsigned long long time,
		 unsigned long long interval,
		 const vector<unsigned long>& flags,
		 const vector<long long>& actualTimes,
		 const vector<long long>& actualDurations,
		 const vector<float>& zeroLags,
		 const vector<int>& longCrossData,
		 const vector<short>& shortCrossData,
		 const vector<float>& floatCrossData,
		 const vector<float>& autoData);

    // Are we done with this ?
    bool done_;

    // The number of bytes written so far.
    unsigned long long numBytes_;


    // A small finite state automaton to control the usage of SDMDataObjectWriter.
    enum States {START, S_TPDATA, S_TPDATAHEADER, S_ADDTPSUBSCAN, S_WVRDATA, S_CORRDATAHEADER, S_ADDINTEGRATION, S_ADDSUBINTEGRATION, END};
    enum Transitions {T_TPDATA, T_TPDATAHEADER, T_ADDTPSUBSCAN, T_WVRDATA, T_CORRDATAHEADER, T_ADDINTEGRATION, T_ADDSUBINTEGRATION, T_DONE};
    States currentState_;

    void checkState(Transitions t, const string& methodName);

  };
} // namespace asdmbinaries
#endif // SDMDataObjectWriter_CLASS

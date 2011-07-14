#ifndef ASAP_ASDM_READER_H
#define ASAP_ASDM_READER_H

#include <string>
#include <map>

#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogSinkInterface.h>
#include <ASDMAll.h>
#include <SDMBinData.h>

class ASDMReader
{
public:
  /**
   * constructor 
   **/
  ASDMReader() ;

  /**
   * destructor
   **/
  ~ASDMReader() ;

  /**
   * open data
   *
   * @param filename input ASDM name
   * @param processing options as casa record
   * @return boolean status (true or false) 
   **/
  bool open( const std::string &filename, const casa::Record &rec ) ;

  /**
   * fill data
   **/
//   void fill() ;

  /**
   * close data
   **/
  void close() ;
  
  /**
   * get antenna id
   **/
  casa::Int getAntennaId() { return antennaId_ ; } ;

  /**
   * get antenna name
   **/
  casa::String getAntennaName() { return antennaName_ ; } ;

  /**
   * fill header
   *
   * @param nchan (maximum) number of channel
   * @param npol (maximum) number of polarization
   * @param nif number of IFs
   * @param nbeam number of beam
   * @param observer observer name
   * @param project name
   * @param obstype observation type
   * @param antennaname antenna name
   * @param antennaposition antenna position
   * @param equinox equinox (numerical value)
   * @param freqref frequency frame
   * @param reffreq reference frequency
   * @param bandwidth (maximum) bandwidth
   * @param utc start time of observation
   * @param fluxunit flux unit (K or Jy)
   * @param epoch epoch (UTC)
   * @param poltype polarization type
   **/
  void fillHeader( casa::Int &nchan, 
                   casa::Int &npol, 
                   casa::Int &nif, 
                   casa::Int &nbeam, 
                   casa::String &observer, 
                   casa::String &project, 
                   casa::String &obstype, 
                   casa::String &antennaname, 
                   casa::Vector<casa::Double> &antennaposition, 
                   casa::Float &equinox, 
                   casa::String &freqref, 
                   casa::Double &reffreq, 
                   casa::Double &bandwidth,
                   casa::Double &utc, 
                   casa::String &fluxunit, 
                   casa::String &epoch, 
                   casa::String &poltype ) ;  

  /**
   * get list of valid configDescriptionId
   * 
   * only return list of configDescriptionId with correlationMode of 
   * AUTO_ONLY or CROSS_AND_AUTO. 
   *
   * @return list of valid configDescriptionId 
   **/
  casa::Vector<casa::uInt> getConfigDescriptionIdList() { return configDescIdList_ ; } ;

  /**
   * get list of fieldId
   *
   * @return list of fieldId as casa::uInt
   **/
  casa::Vector<casa::uInt> getFieldIdList() ;

  /**
   * get number of rows in Main table
   *
   * @return number of rows in Main table
   **/
  casa::uInt getNumMainRow() ;

  /**
   * binary data selection
   **/
  void select() ;

  /**
   * set Main rows that matches given context (configDescId and fieldId) 
   * to mainRow_
   *
   * @param configDescId 
   * @param fieldId
   **/
  casa::Bool setMainRow( casa::uInt configDescId, casa::uInt fieldId ) ;

  /**
   * set Main row to SDMBinData object
   *
   * @param irow row index
   * @return boolean indicating the row is valid or not
   **/
  casa::Bool setMainRow( casa::uInt irow ) ;

  /**
   * get scan number of current row
   *
   * @return scan number
   **/
  unsigned int getScanNo() { return (unsigned int)(mainRow_[row_]->getScanNumber()) ; } ;

  /**
   * get subscan number of current row
   *
   * @return subscan number
   **/
  unsigned int getSubscanNo() { return (unsigned int)(mainRow_[row_]->getSubscanNumber()) ; } ;

  /**
   * get subscan number for given index
   *
   * @param idx for vmsData_
   * @return subscan number
   **/
  unsigned int getSubscanNo( unsigned int idx ) ;

  /**
   * get IF number for given index 
   *
   * @param idx for vmsData_
   * @return IF number
   **/
  casa::uInt getIFNo( unsigned int idx ) ;

  /**
   * get number of polarization for given index
   *
   * @param idx for vmsData_
   * @return number of polarizations
   **/
  int getNumPol( unsigned int idx ) ;

  /**
   * get REFPIX, REFVAL, INCREMENT for given index
   *
   * @param idx for vmsData_
   * @param refpix REFPIX
   * @param refval REFVAL
   * @param incr INCREMENT
   **/
  void getFrequency( unsigned int idx, 
                     double &refpix, 
                     double &refval, 
                     double &incr ) ;

  /**
   * get rest frequencies for given index
   *
   * @param idx for vmsData_
   * @return rest frequencies
   **/
  vector<double> getRestFrequency( unsigned int idx ) ;

  /**
   * get MJD time in day for given index
   *
   * @param idx for vmsData_
   * @return MJD time in day
   **/
  double getTime( unsigned int idx ) ;

  /**
   * get integration time in sec for given index
   *
   * @param idx for vmsData_
   * @return integration time in sec
   **/
  double getInterval( unsigned int idx ) ;

  /**
   * get source name for given index
   *
   * @param idx for vmsData_
   * @return source name
   **/
  string getSourceName( unsigned int idx ) ;

  /**
   * get field name for given index
   * field name = fieldName + "__" + fieldId
   *
   * @param idx for vmsData_
   * @return field name
   **/
  string getFieldName( unsigned int idx ) ;

  /**
   * get source direction for given index
   *
   * @param idx for vmsData_
   * @return source direction as vector<double>
   **/
  std::vector<double> getSourceDirection( unsigned int idx ) ;
  
  /**
   * get source proper motion for given index
   *
   * @param idx for vmsData_
   * @return source proper motion as vector<double>
   **/
  std::vector<double> getSourceProperMotion( unsigned int idx ) ;

  /**
   * get systemic velocity of the source for given index
   * at the moment return 0-th element of sysVel vector
   *
   * @param idx for vmsData_
   * @return systemic velocity of the source
   **/
  double getSysVel( unsigned int idx ) ;

  /**
   * get row-based flag for given index
   *
   * @param idx for vmsData_
   * @return row-based flag 
   **/
  unsigned int getFlagRow( unsigned int idx ) ;

  /**
   * get data shape (nPol, nChan, nApc=1) for given index
   *
   * @param idx for vmsData_
   * @return data shape
   **/
  std::vector<unsigned int> getDataShape( unsigned int idx ) ;

  /**
   * get spectral data for given index
   *
   * @param idx for vmsData_
   * @return spectral data 
   **/
  float *getSpectrum( unsigned int idx ) ;

  /**
   * get channel flag data for given index 
   *
   * @param idx for vmsData_
   * @return channel flag 
   **/
  //bool *getFlagChannel( unsigned int idx ) ;
  
  /**
   * get Tsys for given index
   *
   * @param idx for vmsData_
   * @return Tsys
   **/
  std::vector< std::vector<float> > getTsys( unsigned int idx ) ;
  
  /**
   * get Tcal for given index
   *
   * @param idx for vmsData_
   * @return Tcal
   **/
  std::vector< std::vector<float> > getTcal( unsigned int idx ) ;
  
  /**
   * get opacity for given index
   *
   * @param idx for vmsData_
   * @return opacity
   **/
  std::vector<float> getOpacity( unsigned int idx ) ;
  
  /**
   * get weather information for given index
   *
   * @param idx for vmsData_
   * @param temperature 
   * @param pressure
   * @param humidity
   * @param windspeed 
   * @param windaz
   **/
  void getWeatherInfo( unsigned int idx,
                       float &temperature,
                       float &pressure,
                       float &humidity,
                       float &windspeed,
                       float &windaz ) ;

  /**
   * get pointing information for given index
   *
   * @param idx for vmsData_
   * @param dir direction
   * @param az azimuth
   * @param el elevation
   * @param srate scan rate
   **/
  void getPointingInfo( unsigned int idx,
                        std::vector<double> &dir,
                        double &az,
                        double &el,
                        std::vector<double> &srate ) ;

  /**
   * get source type enum (int) for given scan and subscan
   *
   * @param scan scan No.
   * @param subscan subscan No.
   * @return source type as int
   **/
  int getSrcType( unsigned int scan, 
                  unsigned int subscan ) ;

  /**
   * get list of dataDescId for given configDescId
   *
   * @param configDescId
   * @return list of dataDescId
   **/
//   casa::Vector<casa::uInt> getDataDescIdList( casa::uInt cdid ) ;

  /**
   * get list of switchCycleId for given configDescId
   *
   * @param configDescId
   * @return list of dataDescId
   **/
//   casa::Vector<casa::uInt> getSwitchCycleIdList( casa::uInt cdid ) ;

  /**
   * get list of feedId for given configDescId
   *
   * only return list of feedId that corresponds to specified antenna.
   *
   * @param configDescId
   * @return list of valid feedId 
   **/
//   casa::Vector<casa::uInt> getFeedIdList( casa::uInt cdid ) ;

  /**
   * set binary data to MSData object
   *
   * @return boolean status
   **/
  casa::Bool setData() ;

  /**
   * get number of data in the current row
   *
   * @return number of data
   **/
  unsigned int getNumData() { return numData_ ; } ;

  /**
   * get frequency frame
   *
   * @return string representating frequency frame
   **/
  std::string getFrame() ;

  /**
   * set Logger
   *
   * @param logger (LogSinkInterface)
   **/
  void setLogger( casa::CountedPtr<casa::LogSinkInterface> &logsink ) ;


private:

  /**
   * pick up valid configDescriptionId
   * 
   * only retrieve configDescriptionId with correlationMode of 
   * AUTO_ONLY or CROSS_AND_AUTO. 
   **/
  void selectConfigDescription() ;

  /**
   * pick up valid feedId
   *
   * only retrieve feedId that has corresponding row for antennaId_ 
   **/
  void selectFeed() ;

  /**
   * clear mainRow_
   **/
  void clearMainRow() ;

  /**
   * determine IFNO for each SpectralWindow rows 
   *
   * SpectralWindow row is identified as WVR when basebandName is "NOBB" and numChan is 4.
   * All WVR SpectralWindow is merged into one IFNO.
   **/
  void setupIFNO() ;

  /**
   * check if given SpectralWindow is WVR or not
   **/
  bool isWVR( asdm::SpectralWindowRow *row ) ;
  
  /**
   * process Station table
   * 
   * classify station Ids by its type
   **/
  void processStation() ;

  /**
   * get the closest weather station for given antenna pad
   *
   * @return stationId for weather station
   **/
  int getClosestWeatherStation() ;

  /**
   * get mid-point of ArrayTimeInterval
   *
   * @param time interval as ArrayTimeInterval
   * @return time of mid-point as ArrayTime 
   **/
  asdm::ArrayTime getMidTime( const asdm::ArrayTimeInterval &t ) ;

  /**
   * get start-point of ArrayTimeInterval
   *
   * @param time interval as ArrayTimeInterval
   * @return time of start-point as ArrayTime 
   **/
  asdm::ArrayTime getStartTime( const asdm::ArrayTimeInterval &t ) ;

  /**
   * get end-point of ArrayTimeInterval
   *
   * @param time interval as ArrayTimeInterval
   * @return time of end-point as ArrayTime 
   **/
  asdm::ArrayTime getEndTime( const asdm::ArrayTimeInterval &t ) ;

  /**
   *  AZEL to J2000
   *
   * @param dir pointing direction
   * @param az azimuth
   * @param el elevation
   * @param mjd reference time
   * @param antpos antenna position vector
   **/
  void toJ2000( std::vector<double> &dir,
                double az, 
                double el,
                double mjd,
                casa::Vector<casa::Double> antpos ) ;

  /**
   * get nIF
   *
   * @return number of IFs
   **/
  int getNumIFs() ;

  asdm::ASDM *asdm_ ; // pointer to ASDM object
  sdmbin::SDMBinData *sdmBin_ ; // pointer to ASDM binary data
  /**
   * vmsData_ is a pointer to binary data
   *
   * VMSData contents
   *
   * int processorId
   * vector< double > v_time MJD time in sec
   * vector< int > v_fieldId
   * vector< double > v_interval interval in sec
   * vector< AtmPhaseCorrection > v_atmPhaseCorrection
   * int binNum
   * vector< unsigned int > v_projectPath
   * vector< int > v_antennaId1 antennaId in int
   * vector< int > v_antennaId2 antennaId in int
   * vector< int > v_feedId1 feedId in int
   * vector< int > v_feedId2 feedId in int
   * vector< int > v_dataDescId dataDescriptionId in int
   * vector< double > v_timeCentroid
   * vector< double > exposure
   * vector< int > v_numData
   * vector< vector< unsigned int > > vv_dataShape (nPol,nChan,nApc=1)
   * vector< map< AtmPhaseCorrection, float *> > v_m_data actual data
   * vector< vector< vector< Angle > > > v_pahseDir direction
   * vector< int > v_stateId
   * vector< MSState > v_msState 
   * vector< unsigned int > v_flag
   **/
  const sdmbin::VMSData *vmsData_ ; 
  casa::Int antennaId_ ; // antenna id
  casa::String antennaName_ ; // antenna name
  casa::Vector<asdm::MainRow *> mainRow_ ; // list of pointers to all Main rows
  casa::Vector<casa::uInt> configDescIdList_ ; // list of valid configDescriptionId 
  casa::Vector<casa::uInt> feedIdList_ ; // list of valid feedId 
  casa::Vector<casa::uInt> fieldIdList_ ; // list of fieldId
  casa::Int row_ ; // current row index
  map<asdm::Tag,casa::uInt> ifno_ ; // list of IFNO for each SpectralWindow rows
  unsigned int numData_ ; // number of valid data in vmsData_ where v_antennaId equals antennaId_
  vector<unsigned int> dataIdList_ ; // list of valid data indexes in vmsData_  
  vector<asdm::Tag> antennaPad_ ; // list of Station Tags for ANTENNA_PAD
  vector<asdm::Tag> weatherStation_ ; // list of Station Tags for WEATHER_STATION
  int weatherStationId_ ; // closest weather station for antennaId_
  AtmPhaseCorrectionMod::AtmPhaseCorrection apc_ ; // ATM phase correction
  EnumSet<CorrelationModeMod::CorrelationMode> corrMode_ ; // input correlation mode
  EnumSet<TimeSamplingMod::TimeSampling> timeSampling_ ; // time sampling
  EnumSet<SpectralResolutionTypeMod::SpectralResolutionType> resolutionType_ ; // spectral resolution type
  casa::CountedPtr<casa::LogSinkInterface> logsink_ ; // Logger
  casa::String className_ ;
} ;
#endif // ASAP_ASDM_READER_H

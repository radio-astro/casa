#ifndef ASAP_ASDM_READER_H
#define ASAP_ASDM_READER_H

#include <string>
#include <map>

#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogSinkInterface.h>
#include <ASDMAll.h>
#include <SDMBinData.h>

namespace casa {
  class MDirection;
  class MeasFrame;
  class MPosition;
  class MEpoch;
};

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
  unsigned int getScanNoOfCurrentRow() { return (unsigned int)(mainRow_[row_]->getScanNumber()) ; } ;

  /**
   * get subscan number of current row
   *
   * @return subscan number
   **/
  unsigned int getSubscanNoOfCurrentRow() { return (unsigned int)(mainRow_[row_]->getSubscanNumber()) ; } ;

  /**
   * set data index
   *
   * @param idx for vmsData_
   **/
  void prepareData( unsigned int idx ) ;

  /**
   * get subscan number for given index
   *
   * @param idx for vmsData_
   * @return subscan number
   **/
  unsigned int getSubscanNo( unsigned int idx ) ;
  unsigned int getSubscanNo() ;

  /**
   * get IF number for given index 
   *
   * @param idx for vmsData_
   * @return IF number
   **/
  casa::uInt getIFNo( unsigned int idx ) ;
  casa::uInt getIFNo() ;

  /**
   * get number of polarization for given index
   *
   * @param idx for vmsData_
   * @return number of polarizations
   **/
  int getNumPol( unsigned int idx ) ;
  int getNumPol() ;

  /**
   * get REFPIX, REFVAL, INCREMENT for given index
   *
   * @param idx for vmsData_
   * @param refpix REFPIX
   * @param refval REFVAL
   * @param incr INCREMENT
   * @param freqref frequency reference
   **/
  void getFrequency( unsigned int idx, 
                     double &refpix, 
                     double &refval, 
                     double &incr,
                     std::string &freqref ) ;
  void getFrequency( double &refpix,
                     double &refval,
                     double &incr,
                     std::string &freqref ) ;

  /**
   * get MJD time in day for given index
   *
   * @param idx for vmsData_
   * @return MJD time in day
   **/
  double getTime( unsigned int idx ) ;
  double getTime() ;

  /**
   * get integration time in sec for given index
   *
   * @param idx for vmsData_
   * @return integration time in sec
   **/
  double getInterval( unsigned int idx ) ;
  double getInterval() ;

  /**
   * get source direction with reference 
   *
   * @param dir source direction
   * @param reference frame
   * @param idx for Source table
   **/
  void getSourceDirection( std::vector<double> &dir, std::string &ref ) ;
  
  /**
   * get row-based flag for given index
   *
   * @param idx for vmsData_
   * @return row-based flag 
   **/
  unsigned int getFlagRow( unsigned int idx ) ;
  unsigned int getFlagRow() ;

  /**
   * get data shape (nPol, nChan, nApc=1) for given index
   *
   * @param idx for vmsData_
   * @return data shape
   **/
  std::vector<unsigned int> getDataShape( unsigned int idx ) ;
  std::vector<unsigned int> getDataShape() ;

  /**
   * get spectral data for given index
   *
   * @param idx for vmsData_
   * @return spectral data 
   **/
  float *getSpectrum( unsigned int idx ) ;
  float *getSpectrum() ;

  /**
   * get Tsys for given index
   *
   * @param idx for vmsData_
   * @return Tsys
   **/
  std::vector< std::vector<float> > getTsys( unsigned int idx ) ;
  std::vector< std::vector<float> > getTsys() ;
  
  /**
   * get Tcal for given index
   *
   * @param idx for vmsData_
   * @return Tcal
   **/
  std::vector< std::vector<float> > getTcal( unsigned int idx ) ;
  std::vector< std::vector<float> > getTcal() ;

  /**
   * get Tcal and Tsys for given index
   *
   * @param idx for vmsData_
   * @param tcal Tcal
   * @param tsys Tsys
   **/
  void getTcalAndTsys( unsigned int idx, 
                       std::vector< std::vector<float> > &tcal,
                       std::vector< std::vector<float> > &tsys ) ;
  void getTcalAndTsys( std::vector< std::vector<float> > &tcal,
                       std::vector< std::vector<float> > &tsys ) ;
  
  /**
   * get opacity for given index
   *
   * @param idx for vmsData_
   * @return opacity
   **/
  std::vector<float> getOpacity( unsigned int idx ) ;
  std::vector<float> getOpacity() ;
  
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
  void getWeatherInfo( float &temperature,
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
  void getPointingInfo( std::vector<double> &dir,
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
   * get source properties 
   * 
   * @param idx for vmsData_
   * @param srcname source name
   * @param fieldname field name
   * @param srcdir source direction
   * @param srcpm source proper motion
   * @param sysvel systemic velocity of the source
   * @param restfreq rest frequency
   **/
  void getSourceProperty( unsigned int idx,
                          std::string &srcname,
                          std::string &fieldname,
                          std::vector<double> &srcdir,
                          std::vector<double> &srcpm,
                          double &sysvel,
                          std::vector<double> &restfreq ) ;
  void getSourceProperty( std::string &srcname,
                          std::string &fieldname,
                          std::vector<double> &srcdir,
                          std::vector<double> &srcpm,
                          double &sysvel,
                          std::vector<double> &restfreq ) ;

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
                double &az, 
                double &el,
                double &mjd ) ;
  /**
  * to J2000
  *
  * @param dir pointing direction
  * @param dirref direction reference
  * @param mjd reference time
  * @param antpos antenna position vector 
  * @return new direction
  **/
  std::vector<double> toJ2000( std::vector<double> &dir,
                               casa::String &dirref,
                               double &mjd ) ;

  /**
   * get nIF
   *
   * @return number of IFs
   **/
  int getNumIFs() ;

  /**
   * get appropriate row from SysCal table
   *
   * @param idx for vmsData_
   * @return pointer to SysCalRow object (0 when no appropriate row)
   **/
  asdm::SysCalRow *getSysCalRow( unsigned int idx ) ;
  asdm::SysCalRow *getSysCalRow() ;

  /**
   * limit angule in radian within [-pi,pi]
   *
   * @param any angule in radian
   * @return equivalent angle that satisfies [-pi,pi]
   **/
  double limitedAngle( double angle ) ;

  /**
   * retrieve pointing direction from pointingDirection column
   * or from target+offset 
   *
   * @param row pointer to PointingRow object
   * @return pointing direction vector (matrix)
   **/
  std::vector< std::vector<double> > pointingDir( asdm::PointingRow *row ) ;

  /**
   * initialize conversion engine
   **/
  void initConvert() ;

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
  casa::String stationName_ ; // station name
  casa::Vector<casa::Quantity> antennaPosition_ ; // antenna position
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
  unsigned int dataIndex_ ;

  // Tables/Rows for ASDM
  casa::Vector<asdm::MainRow *> mainRow_ ; // list of pointers to all Main rows
  //asdm::AntennaRow *antennaRow_p ; // pointer to target Antenna row
  //asdm::StationRow *stationRow_p ; // pointer to target Station row that target antenna is located
  asdm::SpectralWindowRow *specWinRow_p ; // pointer to SpectralWindow row
  asdm::PolarizationRow *polarizationRow_p ; // pointer to Polarization row
  asdm::FieldRow *fieldRow_p ; // pointer to Field row 

  // Tags
  asdm::Tag antennaTag_ ;
  asdm::Tag specWinTag_ ;
  asdm::Tag execBlockTag_ ;

  // time
  asdm::ArrayTimeInterval timeInterval_ ;

  casa::MDirection::Convert toj2000_ ;
  casa::MeasFrame mf_ ;
  casa::MPosition mp_ ;
  casa::MEpoch me_ ;
} ;
#endif // ASAP_ASDM_READER_H

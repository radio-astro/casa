#include <iostream>
#include <sstream>
#include "limits.h"
//#include "float.h"

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Logging/LogMessage.h>
#include <casa/BasicSL/Constants.h>

#include "ASDMReader.h"
#include <atnf/PKSIO/SrcType.h>

using namespace std ;
//using namespace casa ;
using namespace asdm ;
using namespace sdmbin ;

// sec to day
double s2d = 1.0 / 86400.0 ;

ASDMReader::ASDMReader()
  : asdm_(0),
    sdmBin_(0),
    vmsData_(0),
    antennaId_( -1 ),
    antennaName_( "" ),
    stationName_( "" ),
    row_(-1),
    apc_(AP_CORRECTED),
    className_("ASDMReader"),
    dataIndex_( UINT_MAX ),
    //antennaRow_p( 0 ),
    //stationRow_p( 0 ),
    specWinRow_p( 0 ),
    polarizationRow_p( 0 ),
    fieldRow_p( 0 )
{
  configDescIdList_.resize(0) ;
  feedIdList_.resize(0) ;
  fieldIdList_.resize(0) ;
  mainRow_.resize(0) ;
  ifno_.clear() ;
  corrMode_.reset() ;
  timeSampling_.reset() ;
  initConvert() ;
}

ASDMReader::~ASDMReader()
{
  close() ;
  logsink_ = 0 ;
}

bool ASDMReader::open( const string &filename, const casa::Record &rec )
{
  casa::String funcName = "open" ;

  // return value
  bool status = true ;

  // set default
  timeSampling_.reset() ;
  corrMode_.reset() ;
  resolutionType_.reset() ;
  apc_ = AP_CORRECTED ;

  // parsing ASDM options
  if ( rec.isDefined( "asdm" ) ) {
    casa::Record asdmrec = rec.asRecord( "asdm" ) ;

    // antenna
    if ( asdmrec.isDefined( "antenna" ) ) {
      if ( asdmrec.type( asdmrec.fieldNumber( "antenna" ) ) == casa::TpInt ) {
        antennaId_ = asdmrec.asInt( "antenna" ) ;
      }
      else {
        antennaName_ = asdmrec.asString( "antenna" ) ;
      }
    }
    else {
      antennaId_ = 0 ;
    }

    // ATM phase correction
    if ( asdmrec.isDefined( "apc" ) ) {
      if ( asdmrec.asBool( "apc" ) )
        apc_ = AP_CORRECTED ;
      else
        apc_ = AP_UNCORRECTED ;
    }

    // time sampling
    String timeSampling = "all" ; // take all time sampling by default
    if ( asdmrec.isDefined( "sampling" ) ) {
      timeSampling = asdmrec.asString( "sampling" ) ;
    }
    if ( timeSampling == "all" ) {
      timeSampling_.set( INTEGRATION ) ;
      timeSampling_.set( SUBINTEGRATION ) ;
    }
    else if ( timeSampling == "integration" ) {
      timeSampling_.set( INTEGRATION ) ;
    }
    else if ( timeSampling == "subintegration" ) {
      timeSampling_.set( SUBINTEGRATION ) ;
    }
    else {
      //throw AipsError( "Unrecognized option for sampling" ) ;
      logsink_->postLocally( LogMessage( "Unrecognized option for time sampling: "+String::toString(timeSampling), LogOrigin(className_,funcName,WHERE), LogMessage::WARN ) ) ;
      status = false ;
    }      

    // spectral resolution type
    string resolutionType = "all" ;
    if ( asdmrec.isDefined( "srt" ) ) {
      resolutionType = string( asdmrec.asString( "srt" ) ) ;
    }
    string resolutionTypes[3] ;
    Int numType = split( resolutionType, resolutionTypes, 3, "," ) ;
    for ( Int it = 0 ; it < numType ; it++ ) {
      if ( resolutionTypes[it] == "all" ) {
        resolutionType_.reset() ;
        resolutionType_.set( FULL_RESOLUTION ) ;
        resolutionType_.set( BASEBAND_WIDE ) ;
        resolutionType_.set( CHANNEL_AVERAGE ) ;
        break ;
      }
      else if ( resolutionTypes[it] == "fr" ) {
        resolutionType_.set( FULL_RESOLUTION ) ;
      }
      else if ( resolutionTypes[it] == "bw" ) {
        resolutionType_.set( BASEBAND_WIDE ) ;
      }
      else if ( resolutionTypes[it] == "ca" ) {
        resolutionType_.set( CHANNEL_AVERAGE ) ;
      }
    }
    if ( resolutionType_.size() == 0 ) {
      logsink_->postLocally( LogMessage( "Unrecognized option for spectral resolution type: "+String::toString(resolutionType), LogOrigin(className_,funcName,WHERE), LogMessage::WARN ) ) ;
      status = false ;
    }
    
    // input correlation mode
    string corrMode = "ao,ca" ;
    if ( asdmrec.isDefined( "corr" ) ) {
      corrMode = string( asdmrec.asString( "corr" ) ) ;
      //logsink_->postLocally( LogMessage("corrMode = "+String(corrMode),LogOrigin(className_,funcName,WHERE)) ) ;
    }
    string corrModes[3] ;
    Int numCorr = split( corrMode, corrModes, 3, "," ) ;
    for ( Int ic = 0 ; ic < numCorr ; ic++ ) {
      if ( corrModes[ic] == "ao" ) {
        corrMode_.set( AUTO_ONLY ) ;
      }
      else if ( corrModes[ic] == "ca" ) {
        corrMode_.set( CROSS_AND_AUTO ) ;
      }
    }
    if ( corrMode_.size() == 0 ) {
      logsink_->postLocally( LogMessage( "Invalid option for correlation mode: "+String::toString(corrMode), LogOrigin(className_,funcName,WHERE), LogMessage::WARN ) ) ;
      status = false ;
    }

//     logsink_->postLocally( LogMessage( "### asdmrec summary ###", LogOrigin(className_,funcName,WHERE) ) ) ;
//     ostringstream oss ;
//     asdmrec.print( oss ) ;
//     logsink_->postLocally( LogMessage( oss.str(), LogOrigin(className_,funcName,WHERE) ) ) ;
//     logsink_->postLocally( LogMessage( "#######################", LogOrigin(className_,funcName,WHERE) ) ) ;
  }

  // create ASDM object
  asdm_ = new ASDM() ;
  asdm_->setFromFile( filename ) ;

  AntennaTable &atab = asdm_->getAntenna() ;
  AntennaRow *antennaRow ;
  if ( antennaId_ == -1 ) {
    vector<AntennaRow *> rows = atab.get() ;
    int idx = -1 ;
    for ( casa::uInt irow = 0 ; irow < rows.size() ; irow++ ) {
      if ( casa::String(rows[irow]->getName()) == antennaName_ ) {
        idx = rows[irow]->getAntennaId().getTagValue() ;
        break ;
      }
    }
    if ( idx == -1 ) {
      close() ;
      throw (casa::AipsError( antennaName_ + " not found." )) ;
    }
    else {
      antennaId_ = idx ;
    }
    antennaTag_ = Tag( antennaId_, TagType::Antenna ) ;
    antennaRow = rows[antennaId_] ;
  }
  else {
    antennaTag_ = Tag( antennaId_, TagType::Antenna ) ;
    antennaRow = atab.getRowByKey( antennaTag_ ) ;
    if ( antennaRow == 0 ) {
      close() ;
      throw (casa::AipsError( "AntennaId " + casa::String::toString(antennaId_) + " is invalid." ) ) ;
    }
  }
  

  // set antenna name
  if ( antennaName_.size() == 0 ) {
    antennaName_ = casa::String( antennaRow->getName() ) ;
  }

  // get Station row
  StationRow *stationRow = antennaRow->getStationUsingStationId() ;

  // station name
  stationName_ = casa::String( stationRow->getName() ) ;

  // antenna position
  antennaPosition_.resize( 3 ) ;
  vector<Length> antpos = stationRow->getPosition() ;
  for ( casa::uInt i = 0 ; i < 3 ; i++ ) 
    antennaPosition_[i] = Quantity( casa::Double( antpos[i].get() ), Unit( "m" ) ) ;
  mp_ = casa::MPosition( casa::MVPosition( antennaPosition_ ),
                         casa::MPosition::ITRF ) ;
  mf_.set( mp_ ) ;

  // create SDMBinData object
  sdmBin_ = new SDMBinData( asdm_, filename ) ; 

  // get Main rows
  //mainRow_ = casa::Vector<MainRow *>(asdm_->getMain().get()) ;

  // set up IFNO
  setupIFNO() ;

  // process Station table
  processStation() ;

  //logsink_->postLocally( LogMessage(  "antennaId_ = "+String::toString(antennaId_), LogOrigin(className_,funcName,WHERE) ) ) ;
  //logsink_->postLocally( LogMessage(  "antennaName_ = "+antennaName_, LogOrigin(className_,funcName,WHERE) ) ) ;

  return true ;
}

// void ASDMReader::fill() 
// {
// }

void ASDMReader::close() 
{
  clearMainRow() ;

  if ( sdmBin_ )
    delete sdmBin_ ;
  sdmBin_ = 0 ;

  if ( asdm_ )
    delete asdm_ ;
  asdm_ = 0 ;

  return ;
}

void ASDMReader::fillHeader( casa::Int &nchan, 
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
                             casa::String &poltype ) 
{
  casa::String funcName = "fillHeader" ;

  ExecBlockTable &ebtab = asdm_->getExecBlock() ;
  // at the moment take first row of ExecBlock table
  ExecBlockRow *ebrow = ebtab.get()[0] ;
  casa::String telescopeName( ebrow->getTelescopeName() ) ;
  //casa::String stationName( stationRow_p->getName() ) ;

  // antennaname
  // <telescopeName>//<antennaName>@stationName
  antennaname = telescopeName + "//" + antennaName_ + "@" + stationName_ ;
  //logsink_->postLocally( LogMessage("antennaName = "+antennaname,LogOrigin(className_,funcName,WHERE)) ) ;

  // antennaposition
  antennaposition.resize( 3 ) ;
  for ( casa::uInt i = 0 ; i < 3 ; i++ ) 
    antennaposition[i] = antennaPosition_[i].getValue( Unit("m") ) ;

  // observer
  observer = ebrow->getObserverName() ;

  // project
  // T.B.D. (project UID?)
  project = "T.B.D. (" + ebrow->getProjectUID().toString() + ")" ;

  // utc
  // start time of the project
  utc = casa::Double( ebrow->getStartTime().getMJD() ) ;
  

  SpectralWindowTable &spwtab = asdm_->getSpectralWindow() ;
  vector<SpectralWindowRow *> spwrows = spwtab.get() ;
  int nspwrow = spwrows.size() ;

  // nif
  //nif = spwrows.size() ;
  nif = getNumIFs() ;

  // nchan 
  int refidx = -1 ;
  vector<int> nchans ;
  for ( int irow = 0 ; irow < nspwrow ; irow++ ) {
    nchans.push_back( spwrows[irow]->getNumChan() ) ;
    if ( refidx == -1 && nchans[irow] != 1 && nchans[irow] != 4 )
      refidx = irow ;
  }
  nchan = casa::Int( *max_element( nchans.begin(), nchans.end() ) ) ;

  //logsink_->postLocally( LogMessage("refidx = "+String::toString(refidx),LogOrigin(className_,funcName,WHERE)) ) ;

  // bandwidth
  vector<double> bws ;
  for ( int irow = 0 ; irow < nspwrow ; irow++ ) {
    if ( nchans[irow] != 4 ) { // exclude WVR data
      bws.push_back( spwrows[irow]->getTotBandwidth().get() ) ;
    }
  }
  bandwidth = casa::Double( *max_element( bws.begin(), bws.end() ) ) ;

  // reffreq
  reffreq = casa::Double( spwrows[refidx]->getRefFreq().get() ) ;

  // freqref
  if ( spwrows[refidx]->isMeasFreqRefExists() ) {
    string mfr = CFrequencyReferenceCode::name( spwrows[refidx]->getMeasFreqRef() ) ;
//     if (mfr == "TOPO") {
//       freqref = "TOPOCENT";
//     } else if (mfr == "GEO") {
//       freqref = "GEOCENTR";
//     } else if (mfr == "BARY") {
//       freqref = "BARYCENT";
//     } else if (mfr == "GALACTO") {
//       freqref = "GALACTOC";
//     } else if (mfr == "LGROUP") {
//       freqref = "LOCALGRP";
//     } else if (mfr == "CMB") {
//       freqref = "CMBDIPOL";
//     } else if (mfr == "REST") {
//       freqref = "SOURCE";
//     }
    freqref = String( mfr ) ;
  }
  else {
    // frequency reference is TOPOCENT by default
    //freqref = "TOPOCENT" ;
    freqref = "TOPO" ;
  }


  PolarizationTable &ptab = asdm_->getPolarization() ;
  vector<PolarizationRow *> prows = ptab.get() ; 
  vector<int> npols ;
  refidx = -1 ;
  for ( unsigned int irow = 0 ; irow < prows.size() ; irow++ ) {
    npols.push_back( prows[irow]->getNumCorr() ) ;
    if ( refidx == -1 && npols[irow] != 1 )
      refidx = irow ;
  }
  if ( refidx == -1 )
    refidx = 0 ;

  // npol
  npol = casa::Int( *max_element( npols.begin(), npols.end() ) ) ;

  // poltype
  vector<StokesParameter> corrType = prows[refidx]->getCorrType() ;
  if ( corrType[0] == I ||
       corrType[0] == Q ||
       corrType[0] == U ||
       corrType[0] == V )
    poltype = "stokes" ;
  else if ( corrType[0] == RR ||
            corrType[0] == RL ||
            corrType[0] == LR ||
            corrType[0] == LL )
    poltype = "circular" ;
  else if ( corrType[0] == XX ||
            corrType[0] == XY ||
            corrType[0] == YX ||
            corrType[0] == YY )
    poltype = "linear" ;
  else if ( corrType[0] == PLINEAR ||
            corrType[0] == PANGLE ) {
    poltype = "linpol" ;
  }
  else {
    poltype = "notype" ;
  }


  FeedTable &ftab = asdm_->getFeed() ;
  vector<FeedRow *> frows = ftab.get() ;
  vector<int> nbeams ;
  for ( unsigned int irow = 0 ; irow < frows.size() ; irow++ ) {
    if ( frows[irow]->isFeedNumExists() )
      nbeams.push_back( frows[irow]->getFeedNum() ) ;
    else 
      nbeams.push_back( 1 ) ;
  }

  // nbeam
  nbeam = casa::Int( *max_element( nbeams.begin(), nbeams.end() ) ) ;

  // fluxunit
  // tentatively set 'K'? or empty?
  fluxunit = "K" ;

  // equinox
  // tentatively set 2000.0
  equinox = 2000.0 ;

  // epoch
  // tentatively set "UTC"
  epoch = "UTC" ;

  // obstype
  // at the moment take observingMode attribute in SBSummary table
  SBSummaryRow *sbrow = ebrow->getSBSummaryUsingSBSummaryId() ;
  vector<string> obsmode = sbrow->getObservingMode() ;
  obstype = "" ;
  for ( unsigned int imode = 0 ; imode < obsmode.size() ; imode++ ) {
    obstype += casa::String(obsmode[imode]) ;
    if ( imode != obsmode.size()-1 )
      obstype += "#" ;
  }
}

void ASDMReader::selectConfigDescription() 
{
  casa::String funcName = "selectConfigDescription" ;

  vector<ConfigDescriptionRow *> cdrows = asdm_->getConfigDescription().get() ;
  vector<Tag> cdidTags ;
  for ( unsigned int irow = 0 ; irow < cdrows.size() ; irow++ ) {
    //logsink_->postLocally( LogMessage("correlationMode["+String::toString(irow)+"] = "+String::toString(cdrows[irow]->getCorrelationMode()),LogOrigin(className_,funcName,WHERE)) ) ;
    if ( cdrows[irow]->getCorrelationMode() != CROSS_ONLY ) {
      cdidTags.push_back( cdrows[irow]->getConfigDescriptionId() ) ;
    }
  }

  configDescIdList_.resize( cdidTags.size() ) ;
  for ( unsigned int i = 0 ; i < cdidTags.size() ; i++ ) {
    configDescIdList_[i] = casa::uInt( cdidTags[i].getTagValue() ) ;
  }
}

void ASDMReader::selectFeed() 
{
  feedIdList_.resize(0) ;
  vector<FeedRow *> frows = asdm_->getFeed().get() ;
  Tag atag( antennaId_, TagType::Antenna ) ;
  for ( unsigned int irow = 0 ; irow < frows.size() ; irow++ ) {
    casa::uInt feedId = (casa::uInt)(frows[irow]->getFeedId() ) ;
    if ( casa::anyEQ( feedIdList_, feedId ) ) 
      continue ;
    if ( frows[irow]->getAntennaId() == atag ) {
      unsigned int oldsize = feedIdList_.size() ;
      feedIdList_.resize( oldsize+1, true ) ;
      feedIdList_[oldsize] = feedId ;
    }
  }
}

casa::Vector<casa::uInt> ASDMReader::getFieldIdList() 
{
  casa::String funcName = "getFieldIdList" ;

  vector<FieldRow *> frows = asdm_->getField().get() ;
  fieldIdList_.resize( frows.size() ) ;
  for ( unsigned int irow = 0 ; irow < frows.size() ; irow++ ) {
    //logsink_->postLocally( LogMessage("fieldId["+String::toString(irow)+"]="+String(frows[irow]->getFieldId().toString()),LogOrigin(className_,funcName,WHERE)) ) ;
    fieldIdList_[irow] = frows[irow]->getFieldId().getTagValue() ;
  }

  return fieldIdList_ ;
}

casa::uInt ASDMReader::getNumMainRow() 
{
  casa::uInt nrow = casa::uInt( mainRow_.size() ) ;

  return nrow ;
}

void ASDMReader::select() 
{
  // selection by input CorrelationMode
  EnumSet<CorrelationMode> esCorrs ;
  sdmBin_->select( corrMode_ ) ;

  // selection by TimeSampling
  sdmBin_->select( timeSampling_ ) ;

  // selection by SpectralResolutionType
  sdmBin_->select( resolutionType_ ) ;

  // selection by AtmPhaseCorrection and output CorrelationMode
  EnumSet<AtmPhaseCorrection> esApcs ;
  esApcs.set( apc_ ) ;
  // always take only autocorrelation data
  Enum<CorrelationMode> esCorr = AUTO_ONLY ;
  sdmBin_->selectDataSubset( esCorr, esApcs ) ;

  // select valid configDescriptionId
  selectConfigDescription() ;

  // select valid feedId
  selectFeed() ;
}

casa::Bool ASDMReader::setMainRow( casa::uInt irow ) 
{
  casa::Bool status = true ;
  row_ = irow ;
  execBlockTag_ = mainRow_[row_]->getExecBlockId() ;

  unsigned int cdid = mainRow_[row_]->getConfigDescriptionId().getTagValue() ;
  if ( (int)count(configDescIdList_.begin(),configDescIdList_.end(),cdid) == 0 ) 
    status = false ;
  else {
    status = (casa::Bool)(sdmBin_->acceptMainRow( mainRow_[row_] )) ;
  }
  return status ;
}

casa::Bool ASDMReader::setMainRow( casa::uInt configDescId, casa::uInt fieldId ) 
{
  clearMainRow() ;

  Tag configDescTag( (unsigned int)configDescId, TagType::ConfigDescription ) ;
  Tag fieldTag( (unsigned int)fieldId, TagType::Field ) ;
  vector<MainRow *> *rows = asdm_->getMain().getByContext( configDescTag, fieldTag );
  if (rows == 0)
    return false;
  mainRow_ = casa::Vector<MainRow *>( *rows ) ;
  
  return true ;
}

void ASDMReader::clearMainRow() 
{
  mainRow_.resize(0) ;
}

void ASDMReader::setupIFNO() 
{
  casa::String funcName = "setupIFNO" ;

  vector<SpectralWindowRow *> spwrows = asdm_->getSpectralWindow().get() ;
  unsigned int nrow = spwrows.size() ;
  ifno_.clear() ;
  casa::uInt idx = 0 ;
  casa::uInt wvridx = 0 ;
  for ( unsigned int irow = 0 ; irow < nrow ; irow++ ) {
    casa::uInt index ;
    if ( isWVR( spwrows[irow] ) ) {
      //logsink_->postLocally( LogMessage(spwrows[irow]->getSpectralWindowId().toString()+" is WVR",LogOrigin(className_,funcName,WHERE)) ) ;
      index = wvridx ;
    }
    else {
      index = ++idx ;
    }
    ifno_.insert( pair<Tag,casa::uInt>(spwrows[irow]->getSpectralWindowId(),index) ) ;
    //logsink_->postLocally( LogMessage(spwrows[irow]->getSpectralWindowId().toString()+": IFNO="+String::toString(index),LogOrigin(className_,funcName,WHERE)) ) ;
  }
}

bool ASDMReader::isWVR( SpectralWindowRow *row )
{
  BasebandName bbname = row->getBasebandName() ;
  int nchan = row->getNumChan() ;
  if ( bbname == NOBB && nchan == 4 )
    return true ;
  else 
    return false ;
} 

casa::Bool ASDMReader::setData()
{
  casa::String funcName = "setData" ;

  //logsink_->postLocally( LogMessage("try to retrieve binary data",LogOrigin(className_,funcName,WHERE)) ) ;
  
//   EnumSet<AtmPhaseCorrection> esApcs ;
//   esApcs.set( apc_ ) ;
//   // always take only autocorrelation data
//   Enum<CorrelationMode> esCorr = AUTO_ONLY ;
//   vmsData_ = sdmBin_->getDataCols( esCorr, esApcs ) ;

  // 2011/07/06 TN
  // Workaround to avoid unwanted message from SDMBinData::getDataCols()
  ostringstream oss ;
  streambuf *buforg = cout.rdbuf(oss.rdbuf()) ;
  vmsData_ = sdmBin_->getDataCols() ;
  cout.rdbuf(buforg) ;

//   logsink_->postLocally( LogMessage("oss.str() = "+oss.str(),LogOrigin(className_,funcName,WHERE)) ) ;
//   cout << "This is test: oss.str()=" << oss.str() << endl ;


  //logsink_->postLocally( LogMessage("processorId = "+String::toString(vmsData_->processorId),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_time.size() = "+String::toString(vmsData_->v_time.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("   v_time[0] = "+String::toString(vmsData_->v_time[0]),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_interval.size() = "+String::toString(vmsData_->v_interval.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("   v_interval[0] = "+String::toString(vmsData_->v_interval[0]),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_atmPhaseCorrection.size() = "+String::toString(vmsData_->v_atmPhaseCorrection.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("binNum = "+String::toString(vmsData_->binNum),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_projectPath.size() = "+String::toString(vmsData_->v_projectPath.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_antennaId1.size() = "+String::toString(vmsData_->v_antennaId1.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_antennaId2.size() = "+String::toString(vmsData_->v_antennaId2.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_feedId1.size() = "+String::toString(vmsData_->v_feedId1.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_feedId2.size() = "+String::toString(vmsData_->v_feedId2.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_dataDescId.size() = "+String::toString(vmsData_->v_dataDescId.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_timeCentroid.size() = "+String::toString(vmsData_->v_timeCentroid.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_exposure.size() = "+String::toString(vmsData_->v_exposure.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_numData.size() = "+String::toString(vmsData_->v_numData.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("vv_dataShape.size() = "+String::toString(vmsData_->vv_dataShape.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_m_data.size() = "+String::toString(vmsData_->v_m_data.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_phaseDir.size() = "+String::toString(vmsData_->v_phaseDir.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_stateId.size() = "+String::toString(vmsData_->v_stateId.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_msState.size() = "+String::toString(vmsData_->v_msState.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("v_flag.size() = "+String::toString(vmsData_->v_flag.size()),LogOrigin(className_,funcName,WHERE)) ) ;

  dataIdList_.clear() ;
  unsigned int numTotalData = vmsData_->v_m_data.size() ;
  for ( unsigned int idata = 0 ; idata < numTotalData ; idata++ ) {
    if ( vmsData_->v_antennaId1[idata] == (int)antennaId_
         && vmsData_->v_antennaId2[idata] == (int)antennaId_ ) 
      dataIdList_.push_back( idata ) ;
  }
  numData_ = dataIdList_.size() ;
  //logsink_->postLocally( LogMessage("numData_ = "+String::toString(numData_),LogOrigin(className_,funcName,WHERE)) ) ;
  //logsink_->postLocally( LogMessage("dataSize = "+String::toString(mainRow_[row_]->getDataSize()),LogOrigin(className_,funcName,WHERE)) ) ;

  return true ;
}

void ASDMReader::prepareData( unsigned int idx )
{
  unsigned int i = dataIdList_[idx] ;
  if ( i != dataIndex_ ) {
    dataIndex_ = dataIdList_[idx] ;
    Tag dataDescTag( vmsData_->v_dataDescId[dataIndex_], TagType::DataDescription ) ;
    DataDescriptionRow *dataDescRow = asdm_->getDataDescription().getRowByKey( dataDescTag ) ;
    specWinTag_ = dataDescRow->getSpectralWindowId() ;
    specWinRow_p = dataDescRow->getSpectralWindowUsingSpectralWindowId() ;
    polarizationRow_p = dataDescRow->getPolarizationUsingPolOrHoloId() ;
    Tag fieldTag( vmsData_->v_fieldId[dataIndex_], TagType::Field ) ;
    fieldRow_p = asdm_->getField().getRowByKey( fieldTag ) ;
    double startSec = vmsData_->v_time[dataIndex_] - 0.5 * vmsData_->v_interval[dataIndex_] ;
    timeInterval_ = ArrayTimeInterval( startSec*s2d, vmsData_->v_interval[dataIndex_]*s2d ) ;
  }
}

casa::uInt ASDMReader::getIFNo( unsigned int idx )
{
  prepareData( idx ) ;
  return getIFNo() ;
}

casa::uInt ASDMReader::getIFNo()
{
  map<Tag,casa::uInt>::iterator iter = ifno_.find( specWinTag_ ) ;
  if ( iter != ifno_.end() )
    return iter->second ;
  else {
    return 0 ;
  }
}

int ASDMReader::getNumPol( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getNumPol() ;
}

int ASDMReader::getNumPol()
{
  return polarizationRow_p->getNumCorr() ;
}

void ASDMReader::getFrequency( unsigned int idx, 
                               double &refpix, 
                               double &refval, 
                               double &incr,
                               string &freqref ) 
{
  prepareData( idx ) ;
  getFrequency( refpix, refval, incr, freqref ) ;
}

void ASDMReader::getFrequency( double &refpix, 
                               double &refval, 
                               double &incr,
                               string &freqref ) 
{
  casa::String funcName = "getFrequency" ;

  int nchan = specWinRow_p->getNumChan() ;
  freqref = "TOPO" ;
  if ( specWinRow_p->isMeasFreqRefExists() )
    freqref = CFrequencyReferenceCode::toString( specWinRow_p->getMeasFreqRef() ) ;
  if ( nchan == 1 ) {
    //logsink_->postLocally( LogMessage("channel averaged data",LogOrigin(className_,funcName,WHERE)) ) ;
    refpix = 0.0 ;
    incr = specWinRow_p->getTotBandwidth().get() ;
    if ( specWinRow_p->isChanFreqStartExists() ) {
      refval = specWinRow_p->getChanFreqStart().get() ;
    }
    else if ( specWinRow_p->isChanFreqArrayExists() ) {
      refval = specWinRow_p->getChanFreqArray()[0].get() ;
    }
    else {
      throw (casa::AipsError( "Either chanFreqArray or chanFreqStart must exist." )) ;
    }      
  }
  else if ( nchan % 2 ) {
    // odd
    //logsink_->postLocally( LogMessage("odd case",LogOrigin(className_,funcName,WHERE)) ) ;
    refpix = 0.5 * ( (double)nchan - 1.0 ) ;
    int ic = ( nchan - 1 ) / 2 ;
    if ( specWinRow_p->isChanWidthExists() ) {
      incr = specWinRow_p->getChanWidth().get() ;
    }
    else if ( specWinRow_p->isChanWidthArrayExists() ) {
      incr = specWinRow_p->getChanWidthArray()[0].get() ;
    }
    else {
      throw (casa::AipsError( "Either chanWidthArray or chanWidth must exist." )) ;
    }      
    if ( specWinRow_p->isChanFreqStepExists() ) {
      if ( specWinRow_p->getChanFreqStep().get() < 0.0 ) 
        incr *= -1.0 ;
    }
    else if ( specWinRow_p->isChanFreqArrayExists() ) {
      vector<Frequency> chanFreqArr = specWinRow_p->getChanFreqArray() ;
      if ( chanFreqArr[0].get() > chanFreqArr[1].get() ) 
        incr *= -1.0 ;
    }
    else {
      throw (casa::AipsError( "Either chanFreqArray or chanFreqStep must exist." )) ;
    }          
    if ( specWinRow_p->isChanFreqStartExists() ) {
      refval = specWinRow_p->getChanFreqStart().get() + refpix * incr ;
    }
    else if ( specWinRow_p->isChanFreqArrayExists() ) {
      refval = specWinRow_p->getChanFreqArray()[ic].get() ;
    }
    else {
      throw (casa::AipsError( "Either chanFreqArray or chanFreqStart must exist." )) ;
    }      
  }
  else {
    // even
    //logsink_->postLocally( LogMessage("even case",LogOrigin(className_,funcName,WHERE)) ) ;
    refpix = 0.5 * ( (double)nchan - 1.0 ) ; 
    int ic = nchan / 2 ;
    if ( specWinRow_p->isChanWidthExists() ) {
      incr = specWinRow_p->getChanWidth().get() ;
    }
    else if ( specWinRow_p->isChanWidthArrayExists() ) {
      incr = specWinRow_p->getChanWidthArray()[0].get() ;
    }
    else {
      throw (casa::AipsError( "Either chanWidthArray or chanWidth must exist." )) ;
    }
    if ( specWinRow_p->isChanFreqStepExists() ) {
      if ( specWinRow_p->getChanFreqStep().get() < 0.0 ) 
        incr *= -1.0 ;
    }
    else if ( specWinRow_p->isChanFreqArrayExists() ) {
      vector<Frequency> chanFreqArr = specWinRow_p->getChanFreqArray() ;
      if ( chanFreqArr[0].get() > chanFreqArr[1].get() ) 
        incr *= -1.0 ;
    }
    else {
      throw (casa::AipsError( "Either chanFreqArray or chanFreqStep must exist." )) ;
    }          
    if ( specWinRow_p->isChanFreqStartExists() ) {
      refval = specWinRow_p->getChanFreqStart().get() + refpix * incr ;
    }
    else if ( specWinRow_p->isChanFreqArrayExists() ) {
      vector<Frequency> freqs = specWinRow_p->getChanFreqArray() ;
      refval = 0.5 * ( freqs[ic-1].get() + freqs[ic].get() ) ;      
    }
    else {
      throw (casa::AipsError( "Either chanFreqArray or chanFreqStart must exist." )) ;
    }      
  }
}

double ASDMReader::getTime( unsigned int idx )
{
  prepareData( idx ) ;
  return getTime() ;
}

double ASDMReader::getTime()
{
  double tsec = vmsData_->v_time[dataIndex_] ;
  return tsec * s2d ;
}

double ASDMReader::getInterval( unsigned int idx )
{
  prepareData( idx ) ;
  return getInterval() ;
}

double ASDMReader::getInterval()
{
  return vmsData_->v_interval[dataIndex_] ;
}

void ASDMReader::getSourceProperty( unsigned int idx,
                                    string &srcname,
                                    string &fieldname,
                                    vector<double> &srcdir,
                                    vector<double> &srcpm,
                                    double &sysvel,
                                    vector<double> &restfreq ) 
{
  prepareData( idx ) ;
  getSourceProperty( srcname, fieldname, srcdir, srcpm, sysvel, restfreq ) ;
}

void ASDMReader::getSourceProperty( string &srcname,
                                    string &fieldname,
                                    vector<double> &srcdir,
                                    vector<double> &srcpm,
                                    double &sysvel,
                                    vector<double> &restfreq ) 
{
  String funcName = "getSourceProperty" ;
  ostringstream oss ;
  oss << fieldRow_p->getFieldName() << "__" << vmsData_->v_fieldId[dataIndex_] ;
  fieldname = oss.str() ;
  SourceRow *srow = 0 ; 
  if ( fieldRow_p->isSourceIdExists() ) {
    int sourceId = fieldRow_p->getSourceId() ;
    srow = asdm_->getSource().getRowByKey( sourceId, timeInterval_, specWinTag_ ) ;
  }
  if ( srow != 0 ) {
    //logsink_->postLocally( LogMessage("timeInterval_="+String::toString(timeInterval_.getStart().getMJD()),LogOrigin(className_,funcName,WHERE)) ) ;
    //logsink_->postLocally( LogMessage("specWinTag_="+String::toString(specWinTag_.toString()),LogOrigin(className_,funcName,WHERE)) ) ;
    //SourceRow *srow = asdm_->getSource().getRowByKey( sourceId, timeInterval_, specWinTag_ ) ;
    //logsink_->postLocally( LogMessage("sourceId="+String::toString(sourceId),LogOrigin(className_,funcName,WHERE)) ) ;
    //if ( srow == 0 ) 
    //logsink_->postLocally( LogMessage("nullpo",LogOrigin(className_,funcName,WHERE)) ) ;

    // source name
    srcname = srow->getSourceName() ;
    //logsink_->postLocally( LogMessage("srcname="+String::toString(srcname),LogOrigin(className_,funcName,WHERE)) ) ;

    // source direction
    vector<Angle> srcdirA = srow->getDirection() ;
    srcdir.resize( 2 ) ;
    srcdir[0] = limitedAngle( srcdirA[0].get() ) ;
    srcdir[1] = limitedAngle( srcdirA[1].get() ) ;
    //logsink_->postLocally( LogMessage("srcdir=["+String::toString(srcdir[0])+","+String::toString(srcdir[1])+"]",LogOrigin(className_,funcName,WHERE)) ) ;
    if ( srow->isDirectionCodeExists() ) {
      DirectionReferenceCodeMod::DirectionReferenceCode dircode = srow->getDirectionCode() ;
      //logsink_->postLocally( LogMessage("dircode="+CDirectionReferenceCode::toString(dircode),LogOrigin(className_,funcName,WHERE)) ) ;
      if ( dircode != DirectionReferenceCodeMod::J2000 ) {
        // if not J2000, need direction conversion
        String ref( CDirectionReferenceCode::toString( dircode ) ) ;
        double mjd = vmsData_->v_time[dataIndex_] * s2d ;
        srcdir = toJ2000( srcdir, ref, mjd ) ;
      }
    }

    // source proper motion
    srcpm.resize( 2 ) ;
    vector<AngularRate> srcpmA = srow->getProperMotion() ;
    srcpm[0] = srcpmA[0].get() ;
    srcpm[1] = srcpmA[1].get() ;
    //logsink_->postLocally( LogMessage("srcpm=["+String::toString(srcpm[0])+","+String::toString(srcpm[1])+"]",LogOrigin(className_,funcName,WHERE)) ) ;
    // systemic velocity
    if ( srow->isSysVelExists() ) {
      vector<Speed> sysvelV = srow->getSysVel() ;
      if ( sysvelV.size() > 0 )
        sysvel = sysvelV[0].get() ;
    }
    else {
      sysvel = 0.0 ;
    }
    //logsink_->postLocally( LogMessage("sysvel="+String::toString(sysvel),LogOrigin(className_,funcName,WHERE)) ) ;
    // rest frequency
    if ( srow->isRestFrequencyExists() ) {
      //logsink_->postLocally( LogMessage("restFrequency exists",LogOrigin(className_,funcName,WHERE)) ) ;
      vector<Frequency> rf = srow->getRestFrequency() ; 
      restfreq.resize( rf.size() ) ;
      for ( unsigned int i = 0 ; i < restfreq.size() ; i++ ) 
        restfreq[i] = rf[i].get() ;
    }
    else {
      restfreq.resize( 0 ) ;
    }
    //logsink_->postLocally( LogMessage("restfreq.size()="+String::toString(restfreq.size()),LogOrigin(className_,funcName,WHERE)) ) ;
  }
  else {
    srcname = fieldRow_p->getFieldName() ;
    srcdir.resize( 2 ) ;
    srcdir[0] = 0.0 ;
    srcdir[1] = 0.0 ;
    srcpm = srcdir ;
    sysvel = 0.0 ;
    restfreq.resize( 0 ) ;
  }
}

int ASDMReader::getSrcType( unsigned int scan,
                            unsigned int subscan ) 
{
  int srctype = SrcType::NOTYPE ;
  ScanRow *scanrow = asdm_->getScan().getRowByKey( execBlockTag_, (int)scan ) ;
  ScanIntentMod::ScanIntent scanIntent = scanrow->getScanIntent()[0] ;
  SubscanRow *subrow = asdm_->getSubscan().getRowByKey( execBlockTag_, (int)scan, (int)subscan ) ;
  if ( subrow == 0 ) {
    return srctype ;
  }
  SubscanIntentMod::SubscanIntent subIntent = subrow->getSubscanIntent() ;
  SwitchingModeMod::SwitchingMode swmode = SwitchingModeMod::NO_SWITCHING ;
  if ( subrow->isSubscanModeExists() )
    swmode = subrow->getSubscanMode() ;
  if ( scanIntent == ScanIntentMod::OBSERVE_TARGET ) {
    // on sky scan
    if ( swmode == SwitchingModeMod::NO_SWITCHING || swmode == SwitchingModeMod::POSITION_SWITCHING ) {
      // position switching
      // tentatively set NO_SWITCHING = POSITION_SWITCHING
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::PSON ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::PSOFF ;
      }
    }
    else if ( swmode == SwitchingModeMod::FREQUENCY_SWITCHING ) {
      // frequency switching
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::FSON ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::FSOFF ;
      }
    }
    else if ( swmode == SwitchingModeMod::NUTATOR_SWITCHING ) {
      // nutator switching
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::PSON ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::PSOFF ;
      }
    }
    else {
      // other switching mode
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::SIG ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::REF ;
      }
    }
  }
  else if ( scanIntent == ScanIntentMod::CALIBRATE_ATMOSPHERE ) {
    if ( swmode == SwitchingModeMod::NO_SWITCHING || swmode == SwitchingModeMod::POSITION_SWITCHING ) {
      // position switching
      // tentatively set NO_SWITCHING = POSITION_SWITCHING
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::PONCAL ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::POFFCAL ;
      }
    }
    else if ( swmode == SwitchingModeMod::FREQUENCY_SWITCHING ) {
      // frequency switching
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::FONCAL ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::FOFFCAL ;
      }
    }
    else if ( swmode == SwitchingModeMod::NUTATOR_SWITCHING ) {
      // nutator switching
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::PONCAL ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::POFFCAL ;
      }
    }
    else {
      // other switching mode
      if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
        srctype = SrcType::CAL ;
      }
      else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
        srctype = SrcType::CAL ;
      }
    }
  }
  else {
    // off sky (e.g. calibrator device) scan
    if ( subIntent == SubscanIntentMod::ON_SOURCE ) {
      srctype = SrcType::SIG ;
    }
    else if ( subIntent == SubscanIntentMod::OFF_SOURCE ) {
      srctype = SrcType::REF ;
    }
    else if ( subIntent == SubscanIntentMod::HOT ) {
      srctype = SrcType::HOT ;
    }
    else if ( subIntent == SubscanIntentMod::AMBIENT ) {
      srctype = SrcType::SKY ;
    }
    else {
      srctype = SrcType::CAL ;
    }
  }

  return srctype ;
}

unsigned int ASDMReader::getSubscanNo( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getSubscanNo() ;
}

unsigned int ASDMReader::getSubscanNo() 
{
  return mainRow_[row_]->getSubscanNumber() ;
}

void ASDMReader::getSourceDirection( vector<double> &dir, string &ref )
{
  dir.resize( 2 ) ;
  ref = "J2000" ; // default is J2000
  SourceTable &tab = asdm_->getSource() ;
  SourceRow *row = tab.get()[0] ;
  vector<Angle> dirA = row->getDirection() ;
  dir[0] = dirA[0].get() ;
  dir[1] = dirA[1].get() ;
  if ( row->isDirectionCodeExists() ) {
    ref = CDirectionReferenceCode::toString( row->getDirectionCode() ) ;
  }
}

unsigned int ASDMReader::getFlagRow( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getFlagRow() ;
}

unsigned int ASDMReader::getFlagRow() 
{
  return vmsData_->v_flag[dataIndex_] ;
}

vector<unsigned int> ASDMReader::getDataShape( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getDataShape() ;
}

vector<unsigned int> ASDMReader::getDataShape() 
{
  return vmsData_->vv_dataShape[dataIndex_] ;
}

float * ASDMReader::getSpectrum( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getSpectrum() ;
}

float * ASDMReader::getSpectrum() 
{
  map<AtmPhaseCorrection, float*> data = vmsData_->v_m_data[dataIndex_] ;
  //map<AtmPhaseCorrection, float*>::iterator iter = data.find(AP_UNCORRECTED) ;
  map<AtmPhaseCorrection, float*>::iterator iter = data.find(apc_) ;
  float *autoCorr = iter->second ;
  return autoCorr ;
}

vector< vector<float> > ASDMReader::getTsys( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getTsys() ;
}

vector< vector<float> > ASDMReader::getTsys() 
{
  vector< vector<float> > defaultTsys( 1, vector<float>( 1, 1.0 ) ) ;
  SysCalRow *scrow = getSysCalRow() ;
  if ( scrow != 0 && scrow->isTsysSpectrumExists() ) {
    vector< vector<Temperature> > tsysSpec = scrow->getTsysSpectrum() ;
    unsigned int numReceptor = tsysSpec.size() ;
    unsigned int numChan = tsysSpec[0].size() ;
    vector< vector<float> > tsys( numReceptor, vector<float>( numChan, 1.0 ) ) ;
    for ( unsigned int ir = 0 ; ir < numReceptor ; ir++ ) {
      for ( unsigned int ic = 0 ; ic < numChan ; ic++ ) {
        tsys[ir][ic] = tsysSpec[ir][ic].get() ;
      }
    }
    return tsys ;
  }
//   else if ( scrow->isTsysExists() ) {
//     vector<Temperature> tsysScalar = scrow->getTsys() ;
//     unsigned int numReceptor = tsysScalar.size() ;
//     vector< vector<float> > tsys( numReceptor ) ;
//     for ( unsigned int ir = 0 ; ir < numReceptor ; ir++ ) 
//       tsys[ir] = vector<float>( 1, tsysScalar[ir].get() ) ;
//     return tsys ;
//   }
  else {
    return defaultTsys ;
  }
} 

vector< vector<float> > ASDMReader::getTcal( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getTcal() ;
}

vector< vector<float> > ASDMReader::getTcal() 
{
  vector< vector<float> > defaultTcal( 1, vector<float>( 1, 1.0 ) ) ;
  SysCalRow *scrow = getSysCalRow() ;
  if ( scrow != 0 && scrow->isTcalSpectrumExists() ) {
    vector< vector<Temperature> > tcalSpec = scrow->getTcalSpectrum() ;
    unsigned int numReceptor = tcalSpec.size() ;
    unsigned int numChan = tcalSpec[0].size() ;
    vector< vector<float> > tcal( numReceptor, vector<float>( numChan, 1.0 ) ) ;
    for ( unsigned int ir = 0 ; ir < numReceptor ; ir++ ) {
      for ( unsigned int ic = 0 ; ic < numChan ; ic++ ) {
        tcal[ir][ic] = tcalSpec[ir][ic].get() ;
      }
    }
    return tcal ;
  }
//   else if ( scrow->isTcalExists() ) {
//     vector<Temperature> tcalScalar = scrow->getTcal() ;
//     unsigned int numReceptor = tcalScalar.size() ;
//     vector< vector<float> > tcal( numReceptor, vector<float>( 1, 1.0 ) ) ;
//     for ( unsigned int ir = 0 ; ir < numReceptor ; ir++ ) 
//       tcal[ir][0] = tcalScalar[ir][0].get() ;
//     return tcal ;
//   }
  else {
    return defaultTcal ;
  }
} 

void ASDMReader::getTcalAndTsys( unsigned int idx,
                                 vector< vector<float> > &tcal,
                                 vector< vector<float> > &tsys ) 
{
  prepareData( idx ) ;
  getTcalAndTsys( tcal, tsys ) ;
}

void ASDMReader::getTcalAndTsys( vector< vector<float> > &tcal,
                                 vector< vector<float> > &tsys ) 
{
  String funcName = "getTcalAndTsys" ;

  vector< vector<float> > defaultT( 1, vector<float>( 1, 1.0 ) ) ;
  SysCalRow *scrow = getSysCalRow() ;
  //logsink_->postLocally( LogMessage("scrow = "+String::toString((long)scrow),LogOrigin(className_,funcName,WHERE)) ) ;
  if ( scrow == 0 ) {
    tcal = defaultT ;
    tsys = defaultT ;
  }
  else {
    if ( scrow->isTsysSpectrumExists() ) {
      vector< vector<Temperature> > tsysSpec = scrow->getTsysSpectrum() ;
      unsigned int numReceptor = tsysSpec.size() ;
      unsigned int numChan = tsysSpec[0].size() ;
      //logsink_->postLocally( LogMessage("TSYS: numReceptor = "+String::toString(numReceptor),LogOrigin(className_,funcName,WHERE)) ) ;
      //logsink_->postLocally( LogMessage("TSYS:numChan = "+String::toString(numChan),LogOrigin(className_,funcName,WHERE)) ) ;
      tsys.resize( numReceptor ) ;
      for ( unsigned int ir = 0 ; ir < numReceptor ; ir++ ) {
        tsys[ir].resize( numChan ) ;
        for ( unsigned int ic = 0 ; ic < numChan ; ic++ ) {
          tsys[ir][ic] = tsysSpec[ir][ic].get() ;
        }
      }
    }
    else {
      tsys = defaultT ;
    }
    if ( scrow->isTcalSpectrumExists() ) {
      vector< vector<Temperature> > tcalSpec = scrow->getTcalSpectrum() ;
      unsigned int numReceptor = tcalSpec.size() ;
      unsigned int numChan = tcalSpec[0].size() ;
      //logsink_->postLocally( LogMessage("TCAL: numReceptor = "+String::toString(numReceptor),LogOrigin(className_,funcName,WHERE)) ) ;
      //logsink_->postLocally( LogMessage("TCAL: numChan = "+String::toString(numChan),LogOrigin(className_,funcName,WHERE)) ) ;
      tcal.resize( numReceptor ) ;
      for ( unsigned int ir = 0 ; ir < numReceptor ; ir++ ) {
        tcal[ir].resize( numChan ) ;
        for ( unsigned int ic = 0 ; ic < numChan ; ic++ ) {
          tcal[ir][ic] = tcalSpec[ir][ic].get() ;
        }
      }
    }
    else {
      tcal = defaultT ;
    }
  }
}

vector<float> ASDMReader::getOpacity( unsigned int idx ) 
{
  prepareData( idx ) ;
  return getOpacity() ;
}

vector<float> ASDMReader::getOpacity() 
{
  vector<float> tau(0) ;
  CalAtmosphereTable &atmtab = asdm_->getCalAtmosphere() ;
  unsigned int nrow = atmtab.size() ;
  if ( nrow > 0 ) {
    //int feedid = vmsData_->v_feedId1[index] ;
    //BasebandName bbname = spwrow->getBasebandName() ;
    //FeedRow *frow = asdm_->getFeed().getRowByKey( atag, spwtag, tint, feedid ) ;
    //int nfeed = frow->getNumReceptor() ;
    //vector<ReceiverRow *> rrows = frow->getReceivers() ;
    vector<CalAtmosphereRow *> atmrows = atmtab.get() ;
    //ReceiverBand rb = rrows[0]->getFrequencyBand() ;
    int row0 = -1 ;
    //double eps = DBL_MAX ;
    double eps = 1.0e10 ;
    for ( unsigned int irow = 0 ; irow < nrow ; irow++ ) {
      CalAtmosphereRow *atmrow = atmrows[irow] ;
      if ( casa::String(atmrow->getAntennaName()) != antennaName_ 
           //|| atmrow->getReceiverBand() != rb 
           //|| atmrow->getBasebandName() != bbname 
           || atmrow->getCalDataUsingCalDataId()->getCalType() != CalTypeMod::CAL_ATMOSPHERE ) 
        continue ;
      else {
        double dt = timeInterval_.getStart().getMJD() - atmrow->getEndValidTime().getMJD() ; 
        if ( dt >= 0 && dt < eps ) {
          eps = dt ;
          row0 = (int)irow ;
        }
      }
    }
    if ( row0 != -1 ) {
      CalAtmosphereRow *atmrow = atmrows[row0] ;
      tau = atmrow->getTau() ;
    }
    else {
      tau.resize( 1 ) ;
      tau[0] = 0.0 ;
    }
  }
  else {
    tau.resize( 1 ) ;
    tau[0] = 0.0 ;
  }
  return tau ;
}

void ASDMReader::getWeatherInfo( unsigned int idx,
                                 float &temperature,
                                 float &pressure,
                                 float &humidity,
                                 float &windspeed,
                                 float &windaz ) 
{
  prepareData( idx ) ;
  getWeatherInfo( temperature, pressure, humidity, windspeed, windaz ) ;
}

void ASDMReader::getWeatherInfo( float &temperature,
                                 float &pressure,
                                 float &humidity,
                                 float &windspeed,
                                 float &windaz ) 
{
  casa::String funcName = "getWeatherInfo" ;

  temperature = 0.0 ;
  pressure = 0.0 ;
  humidity = 0.0 ;
  windspeed = 0.0 ;
  windaz = 0.0 ;

  //logsink_->postLocally( LogMessage("weatherStationId_ = "+String::toString(weatherStationId_),LogOrigin(className_,funcName,WHERE)) ) ;

  WeatherTable &wtab = asdm_->getWeather() ;
  if ( wtab.size() == 0 || weatherStationId_ == -1 ) 
    return ;

  Tag sttag( (unsigned int)weatherStationId_, TagType::Station ) ;
  //WeatherRow *wrow = wtab.getRowByKey( sttag, tint ) ;
  vector<WeatherRow *> *wrows = wtab.getByContext( sttag ) ;
  WeatherRow *wrow = (*wrows)[0] ;
  unsigned int nrow = wrows->size() ;
  //logsink_->postLocally( LogMessage("There are "+String::toString(nrow)+" rows for given context: stationId "+String::toString(weatherStationId_),LogOrigin(className_,funcName,WHERE)) ) ;
  ArrayTime startTime = getMidTime( timeInterval_ ) ;
  if ( startTime < (*wrows)[0]->getTimeInterval().getStart() ) {
    temperature = (*wrows)[0]->getTemperature().get() ;
    pressure = (*wrows)[0]->getPressure().get() ; 
    humidity = (*wrows)[0]->getRelHumidity().get() ;
    windspeed = (*wrows)[0]->getWindSpeed().get() ;
    windaz = (*wrows)[0]->getWindDirection().get() ;
  }
  else if ( startTime > getEndTime( (*wrows)[nrow-1]->getTimeInterval() ) ) {
    temperature = (*wrows)[nrow-1]->getTemperature().get() ;
    pressure = (*wrows)[nrow-1]->getPressure().get() ;
    humidity = (*wrows)[nrow-1]->getRelHumidity().get() ;
    windspeed = (*wrows)[nrow-1]->getWindSpeed().get() ;
    windaz = (*wrows)[nrow-1]->getWindDirection().get() ;
  }
  else {
    for ( unsigned int irow = 1 ; irow < wrows->size() ; irow++ ) {
      wrow = (*wrows)[irow-1] ;
      ArrayTime tStart = wrow->getTimeInterval().getStart() ;
      ArrayTime tEnd = (*wrows)[irow]->getTimeInterval().getStart() ;
      if ( startTime >= tStart && startTime <= tEnd ) {
        temperature = wrow->getTemperature().get() ;
        pressure = wrow->getPressure().get() ;
        humidity = wrow->getRelHumidity().get() ;
        windspeed = wrow->getWindSpeed().get() ;
        windaz = wrow->getWindDirection().get() ;
        break ;
      }
    }
  }

  // Pa -> hPa
  pressure /= 100.0 ;

  return ;
}

void ASDMReader::processStation() 
{
  antennaPad_.resize(0) ;
  weatherStation_.resize(0) ;
  StationTable &stab = asdm_->getStation() ;
  vector<StationRow *> srows = stab.get() ;
  for ( unsigned int irow = 0 ; irow < srows.size() ; irow++ ) {
    StationTypeMod::StationType stype = srows[irow]->getType() ;
    Tag stag = srows[irow]->getStationId() ;
    if ( stype == StationTypeMod::ANTENNA_PAD )
      antennaPad_.push_back( stag ) ;
    else if ( stype == StationTypeMod::WEATHER_STATION )
      weatherStation_.push_back( stag ) ;
  }

   weatherStationId_ = getClosestWeatherStation() ;
}

int ASDMReader::getClosestWeatherStation()
{
  if ( weatherStation_.size() == 0 ) 
    return -1 ;

  vector<double> apos( 3 ) ;
  StationTable &stab = asdm_->getStation() ; 
  apos[0] = antennaPosition_[0].getValue( Unit("m") ) ;
  apos[1] = antennaPosition_[1].getValue( Unit("m") ) ;
  apos[2] = antennaPosition_[2].getValue( Unit("m") ) ;
  
  //double eps = DBL_MAX ;
  double eps = 1.0e10 ;
  int retval = -1 ;
  for ( unsigned int ir = 0 ; ir < weatherStation_.size() ; ir++ ) {
    StationRow *srow = stab.getRowByKey( weatherStation_[ir] ) ;
    vector<Length> wpos = srow->getPosition() ;
    double dist = (apos[0]-wpos[0].get())*(apos[0]-wpos[0].get())
      + (apos[1]-wpos[1].get())*(apos[1]-wpos[1].get())
      + (apos[2]-wpos[2].get())*(apos[2]-wpos[2].get()) ;
    if ( dist < eps ) {
      retval = (int)(weatherStation_[ir].getTagValue()) ;
    }
  }

  return retval ;
}

void ASDMReader::getPointingInfo( unsigned int idx,
                                  vector<double> &dir,
                                  double &az,
                                  double &el,
                                  vector<double> &srate ) 
{
  prepareData( idx ) ;
  getPointingInfo( dir, az, el, srate ) ;
}

void ASDMReader::getPointingInfo( vector<double> &dir,
                                  double &az,
                                  double &el,
                                  vector<double> &srate ) 
{
  String funcName = "getPointingInfo" ;

  dir.resize(0) ;
  az = -1.0 ;
  el = -1.0 ;
  srate.resize(0) ;

  //Tag atag( antennaId_, TagType::Antenna ) ;
  //unsigned int index = dataIdList_[idx] ;
  //vector<PointingRow *> *prows = asdm_->getPointing().getByContext( atag ) ;
  vector<PointingRow *> *prows = asdm_->getPointing().getByContext( antennaTag_ ) ;

  if ( prows == 0 )
    return ;

  PointingRow *prow ;
  PointingRow *qrow ;
  //ArrayTimeInterval tint( vmsData_->v_time[index]*s2d, vmsData_->v_interval[index]*s2d ) ;
  //double startSec = vmsData_->v_time[index] - 0.5 * vmsData_->v_interval[index] ;
  //ArrayTimeInterval tint( startSec*s2d, vmsData_->v_interval[index]*s2d ) ;

  unsigned int nrow = prows->size() ;
  //logsink_->postLocally( LogMessage("There are " << nrow << " rows for given context: antennaId "+String::toString(antennaId_),LogOrigin(className_,funcName,WHERE)) ) ;

//   for ( unsigned int irow = 0 ; irow < nrow ; irow++ ) {
//     prow = (*prows)[irow] ;
//     ArrayTimeInterval ati = prow->getTimeInterval() ;
//     ArrayTime pst = ati.getStart() ;
//     ArrayTime pet( ati.getStartInMJD()+ati.getDurationInDays() ) ;
//     logsink_->postLocally( LogMessage("start: "+pst.toFITS(),LogOrigin(className_,funcName,WHERE)) ) ;
//     logsink_->postLocally( LogMessage("end: "+pet.toFITS(),LogOrigin(className_,funcName,WHERE)) ) ;
//   }
  
  srate.resize(2) ;
  srate[0] = 0.0 ;
  srate[1] = 0.0 ;
  az = 0.0 ;
  el = 0.0 ;
  //double tcen = 0.0 ;
  //double tcen = getMidTime( tint ).getMJD() ;
  double tcen = getMidTime( timeInterval_ ).getMJD() ;

  // 
  // shape of pointingDirection is (numSample,2) if usePolynomial = False, while it is 
  // (numTerm,2) if usePolynomial = True.
  // 
  // In the former case, typical sampled time interval is 48msec, which is very small 
  // compared with typical integration time (~a few sec). 
  // Scan rate for this case is always [0,0] (or get slope?).
  //
  // In the later case, scan rate is (pointingDirection[1][0],pointingDirection[1][1])
  //
  // PointingDirection seems to store AZEL
  //
  ArrayTimeInterval pTime0 = (*prows)[0]->getTimeInterval() ;
  ArrayTimeInterval pTime1 = (*prows)[nrow-1]->getTimeInterval() ;
  //if ( tint.getStartInMJD()+tint.getDurationInDays() < pTime0.getStartInMJD() ) {
  //if ( getEndTime( tint ) < getStartTime( pTime0 ) ) {
  if ( getEndTime( timeInterval_ ) < getStartTime( pTime0 ) ) {
    logsink_->postLocally( LogMessage( "ArrayTimeInterval out of bounds: no data for given position (tint < ptime)", LogOrigin(className_,funcName,WHERE), LogMessage::WARN ) ) ;
    prow = (*prows)[0] ;
    vector< vector<double> > dirA = pointingDir( prow ) ;
    az = dirA[0][0] ;
    el = dirA[0][1] ;
    if ( prow->getUsePolynomials() && prow->getNumTerm() > 1 ) {
      srate[0] = dirA[1][0] ;
      srate[1] = dirA[1][1] ;
    }      
  }
  //else if ( tint.getStartInMJD() > pTime1.getStartInMJD()+pTime1.getDurationInDays() ) {
  //else if ( getStartTime( tint ) > getEndTime( pTime1 ) ) {
  else if ( getStartTime( timeInterval_ ) > getEndTime( pTime1 ) ) {
    logsink_->postLocally( LogMessage( "ArrayTimeInterval out of bounds: no data for given position (tint > ptime)", LogOrigin(className_,funcName,WHERE), LogMessage::WARN ) ) ;
    prow = (*prows)[nrow-1] ;
    int numSample = prow->getNumSample() ;
    vector< vector<double> > dirA = pointingDir( prow ) ;
    if ( prow->getUsePolynomials() ) {
      az = dirA[0][0] ;
      el = dirA[0][1] ; 
      if ( prow->getNumTerm() > 1 ) {
        srate[0] = dirA[1][0] ;
        srate[1] = dirA[1][1] ;
      }
    }
    else {
      az = dirA[numSample-1][0] ;
      el = dirA[numSample-1][1] ;
    }
  }
  else {
    //ArrayTime startTime = tint.getStart() ;
    //ArrayTime endTime = getEndTime( tint ) ;
    ArrayTime startTime = timeInterval_.getStart() ;
    ArrayTime endTime = getEndTime( timeInterval_ ) ;
    int row0 = -1 ;
    int row1 = -1 ;
    int row2 = -1 ;
    //double dt0 = getMidTime( tint ).getMJD() ;
    double dt0 = getMidTime( timeInterval_ ).getMJD() ;
    for ( unsigned int irow = 0 ; irow < nrow ; irow++ ) {
      prow = (*prows)[irow] ;
      //double dt = getMidTime( tint ).getMJD() - getMidTime( prow->getTimeInterval() ).getMJD() ;
      double dt = getMidTime( timeInterval_ ).getMJD() - getMidTime( prow->getTimeInterval() ).getMJD() ;
      if ( dt > 0 && dt < dt0 ) {
        dt0 = dt ;
        row2 = irow ;
      }
      if ( prow->getTimeInterval().contains( startTime ) ) 
        row0 = irow ;
      else if ( prow->getTimeInterval().contains( endTime ) ) 
        row1 = irow ;
      if ( row0 != -1 && row1 != -1 ) 
        break ;
    }
    //logsink_->postLocally( LogMessage("row0 = "+String::toString(row0)+", row1 = "+String::toString(row1)+", row2 = "+String::toString(row2),LogOrigin(className_,funcName,WHERE)) ) ;
    if ( row0 == -1 && row1 == -1 ) {
      prow = (*prows)[row2] ;
      qrow = (*prows)[row2+1] ;
      double t0 = getEndTime( prow->getTimeInterval() ).getMJD() ;
      double t1 = qrow->getTimeInterval().getStartInMJD() ;
      vector< vector<double> > dirA = pointingDir( prow ) ;
      vector< vector<double> > dirB = pointingDir( qrow ) ;
      double da0 = dirA[0][0] ;
      double db0 = dirB[0][0] ;
      double da1 = dirA[0][1] ;
      double db1 = dirB[0][1] ;
      if ( prow->getUsePolynomials() && qrow->getUsePolynomials() ) {
        double dt = ( tcen - t0 ) / ( t1 - t0 ) ;
        az = da0 + ( db0 - da0 ) * dt ;
        el = da1 + ( db1 - da1 ) * dt ;
        if ( prow->getNumTerm() > 0 && qrow->getNumTerm() > 1 ) {
          double ra0 = dirA[1][0] ;
          double rb0 = dirB[1][0] ;
          double ra1 = dirA[1][1] ;
          double rb1 = dirB[1][1] ;
          srate[0] = ra0 + ( rb0 - ra0 ) * dt ;
          srate[1] = ra1 + ( rb1 - ra1 ) * dt ;
        }
      }
      else if ( !(qrow->getUsePolynomials()) ) {
        double dt = ( tcen - t0 ) / ( t1 - t0 ) ;
        az = da0 + ( db0 - da0 ) * dt ;
        el = da1 + ( db1 - da1 ) * dt ;
      }
      //else {
      // nothing to do
      //}
    }
    else {
      int ndir = 0 ;
      if ( row0 == -1 ) {
        row0 = row1 ;
      }
      else if ( row1 == -1 ) {
        row1 = row0 ;
      }
      prow = (*prows)[row0] ;
      qrow = (*prows)[row1] ;
      if ( prow->getUsePolynomials() && qrow->getUsePolynomials() ) {
        //logsink_->postLocally( LogMessage("usePolynomial = True",LogOrigin(className_,funcName,WHERE)) ) ;
        if ( row0 == row1 ) {
          prow = (*prows)[row0] ;
          vector< vector<double> > dirA = pointingDir( prow ) ;
          az = dirA[0][0] ;
          el = dirA[0][1] ; 
          if ( prow->getNumTerm() > 1 ) {
            srate[0] = dirA[1][0] ;
            srate[1] = dirA[1][1] ;
          }
        }
        else {
          prow = (*prows)[row0] ;
          qrow = (*prows)[row1] ;
          vector< vector<double> > dirA = pointingDir( prow ) ;
          vector< vector<double> > dirB = pointingDir( qrow ) ;
          double t0 = getMidTime( prow->getTimeInterval() ).getMJD() ;
          double t1 = getMidTime( qrow->getTimeInterval() ).getMJD() ;
          double dt = ( tcen - t0 ) / ( t1 - t0 ) ;
          double da0 = dirA[0][0] ;
          double db0 = dirB[0][0] ;
          double da1 = dirA[0][1] ;
          double db1 = dirB[0][1] ;
          az = da0 + ( db0 - da0 ) * dt ;
          el = da1 + ( db1 - db0 ) * dt ;
          if ( prow->getNumTerm() > 0 && qrow->getNumTerm() > 1 ) {
            double ra0 = dirA[1][0] ;
            double rb0 = dirB[1][0] ;
            double ra1 = dirA[1][1] ;
            double rb1 = dirB[1][1] ;
            srate[0] = ra0 + ( rb0 - ra0 ) * dt ;
            srate[1] = ra1 + ( rb1 - ra1 ) * dt ;
          }
        }
      }
      else if ( prow->getUsePolynomials() == qrow->getUsePolynomials() ) {
        //logsink_->postLocally( LogMessage("numSample == numTerm",LogOrigin(className_,funcName,WHERE)) ) ;
        tcen = 0.0 ;
        for ( int irow = row0 ; irow <= row1 ; irow++ ) {
          prow = (*prows)[irow] ;
          int numSample = prow->getNumSample() ;
          //logsink_->postLocally( LogMessage("numSample = "+String::toString(numSample),LogOrigin(className_,funcName,WHERE)) ) ;
          vector< vector<double> > dirA = pointingDir( prow ) ;
          if ( prow->isSampledTimeIntervalExists() ) {
            //logsink_->postLocally( LogMessage("sampledTimeIntervalExists",LogOrigin(className_,funcName,WHERE)) ) ;
            vector<ArrayTimeInterval> stime = prow->getSampledTimeInterval() ; 
            for ( int isam = 0 ; isam < numSample ; isam++ ) {
              //if ( tint.overlaps( stime[isam] ) ) {
              //if ( tint.contains( stime[isam] ) ) {
              if ( timeInterval_.contains( stime[isam] ) ) {
                az += dirA[isam][0] ;
                el += dirA[isam][1] ;
                tcen += getMidTime( stime[isam] ).getMJD() ;
                ndir++ ;
              }
            }
          }
          else {
            double sampleStart = prow->getTimeInterval().getStartInMJD() ; 
            double sampleInterval = prow->getTimeInterval().getDurationInDays() / (double)numSample ;
            //logsink_->postLocally( LogMessage("sampleStart = "+String::toString(sampleStart),LogOrigin(className_,funcName,WHERE)) )
            //logsink_->postLocally( LogMessage("sampleInterval = "+String::toString(sampleInterval),LogOrigin(className_,funcName,WHERE)) ) ;
            //logsink_->postLocally( LogMessage("tint = "+tint.toString(),LogOrigin(className_,funcName,WHERE)) ) ;
            for ( int isam = 0 ; isam < numSample ; isam++ ) {
              ArrayTimeInterval stime( sampleStart+isam*sampleInterval, sampleInterval ) ;
              //if ( tint.overlaps( stime ) ) {
              //if ( tint.contains( stime ) ) {
              if ( timeInterval_.contains( stime ) ) {
                az += dirA[isam][0] ;
                el += dirA[isam][1] ;
                tcen += getMidTime( stime ).getMJD() ;
                ndir++ ;
              }
            }
          }
        }
        if ( ndir > 1 ) {
          az /= (double)ndir ;
          el /= (double)ndir ;
          tcen /= (double)ndir ;
        }
      }
      //else {
      // nothing to do
      //}
    }
    
    toJ2000( dir, az, el, tcen ) ;
  }

  return ;
}

ArrayTime ASDMReader::getMidTime( const ArrayTimeInterval &t ) 
{
  return ArrayTime( t.getStartInMJD() + 0.5 * t.getDurationInDays() ) ;
}

ArrayTime ASDMReader::getEndTime( const ArrayTimeInterval &t ) 
{
  return ArrayTime( t.getStartInMJD() + t.getDurationInDays() ) ;
}

ArrayTime ASDMReader::getStartTime( const ArrayTimeInterval &t ) 
{
  return ArrayTime( t.getStartInMJD() ) ;
}

void ASDMReader::toJ2000( vector<double> &dir,
                          double &az, 
                          double &el,
                          double &mjd )
{
  String funcName = "toJ2000" ;

  String ref = "AZELGEO" ;
  vector<double> azel( 2 ) ;
  azel[0] = az ;
  azel[1] = el ;
  dir = toJ2000( azel, ref, mjd ) ;
}

vector<double> ASDMReader::toJ2000( vector<double> &dir,
                                    casa::String &dirref,
                                    double &mjd )
{
  casa::String funcName = "toJ2000" ;

  vector<double> newd( dir ) ;
  if ( dirref != "J2000" ) {
    me_ = casa::MEpoch( casa::Quantity( (casa::Double)mjd, "d" ), casa::MEpoch::UTC ) ;
    mf_.set( me_ ) ;
    casa::MDirection::Types dirtype ;
    casa::Bool b = casa::MDirection::getType( dirtype, dirref ) ;
    if ( b ) {
      casa::Vector<casa::Double> cdir = toj2000_( dir ).getAngle( "rad" ).getValue() ; 
      //logsink_->postLocally( LogMessage("cdir = "+String::toString(cdir),LogOrigin(className_,funcName,WHERE)) ) ;
      newd[0] = (double)(cdir[0]) ;
      newd[1] = (double)(cdir[1]) ;
    }
  }
  return newd ;
}

void ASDMReader::setLogger( CountedPtr<LogSinkInterface> &logsink )
{
  logsink_ = logsink ;
}

string ASDMReader::getFrame()
{
  casa::String funcName = "getFrame" ;
 
  // default is TOPO
  string frame = "TOPO" ;

  SpectralWindowTable &spwtab = asdm_->getSpectralWindow() ;
  vector<SpectralWindowRow *> rows = spwtab.get() ;
  vector<FrequencyReferenceCodeMod::FrequencyReferenceCode> measFreqRef( rows.size() ) ;
  int nref = 0 ;
  for ( unsigned int irow = 0 ; irow < rows.size() ; irow++ ) {
    int nchan = rows[irow]->getNumChan() ;
    if ( nchan != 4 ) {
      if ( rows[irow]->isMeasFreqRefExists() ) {
        measFreqRef[nref] = rows[irow]->getMeasFreqRef() ;
        nref++ ;
      }
    }
  }
  if ( nref != 0 ) {
    frame = CFrequencyReferenceCode::toString( measFreqRef[0] ) ;
  }
  
  //logsink_->postLocally( LogMessage("frame = "+String::toString(frame),LogOrigin(className_,funcName,WHERE)) ) ;

  return frame ;
}

int ASDMReader::getNumIFs()
{
  casa::String funcName = "getNumIFs" ;

  int nif = 0 ;
  vector<SpectralWindowRow *> rows = asdm_->getSpectralWindow().get() ;
  unsigned int nrow = rows.size() ;
  // check if all rows have freqGroup attribute
  bool freqGroupExists = true ;
  bool countedWvr = false ;
  for ( unsigned int irow = 0 ; irow < nrow ; irow++ ) {
    freqGroupExists &= rows[irow]->isFreqGroupExists() ;
    if ( rows[irow]->getNumChan() == 4 ) {
      if ( !countedWvr ) {
        countedWvr = true ;
        nif++ ;
      }
    }
    else {
      nif++ ;
    }
  }
  
  if ( freqGroupExists ) {
    vector<int> freqGroup(0) ;
    for ( unsigned int irow = 0 ; irow < nrow ; irow++ ) {
      int fg = rows[irow]->getFreqGroup() ;
      if ( (int)count( freqGroup.begin(), freqGroup.end(), fg ) == 0 ) {
        freqGroup.push_back( fg ) ;
      }
    }
    nif = freqGroup.size() ;
  }

  //logsink_->postLocally( LogMessage("nif = "+String::toString(nif),LogOrigin(className_,funcName,WHERE)) ) ;

  return nif ;
}

SysCalRow *ASDMReader::getSysCalRow( unsigned int idx )
{
  prepareData( idx ) ;
  return getSysCalRow() ;
}

SysCalRow *ASDMReader::getSysCalRow()
{
  String funcName = "getSysCalRow" ;

  SysCalRow *row = 0 ;
  int feedid = vmsData_->v_feedId1[dataIndex_] ;
  SysCalTable &sctab = asdm_->getSysCal() ;
  vector<SysCalRow *> *rows = sctab.getByContext( antennaTag_, specWinTag_, feedid ) ;
  //if ( nrow == 0 ) {
  if ( rows == 0 ) {
    //logsink_->postLocally( LogMessage("no rows in SysCal table",LogOrigin(className_,funcName,WHERE)) ) ;
    row = 0 ;
  }
  else {
    unsigned int nrow = rows->size() ;
    //logsink_->postLocally( LogMessage("nrow = "+String::toString(nrow),LogOrigin(className_,funcName,WHERE)) ) ;
    int scindex = -1 ;
    if ( nrow == 1 ) {
      scindex = 0 ;
    }
    else if ( getEndTime( timeInterval_ ) <= getStartTime( (*rows)[0]->getTimeInterval() ) )
      scindex = 0 ;
    else {
      for ( unsigned int irow = 0 ; irow < nrow-1 ; irow++ ) {
        ArrayTime t = getMidTime( timeInterval_ ) ;
        if ( t > getStartTime( (*rows)[irow]->getTimeInterval() )
             && t <= getStartTime( (*rows)[irow+1]->getTimeInterval() ) ) {
          scindex = irow ;
          break ;
        }
      }
      if ( scindex == -1 )
        scindex = nrow-1 ;
    }
    //logsink_->postLocally( LogMessage("irow = "+String::toString(scindex),LogOrigin(className_,funcName,WHERE)) ) ;
    row = (*rows)[scindex] ;
  }
  return row ;
}

double ASDMReader::limitedAngle( double angle )
{
  if ( angle > C::pi )
    while ( angle > C::pi ) angle -= C::_2pi ;
  else if ( angle < -C::pi )
    while ( angle < -C::pi ) angle += C::_2pi ;
  return angle ;
}

vector< vector<double> > ASDMReader::pointingDir( PointingRow *row ) 
{
  //String funcName = "pointingDir" ;
  vector< vector<Angle> > aTar = row->getTarget() ;
  vector< vector<Angle> > aOff = row->getOffset() ;
  vector< vector<Angle> > aDir = row->getPointingDirection() ;
  vector< vector<Angle> > aEnc = row->getEncoder() ;
  unsigned int n = aTar.size() ;
  vector< vector<double> > dir( n ) ;
  double factor = 1.0 / cos( aTar[0][1].get() ) ;
  for ( unsigned int i = 0 ; i < n ; i++ ) {
    dir[i].resize( 2 ) ;
    /**
     * This is approximate way to add offset taking tracking error 
     * into account
     * 
     * az = dir[0][0] = target[0][0] + offset[0][0] / cos(el) 
     *                 + encorder[0][0] - direction[0][0]
     * el = dir[0][1] = target[0][1] + offset[0][1]
     *                 + encorder[0][1] - direction[0][1]
     **/
    dir[i][0] = aTar[i][0].get() + factor * aOff[i][0].get()
               + aEnc[i][0].get() - aDir[i][0].get() ;
    dir[i][1] = aTar[i][1].get() + aOff[i][1].get()
               + aEnc[i][1].get() - aDir[i][1].get() ;
    //logsink_->postLocally( LogMessage("tracking offset: ["+String::toString(aEnc[i][0].get()-aDir[i][0].get())+","+String::toString(aEnc[i][0]-aDir[i][0].get())+"]",LogOrigin(className_,funcName,WHERE)) ) ;
  }
  return dir ;
}

void ASDMReader::initConvert()
{
  toj2000_ = MDirection::Convert(MDirection::AZELGEO, 
                                 MDirection::Ref(MDirection::J2000, mf_));
}

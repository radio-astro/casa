//
// C++ Interface: MSFiller
//
// Description:
//
// This class is specific filler for MS format
//
// Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <iostream>
#include <map>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>

#include <casa/Containers/RecordField.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Path.h>

#include <measures/Measures/Stokes.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MCFrequency.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>

#include <ms/MeasurementSets/MSAntennaIndex.h>

#include <atnf/PKSIO/SrcType.h>

#include "MSFiller.h"
#include "STHeader.h" 

#include <ctime>
#include <sys/time.h>

using namespace casa ;
using namespace std ;

namespace asap {
double MSFiller::gettimeofday_sec()
{
  struct timeval tv ;
  gettimeofday( &tv, NULL ) ;
  return tv.tv_sec + (double)tv.tv_usec*1.0e-6 ;
}

MSFiller::MSFiller( casa::CountedPtr<Scantable> stable )
  : table_( stable ),
    tablename_( "" ),
    antenna_( -1 ),
    antennaStr_(""),
    getPt_( False ),
    isFloatData_( False ),
    isData_( False ),
    isDoppler_( False ),
    isFlagCmd_( False ),
    isFreqOffset_( False ),
    isHistory_( False ),
    isProcessor_( False ),
    isSysCal_( False ),
    isWeather_( False ),
    colTsys_( "TSYS_SPECTRUM" ),
    colTcal_( "TCAL_SPECTRUM" )
{
  os_ = LogIO() ;
  os_.origin( LogOrigin( "MSFiller", "MSFiller()", WHERE ) ) ;
}

MSFiller::~MSFiller()
{
  os_.origin( LogOrigin( "MSFiller", "~MSFiller()", WHERE ) ) ;
}

bool MSFiller::open( const std::string &filename, const casa::Record &rec )
{
  os_.origin( LogOrigin( "MSFiller", "open()", WHERE ) ) ;
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::open() startsec=" << startSec << LogIO::POST ;
  //os_ << "   filename = " << filename << endl ;

  // parsing MS options
  if ( rec.isDefined( "ms" ) ) {
    Record msrec = rec.asRecord( "ms" ) ;
    if ( msrec.isDefined( "getpt" ) ) {
      getPt_ = msrec.asBool( "getpt" ) ;
    }
    if ( msrec.isDefined( "antenna" ) ) {
      if ( msrec.type( msrec.fieldNumber( "antenna" ) ) == TpInt ) {
        antenna_ = msrec.asInt( "antenna" ) ;
      }
      else {
        //antenna_ = atoi( msrec.asString( "antenna" ).c_str() ) ;
        antennaStr_ = msrec.asString( "antenna" ) ;
      }
    }
    else {
      antenna_ = 0 ;
    }
  }

  MeasurementSet *tmpMS = new MeasurementSet( filename, Table::Old ) ;
  //mstable_ = (*tmpMS)( tmpMS->col("ANTENNA1") == antenna_ 
  //                     && tmpMS->col("ANTENNA1") == tmpMS->col("ANTENNA2") ) ;
  tablename_ = tmpMS->tableName() ;
  if ( antenna_ == -1 && antennaStr_.size() > 0 ) {
    MSAntennaIndex msAntIdx( tmpMS->antenna() ) ;
    Vector<Int> id = msAntIdx.matchAntennaName( antennaStr_ ) ;
    if ( id.size() > 0 )
      antenna_ = id[0] ;
  }

  os_ << "Parsing MS options" << endl ;
  os_ << "   getPt = " << getPt_ << endl ;
  os_ << "   antenna = " << antenna_ << endl ;
  os_ << "   antennaStr = " << antennaStr_ << LogIO::POST ;

  mstable_ = MeasurementSet( (*tmpMS)( tmpMS->col("ANTENNA1") == antenna_ 
                                       && tmpMS->col("ANTENNA1") == tmpMS->col("ANTENNA2") ) ) ;
//   stringstream ss ;
//   ss << "SELECT FROM $1 WHERE ANTENNA1 == ANTENNA2 && ANTENNA1 == " << antenna_ ;
//   String taql( ss.str() ) ;
//   mstable_ = MeasurementSet( tableCommand( taql, *tmpMS ) ) ;
  delete tmpMS ;

  // check which data column exists
  isFloatData_ = mstable_.tableDesc().isColumn( "FLOAT_DATA" ) ;
  isData_ = mstable_.tableDesc().isColumn( "DATA" ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::open() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return true ;
}

void MSFiller::fill()
{
  os_.origin( LogOrigin( "MSFiller", "fill()", WHERE ) ) ;
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::fill() startSec=" << startSec << LogIO::POST ;

//   double time0 = gettimeofday_sec() ;
//   os_ << "start init fill: " << time0 << LogIO::POST ;

  // Initialize header
  STHeader sdh ;  
  sdh.nchan = 0 ;
  sdh.npol = 0 ;
  sdh.nif = 0 ;
  sdh.nbeam = 0 ;
  sdh.observer = "" ;
  sdh.project = "" ;
  sdh.obstype = "" ;
  sdh.antennaname = "" ;
  sdh.antennaposition.resize( 0 ) ;
  sdh.equinox = 0.0 ;
  sdh.freqref = "" ;
  sdh.reffreq = -1.0 ;
  sdh.bandwidth = 0.0 ;
  sdh.utc = 0.0 ;
  sdh.fluxunit = "" ;
  sdh.epoch = "" ;
  sdh.poltype = "" ;
 
  // check if optional table exists
  //const TableRecord msrec = tablesel_.keywordSet() ;
  const TableRecord msrec = mstable_.keywordSet() ;
  isDoppler_ = msrec.isDefined( "DOPPLER" ) ;
  if ( isDoppler_ )
    if ( mstable_.doppler().nrow() == 0 ) 
      isDoppler_ = False ;
  isFlagCmd_ = msrec.isDefined( "FLAG_CMD" ) ;
  if ( isFlagCmd_ )
    if ( mstable_.flagCmd().nrow() == 0 ) 
      isFlagCmd_ = False ;
  isFreqOffset_ = msrec.isDefined( "FREQ_OFFSET" ) ;
  if ( isFreqOffset_ )
    if ( mstable_.freqOffset().nrow() == 0 ) 
      isFreqOffset_ = False ;
  isHistory_ = msrec.isDefined( "HISTORY" ) ;
  if ( isHistory_ )
    if ( mstable_.history().nrow() == 0 ) 
      isHistory_ = False ;
  isProcessor_ = msrec.isDefined( "PROCESSOR" ) ;
  if ( isProcessor_ )
    if ( mstable_.processor().nrow() == 0 ) 
      isProcessor_ = False ;
  isSysCal_ = msrec.isDefined( "SYSCAL" ) ;
  if ( isSysCal_ )
    if ( mstable_.sysCal().nrow() == 0 ) 
      isSysCal_ = False ;
  isWeather_ = msrec.isDefined( "WEATHER" ) ;
  if ( isWeather_ )
    if ( mstable_.weather().nrow() == 0 ) 
      isWeather_ = False ;

  // Access to MS subtables
  MSField fieldtab = mstable_.field() ;
  MSPolarization poltab = mstable_.polarization() ;
  MSDataDescription ddtab = mstable_.dataDescription() ;
  MSObservation obstab = mstable_.observation() ;
  MSSource srctab = mstable_.source() ;
  MSSpectralWindow spwtab = mstable_.spectralWindow() ;
  MSSysCal caltab = mstable_.sysCal() ; 
  if ( caltab.nrow() == 0 ) 
    isSysCal_ = False ;
  else {
    if ( !caltab.tableDesc().isColumn( colTcal_ ) ) 
      colTcal_ = "TCAL" ;
    if ( !caltab.tableDesc().isColumn( colTsys_ ) ) 
      colTsys_ = "TSYS" ;
  }
//   colTcal_ = "TCAL" ;
//   colTsys_ = "TSYS" ;
  MSPointing pointtab = mstable_.pointing() ;
  if ( mstable_.weather().nrow() == 0 ) 
    isWeather_ = False ;
  MSState stattab = mstable_.state() ;
  MSAntenna anttab = mstable_.antenna() ;

  // TEST
  // memory allocation by boost::object_pool
  boost::object_pool<ROTableColumn> *tpoolr = new boost::object_pool<ROTableColumn> ;
  //

  // SUBTABLES: FREQUENCIES
  table_->frequencies().setFrame( "LSRK" ) ;
  table_->frequencies().setFrame( "LSRK", True ) ;

  // SUBTABLES: WEATHER
  fillWeather() ;

  // SUBTABLES: FOCUS
  fillFocus() ;

  // SUBTABLES: TCAL
  fillTcal( tpoolr ) ;

  // SUBTABLES: FIT
  //fillFit() ;

  // SUBTABLES: HISTORY
  //fillHistory() ;

  // shared pointers
  ROTableColumn *tcolr ;

  // MAIN 
  // Iterate over several ids
  map<Int, uInt> ifmap ; // (IFNO, FREQ_ID) pair
  ROArrayQuantColumn<Double> *sharedQDArrCol = new ROArrayQuantColumn<Double>( anttab, "POSITION" ) ;
  Vector< Quantum<Double> > antpos = (*sharedQDArrCol)( antenna_ ) ;
  delete sharedQDArrCol ;
  MPosition mp( MVPosition( antpos ), MPosition::ITRF ) ;
  if ( getPt_ ) {
    //pointtab = pointtab( pointtab.col("ANTENNA_ID")==antenna_ ).sort("TIME") ;
    pointtab = MSPointing( pointtab( pointtab.col("ANTENNA_ID")==antenna_ ).sort("TIME") ) ;
  }
  tcolr = tpoolr->construct( anttab, "STATION" ) ;
  String stationName = tcolr->asString( antenna_ ) ;
  tpoolr->destroy( tcolr ) ;
  tcolr = tpoolr->construct( anttab, "NAME" ) ;
  String antennaName = tcolr->asString( antenna_ ) ;
  tpoolr->destroy( tcolr ) ;
  sdh.antennaposition.resize( 3 ) ;
  for ( int i = 0 ; i < 3 ; i++ )
    sdh.antennaposition[i] = antpos[i].getValue( "m" ) ;
  String telescopeName = "" ;

//   double time1 = gettimeofday_sec() ;
//   os_ << "end fill init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;

  // row based 
  Table &stab = table_->table() ; 
  TableRow row( stab ) ;
  TableRecord &trec = row.record() ;
  RecordFieldPtr< Array<Float> > spRF( trec, "SPECTRA" ) ;
  RecordFieldPtr< Array<uChar> > ucarrRF( trec, "FLAGTRA" ) ;
  RecordFieldPtr<Double> timeRF( trec, "TIME" ) ;
  RecordFieldPtr< Array<Float> > tsysRF( trec, "TSYS" ) ;
  RecordFieldPtr<Double> intervalRF( trec, "INTERVAL" ) ;
  RecordFieldPtr< Array<Double> > dirRF( trec, "DIRECTION" ) ;
  RecordFieldPtr<Float> azRF( trec, "AZIMUTH" ) ;
  RecordFieldPtr<Float> elRF( trec, "ELEVATION" ) ;
  RecordFieldPtr< Array<Double> > scrRF( trec, "SCANRATE" ) ;
  RecordFieldPtr<uInt> cycleRF( trec, "CYCLENO" ) ;
  RecordFieldPtr<uInt> flrRF( trec, "FLAGROW" ) ;
  RecordFieldPtr<uInt> tcalidRF( trec, "TCAL_ID" ) ;
  RecordFieldPtr<uInt> widRF( trec, "WEATHER_ID" ) ;
  RecordFieldPtr<uInt> polnoRF( trec, "POLNO" ) ;


  // REFBEAMNO
  RecordFieldPtr<Int> intRF( trec, "REFBEAMNO" ) ;
  *intRF = 0 ;

  // FIT_ID
  intRF.attachToRecord( trec, "FIT_ID" ) ;
  *intRF = -1 ;

  // OPACITY
  RecordFieldPtr<Float> floatRF( trec, "OPACITY" ) ;
  *floatRF = 0.0 ;

  //
  // ITERATION: OBSERVATION_ID
  //
  TableIterator iter0( mstable_, "OBSERVATION_ID" ) ;
  while( !iter0.pastEnd() ) {
//     time0 = gettimeofday_sec() ;
//     os_ << "start 0th iteration: " << time0 << LogIO::POST ;
    Table t0 = iter0.table() ;
    tcolr = tpoolr->construct( t0, "OBSERVATION_ID" ) ;
    Int obsId = tcolr->asInt( 0 ) ;
    tpoolr->destroy( tcolr ) ;
    if ( sdh.observer == "" ) {
      tcolr = tpoolr->construct( obstab, "OBSERVER" ) ;
      sdh.observer = tcolr->asString( obsId ) ;
      tpoolr->destroy( tcolr ) ;
    }
    if ( sdh.project == "" ) {
      tcolr = tpoolr->construct( obstab, "PROJECT" ) ;
      sdh.observer = tcolr->asString( obsId ) ;
      tpoolr->destroy( tcolr ) ;
    }
    ROArrayMeasColumn<MEpoch> *tmpMeasCol = new ROArrayMeasColumn<MEpoch>( obstab, "TIME_RANGE" ) ;
    MEpoch me = (*tmpMeasCol)( obsId )( IPosition(1,0) ) ;
    delete tmpMeasCol ;
    if ( sdh.utc == 0.0 ) {
      sdh.utc = me.get( "d" ).getValue() ;
    }
    if ( telescopeName == "" ) {
      tcolr = tpoolr->construct( obstab, "TELESCOPE_NAME" ) ;
      telescopeName = tcolr->asString( obsId ) ;
      tpoolr->destroy( tcolr ) ;
    }
    Int nbeam = 0 ;
//     time1 = gettimeofday_sec() ;
//     os_ << "end 0th iteration init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;
    //
    // ITERATION: FEED1
    //
    TableIterator iter1( t0, "FEED1" ) ;
    while( !iter1.pastEnd() ) {
//       time0 = gettimeofday_sec() ;
//       os_ << "start 1st iteration: " << time0 << LogIO::POST ;
      Table t1 = iter1.table() ;
      // assume FEED1 == FEED2
      tcolr = tpoolr->construct( t1, "FEED1" ) ;
      Int feedId = tcolr->asInt( 0 ) ;
      tpoolr->destroy( tcolr ) ;
      nbeam++ ;

      // BEAMNO
      RecordFieldPtr<uInt> uintRF( trec, "BEAMNO" ) ;
      *uintRF = feedId ;

      // FOCUS_ID
      uintRF.attachToRecord( trec, "FOCUS_ID" ) ;
      *uintRF = 0 ;

//       time1 = gettimeofday_sec() ;
//       os_ << "end 1st iteration init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;
      // 
      // ITERATION: FIELD_ID 
      //
      TableIterator iter2( t1, "FIELD_ID" ) ;
      while( !iter2.pastEnd() ) {
//         time0 = gettimeofday_sec() ;
//         os_ << "start 2nd iteration: " << time0 << LogIO::POST ;
        Table t2 = iter2.table() ;
        tcolr = tpoolr->construct( t2, "FIELD_ID" ) ;
        Int fieldId = tcolr->asInt( 0 ) ;
        tpoolr->destroy( tcolr ) ;
        tcolr = tpoolr->construct( fieldtab, "SOURCE_ID" ) ;
        Int srcId = tcolr->asInt( fieldId ) ;
        tpoolr->destroy( tcolr ) ;
        tcolr = tpoolr->construct( fieldtab, "NAME" ) ;
        String fieldName = tcolr->asString( fieldId ) + "__" + String::toString(fieldId) ;
        tpoolr->destroy( tcolr ) ;
        ROArrayMeasColumn<MDirection> *delayDirCol = new ROArrayMeasColumn<MDirection>( fieldtab, "DELAY_DIR" ) ;
        Vector<MDirection> delayDir = (*delayDirCol)( fieldId ) ;
        delete delayDirCol ;
        Vector<Double> defaultScanrate( 2, 0.0 ) ;
        Vector<Double> defaultDir = delayDir[0].getAngle( "rad" ).getValue() ;
        if ( delayDir.nelements() > 1 ) 
          defaultScanrate = delayDir[1].getAngle( "rad" ).getValue() ;
          

        // FIELDNAME
        RecordFieldPtr<String> strRF( trec, "FIELDNAME" ) ;
        *strRF = fieldName ;


//         time1 = gettimeofday_sec() ;
//         os_ << "end 2nd iteration init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;
        // 
        // ITERATION: DATA_DESC_ID
        //
        TableIterator iter3( t2, "DATA_DESC_ID" ) ;
        while( !iter3.pastEnd() ) {
//           time0 = gettimeofday_sec() ;
//           os_ << "start 3rd iteration: " << time0 << LogIO::POST ;
          Table t3 = iter3.table() ;
          tcolr = tpoolr->construct( t3, "DATA_DESC_ID" ) ;
          Int ddId = tcolr->asInt( 0 ) ;
          tpoolr->destroy( tcolr ) ;
          tcolr = tpoolr->construct( ddtab, "POLARIZATION_ID" ) ;
          Int polId = tcolr->asInt( ddId ) ;
          tpoolr->destroy( tcolr ) ;
          tcolr = tpoolr->construct( ddtab, "SPECTRAL_WINDOW_ID" ) ;
          Int spwId = tcolr->asInt( ddId ) ;
          tpoolr->destroy( tcolr ) ;

          // IFNO
          uintRF.attachToRecord( trec, "IFNO" ) ;
          *uintRF = (uInt)spwId ;

          // polarization information
          tcolr = tpoolr->construct( poltab, "NUM_CORR" ) ;
          Int npol = tcolr->asInt( polId ) ;
          tpoolr->destroy( tcolr ) ;
          ROArrayColumn<Int> *roArrICol = new ROArrayColumn<Int>( poltab, "CORR_TYPE" ) ;
          Vector<Int> corrtype = (*roArrICol)( polId ) ;
          delete roArrICol ;
//           os_ << "npol = " << npol << LogIO::POST ;
//           os_ << "corrtype = " << corrtype << LogIO::POST ;
          // source information
//           os_ << "srcId = " << srcId << ", spwId = " << spwId << LogIO::POST ;
          MSSource srctabSel = srctab( srctab.col("SOURCE_ID") == srcId && srctab.col("SPECTRAL_WINDOW_ID") == spwId ) ;
          if ( srctabSel.nrow() == 0 ) {
            srctabSel = srctab( srctab.col("SOURCE_ID") == srcId && srctab.col("SPECTRAL_WINDOW_ID") == -1 ) ;
          }
          String srcName( "" ) ;
          Vector<Double> srcPM( 2, 0.0 ) ;
          Vector<Double> srcDir( 2, 0.0 ) ;
          MDirection md ;
          Int numLines = 0 ;
          ROArrayColumn<Double> *roArrDCol = 0 ;
          if ( srctabSel.nrow() > 0 ) {
            // source name
            tcolr = tpoolr->construct( srctabSel, "NAME" ) ;
            srcName = tcolr->asString( 0 ) ;
            tpoolr->destroy( tcolr ) ;

            // source proper motion
            roArrDCol = new ROArrayColumn<Double>( srctabSel, "PROPER_MOTION" ) ;
            srcPM = (*roArrDCol)( 0 ) ;
            delete roArrDCol ;
            
            // source direction
            roArrDCol = new ROArrayColumn<Double>( srctabSel, "DIRECTION" ) ;
            srcDir = (*roArrDCol)( 0 ) ;
            delete roArrDCol ;

            // source direction as MDirection object
            ROScalarMeasColumn<MDirection> *tmpMeasCol = new ROScalarMeasColumn<MDirection>( srctabSel, "DIRECTION" ) ;
            md = (*tmpMeasCol)( 0 ) ;
            delete tmpMeasCol ;

            // number of lines
            tcolr = tpoolr->construct( srctabSel, "NUM_LINES" ) ;
            numLines = tcolr->asInt( 0 ) ;
            tpoolr->destroy( tcolr ) ;

          }
          else {
            md = MDirection( Quantum<Double>(0.0,Unit("rad")), Quantum<Double>(0.0,Unit("rad")) ) ;
          }

          // SRCNAME
          strRF.attachToRecord( trec, "SRCNAME" ) ;
          *strRF = srcName ;

//           os_ << "srcName = " << srcName << LogIO::POST ;

          // SRCPROPERMOTION
          RecordFieldPtr< Array<Double> > darrRF( trec, "SRCPROPERMOTION" ) ;
          *darrRF = srcPM ;

          //os_ << "srcPM = " << srcPM << LogIO::POST ;

          // SRCDIRECTION
          darrRF.attachToRecord( trec, "SRCDIRECTION" ) ;
          *darrRF = srcDir ;

          //os_ << "srcDir = " << srcDir << LogIO::POST ;

          // for MOLECULES subtable
//           os_ << "numLines = " << numLines << LogIO::POST ;

          Vector<Double> restFreqs( numLines, 0.0 ) ;
          Vector<String> transitionName( numLines, "" ) ;
          Vector<Double> sysVels ;
          Double sysVel = 0.0 ;
          if ( numLines != 0 ) {
            if ( srctabSel.tableDesc().isColumn( "REST_FREQUENCY" ) ) {
              sharedQDArrCol = new ROArrayQuantColumn<Double>( srctabSel, "REST_FREQUENCY" ) ;
              Array< Quantum<Double> > qRestFreqs = (*sharedQDArrCol)( 0 ) ;
              delete sharedQDArrCol ;
              for ( int i = 0 ; i < numLines ; i++ ) {
                restFreqs[i] = qRestFreqs( IPosition( 1, i ) ).getValue( "Hz" ) ;
              }
            }
//             os_ << "restFreqs = " << restFreqs << LogIO::POST ;
            if ( srctabSel.tableDesc().isColumn( "TRANSITION" ) ) {
              ROArrayColumn<String> transitionCol( srctabSel, "TRANSITION" ) ;
              if ( transitionCol.isDefined( 0 ) )
                transitionName = transitionCol( 0 ) ;
              //os_ << "transitionNameCol.nrow() = " << transitionCol.nrow() << LogIO::POST ;
            }
            if ( srctabSel.tableDesc().isColumn( "SYSVEL" ) ) {
              roArrDCol = new ROArrayColumn<Double>( srctabSel, "SYSVEL" ) ;
              sysVels = (*roArrDCol)( 0 ) ;
              delete roArrDCol ;
            }
            if ( !sysVels.empty() ) {
              //os_ << "sysVels.shape() = " << sysVels.shape() << LogIO::POST ;
              // NB: assume all SYSVEL values are the same
              sysVel = sysVels( IPosition(1,0) ) ;
            }
          }

          // SRCVELOCITY
          RecordFieldPtr<Double> doubleRF( trec, "SRCVELOCITY" ) ;
          *doubleRF = sysVel ;

//           os_ << "sysVel = " << sysVel << LogIO::POST ;

          uInt molId = table_->molecules().addEntry( restFreqs, transitionName, transitionName ) ;

          // MOLECULE_ID
          uintRF.attachToRecord( trec, "MOLECULE_ID" ) ;
          *uintRF = molId ;

          // spectral setup
          MeasFrame mf( me, mp, md ) ;
          tcolr = tpoolr->construct( spwtab, "MEAS_FREQ_REF" ) ;
          MFrequency::Types freqRef = MFrequency::castType((uInt)(tcolr->asInt(spwId))) ;
          tpoolr->destroy( tcolr ) ;
          tcolr = tpoolr->construct( spwtab, "NUM_CHAN" ) ;
          Int nchan = tcolr->asInt( spwId ) ;
          Bool iswvr = False ;
          if ( nchan == 4 ) iswvr = True ;
          tpoolr->destroy( tcolr ) ;
          Bool even = False ;
          if ( (nchan/2)*2 == nchan ) even = True ;
          sdh.nchan = max( sdh.nchan, nchan ) ;
          ROScalarQuantColumn<Double> *tmpQuantCol = new ROScalarQuantColumn<Double>( spwtab, "TOTAL_BANDWIDTH" ) ;
          Double totbw = (*tmpQuantCol)( spwId ).getValue( "Hz" ) ;
          delete tmpQuantCol ;
          sdh.bandwidth = max( sdh.bandwidth, totbw ) ;
          if ( sdh.freqref == "" ) 
            //sdh.freqref = MFrequency::showType( freqRef ) ;
            sdh.freqref = "LSRK" ;
          if ( sdh.reffreq == -1.0 ) {
            tmpQuantCol = new ROScalarQuantColumn<Double>( spwtab, "REF_FREQUENCY" ) ;
            Quantum<Double> qreffreq = (*tmpQuantCol)( spwId ) ;
            delete tmpQuantCol ;
            if ( freqRef == MFrequency::LSRK ) {
              sdh.reffreq = qreffreq.getValue("Hz") ;
            }
            else {
              MFrequency::Convert tolsr( freqRef, MFrequency::Ref( MFrequency::LSRK, mf ) ) ;
              sdh.reffreq = tolsr( qreffreq ).get("Hz").getValue() ; 
            }
          }
          Int refchan = nchan / 2 ;
          IPosition refip( 1, refchan ) ;
          Double refpix = 0.5*(nchan-1) ;
          Double refval = 0.0 ;
          sharedQDArrCol = new ROArrayQuantColumn<Double>( spwtab, "CHAN_WIDTH" ) ;
          Double increment = (*sharedQDArrCol)( spwId )( refip ).getValue( "Hz" ) ;
          delete sharedQDArrCol ;
//           os_ << "nchan = " << nchan << " refchan = " << refchan << "(even=" << even << ") refpix = " << refpix << LogIO::POST ;
          sharedQDArrCol = new ROArrayQuantColumn<Double>( spwtab, "CHAN_FREQ" ) ;
          Vector< Quantum<Double> > chanFreqs = (*sharedQDArrCol)( spwId ) ;
          delete sharedQDArrCol ;
          if ( freqRef == MFrequency::LSRK ) {
            if ( even ) {
              IPosition refip0( 1, refchan-1 ) ;
              Double refval0 = chanFreqs(refip0).getValue("Hz") ;
              Double refval1 = chanFreqs(refip).getValue("Hz") ;
              refval = 0.5 * ( refval0 + refval1 ) ;
            }
            else {
              refval = chanFreqs(refip).getValue("Hz") ;
            }
          }
          else {
            MFrequency::Convert tolsr( freqRef, MFrequency::Ref( MFrequency::LSRK, mf ) ) ;
            if ( even ) {
              IPosition refip0( 1, refchan-1 ) ;
              Double refval0 = chanFreqs(refip0).getValue("Hz") ;
              Double refval1 = chanFreqs(refip).getValue("Hz") ;
              refval = 0.5 * ( refval0 + refval1 ) ;
              refval = tolsr( refval ).get("Hz").getValue() ;
            }
            else {
              refval = tolsr( chanFreqs(refip) ).get("Hz").getValue() ;
            }
          }
          uInt freqId = table_->frequencies().addEntry( refpix, refval, increment ) ;
          if ( ifmap.find( spwId ) == ifmap.end() ) {
            ifmap.insert( pair<Int, uInt>(spwId,freqId) ) ;
            //os_ << "added to ifmap: (" << spwId << "," << freqId << ")" << LogIO::POST ;
          }

          // FREQ_ID
          uintRF.attachToRecord( trec, "FREQ_ID" ) ;
          *uintRF = freqId ;

          // for TSYS and TCAL
          Vector<MEpoch> scTime ;
          Vector<Double> scInterval ;
          ROArrayColumn<Float> scTsysCol ;
          MSSysCal caltabsel ;
          if ( isSysCal_ ) {
            caltabsel = caltab( caltab.col("ANTENNA_ID") == antenna_ && caltab.col("FEED_ID") == feedId && caltab.col("SPECTRAL_WINDOW_ID") == spwId ).sort("TIME") ;
            ROScalarMeasColumn<MEpoch> scTimeCol( caltabsel, "TIME" ) ;
            scTime.resize( caltabsel.nrow() ) ;
            for ( uInt irow = 0 ; irow < caltabsel.nrow() ; irow++ ) 
              scTime[irow] = scTimeCol( irow ) ;
            ROScalarColumn<Double> *scIntervalCol = new ROScalarColumn<Double>( caltabsel, "INTERVAL" ) ;
            scInterval = scIntervalCol->getColumn() ;
            delete scIntervalCol ;
            scTsysCol.attach( caltabsel, colTsys_ ) ;
          }

          sdh.npol = max( sdh.npol, npol ) ;
          if ( !iswvr && sdh.poltype == "" ) sdh.poltype = getPolType( corrtype[0] ) ;

//           time1 = gettimeofday_sec() ;
//           os_ << "end 3rd iteration init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;
          //
          // ITERATION: SCAN_NUMBER
          //
          TableIterator iter4( t3, "SCAN_NUMBER" ) ;
          while( !iter4.pastEnd() ) {
//             time0 = gettimeofday_sec() ;
//             os_ << "start 4th iteration: " << time0 << LogIO::POST ;
            Table t4 = iter4.table() ;
            tcolr = tpoolr->construct( t4, "SCAN_NUMBER" ) ;
            Int scanNum = tcolr->asInt( 0 ) ;
            tpoolr->destroy( tcolr ) ;

            // SCANNO
            uintRF.attachToRecord( trec, "SCANNO" ) ;
            *uintRF = scanNum - 1 ;

//             time1 = gettimeofday_sec() ;
//             os_ << "end 4th iteration init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;
            // 
            // ITERATION: STATE_ID
            //
            TableIterator iter5( t4, "STATE_ID" ) ; 
            while( !iter5.pastEnd() ) {
//               time0 = gettimeofday_sec() ;
//               os_ << "start 5th iteration: " << time0 << LogIO::POST ;
              Table t5 = iter5.table() ;
              tcolr = tpoolr->construct( t5, "STATE_ID" ) ;
              Int stateId = tcolr->asInt( 0 ) ;
              tpoolr->destroy( tcolr ) ;
              tcolr = tpoolr->construct( stattab, "OBS_MODE" ) ;
              String obstype = tcolr->asString( stateId ) ;
              tpoolr->destroy( tcolr ) ;
              if ( sdh.obstype == "" ) sdh.obstype = obstype ;

              Int nrow = t5.nrow() ;
//               time1 = gettimeofday_sec() ;
//               os_ << "end 5th iteration init: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;

              uInt cycle = 0 ;

              Cube<Float> spArr ;
              Cube<Bool> flArr ;
              if ( isFloatData_ ) {
                ROArrayColumn<Bool> mFlagCol( t5, "FLAG" ) ;
                ROArrayColumn<Float> mFloatDataCol( t5, "FLOAT_DATA" ) ;
                spArr = mFloatDataCol.getColumn() ;
                flArr = mFlagCol.getColumn() ;
                if ( sdh.fluxunit == "" ) {
                  const TableRecord &dataColKeys = mFloatDataCol.keywordSet() ;
                  if ( dataColKeys.isDefined( "UNIT" ) )
                    sdh.fluxunit = dataColKeys.asString( "UNIT" ) ;
                } 
              }
              else if ( isData_ ) {
                spArr.resize( npol, nchan, nrow ) ;
                flArr.resize( npol, nchan, nrow ) ;
                ROArrayColumn<Bool> mFlagCol( t5, "FLAG" ) ;
                ROArrayColumn<Complex> mDataCol( t5, "DATA" ) ;
                for ( Int irow = 0 ; irow < nrow ; irow++ ) {
                  Bool crossOK = False ;
                  Matrix<Complex> sp = mDataCol( irow ) ;
                  Matrix<Bool> fl = mFlagCol( irow ) ;
                  Matrix<Float> spxy = spArr.xyPlane( irow ) ;
                  Matrix<Bool> flxy = flArr.xyPlane( irow ) ;
                  for ( Int ipol = 0 ; ipol < npol ; ipol++ ) {
                    if ( corrtype[ipol] == Stokes::XY || corrtype[ipol] == Stokes::YX 
                         || corrtype[ipol] == Stokes::RL || corrtype[ipol] == Stokes::LR ) {
                      if ( !crossOK ) {
                        spxy.row( ipol ) = real( sp.row( ipol ) ) ;
                        flxy.row( ipol ) = fl.row( ipol ) ;
                        if ( corrtype[ipol] == Stokes::XY || corrtype[ipol] == Stokes::RL ) {
                          spxy.row( ipol+1 ) = imag( sp.row( ipol ) ) ;
                          flxy.row( ipol+1 ) = fl.row( ipol ) ;
                        }                        
                        else {
                          spxy.row( ipol+1 ) = imag( conj( sp.row( ipol ) ) ) ;
                          flxy.row( ipol+1 ) = fl.row( ipol ) ;
                        }
                        crossOK = True ;
                      }
                    }
                    else {
                      spxy.row( ipol ) = real( sp.row( ipol ) ) ;
                      flxy.row( ipol ) = fl.row( ipol ) ;
                    }
                  }
                }
                if ( sdh.fluxunit == "" ) {
                  const TableRecord &dataColKeys = mDataCol.keywordSet() ;
                  if ( dataColKeys.isDefined( "UNIT" ) )
                    sdh.fluxunit = dataColKeys.asString( "UNIT" ) ;
                } 
              }
              ROScalarMeasColumn<MEpoch> *mTimeCol = new ROScalarMeasColumn<MEpoch>( t5, "TIME" ) ;
              Block<MEpoch> mTimeB( nrow ) ;
              for ( Int irow = 0 ; irow < nrow ; irow++ ) 
                mTimeB[irow] = (*mTimeCol)( irow ) ;
              ROTableColumn *mIntervalCol = tpoolr->construct( t5, "INTERVAL" ) ;
              ROTableColumn *mFlagRowCol = tpoolr->construct( t5, "FLAG_ROW" ) ;
              Block<Int> sysCalIdx( nrow, -1 ) ;
              if ( isSysCal_ ) {
                getSysCalTime( scTime, scInterval, mTimeB, sysCalIdx ) ;
              }
              delete mTimeCol ;
              Matrix<Float> defaulttsys( npol, 1, 1.0 ) ;
              Int srcType = getSrcType( stateId, tpoolr ) ;
              uInt diridx = 0 ;
              MDirection::Types dirType ;
              uInt wid = 0 ;
              Int pidx = 0 ;
              Bool crossOK = False ;
              Block<uInt> polnos( npol, 99 ) ;
              for ( Int ipol = 0 ; ipol < npol ; ipol++ ) {
                Block<uInt> p = getPolNo( corrtype[ipol] ) ;
                if ( p.size() > 1 ) {
                  if ( crossOK ) continue ;
                  else {
                    polnos[pidx] = p[0] ;
                    pidx++ ;
                    polnos[pidx] = p[1] ;
                    pidx++ ;
                    crossOK = True ;
                  }
                }
                else {
                  polnos[pidx] = p[0] ;
                  pidx++ ;
                }
              }
              
              // SRCTYPE
              intRF.attachToRecord( trec, "SRCTYPE" ) ;
              *intRF = srcType ;

              for ( Int irow = 0 ; irow < nrow ; irow++ ) {
                // CYCLENO
                *cycleRF = cycle ;

                // FLAGROW
                *flrRF = (uInt)mFlagRowCol->asBool( irow ) ;

                // SPECTRA, FLAG
                Matrix<Float> sp = spArr.xyPlane( irow ) ;
                Matrix<Bool> flb = flArr.xyPlane( irow ) ;
                Matrix<uChar> fl( flb.shape() ) ;
                convertArray( fl, flb ) ;

                // TIME
                *timeRF = mTimeB[irow].get("d").getValue() ;

                // INTERVAL
                *intervalRF = (Double)(mIntervalCol->asdouble( irow )) ;

                // TSYS
                Matrix<Float> tsys ;
                if ( sysCalIdx[irow] != -1 )
                  tsys = scTsysCol( irow ) ;
                else 
                  tsys = defaulttsys ;

                // TCAL_ID
                Block<uInt> tcalids( npol, 0 ) ;
                if ( sysCalIdx[irow] != -1 ) {
                  tcalids = getTcalId( feedId, spwId, scTime[sysCalIdx[irow]] ) ;
                }

                // WEATHER_ID
                if ( isWeather_ )
                  wid = getWeatherId( wid, mTimeB[irow].get("s").getValue() ) ;
                *widRF = wid ;
                  

                // DIRECTION, AZEL, SCANRATE
                if ( getPt_ ) {
                  Vector<Double> dir ;
                  Vector<Double> scanrate ;
                  String refString ;
                  diridx = getDirection( diridx, dir, scanrate, refString, pointtab, mTimeB[irow].get("s").getValue() ) ;
                  MDirection::getType( dirType, refString ) ;
                  mf.resetEpoch( mTimeB[irow] ) ;
                  mf.resetDirection( MDirection( MVDirection(dir), dirType ) ) ;
                  if ( refString == "J2000" ) {
                    *dirRF = dir ;
                    MDirection::Convert toazel( dirType, MDirection::Ref( MDirection::AZEL, mf ) ) ;
                    Vector<Double> azel = toazel( dir ).getAngle("rad").getValue() ;
                    *azRF = (Float)azel(0) ;
                    *elRF = (Float)azel(1) ;
                  }
                  else if ( refString(0,4) == "AZEL" ) {
                    *azRF = (Float)dir(0) ;
                    *elRF = (Float)dir(1) ;
                    MDirection::Convert toj2000( dirType, MDirection::Ref( MDirection::J2000, mf ) ) ;
                    Vector<Double> newdir = toj2000( dir ).getAngle("rad").getValue() ;
                    *dirRF = newdir ;
                  }
                  else {
                    MDirection::Convert toazel( dirType, MDirection::Ref( MDirection::AZEL, mf ) ) ;
                    Vector<Double> azel = toazel( dir ).getAngle("rad").getValue() ;
                    MDirection::Convert toj2000( dirType, MDirection::Ref( MDirection::J2000, mf ) ) ;
                    Vector<Double> newdir = toj2000( dir ).getAngle("rad").getValue() ;
                    *dirRF = newdir ;
                    *azRF = (Float)azel(0) ;
                    *elRF = (Float)azel(1) ;
                  }
                  if ( scanrate.size() != 0 ) {
                    *scrRF = scanrate ;
                  }
                  else {
                    *scrRF = defaultScanrate ;
                  }
                }
                else {
                  String ref = md.getRefString() ;
                  //Vector<Double> defaultDir = srcDir ;
                  MDirection::getType( dirType, "J2000" ) ;
                  if ( ref != "J2000" ) {
                    ROScalarMeasColumn<MEpoch> tmCol( pointtab, "TIME" ) ;
                    mf.resetEpoch( tmCol( 0 ) ) ;
                    MDirection::Convert toj2000( dirType, MDirection::Ref( MDirection::J2000, mf ) ) ;
                    defaultDir = toj2000( defaultDir ).getAngle("rad").getValue() ;
                  }
                  mf.resetEpoch( mTimeB[irow] ) ;
                  MDirection::Convert toazel( dirType, MDirection::Ref( MDirection::AZEL, mf ) ) ;
                  Vector<Double> azel = toazel( defaultDir ).getAngle("rad").getValue() ;
                  *azRF = (Float)azel(0) ;
                  *elRF = (Float)azel(1) ;
                  *dirRF = defaultDir ;
                  *scrRF = defaultScanrate ;
                }

                // Polarization dependent things
                for ( Int ipol = 0 ; ipol < npol ; ipol++ ) {
                  // POLNO
                  *polnoRF = polnos[ipol] ;

                  //*spRF = sp.row( ipol ) ;
                  //*ucarrRF = fl.row( ipol ) ;
                  //*tsysRF = tsys.row( ipol ) ;
                  spRF.define( sp.row( ipol ) ) ;
                  ucarrRF.define( fl.row( ipol ) ) ;
                  tsysRF.define( tsys.row( ipol ) ) ;
                  *tcalidRF = tcalids[ipol] ;

                  // Commit row
                  stab.addRow() ;
                  row.put( stab.nrow()-1 ) ;
                }

                cycle++ ;
              }
              tpoolr->destroy( mIntervalCol ) ;
              tpoolr->destroy( mFlagRowCol ) ;

//               time1 = gettimeofday_sec() ;
//               os_ << "end 5th iteration: " << time1 << " (" << time1-time0 << "sec)" << LogIO::POST ;

              iter5.next() ;
            }


            iter4.next() ;
          }


          iter3.next() ;
        }

              
        iter2.next() ;
      }


      iter1.next() ;
    }
    if ( sdh.nbeam < nbeam ) sdh.nbeam = nbeam ;

    iter0.next() ;
  }


  delete tpoolr ;


  // Table Keywords
  sdh.nif = ifmap.size() ;
  if ( ( telescopeName == "" ) || ( antennaName == telescopeName ) ) {
    sdh.antennaname = antennaName ;
  }
  else {
    sdh.antennaname = telescopeName + "//" + antennaName ;
  }
  if ( stationName != "" ) {
    sdh.antennaname += "@" + stationName ;
  }
  ROArrayColumn<Double> pdirCol( pointtab, "DIRECTION" ) ; 
  String dirref = pdirCol.keywordSet().asRecord("MEASINFO").asString("Ref") ;
  if ( dirref == "AZELGEO" || dirref == "AZEL" ) {
    dirref = "J2000" ;
  }
  sscanf( dirref.chars()+1, "%f", &sdh.equinox ) ;
  sdh.epoch = "UTC" ;
  if (sdh.freqref == "TOPO") {
    sdh.freqref = "TOPOCENT";
  } else if (sdh.freqref == "GEO") {
    sdh.freqref = "GEOCENTR";
  } else if (sdh.freqref == "BARY") {
    sdh.freqref = "BARYCENT";
  } else if (sdh.freqref == "GALACTO") {
    sdh.freqref = "GALACTOC";
  } else if (sdh.freqref == "LGROUP") {
    sdh.freqref = "LOCALGRP";
  } else if (sdh.freqref == "CMB") {
    sdh.freqref = "CMBDIPOL";
  } else if (sdh.freqref == "REST") {
    sdh.freqref = "SOURCE";
  }
  table_->setHeader( sdh ) ;

  // save path to POINTING table
  // 2011/3/2 TN
  // So far, path to original POINTING table is always stored 
  // since sd tasks and regressions don't support getpt control
  //if ( !getPt_ ) {
  Path datapath( tablename_ ) ;
  String pTabName = datapath.absoluteName() + "/POINTING" ;
  stab.rwKeywordSet().define( "POINTING", pTabName ) ;
  //}

  // for GBT
  if ( antennaName.contains( "GBT" ) ) {
    String goTabName = datapath.absoluteName() + "/GBT_GO" ;
    stab.rwKeywordSet().define( "GBT_GO", goTabName ) ;
  }

  // for MS created from ASDM
  //mstable_.keywordSet().print(cout) ;
  const TableRecord &msKeys = mstable_.keywordSet() ;
  uInt nfields = msKeys.nfields() ;
  for ( uInt ifield = 0 ; ifield < nfields ; ifield++ ) {
    String name = msKeys.name( ifield ) ;
    //os_ << "name = " << name << LogIO::POST ;
    if ( name.find( "ASDM" ) != String::npos ) {
      String asdmpath = msKeys.asTable( ifield ).tableName() ;
      os_ << "ASDM table: " << asdmpath << LogIO::POST ;
      stab.rwKeywordSet().define( name, asdmpath ) ;
    }
  }

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::fill() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::close()
{
  //tablesel_.closeSubTables() ;
  mstable_.closeSubTables() ;
  //tablesel_.unlock() ;
  mstable_.unlock() ;
}

Int MSFiller::getSrcType( Int stateId, boost::object_pool<ROTableColumn> *tpool ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getSrcType() startSec=" << startSec << LogIO::POST ;

  MSState statetab = mstable_.state() ;
  ROTableColumn *sharedCol ;
  sharedCol = tpool->construct( statetab, "OBS_MODE" ) ;
  String obsMode = sharedCol->asString( stateId ) ;
  tpool->destroy( sharedCol ) ;
  sharedCol = tpool->construct( statetab, "SIG" ) ;
  Bool sig = sharedCol->asBool( stateId ) ;
  tpool->destroy( sharedCol ) ;
  sharedCol = tpool->construct( statetab, "REF" ) ;
  Bool ref = sharedCol->asBool( stateId ) ;
  tpool->destroy( sharedCol ) ;
  sharedCol = tpool->construct( statetab, "CAL" ) ;
  Double cal = (Double)(sharedCol->asdouble( stateId )) ;
  tpool->destroy( sharedCol ) ;
  //os_ << "OBS_MODE = " << obsMode << LogIO::POST ;

  // determine separator
  String sep = "" ;
  String tmpStr = obsMode.substr( 0, obsMode.find_first_of( "," ) ) ;
  //os_ << "tmpStr = " << tmpStr << LogIO::POST ;
  //if ( obsMode.find( ":" ) != String::npos ) {
  if ( tmpStr.find( ":" ) != String::npos ) {
    sep = ":" ;
  }
  //else if ( obsMode.find( "." ) != String::npos ) {
  else if ( tmpStr.find( "." ) != String::npos ) {
    sep = "." ;
  }
  //else if ( obsMode.find( "_" ) != String::npos ) {
  else if ( tmpStr.find( "_" ) != String::npos ) {
    sep = "_" ;
  }
  //os_ << "separator = " << sep << LogIO::POST ;

  // determine SRCTYPE
  Int srcType = SrcType::NOTYPE ;
  if ( sep == ":" ) {
    // sep == ":"
    // 
    // GBT case 
    //
    // obsMode1=Nod
    //    NOD
    // obsMode1=OffOn
    //    obsMode2=PSWITCHON:  PSON
    //    obsMode2=PSWITCHOFF: PSOFF
    // obsMode1=??
    //    obsMode2=FSWITCH: 
    //       SIG=1: FSON
    //       REF=1: FSOFF
    // Calibration scan if CAL != 0
    Int epos = obsMode.find_first_of( sep ) ;
    Int nextpos = obsMode.find_first_of( sep, epos+1 ) ;
    String obsMode1 = obsMode.substr( 0, epos ) ;
    String obsMode2 = obsMode.substr( epos+1, nextpos-epos-1 ) ;
    if ( obsMode1 == "Nod" ) {
      srcType = SrcType::NOD ;
    }
    else if ( obsMode1 == "OffOn" ) {
      if ( obsMode2 == "PSWITCHON" ) srcType = SrcType::PSON ;
      if ( obsMode2 == "PSWITCHOFF" ) srcType = SrcType::PSOFF ;
    }
    else {
      if ( obsMode2 == "FSWITCH" ) {
        if ( sig ) srcType = SrcType::FSON ;
        if ( ref ) srcType = SrcType::FSOFF ;
      }
    }
    if ( cal > 0.0 ) {
      if ( srcType == SrcType::NOD )
        srcType = SrcType::NODCAL ;
      else if ( srcType == SrcType::PSON ) 
        srcType = SrcType::PONCAL ;
      else if ( srcType == SrcType::PSOFF ) 
        srcType = SrcType::POFFCAL ;
      else if ( srcType == SrcType::FSON ) 
        srcType = SrcType::FONCAL ;
      else if ( srcType == SrcType::FSOFF )
        srcType = SrcType::FOFFCAL ;
      else 
        srcType = SrcType::CAL ;
    }
  }
  else if ( sep == "." ) {
    // sep == "."
    //
    // ALMA & EVLA case (MS via ASDM) before3.1
    //
    // obsMode1=CALIBRATE_*
    //    obsMode2=ON_SOURCE: PONCAL
    //    obsMode2=OFF_SOURCE: POFFCAL
    // obsMode1=OBSERVE_TARGET
    //    obsMode2=ON_SOURCE: PON
    //    obsMode2=OFF_SOURCE: POFF
    string substr[2] ; 
    int numSubstr = split( obsMode, substr, 2, "," ) ;
    //os_ << "numSubstr = " << numSubstr << LogIO::POST ;
    //for ( int i = 0 ; i < numSubstr ; i++ )
    //os_ << "substr[" << i << "] = " << substr[i] << LogIO::POST ;
    String obsType( substr[0] ) ;
    //os_ << "obsType = " << obsType << LogIO::POST ;
    Int epos = obsType.find_first_of( sep ) ;
    Int nextpos = obsType.find_first_of( sep, epos+1 ) ;
    String obsMode1 = obsType.substr( 0, epos ) ;
    String obsMode2 = obsType.substr( epos+1, nextpos-epos-1 ) ;
    //os_ << "obsMode1 = " << obsMode1 << LogIO::POST ;
    //os_ << "obsMode2 = " << obsMode2 << LogIO::POST ;
    if ( obsMode1.find( "CALIBRATE_" ) == 0 ) {
      if ( obsMode2 == "ON_SOURCE" ) srcType = SrcType::PONCAL ;
      if ( obsMode2 == "OFF_SOURCE" ) srcType = SrcType::POFFCAL ;
    }
    else if ( obsMode1 == "OBSERVE_TARGET" ) {
      if ( obsMode2 == "ON_SOURCE" ) srcType = SrcType::PSON ;
      if ( obsMode2 == "OFF_SOURCE" ) srcType = SrcType::PSOFF ;
    }
  }
  else if ( sep == "_" ) {
    // sep == "_"
    //
    // ALMA & EVLA case (MS via ASDM) after 3.2
    //
    // obsMode1=CALIBRATE_*
    //    obsMode2=ON_SOURCE: PONCAL
    //    obsMode2=OFF_SOURCE: POFFCAL
    // obsMode1=OBSERVE_TARGET
    //    obsMode2=ON_SOURCE: PON
    //    obsMode2=OFF_SOURCE: POFF
    string substr[2] ; 
    int numSubstr = split( obsMode, substr, 2, "," ) ;
    //os_ << "numSubstr = " << numSubstr << LogIO::POST ;
    //for ( int i = 0 ; i < numSubstr ; i++ )
    //os_ << "substr[" << i << "] = " << substr[i] << LogIO::POST ;
    String obsType( substr[0] ) ;
    //os_ << "obsType = " << obsType << LogIO::POST ;
    string substr2[4] ;
    int numSubstr2 = split( obsType, substr2, 4, sep ) ; 
    //Int epos = obsType.find_first_of( sep ) ;
    //Int nextpos = obsType.find_first_of( sep, epos+1 ) ;
    //String obsMode1 = obsType.substr( 0, epos ) ;
    //String obsMode2 = obsType.substr( epos+1, nextpos-epos-1 ) ;
    String obsMode1( substr2[0] ) ;
    String obsMode2( substr2[2] ) ;
    //os_ << "obsMode1 = " << obsMode1 << LogIO::POST ;
    //os_ << "obsMode2 = " << obsMode2 << LogIO::POST ;
    if ( obsMode1.find( "CALIBRATE" ) == 0 ) {
      if ( obsMode2 == "ON" ) srcType = SrcType::PONCAL ;
      if ( obsMode2 == "OFF" ) srcType = SrcType::POFFCAL ;
    }
    else if ( obsMode1 == "OBSERVE" ) {
      if ( obsMode2 == "ON" ) srcType = SrcType::PSON ;
      if ( obsMode2 == "OFF" ) srcType = SrcType::PSOFF ;
    }
  }
  else {
    if ( sig ) srcType = SrcType::SIG ;
    if ( ref ) srcType = SrcType::REF ;
  }
    
  //os_ << "srcType = " << srcType << LogIO::POST ;
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getSrcType() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return srcType ;
}

//Vector<uInt> MSFiller::getPolNo( Int corrType ) 
Block<uInt> MSFiller::getPolNo( Int corrType ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getPolNo() startSec=" << startSec << LogIO::POST ;
  Block<uInt> polno( 1 ) ;

  if ( corrType == Stokes::I || corrType == Stokes::RR || corrType == Stokes::XX ) {
    polno = 0 ;
  }
  else if ( corrType == Stokes::Q || corrType == Stokes::LL || corrType == Stokes::YY ) {
    polno = 1 ;
  }
  else if ( corrType == Stokes::U ) {
    polno = 2 ;
  }
  else if ( corrType == Stokes::V ) {
    polno = 3 ;
  }
  else if ( corrType == Stokes::RL || corrType == Stokes::XY || corrType == Stokes::LR || corrType == Stokes::RL ) {
    polno.resize( 2 ) ;
    polno[0] = 2 ;
    polno[1] = 3 ;
  }
  else if ( corrType == Stokes::Plinear ) {
    polno[0] = 1 ;
  }
  else if ( corrType == Stokes::Pangle ) {
    polno[0] = 2 ;
  }
  else {
    polno = 99 ;
  }
  //os_ << "polno = " << polno << LogIO::POST ;
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getPolNo() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  
  return polno ;
}

String MSFiller::getPolType( Int corrType ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getPolType() startSec=" << startSec << LogIO::POST ;
  String poltype = "" ;

  if ( corrType == Stokes::I || corrType == Stokes::Q || corrType == Stokes::U || corrType == Stokes::V )
    poltype = "stokes" ;
  else if ( corrType == Stokes::XX || corrType == Stokes::YY || corrType == Stokes::XY || corrType == Stokes::YX ) 
    poltype = "linear" ;
  else if ( corrType == Stokes::RR || corrType == Stokes::LL || corrType == Stokes::RL || corrType == Stokes::LR ) 
    poltype = "circular" ;
  else if ( corrType == Stokes::Plinear || corrType == Stokes::Pangle )
    poltype = "linpol" ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getPolType() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return poltype ;
}

void MSFiller::fillWeather()
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::fillWeather() startSec=" << startSec << LogIO::POST ;

  if ( !isWeather_ ) {
    // add dummy row
    table_->weather().table().addRow(1,True) ;
    return ;
  }

  Table mWeather = mstable_.weather()  ;
  //Table mWeatherSel = mWeather( mWeather.col("ANTENNA_ID") == antenna_ ).sort("TIME") ;
  Table mWeatherSel( mWeather( mWeather.col("ANTENNA_ID") == antenna_ ).sort("TIME") ) ;
  //os_ << "mWeatherSel.nrow() = " << mWeatherSel.nrow() << LogIO::POST ;
  if ( mWeatherSel.nrow() == 0 ) {
    os_ << "No rows with ANTENNA_ID = " << antenna_ << " in WEATHER table, Try -1..." << LogIO::POST ; 
    mWeatherSel = Table( MSWeather( mWeather( mWeather.col("ANTENNA_ID") == -1 ) ) ) ;
    if ( mWeatherSel.nrow() == 0 ) {
      os_ << "No rows in WEATHER table" << LogIO::POST ;
    }
  }
  uInt wnrow = mWeatherSel.nrow() ;
  //os_ << "wnrow = " << wnrow << LogIO::POST ;

  if ( wnrow == 0 ) 
    return ;

  Table wtab = table_->weather().table() ;
  wtab.addRow( wnrow ) ;

  ScalarColumn<Float> *fCol ;
  ROScalarColumn<Float> *sharedFloatCol ;
  if ( mWeatherSel.tableDesc().isColumn( "TEMPERATURE" ) ) {
    fCol = new ScalarColumn<Float>( wtab, "TEMPERATURE" ) ;
    sharedFloatCol = new ROScalarColumn<Float>( mWeatherSel, "TEMPERATURE" ) ;
    fCol->putColumn( *sharedFloatCol ) ;
    delete sharedFloatCol ;
    delete fCol ;
  }
  if ( mWeatherSel.tableDesc().isColumn( "PRESSURE" ) ) {
    fCol = new ScalarColumn<Float>( wtab, "PRESSURE" ) ;
    sharedFloatCol = new ROScalarColumn<Float>( mWeatherSel, "PRESSURE" ) ;
    fCol->putColumn( *sharedFloatCol ) ;
    delete sharedFloatCol ;
    delete fCol ;
  }
  if ( mWeatherSel.tableDesc().isColumn( "REL_HUMIDITY" ) ) {
    fCol = new ScalarColumn<Float>( wtab, "HUMIDITY" ) ;
    sharedFloatCol = new ROScalarColumn<Float>( mWeatherSel, "REL_HUMIDITY" ) ;
    fCol->putColumn( *sharedFloatCol ) ;
    delete sharedFloatCol ;
    delete fCol ;
  }
  if ( mWeatherSel.tableDesc().isColumn( "WIND_SPEED" ) ) {  
    fCol = new ScalarColumn<Float>( wtab, "WINDSPEED" ) ;
    sharedFloatCol = new ROScalarColumn<Float>( mWeatherSel, "WIND_SPEED" ) ;
    fCol->putColumn( *sharedFloatCol ) ;
    delete sharedFloatCol ;
    delete fCol ;
  }
  if ( mWeatherSel.tableDesc().isColumn( "WIND_DIRECTION" ) ) {
    fCol = new ScalarColumn<Float>( wtab, "WINDAZ" ) ;
    sharedFloatCol = new ROScalarColumn<Float>( mWeatherSel, "WIND_DIRECTION" ) ;
    fCol->putColumn( *sharedFloatCol ) ;
    delete sharedFloatCol ;
    delete fCol ;
  }
  ScalarColumn<uInt> idCol( wtab, "ID" ) ;
  for ( uInt irow = 0 ; irow < wnrow ; irow++ ) 
    idCol.put( irow, irow ) ;

  ROScalarQuantColumn<Double> tqCol( mWeatherSel, "TIME" ) ;
  ROScalarColumn<Double> tCol( mWeatherSel, "TIME" ) ;
  String tUnit = tqCol.getUnits() ;
  mwTime_ = tCol.getColumn() ;
  if ( tUnit == "d" ) 
    mwTime_ *= 86400.0 ;
  tqCol.attach( mWeatherSel, "INTERVAL" ) ;
  tCol.attach( mWeatherSel, "INTERVAL" ) ;
  String iUnit = tqCol.getUnits() ;
  mwInterval_ = tCol.getColumn() ;
  if ( iUnit == "d" ) 
    mwInterval_ *= 86400.0 ; 
  //os_ << "mwTime[0] = " << mwTime_[0] << " mwInterval[0] = " << mwInterval_[0] << LogIO::POST ; 
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::fillWeather() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::fillFocus()
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::fillFocus() startSec=" << startSec << LogIO::POST ;
  // tentative
  Table tab = table_->focus().table() ;
  tab.addRow( 1 ) ;
  ScalarColumn<uInt> idCol( tab, "ID" ) ;
  idCol.put( 0, 0 ) ;
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::fillFocus() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::fillTcal( boost::object_pool<ROTableColumn> *tpoolr )
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::fillTcal() startSec=" << startSec << LogIO::POST ;

  if ( !isSysCal_ ) {
    // add dummy row
    os_ << "No SYSCAL rows" << LogIO::POST ;
    table_->tcal().table().addRow(1,True) ;
    Vector<Float> defaultTcal( 1, 1.0 ) ;
    ArrayColumn<Float> tcalCol( table_->tcal().table(), "TCAL" ) ;
    tcalCol.put( 0, defaultTcal ) ;
    return ;
  }

  Table sctab = mstable_.sysCal() ;
  if ( sctab.nrow() == 0 ) {
    os_ << "No SYSCAL rows" << LogIO::POST ;
    return ;
  } 
  Table sctabsel( sctab( sctab.col("ANTENNA_ID") == antenna_ ) ) ;
  if ( sctabsel.nrow() == 0 ) {
    os_ << "No SYSCAL rows" << LogIO::POST ;
    return ;
  } 
  ROArrayColumn<Float> *tmpTcalCol = new ROArrayColumn<Float>( sctabsel, "TCAL" ) ;
  uInt npol = tmpTcalCol->shape( 0 )(0) ;
  delete tmpTcalCol ;
  //os_ << "fillTcal(): npol = " << npol << LogIO::POST ;
  Table tab = table_->tcal().table() ;
  ArrayColumn<Float> tcalCol( tab, "TCAL" ) ;
  ROTableColumn *sharedCol ;
  uInt oldnr = 0 ;
  uInt newnr = 0 ;
  TableRow row( tab ) ;
  TableRecord &trec = row.record() ;
  RecordFieldPtr<uInt> idRF( trec, "ID" ) ;
  RecordFieldPtr<String> timeRF( trec, "TIME" ) ;
  RecordFieldPtr< Array<Float> > tcalRF( trec, "TCAL" ) ;
  TableIterator iter0( sctabsel, "FEED_ID" ) ;
  while( !iter0.pastEnd() ) {
    Table t0 = iter0.table() ;
    sharedCol = tpoolr->construct( t0, "FEED_ID" ) ;
    Int feedId = sharedCol->asInt( 0 ) ;
    tpoolr->destroy( sharedCol ) ;
    TableIterator iter1( t0, "SPECTRAL_WINDOW_ID" ) ;
    while( !iter1.pastEnd() ) {
      Table t1 = iter1.table() ;
      sharedCol = tpoolr->construct( t1, "SPECTRAL_WINDOW_ID" ) ;
      Int spwId = sharedCol->asInt( 0 ) ;
      tpoolr->destroy( sharedCol ) ;
      tmpTcalCol = new ROArrayColumn<Float>( t1, colTcal_ ) ;
      ROScalarQuantColumn<Double> scTimeCol( t1, "TIME" ) ;
      Vector<uInt> idminmax( 2, oldnr ) ;
      for ( uInt irow = 0 ; irow < t1.nrow() ; irow++ ) {
        String sTime = MVTime( scTimeCol(irow) ).string( MVTime::YMD ) ;
        *timeRF = sTime ;
        uInt idx = oldnr ;
        Matrix<Float> subtcal = (*tmpTcalCol)( irow ) ;
        for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
          *idRF = idx++ ;
          //*tcalRF = subtcal.row( ipol ) ;
          tcalRF.define( subtcal.row( ipol ) ) ;

          // commit row
          tab.addRow() ;
          row.put( tab.nrow()-1 ) ;

          newnr++ ;
        }
        idminmax[0] = oldnr ;
        idminmax[1] = newnr - 1 ;
        oldnr = newnr ;

        String key = keyTcal( feedId, spwId, sTime ) ;
        tcalrec_.define( key, idminmax ) ;
      }
      delete tmpTcalCol ;
      iter1++ ;
    }
    iter0++ ;
  }

  //tcalrec_.print( std::cout ) ;
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::fillTcal() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

uInt MSFiller::getWeatherId( uInt idx, Double wtime ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getWeatherId() startSec=" << startSec << LogIO::POST ;
  uInt nrow = mwTime_.size() ;
  if ( nrow < 2 ) 
    return 0 ;
  uInt wid = nrow ;
  for ( uInt i = idx ; i < nrow-1 ; i++ ) {
    Double tStart = mwTime_[i]-0.5*mwInterval_[i] ;
    // use of INTERVAL column is problematic 
    // since there are "blank" time of weather monitoring
    //Double tEnd = tStart + mwInterval_[i] ;
    Double tEnd = mwTime_[i+1]-0.5*mwInterval_[i+1] ;
    //os_ << "tStart = " << tStart << " dtEnd = " << tEnd-tStart << " dwtime = " << wtime-tStart << LogIO::POST ;
    if ( wtime >= tStart && wtime <= tEnd ) {
      wid = i ;
      break ;
    }
  }
  if ( wid == nrow ) {
    uInt i = nrow - 1 ;
    Double tStart = mwTime_[i-1]+0.5*mwInterval_[i-1] ;
    Double tEnd = mwTime_[i]+0.5*mwInterval_[i] ;
    //os_ << "tStart = " << tStart << " dtEnd = " << tEnd-tStart << " dwtime = " << wtime-tStart << LogIO::POST ;
    if ( wtime >= tStart && wtime <= tEnd )
      wid = i-1 ;
    else 
      wid = i ;
  }

  //if ( wid == nrow ) 
  //os_ << LogIO::WARN << "Couldn't find correct WEATHER_ID for time " << wtime << LogIO::POST ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getWeatherId() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return wid ;
}

void MSFiller::getSysCalTime( Vector<MEpoch> &scTime, Vector<Double> &scInterval, Block<MEpoch> &tcol, Block<Int> &tidx )
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getSysCalTime() startSec=" << startSec << LogIO::POST ;

  if ( !isSysCal_ )
    return ;

  uInt nrow = tidx.nelements() ;
  if ( scTime.nelements() == 0 ) 
    return ;
  else if ( scTime.nelements() == 1 ) {
    tidx[0] = 0 ;
    return ;
  }
  uInt scnrow = scTime.nelements() ;
  uInt idx = 0 ;
  const Double half = 0.5e0 ;
  // execute  binary search
  idx = binarySearch( scTime, tcol[0].get( "s" ).getValue() ) ;
  if ( idx != 0 )
    idx -= 1 ;
  for ( uInt i = 0 ; i < nrow ; i++ ) {
    Double t = tcol[i].get( "s" ).getValue() ;
    Double tsc = scTime[0].get( "s" ).getValue() ;
    if ( t < tsc ) {
      tidx[i] = 0 ;
      continue ;
    }
    for ( uInt j = idx ; j < scnrow-1 ; j++ ) {
      Double tsc1 = scTime[j].get( "s" ).getValue() ;
      Double dt1 = scInterval[j] ;
      Double tsc2 = scTime[j+1].get( "s" ).getValue() ;
      Double dt2 = scInterval[j+1] ;
      if ( t > tsc1-half*dt1 && t <= tsc2-half*dt2 ) {
        tidx[i] = j ;
        idx = j ;
        break ;
      }
    }
    if ( tidx[i] == -1 ) {
//       Double tsc = scTime[scnrow-1].get( "s" ).getValue() ;
//       Double dt = scInterval[scnrow-1] ;
//       if ( t <= tsc+0.5*dt ) {
//         tidx[i] = scnrow-1 ;
//       }
      tidx[i] = scnrow-1 ;
    }
  }
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getSysCalTime() endSec=" << endSec << " (" << endSec-startSec << "sec) scnrow = " << scnrow << " tcol.nelements = " << tcol.nelements() << LogIO::POST ;
  return ;
}

Block<uInt> MSFiller::getTcalId( Int fid, Int spwid, MEpoch &t ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getTcalId() startSec=" << startSec << LogIO::POST ;
  //if ( table_->tcal().table().nrow() == 0 ) {
  if ( !isSysCal_ ) {
    os_ << "No TCAL rows" << LogIO::POST ;
    Block<uInt> tcalids( 0 ) ;
    return  tcalids ;
  }    
  //String sctime = MVTime( Quantum<Double>(t,"s") ).string(MVTime::YMD) ;
  String sctime = MVTime( t.getValue() ).string(MVTime::YMD) ;
  String key = keyTcal( fid, spwid, sctime ) ;
  if ( !tcalrec_.isDefined( key ) ) {
    os_ << "No TCAL rows" << LogIO::POST ;
    Block<uInt> tcalids( 0 ) ;
    return tcalids ;
  }
  Vector<uInt> ids = tcalrec_.asArrayuInt( key ) ;
  uInt npol = ids[1] - ids[0] + 1 ;
  Block<uInt> tcalids( npol ) ;
  tcalids[0] = ids[0] ;
  tcalids[1] = ids[1] ;
  for ( uInt ipol = 2 ; ipol < npol ; ipol++ )
    tcalids[ipol] = ids[0] + ipol - 1 ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getTcalId() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return tcalids ;
}

uInt MSFiller::getDirection( uInt idx, Vector<Double> &dir, Vector<Double> &srate, String &ref, MSPointing &tab, Double t ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSFiller::getDirection() startSec=" << startSec << LogIO::POST ;
  // assume that cols is sorted by TIME
  Bool doInterp = False ;
  //uInt nrow = cols.nrow() ;
  uInt nrow = tab.nrow() ;
  if ( nrow == 0 ) 
    return 0 ;
  ROScalarMeasColumn<MEpoch> tcol( tab, "TIME" ) ;
  ROArrayMeasColumn<MDirection> dmcol( tab, "DIRECTION" ) ;
  ROArrayColumn<Double> dcol( tab, "DIRECTION" ) ;
  // binary search if idx == 0
  if ( idx == 0 ) {
    uInt nrowb = 75000 ;
    if ( nrow > nrowb ) {
      uInt nblock = nrow / nrowb + 1 ;
      for ( uInt iblock = 0 ; iblock < nblock ; iblock++ ) {
        uInt high = min( nrowb, nrow-iblock*nrowb ) ;

        if ( tcol( high-1 ).get( "s" ).getValue() < t ) {
          idx = iblock * nrowb ;
          continue ;
        }

        Vector<MEpoch> tarr( high ) ;
        for ( uInt irow = 0 ; irow < high ; irow++ ) {
          tarr[irow] = tcol( iblock*nrowb+irow ) ;
        }

        uInt bidx = binarySearch( tarr, t ) ;

        idx = iblock * nrowb + bidx ;
        break ;
      }
    }
    else {
      Vector<MEpoch> tarr( nrow ) ;
      for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
        tarr[irow] = tcol( irow ) ;
      }
      idx = binarySearch( tarr, t ) ;
    }
  }
  // ensure that tcol(idx) < t
  //os_ << "tcol(idx) = " << tcol(idx).get("s").getValue() << " t = " << t << " diff = " << tcol(idx).get("s").getValue()-t << endl ;
  while ( tcol(idx).get("s").getValue() > t && idx > 0 ) 
    idx-- ;
  //os_ << "idx = " << idx << LogIO::POST ;

  // index search
  for ( uInt i = idx ; i < nrow ; i++ ) {
    Double tref = tcol( i ).get( "s" ).getValue() ;
    if ( tref == t ) {
      idx = i ;
      break ;
    }
    else if ( tref > t ) {
      if ( i == 0 ) {
        idx = i ;
      }
      else {
        idx = i-1 ;
        doInterp = True ;
      }
      break ;
    }
    else {
      idx = nrow - 1 ;
    }
  }
  //os_ << "searched idx = " << idx << LogIO::POST ;

  //os_ << "dmcol(idx).shape() = " << dmcol(idx).shape() << LogIO::POST ;
  IPosition ip( dmcol(idx).shape().nelements(), 0 ) ;
  //os_ << "ip = " << ip << LogIO::POST ;
  ref = dmcol(idx)(ip).getRefString() ;
  //os_ << "ref = " << ref << LogIO::POST ;
  if ( doInterp ) {
    //os_ << "do interpolation" << LogIO::POST ;
    //os_ << "dcol(idx).shape() = " << dcol(idx).shape() << LogIO::POST ;
    Double tref0 = tcol(idx).get("s").getValue() ;
    Double tref1 = tcol(idx+1).get("s").getValue() ;
    Matrix<Double> mdir0 = dcol( idx ) ;
    Matrix<Double> mdir1 = dcol( idx+1 ) ;
    Vector<Double> dir0 = mdir0.column( 0 ) ;
    //os_ << "dir0 = " << dir0 << LogIO::POST ; 
    Vector<Double> dir1 = mdir1.column( 0 ) ;
    //os_ << "dir1 = " << dir1 << LogIO::POST ; 
    Double dt0 = t - tref0 ;
    Double dt1 = tref1 - t ;
    dir.reference( (dt0*dir1+dt1*dir0)/(dt0+dt1) ) ;
    if ( mdir0.ncolumn() > 1 ) {
      if ( dt0 >= dt1 )
        srate.reference( mdir0.column( 1 ) ) ;
      else
        srate.reference( mdir1.column( 1 ) ) ;
    }
    //os_ << "dir = " << dir << LogIO::POST ; 
  }
  else {
    //os_ << "no interpolation" << LogIO::POST ;
    Matrix<Double> mdir0 = dcol( idx ) ;
    dir.reference( mdir0.column( 0 ) ) ;
    if ( mdir0.ncolumn() > 1 )
      srate.reference( mdir0.column( 1 ) ) ;
  }

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSFiller::getDirection() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return idx ;
}

String MSFiller::keyTcal( Int feedid, Int spwid, String stime ) 
{
  String sfeed = "FEED" + String::toString( feedid ) ;
  String sspw = "SPW" + String::toString( spwid ) ;
  return sfeed+":"+sspw+":"+stime ;
}

uInt MSFiller::binarySearch( Vector<MEpoch> &timeList, Double target ) 
{
  Int low = 0 ;
  Int high = timeList.nelements() ;
  uInt idx = 0 ;

  while ( low <= high ) {
    idx = (Int)( 0.5 * ( low + high ) ) ;
    Double t = timeList[idx].get( "s" ).getValue() ;
    if ( t < target ) 
      low = idx + 1 ;
    else if ( t > target )
      high = idx - 1 ;
    else 
      return idx ;
  }

  idx = max( 0, min( low, high ) ) ;

  return idx ;
  
}

} ;


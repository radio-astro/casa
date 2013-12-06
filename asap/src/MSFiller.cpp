//
// C++ Interface: MSFiller
//
// Description:
//
// This class is specific filler for MS format 
// New version that is implemented using TableVisitor instead of TableIterator
//
// Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <assert.h>
#include <iostream>
#include <map>
#include <set>

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

#include "MathUtils.h"

using namespace casa ;
using namespace std ;

namespace asap {

class BaseMSFillerVisitor: public TableVisitor {
  uInt lastRecordNo ;
  Int lastObservationId ;
  Int lastFeedId ;
  Int lastFieldId ;
  Int lastDataDescId ;
  Int lastScanNo ;
  Int lastStateId ;
  Double lastTime ;
protected:
  const Table &table;
  uInt count;
public:
  BaseMSFillerVisitor(const Table &table)
   : table(table)
  {
    count = 0;
  }
  
  virtual void enterObservationId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveObservationId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterFeedId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveFeedId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterFieldId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveFieldId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterDataDescId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveDataDescId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterScanNo(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveScanNo(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterStateId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveStateId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) { }

  virtual Bool visitRecord(const uInt /*recordNo*/,
			   const Int /*ObservationId*/,
			   const Int /*feedId*/,
			   const Int /*fieldId*/,
			   const Int /*dataDescId*/,
			   const Int /*scanNo*/,
			   const Int /*stateId*/,
			   const Double /*time*/) { return True ; }

  virtual Bool visit(Bool isFirst, const uInt recordNo,
		     const uInt nCols, void const *const colValues[]) {
    Int observationId, feedId, fieldId, dataDescId, scanNo, stateId;
    Double time;
    { // prologue
      uInt i = 0;
      {
	const Int *col = (const Int *)colValues[i++];
	observationId = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	feedId = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	fieldId = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	dataDescId = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	scanNo = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	stateId = col[recordNo];
      }
      {
	const Double *col = (const Double *)colValues[i++];
	time = col[recordNo];
      }
      assert(nCols == i);
    }

    if (isFirst) {
      enterObservationId(recordNo, observationId);
      enterFeedId(recordNo, feedId);
      enterFieldId(recordNo, fieldId);
      enterDataDescId(recordNo, dataDescId);
      enterScanNo(recordNo, scanNo);
      enterStateId(recordNo, stateId);
      enterTime(recordNo, time);
    } else {
      if (lastObservationId != observationId) {
	leaveTime(lastRecordNo, lastTime);
	leaveStateId(lastRecordNo, lastStateId);
	leaveScanNo(lastRecordNo, lastScanNo);
	leaveDataDescId(lastRecordNo, lastDataDescId);
	leaveFieldId(lastRecordNo, lastFieldId);
	leaveFeedId(lastRecordNo, lastFeedId);
	leaveObservationId(lastRecordNo, lastObservationId);

	enterObservationId(recordNo, observationId);
	enterFeedId(recordNo, feedId);
	enterFieldId(recordNo, fieldId);
	enterDataDescId(recordNo, dataDescId);
	enterScanNo(recordNo, scanNo);
	enterStateId(recordNo, stateId);
	enterTime(recordNo, time);
      } else if (lastFeedId != feedId) {
	leaveTime(lastRecordNo, lastTime);
	leaveStateId(lastRecordNo, lastStateId);
	leaveScanNo(lastRecordNo, lastScanNo);
	leaveDataDescId(lastRecordNo, lastDataDescId);
	leaveFieldId(lastRecordNo, lastFieldId);
	leaveFeedId(lastRecordNo, lastFeedId);

	enterFeedId(recordNo, feedId);
	enterFieldId(recordNo, fieldId);
	enterDataDescId(recordNo, dataDescId);
	enterScanNo(recordNo, scanNo);
	enterStateId(recordNo, stateId);
	enterTime(recordNo, time);
      } else if (lastFieldId != fieldId) {
	leaveTime(lastRecordNo, lastTime);
	leaveStateId(lastRecordNo, lastStateId);
	leaveScanNo(lastRecordNo, lastScanNo);
	leaveDataDescId(lastRecordNo, lastDataDescId);
	leaveFieldId(lastRecordNo, lastFieldId);

	enterFieldId(recordNo, fieldId);
	enterDataDescId(recordNo, dataDescId);
	enterScanNo(recordNo, scanNo);
	enterStateId(recordNo, stateId);
	enterTime(recordNo, time);
      } else if (lastDataDescId != dataDescId) {
	leaveTime(lastRecordNo, lastTime);
	leaveStateId(lastRecordNo, lastStateId);
	leaveScanNo(lastRecordNo, lastScanNo);
	leaveDataDescId(lastRecordNo, lastDataDescId);

	enterDataDescId(recordNo, dataDescId);
	enterScanNo(recordNo, scanNo);
	enterStateId(recordNo, stateId);
	enterTime(recordNo, time);
      } else if (lastScanNo != scanNo) {
	leaveTime(lastRecordNo, lastTime);
	leaveStateId(lastRecordNo, lastStateId);
	leaveScanNo(lastRecordNo, lastScanNo);

	enterScanNo(recordNo, scanNo);
	enterStateId(recordNo, stateId);
	enterTime(recordNo, time);
      } else if (lastStateId != stateId) {
	leaveTime(lastRecordNo, lastTime);
	leaveStateId(lastRecordNo, lastStateId);

	enterStateId(recordNo, stateId);
	enterTime(recordNo, time);
      } else if (lastTime != time) {
	leaveTime(lastRecordNo, lastTime);
	enterTime(recordNo, time);
      }
    }
    count++;
    Bool result = visitRecord(recordNo, observationId, feedId, fieldId, dataDescId,
                              scanNo, stateId, time);

    { // epilogue
      lastRecordNo = recordNo;

      lastObservationId = observationId;
      lastFeedId = feedId;
      lastFieldId = fieldId;
      lastDataDescId = dataDescId;
      lastScanNo = scanNo;
      lastStateId = stateId;
      lastTime = time;
    }
    return result ;
  }

  virtual void finish() {
    if (count > 0) {
      leaveTime(lastRecordNo, lastTime);
      leaveStateId(lastRecordNo, lastStateId);
      leaveScanNo(lastRecordNo, lastScanNo);
      leaveDataDescId(lastRecordNo, lastDataDescId);
      leaveFieldId(lastRecordNo, lastFieldId);
      leaveFeedId(lastRecordNo, lastFeedId);
      leaveObservationId(lastRecordNo, lastObservationId);
    }
  }
};

class MSFillerVisitor: public BaseMSFillerVisitor, public MSFillerUtils {
public:
  MSFillerVisitor(const Table &from, Scantable &to)
    : BaseMSFillerVisitor(from),
      scantable(to)
  { 
    antennaId = 0 ;
    rowidx = 0 ;
    tablerow = TableRow( scantable.table() ) ;
    feedEntry = Vector<Int>( 64, -1 ) ;
    nbeam = 0 ;
    ifmap.clear() ;
    const TableDesc &desc = table.tableDesc() ;
    if ( desc.isColumn( "DATA" ) )
      dataColumnName = "DATA" ;
    else if ( desc.isColumn( "FLOAT_DATA" ) )
      dataColumnName = "FLOAT_DATA" ;
    getpt = False ;
    isWeather_ = False ;
    isSysCal = False ;
    isTcal = False ;
    cycleNo = 0 ;
    numSysCalRow = 0 ;
    header = scantable.getHeader() ;
    fluxUnit( header.fluxunit ) ;

    // MS subtables
    const TableRecord &hdr = table.keywordSet();
    obstab = hdr.asTable( "OBSERVATION" ) ;
    spwtab = hdr.asTable( "SPECTRAL_WINDOW" ) ;
    statetab = hdr.asTable( "STATE" ) ;
    ddtab = hdr.asTable( "DATA_DESCRIPTION" ) ;
    poltab = hdr.asTable( "POLARIZATION" ) ;
    fieldtab = hdr.asTable( "FIELD" ) ;
    anttab = hdr.asTable( "ANTENNA" ) ;
    if ( hdr.isDefined( "SYSCAL" ) ) 
      sctab = hdr.asTable( "SYSCAL" ) ;
    if ( hdr.isDefined( "SOURCE" ) ) 
      srctab = hdr.asTable( "SOURCE" ) ;

    // attach to columns
    // MS MAIN
    intervalCol.attach( table, "INTERVAL" ) ;
    flagRowCol.attach( table, "FLAG_ROW" ) ;
    flagCol.attach( table, "FLAG" ) ;
    if ( dataColumnName.compare( "DATA" ) == 0 )
      dataCol.attach( table, dataColumnName ) ;
    else
      floatDataCol.attach( table, dataColumnName ) ;

    // set dummy epoch
    mf.set( currentTime ) ;

    //
    // add rows to scantable
    //
    // number of polarization is up to 4
    uInt addrow = table.nrow() * maxNumPol() ;
    scantable.table().addRow( addrow ) ;

    // attach to columns
    // Scantable MAIN
    TableRecord &r = tablerow.record() ;
    timeRF.attachToRecord( r, "TIME" ) ;
    intervalRF.attachToRecord( r, "INTERVAL" ) ;
    directionRF.attachToRecord( r, "DIRECTION" ) ;
    azimuthRF.attachToRecord( r, "AZIMUTH" ) ;
    elevationRF.attachToRecord( r, "ELEVATION" ) ;
    scanRateRF.attachToRecord( r, "SCANRATE" ) ;
    weatherIdRF.attachToRecord( r, "WEATHER_ID" ) ;
    cycleNoRF.attachToRecord( r, "CYCLENO" ) ;
    flagRowRF.attachToRecord( r, "FLAGROW" ) ;
    polNoRF.attachToRecord( r, "POLNO" ) ;
    tcalIdRF.attachToRecord( r, "TCAL_ID" ) ;
    spectraRF.attachToRecord( r, "SPECTRA" ) ;
    flagtraRF.attachToRecord( r, "FLAGTRA" ) ;
    tsysRF.attachToRecord( r, "TSYS" ) ;
    beamNoRF.attachToRecord( r, "BEAMNO" ) ;
    ifNoRF.attachToRecord( r, "IFNO" ) ;
    freqIdRF.attachToRecord( r, "FREQ_ID" ) ;
    moleculeIdRF.attachToRecord( r, "MOLECULE_ID" ) ;
    sourceNameRF.attachToRecord( r, "SRCNAME" ) ;
    sourceProperMotionRF.attachToRecord( r, "SRCPROPERMOTION" ) ;
    sourceDirectionRF.attachToRecord( r, "SRCDIRECTION" ) ;
    sourceVelocityRF.attachToRecord( r, "SRCVELOCITY" ) ;
    focusIdRF.attachToRecord( r, "FOCUS_ID" ) ;
    fieldNameRF.attachToRecord( r, "FIELDNAME" ) ;
    sourceTypeRF.attachToRecord( r, "SRCTYPE" ) ;
    scanNoRF.attachToRecord( r, "SCANNO" ) ;

    // put values
    RecordFieldPtr<Int> refBeamNoRF( r, "REFBEAMNO" ) ;
    *refBeamNoRF = -1 ;
    RecordFieldPtr<Int> fitIdRF( r, "FIT_ID" ) ;
    *fitIdRF = -1 ;
    RecordFieldPtr<Float> opacityRF( r, "OPACITY" ) ;
    *opacityRF = 0.0 ;
  }

  virtual void enterObservationId(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: ObservationId: %d\n", recordNo, columnValue);
    // update header 
    if ( header.observer.empty() ) 
      getScalar( String("OBSERVER"), (uInt)columnValue, obstab, header.observer ) ;
    if ( header.project.empty() )
      getScalar( "PROJECT", (uInt)columnValue, obstab, header.project ) ;
    if ( header.utc == 0.0 ) {
      Vector<MEpoch> amp ;
      getArrayMeas( "TIME_RANGE", (uInt)columnValue, obstab, amp ) ;
      obsEpoch = amp[0];
      header.utc = obsEpoch.get( "d" ).getValue() ;
    }
    if ( header.antennaname.empty() )
      getScalar( "TELESCOPE_NAME", (uInt)columnValue, obstab, header.antennaname ) ;
  }
  virtual void leaveObservationId(const uInt /*recordNo*/, Int /*columnValue*/) {
    // update header
    header.nbeam = max( header.nbeam, (Int)nbeam ) ;

    nbeam = 0 ;
    feedEntry = -1 ;
  }
  virtual void enterFeedId(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: FeedId: %d\n", recordNo, columnValue);

    // update feed entry
    if ( allNE( feedEntry, columnValue ) ) {
      feedEntry[nbeam] = columnValue ;
      nbeam++ ;
    }

    // put values
    *beamNoRF = (uInt)columnValue ;
    *focusIdRF = (uInt)0 ;
  }
  virtual void leaveFeedId(const uInt /*recordNo*/, Int /*columnValue*/) {
    uInt nelem = feedEntry.nelements() ;
    if ( nbeam > nelem ) {
      feedEntry.resize( nelem+64, True ) ;
      Slicer slice( IPosition( 1, nelem ), IPosition( 1, feedEntry.nelements()-1 ) ) ;
      feedEntry( slice ) = -1 ;
    }
  }
  virtual void enterFieldId(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: FieldId: %d\n", recordNo, columnValue);
    // update sourceId and fieldName
    getScalar( "SOURCE_ID", (uInt)columnValue, fieldtab, sourceId ) ;
    String fieldName ;
    getScalar( "NAME", (uInt)columnValue, fieldtab, fieldName ) ; 
    fieldName += "__" + String::toString( columnValue ) ;

    // put values
    *fieldNameRF = fieldName ;
  }
  virtual void leaveFieldId(const uInt /*recordNo*/, Int /*columnValue*/) {
    sourceId = -1 ;
  }
  virtual void enterDataDescId(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: DataDescId: %d\n", recordNo, columnValue);
    // update polarization and spectral window ids
    getScalar( "POLARIZATION_ID", (uInt)columnValue, ddtab, polId ) ;
    getScalar( "SPECTRAL_WINDOW_ID", (uInt)columnValue, ddtab, spwId ) ;

    // polarization setup
    getScalar( "NUM_CORR", (uInt)polId, poltab, npol ) ;
    Vector<Int> corrtype ;
    getArray( "CORR_TYPE", (uInt)polId, poltab, corrtype ) ;
    polnos = getPolNos( corrtype ) ; 
    
    // process SOURCE table
    String sourceName ;
    Vector<Double> sourcePM, restFreqs, sysVels ;
    Vector<String> transition ;
    processSource( sourceId, spwId, sourceName, sourceDir, sourcePM, 
                   restFreqs, transition, sysVels ) ;

    // spectral setup
    uInt freqId ;
    Double reffreq, bandwidth ;
    String freqref ;
    getScalar( "NUM_CHAN", (uInt)spwId, spwtab, nchan ) ;
    Bool iswvr = (Bool)(nchan == 4) ;
    map<Int,uInt>::iterator iter = ifmap.find( spwId ) ;
    if ( iter == ifmap.end() ) {
      //MEpoch me ;
      //getScalarMeas( "TIME", recordNo, table, me ) ;
      //spectralSetup( spwId, me, antpos, sourceDir, 
      spectralSetup(spwId, obsEpoch, antpos, sourceDir, 
                    freqId, nchan, 
                    freqref, reffreq, bandwidth);
      ifmap.insert( pair<Int,uInt>(spwId,freqId) ) ;
    }
    else {
      freqId = iter->second ;
    }
    sp.resize( npol, nchan ) ;
    fl.resize( npol, nchan ) ;


    // molecular setup
    STMolecules mtab = scantable.molecules() ;
    uInt molId = mtab.addEntry( restFreqs, transition, transition ) ;

    // process SYSCAL table
    if ( isSysCal )
      processSysCal( spwId ) ;

    // update header
    if ( !iswvr ) {
      header.nchan = max( header.nchan, nchan ) ;
      header.bandwidth = max( header.bandwidth, bandwidth ) ;
      if ( header.reffreq == -1.0 )
        header.reffreq = reffreq ;
      header.npol = max( header.npol, npol ) ;
      if ( header.poltype.empty() )
        header.poltype = getPolType( corrtype[0] ) ;
      if ( header.freqref.empty() )
        header.freqref = freqref ;
    }
    
    // put values
    *ifNoRF = (uInt)spwId ;
    *freqIdRF = freqId ;
    *moleculeIdRF = molId ;
    *sourceNameRF = sourceName ;
    sourceProperMotionRF.define( sourcePM ) ;
    Vector<Double> srcD = sourceDir.getAngle().getValue( "rad" ) ;
    sourceDirectionRF.define( srcD ) ;
    if ( !sysVels.empty() ) 
      *sourceVelocityRF = sysVels[0] ;
    else {
      *sourceVelocityRF = (Double)0.0 ;
    }
  }
  virtual void leaveDataDescId(const uInt /*recordNo*/, Int /*columnValue*/) {
    npol = 0 ;
    nchan = 0 ;
    numSysCalRow = 0 ;
  }
  virtual void enterScanNo(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: ScanNo: %d\n", recordNo, columnValue);
    // put value
    // CAS-5841: SCANNO should be consistent with MS SCAN_NUMBER
    *scanNoRF = (uInt)columnValue ;
  }
  virtual void leaveScanNo(const uInt /*recordNo*/, Int /*columnValue*/) {
    cycleNo = 0 ;
  }
  virtual void enterStateId(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: StateId: %d\n", recordNo, columnValue);
    // SRCTYPE
    Int srcType = getSrcType( columnValue ) ;
    
    // update header
    if ( header.obstype.empty() ) 
      getScalar( "OBS_MODE", (uInt)columnValue, statetab, header.obstype ) ;

    // put value
    *sourceTypeRF = srcType ;
  }
  virtual void leaveStateId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterTime(const uInt recordNo, Double columnValue) {
    //printf("%u: Time: %f\n", recordNo, columnValue);
    currentTime = MEpoch( Quantity( columnValue, "s" ), MEpoch::UTC ) ;

    // DIRECTION, AZEL, and SCANRATE
    Vector<Double> direction, azel ;
    Vector<Double> scanrate( 2, 0.0 ) ;
    if ( getpt )
      getDirection( direction, azel, scanrate ) ;
    else 
      getSourceDirection( direction, azel, scanrate ) ;

    // INTERVAL
    Double interval = intervalCol.asdouble( recordNo ) ;

    // WEATHER_ID
    uInt wid = 0 ;
    if ( isWeather_ ) 
      wid = getWeatherId() ;

    // put value
    Double t = currentTime.get( "d" ).getValue() ;
    *timeRF = t ;
    *intervalRF = interval ;
    directionRF.define( direction ) ;
    *azimuthRF = (Float)azel[0] ;
    *elevationRF = (Float)azel[1] ;
    scanRateRF.define( scanrate ) ;
    *weatherIdRF = wid ;
  }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual Bool visitRecord(const uInt recordNo,
			   const Int /*observationId*/,
			   const Int /*feedId*/,
			   const Int /*fieldId*/,
			   const Int /*dataDescId*/,
			   const Int /*scanNo*/,
			   const Int /*stateId*/,
			   const Double /*time*/)
  {
    //printf("%u: %d, %d, %d, %d, %d, %d, %f\n", recordNo,
    //observationId, feedId, fieldId, dataDescId, scanNo, stateId, time);

    // SPECTRA and FLAGTRA
    //Matrix<Float> sp;
    //Matrix<uChar> fl;
    spectraAndFlagtra( recordNo, sp, fl ) ;

    // FLAGROW
    Bool flr = flagRowCol.asBool( recordNo ) ;

    // TSYS
    Matrix<Float> tsys ;
    uInt scIdx = getSysCalIndex() ;
    if ( numSysCalRow > 0 ) {
      tsys = sysCalTsysCol( syscalRow[scIdx] ) ;
    }
    else {
      tsys.resize( npol, 1 ) ;
      tsys = 1.0 ;
    }

    // TCAL_ID
    Block<uInt> tcalids( npol, 0 ) ;
    if ( numSysCalRow > 0 ) {
      tcalids = getTcalId( syscalTime[scIdx] ) ;
    }
    else {
      tcalids = getDummyTcalId( spwId ) ;
    }

    // put value
    *cycleNoRF = cycleNo ;
    *flagRowRF = (uInt)flr ;

    // for each polarization component
    for ( Int ipol = 0 ; ipol < npol ; ipol++ ) {
      // put value depending on polarization component
      *polNoRF = polnos[ipol] ;
      *tcalIdRF = tcalids[ipol] ;
      spectraRF.define( sp.row( ipol ) ) ;
      flagtraRF.define( fl.row( ipol ) ) ;
      tsysRF.define( tsys.row( ipol ) ) ;
      
      // commit row
      tablerow.put( rowidx ) ;
      rowidx++ ;
    }

    // increment CYCLENO
    cycleNo++ ;

    return True ;
  }
  virtual void finish()
  {
    BaseMSFillerVisitor::finish();
    //printf("Total: %u\n", count);
    // remove redundant rows
    //cout << "filled " << rowidx << " rows out of " << scantable.nrow() << " rows" << endl ;
    if ( scantable.nrow() > (Int)rowidx ) {
      uInt numRemove = scantable.nrow() - rowidx ;
      //cout << "numRemove = " << numRemove << endl ;
      Vector<uInt> rows( numRemove ) ;
      indgen( rows, rowidx ) ;
      scantable.table().removeRow( rows ) ;
    }
    
    // antenna name and station name
    String antennaName ;
    getScalar( "NAME", (uInt)antennaId, anttab, antennaName ) ;
    String stationName ;
    getScalar( "STATION", (uInt)antennaId, anttab, stationName ) ;

    // update header
    header.nif = ifmap.size() ;
    header.antennaposition = antpos.get( "m" ).getValue() ;
    if ( header.antennaname.empty() || header.antennaname == antennaName )
      header.antennaname = antennaName ;
    else 
      header.antennaname += "//" + antennaName ;
    if ( !stationName.empty() && stationName != antennaName )
      header.antennaname += "@" + stationName ;
    if ( header.fluxunit.empty() || header.fluxunit == "CNTS" )
      header.fluxunit = "K" ;
    header.epoch = "UTC" ;
    header.equinox = 2000.0 ;
    if (header.freqref == "TOPO") {
      header.freqref = "TOPOCENT";
    } else if (header.freqref == "GEO") {
      header.freqref = "GEOCENTR";
    } else if (header.freqref == "BARY") {
      header.freqref = "BARYCENT";
    } else if (header.freqref == "GALACTO") {
      header.freqref = "GALACTOC";
    } else if (header.freqref == "LGROUP") {
      header.freqref = "LOCALGRP";
    } else if (header.freqref == "CMB") {
      header.freqref = "CMBDIPOL";
    } else if (header.freqref == "REST") {
      header.freqref = "SOURCE";
    }
    scantable.setHeader( header ) ;
  }
  void setAntenna( Int id )
  { 
    antennaId = id ; 

    Vector< Quantum<Double> > pos ;
    getArrayQuant( "POSITION", (uInt)antennaId, anttab, pos ) ;
    antpos = MPosition( MVPosition( pos ), MPosition::ITRF ) ; 
    mf.set( antpos ) ;
  } 
  void setPointingTable( const Table &tab, String columnToUse="DIRECTION" )
  {
    // input POINTING table must be
    //  1) selected by antenna
    //  2) sorted by TIME
    ROScalarColumn<Double> tcol( tab, "TIME" ) ;
    ROArrayColumn<Double> dcol( tab, columnToUse ) ;
    tcol.getColumn( pointingTime ) ;
    dcol.getColumn( pointingDirection ) ;
    const TableRecord &rec = dcol.keywordSet() ;
    String pointingRef = rec.asRecord( "MEASINFO" ).asString( "Ref" ) ;
    MDirection::getType( dirType, pointingRef ) ;
    getpt = True ;

    // initialize toj2000 and toazel
    initConvert() ;
  }
  void setWeatherTime( const Vector<Double> &t, const Vector<Double> &it, 
                       const Vector<uInt> &idx )
  {
    isWeather_ = True ;
    weatherTime_ = t ;
    weatherInterval_ = it ;
    weatherIndex_ = idx;
  }
  void setSysCalRecord( const Record &r )
  //void setSysCalRecord( const map< String,Vector<uInt> > &r )
  {
    isSysCal = True ;
    isTcal = True ;
    syscalRecord = r ;
    if ( syscalRecord.nfields() == 0 )
      isTcal = False ;

    const TableDesc &desc = sctab.tableDesc() ;
    uInt nrow = sctab.nrow() ;
    syscalRow.resize( nrow ) ;
    syscalTime.resize( nrow ) ;
    syscalInterval.resize( nrow ) ;
    String tsysCol = "NONE" ;
    Vector<String> tsysCols = stringToVector( "TSYS_SPECTRUM,TSYS" ) ;
    for ( uInt i = 0 ; i < tsysCols.nelements() ; i++ ) {
      if ( tsysCol == "NONE" && desc.isColumn( tsysCols[i] ) )
        tsysCol = tsysCols[i] ;
    }
    sysCalTsysCol.attach( sctab, tsysCol ) ;
  }
  STHeader getHeader() { return header ; }
  uInt getNumBeam() { return nbeam ; }
  uInt getFilledRowNum() { return rowidx ; }
private:
  void initConvert()
  {
    toj2000 = MDirection::Convert( dirType, MDirection::Ref( MDirection::J2000, mf ) ) ;
    toazel = MDirection::Convert( dirType, MDirection::Ref( MDirection::AZELGEO, mf ) ) ;
  }

  void fluxUnit( String &u )
  {
    ROTableColumn col( table, dataColumnName ) ;
    const TableRecord &rec = col.keywordSet() ;
    if ( rec.isDefined( "UNIT" ) )
      u = rec.asString( "UNIT" ) ;
    else if ( rec.isDefined( "QuantumUnits" ) )
      u = rec.asString( "QuantumUnits" ) ;
    if ( u.empty() )
      u = "K" ;
  }
  void processSource( Int sourceId, Int spwId,
                      String &name, MDirection &dir, Vector<Double> &pm,
                      Vector<Double> &rf, Vector<String> &trans, Vector<Double> &vel )
  {
    // find row
    uInt nrow = srctab.nrow() ;
    Int idx = -1 ;
    ROTableRow row( srctab ) ;
    for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
      const TableRecord &r = row.get( irow ) ;
      if ( r.asInt( "SOURCE_ID" ) == sourceId ) {
        Int tmpSpwId = r.asInt( "SPECTRAL_WINDOW_ID" ) ;
        if ( tmpSpwId == spwId || tmpSpwId == -1 ) {
          idx = (Int)irow ;
          break ;
        }
      }
    }

    // fill
    Int numLines = 0 ;
    if ( idx != -1 ) {
      const TableRecord &r = row.get( idx ) ;
      name = r.asString( "NAME" ) ;
      getScalarMeas( "DIRECTION", idx, srctab, dir ) ;
      pm = r.toArrayDouble( "PROPER_MOTION" ) ;
      numLines = r.asInt( "NUM_LINES" ) ;
    }
    else {
      name = "" ;
      pm = Vector<Double>( 2, 0.0 ) ;
      dir = MDirection( Quantum<Double>(0.0,Unit("rad")), Quantum<Double>(0.0,Unit("rad")) ) ;
    }
    if ( !getpt ) {
      String ref = dir.getRefString() ;
      MDirection::getType( dirType, ref ) ;
      
      // initialize toj2000 and toazel
      initConvert() ;
    }

    rf.resize( numLines ) ;
    trans.resize( numLines ) ;
    vel.resize( numLines ) ;
    if ( numLines > 0 ) {
      Block<Bool> isDefined = row.getDefined() ;
      Vector<String> colNames = row.columnNames() ;
      Vector<Int> indexes( 3, -1 ) ;
      Vector<String> cols = stringToVector( "REST_FREQUENCY,TRANSITION,SYSVEL" ) ;
      for ( uInt icol = 0 ; icol < colNames.nelements() ; icol++ ) {
        if ( anyEQ( indexes, -1 ) ) {
          for ( uInt jcol = 0 ; jcol < cols.nelements() ; jcol++ ) {
            if ( colNames[icol] == cols[jcol] )
              indexes[jcol] = icol ;
          }
        }
      }
      if ( indexes[0] != -1 && isDefined[indexes[0]] == True ) {
        Vector< Quantum<Double> > qrf ;
        getArrayQuant( "REST_FREQUENCY", idx, srctab, qrf ) ;
        for ( int i = 0 ; i < numLines ; i++ )
          rf[i] = qrf[i].getValue( "Hz" ) ;
      }
      if ( indexes[1] != -1 && isDefined[indexes[1]] == True ) {
        getArray( "TRANSITION", idx, srctab, trans ) ;
      }
      if ( indexes[2] != -1 && isDefined[indexes[2]] == True ) {
        Vector< Quantum<Double> > qsv ;
        getArrayQuant( "SYSVEL", idx, srctab, qsv ) ;
        for ( int i = 0 ; i < numLines ; i++ )
          vel[i] = qsv[i].getValue( "m/s" ) ;
      }
    }
  }
  void spectralSetup( Int &spwId, MEpoch &me, MPosition &mp, MDirection &md,
                      uInt &freqId, Int &nchan,
                      String &freqref, Double &reffreq, Double &bandwidth )
  {
    // fill
    Int measFreqRef ;
    getScalar( "MEAS_FREQ_REF", spwId, spwtab, measFreqRef ) ;
    MFrequency::Types freqRef = MFrequency::castType( measFreqRef ) ;
    //freqref = MFrequency::showType( freqRef ) ;
    //freqref = "LSRK" ;
    freqref = "TOPO";
    Quantum<Double> q ;
    getScalarQuant( "TOTAL_BANDWIDTH", spwId, spwtab, q ) ;
    bandwidth = q.getValue( "Hz" ) ;
    getScalarQuant( "REF_FREQUENCY", spwId, spwtab, q ) ;
    reffreq = q.getValue( "Hz" ) ;
    Double refpix = 0.5 * ( (Double)nchan-1.0 ) ;
    Int refchan = ( nchan - 1 ) / 2 ;
    Bool even = (Bool)( nchan % 2 == 0 ) ;
    Vector< Quantum<Double> > qa ;
    getArrayQuant( "CHAN_WIDTH", spwId, spwtab, qa ) ;
//     Double increment = qa[refchan].getValue( "Hz" ) ;
    Double increment = abs(qa[refchan].getValue( "Hz" )) ;
    getArrayQuant( "CHAN_FREQ", spwId, spwtab, qa ) ;
    if ( nchan == 1 ) {
      Int netSideband ;
      getScalar( "NET_SIDEBAND", spwId, spwtab, netSideband ) ;
      if ( netSideband == 1 ) increment *= -1.0 ;
    }
    else {
      if ( qa[0].getValue( "Hz" ) > qa[1].getValue( "Hz" ) )
        increment *= -1.0 ;
    }
    Double refval = qa[refchan].getValue( "Hz" ) ;
    if ( even )
      refval = 0.5 * ( refval + qa[refchan+1].getValue( "Hz" ) ) ;
    
    // add new row to FREQUENCIES
    Table ftab = scantable.frequencies().table() ;
    freqId = ftab.nrow() ;
    ftab.addRow() ;
    TableRow row( ftab ) ;
    TableRecord &r = row.record() ;
    RecordFieldPtr<uInt> idRF( r, "ID" ) ;
    *idRF = freqId ;
    RecordFieldPtr<Double> refpixRF( r, "REFPIX" ) ;
    RecordFieldPtr<Double> refvalRF( r, "REFVAL" ) ;
    RecordFieldPtr<Double> incrRF( r, "INCREMENT" ) ;
    *refpixRF = refpix ;
    *refvalRF = refval ;
    *incrRF = increment ;
    row.put( freqId ) ;
  }
  void spectraAndFlagtra( uInt recordNo, Matrix<Float> &sp, Matrix<uChar> &fl )
  {
    Matrix<Bool> b = flagCol( recordNo ) ;
    if ( dataColumnName.compare( "FLOAT_DATA" ) == 0 ) {
      sp = floatDataCol( recordNo ) ;
      convertArray( fl, b ) ;
    }
    else {
      Bool notyet = True ;
      Matrix<Complex> c = dataCol( recordNo ) ;
      for ( Int ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( ( header.poltype == "linear" || header.poltype == "circular" ) 
             && ( polnos[ipol] == 2 || polnos[ipol] == 3 ) ) {
          if ( notyet ) {
            Vector<Float> tmp = ComplexToReal( c.row( ipol ) ) ;
            IPosition start( 1, 0 ) ;
            IPosition end( 1, 2*nchan-1 ) ;
            IPosition inc( 1, 2 ) ;
            if ( polnos[ipol] == 2 ) {
              sp.row( ipol ) = tmp( start, end, inc ) ;
              Vector<Bool> br = b.row( ipol ) ;
              Vector<uChar> flr = fl.row( ipol ) ;
              convertArray( flr, br ) ;
              start = IPosition( 1, 1 ) ;
              Int jpol = ipol+1 ;
              while( polnos[jpol] != 3 && jpol < npol )
                jpol++ ;
              sp.row( jpol ) = tmp( start, end, inc ) ;
              flr.reference( fl.row( jpol ) ) ;
              convertArray( flr, br ) ;
            }
            else if ( polnos[ipol] == 3 ) {
              sp.row( ipol ) = sp.row( ipol ) * (Float)(-1.0) ;
              Int jpol = ipol+1 ;
              while( polnos[jpol] != 2 && jpol < npol )
                jpol++ ;
              Vector<Bool> br = b.row( ipol ) ;
              Vector<uChar> flr = fl.row( jpol ) ;
              sp.row( jpol ) = tmp( start, end, inc ) ;
              convertArray( flr, br ) ;
              start = IPosition( 1, 1 ) ;
              sp.row( ipol ) = tmp( start, end, inc ) * (Float)(-1.0) ;
              flr.reference( fl.row( ipol ) ) ;
              convertArray( flr, br ) ;
            }
            notyet = False ;
          }
        }
        else {
          Vector<Float> tmp = ComplexToReal( c.row( ipol ) ) ;
          IPosition start( 1, 0 ) ;
          IPosition end( 1, 2*nchan-1 ) ;
          IPosition inc( 1, 2 ) ;
          sp.row( ipol ) = tmp( start, end, inc ) ;
          Vector<Bool> br = b.row( ipol ) ;
          Vector<uChar> flr = fl.row( ipol ) ;
          convertArray( flr, br ) ;
        }
      }
    }
  }
  uInt binarySearch( Vector<Double> &timeList, Double target )
  {
    Int low = 0 ;
    Int high = timeList.nelements() ;
    uInt idx = 0 ;
    
    while ( low <= high ) {
      idx = (Int)( 0.5 * ( low + high ) ) ;
      Double t = timeList[idx] ;
      if ( t < target ) 
        low = idx + 1 ;
      else if ( t > target )
        high = idx - 1 ;
      else {
        return idx ;
      }
    }
    
    idx = max( 0, min( low, high ) ) ;
    return idx ;
  }
  void getDirection( Vector<Double> &dir, Vector<Double> &azel, Vector<Double> &srate )
  {
    // @todo At the moment, do binary search every time
    //       if this is bottleneck, frequency of binary search must be reduced
    Double t = currentTime.get( "s" ).getValue() ;
    uInt idx = min( binarySearch( pointingTime, t ), pointingTime.nelements()-1 ) ;
    Matrix<Double> d ;
    if ( pointingTime[idx] == t )
      d = pointingDirection.xyPlane( idx ) ;
    else if ( pointingTime[idx] < t ) {
      if ( idx == pointingTime.nelements()-1 ) 
        d = pointingDirection.xyPlane( idx ) ;
      else 
        d = interp( pointingTime[idx], pointingTime[idx+1], t,
                    pointingDirection.xyPlane( idx ), pointingDirection.xyPlane( idx+1 ) ) ;
    }
    else {
      if ( idx == 0 )
        d = pointingDirection.xyPlane( idx ) ;
      else 
        d = interp( pointingTime[idx-1], pointingTime[idx], t,
                    pointingDirection.xyPlane( idx-1 ), pointingDirection.xyPlane( idx ) ) ;
    }
    mf.set( currentTime ) ;
    Quantum< Vector<Double> > tmp( d.column( 0 ), Unit( "rad" ) ) ;
    if ( dirType != MDirection::J2000 ) {
      dir = toj2000( tmp ).getAngle( "rad" ).getValue() ;
    }
    else {
      dir = d.column( 0 ) ;
    }
    if ( dirType != MDirection::AZELGEO ) {
      azel = toazel( tmp ).getAngle( "rad" ).getValue() ;
    }
    else {
      azel = d.column( 0 ) ;
    }
    if ( d.ncolumn() > 1 )
      srate = d.column( 1 ) ;
  }
  void getSourceDirection( Vector<Double> &dir, Vector<Double> &azel, Vector<Double> &/*srate*/ )
  {
    dir = sourceDir.getAngle( "rad" ).getValue() ;
    mf.set( currentTime ) ;
    azel = toazel( Quantum< Vector<Double> >( dir, Unit("rad") ) ).getAngle( "rad" ).getValue() ;
    if ( dirType != MDirection::J2000 ) {
      dir = toj2000( Quantum< Vector<Double> >( dir, Unit("rad") ) ).getAngle( "rad" ).getValue() ;
    }
  }
  String detectSeparator( String &s )
  {
    String tmp = s.substr( 0, s.find_first_of( "," ) ) ;
    Char *separators[] = { ":", "#", ".", "_" } ;
    uInt nsep = 4 ;
    for ( uInt i = 0 ; i < nsep ; i++ ) {
      if ( tmp.find( separators[i] ) != String::npos )
        return separators[i] ;
    }
    return "" ;
  }
  Int getSrcType( Int stateId )
  {
    // get values
    Bool sig ;
    getScalar( "SIG", stateId, statetab, sig ) ;
    Bool ref ;
    getScalar( "REF", stateId, statetab, ref ) ;
    Double cal ;
    getScalar( "CAL", stateId, statetab, cal ) ;
    String obsmode ;
    getScalar( "OBS_MODE", stateId, statetab, obsmode ) ;
    String sep = detectSeparator( obsmode ) ;
    
    Int srcType = SrcType::NOTYPE ;
    if ( sep == ":" )
      srcTypeGBT( srcType, sep, obsmode, sig, ref, cal ) ;
    else if ( sep == "." || sep == "#" ) 
      srcTypeALMA( srcType, sep, obsmode ) ;
    else if ( sep == "_" )
      srcTypeOldALMA( srcType, sep, obsmode, sig, ref ) ;
    else 
      srcTypeDefault( srcType, sig, ref ) ;

    return srcType ;
  }
  void srcTypeDefault( Int &st, Bool &sig, Bool &ref )
  {
    if ( sig ) st = SrcType::SIG ;
    else if ( ref ) st = SrcType::REF ;
  }
  void srcTypeGBT( Int &st, String &sep, String &mode, Bool &sig, Bool &ref, Double &cal )
  {
    Int epos = mode.find_first_of( sep ) ;
    Int nextpos = mode.find_first_of( sep, epos+1 ) ;
    String m1 = mode.substr( 0, epos ) ;
    String m2 = mode.substr( epos+1, nextpos-epos-1 ) ;
    if ( m1 == "Nod" ) {
      st = SrcType::NOD ;
    }
    else if ( m1 == "OffOn" ) {
      if ( m2 == "PSWITCHON" ) st = SrcType::PSON ;
      if ( m2 == "PSWITCHOFF" ) st = SrcType::PSOFF ;
    }
    else {
      if ( m2 == "FSWITCH" ) {
        if ( sig ) st = SrcType::FSON ;
        else if ( ref ) st = SrcType::FSOFF ;
      }
    }
    if ( cal > 0.0 ) {
      if ( st == SrcType::NOD )
        st = SrcType::NODCAL ;
      else if ( st == SrcType::PSON ) 
        st = SrcType::PONCAL ;
      else if ( st == SrcType::PSOFF )
        st = SrcType::POFFCAL ;
      else if ( st == SrcType::FSON ) 
        st = SrcType::FONCAL ;
      else if ( st == SrcType::FSOFF ) 
        st = SrcType::FOFFCAL ;
      else
        st = SrcType::CAL ;
    }
  }
  void srcTypeALMA( Int &st, String &sep, String &mode )
  {
    Int epos = mode.find_first_of( "," ) ;
    String first = mode.substr( 0, epos ) ;
    epos = first.find_first_of( sep ) ;
    Int nextpos = first.find_first_of( sep, epos+1 ) ;
    String m1 = first.substr( 0, epos ) ;
    String m2 = first.substr( epos+1, nextpos-epos-1 ) ;
    if ( m1.find( "CALIBRATE_" ) == 0 ) {
      if ( m2.find( "ON_SOURCE" ) == 0 )
        st = SrcType::PONCAL ;
      else if ( m2.find( "OFF_SOURCE" ) == 0 )
        st = SrcType::POFFCAL ;
    }
    else if ( m1.find( "OBSERVE_TARGET" ) == 0 ) {
      if ( m2.find( "ON_SOURCE" ) == 0 ) 
        st = SrcType::PSON ;
      else if ( m2.find( "OFF_SOURCE" ) == 0 )
        st = SrcType::PSOFF ;
    }
  }
  void srcTypeOldALMA( Int &st, String &sep, String &mode, Bool &sig, Bool &ref )
  {
    Int epos = mode.find_first_of( "," ) ;
    String first = mode.substr( 0, epos ) ;
    string substr[4] ;
    int numSubstr = split( first, substr, 4, sep ) ;
    String m1( substr[0] ) ;
    String m2( substr[2] ) ;
    if ( numSubstr == 4 ) {
      if ( m1.find( "CALIBRATE" ) == 0 ) {
        if ( m2.find( "ON" ) == 0 ) 
          st = SrcType::PONCAL ;
        else if ( m2.find( "OFF" ) == 0 )
          st = SrcType::POFFCAL ;
      }
      else if ( m1.find( "OBSERVE" ) == 0 ) {
        if ( m2.find( "ON" ) == 0 ) 
          st = SrcType::PSON ;
        else if ( m2.find( "OFF" ) == 0 )
          st = SrcType::PSOFF ;
      }
    }
    else {
      if ( sig ) st = SrcType::SIG ;
      else if ( ref ) st = SrcType::REF ;
    }
  }
  Block<uInt> getPolNos( Vector<Int> &corr )
  {
    Block<uInt> polnos( npol ) ;
    for ( Int ipol = 0 ; ipol < npol ; ipol++ ) {
      if ( corr[ipol] == Stokes::I || corr[ipol] == Stokes::RR || corr[ipol] == Stokes::XX )
        polnos[ipol] = 0 ;
      else if ( corr[ipol] == Stokes::Q || corr[ipol] == Stokes::LL || corr[ipol] == Stokes::YY )
        polnos[ipol] = 1 ;
      else if ( corr[ipol] == Stokes::U || corr[ipol] == Stokes::RL || corr[ipol] == Stokes::XY )
        polnos[ipol] = 2 ;
      else if ( corr[ipol] == Stokes::V || corr[ipol] == Stokes::LR || corr[ipol] == Stokes::YX )
        polnos[ipol] = 3 ;
    }
    return polnos ;
  }
  String getPolType( Int &corr )
  {
    String poltype = "" ;
    if ( corr == Stokes::I || corr == Stokes::Q || corr == Stokes::U || corr == Stokes::V )
      poltype = "stokes" ;
    else if ( corr == Stokes::XX || corr == Stokes::YY || corr == Stokes::XY || corr == Stokes::YX ) 
      poltype = "linear" ;
    else if ( corr == Stokes::RR || corr == Stokes::LL || corr == Stokes::RL || corr == Stokes::LR ) 
      poltype = "circular" ;
    else if ( corr == Stokes::Plinear || corr == Stokes::Pangle )
      poltype = "linpol" ;
    return poltype ;    
  }
  uInt getWeatherId()
  {
    // if only one row, return 0
    if ( weatherTime_.nelements() == 1 )
      return 0 ;

    // @todo At the moment, do binary search every time
    //       if this is bottleneck, frequency of binary search must be reduced
    Double t = currentTime.get( "s" ).getValue() ;
    uInt idx = min( binarySearch( weatherTime_, t ), weatherTime_.nelements()-1 ) ;
    if ( weatherTime_[idx] < t ) {
      if ( idx != weatherTime_.nelements()-1 ) {
        if ( weatherTime_[idx+1] - t < 0.5 * weatherInterval_[idx+1] )
          idx++ ;
      }
    }
    else if ( weatherTime_[idx] > t ) {
      if ( idx != 0 ) {
        if ( weatherTime_[idx] - t > 0.5 * weatherInterval_[idx] )
          idx-- ;
      }
    }
    return weatherIndex_[idx] ;
  }
  void processSysCal( Int &spwId )
  {
    // get feedId from row
    Int feedId = (Int)tablerow.record().asuInt( "BEAMNO" ) ;

    uInt nrow = sctab.nrow() ;
    ROScalarColumn<Int> col( sctab, "ANTENNA_ID" ) ;
    Vector<Int> aids = col.getColumn() ;
    col.attach( sctab, "FEED_ID" ) ;
    Vector<Int> fids = col.getColumn() ;
    col.attach( sctab, "SPECTRAL_WINDOW_ID" ) ;
    Vector<Int> sids = col.getColumn() ;
    ROScalarColumn<Double> timeCol( sctab, "TIME" ) ;
    ROScalarColumn<Double> intCol( sctab, "INTERVAL" ) ;
    for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
      if ( aids[irow] == antennaId 
           && fids[irow] == feedId 
           && sids[irow] == spwId ) {
        syscalRow[numSysCalRow] = irow ;
        syscalTime[numSysCalRow] = timeCol( irow ) ;
        syscalInterval[numSysCalRow] = intCol( irow ) ;
        numSysCalRow++ ;
      }
    }
  }
  uInt getSysCalIndex()
  {
    // if only one row, return 0
    if ( numSysCalRow == 1 || !isSysCal )
      return 0 ;

    // @todo At the moment, do binary search every time
    //       if this is bottleneck, frequency of binary search must be reduced
    Double t = currentTime.get( "s" ).getValue() ;
    Vector<Double> tslice  = syscalTime( Slice(0, numSysCalRow) ) ;
    uInt idx = min( binarySearch( tslice, t ), numSysCalRow-1 ) ;
    if ( syscalTime[idx] < t ) {
      if ( idx != numSysCalRow-1 ) {
        if ( syscalTime[idx+1] - t < 0.5 * syscalInterval[idx+1] )
          idx++ ;
      }
    }
    else if ( syscalTime[idx] > t ) {
      if ( idx != 0 ) {
        if ( syscalTime[idx] - t > 0.5 * syscalInterval[idx] )
          idx-- ;
      }
    }
    return idx ;    
  }
  Block<uInt> getTcalId( Double &t )
  {
    // return 0 if no SysCal table
    if ( !isSysCal or !isTcal ) {
      return Block<uInt>( 4, 0 ) ;
    }
      
    // get feedId from row
    Int feedId = (Int)tablerow.record().asuInt( "BEAMNO" ) ;

    // key
    String key = keyTcal( feedId, spwId, t ) ;
    
    // retrieve ids
    Vector<uInt> ids = syscalRecord.asArrayuInt( key ) ;
    //Vector<uInt> ids = syscalRecord[key] ;
    uInt np = ids[1] - ids[0] + 1 ;
    Block<uInt> tcalids( np ) ;
    if ( np > 0 ) {
      tcalids[0] = ids[0] ;
      if ( np > 1 ) {
        tcalids[1] = ids[1] ;
        for ( uInt ip = 2 ; ip < np ; ip++ )
          tcalids[ip] = ids[0] + ip - 1 ;
      }
    }
    return tcalids ;
  }
  Block<uInt> getDummyTcalId( Int spwId )
  {
    Block<uInt> idList(4, 0);
    uInt nfields = syscalRecord.nfields();
    Int idx = -1;
    for (uInt i = 0; i< nfields ; i++ ) {
      String spw = "SPW" + String::toString(spwId);
      if (syscalRecord.name(i).find(spw) != String::npos) {
        idx = i;
        break;
      }
    }
    if ( idx > -1) {
      Vector<uInt> tmp = syscalRecord.asArrayuInt(idx);
      for (uInt j = 0 ; j < 4 ; j++) {
        idList[j] = tmp[0];
      }
    }
    return idList;
  }
  uInt maxNumPol()
  {
    ROScalarColumn<Int> numCorrCol( poltab, "NUM_CORR" ) ;
    return max( numCorrCol.getColumn() ) ;
  }

  Scantable &scantable;
  Int antennaId;
  uInt rowidx;
  String dataColumnName;
  TableRow tablerow;
  STHeader header;
  Vector<Int> feedEntry;
  uInt nbeam;
  Int npol;
  Int nchan;
  Int sourceId;
  Int polId;
  Int spwId;
  uInt cycleNo;
  MDirection sourceDir;
  MPosition antpos;
  MEpoch currentTime;
  MEpoch obsEpoch;
  MeasFrame mf;
  MDirection::Convert toj2000;
  MDirection::Convert toazel;
  map<Int,uInt> ifmap;
  Block<uInt> polnos;
  Bool getpt;
  Vector<Double> pointingTime;
  Cube<Double> pointingDirection;
  MDirection::Types dirType;
  Bool isWeather_;
  Vector<Double> weatherTime_;
  Vector<Double> weatherInterval_;
  Vector<uInt> weatherIndex_;
  Bool isSysCal;
  Bool isTcal;
  Record syscalRecord;
  //map< String,Vector<uInt> > syscalRecord;
  uInt numSysCalRow ;
  Vector<uInt> syscalRow;
  Vector<Double> syscalTime;
  Vector<Double> syscalInterval;
  //String tsysCol;
  //String tcalCol;

  // MS subtables
  Table obstab;
  Table sctab;
  Table spwtab;
  Table statetab;
  Table ddtab;
  Table poltab;
  Table fieldtab;
  Table anttab;
  Table srctab;
  Matrix<Float> sp;
  Matrix<uChar> fl;

  // MS MAIN columns
  ROTableColumn intervalCol;
  ROTableColumn flagRowCol;
  ROArrayColumn<Float> floatDataCol;
  ROArrayColumn<Complex> dataCol;
  ROArrayColumn<Bool> flagCol;

  // MS SYSCAL columns 
  ROArrayColumn<Float> sysCalTsysCol;

  // Scantable MAIN columns
  RecordFieldPtr<Double> timeRF,intervalRF,sourceVelocityRF;
  RecordFieldPtr< Vector<Double> > directionRF,scanRateRF,
    sourceProperMotionRF,sourceDirectionRF;
  RecordFieldPtr<Float> azimuthRF,elevationRF;
  RecordFieldPtr<uInt> weatherIdRF,cycleNoRF,flagRowRF,polNoRF,tcalIdRF,
    ifNoRF,freqIdRF,moleculeIdRF,beamNoRF,focusIdRF,scanNoRF;
  RecordFieldPtr< Vector<Float> > spectraRF,tsysRF;
  RecordFieldPtr< Vector<uChar> > flagtraRF;
  RecordFieldPtr<String> sourceNameRF,fieldNameRF;
  RecordFieldPtr<Int> sourceTypeRF;
};

class BaseTcalVisitor: public TableVisitor {
  uInt lastRecordNo ;
  Int lastAntennaId ;
  Int lastFeedId ;
  Int lastSpwId ;
  Double lastTime ;
protected:
  const Table &table;
  uInt count;
public:
  BaseTcalVisitor(const Table &table)
   : table(table)
  {
    count = 0;
  }
  
  virtual void enterAntennaId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveAntennaId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterFeedId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveFeedId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterSpwId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveSpwId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) { }

  virtual Bool visitRecord(const uInt /*recordNo*/,
                           const Int /*antennaId*/,
			   const Int /*feedId*/,
			   const Int /*spwId*/,
			   const Double /*time*/) { return True ; }

  virtual Bool visit(Bool isFirst, const uInt recordNo,
		     const uInt nCols, void const *const colValues[]) {
    Int antennaId, feedId, spwId; 
    Double time;
    { // prologue
      uInt i = 0;
      {
        const Int *col = (const Int *)colValues[i++];
        antennaId = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	feedId = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	spwId = col[recordNo];
      }
      {
	const Double *col = (const Double *)colValues[i++];
	time = col[recordNo];
      }
      assert(nCols == i);
    }

    if (isFirst) {
      enterAntennaId(recordNo, antennaId);
      enterFeedId(recordNo, feedId);
      enterSpwId(recordNo, spwId);
      enterTime(recordNo, time);
    } else {
      if ( lastAntennaId != antennaId ) {
	leaveTime(lastRecordNo, lastTime);
	leaveSpwId(lastRecordNo, lastSpwId);
	leaveFeedId(lastRecordNo, lastFeedId);
        leaveAntennaId(lastRecordNo, lastAntennaId);

        enterAntennaId(recordNo, antennaId);
	enterFeedId(recordNo, feedId);
	enterSpwId(recordNo, spwId);
	enterTime(recordNo, time);
      }        
      else if (lastFeedId != feedId) {
	leaveTime(lastRecordNo, lastTime);
	leaveSpwId(lastRecordNo, lastSpwId);
	leaveFeedId(lastRecordNo, lastFeedId);

	enterFeedId(recordNo, feedId);
	enterSpwId(recordNo, spwId);
	enterTime(recordNo, time);
      } else if (lastSpwId != spwId) {
	leaveTime(lastRecordNo, lastTime);
	leaveSpwId(lastRecordNo, lastSpwId);

	enterSpwId(recordNo, spwId);
	enterTime(recordNo, time);
      } else if (lastTime != time) {
	leaveTime(lastRecordNo, lastTime);
	enterTime(recordNo, time);
      }
    }
    count++;
    Bool result = visitRecord(recordNo, antennaId, feedId, spwId, time);

    { // epilogue
      lastRecordNo = recordNo;

      lastAntennaId = antennaId;
      lastFeedId = feedId;
      lastSpwId = spwId;
      lastTime = time;
    }
    return result ;
  }

  virtual void finish() {
    if (count > 0) {
      leaveTime(lastRecordNo, lastTime);
      leaveSpwId(lastRecordNo, lastSpwId);
      leaveFeedId(lastRecordNo, lastFeedId);
      leaveAntennaId(lastRecordNo, lastAntennaId);
    }
  }
};

class TcalVisitor: public BaseTcalVisitor, public MSFillerUtils {
public:
  TcalVisitor(const Table &table, Table &tcaltab, Record &r, Int aid )
  //TcalVisitor(const Table &table, Table &tcaltab, map< String,Vector<uInt> > &r, Int aid )
    : BaseTcalVisitor( table ),
      tcal(tcaltab),
      rec(r),
      antenna(aid) 
  { 
    process = False ;
    rowidx = 0 ;

    // attach to SYSCAL columns
    timeCol.attach( table, "TIME" ) ;

    // add rows
    uInt addrow = table.nrow() * 4 ;
    tcal.addRow( addrow ) ;

    // attach to TCAL columns
    row = TableRow( tcal ) ;
    TableRecord &trec = row.record() ;
    idRF.attachToRecord( trec, "ID" ) ;
    timeRF.attachToRecord( trec, "TIME" ) ;
    tcalRF.attachToRecord( trec, "TCAL" ) ;
  }

  virtual void enterAntennaId(const uInt /*recordNo*/, Int columnValue) {
    if ( columnValue == antenna )
      process = True ;
  }
  virtual void leaveAntennaId(const uInt /*recordNo*/, Int /*columnValue*/) {
    process = False ;
  }
  virtual void enterFeedId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveFeedId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterSpwId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveSpwId(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterTime(const uInt recordNo, Double /*columnValue*/) {
    qtime = timeCol( recordNo ) ;
  }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual Bool visitRecord(const uInt recordNo,
                           const Int /*antennaId*/,
			   const Int feedId,
			   const Int spwId,
			   const Double /*time*/) 
  { 
    //cout << "(" << recordNo << "," << antennaId << "," << feedId << "," << spwId << ")" << endl ;
    if ( process ) {
      String sTime = MVTime( qtime ).string( MVTime::YMD ) ;
      *timeRF = sTime ;
      uInt oldidx = rowidx ;
      Matrix<Float> subtcal = tcalCol( recordNo ) ;
      Vector<uInt> idminmax( 2 ) ;
      for ( uInt ipol = 0 ; ipol < subtcal.nrow() ; ipol++ ) {
        *idRF = rowidx ;
        tcalRF.define( subtcal.row( ipol ) ) ;
        
        // commit row
        row.put( rowidx ) ;
        rowidx++ ;
      }
      
      idminmax[0] = oldidx ;
      idminmax[1] = rowidx - 1 ;
      
      String key = keyTcal( feedId, spwId, sTime ) ;
      rec.define( key, idminmax ) ;
      //rec[key] = idminmax ;
    }
    return True ; 
  }
  virtual void finish() 
  {
    BaseTcalVisitor::finish() ;

    if ( tcal.nrow() > rowidx ) {
      uInt numRemove = tcal.nrow() - rowidx ;
      //cout << "numRemove = " << numRemove << endl ;
      Vector<uInt> rows( numRemove ) ;
      indgen( rows, rowidx ) ;
      tcal.removeRow( rows ) ;
    }

  }
  void setTcalColumn( String &col ) 
  {
    //colName = col ;
    tcalCol.attach( table, col ) ;
  }
private:
  Table &tcal;
  Record &rec;
  //map< String,Vector<uInt> > &rec;
  Int antenna;
  uInt rowidx;
  Bool process;
  Quantum<Double> qtime;
  TableRow row;
  String colName;

  // MS SYSCAL columns
  ROScalarQuantColumn<Double> timeCol;
  ROArrayColumn<Float> tcalCol;

  // TCAL columns
  RecordFieldPtr<uInt> idRF;
  RecordFieldPtr<String> timeRF;
  RecordFieldPtr< Vector<Float> > tcalRF;
};

MSFiller::MSFiller( casa::CountedPtr<Scantable> stable )
  : table_( stable ),
    tablename_( "" ),
    antenna_( -1 ),
    antennaStr_(""),
    getPt_( True ),
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
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSFiller::open() startsec=" << startSec << LogIO::POST ;
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
  tablename_ = tmpMS->tableName() ;
  if ( antenna_ == -1 && antennaStr_.size() > 0 ) {
    MSAntennaIndex msAntIdx( tmpMS->antenna() ) ;
    Vector<Int> id = msAntIdx.matchAntennaName( antennaStr_ ) ;
    if ( id.size() > 0 )
      antenna_ = id[0] ;
    else {
      delete tmpMS ;
      //throw( AipsError( "Antenna " + antennaStr_ + " doesn't exist." ) ) ;
      os_ << LogIO::SEVERE << "Antenna " << antennaStr_ << " doesn't exist." << LogIO::POST ;
      return False ;
    }
  }

  os_ << "Parsing MS options" << endl ;
  os_ << "   getPt = " << (getPt_ ? "True" : "False") << endl ;
  os_ << "   antenna = " << antenna_ << endl ;
  os_ << "   antennaStr = " << antennaStr_ << LogIO::POST;

  mstable_ = MeasurementSet( (*tmpMS)( tmpMS->col("ANTENNA1") == antenna_ 
                                       && tmpMS->col("ANTENNA1") == tmpMS->col("ANTENNA2") ) ) ;

  delete tmpMS ;

  // check which data column exists
  isFloatData_ = mstable_.tableDesc().isColumn( "FLOAT_DATA" ) ;
  isData_ = mstable_.tableDesc().isColumn( "DATA" ) ;

  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSFiller::open() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
  return true ;
}

void MSFiller::fill()
{
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSFiller::fill() startSec=" << startSec << LogIO::POST ;

  os_.origin( LogOrigin( "MSFiller", "fill()", WHERE ) ) ;

  // Initialize header
  STHeader sdh ;  
  initHeader( sdh ) ;
  table_->setHeader( sdh ) ;
 
  // check if optional table exists
  const TableRecord &msrec = mstable_.keywordSet() ;
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

  // column name for Tsys and Tcal
  if ( isSysCal_ ) {
    const MSSysCal &caltab = mstable_.sysCal() ;
    if ( !caltab.tableDesc().isColumn( colTcal_ ) ) {
      colTcal_ = "TCAL" ;
      if ( !caltab.tableDesc().isColumn( colTcal_ ) ) 
        colTcal_ = "NONE" ;
    }
    if ( !caltab.tableDesc().isColumn( colTsys_ ) ) {
      colTsys_ = "TSYS" ;
      if ( !caltab.tableDesc().isColumn( colTcal_ ) ) 
        colTsys_ = "NONE" ;
    }
  }
  else {
    colTcal_ = "NONE" ;
    colTsys_ = "NONE" ;
  }

  // Access to MS subtables
  //MSField &fieldtab = mstable_.field() ;
  //MSPolarization &poltab = mstable_.polarization() ;
  //MSDataDescription &ddtab = mstable_.dataDescription() ;
  //MSObservation &obstab = mstable_.observation() ;
  //MSSource &srctab = mstable_.source() ;
  //MSSpectralWindow &spwtab = mstable_.spectralWindow() ;
  //MSSysCal &caltab = mstable_.sysCal() ; 
  MSPointing &pointtab = mstable_.pointing() ;
  //MSState &stattab = mstable_.state() ;
  //MSAntenna &anttab = mstable_.antenna() ;

  // SUBTABLES: FREQUENCIES
  //string freqFrame = getFrame() ;
  string baseFrame = frameFromSpwTable() ;
  table_->frequencies().setFrame( baseFrame ) ;
  table_->frequencies().setFrame( baseFrame, True ) ;

  // SUBTABLES: WEATHER
  fillWeather() ;

  // SUBTABLES: FOCUS
  fillFocus() ;

  // SUBTABLES: TCAL
  fillTcal() ;

  // SUBTABLES: FIT
  //fillFit() ;

  // SUBTABLES: HISTORY
  //fillHistory() ;

  /***
   * Start iteration using TableVisitor
   ***/
  Table stab = table_->table() ;
  {
    static const char *cols[] = {
      "OBSERVATION_ID", "FEED1", "FIELD_ID", "DATA_DESC_ID", "SCAN_NUMBER", 
      "STATE_ID", "TIME",
      NULL
    };
    static const TypeManagerImpl<Int> tmInt;
    static const TypeManagerImpl<Double> tmDouble;
    static const TypeManager *const tms[] = {
      &tmInt, &tmInt, &tmInt, &tmInt, &tmInt, &tmInt, &tmDouble, NULL
    };
    //double t0 = mathutil::gettimeofday_sec() ;
    MSFillerVisitor myVisitor(mstable_, *table_ );
    //double t1 = mathutil::gettimeofday_sec() ;
    //cout << "MSFillerVisitor(): elapsed time " << t1-t0 << " sec" << endl ;
    myVisitor.setAntenna( antenna_ ) ;
    //myVisitor.setHeader( sdh ) ;
    if ( getPt_ ) {
      Table ptsel = pointtab( pointtab.col("ANTENNA_ID")==antenna_ ).sort( "TIME" ) ;
      myVisitor.setPointingTable( ptsel ) ;
    }
    if ( isWeather_ )
      myVisitor.setWeatherTime( mwTime_, mwInterval_, mwIndex_ ) ;
    if ( isSysCal_ ) 
      myVisitor.setSysCalRecord( tcalrec_ ) ;
    
    //double t2 = mathutil::gettimeofday_sec() ;
    traverseTable(mstable_, cols, tms, &myVisitor);
    //double t3 = mathutil::gettimeofday_sec() ;
    //cout << "traverseTable(): elapsed time " << t3-t2 << " sec" << endl ;
    
    sdh = myVisitor.getHeader() ;
  }
  /***
   * End iteration using TableVisitor
   ***/

  // set header
  //sdh = myVisitor.getHeader() ;
  //table_->setHeader( sdh ) ;

  // save path to POINTING table
  // 2011/07/06 TN
  // Path to POINTING table in original MS will not be written
  // if getPt_ is True
  Path datapath( tablename_ ) ;
  if ( !getPt_ ) {
    String pTabName = datapath.absoluteName() + "/POINTING" ;
    stab.rwKeywordSet().define( "POINTING", pTabName ) ;
  }

  // for GBT
  if ( sdh.antennaname.contains( "GBT" ) ) {
    String goTabName = datapath.absoluteName() + "/GBT_GO" ;
    stab.rwKeywordSet().define( "GBT_GO", goTabName ) ;
  }

  // for MS created from ASDM
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

  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSFiller::fill() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::close()
{
  //tablesel_.closeSubTables() ;
  mstable_.closeSubTables() ;
  //tablesel_.unlock() ;
  mstable_.unlock() ;
}

void MSFiller::fillWeather()
{
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSFiller::fillWeather() startSec=" << startSec << LogIO::POST ;

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

  Bool stationInfoExists = mWeatherSel.tableDesc().isColumn( "NS_WX_STATION_ID" ) ;
  Int stationId = -1 ;
  if ( stationInfoExists ) {
    // determine which station is closer
    ROScalarColumn<Int> stationCol( mWeatherSel, "NS_WX_STATION_ID" ) ;
    ROArrayColumn<Double> stationPosCol( mWeatherSel, "NS_WX_STATION_POSITION" ) ;
    Vector<Int> stationIds = stationCol.getColumn() ;
    Vector<Int> stationIdList( 0 ) ;
    Matrix<Double> stationPosList( 0, 3, 0.0 ) ;
    uInt numStation = 0 ;
    for ( uInt i = 0 ; i < stationIds.size() ; i++ ) {
      if ( !anyEQ( stationIdList, stationIds[i] ) ) {
        numStation++ ;
        stationIdList.resize( numStation, True ) ;
        stationIdList[numStation-1] = stationIds[i] ;
        stationPosList.resize( numStation, 3, True ) ;
        stationPosList.row( numStation-1 ) = stationPosCol( i ) ;
      }
    }
    //os_ << "staionIdList = " << stationIdList << endl ;
    Table mAntenna = mstable_.antenna() ;
    ROArrayColumn<Double> antposCol( mAntenna, "POSITION" ) ;
    Vector<Double> antpos = antposCol( antenna_ ) ;
    Double minDiff = -1.0 ;
    for ( uInt i = 0 ; i < stationIdList.size() ; i++ ) {
      Double diff = sum( square( antpos - stationPosList.row( i ) ) ) ;
      if ( minDiff < 0.0 || minDiff > diff ) {
        minDiff = diff ;
        stationId = stationIdList[i] ;
      }
    }
  }
  //os_ << "stationId = " << stationId << endl ;
  
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
  Vector<Double> mwTime = tCol.getColumn() ;
  if ( tUnit == "d" ) 
    mwTime *= 86400.0 ;
  tqCol.attach( mWeatherSel, "INTERVAL" ) ;
  tCol.attach( mWeatherSel, "INTERVAL" ) ;
  String iUnit = tqCol.getUnits() ;
  Vector<Double> mwInterval = tCol.getColumn() ;
  if ( iUnit == "d" ) 
    mwInterval *= 86400.0 ; 

  if ( stationId > 0 ) {
    ROScalarColumn<Int> stationCol( mWeatherSel, "NS_WX_STATION_ID" ) ;
    Vector<Int> stationVec = stationCol.getColumn() ;
    uInt wsnrow = ntrue( stationVec == stationId ) ;
    mwTime_.resize( wsnrow ) ;
    mwInterval_.resize( wsnrow ) ;
    mwIndex_.resize( wsnrow ) ;
    uInt wsidx = 0 ;
    for ( uInt irow = 0 ; irow < wnrow ; irow++ ) {
      if ( stationId == stationVec[irow] ) {
        mwTime_[wsidx] = mwTime[irow] ;
        mwInterval_[wsidx] = mwInterval[irow] ;
        mwIndex_[wsidx] = irow ;
        wsidx++ ;
      }
    }
  }
  else {
    mwTime_ = mwTime ;
    mwInterval_ = mwInterval ;
    mwIndex_.resize( mwTime_.size() ) ;
    indgen( mwIndex_ ) ;
  }
  //os_ << "mwTime[0] = " << mwTime_[0] << " mwInterval[0] = " << mwInterval_[0] << LogIO::POST ; 
  //os_ << "mwIndex_=" << mwIndex_ << LogIO::POST;
  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSFiller::fillWeather() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::fillFocus()
{
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSFiller::fillFocus() startSec=" << startSec << LogIO::POST ;
  // tentative
  table_->focus().addEntry( 0.0, 0.0, 0.0, 0.0 ) ;
  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSFiller::fillFocus() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::fillTcal()
{
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSFiller::fillTcal() startSec=" << startSec << LogIO::POST ;

  if ( !isSysCal_ ) {
    // add dummy row
    os_ << "No SYSCAL rows" << LogIO::POST ;
    table_->tcal().table().addRow(1,True) ;
    Vector<Float> defaultTcal( 1, 1.0 ) ;
    ArrayColumn<Float> tcalCol( table_->tcal().table(), "TCAL" ) ;
    tcalCol.put( 0, defaultTcal ) ;
    return ;
  }

  if ( colTcal_ == "NONE" ) {
    // add dummy row
    os_ << "No TCAL column" << LogIO::POST ;
    table_->tcal().table().addRow(1,True) ;
    Vector<Float> defaultTcal( 1, 1.0 ) ;
    ArrayColumn<Float> tcalCol( table_->tcal().table(), "TCAL" ) ;
    tcalCol.put( 0, defaultTcal ) ;
    return ;
  }

  Table &sctab = mstable_.sysCal() ;
  if ( sctab.nrow() == 0 ) {
    os_ << "No SYSCAL rows" << LogIO::POST ;
    return ;
  } 
  ROScalarColumn<Int> antCol( sctab, "ANTENNA_ID" ) ;
  Vector<Int> ant = antCol.getColumn() ;
  if ( allNE( ant, antenna_ ) ) {
    os_ << "No SYSCAL rows" << LogIO::POST ;
    return ;
  } 
  ROTableColumn tcalCol( sctab, colTcal_ ) ;
  Bool notDefined = False ;
  for ( uInt irow = 0 ; irow < sctab.nrow() ; irow++ ) {
    if ( ant[irow] == antenna_ && !tcalCol.isDefined( irow ) ) {
      notDefined = True ;
      break ;
    }
  }
  if ( notDefined ) {
    os_ << "No TCAL value" << LogIO::POST ;
    table_->tcal().table().addRow(1,True) ;
    Vector<Float> defaultTcal( 1, 1.0 ) ;
    ArrayColumn<Float> tcalCol( table_->tcal().table(), "TCAL" ) ;
    tcalCol.put( 0, defaultTcal ) ;
    return ;
  }    
  
  static const char *cols[] = {
    "ANTENNA_ID", "FEED_ID", "SPECTRAL_WINDOW_ID", "TIME",
    NULL
  };
  static const TypeManagerImpl<Int> tmInt;
  static const TypeManagerImpl<Double> tmDouble;
  static const TypeManager *const tms[] = {
    &tmInt, &tmInt, &tmInt, &tmDouble, NULL
  };
  Table tab = table_->tcal().table() ;
  TcalVisitor visitor( sctab, tab, tcalrec_, antenna_ ) ;
  visitor.setTcalColumn( colTcal_ ) ;
  
  traverseTable(sctab, cols, tms, &visitor);

  infillTcal();

  //tcalrec_.print( std::cout ) ;
  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSFiller::fillTcal() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSFiller::infillTcal()
{
  uInt nfields = tcalrec_.nfields() ;
  set<Int> spwAvailable;
  for (uInt i = 0; i < nfields; i++) {
    String name = tcalrec_.name(i);
    size_t pos1 = name.find(':') + 4;
    size_t pos2 = name.find(':',pos1);
    Int spwid = String::toInt(name.substr(pos1,pos2-pos1));
    //cout << "spwid=" << spwid << endl;
    spwAvailable.insert(spwid);
  }
  Table spwtab = mstable_.spectralWindow();
  Table tcaltab = table_->tcal().table();
  ScalarColumn<uInt> idCol(tcaltab, "ID");
  ScalarColumn<String> timeCol(tcaltab, "TIME");
  ArrayColumn<Float> tcalCol(tcaltab, "TCAL");
  ROScalarColumn<Int> numChanCol(spwtab, "NUM_CHAN");
  Int numSpw = spwtab.nrow();
  Int dummyFeed = 0;
  Double dummyTime = 0.0;
  Vector<uInt> idminmax(2);
  for (Int i = 0; i < numSpw; i++) {
    if (spwAvailable.find(i) == spwAvailable.end()) {
      String key = keyTcal(dummyFeed, i, dummyTime);
      Vector<Float> tcal(numChanCol(i), 1.0);
      uInt nrow = tcaltab.nrow();
      tcaltab.addRow(1);
      idCol.put(nrow, nrow);
      timeCol.put(nrow, "");
      tcalCol.put(nrow, tcal);
      idminmax = nrow;
      tcalrec_.define(key, idminmax);
    }
  }
  //tcalrec_.print(cout);
}

string MSFiller::getFrame()
{
  MFrequency::Types frame = MFrequency::DEFAULT ;
  ROTableColumn numChanCol( mstable_.spectralWindow(), "NUM_CHAN" ) ;
  ROTableColumn measFreqRefCol( mstable_.spectralWindow(), "MEAS_FREQ_REF" ) ;
  uInt nrow = numChanCol.nrow() ;
  Vector<Int> measFreqRef( nrow, MFrequency::DEFAULT ) ;
  uInt nref = 0 ;
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
    if ( numChanCol.asInt( irow ) != 4 ) { // exclude WVR
      measFreqRef[nref] = measFreqRefCol.asInt( irow ) ;
      nref++ ;
    }
  }
  if ( nref > 0 )
    frame = (MFrequency::Types)measFreqRef[0] ;

  return MFrequency::showType( frame ) ;
}

void MSFiller::initHeader( STHeader &header )
{
  header.nchan = 0 ;
  header.npol = 0 ;
  header.nif = 0 ;
  header.nbeam = 0 ;
  header.observer = "" ;
  header.project = "" ;
  header.obstype = "" ;
  header.antennaname = "" ;
  header.antennaposition.resize( 3 ) ;
  header.equinox = 0.0 ;
  header.freqref = "" ;
  header.reffreq = -1.0 ;
  header.bandwidth = 0.0 ;
  header.utc = 0.0 ;
  header.fluxunit = "" ;
  header.epoch = "" ;
  header.poltype = "" ;
}

string MSFiller::frameFromSpwTable()
{
  string frameString;
  Table tab = mstable_.spectralWindow();
  ROScalarColumn<Int> mfrCol(tab, "MEAS_FREQ_REF");
  Vector<Int> mfr = mfrCol.getColumn();
  if (allEQ(mfr,mfr[0])) {
    frameString = MFrequency::showType(mfr[0]);
    //cout << "all rows have same frame: " << frameString << endl;
  }
  else {
    mfrCol.attach(tab, "NUM_CHAN");
    for (uInt i = 0; i < tab.nrow(); i++) {
      if (mfrCol(i) != 4) {
        frameString = MFrequency::showType(mfr[i]);
        break;
      }
    }
    if (frameString.size() == 0) {
      frameString = "TOPO";
    }
  }

  //cout << "frameString = " << frameString << endl;

  return frameString;
}

};

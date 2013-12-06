//
// C++ Interface: MSWriter
//
// Description:
//
// This class is specific writer for MS format
//
// Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include <set>

#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>
#include <casa/OS/SymLink.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Cube.h>
#include <casa/Containers/RecordField.h>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/RefRows.h>
#include <tables/Tables/TableRow.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSPolIndex.h>
#include <ms/MeasurementSets/MSDataDescIndex.h>
#include <ms/MeasurementSets/MSSourceIndex.h>

#include "MSWriter.h"
#include "STHeader.h"
#include "STFrequencies.h" 
#include "STMolecules.h"
#include "STTcal.h" 
#include "MathUtils.h"
#include "TableTraverse.h"

using namespace casa ;
using namespace std ;

namespace asap {

class CorrTypeHandler {
public:
  CorrTypeHandler() 
  {}
  virtual ~CorrTypeHandler() {}
  virtual Vector<Stokes::StokesTypes> corrType() = 0 ;
  virtual void reset() 
  { 
    npol = 0 ;
  }
  void append( uInt polno ) 
  {
    polnos[npol] = polno ;
    npol++ ;
  }
  uInt nPol() { return npol ; }
protected:
  Vector<Stokes::StokesTypes> polmap ;
  uInt polnos[4] ;
  uInt npol ;
};

class LinearHandler : public CorrTypeHandler {
public:
  LinearHandler()
    : CorrTypeHandler() 
  {
    initMap() ;
  }
  virtual ~LinearHandler() {}
  virtual Vector<Stokes::StokesTypes> corrType()
  {
    Vector<Stokes::StokesTypes> ret( npol, Stokes::Undefined ) ;
    if ( npol < 4 ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) 
        ret[ipol] = polmap[polnos[ipol]] ;
    }
    else if ( npol == 4 ) {
      ret[0] = polmap[0] ;
      ret[1] = polmap[2] ;
      ret[2] = polmap[3] ;
      ret[3] = polmap[1] ;
    }
    else {
      throw( AipsError("npol > 4") ) ;
    }
    return ret ;
  }
protected:
  void initMap()
  {
    polmap.resize( 4 ) ;
    polmap[0] = Stokes::XX ;
    polmap[1] = Stokes::YY ;
    polmap[2] = Stokes::XY ;
    polmap[3] = Stokes::YX ;
  }
};
class CircularHandler : public CorrTypeHandler {
public:
  CircularHandler()
    : CorrTypeHandler() 
  {
    initMap() ;
  }
  virtual ~CircularHandler() {}
  virtual Vector<Stokes::StokesTypes> corrType()
  {
    Vector<Stokes::StokesTypes> ret( npol, Stokes::Undefined ) ;
    if ( npol < 4 ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) 
        ret[ipol] = polmap[polnos[ipol]] ;
    }
    else if ( npol == 4 ) {
      ret[0] = polmap[0] ;
      ret[1] = polmap[2] ;
      ret[2] = polmap[3] ;
      ret[3] = polmap[1] ;
    }
    else {
      throw( AipsError("npol > 4") ) ;
    }
    return ret ;
  }
private:
  void initMap()
  {
    polmap.resize( 4 ) ;
    polmap[0] = Stokes::RR ;
    polmap[1] = Stokes::LL ;
    polmap[2] = Stokes::RL ;
    polmap[3] = Stokes::LR ;
  }
};
class StokesHandler : public CorrTypeHandler {
public:
  StokesHandler()
    : CorrTypeHandler() 
  {
    initMap() ;
  }
  virtual ~StokesHandler() {}
  virtual Vector<Stokes::StokesTypes> corrType()
  {
    Vector<Stokes::StokesTypes> ret( npol, Stokes::Undefined ) ;
    if ( npol <= 4 ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) 
        ret[ipol] = polmap[polnos[ipol]] ;
    }
    else {
      throw( AipsError("npol > 4") ) ;
    }
    return ret ;
  }
private:
  void initMap()
  {
    polmap.resize( 4 ) ;
    polmap[0] = Stokes::I ;
    polmap[1] = Stokes::Q ;
    polmap[2] = Stokes::U ;
    polmap[3] = Stokes::V ;
  }
};
class LinPolHandler : public CorrTypeHandler {
public:
  LinPolHandler()
    : CorrTypeHandler() 
  {
    initMap() ;
  }
  virtual ~LinPolHandler() {}
  virtual Vector<Stokes::StokesTypes> corrType()
  {
    Vector<Stokes::StokesTypes> ret( npol, Stokes::Undefined ) ;
    if ( npol <= 2 ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) 
        ret[ipol] = polmap[polnos[ipol]] ;
    }
    else {
      throw( AipsError("npol > 4") ) ;
    }
    return ret ;
  }
private:
  void initMap()
  {
    polmap.resize( 2 ) ;
    polmap[0] = Stokes::Plinear ;
    polmap[1] = Stokes::Pangle ;
  }
};

class DataHolder {
public:
  DataHolder( TableRow &tableRow, String polType )
    : row( tableRow )
  {
    nchan = 0 ;
    npol = 0 ;
    makeCorrTypeHandler( polType ) ;
    attach() ;
    flagRow.resize( 4 ) ;
    reset() ;
    sigmaTemplate.resize( 4 ) ;
    sigmaTemplate = 1.0 ;
  }
  virtual ~DataHolder() {}
  void post() 
  {
    postData() ;
    postFlag() ;
    postFlagRow() ;
    postAuxiliary() ;
  }
  virtual void reset()
  {
    corr->reset() ;
    flagRow = False ;
    npol = 0 ;
    for ( uInt i = 0 ; i < 4 ; i++ )
      isFilled[i] = False ;    
  }
  virtual void accumulate( uInt id, Vector<Float> &sp, Vector<Bool> &fl, Bool &flr )
  {
    accumulateCorrType( id ) ;
    accumulateData( id, sp ) ;
    accumulateFlag( id, fl ) ;
    accumulateFlagRow( id, flr ) ;
    isFilled[id] = True;
  }
  uInt nPol() { return npol ; }
  uInt nChan() { return nchan ; }
  Vector<Int> corrTypeInt() 
  {
    Vector<Int> v( npol ) ;
    convertArray( v, corr->corrType() ) ;
    return v ;
  }
  Vector<Stokes::StokesTypes> corrType() { return corr->corrType() ; }
  void setNchan( uInt num ) 
  { 
    nchan = num ; 
    resize() ;
  }
protected:
  void postAuxiliary()
  {
    Vector<Float> w = sigmaTemplate( IPosition(1,0), IPosition(1,npol-1) ) ;
    sigmaRF.define( w ) ;
    weightRF.define( w ) ;
    Cube<Bool> c( npol, nchan, 1, False ) ;
    flagCategoryRF.define( c ) ;
  }
  inline void accumulateCorrType( uInt &id )
  {
    corr->append( id ) ;
    npol = corr->nPol() ;
  } 
  inline void accumulateFlagRow( uInt &id, Bool &flr )
  {
    flagRow[id] = flr ;
  }
  void postFlagRow() 
  {
    // CAS-5545 FLAG_ROW must always be set False
    //*flagRowRF = anyEQ( flagRow, True ) ;
    *flagRowRF = False;
  }
  inline void accumulateFlag( uInt &id, Vector<Bool> &fl ) 
  {
    flag.row( id ) = fl ;
  }
  virtual void postFlag() = 0 ;
  inline void accumulateData( uInt &id, Vector<Float> &sp )
  {
    data.row( id ) = sp ;
  }
  uInt filledIndex()
  {
    uInt idx = 0;
    while( !isFilled[idx] && idx < 4 ) ++idx;
    return idx;
  }
  virtual void postData() = 0 ;
  TableRow &row ;
  uInt nchan ;
  uInt npol ;
  CountedPtr<CorrTypeHandler> corr;
  RecordFieldPtr< Vector<Float> > sigmaRF ;
  RecordFieldPtr< Vector<Float> > weightRF ;
  RecordFieldPtr< Array<Bool> > flagRF ;
  RecordFieldPtr<Bool> flagRowRF ;
  RecordFieldPtr< Cube<Bool> > flagCategoryRF ;
  Vector<Bool> flagRow ;
  Matrix<Bool> flag ;
  Matrix<Float> data ;
  Vector<Float> sigmaTemplate ;
  Bool isFilled[4] ;
private:
  void makeCorrTypeHandler( String &polType )
  {
    if ( polType == "linear" ) 
      corr = new LinearHandler() ;
    else if ( polType == "circular" )
      corr = new CircularHandler() ;
    else if ( polType == "stokes" ) 
      corr = new StokesHandler() ;
    else if ( polType == "linpol" )
      corr = new LinPolHandler() ;
    else 
      throw( AipsError("Invalid polarization type") ) ;
  }
  void attach()
  {
    TableRecord &rec = row.record() ;
    sigmaRF.attachToRecord( rec, "SIGMA" ) ;
    weightRF.attachToRecord( rec, "WEIGHT" ) ;
    flagRF.attachToRecord( rec, "FLAG" ) ;
    flagRowRF.attachToRecord( rec, "FLAG_ROW" ) ;
    flagCategoryRF.attachToRecord( rec, "FLAG_CATEGORY" ) ;
  }
  void resize() 
  {
    flag.resize( 4, nchan ) ;
    data.resize( 4, nchan ) ;
  }
};

class FloatDataHolder : public DataHolder {
public:
  FloatDataHolder( TableRow &tableRow, String polType )
    : DataHolder( tableRow, polType )
  {
    attachData() ;
  }
  virtual ~FloatDataHolder() {}
protected:
  virtual void postFlag()
  {
    // CAS-5545 FLAG must all be set True if FLAG_ROW is True
    for (uInt ipol = 0; ipol < npol; ++ipol) {
      if (flagRow[ipol]) {
	flag(IPosition(2, ipol, 0), IPosition(2, ipol, nchan-1)) = True;
      }
    }
    
    if ( npol == 2 ) {
      flagRF.define( flag( IPosition( 2, 0, 0 ), IPosition( 2, npol-1, nchan-1 ) ) ) ; 
    }
    else {
      // should be npol == 1
      uInt idx = filledIndex() ;
      flagRF.define( flag( IPosition( 2, idx, 0 ), IPosition( 2, idx, nchan-1 ) ) ) ; 
    }
  }
  virtual void postData()
  {
    if ( npol == 2 ) {
      dataRF.define( data( IPosition( 2, 0, 0 ), IPosition( 2, npol-1, nchan-1 ) ) ) ;
    }
    else {
      // should be npol == 1
      uInt idx = filledIndex() ;
      dataRF.define( data( IPosition( 2, idx, 0 ), IPosition( 2, idx, nchan-1 ) ) ) ; 
    }
  }
private:
  void attachData() 
  {
    dataRF.attachToRecord( row.record(), "FLOAT_DATA" ) ;
  }
  RecordFieldPtr< Matrix<Float> > dataRF; 
};

class ComplexDataHolder : public DataHolder {
public:
  ComplexDataHolder( TableRow &tableRow, String polType )
    : DataHolder( tableRow, polType )
  {
    attachData() ;
  }
  virtual ~ComplexDataHolder() {}
protected:
  virtual void postFlag()
  {
    // CAS-5545 FLAG must all be set True if FLAG_ROW is True
    for (uInt ipol = 0; ipol < npol; ++ipol) {
      if (flagRow[ipol]) {
	flag(IPosition(2, ipol, 0), IPosition(2, ipol, nchan-1)) = True;
      }
    }
    
    if ( npol == 4 ) {
      Vector<Bool> tmp = flag.row( 3 ) ;
      flag.row( 3 ) = flag.row( 1 ) ;
      flag.row( 2 ) = flag.row( 2 ) || tmp ;
      flag.row( 1 ) = flag.row( 2 ) ;
      flagRF.define( flag ) ;
    }
    else if ( npol == 2 ) {
      flagRF.define( flag( IPosition( 2, 0, 0 ), IPosition( 2, npol-1, nchan-1 ) ) ) ; 
    }
    else {
      // should be npol == 1
      uInt idx = filledIndex() ;
      flagRF.define( flag( IPosition( 2, idx, 0 ), IPosition( 2, idx, nchan-1 ) ) ) ; 
    }
  }
  virtual void postData()
  {
    Matrix<Float> tmp( 2, nchan, 0.0 ) ;
    Matrix<Complex> v( npol, nchan ) ;
    if ( isFilled[0] ) {
      tmp.row( 0 ) = data.row( 0 ) ;
      v.row( 0 ) = RealToComplex( tmp ) ;
    }
    if ( isFilled[1] ) {
      tmp.row( 0 ) = data.row( 1 ) ;
      v.row( npol-1 ) = RealToComplex( tmp ) ;
    }
    if ( isFilled[2] && isFilled[3] ) {
      tmp.row( 0 ) = data.row( 2 ) ;
      tmp.row( 1 ) = data.row( 3 ) ;
      v.row( 1 ) = RealToComplex( tmp ) ;
      v.row( 2 ) = conj( v.row( 1 ) ) ;
    }
    dataRF.define( v ) ;
  }
private:
  void attachData()
  {
    dataRF.attachToRecord( row.record(), "DATA" ) ;
  }
  RecordFieldPtr< Matrix<Complex> > dataRF; 
};

class BaseMSWriterVisitor: public TableVisitor {
  const String *lastFieldName;
  uInt lastRecordNo;
  uInt lastBeamNo, lastScanNo, lastIfNo, lastPolNo;
  Int lastSrcType;
  uInt lastCycleNo;
  Double lastTime;
protected:
  const Table &table;
  uInt count;
public:
  BaseMSWriterVisitor(const Table &table)
    : table(table)
  {
    static const String dummy;
    lastFieldName = &dummy;
    count = 0;
  }
  
  virtual void enterFieldName(const uInt /*recordNo*/, const String &/*columnValue*/) {
  }
  virtual void leaveFieldName(const uInt /*recordNo*/, const String &/*columnValue*/) {
  }
  virtual void enterBeamNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leaveBeamNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterScanNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leaveScanNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterIfNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leaveIfNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterSrcType(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void leaveSrcType(const uInt /*recordNo*/, Int /*columnValue*/) { }
  virtual void enterCycleNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leaveCycleNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual void enterPolNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leavePolNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }

  virtual Bool visitRecord(const uInt /*recordNo*/,
			   const String &/*fieldName*/,
			   const uInt /*beamNo*/,
			   const uInt /*scanNo*/,
			   const uInt /*ifNo*/,
			   const Int /*srcType*/,
			   const uInt /*cycleNo*/,
			   const Double /*time*/,
			   const uInt /*polNo*/) { return True ;}

  virtual Bool visit(Bool isFirst, const uInt recordNo,
		     const uInt nCols, void const *const colValues[]) {
    const String *fieldName = NULL;
    uInt beamNo, scanNo, ifNo;
    Int srcType;
    uInt cycleNo;
    Double time;
    uInt polNo;
    { // prologue
      uInt i = 0;
      {
	const String *col = (const String*)colValues[i++];
	fieldName = &col[recordNo];
      }
      {
	const uInt *col = (const uInt *)colValues[i++];
	beamNo = col[recordNo];
      }
      {
	const uInt *col = (const uInt *)colValues[i++];
	scanNo = col[recordNo];
      }
      {
	const uInt *col = (const uInt *)colValues[i++];
	ifNo = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	srcType = col[recordNo];
      }
      {
	const uInt *col = (const uInt *)colValues[i++];
	cycleNo = col[recordNo];
      }
      {
	const Double *col = (const Double *)colValues[i++];
	time = col[recordNo];
      }
      {
	const Int *col = (const Int *)colValues[i++];
	polNo = col[recordNo];
      }
      assert(nCols == i);
    }

    if (isFirst) {
      enterFieldName(recordNo, *fieldName);
      enterBeamNo(recordNo, beamNo);
      enterScanNo(recordNo, scanNo);
      enterIfNo(recordNo, ifNo);
      enterSrcType(recordNo, srcType);
      enterCycleNo(recordNo, cycleNo);
      enterTime(recordNo, time);
      enterPolNo(recordNo, polNo);
    } else {
      if (lastFieldName->compare(*fieldName) != 0) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveCycleNo(lastRecordNo, lastCycleNo);
	leaveSrcType(lastRecordNo, lastSrcType);
	leaveIfNo(lastRecordNo, lastIfNo);
	leaveScanNo(lastRecordNo, lastScanNo);
	leaveBeamNo(lastRecordNo, lastBeamNo);
	leaveFieldName(lastRecordNo, *lastFieldName);

	enterFieldName(recordNo, *fieldName);
	enterBeamNo(recordNo, beamNo);
	enterScanNo(recordNo, scanNo);
	enterIfNo(recordNo, ifNo);
	enterSrcType(recordNo, srcType);
	enterCycleNo(recordNo, cycleNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastBeamNo != beamNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveCycleNo(lastRecordNo, lastCycleNo);
	leaveSrcType(lastRecordNo, lastSrcType);
	leaveIfNo(lastRecordNo, lastIfNo);
	leaveScanNo(lastRecordNo, lastScanNo);
	leaveBeamNo(lastRecordNo, lastBeamNo);

	enterBeamNo(recordNo, beamNo);
	enterScanNo(recordNo, scanNo);
	enterIfNo(recordNo, ifNo);
	enterSrcType(recordNo, srcType);
	enterCycleNo(recordNo, cycleNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastScanNo != scanNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveCycleNo(lastRecordNo, lastCycleNo);
	leaveSrcType(lastRecordNo, lastSrcType);
	leaveIfNo(lastRecordNo, lastIfNo);
	leaveScanNo(lastRecordNo, lastScanNo);

	enterScanNo(recordNo, scanNo);
	enterIfNo(recordNo, ifNo);
	enterSrcType(recordNo, srcType);
	enterCycleNo(recordNo, cycleNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastIfNo != ifNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveCycleNo(lastRecordNo, lastCycleNo);
	leaveSrcType(lastRecordNo, lastSrcType);
	leaveIfNo(lastRecordNo, lastIfNo);

	enterIfNo(recordNo, ifNo);
	enterSrcType(recordNo, srcType);
	enterCycleNo(recordNo, cycleNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastSrcType != srcType) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveCycleNo(lastRecordNo, lastCycleNo);
	leaveSrcType(lastRecordNo, lastSrcType);

	enterSrcType(recordNo, srcType);
	enterCycleNo(recordNo, cycleNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastCycleNo != cycleNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveCycleNo(lastRecordNo, lastCycleNo);

	enterCycleNo(recordNo, cycleNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastTime != time) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);

	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastPolNo != polNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	enterPolNo(recordNo, polNo);
      }
    }
    count++;
    Bool result = visitRecord(recordNo, *fieldName, beamNo, scanNo, ifNo, srcType,
                              cycleNo, time, polNo);

    { // epilogue
      lastRecordNo = recordNo;

      lastFieldName = fieldName;
      lastBeamNo = beamNo;
      lastScanNo = scanNo;
      lastIfNo = ifNo;
      lastSrcType = srcType;
      lastCycleNo = cycleNo;
      lastTime = time;
      lastPolNo = polNo;
    }
    return result ;
  }

  virtual void finish() {
    if (count > 0) {
      leavePolNo(lastRecordNo, lastPolNo);
      leaveTime(lastRecordNo, lastTime);
      leaveCycleNo(lastRecordNo, lastCycleNo);
      leaveSrcType(lastRecordNo, lastSrcType);
      leaveIfNo(lastRecordNo, lastIfNo);
      leaveScanNo(lastRecordNo, lastScanNo);
      leaveBeamNo(lastRecordNo, lastBeamNo);
      leaveFieldName(lastRecordNo, *lastFieldName);
    }
  }
};

class MSWriterVisitor: public BaseMSWriterVisitor, public MSWriterUtils {
public:
  MSWriterVisitor(const Table &table, Table &mstable)
    : BaseMSWriterVisitor(table),
      ms(mstable)
  { 
    rowidx = 0 ;
    fieldName = "" ;
    defaultFieldId = 0 ;
    spwId = -1 ;
    subscan = 1 ;
    ptName = "" ;
    srcId = 0 ;
    
    row = TableRow( ms ) ;

    initPolarization() ;
    initFrequencies() ;

    //
    // add rows to MS
    //
    uInt addrow = table.nrow() ;
    ms.addRow( addrow ) ;

    // attach to Scantable columns
    spectraCol.attach( table, "SPECTRA" ) ;
    flagtraCol.attach( table, "FLAGTRA" ) ;
    flagRowCol.attach( table, "FLAGROW" ) ;
    tcalIdCol.attach( table, "TCAL_ID" ) ;
    intervalCol.attach( table, "INTERVAL" ) ;
    directionCol.attach( table, "DIRECTION" ) ;
    scanRateCol.attach( table, "SCANRATE" ) ;
    timeCol.attach( table, "TIME" ) ;
    freqIdCol.attach( table, "FREQ_ID" ) ;
    sourceNameCol.attach( table, "SRCNAME" ) ;
    sourceDirectionCol.attach( table, "SRCDIRECTION" ) ;
    fieldNameCol.attach( table, "FIELDNAME" ) ;

    // MS subtables
    attachSubtables() ;

    // attach to MS columns
    attachMain() ;
    attachPointing() ;
  }
  
  virtual void enterFieldName(const uInt recordNo, const String &/*columnValue*/) {
    //printf("%u: FieldName: %s\n", recordNo, columnValue.c_str());
    fieldName = fieldNameCol.asString( recordNo ) ;
    String::size_type pos = fieldName.find( "__" ) ;
    if ( pos != String::npos ) {
      fieldId = String::toInt( fieldName.substr( pos+2 ) ) ;
      fieldName = fieldName.substr( 0, pos ) ;
    }
    else {
      fieldId = defaultFieldId ;
      defaultFieldId++ ;
    }
    Double tSec = timeCol.asdouble( recordNo ) * 86400.0 ;
    Vector<Double> srcDir = sourceDirectionCol( recordNo ) ;
    Vector<Double> srate = scanRateCol( recordNo ) ;
    String srcName = sourceNameCol.asString( recordNo ) ;

    addField( fieldId, fieldName, srcName, srcDir, srate, tSec ) ;

    // put value
    *fieldIdRF = fieldId ;
  }
  virtual void leaveFieldName(const uInt /*recordNo*/, const String &/*columnValue*/) {
  }
  virtual void enterBeamNo(const uInt /*recordNo*/, uInt columnValue) {
    //printf("%u: BeamNo: %u\n", recordNo, columnValue);
    
    feedId = (Int)columnValue ;

    // put value
    *feed1RF = feedId ;
    *feed2RF = feedId ;
  }
  virtual void leaveBeamNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
  }
  virtual void enterScanNo(const uInt /*recordNo*/, uInt columnValue) {
    //printf("%u: ScanNo: %u\n", recordNo, columnValue);

    // put value
    // CAS-5841: SCANNO should be consistent with MS SCAN_NUMBER
    *scanNumberRF = (Int)columnValue ;
  }
  virtual void leaveScanNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
    subscan = 1 ;
  }
  virtual void enterIfNo(const uInt recordNo, uInt columnValue) {
    //printf("%u: IfNo: %u\n", recordNo, columnValue);

    spwId = (Int)columnValue ;
    uInt freqId = freqIdCol.asuInt( recordNo ) ;

    Vector<Float> sp = spectraCol( recordNo ) ;
    uInt nchan = sp.nelements() ;
    holder->setNchan( nchan ) ;

    addSpectralWindow( spwId, freqId ) ;

    addFeed( feedId, spwId ) ;
  }
  virtual void leaveIfNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
  }
  virtual void enterSrcType(const uInt /*recordNo*/, Int columnValue) {
    //printf("%u: SrcType: %d\n", recordNo, columnValue);

    Int stateId = addState( columnValue ) ;

    // put value
    *stateIdRF = stateId ;
  }
  virtual void leaveSrcType(const uInt /*recordNo*/, Int /*columnValue*/) {
  }
  virtual void enterCycleNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
    //printf("%u: CycleNo: %u\n", recordNo, columnValue);
  }
  virtual void leaveCycleNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
  }
  virtual void enterTime(const uInt recordNo, Double columnValue) {
    //printf("%u: Time: %f\n", recordNo, columnValue);

    Double timeSec = columnValue * 86400.0 ;
    Double interval = intervalCol.asdouble( recordNo ) ;

    if ( ptName.empty() ) {
      Vector<Double> dir = directionCol( recordNo ) ;
      Vector<Double> rate = scanRateCol( recordNo ) ;
      if ( anyNE( rate, 0.0 ) ) {
        Matrix<Double> msdir( 2, 2 ) ;
        msdir.column( 0 ) = dir ;
        msdir.column( 1 ) = rate ;
        addPointing( timeSec, interval, msdir ) ;
      }
      else {
        Matrix<Double> msdir( 2, 1 ) ;
        msdir.column( 0 ) = dir ;
        addPointing( timeSec, interval, msdir ) ;
      }
    }

    // put value
    *timeRF = timeSec ;
    *timeCentroidRF = timeSec ;
    *intervalRF = interval ;
    *exposureRF = interval ;
  }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) {
    if ( holder->nPol() > 0 ) {
      Int polId = addPolarization() ;
      Int ddId = addDataDescription( polId, spwId ) ;
       
      // put field
      *dataDescIdRF = ddId ;
      holder->post() ;
      
      // commit row
      row.put( rowidx ) ;
      rowidx++ ;

      // reset holder
      holder->reset() ;
    }
  }
  virtual void enterPolNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
    //printf("%u: PolNo: %d\n", recordNo, columnValue);
  }
  virtual void leavePolNo(const uInt /*recordNo*/, uInt /*columnValue*/) {
  }

  virtual Bool visitRecord(const uInt recordNo,
			   const String &/*fieldName*/,
			   const uInt /*beamNo*/,
			   const uInt /*scanNo*/,
			   const uInt /*ifNo*/,
			   const Int /*srcType*/,
			   const uInt /*cycleNo*/,
			   const Double /*time*/,
			   const uInt polNo) {
    //printf("%u: %s, %u, %u, %u, %d, %u, %f, %d\n", recordNo,
    //       fieldName.c_str(), beamNo, scanNo, ifNo, srcType, cycleNo, time, polNo);

    Vector<Float> sp = spectraCol( recordNo ) ;
    Vector<uChar> tmp = flagtraCol( recordNo ) ;
    Vector<Bool> fl( tmp.shape() ) ;
    convertArray( fl, tmp ) ;
    Bool flr = (Bool)flagRowCol.asuInt( recordNo ) ;
    holder->accumulate( polNo, sp, fl, flr ) ;

    return True ;
  }

  virtual void finish() {
    BaseMSWriterVisitor::finish();
    //printf("Total: %u\n", count);

    // remove rows
    if ( ms.nrow() > rowidx ) {
      uInt numRemove = ms.nrow() - rowidx ;
      //cout << "numRemove = " << numRemove << endl ;
      Vector<uInt> rows( numRemove ) ;
      indgen( rows, rowidx ) ;
      ms.removeRow( rows ) ;
    }

    // fill empty SPECTRAL_WINDOW rows
    infillSpectralWindow() ;

    // fill empty FIELD rows
    infillField() ;
  }

  void dataColumnName( String name ) 
  {
    if ( name == "DATA" ) 
      holder = new ComplexDataHolder( row, poltype ) ;
    else if ( name == "FLOAT_DATA" ) 
      holder = new FloatDataHolder( row, poltype ) ;
  }
  void pointingTableName( String name ) {
    ptName = name ;
  }
  void setSourceRecord( Record &r ) {
    srcRec = r ;
  }
private:
  void addField( Int &fid, String &fname, String &srcName,
                 Vector<Double> &sdir, Vector<Double> &srate, 
                 Double &tSec ) 
  {
    uInt nrow = fieldtab.nrow() ;
    while( (Int)nrow <= fid ) {
      fieldtab.addRow( 1, True ) ;
      nrow++ ;
    }

    Matrix<Double> dir ;
    Int numPoly = 0 ;
    if ( anyNE( srate, 0.0 ) ) {
      dir.resize( 2, 2 ) ;
      dir.column( 0 ) = sdir ;
      dir.column( 1 ) = srate ;
      numPoly = 1 ;
    }
    else {
      dir.resize( 2, 1 ) ;
      dir.column( 0 ) = sdir ;
    }
    srcId = srcRec.asInt( srcName ) ;

    TableRow tr( fieldtab ) ;
    TableRecord &r = tr.record() ;
    putField( "NAME", r, fname ) ;
    putField( "NUM_POLY", r, numPoly ) ;
    putField( "TIME", r, tSec ) ;
    putField( "SOURCE_ID", r, srcId ) ;
    defineField( "DELAY_DIR", r, dir ) ;
    defineField( "REFERENCE_DIR", r, dir ) ;
    defineField( "PHASE_DIR", r, dir ) ;
    tr.put( fid ) ;

    // for POINTING table
    *poNameRF = fname ;
  }
  Int addState( Int &id ) 
  {
    String obsMode ;
    Bool isSignal ;
    Double tnoise ;
    Double tload ;
    queryType( id, obsMode, isSignal, tnoise, tload ) ;

    String key = obsMode+"_"+String::toString( subscan ) ;
    Int idx = -1 ;
    uInt nEntry = stateEntry.nelements() ;
    for ( uInt i = 0 ; i < nEntry ; i++ ) {
      if ( stateEntry[i] == key ) {
        idx = i ;
        break ;
      }
    }
    if ( idx == -1 ) {
      uInt nrow = statetab.nrow() ;
      statetab.addRow( 1, True ) ;
      TableRow tr( statetab ) ;
      TableRecord &r = tr.record() ;
      putField( "OBS_MODE", r, obsMode ) ;
      putField( "SIG", r, isSignal ) ;
      isSignal = !isSignal ;
      putField( "REF", r, isSignal ) ;
      putField( "CAL", r, tnoise ) ;
      putField( "LOAD", r, tload ) ;
      tr.put( nrow ) ;
      idx = nrow ;

      stateEntry.resize( nEntry+1, True ) ;
      stateEntry[nEntry] = key ;
    }
    subscan++ ;

    return idx ;
  }
  void addPointing( Double &tSec, Double &interval, Matrix<Double> &dir ) 
  {
    uInt nrow = potab.nrow() ;
    potab.addRow() ;

    *poNumPolyRF = dir.ncolumn() - 1 ;
    *poTimeRF = tSec ;
    *poTimeOriginRF = tSec ;
    *poIntervalRF = interval ;
    poDirectionRF.define( dir ) ;
    poTargetRF.define( dir ) ;
    porow.put( nrow ) ;
  }
  Int addPolarization()
  {
    Int idx = -1 ;
    Vector<Int> corrType = holder->corrTypeInt() ;
    uInt nEntry = polEntry.size() ;
    for ( uInt i = 0 ; i < nEntry ; i++ ) {
      if ( polEntry[i].conform( corrType ) && allEQ( polEntry[i], corrType ) ) {
        idx = i ;
        break ;
      }
    }
    
    Int numCorr = holder->nPol() ;
    Matrix<Int> corrProduct = corrProductTemplate[numCorr].copy() ;
    if ( numCorr == 1 && (corrType[0] == Stokes::YY || corrType[0] == Stokes::LL ) ) {
      corrProduct = 1;
    }

    if ( idx == -1 ) {
      uInt nrow = poltab.nrow() ;
      poltab.addRow( 1, True ) ;
      TableRow tr( poltab ) ;
      TableRecord &r = tr.record() ;
      putField( "NUM_CORR", r, numCorr ) ;
      defineField( "CORR_TYPE", r, corrType ) ;
      defineField( "CORR_PRODUCT", r, corrProduct ) ;
      tr.put( nrow ) ;
      idx = nrow ;

      polEntry.resize( nEntry+1 ) ;
      polEntry[nEntry] = corrType ;
    }

    return idx ;
  }
  Int addDataDescription( Int pid, Int sid ) 
  {
    Int idx = -1 ;
    uInt nItem = 2 ;
    uInt len = ddEntry.nelements() ;
    uInt nEntry = len / nItem ;
    const Int *dd_p = ddEntry.storage() ;
    for ( uInt i = 0 ; i < nEntry ; i++ ) {
      Int pol = *dd_p ;
      dd_p++ ;
      Int spw = *dd_p ;
      dd_p++ ;
      if ( pid == pol && sid == spw ) {
        idx = i ;
        break ;
      }
    }

    if ( idx == -1 ) {
      uInt nrow = ddtab.nrow() ;
      ddtab.addRow( 1, True ) ;
      TableRow tr( ddtab ) ;
      TableRecord &r = tr.record() ;
      putField( "POLARIZATION_ID", r, pid ) ;
      putField( "SPECTRAL_WINDOW_ID", r, sid ) ;
      tr.put( nrow ) ;
      idx = nrow ;

      ddEntry.resize( len+nItem ) ;
      ddEntry[len] = pid ;
      ddEntry[len+1] = sid ;
    }

    return idx ;
  }
  void infillSpectralWindow()
  {
    ROScalarColumn<Int> nchanCol( spwtab, "NUM_CHAN" ) ;
    Vector<Int> nchan = nchanCol.getColumn() ;
    TableRow tr( spwtab ) ;
    TableRecord &r = tr.record() ;
    Int mfr = freqframe ;
    Int oneChan = 1 ;
    Vector<Double> dummy( 1, 0.0 ) ;
    putField( "MEAS_FREQ_REF", r, mfr ) ;
    defineField( "CHAN_FREQ", r, dummy ) ;
    defineField( "CHAN_WIDTH", r, dummy ) ;
    defineField( "EFFECTIVE_BW", r, dummy ) ;
    defineField( "RESOLUTION", r, dummy ) ;
    putField( "NUM_CHAN", r, oneChan ) ;

    for ( uInt i = 0 ; i < spwtab.nrow() ; i++ ) {
      if ( nchan[i] == 0 )
        tr.put( i ) ;
    }
  }
  void infillField()
  {
    ScalarColumn<Int> sourceIdCol(fieldtab, "SOURCE_ID");
    ArrayColumn<Double> delayDirCol(fieldtab, "DELAY_DIR");
    ArrayColumn<Double> phaseDirCol(fieldtab, "PHASE_DIR");
    ArrayColumn<Double> referenceDirCol(fieldtab, "REFERENCE_DIR");
    uInt nrow = fieldtab.nrow();
    Matrix<Double> dummy(IPosition(2, 2, 1), 0.0);
    for (uInt irow = 0; irow < nrow; ++irow) {
      if (!phaseDirCol.isDefined(irow)) {
        delayDirCol.put(irow, dummy);
        phaseDirCol.put(irow, dummy);
        referenceDirCol.put(irow, dummy);
        sourceIdCol.put(irow, -1);
      }
    }
  }
  void addSpectralWindow( Int sid, uInt fid )
  {
    if (processedFreqId.find((uInt)fid) == processedFreqId.end() 
        || processedIFNO.find((uInt)sid) == processedIFNO.end() ) {
      uInt nrow = spwtab.nrow() ;
      while( (Int)nrow <= sid ) {
        spwtab.addRow( 1, True ) ;
        nrow++ ;
      }
      processedFreqId.insert((uInt)fid);
      processedIFNO.insert((uInt)sid);
    }
    else {
      return ;
    }
      

    Double rp = refpix[fid] ;
    Double rv = refval[fid] ;
    Double ic = increment[fid] ;

    Int mfrInt = (Int)freqframe ;
    Int nchan = holder->nChan() ;
    Double bw = nchan * abs( ic ) ;
    Double reffreq = rv - rp * ic ;
    Int netsb = 0 ; // USB->0, LSB->1
    if ( ic < 0 )
      netsb = 1 ;
    Vector<Double> res( nchan, abs(ic) ) ;
    Vector<Double> cw( nchan, ic ) ;
    Vector<Double> chanf( nchan ) ;
    indgen( chanf, reffreq, ic ) ;

    TableRow tr( spwtab ) ;
    TableRecord &r = tr.record() ;
    putField( "MEAS_FREQ_REF", r, mfrInt ) ;
    putField( "NUM_CHAN", r, nchan ) ;
    putField( "TOTAL_BANDWIDTH", r, bw ) ;
    putField( "REF_FREQUENCY", r, reffreq ) ;
    putField( "NET_SIDEBAND", r, netsb ) ;
    defineField( "RESOLUTION", r, res ) ;
//     defineField( "CHAN_WIDTH", r, res ) ;
    defineField( "CHAN_WIDTH", r, cw ) ;
    defineField( "EFFECTIVE_BW", r, res ) ;
    defineField( "CHAN_FREQ", r, chanf ) ;
    tr.put( sid ) ;
  }
  void addFeed( Int fid, Int sid )
  {
    Int idx = -1 ;
    uInt nItem = 2 ;
    uInt len = feedEntry.nelements() ;
    uInt nEntry = len / nItem ;
    const Int *fe_p = feedEntry.storage() ;
    for ( uInt i = 0 ; i < nEntry ; i++ ) {
      Int feed = *fe_p ;
      fe_p++ ;
      Int spw = *fe_p ;
      fe_p++ ;
      if ( fid == feed && sid == spw ) {
        idx = i ;
        break ;
      }
    }


    if ( idx == -1 ) {
      uInt nrow = feedtab.nrow() ;
      feedtab.addRow( 1, True ) ;
      Int numReceptors = 2 ;
      Vector<String> polType( numReceptors ) ;
      Matrix<Double> beamOffset( 2, numReceptors, 0.0 ) ;
      Vector<Double> receptorAngle( numReceptors, 0.0 ) ;
      if ( poltype == "linear" ) {
        polType[0] = "X" ;
        polType[1] = "Y" ;
      }
      else if ( poltype == "circular" ) {
        polType[0] = "R" ;
        polType[1] = "L" ;
      }
      else {
        polType[0] = "X" ;
        polType[1] = "Y" ;
      }
      Matrix<Complex> polResponse( numReceptors, numReceptors, 0.0 ) ;
      
      TableRow tr( feedtab ) ;
      TableRecord &r = tr.record() ;
      putField( "FEED_ID", r, fid ) ;
      putField( "BEAM_ID", r, fid ) ;
      Int tmp = 0 ;
      putField( "ANTENNA_ID", r, tmp ) ;
      putField( "SPECTRAL_WINDOW_ID", r, sid ) ;
      putField( "NUM_RECEPTORS", r, numReceptors ) ;
      defineField( "POLARIZATION_TYPE", r, polType ) ;
      defineField( "BEAM_OFFSET", r, beamOffset ) ;
      defineField( "RECEPTOR_ANGLE", r, receptorAngle ) ;
      defineField( "POL_RESPONSE", r, polResponse ) ;
      tr.put( nrow ) ;

      feedEntry.resize( len+nItem ) ;
      feedEntry[len] = fid ;
      feedEntry[len+1] = sid ;
    }
  }
  void initPolarization() 
  {
    const TableRecord &keys = table.keywordSet() ;
    poltype = keys.asString( "POLTYPE" ) ;

    initCorrProductTemplate() ;
  }
  void initFrequencies()
  {
    const TableRecord &keys = table.keywordSet() ;
    Table tab = keys.asTable( "FREQUENCIES" ) ;
    ROScalarColumn<uInt> idcol( tab, "ID" ) ;
    ROScalarColumn<Double> rpcol( tab, "REFPIX" ) ;
    ROScalarColumn<Double> rvcol( tab, "REFVAL" ) ;
    ROScalarColumn<Double> iccol( tab, "INCREMENT" ) ;
    Vector<uInt> id = idcol.getColumn() ;
    Vector<Double> rp = rpcol.getColumn() ;
    Vector<Double> rv = rvcol.getColumn() ;
    Vector<Double> ic = iccol.getColumn() ;
    for ( uInt i = 0 ; i < id.nelements() ; i++ ) {
      refpix.insert( pair<uInt,Double>( id[i], rp[i] ) ) ;
      refval.insert( pair<uInt,Double>( id[i], rv[i] ) ) ;
      increment.insert( pair<uInt,Double>( id[i], ic[i] ) ) ;
    }
    String frameStr = tab.keywordSet().asString( "BASEFRAME" ) ;
    MFrequency::getType( freqframe, frameStr ) ;
  }
  void attachSubtables()
  {
    //const TableRecord &keys = table.keywordSet() ;
    TableRecord &mskeys = ms.rwKeywordSet() ;

    // FIELD table
    fieldtab = mskeys.asTable( "FIELD" ) ;

    // SPECTRAL_WINDOW table
    spwtab = mskeys.asTable( "SPECTRAL_WINDOW" ) ;

    // POINTING table
    potab = mskeys.asTable( "POINTING" ) ;

    // POLARIZATION table
    poltab = mskeys.asTable( "POLARIZATION" ) ;

    // DATA_DESCRIPTION table
    ddtab = mskeys.asTable( "DATA_DESCRIPTION" ) ;

    // STATE table 
    statetab = mskeys.asTable( "STATE" ) ;

    // FEED table
    feedtab = mskeys.asTable( "FEED" ) ;
  }
  void attachMain()
  {
    TableRecord &r = row.record() ;
    dataDescIdRF.attachToRecord( r, "DATA_DESC_ID" ) ;
    timeRF.attachToRecord( r, "TIME" ) ;
    timeCentroidRF.attachToRecord( r, "TIME_CENTROID" ) ;
    intervalRF.attachToRecord( r, "INTERVAL" ) ;
    exposureRF.attachToRecord( r, "EXPOSURE" ) ;
    fieldIdRF.attachToRecord( r, "FIELD_ID" ) ;
    feed1RF.attachToRecord( r, "FEED1" ) ;
    feed2RF.attachToRecord( r, "FEED2" ) ;
    scanNumberRF.attachToRecord( r, "SCAN_NUMBER" ) ;
    stateIdRF.attachToRecord( r, "STATE_ID" ) ;

    // constant values
    //Int id = 0 ;
    RecordFieldPtr<Int> intRF( r, "OBSERVATION_ID" ) ;
    *intRF = 0 ;
    intRF.attachToRecord( r, "ANTENNA1" ) ;
    *intRF = 0 ;
    intRF.attachToRecord( r, "ANTENNA2" ) ;
    *intRF = 0 ;
    intRF.attachToRecord( r, "ARRAY_ID" ) ;
    *intRF = 0 ;
    intRF.attachToRecord( r, "PROCESSOR_ID" ) ;
    *intRF = 0 ;
    RecordFieldPtr< Vector<Double> > arrayRF( r, "UVW" ) ; 
    arrayRF.define( Vector<Double>( 3, 0.0 ) ) ;
  }
  void attachPointing()
  {
    porow = TableRow( potab ) ;
    TableRecord &r = porow.record() ;
    poNumPolyRF.attachToRecord( r, "NUM_POLY" ) ;
    poTimeRF.attachToRecord( r, "TIME" ) ;
    poTimeOriginRF.attachToRecord( r, "TIME_ORIGIN" ) ;
    poIntervalRF.attachToRecord( r, "INTERVAL" ) ;
    poNameRF.attachToRecord( r, "NAME" ) ;
    poDirectionRF.attachToRecord( r, "DIRECTION" ) ;
    poTargetRF.attachToRecord( r, "TARGET" ) ;
    
    // constant values
    RecordFieldPtr<Int> antIdRF( r, "ANTENNA_ID" ) ;
    *antIdRF = 0 ;
    RecordFieldPtr<Bool> trackingRF( r, "TRACKING" ) ;
    *trackingRF = True ;
  }
  void queryType( Int type, String &stype, Bool &b, Double &t, Double &l )
  {
    t = 0.0 ;
    l = 0.0 ;

    String sep1="#" ;
    String sep2="," ;
    String target="OBSERVE_TARGET" ;
    String atmcal="CALIBRATE_TEMPERATURE" ;
    String onstr="ON_SOURCE" ;
    String offstr="OFF_SOURCE" ;
    String pswitch="POSITION_SWITCH" ;
    String nod="NOD" ;
    String fswitch="FREQUENCY_SWITCH" ;
    String sigstr="SIG" ;
    String refstr="REF" ;
    String unspecified="UNSPECIFIED" ;
    String ftlow="LOWER" ;
    String fthigh="HIGHER" ;
    switch ( type ) {
    case SrcType::PSON:
      stype = target+sep1+onstr+sep2+pswitch ;
      b = True ;
      break ;
    case SrcType::PSOFF:
      stype = target+sep1+offstr+sep2+pswitch ;
      b = False ;
      break ;
    case SrcType::NOD:
      stype = target+sep1+onstr+sep2+nod ;
      b = True ;
      break ;
    case SrcType::FSON:
      stype = target+sep1+onstr+sep2+fswitch+sep1+sigstr ;
      b = True ;
      break ;
    case SrcType::FSOFF:
      stype = target+sep1+onstr+sep2+fswitch+sep1+refstr ;
      b = False ;
      break ;
    case SrcType::SKY:
      stype = atmcal+sep1+offstr+sep2+unspecified ;
      b = False ;
      break ;
    case SrcType::HOT:
      stype = atmcal+sep1+offstr+sep2+unspecified ;
      b = False ;
      break ;
    case SrcType::WARM:
      stype = atmcal+sep1+offstr+sep2+unspecified ;
      b = False ;
      break ;
    case SrcType::COLD:
      stype = atmcal+sep1+offstr+sep2+unspecified ;
      b = False ;
      break ;
    case SrcType::PONCAL:
      stype = atmcal+sep1+onstr+sep2+pswitch ;
      b = True ;
      break ;
    case SrcType::POFFCAL:
      stype = atmcal+sep1+offstr+sep2+pswitch ;
      b = False ;
      break ;
    case SrcType::NODCAL:
      stype = atmcal+sep1+onstr+sep2+nod ;
      b = True ;
      break ;
    case SrcType::FONCAL:
      stype = atmcal+sep1+onstr+sep2+fswitch+sep1+sigstr ;
      b = True ;
      break ;
    case SrcType::FOFFCAL:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+refstr ;
      b = False ;
      break ;
    case SrcType::FSLO:
      stype = target+sep1+onstr+sep2+fswitch+sep1+ftlow ;
      b = True ;
      break ;
    case SrcType::FLOOFF:
      stype = target+sep1+offstr+sep2+fswitch+sep1+ftlow ;
      b = False ;
      break ;
    case SrcType::FLOSKY:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+ftlow ;
      b = False ;
      break ;
    case SrcType::FLOHOT:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+ftlow ;
      b = False ;
      break ;
    case SrcType::FLOWARM:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+ftlow ;
      b = False ;
      break ;
    case SrcType::FLOCOLD:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+ftlow ;
      b = False ;
      break ;
    case SrcType::FSHI:
      stype = target+sep1+onstr+sep2+fswitch+sep1+fthigh ;
      b = True ;
      break ;
    case SrcType::FHIOFF:
      stype = target+sep1+offstr+sep2+fswitch+sep1+fthigh ;
      b = False ;
      break ;
    case SrcType::FHISKY:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+fthigh ;
      b = False ;
      break ;
    case SrcType::FHIHOT:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+fthigh ;
      b = False ;
      break ;
    case SrcType::FHIWARM:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+fthigh ;
      b = False ;
      break ;
    case SrcType::FHICOLD:
      stype = atmcal+sep1+offstr+sep2+fswitch+sep1+fthigh ;
      b = False ;
      break ;
    case SrcType::SIG:
      stype = target+sep1+onstr+sep2+unspecified ;
      b = True ;
      break ;
    case SrcType::REF:
      stype = target+sep1+offstr+sep2+unspecified ;
      b = False ;
      break ;
    default:
      stype = unspecified ;
      b = True ;
      break ;
    }
  }
  void initCorrProductTemplate()
  {
    Int n = 1 ;
    {
      Matrix<Int> c( 2, n, 0 ) ;
      corrProductTemplate[n] = c ;
    }
    n = 2 ;
    { 
      Matrix<Int> c( 2, n, 0 ) ;
      c.column( 1 ) = 1 ;
      corrProductTemplate[n] = c ;
    }
    n = 4 ;
    {
      Matrix<Int> c( 2, n, 0 ) ;
      c( 0, 2 ) = 1 ;
      c( 0, 3 ) = 1 ;
      c( 1, 1 ) = 1 ;
      c( 1, 3 ) = 1 ;
      corrProductTemplate[n] = c ;
    }
  }

  Table &ms;
  TableRow row;
  uInt rowidx;
  String fieldName;
  Int fieldId;
  Int srcId;
  Int defaultFieldId;
  Int spwId;
  Int feedId;
  Int subscan;
  CountedPtr<DataHolder> holder;
  String ptName;
  Bool useFloat;
  String poltype;

  // MS subtables
  Table spwtab;
  Table statetab;
  Table ddtab;
  Table poltab;
  Table fieldtab;
  Table feedtab;
  Table potab;

  // Scantable MAIN columns
  ROArrayColumn<Float> spectraCol;
  ROArrayColumn<Double> directionCol,scanRateCol,sourceDirectionCol;
  ROArrayColumn<uChar> flagtraCol;
  ROTableColumn tcalIdCol,intervalCol,flagRowCol,timeCol,freqIdCol,
    sourceNameCol,fieldNameCol;

  // MS MAIN columns
  RecordFieldPtr<Int> dataDescIdRF,fieldIdRF,feed1RF,feed2RF,
    scanNumberRF,stateIdRF;
  RecordFieldPtr<Double> timeRF,timeCentroidRF,intervalRF,exposureRF;

  // MS POINTING columns
  TableRow porow;
  RecordFieldPtr<Int> poNumPolyRF ;
  RecordFieldPtr<Double> poTimeRF,
    poTimeOriginRF,
    poIntervalRF ;
  RecordFieldPtr<String> poNameRF ;
  RecordFieldPtr< Matrix<Double> > poDirectionRF,
    poTargetRF ;

  Vector<String> stateEntry;
  Block<Int> ddEntry;
  Block<Int> feedEntry;
  vector< Vector<Int> > polEntry;
  set<uInt> processedFreqId;
  set<uInt> processedIFNO;
  map<uInt,Double> refpix;
  map<uInt,Double> refval;
  map<uInt,Double> increment;
  MFrequency::Types freqframe;
  Record srcRec;
  map< Int, Matrix<Int> > corrProductTemplate;
};

class BaseMSSysCalVisitor: public TableVisitor {
  uInt lastRecordNo;
  uInt lastBeamNo, lastIfNo, lastPolNo;
  Double lastTime;
protected:
  const Table &table;
  uInt count;
public:
  BaseMSSysCalVisitor(const Table &table)
    : table(table)
  {
    count = 0;
  }
  
  virtual void enterBeamNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leaveBeamNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterIfNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leaveIfNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterPolNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void leavePolNo(const uInt /*recordNo*/, uInt /*columnValue*/) { }
  virtual void enterTime(const uInt /*recordNo*/, Double /*columnValue*/) { }
  virtual void leaveTime(const uInt /*recordNo*/, Double /*columnValue*/) { }

  virtual Bool visitRecord(const uInt /*recordNo*/,
			   const uInt /*beamNo*/,
			   const uInt /*ifNo*/,
			   const uInt /*polNo*/,
			   const Double /*time*/) { return True ;}

  virtual Bool visit(Bool isFirst, const uInt recordNo,
		     const uInt nCols, void const *const colValues[]) {
    uInt beamNo, ifNo, polNo;
    Double time;
    { // prologue
      uInt i = 0;
      {
	const uInt *col = (const uInt *)colValues[i++];
	beamNo = col[recordNo];
      }
      {
	const uInt *col = (const uInt *)colValues[i++];
	ifNo = col[recordNo];
      }
      {
	const Double *col = (const Double *)colValues[i++];
	time = col[recordNo];
      }
      {
	const uInt *col = (const uInt *)colValues[i++];
	polNo = col[recordNo];
      }
      assert(nCols == i);
    }

    if (isFirst) {
      enterBeamNo(recordNo, beamNo);
      enterIfNo(recordNo, ifNo);
      enterTime(recordNo, time);
      enterPolNo(recordNo, polNo);
    } else {
      if (lastBeamNo != beamNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveIfNo(lastRecordNo, lastIfNo);
	leaveBeamNo(lastRecordNo, lastBeamNo);

	enterBeamNo(recordNo, beamNo);
	enterIfNo(recordNo, ifNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastIfNo != ifNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);
	leaveIfNo(lastRecordNo, lastIfNo);
        
	enterIfNo(recordNo, ifNo);
	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastTime != time) {
	leavePolNo(lastRecordNo, lastPolNo);
	leaveTime(lastRecordNo, lastTime);

	enterTime(recordNo, time);
	enterPolNo(recordNo, polNo);
      } else if (lastPolNo != polNo) {
	leavePolNo(lastRecordNo, lastPolNo);
	enterPolNo(recordNo, polNo);
      }
    }
    count++;
    Bool result = visitRecord(recordNo, beamNo, ifNo, polNo, time);

    { // epilogue
      lastRecordNo = recordNo;

      lastBeamNo = beamNo;
      lastIfNo = ifNo;
      lastPolNo = polNo;
      lastTime = time;
    }
    return result ;
  }

  virtual void finish() {
    if (count > 0) {
      leavePolNo(lastRecordNo, lastPolNo);
      leaveTime(lastRecordNo, lastTime);
      leaveIfNo(lastRecordNo, lastIfNo);
      leaveBeamNo(lastRecordNo, lastBeamNo);
    }
  }
};

class BaseTsysHolder
{
public:
  BaseTsysHolder( ROArrayColumn<Float> &tsysCol )
    : col( tsysCol ),
      nchan(0)
  {
    reset() ;
  }
  virtual ~BaseTsysHolder() {}
  virtual Array<Float> getTsys() = 0 ;
  void setNchan( uInt n ) { nchan = n ; }
  void appendTsys( uInt row ) 
  {
    Vector<Float> v = col( row ) ;
    uInt len = tsys.nrow() ;
    tsys.resize( len+1, nchan, True ) ;
    if ( v.nelements() == nchan )
      tsys.row( len ) = v ;
    else
      tsys.row( len ) = v[0] ;
  }
  void setTsys( uInt row, uInt idx ) 
  {
    if ( idx >= nrow() ) 
      appendTsys( row ) ;
    else {
      Vector<Float> v = col( row ) ;
      if ( v.nelements() == nchan )
        tsys.row( idx ) = v ;
      else
        tsys.row( idx ) = v[0] ;
    }
  }
  void reset() 
  {
    tsys.resize() ;
  } 
  uInt nrow() { return tsys.nrow() ; }
  Bool isEffective() 
  {
    return ( !(tsys.empty()) && anyNE( tsys, (Float)1.0 ) ) ;
  }
  BaseTsysHolder &operator= ( const BaseTsysHolder &v )
  {
    if ( this != &v )
      tsys.assign( v.tsys ) ;
    return *this ;
  }
protected:
  ROArrayColumn<Float> col ;
  Matrix<Float> tsys ;
  uInt nchan ;
};

class TsysHolder : public BaseTsysHolder
{
public:
  TsysHolder( ROArrayColumn<Float> &tsysCol )
    : BaseTsysHolder( tsysCol )
  {}
  virtual ~TsysHolder() {}
  virtual Array<Float> getTsys() 
  {
    return tsys.column( 0 ) ;
  }
};

class TsysSpectrumHolder : public BaseTsysHolder
{
public:
  TsysSpectrumHolder( ROArrayColumn<Float> &tsysCol )
    : BaseTsysHolder( tsysCol ) 
  {}
  virtual ~TsysSpectrumHolder() {}
  virtual Array<Float> getTsys() 
  { 
    return tsys ; 
  }
};

class BaseTcalProcessor
{
public:
  BaseTcalProcessor( ROArrayColumn<Float> &tcalCol )
    : col_( tcalCol )
  {}
  virtual ~BaseTcalProcessor() {}
  void setTcalId( Vector<uInt> &tcalId ) { id_.assign( tcalId ) ; } 
  virtual Array<Float> getTcal() = 0 ;
protected:
  ROArrayColumn<Float> col_ ;
  Vector<uInt> id_ ;
};

class TcalProcessor : public BaseTcalProcessor
{
public:
  TcalProcessor( ROArrayColumn<Float> &tcalCol )
    : BaseTcalProcessor( tcalCol )
  {}
  virtual ~TcalProcessor() {}
  virtual Array<Float> getTcal()
  {
    uInt npol = id_.nelements() ;
    Vector<Float> tcal( npol ) ;
    for ( uInt ipol = 0 ; ipol < npol ; ipol++ )
      tcal[ipol] = col_( id_[ipol] ).data()[0] ;
    //cout << "TcalProcessor: tcal = " << tcal << endl ;
    return tcal ;
  }
};

class TcalSpectrumProcessor : public BaseTcalProcessor
{
public:
  TcalSpectrumProcessor( ROArrayColumn<Float> &tcalCol )
    : BaseTcalProcessor( tcalCol )
  {}
  virtual ~TcalSpectrumProcessor() {}
  virtual Array<Float> getTcal()
  {
    uInt npol = id_.nelements() ;
    //Vector<Float> tcal0 = col_( 0 ) ;
    Vector<Float> tcal0 = col_( id_[0] ) ;
    uInt nchan = tcal0.nelements() ;
    Matrix<Float> tcal( npol, nchan ) ;
    tcal.row( 0 ) = tcal0 ;
    for ( uInt ipol = 1 ; ipol < npol ; ipol++ ) 
      tcal.row( ipol ) = col_( id_[ipol] ) ;
    return tcal ;
  }
};

class MSSysCalVisitor : public BaseMSSysCalVisitor
{
public:
  MSSysCalVisitor( const Table &from, Table &to )
    : BaseMSSysCalVisitor( from ),
      sctab( to ),
      rowidx( 0 ),
      polno_()
  {
    scrow = TableRow( sctab ) ;

    lastTcalId.resize() ;
    theTcalId.resize() ;
    startTime = 0.0 ;
    endTime = 0.0 ;

    const TableRecord &keys = table.keywordSet() ;
    Table tcalTable = keys.asTable( "TCAL" ) ;
    tcalCol.attach( tcalTable, "TCAL" ) ;
    tsysCol.attach( table, "TSYS" ) ;
    tcalIdCol.attach( table, "TCAL_ID" ) ;
    intervalCol.attach( table, "INTERVAL" ) ;
    effectiveTcal.resize( tcalTable.nrow() ) ;
    for ( uInt irow = 0 ; irow < tcalTable.nrow() ; irow++ ) {
      if ( allEQ( tcalCol( irow ), (Float)1.0 ) )
        effectiveTcal[irow] = False ;
      else 
        effectiveTcal[irow] = True ;
    }
    
    TableRecord &r = scrow.record() ;
    RecordFieldPtr<Int> antennaIdRF( r, "ANTENNA_ID" ) ;
    *antennaIdRF = 0 ;
    feedIdRF.attachToRecord( r, "FEED_ID" ) ;
    specWinIdRF.attachToRecord( r, "SPECTRAL_WINDOW_ID" ) ;
    timeRF.attachToRecord( r, "TIME" ) ;
    intervalRF.attachToRecord( r, "INTERVAL" ) ;
    if ( r.isDefined( "TCAL" ) ) {
      tcalRF.attachToRecord( r, "TCAL" ) ;
      tcalProcessor = new TcalProcessor( tcalCol ) ;
    }
    else if ( r.isDefined( "TCAL_SPECTRUM" ) ) {
      tcalRF.attachToRecord( r, "TCAL_SPECTRUM" ) ;
      tcalProcessor = new TcalSpectrumProcessor( tcalCol ) ;
    }
    if ( r.isDefined( "TSYS" ) ) {
      tsysRF.attachToRecord( r, "TSYS" ) ;
      theTsys = new TsysHolder( tsysCol ) ;
      lastTsys = new TsysHolder( tsysCol ) ;
    }
    else {
      tsysRF.attachToRecord( r, "TSYS_SPECTRUM" ) ;
      theTsys = new TsysSpectrumHolder( tsysCol ) ;
      lastTsys = new TsysSpectrumHolder( tsysCol ) ;
    }

  }

  virtual void enterBeamNo(const uInt /*recordNo*/, uInt columnValue) 
  { 
    *feedIdRF = (Int)columnValue ;
  }
  virtual void leaveBeamNo(const uInt /*recordNo*/, uInt /*columnValue*/) 
  { 
  }
  virtual void enterIfNo(const uInt recordNo, uInt columnValue) 
  { 
    //cout << "enterIfNo" << endl ;
    ROArrayColumn<Float> sp( table, "SPECTRA" ) ;
    uInt nchan = sp( recordNo ).nelements() ;
    theTsys->setNchan( nchan ) ;
    lastTsys->setNchan( nchan ) ;

    *specWinIdRF = (Int)columnValue ;
  }
  virtual void leaveIfNo(const uInt /*recordNo*/, uInt /*columnValue*/) 
  { 
    //cout << "leaveIfNo" << endl ;
    post() ;
    reset(true);
    startTime = 0.0 ;
    endTime = 0.0 ;
  }
  virtual void enterTime(const uInt recordNo, Double columnValue) 
  { 
    //cout << "enterTime" << endl ;
    interval = intervalCol.asdouble( recordNo ) ;
    // start time and end time
    if ( startTime == 0.0 ) {
      startTime = columnValue * 86400.0 - 0.5 * interval ;
      endTime = columnValue * 86400.0 + 0.5 * interval ;
    }
  }
  virtual void leaveTime(const uInt /*recordNo*/, Double columnValue) 
  { 
    //cout << "leaveTime" << endl ;
    if ( isUpdated() ) {
      post() ;
      *lastTsys = *theTsys ;
      lastTcalId = theTcalId ;
      reset(false);
      startTime = columnValue * 86400.0 - 0.5 * interval ;
      endTime = columnValue * 86400.0 + 0.5 * interval ;
    }
    else {
      endTime = columnValue * 86400.0 + 0.5 * interval ;
    }
  }
  virtual void enterPolNo(const uInt recordNo, uInt columnValue) 
  {
    //cout << "enterPolNo" << endl ;
    Vector<Float> tsys = tsysCol( recordNo ) ;
    uInt tcalId = tcalIdCol.asuInt( recordNo ) ;
    polno_.insert( columnValue ) ;
    uInt numPol = polno_.size() ;
    if ( lastTsys->nrow() < numPol )
      lastTsys->appendTsys( recordNo ) ;
    if ( lastTcalId.nelements() <= numPol ) 
      appendTcalId( lastTcalId, tcalId, numPol-1 ) ;
    if ( theTsys->nrow() < numPol )
      theTsys->appendTsys( recordNo ) ;
    else {
      theTsys->setTsys( recordNo, numPol-1 ) ;
    }
    if ( theTcalId.nelements() < numPol )
      appendTcalId( theTcalId, tcalId, numPol-1 ) ;
    else 
      setTcalId( theTcalId, tcalId, numPol-1 ) ;
  }
  virtual void leavePolNo( const uInt /*recordNo*/, uInt /*columnValue*/ )
  {
  }
    
private:
  void reset(bool completely)
  {
    if (completely) {
      lastTsys->reset() ;
      lastTcalId.resize() ;
    }
    theTsys->reset() ;
    theTcalId.resize() ;
    polno_.clear();
  }
  void appendTcalId( Vector<uInt> &v, uInt &elem, uInt polId )
  {
    v.resize( polId+1, True ) ;
    v[polId] = elem ;
  }
  void setTcalId( Vector<uInt> &v, uInt &elem, uInt polId )
  {
    v[polId] = elem ;
  }
  void post()
  {
    // check if given Tcal and Tsys is effective 
    Bool isEffective = False ;
    for ( uInt ipol = 0 ; ipol < lastTcalId.nelements() ; ipol++ ) {
      if ( effectiveTcal[lastTcalId[ipol]] ) {
        isEffective = True ;
        break ;
      }
    }
    if ( !isEffective ) {
      if ( !(lastTsys->isEffective()) )
        return ;
    }

    //cout << " interval: " << (endTime-startTime) << " lastTcalId = " << lastTcalId << endl ;
    Double midTime = 0.5 * ( startTime + endTime ) ;
    Double interval = endTime - startTime ;
    *timeRF = midTime ;
    *intervalRF = interval ;
    tcalProcessor->setTcalId( lastTcalId ) ;
    Array<Float> tcal = tcalProcessor->getTcal() ;
    Array<Float> tsys = lastTsys->getTsys() ;
    tcalRF.define( tcal ) ;
    tsysRF.define( tsys ) ;
    sctab.addRow( 1, True ) ;
    scrow.put( rowidx ) ;
    rowidx++ ;
  }
  
  Bool isUpdated()
  {
    Bool ret = (anyNE( theTcalId, lastTcalId ) || anyNE( theTsys->getTsys(), lastTsys->getTsys() )) ;
    return ret ;
  }

  Table &sctab;
  TableRow scrow;
  uInt rowidx;

  Double startTime,endTime,interval;
  
  CountedPtr<BaseTsysHolder> lastTsys,theTsys;
  Vector<uInt> lastTcalId,theTcalId;
  set<uInt> polno_;
  CountedPtr<BaseTcalProcessor> tcalProcessor ;
  Vector<Bool> effectiveTcal;

  RecordFieldPtr<Int> feedIdRF,specWinIdRF;
  RecordFieldPtr<Double> timeRF,intervalRF;
  RecordFieldPtr< Array<Float> > tcalRF,tsysRF;

  ROArrayColumn<Float> tsysCol,tcalCol;
  ROTableColumn tcalIdCol,intervalCol;
};

MSWriter::MSWriter(CountedPtr<Scantable> stable) 
  : table_(stable),
    mstable_(NULL),
    isWeather_(False),
    tcalSpec_(False),
    tsysSpec_(False),
    ptTabName_("")
{
  os_ = LogIO() ;
  os_.origin( LogOrigin( "MSWriter", "MSWriter()", WHERE ) ) ;
//   os_ << "MSWriter::MSWriter()" << LogIO::POST ;

  // initialize writer
  init() ;
}

MSWriter::~MSWriter() 
{
  os_.origin( LogOrigin( "MSWriter", "~MSWriter()", WHERE ) ) ;
//   os_ << "MSWriter::~MSWriter()" << LogIO::POST ;

  if ( mstable_ != 0 ) 
    delete mstable_ ;
}

bool MSWriter::write(const string& filename, const Record& rec) 
{
  os_.origin( LogOrigin( "MSWriter", "write()", WHERE ) ) ;
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSWriter::write() startSec=" << startSec << LogIO::POST ;

  filename_ = filename ;

  // parsing MS options
  Bool overwrite = False ;
  if ( rec.isDefined( "ms" ) ) {
    Record msrec = rec.asRecord( "ms" ) ;
    if ( msrec.isDefined( "overwrite" ) ) {
      overwrite = msrec.asBool( "overwrite" ) ;
    }
  }

  os_ << "Parsing MS options" << endl ;
  os_ << "   overwrite = " << overwrite << LogIO::POST ; 

  File file( filename_ ) ;
  if ( file.exists() ) {
    if ( overwrite ) {
      os_ << filename_ << " exists. Overwrite existing data... " << LogIO::POST ;
      if ( file.isRegular() ) RegularFile(file).remove() ;
      else if ( file.isDirectory() ) Directory(file).removeRecursive() ;
      else SymLink(file).remove() ;
    }
    else {
      os_ << LogIO::SEVERE << "ERROR: " << filename_ << " exists..." << LogIO::POST ;
      return False ;
    }
  }

  // set up MS
  setupMS() ;
  
  // subtables
  // OBSERVATION
  fillObservation() ;

  // ANTENNA
  fillAntenna() ;

  // PROCESSOR
  fillProcessor() ;

  // SOURCE
  fillSource() ;

  // WEATHER
  if ( isWeather_ ) 
    fillWeather() ;

  // SYSCAL
  fillSysCal() ;

  /***
   * Start iteration using TableVisitor
   ***/
  {
    static const char *cols[] = {
      "FIELDNAME", "BEAMNO", "SCANNO", "IFNO", "SRCTYPE", "CYCLENO", "TIME",
      "POLNO",
      NULL
    };
    static const TypeManagerImpl<uInt> tmUInt;
    static const TypeManagerImpl<Int> tmInt;
    static const TypeManagerImpl<Double> tmDouble;
    static const TypeManagerImpl<String> tmString;
    static const TypeManager *const tms[] = {
      &tmString, &tmUInt, &tmUInt, &tmUInt, &tmInt, &tmUInt, &tmDouble, &tmUInt, NULL
    };
    //double t0 = mathutil::gettimeofday_sec() ;
    MSWriterVisitor myVisitor(table_->table(),*mstable_);
    //double t1 = mathutil::gettimeofday_sec() ;
    //cout << "MSWriterVisitor(): elapsed time " << t1-t0 << " sec" << endl ;
    String dataColName = "FLOAT_DATA" ;
    if ( useData_ )
      dataColName = "DATA" ;
    myVisitor.dataColumnName( dataColName ) ;
    myVisitor.pointingTableName( ptTabName_ ) ;
    myVisitor.setSourceRecord( srcRec_ ) ;
    //double t2 = mathutil::gettimeofday_sec() ;
    traverseTable(table_->table(), cols, tms, &myVisitor);
    //double t3 = mathutil::gettimeofday_sec() ;
    //cout << "traverseTable(): elapsed time " << t3-t2 << " sec" << endl ;
  }
  /***
   * End iteration using TableVisitor
   ***/

  // ASDM tables 
  const TableRecord &stKeys = table_->table().keywordSet() ;
  TableRecord &msKeys = mstable_->rwKeywordSet() ;
  uInt nfields = stKeys.nfields() ;
  for ( uInt ifield = 0 ; ifield < nfields ; ifield++ ) {
    String kname = stKeys.name( ifield ) ;
    if ( kname.find( "ASDM" ) != String::npos ) {
      String asdmpath = stKeys.asString( ifield ) ;
      os_ << "found ASDM table: " << asdmpath << LogIO::POST ;
      if ( Table::isReadable( asdmpath ) ) {
        Table newAsdmTab( asdmpath, Table::Old ) ;
        newAsdmTab.copy( filename_+"/"+kname, Table::New ) ;
        os_ << "add subtable: " << kname << LogIO::POST ;
        msKeys.defineTable( kname, Table( filename_+"/"+kname, Table::Old ) ) ;
      }
    }
  }

  // replace POINTING table with original one if exists
  if ( ptTabName_ != "" ) {
    delete mstable_ ;
    mstable_ = 0 ;
    Table newPtTab( ptTabName_, Table::Old ) ;
    newPtTab.copy( filename_+"/POINTING", Table::New ) ;
  }

  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSWriter::write() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;

  os_ << "Exported data as MS" << LogIO::POST ;

  return True ;
}

void MSWriter::init()
{
//   os_.origin( LogOrigin( "MSWriter", "init()", WHERE ) ) ;
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::init() startSec=" << startSec << LogIO::POST ;
  
  // access to scantable
  header_ = table_->getHeader() ;

  // FLOAT_DATA? or DATA?
  if ( header_.npol > 2 ) {
    useFloatData_ = False ;
    useData_ = True ;
  }
  else {
    useFloatData_ = True ;
    useData_ = False ;
  }

  // polarization type 
  polType_ = header_.poltype ;
  if ( polType_ == "" ) 
    polType_ = "stokes" ;
  else if ( polType_.find( "linear" ) != String::npos ) 
    polType_ = "linear" ;
  else if ( polType_.find( "circular" ) != String::npos )
    polType_ = "circular" ;
  else if ( polType_.find( "stokes" ) != String::npos ) 
    polType_ = "stokes" ;
  else if ( polType_.find( "linpol" ) != String::npos )
    polType_ = "linpol" ;
  else 
    polType_ = "notype" ;

  // Check if some subtables are exists
  Bool isTcal = False ;
  if ( table_->tcal().table().nrow() != 0 ) {
    ROTableColumn col( table_->tcal().table(), "TCAL" ) ;
    if ( col.isDefined( 0 ) ) {
      os_ << "TCAL table exists: nrow=" << table_->tcal().table().nrow() << LogIO::POST ;
      isTcal = True ;
    }
    else {
      os_ << "No TCAL rows" << LogIO::POST ;
    }
  }
  else {
    os_ << "No TCAL rows" << LogIO::POST ;
  }
  if ( table_->weather().table().nrow() != 0 ) {
    ROTableColumn col( table_->weather().table(), "TEMPERATURE" ) ;
    if ( col.isDefined( 0 ) ) {
      os_ << "WEATHER table exists: nrow=" << table_->weather().table().nrow() << LogIO::POST ;
      isWeather_ =True ;
    }
    else {
      os_ << "No WEATHER rows" << LogIO::POST ;
    }
  }
  else {
    os_ << "No WEATHER rows" << LogIO::POST ;
  }

  // Are TCAL_SPECTRUM and TSYS_SPECTRUM necessary?
  if ( header_.nchan != 1 ) {
    if ( isTcal ) {
      // examine TCAL subtable
      Table tcaltab = table_->tcal().table() ;
      ROArrayColumn<Float> tcalCol( tcaltab, "TCAL" ) ;
      for ( uInt irow = 0 ; irow < tcaltab.nrow() ; irow++ ) {
        if ( tcalCol( irow ).size() != 1 )
          tcalSpec_ = True ;
      }
    }
    // examine spectral data
    TableIterator iter0( table_->table(), "IFNO" ) ;
    while( !iter0.pastEnd() ) {
      Table t0( iter0.table() ) ;
      ROArrayColumn<Float> sharedFloatArrCol( t0, "SPECTRA" ) ;
      uInt len = sharedFloatArrCol( 0 ).size() ;
      if ( len != 1 ) {
        sharedFloatArrCol.attach( t0, "TSYS" ) ;
        if ( sharedFloatArrCol( 0 ).size() != 1 ) 
          tsysSpec_ = True ;
      }
      iter0.next() ;
    }
  }

  // check if reference for POINTING table exists
  const TableRecord &rec = table_->table().keywordSet() ;
  if ( rec.isDefined( "POINTING" ) ) {
    ptTabName_ = rec.asString( "POINTING" ) ;
    if ( !Table::isReadable( ptTabName_ ) ) {
      ptTabName_ = "" ;
    }
  }

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::init() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::setupMS()
{
//   os_.origin( LogOrigin( "MSWriter", "setupMS()", WHERE ) ) ;
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::setupMS() startSec=" << startSec << LogIO::POST ;
  
  String dunit = table_->getHeader().fluxunit ;

  TableDesc msDesc = MeasurementSet::requiredTableDesc() ;
  if ( useFloatData_ )
    MeasurementSet::addColumnToDesc( msDesc, MSMainEnums::FLOAT_DATA, 2 ) ;
  else if ( useData_ )
    MeasurementSet::addColumnToDesc( msDesc, MSMainEnums::DATA, 2 ) ;

  SetupNewTable newtab( filename_, msDesc, Table::New ) ;

  mstable_ = new MeasurementSet( newtab ) ;

  TableColumn col ;
  if ( useFloatData_ )
    col.attach( *mstable_, "FLOAT_DATA" ) ;
  else if ( useData_ )
    col.attach( *mstable_, "DATA" ) ;
  col.rwKeywordSet().define( "UNIT", dunit ) ;

  // create subtables
  TableDesc antennaDesc = MSAntenna::requiredTableDesc() ;
  SetupNewTable antennaTab( mstable_->antennaTableName(), antennaDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::ANTENNA ), Table( antennaTab ) ) ;

  TableDesc dataDescDesc = MSDataDescription::requiredTableDesc() ;
  SetupNewTable dataDescTab( mstable_->dataDescriptionTableName(), dataDescDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::DATA_DESCRIPTION ), Table( dataDescTab ) ) ;

  TableDesc dopplerDesc = MSDoppler::requiredTableDesc() ;
  SetupNewTable dopplerTab( mstable_->dopplerTableName(), dopplerDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::DOPPLER ), Table( dopplerTab ) ) ;

  TableDesc feedDesc = MSFeed::requiredTableDesc() ;
  SetupNewTable feedTab( mstable_->feedTableName(), feedDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::FEED ), Table( feedTab ) ) ;

  TableDesc fieldDesc = MSField::requiredTableDesc() ;
  SetupNewTable fieldTab( mstable_->fieldTableName(), fieldDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::FIELD ), Table( fieldTab ) ) ;

  TableDesc flagCmdDesc = MSFlagCmd::requiredTableDesc() ;
  SetupNewTable flagCmdTab( mstable_->flagCmdTableName(), flagCmdDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::FLAG_CMD ), Table( flagCmdTab ) ) ;

  TableDesc freqOffsetDesc = MSFreqOffset::requiredTableDesc() ;
  SetupNewTable freqOffsetTab( mstable_->freqOffsetTableName(), freqOffsetDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::FREQ_OFFSET ), Table( freqOffsetTab ) ) ;

  TableDesc historyDesc = MSHistory::requiredTableDesc() ;
  SetupNewTable historyTab( mstable_->historyTableName(), historyDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::HISTORY ), Table( historyTab ) ) ;

  TableDesc observationDesc = MSObservation::requiredTableDesc() ;
  SetupNewTable observationTab( mstable_->observationTableName(), observationDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::OBSERVATION ), Table( observationTab ) ) ;

  TableDesc pointingDesc = MSPointing::requiredTableDesc() ;
  SetupNewTable pointingTab( mstable_->pointingTableName(), pointingDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::POINTING ), Table( pointingTab ) ) ;

  TableDesc polarizationDesc = MSPolarization::requiredTableDesc() ;
  SetupNewTable polarizationTab( mstable_->polarizationTableName(), polarizationDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::POLARIZATION ), Table( polarizationTab ) ) ;

  TableDesc processorDesc = MSProcessor::requiredTableDesc() ;
  SetupNewTable processorTab( mstable_->processorTableName(), processorDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::PROCESSOR ), Table( processorTab ) ) ;

  TableDesc sourceDesc = MSSource::requiredTableDesc() ;
  MSSource::addColumnToDesc( sourceDesc, MSSourceEnums::TRANSITION, 1 ) ;
  MSSource::addColumnToDesc( sourceDesc, MSSourceEnums::REST_FREQUENCY, 1 ) ;
  MSSource::addColumnToDesc( sourceDesc, MSSourceEnums::SYSVEL, 1 ) ;
  SetupNewTable sourceTab( mstable_->sourceTableName(), sourceDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::SOURCE ), Table( sourceTab ) ) ;

  TableDesc spwDesc = MSSpectralWindow::requiredTableDesc() ;
  SetupNewTable spwTab( mstable_->spectralWindowTableName(), spwDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::SPECTRAL_WINDOW ), Table( spwTab ) ) ;

  TableDesc stateDesc = MSState::requiredTableDesc() ;
  SetupNewTable stateTab( mstable_->stateTableName(), stateDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::STATE ), Table( stateTab ) ) ;

  TableDesc sysCalDesc = MSSysCal::requiredTableDesc() ;
  if ( tcalSpec_ ) 
    MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TCAL_SPECTRUM, 2 ) ;
  else 
    MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TCAL, 1 ) ;
  if ( tsysSpec_ )
    MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TSYS_SPECTRUM, 2 ) ;
  else 
    MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TSYS, 1 ) ;
  SetupNewTable sysCalTab( mstable_->sysCalTableName(), sysCalDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::SYSCAL ), Table( sysCalTab ) ) ;

  TableDesc weatherDesc = MSWeather::requiredTableDesc() ;
  MSWeather::addColumnToDesc( weatherDesc, MSWeatherEnums::TEMPERATURE ) ;
  MSWeather::addColumnToDesc( weatherDesc, MSWeatherEnums::PRESSURE ) ;
  MSWeather::addColumnToDesc( weatherDesc, MSWeatherEnums::REL_HUMIDITY ) ;
  MSWeather::addColumnToDesc( weatherDesc, MSWeatherEnums::WIND_SPEED ) ;
  MSWeather::addColumnToDesc( weatherDesc, MSWeatherEnums::WIND_DIRECTION ) ;
  SetupNewTable weatherTab( mstable_->weatherTableName(), weatherDesc, Table::New ) ;
  mstable_->rwKeywordSet().defineTable( MeasurementSet::keywordName( MeasurementSet::WEATHER ), Table( weatherTab ) ) ;

  mstable_->initRefs() ;

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::setupMS() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillObservation() 
{
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSWriter::fillObservation() startSec=" << startSec << LogIO::POST ;

  // only 1 row
  mstable_->observation().addRow( 1, True ) ;
  MSObservationColumns msObsCols( mstable_->observation() ) ;
  msObsCols.observer().put( 0, header_.observer ) ;
  // tentatively put antennaname (from ANTENNA subtable)
  String hAntennaName = header_.antennaname ;
  String::size_type pos = hAntennaName.find( "//" ) ;
  String telescopeName ;
  if ( pos != String::npos ) {
    telescopeName = hAntennaName.substr( 0, pos ) ;
  }
  else {
    pos = hAntennaName.find( "@" ) ;
    telescopeName = hAntennaName.substr( 0, pos ) ;
  }
//   os_ << "telescopeName = " << telescopeName << LogIO::POST ;
  msObsCols.telescopeName().put( 0, telescopeName ) ;
  msObsCols.project().put( 0, header_.project ) ;
  //ScalarMeasColumn<MEpoch> timeCol( table_->table().sort("TIME"), "TIME" ) ;
  Table sortedtable = table_->table().sort("TIME") ;
  ScalarMeasColumn<MEpoch> timeCol( sortedtable, "TIME" ) ;
  Vector<MEpoch> trange( 2 ) ;
  trange[0] = timeCol( 0 ) ;
  trange[1] = timeCol( table_->nrow()-1 ) ;
  msObsCols.timeRangeMeas().put( 0, trange ) ;

  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSWriter::fillObservation() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::antennaProperty( String &name, String &m, String &t, Double &d )
{
  name.upcase() ;
  
  m = "ALT-AZ" ;
  t = "GROUND-BASED" ;
  if ( name.matches( Regex( "DV[0-9]+$" ) ) 
       || name.matches( Regex( "DA[0-9]+$" ) )
       || name.matches( Regex( "PM[0-9]+$" ) ) )
    d = 12.0 ;
  else if ( name.matches( Regex( "CM[0-9]+$" ) ) )
    d = 7.0 ;
  else if ( name.contains( "GBT" ) ) 
    d = 104.9 ;
  else if ( name.contains( "MOPRA" ) )
    d = 22.0 ;
  else if ( name.contains( "PKS" ) || name.contains( "PARKS" ) )
    d = 64.0 ;
  else if ( name.contains( "TIDBINBILLA" ) ) 
    d = 70.0 ;
  else if ( name.contains( "CEDUNA" ) )
    d = 30.0 ;
  else if ( name.contains( "HOBART" ) ) 
    d = 26.0 ;
  else if ( name.contains( "APEX" ) ) 
    d = 12.0 ;
  else if ( name.contains( "ASTE" ) ) 
    d = 10.0 ;
  else if ( name.contains( "NRO" ) ) 
    d = 45.0 ;
  else 
    d = 1.0 ;
} 

void MSWriter::fillAntenna() 
{
  //double startSec = mathutil::gettimeofday_sec() ;
  //os_ << "start MSWriter::fillAntenna() startSec=" << startSec << LogIO::POST ;

  // only 1 row
  Table anttab = mstable_->antenna() ;
  anttab.addRow( 1, True ) ;
  
  Table &table = table_->table() ;
  const TableRecord &keys = table.keywordSet() ;
  String hAntName = keys.asString( "AntennaName" ) ;
  String::size_type pos = hAntName.find( "//" ) ;
  String antennaName ;
  String stationName ;
  if ( pos != String::npos ) {
    stationName = hAntName.substr( 0, pos ) ;
    hAntName = hAntName.substr( pos+2 ) ;
  }
  pos = hAntName.find( "@" ) ;
  if ( pos != String::npos ) {
    antennaName = hAntName.substr( 0, pos ) ;
    stationName = hAntName.substr( pos+1 ) ;
  }
  else {
    antennaName = hAntName ;
  }
  Vector<Double> antpos = keys.asArrayDouble( "AntennaPosition" ) ;
  
  String mount, atype ;
  Double diameter ;
  antennaProperty( antennaName, mount, atype, diameter ) ;
  
  TableRow tr( anttab ) ;
  TableRecord &r = tr.record() ;
  RecordFieldPtr<String> nameRF( r, "NAME" ) ;
  RecordFieldPtr<String> stationRF( r, "STATION" ) ;
  RecordFieldPtr<String> mountRF( r, "MOUNT" ) ;
  RecordFieldPtr<String> typeRF( r, "TYPE" ) ;
  RecordFieldPtr<Double> dishDiameterRF( r, "DISH_DIAMETER" ) ;
  RecordFieldPtr< Vector<Double> > positionRF( r, "POSITION" ) ;
  *nameRF = antennaName ;
  *mountRF = mount ;
  *typeRF = atype ;
  *dishDiameterRF = diameter ;
  *positionRF = antpos ;
  *stationRF = stationName ;
  
  tr.put( 0 ) ;

  //double endSec = mathutil::gettimeofday_sec() ;
  //os_ << "end MSWriter::fillAntenna() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}
  
void MSWriter::fillProcessor() 
{
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::fillProcessor() startSec=" << startSec << LogIO::POST ;
  
  // only add empty 1 row
  MSProcessor msProc = mstable_->processor() ;
  msProc.addRow( 1, True ) ;

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::fillProcessor() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillSource()
{
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::fillSource() startSec=" << startSec << LogIO::POST ;
 
  // access to MS SOURCE subtable
  MSSource msSrc = mstable_->source() ;

  // access to MOLECULE subtable
  STMolecules stm = table_->molecules() ;

  Int srcId = 0 ;

  // row based
  TableRow row( msSrc ) ;
  TableRecord &rec = row.record() ;
  RecordFieldPtr<Int> srcidRF( rec, "SOURCE_ID" ) ;
  RecordFieldPtr<String> nameRF( rec, "NAME" ) ;
  RecordFieldPtr< Array<Double> > srcpmRF( rec, "PROPER_MOTION" ) ;
  RecordFieldPtr< Array<Double> > srcdirRF( rec, "DIRECTION" ) ;
  RecordFieldPtr<Int> numlineRF( rec, "NUM_LINES" ) ;
  RecordFieldPtr< Array<Double> > restfreqRF( rec, "REST_FREQUENCY" ) ;
  RecordFieldPtr< Array<Double> > sysvelRF( rec, "SYSVEL" ) ;
  RecordFieldPtr< Array<String> > transitionRF( rec, "TRANSITION" ) ;
  RecordFieldPtr<Double> timeRF( rec, "TIME" ) ;
  RecordFieldPtr<Double> intervalRF( rec, "INTERVAL" ) ;
  RecordFieldPtr<Int> spwidRF( rec, "SPECTRAL_WINDOW_ID" ) ;

  // 
  // ITERATION: SRCNAME
  //
  TableIterator iter0( table_->table(), "SRCNAME" ) ;
  while( !iter0.pastEnd() ) {
    //Table t0( iter0.table() ) ;
    Table t0 =  iter0.table()  ;

    // get necessary information
    ROScalarColumn<String> srcNameCol( t0, "SRCNAME" ) ;
    String srcName = srcNameCol( 0 ) ;
    ROArrayColumn<Double> sharedDArrRCol( t0, "SRCPROPERMOTION" ) ;
    Vector<Double> srcPM = sharedDArrRCol( 0 ) ;
    sharedDArrRCol.attach( t0, "SRCDIRECTION" ) ;
    Vector<Double> srcDir = sharedDArrRCol( 0 ) ;
    ROScalarColumn<Double> srcVelCol( t0, "SRCVELOCITY" ) ;
    Double srcVel = srcVelCol( 0 ) ;
    srcRec_.define( srcName, srcId ) ;

    // NAME
    *nameRF = srcName ;

    // SOURCE_ID
    *srcidRF = srcId ;

    // PROPER_MOTION
    *srcpmRF = srcPM ;
    
    // DIRECTION
    *srcdirRF = srcDir ;

    //
    // ITERATION: MOLECULE_ID
    //
    TableIterator iter1( t0, "MOLECULE_ID" ) ;
    while( !iter1.pastEnd() ) {
      //Table t1( iter1.table() ) ;
      Table t1 = iter1.table() ;

      // get necessary information
      Vector<Double> restFreq ;
      Vector<String> molName ;
      Vector<String> fMolName ;
      ROScalarColumn<uInt> molIdCol( t1, "MOLECULE_ID" ) ;
      uInt molId = molIdCol( 0 ) ;
      stm.getEntry( restFreq, molName, fMolName, molId ) ;

      uInt numFreq = restFreq.size() ;
      
      // NUM_LINES
      *numlineRF = numFreq ;

      // REST_FREQUENCY
      restfreqRF.define(restFreq);

      // TRANSITION
      Vector<String> transition ;
      if ( fMolName.size() != 0 ) {
        transition = fMolName ;
      }
      else if ( molName.size() != 0 ) {
        transition = molName ;
      }
      else {
        transition.resize( numFreq ) ;
        transition = "" ;
      }
      transitionRF.define(transition);

      // SYSVEL
      Vector<Double> sysvelArr( numFreq, srcVel ) ;
      sysvelRF.define(sysvelArr);

      //
      // ITERATION: IFNO
      //
      TableIterator iter2( t1, "IFNO" ) ;
      while( !iter2.pastEnd() ) {
        //Table t2( iter2.table() ) ;
        Table t2 = iter2.table() ;
        uInt nrow = msSrc.nrow() ;

        // get necessary information
        ROScalarColumn<uInt> ifNoCol( t2, "IFNO" ) ;
        uInt ifno = ifNoCol( 0 ) ; // IFNO = SPECTRAL_WINDOW_ID
        Double midTime ;
        Double interval ;
        getValidTimeRange( midTime, interval, t2 ) ;

        // fill SPECTRAL_WINDOW_ID
        *spwidRF = ifno ;

        // fill TIME, INTERVAL
        *timeRF = midTime ;
        *intervalRF = interval ;

        // add row
        msSrc.addRow( 1, True ) ;
        row.put( nrow ) ;

        iter2.next() ;
      }

      iter1.next() ;
    }

    // increment srcId if SRCNAME changed
    srcId++ ;

    iter0.next() ;
  }

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::fillSource() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillWeather() 
{
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::fillWeather() startSec=" << startSec << LogIO::POST ;

  // access to MS WEATHER subtable
  MSWeather msw = mstable_->weather() ;

  // access to WEATHER subtable
  Table stw = table_->weather().table() ;
  uInt nrow = stw.nrow() ;

  if ( nrow == 0 ) 
    return ;

  msw.addRow( nrow, True ) ;
  MSWeatherColumns mswCols( msw ) ;

  // ANTENNA_ID is always 0
  Vector<Int> antIdArr( nrow, 0 ) ;
  mswCols.antennaId().putColumn( antIdArr ) ;

  // fill weather status
  ROScalarColumn<Float> sharedFloatCol( stw, "TEMPERATURE" ) ;
  mswCols.temperature().putColumn( sharedFloatCol ) ;
  sharedFloatCol.attach( stw, "PRESSURE" ) ;
  mswCols.pressure().putColumn( sharedFloatCol ) ;
  sharedFloatCol.attach( stw, "HUMIDITY" ) ;
  mswCols.relHumidity().putColumn( sharedFloatCol ) ;
  sharedFloatCol.attach( stw, "WINDSPEED" ) ;
  mswCols.windSpeed().putColumn( sharedFloatCol ) ;
  sharedFloatCol.attach( stw, "WINDAZ" ) ;
  mswCols.windDirection().putColumn( sharedFloatCol ) ;

  // fill TIME and INTERVAL
  Double midTime ;
  Double interval ;
  Vector<Double> intervalArr( nrow, 0.0 ) ;
  TableIterator iter( table_->table(), "WEATHER_ID" ) ;
  while( !iter.pastEnd() ) {
    //Table tab( iter.table() ) ;
    Table tab = iter.table() ;

    ROScalarColumn<uInt> widCol( tab, "WEATHER_ID" ) ;
    uInt wid = widCol( 0 ) ;

    getValidTimeRange( midTime, interval, tab ) ;
    mswCols.time().put( wid, midTime ) ;
    intervalArr[wid] = interval ;

    iter.next() ;
  }
  mswCols.interval().putColumn( intervalArr ) ;

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::fillWeather() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillSysCal() 
{
  Table mssc = mstable_->sysCal() ;

  {
    static const char *cols[] = {
      "BEAMNO", "IFNO", "TIME", "POLNO",
      NULL
    };
    static const TypeManagerImpl<uInt> tmUInt;
    static const TypeManagerImpl<Double> tmDouble;
    static const TypeManager *const tms[] = {
      &tmUInt, &tmUInt, &tmDouble, &tmUInt, NULL
    };
    //double t0 = mathutil::gettimeofday_sec() ;
    MSSysCalVisitor myVisitor(table_->table(),mssc);
    //double t1 = mathutil::gettimeofday_sec() ;
    //cout << "MSWriterVisitor(): elapsed time " << t1-t0 << " sec" << endl ;
    traverseTable(table_->table(), cols, tms, &myVisitor);
    //double t3 = mathutil::gettimeofday_sec() ;
    //cout << "traverseTable(): elapsed time " << t3-t2 << " sec" << endl ;
  }
 
}

void MSWriter::getValidTimeRange( Double &me, Double &interval, Table &tab ) 
{
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::getVaridTimeRange() startSec=" << startSec << LogIO::POST ;

  // sort table
  //Table stab = tab.sort( "TIME" ) ;

  ROScalarColumn<Double> timeCol( tab, "TIME" ) ;
  Vector<Double> timeArr = timeCol.getColumn() ;
  Double minTime ; 
  Double maxTime ;
  minMax( minTime, maxTime, timeArr ) ;
  Double midTime = 0.5 * ( minTime + maxTime ) * 86400.0 ;
  // unit for TIME
  // Scantable: "d"
  // MS: "s"
  me = midTime ;
  interval = ( maxTime - minTime ) * 86400.0 ;

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::getValidTimeRange() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::getValidTimeRange( Double &me, Double &interval, Vector<Double> &atime, Vector<Double> &ainterval ) 
{
//   double startSec = mathutil::gettimeofday_sec() ;
//   os_ << "start MSWriter::getVaridTimeRange() startSec=" << startSec << LogIO::POST ;

  // sort table
  //Table stab = tab.sort( "TIME" ) ;

  Double minTime ; 
  Double maxTime ;
  minMax( minTime, maxTime, atime ) ;
  Double midTime = 0.5 * ( minTime + maxTime ) * 86400.0 ;
  // unit for TIME
  // Scantable: "d"
  // MS: "s"
  me = midTime ;
  interval = ( maxTime - minTime ) * 86400.0 + mean( ainterval ) ;

//   double endSec = mathutil::gettimeofday_sec() ;
//   os_ << "end MSWriter::getValidTimeRange() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

}

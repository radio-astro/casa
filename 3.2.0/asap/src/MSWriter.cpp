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

#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>
#include <casa/OS/SymLink.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/RecordField.h>
#include <casa/Arrays/Cube.h>

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

#include <ctime>
#include <sys/time.h>

using namespace casa ;
using namespace std ;

namespace asap {
double MSWriter::gettimeofday_sec()
{
  struct timeval tv ;
  gettimeofday( &tv, NULL ) ;
  return tv.tv_sec + (double)tv.tv_usec*1.0e-6 ;
}

MSWriter::MSWriter(CountedPtr<Scantable> stable) 
  : table_(stable),
    isTcal_(False),
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
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::write() startSec=" << startSec << LogIO::POST ;

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

  // MAIN
  // Iterate over several ids
  Vector<uInt> processedFreqId( 0 ) ;
  Int defaultFieldId = 0 ;

  // row based
  TableRow row( *mstable_ ) ;
  TableRecord &trec = row.record() ;
  NoticeTarget *dataRF = 0 ;
  if ( useFloatData_ ) 
    dataRF = new RecordFieldPtr< Array<Float> >( trec, "FLOAT_DATA" ) ;
  else if ( useData_ )
    dataRF = new RecordFieldPtr< Array<Complex> >( trec, "DATA" ) ;
  RecordFieldPtr< Array<Bool> > flagRF( trec, "FLAG" ) ;
  RecordFieldPtr<Bool> flagrowRF( trec, "FLAG_ROW" ) ;
  RecordFieldPtr<Double> timeRF( trec, "TIME" ) ;
  RecordFieldPtr<Double> timecRF( trec, "TIME_CENTROID" ) ;
  RecordFieldPtr<Double> intervalRF( trec, "INTERVAL" ) ;
  RecordFieldPtr<Double> exposureRF( trec, "EXPOSURE" ) ;
  RecordFieldPtr< Array<Float> > weightRF( trec, "WEIGHT" ) ;
  RecordFieldPtr< Array<Float> > sigmaRF( trec, "SIGMA" ) ;
  RecordFieldPtr<Int> ddidRF( trec, "DATA_DESC_ID" ) ;
  RecordFieldPtr<Int> stateidRF( trec, "STATE_ID" ) ;
  RecordFieldPtr< Array<Bool> > flagcatRF( trec, "FLAG_CATEGORY" ) ;

  // OBSERVATION_ID is always 0
  RecordFieldPtr<Int> intRF( trec, "OBSERVATION_ID" ) ;
  *intRF = 0 ;
  
  // ANTENNA1 and ANTENNA2 are always 0
  intRF.attachToRecord( trec, "ANTENNA1" ) ;
  *intRF = 0 ;
  intRF.attachToRecord( trec, "ANTENNA2" ) ;
  *intRF = 0 ;
  
  // ARRAY_ID is tentatively set to 0
  intRF.attachToRecord( trec, "ARRAY_ID" ) ;
  *intRF = 0 ;

  // PROCESSOR_ID is tentatively set to 0
  intRF.attachToRecord( trec, "PROCESSOR_ID" ) ;
  *intRF = 0 ;

  // UVW is always [0,0,0]
  RecordFieldPtr< Array<Double> > uvwRF( trec, "UVW" ) ;
  *uvwRF = Vector<Double>( 3, 0.0 ) ;

  //
  // ITERATION: FIELDNAME
  // 
  TableIterator iter0( table_->table(), "FIELDNAME" ) ;
  while( !iter0.pastEnd() ) {
    //Table t0( iter0.table() ) ;
    Table t0 = iter0.table() ;
    ROTableColumn sharedCol( t0, "FIELDNAME" ) ;
    String fieldName = sharedCol.asString( 0 ) ;
    sharedCol.attach( t0, "SRCNAME" ) ;
    String srcName = sharedCol.asString( 0 ) ;
    sharedCol.attach( t0, "TIME" ) ;
    Double minTime = (Double)sharedCol.asdouble( 0 ) * 86400.0 ; // day->sec
    ROArrayColumn<Double> scanRateCol( t0, "SCANRATE" ) ;
    Vector<Double> scanRate = scanRateCol( 0 ) ;
    String::size_type pos = fieldName.find( "__" ) ;
    Int fieldId = -1 ;
    if ( pos != String::npos ) {
//       os_ << "fieldName.substr( pos+2 )=" << fieldName.substr( pos+2 ) << LogIO::POST ;
      fieldId = String::toInt( fieldName.substr( pos+2 ) ) ;
      fieldName = fieldName.substr( 0, pos ) ;
    }
    else {
//       os_ << "use default field id" << LogIO::POST ;
      fieldId = defaultFieldId ;
      defaultFieldId++ ;
    }
//     os_ << "fieldId" << fieldId << ": " << fieldName << LogIO::POST ;

    // FIELD_ID
    intRF.attachToRecord( trec, "FIELD_ID" ) ;
    *intRF = fieldId ;

    //
    // ITERATION: BEAMNO
    //
    TableIterator iter1( t0, "BEAMNO" ) ;
    while( !iter1.pastEnd() ) {
      Table t1 = iter1.table() ;
      sharedCol.attach( t1, "BEAMNO" ) ;
      uInt beamNo = sharedCol.asuInt( 0 ) ;
//       os_ << "beamNo = " << beamNo << LogIO::POST ;

      // FEED1 and FEED2
      intRF.attachToRecord( trec, "FEED1" ) ;
      *intRF = beamNo ;
      intRF.attachToRecord( trec, "FEED2" ) ;
      *intRF = beamNo ;

      // 
      // ITERATION: SCANNO
      //
      TableIterator iter2( t1, "SCANNO" ) ;
      while( !iter2.pastEnd() ) {
        Table t2 = iter2.table() ;
        sharedCol.attach( t2, "SCANNO" ) ;
        uInt scanNo = sharedCol.asuInt( 0 ) ;
//         os_ << "scanNo = " << scanNo << LogIO::POST ;

        // SCAN_NUMBER
        // MS: 1-based
        // Scantable: 0-based
        intRF.attachToRecord( trec, "SCAN_NUMBER" ) ;
        *intRF = scanNo + 1 ;

        // 
        // ITERATION: IFNO
        //
        TableIterator iter3( t2, "IFNO" ) ;
        while( !iter3.pastEnd() ) {
          Table t3 = iter3.table() ;
          sharedCol.attach( t3, "IFNO" ) ;
          uInt ifNo = sharedCol.asuInt( 0 ) ;
//           os_ << "ifNo = " << ifNo << LogIO::POST ;
          sharedCol.attach( t3, "FREQ_ID" ) ;
          uInt freqId = sharedCol.asuInt( 0 ) ;
//           os_ << "freqId = " << freqId << LogIO::POST ;
          Int subscan = 1 ; // 1-base
          // 
          // ITERATION: SRCTYPE
          //
          TableIterator iter4( t3, "SRCTYPE" ) ;
          while( !iter4.pastEnd() ) {
            Table t4 = iter4.table() ;
            sharedCol.attach( t4, "SRCTYPE" ) ;
            Int srcType = sharedCol.asInt( 0 ) ;
            Int stateId = addState( srcType, subscan ) ;
            *stateidRF = stateId ;
            // 
            // ITERATION: CYCLENO and TIME
            //
            Block<String> cols( 2 ) ;
            cols[0] = "CYCLENO" ;
            cols[1] = "TIME" ;
            TableIterator iter5( t4, cols ) ;
            while( !iter5.pastEnd() ) {
              Table t5 =  iter5.table().sort("POLNO") ;
              //sharedCol.attach( t5, "CYCLENO" ) ;
              //uInt cycleNo = sharedCol.asuInt( 0 ) ;
              Int nrow = t5.nrow() ;
//               os_ << "nrow = " << nrow << LogIO::POST ;
              
              Vector<Int> polnos( nrow ) ;
              indgen( polnos, 0 ) ;
              Int polid = addPolarization( polnos ) ;
//               os_ << "polid = " << polid << LogIO::POST ;
              
              // DATA/FLOAT_DATA
              ROArrayColumn<Float> specCol( t5, "SPECTRA" ) ;
              ROArrayColumn<uChar> flagCol( t5, "FLAGTRA" ) ;
              uInt nchan = specCol( 0 ).size() ;
              IPosition cellshape( 2, nrow, nchan ) ;
              if ( useFloatData_ ) {
                // FLOAT_DATA
                Matrix<Float> dataArr( cellshape ) ;
                Matrix<Bool> flagArr( cellshape ) ;
                Vector<Bool> tmpB ;
                for ( Int ipol = 0 ; ipol < nrow ; ipol++ ) {
                  dataArr.row( ipol ) = specCol( ipol ) ;
                  tmpB.reference( flagArr.row( ipol ) ) ;
                  convertArray( tmpB, flagCol( ipol ) ) ;
                }
                ((RecordFieldPtr< Array<Float> > *)dataRF)->define( dataArr ) ; 
                
                // FLAG
                flagRF.define( flagArr ) ;
              }
              else if ( useData_ ) {
                // DATA
                // assume nrow = 4
                Matrix<Complex> dataArr( cellshape ) ;
                Vector<Float> zeroIm( nchan, 0 ) ;
                Matrix<Float> dummy( IPosition( 2, 2, nchan ) ) ;
                dummy.row( 0 ) = specCol( 0 ) ;
                dummy.row( 1 ) = zeroIm ;
                dataArr.row( 0 ) = RealToComplex( dummy ) ;
                dummy.row( 0 ) = specCol( 1 ) ;
                dataArr.row( 3 ) = RealToComplex( dummy ) ;
                dummy.row( 0 ) = specCol( 2 ) ;
                dummy.row( 1 ) = specCol( 3 ) ;
                dataArr.row( 1 ) = RealToComplex( dummy ) ;
                dataArr.row( 2 ) = conj( dataArr.row( 1 ) ) ;
                ((RecordFieldPtr< Array<Complex> > *)dataRF)->define( dataArr ) ; 
                
                
                // FLAG
                Matrix<Bool> flagArr( cellshape ) ;
                Vector<Bool> tmpB ;
                tmpB.reference( flagArr.row( 0 ) ) ;
                convertArray( tmpB, flagCol( 0 ) ) ;
                tmpB.reference( flagArr.row( 3 ) ) ;
                convertArray( tmpB, flagCol( 3 ) ) ;
                tmpB.reference( flagArr.row( 1 ) ) ;
                convertArray( tmpB, ( flagCol( 2 ) | flagCol( 3 ) ) ) ;
                flagArr.row( 2 ) = flagArr.row( 1 ) ;
                flagRF.define( flagArr ) ;
              }

              // FLAG_ROW
              sharedCol.attach( t5, "FLAGROW" ) ;
              Vector<uInt> flagRowArr( nrow ) ;
              for ( Int irow = 0 ; irow < nrow ; irow++ ) 
                flagRowArr[irow] = sharedCol.asuInt( irow ) ;
              *flagrowRF = anyNE( flagRowArr, (uInt)0 ) ;

              // TIME and TIME_CENTROID
              sharedCol.attach( t5, "TIME" ) ;
              Double mTimeV = (Double)sharedCol.asdouble( 0 ) * 86400.0 ; // day -> sec 
              *timeRF = mTimeV ;
              *timecRF = mTimeV ;

              // INTERVAL and EXPOSURE
              sharedCol.attach( t5, "INTERVAL" ) ;
              Double interval = (Double)sharedCol.asdouble( 0 ) ;
              *intervalRF = interval ;
              *exposureRF = interval ;
              
              // WEIGHT and SIGMA
              // always 1 at the moment
              Vector<Float> wArr( nrow, 1.0 ) ;
              weightRF.define( wArr ) ;
              sigmaRF.define( wArr ) ;
              
              // add DATA_DESCRIPTION row
              Int ddid = addDataDescription( polid, ifNo ) ;
//               os_ << "ddid = " << ddid << LogIO::POST ;
              *ddidRF = ddid ;
              
              // for SYSCAL table
              sharedCol.attach( t5, "TCAL_ID" ) ;
              Vector<uInt> tcalIdArr( nrow ) ;
              for ( Int irow = 0 ; irow < nrow ; irow++ )
                tcalIdArr[irow] = sharedCol.asuInt( irow ) ;
//               os_ << "tcalIdArr = " << tcalIdArr << LogIO::POST ;
              String key = String::toString( tcalIdArr[0] ) ;
              if ( !tcalIdRec_.isDefined( key ) ) {
                tcalIdRec_.define( key, tcalIdArr ) ;
                tcalRowRec_.define( key, t5.rowNumbers() ) ;
              }
              else {
                Vector<uInt> pastrows = tcalRowRec_.asArrayuInt( key ) ;
                tcalRowRec_.define( key, concatenateArray( pastrows, t5.rowNumbers() ) ) ;
              }
                            
              // for POINTING table
              if ( ptTabName_ == "" ) {
                ROArrayColumn<Double> dirCol( t5, "DIRECTION" ) ;
                Vector<Double> dir = dirCol( 0 ) ;
                dirCol.attach( t5, "SCANRATE" ) ;
                Vector<Double> rate = dirCol( 0 ) ;
                Matrix<Double> msDir( 2, 1 ) ;
                msDir.column( 0 ) = dir ;
                if ( anyNE( rate, 0.0 ) ) {
                  msDir.resize( 2, 2 ) ;
                  msDir.column( 1 ) = rate ;
                }
                addPointing( fieldName, mTimeV, interval, msDir ) ;
              }
              
              // FLAG_CATEGORY is tentatively set
              flagcatRF.define( Cube<Bool>( nrow, nchan, 1, False ) ) ; 
              
              // add row
              mstable_->addRow( 1, True ) ;
              row.put( mstable_->nrow()-1 ) ;
              
              iter5.next() ;
            }
            
            iter4.next() ;
          }

          // add SPECTRAL_WINDOW row
          if ( allNE( processedFreqId, freqId ) ) {
            uInt vsize = processedFreqId.size() ;
            processedFreqId.resize( vsize+1, True ) ;
            processedFreqId[vsize] = freqId ;
            addSpectralWindow( ifNo, freqId ) ;
          }
          
          iter3.next() ;
        }
        
        iter2.next() ;
      }
      
      // add FEED row
      addFeed( beamNo ) ;
      
      iter1.next() ;
    }
    
    // add FIELD row
    addField( fieldId, fieldName, srcName, minTime, scanRate ) ;

    iter0.next() ;
  }

//   delete tpoolr ;
  delete dataRF ;

  // SYSCAL
  if ( isTcal_ ) 
    fillSysCal() ;

  // fill empty SPECTRAL_WINDOW rows
  infillSpectralWindow() ;

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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::write() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;

  return True ;
}
  
void MSWriter::init()
{
//   os_.origin( LogOrigin( "MSWriter", "init()", WHERE ) ) ;
//   double startSec = gettimeofday_sec() ;
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
  if ( table_->tcal().table().nrow() != 0 ) {
    ROTableColumn col( table_->tcal().table(), "TCAL" ) ;
    if ( col.isDefined( 0 ) ) {
      os_ << "TCAL table exists: nrow=" << table_->tcal().table().nrow() << LogIO::POST ;
      isTcal_ = True ;
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
  if ( isTcal_ && header_.nchan != 1 ) {
    // examine TCAL subtable
    Table tcaltab = table_->tcal().table() ;
    ROArrayColumn<Float> tcalCol( tcaltab, "TCAL" ) ;
    for ( uInt irow = 0 ; irow < tcaltab.nrow() ; irow++ ) {
      if ( tcalCol( irow ).size() != 1 )
        tcalSpec_ = True ;
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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::init() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::setupMS()
{
//   os_.origin( LogOrigin( "MSWriter", "setupMS()", WHERE ) ) ;
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::setupMS() startSec=" << startSec << LogIO::POST ;

  TableDesc msDesc = MeasurementSet::requiredTableDesc() ;
  if ( useFloatData_ )
    MeasurementSet::addColumnToDesc( msDesc, MSMainEnums::FLOAT_DATA, 2 ) ;
  else if ( useData_ )
    MeasurementSet::addColumnToDesc( msDesc, MSMainEnums::DATA, 2 ) ;

  SetupNewTable newtab( filename_, msDesc, Table::New ) ;

  mstable_ = new MeasurementSet( newtab ) ;

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

  // TODO: add TCAL_SPECTRUM and TSYS_SPECTRUM if necessary
  TableDesc sysCalDesc = MSSysCal::requiredTableDesc() ;
  MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TCAL, 2 ) ;
  MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TSYS, 2 ) ;
  if ( tcalSpec_ ) 
    MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TCAL_SPECTRUM, 2 ) ;
  if ( tsysSpec_ )
    MSSysCal::addColumnToDesc( sysCalDesc, MSSysCalEnums::TSYS_SPECTRUM, 2 ) ;
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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::setupMS() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillObservation() 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::fillObservation() startSec=" << startSec << LogIO::POST ;

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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::fillObservation() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillAntenna() 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::fillAntenna() startSec=" << startSec << LogIO::POST ;

  // only 1 row
  mstable_->antenna().addRow( 1, True ) ;
  MSAntennaColumns msAntCols( mstable_->antenna() ) ;

  String hAntennaName = header_.antennaname ;
  String::size_type pos = hAntennaName.find( "//" ) ;
  String antennaName ;
  String stationName ;
  if ( pos != String::npos ) {
    hAntennaName = hAntennaName.substr( pos+2 ) ;
  }
  pos = hAntennaName.find( "@" ) ;
  if ( pos != String::npos ) {
    antennaName = hAntennaName.substr( 0, pos ) ;
    stationName = hAntennaName.substr( pos+1 ) ;
  }
  else {
    antennaName = hAntennaName ;
    stationName = hAntennaName ;
  }
//   os_ << "antennaName = " << antennaName << LogIO::POST ;
//   os_ << "stationName = " << stationName << LogIO::POST ;
  
  msAntCols.name().put( 0, antennaName ) ;
  msAntCols.station().put( 0, stationName ) ;

//   os_ << "antennaPosition = " << header_.antennaposition << LogIO::POST ;
  
  msAntCols.position().put( 0, header_.antennaposition ) ;

  // MOUNT is set to "ALT-AZ"
  msAntCols.mount().put( 0, "ALT-AZ" ) ;

  Double diameter = getDishDiameter( antennaName ) ;
  msAntCols.dishDiameterQuant().put( 0, Quantity( diameter, "m" ) ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::fillAntenna() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillProcessor() 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::fillProcessor() startSec=" << startSec << LogIO::POST ;
  
  // only add empty 1 row
  MSProcessor msProc = mstable_->processor() ;
  msProc.addRow( 1, True ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::fillProcessor() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillSource()
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::fillSource() startSec=" << startSec << LogIO::POST ;
 
  // access to MS SOURCE subtable
  MSSource msSrc = mstable_->source() ;

  // access to MOLECULE subtable
  STMolecules stm = table_->molecules() ;

  Int srcId = 0 ;
  Vector<Double> restFreq ;
  Vector<String> molName ;
  Vector<String> fMolName ;

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
      ROScalarColumn<uInt> molIdCol( t1, "MOLECULE_ID" ) ;
      uInt molId = molIdCol( 0 ) ;
      stm.getEntry( restFreq, molName, fMolName, molId ) ;

      uInt numFreq = restFreq.size() ;
      
      // NUM_LINES
      *numlineRF = numFreq ;

      // REST_FREQUENCY
      *restfreqRF = restFreq ;

      // TRANSITION
      //*transitionRF = fMolName ;
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
      *transitionRF = transition ;

      // SYSVEL 
      Vector<Double> sysvelArr( numFreq, srcVel ) ;
      *sysvelRF = sysvelArr ;

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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::fillSource() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillWeather() 
{
//   double startSec = gettimeofday_sec() ;
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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::fillWeather() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::fillSysCal()
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::fillSysCal() startSec=" << startSec << LogIO::POST ;

  //tcalIdRec_.print( cout ) ;

  // access to MS SYSCAL subtable
  MSSysCal mssc = mstable_->sysCal() ;

  // access to TCAL subtable
  Table stt = table_->tcal().table() ;
  uInt nrow = stt.nrow() ;

  // access to MAIN table
  Block<String> cols( 6 ) ;
  cols[0] = "TIME" ;
  cols[1] = "TCAL_ID" ;
  cols[2] = "TSYS" ;
  cols[3] = "BEAMNO" ;
  cols[4] = "IFNO" ;
  cols[5] = "INTERVAL" ;
  Table tab = table_->table().project( cols ) ;

  if ( nrow == 0 ) 
    return ;

  nrow = tcalIdRec_.nfields() ;

  Double midTime ;
  Double interval ;
  String timeStr ;

  // row base
  TableRow row( mssc ) ;
  TableRecord &trec = row.record() ;
  RecordFieldPtr<Int> antennaRF( trec, "ANTENNA_ID" ) ;
  RecordFieldPtr<Int> feedRF( trec, "FEED_ID" ) ;
  RecordFieldPtr<Int> spwRF( trec, "SPECTRAL_WINDOW_ID" ) ;
  RecordFieldPtr<Double> timeRF( trec, "TIME" ) ;
  RecordFieldPtr<Double> intervalRF( trec, "INTERVAL" ) ;
  RecordFieldPtr< Array<Float> > tsysRF( trec, "TSYS" ) ;
  RecordFieldPtr< Array<Float> > tcalRF( trec, "TCAL" ) ;
  RecordFieldPtr< Array<Float> > tsysspRF ;
  RecordFieldPtr< Array<Float> > tcalspRF ;
  if ( tsysSpec_ )
    tsysspRF.attachToRecord( trec, "TSYS_SPECTRUM" ) ;
  if ( tcalSpec_ )
    tcalspRF.attachToRecord( trec, "TCAL_SPECTRUM" ) ;

  // ANTENNA_ID is always 0
  *antennaRF = 0 ;

  Table sortedstt = stt.sort( "ID" ) ;
  ROArrayColumn<Float> tcalCol( sortedstt, "TCAL" ) ;
  ROTableColumn idCol( sortedstt, "ID" ) ;
  ROArrayColumn<Float> tsysCol( tab, "TSYS" ) ;
  ROTableColumn tcalidCol( tab, "TCAL_ID" ) ;
  ROTableColumn timeCol( tab, "TIME" ) ;
  ROTableColumn intervalCol( tab, "INTERVAL" ) ;
  ROTableColumn beamnoCol( tab, "BEAMNO" ) ;
  ROTableColumn ifnoCol( tab, "IFNO" ) ;
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
//     double t1 = gettimeofday_sec() ;
    Vector<uInt> ids = tcalIdRec_.asArrayuInt( irow ) ;
//     os_ << "ids = " << ids << LogIO::POST ;
    uInt npol = ids.size() ;
    Vector<uInt> rows = tcalRowRec_.asArrayuInt( irow ) ;
//     os_ << "rows = " << rows << LogIO::POST ;
    Vector<Double> atime( rows.nelements() ) ;
    Vector<Double> ainterval( rows.nelements() ) ;
    Vector<uInt> atcalid( rows.nelements() ) ;
    for( uInt jrow = 0 ; jrow < rows.nelements() ; jrow++ ) {
      atime[jrow] = (Double)timeCol.asdouble( rows[jrow] ) ;
      ainterval[jrow] = (Double)intervalCol.asdouble( rows[jrow] ) ;
      atcalid[jrow] = tcalidCol.asuInt( rows[jrow] ) ;
    }
    Vector<Float> dummy = tsysCol( rows[0] ) ;
    Matrix<Float> tsys( npol,dummy.nelements() ) ;
    tsys.row( 0 ) = dummy ;
    for ( uInt jrow = 1 ; jrow < npol ; jrow++ )
      tsys.row( jrow ) = tsysCol( rows[jrow] ) ;

    // FEED_ID
    *feedRF = beamnoCol.asuInt( rows[0] ) ;

    // SPECTRAL_WINDOW_ID
    *spwRF = ifnoCol.asuInt( rows[0] ) ;

    // TIME and INTERVAL
    getValidTimeRange( midTime, interval, atime, ainterval ) ;
    *timeRF = midTime ;
    *intervalRF = interval ;

    // TCAL and TSYS
    Matrix<Float> tcal ;
    Table t ;
    if ( idCol.asuInt( ids[0] ) == ids[0] ) {
//       os_ << "sorted at irow=" << irow << " ids[0]=" << ids[0] << LogIO::POST ;
      Vector<Float> dummyC = tcalCol( ids[0] ) ;
      tcal.resize( npol, dummyC.size() ) ;
      tcal.row( 0 ) = dummyC ;
    }
    else {
//       os_ << "NOT sorted at irow=" << irow << " ids[0]=" << ids[0] << LogIO::POST ;
      t = stt( stt.col("ID") == ids[0] ) ;
      Vector<Float> dummyC = tcalCol( 0 ) ;
      tcal.resize( npol, dummyC.size() ) ;
      tcal.row( 0 ) = dummyC ;
    }
    if ( npol == 2 ) {
      if ( idCol.asuInt( ids[1] ) == ids[1] ) {
//         os_ << "sorted at irow=" << irow << " ids[1]=" << ids[1] << LogIO::POST ;
        tcal.row( 1 ) = tcalCol( ids[1] ) ;
      }
      else {
//         os_ << "NOT sorted at irow=" << irow << " ids[1]=" << ids[1] << LogIO::POST ;
        t = stt( stt.col("ID") == ids[1] ) ;
        tcalCol.attach( t, "TCAL" ) ;
        tcal.row( 1 ) = tcalCol( 1 ) ;
      }
    }
    else if ( npol == 3 ) {
      if ( idCol.asuInt( ids[2] ) == ids[2] )
        tcal.row( 1 ) = tcalCol( ids[2] ) ;
      else {
        t = stt( stt.col("ID") == ids[2] ) ;
        tcalCol.attach( t, "TCAL" ) ;
        tcal.row( 1 ) = tcalCol( 0 ) ;
      }
      if ( idCol.asuInt( ids[1] ) == ids[1] )
        tcal.row( 2 ) = tcalCol( ids[1] ) ;
      else {
        t = stt( stt.col("ID") == ids[1] ) ;
        tcalCol.attach( t, "TCAL" ) ;
        tcal.row( 2 ) = tcalCol( 0 ) ;
      }
    }
    else if ( npol == 4 ) {
      if ( idCol.asuInt( ids[2] ) == ids[2] )
        tcal.row( 1 ) = tcalCol( ids[2] ) ;
      else {
        t = stt( stt.col("ID") == ids[2] ) ;
        tcalCol.attach( t, "TCAL" ) ;
        tcal.row( 1 ) = tcalCol( 0 ) ;
      }
      if ( idCol.asuInt( ids[3] ) == ids[3] )
        tcal.row( 2 ) = tcalCol( ids[3] ) ;
      else {
        t = stt( stt.col("ID") == ids[3] ) ;
        tcalCol.attach( t, "TCAL" ) ;
        tcal.row( 2 ) = tcalCol( 0 ) ;
      }
      if ( idCol.asuInt( ids[1] ) == ids[1] )
        tcal.row( 2 ) = tcalCol( ids[1] ) ;
      else {
        t = stt( stt.col("ID") == ids[1] ) ;
        tcalCol.attach( t, "TCAL" ) ;
        tcal.row( 3 ) = tcalCol( 0 ) ;
      }
    }
    if ( tcalSpec_ ) {
      // put TCAL_SPECTRUM 
      //*tcalspRF = tcal ;
      tcalspRF.define( tcal ) ;
      // set TCAL (mean of TCAL_SPECTRUM)
      Matrix<Float> tcalMean( npol, 1 ) ;
      for ( uInt iid = 0 ; iid < npol ; iid++ ) {
        tcalMean( iid, 0 ) = mean( tcal.row(iid) ) ;
      }
      // put TCAL
      *tcalRF = tcalMean ;
    }
    else {
      // put TCAL
      *tcalRF = tcal ;
    }
    
    if ( tsysSpec_ ) {
      // put TSYS_SPECTRUM
      //*tsysspRF = tsys ;
      tsysspRF.define( tsys ) ;
      // set TSYS (mean of TSYS_SPECTRUM)
      Matrix<Float> tsysMean( npol, 1 ) ;
      for ( uInt iid = 0 ; iid < npol ; iid++ ) {
        tsysMean( iid, 0 ) = mean( tsys.row(iid) ) ;
      }
      // put TSYS
      *tsysRF = tsysMean ;
    }
    else {
      // put TSYS
      *tsysRF = tsys ;
    }

    // add row 
    mssc.addRow( 1, True ) ;
    row.put( mssc.nrow()-1 ) ;

//     double t2 = gettimeofday_sec() ;
//     os_ << irow << "th loop elapsed time = " << t2-t1 << "sec" << LogIO::POST ;
  }
  
//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::fillSysCal() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::addFeed( Int id ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addFeed() startSec=" << startSec << LogIO::POST ;

  // add row
  MSFeed msFeed = mstable_->feed() ;
  msFeed.addRow( 1, True ) ;
  Int nrow = msFeed.nrow() ;
  Int numReceptors = 2 ;
  Vector<String> polType( numReceptors ) ;
  Matrix<Double> beamOffset( 2, numReceptors ) ;
  beamOffset = 0.0 ;
  Vector<Double> receptorAngle( numReceptors, 0.0 ) ;
  if ( polType_ == "linear" ) {
    polType[0] = "X" ;
    polType[1] = "Y" ;
  }
  else if ( polType_ == "circular" ) {
    polType[0] = "R" ;
    polType[1] = "L" ;
  }
  else {
    polType[0] = "X" ;
    polType[1] = "Y" ;
  }
  Matrix<Complex> polResponse( numReceptors, numReceptors, 0.0 ) ;
  for ( Int i = 0 ; i < numReceptors ; i++ )
    polResponse( i, i ) = 0.0 ;

  MSFeedColumns msFeedCols( mstable_->feed() ) ;

  msFeedCols.feedId().put( nrow-1, id ) ;
  msFeedCols.antennaId().put( nrow-1, 0 ) ;
  msFeedCols.numReceptors().put( nrow-1, numReceptors ) ;
  msFeedCols.polarizationType().put( nrow-1, polType ) ;
  msFeedCols.beamOffset().put( nrow-1, beamOffset ) ;
  msFeedCols.receptorAngle().put( nrow-1, receptorAngle ) ;
  msFeedCols.polResponse().put( nrow-1, polResponse ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addFeed() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::addSpectralWindow( Int spwid, Int freqid ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addSpectralWindow() startSec=" << startSec << LogIO::POST ;
  
  // add row
  MSSpectralWindow msSpw = mstable_->spectralWindow() ;
  while( (Int)msSpw.nrow() <= spwid ) {
    msSpw.addRow( 1, True ) ;
  }
  
  MSSpWindowColumns msSpwCols( msSpw ) ;

  STFrequencies stf = table_->frequencies() ;

  // MEAS_FREQ_REF
  msSpwCols.measFreqRef().put( spwid, stf.getFrame( True ) ) ;

  Double refpix ;
  Double refval ;
  Double inc ;
  stf.getEntry( refpix, refval, inc, (uInt)freqid ) ;

  // NUM_CHAN
  Int nchan = (Int)(refpix * 2) + 1 ;
  if ( nchan == 0 )
    nchan = 1 ;
  msSpwCols.numChan().put( spwid, nchan ) ;

  // TOTAL_BANDWIDTH
  Double bw = nchan * inc ;
  msSpwCols.totalBandwidth().put( spwid, bw ) ;

  // REF_FREQUENCY
  Double refFreq = refval - refpix * inc ;
  msSpwCols.refFrequency().put( spwid, refFreq ) ;

  // NET_SIDEBAND
  // tentative: USB->0, LSB->1
  Int netSideband = 0 ;
  if ( inc < 0 ) 
    netSideband = 1 ;
  msSpwCols.netSideband().put( spwid, netSideband ) ;

  // RESOLUTION, CHAN_WIDTH, EFFECTIVE_BW
  Vector<Double> sharedDoubleArr( nchan, inc ) ;
  msSpwCols.resolution().put( spwid, sharedDoubleArr ) ;
  msSpwCols.chanWidth().put( spwid, sharedDoubleArr ) ;
  msSpwCols.effectiveBW().put( spwid, sharedDoubleArr ) ;

  // CHAN_FREQ
  indgen( sharedDoubleArr, refFreq, inc ) ;
  msSpwCols.chanFreq().put( spwid, sharedDoubleArr ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addSpectralWindow() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::addField( Int fid, String fieldname, String srcname, Double t, Vector<Double> rate ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addField() startSec=" << startSec << LogIO::POST ;
 
  MSField msField = mstable_->field() ;
  while( (Int)msField.nrow() <= fid ) {
    msField.addRow( 1, True ) ;
  }
  MSFieldColumns msFieldCols( msField ) ;

  // Access to SOURCE table
  MSSource msSrc = mstable_->source() ;

  // fill target row
  msFieldCols.name().put( fid, fieldname ) ;
  msFieldCols.time().put( fid, t ) ;
  Int numPoly = 0 ;
  if ( anyNE( rate, 0.0 ) ) 
    numPoly = 1 ;
  msFieldCols.numPoly().put( fid, numPoly ) ;
  MSSourceIndex msSrcIdx( msSrc ) ;
  Int srcId = -1 ;
  Vector<Int> srcIdArr = msSrcIdx.matchSourceName( srcname ) ;
  if ( srcIdArr.size() != 0 ) {
    srcId = srcIdArr[0] ;
    MSSource msSrcSel = msSrc( msSrc.col("SOURCE_ID") == srcId ) ;
    ROMSSourceColumns msSrcCols( msSrcSel ) ;
    Vector<Double> srcDir = msSrcCols.direction()( 0 ) ;
    Matrix<Double> srcDirA( IPosition( 2, 2, 1+numPoly ) ) ;
//     os_ << "srcDirA = " << srcDirA << LogIO::POST ;
//     os_ << "sliced srcDirA = " << srcDirA.column( 0 ) << LogIO::POST ;
    srcDirA.column( 0 ) = srcDir ;
//     os_ << "srcDirA = " << srcDirA << LogIO::POST ;
    if ( numPoly != 0 ) 
      srcDirA.column( 1 ) = rate ;
    msFieldCols.phaseDir().put( fid, srcDirA ) ;
    msFieldCols.referenceDir().put( fid, srcDirA ) ;
    msFieldCols.delayDir().put( fid, srcDirA ) ;
  }
  msFieldCols.sourceId().put( fid, srcId ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addField() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::addPointing( String &name, Double &me, Double &interval, Matrix<Double> &dir ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addPointing() startSec=" << startSec << LogIO::POST ;
  
  // access to POINTING subtable
  MSPointing msp = mstable_->pointing() ;
  uInt nrow = msp.nrow() ;

  // add row
  msp.addRow( 1, True ) ;

  // fill row
  TableRow row( msp ) ;
  TableRecord &rec = row.record() ;
  RecordFieldPtr<Int> antennaRF( rec, "ANTENNA_ID" ) ;
  *antennaRF = 0 ;
  RecordFieldPtr<Int> numpolyRF( rec, "NUM_POLY" ) ;
  *numpolyRF = dir.ncolumn() ;
  RecordFieldPtr<Double> timeRF( rec, "TIME" ) ;
  *timeRF = me ;
  RecordFieldPtr<Double> toriginRF( rec, "TIME_ORIGIN" ) ;
  *toriginRF = me ;
  RecordFieldPtr<Double> intervalRF( rec, "INTERVAL" ) ;
  *intervalRF = interval ;
  RecordFieldPtr<String> nameRF( rec, "NAME" ) ;
  *nameRF = name ;
  RecordFieldPtr<Bool> trackRF( rec, "TRACKING" ) ;
  *trackRF = True ;
  RecordFieldPtr< Array<Double> > dirRF( rec, "DIRECTION" ) ;
  *dirRF = dir ;
  RecordFieldPtr< Array<Double> > targetRF( rec, "TARGET" ) ;
  *dirRF = dir ;
  row.put( nrow ) ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addPointing() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

Int MSWriter::addPolarization( Vector<Int> polnos ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addPolarization() startSec=" << startSec << LogIO::POST ;

//   os_ << "polnos = " << polnos << LogIO::POST ;
  MSPolarization msPol = mstable_->polarization() ;
  uInt nrow = msPol.nrow() ;

//   // only 1 POLARIZATION row for 1 scantable
//   if ( nrow > 0 )
//     return 0 ;
  
  Vector<Int> corrType = toCorrType( polnos ) ;
  
  ROArrayColumn<Int> corrtCol( msPol, "CORR_TYPE" ) ;
  //Matrix<Int> corrTypeArr = corrtCol.getColumn() ;
  Int polid = -1 ;
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
    Vector<Int> corrTypeArr = corrtCol( irow ) ;
    if ( corrType.nelements() == corrTypeArr.nelements() 
         && allEQ( corrType, corrTypeArr ) ) {
      polid = irow ;
      break ;
    }
  }

  if ( polid == -1 ) {
    MSPolarizationColumns msPolCols( msPol ) ;

    // add row
    msPol.addRow( 1, True ) ;
    polid = (Int)nrow ;

    // CORR_TYPE
    msPolCols.corrType().put( nrow, corrType ) ;

    // NUM_CORR
    uInt npol = corrType.size() ;
    msPolCols.numCorr().put( nrow, npol ) ;

    // CORR_PRODUCT
    Matrix<Int> corrProd( 2, npol, -1 ) ;
    if ( npol == 1 ) {
      corrProd = 0 ;
    }
    else if ( npol == 2 ) {
      corrProd.column( 0 ) = 0 ;
      corrProd.column( 1 ) = 1 ;
    }
    else {
      corrProd.column( 0 ) = 0 ;
      corrProd.column( 3 ) = 1 ;
      corrProd( 0,1 ) = 0 ;
      corrProd( 1,1 ) = 1 ;
      corrProd( 0,2 ) = 1 ;
      corrProd( 1,2 ) = 0 ;
    }
    msPolCols.corrProduct().put( nrow, corrProd ) ;    
  }

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addPolarization() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;

  return polid ;
} 

Int MSWriter::addDataDescription( Int polid, Int spwid ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addDataDescription() startSec=" << startSec << LogIO::POST ;

  MSDataDescription msDataDesc = mstable_->dataDescription() ;
  uInt nrow = msDataDesc.nrow() ;

  // only 1 POLARIZATION_ID for 1 scantable
  Int ddid = -1 ;
  ROScalarColumn<Int> spwCol( msDataDesc, "SPECTRAL_WINDOW_ID" ) ;
  Vector<Int> spwIds = spwCol.getColumn() ;
  //ROScalarColumn<Int> polCol( msDataDesc, "POLARIZATION_ID" ) ;
  //Vector<Int> polIds = polCol.getColumn() ;
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
    //if ( spwid == spwIds[irow] && polid == polIds[irow] ) {
    if ( spwid == spwIds[irow] ) {
      ddid = irow ;
      break ;
    }
  }
//   os_ << "ddid = " << ddid << LogIO::POST ;
  

  if ( ddid == -1 ) {
    msDataDesc.addRow( 1, True ) ;
    MSDataDescColumns msDataDescCols( msDataDesc ) ;
    msDataDescCols.polarizationId().put( nrow, polid ) ;
    msDataDescCols.spectralWindowId().put( nrow, spwid ) ;
    ddid = (Int)nrow ;
  }

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addDataDescription() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;

  return ddid ;
}

Int MSWriter::addState( Int st, Int &subscan ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::addState() startSec=" << startSec << LogIO::POST ;

  // access to STATE subtable
  MSState msState = mstable_->state() ;
  uInt nrow = msState.nrow() ;

  String obsMode ;
  Bool isSignal ;
  Double tnoise ;
  Double tload ;
  queryType( st, obsMode, isSignal, tnoise, tload ) ;
//   os_ << "obsMode = " << obsMode << " isSignal = " << isSignal << LogIO::POST ;

  Int idx = -1 ;
  ROScalarColumn<String> obsModeCol( msState, "OBS_MODE" ) ;
  ROScalarColumn<Int> subscanCol( msState, "SUB_SCAN" ) ;
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
    if ( obsModeCol(irow) == obsMode 
         //&& sigCol(irow) == isSignal 
         //&& refCol(irow) != isSignal
         && subscanCol(irow) == subscan ) {
      idx = irow ;
      break ;
    }
  }
  if ( idx == -1 ) {
    msState.addRow( 1, True ) ;
    TableRow row( msState ) ;
    TableRecord &rec = row.record() ;
    RecordFieldPtr<String> obsmodeRF( rec, "OBS_MODE" ) ;
    *obsmodeRF = obsMode ;
    RecordFieldPtr<Bool> sigRF( rec, "SIG" ) ;
    *sigRF = isSignal ;
    RecordFieldPtr<Bool> refRF( rec, "REF" ) ;
    *refRF = !isSignal ;
    RecordFieldPtr<Int> subscanRF( rec, "SUB_SCAN" ) ;
    *subscanRF = subscan ;
    RecordFieldPtr<Double> noiseRF( rec, "CAL" ) ;
    *noiseRF = tnoise ;
    RecordFieldPtr<Double> loadRF( rec, "LOAD" ) ;
    *loadRF = tload ;
    row.put( nrow ) ;
    idx = nrow ;
  }
  subscan++ ;

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::addState() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;

  return idx ;
}

Vector<Int> MSWriter::toCorrType( Vector<Int> polnos ) 
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::toCorrType() startSec=" << startSec << LogIO::POST ;

  uInt npol = polnos.size() ;
  Vector<Int> corrType( npol, Stokes::Undefined ) ;
  
  if ( npol == 4 ) {
    if ( polType_ == "linear" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 0 )
          corrType[ipol] = Stokes::XX ;
        else if ( polnos[ipol] == 1 )
          corrType[ipol] = Stokes::XY ;
        else if ( polnos[ipol] == 2 ) 
          corrType[ipol] = Stokes::YX ;
        else if ( polnos[ipol] == 3 ) 
          corrType[ipol] = Stokes::YY ;
      }
    }
    else if ( polType_ == "circular" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 0 )
          corrType[ipol] = Stokes::RR ;
        else if ( polnos[ipol] == 1 )
          corrType[ipol] = Stokes::RL ;
        else if ( polnos[ipol] == 2 ) 
          corrType[ipol] = Stokes::LR ;
        else if ( polnos[ipol] == 3 ) 
          corrType[ipol] = Stokes::LL ;
      }
    }
    else if ( polType_ == "stokes" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 0 )
          corrType[ipol] = Stokes::I ;
        else if ( polnos[ipol] == 1 )
          corrType[ipol] = Stokes::Q ;
        else if ( polnos[ipol] == 2 ) 
          corrType[ipol] = Stokes::U ;
        else if ( polnos[ipol] == 3 ) 
          corrType[ipol] = Stokes::V ;
      }
    }
  }
  else if ( npol == 2 ) {
    if ( polType_ == "linear" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 0 )
          corrType[ipol] = Stokes::XX ;
        else if ( polnos[ipol] == 1 )
          corrType[ipol] = Stokes::YY ;
      }
    }
    else if ( polType_ == "circular" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 0 )
          corrType[ipol] = Stokes::RR ;
        else if ( polnos[ipol] == 1 ) 
          corrType[ipol] = Stokes::LL ;
      }
    }
    else if ( polType_ == "stokes" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 0 )
          corrType[ipol] = Stokes::I ;
        else if ( polnos[ipol] == 1 ) 
          corrType[ipol] = Stokes::V ;
      }
    }
    else if ( polType_ == "linpol" ) {
      for ( uInt ipol = 0 ; ipol < npol ; ipol++ ) {
        if ( polnos[ipol] == 1 )
          corrType[ipol] = Stokes::Plinear ;
        else if ( polnos[ipol] == 2 ) 
          corrType[ipol] = Stokes::Pangle ;
      }
    }
  }      
  else if ( npol == 1 ) {
    if ( polType_ == "linear" )
      corrType[0] = Stokes::XX ;
    else if ( polType_ == "circular" )
      corrType[0] = Stokes::RR ;
    else if ( polType_ == "stokes" ) 
      corrType[0] = Stokes::I ;
  }

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::toCorrType() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;

  return corrType ;
}

void MSWriter::getValidTimeRange( Double &me, Double &interval, Table &tab ) 
{
//   double startSec = gettimeofday_sec() ;
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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::getValidTimeRange() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

void MSWriter::getValidTimeRange( Double &me, Double &interval, Vector<Double> &atime, Vector<Double> &ainterval ) 
{
//   double startSec = gettimeofday_sec() ;
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

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::getValidTimeRange() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

//void MSWriter::queryType( Int type, String &stype, Bool &b )
void MSWriter::queryType( Int type, String &stype, Bool &b, Double &t, Double &l )
{
//   double startSec = gettimeofday_sec() ;
//   os_ << "start MSWriter::queryType() startSec=" << startSec << LogIO::POST ;

  // 2011/03/14 TN
  // OBS_MODE string of MS created by importasdm task is slightly 
  // (but critically) changed.
  switch ( type ) {
  case SrcType::PSON:
    stype = "OBSERVE_TARGET_ON_SOURCE,POSITION_SWITCH" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::PSOFF:
    stype = "OBSERVE_TARGET_OFF_SOURCE,POSITION_SWITCH" ;
    b = False ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::NOD:
    stype = "OBSERVE_TARGET_ON_SOURCE,NOD" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FSON:
    stype = "OBSERVE_TARGET_ON_SOURCE,FREQUENCY_SWITCH_SIG" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FSOFF:
    stype = "OBSERVE_TARGET_ON_SOURCE,FREQUENCY_SWITCH_REF" ;
    b = False ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::SKY:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,UNSPECIFIED" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::HOT:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,UNSPECIFIED" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::WARM:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,UNSPECIFIED" ;
    t = 0.0 ;
    b = False ;
    l = 1.0 ;
    break ;
  case SrcType::COLD:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,UNSPECIFIED" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::PONCAL:
    stype = "CALIBRATE_TEMPERATURE_ON_SOURCE,POSITION_SWITCH" ;
    b = True ;
    t = 1.0 ;
    l = 0.0 ;
    break ;
  case SrcType::POFFCAL:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,POSITION_SWITCH" ;
    b = False ;
    t = 1.0 ;
    l = 0.0 ;
    break ;
  case SrcType::NODCAL:
    stype = "CALIBRATE_TEMPERATURE_ON_SOURCE,NOD" ;
    b = True ;
    t = 1.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FONCAL:
    stype = "CALIBRATE_TEMPERATURE_ON_SOURCE,FREQUENCY_SWITCH_SIG" ;
    b = True ;
    t = 1.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FOFFCAL:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_REF" ;
    b = False ;
    t = 1.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FSLO:
    stype = "OBSERVE_TARGET_ON_SOURCE,FREQUENCY_SWITCH_LOWER" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FLOOFF:
    stype = "OBSERVE_TARGET_OFF_SOURCE,FREQUENCY_SWITCH_LOWER" ;
    b = False ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FLOSKY:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_LOWER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FLOHOT:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_LOWER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FLOWARM:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_LOWER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FLOCOLD:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_LOWER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FSHI:
    stype = "OBSERVE_TARGET_ON_SOURCE,FREQUENCY_SWITCH_HIGHER" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FHIOFF:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_HIGHER" ;
    b = False ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::FHISKY:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_HIGHER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FHIHOT:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_HIGHER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FHIWARM:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_HIGHER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::FHICOLD:
    stype = "CALIBRATE_TEMPERATURE_OFF_SOURCE,FREQUENCY_SWITCH_HIGHER" ;
    b = False ;
    t = 0.0 ;
    l = 1.0 ;
    break ;
  case SrcType::SIG:
    stype = "OBSERVE_TARGET_ON_SOURCE,UNSPECIFIED" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  case SrcType::REF:
    stype = "OBSERVE_TARGET_ON_SOURCE,UNSPECIFIED" ;
    b = False ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  default:
    stype = "UNSPECIFIED" ;
    b = True ;
    t = 0.0 ;
    l = 0.0 ;
    break ;
  }

//   double endSec = gettimeofday_sec() ;
//   os_ << "end MSWriter::queryType() endSec=" << endSec << " (" << endSec-startSec << "sec)" << LogIO::POST ;
}

Double MSWriter::getDishDiameter( String antname )
{
  Double diameter = 0.0 ;
  
  antname.upcase() ;

  if ( antname.matches( Regex( "DV[0-9]+$" ) ) 
       || antname.matches( Regex( "DA[0-9]+$" ) )
       || antname.matches( Regex( "PM[0-9]+$" ) ) )
    diameter = 12.0 ;
  else if ( antname.matches( Regex( "CM[0-9]+$" ) ) )
    diameter = 7.0 ;
  else if ( antname.contains( "GBT" ) ) 
    diameter = 104.9 ;
  else if ( antname.contains( "MOPRA" ) )
    diameter = 22.0 ;
  else if ( antname.contains( "PKS" ) || antname.contains( "PARKS" ) )
    diameter = 64.0 ;
  else if ( antname.contains( "TIDBINBILLA" ) ) 
    diameter = 70.0 ;
  else if ( antname.contains( "CEDUNA" ) )
    diameter = 30.0 ;
  else if ( antname.contains( "HOBART" ) ) 
    diameter = 26.0 ;
  else if ( antname.contains( "APEX" ) ) 
    diameter = 12.0 ;
  else if ( antname.contains( "ASTE" ) ) 
    diameter = 10.0 ;
  else if ( antname.contains( "NRO" ) ) 
    diameter = 45.0 ;
  else 
    diameter = 1.0 ;

  return diameter ;
}

void MSWriter::infillSpectralWindow()
{
  MSSpectralWindow msSpw = mstable_->spectralWindow() ;
  MSSpWindowColumns msSpwCols( msSpw ) ;
  uInt nrow = msSpw.nrow() ;

  ScalarColumn<Int> measFreqRefCol = msSpwCols.measFreqRef() ;
  ArrayColumn<Double> chanFreqCol = msSpwCols.chanFreq() ;
  ArrayColumn<Double> chanWidthCol = msSpwCols.chanWidth() ;
  ArrayColumn<Double> effectiveBWCol = msSpwCols.effectiveBW() ;
  ArrayColumn<Double> resolutionCol = msSpwCols.resolution() ;
  Vector<Double> dummy( 1, 0.0 ) ;
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
    if ( !(chanFreqCol.isDefined( irow )) ) {
      measFreqRefCol.put( irow, 1 ) ;
      chanFreqCol.put( irow, dummy ) ;
      chanWidthCol.put( irow, dummy ) ;
      effectiveBWCol.put( irow, dummy ) ;
      resolutionCol.put( irow, dummy ) ;
    }
  }

}

}

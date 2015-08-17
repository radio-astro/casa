//# MSAnalysis.cc: MS analysis and handling tool
//# Copyright (C) 2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA

// include
#include <msvis/MSVis/MSAnalysis.h>
#include <msvis/MSVis/MSMoments.h>
#include <casa/Exceptions/Error.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MSSel/MSSelection.h>
#include <ms/MeasurementSets/MSIter.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MSOper/MSConcat.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/Time.h>
#include <casa/OS/File.h>
#include <casa/Containers/Block.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/RefRows.h>
#include <casa/Utilities/CountedPtr.h>

namespace casa { // namespace casa begins

MSAnalysis::MSAnalysis( const MeasurementSet *inMS )
  : ms_p( 0 ),
    tableName_( "" ),
    tableIn_( "" ) 
{
  itsLog = new LogIO() ;
  init( inMS ) ;
}

MSAnalysis::~MSAnalysis() 
{
  cleanup() ;
}

//init
void MSAnalysis::init( const MeasurementSet *inMS ) 
{
  if ( tableIn_ != "" || inMS->tableName() == tableIn_ ) {
    return ;
  }

  tableIn_ = inMS->tableName() ;
  tableName_ = tableIn_ ;

  ms_p = new MeasurementSet( tableName_ ) ;
  
  if ( !(ms_p->isColumn( MSMainEnums::FLOAT_DATA )) ) {
    delete ms_p ;
    ms_p = 0 ;
    *itsLog << LogIO::SEVERE 
            << "MSAnalysis only accepts MS with FLOAT_DATA" 
            << LogIO::EXCEPTION ;
    return ;
  }
} 

// setMS
void MSAnalysis::setMS( MeasurementSet *inMS ) 
{
  if ( tableIn_ == "" ) {
    init( inMS ) ;
  }
  else if ( inMS->tableName() == tableName_ ) {
    return ;
  }
  else {
    //setMS( inMS->tableName() ) ;
    cleanup() ;
    tableName_ = inMS->tableName() ;
    ms_p = inMS ;
  }
}

void MSAnalysis::setMS( const String name ) 
{
  if ( name == tableName_ ) {
    return ;
  }

  cleanup() ;
  tableName_ = name ;
  ms_p = new MeasurementSet( tableName_, Table::Update ) ;
}

// release memory & remove temporary file
void MSAnalysis::cleanup() 
{
  if ( ms_p != 0 ) {
    delete ms_p ;
    ms_p = 0 ;
  }
  if ( tableName_ != "" && tableName_ != tableIn_ ) {
    File tableDir( tableName_ ) ;
    if ( tableDir.exists() && Table::canDeleteTable( tableName_, True ) ) {
      Table::deleteTable( tableName_, True ) ;
    }
  }
}

// moment calculation
MeasurementSet* MSAnalysis::moments( const Vector<Int> &whichmoments,
                                     //const String &mask,
                                     const String &antenna,
                                     const String &field,
                                     const String &spw,
                                     const Vector<String> &method,
                                     const Vector<Int> &smoothAxes,
                                     const Vector<String> &kernels,
                                     const Vector<Quantity> &kernelWidths,
                                     const Vector<Float> &/*includepix*/,
                                     const Vector<Float> &/*excludepix*/,
                                     const Double peaksnr,
                                     const Double stddev,
                                     const String &velocityType,
                                     const String &out,
                                     const String &smoothOut,
                                     //const String &pgdevice,
                                     //const Int nx,
                                     //const Int ny,
                                     //const Bool yind,
                                     const Bool overwrite
                                     ) 
{
  *itsLog << LogOrigin( "MSMoments", "moments" ) ;
  uInt numMoments = whichmoments.nelements() ;
  PtrBlock<MeasurementSet *> moments ;
  moments.resize( numMoments ) ;
  for ( uInt i = 0 ; i < numMoments ; i++ )
    moments[i] = 0 ;
  PtrBlock<MeasurementSet *> msMoments ;

  try {
    // check if ms_p is not empty
    if ( ms_p == 0 ) {
      *itsLog << "No MS data set" << LogIO::EXCEPTION ;
    }

    // data selection
    selectMS( antenna, field, spw ) ;
    
    // smoothing is not implemented yet
    Bool doSmooth = False ;
    if ( !smoothOut.empty() or ( smoothAxes.nelements() != 0 ) 
         or ( kernels.nelements() != 0 ) or ( kernelWidths.nelements() != 0 ) ) {
      *itsLog << LogIO::WARN << "Smoothing is not implemented yet. Disabled." << LogIO::POST ;
    }

//     if ( !pgdevice.empty() ) {
//       *itsLog << LogIO::WARN << "Making plots is not implemented yet. Disabled." << LogIO::POST ;
//     }

    // check that we can write to smoothout if specified
    if (!smoothOut.empty() and !overwrite) {
      NewFile validfile;
      String errmsg;
      if (!validfile.valueOK(smoothOut, errmsg)) {
        *itsLog << errmsg << LogIO::EXCEPTION;
      }
    }

    // doTemp
    Bool doTemp = True ;

    //
    // loop on FIELD_ID, DATA_DESC_ID, ANTENNA1
    //
//     Block<Int> sortCol( 3 ) ;
//     sortCol[0] = MeasurementSet::ANTENNA1 ;
//     sortCol[1] = MeasurementSet::DATA_DESC_ID ;
//     sortCol[2] = MeasurementSet::FIELD_ID ;
    Block<Int> sortCol( 1 ) ;
    sortCol[0] = MeasurementSet::DATA_DESC_ID ;
    MSIter iter( *ms_p, sortCol, 0.0, False, False ) ;
    vector<String> selList ;
//     *itsLog << LogIO::NORMAL 
//             << "Split MS by FIELD_ID, DATA_DESC_ID, and ANTENNA1"  
//             << LogIO::POST ;
    ///// unused Bool done = False ;
    uInt numIter = 0 ;
    for ( iter.origin() ; iter.more() ; iter++ ) {
      PtrBlock<MeasurementSet *> tmpMoments ;
      MeasurementSet tmpMS( iter.table() ) ;

      // if NUM_CHAN is 1, skip
      ROArrayColumn<Float> floatDataCol( tmpMS, "FLOAT_DATA" ) ;
      if ( (floatDataCol.shape( 0 ))[1] == 1 ) {
        *itsLog << "NUM_CHAN is 1. Nothing to do." << LogIO::POST ;
        continue ;
      }

      // if all FLAG_ROW is True, skip
      ROScalarColumn<Bool> rowFlagCol( tmpMS, "FLAG_ROW" ) ;
      if ( allEQ( rowFlagCol.getColumn(), True ) ) {
        *itsLog << "All rows are flagged. Skip." << LogIO::POST ;
        continue ;
      }
      
      // create MSMoments object 
      CountedPtr< MSMoments<Float> > momentMaker = new MSMoments<Float>( tmpMS, *itsLog, True, False ) ; 

      // Set which moments to output
      if (!momentMaker->setMoments(whichmoments + 1)) {
        *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
      }
      
      // Set moment axis
      // fixed to SpectralCoordinate axis
      const CoordinateSystem cSys = momentMaker->coordinates() ;
      Int spectralAxis = CoordinateUtil::findSpectralAxis( cSys ) ;
      if ( !momentMaker->setMomentAxis( spectralAxis ) ) {
        *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
      }
      
      // Set moment methods
      if (method.nelements() > 0 && method(0) != "") {
        String tmp;
        for (uInt i = 0; i < method.nelements(); i++) {
          tmp += method(i) + " ";
        }
        Vector<Int> intmethods = MomentsBase<Float>::toMethodTypes(tmp);
        if (!momentMaker->setWinFitMethod(intmethods)) {
          *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
        }
      }
      
      // Set smoothing
      if (kernels.nelements() >= 1 && kernels(0) != "" && smoothAxes.size() >= 1
          && kernelWidths.nelements() >= 1 && doSmooth ) {
        String tmp;
        for (uInt i = 0; i < kernels.nelements(); i++) {
          tmp += kernels(i) + " ";
        }
        Vector<Int> intkernels = VectorKernel::toKernelTypes(kernels);
        Vector<Int> intaxes(smoothAxes);
        if (!momentMaker->setSmoothMethod(intaxes, intkernels, kernelWidths)) {
          *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
        }
      }
      
      // Set pixel include/exclude range
//       if (!momentMaker->setInExCludeRange(includepix, excludepix)) {
//         *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
//       }
      
      // Set SNR cutoff
      if (!momentMaker->setSnr(peaksnr, stddev)) {
        *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
      }
      
      // Set velocity type
      if (!velocityType.empty()) {
        MDoppler::Types velType;
        if (!MDoppler::getType(velType, velocityType)) {
          *itsLog << LogIO::WARN << "Illegal velocity type, using RADIO"
                  << LogIO::POST;
          velType = MDoppler::RADIO;
        }
        momentMaker->setVelocityType(velType);
      }
      
      // Create moments
      if (!momentMaker->createMoments(tmpMoments, doTemp, "")) {
        *itsLog << momentMaker->errorMessage() << LogIO::EXCEPTION;
      }
      uInt oldSize = msMoments.size() ;
      msMoments.resize( oldSize+tmpMoments.size() ) ;
      for ( uInt j = 0 ; j < tmpMoments.size() ; j++ ) {
        msMoments[oldSize+j] = tmpMoments[j] ;
        tmpMoments[j] = 0 ;
      }
      numIter++ ;
    } 
    // end of the MSIter loop

    if ( msMoments.nelements() == 0 ) {
      *itsLog << LogIO::WARN << "Moments not generated." << LogIO::POST ;
      return 0 ;
    }
    else if ( msMoments.size() > numMoments ) {
      *itsLog << LogIO::NORMAL << "Concatenate output MS" << LogIO::POST ;
      for ( uInt i = 0 ; i < numMoments ; i++ ) {
        //MSConcat msconcat( *(msMoments[i]) ) ;
        MSConcat *msconcat = 0 ;
        Int idx = -1 ;
        for ( uInt j = 0 ; j < numIter ; j++ ) {
          if ( msMoments[i+j*numMoments] == 0 ) {
            continue ;
          }
          else if ( msconcat == 0 ) {
            msconcat = new MSConcat( *(msMoments[i]) ) ;
            idx = i + j * numMoments ;
          }
          else {
            msconcat->concatenate( *(msMoments[i+j*numMoments]) ) ;
          }
        }
        delete msconcat ;
        msconcat = 0 ;
        // output MS
        if ( idx == -1 ) {
          //*itsLog << LogIO::SEVERE << "Failed to generate moments." << LogIO::EXCEPTION ;
          throw AipsError( "Failed to generate moments." ) ;
          return 0 ;
        }
        String msName = msMoments[idx]->tableName() ;
        String suffix = msName.substr( msName.rfind( '.', msName.size()-1 ), msName.size()-1 ) ;
        String outTable = "" ;
        if ( out == "" ) {
          outTable = tableIn_ + suffix ;
        }
        else {
          if ( numMoments == 1 )
            outTable = out ;
          else 
            outTable = out + suffix ;
        }
        if ( !overwrite ) {
          File fOut( outTable ) ;
          if ( fOut.exists() ) {
            //*itsLog << LogIO::SEVERE << outTable << " exists." << LogIO::EXCEPTION ;
            throw AipsError( outTable + " exists." ) ;
          }
        }          
        msMoments[i]->deepCopy( outTable, Table::New, True ) ;
        addHistory( outTable, suffix.substr(1,suffix.size()-1) ) ;
        if ( i == 0 )
          moments[i] = new MeasurementSet( outTable ) ;
        *itsLog << "Created " << outTable << LogIO::POST ;
      }
    }
    else {
      for ( uInt i = 0 ; i < numMoments ; i++ ) {
        if ( msMoments[i] == 0 ) {
          //*itsLog << LogIO::SEVERE << "Failed to generate moments." << LogIO::EXCEPTION ;
          throw AipsError( "Failed to generate moments." ) ;
          return 0 ;
        }
        // output MS
        String msName = msMoments[i]->tableName() ;
        String suffix = msName.substr(  msName.rfind( '.', msName.size()-1 ), msName.size()-1 ) ;
        String outTable = "" ;
        if ( out == "" ) {
          outTable = tableIn_ + suffix ;
        }
        else {
          if ( numMoments == 1 ) 
            outTable = out ;
          else 
            outTable = out + suffix ;
        }
        if ( !overwrite ) {
          File fOut( outTable ) ;
          if ( fOut.exists() ) {
            //*itsLog << LogIO::SEVERE << outTable << " exists." << LogIO::EXCEPTION ;
            throw AipsError( outTable + " exists." ) ;
          }
        }          
        msMoments[i]->deepCopy( outTable, Table::New, True ) ;
        addHistory( outTable, suffix.substr(1,suffix.size()-1) ) ;
        if ( i == 0 ) 
          moments[i] = new MeasurementSet( outTable ) ;
        *itsLog << "Created " << outTable << LogIO::POST ;
      }
    }
  }
  catch ( AipsError e ) {
    for ( uInt i = 0 ; i < msMoments.size() ; i++ ) {
      if ( msMoments[i] != 0 ) {
        File f( msMoments[i]->tableName() ) ;
        if ( f.exists() && Table::canDeleteTable( msMoments[i]->tableName() ) ) 
          Table::deleteTable( msMoments[i]->tableName(), True ) ;
      }
    }
    cleanup() ;
    RETHROW( e ) ;
  }

  // clean up
  for ( uInt i = 0 ; i < msMoments.size() ; i++ ) {
    if ( msMoments[i] != 0 ) {
      String tabName = msMoments[i]->tableName() ;
      delete msMoments[i] ;
      msMoments[i] = 0 ;
      if ( Table::canDeleteTable( tabName ) ) {
        Table::deleteTable( tabName, True ) ;
      }
    }
  }
  
  return moments[0] ;
}
  
void MSAnalysis::selectMS( const String antenna, const String field, const String spw )
{
  // Parse antenna, field, spw
  MSSelection sel ;
  Bool doAntenna = False ;
  Bool doField = False ;
  Bool doSpw = False ;
//   *itsLog << "antenna = " << antenna << "(" << antenna.size() 
//           << "), field = " << field << "(" << field.size() 
//           << "), spw = " << spw << "(" << spw.size() << ")" << LogIO::POST ; 
  if ( antenna.size() != 0 ) {
    doAntenna = True ;
    sel.setAntennaExpr( antenna ) ;
  }
  if ( field.size() != 0 ) {
    doField = True ;
    sel.setFieldExpr( field ) ;
  }
  if ( spw.size() != 0 ) {
    doSpw = True ;
    sel.setSpwExpr( spw ) ;
  }

  // no selection
  if ( !doAntenna && !doField && !doSpw ) 
    return ;

  // selected MS name
  ostringstream sstr ;
  sstr << tableIn_ 
       << ".select" ;
  String tableSel( sstr ) ;

  try {
    TableExprNode node = sel.toTableExprNode( ms_p ) ;
    //*itsLog << "ms_p->nrow() = " << ms_p->nrow() << LogIO::POST ;
    //*itsLog << "node.nrow() = " << node.nrow() << LogIO::POST ;
    Table subtable = (*ms_p)( node, node.nrow() ) ;
    //*itsLog << "subtable.nrow() = " << subtable.nrow() << LogIO::POST ; 
    subtable.deepCopy( tableSel, Table::New, True ) ;
    
    // replace ms_p and tableName
    setMS( tableSel ) ;
  }
  catch ( AipsError e ) {
    cleanup() ;
    RETHROW( e ) ;
  }
}

void MSAnalysis::addHistory( String tableName, String suffix )
{
  suffix.upcase() ;
  Table hisTable( tableName+"/HISTORY", Table::Update ) ;
  //MSHistory history = ms.history() ;
  hisTable.addRow( 1, True ) ;
  ScalarColumn<String> commentCol( hisTable, "MESSAGE" ) ;
  String message = "FLOAT_DATA contains moment of type " + suffix ;
  commentCol.put( commentCol.nrow()-1, message ) ;
  hisTable.unlock() ;
}

}

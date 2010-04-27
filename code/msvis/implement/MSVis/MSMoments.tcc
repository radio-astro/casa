//# MSMoments.cc: generate moments from MS
//# Copyright (C) 1997,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id: MSMoments.tcc 19950 2007-02-27 12:25:11Z Malte.Marquarding $

#include <msvis/MSVis/MSMoments.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/LinearSearch.h>

#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <scimath/Mathematics/VectorKernel.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeApply.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <lattices/Lattices/SubLattice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template<class T> 
MSMoments<T>::MSMoments(MeasurementSet& ms,
                        LogIO &os,
                        Bool overwriteOutput,
                        Bool showProgressU)
  : MomentsBase<T>(os, overwriteOutput, showProgressU),
    ms_p(0),
    msSel_p(0),
    spwId_(-1)
{
  // set MS
  if ( setNewMS(ms) ) {
    goodParameterStatus_p = True;
  } 
  else {
    goodParameterStatus_p = False;
  }

  // CoordinateSystem
  cSys_ = CoordinateSystem() ;

  // showProgress is not implemented yet
  if ( showProgress_p ) {
    os_p << LogIO::WARN 
         << "showProgress is not implemented yet. Disabled." << LogIO::POST ;
    showProgress_p = False ;
  }
}

template<class T> 
MSMoments<T>::MSMoments(const MSMoments<T>& other)
{
  operator=(other); 
}

template<class T> 
MSMoments<T>::~MSMoments()
{
  delete ms_p ;
  if ( msSel_p != 0 ) {
    String name = msSel_p->tableName() ;
    delete msSel_p ;
    msSel_p = 0 ;
    Table::deleteTable( name, True ) ;
  }
}

template<class T> 
MSMoments<T>  &MSMoments<T>::operator=(const MSMoments<T> &other)
{
  if ( this != &other) {

    if ( ms_p != 0 ) 
      delete ms_p ;
    ms_p = new MeasurementSet( other.ms_p ) ;
    
    os_p = other.os_p ;
    showProgress_p = other.showProgress_p ;
    momentAxisDefault_p = other.momentAxisDefault_p ;
    kernelTypes_p = other.kernelTypes_p.copy() ;
    kernelWidths_p = other.kernelWidths_p.copy() ;
    moments_p = other.moments_p.copy() ;
    selectRange_p = other.selectRange_p.copy() ;
    peakSNR_p = other.peakSNR_p ;
    stdDeviation_p = other.stdDeviation_p ;
    yMin_p = other.yMin_p ;
    yMax_p = other.yMax_p ;
    smoothOut_p = other.smoothOut_p ;
    goodParameterStatus_p = other.goodParameterStatus_p ;
    doWindow_p = other.doWindow_p ;
    doFit_p = other.doFit_p ;
    doAuto_p = other.doAuto_p ;
    doSmooth_p = other.doSmooth_p ;
    noInclude_p = other.noInclude_p ;
    noExclude_p = other.noExclude_p ;
    fixedYLimits_p = other.fixedYLimits_p ;
    overWriteOutput_p = other.overWriteOutput_p ;
    convertToVelocity_p = other.convertToVelocity_p ;
    error_p = other.error_p ;
  }

  return *this;
}

template<class T> 
Bool MSMoments<T>::setNewMS(MeasurementSet& ms)
{
  if ( !goodParameterStatus_p ) {
    os_p << LogIO::SEVERE << "Internal class status is bad" << LogIO::POST ;
    return False ;
  }

  // MSMoments class accepts FLOAT or DOUBLE data
  T *dummy = 0 ;
  DataType msType = whatType(dummy) ;
  if ( msType != TpFloat && msType != TpDouble ) {
    ostringstream oss ;
    oss << "Moments can only be evaluated from MS of type : " 
        << TpFloat << " and " << TpDouble << endl ;
    String tmp( oss ) ;
    os_p << LogIO::SEVERE << tmp << LogIO::POST ;
    goodParameterStatus_p = False ;
    return False ;
  }

  // check if FLOAT_DATA exists
  if ( !ms.isColumn( MSMainEnums::FLOAT_DATA ) ) {
    os_p << LogIO::SEVERE
         << "MSMoments only accepts MS with FLOAT_DATA column" << endl 
         << "Please set appropriate MS using setNewMS()" << LogIO::POST ;
    goodParameterStatus_p = False ;
    return False ;
  }

  // At least, DATA_DESC_ID must be unique 
  ROScalarColumn<Int> ddIdCol( ms, "DATA_DESC_ID" ) ;
  Vector<Int> ddIds = ddIdCol.getColumn() ;
  if ( !allEQ( ddIds, ddIds[0] ) ) {
    os_p << LogIO::SEVERE
         << "DATA_DESC_ID must be unique in input MS." << LogIO::POST ;
    goodParameterStatus_p = False ;
    return False ;
  }

  // Select out rows with FLAG_ROW = True
  MSSelection sel ;
  sel.setTaQLExpr( "FLAG_ROW == FALSE" ) ;
  TableExprNode node = sel.toTableExprNode( &ms ) ;
  Table subtable = ms( node, node.nrow() ) ;
  if ( subtable.nrow() == 0 ) {
    os_p << LogIO::WARN << "Selected Table has no row." << LogIO::POST ;
    return False ;
  }
  if ( ms_p != 0 ) 
    delete ms_p ;
  ms_p = new MeasurementSet( subtable ) ;

  // spectral window ID
  ddIdCol.attach( *ms_p, "DATA_DESC_ID" ) ;
  dataDescId_ = ddIdCol( 0 ) ;
  ROScalarColumn<Int> dSpwIdCol( ms_p->dataDescription(), "SPECTRAL_WINDOW_ID" ) ;
  spwId_ = dSpwIdCol( dataDescId_ ) ;

  // Check if number of channel is not 1
  ROScalarColumn<Int> *numChanCol = new ROScalarColumn<Int>( ms_p->spectralWindow(), "NUM_CHAN" ) ;
  if ( (*numChanCol)(spwId_) == 1 ) {
    os_p << "NUM_CHAN is 1. Nothing to do." << LogIO::POST ;
    delete numChanCol ;
    goodParameterStatus_p = False ;
    return False ;
  } 
  delete numChanCol ;
  numChanCol = 0 ;

  return True;
}

template<class T>   
Bool MSMoments<T>::setSmoothMethod(const Vector<Int> &smoothAxesU,
                                   const Vector<Int>& kernelTypesU,
                                   const Vector< Quantum<Double> > &kernelWidthsU)
{
  if ( !goodParameterStatus_p ) {
    error_p = "Internal class status is bad" ;
    return False ;
  }

  Int i ;
  if ( smoothAxesU.nelements() > 0 ) {
    smoothAxes_p = smoothAxesU ;
    for ( i = 0 ; i < Int(smoothAxes_p.nelements()) ; i++ ) {
      // TODO: should check max number of dimension for MS data: 
      //       3 (spec,stokes,time) or 2 (spec,stokes) or 1 (spec)
      if ( smoothAxes_p( i ) < 0 || smoothAxes_p( i ) > 3 ) { 
        error_p = "Illegal smoothing axis given" ;
        goodParameterStatus_p = False ;
        return False ;
      }
    }
    doSmooth_p = True ;
  }
  else {
    doSmooth_p = False ;
    return True ;
  }


  if ( kernelTypesU.nelements() > 0 ) {
    kernelTypes_p = kernelTypesU ;
    for ( i = 0 ; i < Int(kernelTypes_p.nelements()) ; i++ ) {
      if ( kernelTypes_p( i ) < 0 || kernelTypes_p( i ) > VectorKernel::NKERNELS - 1 ) {
        error_p = "Illegal smoothing kernel types given" ;
        goodParameterStatus_p = False ;
        return False ;
      }
    }
  }
  else {
    error_p = "Smoothing kernel types were not given" ;
    goodParameterStatus_p = False ;
    return False ;
  }

  if ( smoothAxesU.nelements() != kernelTypes_p.nelements() ) {
    error_p = "Different number of smoothing axes to kernel types" ;
    goodParameterStatus_p = False ;
    return False ;
  }

  kernelWidths_p.resize( kernelTypes_p.nelements() ) ;
  Int nK = kernelWidthsU.nelements() ;
  for ( i = 0 ; i < Int(kernelWidths_p.nelements()) ; i++ ) {
    if ( kernelTypes_p( i ) == VectorKernel::HANNING ) {
      Quantum<Double> tmp( 3.0, String("pix") ) ;
      kernelWidths_p( i ) = tmp ;
    }
    else if ( kernelTypes_p( i ) == VectorKernel::BOXCAR ) {
      if ( i > nK - 1 ) {
        error_p = "Not enough smoothing widths given" ;
        goodParameterStatus_p = False ;
        return False ;
      }
      else {
        kernelWidths_p( i ) = kernelWidthsU( i ) ;
      }
    }
    else if ( kernelTypes_p( i ) == VectorKernel::GAUSSIAN ) {
      if ( i > nK - 1 ) {
        error_p = "Not enough smoothing widths given" ;
        goodParameterStatus_p = False ;
        return False ;
      }
      else {
        kernelWidths_p( i ) = kernelWidthsU( i ) ;
      }
    }
    else {
      error_p = "Internal logic error" ;
      goodParameterStatus_p = False ;
      return False ;
    }
  }

  return True;
}

template<class T>
Bool MSMoments<T>::createMoments(PtrBlock< MeasurementSet* >& outPt,
                                 Bool doTemp, 
                                 const String& outName )
{
  // Resize the vector of pointers for output data
  outPt.resize( moments_p.nelements() ) ;
  for ( uInt i = 0 ; i < outPt.nelements() ; i++ ) 
    outPt[i] = 0 ;

  // smoothing is not implemented yet
  if ( doSmooth_p ) {
    os_p << LogIO::WARN 
         << "Smoothing is not implemented yet. Disabled." << LogIO::POST ;
  }
    
  if ( !goodParameterStatus_p ) {
    error_p = "Internal status of class is bad. You have ignored errors" ;
    return False ;
  }

  // Find spectral axis
  const CoordinateSystem cSys = coordinates() ;
  Int spectralAxis = CoordinateUtil::findSpectralAxis( cSys ) ;

  if ( momentAxis_p == momentAxisDefault_p ) {
    if ( spectralAxis == -1 ) {
      error_p = "There is no spectral axis in this data -- specify the moment axis" ;
      return False ;
    }
    momentAxis_p = spectralAxis ;

    if ( getShape()( momentAxis_p ) <= 1 ) {
      error_p = "Illegal moment axis: it has only 1 pixel" ;
      goodParameterStatus_p = False ;
      return False ;
    }

    worldMomentAxis_p = cSys.pixelAxisToWorldAxis( momentAxis_p ) ;
  }
  String momentAxisUnits = cSys.worldAxisUnits()( worldMomentAxis_p ) ;
  os_p << LogIO::NORMAL << "Moment axis type is " << cSys.worldAxisNames()( worldMomentAxis_p ) 
       << LogIO::POST ;

  // If the moment axis is a spectral axis, indicate we want to convert to velocity
  convertToVelocity_p = False ;
  if ( momentAxis_p == spectralAxis ) 
    convertToVelocity_p = True ;

  // Check the user's requests are allowed
  if ( !checkMethod() )
    return False ;

  // Check that input and output MS names aren't the same.
  // if there is only one output MS
  if ( moments_p.nelements() == 1 ) {
    if ( !outName.empty() && ( outName == ms_p->tableName() ) ) {
      error_p = "Input MS and output MS have same name" ;
      return False ;
    }
  }

  // Try and set some useful Booleans
  Bool smoothClipMethod = False ;
  Bool windowMethod = False ;
  Bool fitMethod = False ;
  Bool clipMethod = False ;
  Bool doPlot = plotter_p.isAttached() ;

  if ( doSmooth_p && !doWindow_p ) {
    smoothClipMethod = True ;
  }
  else if ( doWindow_p ) {
    windowMethod = True ;
  }
  else if ( doFit_p ) {
    fitMethod = True ;
  }
  else {
    clipMethod = True ;
  }

  // Only smooth the image if smooth/clip method or interactive window method is 
  // selected.
  PtrHolder<MeasurementSet> pSmoothedDataHolder ;
  MeasurementSet *pSmoothedData = 0 ;
  String smoothName ;
  if ( doSmooth_p ) {
    if ( !smoothData( pSmoothedDataHolder, smoothName ) )
      return False ;
    pSmoothedData = pSmoothedDataHolder.ptr() ;
  }

  Vector<String> outFiles( moments_p.nelements(), "" ) ;

  Bool goodUnits ;
  Bool giveMessage = True ;

  // Prepare output files
  Vector<String> suffixes( moments_p.nelements() ) ;
  ROArrayColumn<Float> floatDataCol( *ms_p, "FLOAT_DATA" ) ;
  ColumnDesc desc = floatDataCol.columnDesc() ;
  TableRecord rec = desc.rwKeywordSet() ;
  String inUnit ;
  rec.get( rec.fieldNumber( "UNIT" ), inUnit ) ;
  Unit dataUnits = Unit( inUnit ) ;
  for ( uInt i = 0 ; i < moments_p.nelements() ; i++ ) {
    // Loop over desired output moments
    String suffix ;
    Unit momentUnits ;

    // Set moment data units and assign pointer to output moments array
    // Value of goodUnits is the same for each output moment
    goodUnits = setOutThings( suffix, momentUnits, dataUnits, 
                              momentAxisUnits, moments_p( i ),
                              convertToVelocity_p ) ;
    // create output MSs
    const String in = ms_p->tableName() ;
    String outFileName ;
    if ( outName.empty() ) {
      outFileName = in + suffix ;
    }
    else {
      if ( moments_p.nelements() == 1 )
        outFileName = outName ;
      else 
        outFileName = outName + suffix ;
    }
    if ( !overWriteOutput_p ) {
      NewFile x ;
      String error ;
      if ( !x.valueOK( outFileName, error ) ) {
        os_p << LogIO::NORMAL << error << LogIO::POST ;
        return False ;
      }
    }
    
    ms_p->deepCopy( outFileName, Table::New, True ) ;
    
    //os_p << LogIO::NORMAL << "Created " << outFileName << LogIO::POST ;

    // TODO: Setup output MS
    //          - SpectralWindow setting
    //          - FLOAT_DATA column description
    // update SPECTRAL_WINDOW table
    Table *tab = new Table( outFileName + "/SPECTRAL_WINDOW", Table::Update ) ;
    IPosition newShape( 1, 1 ) ;
    ArrayColumn<Double> *chanFreqCol = new ArrayColumn<Double>( *tab, "CHAN_FREQ" ) ;
    ArrayColumn<Double> *chanWidthCol = new ArrayColumn<Double>( *tab, "CHAN_WIDTH" ) ;
    ArrayColumn<Double> *effBWCol = new ArrayColumn<Double>( *tab, "EFFECTIVE_BW" ) ;
    ArrayColumn<Double> *resCol = new ArrayColumn<Double>( *tab, "RESOLUTION" ) ;
    ROScalarColumn<Double> *totBWCol = new ROScalarColumn<Double>( *tab, "TOTAL_BANDWIDTH" ) ;
    ScalarColumn<Int> *numChanCol = new ScalarColumn<Int>( *tab, "NUM_CHAN" ) ;
    for ( uInt j = 0 ; j < tab->nrow() ; j++ ) {
      // test if row is empty
      IPosition cellShape ;
      try {
        cellShape = chanFreqCol->shape( j ) ;
      }
      catch ( AipsError x ) {
        cellShape = IPosition() ;
      }
      numChanCol->put( j, 1 ) ;
      if ( !(cellShape.empty()) ) {
        Double medFreq = median( (*chanFreqCol)( j ) ) ;
        Vector<Double> chanFreq( 1, medFreq ) ;
        chanFreqCol->setShape( j, newShape ) ;
        chanFreqCol->put( j, chanFreq ) ;
        Vector<Double> chanWidth( 1, (*totBWCol)( j ) ) ;
        chanWidthCol->setShape( j, newShape ) ;
        chanWidthCol->put( j, chanWidth ) ;
        Vector<Double> effBW( 1, (*totBWCol)( j ) ) ;
        effBWCol->setShape( j, newShape ) ;
        effBWCol->put( j, effBW ) ;
        Vector<Double> resol( 1, (*totBWCol)( j ) ) ;
        resCol->setShape( j, newShape ) ;
        resCol->put( j, resol ) ;
      }
      else {
        chanFreqCol->setShape( j, newShape ) ;
        chanWidthCol->setShape( j, newShape ) ;
        effBWCol->setShape( j, newShape ) ;
        resCol->setShape( j, newShape ) ;
        Vector<Double> dummy( 1, (Double)j ) ;
        chanFreqCol->put( j, dummy ) ;
        chanWidthCol->put( j, dummy ) ;
        effBWCol->put( j, dummy ) ;
        resCol->put( j, dummy ) ;
      }
        
    }
    tab->flush() ;
    delete chanFreqCol ;
    delete chanWidthCol ;
    delete effBWCol ;
    delete totBWCol ;
    delete resCol ;
    delete numChanCol ;
    delete tab ;
    tab = 0 ;

    // update MAIN table
    tab = new Table( outFileName, Table::Update ) ;
    // replace FLOAT_DATA column
    tab->removeColumn( "FLOAT_DATA" ) ;
    ROArrayColumn<Float> floatDataCol( *ms_p, "FLOAT_DATA" ) ;
    ColumnDesc colDesc( floatDataCol.columnDesc() ) ;
    IPosition updateShape = floatDataCol.shape( 0 ) ;
    updateShape( 1 ) = 1 ;
    colDesc.setShape( updateShape ) ;
    colDesc.setName( "FLOAT_DATA" ) ;
    if ( goodUnits ) {
      // TODO: Set unit for MS?
      colDesc.rwKeywordSet().define( colDesc.rwKeywordSet().fieldNumber( "UNIT" ), momentUnits.getName() ) ;
    }
    else {
      if ( giveMessage ) {
        os_p << LogIO::NORMAL
             << "Could not determine the units of the moments MS(s) so the units " << endl ;
        os_p << "will be the same as those of the input MS. This may not be very useful." << LogIO::POST ;
        giveMessage = False ;
      }
    }
    tab->addColumn( colDesc ) ;
    tab->flush() ;
    delete tab ;
    tab = 0 ;

    outFiles[i] = outFileName ;
  }
                   
  // If the user is using the automatic, non-fitting window method, they need a 
  // good assesment of the noise, The use can input that value, but if they don't, 
  // we work it out here.
  T noise ;
  if ( stdDeviation_p <= T(0) && ( (doWindow_p && doAuto_p) || (doFit_p && !doWindow_p && doAuto_p) ) ) {
    if ( pSmoothedData ) {
      os_p << LogIO::NORMAL << "Evaluating noise level from smoothed data" << LogIO::POST ;
      if ( !whatIsTheNoise( noise, *pSmoothedData ) )
        return False ;
    }
    else {
      os_p << LogIO::NORMAL << "Evaluating noise level from input image" << LogIO::POST ;
      if ( !whatIsTheNoise( noise, *ms_p ) )
        return False ;
    }
    stdDeviation_p = noise ;
  }

  // Create Lattice from input or smoothed MS
  if ( pSmoothedData ) {
    floatDataCol.attach( *pSmoothedData, "FLOAT_DATA" ) ;
  }
  else {
    floatDataCol.attach( *ms_p, "FLOAT_DATA" ) ;
  }
  ArrayLattice<T> arrLatIn( floatDataCol.getColumn().nonDegenerate( IPosition( 2, 0, 1 ) ) ) ;
  MaskedLattice<T> *mskArrIn = new SubLattice<T>( arrLatIn, False) ;
  PtrBlock< MaskedLattice<T>* > arrLatOutPts( moments_p.nelements() ) ;
  IPosition outDataShape ;
  CoordinateSystem cSysOut = makeOutputCoordinates( outDataShape, 
                                                    cSys, 
                                                    mskArrIn->shape(), 
                                                    momentAxis_p, 
                                                    False ) ;
  for ( uInt i = 0 ; i < moments_p.nelements() ; i++ ) {
    ArrayLattice<T> tmpLattice( outDataShape ) ;
    tmpLattice.set( (T)0 ) ;
    arrLatOutPts[i] = new SubLattice<T>( tmpLattice, True ) ; 
  }

  // Create appropriate MomentCalculator object
  //os_p << LogIO::NORMAL << "Begin computation of moments" << LogIO::POST ;
  PtrHolder< MomentCalcBase<T> > pMomentCalculatorHolder ;
  try {
    if ( clipMethod || smoothClipMethod ) {
      //os_p << "create MomentClip" << LogIO::POST ;
      pMomentCalculatorHolder.set( new MomentClip<T>( mskArrIn, *this, os_p, moments_p.nelements() ),
                                   False, False ) ;
    }
    else if ( windowMethod ) {
      //os_p << "create MomentWindow" << LogIO::POST ;
      pMomentCalculatorHolder.set( new MomentWindow<T>( mskArrIn, *this, os_p, moments_p.nelements() ),
                                   False, False ) ;
    }
    else if ( fitMethod ) {
      //os_p << "create MomentFit" << LogIO::POST ;
      pMomentCalculatorHolder.set( new MomentFit<T>( *this, os_p, moments_p.nelements() ),
                                   False, False ) ;
    }
  }
  catch ( AipsError x ) {
    delete mskArrIn ;
    mskArrIn = 0 ;
    for ( uInt i = 0 ; i < arrLatOutPts.nelements() ; i++ ) {
      delete arrLatOutPts[i] ;
      arrLatOutPts[i] = 0 ;
    }
    if ( msSel_p != 0 ) {
      String name = msSel_p->tableName() ;
      delete msSel_p ;
      msSel_p = 0 ;
      Table::deleteTable( name, True ) ;
    }
    RETHROW( x ) ;
  }

  // Iterate optimally through the data, compute the moments, fill the output MSs
  MomentCalcBase<T> *pMomentCalculator = pMomentCalculatorHolder.ptr() ;
  try {
    LatticeApply<T>::lineMultiApply( arrLatOutPts, *mskArrIn, *pMomentCalculator, momentAxis_p, NULL ) ;
  }
  catch ( AipsError x ) {
    delete mskArrIn ;
    mskArrIn = 0 ;
    for ( uInt i = 0 ; i < arrLatOutPts.nelements() ; i++ ) {
      delete arrLatOutPts[i] ;
      arrLatOutPts[i] = 0 ;
    }
    if ( msSel_p != 0 ) {
      String name = msSel_p->tableName() ;
      delete msSel_p ;
      msSel_p = 0 ;
      Table::deleteTable( name, True ) ;
    }
    RETHROW( x ) ;
  }

  // Put moment results in output MS
  try {
    ostringstream oss ;
    for ( uInt i = 0 ; i < arrLatOutPts.nelements() ; i++ ) {
      outPt[i] = new MeasurementSet( outFiles[i], Table::Update ) ;
      Array<T> tmpData = arrLatOutPts[i]->get() ;
      ArrayColumn<T> outDataCol( *outPt[i], "FLOAT_DATA" ) ;
      if ( ms_p->nrow() == 1 )
        outDataCol.put( 0, tmpData ) ;
      else 
        outDataCol.putColumn( tmpData ) ;
      outPt[i]->flush() ;
      delete arrLatOutPts[i] ;
      arrLatOutPts[i] = 0 ;
    }
  }
  catch ( AipsError x ) {
    delete mskArrIn ;
    mskArrIn = 0 ;
    for ( uInt i = 0 ; i < arrLatOutPts.nelements() ; i++ ) {
      delete arrLatOutPts[i] ;
      arrLatOutPts[i] = 0 ;
    }
    for ( uInt i = 0 ; i < outPt.nelements() ; i++ ) {
      if ( outPt[i] ) {
        String tmpname = outPt[i]->tableName() ;
        delete outPt[i] ;
        outPt[i] = 0 ;
        Table::deleteTable( tmpname, True ) ;
      }
    }
    if ( msSel_p != 0 ) {
      String name = msSel_p->tableName() ;
      delete msSel_p ;
      msSel_p = 0 ;
      Table::deleteTable( name, True ) ;
    }
    RETHROW( x ) ;
  }

  // Clean up
  delete mskArrIn ;
  mskArrIn = 0 ;
  if ( windowMethod || fitMethod ) {
    if ( pMomentCalculator->nFailedFits() != 0 ) {
      os_p << LogIO::NORMAL << "There were " << pMomentCalculator->nFailedFits() << " failed fits" << LogIO::POST ;
    }
  }

  if ( pSmoothedData ) {
    pSmoothedDataHolder.clear( True ) ;
    if ( smoothOut_p.empty() ) {
      Directory dir( smoothName ) ;
      dir.removeRecursive() ;
    }
  }

  return True;
}

template<class T>
CoordinateSystem MSMoments<T>::coordinates()
{
  if ( cSys_.nCoordinates() == 0 ) {
    CoordinateSystem cSys = CoordinateSystem() ;
    
    //
    // Set StokesCoordinate
    //
    // create and set Stokes coordinate
    ROArrayColumn<Int> corrTypeCol( ms_p->polarization(), "CORR_TYPE" ) ;
    ROScalarColumn<Int> polIdCol( ms_p->dataDescription(), "POLARIZATION_ID" ) ;
    Int polId = polIdCol( dataDescId_ ) ;
    Vector<Int> polType = corrTypeCol( polId ) ;
    StokesCoordinate polSys = StokesCoordinate( polType ) ;
    cSys.addCoordinate( polSys ) ;

    //
    // Set SpectralCoordinate
    //
    // get CHAN_FREQ and MEAS_FREQ_REF from SPECTRAL_WINDOW table
    ROArrayColumn<Double> chanFreqCol( ms_p->spectralWindow(), "CHAN_FREQ" ) ; 
    ROScalarColumn<Int> measFreqRefCol( ms_p->spectralWindow(), "MEAS_FREQ_REF" ) ;
    Vector<Double> chanFreq ;
    Int measFreqRef ;
    chanFreqCol.get( spwId_, chanFreq, True ) ;
    measFreqRefCol.get( spwId_, measFreqRef ) ;
    
    // get REST_FREQUENCY from SOURCE table
    ROScalarColumn<Int> spwIdCol( ms_p->source(), "SPECTRAL_WINDOW_ID" ) ;
    ROArrayColumn<Double> restFreqCol( ms_p->source(), "REST_FREQUENCY" ) ;
    Vector<Int> spwIds = spwIdCol.getColumn() ;
    Double restFreq ;
    Bool noCorrRow = True ;
    for ( uInt i = 0 ; i < spwIds.nelements() ; i++ ) {
      if ( spwIds( i ) == spwId_ ) {
        Vector<Double> rfs ;
        restFreqCol.get( 0, rfs, True ) ;
        if ( rfs.nelements() == 0 ) {
          os_p << LogIO::WARN
               << "No rest frequency is given in SOURCE table" << endl 
               << "Rest frequency set to REF_FREQUENCY" << LogIO::POST ;
          ROScalarColumn<Double> refFreqCol( ms_p->spectralWindow(), "REF_FREQUENCY" ) ;
          restFreq = refFreqCol( spwId_ ) ;
        }
        else {
          restFreq = rfs( 0 ) ;
        }
        noCorrRow = False ;
        break ;
      }
    }
    if ( noCorrRow ) {
      os_p << LogIO::WARN
           << "No corresponding row in SOURCE table for spwid " << spwId_ << endl 
           << "Rest frequency set to REF_FREQUENCY" << LogIO::POST ;
      ROScalarColumn<Double> refFreqCol( ms_p->spectralWindow(), "REF_FREQUENCY" ) ;
      restFreq = refFreqCol( spwId_ ) ;
    }
    
    // create and set SpectralCoordinate
    SpectralCoordinate spSys = SpectralCoordinate( MFrequency::Types(measFreqRef), 
                                                   chanFreq, restFreq ) ;
    cSys.addCoordinate( spSys ) ;

    //
    // Set time coordinate as TabularCoordinate
    //
//     ROScalarColumn<Double> timeCentCol( *ms_p, "TIME_CENTROID" ) ;
//     Vector<Double> timeCentroid = timeCentCol.getColumn() ;
//     Vector<Double> tPixel( timeCentroid.nelements() ) ;
//     indgen( tPixel ) ;
//     TabularCoordinate tSys( tPixel, timeCentroid, "day", "TIME" ) ;
    //
    
    //
    // Set dummy coordinate as TabularCoordinate
    //
    if ( ms_p->nrow() != 1 ) {
      Vector<Double> tPixel( ms_p->nrow() ) ;
      indgen( tPixel ) ;
      TabularCoordinate tSys( tPixel, tPixel, "sec", "DUMMY" ) ;
      cSys.addCoordinate( tSys ) ;
    }
    //
    cSys_ = cSys ;
  }

  return cSys_ ;
}

template<class T>
IPosition MSMoments<T>::getShape()
{
//   // TODO: implement method to get shape of MS
  ROArrayColumn<Float> dataCol( *ms_p, "FLOAT_DATA" ) ;
  uInt nrow = dataCol.nrow() ;
  IPosition cellShape = dataCol.shape( 0 ) ;
  IPosition colShape( 1, nrow ) ;
  cellShape.append( colShape ) ;
  return cellShape ;
}

template<class T>
Bool MSMoments<T>::smoothData (PtrHolder<MeasurementSet> &pSmoothedMS,
                               String& smoothName)
{
  Int axMax = max( smoothAxes_p ) + 1 ;
  if ( axMax > Int( getShape().size() ) ) {
    error_p = "You have specified an illegal smoothing axis" ;
    return False ;
  }

  if ( smoothOut_p.empty() ) {
    File inputMSName( ms_p->tableName() ) ;
    const String path = inputMSName.path().dirName() + "/" ;
    Path fileName = File::newUniqueName( path, String("MSMoments_Smooth_") ) ;
    smoothName = fileName.absoluteName() ;
  }
  else {
    smoothName = smoothOut_p ;
  }

  // TODO: implement MS convolution



  return True;
}

template<class T>
Bool MSMoments<T>::whatIsTheNoise (T& noise,
                                   MeasurementSet &ms)
{
  return True ;
}

template<class T>
Bool MSMoments<T>::setMomentAxis (const Int& momentAxisU)
{
   if (!goodParameterStatus_p) {
      error_p = "Internal class status is bad";
      return False;
   }

   CoordinateSystem cSys = coordinates() ;

   momentAxis_p = momentAxisU;
   if (momentAxis_p == momentAxisDefault_p) {
     momentAxis_p = CoordinateUtil::findSpectralAxis(cSys);
     if (momentAxis_p == -1) {
       error_p = "There is no spectral axis in this MS -- specify the axis";
       goodParameterStatus_p = False;
       return False;
     }
   } else {
      if (momentAxis_p < 0 || momentAxis_p > Int(cSys.nCoordinates()-1)) {
         error_p = "Illegal moment axis; out of range";
         goodParameterStatus_p = False;
         return False;
      }
      if (getShape()(momentAxis_p) <= 0) {
         error_p = "Illegal moment axis; it has no pixels";
         goodParameterStatus_p = False;
         return False;
      }
      // Moment axis should be spectral axis right now
      if ( cSys.type( momentAxis_p ) != Coordinate::SPECTRAL ) {
        error_p = "Moment axis should be spectral axis." ;
        goodParameterStatus_p = False ;
        return False ;
      }
   }
   worldMomentAxis_p = cSys.pixelAxisToWorldAxis(momentAxis_p);

   return True;
}


template<class T>
Bool MSMoments<T>::setIncludeExclude (Vector<T>& range,
                                      Bool& noInclude,
                                      Bool& noExclude,
                                      const Vector<T>& include,
                                      const Vector<T>& exclude,
                                      ostream& os)
{
  if ( include.nelements() != 0 && exclude.nelements() != 0 ) {
    os << "You can only give one of arguments include or exclude" << endl;
    return False;
  }

  vector<T> stlInc( 0 ) ;
  vector<uInt> stlExc( 0 ) ;
  noInclude = True ;
  if ( include.nelements() != 0 ) {
    for ( uInt i = 0 ; i < ms_p->nrow() ; i++ ) {
      if ( allNE( include, (T)i ) ) {
        stlExc.push_back( i ) ;
      }
      else {
        stlInc.push_back( (T)i ) ;
      }
    }
    noInclude = False ;
  }
  noExclude = True ;
  if ( exclude.nelements() != 0 ) {
    for ( uInt i = 0 ; i < ms_p->nrow() ; i++ ) {
      if ( allNE( exclude, (T)i ) ) {
        stlInc.push_back( (T)i ) ;
      }
      else {
        stlExc.push_back( i ) ;
      }
    }
    noExclude = False ;
  }

  range = Vector<T>( stlInc ) ;

  if ( !noInclude || !noExclude ) {
    String tmpName = ms_p->tableName() + ".MS_MOMENTS_TMP" ;
    if ( msSel_p ) {
      delete msSel_p ;
      msSel_p = 0 ;
      Table::deleteTable( tmpName, True ) ;
    }
    ms_p->deepCopy( tmpName, Table::New, True ) ;
    Table *tab = new Table( tmpName, Table::Update ) ;
    ScalarColumn<Bool> flagRowCol( *tab, "FLAG_ROW" ) ;
    for ( uInt i = 0 ; i < stlExc.size() ; i++ ) {
      flagRowCol.put( stlExc[i], True ) ;
    }
    tab->flush() ;
    delete tab ;
    tab = 0 ;
    msSel_p = new MeasurementSet( tmpName ) ;
  }

  return True ;
}

} //# NAMESPACE CASA - END


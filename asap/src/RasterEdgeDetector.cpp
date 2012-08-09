//
// C++ Implimentation: RasterEdgeDetector
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Containers/Record.h>
//#include <casa/Utilities/GenSort.h>

#include "RasterEdgeDetector.h"

using namespace std ;
using namespace casa ;

namespace asap {

RasterEdgeDetector::RasterEdgeDetector() 
  : EdgeDetector()
{}

RasterEdgeDetector::~RasterEdgeDetector()
{}

Vector<uInt> RasterEdgeDetector::detect() 
{
  os_.origin(LogOrigin( "RasterEdgeDetector", "detect", WHERE )) ;

  initDetect() ;

  detectGap() ;
  selection() ;
  
  os_ << LogIO::DEBUGGING
      << "Detected " << off_.nelements() << " integrations as OFF" << LogIO::POST ;

  return off_ ;
}

void RasterEdgeDetector::parseOption( const Record &option ) 
{
  os_.origin(LogOrigin( "RasterEdgeDetector", "parseOption", WHERE )) ;

  String name = "fraction" ;
  if ( option.isDefined( name ) ) {
    if ( option.dataType( name ) == TpString ) {
      String fstr = option.asString( name ) ;
      if ( fstr == "auto" ) {
        fraction_ = -1.0 ; // indicates "auto"
      }
      else {
        // should be "xx%" format
        fstr.substr(0,fstr.size()-1) ;
        fraction_ = String::toFloat( fstr ) * 0.01 ;
      }
    }
    else {
      fraction_ = option.asFloat( name ) ;
    }
  }
  else {
    fraction_ = 0.1 ; // default is 10%
  }
  name = "npts" ;
  if ( option.isDefined( name ) ) {
    npts_ = option.asInt( name ) ;
  }
  else {
    npts_ = -1 ; // default is -1 (use fraction_)
  }

  os_ << "OPTION SUMMARY: " << endl 
      << "   fraction=" << fraction_ << endl 
      << "   npts=" << npts_ << LogIO::POST ; 
}

void RasterEdgeDetector::detectGap()
{
  os_.origin(LogOrigin( "RasterEdgeDetector", "detectGap", WHERE )) ;

  uInt n = time_.nelements() ;
  Vector<Double> tdiff( n-1 ) ;
  for ( uInt i = 0 ; i < n-1 ; i++ ) {
    tdiff[i] = time_[i+1] - time_[i] ;
  }
  Double tmed = median( tdiff, False, True, False ) ;
  Double threshold = tmed * 5.0 ;
  uInt idx = 0 ;
  tempuInt_[idx++] = 0 ;
  for ( uInt i = 0 ; i < n-1 ; i++ ) {
    if ( tdiff[i] > threshold ) {
      tempuInt_[idx++] = i+1 ;
    }
  }
  if ( tempuInt_[idx-1] != n ) {
    tempuInt_[idx++] = n ;
  }
  gaplist_ = vectorFromTempStorage( idx ) ;
  
  os_ << LogIO::DEBUGGING
      << "Detected " << gaplist_.nelements() << " time gaps." << LogIO::POST ;
}

void RasterEdgeDetector::selection()
{
//   os_.origin(LogOrigin( "RasterEdgeDetector", "selection", WHERE )) ;

  uInt n = gaplist_.nelements() - 1 ;
  uInt idx = 0 ;
  for ( uInt i = 0 ; i < n ; i++ ) {
    uInt start = gaplist_[i] ;
    uInt end = gaplist_[i+1] ;
    selectionPerRow( idx, start, end ) ;
  }
  off_ = vectorFromTempStorage( idx ) ;
  //sort( off_, Sort::Ascending, Sort::QuickSort | Sort::NoDuplicates ) ;
}

void RasterEdgeDetector::selectionPerRow( uInt &idx,
                                          const uInt &start,
                                          const uint &end ) 
{
//   os_.origin(LogOrigin( "RasterEdgeDetector", "selectionPerRow", WHERE )) ;

  uInt len = end - start ;
  uInt noff = numOff( len ) ;

  if ( len >  noff * 2 ) {
    uInt n = start + noff ;
    for ( uInt i = start ; i < n ; i++ ) {
      tempuInt_[idx++] = i ;
    }
    n = end - noff ;
    for ( uInt i = n ; i < end ; i++ ) {
      tempuInt_[idx++] = i ;
    }
  }
  else {
    // add all elements to off position list
    for ( uInt i = start ; i < end ; i++ ) {
      tempuInt_[idx++] = i ;
    }
  }
}
                                          
uInt RasterEdgeDetector::numOff( const uInt &n ) 
{
  uInt ret = 0 ;
  if ( npts_ > 0 ) {
    ret = min( (uInt)npts_, n ) ;
  }
  else if ( fraction_ < 0 ) {
    ret = optimumNumber( n ) ;
  }
  else {
    ret = int( n * fraction_ ) ;
  }
  return ((ret > 0) ? ret : 1) ;
}

uInt RasterEdgeDetector::optimumNumber( const uInt &n ) 
{
  return uInt( sqrt( n + 1 ) ) - 1 ;
}

} // namespace asap

//
// C++ Implimentation: EdgeDetector
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "EdgeDetector.h"
#include "MathUtils.h"

using namespace std ;
using namespace casa ;

namespace asap {

EdgeDetector::EdgeDetector()
  : off_(),
    tempIP_( 1, 1 )
{}

EdgeDetector::~EdgeDetector()
{}

void EdgeDetector::setDirection( const Matrix<Double> &dir ) 
{
  //dir_.reference( dir ) ;
  //dir_ = dir.copy() ;
  dir_.reference(dir.copy());
  Vector<Double> ra( dir_.row(0) ) ;
  mathutil::rotateRA( ra ) ;
  resizeTempArea( dir.nrow() ) ;
}

void EdgeDetector::setTime( const Vector<Double> &t ) 
{
  time_.reference( t ) ;
  resizeTempArea( t.nelements() ) ;
}

void EdgeDetector::resizeTempArea( const uInt &n ) 
{
  if ( tempuInt_.nelements() < n ) {
    tempuInt_.resize( n, False, False ) ;
  }
}

Vector<uInt> EdgeDetector::vectorFromTempStorage( const uInt &n ) 
{
  tempIP_[0] = n ;
  return Vector<uInt>( tempIP_, tempuInt_.storage(), COPY ) ;
}

void EdgeDetector::initDetect()
{
  off_.resize() ;
}

void EdgeDetector::setOption( const Record &option )
{
  this->parseOption( option ) ;
}

} // namespace asap

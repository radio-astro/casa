//
// C++ Interface: EdgeDetector
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _EDGE_DETECTOR_H_ 
#define _EDGE_DETECTOR_H_ 

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>

namespace asap {

class EdgeDetector 
{
public:
  EdgeDetector() ;
  virtual ~EdgeDetector() ;

  void setDirection( const casa::Matrix<casa::Double> &dir ) ;
  void setTime( const casa::Vector<casa::Double> &t ) ;
  void setOption( const casa::Record &option ) ;
  virtual casa::Vector<casa::uInt> detect() = 0 ;

protected:
  virtual void parseOption( const casa::Record &option ) = 0 ;
  casa::Vector<casa::uInt> vectorFromTempStorage( const casa::uInt &n ) ;
  void initDetect() ;

  // input data
  casa::Matrix<casa::Double> dir_ ;
  casa::Vector<casa::Double> time_ ;

  // output data: list of indexes for OFF positions
  casa::Vector<casa::uInt> off_ ;
  
  // temporary memory storage
  casa::Block<casa::uInt> tempuInt_ ;
  casa::IPosition tempIP_ ;

  // logging
  casa::LogIO os_ ;

private:
  void resizeTempArea( const casa::uInt &n ) ;
} ;

}
#endif /* _EDGE_DETECTOR_H_ */

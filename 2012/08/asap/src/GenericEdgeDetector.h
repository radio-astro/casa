//
// C++ Interface: GenericEdgeDetector
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _GENERIC_EDGE_DETECTOR_H_ 
#define _GENERIC_EDGE_DETECTOR_H_ 

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>

#include "EdgeDetector.h"

namespace asap {

class GenericEdgeDetector : public EdgeDetector
{
public:
  GenericEdgeDetector() ;
  virtual ~GenericEdgeDetector() ;

  casa::Vector<casa::uInt> detect() ;

private:
  // parse options
  void parseOption( const casa::Record &option ) ;

  // steps for edge detection algorithm
  void topixel() ;
  void countup() ;
  void thresholding() ;
  void labeling() ;
  void trimming() ;
  void selection() ;
  void tuning() ;

  // internal methods
  void setup() ;
  casa::uInt _labeling() ;
  casa::uInt __labeling( casa::Vector<casa::uInt> &a ) ;
  casa::uInt _trimming() ;
  casa::uInt _trimming1DX() ;
  casa::uInt _trimming1DY() ;
  casa::uInt _trimming1D( casa::Vector<casa::uInt> &a ) ;
  void _search( casa::uInt &start, 
                casa::uInt &end, 
                const casa::Vector<casa::uInt> &a ) ;

  // pixel info
  casa::Double cenx_ ;
  casa::Double ceny_ ;
  casa::Double pcenx_ ;
  casa::Double pceny_ ;
  casa::uInt nx_ ;
  casa::uInt ny_ ;
  casa::Double dx_ ;
  casa::Double dy_ ;
  
  // storage for detection
  casa::Matrix<casa::Double> pdir_ ;
  casa::Matrix<casa::uInt> apix_ ;

  // options
  casa::Float width_ ;
  casa::Float fraction_ ;
  casa::Bool elongated_ ;
} ;

}
#endif /* _GENERIC_EDGE_DETECTOR_H_ */

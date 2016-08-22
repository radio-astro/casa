//
// C++ Interface: RasterEdgeDetector
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _RASTER_EDGE_DETECTOR_H_ 
#define _RASTER_EDGE_DETECTOR_H_ 

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>

#include "EdgeDetector.h"

namespace asap {

class RasterEdgeDetector : public EdgeDetector
{
public:
  RasterEdgeDetector() ;
  virtual ~RasterEdgeDetector() ;

  casa::Vector<casa::uInt> detect() ;

private:
  // parse options
  void parseOption( const casa::Record &option ) ;
  
  // edge detection algorithm for raster
  void detectGap() ;
  void selection() ;
  void selectionPerRow( casa::uInt &idx, 
                        const casa::uInt &start, 
                        const casa::uInt &end ) ;
  void extractRow( const casa::uInt &irow ) ;
  casa::uInt numOff( const casa::uInt &n ) ;
  casa::uInt optimumNumber( const casa::uInt &n ) ;

  // gap list
  casa::Vector<casa::uInt> gaplist_ ;

  // options
  casa::Float fraction_ ;
  casa::Int npts_ ;
} ;

}
#endif /* _RASTER_EDGE_DETECTOR_H_ */

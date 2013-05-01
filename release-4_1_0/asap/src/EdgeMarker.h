//
// C++ Interface: EdgeMarker
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _EDGE_MARKER_H_ 
#define _EDGE_MARKER_H_ 

#include <string>

#include <casa/Arrays/Vector.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/Table.h>

#include "Scantable.h"
#include "EdgeDetector.h"

namespace asap {

class EdgeMarker 
{
public:
  EdgeMarker() ;
  EdgeMarker( bool israster ) ;

  virtual ~EdgeMarker() ;

  void setdata( const casa::CountedPtr<Scantable> &s,
                const casa::Bool &insitu ) ;
  void examine() ;
  void setoption( const casa::Record &option ) ;
  void detect() ;
  void mark() ;
  casa::Block<casa::uInt> getDetectedRows() ;
  casa::CountedPtr<Scantable> get() ;
//   void reset() ;

private:
  void initDetect() ;

  // data
  casa::CountedPtr<Scantable> st_ ;

  // pointer to detector object
  casa::CountedPtr<EdgeDetector> detector_ ;
  
  // list of IFNO for WVR
  casa::Vector<casa::uInt> wvr_ ;

  // off position list
  casa::Block<casa::uInt> off_ ;
  casa::uInt noff_ ;

  // logger
  casa::LogIO os_ ;
} ;

}
#endif /* _EDGE_MARKER_H_ */

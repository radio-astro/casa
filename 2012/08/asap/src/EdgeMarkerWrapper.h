//
// C++ Interface: EdgeMarkerWrapper
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _OFF_POSITION_MARKER_WRAPPER_H_ 
#define _OFF_POSITION_MARKER_WRAPPER_H_ 

#include <casa/Utilities/CountedPtr.h>

#include "Scantable.h"
#include "EdgeMarker.h"
#include "ScantableWrapper.h"

namespace asap {

class EdgeMarkerWrapper : public EdgeMarker
{
public:
  EdgeMarkerWrapper() 
    : EdgeMarker()
  {}

  EdgeMarkerWrapper( bool israster )
    : EdgeMarker( israster )
  {}

  ~EdgeMarkerWrapper() {} 

  void setdata( const ScantableWrapper &s,
                const bool &insitu ) 
  { 
    EdgeMarker::setdata( s.getCP(), (casa::Bool)insitu ) ; 
  }

  ScantableWrapper get()
  {
    casa::CountedPtr<Scantable> s = EdgeMarker::get() ;
    return ScantableWrapper( s ) ;
  }
} ;

}
#endif /* _OFF_POSITION_MARKER_WRAPPER_H_ */

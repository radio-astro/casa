//# WCRTZoomer.h: WorldCanvas event-based zoomer
//# Copyright (C) 1999,2000
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
//# $Id$

#ifndef TRIALDISPLAY_WCRTZOOMER_H
#define TRIALDISPLAY_WCRTZOOMER_H

//# aips includes
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>

//# trial includes

//# display library includes
#include <display/DisplayEvents/WCRectTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// WorldCanvas event-based zoomer
// </summary>

// <use visibility=export>

// <reviewed reviewer="" data="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisites>
//   <li> WCRectTool
// </prerequisites>

// <etymology>
// WCRTZoomer stands for WorldCanvas Rectangling Tool Zoomer
// </etymology>

// <synopsis>
// This class finalises the functionality in WCRectTool to implement
// event-based zooming on a WorldCanvas.  Double clicking inside the
// constructed rectangle results in zooming in; double clicking outside
// the rectangle gives zooming out.
// </synopsis>

// <example>
// </example>

// <motivation>
// Zooming is an essential tool for interacting with data displays.
// </motivation>

// <todo asof="1999/02/10">
//   <li> Nothing known
// </todo>

class WCRTZoomer : public WCRectTool {

 public:

  // Constructor
  WCRTZoomer(WorldCanvas *wcanvas,
	     Display::KeySym keysym = Display::K_Pointer_Button1);

  // Destructor
  virtual ~WCRTZoomer();

  // Functions specific to the zooming - these are called by 
  // lower-level event handlers in the base classes
  // <group>
  virtual void doubleInside();
  virtual void doubleOutside();
  // </group>

  // This function is called when a zoom occurs.  It is supplied
  // with the linear coords of the new zoom box, and thus can be
  // implemented to do further actions, or perhaps report the 
  // new zoom box to the application
  virtual void zoomed(const Vector<Double> &linBlc, 
		      const Vector<Double> &linTrc);

 private:
  
  // get the linear coords of the current zoom box
  void getLinearCoords(Vector<Double> &blc, Vector<Double> &trc);

};


} //# NAMESPACE CASA - END

#endif

//# PCInvisTool.h: base class for PC event-based invisible position tools
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

#ifndef TRIALDISPLAY_PCINVISTOOL_H
#define TRIALDISPLAY_PCINVISTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/PCTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base implementation of PCTool for invisible (non-drawing) tools.
// </summary>
// 
// <synopsis>
// This class implements some of PCTool, and adds additional interface
// to support invisible (ie. non-drawing) tools for PixelCanvases.
// </synopsis>

class PCInvisTool : public PCTool {

 public:

  // Constructor.  Requires a PixelCanvas to operate on, and a key
  // to catch.
  PCInvisTool(PixelCanvas *pcanvas,
	      Display::KeySym keysym = Display::K_Pointer_Button1);

  // Destructor.
  virtual ~PCInvisTool();

  // Functions called by the local event handling operators.
  // <group>
  virtual void keyPressed(const PCPositionEvent &ev);
  //virtual void keyReleased(const PCPositionEvent &ev);
  virtual void moved(const PCMotionEvent &ev);
  // </group>

  // Function called when a position is ready.
  virtual void positionReady() { };

  // Retrieve the position in PixelCanvas pixel coordinates.
  virtual void get(Int &x, Int &y) const;

  // Retrieve the position in fractional PixelCanvas coordinates.
  virtual void getFractional(Float &x, Float &y) const;

 private:

  // Have we moved?
  Bool itsMoved;

  // Pixel coordinates of the position.
  Int itsX, itsY;

};


} //# NAMESPACE CASA - END

#endif

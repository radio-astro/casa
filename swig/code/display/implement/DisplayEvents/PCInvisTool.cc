//# PCInvisTool.cc: base class for PC event-based invisible position tools
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

#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/PCInvisTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
PCInvisTool::PCInvisTool(PixelCanvas *pcanvas, Display::KeySym keysym) :
  PCTool(pcanvas, keysym) {
}

// Destructor.
PCInvisTool::~PCInvisTool() {
}

// Functions called by the local event handling operators.
void PCInvisTool::keyPressed(const PCPositionEvent &ev) {
  itsX = ev.x();
  itsY = ev.y();
  positionReady();
}
void PCInvisTool::moved(const PCMotionEvent &ev) {
  if (!keyPresentlyDown(ev)) {
    return;
  } else {
    itsX = ev.x();
    itsY = ev.y();
    positionReady();
  }
}

// Retrieve the position in PixelCanvas pixel coordinates.
void PCInvisTool::get(Int &x, Int &y) const {
  x = itsX;
  y = itsY;
}

// Retrieve the position in fractional PixelCanvas coordinates.
void PCInvisTool::getFractional(Float &x, Float &y) const {
  Int lx, ly;
  get(lx, ly);
  PixelCanvas *pc = pixelCanvas();
  x = Float(lx) / Float(pc->width() - 1);
  y = Float(ly) / Float(pc->height() - 1);
}

} //# NAMESPACE CASA - END


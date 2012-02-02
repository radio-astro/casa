//# WCCrosshairTool.cc: Base class for WorldCanvas event-based crosshair tools
//# Copyright (C) 1999,2000,2001
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

#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/WCCrosshairTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

WCCrosshairTool::WCCrosshairTool(WorldCanvas *wcanvas,
				 Display::KeySym keysym,
				 const Bool persistent) :
  WCTool(wcanvas, keysym),
  itsCrosshairPersistent(persistent),
  itsCrosshairRadius(8),
  itsOnScreen(False),
  itsActive(False),
  itsCrosshairExists(False) {
  reset();
}

WCCrosshairTool::~WCCrosshairTool() {
}

void WCCrosshairTool::disable() {
  WCTool::disable();
  if (itsCrosshairExists || itsActive) {
    draw(itsCrosshairExists);
    reset();
  }
}

void WCCrosshairTool::keyPressed(const WCPositionEvent &ev) {
  Int x = ev.pixX();
  Int y = ev.pixY();
  its2ndLastPressX = itsLastPressX;
  its2ndLastPressY = itsLastPressY;
  its2ndLastPressTime = itsLastPressTime;
  itsLastPressX = x;
  itsLastPressY = y;
  itsLastPressTime = ev.timeOfEvent();
  if (itsCrosshairExists && !itsActive) {
    
    Int x1, y1;
    get (x1, y1);
    Bool inside = (((x - x1) * (x - x1) + (y - y1) * (y - y1)) <=
			 itsCrosshairRadius * itsCrosshairRadius);
    if (inside) {
      // user has pressed inside the crosshair
      itsAdjustMode = WCCrosshairTool::Move;
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      itsActive = True;
      itsMoved = False;
      crosshairNotReady();
    } else {
    }
    return;
  } else {
    if (!itsActive) {
      // key pressed down first time: start crosshair
      set(ev.pixX(), ev.pixY());
      ev.worldCanvas()->pixelCanvas()->copyFrontBufferToBackBuffer();
      draw();
      reset();
      itsAdjustMode = WCCrosshairTool::Move;
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      itsActive = True;
      itsCrosshairExists = True;
      crosshairNotReady();
    }
  }
}

void WCCrosshairTool::keyReleased(const WCPositionEvent &ev) {
  Int x = ev.pixX();
  Int y = ev.pixY();
  if (itsActive && (itsAdjustMode == WCCrosshairTool::Off)) {
    itsActive = False;
    itsLastPressX = itsLastPressY = -99999;
    itsLastReleaseX = itsLastReleaseY = -99999;
    itsLastPressTime = its2ndLastPressTime = -1.0;
    itsCrosshairExists = True;
    preserve();
    crosshairReady();
    itsLastPressX = itsLastPressY = -99999;
    itsLastReleaseX = itsLastReleaseY = -99999;
    itsLastPressTime = its2ndLastPressTime = -1.0;
  } else if (itsActive && (itsAdjustMode == WCCrosshairTool::Move)) {
    itsAdjustMode = WCCrosshairTool::Off;
    itsActive = False;
    preserve();
    crosshairReady();
  }
  if ((abs(ev.timeOfEvent() - its2ndLastPressTime) < doubleClickInterval()) &&
      itsCrosshairExists) {
    itsLastReleaseX = itsLastReleaseY = -99999;
    // "double click" & crosshair exists
    if (!itsCrosshairPersistent) {
      draw();
      reset();
    } else {
      itsActive = False;
      crosshairReady();
      itsMoved = False;
      itsAdjustMode = WCCrosshairTool::Off;
    }
    // emit either inside or outside dbl click
    Int x1, y1;
    get (x1, y1);
    Bool inside = (((x - x1) * (x - x1) + (y - y1) * (y - y1)) <=
			 itsCrosshairRadius * itsCrosshairRadius);
    if (inside) {
      doubleInside();
    } else {
      doubleOutside();
    }
    return;
  }

  itsLastReleaseX = x;
  itsLastReleaseY = y;
}

void WCCrosshairTool::otherKeyPressed(const WCPositionEvent &ev) {
  if (ev.key() == Display::K_Escape) {
    if (itsCrosshairExists || itsActive) {
      draw();
      reset();
    }
  }
}

void WCCrosshairTool::moved(const WCMotionEvent &ev) {
  itsMoved = True;
  if (!itsActive) {
    return;
  }
  uInt x = ev.pixX();
  uInt y = ev.pixY();
  if (itsAdjustMode == WCCrosshairTool::Move) {
    Int x1, y1;
    get(x1, y1);
    Int dx = x - itsBaseMoveX;
    Int dy = y - itsBaseMoveY;
    itsBaseMoveX = x;
    itsBaseMoveY = y;
    draw();
    set(x1 + dx, y1 + dy);
    draw();
  }
}

void WCCrosshairTool::refresh(const WCRefreshEvent &ev) {
  if (ev.reason() == Display::BackCopiedToFront) {
    itsOnScreen = False;
  }
  if (!itsActive && itsCrosshairExists) {
    restore();
    draw(True);
  } else {
    reset();
  }  
}

void WCCrosshairTool::set(const Int &x1, const Int &y1) {
  itsX1 = x1;
  itsY1 = y1;
}

void WCCrosshairTool::get(Int &x1, Int &y1) const {
  x1 = itsX1;
  y1 = itsY1;
}

void WCCrosshairTool::preserve() {
  Vector<Double> from(2);
  from(0) = itsX1;
  from(1) = itsY1;
  worldCanvas()->pixToWorld(itsStoredWorldPosition, from);
}

void WCCrosshairTool::restore() {
  Vector<Double> to(2);
  worldCanvas()->worldToPix(to, itsStoredWorldPosition);
  itsX1 = Int(to(0) + 0.5);
  itsY1 = Int(to(1) + 0.5);
}

void WCCrosshairTool::draw(const Bool drawHandles) {
  PixelCanvas *pCanvas = pixelCanvas();
  if (itsOnScreen) {
    pCanvas->copyBackBufferToFrontBuffer();
    itsOnScreen = False;
    return;
  }
  itsOnScreen = True;
  Display::DrawBuffer oldBuffer = pCanvas->drawBuffer();
  pCanvas->setDrawBuffer(Display::FrontBuffer);
  pCanvas->setCapStyle(Display::CSRound);
  pCanvas->setColor(drawColor());
  pCanvas->setLineWidth(lineWidth());
  pCanvas->setDrawFunction(Display::DFCopy);

  pCanvas->setLineStyle(Display::LSSolid);
  pCanvas->drawEllipse(itsX1, itsY1, itsCrosshairRadius,
		       itsCrosshairRadius, 0.0);
  pCanvas->drawEllipse(itsX1, itsY1, itsCrosshairRadius + 3,
		       itsCrosshairRadius + 3, 0.0);
  pCanvas->drawLine(itsX1 + itsCrosshairRadius / 2, itsY1, 
		    itsX1 + 3 * itsCrosshairRadius, itsY1);
  pCanvas->drawLine(itsX1 - itsCrosshairRadius / 2, itsY1, 
		    itsX1 - 3 * itsCrosshairRadius, itsY1);
  pCanvas->drawLine(itsX1, itsY1 + itsCrosshairRadius / 2,
		    itsX1, itsY1 + 3 * itsCrosshairRadius);
  pCanvas->drawLine(itsX1, itsY1 - itsCrosshairRadius / 2,
		    itsX1, itsY1 - 3 * itsCrosshairRadius);

  pCanvas->setDrawBuffer(oldBuffer);
}

void WCCrosshairTool::reset() {
  if (!itsActive && itsCrosshairExists) {
    crosshairNotReady();
  }
  itsActive = False;
  itsMoved = False;
  itsCrosshairExists = False;
  itsAdjustMode = WCCrosshairTool::Off;
  itsLastPressX = itsLastPressY = -99999;
  its2ndLastPressX = its2ndLastPressY = -99999;
  itsLastReleaseX = itsLastReleaseY = -99999;
  itsLastPressTime = its2ndLastPressTime = -1.0;
}


} //# NAMESPACE CASA - END


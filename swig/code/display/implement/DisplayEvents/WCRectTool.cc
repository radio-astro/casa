//# WCRectTool.cc: Base class for WorldCanvas event-based rectangle tools
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
#include <display/DisplayEvents/WCRectTool.h>
#include <casa/BasicMath/Math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

WCRectTool::WCRectTool(WorldCanvas *wcanvas,
		       Display::KeySym keysym,
		       const Bool persistent) :
  WCTool(wcanvas, keysym),
  itsRectanglePersistent(persistent),
  itsOnScreen(False),
  itsActive(False),
  itsRectangleExists(False),
  itsHX(4),
  itsHY(4){
  reset();
}

WCRectTool::~WCRectTool() {
}

void WCRectTool::disable() {
  WCTool::disable();
  if (itsRectangleExists || itsActive) {
    draw(itsRectangleExists);
    reset();
  }
}

void WCRectTool::keyPressed(const WCPositionEvent &ev) {
  Int x = ev.pixX();
  Int y = ev.pixY();
  its2ndLastPressX = itsLastPressX;
  its2ndLastPressY = itsLastPressY;
  its2ndLastPressTime = itsLastPressTime;
  itsLastPressX = x;
  itsLastPressY = y;
  itsLastPressTime = ev.timeOfEvent();
  if (itsRectangleExists && !itsActive) {
    Bool left = ((x >= min(itsHX(0), itsHX(1))) && 
		       (x <= max(itsHX(0), itsHX(1))));
    Bool right = ((x >= min(itsHX(2), itsHX(3))) && 
			(x <= max(itsHX(2), itsHX(3))));
    Bool top = ((y >= min(itsHY(0), itsHY(1))) &&
		      (y <= max(itsHY(0), itsHY(1))));
    Bool bottom = ((y >= min(itsHY(2), itsHY(3))) &&
			 (y <= max(itsHY(2), itsHY(3))));
    if ((left || right) && (top || bottom)) {
      // user has pressed on a handle
      Int x1, y1, x2, y2, tmp;
      get(x1, y1, x2, y2);
      if ((left && (itsX1 < itsX2)) || (right && (itsX1 > itsX2))) {
	tmp = x1;
	x1 = x2;
	x2 = tmp;
      }
      if ((top && (itsY1 < itsY2)) || (bottom && (itsY1 > itsY2))){
	tmp = y1;
	y1 = y2;
	y2 = tmp;
      }
      draw(True); 
      set(x1, y1, x2, y2);
      draw(); 
      reset();
      itsActive = True;
      return;
    }
    if ((x >= min(itsX1, itsX2)) && (x <= max(itsX1, itsX2)) &&
	(y >= min(itsY1, itsY2)) && (y <= max(itsY1, itsY2))) {
      // user has pressed inside the rectangle
      itsAdjustMode = WCRectTool::Move;
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      itsActive = True;
      itsMoved = False;
      rectangleNotReady();
      draw(True);
      draw();
      return;
    }
  } else {
    if (!itsActive) {
      // key pressed down first time: start rectangle
      set(ev.pixX(), ev.pixY());
      ev.worldCanvas()->pixelCanvas()->copyFrontBufferToBackBuffer();
      draw();
      reset();
      itsActive = True;
    }
  }
}

void WCRectTool::keyReleased(const WCPositionEvent &ev) {
  Int x = ev.pixX();
  Int y = ev.pixY();
  if (itsActive && (itsAdjustMode == WCRectTool::Off)) {
    itsActive = False;
    draw(); // erase
    itsLastPressX = itsLastPressY = -99999;
    itsLastReleaseX = itsLastReleaseY = -99999;
    itsLastPressTime = its2ndLastPressTime = -1.0;
    if ((itsX1 == itsX2) && (itsY1 == itsY2)) {
      // null rectangle
      itsRectangleExists = False;
      return;
    }
    draw(True); // redraw with handles
    itsRectangleExists = True;
    preserve();
    rectangleReady();
    //itsLastPressX = itsLastPressY = -99999;
    //itsLastReleaseX = itsLastReleaseY = -99999;
    //itsLastPressTime = its2ndLastPressTime = -1.0;
  } else if (itsActive && (itsAdjustMode == WCRectTool::Move)) {
    itsAdjustMode = WCRectTool::Off;
    itsActive = False;
    draw();
    draw(True);
    preserve();
    rectangleReady();
  }
  if ((abs(ev.timeOfEvent() - its2ndLastPressTime) < doubleClickInterval()) &&
      itsRectangleExists) {
    itsLastReleaseX = itsLastReleaseY = -99999;
    // "double click" & rectangle exists
    if (!itsRectanglePersistent) {
      draw(True); // - erase the rectangle with handles
      reset();
    } else {
      itsActive = False;
      rectangleReady();
      itsMoved = False;
      itsAdjustMode = WCRectTool::Off;
    }
    // now only proceed if we are not in a handle
    Bool left = ((x >= min(itsHX(0), itsHX(1))) && 
		       (x <= max(itsHX(0), itsHX(1))));
    Bool right = ((x >= min(itsHX(2), itsHX(3))) && 
			(x <= max(itsHX(2), itsHX(3))));
    Bool top = ((y >= min(itsHY(0), itsHY(1))) &&
		      (y <= max(itsHY(0), itsHY(1))));
    Bool bottom = ((y >= min(itsHY(2), itsHY(3))) &&
			 (y <= max(itsHY(2), itsHY(3))));
    if ((left || right) && (top || bottom)) {
      return;
    }
    // not in a handle, so potentially emit...
    if ((x >= min(itsX1, itsX2)) && (x <= max(itsX1, itsX2)) &&
	(y >= min(itsY1, itsY2)) && (y <= max(itsY1, itsY2))) {
      doubleInside();
    } else if ((x < min(itsX1, itsX2)) || (x > max(itsX1, itsX2)) ||
	       (y < min(itsY1, itsY2)) || (y > max(itsY1, itsY2))) {
      doubleOutside();
    }
    return;
  }

  itsLastReleaseX = x;
  itsLastReleaseY = y;
}

void WCRectTool::otherKeyPressed(const WCPositionEvent &ev) {
  if (ev.key() == Display::K_Escape) {
    if (itsRectangleExists || itsActive) {
      draw(itsRectangleExists);
      reset();
    }
  }
}

void WCRectTool::moved(const WCMotionEvent &ev) {
  itsMoved = True;
  if (!itsActive) {
    return;
  }
  uInt x = ev.pixX();
  uInt y = ev.pixY();
  if (itsAdjustMode == WCRectTool::Off) {
    draw();
    Int x1, y1;
    get(x1, y1);
    set(x1, y1, x, y);
    draw();
  } else if (itsAdjustMode == WCRectTool::Move) {
    Int x1, y1, x2, y2;
    get(x1, y1, x2, y2);
    Int dx = x - itsBaseMoveX;
    Int dy = y - itsBaseMoveY;
    itsBaseMoveX = x;
    itsBaseMoveY = y;
    draw();
    set(x1 + dx, y1 + dy, x2 + dx, y2 + dy);
    draw();
  }
}

void WCRectTool::refresh(const WCRefreshEvent &ev) {
  if (ev.reason() == Display::BackCopiedToFront) {
    itsOnScreen = False;
  }
  if (!itsActive && itsRectangleExists) {
    restore();
    draw(True);
  } else {
    reset();
  }  
}

void WCRectTool::set(const Int &x1, const Int &y1,
			   const Int &x2, const Int &y2) {
  itsX1 = x1;
  itsY1 = y1;
  itsX2 = x2;
  itsY2 = y2;
}

void WCRectTool::get(Int &x1, Int &y1, Int &x2, Int &y2) const {
  x1 = itsX1;
  y1 = itsY1;
  x2 = itsX2;
  y2 = itsY2;
}

void WCRectTool::set(const Int &x1, const Int &y1) {
  itsX1 = itsX2 = x1;
  itsY1 = itsY2 = y1;
}

void WCRectTool::get(Int &x1, Int &y1) const {
  x1 = itsX1;
  y1 = itsY1;
}

void WCRectTool::preserve() {
  Vector<Double> from(2);
  from(0) = min(itsX1, itsX2);
  from(1) = min(itsY1, itsY2);
  worldCanvas()->pixToWorld(itsStoredWorldBlc, from);
  from(0) = max(itsX1, itsX2);
  from(1) = max(itsY1, itsY2);
  worldCanvas()->pixToWorld(itsStoredWorldTrc, from);
}

void WCRectTool::restore() {
  Vector<Double> to(2);
  worldCanvas()->worldToPix(to, itsStoredWorldBlc);
  itsX1 = Int(to(0) + 0.5);
  itsY1 = Int(to(1) + 0.5);
  worldCanvas()->worldToPix(to, itsStoredWorldTrc);
  itsX2 = Int(to(0) + 0.5);
  itsY2 = Int(to(1) + 0.5);
}

void WCRectTool::draw(const Bool drawHandles) {
  PixelCanvas *pCanvas = pixelCanvas();
  if (itsOnScreen) {
    pCanvas->copyBackBufferToFrontBuffer();
    itsOnScreen = False;
    return;
  }
  itsOnScreen = True;
  Display::DrawBuffer oldBuffer = pCanvas->drawBuffer();
  pCanvas->setDrawBuffer(Display::FrontBuffer);
  pCanvas->setLineWidth(1);
  pCanvas->setCapStyle(Display::CSRound);
  pCanvas->setColor(drawColor());
  pCanvas->setLineWidth(lineWidth());
  pCanvas->setDrawFunction(Display::DFCopy);
  pCanvas->drawRectangle(itsX1,itsY1,itsX2,itsY2);
  if (drawHandles) {
    Int x1 = min(itsX1, itsX2);
    Int x2 = max(itsX1, itsX2);
    Int y1 = min(itsY1, itsY2);
    Int y2 = max(itsY1, itsY2);
    Int w = x2 - x1;
    Int h = y2 - y1;
    Int h1 = 0;
    if (w>=35 && h>=35) { 
      h1 = 6; 
    } else if (w>=20 && h>=20) { 
      h1 = 4; 
    } else if (w>= 9 && h>= 9) { 
      h1 = 2; 
    }
    itsHX(0) = x1 + 1;
    itsHX(1) = x1 + 1 + h1 - 1;
    itsHX(2) = x1 - 1 + w - h1 + 1;
    itsHX(3) = x1 - 1 + w - 1 + 1;
    itsHY(0) = y1 + 1 + 1 - 1;
    itsHY(1) = y1 + 1 + h1 - 1;
    itsHY(2) = y1 - 1 + h - h1 + 1;
    itsHY(3) = y1 - 1 + h;    
    if (h1) {
      // (+1 in drawing because of strange X behaviour!)
      pCanvas->drawFilledRectangle(itsHX(0), itsHY(0) - 1, itsHX(1) + 1, 
				  itsHY(1) + 0);
      pCanvas->drawFilledRectangle(itsHX(2), itsHY(0) - 1, itsHX(3) + 1, 
				  itsHY(1) + 0);
      pCanvas->drawFilledRectangle(itsHX(0), itsHY(2) - 1, itsHX(1) + 1,
				  itsHY(3) + 0);
      pCanvas->drawFilledRectangle(itsHX(2), itsHY(2) - 1, itsHX(3) + 1, 
				  itsHY(3) + 0);
    }
  }
  pCanvas->setDrawBuffer(oldBuffer);
}

void WCRectTool::reset() {
  if (!itsActive && itsRectangleExists) {
    rectangleNotReady();
  }
  itsActive = False;
  itsMoved = False;
  itsRectangleExists = False;
  itsAdjustMode = WCRectTool::Off;
  itsLastPressX = itsLastPressY = -99999;
  its2ndLastPressX = its2ndLastPressY = -99999;
  itsLastReleaseX = itsLastReleaseY = -99999;
  itsLastPressTime = its2ndLastPressTime = -1.0;
}


} //# NAMESPACE CASA - END


//# MWCCrosshairTool.cc: Base class for WorldCanvas event-based crosshair tools
//# Copyright (C) 1999,2000,2001,2002
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
#include <display/DisplayEvents/MWCCrosshairTool.h>
#include <casa/BasicMath/Math.h>
#include <display/DisplayEvents/CrosshairEvent.h>
#include <display/DisplayEvents/MWCEvents.h>

namespace casa { //# NAMESPACE CASA - BEGIN

MWCCrosshairTool::MWCCrosshairTool(Display::KeySym keysym,
				   const Bool persistent) :
  MultiWCTool(keysym),
  itsPos(2),
  itsPersist(persistent),
  itsRadius(9),
  itsShowing(False),
  itsShow(False),
  itsCross(False),
  itsBtnDn(False) {
}

MWCCrosshairTool::~MWCCrosshairTool() {
  reset();
}

void MWCCrosshairTool::disable() {
  reset();
  MultiWCTool::disable();
}

void MWCCrosshairTool::keyPressed(const WCPositionEvent &ev) {
  WorldCanvas *wc = ev.worldCanvas();

  if(itsShowing && itsCurrentWC->pixelCanvas() != wc->pixelCanvas()) reset();
	// Clear drawing on other PC in (unlikely) event that this tool is
        // registered on more than one PC.

  // Record that button is pressed.  This means the crosshair will draw
  // if the mouse moves into the draw area, even if it is outside it now.
  itsBtnDn = True;
  itsCurrentWC = wc;
  Int x = ev.pixX(), y = ev.pixY(); set(x, y);
  itsShow = wc->inDrawArea(x, y);
  if(itsShowing || itsShow) refresh();
	// Cause draw or erase, as necessary

  if(itsShow) {		// valid position chosen--dispatch events.
    CrosshairEvent ev(itsCurrentWC, x,y, "down");
    itsCurrentWC->handleEvent(ev);	// send selected crosshair
					// position to WC for distribution
					// to new handlers.
    //cout << "cross hair click" << endl;
    crosshairReady("down");  }	// send crosshair position ready message
				// to any derived handlers (Gtk...)
}

void MWCCrosshairTool::moved(const WCMotionEvent &ev) {
  if (!itsBtnDn) return;
  WorldCanvas *wc = ev.worldCanvas();
  if (wc != itsCurrentWC) {	// shouldn't happen.
    reset();
    return;  }
  Int x = ev.pixX(), y = ev.pixY(); set(x, y);
  itsShow = wc->inDrawArea(x, y);
  if(itsShowing || itsShow) refresh();
		// Draw or erase as necessary.

  if(itsShow) {		// valid position chosen--dispatch events.
    CrosshairEvent ev(itsCurrentWC, x,y, "move");
    itsCurrentWC->handleEvent(ev);	// send selected crosshair
					// position to WC for distribution
					// to new handlers.
    //cout << "cross hair move" << endl;
    crosshairReady("move");  }	// send crosshair position ready message
				// to any derived handlers (Gtk...)
}

void MWCCrosshairTool::keyReleased(const WCPositionEvent &ev) {
  if (!itsBtnDn) return;
  WorldCanvas *wc = ev.worldCanvas();
  if (wc != itsCurrentWC) {	// shouldn't happen.
    reset();
    return;  }
  itsBtnDn = False;
  Int x = ev.pixX(), y = ev.pixY(); set(x, y);
  Bool inDA = wc->inDrawArea(x, y);
  itsShow = (inDA && itsPersist);
  if(itsShowing || itsShow) refresh();
		// Draw or erase as necessary.

  if(inDA) {		// valid position chosen--dispatch events.
    CrosshairEvent ev(itsCurrentWC, x,y, "up");
    itsCurrentWC->handleEvent(ev);	// send selected crosshair
					// position to WC for distribution
					// to new handlers.
    crosshairReady("up");  }	// send crosshair position ready message
				// to any derived handlers (Gtk...)
}

void MWCCrosshairTool::otherKeyPressed(const WCPositionEvent &ev) {
  if (ev.worldCanvas()==itsCurrentWC && ev.key() == Display::K_Escape)
      reset();
}

void MWCCrosshairTool::set(Int x, Int y) {
  if (!itsCurrentWC) return;
  Vector<Double> pix(2);
  pix(0) = x; pix(1) = y;
  itsCurrentWC->pixToLin(itsPos, pix);
  itsCurrentWC->pixToWorld(itsWorld, pix);
}

void MWCCrosshairTool::get(Int &x, Int &y) const {
  if (!itsCurrentWC) return;
  Vector<Double> pix(2);
  itsCurrentWC->linToPix(pix, itsPos);
  x = ifloor(pix(0) + 0.5);
  y = ifloor(pix(1) + 0.5);
}

void MWCCrosshairTool::getLin(Double &x, Double &y) const {
  if (!itsCurrentWC) return;
  Int a;
  itsPos.shape(a);
  if (a != 2) return;
  x = itsPos(0);
  y = itsPos(1);
}

void MWCCrosshairTool::getWorld(Double &x, Double &y) const {
  if (!itsCurrentWC) return;
  Int a;
  itsWorld.shape(a);
  if (a != 2) return;
  x = itsWorld(0);
  y = itsWorld(1);
}

void MWCCrosshairTool::draw(const WCRefreshEvent &) {
  if(!itsShow) {itsShowing = False; return;  }
  Int x, y; get(x, y);
  if(!itsCurrentWC->inDrawArea(x, y)) { itsShowing = False; return;  }

  PixelCanvas *pCanvas = itsCurrentWC->pixelCanvas();
  setClipToDrawArea();

  pCanvas->setCapStyle(Display::CSRound);
  pCanvas->setColor(drawColor());
  pCanvas->setLineWidth(lineWidth());
  pCanvas->setDrawFunction(Display::DFCopy);
   pCanvas->setLineStyle(Display::LSSolid);

  if (itsCross) {
     pCanvas->setColor("White");
     pCanvas->drawLine(x + itsRadius, y,  x + 3*itsRadius, y);
     pCanvas->drawLine(x - itsRadius, y,  x - 3*itsRadius, y);
     pCanvas->drawLine(x, y + itsRadius,  x, y + 3*itsRadius);
     pCanvas->drawLine(x, y - itsRadius,  x, y - 3*itsRadius);

  }
  else {
     //pCanvas->drawEllipse(x, y,  itsRadius, itsRadius,  0.0);
     pCanvas->drawEllipse(x, y,  itsRadius * 2, itsRadius * 2,  0.0);
     pCanvas->drawLine(x + itsRadius/2, y,  x + 3*itsRadius, y);
     pCanvas->drawLine(x - itsRadius/2, y,  x - 3*itsRadius, y);
     pCanvas->drawLine(x, y + itsRadius/2,  x, y + 3*itsRadius);
     pCanvas->drawLine(x, y - itsRadius/2,  x, y - 3*itsRadius);
  }
  resetClip();
  itsShowing = True;
}

void MWCCrosshairTool::handleEvent(DisplayEvent& ev) {
  // currently just for reset events.
  ResetCrosshairEvent* rchev = dynamic_cast<ResetCrosshairEvent*>(&ev);
  if(rchev != 0) reset(rchev->skipRefresh());
  MultiWCTool::handleEvent(ev);  }	// Let base class handle too.

void MWCCrosshairTool::reset(Bool skipRefresh) {
  itsBtnDn = False;
  itsShow = False;
  if(itsShowing && !skipRefresh) { refresh(); itsShowing = False;  }
}

void MWCCrosshairTool::setCross(Bool cross) {
  itsCross = cross;
}

} //# NAMESPACE CASA - END


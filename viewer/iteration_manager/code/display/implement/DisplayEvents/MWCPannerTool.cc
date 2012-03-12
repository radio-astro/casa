//# MWCPannerTool.cc: MultiWorldCanvas panning tool
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

#include <casa/Arrays/ArrayMath.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/MWCPannerTool.h>
#include <casa/OS/Time.h>

namespace casa { //# NAMESPACE CASA - BEGIN

MWCPannerTool::MWCPannerTool(Display::KeySym keysym,
			     Bool  scrollingAllowed) :
  MultiWCTool(keysym),
  itsActive(False),
  itsScrollingAllowed(scrollingAllowed),
  itsLastScrollTime(0.),
  itsLastKey(-1) {
}

MWCPannerTool::~MWCPannerTool() { reset();
}
  
void MWCPannerTool::keyPressed(const WCPositionEvent &ev) {
  itsX1=itsX2=ev.pixX(); itsY1=itsY2=ev.pixY();
  itsCurrentWC = ev.worldCanvas();
  itsActive = True;
}

void MWCPannerTool::moved(const WCMotionEvent &ev) {
  if (!itsActive) return;
  itsX2 = ev.pixX(); itsY2 = ev.pixY();
  refresh();
}

void MWCPannerTool::keyReleased(const WCPositionEvent &ev) {
  if(!itsActive) return;
  if(itsCurrentWC!=ev.worldCanvas()) { refresh(); return;  }
			//shouldn't happen

  // create shift vector.

  Vector<Double> p1(2),p2(2), linP1(2),linP2(2), shift(2);
  p1(0) = itsX1; p1(1) = itsY1;
  p2(0) = itsX2; p2(1) = itsY2;
  itsCurrentWC->pixToLin(linP1, p1);
  itsCurrentWC->pixToLin(linP2, p2);
  shift = linP1 - linP2;	// To move image by (P2-P1), zoom window
				// must move in opposite direction.

  // execute pan (zoom window shift).  Resets tool, erases line.

  pan(shift);
}

void MWCPannerTool::otherKeyPressed(const WCPositionEvent &ev) {
  if (ev.worldCanvas() == itsCurrentWC &&
	      ev.key() == Display::K_Escape) reset();

  // control panning/scrolling via arrow keys et. al.

  if(!itsScrollingAllowed) return;

  static const uInt panKeys[10] = { 
	Display::K_Right, Display::K_Left, 
	Display::K_Up, Display::K_Down,		// arrows
	Display::K_Pointer_Button4,
	Display::K_Pointer_Button5,		// scroll wheel
	Display::K_Page_Up, Display::K_Page_Down,
	Display::K_Home, Display::K_End };
  static const Int UP=2, WHEELUP=4, WHEELDOWN=5, PAGEUP=6, HOME=8;
	// Indices into above

  uInt k = ev.key();
  Int iKey;  Bool found = False;
  for(iKey=0; iKey<10; iKey++) if(panKeys[iKey]==k) { found = True; break;  }
  if(!found) return;

  if(ev.timeOfEvent() - itsLastScrollTime < .03 &&
     iKey!=WHEELUP && iKey!=WHEELDOWN && 
     iKey==itsLastKey) return;
	// prevent key repeat from piling up scroll events.

  WorldCanvas *wc = ev.worldCanvas();
  itsCurrentWC = wc;

  // compute shift from key pressed

  Vector<Double> shift(2);
  shift=0.;
  if(iKey<HOME) {
    if(iKey<UP) shift(0) = wc->linXMax() - wc->linXMin();  	// right/left
    else        shift(1) = wc->linYMax() - wc->linYMin();  	// up/down
    if(iKey<PAGEUP && (ev.modifiers() & Display::KM_Ctrl)==0 )
	 shift = shift * .16;		// simple arrow key--small shift
    else shift = shift * .9;  }		// shift by (nearly) full window
  else   shift(1) = wc->linYMaxLimit() - wc->linYMinLimit();	// home/end
  if(iKey % 2 == 1) shift = -shift;	// shift direction
  for(Int i=0; i<2; i++) shift(i) = Double(Float(shift(i)));
	// (rounding makes cached drawings more reusable)

  pan(shift);  // execute pan.

  Time t;		// record pan completion time (sec).
  itsLastScrollTime = t.julianDay() * 24.0 * 60.0 * 60.0;
  itsLastKey = iKey;	// record key (avoid too many delayed repeats)
}

void MWCPannerTool::pan(Vector<Double> &shift) {

  // get current zoom window corners.
  WorldCanvas *cwc = itsCurrentWC;
  Vector<Double>  blc(2),trc(2);
  blc(0) = cwc->linXMin(); blc(1) = cwc->linYMin();
  trc(0) = cwc->linXMax(); trc(1) = cwc->linYMax();

  // do not shift beyond zoom extents.
  Double shiftLimit;
  shiftLimit = cwc->linXMinLimit() - blc(0);	// lower shift limit (X)
  if(shift(0) < shiftLimit) shift(0) = shiftLimit;
  shiftLimit = cwc->linYMinLimit() - blc(1);	// lower shift limit (Y)
  if(shift(1) < shiftLimit) shift(1) = shiftLimit;
  shiftLimit = cwc->linXMaxLimit() - trc(0);	// upper shift limit (X)
  if(shift(0) > shiftLimit) shift(0) = shiftLimit;
  shiftLimit = cwc->linYMaxLimit() - trc(1);	// upper shift limit (Y)
  if(shift(1) > shiftLimit) shift(1) = shiftLimit;

  if(shift(0)==0. && shift(1)==0.) { reset(); return;  }

  itsActive=False;	// erases pan line; resets tool w/o
			// unnecessary extra refresh

  // apply shift:
  blc = blc + shift;
  trc = trc + shift;

  // Shift the zoom window for all the tool's WCs.  (Note that
  // all must be using compatible coordinate conversions).
  // (std. WC refresh will refresh MWCTools as well).
  itsWCListIter->toStart();
  while (!itsWCListIter->atEnd()) {
    WorldCanvas *wc = itsWCListIter->getRight();
    wc->setZoomRectangleLCS(blc, trc);
    wc->refresh(Display::LinearCoordinateChange);
    (*itsWCListIter)++;  }

  zoomed(blc, trc);
}

void MWCPannerTool::draw(const WCRefreshEvent &) {
  // draws a line from initial to final mouse position,
  // when the button is down.
  if (!itsActive) return;
  PixelCanvas *pCanvas = itsCurrentWC->pixelCanvas();
  if(!pCanvas) return;
  pCanvas->setLineWidth(2);
  pCanvas->setColor(drawColor());
  pCanvas->setDrawFunction(Display::DFCopy);
  pCanvas->drawLine(itsX1, itsY1, itsX2, itsY2);
}

void MWCPannerTool::reset(Bool skipRefresh) {
  Bool wasActive=itsActive;
  itsActive=False;	// erases pan line (if any) on refresh
  if(wasActive && !skipRefresh) refresh();
}

} //# NAMESPACE CASA - END


//# WCPolyTool.cc: Base class for WorldCanvas event-based polygon tools
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

#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/WCPolyTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	WCPolyTool::WCPolyTool(WorldCanvas *wcanvas,
	                       Display::KeySym keysym,
	                       const Bool persistent) :
		WCTool(wcanvas, keysym),
		itsPolygonPersistent(persistent),
		itsOnScreen(False),
		itsActive(False),
		itsPolygonExists(False),
		itsHandleSize(7) {
		reset();
		itsX.resize(1024);
		itsY.resize(1024);
	}

	WCPolyTool::~WCPolyTool() {
	}

	void WCPolyTool::disable() {
		WCTool::disable();
		if (itsPolygonExists || itsActive) {
			draw(itsPolygonExists);
			reset();
		}
	}

	void WCPolyTool::keyPressed(const WCPositionEvent &ev) {
		Int x = ev.pixX();
		Int y = ev.pixY();
		its2ndLastPressX = itsLastPressX;
		its2ndLastPressY = itsLastPressY;
		its2ndLastPressTime = itsLastPressTime;
		itsLastPressX = x;
		itsLastPressY = y;
		itsLastPressTime = ev.timeOfEvent();
		if (itsPolygonExists && !itsActive) {
			for (uInt i = 0; i < itsNPoints; i++) {
				if (inHandle(i, x, y)) {
					// user has pressed on a handle
					itsSelectedHandle = i;
					itsAdjustMode = WCPolyTool::Handle;
					itsActive = True;
					itsMoved = False;
					polygonNotReady();
					draw(True);
					draw();
					return;
				}
			}
			if (inPolygon(x, y)) {
				// user has pressed inside the polygon
				itsAdjustMode = WCPolyTool::Move;
				itsBaseMoveX = x;
				itsBaseMoveY = y;
				itsActive = True;
				itsMoved = False;
				polygonNotReady();
				draw(True);
				draw();
				return;
			}
		} else {
			if (!itsActive) {
				// key pressed down first time, start polygon
				pushPoint(x, y);
				pushPoint(x, y);
				itsActive = True;
				draw();
			} else if (itsAdjustMode == WCPolyTool::Off) {
				if (inHandle(0, x, y) || inHandle(itsNPoints - 2, x, y)) {
					draw();
					popPoint();
					draw(True);
					itsActive = False;
					itsPolygonExists = True;
					polygonReady();
					preserve();
				} else {
					// key pressed - add point
					draw();
					popPoint();
					pushPoint(x, y);
					pushPoint(x, y);
					draw();
				}
			}
		}
	}

	void WCPolyTool::keyReleased(const WCPositionEvent &ev) {
		Int x = ev.pixX();
		Int y = ev.pixY();
		if (itsPolygonExists) {
			if (itsActive && (itsAdjustMode == WCPolyTool::Handle)) {
				itsAdjustMode = WCPolyTool::Off;
				itsActive = False;
				draw(); // erase
				itsLastPressX = itsLastPressY = -99999;
				itsLastReleaseX = itsLastReleaseY = -99999;
				itsLastPressTime = its2ndLastPressTime = -1.0;
				draw(True); // redraw with handles
				preserve();
				polygonReady();

			} else if (itsActive && (itsAdjustMode == WCPolyTool::Move)) {
				itsAdjustMode = WCPolyTool::Off;
				itsActive = False;
				draw();
				draw(True);
				preserve();
				polygonReady();
			}
		}
		//if ((x == itsLastReleaseX) && (y == itsLastReleaseY) &&
		//   (x == its2ndLastPressX) && (y == its2ndLastPressY) &&
		//    (x == itsLastPressX) && (y == itsLastPressY) &&
		if ((abs(ev.timeOfEvent() - its2ndLastPressTime) < doubleClickInterval()) &&
		        itsPolygonExists) {
			itsLastReleaseX = itsLastReleaseY = -99999;
			// "double click" & polygon exists
			if (!itsPolygonPersistent) {
				draw(True); // - erase the polygon with handles
				reset();
			} else {
				itsActive = False;
				polygonReady();
				itsMoved = False;
				itsAdjustMode = WCPolyTool::Off;
			}
			// now only proceed if we are not in a handle
			for (uInt i = 0; i < itsNPoints; i++) {
				if (inHandle(i, x, y)) {
					return;
				}
			}
			// not in a handle, so potentially emit...
			if (inPolygon(x, y)) {
				doubleInside();
			} else {
				doubleOutside();
			}
			return;
		}

		itsLastReleaseX = x;
		itsLastReleaseY = y;
	}

	void WCPolyTool::otherKeyPressed(const WCPositionEvent &ev) {
		if (ev.key() == Display::K_Escape) {
			if (itsPolygonExists || itsActive) {
				draw(itsPolygonExists && !(itsAdjustMode == WCPolyTool::Handle));
				//polygonNotReady();
				reset();
			}
		}
	}

	void WCPolyTool::moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/) {
		itsMoved = True;
		if (!itsActive) {
			return;
		}
		uInt x = ev.pixX();
		uInt y = ev.pixY();
		if (itsAdjustMode == WCPolyTool::Off) {
			draw();
			popPoint(); // pop existing moving position
			pushPoint(x, y); // push new moving position
			draw();
		} else if (itsAdjustMode == WCPolyTool::Move) {
			draw();
			Int dx = x - itsBaseMoveX;
			Int dy = y - itsBaseMoveY;
			itsX = itsX + dx;
			itsY = itsY + dy;
			draw();
			preserve();
			itsBaseMoveX = x;
			itsBaseMoveY = y;
		} else if (itsAdjustMode == WCPolyTool::Handle) {
			draw();
			itsX(itsSelectedHandle) = x;
			itsY(itsSelectedHandle) = y;
			draw();
		}
	}

	void WCPolyTool::refresh(const WCRefreshEvent &ev) {
		if (ev.reason() == Display::BackCopiedToFront) {
			itsOnScreen = False;
		}
		if (!itsActive && itsPolygonExists) {
			restore();
			draw(True);
		} else {
			reset();
		}
	}

	void WCPolyTool::get(Vector<Int> &x, Vector<Int> &y) {
		x.resize(itsNPoints);
		y.resize(itsNPoints);
		for (uInt i = 0; i < itsNPoints; i++) {
			x(i) = itsX(i);
			y(i) = itsY(i);
		}
	}

	void WCPolyTool::preserve() {
		itsStoredWorldX.resize(itsNPoints);
		itsStoredWorldY.resize(itsNPoints);
		Vector<Double> to(2), from(2);
		for (uInt i = 0; i < itsNPoints; i++) {
			from(0) = itsX(i);
			from(1) = itsY(i);
			worldCanvas()->pixToWorld(to, from);
			itsStoredWorldX(i) = to(0);
			itsStoredWorldY(i) = to(1);
		}
	}

	void WCPolyTool::restore() {
		Vector<Double> to(2), from(2);
		for (uInt i = 0; i < itsNPoints; i++) {
			from(0) = itsStoredWorldX(i);
			from(1) = itsStoredWorldY(i);
			worldCanvas()->worldToPix(to, from);
			itsX(i) = Int(to(0) + 0.5);
			itsY(i) = Int(to(1) + 0.5);
		}
	}

	void WCPolyTool::pushPoint(uInt x1, uInt y1) {
		if (itsNPoints < itsX.nelements() - 1) {
			itsNPoints++;
			itsX(itsNPoints - 1) = x1;
			itsY(itsNPoints - 1) = y1;
		}
	}

	void WCPolyTool::popPoint() {
		if (itsNPoints > 0) {
			itsNPoints--;
		}
	}

	void WCPolyTool::draw(const Bool drawHandles) {
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
		pCanvas->setDrawFunction(Display::DFCopy);

		Vector<Int> xp(itsNPoints), yp(itsNPoints);
		for (uInt i = 0; i < itsNPoints; i++) {
			xp(i) = itsX(i);
			yp(i) = itsY(i);
		}

		pCanvas->drawPolygon(xp, yp);

		if (drawHandles) {
			Int del = (itsHandleSize - 1) / 2;
			Int x, y;
			for (uInt i = 0; i < itsNPoints; i++) {
				x = itsX(i);
				y = itsY(i);
				// (+1 in drawing because of strange X behaviour!)
				pCanvas->drawFilledRectangle(x - del, y - del - 1, x + del + 1,
				                             y + del + 1);
			}
		}
		pCanvas->setDrawBuffer(oldBuffer);
	}


	void WCPolyTool::reset() {
		if (!itsActive && itsPolygonExists) {
			polygonNotReady();
		}
		itsActive = False;
		itsMoved = False;
		itsPolygonExists = False;
		itsAdjustMode = WCPolyTool::Off;
		itsNPoints = 0;
		itsLastPressX = itsLastPressY = -99999;
		its2ndLastPressX = its2ndLastPressY = -99999;
		itsLastReleaseX = itsLastReleaseY = -99999;
		itsLastPressTime = its2ndLastPressTime = -1.0;
	}

	Bool WCPolyTool::inHandle(const uInt &pt, const uInt &x,
	                          const uInt &y) const {
		if (pt >= itsNPoints) {
			return False;
		}
		uInt ptx = itsX(pt);
		uInt pty = itsY(pt);
		uInt del = (itsHandleSize - 1) / 2;
		return ((x >= ptx - del) && (x <= ptx + del) &&
		        (y >= pty - del) && (y <= pty + del));
	}

	Bool WCPolyTool::inPolygon(const uInt &x, const uInt &y) const {
		uInt nabove = 0, nbelow = 0; // counts of crossing lines above and below

		uInt i, j;
		for (i = 0; i < itsNPoints; i++) {
			if (i > 0) {
				j = i - 1;
			} else {
				j = itsNPoints - 1;
			}
			if ((min(itsX(j), itsX(i)) < (Int)x) && (max(itsX(j), itsX(i)) > (Int)x)) {
				Float ycut = (Float)itsY(j) + (Float)(itsY(i) - itsY(j)) /
				             (Float)(itsX(i) - itsX(j)) * (Float)((Int)x - itsX(j));
				if (ycut > (Float)y) {
					nabove++;
				} else {
					nbelow++;
				}
			}
		}

		if ((nabove + nbelow) % 2) {
			// not even - possibly on a line of the polygon.
			return True;
		} else {
			if (nabove % 2) {
				return True;
			} else {
				return False;
			}
		}
		return False; // should never get here
	}


} //# NAMESPACE CASA - END


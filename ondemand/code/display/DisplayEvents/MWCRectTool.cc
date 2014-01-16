//# MWCRectTool.cc: Base class for MultiWorldCanvas event-based rectangle tools
//# Copyright (C) 2000,2001,2002
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
#include <display/DisplayEvents/MWCRectTool.h>
#include <casa/BasicMath/Math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	MWCRectTool::MWCRectTool(Display::KeySym keysym,
	                         const Bool persistent) :
		MultiWCTool(keysym),
		itsRectanglePersistent(persistent),
		itsRectangleExists(False),
		itsActive(False),
		itsP1(2), itsP2(2),
		itsHX(4), itsHY(4),
		itsLastPressTime(-1.),	// 'long ago..'
		its2ndLastPressTime(-1.) {
	}

	MWCRectTool::~MWCRectTool() {
		reset();
	}

	void MWCRectTool::disable() {
		reset();
		MultiWCTool::disable();
	}

	void MWCRectTool::keyPressed(const WCPositionEvent &ev) {

		its2ndLastPressTime = itsLastPressTime;
		itsLastPressTime = ev.timeOfEvent();

		if(itsActive) reset();	// (shouldn't happen)

		Int x = ev.pixX();
		Int y = ev.pixY();

		clicked(x, y);

		if (itsRectangleExists && ev.worldCanvas()==itsCurrentWC) {

			// press on the the WC that has the existing rectangle.

			Int x1,y1, x2,y2;
			get(x1,y1, x2,y2);

			Bool left  = (x >= min(itsHX(0),itsHX(1)) && x <= max(itsHX(0),itsHX(1)));
			Bool right = (x >= min(itsHX(2),itsHX(3)) && x <= max(itsHX(2),itsHX(3)));
			Bool top   = (y >= min(itsHY(0),itsHY(1)) && y <= max(itsHY(0),itsHY(1)));
			Bool bottom= (y >= min(itsHY(2),itsHY(3)) && y <= max(itsHY(2),itsHY(3)));

			if ((left || right) && (top || bottom)) {

				// user has pressed on a handle
				// shuffle so that anchor corner (x1,y1) is opposite to corner pressed.

				if ((left && (x1 < x2)) || (right && (x1 > x2))) {
					Int tmp = x1;
					x1 = x2;
					x2 = tmp;
				}
				if ((top && (y1 < y2)) || (bottom && (y1 > y2))) {
					Int tmp = y1;
					y1 = y2;
					y2 = tmp;
				}
				set(x1, y1, x2, y2);

				itsActive = True;
				itsMoving = False;	// enter resizing state
				return;
			}

			if ( x >= min(x1, x2) && x <= max(x1, x2) &&
			        y >= min(y1, y2) && y <= max(y1, y2)) {

				// user has pressed inside the rectangle

				itsActive = True;
				itsMoving = True;		// enter moving state
				itsBaseMoveX = x;
				itsBaseMoveY = y;
				return;
			}

			if(!itsEmitted) return;
		}  // ignore click outside the rectangle
		// if it hasn't been emitted yet.
		// But if it has...

		// start new rectangle

		if(itsRectangleExists) reset();
		// erase old rectangle, if any. (In this case, either it has
		// already been emitted and the press was outside it,
		// or the press was on a different WC).
		itsCurrentWC = ev.worldCanvas();
		set(x,y, x,y);
		itsActive = True;
		itsMoving = False;	// enter resizing state
		rectangleReady();
		return;
	}

	void MWCRectTool::moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/) {
		if (!itsActive) return;
		if (ev.worldCanvas() != itsCurrentWC) return;  // shouldn't happen

		uInt x = ev.pixX();
		uInt y = ev.pixY();

		if (itsMoving) {	// move the rectangle
			Int x1, y1, x2, y2;
			get(x1, y1, x2, y2);
			Int dx = x - itsBaseMoveX;
			Int dy = y - itsBaseMoveY;
			itsBaseMoveX = x;
			itsBaseMoveY = y;
			set(x1 + dx, y1 + dy, x2 + dx, y2 + dy);
		}

		else {		// resize the rectangle
			Int x1, y1;
			get(x1, y1);
			set(x1, y1, x, y);
			itsRectangleExists = True;
		}

		itsEmitted = False;	// (changed) rectangle has never been emitted.
		refresh();		// draw over in new state
		updateRegion();
	}

	void MWCRectTool::keyReleased(const WCPositionEvent &ev) {
		Bool wasActive = itsActive;
		itsActive = False;
		if (!itsRectangleExists) {
			if (ev.timeOfEvent() - its2ndLastPressTime < doubleClickInterval()) {
				Int x = ev.pixX();
				Int y = ev.pixY();
				doubleClicked(x, y);
			} else
				return;
		}

		if(ev.worldCanvas() != itsCurrentWC) {
			reset();    // shouldn't happen.
			return;
		}

		if (wasActive && !itsMoving) {  // resize finished.
			Int x1, y1, x2, y2;
			get(x1,y1, x2,y2);
			if (abs(x1 - x2) <= 2 && abs(y1 - y2) <= 2 ) {
				reset();      // rectangle too small, probably accidental
				return;
			}
		}

		if (ev.timeOfEvent() - its2ndLastPressTime < doubleClickInterval()) {

			// double click--invoke callbacks

			itsEmitted = True;
			itsLastPressTime = its2ndLastPressTime = -1.0;
			if (!itsRectanglePersistent) reset();	// NB: rect. coordinates &
			else refresh();	// current WC still valid, until new rect. started.
			// In particular, still valid during callbacks below.

			Int x = ev.pixX(), y = ev.pixY();
			Int x1, y1, x2, y2;
			get(x1,y1, x2,y2);
			if (x >= min(x1, x2) && x <= max(x1, x2) &&
			        y >= min(y1, y2) && y <= max(y1, y2))
				doubleInside();
			else doubleOutside();
		}

		else if(wasActive) {	// end of move or sizing of rectangle:
			refresh();		// redraw with handles
			rectangleReady();
		}
		// (this callback is unused on glish level (12/01))
	}


	void MWCRectTool::otherKeyPressed(const WCPositionEvent &ev) {
		if (ev.worldCanvas() == itsCurrentWC &&
		        ev.key() == Display::K_Escape)  reset();
	}


	void MWCRectTool::set(const Int &x1, const Int &y1,
	                      const Int &x2, const Int &y2) {
		if (!itsCurrentWC) return;
		Vector<Double> pix(2);
		pix(0)=x1;
		pix(1)=y1;
		itsCurrentWC->pixToLin(itsP1, pix);
		pix(0)=x2;
		pix(1)=y2;
		itsCurrentWC->pixToLin(itsP2, pix);
	}

	void MWCRectTool::get(Int &x1, Int &y1, Int &x2, Int &y2) const {
		if (!itsCurrentWC) return;
		get(x1,y1);
		Vector<Double> pix(2);
		itsCurrentWC->linToPix(pix, itsP2);
		x2 = ifloor(pix(0)+.5);
		y2 = ifloor(pix(1)+.5);
	}

	void MWCRectTool::get(Int &x1, Int &y1) const {
		if (!itsCurrentWC) return;
		Vector<Double> pix(2);
		itsCurrentWC->linToPix(pix, itsP1);
		x1 = ifloor(pix(0)+.5);
		y1 = ifloor(pix(1)+.5);
	}


	void MWCRectTool::draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/) {
		if(!itsRectangleExists) return;

		setClipToWC();
		PixelCanvas *pCanvas = itsCurrentWC->pixelCanvas();

		pCanvas->setLineWidth(1);
		pCanvas->setCapStyle(Display::CSRound);
		pCanvas->setColor(drawColor());
		pCanvas->setLineWidth(lineWidth());
		pCanvas->setDrawFunction(Display::DFCopy);

		Int X1, Y1, X2, Y2;
		get(X1,Y1, X2,Y2);
		pCanvas->drawRectangle(X1,Y1,X2,Y2);

		if (!itsActive) {	// show handles too.
			Int x1 = min(X1, X2);
			Int x2 = max(X1, X2);
			Int y1 = min(Y1, Y2);
			Int y2 = max(Y1, Y2);
			Int w = x2 - x1;
			Int h = y2 - y1;

			Int s = 0;		// handle size
			if (w>=35 && h>=35) s = 6;
			else if (w>=20 && h>=20) s = 4;
			else if (w>= 9 && h>= 9) s = 3;

			itsHX(0) = x1;
			itsHX(1) = x1 + s;
			itsHX(2) = x2 - s;
			itsHX(3) = x2;
			itsHY(0) = y1;
			itsHY(1) = y1 + s;
			itsHY(2) = y2 - s;
			itsHY(3) = y2;	// set handle coordinates
			if (s) {
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
		resetClip();
	}

	void MWCRectTool::reset(Bool skipRefresh) {
		itsActive = False;
		Bool wasShowing = itsRectangleExists;
		itsRectangleExists = False;
		if(wasShowing && !skipRefresh) refresh();
		itsLastPressTime = its2ndLastPressTime = -1.;
	}

} //# NAMESPACE CASA - END


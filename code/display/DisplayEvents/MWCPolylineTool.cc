//# MWCPolylineTool.cc: Base class for MulitWorldCanvas event-based polygon tools
//# Copyright (C) 2003
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
#include <scimath/Mathematics.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/MWCPolylineTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	MWCPolylineTool::MWCPolylineTool(Display::KeySym keysym,
	                                 const Bool persistent) :
		MultiWCTool(keysym),
		itsPolylinePersistent(persistent),
		itsMode(Off),
		itsEmitted(False),
		itsNPoints(0),
		itsHandleSize(5) {
		reset();
		itsX.resize(1024);
		itsY.resize(1024);
	}

	MWCPolylineTool::~MWCPolylineTool() {
	}

	void MWCPolylineTool::disable() {
		reset();
		MultiWCTool::disable();
	}

	void MWCPolylineTool::keyPressed(const WCPositionEvent &ev) {
		Int x = ev.pixX();
		Int y = ev.pixY();
		its2ndLastPressTime = itsLastPressTime;
		itsLastPressTime = ev.timeOfEvent();

		if (itsMode==Def) {
			// (still) defining new polyline.
			if( ev.worldCanvas() != itsCurrentWC || !itsCurrentWC->inDrawArea(x,y) ) {
				return;
			}
			if ( inHandle(itsNPoints-2, x, y) ) {
				// 2nd press on first or last point--polylin ready

				popPoint();
				if (itsNPoints<1) {
					reset();
					return;
				}
				itsMode = Ready;
				itsLastPressTime = its2ndLastPressTime = -1.0;
				refresh();
				polylineReady();
			} else {	// key pressed elsewhere - add point
				popPoint();
				pushPoint(x, y);
				pushPoint(x, y);
			}
			return;
		}


		if ( itsMode==Ready && ev.worldCanvas()==itsCurrentWC ) {

			// Click on WC with previously defined polygon

			for (Int i = 0; i < itsNPoints; i++) {
				if (inHandle(i, x, y)) {
					// user has pressed on a handle
					itsSelectedHandle = i;
					itsMode = Resize;
					refresh();
					return;
				}
			}

			if (inPolyline(x, y)) {
				// user has pressed on the slice
				itsMode = Move;
				itsBaseMoveX = x;
				itsBaseMoveY = y;
				refresh();
				return;
			}

			if (!itsEmitted) return;

		}
		// if polygon was already emitted, code below will erase it
		// and start a new one.

		if (itsMode==Move || itsMode==Resize) return;
		// shouldn't happen; last button release should have
		// taken it out of Move or Resize state.


		// no previously existing polyline,
		// or click in a different WC,
		// or click outside a polyline already emitted:

		// Start new polyline

		if (itsMode!=Off) {
			reset();	// erase old one, if any.
		}
		itsCurrentWC = ev.worldCanvas();
		itsNPoints=0;
		pushPoint(x, y);
		pushPoint(x, y);
		itsMode = Def;
		return;
	}


	void MWCPolylineTool::moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/) {
		if ( itsMode==Off || itsMode==Ready) return;
		if ( ev.worldCanvas()!=itsCurrentWC ) return;

		Int x = ev.pixX();
		Int y = ev.pixY();
		if ( !itsCurrentWC->inDrawArea(x,y) ) return;

		if (itsMode == Def) {
			popPoint();		// pop existing moving position
			pushPoint(x, y);  	// push new moving position
		} else if (itsMode == Move) {
			Int dx = x-itsBaseMoveX,  dy = y-itsBaseMoveY;
			Vector<Int> pX, pY;
			get(pX, pY);
			pX = pX + dx;
			pY = pY + dy;
			set(pX, pY);	// move all the points by (dx,dy)
			itsBaseMoveX = x;
			itsBaseMoveY = y;
		} else if (itsMode == Resize) {
			set(x,y, itsSelectedHandle);  	// move selected vertex.
		}
		itsEmitted = False;  // changed polygon => not yet emitted.
		refresh();
	}


	void MWCPolylineTool::keyReleased(const WCPositionEvent &/*ev*/) {
		Bool needsHandles=False;
		if ( itsMode==Move || itsMode==Resize ) {
			itsMode=Ready;
			needsHandles=True;
		}

		if (needsHandles) {
			refresh();
			polylineReady();
		}
	}

	void MWCPolylineTool::otherKeyPressed(const WCPositionEvent &ev) {
		if (ev.worldCanvas() == itsCurrentWC &&
		        ev.key() == Display::K_Escape) {
			reset();
		}
	}


	void MWCPolylineTool::draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/) {
		if (itsMode==Off) return;

		setClipToDrawArea();
		PixelCanvas *pCanvas = itsCurrentWC->pixelCanvas();

		pCanvas->setLineWidth(1);
		pCanvas->setCapStyle(Display::CSRound);
		pCanvas->setColor(drawColor());
		pCanvas->setDrawFunction(Display::DFCopy);

		Vector<Int> xp(itsNPoints), yp(itsNPoints);
		get(xp, yp);

		for(Int i=0; i<itsNPoints-1; i++) {
			pCanvas->drawLine(xp(i),yp(i), xp(i+1),yp(i+1));
		}

		//if (itsMode!=Def) {	//close only after defined.
		//pCanvas->drawLine(xp(itsNPoints-1),yp(itsNPoints-1), xp(0),yp(0));
		//}

		if (itsMode==Ready) {		// draw handles
			Int del = (itsHandleSize - 1) / 2;
			Int x, y;
			for (Int i = 0; i < itsNPoints; i++) {
				get(x,y, i);
				// (+1 in drawing because of strange X behaviour!)
				pCanvas->drawFilledRectangle(x-del,   y-del-1,
				                             x+del+1, y+del+1);
			}
		}
		resetClip();
	}


	void MWCPolylineTool::reset(Bool skipRefresh) {
		Bool existed = (itsMode!=Off);
		itsMode = Off;
		itsEmitted = False;
		itsLastPressTime = its2ndLastPressTime = -1.0;
		if (existed && !skipRefresh) refresh();	// erase old drawing if necessary.
	}

	void MWCPolylineTool::getLinear(Vector<Float> &x, Vector<Float> &y) const {
		x.resize(itsNPoints);
		y.resize(itsNPoints);
		for (Int i = 0; i < itsNPoints; i++) {
			x(i) = Float(itsX(i));
			y(i) = Float(itsY(i));
		}
	}

	void MWCPolylineTool::get(Vector<Int> &x, Vector<Int> &y) const {
		if (!itsCurrentWC) return;
		x.resize(itsNPoints);
		y.resize(itsNPoints);
		Int ix, iy;
		for (Int i = 0; i < itsNPoints; i++) {
			get(ix,iy, i);
			x(i)=ix;
			y(i)=iy;
		}
	}

	void MWCPolylineTool::get(Int &x, Int &y, const Int pt) const {
		if (!itsCurrentWC || pt>=itsNPoints) return;
		Vector<Double> pix(2), lin(2);
		lin(0) = itsX(pt);
		lin(1) = itsY(pt);
		itsCurrentWC->linToPix(pix, lin);
		x = ifloor(pix(0) + 0.5);
		y = ifloor(pix(1) + 0.5);
	}

	void MWCPolylineTool::set(const Vector<Int> &x, const Vector<Int> &y) {
		if (!itsCurrentWC) return;
		if (x.shape()<itsNPoints || y.shape()<itsNPoints) return;
		Int ix, iy;
		for (Int i = 0; i < itsNPoints; i++) {
			ix = x(i);
			iy = y(i);
			set(ix, iy, i);
		}
	}

	void MWCPolylineTool::set(const Int x, const Int y, const Int pt) {
		if(!itsCurrentWC || pt>=Int(itsX.nelements())) return;
		Vector<Double> pix(2), lin(2);
		pix(0) = x;
		pix(1) = y;
		itsCurrentWC->pixToLin(lin, pix);
		itsX(pt) = lin(0);
		itsY(pt) = lin(1);
	}

	void MWCPolylineTool::pushPoint(Int x, Int y) {
		if (itsNPoints < Int(itsX.nelements())) {
			set(x,y, itsNPoints);
			itsNPoints++;
		}
	}

	void MWCPolylineTool::popPoint() {
		if (itsNPoints > 0) itsNPoints--;
	}

	Bool MWCPolylineTool::inHandle(const Int &pt,
	                               const Int &x, const Int &y) const {
		if (pt<0 || pt >= itsNPoints) return False;

		Int ptx,pty;
		get(ptx,pty, pt);
		Int del = (itsHandleSize - 1) / 2;
		return (x >= ptx - del  &&  x <= ptx + del &&
		        y >= pty - del  &&  y <= pty + del);
	}


	Bool MWCPolylineTool::inPolyline(const Int &xPos, const Int &yPos) const {
		Bool onLine = False;

		Vector<Int> pX, pY;
		get(pX, pY);
		// Calculate distance from point - line
		// This might be too slow?

		for (uInt i=0; i< (uInt(itsNPoints)-1); i++) {
			Float u, x, y;

			// Calculate where a tangent to the line meets with the point
			u = Float( ( (xPos-pX(i)) * (pX(i+1)-pX(i)) )
			           + ( (yPos-pY(i)) * (pY(i+1)-pY(i)) ) )
			    / Float((square(pX(i+1)-pX(i)) )
			            + (square(pY(i+1)-pY(i)) ));

			x = Float(pX(i)) + u * Float( pX(i+1) - pX(i) );
			y = Float(pY(i)) + u * Float( pY(i+1) - pY(i) );

			// Now check that the tangent hits the line in between the two
			// points This might not be needed for this purpose - although
			// might be good when we get thick lines

			Float P2P = hypot(Float(pX(i)-pX(i+1)) , Float(pY(i)-pY(i+1)));
			if ((P2P > hypot( Float(pX(i)-x), Float(pY(i)-y)))
			        && (P2P > hypot(Float(pX(i+1)-x), Float(pY(i+1)-y)))) {

				Float distance(hypot(Float(xPos - x), Float(yPos - y)));
				// x,y is the point on the line where the tanget from the point meets it
				if (Int(distance+0.5) <= 3) {
					// This should be itsThreshold or something I guess
					onLine = True;
					break;
				}
			}
		}
		//if ( onLine )

		return onLine;
	}

} //# NAMESPACE CASA - END


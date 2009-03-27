//# MWCPolyTool.cc: Base class for MulitWorldCanvas event-based polygon tools
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

#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/MWCPolyTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

MWCPolyTool::MWCPolyTool(Display::KeySym keysym,
		       const Bool persistent) :
  MultiWCTool(keysym),
  itsPolygonPersistent(persistent),
  itsMode(Off),
  itsEmitted(False),
  itsNPoints(0),
  itsHandleSize(7) {
  reset();
  itsX.resize(1024);
  itsY.resize(1024);  }

MWCPolyTool::~MWCPolyTool() {  }

void MWCPolyTool::disable() {
  reset();
  MultiWCTool::disable();  }

void MWCPolyTool::keyPressed(const WCPositionEvent &ev) {
  Int x = ev.pixX();
  Int y = ev.pixY();
  its2ndLastPressTime = itsLastPressTime;
  itsLastPressTime = ev.timeOfEvent();


  if(itsMode==Def) {

    // (still) defining new polygon.

    if( ev.worldCanvas()!=itsCurrentWC || !itsCurrentWC->inDrawArea(x,y) )
        return;		// ignore these.

    if (inHandle(0, x, y) || inHandle(itsNPoints - 2, x, y)) {

      // 2nd press on first or last point--polygon ready

      popPoint();
      if(itsNPoints<3) {reset(); return;  }	// ignore <3 points
      itsMode = Ready;
      itsLastPressTime = its2ndLastPressTime = -1.0;
      refresh();
      polygonReady();  }
	// this callback is unused (and useless?) on glish level (12/01)

    else {	// key pressed elsewhere - add point
	popPoint();
	pushPoint(x, y);
	pushPoint(x, y);  }
    return;  }     


  if(itsMode==Ready && ev.worldCanvas() == itsCurrentWC) {

    // Click on WC with previously defined polygon

    for (Int i = 0; i < itsNPoints; i++) if (inHandle(i, x, y)) {

      // user has pressed on a handle

      itsSelectedHandle = i;
      itsMode = Resize;
      refresh();
      return;  }

    if (inPolygon(x, y)) {

      // user has pressed inside the polygon

      itsMode = Move;
      itsBaseMoveX = x;
      itsBaseMoveY = y;
      refresh();
      return;  }

    // click outside polygon.

    // Next line disabled 5/07 dk.  Any click outside a defined polygon
    // will now start a new one, regardless of whether the poly has been 
    // double-clicked _after_ definition (polygonReady() _has_ been
    // emitted in any case).  Maintenance of itsEmitted is now superfluous.
 // if(!itsEmitted) return;
	// if polygon was already emitted, code below will erase it
	// and start a new one.
  }
  
  if(itsMode==Move || itsMode==Resize) return;
		// shouldn't happen; last button release should have
		// taken it out of Move or Resize state.


  // no previously existing polygon,
  // or click in a different WC,
  // or click outside a polygon already emitted:
  
  // Start new polygon

  if(itsMode!=Off) reset();	// erase old one, if any.
  itsCurrentWC = ev.worldCanvas();
  itsNPoints=0;
  pushPoint(x, y);
  pushPoint(x, y);
  itsMode = Def;
  return;
}


void MWCPolyTool::moved(const WCMotionEvent &ev) {
  if(itsMode==Off || itsMode==Ready) return;
  if(ev.worldCanvas()!=itsCurrentWC) return;
  Int x = ev.pixX();
  Int y = ev.pixY();
  if(!itsCurrentWC->inDrawArea(x,y)) return;

  if(itsMode == Def) {
    popPoint();		// pop existing moving position
    pushPoint(x, y);  }	// push new moving position

  else if(itsMode == Move) {
    Int dx = x-itsBaseMoveX,  dy = y-itsBaseMoveY;
    Vector<Int> pX, pY;
    get(pX, pY);
    pX = pX + dx;
    pY = pY + dy;
    set(pX, pY);	// move all the points by (dx,dy)
    itsBaseMoveX = x;
    itsBaseMoveY = y;  
    updateRegion(); }

  else if (itsMode == Resize) {
    set(x,y, itsSelectedHandle); // move selected vertex.
    updateRegion(); }

  itsEmitted = False;  // changed polygon => not yet emitted.
  refresh();  
}


void MWCPolyTool::keyReleased(const WCPositionEvent &ev) {
  Bool needsHandles=False;
  if(itsMode==Move || itsMode==Resize) {
    itsMode=Ready;
    needsHandles=True;  }

  if (itsMode==Ready &&
      ev.worldCanvas()==itsCurrentWC && 
      ev.timeOfEvent()-its2ndLastPressTime < doubleClickInterval() )  {
    Int x = ev.pixX();
    Int y = ev.pixY();
    if(itsCurrentWC->inDrawArea(x,y)) {

      // "double click" in draw area & polygon exists

      itsLastPressTime = its2ndLastPressTime = -1.0;
				// reset dbl click timing

      if (!itsPolygonPersistent) reset();
      else {
        itsEmitted = True;
        if(needsHandles) refresh();  }
		// vertices and WC still remain valid until next
		// polygon started. In particular, during callbacks below.

      if (inPolygon(x, y)) doubleInside();
      else doubleOutside();

      return;  }  }

  if(needsHandles) {
    refresh();
    polygonReady();  }	}
	// this callback is unused (and useless?) on glish level (12/01)


void MWCPolyTool::otherKeyPressed(const WCPositionEvent &ev) {
  if (ev.worldCanvas() == itsCurrentWC &&
	      ev.key() == Display::K_Escape) reset();  }


void MWCPolyTool::draw(const WCRefreshEvent &) { 
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
    pCanvas->drawLine(xp(i),yp(i), xp(i+1),yp(i+1));  }

  if(itsMode!=Def) {	//close only after defined.
    pCanvas->drawLine(xp(itsNPoints-1),yp(itsNPoints-1), xp(0),yp(0));  }

  if (itsMode==Ready) {		// draw handles
    Int del = (itsHandleSize - 1) / 2;
    Int x, y;
    for (Int i = 0; i < itsNPoints; i++) {
      get(x,y, i);
      // (+1 in drawing because of strange X behaviour!)
      pCanvas->drawFilledRectangle(x-del,   y-del-1,
				   x+del+1, y+del+1);  }  }
  resetClip();  }


void MWCPolyTool::reset(Bool skipRefresh) {
  Bool existed = (itsMode!=Off);
  itsMode = Off;
  itsEmitted = False;
  itsLastPressTime = its2ndLastPressTime = -1.0;
  if(existed && !skipRefresh) refresh();  }	// erase old drawing if necessary.



void MWCPolyTool::get(Vector<Int> &x, Vector<Int> &y) const {
  if(!itsCurrentWC) return;
  x.resize(itsNPoints);
  y.resize(itsNPoints);
  Int ix, iy;
  for (Int i = 0; i < itsNPoints; i++) {
    get(ix,iy, i);
    x(i)=ix; y(i)=iy;  }  }

void MWCPolyTool::get(Int &x, Int &y, const Int pt) const {
  if(!itsCurrentWC || pt>=itsNPoints) return;
  Vector<Double> pix(2), lin(2);
  lin(0) = itsX(pt);
  lin(1) = itsY(pt);
  itsCurrentWC->linToPix(pix, lin);
  x = ifloor(pix(0) + 0.5);
  y = ifloor(pix(1) + 0.5);  }

void MWCPolyTool::set(const Vector<Int> &x, const Vector<Int> &y) {
  if (!itsCurrentWC) return;
  if(x.shape()<itsNPoints || y.shape()<itsNPoints) return;
  Int ix, iy;
  for (Int i = 0; i < itsNPoints; i++) {
    ix = x(i); iy = y(i);
    set(ix, iy, i);  }  }

void MWCPolyTool::set(const Int x, const Int y, const Int pt) {
  if(!itsCurrentWC || pt>=Int(itsX.nelements())) return;
  Vector<Double> pix(2), lin(2);
  pix(0) = x;
  pix(1) = y;
  itsCurrentWC->pixToLin(lin, pix);
  itsX(pt) = lin(0);
  itsY(pt) = lin(1);  }

void MWCPolyTool::pushPoint(Int x, Int y) {
  if (itsNPoints < Int(itsX.nelements())) {
    set(x,y, itsNPoints);
    itsNPoints++;  }  }

void MWCPolyTool::popPoint() {
  if (itsNPoints > 0) itsNPoints--;  }

Bool MWCPolyTool::inHandle(const Int &pt, const Int &x, 
			  const Int &y) const {
  if (pt<0 || pt >= itsNPoints) return False;

  Int ptx,pty;
  get(ptx,pty, pt);
  Int del = (itsHandleSize - 1) / 2;
  return (x >= ptx - del  &&  x <= ptx + del &&
	  y >= pty - del  &&  y <= pty + del);  }

Bool MWCPolyTool::inPolygon(const Int &x, const Int &y) const {
  Int nabove = 0, nbelow = 0; // counts of crossing lines above and below
  
  Vector<Int> pX, pY;
  get(pX, pY);
  Int i, j;
  for (i = 0; i < itsNPoints; i++) {
    if (i > 0) j = i - 1;
    else j = itsNPoints - 1;

    if (min(pX(j), pX(i)) < x && max(pX(j), pX(i)) > x) {
      Float ycut = (Float)pY(j) + (Float)(pY(i) - pY(j)) /
	(Float)(pX(i) - pX(j)) * (Float)(x - pX(j));
      if (ycut > (Float)y) nabove++;
      else nbelow++;  }  }

  if ((nabove + nbelow) % 2) return True;
    // not even - possibly on a line of the polygon.

  return (nabove % 2);  }


} //# NAMESPACE CASA - END


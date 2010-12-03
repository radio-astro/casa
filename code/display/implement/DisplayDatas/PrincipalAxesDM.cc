//# PrincipalAxesDM.cc: Base class for drawing axis-bound datasets
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/IPosition.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/DisplayDatas/PrincipalAxesDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

PrincipalAxesDM::PrincipalAxesDM(const uInt xAxis,
				 const uInt yAxis,
				 const uInt mAxis,
				 PrincipalAxesDD *padd) :
  DisplayMethod(padd),
  itsXAxisNum(xAxis),
  itsYAxisNum(yAxis),
  itsZAxisNum(mAxis),
  notUsed(True) {
}

PrincipalAxesDM::~PrincipalAxesDM() {
  PrincipalAxesDM::cleanup();
}

// Draw this element of the dataset, using methods in the derived
// classes: dataShape() which returns the shape of the dataset, and
// dataGetSlice(...) which fills an Array with data from the 
// appropriate part of the dataset.  Once we're ready to go, we then
// need to call a specific drawing type routine.  The derived class 
// must provide a method like dataDrawSelf(...).
void PrincipalAxesDM::draw(Display::RefreshReason reason,
			   WorldCanvasHolder &wcHolder) {
  // retrieve a pointer to the WorldCanvas
  WorldCanvas *wCanvas = wcHolder.worldCanvas();

  if(dataRedrawSelf(wCanvas, reason)) {
      // Return if successful quick redraw (colormap fiddling).
      parentDisplayData()->setDisplayState( DisplayData::DISPLAYED );
      return;
  }
  
  // get the linear coordinates from WorldCanvas. We assume that the
  // sizecontrol has been done by the ImageDisplayData owning this
  // ImageDDImage and that the refreshEH of the ImageDisplayData has checked
  // that this is the case. So we do not check again here
  Double linMinX = wCanvas->linXMin();
  Double linMaxX = wCanvas->linXMax();
  Double linMinY = wCanvas->linYMin();
  Double linMaxY = wCanvas->linYMax();

  Int intMinX = Int(linMinX+0.5);
  Int intMaxX = Int(linMaxX+0.5);
  Int intMinY = Int(linMinY+0.5);
  Int intMaxY = Int(linMaxY+0.5);
  
  // get drawable sizes
  uInt canvasDrawXSize = wCanvas->canvasDrawXSize();
  uInt canvasDrawYSize = wCanvas->canvasDrawYSize();
  uInt canvasDrawXOffset = wCanvas->canvasDrawXOffset();
  uInt canvasDrawYOffset = wCanvas->canvasDrawYOffset();

  // added dgb 1998/07/24 for case where canvas is moved, but
  // not resized...  but actually check both because 
  // translateAllLists is not useable at the moment...
  uInt canvasXOffset = wCanvas->canvasXOffset();
  uInt canvasYOffset = wCanvas->canvasYOffset();
  uInt canvasXSize = wCanvas->canvasXSize();
  uInt canvasYSize = wCanvas->canvasYSize();

  AttributeBuffer worldCanvasState;
  
  // store the parameters of this images
  worldCanvasState.set("canvasDrawXSize", canvasDrawXSize);
  worldCanvasState.set("canvasDrawYSize", canvasDrawYSize);
  worldCanvasState.set("canvasDrawXOffset", canvasDrawXOffset);
  worldCanvasState.set("canvasDrawYOffset", canvasDrawYOffset);
  worldCanvasState.set("linXMin", intMinX);
  worldCanvasState.set("linXMax", intMaxX);
  worldCanvasState.set("linYMin", intMinY);
  worldCanvasState.set("linYMax", intMaxY);
  worldCanvasState.set("canvasXOffset", canvasXOffset);
  worldCanvasState.set("canvasYOffset", canvasYOffset);
  worldCanvasState.set("canvasXSize", canvasXSize);
  worldCanvasState.set("canvasYSize", canvasYSize);

  worldCanvasState.set("deviceBackgroundColor", 
		       wCanvas->pixelCanvas()->deviceBackgroundColor());
  worldCanvasState.set("deviceForegroundColor",
		       wCanvas->pixelCanvas()->deviceForegroundColor());

  if (!notUsed && &wcHolder==holder &&
      wCanvas->validList(drawListNumber) &&
      reason!=Display::ColorTableChange &&
      drawState.matches(worldCanvasState)) {

    // The state of the WorldCanvas is ok, we have drawn on this
    // canvas the same thing before, so redraw (ie this is a quick
    // refresh) If the Colortable has changed, we need to rebuild the
    // list...
    wCanvas->drawList(drawListNumber);
    return;
  } else {
    parentDisplayData()->setDisplayState( DisplayData::UNDISPLAYED );
  }

  // No reusable drawlist--rebuild it.

  // clear old, inapplicable drawlist state, if any.
  cleanup();

  /* (old DB comments and code, replaced by cleanup call above)

  // need to check wcHolder too!

  // [dk note: ...not if cleanup() is called in notifyUnreg(wch),
  // so that we know *holder still exists if !notUsed.  Failure to
  // delete drawlists when holder changed (during multipanel animation,
  // e.g.) was causing massive memory leakage.]

  if (!notUsed && holder &&
      (&wcHolder == holder) &&		// <<--cause of the leak.
      (holder->worldCanvas()->validList(drawListNumber))) {
    (holder->worldCanvas())->deleteList(drawListNumber);
  }

  // THIS SHOULD BE FIXED TO STORE
  // MULTIPLE notUseds AND holders AND drawStates
  // [and drawlist numbers--dk] FOR MULTIPLE
  // WORLDCANVASHOLDERS THAT THIS DM DRAWS ON...

  // [dk note: above is true: multicanvas animation still does not
  // reuse drawlists.  )-;  The ultimate solution of course is to
  // move the whole PADD/PADM branch to the Caching side.

  // Ironically, a quicker way would be for this (non-caching) PADM
  // to _own_ a list of (thinly-derived) CachingDisplayMethods--one
  // for each WCH.  (In essence, CDM consists of exactly the drawlist
  // state mentioned above).
  // The CDMs (rather than dataDrawSelf) would start the wc->newlist,
  // but their drawIntoList() could kick the main drawing chore back up
  // here to the code which follows.  It adds too much confusion to mess
  // with it now, though....  The spirit of Rube Goldberg is already
  // adequately represented in the existing code, imo.  :=) ]

  */

  // determine what slice to pull from the data: LINEAR COORD APPROACH
  Vector<Double> wBlc(2), wTrc(2); // worldcanvas corners
  Vector<Double> dBlc(2), dTrc(2); // data corners
  wBlc(0) = intMinX;
  wBlc(1) = intMinY;
  wTrc(0) = intMaxX;
  wTrc(1) = intMaxY;
  Vector<Double> conv(2), temp(2);
  conv(0) = 0.0;
  conv(1) = 0.0;
  Bool blcsuccess = 
    ((PrincipalAxesDD *)parentDisplayData())->linToWorld(temp, conv);
  if ( ! blcsuccess ) {
    ((PrincipalAxesDD *)parentDisplayData())->installFakeCoordinateSystem();
    ((PrincipalAxesDD *)parentDisplayData())->linToWorld(temp, conv);
  } 
  wCanvas->worldToLin(dBlc, temp);

  conv(0) = dataShape()(itsXAxisNum) - 1;
  conv(1) = dataShape()(itsYAxisNum) - 1;
  ((PrincipalAxesDD *)parentDisplayData())->linToWorld(temp, conv);
  wCanvas->worldToLin(dTrc, temp);

  Vector<Double> nBlc(2), nTrc(2);
  nBlc(0) = max(min(dBlc(0), dTrc(0)), min(wBlc(0), wTrc(0)));
  nBlc(1) = max(min(dBlc(1), dTrc(1)), min(wBlc(1), wTrc(1)));
  nTrc(0) = min(max(dBlc(0), dTrc(0)), max(wBlc(0), wTrc(0)));
  nTrc(1) = min(max(dBlc(1), dTrc(1)), max(wBlc(1), wTrc(1)));
  // (mixing ordinates should be ok on linear system!)

  Vector<Double> tBlc(2), tTrc(2); // the selected corners
  wCanvas->linToWorld(wBlc, nBlc);
  ((PrincipalAxesDD *)parentDisplayData())->worldToLin(tBlc, wBlc);

  wCanvas->linToWorld(wTrc, nTrc);
  ((PrincipalAxesDD *)parentDisplayData())->worldToLin(tTrc, wTrc);

  // A little border is added around the slice to be retrived, to take care
  // of possible round-off errors, etc.  It doesn't hurt to make the display
  // matrix slightly larger than necessary; the WC will crop it to its
  // zoom window.

  start(itsXAxisNum) = max((Int)(min(tBlc(0), tTrc(0))-1.), 0);
  start(itsYAxisNum) = max((Int)(min(tBlc(1), tTrc(1))-1.), 0);
  sliceShape(itsXAxisNum) =
	min((Int)(max(tBlc(0),tTrc(0))+2.), dataShape()(itsXAxisNum))
      - start(itsXAxisNum);
  sliceShape(itsYAxisNum) =
	min((Int)(max(tBlc(1),tTrc(1))+2.), dataShape()(itsYAxisNum))
      - start(itsYAxisNum);

  if (sliceShape(itsXAxisNum) < 1 || sliceShape(itsYAxisNum) < 1) {
    parentDisplayData()->setDisplayState( DisplayData::UNDISPLAYED );
    return;
  }

  Vector<Double> offsets(2);
  offsets(0) = (Double)start(itsXAxisNum) - .5;
  offsets(1) = (Double)start(itsYAxisNum) - .5;
  Vector<Double> blc;
  if (! (((PrincipalAxesDD *)parentDisplayData())->
	 linToWorld(blc, offsets)) ) {
    return;
  }

  offsets(0) = (Double)sliceShape(itsXAxisNum) +
	       (Double)start(itsXAxisNum) - .5;
  offsets(1) = (Double)sliceShape(itsYAxisNum) +
	       (Double)start(itsYAxisNum) - .5;
  Vector<Double> trc;
  if (! (((PrincipalAxesDD *)parentDisplayData())->
	   linToWorld(trc, offsets)) ) {
    return;
  }

  parentDisplayData()->setDisplayState( DisplayData::DISPLAYED );
  drawListNumber = dataDrawSelf(wCanvas, blc, trc, start, sliceShape,
				stride, True);

  if ( !blcsuccess ) {
    ((PrincipalAxesDD *)parentDisplayData())->removeFakeCoordinateSystem();
  }
  if (wCanvas->validList(drawListNumber)) {
    wCanvas->drawList(drawListNumber);
    drawState = worldCanvasState;
    holder = &wcHolder;
    notUsed = False;
  }

}


// do setup stuff that is common to all derived objects
void PrincipalAxesDM::setup(IPosition fixedPos) {

  setup2d();
  start = fixedPos;
  start(itsXAxisNum) = 0;
  start(itsYAxisNum) = 0;

  Vector<Double> pixPos(fixedPos.nelements());
  for (uInt i = 0; i < fixedPos.nelements(); i++) {
    pixPos(i) = fixedPos(i);
  }
  Vector<Double> worldPos;
  ((PrincipalAxesDD *)parentDisplayData())->
    coordinateSystem().toWorld(worldPos, pixPos);
  if ((((PrincipalAxesDD *)parentDisplayData())->
       dataShape())(itsZAxisNum) > 1) {
    restrictions.add("zIndex", fixedPos(itsZAxisNum), 0, False, True);
    Double tol = ((PrincipalAxesDD *)parentDisplayData())->
      coordinateSystem().increment()(itsZAxisNum) / 2.0;
    restrictions.add("zValue", worldPos(itsZAxisNum), tol, False, True);
  }
}

// do setup stuff that is common to all derived objects
void PrincipalAxesDM::setup2d() {
  stride.resize(((PrincipalAxesDD *)parentDisplayData())->dataDim());
  stride = 1;

  sliceShape.resize(((PrincipalAxesDD *)parentDisplayData())->dataDim());
  sliceShape = 1;
  sliceShape(itsXAxisNum) = ((PrincipalAxesDD *)parentDisplayData())->
    dataShape()(itsXAxisNum);
  sliceShape(itsYAxisNum) = ((PrincipalAxesDD *)parentDisplayData())->
    dataShape()(itsYAxisNum);

  start.resize(((PrincipalAxesDD *)parentDisplayData())->dataDim());
  start = 0;

  // no restrictions in 2d case, always display the element.
}

// clear drawlist state.
void PrincipalAxesDM::cleanup() {
  parentDisplayData()->setDisplayState( DisplayData::UNDISPLAYED );
  drawState.clear();
  if (!notUsed) {
    notUsed = True;
    if (holder->worldCanvas()->validList(drawListNumber)) {
        holder->worldCanvas()->deleteList(drawListNumber);
    }
  }
}

// (Required) default constructor.
PrincipalAxesDM::PrincipalAxesDM() {
}

// (Required) copy constructor.
PrincipalAxesDM::PrincipalAxesDM(const PrincipalAxesDM &) {
}

// (Required) copy assignment.
void PrincipalAxesDM::operator=(const PrincipalAxesDM &) {
}

} //# NAMESPACE CASA - END


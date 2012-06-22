//# MWCRTZoomer.cc: MultiWorldCanvas event-based zoomer
//# Copyright (C) 1999,2000,2001,2002,2003
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
#include <casa/BasicSL/String.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayEvents/MWCRTZoomer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

MWCRTZoomer::MWCRTZoomer(Display::KeySym keysym) :
  MWCRectTool(keysym) {
}

MWCRTZoomer::~MWCRTZoomer() {
}

void MWCRTZoomer::doubleInside() {
  Vector<Double> linBlc, linTrc;
  getLinearCoords(linBlc, linTrc);
  
  // for "relaxed" zooming, as needed in HistogramDD, don't bother
  // checking that the zoom is > 1.0, and execute the contents of
  // this scope always.
  if ((fabs(linBlc(0) - linTrc(0)) > (Double)0.0) &&
      (fabs(linBlc(1) - linTrc(1)) > (Double)0.0)) {
    itsWCListIter->toStart();
    while (!itsWCListIter->atEnd()) {
      WorldCanvas *wc = itsWCListIter->getRight();
      wc->setZoomRectangleLCS(linBlc, linTrc);
      wc->refresh(Display::LinearCoordinateChange);
      (*itsWCListIter)++;
    }
    zoomed(linBlc, linTrc);
  }
}

void MWCRTZoomer::unzoom() {
  reset();
  itsWCListIter->toStart();
  if(itsWCListIter->atEnd()) return;
  itsCurrentWC = itsWCListIter->getRight();
	// (so that zoomed() callback always has a valid 'current WC'...)

  String attString = "resetCoordinates";
  Attribute resetAtt(attString, True);
  while (!itsWCListIter->atEnd()) {
    WorldCanvas *wc = itsWCListIter->getRight();    
    wc->setAttribute(resetAtt);
    wc->refresh(Display::LinearCoordinateChange);  
    (*itsWCListIter)++;
  }
  Vector<Double> blc(2), trc(2);
  WorldCanvas *cwc = itsCurrentWC;
  blc(0) = cwc->linXMin(); blc(1) = cwc->linYMin();
  trc(0) = cwc->linXMax(); trc(1) = cwc->linYMax();
  zoomed(blc, trc);
}

void MWCRTZoomer::zoomIn(Double factor) { if(factor>0.) zoomOut(1./factor);  }


void MWCRTZoomer::zoomOut(Double factor) {
  // Zoom out by given factor.
  
  if(factor<=0. || factor==1.) return;
  
  reset();
  itsWCListIter->toStart();
  if(itsWCListIter->atEnd()) return;
  WorldCanvas *wc = itsWCListIter->getRight();
  
  Vector<Double> blc(2), trc(2), ctr(2), radius(2);
  blc(0) = wc->linXMin(); blc(1) = wc->linYMin();
  trc(0) = wc->linXMax(); trc(1) = wc->linYMax();
  
  ctr    = (trc+blc)/2.;
  radius = abs(trc-blc)/2.;
  blc    = ctr - (factor*radius);
  trc    = ctr + (factor*radius);
  
  zoom(blc, trc);
}


void MWCRTZoomer::zoom(const Vector<Double>& blc,
		       const Vector<Double>& trc) {
  reset();
  itsWCListIter->toStart();
  if(itsWCListIter->atEnd()) return;
  itsCurrentWC = itsWCListIter->getRight();
	// (so that zoomed() callback always has a valid current WC...)

  while (!itsWCListIter->atEnd()) {
    WorldCanvas *wc = itsWCListIter->getRight();
    wc->setZoomRectangleLCS(blc, trc);
    wc->refresh(Display::LinearCoordinateChange);
    (*itsWCListIter)++;
  }
  zoomed(blc, trc);
}

void MWCRTZoomer::doubleOutside() {
  Vector<Double> linBlc, linTrc;
  getLinearCoords(linBlc, linTrc);
  Vector<Double> wblc(2), wtrc(2);
  WorldCanvas *wCnvs = itsCurrentWC;
  wblc(0) = wCnvs->linXMin();
  wblc(1) = wCnvs->linYMin();
  wtrc(0) = wCnvs->linXMax();
  wtrc(1) = wCnvs->linYMax();
  Vector<Double> nblc(2), ntrc(2);
  nblc = (wblc + wtrc) / 2.0 +
    (wtrc - wblc) / (linTrc - linBlc) *
    (wblc - (linTrc + linBlc) / 2.0);
  ntrc = (wblc + wtrc) / 2.0 +
    (wtrc - wblc) / (linTrc - linBlc) *
    (wtrc - (linTrc + linBlc) / 2.0);
  itsWCListIter->toStart();
  while (!itsWCListIter->atEnd()) {
    WorldCanvas *wc = itsWCListIter->getRight();
    wc->setZoomRectangleLCS(nblc, ntrc);
    wc->refresh(Display::LinearCoordinateChange);
    (*itsWCListIter)++;
  }
  zoomed(nblc, ntrc);
}

void MWCRTZoomer::zoomed(const Vector<Double> &,
			 const Vector<Double> &) {  }

void MWCRTZoomer::getLinearCoords(Vector<Double> &blc, Vector<Double> &trc) {
  // get the pixel coordinates of the zoom box
  Int x1, y1, x2, y2;
  get(x1, y1, x2, y2);
  // sort them into blc, trc
  Vector<Double> pixblc(2);
  Vector<Double> pixtrc(2);
  pixblc(0) = min(x1, x2);
  pixtrc(0) = max(x1, x2);
  pixblc(1) = min(y1, y2);
  pixtrc(1) = max(y1, y2);
  // convert pixel to linear coordinates
  blc.resize(2);
  trc.resize(2);
  itsCurrentWC->pixToLin(blc, pixblc);
  itsCurrentWC->pixToLin(trc, pixtrc);
}


} //# NAMESPACE CASA - END


//# WedgeDM.cc: drawing for Color wedges
//# Copyright (C) 2001
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

#include <casa/Arrays/MatrixMath.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/WedgeDM.h>
#include <display/DisplayDatas/WedgeDD.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

WedgeDM::WedgeDM(WorldCanvas *worldCanvas, 
			     AttributeBuffer *wchAttributes,
			     AttributeBuffer *ddAttributes,
			     CachingDisplayData *dd) :
  CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
}

WedgeDM::~WedgeDM() {
  cleanup();
}

Bool WedgeDM::drawIntoList(Display::RefreshReason reason,
				 WorldCanvasHolder &wcHolder) {
  // Locate the WorldCanvas to draw upon
  WorldCanvas *wc = wcHolder.worldCanvas();

  WedgeDD *parent = dynamic_cast<WedgeDD *>
    (parentDisplayData());
  if (!parent) {
    throw(AipsError("invalid parent of WedgeDM"));
  }
  if ( (wc->pixelCanvas()->pcctbl()->colorModel() == Display::RGB) ||
       (wc->pixelCanvas()->pcctbl()->colorModel() == Display::HSV)) {
    throw(AipsError("Wedges for RGB and HSV not yet supported"));
  }
  if (parent->itsMin == parent->itsMax) return False;
  Attribute dmin("dataMin", static_cast<Double>(0.0));
  Attribute dmax("dataMax", static_cast<Double>(1.0));
  wc->setAttribute(dmin);
  wc->setAttribute(dmax);
  try {
    wc->setDataScaleHandler(parent->itsPowerScaleHandler);    
  } catch (const AipsError &x) {
    throw(AipsError(x));
  }  
  Vector<Double> blc(2,0.0);
  Vector<Double> trc(2,1.0);
  if (parent->itsOptionsMode == "horizontal") {
    Matrix<Float> tmpMat;
    tmpMat = transpose(parent->itsColorbar);
    blc(0) = static_cast<Double>(parent->itsMin);
    trc(0) = static_cast<Double>(parent->itsMax);
    wc->drawImage(blc,trc, tmpMat);
  } else {
    blc(1) = static_cast<Double>(parent->itsMin);
    trc(1) = static_cast<Double>(parent->itsMax);
    wc->drawImage(blc,trc, parent->itsColorbar);
  }
  return True;
}

void WedgeDM::cleanup() {
}

WedgeDM::WedgeDM() {
}

WedgeDM::WedgeDM(const WedgeDM &other) :
  CachingDisplayMethod(other) {
}

void WedgeDM::operator=(const WedgeDM &) {
}

} //# NAMESPACE CASA - END


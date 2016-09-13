//# WCRTRegion.cc: WorldCanvas event-based rectangle region drawer
//# Copyright (C) 1999,2000
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

// aips includes:
#include <casa/aips.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>

// trial includes:

// display library includes:
#include <display/Display/WorldCanvas.h>

// this include:
#include <display/DisplayEvents/WCRTRegion.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	WCRTRegion::WCRTRegion(WorldCanvas *wcanvas,
	                       Display::KeySym keysym) :
		WCRectTool(wcanvas, keysym, True) {
	}

	WCRTRegion::~WCRTRegion() {
	}

	void WCRTRegion::doubleInside() {
		Vector<Double> linBlc, linTrc;
		getLinearCoords(linBlc, linTrc);
		if ((fabs(linBlc(0) - linTrc(0)) > (Double)1.0) &&
		        (fabs(linBlc(1) - linTrc(1)) > (Double)1.0)) {
			//regionReady(linBlc, linTrc);
			regionReady();
		}
	}

	void WCRTRegion::getLinearCoords(Vector<Double> &blc, Vector<Double> &trc) {
		// get the pixel coordinates of the rectangular region
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
		worldCanvas()->pixToLin(blc, pixblc);
		worldCanvas()->pixToLin(trc, pixtrc);
	}


} //# NAMESPACE CASA - END


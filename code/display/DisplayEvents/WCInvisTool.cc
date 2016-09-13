//# WCInvisTool.cc: Base class for WC event-based invisible position tools
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

#include <casa/aips.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayEvents/WCInvisTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	WCInvisTool::WCInvisTool(WorldCanvas *wcanvas,
	                         Display::KeySym keysym) :
		WCTool(wcanvas, keysym),
		itsActive(False),
		itsMoved(False) {
	}

	WCInvisTool::~WCInvisTool() {
	}

	void WCInvisTool::keyPressed(const WCPositionEvent &ev) {
		itsX = ev.linX();
		itsY = ev.linY();
		itsActive = True;
		itsMoved = False;
		positionReady();
	}

	void WCInvisTool::keyReleased(const WCPositionEvent &) {
		itsActive = False;
	}

	void WCInvisTool::moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/) {
		if (!itsActive) {
			return;
		} else {
			itsMoved = True;
			itsX = ev.linX();
			itsY = ev.linY();
			positionReady();
		}
	}

	void WCInvisTool::get(Double &x, Double &y) const {
		x = itsX;
		y = itsY;
	}

	void WCInvisTool::getFractional(Double &x, Double &y) const {
		Double lx, ly;
		get(lx, ly);
		WorldCanvas *wc = worldCanvas();
		x = (lx - wc->linXMin()) / (wc->linXMax() - wc->linXMin());
		y = (ly - wc->linYMin()) / (wc->linYMax() - wc->linYMin());
	}

} //# NAMESPACE CASA - END


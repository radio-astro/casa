//# MWCEvents.h: various small MWCTool Event classes.
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2002
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

#ifndef TRIALDISPLAY_MWCEVENTS_H
#define TRIALDISPLAY_MWCEVENTS_H

#include <casa/aips.h>
#include <display/DisplayEvents/DisplayEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Event sent to tell MWCCrosshairTool to reset.
// </summary>

// <synopsis>
// ResetCrosshairEvent is created and sent to MWCCrosshairTool
// (via WorldCanvas::handleEvent()) to tell the crosshair to reset itself.
// </synopsis>

// <motivation>
// This allows display library objects (e.g. MSAsRaster) to remove the
// crosshair from the screen when appropriate (e.g. when it has received
// a selection event from it).
// </motivation>

	class ResetCrosshairEvent : public DisplayEvent {

	public:
		// caller sets skipRefresh=True iff it is handling refresh itself.
		ResetCrosshairEvent(Bool skipRefresh=False):
			DisplayEvent(), skipRefresh_(skipRefresh) {  }
		virtual const Bool skipRefresh() const {
			return skipRefresh_;
		}

	private:
		Bool skipRefresh_;
	};


// <summary>
// Event sent to tell MWCRTRegionTool to reset.
// </summary>

// <synopsis>
// ResetRTRegionEvent is created and sent to MWCRTRegionTool
// (via WorldCanvas::handleEvent()) to tell the RTRegion to reset itself.
// </synopsis>

// <motivation>
// This allows display library objects (e.g. MSAsRaster) to remove the
// rectangle from the screen when appropriate (e.g. when it has received
// a selection event from it).
// </motivation>

	class ResetRTRegionEvent : public DisplayEvent {

	public:
		// caller sets skipRefresh=True iff it is handling refresh itself.
		ResetRTRegionEvent(Bool skipRefresh=False):
			DisplayEvent(), skipRefresh_(skipRefresh) {  }
		virtual const Bool skipRefresh() const {
			return skipRefresh_;
		}

	private:
		Bool skipRefresh_;
	};


} //# NAMESPACE CASA - END

#endif



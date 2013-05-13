//# CrosshairEvent.h: class for MWCCrosshairTool point selection event.
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2002,2003
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

#ifndef TRIALDISPLAY_CROSSHAIREVENT_H
#define TRIALDISPLAY_CROSSHAIREVENT_H

#include <casa/aips.h>
#include <display/DisplayEvents/WorldCanvasEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;

// <summary>
// WorldCanvasEvent:contains info on the WC point selected by MWCCrosshairTool
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "CrosshairEvent" holds information about a point selected by the user
// with the MWCCrosshairTool.
// </etymology>

// <synopsis>
// CrosshairEvent is created by
// <linkto class="MWCCrosshairTool">MWCCrosshairTool</linkto>
// when a point is selected by the crosshair on a
// <linkto class="WorldCanvas">WorldCanvas</linkto> draw area.
// The event is passed (via WorldCanvas::handleEvent()) to the generic
// <linkto class="DisplayEH">DisplayEH</linkto>s
// registered with the WorldCanvas.
// </synopsis>

// <motivation>
// A mechanism was needed to notify library objects associated with a
// WorldCanvas (in particular,
// <linkto class="DisplayData">DisplayData</linkto>s)
// when a position was selected on the WC via MWCCrosshairTool.
// Formerly, this information was sent only to glish.
// This event is on a different level from the mouse/keyboard
// WorldCanvasEvents and serves a different purpose, so a new event type
// was created.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

	class CrosshairEvent : public WorldCanvasEvent {

	public:

		// Constructor taking a pointer to the WorldCanvas where the
		// event occured, and the pixel coordinates of the event.
		// evtype should be "down", "move", or "up", according to mouse state.
		CrosshairEvent(WorldCanvas *wc, const Int pixX, const Int pixY,
		               const String& evtype) :
			WorldCanvasEvent(wc),
			itsPixX(pixX), itsPixY(pixY),
			itsEvType(evtype)
		{  }

		// The x and y pixel position of the crosshair when the event occurred.
		// <group>
		virtual Int pixX() const {
			return itsPixX;
		}
		virtual Int pixY() const {
			return itsPixY;
		}
		virtual String evtype() const {
			return itsEvType;
		}
		// </group>

	private:

		// pixel position of the event
		Int itsPixX, itsPixY;
		// "down", "move", or "up", according to mouse state.
		String itsEvType;

	};


} //# NAMESPACE CASA - END

#endif



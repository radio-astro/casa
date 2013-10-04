//# RectRegionEvent.h: class for rectangle region selection event
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

#ifndef TRIALDISPLAY_RECTREGIONEVENT_H
#define TRIALDISPLAY_RECTREGIONEVENT_H

#include <casa/aips.h>
#include <display/DisplayEvents/WorldCanvasEvent.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicMath/Math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;

// <summary>
// Contains info on the WC rectanglar area selected by MWCRTRegion mouse tool
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <etymology>
// "RectRegionEvent" holds information about a rectangular 'region' selected
// by the user with the MWCRTRegion mouse tool.
// NB: glish uses the term 'pseudoregion' instead to refer areas selected
// by MWCRTRegion et. al.  In the context of Image Analysis, 'Region'
// refers to a more complex object, which may be assembled from
// multiple pseudoregions.
// </etymology>

// <synopsis>
// RectRegionEvent is created by the
// <linkto class="MWCRTRegion">MWCRTRegion</linkto> display library
// mouse tool, when a rectangular area is selected by it on a
// <linkto class="WorldCanvas">WorldCanvas</linkto> where the
// tool is active.  The event is passed (via WorldCanvas::sendEvent())
// to the generic <linkto class="DisplayEH">DisplayEH</linkto>s
// registered with that WorldCanvas.
// </synopsis>

// <motivation>
// A mechanism was needed to notify library objects associated with a
// WorldCanvas (in particular,
// <linkto class="DisplayData">DisplayData</linkto>s) when a rectangular
// region was selected on the WC via MWCRTRegion.  Formerly, this information
// was sent only to glish.  This event is on a different level from the
// mouse/keyboard WorldCanvasEvents and serves a different purpose,
// so a new event type was created.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/10/15">
// None.
// </todo>

	class RectRegionEvent : public WorldCanvasEvent {

	public:

		// Constructor taking a pointer to the WorldCanvas where the
		// event occured, and the pixel coordinates of the rectangle.
		RectRegionEvent(WorldCanvas *wc,
		                const Int pixX1, const Int pixY1,
		                const Int pixX2, const Int pixY2) :

			WorldCanvasEvent(wc),
			itsPixBlc(2), itsPixTrc(2) {

			itsPixBlc(0) = min(pixX1, pixX2);
			itsPixTrc(0) = max(pixX1, pixX2);	// sort into top-right
			itsPixBlc(1) = min(pixY1, pixY2);	// and and bottom-left corners.
			itsPixTrc(1) = max(pixY1, pixY2);
		}

		// The corners of the selected rectangle, in screen pixel coordinates.
		// <group>
		virtual const Vector<Int> pixBlc() const {
			return itsPixBlc;
		}
		virtual const Vector<Int> pixTrc() const {
			return itsPixTrc;
		}
		// </group>

	private:

		// pixel position of the rectangle
		Vector<Int> itsPixBlc, itsPixTrc;

	};


} //# NAMESPACE CASA - END

#endif



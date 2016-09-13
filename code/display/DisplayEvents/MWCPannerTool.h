//# MWCPannerTool.h: MultiWorldCanvas panning tool
//# Copyright (C) 2000,2001,2002
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

#ifndef TRIALDISPLAY_MWCPANNERTOOL_H
#define TRIALDISPLAY_MWCPANNERTOOL_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <display/DisplayEvents/MultiWCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvas;

// <summary>
//
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisites>
//   <li> MWCTool
// </prerequisites>
//
// <etymology>
// MWCPannerTool stands for MultiWorldCanvas Panner Tool
// </etymology>
//
// <synopsis>
// This class provides panning functionality.  The zoomed() function should
// be overriden to catch the event, if needed.
//
// The pan line is drawn by dragging the mouse from one point to
// another. The release of the mouse button triggers the movement of the
// image by the same displacement.
//
// The Tool also responds to the arrow keys, scroll wheel, et. al.,
// moving the zoom window in the same direction through the image.
// A constructor parameter is provided to turn these features off,
// if needed.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Panning is a common function in most visualization applications
// </motivation>
//
// <todo asof="2001/02/24">
// Nothing known
// </todo>

	class MWCPannerTool : public MultiWCTool, public DTVisible {

	public:
		// Constructor
		MWCPannerTool(Display::KeySym keysym = Display::K_Pointer_Button1,
		              Bool scrollingAllowed = True);

		// Destructor
		virtual ~MWCPannerTool();

		// Reset to non-showing, non-active.  Refreshes if necessary to erase
		// (unless skipRefresh==True).
		// (Does not unregister from WCs or disable future event handling).
		virtual void reset(Bool skipRefresh=False);

	protected:

		// Functions called by the base class mouse event handling operators -
		// these maintain the state of the pan vector and order it to be drawn
		// via refresh(), send the pan request to the WC, and pass on notice
		// of the pan (zoomed()) when it is complete.
		// functions
		// <group>
		virtual void keyPressed(const WCPositionEvent &/*ev*/);
		virtual void keyReleased(const WCPositionEvent &/*ev*/);
		virtual void moved(const WCMotionEvent &/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);
		virtual void otherKeyPressed(const WCPositionEvent &/*ev*/);
		// </group>

		// draw the pan vector on a PixelCanvas.  (To be called only by the
		// base class refresh event handler).
		virtual void draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);

		// Output callback to indicate that we have zoomed.  Override to
		// handle, if needed.
		virtual void zoomed(const Vector<Double> &/*linBlc*/,
		                    const Vector<Double> &/*linTrc*/) {  };

	private:

		// execute the pan.  shift is the (2-element) shift vector for the
		// zoom window, in linear coordinates.  Resets the tool, removing
		// the line (if any) from screen
		virtual void pan(Vector<Double> &shift);

		// Should we respond to mouse movement and button release?  Should
		// we draw?  Set when the button is pushed in one of the tool's WCs.
		Bool itsActive;

		// pixel coordinates of the pan vector.  1 = anchor, 2 = new position.
		Int itsX1, itsY1, itsX2, itsY2;

		// allow scrolling via arrow keys, et. al.?
		Bool itsScrollingAllowed;

		// prevents key repeat from piling up scroll events,
		// if refresh can't keep up with them
		// <group>
		Double itsLastScrollTime;
		Int itsLastKey;
		// </group>

	};


} //# NAMESPACE CASA - END

#endif



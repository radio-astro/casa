//# MWCPolyTool.h: Base class for MultiWorldCanvas event-based polygon tools
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

#ifndef TRIALDISPLAY_MWCPOLYTOOL_H
#define TRIALDISPLAY_MWCPOLYTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/MultiWCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas event-based polygon tools
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisites>
//   <li><linkto>WCTool</linkto>
// </prerequisites>
//
// <etymology>
// MWCPolyTool stands for Multi-WorldCanvas Polygon Tool
// </etymology>
//
// <synopsis>
// This class adds to its base MWCTool to provide a tool for drawing,
// reshaping and moving polygons on a WorldCanvas.  While MWCPolyTool
// is not abstract, it performs no useful function.  The programmer
// should derive from this class and override the functions doubleInside
// and doubleOutside at the very least.  These are called when the user
// double-clicks a particular key or mouse button inside or outside an
// existing polygon respectively.  It is up to the programmer to decide
// what these events mean, but it is recommended that an internal double-
// click correspond to the main action of the tool, eg. emitting the
// polygon vertices to the application, and that an external double-click
// correspond to a secondary action of the tool, if indeed there are
// additional actions suitable to the tool.
//
// The polygon is drawn by clicking at each of the vertices, and
// clicking again on the last or first vertex to complete the polygon.
// Once drawn, the vertices can be moved by dragging their handles,
// and the entire polygon relocated by dragging inside the polygon.
// The polygon is removed from the display when the Esc key is
// pressed.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Many activities on the WorldCanvas will be based on the user drawing
// a polygon and using the polygon in some operation.
// </motivation>
//
// <todo asof="1999/02/24">
//   <li> Add time constraint to double click detection
// </todo>

	class MWCPolyTool : public MultiWCTool, public DTVisible {

	public:

		// Constructor
		MWCPolyTool(Display::KeySym keysym = Display::K_Pointer_Button1,
		            const Bool persistent = False);

		// Destructor
		virtual ~MWCPolyTool();

		// Switch the tool off - this calls the base class disable,
		// and then erases the polygon if it's around
		virtual void disable();

		// reset to non-existent, non-active polygon.
		// Refreshes if necessary to erase (unless skipRefresh==True).
		// (Does not unregister from WCs or disable future event handling).
		virtual void reset(Bool skipRefresh=False);

		// Is a polygon currently defined?
		virtual Bool polygonDefined() {
			return itsMode>=Ready;
		}

	protected:

		// Functions called by the base class event handling operators--and
		// normally only those.  This is the input that controls the polygon's
		// appearance and action.  When the polygon is ready and double-click
		// is received, the doubleInside/Outside routine is invoked.
		// <group>
		virtual void keyPressed(const WCPositionEvent &/*ev*/);
		virtual void moved(const WCMotionEvent &/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);
		virtual void keyReleased(const WCPositionEvent &/*ev*/);
		virtual void otherKeyPressed(const WCPositionEvent &/*ev*/);
		// </group>

		// draw the polygon (if any) on the object's currently active WC.
		// Only to be called by the base class refresh event handler.  Derived
		// objects should use refresh() if they need to redraw, but even that
		// is normally handled automatically by this class.
		virtual void draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);

		// Output callback functions--to be overridden in derived class as needed.
		// Called when there is a double click inside/outside the polygon
		// <group>
		virtual void doubleInside() { };
		virtual void doubleOutside() { };
		// </group>

		// Function called when a polygon is ready and not being
		// edited.  (Unused so far on the glish level (12/01)).
		virtual void polygonReady() { };

		// Retrieve polygon vertices, or a single vertex, in screen pixels.
		// Valid results during the callback functions; to be used by them,
		// as well as internally.
		// <group>
		virtual void get(Vector<Int> &x, Vector<Int> &y) const;
		virtual void get(Int &x, Int &y, const Int pt) const;
		// </group>

	private:

		// Set the polygon vertices. itsNPoints should already be set, and
		// x and y must contain (at least) this many points.
		virtual void set(const Vector<Int> &x, const Vector<Int> &y);

		// replace a single vertex.
		virtual void set(const Int x, const Int y, const Int pt);
		// push/pop last vertex
		// <group>
		void pushPoint(Int x1, Int y1);
		void popPoint();
		// </group>

		// are we inside the polygon?
		Bool inPolygon(const Int &x, const Int &y) const;

		// are we within the specified handle?
		Bool inHandle(const Int &pt, const Int &x, const Int &y) const;


		// should the polygon remain on screen after double clicks?
		Bool itsPolygonPersistent;

		// state of the polygon tool
		enum AdjustMode {
		    Off,	// Nothing exists yet
		    Def,	// defining initial polygon
		    Ready,	// polygon finished, no current activity
		    Move,	// moving entire polygon
		    Resize
		};	// moving single vertex whose handle was pressed
		MWCPolyTool::AdjustMode itsMode;

		// set True on double-click, if the polygon is persistent.
		// set False when the polygon is moved, resized or reset.
		// If True, a click outside the polygon will erase it and begin
		// definition of a new one.
		Bool itsEmitted;

		// Number of points
		Int itsNPoints;

		// Polygon points (linear).  Not to be used directly.
		// use get, set, push, pop instead, which take pixel coordinate arguments.
		// It's done this way so that zooms work on the figures.
		Vector<Double> itsX, itsY;

		// size in pixels of the handles
		Int itsHandleSize;

		// vertex being moved
		Int itsSelectedHandle;

		// position that move started from
		Int itsBaseMoveX, itsBaseMoveY;

		// times of the last two presses
		Double itsLastPressTime, its2ndLastPressTime;

	};


} //# NAMESPACE CASA - END

#endif



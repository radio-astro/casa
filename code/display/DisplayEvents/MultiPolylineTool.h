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

#ifndef DISPLAY_MULTIPOLYLINETOOL_H
#define DISPLAY_MULTIPOLYLINETOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/RegionTool.h>
#include <display/DisplayEvents/DTVisible.h>
#include <display/region/RegionSourceFactory.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas event-based polyline tools
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
// MultiPolylineTool stands for Multi-WorldCanvas Polyline Tool
// </etymology>
//
// <synopsis>
// This class adds to its base MWCTool to provide a tool for drawing,
// reshaping and moving polylines on a WorldCanvas.  While MultiPolylineTool
// is not abstract, it performs no useful function.  The programmer
// should derive from this class and override the functions doubleInside
// and doubleOutside at the very least.  These are called when the user
// double-clicks a particular key or mouse button inside or outside an
// existing polyline respectively.  It is up to the programmer to decide
// what these events mean, but it is recommended that an internal double-
// click correspond to the main action of the tool, eg. emitting the
// polyline vertices to the application, and that an external double-click
// correspond to a secondary action of the tool, if indeed there are
// additional actions suitable to the tool.
//
// The polyline is drawn by clicking at each of the vertices, and
// clicking again on the last or first vertex to complete the polyline.
// Once drawn, the vertices can be moved by dragging their handles,
// and the entire polyline relocated by dragging inside the polyline.
// The polyline is removed from the display when the Esc key is
// pressed.
// </synopsis>
//
// <example>
// </example>
//



	class MultiPolylineTool : public RegionTool, public DTVisible, public viewer::RegionCreator {

	public:

		// Constructor
		MultiPolylineTool( viewer::RegionSourceFactory *rsf, PanelDisplay* pd,
		                   Display::KeySym keysym = Display::K_Pointer_Button1, const Bool persistent = False );

		// Destructor
		virtual ~MultiPolylineTool();

		// Switch the tool off - this calls the base class disable,
		// and then erases the polyline if it's around
		virtual void disable();

		// reset to non-existent, non-active polyline.
		// Refreshes if necessary to erase (unless skipRefresh==True).
		// (Does not unregister from WCs or disable future event handling).
		virtual void reset(Bool skipRefresh=False);

		// Is a polyline currently defined?
		virtual Bool polylineDefined() {
			return itsMode>=Ready;
		}

		viewer::RegionSource *getRegionSource( ) {
			return rfactory;
		}

		void checkPoint( WorldCanvas *wc, State &state );

		// called when the user (read GUI user) indicates that a region should be deleted...
		void revokeRegion( viewer::Region * );

		// returns a set which indicates regions this creator creates...
		const std::set<viewer::region::RegionTypes> &regionsCreated( ) const;

		bool create( viewer::region::RegionTypes /*region_type*/, WorldCanvas */*wc*/, const std::vector<std::pair<double,double> > &/*pts*/,
		             const std::string &/*label*/, viewer::region::TextPosition /*label_pos*/, const std::vector<int> &/*label_off*/,
		             const std::string &/*font*/, int /*font_size*/, int /*font_style*/, const std::string &/*font_color*/,
		             const std::string &/*line_color*/, viewer::region::LineStyle /*line_style*/, unsigned int /*line_width*/,
		             bool /*annotation*/, VOID */*region_specific_state*/ );

		RegionToolTypes type( ) const {
			return POLYLINETOOL;
		}

	protected:

		// Functions called by the base class event handling operators--and
		// normally only those.  This is the input that controls the polyline's
		// appearance and action.  When the polyline is ready and double-click
		// is received, the doubleInside/Outside routine is invoked.
		// <group>
		virtual void keyPressed(const WCPositionEvent &/*ev*/);
		virtual void moved(const WCMotionEvent &/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);
		virtual void keyReleased(const WCPositionEvent &/*ev*/);
		virtual void otherKeyPressed(const WCPositionEvent &/*ev*/);
		// </group>

		// draw the polyline (if any) on the object's currently active WC.
		// Only to be called by the base class refresh event handler.  Derived
		// objects should use refresh() if they need to redraw, but even that
		// is normally handled automatically by this class.
		virtual void draw(const WCRefreshEvent&/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);

		// Output callback functions--to be overridden in derived class as needed.
		// Called when there is a double click inside/outside the polyline
		// <group>
		virtual void doubleInside() { };
		virtual void doubleOutside() { };
		// </group>

		// Function called when a polyline is ready and not being
		// edited.  (Unused so far on the glish level (12/01)).
		virtual void polylineReady() { };

		// Retrieve polyline vertices, or a single vertex, in screen pixels.
		// Valid results during the callback functions; to be used by them,
		// as well as internally.
		// <group>
		virtual void get(Vector<Int> &x, Vector<Int> &y) const;
		virtual void get(Int &x, Int &y, const Int pt) const;
		// </group>

		virtual bool checkType( viewer::region::RegionTypes t ) {
			return t == viewer::region::PolylineRegion;
		}

	private:
		typedef std::list<SHARED_PTR<viewer::Polyline> > polylinelist;
		void start_new_polyline( WorldCanvas *, int x, int y );

		// Set the polyline vertices. itsNPoints should already be set, and
		// x and y must contain (at least) this many points.
		virtual void set(const Vector<Int> &x, const Vector<Int> &y);

		// replace a single vertex.
		virtual void set(const Int x, const Int y, const Int pt);

		SHARED_PTR<viewer::Polyline> resizing_region;
		SHARED_PTR<viewer::Polyline> creating_region;

		// push/pop last vertex
		// <group>
		void pushPoint(Int x1, Int y1);
		void popPoint();
		// </group>

		// are we inside the polyline?
		Bool inPolyline(const Int &x, const Int &y) const;

		// are we within the specified handle?
		Bool inHandle(const Int &pt, const Int &x, const Int &y) const;


		// should the polyline remain on screen after double clicks?
		Bool itsPolylinePersistent;

		// state of the polyline tool
		enum AdjustMode {
		    Off,	// Nothing exists yet
		    Def,	// defining initial polyline
		    Ready,	// polyline finished, no current activity
		    Move,	// moving entire polyline
		    Resize
		};	// moving single vertex whose handle was pressed
		MultiPolylineTool::AdjustMode itsMode;

		// set True on double-click, if the polyline is persistent.
		// set False when the polyline is moved, resized or reset.
		// If True, a click outside the polyline will erase it and begin
		// definition of a new one.
		Bool itsEmitted;

		// Number of points
		Int itsNPoints;

		// Polyline points (linear).  Not to be used directly.
		// use get, set, push, pop instead, which take pixel coordinate arguments.
		// It's done this way so that zooms work on the figures.
		Vector<Double> itsX, itsY;
		// pixel coordinates of the pan vector.  1 = anchor, 2 = new position.
		Int itsX1, itsY1, itsX2, itsY2, itsX3, itsY3;


		// size in pixels of the handles
		Int itsHandleSize;

		// vertex being moved
		Int itsSelectedHandle;

		// position that move started from
		Int itsBaseMoveX, itsBaseMoveY;

		// may not be needed...
		int resizing_region_handle;

		polylinelist moving_regions;
		double moving_linx_;
		double moving_liny_;

		SHARED_PTR<viewer::Polyline> building_polyline;
		viewer::RegionSource *rfactory;
		polylinelist polylines;
		PanelDisplay *pd_;
	};

} //# NAMESPACE CASA - END

#endif



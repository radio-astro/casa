//# WCRectTool.h: Base class for WorldCanvas event-based rectangle tools
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

#ifndef TRIALDISPLAY_WCRECTTOOL_H
#define TRIALDISPLAY_WCRECTTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCTool.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for WorldCanvas event-based rectangle tools
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisites>
//   <li> WCTool
// </prerequisites>
//
// <etymology>
// WCRectTool stands for WorldCanvas Rectangle Tool
// </etymology>
//
// <synopsis>
// This class adds to its base WCTool to provide a tool for drawing,
// resizing and moving rectangles on a WorldCanvas.  While WCRectTool
// is not abstract, it performs no useful function.  The programmer
// should derive from this class, and implement the functions
// doubleInside and doubleOutside, which are called when the user
// double-clicks the key or mouse button inside or outside an existing
// rectangle respectively.  It is up to the programmer to decide what
// double clicks inside and outside the rectangle correspond to,
// although it is recommended that a double click inside correspond to
// the main action of the tool, and a double click outside correspond
// to a secondary action of the tool, if indeed a secondary action
// exists.
//
// The rectangle is drawn by dragging the mouse from one corner to
// the diagonally opposite corner.  Once constructed, the rectangle
// can be resized by dragging its corners, or relocated by dragging
// inside the rectangle.  The rectangle is removed from the display
// when the Esc key is pressed.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Many activities on the WorldCanvas will be based on the user drawing
// a rectangle, and then proceeding to some action with that rectangle.
// A nice example is zooming.
// </motivation>
//
// <todo asof="1999/07/23">
//   <li> Add time constraint to double click detection
// </todo>

	class WCRectTool : public WCTool, public DTVisible {

	public:

		// Constructor
		WCRectTool(WorldCanvas *wcanvas,
		           Display::KeySym keysym = Display::K_Pointer_Button1,
		           const Bool persistent = False);

		// Destructor
		virtual ~WCRectTool();

		// Switch the tool off - this calls the base class disable, and
		// then erases the rectangle if it's around
		virtual void disable();

		// Functions called by the local event handling operators -
		// these handle the drawing of the rectangle.  In special
		// conditions, namely double clicking the key, they will
		// pass control on to the doubleInside and doubleOutside
		// functions
		// <group>
		virtual void keyPressed(const WCPositionEvent &ev);
		virtual void keyReleased(const WCPositionEvent &ev);
		virtual void otherKeyPressed(const WCPositionEvent &ev);
		virtual void moved(const WCMotionEvent &ev, const viewer::region::region_list_type & /*selected_regions*/);
		virtual void refresh(const WCRefreshEvent &ev);
		// </group>

		// Functions special to the rectangle event handling - called when
		// there is a double click inside/outside the rectangle
		// <group>
		virtual void doubleInside() { };
		virtual void doubleOutside() { };
		// </group>

		// Functions called when a rectangle is ready and not being
		// editted, and when this status changes
		// <group>
		virtual void rectangleReady() { };
		virtual void rectangleNotReady() { };
		// </group>

		// Retrieve the rectangle corners
		virtual void get(Int &x1, Int &y1, Int &x2, Int &y2) const ;

	private:

		// do the rectangles persist after double clicks?
		Bool itsRectanglePersistent;

		// is the rectangle on screen?
		Bool itsOnScreen;

		// are we actively zooming?
		Bool itsActive;

		// have we moved?
		Bool itsMoved;

		// do we have a rectangle drawn yet?
		Bool itsRectangleExists;

		// adjustment mode
		enum AdjustMode {
		    Off,
		    Move
		};
		WCRectTool::AdjustMode itsAdjustMode;

		// coordinates of the rectangle: pixel and world
		// <group>
		Int itsX1, itsY1, itsX2, itsY2;
		Vector<Double> itsStoredWorldBlc, itsStoredWorldTrc;
		// </group>

		// set the coordinates of the rectangle
		// <group>
		virtual void set(const Int &x1, const Int &y1, const Int &x2, const Int &y2);
		// </group>

		// set/get only the anchor point
		// <group>
		virtual void set(const Int &x1, const Int &y1);
		virtual void get(Int &x1, Int &y1) const ;
		// </group>

		// preserve/restore the world coordinates
		// <group>
		virtual void preserve();
		virtual void restore();
		// </group>

		// draw the rubberband box on a PixelCanvas
		virtual void draw(const Bool drawHandles = False);

		// reset this drawer
		virtual void reset();

		// storage of the handle coordinates
		Vector<Int> itsHX, itsHY;

		// position that move started from
		Int itsBaseMoveX, itsBaseMoveY;

		// position of last press event
		Int itsLastPressX, itsLastPressY;
		Int its2ndLastPressX, its2ndLastPressY;

		// position of last release event
		Int itsLastReleaseX, itsLastReleaseY;

		// store the times of the last two presses here:
		Double itsLastPressTime, its2ndLastPressTime;

	};


} //# NAMESPACE CASA - END

#endif



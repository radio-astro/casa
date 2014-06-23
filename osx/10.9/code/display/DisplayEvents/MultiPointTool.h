//# MultiPointTool.h: Base class for MultiWorldCanvas event-based rectangle tools
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
//# $Id$

#ifndef DISPLAY_MULTIPOINTTOOL_H
#define DISPLAY_MULTIPOINTTOOL_H

#include <list>
#include <casa/aips.h>
#include <display/DisplayEvents/MultiRectTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for MultiWorldCanvas event-based rectangle tools
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
// MultiPointTool stands for MultiWorldCanvas Rectangle Tool
// </etymology>
//
// <synopsis>
// This class adds to its base WCTool to provide a tool for drawing,
// resizing and moving rectangles on a WorldCanvas.  While MultiPointTool
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

	class MultiPointTool : public MultiRectTool {

	public:

		// Constructor
		MultiPointTool( viewer::RegionSourceFactory *rsf, PanelDisplay* pd,
		                Display::KeySym keysym = Display::K_Pointer_Button1, const Bool persistent = False ) :
			MultiRectTool( rsf, pd, keysym, persistent ) { }

		// Destructor
		virtual ~MultiPointTool() { }

		// returns a set which indicates regions this creator creates...
		const std::set<viewer::region::RegionTypes> &regionsCreated( ) const;

		RegionToolTypes type( ) const {
			return POINTTOOL;
		}

	protected:

		bool checkType( viewer::region::RegionTypes t ) {
			return t == viewer::region::PointRegion;
		}
		virtual shared_ptr<viewer::Rectangle> allocate_region( WorldCanvas *wc, double x1, double y1, double x2, double y2, VOID *region_specific_state ) const;

	};

} //# NAMESPACE CASA - END

#endif

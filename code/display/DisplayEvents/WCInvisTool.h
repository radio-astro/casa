//# WCInvisTool.h: Base class for WC event-based invisible position tools
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

#ifndef TRIALDISPLAY_WCINVISTOOL_H
#define TRIALDISPLAY_WCINVISTOOL_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base implementation of WCTool for invisible (non-drawing) tools.
// </summary>
//
// <synopsis>
// This class implements some of WCTool, and adds additional interface
// to support invisible (ie. non-drawing) tools for WorldCanvases.
// </synopsis>

	class WCInvisTool : public WCTool {

	public:

		// constructor
		WCInvisTool(WorldCanvas *wcanvas,
		            Display::KeySym keysym = Display::K_Pointer_Button1);

		// destructor
		virtual ~WCInvisTool();

		// Functions called by the local event handling operators -
		// these handle the drawing of the rectangle.  In special
		// conditions, namely double clicking the key, they will
		// pass control on to the doubleInside and doubleOutside
		// functions
		// <group>
		virtual void keyPressed(const WCPositionEvent &/*ev*/);
		virtual void keyReleased(const WCPositionEvent &/*ev*/);
		virtual void moved(const WCMotionEvent &/*ev*/, const viewer::region::region_list_type & /*selected_regions*/);
		// </group>

		// Function called when a position is ready
		virtual void positionReady() { };

		// Retrieve the position in linear WorldCanvas coordinates
		virtual void get(Double &x, Double &y) const;

		// Retrieve the position in fractional linear WorldCanvas coordinates
		virtual void getFractional(Double &x, Double &y) const;

	private:

		// are we actively marking positions?
		Bool itsActive;

		// have we moved
		Bool itsMoved;

		// linear coordinates of the position
		Double itsX, itsY;

	};


} //# NAMESPACE CASA - END

#endif

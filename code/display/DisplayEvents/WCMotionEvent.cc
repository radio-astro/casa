//# WCMotionEvent.cc: class which stores WorldCanvas motion event information
//# Copyright (C) 1993,1994,1995,1996,1999,2000
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

#include <display/DisplayEvents/WCMotionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	WCMotionEvent::WCMotionEvent(WorldCanvas *wc, const uInt &modifiers,
	                             const Int &pixX, const Int &pixY,
	                             const Double &linX, const Double &linY,
	                             const Vector<Double> &world) :
		WorldCanvasEvent(wc),
		itsPixX(pixX),
		itsPixY(pixY),
		itsLinX(linX),
		itsLinY(linY),
		itsWorld(world),
		itsModifiers(modifiers) {
	}

// Destructor.
	WCMotionEvent::~WCMotionEvent() {
	}

// (Required) default constructor.
	WCMotionEvent::WCMotionEvent() :
		WorldCanvasEvent(),
		itsPixX(0),
		itsPixY(0),
		itsLinX(0),
		itsLinY(0),
		itsModifiers(0) {
		itsWorld.resize(2);
		itsWorld = 0;
	}

// (Required) copy constructor.
	WCMotionEvent::WCMotionEvent(const WCMotionEvent &other) :
		WorldCanvasEvent(other),
		itsPixX(other.itsPixX),
		itsPixY(other.itsPixY),
		itsLinX(other.itsLinX),
		itsLinY(other.itsLinY),
		itsWorld(other.itsWorld),
		itsModifiers(other.itsModifiers) {
	}

// (Required) copy assignment.
	WCMotionEvent &WCMotionEvent::operator=(const WCMotionEvent &other) {
		if (this != &other) {
			WorldCanvasEvent::operator=(other);
			itsPixX = other.itsPixX;
			itsPixY = other.itsPixY;
			itsLinX = other.itsLinX;
			itsLinY = other.itsLinY;
			itsWorld.resize(other.itsWorld.size( ));
			itsWorld = other.itsWorld;
			itsModifiers = other.itsModifiers;
		}
		return *this;
	}



} //# NAMESPACE CASA - END


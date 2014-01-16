//# PCPositionEvent.cc: class which stores PixelCanvas position event info
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

#include <display/DisplayEvents/PCPositionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	PCPositionEvent::PCPositionEvent(PixelCanvas *pc, const Display::KeySym &key,
	                                 const Bool &keystate, const Int &x,
	                                 const Int &y, const uInt &modifiers) :
		PCMotionEvent(pc, x, y, modifiers),
		itsKey(key),
		itsKeyState(keystate) {
	}

// Destructor.
	PCPositionEvent::~PCPositionEvent() {
	}

// (Required) default constructor.
	PCPositionEvent::PCPositionEvent() :
		PCMotionEvent(),
		itsKey(Display::K_None),
		itsKeyState(0) {
	}

// (Required) copy constructor.
	PCPositionEvent::PCPositionEvent(const PCPositionEvent &other) :
		PCMotionEvent(other),
		itsKey(other.itsKey),
		itsKeyState(other.itsKeyState) {
	}

// (Required) copy assignment.
	PCPositionEvent &PCPositionEvent::operator=(const PCPositionEvent &other) {
		if (this != &other) {
			PCMotionEvent::operator=(other);
			itsKey = other.itsKey;
			itsKeyState = other.itsKeyState;
		}
		return *this;
	}



} //# NAMESPACE CASA - END


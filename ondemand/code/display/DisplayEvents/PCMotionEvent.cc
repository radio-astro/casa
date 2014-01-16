//# PCMotionEvent.cc: class which stores PixelCanvas motion event information
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

#include <display/DisplayEvents/PCMotionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	PCMotionEvent::PCMotionEvent(PixelCanvas *pc, const Int &x, const Int &y,
	                             const uInt &modifiers) :
		PixelCanvasEvent(pc),
		itsX(x),
		itsY(y),
		itsModifiers(modifiers) {
	}

// Destructor.
	PCMotionEvent::~PCMotionEvent() {
	}

// (Required) default constructor.
	PCMotionEvent::PCMotionEvent() :
		PixelCanvasEvent(),
		itsX(0),
		itsY(0),
		itsModifiers(0) {
	}

// (Required) copy constructor.
	PCMotionEvent::PCMotionEvent(const PCMotionEvent &other) :
		PixelCanvasEvent(other),
		itsX(other.itsX),
		itsY(other.itsY),
		itsModifiers(other.itsModifiers) {
	}

// (Required) copy assignment.
	PCMotionEvent &PCMotionEvent::operator=(const PCMotionEvent &other) {
		if (this != &other) {
			PixelCanvasEvent::operator=(other);
			itsX = other.itsX;
			itsY = other.itsY;
			itsModifiers = other.itsModifiers;
		}
		return *this;
	}



} //# NAMESPACE CASA - END


//# WCPositionEvent.cc: class which stores WorldCanvas position event info
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

#include <display/DisplayEvents/WCPositionEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
WCPositionEvent::WCPositionEvent(WorldCanvas *wc, 
				 const Display::KeySym &key,
				 const Bool &keystate,
				 const uInt &modifiers,
				 const Int &pixX, const Int &pixY,
				 const Double &linX, const Double &linY,
				 const Vector<Double> &world) :
  WCMotionEvent(wc, modifiers, pixX, pixY, linX, linY, world),
  itsKey(key),
  itsKeyState(keystate) {
}

// Destructor.
WCPositionEvent::~WCPositionEvent() {
}

// (Required) default constructor.
WCPositionEvent::WCPositionEvent() :
  WCMotionEvent(),
  itsKey(Display::K_None),
  itsKeyState(0) {
}

// (Required) copy constructor.
WCPositionEvent::WCPositionEvent(const WCPositionEvent &other) :
  WCMotionEvent(other),
  itsKey(other.itsKey),
  itsKeyState(other.itsKeyState) {
}

// (Required) copy assignment.
WCPositionEvent &WCPositionEvent::operator=(const WCPositionEvent &other) {
  if (this != &other) {
    WCMotionEvent::operator=(other);
    itsKey = other.itsKey;
    itsKeyState = other.itsKeyState;
  }
  return *this;
}



} //# NAMESPACE CASA - END


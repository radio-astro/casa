//# DisplayEvent.cc: class for basic event information in the display classes
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

#include <display/DisplayEvents/DisplayEvent.h>
#include <casa/OS/Time.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
DisplayEvent::DisplayEvent() {
  Time x;
  itsTimeOfEvent = x.julianDay() * 24.0 * 60.0 * 60.0;
}

// (Required) copy constructor.
DisplayEvent::DisplayEvent(const DisplayEvent &other) :
  itsTimeOfEvent(other.itsTimeOfEvent) {
}

// Destructor.
DisplayEvent::~DisplayEvent() {
}

// (Required) copy assignment.
DisplayEvent &DisplayEvent::operator=(const DisplayEvent &other) {
  if (this != &other) {
    itsTimeOfEvent = other.itsTimeOfEvent;
  }
  return *this;
}

// Return the time of this event.
Double DisplayEvent::timeOfEvent() const {
  return itsTimeOfEvent;
}

} //# NAMESPACE CASA - END


//# WCRefreshEvent.cc: class which stores WorldCanvas refresh event information
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

#include <casa/aips.h>
#include <display/DisplayEvents/WCRefreshEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
WCRefreshEvent::WCRefreshEvent(WorldCanvas *wc, 
			       const Display::RefreshReason &reason) :
  WorldCanvasEvent(wc),
  itsRefreshReason(reason) {
}

// Destructor.
WCRefreshEvent::~WCRefreshEvent() {
}

// (Required) default constructor.
WCRefreshEvent::WCRefreshEvent() :
  WorldCanvasEvent(),
  itsRefreshReason(Display::UserCommand) {
}

// (Required) copy constructor.
WCRefreshEvent::WCRefreshEvent(const WCRefreshEvent &other) :
  WorldCanvasEvent(other),
  itsRefreshReason(other.itsRefreshReason) {
}

// (Required) copy assignment.
WCRefreshEvent &WCRefreshEvent::operator=(const WCRefreshEvent &other) {
  if (this != &other) {
    WorldCanvasEvent::operator=(other);
    itsRefreshReason = other.itsRefreshReason;
  }
  return *this;
}



} //# NAMESPACE CASA - END


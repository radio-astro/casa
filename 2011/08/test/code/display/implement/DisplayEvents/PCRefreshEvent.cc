//# PCRefreshEvent.cc: class which stores PixelCanvas refresh event information
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
#include <display/DisplayEvents/PCRefreshEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
PCRefreshEvent::PCRefreshEvent(PixelCanvas *pc, 
			       const Display::RefreshReason &reason) :
  PixelCanvasEvent(pc),
  itsRefreshReason(reason) {
}

// Destructor.
PCRefreshEvent::~PCRefreshEvent() {
}

// (Required) default constructor.
PCRefreshEvent::PCRefreshEvent() :
  PixelCanvasEvent(),
  itsRefreshReason(Display::UserCommand) {
}

// (Required) copy constructor.
PCRefreshEvent::PCRefreshEvent(const PCRefreshEvent &other) :
  PixelCanvasEvent(other),
  itsRefreshReason(other.itsRefreshReason) {
}

// (Required) copy assignment.
PCRefreshEvent &PCRefreshEvent::operator=(const PCRefreshEvent &other) {
  if (this != &other) {
    PixelCanvasEvent::operator=(other);
    itsRefreshReason = other.itsRefreshReason;
  }
  return *this;
}


} //# NAMESPACE CASA - END


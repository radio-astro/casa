//# DisplayMethod.cc: Base class for drawing data in the Display Library
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/DisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DisplayMethod::DisplayMethod(DisplayData *parentDisplayData) : 
  itsParentDisplayData(parentDisplayData) {
}

DisplayMethod::~DisplayMethod() { }

void DisplayMethod::cleanup() { }

void DisplayMethod::addRestriction(Attribute &at, Bool permanent) {
  restrictions.add(at, permanent);
}

void DisplayMethod::addRestrictions(AttributeBuffer &at) {
  restrictions.add(at);
}

void DisplayMethod::setRestriction(Attribute &at) {
  restrictions.set(at);
}

void DisplayMethod::setRestrictions(AttributeBuffer &at) {
  restrictions.set(at);
}

void DisplayMethod::removeRestriction(const String &name) {
  restrictions.remove(name);
}

Bool DisplayMethod::existRestriction(const String &name) {
  return restrictions.exists(name);
}

void DisplayMethod::clearRestrictions() {
  restrictions.clear();
}

Bool DisplayMethod::matches(Attribute &at) {
  return restrictions.matches(at);
}

Bool DisplayMethod::matches(AttributeBuffer &atBuf) {
  return restrictions.matches(atBuf);
}

// (Required) default constructor.
DisplayMethod::DisplayMethod() {
}

// (Required) copy constructor.
DisplayMethod::DisplayMethod(const DisplayMethod &other) {
}

// (Required) copy assignment.
void DisplayMethod::operator=(const DisplayMethod &other) {
}



} //# NAMESPACE CASA - END


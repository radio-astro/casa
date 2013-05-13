//# DDModEvent.cc: DisplayData Modified Event used to signal change in data
//# Copyright (C) 1999,2000,2003
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

#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayEvents/DDModEvent.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	DDModEvent::DDModEvent(DisplayData* dd, const Record *rec) :
		DisplayDataEvent(dd),
		itsRec(rec) {
	}

// Destructor.
	DDModEvent::~DDModEvent() {}

// (Required) default constructor.
	DDModEvent::DDModEvent() :
		DisplayDataEvent(),
		itsRec(0) {
	}

// (Required) copy constructor.
	DDModEvent::DDModEvent(const DDModEvent &other) :
		DisplayDataEvent(other),
		itsRec(other.dataRecord()) {
	}

// (Required) copy assignment.
	DDModEvent &DDModEvent::operator=(const DDModEvent &other) {
		if (this != &other) {
			DisplayDataEvent::operator=(other);
			itsRec = other.itsRec;
		}
		return *this;
	}

} //# NAMESPACE CASA - END


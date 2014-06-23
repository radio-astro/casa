//# DTVisible.cc: class providing draw style settings for visible DisplayTools
//# Copyright (C) 1999,2000,2001
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

#include <casa/System/Aipsrc.h>
#include <casa/System/AipsrcValue.h>
#include <display/DisplayEvents/DTVisible.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	DTVisible::DTVisible() {
		Aipsrc::find(itsDrawColor, "display.controls.color", "green");
		AipsrcValue<Int>::find(itsLineWidth, "display.controls.linewidth",
		                       Int(1));
		AipsrcValue<Double>::find(itsDoubleClickInterval,
		                          "display.controls.doubleclickinterval",
		                          Double(0.5));
	}

// Destructor.
	DTVisible::~DTVisible() {
	}

// (Required) copy constructor.
	DTVisible::DTVisible(const DTVisible &other) :
		itsDrawColor(other.itsDrawColor),
		itsLineWidth(other.itsLineWidth),
		itsDoubleClickInterval(other.itsDoubleClickInterval) {
	}

// (Required) copy assignment.
	DTVisible &DTVisible::operator=(const DTVisible &other) {
		if (this != &other) {
			itsDrawColor = other.itsDrawColor;
			itsLineWidth = other.itsLineWidth;
			itsDoubleClickInterval = other.itsDoubleClickInterval;
		}
		return *this;
	}


} //# NAMESPACE CASA - END


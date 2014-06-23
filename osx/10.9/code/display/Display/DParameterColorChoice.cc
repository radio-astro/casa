//# DParameterColorChoice.cc: class to store and retrieve color selections
//# Copyright (C) 2000
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

#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/DParameterColorChoice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor
	DParameterColorChoice::DParameterColorChoice(const String name,
	        const String description,
	        const String help,
	        const String context,
	        const Bool editable) :
		DParameterChoice() {
		setName(name);
		setDescription(description);
		setHelp(help);
		Vector<String> colors(11);
		colors(0) = "foreground";
		colors(1) = "background";
		colors(2) = "black";
		colors(3) = "white";
		colors(4) = "red";
		colors(5) = "green";
		colors(6) = "blue";
		colors(7) = "cyan";
		colors(8) = "magenta";
		colors(9) = "yellow";
		colors(10) = "gray";
		setOptions(colors);
		setDefaultValue(colors(0));
		setValue(colors(0));
		setContext(context);
		setEditable(editable);

	}

// Copy constructor.
	DParameterColorChoice::DParameterColorChoice(const DParameterColorChoice &
	        other) :
		DParameterChoice(other) {
	}

// Destructor.
	DParameterColorChoice::~DParameterColorChoice() {
	}

// Copy assignment.
	DParameterColorChoice &DParameterColorChoice::
	operator=(const DParameterColorChoice &other) {
		if (this != &other) {
			DParameterChoice::operator=(other);
		}
		return *this;
	}

// Default constructor.
	DParameterColorChoice::DParameterColorChoice() :
		DParameterChoice() {
	}

} //# NAMESPACE CASA - END


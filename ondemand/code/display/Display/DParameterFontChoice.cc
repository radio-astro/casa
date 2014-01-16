//# DParameterFontChoice.cc: class to store and retrieve Font selections
//# Copyright (C) 2000,2002
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
//# $Id:

#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/DParameterFontChoice.h>
#include <display/Display/DLFont.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor
	DParameterFontChoice::DParameterFontChoice(const String name,
	        const String description,
	        const String help,
	        const String context) :

		DParameterChoice() {

		setName(name);
		setDescription(description);
		setHelp(help);

		DLFont* dummy = new DLFont();
		Vector<String> fontNames = dummy->getAllNames();
		delete dummy;
		dummy = 0;

		setOptions(fontNames);
		// Try for helvetica (elem 8 in dlfont).. else whatever we get first.
		if (fontNames.nelements() >= 9) {
			setDefaultValue(fontNames(8));
			setValue(fontNames(8));
		} else {
			setDefaultValue(fontNames(0));
			setValue(fontNames(0));
		}
		setContext(context);
		setEditable(False);

	}

// Copy constructor.
	DParameterFontChoice::DParameterFontChoice(const DParameterFontChoice &
	        other) :
		DParameterChoice(other) {
	}

// Destructor.
	DParameterFontChoice::~DParameterFontChoice() {
	}

// Copy assignment.
	DParameterFontChoice &DParameterFontChoice::
	operator=(const DParameterFontChoice &other) {
		if (this != &other) {
			DParameterChoice::operator=(other);
		}
		return *this;
	}


// Default constructor.
	DParameterFontChoice::DParameterFontChoice() :
		DParameterChoice() {
	}


} //# NAMESPACE CASA - END


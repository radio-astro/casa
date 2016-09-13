//# DParameterButton.cc: class to package interface to button autoguis
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
//# $Id$

#include <display/Display/DParameterButton.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	DParameterButton::DParameterButton(const String name,
	                                   const String description, const String text,
	                                   const String help, const String context,
	                                   const Bool editable) :
		DisplayParameter(name, description, help, context, False, editable),
		itsText(text) {  }

// Copy constructor.
	DParameterButton::DParameterButton(const DParameterButton &other) :
		DisplayParameter(other), itsText(other.itsText) {  }

// Destructor.
	DParameterButton::~DParameterButton() {  }

// Copy assignment.
	DParameterButton& DParameterButton::operator=(const DParameterButton& other) {
		if (this != &other) itsText = other.itsText;
		return *this;
	}

// Determine whether the record indicated that the button was pressed.
// (This will be True if the record simply contains a field with this
// DParameter's name-id).
	Bool DParameterButton::fromRecord(const RecordInterface &record) {
		Bool btnval, notFound;
		displayOptions().readOptionRecord(btnval, notFound, record, name());
		return !notFound;
	}

// Describe this parameter in a record.
	void DParameterButton::toRecord(RecordInterface &record, const Bool,
	                                const Bool overwrite) {
		if (record.isDefined(name())) {
			if (overwrite) record.removeField(name());
			else return;
		}

		Record rec = baseDescription();
		rec.define("ptype", "button");
		rec.define("text", itsText);
		rec.define("default", True);
		rec.define("value", True);

		record.defineRecord(name(), rec);
	}

} //# NAMESPACE CASA - END


//# DParameterSwitch.cc: class to store and retrieve boolean parameters
//# Copyright (C) 2000,2001,2002
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

#include <display/Utilities/DisplayOptions.h>
#include <display/Display/DParameterSwitch.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	DParameterSwitch::DParameterSwitch(const String name,
	                                   const String description,
	                                   const String help,
	                                   const Bool defaultvalue,
	                                   const Bool value,
	                                   const String context) :
		DisplayParameter(name, description, help, context, False, False),
		itsDefaultValue(defaultvalue),
		itsValue(value) {
	}

// Copy constructor.
	DParameterSwitch::DParameterSwitch(const DParameterSwitch &other) :
		DisplayParameter(other),
		itsDefaultValue(other.itsDefaultValue),
		itsValue(other.itsValue) {
	}

// Destructor.
	DParameterSwitch::~DParameterSwitch() {
	}

// Copy assignment.
	DParameterSwitch& DParameterSwitch::operator=(const DParameterSwitch &other) {
		if (this != &other) {
			DisplayParameter::operator=(other);
			itsDefaultValue = other.itsDefaultValue;
			itsValue = other.itsValue;
		}
		return *this;
	}

// Update the value of this parameter from a record.
	Bool DParameterSwitch::fromRecord(const RecordInterface& record) {
		static Bool error, result;
		Bool previousval = itsValue;
		result = displayOptions().readOptionRecord(itsValue, error, record, name());
		if (result) {
			return True;
		} else {
			itsValue = previousval;
		}
		return False;
	}

// Describe this parameter in a record.
	void DParameterSwitch::toRecord(RecordInterface& record,
	                                const Bool, const Bool overwrite) {
		if (record.isDefined(name())) {
			if (overwrite) {
				record.removeField(name());
			} else {
				return;
			}
		}

		Record rec = baseDescription();
		rec.define("ptype", "boolean");
		rec.define("default", itsDefaultValue);
		rec.define("value", itsValue);
		record.defineRecord(name(), rec);
	}

// Default constructor.
	DParameterSwitch::DParameterSwitch() :
		DisplayParameter(),
		itsDefaultValue(False),
		itsValue(False) {
	}

} //# NAMESPACE CASA - END


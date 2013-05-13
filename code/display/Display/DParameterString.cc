//# DParameterString.cc: class to store and retrieve String parameters
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
#include <casa/Utilities/DataType.h>
#include <display/Utilities/DisplayOptions.h>
#include <display/Display/DParameterString.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	DParameterString::DParameterString(const String name, const String description,
	                                   const String help,
	                                   const String defaultvalue,
	                                   const String value, const String context,
	                                   const Bool editable) :
		DisplayParameter(name, description, help, context, False, editable),
		itsDefault(defaultvalue),
		itsValue(value) {
	}

// Copy constructor.
	DParameterString::DParameterString(const DParameterString &other) :
		DisplayParameter(other),
		itsDefault(other.itsDefault),
		itsValue(other.itsValue) {
	}

// Destructor.
	DParameterString::~DParameterString() {
	}

// Copy assignment.
	DParameterString &DParameterString::operator=(const DParameterString &other) {
		if (this != &other) {
			itsDefault = other.itsDefault;
			itsValue = other.itsValue;
		}
		return *this;
	}

// Update the value of this parameter from a record.
	Bool DParameterString::fromRecord(const RecordInterface &record) {
		static Bool error;
		return displayOptions().readOptionRecord(itsValue, error, record, name());
	}

// Describe this parameter in a record.
	void DParameterString::toRecord(RecordInterface &record,
	                                const Bool, const Bool overwrite) {
		if (record.isDefined(name())) {
			if (overwrite) {
				record.removeField(name());
			} else {
				return;
			}
		}

		Record rec = baseDescription();
		rec.define("ptype", "string");
		rec.define("default", itsDefault);
		rec.define("value", itsValue);

		record.defineRecord(name(), rec);
	}

// Default constructor.
	DParameterString::DParameterString() :
		DisplayParameter(),
		itsDefault(""),
		itsValue("") {
	}

} //# NAMESPACE CASA - END


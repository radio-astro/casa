//# DParameterChoice.cc: class to store and retrieve choice parameters 
//# Copyright (C) 2000,2001
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
#include <display/Display/DParameterChoice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
DParameterChoice::DParameterChoice(const String name, const String description,
				   const String help, 
				   const Vector<String> &options,
				   const String defaultvalue,
				   const String value, const String context, 
				   const Bool editable) :
  DisplayParameter(name, description, help, context, False, False),
  itsEditable(editable),
  itsOptions(options),
  itsDefaultValue(defaultvalue),
  itsValue(value) {
  if (!existsOption(defaultvalue) || !existsOption(value)) {
    throw(AipsError("Invalid default or value in DParameterChoice"));
  }
}

// Copy constructor.
DParameterChoice::DParameterChoice(const DParameterChoice &other) :
  DisplayParameter(other),
  itsEditable(other.itsEditable),
  itsOptions(other.itsOptions),
  itsDefaultValue(other.itsDefaultValue),
  itsValue(other.itsValue) {
}

// Destructor.
DParameterChoice::~DParameterChoice() {
}

// Copy assignment.
DParameterChoice &DParameterChoice::operator=(const DParameterChoice &other) {
  if (this != &other) {
    DisplayParameter::operator=(other);
    itsEditable = other.itsEditable;
    itsOptions = other.itsOptions;
    itsDefaultValue = other.itsDefaultValue;
    itsValue = other.itsValue;
  }
  return *this;
}

// Update the value of this parameter from a record.
Bool DParameterChoice::fromRecord(const RecordInterface &record) {
  static Bool error, result;
  String previousval = itsValue;
  result = displayOptions().readOptionRecord(itsValue, error, record, name());
  if (result) {
    // check that new itsValue is a member of itsOption
    if (existsOption(itsValue)) {
      return True;
    } else {
      if (itsEditable) {
	itsOptions.resize(itsOptions.nelements()+1,True);
	itsOptions(itsOptions.nelements()-1) = itsValue;
	return True;
      }
      itsValue = previousval;
      return False;
    }
  }
  return False;
}

// Describe this parameter in a record.
void DParameterChoice::toRecord(RecordInterface &record, 
				const Bool, const Bool overwrite) {
  if (record.isDefined(name())) {
    if (overwrite) {
      record.removeField(name());
    } else {
      return;
    }
  }

  Record rec = baseDescription();
  if (itsEditable) {
    rec.define("ptype", "userchoice");
  } else {
    rec.define("ptype", "choice");
  }
  rec.define("popt", itsOptions);
  rec.define("default", itsDefaultValue);
  rec.define("value", itsValue);
  record.defineRecord(name(), rec);
}

// Default constructor.
DParameterChoice::DParameterChoice() :
  DisplayParameter(),
  itsEditable(False),
  itsDefaultValue(""),
  itsValue("") {
  itsOptions.resize(0);
}

Bool DParameterChoice::existsOption(const String value) {
  Bool exists = False;
  for (uInt i = 0; (i < itsOptions.nelements()) && !exists; i++) {
    exists = (itsOptions(i) == value);    
  }
  return exists;
}

} //# NAMESPACE CASA - END


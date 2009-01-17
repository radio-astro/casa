//# DParameterRange.cc: class to store and retrieve range parameters 
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
#include <display/Display/DParameterRange.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
template <class T>
DParameterRange<T>::DParameterRange(const String name, 
				    const String description,
				    const String help, const T minimum, 
				    const T maximum, const T resolution, 
				    const T defaultvalue, const T value, 
				    const String context, 
				    const Bool editable,
				    const Bool provideentry) :
  DisplayParameter(name, description, help, context, False, editable),
  itsMinimum(minimum),
  itsMaximum(maximum),
  itsResolution(resolution),
  itsDefault(defaultvalue),
  itsValue(value),
  itsProvideEntry(provideentry) {

}

// Default constructor.
template <class T>
DParameterRange<T>::DParameterRange() :
  DisplayParameter(),
  itsMinimum(0),
  itsMaximum(0),
  itsResolution(1),
  itsDefault(0),
  itsValue(0),
  itsProvideEntry(False) {
}

// Copy constructor.
template <class T>
DParameterRange<T>::DParameterRange(const DParameterRange<T> &other) :
  DisplayParameter(other),
  itsMinimum(other.itsMinimum),
  itsMaximum(other.itsMaximum),
  itsResolution(other.itsResolution),
  itsDefault(other.itsDefault),
  itsValue(other.itsValue),
  itsProvideEntry(other.itsProvideEntry) {
}

// Destructor.
template <class T>
DParameterRange<T>::~DParameterRange() {
}

// Copy assignment.
template <class T>
DParameterRange<T> &DParameterRange<T>::operator=(const DParameterRange<T> &other) {
  if (this != &other) {
    itsMinimum = other.itsMinimum;
    itsMaximum = other.itsMaximum;
    itsResolution = other.itsResolution;
    itsDefault = other.itsDefault;
    itsValue = other.itsValue;
    itsProvideEntry = other.itsProvideEntry;
  }
  return *this;
}

// Update the value of this parameter from a record.
template <class T>
Bool DParameterRange<T>::fromRecord(const RecordInterface &record) {
  static Bool error;
  return displayOptions().readOptionRecord(itsValue, error, record, name());
}

// Describe this parameter in a record.
template <class T>
void DParameterRange<T>::toRecord(RecordInterface &record, 
				  const Bool, const Bool overwrite) {
  if (record.isDefined(name())) {
    if (overwrite) {
      record.removeField(name());
    } else {
      return;
    }
  }

  Record rec = baseDescription();
  T tmp;
  DataType dtype = whatType(&tmp);
  switch(dtype) {
  case TpInt:
    rec.define("ptype", "intrange");
    break;
  case TpFloat:
    rec.define("ptype", "floatrange");
    break;
  default:
    throw(AipsError("Invalid template for DParameterRange"));
    break;
  }
  rec.define("pmin", itsMinimum);
  rec.define("pmax", itsMaximum);
  rec.define("presolution", itsResolution);
  rec.define("default", itsDefault);
  rec.define("value", itsValue);
  rec.define("provideentry", itsProvideEntry);

  record.defineRecord(name(), rec);
}


} //# NAMESPACE CASA - END


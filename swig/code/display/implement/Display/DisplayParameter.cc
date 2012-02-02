//# DisplayParameter.cc: class to store and retrieve parameters 
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

#include <display/Display/DisplayParameter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
DisplayParameter::DisplayParameter(const String name, const String description,
				   const String help, const String context,
				   const Bool allowunset,
				   const Bool editable) :
  itsName(name),
  itsDescription(description),
  itsHelp(help),
  itsContext(context),
  itsAllowUnset(allowunset),
  itsEditable(editable) {
}

// Copy constructor.
DisplayParameter::DisplayParameter(const DisplayParameter &other) :
  itsName(other.itsName),
  itsDescription(other.itsDescription),
  itsHelp(other.itsHelp),
  itsContext(other.itsContext),
  itsAllowUnset(other.itsAllowUnset),
  itsEditable(other.itsEditable) {
}

// Destructor.
DisplayParameter::~DisplayParameter() {
}

// Copy assignment.
DisplayParameter &DisplayParameter::operator=(const DisplayParameter &other) {
  if (this != &other) {
    itsName = other.itsName;
    itsDescription = other.itsDescription;
    itsHelp = other.itsHelp;
    itsContext = other.itsContext;
    itsAllowUnset = other.itsAllowUnset;
    itsEditable = other.itsEditable;
  }
  return *this;
}

// Default constructor.
DisplayParameter::DisplayParameter() :
  itsName(""),
  itsDescription(""),
  itsHelp(""),
  itsContext(""),
  itsAllowUnset(False),
  itsEditable(False) {
}

// Return a basic description of this parameter.
Record DisplayParameter::baseDescription() {
  Record rec;
  rec.define("dlformat", itsName);
  rec.define("listname", itsDescription);
  rec.define("help", itsHelp);
  rec.define("allowunset", itsAllowUnset);
  rec.define("editable", itsEditable);
  if (itsContext != String("")) {
    rec.define("context", itsContext);
  }
  return rec;
}


} //# NAMESPACE CASA - END


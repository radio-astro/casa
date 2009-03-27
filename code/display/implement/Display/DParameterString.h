//# DParameterString.h: class to store and retrieve String parameters 
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

#ifndef TRIALDISPLAY_DPARAMETERSTRING_H
#define TRIALDISPLAY_DPARAMETERSTRING_H

#include <casa/aips.h>
#include <display/Display/DisplayParameter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of DisplayParameter to store String parameters.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DisplayParameter>DisplayParameter</linkto>
// </prerequisite>

// <etymology>
// DParameterString is an implementation of a DisplayParameter
// providing a String parameter type.
// </etymology>

// <synopsis>
// This class is derived from <linkto
// class=DisplayParameter>DisplayParameter</linkto> and provides a
// String-type parameter.  String parameters simply have a String
// value, and a default String value.  They cannot be unset.
// </synopsis>

// <example>

// A DParameterString can easily be used to store and update any
// parameter which can be expressed as a String.  In the following
// example, a DParameterString is constructed to store some text to be
// used for axis labelling:
// <srcblock>
// DParameterString pstring("xaxislabel", "X Axis Label", 
//                          "Enter the text to appear alongside the X Axis");
//
// // ...
// 
// // update the parameter from some Record
// pstring.fromRecord(rec);
//
// // examine the value of the parameter
// if (pstring.value() == "Four frogs found fabulous frocks") {
// // ...
// }
// </srcblock>
// </example>

// <motivation>
// Many parameters are naturally Strings, or can be expressed as
// Strings, hence this class.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2000/01/28">
// <li> Nothing known.
// </todo>

class DParameterString : public DisplayParameter {

 public:

  // Constructor taking the name of the parameter, a short
  // description, some help text, a default value, an initial value,
  // and flags indicating whether the parameter can be unset and is
  // editable.
  DParameterString(const String name, const String description,
		   const String help, const String defaultvalue,
		   const String value, const String context = "",
		   const Bool editable = True);

  // (Required) copy constructor.
  DParameterString(const DParameterString &other);

  // Destructor.
  virtual ~DParameterString();

  // (Required) copy assignment.
  DParameterString &operator=(const DParameterString &other);

  // Parse <src>record</src>, and update this parameter if a field
  // exists whose name matches that of this parameter.  Return
  // <src>True</src> if the parameter is changed, otherwise return
  // <src>False</src>.
  virtual Bool fromRecord(const RecordInterface &record);

  // Place a record describing this parameter in a sub-field of
  // <src>record</src> with name matching that of this parameter.  If
  // <src>overwrite</src> is <src>True</src>, then any existing field
  // with matching name will be overwritten.  If <src>fullrecord</src>
  // is <src>True</src>, then a complete description of the parameter
  // is given, otherwise just its current value is stored in
  // <src>record</src>.  Presently <src>fullrecord</src> is ignored.
  virtual void toRecord(RecordInterface &record, const Bool fullrecord = True,
			const Bool overwrite = False);

  // Return the default for this parameter.
  String defaultValue() 
    { return itsDefault; }

  // Return the current value of this parameter.
  String value() 
    { return itsValue; }

 protected:
  
  // (Required) default constructor.
  DParameterString();

 private:

  // Store for the default of this parameter.
  String itsDefault;

  // Store for the value of this parameter.
  String itsValue;

};


} //# NAMESPACE CASA - END

#endif

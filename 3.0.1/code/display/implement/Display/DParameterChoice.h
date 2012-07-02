//# DParameterChoice.h: class to store and retrieve Choice parameters 
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

#ifndef TRIALDISPLAY_DPARAMETERCHOICE_H
#define TRIALDISPLAY_DPARAMETERCHOICE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/DisplayParameter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of DisplayParameter to store choice parameters.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DisplayParameter>DisplayParameter</linkto>
// </prerequisite>

// <etymology>
// DParameterChoice is an implementation of a DisplayParameter
// providing a choice parameter type.
// </etymology>

// <synopsis>
// This class is derived from <linkto
// class=DisplayParameter>DisplayParameter</linkto> and provides a
// choice-type parameter.  Choice parameters simply have a vector
// of possible String selections, and a default selection.  They
// cannot be unset.
// </synopsis>

// <example>

// A DParameterChoice can easily be used to store and update any
// parameter which can be expressed as a selection from two or more
// options.  In the following example, a DParameterChoice is
// constructed to provide the name of the font to use in axis
// labelling:
// <srcblock>
// Vector<String> fonts(4);
// fonts(0) = "roman";
// // ...
// DParameterChoice pchoice("font", "Label font", 
//                          "Select the font to use for axis labelling",
//                          fonts, fonts(0), fonts(0));
//
// // ...
// 
// // update the parameter from some Record
// pchoice.fromRecord(rec);
//
// // examine the value of the parameter
// if (pchoice.value() == "italic") {
// // ...
// }
// </srcblock>
// </example>

// <motivation>
// Many parameters are naturally choices, or can be expressed as
// choices, hence this class.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2000/01/31">
// <li> Nothing known.
// </todo>

class DParameterChoice : public DisplayParameter {

 public:

  // Constructor taking the name of the parameter, a short
  // description, some help text, a list of allowed options, a default
  // value, an initial value, the context of the parameter, and a flag
  // indicating whether the parameter is editable.  Obviously the
  // <src>defaultvalue</src> and <src>value</src> parameters must
  // exist in the list of allowed options, otherwise an exception
  // is thrown.
  DParameterChoice(const String name, const String description,
		   const String help, const Vector<String> &options,
		   const String defaultvalue, const String value, 
		   const String context = "", const Bool editable = False);

  // (Required) copy constructor.
  DParameterChoice(const DParameterChoice &other);

  // Destructor.
  virtual ~DParameterChoice();

  // (Required) copy assignment.
  DParameterChoice &operator=(const DParameterChoice &other);

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

  // Return the list of allowed options for this parameter.
  Vector<String> options() const
    { return itsOptions; }

  // Return the default for this parameter.
  String defaultValue() 
    { return itsDefaultValue; }

  // Return the current value of this parameter.
  String value() 
    { return itsValue; }

  // Set or change if this parameter is editable (determines wether
  // choice or userchoice is returned in the record
  void setEditable(const Bool editable)
  { itsEditable = editable; } 

  // Set or change the list of allowed options for this parameter.
  void setOptions(const Vector<String> &options)
    { itsOptions = options; }

  // Set or change the default value for this parameter.
  void setDefaultValue(const String defaultvalue)
    { itsDefaultValue = defaultvalue; }

  // Set or change the current value.
  void setValue(const String value)
    { itsValue = value; }

  // Convenient syntax to set (only) the value.
  const String& operator=(const String &value) {
    itsValue = value; return value;  }
  
  // Determine whether this value exists in the list of options.
  Bool existsOption(const String value);

 protected:
  
  // (Required) default constructor.
  DParameterChoice();

 private:

  Bool itsEditable;
  // Store for the allowed options for this parameter.
  Vector<String> itsOptions;

  // Store for the default of this parameter.
  String itsDefaultValue;

  // Store for the value of this parameter.
  String itsValue;

};


} //# NAMESPACE CASA - END

#endif

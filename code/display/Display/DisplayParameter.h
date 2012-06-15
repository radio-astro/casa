//# DisplayParameter.h: base class for storing and parsing parameters 
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

#ifndef TRIALDISPLAY_DISPLAYPARAMETER_H
#define TRIALDISPLAY_DISPLAYPARAMETER_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <display/Utilities/DisplayOptions.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Base class for storing and parsing of parameters for display classes.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DisplayOptions>DisplayOptions</linkto>
// </prerequisite>

// <etymology>
// DisplayParameter is a base class which provides services for
// conveniently storing and parsing parameters relevant to the display
// classes.
// </etymology>

// <synopsis>
// DisplayParameter defines a relatively simple interface for writing
// small containers for the various types of parameters used
// throughout the display classes.  The required interface consists of
// methods to update the parameter value from an incoming
// <src>RecordInterface</src>-type object, and to add the description
// of the parameter to a provided <src>RecordInterface</src> object.
//
// Other than this, all parameters share these common elements: 
//
// <li> <src>name</src>, a short <src>String</src> uniquely
// identifiying the parameter.
//
// <li> <src>description</src>, a slightly longer <src>String</src>
// which offers a description of the parameter, suitably short for
// display in a graphical user interface, for example.
//
// <li> <src>help</src>, an even longer <src>String</src> (!) which
// should be filled in with text describing the option in more detail
// than can be given in <src>description</src>
//
// <li> <src>context</src>, which if provided (ie. <src>context !=
// String("")</src>) gives a category name for this parameter, and is
// used, for example, by the autogui to place the parameter in an 
// appropriately named roll-up.
//
// <li> <src>allowunset</src>, which indicates whether this parameter
// can be "unset" or not.  An "unset" parameter is one for which a
// sensible (perhaps context sensitive) default can be determined and
// used.  Derived classes need not provide this option in their
// constructor.
//
// <li> <src>editable</src>, which indicates whether this parameter is
// allowed to be modified by the user.
//
// DisplayParameter makes use of the <linkto
// class=DisplayOptions>DisplayOptions</linkto> class to parse Records
// containing parameter descriptions.  Derived classes must implement
// the two virtual methods <src>toRecord</src> and
// <src>fromRecord</src>, which respectively store a description of
// the DisplayParameter in a sub-field of the provided
// RecordInterface, and extract the value of the DisplayParameter from
// a sub-field in the provided record.  The sub-field is identified by
// the <src>name</src> of the DisplayParameter.
//
// Derived classes should also add utility functions which return the
// various aspects of the DisplayParameter to the programmer.  Most
// importantly, a <src>value()</src> function should be provided to
// enable the user to easily retrieve the current value of the
// parameter.
// </synopsis>

// <motivation> 
// To avoid littering many of the display classes with code fragments
// for constructing and parsing Records.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2000/01/28">
// <li> Provide base class support for unset values.
// </todo>

class DisplayParameter {

 public:

  // Destructor.
  virtual ~DisplayParameter();

  // Parse <src>record</src>, and update this parameter if a field
  // exists whose name matches that of this parameter.  Return
  // <src>True</src> if the parameter is changed, otherwise return
  // <src>False</src>.
  virtual Bool fromRecord(const RecordInterface &record) = 0;

  // Place a record describing this parameter in a sub-field of
  // <src>record</src> with name matching that of this parameter.  If
  // <src>overwrite</src> is <src>True</src>, then any existing field
  // with matching name will be overwritten.  If <src>fullrecord</src>
  // is <src>True</src>, then a complete description of the parameter
  // is given, otherwise just its current value is stored in
  // <src>record</src>.
  virtual void toRecord(RecordInterface &record, const Bool fullrecord = True,
			const Bool overwrite = False) = 0;

  // Return the name of this parameter.
  String name() const 
    { return itsName; }

  // Return the description of this parameter.
  String description() const 
    { return itsDescription; }

  // Return the help for this parameter.
  String help() const 
    { return itsHelp; }

  // Return the context of this parameter.
  String context() const
    { return itsContext; }

  // Return whether this parameter can be unset.
  Bool allowUnset() const
    { return itsAllowUnset; }

  // Return whether this parameter is editable.
  Bool editable() const
    { return itsEditable; }

  // Set or change the name of this parameter to that specified.
  void setName(const String name)
    { itsName = name; }

  // Set or change the description of this parameter to what is
  // specified.
  void setDescription(const String description)
    { itsDescription = description; }

  // Set or change the help for this parameter to what is specified.
  void setHelp(const String help) 
    { itsHelp = help; }

  // Set or change the context of this parameter to what is specified.
  void setContext(const String context) 
    { itsContext = context; }

  // Set or change whether this parameter may be unset, according to
  // the function argument value.
  void setAllowUnset(const Bool allowunset) 
    { itsAllowUnset = allowunset; }

  // Set or change whether this parameter is editable according to
  // the function argument.
  void setEditable(const Bool editable) 
    { itsEditable = editable; }

 protected:

  // Constructor taking the name of the parameter, a short
  // description, some help text, and flags indicating whether the
  // parameter can be unset and is editable.
  DisplayParameter(const String name, const String description,
		   const String help, const String context = "",
		   const Bool allowunset = False, 
		   const Bool editable = True);

  // Copy constructor using copy semantics.
  DisplayParameter(const DisplayParameter &other);

  // Default constructor yielding a useless DisplayParameter.
  DisplayParameter();

  // Copy assignment.
  DisplayParameter &operator=(const DisplayParameter &other);

  // Return a basic description of this parameter; used by virtual
  // implementations of <src>toRecord</src> method to fill out
  // a Record describing this DisplayParameter.
  Record baseDescription();

  // Return the DisplayOptions to use for parsing Records.
  const DisplayOptions &displayOptions() const 
    { return itsDisplayOptions; }

 private:

  // Store for the name of this parameter.
  String itsName;

  // Store for the description of this parameter.
  String itsDescription;

  // Store for the help for this parameter.
  String itsHelp;

  // Store for the context of this parameter.
  String itsContext;

  // Store for whether this parameter can be unset.
  Bool itsAllowUnset;

  // Store for whether this parameter is editable.
  Bool itsEditable;

  // Store for a DisplayOptions object for parsing Records.
  DisplayOptions itsDisplayOptions;

};


} //# NAMESPACE CASA - END

#endif

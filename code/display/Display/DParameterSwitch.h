//# DParameterSwitch.h: store and retrieve Boolean parameters (switches)
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

#ifndef TRIALDISPLAY_DPARAMETERSWITCH_H
#define TRIALDISPLAY_DPARAMETERSWITCH_H

//#include <casa/aips.h>
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
// DParameterSwitch is an implementation of a DisplayParameter
// providing a boolean (switch) parameter type.
// </etymology>

// <synopsis>

// This class is derived from <linkto
// class=DisplayParameter>DisplayParameter</linkto> and provides a
// boolean switch parameter. Parameters can be True or False.
// They cannot be unset.

// </synopsis>

// <example>
// <srcblock>

// DParameterSwitch pswitch("myswitch", "Axis labels",
//                          "Do you want to draw axis labels?",
//                          True,True);
//
// // ...
//
// // update the parameter from some Record
// pswitch.fromRecord(rec);
//
// // examine the value of the parameter
// if (pswitch.value()) {
// // ...
// }
// </srcblock>
// </example>

// <motivation>
// Switches are common.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2002/04/22">
// <li> Nothing known.
// </todo>

	class DParameterSwitch : public DisplayParameter {

	public:

		// Constructor taking the name of the parameter, a short
		// description, some help text, a default value, an initial value,
		// the context of the parameter.
		DParameterSwitch(const String name, const String description,
		                 const String help,
		                 const Bool defaultvalue, const Bool value,
		                 const String context="");

		// (Required) copy constructor.
		DParameterSwitch(const DParameterSwitch& other);

		// Destructor.
		virtual ~DParameterSwitch();

		// (Required) copy assignment.
		DParameterSwitch& operator=(const DParameterSwitch& other);

		// Parse <src>record</src>, and update this parameter.  Return
		// <src>True</src> if the parameter is changed, otherwise return
		// <src>False</src>.
		virtual Bool fromRecord(const RecordInterface& record);

		// Place a record describing this parameter in a sub-field of
		// <src>record</src> with name matching that of this parameter.  If
		// <src>overwrite</src> is <src>True</src>, then any existing field
		// with matching name will be overwritten.  If <src>fullrecord</src>
		// is <src>True</src>, then a complete description of the parameter
		// is given, otherwise just its current value is stored in
		// <src>record</src>.  Presently <src>fullrecord</src> is ignored.
		virtual void toRecord(RecordInterface& record, const Bool fullrecord = True,
		                      const Bool overwrite = False);

		// Return the default for this parameter.
		Bool defaultValue() const {
			return itsDefaultValue;
		}

		// Return the current value of this parameter.
		Bool value() const {
			return itsValue;
		}

		// Set or change the default value for this parameter.
		void setDefaultValue(const Bool defaultvalue) {
			itsDefaultValue = defaultvalue;
		}

		// Set or change the current value.
		void setValue(const Bool value) {
			itsValue = value;
		}

	protected:

		// (Required) default constructor.
		DParameterSwitch();

	private:

		// Store for the default of this parameter.
		Bool itsDefaultValue;

		// Store for the value of this parameter.
		Bool itsValue;

	};


} //# NAMESPACE CASA - END

#endif

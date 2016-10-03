//# DisplayOptions.h: option parsing for display classes
//# Copyright (C) 1999,2000,2002
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

#ifndef DISPLAY_DISPLAYOPTIONS_H
#define DISPLAY_DISPLAYOPTIONS_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class to provide option parsing routines for display classes.
// </summary>

// <use visibility=local>

// <reviewed reviewer="Peter Barnes" date="1999/10/27" tests="tDisplayOptions" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=casacore::Record>Record</linkto>
// </prerequisite>

// <etymology>
// DisplayOptions contains methods for parsing "Options" for
// various "Display" classes.
// </etymology>

// <synopsis>
// DisplayOptions is a simple class which provides methods for parsing
// Records containing fields which conform to "options" as used in the
// Display Library classes.  Consider a record having the following
// structure:
// <srcblock>
// rec.minimum.value = 4.0
// rec.minimum.otherfield = "some text"
// rec.maximum = 8.0
// </srcblock>
// The DisplayOptions methods can be used to extract the values of the
// <src>minimum</src> and <src>maximum</src> fields of the Record
// <src>rec</src>, regardless of whether the value itself is stored in
// a <src>value</src> sub-field, or at the next higher level.  The
// DisplayOptions methods also offer the capability to detect "unset"
// casacore::Record values.
//
// This class can be used as-is, or inherited from by higher-level
// classes.
// </synopsis>

// <example>
// The following example shows the use of DisplayOptions as a
// stand-alone class.
// <srcblock>
// /* assume some casacore::Record "rec" is to be parsed */
// DisplayOptions dopt;
// casacore::Bool error;
//
// casacore::Float min = 0.0;
// if (dopt.readOptionRecord(min, error, rec, "minimum") && !error) {
//   cerr << "minimum found and changed, new value is " << min << endl;
// }
//
// casacore::String color;
// casacore::Bool colorIsUnset = false;
// if (dopt.readOptionRecord(color, colorIsUnset, error,
//                           rec, "color") && !error) {
//   if (colorIsUnset) {
//     cerr << "color found and unset" << endl;
//   } else {
//     cerr << "color found and changed, new value is " << color << endl;
//   }
// }
// </srcblock>
// </example>

// <motivation>
// Options are used prolifically throughout the display classes,
// so some unified processing of options is desirable.
// </motivation>

// <thrown>
// None.
// </thrown>

// <todo asof="1999/08/26">
// <li> add unset support for casacore::Bool, casacore::Float and casacore::Int types
// </todo>

	class DisplayOptions {

	public:

		// Constructor.
		DisplayOptions();

		// Destructor.
		virtual ~DisplayOptions() { };

		// Find the field <src>fieldname</src> in casacore::Record <src>rec</src>,
		// containing the requested type (casacore::Bool, casacore::String, casacore::Float, casacore::Int) in
		// itself or its "value" sub-field, and return the value in
		// <src>target</src>.  If the field doesn't exist, or does not
		// contain the requested type, or a "value" sub-field containing the
		// requested type, then <src>error</src> is set true.  When
		// <src>error</src> is false, the return value indicates whether
		// <src>target</src> was modified.  The casacore::Float version will also read
		// records containing Ints or Doubles, and cast their value to
		// Float.
		// <group>

		template <class T> casacore::Bool readOptionRecord(casacore::Vector<T> &target, casacore::Bool &error,
		        const casacore::Record &rec,
		        const casacore::String &fieldname) const;

		template <class T> casacore::Bool readOptionRecord(T &target, casacore::Bool &error, const casacore::Record &rec,
		        const casacore::String &fieldname) const;
		// </group>

		// Find the field <src>fieldname</src> in casacore::Record <src>rec</src>,
		// containing the requested type (casacore::String) in itself or its "value"
		// sub-field, and return the value in <src>target</src>.  If the
		// field (or "value" sub-field) instead contains an "unset" casacore::Record,
		// then <src>unsetTarget</src> is set true.  If the field doesn't
		// exist, or does not contain the requested type (casacore::String) or an
		// "unset" casacore::Record, or a "value" sub-field containing either of
		// these, then <src>error</src> is set true.  When <src>error</src>
		// is false, the return value indicates whether <src>target</src>
		// (or <src>unsetTarget</src>) was modified.
		virtual casacore::Bool readOptionRecord(casacore::String &target, casacore::Bool &unsetTarget,
		                              casacore::Bool &error, const casacore::Record &rec,
		                              const casacore::String &fieldname) const;

		// Return a casacore::Record which is an "unset" casacore::Record, ie. has a field with
		// name <src>casacore::String("i_am_unset")</src> whose value is
		// <src>casacore::String("i_am_unset")</src>.
		casacore::Record unset() const {
			return itsUnsetRecord;
		}

		// Return true or false indicating if the provided casacore::Record is equal
		// to an "unset" Record.
		casacore::Bool isUnset(const casacore::Record &rec) const;

	protected:

		// (Required) copy constructor.
		DisplayOptions(const DisplayOptions &other);

		// (Required) copy assignment.
		void operator=(const DisplayOptions &other);

	private:
		// A utility to check if two types are compatible. (I.e. double is
		// "compatible" with int. Used by readOptionRecord.
		casacore::Bool compatible(const casacore::DataType &compareme, const casacore::DataType &tome) const;

		// An unset record to return with protected <src>unset()</src>
		// function.
		casacore::Record itsUnsetRecord;

	};


} //# NAMESPACE CASA - END
#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Utilities/DisplayOptionsTemplate.tcc>
#endif
#endif

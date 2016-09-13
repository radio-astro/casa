//# DParameterRange.h: class to store and retrieve range parameters
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

#ifndef TRIALDISPLAY_DPARAMETERRANGE_H
#define TRIALDISPLAY_DPARAMETERRANGE_H

#include <casa/aips.h>
#include <display/Display/DisplayParameter.h>
namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A helper class to deal with data ranges to support options
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> DisplayParameter
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
//
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// Making things easier
// </motivation>
//
// <todo>
// </todo>
//

	template <class T> class DParameterRange : public DisplayParameter {

	public:

		// Constructor taking the name of the parameter, a short
		// description, some help text, a minimum value, a maximum value, a
		// default value, a current value, and flags indicating whether the
		// parameter is editable.
		DParameterRange(const String name, const String description,
		                const String help, const T minimum, const T maximum,
		                const T resolution, const T defaultvalue, const T value,
		                const String context = "", const Bool editable = True,
		                const Bool provideentry = False, const Bool onrelease=False );

		// (Required) default constructor.
		DParameterRange();

		// (Required) copy constructor.
		DParameterRange(const DParameterRange<T> &other);

		// Destructor.
		virtual ~DParameterRange();

		// (Required) copy assignment.
		DParameterRange<T> &operator=(const DParameterRange<T> &other);

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

		// Return the minimum for this parameter.
		T minimum() {
			return itsMinimum;
		}

		// Return the maximum for this parameter.
		T maximum() {
			return itsMaximum;
		}

		// Return the resolution of this parameter.
		T resolution() {
			return itsResolution;
		}

		// Return the default for this parameter.
		T defaultValue() {
			return itsDefault;
		}

		// Return the current value of this parameter.
		T value() {
			return itsValue;
		}

		// Return whether there should be a text box beside the slider.
		// See 'intrange' in the autogui tool documentation for more information.
		Bool provideEntry() {
			return itsProvideEntry;
		}

		// Return whether the slider event should occur when the user releases the
		// slider, i.e. at the end of setting the value, rather than in real time
		// as the user moves the slider (good for operations which take a long time)
		Bool onRelease( ) {
			return itsOnRelease;
		}


		// Set or change the minimum for this parameter.
		void setMinimum(const T minimum) {
			itsMinimum = minimum;
		}

		// Set or change the maximum for this parameter.
		void setMaximum(const T maximum) {
			itsMaximum = maximum;
		}

		// Set or change the resolution for this parameter.
		void setResolution(const T resolution) {
			itsResolution = resolution;
		}

		// Set or change the default for this parameter.
		void setDefaultValue(const T defaultValue) {
			itsDefault = defaultValue;
		}

		// Set or change the current value for this parameter.
		void setValue(const T value) {
			itsValue = value;
		}

		// Convenient syntax to set (only) the value.
		const T& operator=(const T &value) {
			itsValue = value;
			return value;
		}

		// Set or change the provideentry state for this parameter.
		void setProvideEntry(const Bool provideentry) {
			itsProvideEntry = provideentry;
		}

		// Set or change the onrelease state for this parameter.
		void setOnRelease(const Bool onrelease) {
			itsOnRelease = onrelease;
		}

	private:

		// Store for the minimum of this parameter.
		T itsMinimum;

		// Store for the maximum of this parameter.
		T itsMaximum;

		// Store for the resolution of this parameter.
		T itsResolution;

		// Store for the default of this parameter.
		T itsDefault;

		// Store for the value of this parameter.
		T itsValue;

		// Store for the 'provideentry' state of this parameter.
		Bool itsProvideEntry;

		// Store for the 'onrelease' state of this parameter
		Bool itsOnRelease;

	};

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/Display/DParameterRange.tcc>
#endif

#endif

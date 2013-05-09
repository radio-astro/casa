//# DParameterMapKeyChoice.h: class to store/retrieve a parameter and its key
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
//# $Id:

#ifndef TRIALDISPLAY_DPARAMETERMAPKEYCHOICE_H
#define TRIALDISPLAY_DPARAMETERMAPKEYCHOICE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/DParameterChoice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of DParameterChoice to store parameter and
// associated key
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DParameterChoice>DParameterChoice</linkto>
// </prerequisite>

// <etymology>
// DParameterMapKeyChoice is an implementation of a DisplayParameter
// providing a choice parameter type. It also maintains a "key". eg it
// can associate a vector of strings with a vector of integers.
// </etymology>

// <synopsis>
// This class is derived from <linkto
// class=DParameterChoice>DParameterChoice</linkto> and provides a
// choice-type parameter.  The string selection correspond to a "key"
// selection. It is desgined for use with, for example, an enum. In such a
// case, each string value can have an associated integer value which
// can be cast to / from a enum.
// </synopsis>
// <example>

// A DParameterMapKeyChoice can easily be used to store and update any
// parameter which can be expressed as a selection from two or more
// options.
// <srcblock>
// Vector<String> myStrings(2);
//   myStrings(0) = "Ten";
//   myStrings(1) = "Twenty";
// Vector<Int> myInts(2);
//   myInts(0) = 10;
//   myInts(1) = 20;
//
// // ...
// DParameterMapKeyChoice mchoice("number", "Choose a number of ...",
//                          "Select the number of ... to use for ...",
//                          myStrings, myInts,
//                          myStrings(0), myStrings(0));
//
// // ...
//
// // update the parameter from some Record (String representation);
// mchoice.fromRecord(rec);
//
// // examine the value of the parameter
// cerr << "You have selected " << mchoice.value() << " things." << endl;
// // ...
// itsNumberThings = mchoice.keyValue();
// </srcblock>
// </example>

// <motivation>
// Often at C++ level, parameters will be described by enums etc.
// At glish level however, it is often required for these parameters
// to be displayed as text options. This class is designed to make that
// process easier. It can also be used more generally to associate any key
// with a text option.
// </motivation>

// <thrown>
// <li> Throws AipsErrors if number of keys do not match number of options
// or if there is a problem looking up a key.
// </thrown>

// <todo asof="2002/05/08">
// <li> Template the class so a "key" can be anything.
// </todo>

	class DParameterMapKeyChoice : public DParameterChoice {

	public:

		// Constructor taking the name of the parameter, a short
		// description, some help text, the enum which is to form
		// the basis of the options (must have an overloaded ostream <<
		// operator), a vector of integers corresponding to the options
		// within that enum, a default value, an initial value, and the
		// context of the parameter. Obviously the
		// <src>defaultvalue</src> and <src>value</src> parameters must
		// exist in the list of allowed options, otherwise an exception
		// is thrown.
		DParameterMapKeyChoice(const String name, const String description,
		                       const String help, const Vector<String>& options,
		                       const Vector<Int>& keys,
		                       const String defaultvalue, const String value,
		                       const String context = "");


		// (Required) copy constructor.
		DParameterMapKeyChoice(const DParameterMapKeyChoice &other);

		// Destructor.
		virtual ~DParameterMapKeyChoice();

		// (Required) copy assignment.
		DParameterMapKeyChoice &operator=(const DParameterMapKeyChoice &other);

		// Return the list of all keys for this parameter.
		Vector<Int> keys() const {
			return itsKeys;
		}

		// Return the current value of this parameter.
		Int keyValue();

		// Thise function has very little implementation, it is here so if
		// a user changes the list of options, we can ensure we do not make a
		// mistake when looking up its corresponding key
		void setOptions(const Vector<String>& newOptions) {
			itsLastString = "";
			DParameterChoice::setOptions(newOptions);
		}

		// Set the current value, based on a key.
		Bool setKeyValue(const Int newValue);

		// Set or change the list of allowed options for this parameter.
		void setKeys (const Vector<Int>& newKeys);

	protected:

		// (Required) default constructor.
		DParameterMapKeyChoice();

	private:
		// Lookup the key based on the string provided
		Bool lookUpKey(const String& value);

		// Store for the allowed options for this parameter.
		Vector<Int> itsKeys;
		Int itsKeyValue;
		String itsLastString;
	};


} //# NAMESPACE CASA - END

#endif

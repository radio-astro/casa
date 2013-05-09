//# DParameterMapKeyChoice.cc: class to store/retrieve a parameter and its key
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

#include <casa/Exceptions/Error.h>
#include <display/Display/DParameterMapKeyChoice.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DParameterMapKeyChoice::DParameterMapKeyChoice(
	    const String name, const String description,
	    const String help,
	    const Vector<String>& options,
	    const Vector<Int>& keys,
	    const String defaultvalue, const String value,
	    const String context) :
		DParameterChoice(name, description, help, options, defaultvalue,
		                 value, context, False) {

		if (keys.nelements() != options.nelements())
			throw(AipsError("DParameterMapKeyChoice error - length of options must equal length of keys"));

		// Save the list of keys
		itsKeys = keys;

		// save the current value
		itsLastString = value;

		// Look up the key for the first value
		if (!lookUpKey(value))
			throw(AipsError("DParameterMapKeyChoice error - Couldn't find the initial value in the list of options"));
	}

	DParameterMapKeyChoice::DParameterMapKeyChoice(
	    const DParameterMapKeyChoice &other) :
		DParameterChoice(other),
		itsKeys(other.itsKeys),
		itsKeyValue(other.itsKeyValue),
		itsLastString(other.itsLastString) {
	}

	DParameterMapKeyChoice::~DParameterMapKeyChoice() {

	}

	DParameterMapKeyChoice::DParameterMapKeyChoice() :
		DParameterChoice() {

	}

	DParameterMapKeyChoice &DParameterMapKeyChoice::operator=(
	    const DParameterMapKeyChoice& other)

	{
		if(this != &other) {
			DParameterChoice::operator=(other);
			itsKeys = other.itsKeys;
			itsLastString = other.itsLastString;
			itsKeyValue = other.itsKeyValue;
		}
		return *this;

	}

	Bool DParameterMapKeyChoice::lookUpKey(const String& value) {
		uInt numberSearched=0;
		Bool found=False;

		Vector<String> currentOptions(options());

		if (itsKeys.nelements() != currentOptions.nelements())
			throw(AipsError("DParameterMapKeyChoice - The number of options does not match the number of keys."));

		while(numberSearched < itsKeys.nelements() && !found) {
			if (currentOptions[numberSearched] == value)
				found = True;
			else
				numberSearched++;
		}

		if (!found) return False;
		else {
			itsKeyValue = itsKeys[numberSearched];
			itsLastString = value;
		}

		return True;
	}

	Int DParameterMapKeyChoice::keyValue() {

		// Assume if no change we are still valid
		if (value() == itsLastString) return itsKeyValue;
		else if (!lookUpKey(value()))
			throw(AipsError("DParameterMapKeyChoice error - Couldn't get key value"));

		return itsKeyValue;
	}

	void DParameterMapKeyChoice::setKeys(const Vector<Int>& newKeys) {
		if (newKeys.nelements() != options().nelements())
			throw(AipsError("DParameterMapKeyChoice error - Bad number of keys"));

		itsKeys = newKeys;
		itsLastString = "";

	}

	Bool DParameterMapKeyChoice::setKeyValue(const Int newValue) {
		uInt numberSearched=0;
		Bool found=False;

		if (itsKeys.nelements() != options().nelements())
			throw(AipsError("DParameterMapKeyChoice - The number of options does not match the number of keys."));

		while(numberSearched < itsKeys.nelements() && !found) {
			if (itsKeys[numberSearched] == newValue)
				found = True;
			else
				numberSearched++;
		}
		if (!found) return False;

		Vector<String> currentOptions(options());
		setValue(currentOptions[numberSearched]);

		return True;
	}





} //# NAMESPACE CASA - END


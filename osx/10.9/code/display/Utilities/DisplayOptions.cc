//# DisplayOptions.cc: option parsing for display classes
//# Copyright (C) 1999,2000,2001,2002
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

#include <casa/aips.h>
#include <casa/Containers/Record.h>

#include <display/Utilities/DisplayOptions.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	DisplayOptions::DisplayOptions() {
		itsUnsetRecord.define("i_am_unset", "i_am_unset");
	}

	Bool DisplayOptions::compatible(const DataType &compareme,
	                                const DataType &tome) const {
		if(compareme>=TpRecord) return False;
		if(compareme==tome) return True;
		if (compareme==TpFloat || compareme==TpDouble)
			return (tome == TpDouble || tome == TpInt || tome ==TpFloat);
		if (compareme==TpArrayFloat || compareme==TpArrayDouble)
			return (tome==TpArrayFloat || tome == TpArrayInt || tome==TpArrayDouble);
		return False;
	}

	Bool DisplayOptions::readOptionRecord(String &target, Bool &unsetTarget,
	                                      Bool &error,
	                                      const Record &rec,
	                                      const String &fieldname) const {
		Bool result = readOptionRecord(target, error, rec, fieldname);
		if (!error) {
			result = (result || unsetTarget);
			unsetTarget = False;
			return result;
		}
		static String i_am_unset("i_am_unset");
		// if we are here, we ought to look for an unset now...
		if (!rec.isDefined(fieldname)) {
			error = True;
			return False;
		}
		if (rec.dataType(fieldname) != TpRecord) {
			error = True;
			return False;
		}
		Record subrec = rec.subRecord(fieldname);
		String subfield("");
		if (subrec.isDefined(i_am_unset)) {
			subfield = i_am_unset;
		} else if (subrec.isDefined("value")) {
			Record trec = subrec.subRecord("value");
			if (trec.isDefined(i_am_unset)) {
				subrec = trec;
				subfield = i_am_unset;
			}
		}
		if (subfield != i_am_unset) {
			error = True;
			return False;
		}
		if (subrec.dataType(subfield) != TpString) {
			error = True;
			return False;
		}
		String temp;
		subrec.get(subfield, temp);
		if (temp != i_am_unset) {
			error = True;
			return False;
		}

		Bool ret = (!unsetTarget);
		unsetTarget = True;
		error = False;
		return ret;
	}

	Bool DisplayOptions::isUnset(const Record &rec) const {
		static String i_am_unset("i_am_unset");
		if (rec.isDefined(i_am_unset)) {
			if (rec.dataType(i_am_unset) == TpString) {
				String temp;
				rec.get(i_am_unset, temp);
				if (temp == i_am_unset) {
					return True;
				}
			}
		}
		return False;
	}

	DisplayOptions::DisplayOptions(const DisplayOptions &) {
	}

	void DisplayOptions::operator=(const DisplayOptions &) {
	}

} //# NAMESPACE CASA - END


//# DisplayOptions.cc: option parsing for display classes
//# Copyright (C) 2002
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

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/LogiArray.h>
#include <display/Utilities/DisplayOptions.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T>
	casacore::Bool DisplayOptions::readOptionRecord(T &target, casacore::Bool &error,
	                                      const casacore::Record &rec,
	                                      const casacore::String &fieldname) const {
		// check that the specified field exists in the supplied record
		if (!rec.isDefined(fieldname)) {
			error = true;
			return false;
		}

		casacore::Record subrec;
		casacore::String subfield;
		casacore::DataType targ = casacore::whatType(&target);
		casacore::DataType field = rec.dataType(fieldname);

		if (compatible(targ, field)) {
			// the record type is the same as that of the target,
			// so the value is stored at the top-level
			subrec = rec;
			subfield = fieldname;

		} else if (rec.dataType(fieldname) ==casacore:: TpRecord) {
			// the record type is casacore::Record, so see if the value is stored in a sub-record
			subrec = rec.subRecord(fieldname);
			subfield = "value";
			// see if there is a value sub-record
			if (!subrec.isDefined(subfield)) {
				error = true;
				return false;
			}
			// there is, so check that its type is of the correct type.
			field = subrec.dataType(subfield);
			if (!compatible(targ , field)) {
				error = true;
				return false;

			}
		} else {
			// wrong record type
			error = true;
			return false;

		}
		// now subrec is the record containing a field called fieldname which
		// actually contains the required value --- extract the value

		T temp;
		//  casacore::DataType toGet = whatType(&temp);
		casacore::Bool changed = false;

		subrec.get(subfield, temp);

		changed = (target != temp);

		target = temp;
		error = false;
		return changed;

	}

	template <class T>
	casacore::Bool DisplayOptions::readOptionRecord(casacore::Vector<T> &target, casacore::Bool &error,
	                                      const casacore::Record &rec,
	                                      const casacore::String &fieldname) const {

		// check that the specified field exists in the supplied record
		if (!rec.isDefined(fieldname)) {
			error = true;
			return false;
		}

		casacore::Record subrec;
		casacore::String subfield;
		casacore::DataType targ = whatType(&target);
		casacore::DataType field = rec.dataType(fieldname);

		if (compatible(targ, field)) {
			// the record type is the same as that of the target,
			// so the value is stored at the top-level
			subrec = rec;
			subfield = fieldname;

		} else if (rec.dataType(fieldname) == casacore::TpRecord) {
			// the record type is casacore::Record, so see if the value is stored in a sub-record
			subrec = rec.subRecord(fieldname);
			subfield = "value";
			// see if there is a value sub-record
			if (!subrec.isDefined(subfield)) {
				error = true;
				return false;
			}
			// there is, so check that its type is of the correct type.
			field = subrec.dataType(subfield);

			if (!compatible(targ , field)) {
				error = true;
				return false;

			}
		} else {
			// wrong record type
			error = true;
			return false;

		}
		// now subrec is the record containing a field called fieldname which
		// actually contains the required value --- extract the value

		casacore::Vector<T> temp;
		casacore::Bool changed = false;

		subrec.get(subfield, temp);

		// Check shape of Vectors match
		int targetLength;
		int recordLength;

		target.shape(targetLength);
		temp.shape(recordLength);

		if (targetLength != recordLength) {
			changed = true;
			target.resize(recordLength);
		} else changed = !allEQ(target, temp);

		target = temp;
		error = false;
		return changed;

	}

} //# NAMESPACE CASA - END


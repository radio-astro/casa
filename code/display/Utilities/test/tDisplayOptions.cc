//# tDisplayOptions.cc: test program for DisplayOptions class
//# Copyright (C) 1999,2002
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
#include <casa/Arrays/Vector.h>
#include <display/Utilities/DisplayOptions.h>
#include <casa/iostream.h>

#include <casa/namespace.h>
void localAbort(String message) {
	cout << message << endl;
	exit(1);
}

int main(int, char **) {

	DisplayOptions dopt;
	Record rec;

	String bFieldname("boolField");
	String sFieldname("stringField");
	String fFieldname("floatField");
	String iFieldname("intField");
	String vFFieldname("vecFloatField");
	String vSub("value");

	Bool bTarget = True;
	String sTarget = "red";
	Bool sUnsetTarget, soUnsetTarget;
	Float fTarget = 4.2;
	Int iTarget = -3;
	Vector<Float> vfTarget(2);
	vfTarget(0) = 32.1234;
	vfTarget(1) = 54.2131;

	Bool boTarget;
	String soTarget;
	Float foTarget;
	Int ioTarget;
	Vector<Float> vfoTarget;

	Record sRecord;

	Bool error = False;

	// ------------------------------------------------------------
	// Test return values of unset(), isUnset() functions
	sRecord = dopt.unset();
	String i_am_unset("i_am_unset");
	if (!sRecord.isDefined(i_am_unset)) {
		localAbort("unset() returns incorrect Record");
	} else {
		String temp;
		sRecord.get(i_am_unset, temp);
		if (temp != i_am_unset) {
			localAbort("unset() returns incorrect Record");
		}
	}
	if (!dopt.isUnset(dopt.unset())) {
		localAbort("incompatibility between unset() and isUnset()");
	}
	sRecord.removeField(i_am_unset);
	if (dopt.isUnset(sRecord)) {
		localAbort("incorrect return value of isUnset()");
	}

	// ------------------------------------------------------------
	// Test reading of non-existent fields

	// attempt to read absent Bool field
	if (dopt.readOptionRecord(bTarget, error, rec, bFieldname)) {
		localAbort("inconsistent return value for Bool read");
	} else {
		if (!error) {
			localAbort("incorrect value of error for Bool read");
		}
	}

	// attempt to read absent Vector<Float> field
	if (dopt.readOptionRecord(vfTarget, error, rec, vFFieldname)) {
		localAbort("inconsistent return value for Vector<Float> read");
	} else {
		if (!error) {
			localAbort("incorrect value of error for Vector<Float> read");
		}
	}



	// attempt to read absent String field
	if (dopt.readOptionRecord(sTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for String read");
	} else {
		if (!error) {
			localAbort("incorrect value of error for String read");
		}
	}

	// attempt to read absent Float field
	if (dopt.readOptionRecord(fTarget, error, rec, fFieldname)) {
		localAbort("inconsistent return value for Float read");
	} else {
		if (!error) {
			localAbort("incorrect value of error for Float read");
		}
	}

	// attempt to read absent Int field
	if (dopt.readOptionRecord(iTarget, error, rec, iFieldname)) {
		localAbort("inconsistent return value for Int read");
	} else {
		if (!error) {
			localAbort("incorrect value of error for Int read");
		}
	}

	// ------------------------------------------------------------
	// Test reading of existing fields, with values the same as
	// already set in the target variables

	// attempt to read Bool field matching existing value
	rec.define(bFieldname, bTarget);
	boTarget = bTarget;
	if (dopt.readOptionRecord(bTarget, error, rec, bFieldname)) {
		localAbort("inconsistent return value for Bool read");
	} else {
		if (error) {
			localAbort("unexpected error in Bool read");
		}
	}
	if (bTarget != boTarget) {
		localAbort("unexpected result of Bool read");
	}
	rec.removeField(bFieldname);


	// attempt to read Vector<Float> field matching existing value
	rec.define(vFFieldname, vfTarget);
	vfoTarget = vfTarget;
	if (dopt.readOptionRecord(vfTarget, error, rec, vFFieldname)) {
		localAbort("inconsistent return value for Vector<Float> read");
	} else {
		if (error) {
			localAbort("unexpected error in Vector<Float> read");
		}
	}
	if (!allEQ(vfoTarget,vfTarget)) {
		localAbort("unexpected result of Vector<Float> read");
	}
	rec.removeField(vFFieldname);


	// attempt to read String field matching existing value
	rec.define(sFieldname, sTarget);
	soTarget = sTarget;
	if (dopt.readOptionRecord(sTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for String read");
	} else {
		if (error) {
			localAbort("unexpected error in String read");
		}
	}
	if (sTarget != soTarget) {
		localAbort("unexpected result of String read");
	}
	rec.removeField(sFieldname);

	// attempt to read Float field matching existing value
	rec.define(fFieldname, fTarget);
	foTarget = fTarget;
	if (dopt.readOptionRecord(fTarget, error, rec, fFieldname)) {
		localAbort("inconsistent return value for Float read");
	} else {
		if (error) {
			localAbort("unexpected error in Float read");
		}
	}
	if (fTarget != foTarget) {
		localAbort("unexpected result of Float read");
	}
	rec.removeField(fFieldname);

	// attempt to read Int field matching existing value
	rec.define(iFieldname, iTarget);
	ioTarget = iTarget;
	if (dopt.readOptionRecord(iTarget, error, rec, iFieldname)) {
		localAbort("inconsistent return value for Int read");
	} else {
		if (error) {
			localAbort("unexpected error in Int read");
		}
	}
	if (iTarget != ioTarget) {
		localAbort("unexpected result of Int read");
	}
	rec.removeField(iFieldname);

	// ------------------------------------------------------------
	// Test reading of existing fields, with values stored in the
	// "value" sub-fields, and the same as existing variable values

	// attempt to read Bool sub-field matching existing value
	sRecord.define(vSub, bTarget);
	rec.defineRecord(bFieldname, sRecord);
	boTarget = bTarget;
	if (dopt.readOptionRecord(bTarget, error, rec, bFieldname)) {
		localAbort("inconsistent return value for Bool read");
	} else {
		if (error) {
			localAbort("unexpected error in Bool read");
		}
	}
	if (bTarget != boTarget) {
		localAbort("unexpected result of Bool read");
	}
	sRecord.removeField(vSub);
	rec.removeField(bFieldname);

	// attempt to read Vector<Float> sub-field matching existing value
	sRecord.define(vSub, vfTarget);
	rec.defineRecord(vFFieldname, sRecord);
	vfoTarget = vfTarget;

	if (dopt.readOptionRecord(vfTarget, error, rec, vFFieldname)) {
		localAbort("inconsistent return value for Vector<Float> read");
	} else {
		if (error) {
			localAbort("unexpected error in Vector<Float> read");
		}
	}
	if (!allEQ(vfTarget,vfoTarget)) {
		localAbort("unexpected result of Vector<Float> read");
	}
	sRecord.removeField(vSub);
	rec.removeField(vFFieldname);




	// attempt to read String sub-field matching existing value
	sRecord.define(vSub, sTarget);
	rec.defineRecord(sFieldname, sRecord);
	soTarget = sTarget;
	if (dopt.readOptionRecord(sTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for String read");
	} else {
		if (error) {
			localAbort("unexpected error in String read");
		}
	}
	if (sTarget != soTarget) {
		localAbort("unexpected result of String read");
	}
	sRecord.removeField(vSub);
	rec.removeField(sFieldname);

	// attempt to read Float sub-field matching existing value
	sRecord.define(vSub, fTarget);
	rec.defineRecord(fFieldname, sRecord);
	foTarget = fTarget;
	if (dopt.readOptionRecord(fTarget, error, rec, fFieldname)) {
		localAbort("inconsistent return value for Float read");
	} else {
		if (error) {
			localAbort("unexpected error in Float read");
		}
	}
	if (fTarget != foTarget) {
		localAbort("unexpected result of Float read");
	}
	sRecord.removeField(vSub);
	rec.removeField(fFieldname);

	// attempt to read Int sub-field matching existing value
	sRecord.define(vSub, iTarget);
	rec.defineRecord(iFieldname, sRecord);
	ioTarget = iTarget;
	if (dopt.readOptionRecord(iTarget, error, rec, iFieldname)) {
		localAbort("inconsistent return value for Int read");
	} else {
		if (error) {
			localAbort("unexpected error in Int read");
		}
	}
	if (iTarget != ioTarget) {
		localAbort("unexpected result of Int read");
	}
	sRecord.removeField(vSub);
	rec.removeField(iFieldname);

	// ------------------------------------------------------------
	// Test reading of existing fields, with values stored in the
	// "value" sub-fields, and different to existing variable values

	// attempt to read Bool sub-field matching existing value
	sRecord.define(vSub, !bTarget);
	rec.defineRecord(bFieldname, sRecord);
	boTarget = bTarget;
	if (!dopt.readOptionRecord(bTarget, error, rec, bFieldname)) {
		localAbort("inconsistent return value for Bool read");
	} else {
		if (error) {
			localAbort("unexpected error in Bool read");
		}
	}
	if (bTarget == boTarget) {
		localAbort("unexpected result of Bool read");
	}
	sRecord.removeField(vSub);
	rec.removeField(bFieldname);

	// attempt to read Vector<Float> String sub-field matching existing value
	Vector<Float> dif(2);
	dif(0) = 231.123;
	dif(1) = 213.123;
	sRecord.define(vSub, dif);
	rec.defineRecord(vFFieldname, sRecord);
	vfoTarget = vfTarget;
	if (!dopt.readOptionRecord(vfTarget, error, rec, vFFieldname)) {
		localAbort("inconsistent return value for Vector<Float> read");
	} else {
		if (error) {
			localAbort("unexpected error in Vector<Float> read");
		}
	}
	if (allEQ(vfTarget,vfoTarget)) {
		localAbort("unexpected result of Vector<Float> read");
	}
	sRecord.removeField(vSub);
	rec.removeField(vFFieldname);


	// attempt to read String sub-field matching existing value
	sRecord.define(vSub, sTarget + "a");
	rec.defineRecord(sFieldname, sRecord);
	soTarget = sTarget;
	if (!dopt.readOptionRecord(sTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for String read");
	} else {
		if (error) {
			localAbort("unexpected error in String read");
		}
	}
	if (sTarget == soTarget) {
		localAbort("unexpected result of String read");
	}
	sRecord.removeField(vSub);
	rec.removeField(sFieldname);

	// attempt to read Float sub-field matching existing value
	sRecord.define(vSub, fTarget + 4.0);
	rec.defineRecord(fFieldname, sRecord);
	foTarget = fTarget;
	if (!dopt.readOptionRecord(fTarget, error, rec, fFieldname)) {
		localAbort("inconsistent return value for Float read");
	} else {
		if (error) {
			localAbort("unexpected error in Float read");
		}
	}
	if (fTarget == foTarget) {
		localAbort("unexpected result of Float read");
	}
	sRecord.removeField(vSub);
	rec.removeField(fFieldname);

	// attempt to read Int sub-field matching existing value
	sRecord.define(vSub, iTarget + 2);
	rec.defineRecord(iFieldname, sRecord);
	ioTarget = iTarget;
	if (!dopt.readOptionRecord(iTarget, error, rec, iFieldname)) {
		localAbort("inconsistent return value for Int read");
	} else {
		if (error) {
			localAbort("unexpected error in Int read");
		}
	}
	if (iTarget == ioTarget) {
		localAbort("unexpected result of Int read");
	}
	sRecord.removeField(vSub);
	rec.removeField(iFieldname);

	// ------------------------------------------------------------
	// Attempt to read fields of wrong type

	// attempt to read Bool from Record subfield
	rec.defineRecord(bFieldname, dopt.unset());
	boTarget = bTarget;
	if (dopt.readOptionRecord(bTarget, error, rec, bFieldname)) {
		localAbort("inconsistent return value for Bool read");
	} else {
		if (!error) {
			localAbort("unexpected error in Bool read");
		}
	}
	if (bTarget != boTarget) {
		localAbort("unexpected result of Bool read");
	}
	rec.removeField(bFieldname);

// attempt to read Vector<Float> from Record subfield
	rec.defineRecord(vFFieldname, dopt.unset());
	vfoTarget = vfTarget;
	if (dopt.readOptionRecord(vfTarget, error, rec, vFFieldname)) {
		localAbort("inconsistent return value for Vector<Float> read");
	} else {
		if (!error) {
			localAbort("unexpected error in Vector<Float> read");
		}
	}
	if (!allEQ(vfTarget,vfoTarget)) {
		localAbort("unexpected result of Vector<Float> read");
	}
	rec.removeField(vFFieldname);



	// attempt to read String from Record subfield
	rec.defineRecord(sFieldname, dopt.unset());
	soTarget = sTarget;
	if (dopt.readOptionRecord(sTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for String read");
	} else {
		if (!error) {
			localAbort("unexpected error in String read");
		}
	}
	if (sTarget != soTarget) {
		localAbort("unexpected result of String read");
	}
	rec.removeField(sFieldname);

	// attempt to read Float from Record subfield
	rec.defineRecord(fFieldname, dopt.unset());
	foTarget = fTarget;
	if (dopt.readOptionRecord(fTarget, error, rec, fFieldname)) {
		localAbort("inconsistent return value for Float read");
	} else {
		if (!error) {
			localAbort("unexpected error in Float read");
		}
	}
	if (fTarget != foTarget) {
		localAbort("unexpected result of Float read");
	}
	rec.removeField(fFieldname);

	// attempt to read Int from Record subfield
	rec.defineRecord(iFieldname, dopt.unset());
	ioTarget = iTarget;
	if (dopt.readOptionRecord(iTarget, error, rec, iFieldname)) {
		localAbort("inconsistent return value for Int read");
	} else {
		if (!error) {
			localAbort("unexpected error in Int read");
		}
	}
	if (iTarget != ioTarget) {
		localAbort("unexpected result of Int read");
	}
	rec.removeField(iFieldname);

	// ------------------------------------------------------------
	// Test unset behaviour for reading Strings

	// attempt to read absent String field
	if (dopt.readOptionRecord(sTarget, sUnsetTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for (with unset) String read");
	} else {
		if (!error) {
			localAbort("incorrect value of error for (with unset) String read");
		}
	}

	// attempt to read String field matching existing value, not unset.
	rec.define(sFieldname, sTarget);
	soTarget = sTarget;
	sUnsetTarget = False;
	soUnsetTarget = sUnsetTarget;
	if (dopt.readOptionRecord(sTarget, sUnsetTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for (with unset) String read");
	} else {
		if (error) {
			localAbort("unexpected error in (with unset) String read");
		}
	}
	if ((sTarget != soTarget) || (sUnsetTarget != soUnsetTarget)) {
		localAbort("unexpected result of (with unset) String read");
	}
	rec.removeField(sFieldname);

	// attempt to read String field not matching existing value, not unset.
	rec.define(sFieldname, sTarget + "a");
	soTarget = sTarget;
	sUnsetTarget = False;
	soUnsetTarget = sUnsetTarget;
	if (!dopt.readOptionRecord(sTarget, sUnsetTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for (with unset) String read");
	} else {
		if (error) {
			localAbort("unexpected error in (with unset) String read");
		}
	}
	if ((sTarget == soTarget) || (sUnsetTarget != soUnsetTarget)) {
		localAbort("unexpected result of (with unset) String read");
	}
	rec.removeField(sFieldname);

	// attempt to read unset string, not presently unset.
	rec.defineRecord(sFieldname, dopt.unset());
	soTarget = sTarget;
	sUnsetTarget = False;
	soUnsetTarget = sUnsetTarget;
	if (!dopt.readOptionRecord(sTarget, sUnsetTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for (with unset) String read");
	} else {
		if (error) {
			localAbort("unexpected error in (with unset) String read");
		}
	}
	if ((sTarget != soTarget) || (sUnsetTarget == soUnsetTarget)) {
		localAbort("unexpected result of (with unset) String read");
	}
	rec.removeField(sFieldname);

	// attempt to read unset string, presently unset.
	rec.defineRecord(sFieldname, dopt.unset());
	soTarget = sTarget;
	sUnsetTarget = True;
	soUnsetTarget = sUnsetTarget;
	if (dopt.readOptionRecord(sTarget, sUnsetTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for (with unset) String read");
	} else {
		if (error) {
			localAbort("unexpected error in (with unset) String read");
		}
	}
	if ((sTarget != soTarget) || (sUnsetTarget != soUnsetTarget)) {
		localAbort("unexpected result of (with unset) String read");
	}
	rec.removeField(sFieldname);

	// attempt to read unset string in sub-field, presently unset.
	sRecord.defineRecord(vSub, dopt.unset());
	rec.defineRecord(sFieldname, sRecord);
	soTarget = sTarget;
	sUnsetTarget = True;
	soUnsetTarget = sUnsetTarget;
	if (dopt.readOptionRecord(sTarget, sUnsetTarget, error, rec, sFieldname)) {
		localAbort("inconsistent return value for (with unset) String read");
	} else {
		if (error) {
			localAbort("unexpected error in (with unset) String read");
		}
	}
	if ((sTarget != soTarget) || (sUnsetTarget != soUnsetTarget)) {
		localAbort("unexpected result of (with unset) String read");
	}
	rec.removeField(sFieldname);
	sRecord.removeField(vSub);

	// ------------------------------------------------------------

	cout << "OK" << endl;
	return 0;
}





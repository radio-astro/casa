//# SDTabRecCopier.cc : copies a field with units between a Table and SDRecord
//# Copyright (C) 2000
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

//# Includes

#include <dish/SDIterators/SDTabRecCopier.h>

#include <casa/Containers/Record.h>
#include <casa/BasicMath/Math.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/UnitMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SDTabRecCopier::SDTabRecCopier() 
    : ok_p(False)
{;}

SDTabRecCopier::SDTabRecCopier(const String &columnName, const Table &tab,
			       const String &fieldName, Record &rec,
			       const String &fieldUnits, Bool readonly)
    : ok_p(False)
{ 
    ok_p = init(columnName, tab, fieldName, rec, fieldUnits, readonly);
}

SDTabRecCopier::~SDTabRecCopier() {;}

SDTabRecCopier &SDTabRecCopier::operator=(const SDTabRecCopier &other)
{
    if (this != &other) {
	ok_p = other.ok_p;
	readOnly_p = other.readOnly_p;
	unitsAreIdentical_p = other.unitsAreIdentical_p;
	colName_p = other.colName_p;
	fieldName_p = other.fieldName_p;
	tableUnits_p = other.tableUnits_p;
	fieldUnits_p = other.fieldUnits_p;
	recField_p = other.recField_p;
	roTabCol_p.reference(other.roTabCol_p);
	rwTabCol_p.reference(other.rwTabCol_p);
	tabToFieldFact_p = other.tabToFieldFact_p;
    }
    return *this;
}

Bool SDTabRecCopier::init(const String &columnName, const Table &tab,
			  const String &fieldName, Record &rec,
			  const String &fieldUnits, Bool readonly)
{
    // turn on the FITS unit map
    UnitMap::addFITS();

    readOnly_p = readonly;
    ok_p = tab.tableDesc().isColumn(columnName) && tab.tableDesc().columnDesc(columnName).isScalar();
    if (ok_p) {
	colName_p = columnName;
	roTabCol_p.attach(tab, columnName);
	if (!readOnly()) {
	    rwTabCol_p.attach(tab, columnName);
	}
    }

    ok_p = ok_p && (rec.fieldNumber(fieldName) >= 0) && (rec.dataType(fieldName) == TpDouble);
    if (ok_p) {
	fieldName_p = fieldName;
	recField_p.attachToRecord(rec, fieldName);
	// SDRecords should always be using known units - don't check here
	fieldUnits_p = fieldUnits;
	
    }

    if (ok_p) {
	String testUnits;
	if (roTabCol_p.keywordSet().fieldNumber("UNIT") >= 0) {
	    testUnits = roTabCol_p.keywordSet().asString("UNIT");
	    if (tableUnits_p == "") testUnits = fieldUnits;
	} else {
	    testUnits = fieldUnits;
	}

	// is this unit string known
	UnitName name;
	if (!UnitMap::getUnit(testUnits, name)) {
	    // units are unknown - add them in to the user definition as an unknown pixel-like unit
	    UnitMap::putUser(testUnits, UnitVal(), "Undefined units as found in SDFITS table");
	}

	tableUnits_p = testUnits;

	if (tableUnits() == SDTabRecCopier::fieldUnits()) {
	    unitsAreIdentical_p = True;
	    tabToFieldFact_p = 1.0;
	} else {
	    unitsAreIdentical_p = False;
	    Quantity tabVal(1.0,tableUnits_p);
	    // are they compatible
	    ok_p = tabVal.check(fieldUnits_p.getValue());
	    // get the conversion factor
	    if (ok_p) {
		tabToFieldFact_p = tabVal.get(fieldUnits_p).getValue();
		// see if they ultimately are really identical for our purposes
		unitsAreIdentical_p = near(tabToFieldFact_p, 1.0);
	    } else {
		tabToFieldFact_p = 0.0;
	    }
	}
    }

    return ok_p;
}

Bool SDTabRecCopier::copyToField(uInt rownr) 
{
    if (ok_p) 
	*recField_p = unitsAreIdentical_p ? 
	    roTabCol_p.asdouble(rownr) : roTabCol_p.asdouble(rownr)*tabToFieldFact_p;
    return ok_p;
}

Bool SDTabRecCopier::copyFromField(uInt rownr)
{
    if (ok_p && !readOnly()) 
	unitsAreIdentical_p ?
	    rwTabCol_p.putScalar(rownr, *recField_p) : rwTabCol_p.putScalar(rownr, (*recField_p)*tabToFieldFact_p);
    return (ok_p && !readOnly());
}

} //# NAMESPACE CASA - END


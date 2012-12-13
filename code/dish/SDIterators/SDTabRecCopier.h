//# SDTabRecCopier.h : copies a field with units between a Table and SDRecord
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
//#
//# $Id$

#ifndef DISH_SDTABRECCOPIER_H
#define DISH_SDTABRECCOPIER_H

#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableColumn.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward declarations
class Table;
class Record;

// <summary>
// SDTabRecCopier copies a scalar field with units between an SDFITS table and an SDRecord
// </summary>

// <use visibility=local> 

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// A few comments on rules before I flesh out this document.  All SDFITS table columns
// have fixed type and units.  The value is held in an SDRecord with fixed units,
// which may be different fromthose in the table.  The copy operations return False
// and do not copy any values if there is a problem making the Unit conversion.
// The table column must be a scalar column.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//

class SDTabRecCopier
{
public:
    // empty, not attached to anything, copy operations always return False
    SDTabRecCopier();

    // attach to the column name in a table and the field name in the record
    // making the copier read-only (table to field) or writable (both directions)
    // The named column must exist in tab.  The named field must exist as
    // TpDouble of rec.  The fieldUnit are the desired unit of the SDRecord
    // field.  If the table has no UNITS keyword for this column, it is assumed to
    // already be in the correct units.
    SDTabRecCopier(const String &columnName, const Table &tab,
		   const String &fieldName, Record &rec,
		   const String &fieldUnit, Bool readOnly);
    
    // construct from another
    SDTabRecCopier(const SDTabRecCopier &other);

    ~SDTabRecCopier();

    // assignment operator
    SDTabRecCopier &operator=(const SDTabRecCopier &other);

    // re-initialize everything.  Useful if the default constructor was used
    // or if the contents of the record or table change.  Returns False if
    // the named column does not exist, if the named field does not exist
    // or if the named field is not a TpRecord.
    Bool init(const String &columnName, const Table &tab,
	      const String &fieldName, Record &rec,
	      const String &fieldUnits,  Bool readOnly);

    // Copy the table value at rownr to the field this is attached to.
    // Returns False when there is a problem converting the units.
    // There is no checking on the validity of the row
    // number or if this object is attached.
    Bool copyToField(uInt rownr);

    // Copy the value from the field this is attached to and store the result in 
    // the table at rownr in the units found in the table.  Returns False if the value
    // can not be converted to the units found in the table.  Also returns False if this
    // copier is read-only.  There is no checking on the validity of the row number or if this
    // object is attached.
    Bool copyFromField(uInt rownr);

    // is this attached
    Bool isAttached() {return ok_p;}

    // The column name
    const String &columnName() { return colName_p;}

    // The field name
    const String &fieldName() { return fieldName_p;}

    // is it read-only?
    Bool readOnly() {return readOnly_p;}

    // The units of the table column
    const String &tableUnits() { return tableUnits_p.getName();}

    // The units of the field
    const String &fieldUnits() { return fieldUnits_p.getName();}

    // return the value of the field
    const Double &getRecord() {return *recField_p;}

private:
    Bool ok_p, readOnly_p, unitsAreIdentical_p;

    String colName_p, fieldName_p;

    Unit tableUnits_p, fieldUnits_p;

    RecordFieldPtr<Double> recField_p;
    ROTableColumn roTabCol_p;
    TableColumn rwTabCol_p;

    // field = table * tabToFieldFact_p
    Double tabToFieldFact_p;
};


} //# NAMESPACE CASA - END

#endif

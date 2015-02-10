//# GBTSimpleTable.h: a GBTSimpleTable is used to concatenate simple GBT tables
//# Copyright (C) 1999,2000,2001
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

#ifndef NRAO_GBTSIMPLETABLE_H
#define NRAO_GBTSIMPLETABLE_H


#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class TableRow;
class RecordInterface;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// Used to concatenate simple GBT tables.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=GBTBackendFiller>GBTBackendFiller</linkto>
// </prerequisite>
//
// <etymology>
// This contains a Table to which other Tables can be appended.
// Its quite simple and it it used by the GBTBackendFillers.
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// The GBT backend FITS files generally have ancilliar tables such
// as PHASE and RECEIVER.  In the resulting MS, these are just
// appended to the end of a subtable.  This class contains those
// subtables and does the appending.
//
// With MS 2, this is somewhat more complicated and hence this class
// probably needs a better name.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> Is there a mechanism in the Table system which essentially
//        already does this? 
// </todo>

class GBTSimpleTable
{
public:
    // open an already existing table, use the optional index column
    GBTSimpleTable(const String &tableName,
		   const String &indexColumn=String(""));

    // create a new, empty table attached to the given table at the
    // given keyword name, supply an optional index column
    GBTSimpleTable(Table &attachTable, 
		   const String &attachKeywordName,
		   const String &tableName,
		   const String &indexColumn=String(""));

    ~GBTSimpleTable();

    // append the given table to the end of this table
    // adding columns as required, any columns with the
    // same name must have the same type and dimensionality
    void add(const Table &other);

    // append this record to the end of this table.
    // Each field in the record maps to a column in the
    // the table.  There must be no subrecords.  The types
    // of the fields must match that in the existing table.
    // Fields not already in the table will result in new
    // columns for that table
    void add(const RecordInterface &other, Bool nextIndex=True);

    // how many rows in this table
    uInt nrow() { return tab_p->nrow(); }

    // the current value of the index, returns -1 if this table isn't 
    // indexed
    Int index() { return index_p;}

    // return a const reference to the underlying table being filled
    const Table &table() {return *tab_p;}

    // flush this table
    void flush() {tab_p->flush();}
private:
    Table *tab_p;
    TableRow *tabRow_p;

    Int index_p;
    ScalarColumn<Int> indexCol_p;

    void updateTableRow();

    // this checks the record and adds columns as required
    void checkRecord(const RecordInterface &other);

    void attachIndexCol(const String &indexColumn);

    void setIndex(Int rownr, Bool nextIndex=True);

    // Undefined and unavailable
    GBTSimpleTable();
    GBTSimpleTable(const GBTSimpleTable &other);
    GBTSimpleTable &operator=(const GBTSimpleTable &other);
};


#endif



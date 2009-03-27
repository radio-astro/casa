//# GBTMSTypeDataFiller.h: this is used to fill the GBT_type_DATA table
//# Copyright (C) 2000,2001,2003
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

#ifndef NRAO_GBTMSTYPEDATAFILLER_H
#define NRAO_GBTMSTYPEDATAFILLER_H


#include <casa/aips.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/Table.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class ColumnsIndex;
class TableRow;
class Record;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// Used to fill the GBT_type_DATA table (e.g. GBT_HOLOGRAPHY_DATA)
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=GBTBackendFiller>GBTBackendFiller</linkto>
// </prerequisite>
//
// <etymology>
// The PROCESSOR
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// The GBT backend FITS DATA tables have columns which are unhandled by the
// regular filling process.  These columns are placed
// in a subtable name GBT_type)DATA where type is the type of backend such
// as HOLOGRAPHY, DCR, SPECTRALPROCESSOR or ACS.  These are indexed
// using the TYPE_ID value from the PROCESSOR table.  They also have TIME
// and INTERVAL keys.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> Some of the mechanisms here are shared with GBTSimpleTable and GBTMSTypeFiller.
//        They should share code where possible.
// </todo>

class GBTMSTypeDataFiller
{
public:
    // open an already existing table - type is extracted from the tableName
    // using everything after the second to last "_".
     GBTMSTypeDataFiller(const String &tableName);

    // create a new, empty table attached to the given table using 
    // the indicate typeData.
    GBTMSTypeDataFiller(Table &attachTable, const String &type);

    ~GBTMSTypeDataFiller();

    // fill this record to this table.
    // Each field in the record maps to a column in the
    // the table.  There must be no subrecords.  The type
    // of the fields must match that in the existing table.
    // Fields not already in the table will result in new
    // columns for that table.  A new row is always added.
    void fill(const Record &row, Double time, Double interval, Int typeId);

    // how many rows in this table
    uInt nrow() { return tab_p->nrow(); }

    // what is the type of this GBT_type_DATA table
    const String &type() {return type_p;}

    // flush this table
    void flush() { tab_p->flush();}
private:
    Table *tab_p;
    TableRow *tabRow_p;

    String type_p;

    ScalarColumn<Int> typeIdCol_p;
    ScalarColumn<Double> timeCol_p, intervalCol_p;

    void updateTableRow();

    // this checks the record and adds columns as required
    void checkRecord(const Record &other);

    // Undefined and unavailable
    GBTMSTypeDataFiller();
    GBTMSTypeDataFiller(const GBTMSTypeDataFiller &other);
    GBTMSTypeDataFiller &operator=(const GBTMSTypeDataFiller &other);
};


#endif



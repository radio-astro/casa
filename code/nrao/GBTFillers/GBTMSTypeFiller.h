//# GBTMSTypeFiller.h: a GBTMSTypeFiller is used to fill the GBT_type table
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

#ifndef NRAO_GBTMSTYPEFILLER_H
#define NRAO_GBTMSTYPEFILLER_H


#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <casa/BasicSL/String.h>

//# Forward Declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class ColumnsIndex;
class TableRow;
class Record;
class RecordDesc;
template <class T> class Vector;
} //# NAMESPACE CASA - END

#include <casa/namespace.h>

// <summary>
// Used to fill the GBT_type table (e.g. GBT_HOLOGRAPHY or GBT_DCR)
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
// The GBT backend FITS files have keywords which are unhandled by the
// regular filling process.  These are backend specific keywords which
// tend to be constant or re-used with time.  These values are placed
// in a subtable name GBT_type where type is the type of backend such
// as HOLOGRAPHY, DCR, SPECTRALPROCESSOR or ACS.  These are
// indexed directly from the PROCESSOR subtable of MS where TYPE_ID is
// the row into this table.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> Is there a mechanism in the Table system which essentially
//        already does this? 
// </todo>

class GBTMSTypeFiller
{
public:
    // open an already existing table - type is extracted from the tableName
    // using everything after the last "_".
     GBTMSTypeFiller(const String &tableName);

    // create a new, empty table attached to the given table using the indicate type.
    GBTMSTypeFiller(Table &attachTable, const String &type);


    ~GBTMSTypeFiller();

    // fill this record to this table.
    // Each field in the record maps to a column in the
    // the table.  There must be no subrecords.  The types
    // of the fields must match that in the existing table.
    // Fields not already in the table will result in new
    // columns for that table.  If there are only scalar columns, then new rows
    // will only be added if this row does not already exist in the table.
    // If there are any non-scalar columns, a new row is always added.
    void fill(const Record &other);

    // how many rows in this table
    uInt nrow() { return tab_p->nrow(); }

    // what is the type of this GBT_type table
    const String &type() {return type_p;}

    // what is the most recent typeId filled
    Int typeId() { return typeId_p;}

    // flush this table
    void flush() { tab_p->flush();}
private:
    Table *tab_p;
    TableRow *tabRow_p;

    ColumnsIndex *index_p;
    Int typeId_p;

    String type_p;

    void updateTableRow();
    void updateIndex(const RecordDesc &other);
    void updateIndex(const Vector<String> &colnames);

    // this checks the record and adds columns as required
    void checkRecord(Record &other);

    // Undefined and unavailable
    GBTMSTypeFiller();
    GBTMSTypeFiller(const GBTMSTypeFiller &other);
    GBTMSTypeFiller &operator=(const GBTMSTypeFiller &other);
};


#endif



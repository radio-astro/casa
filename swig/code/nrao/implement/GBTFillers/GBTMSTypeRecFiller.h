//# GBTMSTypeRecFiller.h: used to fill the GBT_type_SAMPLER table
//# Copyright (C) 2000,2001
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

#ifndef NRAO_GBTMSTYPERECFILLER_H
#define NRAO_GBTMSTYPERECFILLER_H


#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
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
// Used to fill the GBT_type_SAMPLER table (e.g. GBT_DCR_SAMPLER)
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
// Several GBT backends have a SAMPLER table associated with each scan.
// These tables are often very much the same from scan to scan and
// there are a limited number of different samplers which a user will
// typically choose.  Hence it makes sense to have these be simply
// indexed by row number and to re-use rows in the filled MS table
// when possible.
// </motivation>
//
// <todo asof="yyyy/mm/dd">
//   <li> the GBTMSType*Filler classes need to share a base class.
// </todo>

class GBTMSTypeSampFiller
{
public:
    // open an already existing table - type is extracted from the tableName
    // using everything before the last "_SAMPLER" to the previous "_"
     GBTMSTypeSampFiller(const String &tableName);

    // create a new, empty table attached to the given table using the 
    // indicate typeSamp.
    GBTMSTypeSampFiller(Table &attachTable, const String &type);

    ~GBTMSTypeSampFiller();

    // Fill the indicated table to this table.
    // Each column in other maps to a column in the
    // this table.  The type
    // of the columns must match that in the existing table.
    // Columns not already in this table will result in new
    // columns.  If there are only scalar columns, then new rows
    // will only be added if this row does not already exist in the table.
    // If there are any non-scalar columns, a new row is always added.
    void fill(const Table &other);

    // how many rows in this table
    uInt nrow() { return tab_p->nrow(); }

    // what is the type of this GBT_type_SAMPLER table
    const String &type() {return type_p;}

    // how many samplers were recently filled
    uInt nsamplers() {return samplerIds().nelements();}

    // what are the most recent sampler Ids filled
    const Vector<Int> samplerIds() { return sampIds_p;}

    // return the sampler Id for a particular row in the table which was
    // most recently used in a call to fill
    Int samplerId(Int whichSamp) {return samplerIds()(whichSamp);}

    // mark the column of the given name as handled - there is no check to see if
    // that column exists
    void handleColumn(const String &colname);

    // flush this table
    void flush() { tab_p->flush();}
private:
    Table *tab_p;
    TableRow *tabRow_p;

    ColumnsIndex *index_p;
    Vector<Int> sampIds_p;

    Vector<String> handledCols_p;

    String type_p;

    void updateTableRow();
    void updateIndex();

    // we fill on a row by row basis
    void fillRow(const Record &row);

    // this checks the record and adds columns as required
    void checkRecord(const Record &row);

    // Undefined and unavailable
    GBTMSTypeSampFiller();
    GBTMSTypeSampFiller(const GBTMSTypeSampFiller &other);
    GBTMSTypeSampFiller &operator=(const GBTMSTypeSampFiller &other);
};


#endif



//# OldGBTPositionTable.h: GBT Position FITS files are special
//# Copyright (C) 1995,1997,1999
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

#ifndef NRAO_OLDGBTPOSITIONTABLE_H
#define NRAO_OLDGBTPOSITIONTABLE_H

#include <casa/aips.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Containers/RecordField.h>

#include <casa/namespace.h>
// <summary>
// Attach a FITSTabular to a binary or ASCII table
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> General knowledge of FITS binary and ASCII tables.
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// FITSDtable is a FITSTabular which is attached to a FITS table (on disk only
// presently), either Binary or ASCII.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="1995/06/01">
//   <li>
// </todo>

class OldGBTPositionTable : public FITSTabular
{
public:
    // 0-relative HDU. It can never be zero by the FITS rules.
    OldGBTPositionTable(const String &fileName);
    ~OldGBTPositionTable();

    // Attach this FITSDtable to a new file name, same HDU# as at open time
    Bool reopen(const String &fileName);
    virtual const String &name() const { return name_p;}

    virtual Bool isValid() const;

    virtual const TableRecord &keywords() const;
    virtual const RecordDesc &description() const;
    virtual const Record &units() const;
    virtual const Record &displayFormats() const;
    virtual const Record &nulls() const;

    virtual Bool pastEnd() const;
    virtual void next();
    virtual const Record &currentRow() const;
private:
    // Undefined and inaccessible. An alternative would be to use reference
    // semantics like Table.
    OldGBTPositionTable();
    OldGBTPositionTable(const OldGBTPositionTable &);
    OldGBTPositionTable &operator=(const OldGBTPositionTable &);

    void fill_row();
    void clear_self();

    Bool isValid_p;

    String name_p;

    uInt maxCount_p;
    uInt count_p;
    Double delta_time_p;
    RecordFieldPtr<Double> dmjdField_p;

    Int row_nr_p;
    BinaryTableExtension *raw_table_p;
    FitsInput *io_p;
    TableRecord keywords_p;
    RecordDesc description_p;
    Record units_p;
    Record disps_p;
    Record nulls_p;
    Record row_p;
    // One per field in row_p, of the right type. i.e. casting required.
    Block<void *> row_fields_p;
};


#endif

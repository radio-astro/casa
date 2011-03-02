//# OldGBTBackendTable.h:  GBT backend tables with AIPS++ Look and Feel.
//# Copyright (C) 1995,1997,1999,2000,2001
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

#ifndef NRAO_OLDGBTBACKENDTABLE_H
#define NRAO_OLDGBTBACKENDTABLE_H

#include <casa/aips.h>
#include <fits/FITS/FITSTable.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/RecordDesc.h>
#include <casa/Utilities/DataType.h>
#include <tables/Tables/TableColumn.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <fits/FITS/hdu.h>

#include <casa/namespace.h>
//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
class HeaderDataUnit;
class Table;
class FitsInput;
class BinaryTable;
} //# NAMESPACE CASA - END


// <summary>
// Attach a FITSTabular to a GBT backend FITS file
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
//   <li> General knowledge of FITS binary and ASCII tables.
//   <li> General knowledge of the GBT FITS file format for backend data.
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// OldGBTBackendTable is a FITSTabular which is attached to a GBT FITS 
// backend file.   GBT FITS backend files contain a DATA table and 
// several anciliary tables that correspond to the axes of the data 
// column in the DATA table.  This class makes such a file appear as a 
// single table.
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

class OldGBTBackendTable : public FITSTabular
{
public:
    // The file name
    OldGBTBackendTable(const String &fileName);
    ~OldGBTBackendTable();

    // Attach this OldGBTBackendTable to a new file name.
    Bool reopen(const String &fileName);
    virtual const String &name() const { return name_p;}
    virtual Bool isValid() const {return isValid_p;}

    virtual const TableRecord &keywords() const {return keywords_p;}
    virtual const RecordDesc &description() const {return description_p;}
    virtual const Record &units() const {return units_p;}
    virtual const Record &displayFormats() const {return disps_p;}
    virtual const Record &nulls() const {return nulls_p;}

    virtual Bool pastEnd() const {
	return (row_nr_p >= max_row_nr_p || ! isValid());
    }
    virtual void next() {if (isValid()) fill_row();}
    virtual const Record &currentRow() const {return row_p;}
private:
    // Undefined and inaccessible. An alternative would be to use reference
    // semantics like Table.
    OldGBTBackendTable();
    OldGBTBackendTable(const OldGBTBackendTable &);
    OldGBTBackendTable &operator=(const OldGBTBackendTable &);

    void fill_row();
    void clear_self();

    void putField(void *fieldPtr, FitsBase &base, DataType dtype, 
		  Int nelements = -1, uInt offset = 0);
    void putColumn(void *fieldPtr, const ROTableColumn& tabCol, 
		   uInt tabRow, DataType dtype, uInt index = 0);

    Bool isValid_p;

    String name_p;

    Int row_nr_p;
    Int max_row_nr_p;
    // Used by dataOffset to convert phase number and receiver nr to an 
    // offset in the data array
    uInt phase_offset_p, rec_offset_p;
    // one table phase and one for receiver info, these are held in memory
    // that may need to be changed if these supporting table get to be
    // too large
    Table *phase_table_p, *rec_table_p;
    // Similarly, we need a map of table column number to row field number
    // for each supporting table, this holds that info, if a value is < 0
    // then that column is not in use in the row
    Block<Int> phase_map_p, rec_map_p;
    // and this is the field that will hold the current row number
    // for each support table
    RecordFieldPtr<Int> *phase_id_p, *rec_id_p;
    // SP (and possibly other?) types of backends have most of the columns 
    // indexed by receiver number - we need to make that transparent so 
    // here we determine which Fields are so indexed
    Block<Bool> rcvrIndexed_p;

    // The total duration of a subscan (individual row in data table)
    Double duration_p;
    // this is a cumulative duration offset
    Double duration_offset_p;
    // the  time at the center of each phase averaged over all cycles
    // relative to UTCSTART at the start of the integration
    Matrix<Double> center_phase_time_p;

    // the time 
    RecordFieldPtr<Double> *time_p;
    // the UTCSTART and UTDATE field numbers
    Int utcstartField_p, utdateField_p;
    // sometime utcstart is in RADIANS (spectral processor), but mostly it
    // is in seconds - this factor is usually 1 but is sometimes seconds/radian
    Double ut_factor_p;
    // We need to know if the utcstart column is a keyword or an actual column
    Bool utcIsColumn_p;
    // only the Holo backend has this, but use it when available
    Int dmjdField_p;

    // one table for the DATA table, this is the only fitsio kept active
    // throughout the life of the object
    BinaryTable *data_table_p;
    // this is the map from table column to row field
    // Int is < 0 for DATA since that is handled sepearately
    Block<Int> data_map_p;
    // This is needed to prevent a memory leak, it corresponds to the above binary table
    FitsInput * fitsIO_p;
    // the field number in the data_table_p of the DATA column
    Int dataCol_p;
    // this field in row_fields_p holds the data
    Int dataField_p;
    // the number of elements of data to write at a time
    uInt datalength_p;

    // This is a merged collection from all the tables, it may not
    // be that useful
    TableRecord keywords_p;
    // This is a merged description from all the tables, it also contains
    // some things that are in the keywords, it corresponds to the
    // description actually seen for each row.
    RecordDesc description_p;
    // These are only the units of the for data table columns.
    Record units_p;
    // these are also from the data table columns only.
    Record disps_p;
    Record nulls_p;
    // The current row
    Record row_p;
    // One per field in row_p, of the right type. i.e. casting required.
    Block<void *> row_fields_p;

    // this is needed strictly for a bug in the SP FITS files involving
    // utcstart and utdate
    Double oldStart_p;

    Double utcstart() const { return scalarFieldAsDouble(utcstartField_p); }
    Double utdate() const { return scalarFieldAsDouble(utdateField_p); }
    Double scalarFieldAsDouble(Int whichField) const;
    // This is ugly - These are the reserved FITS keywords that I think do NOT belong
    // in the output table - essentially these are keywords that describe the FITS file
    // as opposed to the data therin.  They are fundamental to FITS, not the data.

    Vector<String> ignoredFITS;
    void init_ignoredFITS();

    // these should probably be made public/static members of the RecordDesc class?
    RecordDesc descriptionFromOKKeys(const TableRecord& kwset);
    RecordDesc descriptionFromTableDesc(const TableDesc& tabDesc);
};


#ifndef AIPS_NO_TEMPLATE_SRC
#include <nrao/FITS/OldGBTBackendTable.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif

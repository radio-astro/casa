//# NewCalTableDesc.cc: Implementation of NewCalTableDesc.h
//# Copyright (C) 2011 
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
//----------------------------------------------------------------------------

#include <calibration/CalTables/NewCalTableDesc.h>
#include <calibration/CalTables/VisCalEnum.h>
#include <msvis/MSVis/MSCalEnums.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableQuantumDesc.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/IPosition.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

NewCalTableDesc::NewCalTableDesc() : itsCalMainDesc(defaultCalMain(""))
{
// Default null constructor for new calibration table description (v1.0)
// Output to private data:
//    itsCalMainDesc      TableDesc        Table descriptor (cal_main)
//    itsCalHistoryDesc   TableDesc        Table descriptor (cal_history)
//
};

//----------------------------------------------------------------------------

NewCalTableDesc::NewCalTableDesc (const String& type) : 
  itsCalMainDesc(defaultCalMain(type))
{
// Constructor for new calibration table description (v1.0)
// Inputs:
//    type                const String&    Cal table type (eg. "P Jones")
// Output to private data:
//    itsCalMainDesc      TableDesc        Table descriptor (cal_main)
//    itsCalHistoryDesc   TableDesc        Table descriptor (cal_history)
//
};

//----------------------------------------------------------------------------
/***
TableDesc CalTableDesc::defaultCalHistory()
{
// Generate the default table descriptor for the Cal History sub-table
// Output:
//    defaultCalHistory     TableDesc     Default Cal History descriptor
//
  TableDesc td ("Cal History", "2.0", TableDesc::Scratch);
  td.comment() = "Calibration history sub-table";
  td.addColumn (ScalarColumnDesc <String> (MSC::fieldName (MSC::CAL_PARMS),
					   ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <String> (MSC::fieldName (MSC::CAL_TABLES),
					   ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <String> (MSC::fieldName (MSC::CAL_SELECT),
					   ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <String> (MSC::fieldName (MSC::CAL_NOTES),
					   ColumnDesc::Direct));
  return td;
};
***/
//----------------------------------------------------------------------------

TableDesc NewCalTableDesc::defaultCalMain (const String& type)
{
// Generate the default table descriptor for the Cal Main sub-table
// Input:
//    type             const String&      Cal table type (eg. "P Jones")
// Output:
//    defaultCalMain   TableDesc          Default Cal Main descriptor
//
  // Set up table descriptor and add comment field
  TableDesc td (type, "1.0", TableDesc::Scratch);
  td.comment() = "New calibration table";

  // Define keywords
  Record keyWordRec;
  // Cal_desc and cal_history indices
  // calibration type
  keyWordRec.define ("Type", "Complex");
  keyWordRec.define ("MSName","dummy");
  keyWordRec.define ("PolBasis", "lin");
  // Add to table descriptor
  td.rwKeywordSet().assign (keyWordRec);

  // Cal Main columns (MS Main indices)
  // keys
  td.addColumn (ScalarColumnDesc <Double> (MSC::fieldName (MSC::TIME),
					   ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <Int> (MSC::fieldName (MSC::FIELD_ID),
					ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <Int> (MSC::fieldName (MSC::SPECTRAL_WINDOW_ID),
					ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <Int> (MSC::fieldName (MSC::ANTENNA1),
					ColumnDesc::Direct));
  // optical? ANTENNA2
  td.addColumn (ScalarColumnDesc <Int> (MSC::fieldName (MSC::ANTENNA2),
					ColumnDesc::Direct));
  //non keys
  td.addColumn (ScalarColumnDesc <Double> (MSC::fieldName (MSC::INTERVAL),
					   ColumnDesc::Direct));
  td.addColumn (ScalarColumnDesc <Int> (MSC::fieldName (MSC::SCAN_NUMBER),
					ColumnDesc::Direct));

  //Parameter columns
  td.addColumn (ArrayColumnDesc <Float>  ("PARAM"));
  td.addColumn (ArrayColumnDesc <Float>  ("PARAMERR"));
  td.addColumn (ArrayColumnDesc <Bool>  (MSC::fieldName (MSC::FLAG)));
  td.addColumn (ArrayColumnDesc <Float>  (MSC::fieldName (MSC::SNR)));
  td.addColumn (ArrayColumnDesc <Float>  ("WEIGHT"));

  // Add TableMeasures information for designated Measures/Quanta columns
  // 
  // TIME
  TableMeasValueDesc timeMeasVal (td, MSC::fieldName (MSC::TIME));
  TableMeasRefDesc timeMeasRef (MEpoch::DEFAULT);
  TableMeasDesc<MEpoch> timeMeasCol (timeMeasVal, timeMeasRef);
  timeMeasCol.write (td);

  // TIME fix Unit
  TableQuantumDesc timeQuantDesc (td, MSC::fieldName (MSC::TIME),
                                  Unit ("s"));
  timeQuantDesc.write (td);
  // INTERVAL
  TableQuantumDesc intervalQuantDesc (td, MSC::fieldName(MSC::INTERVAL),
				      Unit ("s"));
  intervalQuantDesc.write (td);

  return td;
};

//----------------------------------------------------------------------------

TableDesc NewCalTableDesc::calMainDesc()
{
// Return the main calibration table descriptor
// Output:
//    calMainDesc     TableDesc        Table descriptor (cal_main)
//
  return itsCalMainDesc;
};

//----------------------------------------------------------------------------
/***
TableDesc NewCalTableDesc::calHistoryDesc()
{
// Return the calibration history sub-table descriptor
// Output:
//    calHistoryDesc  TableDesc        Table descriptor (cal_history)
//
  return itsCalHistoryDesc;
};
***/
//----------------------------------------------------------------------------

TableDesc NewCalTableDesc::insertDesc (const TableDesc& tableDesc, 
				    const TableDesc& insert, 
				    const String& insertAfter)
{
// Insert one table descriptor into another after a specified
// column name.
// Input:
//    tableDesc       const TableDesc&      Input table descriptor
//    insert          const TableDesc&      Table descriptor to be inserted
//    insertAfter     const String&         Column name to insert after
// Output:
//    insertDesc      TableDesc             Output table descriptor
//
  TableDesc tdout;
  ColumnDesc* colDesc;
  Int ncol = tableDesc.ncolumn();
  Int jcol;

  // Loop over the number of columns in the input table descriptor
  for (jcol = 0; jcol < ncol; jcol++) {
    colDesc = new ColumnDesc (tableDesc.columnDesc (jcol));

    // Add column to output descriptor
    tdout.addColumn (*colDesc);

    // Insert second descriptor if appropriate
    if (colDesc->name() == insertAfter) {
      tdout.add (insert);
    };
    delete colDesc;
  };

  return tdout;
};

//----------------------------------------------------------------------------


} //# NAMESPACE CASA - END


//# CalTable.h: Calibration table access and creation
//# Copyright (C) 1996,1997,1998,1999,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_CALTABLE2_H
#define CALIBRATION_CALTABLE2_H

#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ExprNode.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Sort.h>
#include <synthesis/CalTables/CalTableDesc2.h>
#include <synthesis/CalTables/CalMainRecord.h>
#include <synthesis/CalTables/CalDescRecord.h>
#include <synthesis/CalTables/CalHistRecord.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  // <summary> 
  // CalTable: Calibration table access and creation
  // </summary>
  
  // <use visibility=export>
  
  // <reviewed reviewer="" date="" tests="" demos="">
  
  // <prerequisite>
  //   <li> <linkto class="CalTableDesc">CalTableDesc</linkto> module
  // </prerequisite>
  //
  // <etymology>
  // From "calibration" and "table".
  // </etymology>
  //
  // <synopsis>
  // The CalTable classes provide basic access to calibration tables. This 
  // includes the creation of calibration tables, sorting and selection
  // capabilities, and basic data access. Specializations for baseline-based,
  // time-variable and solvable VisJones types, and sub-types, are provided 
  // through inheritance.
  // </etymology>
  //
  // <example>
  // <srcblock>
  // </srcblock>
  // </example>
  //
  // <motivation>
  // This class is used by other calibration table accessors and iterators.
  // </motivation>
  //
  // <todo asof="98/01/01">
  // (i) Generic synthesis selection
  // </todo>
  
  class CalTable2
  {
  public:
    // Default null constructor, and destructor
    CalTable2();
    virtual ~CalTable2();
    
    // Construct from a specified table name, calibration table descriptor 
    // and table access option. Used for creating new tables.
    CalTable2(const String& tableName, CalTableDesc2& ctableDesc,
	      Table::TableOption access = Table::New);
    
    // Construct from a specified table name, and access option. Used
    // for accessing existing tables.
    CalTable2(const String& tableName, Table::TableOption access = Table::Old);
    
    // Construct from an existing table object
    CalTable2(const Table& table);
    
    // Copy constructor
    CalTable2(const CalTable2& other);
    
    //
    const Int parType() {return parType_;};
    // Assignment operator
    CalTable2& operator= (const CalTable2& other);
    
    // Sort (cal_main)
    CalTable2 sort (const Block <String>& columnNames, 
		    Sort::Order order = Sort::Ascending,
		    Sort::Option option = Sort::HeapSort); 
    
    // Sort (cal_main) IN PLACE
    void sort2 (const Block <String>& columnNames, 
		Sort::Order order = Sort::Ascending,
		Sort::Option option = Sort::HeapSort); 
    
    // Apply selection to the calibration table 
    CalTable2 select (const String& calSelect);
    
    // Apply selection to the calibration table  IN PLACE
    void select2 (const String& calSelect);
    
    // Return number of rows in cal_main, cal_desc or cal_history
    Int nRowMain() const;
    Int nRowDesc() const;
    Int nRowHistory() const;
    
    // Add rows to cal_main, cal_desc or cal_history
    void addRowMain (uInt nrrow = 1, Bool initialize = False) 
    {itsMainTable->addRow(nrrow, initialize);};
    void addRowDesc (uInt nrrow = 1, Bool initialize = False)
    {itsDescTable->addRow(nrrow, initialize);};
    void addRowHistory (uInt nrrow = 1, Bool initialize = False)
    {itsHistoryTable->addRow(nrrow, initialize);};
    
    // Get a row from cal_main, cal_desc or cal_history
    Record getRowMain (const Int& jrow);
    Record getRowDesc (const Int& jrow);
    Record getRowHistory (const Int& jrow);
    
    // Put a row to cal_main, cal_desc or cal_history
    void putRowMain (const Int& jrow, CalMainRecord& tableRec);
    void putRowDesc (const Int& jrow, CalDescRecord& tableRec);
    void putRowHistory (const Int& jrow, CalHistoryRecord& tableRec);
    
    // Return the maximum antenna number in the table
    virtual Int maxAntenna();
    
    // Return the number of unique time slots in the table
    Int numberTimeSlots (const Double& fracError);
    
    // Create a new table
    void createCalTable(const String& tableName, CalTableDesc2& ctableDesc,
			Table::TableOption access = Table::New);
    
    // Open an existing table
    void openCalTable(const String& tableName, 
		      Table::TableOption access = Table::Old);
    
    // Return cal_main and sub-tables as Table references
    Table& calMainAsTable() {return *itsMainTable;};
    Table& calDescAsTable() {return *itsDescTable;};
    Table& calHistoryAsTable() {return *itsHistoryTable;};
    const Table& calMainAsTable() const {return *itsMainTable;};
    const Table& calDescAsTable() const {return *itsDescTable;};
    const Table& calHistoryAsTable() const {return *itsHistoryTable;};
  protected:
    
    // Friend class access from the ROCalMainColumns, ROCalDescColumns
    // and ROCalHistoryColumns class hierarchies
//    friend class ROCalMainColumns2;
//    friend class CalMainColumns2;
//    friend class ROCalDescColumns2;
//    friend class CalDescColumns2;
    friend class ROCalHistoryColumns;
    friend class CalHistoryColumns;
    
  private:
    // Pointer to underlying cal_main Table object
    Table* itsMainTable;
    
    // Pointers to the cal_desc and cal_history sub-tables
    Table* itsDescTable;
    Table* itsHistoryTable;
    Int parType_;
  };

} //# NAMESPACE CASA - END

#endif


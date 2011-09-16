//# Table.h: Primary interface for the rest of the browser to a table.
//# Copyright (C) 2007-2008
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
//# $Id: $
#ifndef TBTABLE_H_
#define TBTABLE_H_

#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBPlotter.qo.h>

#include <casa/BasicSL/String.h>

#include <map>
#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBField;
class TBKeyword;
class TBArray;
class ProgressHelper;
class XMLtoken;
class TBParser;
class TBTableDriver;
class TBData;
class TBArrayData;
class TBPlotData;
class TBFilterRuleSequence;

// <summary>
// Parameters to indicate what driver should be used for the table backend.
// </summary>
//
// <synopsis>
// The DriverParams consist of two parameters: the driver type and (if
// applicable) the parser type.  The parser type is only used for the XML
// driver.
// </synopsis>

class DriverParams {
public:
    // Default Constructor.
    DriverParams(Driver t = DIRECT, Parser p = XERCES_SAX) : type(t),
                                                             parser(p) { }

    // Copy Constructor.
    DriverParams(DriverParams* dp) : type(dp->type), parser(dp->parser) { }

    
    // Driver type.
    Driver type;

    // Parser type (for XML driver).
    Parser parser;
};

// <summary>
// Parameters that define all table data and meta-deta.
// </summary>
//
// <synopsis>
// A TableParams contains references to all important table members.  Since
// this information needs to be accessed and potentially changed by both the
// TBTable and the TBTableDriver, the references are shared by passing a
// TableParams object.
// </synopsis>

class TableParams {
public:
    TableParams(String& l, bool& ir, bool& rr,
                vector<vector<TBData*>*>& d,
                vector<TBField*>& f, vector<TBKeyword*>& k,
                vector<int>& sr, int& tr, int& lr, vector<bool>& w, bool& t,
                DriverParams* dp): location(l), insertRow(ir), removeRow(rr),
                data(d), fields(f), keywords(k), subtableRows(sr),
                totalRows(tr), loadedRows(lr), writable(w), taql(t),
                dParams(dp) { }

                
    // See TBTable::location.
    String& location;
    
    // See TBTable::insertRow.
    bool& insertRow;
    
    // See TBTable::removeRow.
    bool& removeRow;
    
    // See TBTable::data.    
    vector<vector<TBData*>*>& data;
    
    // See TBTable::fields.
    vector<TBField*>& fields;
    
    // See TBTable::keywords.
    vector<TBKeyword*>& keywords;
    
    // See TBTable::subtableRows.
    vector<int>& subtableRows;
    
    // See TBTable::totalRows.
    int& totalRows;
    
    // See TBTable::loadedRows.
    int& loadedRows;
    
    // See TBTable::writable.
    vector<bool>& writable;
    
    // See TBTable::taql.
    bool& taql;
    
    // See TBTable::dParams.
    DriverParams* dParams;
};

// <summary>
// Primary interface for the rest of the browser to a table.
// </summary>
// 
// <synopsis>
// Although TBTable is considered the table backend, it is actually more like
// an interface over a TBTableDriver.  However, since there may be multiple
// implementations of the driver, the table provides a unified interface over
// the driver as well as other useful operations.  Generally speaking, the
// driver handles dealing with the table on disk while the TBTable handles
// dealing with the data represented in memory.
// </synopsis>

class TBTable {
public:
    // Constructor that takes the filename and driver parameters.  If taql is
    // true, then this table is a table constructed from a TaQL command stored
    // in filename; otherwise filename holds the location on disk of the table.
    TBTable(String filename, DriverParams* dp, bool taql = false);

    ~TBTable();
    

    // Returns the filename where this table is located.
    String getFile();

    // Returns the name of this table (which the last the part of the
    // filename).  If this is a TaQL table, returns "TaQL Table".
    String getName();

    // Returns the driver parameters for this table.
    DriverParams* getDriverParams();

    // Returns the fields for this table.
    vector<TBField*>* getFields();
    
    // Returns the field at the given index, or NULL if there is none.
    TBField* field(int i);
    
    // Returns the field with the given name, or NULL if there is none.
    TBField* field(String name);

    // Returns the table keywords.
    vector<TBKeyword*>* getTableKeywords();

    // Returns the keyword at the given index, or NULL if there is none.
    TBKeyword* keyword(int i);
    
    // Returns the keyword with the given name, or NULL if there is none.
    TBKeyword* keyword(String name);
    
    // Indicates if the table is ready for access or not.
    bool isReady();

    // Returns true if the underlying table allows for insertion of rows,
    // false otherwise.
    bool canInsertRows();

    // Returns true if the underlying table allows for deletion of rows,
    // false otherwise.
    bool canDeleteRows();

    // Returns the total number of rows in the table.
    int getTotalRows();

    // Returns the number of rows loaded into the table.
    int getLoadedRows();
    
    // Returns the number of selected rows.  Note: this may be different than
    // the number of loaded rows.  For example, if you load rows [0 - 1000] but
    // the table has only 20 rows, getSelectedRows() would return 1000 while
    // getLoadedRows() would return 20.
    int getSelectedRows();
    
    // Returns the number of pages in the table.
    int getNumPages();

    // Return the currently loaded page.
    int getPage();

    // Returns the first loaded row in the table.
    int getRowIndex();
    
    // Returns the number of columns in the table.
    int getNumFields();
    
    // Sets whether the table should print debug information or not.
    void setPrintDebug(bool pdb);
    
    // Returns the data at the given indices, or NULL if they are invalid.
    TBData* dataAt(unsigned int row, unsigned int col);

    // Returns the shape of the array for the given field, or an empty list
    // if the field is not an array.
    vector<int> dataDimensionsAt(unsigned int col);
    
    // Returns whether all the columns in this table are editable or not.
    bool isEditable();
    
    // Returns whether any of the columns in this table are editable or not.
    bool isAnyEditable();
    
    // Returns whether the column at the given index is editable or not.
    bool isEditable(int index);
    
    // Returns whether this table is constructed from a TaQL query or not.
    bool isTaQL();
    
    // See TBTableDriver::canRead().
    bool canRead();
    
    // See TBTableDriver::canWrite().
    bool canWrite();
    
    // See TBTableDriver::tryWriteLock().
    bool tryWriteLock();

    // See TBTableDriver::releaseWriteLock().
    bool releaseWriteLock();
    
    
    // Loads the given rows into the table and returns a Result indicating the
    // success of the loading.  The rows loaded will be from start to start +
    // number (inclusive).  If full is true, array data will be included;
    // otherwise only the shapes of the array will be included.  If columns
    // is a non-NULL, non-empty list, then only those fields will be loaded
    // into the table.  If parsedata is true, the table data will be loaded;
    // otherwise only the meta-data will be loaded.  If a ProgressHelper is
    // provided, the label and progress meter will be periodically updated
    // as progress is made.
    Result loadRows(int start = 0,
                    int number = TBConstants::DEFAULT_SELECT_NUM,
                    bool full = false, vector<String>* columns = NULL,
                    bool parsedata = true,
                    ProgressHelper* progressPanel = NULL);

    // Returns the column headers (field names) for this table.
    vector<String> getColumnHeaders();

    // Returns the row headers (row numbers) for the loaded rows in this table.
    vector<String> getRowHeaders();

    // Loads and returns the array at the given indices.
    TBArrayData* loadArray(unsigned int row, unsigned int col);
    
    // Release the array at the given indices and returns whether the release
    // succeeded.
    bool releaseArray(unsigned int row, unsigned int col);

    // Edits the data at the given indices to have the new value and returns
    // whether the operation succeeded.
    Result editData(unsigned int row, unsigned int col, TBData* newVal);
    
    // Updates the table on disk with the new value at the given coordinates
    // in the array at the given row and column.  oneDim must be true for a
    // one-dimensional array, false otherwise.  Returns a result indicating
    // the success of the editing.    
    Result editArrayData(unsigned int row, unsigned int col,
                         vector<int> coords, TBData* newVal, bool oneDim);

    // Exports the entirety of this table to VOTable XML format to the given
    // file.  If a ProgressHelper is provided, progress information is updated
    // periodically.
    void exportVOTable(String file, ProgressHelper* progressPanel = NULL);

    // Returns data used for plotting, using the given plot parameters and row
    // information.  If a ProgressHelper is provided, progress information is
    // updated periodically.
    TBPlotData* plotRows(PlotParams& x, PlotParams& y, int rowFrom, int rowTo,
                       int rowInterval, TBFilterRuleSequence* rule = NULL,
                       ProgressHelper* ph = NULL);

    // Returns data used for plotting (an 1-D array and its indices),
    // using the given plot parameters, slice axis, and row number.
    // If a ProgressHelperis provided, progress information is updated 
    // periodically.
    TBPlotData* plotIndices(PlotParams& dp, int axis, bool x, int row,
                       TBFilterRuleSequence* rule = NULL,
                       ProgressHelper* ph = NULL);

    // Returns the total number of rows for the table at the given location, or
    // -1 for an invalid location or other problem.  TBTable caches the total
    // rows of all subtables (i.e., tables pointed to in the table keywords).
    int totalRowsOf(String location);

    // Returns a tooltip for the field at index i.
    String fieldToolTip(int i);

    // Returns a tooltip for this table.
    String tableToolTip();

    // Inserts the given number of rows at the end of the table.  Returns a
    // Result indicating the success of the operation.
    Result insertRows(int n);

    // Deletes the given rows from the table.  Each element in the vector
    // should be a row number.  Returns a Result indicating the success of the
    // operation.
    Result deleteRows(vector<int> r);

private:
    // Driver parameters.
    DriverParams* dParams;

    // Location of the table.
    String location;
    
    // Indicates whether the table is ready for access or not.
    bool ready;
    
    // Indicates whether this table allows for the insertion of rows or not.
    bool insertRow;
    
    // Indicates whether this table allows for the deletion of rows or not.
    bool removeRow;
    
    // Holds the table data.    
    vector< vector<TBData*>* > data;
    
    // The total number of rows in the table.
    int totalRows;
    
    // The number of rows currently loaded in the table.
    int loadedRows;
    
    // The last number of rows selected for loading during the last load.
    int selectedRows;
    
    // The index of the first row loaded in this table.
    int rowIndex;

    // The table fields.
    vector<TBField*> fields;
    
    // The table keywords.
    vector<TBKeyword*> keywords;
    
    // Cache of total number of rows for subtables.
    vector<int> subtableRows;
    
    // Indicates whether this table is writable or not.
    vector<bool> writable;
    
    // Indates whether this table was constructed from a TaQL command or not.
    bool taql;

    // Table driver.
    TBTableDriver* driver;

    
    // Helper method for plotRows().
    double getDouble(TBData* d, vector<int>* slice, bool complex, bool amp);
    
    // Helper method for plotRows().
    void filter(TBPlotData* data, TBTable& table, TBFilterRuleSequence* rules,
                int rowFrom, int rowTo, int rowInterval);
};

}

#endif /* TBTABLE_H_ */

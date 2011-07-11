//# NewCalTable.cc: Implementation of NewCalTable.h
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

#include <calibration/CalTables/NewCalTable.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableParse.h>
#include <casa/Arrays.h>
#include <casa/Arrays/ArrayMath.h>
#include <msvis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//----------------------------------------------------------------------------

NewCalTable::NewCalTable()
{
// Default null constructor for calibration table; do nothing for now
};

//----------------------------------------------------------------------------

NewCalTable::~NewCalTable()
{
// Default desctructor
};

//----------------------------------------------------------------------------
NewCalTable::NewCalTable (const String& tableName, NewCalTableDesc& ctableDesc,
		    Table::TableOption access, Table::TableType ttype):
              Table() 
{
// Construct from a cal table name, descriptor and access option.
// Used for creating new tables.
// Input:
//    tableName        const String&         Cal table name
//    ctableDesc       const NewCalTableDesc&   Cal table descriptor
//    access           Table::TableOption    Access mode
//    ttype            Table::TableType      Memory or Plain
//
  ttype_p = ttype;
  if (access == Table::New || access == Table::NewNoReplace ||
      access == Table::Scratch) {
      SetupNewTable calMainTab(tableName,ctableDesc.calMainDesc(),access);
      NewCalTable tab(calMainTab, 0);
      *this = tab;
      
      // subtables
      String  calAntennaName=tableName+"/ANTENNA";
      String  calFieldName=tableName+"/FIELD";
      String  calSpectralWindowName=tableName+"/SPECTRAL_WINDOW";
      String  calHistoryName=tableName+"/HISTORY";

      SetupNewTable antennatab(calAntennaName,CalAntenna::requiredTableDesc(),access); 
      this->rwKeywordSet().defineTable("ANTENNA", Table(antennatab));
      antenna_p = CalAntenna(this->keywordSet().asTable("ANTENNA"));

      SetupNewTable fieldtab(calFieldName,CalField::requiredTableDesc(),access); 
      this->rwKeywordSet().defineTable("FIELD", Table(fieldtab));
      field_p = CalField(this->keywordSet().asTable("FIELD"));

      SetupNewTable spwtab(calSpectralWindowName,CalSpectralWindow::requiredTableDesc(),access); 
      this->rwKeywordSet().defineTable("SPECTRAL_WINDOW", Table(spwtab));
      spectralWindow_p = CalSpectralWindow(this->keywordSet().asTable("SPECTRAL_WINDOW"));

      SetupNewTable histab(calHistoryName,CalHistory::requiredTableDesc(),access); 
      this->rwKeywordSet().defineTable("HISTORY", Table(histab));
      history_p = CalHistory(this->keywordSet().asTable("HISTORY"));
  }
  else {
      NewCalTable (tableName, access);
  }
};

//----------------------------------------------------------------------------
NewCalTable::NewCalTable (SetupNewTable& newtab, uInt nrow, Bool initialize):
              Table(newtab, nrow, initialize)
{
// Construct from a cal table name, descriptor and access option.
// Used for creating new tables.
// Input:
//    newtab           SetupNewtable&   
//    nrow             uInt                  n rows
//    initialize       Bool                   
//
};

//----------------------------------------------------------------------------
NewCalTable::NewCalTable (const String& tableName, Table::TableOption access, 
                      Table::TableType ttype): Table(tableName,access)
{
// Construct from an exisiting cal table, and access option.
// 
// Input:
//    tableName        const String&         Cal table name
//    ctableDesc       const CalTableDesc&   Cal table descriptor
//    access           Table::TableOption    Access mode
//    ttype            Table::TableType      Memory or Plain
//
  if (ttype==Table::Memory) {
    *this = this->copyToMemoryTable("tempcaltable");
  }

//  else {
//    *this = tab;
//  }
};

//----------------------------------------------------------------------------

NewCalTable::NewCalTable (const Table& table): Table(table)
{
// Construct from an existing table object
};

//----------------------------------------------------------------------------

NewCalTable::NewCalTable (const NewCalTable& other): Table(other)
{
// Copy constructor
// Input:
//    other            const NewCalTable&       Existing NewCalTable object
//
};

//----------------------------------------------------------------------------

NewCalTable& NewCalTable::operator= (const NewCalTable& other)
{
// Assignment operator
// Input:
//    other            const CalTable&       RHS CalTable object
//
  if (this != &other) {
    Table::operator=(other);
    if (!conformant(this->tableDesc()))
        throw (AipsError("NewCalTable( const NewCalTable&) - "
                         "table is not a valid caltable"));
  }
  return *this;
};

//----------------------------------------------------------------------------
//Int NewCalTable::nRowMain() const 
//{
// Output: 
//    nMainRow         Int
//
//  return caltable_.nrow();
//};

//----------------------------------------------------------------------------
Record NewCalTable::getRowMain (const Int& jrow)
{
// Get a row from cal_main
// Input:
//    jrow             const Int&            Row number
// Output:
//    getRowMain       Record                Row record
//
  ROTableRow trow (*this);
  trow.get (jrow);
  return trow.record();
};

//----------------------------------------------------------------------------
void NewCalTable::putRowMain (const Int& jrow, NewCalMainRecord& tableRec)
{
// Get a row from cal_main
// Input:
//    jrow             const Int&            Row number
//    tableRec         const CalMainRecord&  Table record 
//
// Add rows as required
  Int nMaxRow = this->nrow();
  Int nAdd = jrow - nMaxRow + 1;
  if (nAdd > 0) {
    this->addRow (nAdd);
  };

// Write the record
  TableRow trow (*this);
  TableRecord trec = tableRec.record();
  trow.putMatchingFields (jrow, trec);
};

//----------------------------------------------------------------------------
void NewCalTable::setMetaInfo(const String& msName)
{
// set Meta data info:
// put parent MS name and (for now) make copy of Antenna, Field, and SpW 
// sub-tables.
  MeasurementSet inms(msName);
  const MSAntenna msantab = inms.antenna();
  const MSField msfldtab = inms.field();
  const MSSpectralWindow msspwtab = inms.spectralWindow();

  //copy antenna table
  CalAntenna calantab(this->antenna());
  //String calantabname = this->antenna().tableName();
  TableCopy::copyRows(calantab,msantab);
  //msantab.deepCopy(calantabname,Table::New,True);
  //copy field table
  CalField calfldtab(this->field());
  TableCopy::copyRows(calfldtab,msfldtab);
  //String calfldtabname = this->field().tableName();
  //msfldtab.deepCopy(calfldtabname,Table::New,True);
  //copy spectralWindow table
  CalSpectralWindow calspwtab(this->spectralWindow());
  TableCopy::copyRows(calspwtab,msspwtab);
  //String calspwtabname = this->spectralWindow().tableName();
  //msspwtab.deepCopy(calspwtabname,Table::New,True);

  this->rwKeywordSet().define(RecordFieldId("MSName"),Path(msName).absoluteName());
}
//----------------------------------------------------------------------------
Bool NewCalTable::conformant(const TableDesc& tabDesc)
{
// Check if input table description is confomrant with
// the new caltable format (or should I named this "validate" ...as 
// in MS case...)
  Bool eqDType=False;
  NewCalTableDesc calTD = NewCalTableDesc();
  TableDesc requiredCalTD = calTD.calMainDesc();
  Bool isCalTableDesc = tabDesc.columnDescSet().isSuperset(requiredCalTD.columnDescSet(), eqDType);
  if (!isCalTableDesc) {
    cerr<<"NewCalTable::confomant tabDesc is not superset of requiredCalMain"<<endl;
  };
  Vector<String> colNames(requiredCalTD.columnNames());
  Vector<String> incolNames(tabDesc.columnNames());
  uInt ncols = colNames.nelements();
  for (uInt j=0; j < ncols; j++) {
  }
  Bool check = True;
  for (uInt i=0; i < ncols; i++) {
    TableRecord keySet = tabDesc[colNames(i)].keywordSet();
    TableRecord reqKeySet = requiredCalTD[colNames(i)].keywordSet();
    if (reqKeySet.isDefined("QuantumUnits")) {
      check = keySet.isDefined("QuantumUnits");
      if (!check) {
        cerr<<"NewCalTable::confomant column:"<<colNames(i)<<" does not have a unit"<<endl;
      }
      else {
        check =  allEQ(keySet.asArrayString("QuantumUnits"), reqKeySet.asArrayString("QuantumUnits"));
        if (!check) {
          cerr<<"NewCalTable::conformant column:"<<colNames(i)
               <<" has an invalid QuantumUnits:"<<keySet.asArrayString("QuantumUnits")<<endl;
        }
      }
    }
  }
  return isCalTableDesc && check;
};

//----------------------------------------------------------------------------
void NewCalTable::writeToDisk(const String& outTableName)
{
  Block<String> sortcols(4);
  sortcols[0]="SPECTRAL_WINDOW_ID"; 
  sortcols[1]="TIME"; 
  sortcols[2]="ANTENNA1"; 
  sortcols[3]="ANTENNA2"; 
  Table sorted = this->sort(sortcols,Sort::Ascending,Sort::HeapSort);
  sorted.deepCopy(outTableName,Table::New);
};


} //# NAMESPACE CASA - END

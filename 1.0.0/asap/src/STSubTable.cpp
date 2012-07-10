//
// C++ Implementation: STSubTable
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/Exceptions/Error.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>

#include "Scantable.h"
#include "STSubTable.h"


using namespace casa;

namespace asap {

STSubTable::STSubTable( const Scantable& parent, const casa::String& name )
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.addColumn(ScalarColumnDesc<uInt>("ID"));
  String tabname = parent.table().tableName()+"/"+name;
  SetupNewTable aNewTab(tabname, td, Table::Scratch);
  table_ = Table(aNewTab, parent.table().tableType());
  idCol_.attach(table_,"ID");

}
STSubTable::STSubTable(Table tab, const String& name)
{
  table_ = tab.rwKeywordSet().asTable(name);
  idCol_.attach(table_,"ID");
}


STSubTable::~STSubTable()
{
}

STSubTable& asap::STSubTable::operator=( const STSubTable& other)
{
  if (&other != this) {
    this->table_ = other.table_;
    idCol_.attach(this->table_,"ID");
  }
  return *this;
}


}

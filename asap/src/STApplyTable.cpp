//
// C++ Implementation: STApplyTable
//
// Description:
//
//
// Author: Takeshi Nakazato
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/Exceptions/Error.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include "Scantable.h"
#include "STApplyTable.h"


using namespace casa;

namespace asap {

STApplyTable::STApplyTable( const Scantable& parent, const casa::String& name )
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.addColumn(ScalarColumnDesc<uInt>("SCANNO"));
  td.addColumn(ScalarColumnDesc<uInt>("CYCLENO"));
  td.addColumn(ScalarColumnDesc<uInt>("BEAMNO"));
  td.addColumn(ScalarColumnDesc<uInt>("IFNO"));
  td.addColumn(ScalarColumnDesc<uInt>("POLNO"));
  td.addColumn(ScalarColumnDesc<Double>("TIME"));
  TableMeasRefDesc measRef(MEpoch::UTC); // UTC as default
  TableMeasValueDesc measVal(td, "TIME");
  TableMeasDesc<MEpoch> mepochCol(measVal, measRef);
  mepochCol.write(td);
  String tabname = parent.table().tableName()+"/"+name;
  SetupNewTable aNewTab(tabname, td, Table::Scratch);
  //table_ = Table(aNewTab, parent.table().tableType());
  table_ = Table(aNewTab, Table::Memory);
  attachBaseColumns();

  table_.rwKeywordSet().define("VERSION", 1);
  table_.rwKeywordSet().define("ScantableName", parent.table().tableName());
  table_.rwKeywordSet().define("ApplyType", "NONE");
  table_.rwKeywordSet().defineTable("FREQUENCIES", parent.frequencies().table());

  table_.tableInfo().setType("ApplyTable");

  originaltable_ = table_;
}


STApplyTable::~STApplyTable()
{
}

void STApplyTable::attach()
{
  attachBaseColumns();
  attachOptionalColumns();
}

void STApplyTable::attachBaseColumns()
{
  scanCol_.attach(table_, "SCANNO");
  cycleCol_.attach(table_, "CYCLENO");
  beamCol_.attach(table_, "BEAMNO");
  ifCol_.attach(table_, "IFNO");
  polCol_.attach(table_, "POLNO");
  timeCol_.attach(table_, "TIME");
  timeMeasCol_.attach(table_, "TIME");
}

void STApplyTable::setSelection(STSelector &sel)
{
  table_ = sel.apply(originaltable_);
  attach();
  sel_ = sel;
}

void STApplyTable::unsetSelection()
{
  table_ = originaltable_;
  attach();
  sel_.reset();
}

void STApplyTable::setbasedata(uInt irow, uInt scanno, uInt cycleno,
                               uInt beamno, uInt ifno, uInt polno, 
                               Double time)
{
  scanCol_.put(irow, scanno);
  cycleCol_.put(irow, cycleno);
  beamCol_.put(irow, beamno);
  ifCol_.put(irow, ifno);
  polCol_.put(irow, polno);
  timeCol_.put(irow, time);
}

}

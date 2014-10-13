//
// C++ Implementation: STApplyTable
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ExprNode.h>
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
  td.addColumn(ScalarColumnDesc<uInt>("FREQ_ID"));
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

STApplyTable::STApplyTable(const String &name)
{
  table_ = Table(name, Table::Update);
  attachBaseColumns();
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
  freqidCol_.attach(table_, "FREQ_ID");
}

void STApplyTable::setSelection(STSelector &sel, bool sortByTime)
{
  table_ = sel.apply(originaltable_);
  if (sortByTime)
    table_.sort("TIME", Sort::Descending);
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
                               uInt freqid, Double time)
{
  scanCol_.put(irow, scanno);
  cycleCol_.put(irow, cycleno);
  beamCol_.put(irow, beamno);
  ifCol_.put(irow, ifno);
  polCol_.put(irow, polno);
  timeCol_.put(irow, time);
  freqidCol_.put(irow, freqid);
}

void STApplyTable::save(const String &name)
{
  assert_<AipsError>(name.size() > 0, "Output name is empty.");  
  table_.deepCopy(name, Table::New);
}

String STApplyTable::caltype()
{
  if (table_.keywordSet().isDefined("ApplyType")) {
    return table_.keywordSet().asString("ApplyType");
  }
  else 
    return "NONE";
}

STCalEnum::CalType  STApplyTable::getCalType(const String &name)
{
  Table t(name, Table::Old);
  return stringToType(t.keywordSet().asString("ApplyType"));
}

STCalEnum::CalType STApplyTable::getCalType(CountedPtr<STApplyTable> tab)
{
  return stringToType(tab->caltype());
}

STCalEnum::CalType STApplyTable::getCalType(STApplyTable *tab)
{
  return stringToType(tab->caltype());
}

STCalEnum::CalType STApplyTable::stringToType(const String &caltype)
{
  if (caltype == "CALSKY_PSALMA")
    return STCalEnum::CalPSAlma;
  else if (caltype == "CALTSYS")
    return STCalEnum::CalTsys;
  else
    return STCalEnum::NoType;
}

Block<Double> STApplyTable::getFrequenciesRow(uInt id)
{
  const TableRecord &rec = table_.keywordSet();
  //rec.print(os_.output());
  //os_ << LogIO::POST;
  Table ftab = rec.asTable("FREQUENCIES");
  Table t = ftab(ftab.col("ID") == id);
  ROTableColumn col(t, "REFPIX");
  Block<Double> r(3);
  r[0] = col.asdouble(0);
  col.attach(t, "REFVAL");
  r[1] = col.asdouble(0);
  col.attach(t, "INCREMENT");
  r[2] = col.asdouble(0);
  return r;
}
}

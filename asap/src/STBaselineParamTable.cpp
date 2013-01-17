//
// C++ Implementation: STBaselineParamTable
//
// Description:
//
//
// Author: Wataru Kawasaki <wataru.kawasaki@nao.ac.jp> (C) 2013
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include <casa/Exceptions/Error.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include "Scantable.h"
#include "STBaselineParamTable.h"


using namespace casa;

namespace asap {

const String STBaselineParamTable::name_ = "APPLY_BASELINE";

STBaselineParamTable::STBaselineParamTable(const Scantable& parent)
  : STApplyTable(parent, name_)
{
  setup();
}

STBaselineParamTable::STBaselineParamTable(const String &name)
  : STApplyTable(name)
{
  attachOptionalColumns();
}

STBaselineParamTable::~STBaselineParamTable()
{
}

void STBaselineParamTable::setup()
{
  table_.addColumn(ScalarColumnDesc<uInt>("BLFUNC"));
  table_.addColumn(ScalarColumnDesc<uInt>("ORDER"));
  table_.addColumn(ArrayColumnDesc<Float>("BOUNDARY"));
  table_.addColumn(ArrayColumnDesc<Float>("PARAMETER"));

  table_.rwKeywordSet().define("ApplyType", "BASELINE");

  attachOptionalColumns();
}

void STBaselineParamTable::attachOptionalColumns()
{
  blfuncCol_.attach(table_, "BLFUNC");
  orderCol_.attach(table_, "ORDER");
  boundaryCol_.attach(table_, "BOUNDARY");
  paramCol_.attach(table_, "PARAMETER");
  
}

void STBaselineParamTable::setdata(uInt irow, uInt scanno, uInt cycleno, 
                             uInt beamno, uInt ifno, uInt polno, uInt freqid,  
                             Double time, uInt blfunc, uInt order, 
                             Vector<Float> boundary, Vector<Float> param)
{
  if (irow >= (uInt)nrow()) {
    throw AipsError("row index out of range");
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STBaselineParamTable","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time);
  blfuncCol_.put(irow, blfunc);
  orderCol_.put(irow, order);
  boundaryCol_.put(irow, boundary);
  paramCol_.put(irow, param);
}

void STBaselineParamTable::appenddata(uInt scanno, uInt cycleno, 
                                uInt beamno, uInt ifno, uInt polno, uInt freqid,
                                Double time, uInt blfunc, uInt order, 
                                Vector<Float> boundary, Vector<Float> param)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time, blfunc, order, boundary, param);
}

  /*
Vector<STBaselineEnum::BaselineType> getBlfunc()
{
  Vector<uInt> rawBlfuncColumn = blfuncCol_.getColumn();
  Vector<STBaselineEnum::BaselineType> blfuncColumn;
  for (uInt i = 0; i < rawBlfuncColumn.size(); ++i) {
    blfuncColumn.append(STBaselineEnum::BaselineType(rawBlfuncColumn.get(i)));
  }
  return blfuncColumn;
}
  */

}

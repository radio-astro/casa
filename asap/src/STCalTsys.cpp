//
// C++ Implementation: STCalTsys
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
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include "Scantable.h"
#include "STCalTsys.h"


using namespace casa;

namespace asap {

const String STCalTsys::name_ = "APPLY_TSYS";

STCalTsys::STCalTsys(const Scantable& parent)
  : STApplyTable(parent, name_)
{
  setup();
}

STCalTsys::~STCalTsys()
{
}

void STCalTsys::setup()
{
  table_.addColumn(ArrayColumnDesc<Float>("TSYS"));
  table_.addColumn(ScalarColumnDesc<Float>("ELEVATION"));

  table_.rwKeywordSet().define("ApplyType", "TSYS");

  attachOptionalColumns();
}

void STCalTsys::attachOptionalColumns()
{
  tsysCol_.attach(table_, "TSYS");
  elCol_.attach(table_,"ELEVATION");
  
}

void STCalTsys::setdata(uInt irow, uInt scanno, uInt cycleno, 
                        uInt beamno, uInt ifno, uInt polno, 
                        Double time, Float elevation, Vector<Float> tsys)
{
  if (irow >= (uInt)nrow()) {
    throw AipsError("row index out of range");
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STCalTsys","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, time);
  elCol_.put(irow, elevation);
  tsysCol_.put(irow, tsys);
}

void STCalTsys::appenddata(uInt scanno, uInt cycleno, 
                           uInt beamno, uInt ifno, uInt polno, 
                           Double time, Float elevation, Vector<Float> tsys)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, time, elevation, tsys);
}
}

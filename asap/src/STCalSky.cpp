//
// C++ Implementation: STCalSky
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
#include <casa/Logging/LogIO.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include "Scantable.h"
#include "STCalSky.h"


using namespace casa;

namespace asap {

const String STCalSky::name_ = "APPLY_SKY";

STCalSky::STCalSky(const Scantable& parent)
  : STApplyTable(parent, name_)
{
  setup();
}

STCalSky::~STCalSky()
{
}

void STCalSky::setup()
{
  table_.addColumn(ArrayColumnDesc<Float>("SPECTRA"));
  table_.addColumn(ScalarColumnDesc<Float>("ELEVATION"));

  table_.rwKeywordSet().define("ApplyType", "SKY");

  attachOptionalColumns();
}

void STCalSky::attachOptionalColumns()
{
  spectraCol_.attach(table_, "SPECTRA");
  elCol_.attach(table_,"ELEVATION");
  
}

void STCalSky::setdata(uInt irow, uInt scanno, uInt cycleno, 
                       uInt beamno, uInt ifno, uInt polno, 
                       Double time, Float elevation, Vector<Float> spectra)
{
  if (irow >= (uInt)nrow()) {
    throw AipsError("row index out of range");
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STCalSky","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, time);
  elCol_.put(irow, elevation);
  spectraCol_.put(irow, spectra);
}

void STCalSky::appenddata(uInt scanno, uInt cycleno, 
                          uInt beamno, uInt ifno, uInt polno, 
                          Double time, Float elevation, Vector<Float> spectra)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, time, elevation, spectra);
}
}

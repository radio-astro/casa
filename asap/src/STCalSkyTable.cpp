//
// C++ Implementation: STCalSkyTable
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

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
#include "STCalSkyTable.h"


using namespace casa;

namespace asap {

const String STCalSkyTable::name_ = "APPLY_SKY";

STCalSkyTable::STCalSkyTable(const Scantable& parent, const String &caltype)
  : STApplyTable(parent, name_),
    caltype_(caltype)
{
  setup();
}

STCalSkyTable::STCalSkyTable(const String &name)
  : STApplyTable(name)
{
  attachOptionalColumns();
}

STCalSkyTable::~STCalSkyTable()
{
}

void STCalSkyTable::setup()
{
  table_.addColumn(ArrayColumnDesc<Float>("SPECTRA"));
  table_.addColumn(ScalarColumnDesc<Float>("ELEVATION"));

  //table_.rwKeywordSet().define("ApplyType", "SKY");
  String caltype = "CALSKY_" + caltype_;
  caltype.upcase();
  table_.rwKeywordSet().define("ApplyType", caltype);

  attachOptionalColumns();
}

void STCalSkyTable::attachOptionalColumns()
{
  spectraCol_.attach(table_, "SPECTRA");
  elCol_.attach(table_,"ELEVATION");
  
}

void STCalSkyTable::setdata(uInt irow, uInt scanno, uInt cycleno, 
                            uInt beamno, uInt ifno, uInt polno, uInt freqid,  
                            Double time, Float elevation, Vector<Float> spectra)
{
  if (irow >= (uInt)nrow()) {
    throw AipsError("row index out of range");
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STCalSkyTable","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time);
  elCol_.put(irow, elevation);
  spectraCol_.put(irow, spectra);
}

void STCalSkyTable::appenddata(uInt scanno, uInt cycleno, 
                               uInt beamno, uInt ifno, uInt polno, uInt freqid,
                               Double time, Float elevation, Vector<Float> spectra)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time, elevation, spectra);
}

uInt STCalSkyTable::nchan(uInt ifno)
{
  STSelector org = sel_;
  STSelector sel;
  sel.setIFs(vector<int>(1,(int)ifno));
  setSelection(sel);
  uInt n = spectraCol_(0).nelements();
  unsetSelection();
  if (!org.empty())
    setSelection(org);
  return n;
}

// Vector<Double> STCalSkyTable::getBaseFrequency(uInt whichrow)
// {
//   assert(whichrow < nrow());
//   uInt freqid = freqidCol_(whichrow);
//   uInt nc = spectraCol_(whichrow).nelements();
//   Block<Double> f = getFrequenciesRow(freqid);
//   Vector<Double> freqs(nc);
//   indgen(freqs, f[1]-f[0]*f[2], f[2]);
//   return freqs;
// }
}

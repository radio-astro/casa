//
// C++ Implementation: STCalTsysTable
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
#include <casa/Utilities/Assert.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableIter.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include "Scantable.h"
#include "STCalTsysTable.h"


using namespace casa;

namespace asap {

const String STCalTsysTable::name_ = "APPLY_TSYS";

STCalTsysTable::STCalTsysTable(const Scantable& parent)
  : STApplyTable(parent, name_)
{
  setup();
}

STCalTsysTable::STCalTsysTable(const String &name)
  : STApplyTable(name)
{
  if (!table_.tableDesc().isColumn("FLAGTRA")) {
    LogIO os(LogOrigin("STCalTsysTable", "STCalTsysTable", WHERE));
    os << "Adding FLAGTRA column to " << name << " with initial value of 0 (all data valid)." << LogIO::POST;
    table_.addColumn(ArrayColumnDesc<uChar>("FLAGTRA"));
    TableIterator iter(table_, "IFNO");
    while (!iter.pastEnd()) {
      Table t = iter.table();
      ArrayColumn<Float> tsysCol(t, "TSYS");
      IPosition shape(2, tsysCol.shape(0)[0], t.nrow());
      ArrayColumn<uChar> flagtraCol(t, "FLAGTRA");
      Array<uChar> flagtra(shape, (uChar)0);
      flagtraCol.putColumn(flagtra);
      iter.next();
    }
  }
  
  attachOptionalColumns();
}

STCalTsysTable::~STCalTsysTable()
{
}

void STCalTsysTable::setup()
{
  table_.addColumn(ArrayColumnDesc<Float>("TSYS"));
  table_.addColumn(ArrayColumnDesc<uChar>("FLAGTRA"));
  table_.addColumn(ScalarColumnDesc<Float>("ELEVATION"));

  table_.rwKeywordSet().define("ApplyType", "CALTSYS");

  attachOptionalColumns();
}

void STCalTsysTable::attachOptionalColumns()
{
  tsysCol_.attach(table_, "TSYS");
  flagtraCol_.attach(table_, "FLAGTRA");
  elCol_.attach(table_,"ELEVATION");
  
}

void STCalTsysTable::setdata(uInt irow, uInt scanno, uInt cycleno, 
                             uInt beamno, uInt ifno, uInt polno, uInt freqid,  
                             Double time, Float elevation,
			     const Vector<Float> &tsys,
			     const Vector<uChar> &flagtra)
{
  if (irow >= (uInt)nrow()) {
    throw AipsError("row index out of range");
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STCalTsysTable","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time);
  elCol_.put(irow, elevation);
  tsysCol_.put(irow, tsys);
  flagtraCol_.put(irow, flagtra);
}

void STCalTsysTable::appenddata(uInt scanno, uInt cycleno, 
                                uInt beamno, uInt ifno, uInt polno, uInt freqid,
                                Double time, Float elevation,
				const Vector<Float> &tsys,
				const Vector<uChar> &flagtra)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time, elevation,
	  tsys, flagtra);
}

uInt STCalTsysTable::nchan(uInt ifno)
{
  STSelector org = sel_;
  STSelector sel;
  sel.setIFs(vector<int>(1,(int)ifno));
  setSelection(sel);
  uInt n = tsysCol_(0).nelements();
  unsetSelection();
  if (!org.empty())
    setSelection(org);
  return n;
}

Vector<Double> STCalTsysTable::getBaseFrequency(uInt whichrow)
{
  //assert(whichrow < nrow());
  assert_<AipsError>(whichrow < nrow(), "row index out of range.");
  uInt freqid = freqidCol_(whichrow);
  uInt nc = tsysCol_(whichrow).nelements();
  Block<Double> f = getFrequenciesRow(freqid);
  Vector<Double> freqs(nc);
  indgen(freqs, f[1]-f[0]*f[2], f[2]);
  return freqs;
}
}

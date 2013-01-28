//
// C++ Implementation: STBaselineTable
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
#include <casa/OS/Path.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include "Scantable.h"
#include "STBaselineTable.h"


using namespace casa;

namespace asap {

const String STBaselineTable::name_ = "APPLY_BASELINE";

STBaselineTable::STBaselineTable(const Scantable& parent)
  : STApplyTable(parent, name_)
{
  setup();
}

STBaselineTable::STBaselineTable(const String &name)
  : STApplyTable(name)
{
  attachOptionalColumns();
}

STBaselineTable::~STBaselineTable()
{
}

void STBaselineTable::setup()
{
  table_.addColumn(ScalarColumnDesc<uInt>("NCHAN"));
  table_.addColumn(ScalarColumnDesc<uInt>("FUNC_TYPE"));
  table_.addColumn(ArrayColumnDesc<uInt>("FUNC_PARAM"));
  table_.addColumn(ArrayColumnDesc<Float>("FUNC_FPARAM"));
  table_.addColumn(ScalarColumnDesc<uInt>("CLIP_ITERATION"));
  table_.addColumn(ScalarColumnDesc<Float>("CLIP_THRESHOLD"));
  table_.addColumn(ArrayColumnDesc<uInt>("MASKLIST"));
  table_.addColumn(ArrayColumnDesc<Float>("RESULT"));
  table_.addColumn(ScalarColumnDesc<Float>("RMS"));

  table_.rwKeywordSet().define("ApplyType", "BASELINE");

  attachOptionalColumns();
}

void STBaselineTable::attachOptionalColumns()
{
  nchanCol_.attach(table_, "NCHAN");
  ftypeCol_.attach(table_, "FUNC_TYPE");
  fparCol_.attach(table_, "FUNC_PARAM");
  ffparCol_.attach(table_, "FUNC_FPARAM");
  citerCol_.attach(table_, "CLIP_ITERATION");
  cthresCol_.attach(table_, "CLIP_THRESHOLD");
  maskCol_.attach(table_, "MASKLIST");
  resCol_.attach(table_, "RESULT");
  rmsCol_.attach(table_, "RMS");
}

void STBaselineTable::save(const std::string &filename)
{
  String inname(filename);
  Path path(inname);
  inname = path.expandedName();
  table_.deepCopy(inname, Table::New);
}

void STBaselineTable::setdata(uInt irow, uInt scanno, uInt cycleno, 
			      uInt beamno, uInt ifno, uInt polno, 
			      uInt freqid, Double time, 
			      uInt nchan, 
			      STBaselineFunc::FuncName ftype, 
			      Vector<uInt> fpar, 
			      Vector<Float> ffpar, 
			      uInt citer, 
			      Float cthres,
			      Vector<uInt> mask,
			      Vector<Float> res,
			      Float rms)
{
  if (irow >= (uInt)nrow()) {
    throw AipsError("row index out of range");
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STBaselineTable","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time);
  nchanCol_.put(irow, nchan);
  ftypeCol_.put(irow, uInt(ftype));
  fparCol_.put(irow, fpar);
  ffparCol_.put(irow, ffpar);
  citerCol_.put(irow, citer);
  cthresCol_.put(irow, cthres);
  maskCol_.put(irow, mask);
  resCol_.put(irow, res);
  rmsCol_.put(irow, rms);
}

void STBaselineTable::appenddata(uInt scanno, uInt cycleno, 
				 uInt beamno, uInt ifno, uInt polno, 
				 uInt freqid, Double time, 
				 uInt nchan, 
				 STBaselineFunc::FuncName ftype, 
				 Vector<uInt> fpar, 
				 Vector<Float> ffpar, 
				 uInt citer, 
				 Float cthres,
				 Vector<uInt> mask,
				 Vector<Float> res,
				 Float rms)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time, 
	  nchan, ftype, fpar, ffpar, citer, cthres, mask, res, rms);
}

Vector<STBaselineFunc::FuncName> STBaselineTable::getFunctionAsString()
{
  Vector<uInt> rawBlfuncColumn = ftypeCol_.getColumn();
  uInt n = rawBlfuncColumn.nelements();
  Vector<STBaselineFunc::FuncName> blfuncColumn(n);
  for (uInt i = 0; i < n; ++i) {
    blfuncColumn[i] = STBaselineFunc::FuncName(rawBlfuncColumn(i));
  }
  return blfuncColumn;
}

uInt STBaselineTable::nchan(uInt ifno)
{
  STSelector org = sel_;
  STSelector sel;
  sel.setIFs(vector<int>(1,(int)ifno));
  setSelection(sel);
  uInt n = nchanCol_(0);
  unsetSelection();
  if (!org.empty())
    setSelection(org);
  return n;
}
}

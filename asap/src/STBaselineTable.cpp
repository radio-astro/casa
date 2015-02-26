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
  table_.addColumn(ScalarColumnDesc<Bool>("APPLY"));
  table_.addColumn(ScalarColumnDesc<uInt>("FUNC_TYPE"));
  table_.addColumn(ArrayColumnDesc<Int>("FUNC_PARAM"));
  table_.addColumn(ArrayColumnDesc<Float>("FUNC_FPARAM"));
  table_.addColumn(ArrayColumnDesc<uInt>("MASKLIST"));
  table_.addColumn(ArrayColumnDesc<Float>("RESULT"));
  table_.addColumn(ScalarColumnDesc<Float>("RMS"));
  table_.addColumn(ScalarColumnDesc<uInt>("NCHAN"));
  table_.addColumn(ScalarColumnDesc<Float>("CLIP_THRESHOLD"));
  table_.addColumn(ScalarColumnDesc<uInt>("CLIP_ITERATION"));
  table_.addColumn(ScalarColumnDesc<Float>("LF_THRESHOLD"));
  table_.addColumn(ScalarColumnDesc<uInt>("LF_AVERAGE"));
  table_.addColumn(ArrayColumnDesc<uInt>("LF_EDGE"));

  table_.rwKeywordSet().define("ApplyType", "BASELINE");

  attachOptionalColumns();
}

void STBaselineTable::attachOptionalColumns()
{
  applyCol_.attach(table_, "APPLY");
  ftypeCol_.attach(table_, "FUNC_TYPE");
  fparCol_.attach(table_, "FUNC_PARAM");
  ffparCol_.attach(table_, "FUNC_FPARAM");
  maskCol_.attach(table_, "MASKLIST");
  resCol_.attach(table_, "RESULT");
  rmsCol_.attach(table_, "RMS");
  nchanCol_.attach(table_, "NCHAN");
  cthresCol_.attach(table_, "CLIP_THRESHOLD");
  citerCol_.attach(table_, "CLIP_ITERATION");
  lfthresCol_.attach(table_, "LF_THRESHOLD");
  lfavgCol_.attach(table_, "LF_AVERAGE");
  lfedgeCol_.attach(table_, "LF_EDGE");
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
			      Bool apply, 
			      STBaselineFunc::FuncName ftype, 
			      Vector<Int> fpar, 
			      Vector<Float> ffpar, 
			      Vector<uInt> mask,
			      Vector<Float> res,
			      Float rms, 
			      uInt nchan, 
			      Float cthres,
			      uInt citer, 
			      Float lfthres, 
			      uInt lfavg, 
			      Vector<uInt> lfedge)
{
  if (irow >= (uInt)nrow()) {
    //throw AipsError("row index out of range");
    stringstream ss;
    ss << "row index out of range[irow=" << irow << "][nrow=" << nrow() << "]";
    throw AipsError(ss.str());
  }

  if (!sel_.empty()) {
    os_.origin(LogOrigin("STBaselineTable","setdata",WHERE));
    os_ << LogIO::WARN << "Data selection is effective. Specified row index may be wrong." << LogIO::POST;
  }  

  setbasedata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time);
  applyCol_.put(irow, apply);
  ftypeCol_.put(irow, uInt(ftype));
  fparCol_.put(irow, fpar);
  ffparCol_.put(irow, ffpar);
  maskCol_.put(irow, mask);
  resCol_.put(irow, res);
  rmsCol_.put(irow, rms);
  nchanCol_.put(irow, nchan);
  cthresCol_.put(irow, cthres);
  citerCol_.put(irow, citer);
  lfthresCol_.put(irow, lfthres);
  lfavgCol_.put(irow, lfavg);
  lfedgeCol_.put(irow, lfedge);
}

void STBaselineTable::appenddata(int scanno, int cycleno, 
				 int beamno, int ifno, int polno, 
				 int freqid, Double time, 
				 bool apply, 
				 STBaselineFunc::FuncName ftype, 
				 int fpar, 
				 vector<float> ffpar, 
				 Vector<uInt> mask,
				 vector<float> res,
				 float rms,
				 int nchan, 
				 float cthres,
				 int citer, 
				 float lfthres, 
				 int lfavg, 
				 vector<int> lfedge)
{
  vector<int> fparam(1);
  fparam[0] = fpar;

  appenddata(scanno, cycleno, beamno, ifno, polno, freqid, time,
	     apply, ftype, fparam, ffpar, mask, res, rms, nchan, 
	     cthres, citer, lfthres, lfavg, lfedge);
}

void STBaselineTable::appenddata(int scanno, int cycleno, 
				 int beamno, int ifno, int polno, 
				 int freqid, Double time, 
				 bool apply, 
				 STBaselineFunc::FuncName ftype, 
				 vector<int> fpar, 
				 vector<float> ffpar, 
				 Vector<uInt> mask,
				 vector<float> res,
				 float rms,
				 int nchan, 
				 float cthres,
				 int citer, 
				 float lfthres, 
				 int lfavg, 
				 vector<int> lfedge)
{
  Vector<Int> fparam(fpar.size());
  for (uInt i = 0; i < fpar.size(); ++i) {
    fparam[i] = fpar[i];
  }
  Vector<uInt> edge(lfedge.size());
  for (uInt i = 0; i < lfedge.size(); ++i) {
    edge[i] = lfedge[i];
  }
  appenddata(uInt(scanno), uInt(cycleno), uInt(beamno), 
	     uInt(ifno), uInt(polno), uInt(freqid), time,
	     Bool(apply), ftype, fparam, Vector<Float>(ffpar), 
	     mask, Vector<Float>(res), Float(rms), uInt(nchan), 
	     Float(cthres), uInt(citer), 
	     Float(lfthres), uInt(lfavg), edge);
}

void STBaselineTable::appenddata(uInt scanno, uInt cycleno, 
				 uInt beamno, uInt ifno, uInt polno, 
				 uInt freqid, Double time, 
				 Bool apply, 
				 STBaselineFunc::FuncName ftype, 
				 Vector<Int> fpar, 
				 Vector<Float> ffpar, 
				 Vector<uInt> mask,
				 Vector<Float> res,
				 Float rms,
				 uInt nchan, 
				 Float cthres,
				 uInt citer, 
				 Float lfthres, 
				 uInt lfavg, 
				 Vector<uInt> lfedge)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, cycleno, beamno, ifno, polno, freqid, time, 
	  apply, ftype, fpar, ffpar, mask, res, rms, 
	  nchan, cthres, citer, lfthres, lfavg, lfedge);
}

void STBaselineTable::appendbasedata(int scanno, int cycleno, 
				     int beamno, int ifno, int polno, 
				     int freqid, Double time)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setbasedata(irow, uInt(scanno), uInt(cycleno), uInt(beamno), uInt(ifno), uInt(polno), uInt(freqid), time);
}

void STBaselineTable::setresult(casa::uInt irow, 
				casa::Vector<casa::Float> res, 
				casa::Float rms)
{
  resCol_.put(irow, res);
  rmsCol_.put(irow, rms);
}

bool STBaselineTable::getApply(int irow)
{
  return (bool)applyCol_.get(irow);
}

void STBaselineTable::setApply(int irow, bool apply)
{
  applyCol_.put(uInt(irow), Bool(apply));
}

Vector<STBaselineFunc::FuncName> STBaselineTable::getFunctionNames()
{
  Vector<uInt> rawBlfuncColumn = ftypeCol_.getColumn();
  uInt n = rawBlfuncColumn.nelements();
  Vector<STBaselineFunc::FuncName> blfuncColumn(n);
  for (uInt i = 0; i < n; ++i) {
    blfuncColumn[i] = STBaselineFunc::FuncName(rawBlfuncColumn(i));
  }
  return blfuncColumn;
}

STBaselineFunc::FuncName STBaselineTable::getFunctionName(int irow)
{
  return STBaselineFunc::FuncName(ftypeCol_.get(irow));
}

std::vector<int> STBaselineTable::getFuncParam(int irow)
{
  Vector<Int> uiparam = fparCol_.get(irow);
  std::vector<int> res(uiparam.size());
  for (uInt i = 0; i < res.size(); ++i) {
    res[i] = (int)uiparam[i];
  }
  return res;
}

std::vector<bool> STBaselineTable::getMask(int irow)
{
  uInt nchan = getNChan(irow);
  Vector<uInt> masklist = maskCol_.get(irow);
  std::vector<int> masklist1(masklist.size());
  for (uInt i = 0; i < masklist1.size(); ++i) {
    masklist1[i] = (int)masklist[i];
  }
  return Scantable::getMaskFromMaskList(nchan, masklist1);
}

uInt STBaselineTable::getNChan(int irow)
{
  return nchanCol_.get(irow);
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

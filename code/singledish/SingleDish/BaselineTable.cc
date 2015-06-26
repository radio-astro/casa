#include <assert.h>

#include <casa/Containers/ValueHolder.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Path.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <singledish/SingleDish/BaselineTable.h>
#include <stdcasa/StdCasa/CasacSupport.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableProxy.h>

namespace casa {

const String BaselineTable::name_ = "APPLY_BASELINE";

BaselineTable::BaselineTable(const MeasurementSet& parent)
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.addColumn(ScalarColumnDesc<uInt>("SCANNO"));
  td.addColumn(ScalarColumnDesc<uInt>("BEAMNO"));
  td.addColumn(ScalarColumnDesc<uInt>("IFNO"));
  td.addColumn(ScalarColumnDesc<uInt>("FREQ_ID"));
  td.addColumn(ScalarColumnDesc<Double>("TIME"));
  TableMeasRefDesc measRef(MEpoch::UTC); // UTC as default
  TableMeasValueDesc measVal(td, "TIME");
  TableMeasDesc<MEpoch> mepochCol(measVal, measRef);
  mepochCol.write(td);
  String tabname = parent.tableName()+"/"+name_;
  SetupNewTable aNewTab(tabname, td, Table::Scratch);
  table_ = Table(aNewTab, Table::Memory);
  attachBaseColumns();

  table_.rwKeywordSet().define("VERSION", 1);
  table_.rwKeywordSet().define("MSName", parent.tableName());
  table_.rwKeywordSet().define("ApplyType", "NONE");
  table_.rwKeywordSet().defineTable("FREQUENCIES", parent.spectralWindow());
  table_.tableInfo().setType("ApplyTable");
  originaltable_ = table_;

  setup();
  sorted_ = false;
}

BaselineTable::BaselineTable(const String &name)
{
  table_ = Table(name, Table::Update);
  attachBaseColumns();
  originaltable_ = table_;
  attachOptionalColumns();
  sorted_ = false;
}

BaselineTable::~BaselineTable()
{
}

void BaselineTable::attach()
{
  attachBaseColumns();
  attachOptionalColumns();
}

void BaselineTable::attachBaseColumns()
{
  scanCol_.attach(table_, "SCANNO");
  beamCol_.attach(table_, "BEAMNO");
  ifCol_.attach(table_, "IFNO");
  timeCol_.attach(table_, "TIME");
  timeMeasCol_.attach(table_, "TIME");
  freqidCol_.attach(table_, "FREQ_ID");
}
void BaselineTable::setup()
{
  table_.addColumn(ArrayColumnDesc<Bool>("APPLY"));
  table_.addColumn(ArrayColumnDesc<uInt>("FUNC_TYPE"));
  table_.addColumn(ArrayColumnDesc<Int>("FUNC_PARAM"));
  table_.addColumn(ArrayColumnDesc<Float>("FUNC_FPARAM"));
  table_.addColumn(ArrayColumnDesc<uInt>("MASKLIST"));
  table_.addColumn(ArrayColumnDesc<Float>("RESULT"));
  table_.addColumn(ArrayColumnDesc<Float>("RMS"));
  table_.addColumn(ScalarColumnDesc<uInt>("NCHAN"));
  table_.addColumn(ArrayColumnDesc<Float>("CLIP_THRESHOLD"));
  table_.addColumn(ArrayColumnDesc<uInt>("CLIP_ITERATION"));
  table_.addColumn(ArrayColumnDesc<Bool>("USE_LF"));
  table_.addColumn(ArrayColumnDesc<Float>("LF_THRESHOLD"));
  table_.addColumn(ArrayColumnDesc<uInt>("LF_AVERAGE"));
  table_.addColumn(ArrayColumnDesc<uInt>("LF_EDGE"));

  table_.rwKeywordSet().define("ApplyType", "BASELINE");

  attachOptionalColumns();
}

void BaselineTable::attachOptionalColumns()
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
  uselfCol_.attach(table_, "USE_LF");
  lfthresCol_.attach(table_, "LF_THRESHOLD");
  lfavgCol_.attach(table_, "LF_AVERAGE");
  lfedgeCol_.attach(table_, "LF_EDGE");

  ftypeCol_.rwKeywordSet().define("Polynomial",   (uInt)LIBSAKURA_SYMBOL(BaselineType_kPolynomial));
  ftypeCol_.rwKeywordSet().define("Chebyshev",    (uInt)LIBSAKURA_SYMBOL(BaselineType_kChebyshev));
  ftypeCol_.rwKeywordSet().define("Cubic Spline", (uInt)LIBSAKURA_SYMBOL(BaselineType_kCubicSpline));
  ftypeCol_.rwKeywordSet().define("Sinusoid",     (uInt)LIBSAKURA_SYMBOL(BaselineType_kSinusoid));
}

void BaselineTable::save(const std::string &filename)
{
  String inname(filename);
  Path path(inname);
  inname = path.expandedName();
  table_.deepCopy(inname, Table::New);
}

bool BaselineTable::getApply(uInt irow, uInt ipol) const
{
  Vector<Bool> apply = applyCol_.get(irow);
  return static_cast<bool>(apply[ipol]);
}

uint BaselineTable::getBaselineType(uInt irow, uInt ipol) const
{
  Vector<uInt> ftype = ftypeCol_.get(irow);
  return static_cast<uint>(ftype[ipol]);
}

int BaselineTable::getFPar(uInt irow, uInt ipol) const
{
  Vector<Int> fpar = fparCol_.get(irow);
  return static_cast<int>(fpar[ipol]);
}

double BaselineTable::getTimeTimeSorted(uInt irow)
{
  if (!sorted_) {
    timeSortedTable_ = table_.sort("TIME");
  }

  TableProxy tst(timeSortedTable_);
  double *time = reinterpret_cast<double *>(fromValueHolder(tst.getCell("TIME", irow)));
  return *time;
}
   
void BaselineTable::getIdsTimeSorted(uInt irow, uInt *scanno, 
				     uInt *beamno, uInt *ifno)
{
  if (!sorted_) {
    timeSortedTable_ = table_.sort("TIME");
  }

  TableProxy tst(timeSortedTable_);

  scanno  = (uInt *)fromValueHolder(tst.getCell("SCANNO", irow));
  beamno  = (uInt *)fromValueHolder(tst.getCell("BEAMNO", irow));
  ifno    = (uInt *)fromValueHolder(tst.getCell("IFNO", irow));
}

void BaselineTable::getDataTimeSorted(uInt irow, uInt *scanno, 
				      uInt *beamno, uInt *ifno, 
				      Double *time, 
				      Array<Bool> *apply,
				      Array<uInt> *ftype, 
				      Array<Int> *fpar, 
				      Array<Float> *ffpar, 
				      Array<uInt> *mask,
				      Array<Float> *res
				      )
{
  if (!sorted_) {
    timeSortedTable_ = table_.sort("TIME");
  }

  TableProxy tst(timeSortedTable_);

  scanno  = (uInt *)fromValueHolder(tst.getCell("SCANNO", irow));
  beamno  = (uInt *)fromValueHolder(tst.getCell("BEAMNO", irow));
  ifno    = (uInt *)fromValueHolder(tst.getCell("IFNO", irow));
  time    = (Double *)fromValueHolder(tst.getCell("TIME", irow));
  apply   = (Array<Bool> *)fromValueHolder(tst.getCell("APPLY", irow));
  ftype   = (Array<uInt> *)fromValueHolder(tst.getCell("FUNC_TYPE", irow));
  fpar    = (Array<Int> *)fromValueHolder(tst.getCell("FUNC_PARAM", irow));
  ffpar   = (Array<Float> *)fromValueHolder(tst.getCell("FUNC_FPARAM", irow));
  mask    = (Array<uInt> *)fromValueHolder(tst.getCell("MASKLIST", irow));
  res     = (Array<Float> *)fromValueHolder(tst.getCell("RESULT", irow));
}

void BaselineTable::setbasedata(uInt irow, uInt scanno, 
                                uInt beamno, uInt ifno, 
                                uInt freqid, Double time)
{
  scanCol_.put(irow, scanno);
  beamCol_.put(irow, beamno);
  ifCol_.put(irow, ifno);
  timeCol_.put(irow, time);
  freqidCol_.put(irow, freqid);
}

void BaselineTable::setdata(uInt irow, uInt scanno, 
			    uInt beamno, uInt ifno, 
			    uInt freqid, Double time, 
			    Array<Bool> apply, 
		            Array<uInt> ftype,
		            Array<Int> fpar, 
		            Array<Float> ffpar, 
		            Array<uInt> mask,
			    Array<Float> res,
		            Array<Float> rms, 
		            uInt nchan, 
		            Array<Float> cthres,
		            Array<uInt> citer, 
			    Array<Bool> uself, 
		            Array<Float> lfthres, 
		            Array<uInt> lfavg, 
		            Array<uInt> lfedge)
{
  if (irow >= (uInt)nrow()) {
    stringstream ss;
    ss << "row index out of range[irow=" << irow << "][nrow=" << nrow() << "]";
    throw AipsError(ss.str());
  }

  setbasedata(irow, scanno, beamno, ifno, freqid, time);
  applyCol_.put(irow, apply);
  ftypeCol_.put(irow, ftype);
  fparCol_.put(irow, fpar);
  ffparCol_.put(irow, ffpar);
  maskCol_.put(irow, mask);
  resCol_.put(irow, res);
  rmsCol_.put(irow, rms);
  nchanCol_.put(irow, nchan);
  cthresCol_.put(irow, cthres);
  citerCol_.put(irow, citer);
  uselfCol_.put(irow, uself);
  lfthresCol_.put(irow, lfthres);
  lfavgCol_.put(irow, lfavg);
  lfedgeCol_.put(irow, lfedge);
}

void BaselineTable::appenddata(uInt scanno, 
			       uInt beamno, uInt ifno, 
			       uInt freqid, Double time, 
			       Array<Bool> apply, 
			       Array<uInt> ftype, 
			       Array<Int> fpar, 
			       Array<Float> ffpar, 
			       Array<uInt> mask,
			       Array<Float> res,
			       Array<Float> rms,
			       uInt nchan, 
			       Array<Float> cthres,
			       Array<uInt> citer, 
			       Array<Bool> uself, 
			       Array<Float> lfthres, 
			       Array<uInt> lfavg, 
			       Array<uInt> lfedge)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setdata(irow, scanno, beamno, ifno, freqid, time, 
	  apply, ftype, fpar, ffpar, mask, res, rms, 
	  nchan, cthres, citer, uself, lfthres, lfavg, lfedge);
}

void BaselineTable::appendbasedata(int scanno, 
				   int beamno, int ifno, 
				   int freqid, Double time)
{
  uInt irow = nrow();
  table_.addRow(1, True);
  setbasedata(irow, uInt(scanno), uInt(beamno), uInt(ifno), uInt(freqid), time);
}

void BaselineTable::setresult(uInt irow, 
			      Vector<Float> res, 
			      Array<Float> rms)
{
  resCol_.put(irow, res);
  rmsCol_.put(irow, rms);
}

/*
bool BaselineTable::getApply(int irow, int ipol)
{
  //return (bool)applyCol_.get(irow);
  return (bool)((applyCol_.get(irow))[0][0][ipol]);
}

void BaselineTable::setApply(int irow, int ipol, bool apply)
{
  //applyCol_.put(uInt(irow), Bool(apply));
}

Vector<BaselineType> BaselineTable::getFunctionNames()
{
  Vector<uInt> rawBlfuncColumn = ftypeCol_.getColumn();
  uInt n = rawBlfuncColumn.nelements();
  Vector<BaselineType> blfuncColumn(n);
  for (uInt i = 0; i < n; ++i) {
    blfuncColumn[i] = BaselineType(rawBlfuncColumn(i));
  }
  return blfuncColumn;
}

BaselineType BaselineTable::getFunctionName(int irow, int ipol)
{
  return BaselineType(ftypeCol_.get(irow)[ipol]);
}

std::vector<int> BaselineTable::getFuncParam(int irow, int ipol)
{
  Vector<Int> uiparam = fparCol_.get(irow)[ipol];
  std::vector<int> res(uiparam.size());
  for (uInt i = 0; i < res.size(); ++i) {
    res[i] = (int)uiparam[i];
  }
  return res;
}

std::vector<bool> BaselineTable::getMask(int irow, int ipol)
{
  uInt nchan = getNChan(irow);
  Vector<uInt> masklist = maskCol_.get(irow);
  std::vector<int> masklist1(masklist.size());
  for (uInt i = 0; i < masklist1.size(); ++i) {
    masklist1[i] = (int)masklist[i];
  }
  return getMaskFromMaskList(nchan, masklist1);
}
*/
uInt BaselineTable::getNChan(int irow)
{
  return nchanCol_.get(irow);
}

uInt BaselineTable::nchan(uInt ifno)
{
  uInt tmp = ifno;
  return tmp;
}

std::vector<bool> BaselineTable::getMaskFromMaskList(uInt const nchan, std::vector<int> const& masklist)
{
  if (masklist.size() % 2 != 0) {
    throw(AipsError("masklist must have even number of elements."));
  }

  std::vector<bool> res((int)nchan);

  for (int i = 0; i < (int)nchan; ++i) {
    res[i] = false;
  }
  for (uInt j = 0; j < masklist.size(); j += 2) {
    for (int i = masklist[j]; i <= min((Int)nchan-1, (Int)masklist[j+1]); ++i) {
      res[i] = true;
    }
  }

  return res;
}
}

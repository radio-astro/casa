//
// C++ Implementation: STApplyCal
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

#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Utilities/Sort.h>
#include <casa/Utilities/Assert.h>
#include <tables/Tables/Table.h>

#include "Scantable.h"
#include "STApplyCal.h"
#include "STApplyTable.h"
#include "STCalTsysTable.h"
#include "STCalSkyTable.h"
#include "STCalEnum.h"
#include "STIdxIter.h"
#include "Calibrator.h"
#include "PSAlmaCalibrator.h"
#include "Interpolator1D.h"
#include "NearestInterpolator1D.h"
#include "BufferedLinearInterpolator1D.h"
#include "PolynomialInterpolator1D.h"
#include "CubicSplineInterpolator1D.h"
#include <atnf/PKSIO/SrcType.h>


using namespace casa;
using namespace std;

namespace {
template<class Accessor, class Type>
class AccessorInterface
{
public:
  typedef Type TableType;
  static void GetSortedData(const vector<TableType *> &tablelist,
			   const Vector<uInt> &tableIndex,
			   const uInt nrow,
			   const uInt nchan,
			   Vector<Double> &time,
			   Matrix<Float> &data,
			   Matrix<uChar> &flag)
  {
    Vector<Double> timeUnsorted;
    Matrix<Float> dataUnsorted;
    Matrix<uChar> flagUnsorted;
    GetFromTable(tablelist, tableIndex, nrow, nchan,
		 timeUnsorted, dataUnsorted, flagUnsorted);
    SortData(timeUnsorted, dataUnsorted, flagUnsorted,
	     time, data, flag);
  }
private:
  static void GetFromTable(const vector<TableType *> &tableList,
			   const Vector<uInt> &tableIndex,
			   const uInt nrow,
			   const uInt nchan,
			   Vector<Double> &time,
			   Matrix<Float> &data,
			   Matrix<uChar> &flag)
  {
    time.resize(nrow, False);
    const IPosition shape(2, nrow, nchan);
    data.resize(shape, False);
    flag.resize(shape, False);
    uInt rowIndex = 0;
    for (uInt i = 0 ; i < tableIndex.nelements(); i++) {
      TableType *p = tableList[tableIndex[i]];
      Vector<Double> t = Accessor::GetTime(p);
      Matrix<Float> dt = Accessor::GetData(p);
      Matrix<uChar> fl = Accessor::GetFlag(p);
      for (uInt j = 0; j < t.nelements(); j++) {
	time[rowIndex] = t[j];
	data.row(rowIndex) = dt.column(j);
	flag.row(rowIndex) = fl.column(j);
	rowIndex++;
      }
    }    
  }

  static Vector<uInt> IndexSort(const Vector<Double> &t)
  {
    Sort sort;
    sort.sortKey(&t[0], TpDouble, 0, Sort::Ascending);
    Vector<uInt> idx;
    sort.sort(idx, t.nelements(), Sort::QuickSort|Sort::NoDuplicates);
    return idx;
  }

  static void SortData(const Vector<Double> &key, const Matrix<Float> &data,
			 const Matrix<uChar> &flag,
			 Vector<Double> &sortedKey, Matrix<Float> &sortedData,
			 Matrix<uChar> &sortedFlag)
  {
    Vector<uInt> sortIndex = IndexSort(key);
    uInt len = sortIndex.nelements();
    IPosition shape = data.shape();
    shape[0] = len;
    Int64 nelements = shape.product();
    sortedKey.takeStorage(IPosition(1, len), new Double[len], TAKE_OVER);
    sortedData.takeStorage(shape, new Float[nelements], TAKE_OVER);
    sortedFlag.takeStorage(shape, new uChar[nelements], TAKE_OVER);
    for (uInt i = 0 ; i < len; i++) {
      sortedKey[i] = key[sortIndex[i]];
    }
    for (uInt i = 0; i < len; ++i) {
      sortedData.row(i) = data.row(sortIndex[i]);
      sortedFlag.row(i) = flag.row(sortIndex[i]);
    }
  }

};

class SkyTableAccessor : public AccessorInterface<SkyTableAccessor, asap::STCalSkyTable>
{
public:
  static Vector<Double> GetTime(const TableType *t) {return t->getTime();}
  static Matrix<Float> GetData(const TableType *t) {return t->getSpectra();}
  static Matrix<uChar> GetFlag(const TableType *t) {return t->getFlagtra();}
};

class TsysTableAccessor : public AccessorInterface<TsysTableAccessor, asap::STCalTsysTable>
{
public:
  static Vector<Double> GetTime(const TableType *t) {return t->getTime();}
  static Matrix<Float> GetData(const TableType *t) {return t->getTsys();}
  static Matrix<uChar> GetFlag(const TableType *t) {return t->getFlagtra();}
};

inline uInt setupWorkingData(const uInt n, const Double *xin, const Float *yin,
		      const uChar *f, Double *xout, Float *yout)
{
  uInt nValid = 0;
  for (uInt i = 0; i < n; ++i) {
    if (f[i] == 0) {
      xout[nValid] = xin[i];
      yout[nValid] = yin[i];
      nValid++;
    }
  }
  return nValid;
}

template<class InterpolationHelperImpl>
class InterpolationHelperInterface
{
public:
  static void Interpolate(const Double xref, const uInt nx, const uInt ny,
			  Double *xin, Float *yin, uChar *fin,
			  asap::Interpolator1D<Double, Float> *interpolator,
			  Double *xwork, Float *ywork,
			  Float *yout, uChar *fout)
  {
    for (uInt i = 0; i < ny; i++) {
      Float *tmpY = &(yin[i * nx]);
      uInt wnrow = setupWorkingData(nx, xin, tmpY, &(fin[i * nx]), xwork, ywork);
      if (wnrow > 0) {
	// any valid reference data
	InterpolationHelperImpl::ProcessValid(xref, i, interpolator,
					      xwork, ywork, wnrow,
					      yout, fout);
      }
      else {
	// no valid reference data
	InterpolationHelperImpl::ProcessInvalid(xref, i, interpolator,
						xin, tmpY, nx,
						yout, fout);
      }
    }
  }
};

class SkyInterpolationHelper : public InterpolationHelperInterface<SkyInterpolationHelper>
{
public:
  static void ProcessValid(const Double xref, const uInt index,
			   asap::Interpolator1D<Double, Float> *interpolator,
			   Double *xwork, Float *ywork,
			   const uInt wnrow, Float *yout, uChar *fout)
  {
    interpolator->setData(xwork, ywork, wnrow);
    yout[index] = interpolator->interpolate(xref);
  }
  static void ProcessInvalid(const Double xref, const uInt index,
			     asap::Interpolator1D<Double, Float> *interpolator,
			     Double *xwork, Float *ywork,
			     const uInt wnrow, Float *yout, uChar *fout)
  {
    // interpolate data regardless of flag
    ProcessValid(xref, index, interpolator, xwork, ywork, wnrow, yout, fout);
    // flag this channel for calibrated data
    fout[index] = 1 << 7; // user flag
  }
};

class TsysInterpolationHelper : public InterpolationHelperInterface<TsysInterpolationHelper>
{
public:
  static void ProcessValid(const Double xref, const uInt index,
			   asap::Interpolator1D<Double, Float> *interpolator,
			   Double *xwork, Float *ywork,
			   const uInt wnrow, Float *yout, uChar *fout)
  {
    interpolator->setData(xwork, ywork, wnrow);
    yout[index] = interpolator->interpolate(xref);
    fout[index] = 0;
  }
  static void ProcessInvalid(const Double xref, const uInt index,
			     asap::Interpolator1D<Double, Float> *interpolator,
			     Double *xwork, Float *ywork,
			     const uInt wnrow, Float *yout, uChar *fout)
  {
    fout[index] = 1 << 7; // user flag
  }
};

inline void flagSpectrum(uInt &flagrow, Vector<uChar> &flagchan) {
  flagrow = 1;
  flagchan = (uChar)(1 << 7);
}

inline void flagSpectra(Table &tab, const Vector<uInt> &rows) {
  //os_ << LogIO::WARN << "No valid sky data in sky caltable. Completely flag Beam " << beamno << " Spw " << ifno << " Pol " << polno << LogIO::POST;
  // Given sky tables are all empty
  // So, flag all data
  //Table tab = work_->table();
  ArrayColumn<uChar> flCol(tab, "FLAGTRA");
  ScalarColumn<uInt> frCol(tab, "FLAGROW");
  Vector<uChar> flag;
  uInt flagrow;
  for (uInt i = 0; i < rows.nelements(); i++) {
    uInt irow = rows[i];
    flCol.get(irow, flag);
    frCol.get(irow, flagrow);
    flagSpectrum(flagrow, flag);
    //flag = (uChar)(1 << 7);
    flCol.put(irow, flag);
    frCol.put(irow, flagrow);
  }
}

}

namespace asap {
STApplyCal::STApplyCal()
{
  init();
}

STApplyCal::STApplyCal(CountedPtr<Scantable> target)
  : target_(target)
{
  init();
}

STApplyCal::~STApplyCal()
{
}

void STApplyCal::init()
{
  caltype_ = STCalEnum::NoType;
  doTsys_ = False;
  iTime_ = STCalEnum::DefaultInterpolation;
  iFreq_ = STCalEnum::DefaultInterpolation;
}

void STApplyCal::reset()
{
  // call init
  init();

  // clear apply tables
  // do not delete object here
  skytable_.resize(0);
  tsystable_.resize(0);

  // clear mapping for Tsys transfer
  spwmap_.clear();

  // reset selector
  sel_.reset();
  
  // delete interpolators
  interpolatorT_ = 0;
  interpolatorS_ = 0;
  interpolatorF_ = 0;

  // clear working scantable
  work_ = 0;
  
  // clear calibrator
  calibrator_ = 0;
}

void STApplyCal::completeReset()
{
  reset();
  target_ = 0;
}

void STApplyCal::setTarget(CountedPtr<Scantable> target)
{
  target_ = target;
}

void STApplyCal::setTarget(const String &name)
{
  // always create PlainTable
  target_ = new Scantable(name, Table::Plain);
}

void STApplyCal::push(STCalSkyTable *table)
{
  os_.origin(LogOrigin("STApplyCal","push",WHERE));
  skytable_.push_back(table);
  STCalEnum::CalType caltype = STApplyTable::getCalType(table);
  os_ << "caltype=" <<  caltype << LogIO::POST;
  if (caltype_ == STCalEnum::NoType || 
      caltype_ == STCalEnum::DefaultType ||
      caltype_ == STCalEnum::CalTsys) {
    caltype_ = caltype;
  }
  os_ << "caltype_=" << caltype_ << LogIO::POST;
}

void STApplyCal::push(STCalTsysTable *table)
{
  tsystable_.push_back(table);
  doTsys_ = True;
}

void STApplyCal::setTimeInterpolation(STCalEnum::InterpolationType itype, Int order)
{
  iTime_ = itype;
  order_ = order;
}

void STApplyCal::setFrequencyInterpolation(STCalEnum::InterpolationType itype, Int order)
{
  iFreq_ = itype;
  order_ = order;
}

void STApplyCal::setTsysTransfer(uInt from, Vector<uInt> to)
{
  os_.origin(LogOrigin("STApplyCal","setTsysTransfer",WHERE));
  os_ << "from=" << from << ", to=" << to << LogIO::POST;
  map<uInt, Vector<uInt> >::iterator i = spwmap_.find(from);
  if (i == spwmap_.end()) {
    spwmap_.insert(pair<uInt, Vector<uInt> >(from, to));
  }
  else {
    Vector<uInt> toNew = i->second;
    spwmap_.erase(i);
    uInt k = toNew.nelements();
    toNew.resize(k+to.nelements(), True);
    for (uInt i = 0; i < to.nelements(); i++)
      toNew[i+k] = to[i];
    spwmap_.insert(pair<uInt, Vector<uInt> >(from, toNew));
  }
}

void STApplyCal::apply(Bool insitu, Bool filltsys)
{
  os_.origin(LogOrigin("STApplyCal","apply",WHERE));
  
  //assert(!target_.null());
  assert_<AipsError>(!target_.null(),"You have to set target scantable first.");

  // calibrator
  if (caltype_ == STCalEnum::CalPSAlma)
    calibrator_ = new PSAlmaCalibrator();

  // interpolator
  initInterpolator();

  // select data
  sel_.reset();
  sel_ = target_->getSelection();
  if (caltype_ == STCalEnum::CalPSAlma ||
      caltype_ == STCalEnum::CalPS) {
    sel_.setTypes(vector<int>(1,(int)SrcType::PSON));
  }
  target_->setSelection(sel_);

  //os_ << "sel_.print()=" << sel_.print() << LogIO::POST;

  // working data
  if (insitu) {
    os_.origin(LogOrigin("STApplyCal","apply",WHERE));
    os_ << "Overwrite input scantable" << LogIO::POST;
    work_ = target_;
  }
  else {
    os_.origin(LogOrigin("STApplyCal","apply",WHERE));
    os_ << "Create output scantable from input" << LogIO::POST;
    work_ = new Scantable(*target_, false);
  }

  //os_ << "work_->nrow()=" << work_->nrow() << LogIO::POST;

  // list of apply tables for sky calibration
  Vector<uInt> skycalList(skytable_.size());
  uInt numSkyCal = 0;

  // list of apply tables for Tsys calibration
  for (uInt i = 0 ; i < skytable_.size(); i++) {
    STCalEnum::CalType caltype = STApplyTable::getCalType(skytable_[i]);
    if (caltype == caltype_) {
      skycalList[numSkyCal] = i;
      numSkyCal++;
    }
  }
  skycalList.resize(numSkyCal, True);


  vector<string> cols( 3 ) ;
  cols[0] = "BEAMNO" ;
  cols[1] = "POLNO" ;
  cols[2] = "IFNO" ;
  CountedPtr<STIdxIter2> iter = new STIdxIter2(work_, cols) ;
  double start = mathutil::gettimeofday_sec();
  os_ << LogIO::DEBUGGING << "start iterative doapply: " << start << LogIO::POST;
  while (!iter->pastEnd()) {
    Record ids = iter->currentValue();
    Vector<uInt> rows = iter->getRows(SHARE);
    if (rows.nelements() > 0)
      doapply(ids.asuInt("BEAMNO"), ids.asuInt("IFNO"), ids.asuInt("POLNO"), rows, skycalList, filltsys);
    iter->next();
  }
  double end = mathutil::gettimeofday_sec();
  os_ << LogIO::DEBUGGING << "end iterative doapply: " << end << LogIO::POST;
  os_ << LogIO::DEBUGGING << "elapsed time for doapply: " << end - start << " sec" << LogIO::POST;

  target_->unsetSelection();
}

void STApplyCal::doapply(uInt beamno, uInt ifno, uInt polno, 
                         Vector<uInt> &rows,
                         Vector<uInt> &skylist, 
                         Bool filltsys)
{
  os_.origin(LogOrigin("STApplyCal","doapply",WHERE));
  Bool doTsys = doTsys_;

  STSelector sel;
  vector<int> id(1);
  id[0] = beamno;
  sel.setBeams(id);
  id[0] = ifno;
  sel.setIFs(id);
  id[0] = polno;
  sel.setPolarizations(id);  

  // apply selection to apply tables
  uInt nrowSkyTotal = 0;
  uInt nrowTsysTotal = 0;
  for (uInt i = 0; i < skylist.nelements(); i++) {
    skytable_[skylist[i]]->setSelection(sel);
    nrowSkyTotal += skytable_[skylist[i]]->nrow();
    os_ << "nrowSkyTotal=" << nrowSkyTotal << LogIO::POST;
  }

  // Skip IFNO without sky data
  if (nrowSkyTotal == 0) {
    if (skytable_.size() > 0) {
      os_ << LogIO::WARN << "No data in sky caltable. Completely flag Beam " << beamno << " Spw " << ifno << " Pol " << polno << LogIO::POST;
      // Given sky tables are all empty
      // So, flag all data
      flagSpectra(work_->table(), rows);
    }
    return;
  }

  uInt nchanTsys = 0;
  Vector<Double> ftsys;
  uInt tsysifno = getIFForTsys(ifno);
  Bool onlyInvalidTsys = False;
  os_ << "tsysifno=" << (Int)tsysifno << LogIO::POST;
  if (tsystable_.size() == 0) {
    os_.origin(LogOrigin("STApplyTable", "doapply", WHERE));
    os_ << "No Tsys tables are given. Skip Tsys calibratoin." << LogIO::POST;
    doTsys = False;
  }
  else if (tsysifno == (uInt)-1) {
    os_.origin(LogOrigin("STApplyTable", "doapply", WHERE));
    os_ << "No corresponding Tsys for IFNO " << ifno << ". Skip Tsys calibration" << LogIO::POST;
    doTsys = False;
  }
  else {
    id[0] = (int)tsysifno;
    sel.setIFs(id);
    for (uInt i = 0; i < tsystable_.size() ; i++) {
      tsystable_[i]->setSelection(sel);
      uInt nrowThisTsys = tsystable_[i]->nrow();
      nrowTsysTotal += nrowThisTsys;
      if (nrowThisTsys > 0 && nchanTsys == 0) {
	nchanTsys = tsystable_[i]->nchan(tsysifno);
	ftsys = tsystable_[i]->getBaseFrequency(0);
      }
    }
    if (nrowTsysTotal == 0) {
      os_ << "No valid Tsys measurement. for Beam " << beamno << " Spw " << ifno << " Pol " << polno << ". Skip Tsys calibration." << LogIO::POST;
      doTsys = False;
      onlyInvalidTsys = True;
    }
  }

  uInt nchanSp = skytable_[skylist[0]]->nchan(ifno);
  uInt nrowSky = nrowSkyTotal;
  Vector<Double> timeSky;
  Matrix<Float> spoff;
  Matrix<uChar> flagoff;
  SkyTableAccessor::GetSortedData(skytable_, skylist,
				  nrowSkyTotal, nchanSp,
				  timeSky, spoff, flagoff);
  if (allNE(flagoff, (uChar)0)) {
    os_ << LogIO::WARN << "No valid sky data in sky caltable. Completely flag Beam " << beamno << " Spw " << ifno << " Pol " << polno << LogIO::POST;
    // Given sky tables are all invalid
    // So, flag all data
    flagSpectra(work_->table(), rows);
  }
  nrowSky = timeSky.nelements();

  uInt nrowTsys = nrowTsysTotal;
  Vector<Double> timeTsys;
  Matrix<Float> tsys;
  Matrix<uChar> flagtsys;
  if (doTsys) {
    //os_ << "doTsys" << LogIO::POST;
    Vector<uInt> tsyslist(tsystable_.size());
    indgen(tsyslist);
    TsysTableAccessor::GetSortedData(tsystable_, tsyslist,
				     nrowTsysTotal, nchanTsys,
				     timeTsys, tsys, flagtsys);
    nrowTsys = timeTsys.nelements();

    if (allNE(flagtsys, (uChar)0)) {
      os_ << LogIO::WARN << "No valid Tsys measurement for Beam " << beamno << " Spw " << ifno << " Pol " << polno << ". Skip Tsys calibration." << LogIO::POST;
      doTsys = False;
      onlyInvalidTsys = True;
    }
  }

  Table tab = work_->table();
  ArrayColumn<Float> spCol(tab, "SPECTRA");
  ArrayColumn<uChar> flCol(tab, "FLAGTRA");
  ArrayColumn<Float> tsysCol(tab, "TSYS");
  ScalarColumn<Double> timeCol(tab, "TIME");
  ScalarColumn<uInt> frCol(tab, "FLAGROW");

  // Array for scaling factor (aka Tsys)
  Vector<Float> iTsys(IPosition(1, nchanSp), new Float[nchanSp], TAKE_OVER);
  // Array for Tsys interpolation
  // This is empty array and is never referenced if doTsys == false
  // (i.e. nchanTsys == 0)
  Vector<Float> iTsysT(IPosition(1, nchanTsys), new Float[nchanTsys], TAKE_OVER);

  // Array for interpolated off spectrum
  Vector<Float> iOff(IPosition(1, nchanSp), new Float[nchanSp], TAKE_OVER);

  // working array for interpolation with flags
  uInt arraySize = max(max(nrowTsys, nchanTsys), nrowSky);
  Vector<Double> xwork(IPosition(1, arraySize), new Double[arraySize], TAKE_OVER);
  Vector<Float> ywork(IPosition(1, arraySize), new Float[arraySize], TAKE_OVER);
  Vector<uChar> fwork(IPosition(1, nchanTsys), new uChar[nchanTsys], TAKE_OVER);

  // data array 
  Vector<Float> on(IPosition(1, nchanSp), new Float[nchanSp], TAKE_OVER);
  Vector<uChar> flag(on.shape(), new uChar[on.shape().product()], TAKE_OVER);
  
  for (uInt i = 0; i < rows.nelements(); i++) {
    //os_ << "start i = " << i << " (row = " << rows[i] << ")" << LogIO::POST;
    uInt irow = rows[i];

    // target spectral data
    spCol.get(irow, on);
    flCol.get(irow, flag);
    uInt flagrow = frCol(irow);
    //os_ << "on=" << on[0] << LogIO::POST;
    calibrator_->setSource(on);

    // interpolation
    Double t0 = timeCol(irow);
    Double *xwork_p = xwork.data();
    Float *ywork_p = ywork.data();
    SkyInterpolationHelper::Interpolate(t0, nrowSky, nchanSp,
					timeSky.data(), spoff.data(),
					flagoff.data(), &(*interpolatorS_),
					xwork_p, ywork_p, 
					iOff.data(), flag.data());
    calibrator_->setReference(iOff);
    
    if (doTsys) {
      // Tsys correction
      // interpolation on time axis
      TsysInterpolationHelper::Interpolate(t0, nrowTsys, nchanTsys,
					   timeTsys.data(), tsys.data(),
					   flagtsys.data(), &(*interpolatorT_),
					   xwork_p, ywork_p,
					   iTsysT.data(), fwork.data());
      uChar *fwork_p = fwork.data();
      if (nchanSp == 1) {
        // take average
        iTsys[0] = mean(iTsysT);
      }
      else {
        // interpolation on frequency axis
        Vector<Double> fsp = getBaseFrequency(rows[i]);
	uInt wnchan = setupWorkingData(nchanTsys, ftsys.data(), iTsysT.data(),
				       fwork_p, xwork_p, ywork_p);
	assert(wnchan > 0);
	if (wnchan == 0) {
	  throw AipsError("No valid Tsys measurements.");
	}
	interpolatorF_->setData(xwork_p, ywork_p, wnchan);
	for (uInt ichan = 0; ichan < nchanSp; ichan++) {
          iTsys[ichan] = interpolatorF_->interpolate(fsp[ichan]);
        }
      }
    }
    else {
      Vector<Float> tsysInRow = tsysCol(irow);
      if (tsysInRow.nelements() == 1) {
        iTsys = tsysInRow[0];
      }
      else {
        for (uInt ichan = 0; ichan < tsysInRow.nelements(); ++ichan)
          iTsys[ichan] = tsysInRow[ichan];
      }
    } 
    //os_ << "iTsys=" << iTsys[0] << LogIO::POST;
    calibrator_->setScaler(iTsys);
  
    // do calibration
    calibrator_->calibrate();

    // flag spectrum if no valid Tsys measurement available
    if (onlyInvalidTsys) {
      flagSpectrum(flagrow, flag);
    }

    // update table
    //os_ << "calibrated=" << calibrator_->getCalibrated()[0] << LogIO::POST; 
    spCol.put(irow, calibrator_->getCalibrated());
    flCol.put(irow, flag);
    frCol.put(irow, flagrow);
    if (filltsys && !onlyInvalidTsys)
      tsysCol.put(irow, iTsys);
  }
  

  // reset selection on apply tables
  for (uInt i = 0; i < skylist.nelements(); i++) 
    skytable_[i]->unsetSelection();
  for (uInt i = 0; i < tsystable_.size(); i++)
    tsystable_[i]->unsetSelection();


  // reset interpolator
  interpolatorS_->reset();
  interpolatorF_->reset();
  interpolatorT_->reset();
}

uInt STApplyCal::getIFForTsys(uInt to)
{
  for (map<casa::uInt, Vector<uInt> >::iterator i = spwmap_.begin(); 
       i != spwmap_.end(); i++) {
    Vector<uInt> tolist = i->second;
    os_ << "from=" << i->first << ": tolist=" << tolist << LogIO::POST;
    for (uInt j = 0; j < tolist.nelements(); j++) {
      if (tolist[j] == to)
        return i->first;
    }
  }
  return (uInt)-1;
}

void STApplyCal::save(const String &name)
{
  //assert(!work_.null());
  assert_<AipsError>(!work_.null(),"You have to execute apply method first.");

  work_->setSelection(sel_);
  work_->makePersistent(name);
  work_->unsetSelection();
}

Vector<Double> STApplyCal::getBaseFrequency(uInt whichrow)
{
  //assert(whichrow <= (uInt)work_->nrow());
  assert_<AipsError>(whichrow <= (uInt)work_->nrow(),"row index out of range.");
  ROTableColumn col(work_->table(), "IFNO");
  uInt ifno = col.asuInt(whichrow);
  col.attach(work_->table(), "FREQ_ID");
  uInt freqid = col.asuInt(whichrow);
  uInt nc = work_->nchan(ifno);
  STFrequencies ftab = work_->frequencies();
  Double rp, rf, inc;
  ftab.getEntry(rp, rf, inc, freqid);
  Vector<Double> r(nc);
  indgen(r, rf-rp*inc, inc);
  return r;
}

void STApplyCal::initInterpolator()
{
  os_.origin(LogOrigin("STApplyCal","initInterpolator",WHERE));
  int order = (order_ > 0) ? order_ : 1;
  switch (iTime_) {
  case STCalEnum::NearestInterpolation:
    {
      os_ << "use NearestInterpolator in time axis" << LogIO::POST;
      interpolatorS_ = new NearestInterpolator1D<Double, Float>();
      interpolatorT_ = new NearestInterpolator1D<Double, Float>();
      break;
    }
  case STCalEnum::LinearInterpolation:
    {
      os_ << "use BufferedLinearInterpolator in time axis" << LogIO::POST;
      interpolatorS_ = new BufferedLinearInterpolator1D<Double, Float>();
      interpolatorT_ = new BufferedLinearInterpolator1D<Double, Float>();
      break;      
    }
  case STCalEnum::CubicSplineInterpolation:
    {
      os_ << "use CubicSplineInterpolator in time axis" << LogIO::POST;
      interpolatorS_ = new CubicSplineInterpolator1D<Double, Float>();
      interpolatorT_ = new CubicSplineInterpolator1D<Double, Float>();
      break;
    }
  case STCalEnum::PolynomialInterpolation:
    {
      os_ << "use PolynomialInterpolator in time axis" << LogIO::POST;
      if (order == 0) {
        interpolatorS_ = new NearestInterpolator1D<Double, Float>();
        interpolatorT_ = new NearestInterpolator1D<Double, Float>();
      }
      else {
        interpolatorS_ = new PolynomialInterpolator1D<Double, Float>();
        interpolatorT_ = new PolynomialInterpolator1D<Double, Float>();
        interpolatorS_->setOrder(order);
        interpolatorT_->setOrder(order);
      }
      break;
    }
  default:
    {
      os_ << "use BufferedLinearInterpolator in time axis" << LogIO::POST;
      interpolatorS_ = new BufferedLinearInterpolator1D<Double, Float>();
      interpolatorT_ = new BufferedLinearInterpolator1D<Double, Float>();
      break;      
    }
  }
   
  switch (iFreq_) {
  case STCalEnum::NearestInterpolation:
    {
      os_ << "use NearestInterpolator in frequency axis" << LogIO::POST;
      interpolatorF_ = new NearestInterpolator1D<Double, Float>();
      break;
    }
  case STCalEnum::LinearInterpolation:
    {
      os_ << "use BufferedLinearInterpolator in frequency axis" << LogIO::POST;
      interpolatorF_ = new BufferedLinearInterpolator1D<Double, Float>();
      break;      
    }
  case STCalEnum::CubicSplineInterpolation:
    {
      os_ << "use CubicSplineInterpolator in frequency axis" << LogIO::POST;
      interpolatorF_ = new CubicSplineInterpolator1D<Double, Float>();
      break;
    }
  case STCalEnum::PolynomialInterpolation:
    {
      os_ << "use PolynomialInterpolator in frequency axis" << LogIO::POST;
      if (order == 0) {
        interpolatorF_ = new NearestInterpolator1D<Double, Float>();
      }
      else {
        interpolatorF_ = new PolynomialInterpolator1D<Double, Float>();
        interpolatorF_->setOrder(order);
      }
      break;
    }
  default:
    {
      os_ << "use LinearInterpolator in frequency axis" << LogIO::POST;
      interpolatorF_ = new BufferedLinearInterpolator1D<Double, Float>();
      break;      
    }
  }
}

}

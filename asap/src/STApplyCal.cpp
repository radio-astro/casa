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
  Vector<uInt> skycalList;
  uInt numSkyCal = 0;
  uInt nrowSky = 0;

  // list of apply tables for Tsys calibration
  for (uInt i = 0 ; i < skytable_.size(); i++) {
    STCalEnum::CalType caltype = STApplyTable::getCalType(skytable_[i]);
    if (caltype == caltype_) {
      skycalList.resize(numSkyCal+1, True);
      skycalList[numSkyCal] = i;
      numSkyCal++;
      nrowSky += skytable_[i]->nrow();
    }
  }


  vector<string> cols( 3 ) ;
  cols[0] = "BEAMNO" ;
  cols[1] = "POLNO" ;
  cols[2] = "IFNO" ;
  CountedPtr<STIdxIter> iter = new STIdxIterAcc(work_, cols) ;
  while (!iter->pastEnd()) {
    Vector<uInt> ids = iter->current();
    Vector<uInt> rows = iter->getRows(SHARE);
    if (rows.nelements() > 0)
      doapply(ids[0], ids[2], ids[1], rows, skycalList, filltsys);
    iter->next();
  }

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
  uInt nrowSky = 0;
  uInt nrowTsys = 0;
  for (uInt i = 0; i < skylist.nelements(); i++) {
    skytable_[skylist[i]]->setSelection(sel);
    nrowSky += skytable_[skylist[i]]->nrow();
    os_ << "nrowSky=" << nrowSky << LogIO::POST;
  }

  // Skip IFNO without sky data
  if (nrowSky == 0)
    return;

  uInt nchanTsys = 0;
  Vector<Double> ftsys;
  uInt tsysifno = getIFForTsys(ifno);
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
      nrowTsys += nrowThisTsys;
      if (nrowThisTsys > 0 and nchanTsys == 0) {
	nchanTsys = tsystable_[i]->nchan(tsysifno);
	ftsys = tsystable_[i]->getBaseFrequency(0);
      }
    }
    interpolatorF_->setX(ftsys.data(), nchanTsys);
  }

  uInt nchanSp = skytable_[skylist[0]]->nchan(ifno);
  Vector<Double> timeSky(nrowSky);
  Matrix<Float> spoff(nchanSp, nrowSky);
  Vector<Float> iOff(nchanSp);
  nrowSky = 0;
  for (uInt i = 0 ; i < skylist.nelements(); i++) {
    STCalSkyTable *p = skytable_[skylist[i]];
    Vector<Double> t = p->getTime();
    Matrix<Float> sp = p->getSpectra();
    for (uInt j = 0; j < t.nelements(); j++) {
      timeSky[nrowSky] = t[j];
      spoff.column(nrowSky) = sp.column(j);
      nrowSky++;
    }
  }

  Vector<uInt> skyIdx = timeSort(timeSky);

  Double *xa = new Double[skyIdx.nelements()];
  Float *ya = new Float[skyIdx.nelements()];
  IPosition ipos(1, skyIdx.nelements());
  Vector<Double> timeSkySorted(ipos, xa, TAKE_OVER);
  Vector<Float> tmpOff(ipos, ya, TAKE_OVER);
  for (uInt i = 0 ; i < skyIdx.nelements(); i++) {
    timeSkySorted[i] = timeSky[skyIdx[i]];
  }

  interpolatorS_->setX(xa, skyIdx.nelements());

  Vector<uInt> tsysIdx;
  Vector<Double> timeTsys(nrowTsys);
  Matrix<Float> tsys;
  Vector<Double> timeTsysSorted;
  Vector<Float> tmpTsys;
  if (doTsys) {
    //os_ << "doTsys" << LogIO::POST;
    timeTsys.resize(nrowTsys);
    tsys.resize(nchanTsys, nrowTsys);
    nrowTsys = 0;
    for (uInt i = 0 ; i < tsystable_.size(); i++) {
      STCalTsysTable *p = tsystable_[i];
      Vector<Double> t = p->getTime();
      Matrix<Float> ts = p->getTsys();
      for (uInt j = 0; j < t.nelements(); j++) {
        timeTsys[nrowTsys] = t[j];
        tsys.column(nrowTsys) = ts.column(j);
        nrowTsys++;
      }
    }
    tsysIdx = timeSort(timeTsys);

    Double *xb = new Double[tsysIdx.nelements()];
    Float *yb = new Float[tsysIdx.nelements()];
    IPosition ipos(1, tsysIdx.nelements());
    timeTsysSorted.takeStorage(ipos, xb, TAKE_OVER);
    tmpTsys.takeStorage(ipos, yb, TAKE_OVER);
    for (uInt i = 0 ; i < tsysIdx.nelements(); i++) {
      timeTsysSorted[i] = timeTsys[tsysIdx[i]];
    }
    interpolatorT_->setX(xb, tsysIdx.nelements());
  }

  Table tab = work_->table();
  ArrayColumn<Float> spCol(tab, "SPECTRA");
  ArrayColumn<Float> tsysCol(tab, "TSYS");
  ScalarColumn<Double> timeCol(tab, "TIME");
  Vector<Float> on;
  for (uInt i = 0; i < rows.nelements(); i++) {
    //os_ << "start i = " << i << " (row = " << rows[i] << ")" << LogIO::POST;
    uInt irow = rows[i];

    // target spectral data
    on = spCol(irow);
    //os_ << "on=" << on[0] << LogIO::POST;
    calibrator_->setSource(on);

    // interpolation
    Double t0 = timeCol(irow);
    for (uInt ichan = 0; ichan < nchanSp; ichan++) {
      Vector<Float> spOffSlice = spoff.row(ichan);
      //os_ << "spOffSlice = " << spOffSlice << LogIO::POST;
      for (uInt j = 0; j < skyIdx.nelements(); j++) {
        tmpOff[j] = spOffSlice[skyIdx[j]];
      }
      interpolatorS_->setY(ya, skyIdx.nelements());
      iOff[ichan] = interpolatorS_->interpolate(t0);
    }
    //os_ << "iOff=" << iOff[0] << LogIO::POST;
    calibrator_->setReference(iOff);
    
    Float *Y = new Float[nchanSp];
    Vector<Float> iTsys(IPosition(1,nchanSp), Y, TAKE_OVER);
    if (doTsys) {
      // Tsys correction
      Float *yt = new Float[nchanTsys];
      Vector<Float> iTsysT(IPosition(1,nchanTsys), yt, TAKE_OVER);
      Float *yb = tmpTsys.data();
      for (uInt ichan = 0; ichan < nchanTsys; ichan++) {
        Vector<Float> tsysSlice = tsys.row(ichan);
        for (uInt j = 0; j < tsysIdx.nelements(); j++) {
          tmpTsys[j] = tsysSlice[tsysIdx[j]];
        }
        interpolatorT_->setY(yb, tsysIdx.nelements());
        iTsysT[ichan] = interpolatorT_->interpolate(t0);
      }
      if (nchanSp == 1) {
        // take average
        iTsys[0] = mean(iTsysT);
      }
      else {
        // interpolation on frequency axis
        Vector<Double> fsp = getBaseFrequency(rows[i]);
        interpolatorF_->setY(yt, nchanTsys);
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

    // update table
    //os_ << "calibrated=" << calibrator_->getCalibrated()[0] << LogIO::POST; 
    spCol.put(irow, calibrator_->getCalibrated());
    if (filltsys)
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

Vector<uInt> STApplyCal::timeSort(Vector<Double> &t)
{
  Sort sort;
  sort.sortKey(&t[0], TpDouble, 0, Sort::Ascending);
  Vector<uInt> idx;
  sort.sort(idx, t.nelements(), Sort::QuickSort|Sort::NoDuplicates);
  return idx;
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

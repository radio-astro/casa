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
#include "NearestInterpolator1D.h"
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
  interp_.resize((int)STCalEnum::NumAxis);
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

void STApplyCal::setInterpolation(STCalEnum::InterpolationAxis axis, STCalEnum::InterpolationType itype, Int order)
{
  interp_[(int)axis] = itype;
  order_ = order;
}

void STApplyCal::setTsysTransfer(uInt from, Vector<uInt> to)
{
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

void STApplyCal::apply(Bool insitu)
{
  // calibrator
  if (caltype_ == STCalEnum::CalPSAlma)
    calibrator_ = new PSAlmaCalibrator();

  // interpolator
  interpolatorS_ = new NearestInterpolator1D();
  interpolatorT_ = new NearestInterpolator1D();
  interpolatorF_ = new NearestInterpolator1D();

  // select data
  sel_.reset();
  if (caltype_ == STCalEnum::CalPSAlma ||
      caltype_ == STCalEnum::CalPS) {
    sel_.setTypes(vector<int>(1,(int)SrcType::PSON));
  }
  target_->setSelection(sel_);

  os_ << "sel_.print()=" << sel_.print() << LogIO::POST;

  // working data
  if (insitu)
    work_ = target_;
  else
    work_ = new Scantable(*target_, false);

  os_ << "work_->nrow()=" << work_->nrow() << LogIO::POST;

  // list of apply tables for sky calibration
  Vector<uInt> skycalList;
  uInt numSkyCal = 0;
  uInt nrowSky = 0;
  // list of apply tables for Tsys calibration
//   Vector<uInt> tsyscalList;

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
    os_ << "ids=" << ids << LogIO::POST;
    if (rows.nelements() > 0)
      doapply(ids[0], ids[2], ids[1], rows, skycalList);
    iter->next();
  }

  target_->unsetSelection();
}

void STApplyCal::doapply(uInt beamno, uInt ifno, uInt polno, 
                         Vector<uInt> &rows,
                         Vector<uInt> &skylist)
{
  os_ << "skylist=" << skylist << LogIO::POST;
  os_ << "rows=" << rows << LogIO::POST;
  Bool doTsys = doTsys_;

  //STSelector sel = sel_;
  STSelector sel;
  vector<int> id(1);
  id[0] = beamno;
  sel.setBeams(id);
  id[0] = ifno;
  sel.setIFs(id);
  id[0] = polno;
  sel.setPolarizations(id);  
  os_ << "sel=" << sel.print() << LogIO::POST;

  // apply selection to apply tables
  uInt nrowSky = 0;
  uInt nrowTsys = 0;
  for (uInt i = 0; i < skylist.nelements(); i++) {
    skytable_[skylist[i]]->setSelection(sel);
    nrowSky += skytable_[skylist[i]]->nrow();
    os_ << "nrowSky=" << nrowSky << LogIO::POST;
  }
  uInt nchanTsys = 0;
  Vector<Double> ftsys;
  uInt tsysifno = getIFForTsys(ifno);
  os_ << "tsysifno=" << tsysifno << LogIO::POST;
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
    nchanTsys = tsystable_[0]->nchan(tsysifno);
    ftsys = tsystable_[0]->getBaseFrequency(0);
    interpolatorF_->setX(ftsys.data(), nchanTsys);
    os_ << "nchanTsys=" << nchanTsys << LogIO::POST;
    id[0] = (int)tsysifno;
    sel.setIFs(id);
    for (uInt i = 0; i < tsystable_.size() ; i++) {
      tsystable_[i]->setSelection(sel);
      nrowTsys += tsystable_[i]->nrow();
      os_ << "nrowTsys=" << nrowTsys << LogIO::POST;
    }
  }

  uInt nchanSp = skytable_[skylist[0]]->nchan(ifno);
  os_ << "nchanSp = " << nchanSp << LogIO::POST;
  Vector<Double> timeSky(nrowSky);
  Matrix<Float> spoff(nchanSp, nrowSky);
  Vector<Float> iOff(nchanSp);
  nrowSky = 0;
  os_ << "spoff.shape()=" << spoff.shape() << LogIO::POST;
  for (uInt i = 0 ; i < skylist.nelements(); i++) {
    STCalSkyTable *p = skytable_[skylist[i]];
    os_ << "table " << i << ": nrow=" << p->nrow() << LogIO::POST;
    Vector<Double> t = p->getTime();
    Matrix<Float> sp = p->getSpectra();
    os_ << "sp.shape()=" << sp.shape() << LogIO::POST;
    os_ << "t.nelements()=" << t.nelements() << LogIO::POST;
    for (uInt j = 0; j < t.nelements(); j++) {
      timeSky[nrowSky] = t[j];
      os_ << "timeSky[" << nrowSky << "]-timeSky[0]=" << timeSky[nrowSky] - timeSky[0] << LogIO::POST;
      spoff.column(nrowSky) = sp.column(j);
      nrowSky++;
    }
  }
  os_ << "timeSky-timeSky[0]=" << timeSky-timeSky[0] << LogIO::POST;

  Vector<uInt> skyIdx = timeSort(timeSky);
  os_ << "skyIdx = " << skyIdx << LogIO::POST;

  double *xa = new double[skyIdx.nelements()];
  float *ya = new float[skyIdx.nelements()];
  IPosition ipos(1, skyIdx.nelements());
  Vector<double> timeSkySorted(ipos, xa, TAKE_OVER);
  Vector<Float> tmpOff(ipos, ya, TAKE_OVER);
  for (uInt i = 0 ; i < skyIdx.nelements(); i++) {
    timeSkySorted[i] = (double)timeSky[skyIdx[i]];
    os_ << "timeSkySorted[" << i << "]-timeSkySorted[0]=" << timeSkySorted[i] - timeSkySorted[0] << LogIO::POST;
  }
  os_ << "timeSkySorted-timeSkySorted[0]=" << timeSkySorted-timeSkySorted[0] << LogIO::POST;

  interpolatorS_->setX(xa, skyIdx.nelements());

  os_ << "doTsys = " << doTsys << LogIO::POST;
  Vector<uInt> tsysIdx;
  Vector<Double> timeTsys(nrowTsys);
  Matrix<Float> tsys;
  Vector<double> timeTsysSorted;
  Vector<Float> tmpTsys;
  if (doTsys) {
    os_ << "doTsys" << LogIO::POST;
    timeTsys.resize(nrowTsys);
    tsys.resize(nchanTsys, nrowTsys);
    nrowTsys = 0;
    for (uInt i = 0 ; i < tsystable_.size(); i++) {
      STCalTsysTable *p = tsystable_[i];
      os_ << "p->nrow()=" << p->nrow() << LogIO::POST;
      Vector<Double> t = p->getTime();
      os_ << "t=" << t << LogIO::POST;
      Matrix<Float> ts = p->getTsys();
      for (uInt j = 0; j < t.nelements(); j++) {
        timeTsys[nrowTsys] = t[j];
        tsys.column(nrowTsys) = ts.column(j);
        nrowTsys++;
      }
    }
    tsysIdx = timeSort(timeTsys);
    os_ << "tsysIdx = " << tsysIdx << LogIO::POST;

    double *xb = new double[tsysIdx.nelements()];
    float *yb = new float[tsysIdx.nelements()];
    IPosition ipos(1, tsysIdx.nelements());
    timeTsysSorted.takeStorage(ipos, xb, TAKE_OVER);
    tmpTsys.takeStorage(ipos, yb, TAKE_OVER);
    for (uInt i = 0 ; i < tsysIdx.nelements(); i++) {
      timeTsysSorted[i] = (double)timeTsys[tsysIdx[i]];
      os_ << "timeTsysSorted[" << i << "]-timeTsysSorted[0]=" << timeTsysSorted[i] - timeTsysSorted[0] << LogIO::POST;
    }
    os_ << "timeTsysSorted=" << timeTsysSorted << LogIO::POST;
    interpolatorT_->setX(xb, tsysIdx.nelements());
  }

  Table tab = work_->table();
  ArrayColumn<Float> spCol(tab, "SPECTRA");
  ScalarColumn<Double> timeCol(tab, "TIME");
  Vector<Float> on;
  for (uInt i = 0; i < rows.nelements(); i++) {
    os_ << "start row " << i << LogIO::POST;
    uInt irow = rows[i];

    // target spectral data
    on = spCol(irow);
    calibrator_->setSource(on);

    // interpolation
    double t0 = (double)timeCol(irow);
    for (uInt ichan = 0; ichan < nchanSp; ichan++) {
      Vector<Float> spOffSlice = spoff.row(ichan);
      //os_ << "spOffSlice = " << spOffSlice << LogIO::POST;
      for (uInt j = 0; j < skyIdx.nelements(); j++) {
        tmpOff[j] = (float)spOffSlice[skyIdx[j]];
      }
      interpolatorS_->setY(ya, skyIdx.nelements());
      iOff[ichan] = interpolatorS_->interpolate(t0);
    }
    //os_ << "iOff=" << iOff << LogIO::POST;
    calibrator_->setReference(iOff);
    
    Float *Y = new Float[nchanSp];
    Vector<Float> iTsys(IPosition(1,nchanSp), Y, TAKE_OVER);
    if (doTsys) {
      // Tsys correction
      float *yt = new float[nchanTsys];
      Vector<Float> iTsysT(IPosition(1,nchanTsys), yt, TAKE_OVER);
      float *yb = tmpTsys.data();
      for (uInt ichan = 0; ichan < nchanTsys; ichan++) {
        Vector<Float> tsysSlice = tsys.row(ichan);
        for (uInt j = 0; j < tsysIdx.nelements(); j++) {
          tmpTsys[j] = (float)tsysSlice[tsysIdx[j]];
        }
        interpolatorT_->setY(yb, tsysIdx.nelements());
        iTsysT[ichan] = interpolatorT_->interpolate(t0);
      }
      os_ << "iTsysT=" << iTsysT << LogIO::POST;
      if (nchanSp == 1) {
        // take average
        iTsys[0] = mean(iTsysT);
      }
      else {
        // interpolation on frequency axis
        os_ << "getBaseFrequency for target" << LogIO::POST;
        Vector<Double> fsp = getBaseFrequency(rows[i]);
        os_ << "fsp = " << fsp << LogIO::POST;
        interpolatorF_->setY(yt, nchanTsys);
        for (uInt ichan = 0; ichan < nchanSp; ichan++) {
          iTsys[ichan] = (Float)interpolatorF_->interpolate(fsp[ichan]);
        }
      }
    }
    else {
      iTsys = 1.0;
    } 
    os_ << "iTsys=" << iTsys << LogIO::POST;
    calibrator_->setScaler(iTsys);
  
    // do calibration
    calibrator_->calibrate();

    // update table
    os_ << "calibrated=" << calibrator_->getCalibrated() << LogIO::POST; 
    spCol.put(irow, calibrator_->getCalibrated());
    
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
    os_ << i->first << ": tolist=" << tolist << LogIO::POST;
    for (uInt j = 0; j < tolist.nelements(); j++) {
      if (tolist[j] == to)
        return i->first;
    }
  }
  return (uInt)-1;
}

void STApplyCal::save(const String &name)
{
  if (work_.null())
    return;

  work_->setSelection(sel_);
  work_->makePersistent(name);
  work_->unsetSelection();
}

Vector<Double> STApplyCal::getBaseFrequency(uInt whichrow)
{
  assert(whichrow <= (uInt)work_->nrow());
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

}

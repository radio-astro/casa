//
// C++ Implementation: Calibrator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>

#include "Calibrator.h"

using namespace casa;

namespace asap {

Calibrator::Calibrator()
  : nchan_(0),
    nchanS_(0),
    source_(0),
    ref_(0),
    ref2_(0),
    scaler_(0),
    calibrated_(0)
{}

Calibrator::Calibrator(unsigned int nchan)
  : nchan_(nchan),
    nchanS_(0),
    source_(0),
    ref_(0),
    ref2_(0),
    scaler_(0),
    calibrated_(0)
{}

Calibrator::~Calibrator()
{
  freeStorage();
}

void Calibrator::setSource(Vector<Float> &v)
{
  if (nchan_ == 0) {
    nchan_ = v.nelements();
    initStorage();
  }
  else if (nchan_ != v.nelements()) {
    freeStorage();
    nchan_ = v.nelements();
    initStorage();
  }
  set(source_, v);
}

void Calibrator::set(Float *p, Vector<Float> &v)
{
  Float *work = p;
  for (unsigned int i = 0; i < v.nelements(); i++) {
    *work = v[i];
    work++;
  }
}

void Calibrator::setReference(Vector<Float> &v)
{
  //assert(v.nelements() == nchan_);
  assert_<AipsError>(v.nelements() == nchan_, "Reference spectrum shape mismatch.");
  set(ref_, v);
}

void Calibrator::setReference2(Vector<Float> &v)
{
  //assert(v.nelements() == nchan_);
  assert_<AipsError>(v.nelements() == nchan_, "Second reference spectrum shape mismatch.");
  if (!ref2_)
    ref2_ = new Float[nchan_];
  set(ref2_, v);
}

void Calibrator::setScaler(Vector<Float> &v)
{
  //assert(v.nelements() == nchan_ || v.nelements() == 1);
  assert_<AipsError>(v.nelements() == nchan_ || v.nelements() == 1,
         "Scaling factor shape mismatch.");
  if (nchanS_ == 0) {
    nchanS_ = v.nelements();
    if (!scaler_)
      scaler_ = new Float[nchanS_];
  }
  else if (v.nelements() != nchanS_) {
    if (scaler_)
      delete[] scaler_;
    nchanS_ = v.nelements();
    scaler_ = new Float[nchanS_];
  }
  set(scaler_, v);
}

void Calibrator::initStorage()
{
  freeStorage();
  source_ = new Float[nchan_];
  ref_ = new Float[nchan_];
  calibrated_ = new Float[nchan_];
}

void Calibrator::freeStorage()
{
  if (source_) {
    delete[] source_;
    source_ = 0;
  }
  if (ref_) {
    delete[] ref_;
    ref_ = 0;
  }
  if (ref2_) {
    delete[] ref2_;
    ref2_ = 0;
  }
  if (scaler_) {
    delete[] scaler_;
    scaler_ = 0;
  }
  if (calibrated_) {
    delete[] calibrated_;
    calibrated_ = 0;
  }
}

const Vector<Float> Calibrator::getCalibrated()
{
  return Vector<Float>(IPosition(1,nchan_), calibrated_, SHARE);
}

}

//
// C++ Implementation: RowAccumulator
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <casa/iomanip.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/MaskArrLogi.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include "RowAccumulator.h"

using namespace casa;
using namespace asap;

RowAccumulator::RowAccumulator(WeightType wt) : weightType_(wt), initialized_(False)
{
  reset();
}

RowAccumulator::~RowAccumulator()
{
}


void RowAccumulator::reset(const uInt size, const uInt tsysSize)
{
  Vector<Bool> m(size, True);

  spectrum_.setData(Vector<Float>(size, 0.0), Vector<Bool>(size, True));
  spectrumNoMask_.setData(Vector<Float>(size, 0.0), Vector<Bool>(size, True));

  n_.setData(Vector<uInt>(size, 0), Vector<Bool>(size, True));
  nNoMask_.setData(Vector<uInt>(size, 0), Vector<Bool>(size, True));

  weightSum_.setData(Vector<Float>(size, 0.0), Vector<Bool>(size, True));
  weightSumNoMask_.setData(Vector<Float>(size, 0.0), Vector<Bool>(size, True));

  tsysSum_.resize(tsysSize); tsysSum_=0.0;
  tsysSumNoMask_.resize(tsysSize); tsysSumNoMask_=0.0;

  intervalSum_ = 0.0;
  intervalSumNoMask_ = 0.0;

  timeSum_ = 0.0;
  timeSumNoMask_ = 0.0;

  initialized_ = False;
}

void RowAccumulator::initialize(const uInt size, const uInt tsysSize)
{
  reset(size, tsysSize);
  initialized_ = True;
}

void RowAccumulator::add(const Vector<Float>& v,
                         const Vector<Bool>& m,
                         const Vector<Float>& tsys,
                         const Double interval,
                         const Double time)
{
  uInt size = v.nelements();
  //if (size != m.nelements()) raiseError;
  if (!initialized_) initialize(size, tsys.nelements());

  addSpectrum(v, m, tsys, interval, time);
}

void RowAccumulator::addSpectrum(const Vector<Float>& v,
				 const Vector<Bool>& m,
				 const Vector<Float>& tsys,
				 const Double interval,
				 const Double time)
{
  doAddSpectrum(v, m, tsys, interval, time, False);
  doAddSpectrum(v, m, tsys, interval, time, True);  // CAS-2776
}

void RowAccumulator::doAddSpectrum(const Vector<Float>& v,
				   const Vector<Bool>& m,
				   const Vector<Float>& tsys,
				   const Double interval,
				   const Double time,
				   const Bool ignoreMask)
{
  Vector<Float> vUse = v.copy();
  Vector<Bool> mUse = m.copy();
  if (ignoreMask) mUse = !mUse;

  MaskedArray<Float> vadd(vUse, mUse);
  Float totalWeight = getTotalWeight(vadd, tsys, interval, time, ignoreMask);
  vadd *= totalWeight;
  MaskedArray<Float> wadd(Vector<Float>(mUse.nelements(), totalWeight), mUse);
  MaskedArray<uInt> inc(Vector<uInt>(mUse.nelements(), 1), mUse);

  if (ignoreMask) {
    spectrumNoMask_ += vadd;
    weightSumNoMask_ += wadd;
    nNoMask_ += inc;
  } else {
    spectrum_ += vadd;
    weightSum_ += wadd;
    n_ += inc;
  }
}

Float RowAccumulator::getTotalWeight(const MaskedArray<Float>& data,
				     const Vector<Float>& tsys,
				     const Double interval,
				     const Double time,
				     const Bool ignoreMask)
{
  Float totalWeight = 1.0;

  Vector<Bool> m = data.getMask();
  if (!allEQ(m, False)) {  // only add these if not everything masked
    totalWeight *= addTsys(tsys, ignoreMask);
    totalWeight *= addInterval(interval, ignoreMask);
    addTime(time, ignoreMask);
  }

  if (weightType_ == W_VAR) {
    Float fac = 1.0/variance(data);
    if (!ignoreMask && (m.nelements() == userMask_.nelements()))
      fac = 1.0/variance(data(userMask_));

    totalWeight *= fac;
  }

  return totalWeight;
}

Float RowAccumulator::addTsys(const Vector<Float>& v, Bool ignoreMask)
{
  // @fixme this assume tsys is the same for all channels

  Float w = 1.0;
  if (ignoreMask) {
    tsysSumNoMask_ += v[0];
  } else {
    tsysSum_ += v[0];
  }
  if ( weightType_ == W_TSYS  || weightType_ == W_TINTSYS ) {
    w /= (v[0]*v[0]);
  }
  return w;
}

void RowAccumulator::addTime(Double t, Bool ignoreMask)
{
  if (ignoreMask) {
    timeSumNoMask_ += t;
  } else {
    timeSum_ += t;
  }
}

Float RowAccumulator::addInterval(Double inter, Bool ignoreMask)
{
  Float w = 1.0;
  if (ignoreMask) {
    intervalSumNoMask_ += inter;
  } else {
    intervalSum_ += inter;
  }
  if (weightType_ == W_TINT || weightType_ == W_TINTSYS) {
    w /= Float(inter);
  }
  return w;
}

Vector<Float> RowAccumulator::getSpectrum() const
{
  return (spectrum_/weightSum_).getArray();
}

Double RowAccumulator::getTime() const
{
  return timeSum_/Float(max(n_));
}

Double RowAccumulator::getInterval() const
{
  return intervalSum_;
}

Vector<Bool> RowAccumulator::getMask() const
{
  // Return the "total" mask - False where no points have been accumulated.
  return (n_.getArray() > uInt(0));
}

Vector<Float> RowAccumulator::getTsys() const
{
  // @fixme this assumes tsys.nelements() == 1
  return tsysSum_/Float(max(n_));
}

void RowAccumulator::setUserMask(const Vector<Bool>& m)
{
  userMask_.resize();
  userMask_ = m;
}

// Added by TT  check the state of RowAccumulator
Bool RowAccumulator::state() const
{
  return initialized_;
}

void RowAccumulator::replaceNaN()
{
  Vector<Float> v = spectrum_.getArray();
  Vector<Float> w = weightSum_.getArray();
  Vector<Float> vRef = spectrumNoMask_.getArray();
  Vector<Float> wRef = weightSumNoMask_.getArray();

  for (uInt i = 0; i < v.size(); ++i) {
    if (w[i] == 0.0) {
      v[i] = vRef[i];
      w[i] = wRef[i];
    }
  }

  spectrum_.setData(v, Vector<Bool>(v.nelements(), True));
  weightSum_.setData(w, Vector<Bool>(w.nelements(), True));
}

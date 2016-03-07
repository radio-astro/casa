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

namespace {
inline uInt nNominal(MaskedArray<uInt> nvalid, MaskedArray<uInt> ninvalid)
{
  return max((allEQ(nvalid, uInt(0))) ? ninvalid : nvalid);
}
} // anonymous namespace

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
//   doAddSpectrum(v, m, tsys, interval, time, False);
//   doAddSpectrum(v, m, tsys, interval, time, True);  // CAS-2776
  doAddSpectrum2( v, m, tsys, interval, time ) ;
}

void RowAccumulator::doAddSpectrum(const Vector<Float>& v,
				   const Vector<Bool>& m,
				   const Vector<Float>& tsys,
				   const Double interval,
				   const Double time,
				   const Bool inverseMask)
{
  Vector<Float> vUse = v.copy();
  Vector<Bool> mUse = m.copy();
  if (inverseMask) mUse = !mUse;
  MaskedArray<Float> vadd(vUse, mUse);
  Float totalWeight = getTotalWeight(vadd, tsys, interval, time, inverseMask);
  vadd *= totalWeight;
  MaskedArray<Float> wadd(Vector<Float>(mUse.nelements(), totalWeight), mUse);
  MaskedArray<uInt> inc(Vector<uInt>(mUse.nelements(), 1), mUse);

  if (inverseMask) {
    spectrumNoMask_ += vadd;
    weightSumNoMask_ += wadd;
    nNoMask_ += inc;
  } else {
    spectrum_ += vadd;
    weightSum_ += wadd;
    n_ += inc;
  }
}

void RowAccumulator::doAddSpectrum2(const Vector<Float>& v,
                                    const Vector<Bool>& m,
                                    const Vector<Float>& tsys,
                                    const Double interval,
                                    const Double time)
{
  const MaskedArray<Float> vadd(v, m);
  const MaskedArray<Float> vaddN(v, !m);
  Float totalWeight = getTotalWeight( vadd, tsys, interval, time, False ) ;
  Float totalWeightNoMask = getTotalWeight( vaddN, tsys, interval, time, True ) ;
  // process spectrum with input mask and spectrum with inverted mask 
  // together
  // prepare pointers
  Bool vD, mD ;
  const Float *v_p = v.getStorage( vD ) ;
  const Float *v_wp = v_p ;
  const Bool *m_p = m.getStorage( mD ) ;
  const Bool *m_wp = m_p ;

  Bool spD, spND, wgtD, wgtND, nD, nND ;
  Float *sp_p = spectrum_.getRWArrayStorage( spD ) ;
  Float *sp_wp = sp_p ;
  Float *wgt_p = weightSum_.getRWArrayStorage( wgtD ) ;
  Float *wgt_wp = wgt_p ;
  Float *spN_p = spectrumNoMask_.getRWArrayStorage( spND ) ;
  Float *spN_wp = spN_p ;
  Float *wgtN_p = weightSumNoMask_.getRWArrayStorage( wgtND ) ;
  Float *wgtN_wp = wgtN_p ;
  uInt *n_p = n_.getRWArrayStorage( nD ) ;
  uInt *n_wp = n_p ;
  uInt *nN_p = nNoMask_.getRWArrayStorage( nND ) ;
  uInt *nN_wp = nN_p ;

  // actual processing
  uInt len = m.nelements() ;
  // loop over channels
  for ( uInt i = 0 ; i < len ; i++ ) {
    // masks for spectrum_ and spectrumNoMask_ are not needed since 
    // it is initialized as True for all channels and those are constant
    if ( *m_wp ) {
      // mask is True
      // add v * totalWeight to spectrum_
      // add totalWeight to weightSum_
      // increment n_
      *sp_wp += *v_wp * totalWeight ;
      *wgt_wp += totalWeight ;
      *n_wp += 1 ;
    }
    else {
      // mask is False
      // add v * totalWeightNoMask to spectrumNoMask_
      // add totalWeightNoMask to weightSumNoMask_
      // increment nNoMask_
      *spN_wp += *v_wp * totalWeightNoMask ;
      *wgtN_wp += totalWeightNoMask ;
      *nN_wp += 1 ;
    }
    sp_wp++ ;
    wgt_wp++ ;
    n_wp++ ;
    spN_wp++ ;
    wgtN_wp++ ;
    nN_wp++ ;
    v_wp++ ;
    m_wp++ ;
  }

  // free storage
  spectrum_.putArrayStorage( sp_p, spD ) ;
  weightSum_.putArrayStorage( wgt_p, wgtD ) ;
  spectrumNoMask_.putArrayStorage( spN_p, spND ) ;
  weightSumNoMask_.putArrayStorage( wgtN_p, wgtND ) ;
  n_.putArrayStorage( n_p, nD ) ;
  nNoMask_.putArrayStorage( nN_p, nND ) ;

  v.freeStorage( v_p, vD ) ; 
  m.freeStorage( m_p, mD ) ;
}

Float RowAccumulator::getTotalWeight(const MaskedArray<Float>& data,
				     const Vector<Float>& tsys,
				     const Double interval,
				     const Double time,
				     const Bool inverseMask)
{
  Float totalWeight = 1.0;
  Vector<Bool> m = data.getMask();
  if (!allEQ(m, False)) {  // only add these if not everything masked
    totalWeight *= addTsys(tsys, inverseMask);
    totalWeight *= addInterval(interval, inverseMask);
    addTime(time, inverseMask);

    if (weightType_ == W_VAR) {
      Float fac = 1.0/variance(data);
      if (!inverseMask && (m.nelements() == userMask_.nelements()))
	fac = 1.0/variance(data(userMask_));

      totalWeight *= fac;
    }
  }

  return totalWeight;
}

Float RowAccumulator::addTsys(const Vector<Float>& v, Bool inverseMask)
{
  // @fixme this assume tsys is the same for all channels

  Float w = 1.0;
  if (inverseMask) {
    tsysSumNoMask_ += v[0];
  } else {
    tsysSum_ += v[0];
  }
  if ( weightType_ == W_TSYS  || weightType_ == W_TINTSYS ) {
    w /= (v[0]*v[0]);
  }
  return w;
}

void RowAccumulator::addTime(Double t, Bool inverseMask)
{
  if (inverseMask) {
    timeSumNoMask_ += t;
  } else {
    timeSum_ += t;
  }
}

Float RowAccumulator::addInterval(Double inter, Bool inverseMask)
{
  Float w = 1.0;
  if (inverseMask) {
    intervalSumNoMask_ += inter;
  } else {
    intervalSum_ += inter;
  }
  if (weightType_ == W_TINT || weightType_ == W_TINTSYS) {
    w *= Float(inter);
  }
  return w;
}

Vector<Float> RowAccumulator::getSpectrum() const
{
  return (spectrum_/weightSum_).getArray();
}

Double RowAccumulator::getTime() const
{
  return timeSum_/Double(nNominal(n_, nNoMask_));
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
  return tsysSum_/Float(nNominal(n_, nNoMask_));
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

  if (allEQ(tsysSum_, 0.0f))
    tsysSum_ = tsysSumNoMask_;
  if (intervalSum_ == 0.0)
    intervalSum_ = intervalSumNoMask_;
}

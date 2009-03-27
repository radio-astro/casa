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

RowAccumulator::RowAccumulator(WeightType wt) :
  weightType_(wt),
  initialized_(False)
{
  reset();
}

RowAccumulator::~RowAccumulator()
{
}


void RowAccumulator::add( const Vector< Float >& v,
                          const Vector< Bool >& m,
                          const Vector< Float >& tsys,
                          Double interval,
                          Double time )
{
  if (!initialized_) {
    Vector<Float> dummy(v.nelements(), 0.0);
    Vector<Bool> dummymsk(m.nelements(), True);
    spectrum_.setData(dummy, dummymsk);
    n_.setData(Vector<Float>(v.nelements(), 0.0), dummymsk);
    weightSum_.setData(Vector<Float>(v.nelements(), 0.0), dummymsk);
    tsysSum_.resize(tsys.nelements()); tsysSum_=0.0;
  }
  // add spectrum related weights, so far it is variance only.
  Float totalweight = 1.0;
  totalweight *= addTsys(tsys);
  // only add these if not everything masked
  if ( !allEQ(m, False) ) {
    totalweight *= addInterval(interval);
    addTime(time);
  }
  addSpectrum(v, m, totalweight);
  initialized_ = True;
}

void RowAccumulator::addSpectrum( const Vector< Float >& v,
                                  const Vector< Bool >& m,
                                  Float weight)
{
  Float totalweight = weight;
  MaskedArray<Float> data(v,m);
  if ( weightType_ == asap::VAR ) {
    if (m.nelements() == userMask_.nelements()) {
      Float fac = 1.0/variance(data(userMask_));
      totalweight *= fac;
    } else {
      Float fac = 1.0/variance(data);
      totalweight *= fac;
    }
  }
  data *= totalweight;
  MaskedArray<Float> wadd(Vector<Float>(m.nelements(),totalweight), m);
  weightSum_ += wadd;
  spectrum_ += data;
  const MaskedArray<Float> inc(Vector<Float>(m.nelements(),1.0), m);
  n_ += inc;
}

Float RowAccumulator::addTsys( const casa::Vector< casa::Float > & v )
{
  // @fixme this assume tsys is the same for all channels

  Float w = 1.0;
  tsysSum_ += v[0];
  if ( weightType_ == asap::TSYS  || weightType_ == asap::TINTSYS ) {
    w /= (v[0]*v[0]);
  }
  return w;
}

void asap::RowAccumulator::addTime( casa::Double t )
{
  timeSum_ += t;
}

Float asap::RowAccumulator::addInterval( casa::Double inter )
{
  Float w = 1.0;
  intervalSum_ += inter;
  if ( weightType_ == asap::TINT || weightType_ == asap::TINTSYS ) {
    w /= Float(inter);
  }
  return w;
}

void asap::RowAccumulator::reset( )
{
  initialized_ = False;
  intervalSum_ = 0.0;
  tsysSum_.resize();
  timeSum_ = 0.0;
}

casa::Vector< casa::Float > RowAccumulator::getSpectrum( ) const
{
  return (spectrum_/weightSum_).getArray();
}

casa::Double asap::RowAccumulator::getTime( ) const
{
  Float n = max(n_);
  if (n < 1.0) n = 1.0; 
  return timeSum_/n;
}

casa::Double asap::RowAccumulator::getInterval( ) const
{
  return intervalSum_;
}

casa::Vector< casa::Bool > RowAccumulator::getMask( ) const
{
  // Return the "total" mask - False where no points have been accumulated.
  return (n_.getArray() > Float(0.0));
}

casa::Vector< casa::Float > asap::RowAccumulator::getTsys( ) const
{
  // @fixme this assumes tsys.nelements() == 1
  return tsysSum_/max(n_);
}

void asap::RowAccumulator::setUserMask( const casa::Vector< casa::Bool > & m )
{
  userMask_.resize();
  userMask_ = m;
}

// Added by TT  check the state of RowAccumulator
casa::Bool RowAccumulator::state() const
{
  return initialized_;
}

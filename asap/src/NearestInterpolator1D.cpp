//
// C++ Implementation: NearestInterpolator1D
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

#include "NearestInterpolator1D.h"

using namespace casa;

namespace asap {

NearestInterpolator1D::NearestInterpolator1D()
  : Interpolator1D()
{}

NearestInterpolator1D::~NearestInterpolator1D()
{}

float NearestInterpolator1D::interpolate(double x)
{
  assert(isready());
  if (n_ == 1)
    return y_[0];

  unsigned int i = locator_->locate(x);
  if (i == 0) {
    return y_[i];
  }
  else if (i == n_ || abs(x - x_[i]) > abs(x - x_[i-1])) {
    i--;
  }
  return y_[i];
}

}

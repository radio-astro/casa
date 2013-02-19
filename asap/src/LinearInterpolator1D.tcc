//
// C++ Implementation: LinearInterpolator1D
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

#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>

#include "LinearInterpolator1D.h"

namespace asap {

template <class T, class U>
LinearInterpolator1D<T, U>::LinearInterpolator1D()
  : Interpolator1D<T, U>()
{}

template <class T, class U>
LinearInterpolator1D<T, U>::~LinearInterpolator1D()
{}

template <class T, class U> U LinearInterpolator1D<T, U>::interpolate(T x)
{
  //assert(this->isready());
  assert_<AipsError>(this->isready(),"object is not ready to process.");
  if (this->n_ == 1)
    return this->y_[0];

  unsigned int i = this->locator_->locate(x);

  // do not perform extrapolation
  if (i == 0) {
    return this->y_[i];
  }
  else if (i == this->n_) {
    return this->y_[i-1];
  }

  // linear interpolation
  U y = this->y_[i-1] + (this->y_[i] - this->y_[i-1]) * (x - this->x_[i-1])
    / (this->x_[i] - this->x_[i-1]);
  return y;
}

}

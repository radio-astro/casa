//
// C++ Implementation: BufferedLinearInterpolator1D
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

#include "BufferedLinearInterpolator1D.h"

namespace asap {

template <class T, class U>
BufferedLinearInterpolator1D<T, U>::BufferedLinearInterpolator1D()
  : Interpolator1D<T, U>(),
    reusable_(false)
{}

template <class T, class U>
BufferedLinearInterpolator1D<T, U>::~BufferedLinearInterpolator1D()
{}

template <class T, class U>
void BufferedLinearInterpolator1D<T, U>::setData(T *x, U *y, unsigned int n)
{
  Interpolator1D<T, U>::setData(x, y, n);
  reusable_ = false;
}

template <class T, class U>
void BufferedLinearInterpolator1D<T, U>::setX(T *x, unsigned int n)
{
  Interpolator1D<T, U>::setX(x, n);
  reusable_ = false;
}

template <class T, class U> 
U BufferedLinearInterpolator1D<T, U>::interpolate(T x)
{
  //assert(this->isready());
  assert_<casa::AipsError>(this->isready(), "object is not ready to process.");
  if (this->n_ == 1)
    return this->y_[0];

  unsigned int i;
  bool b = (reusable_ && x == xold_);
  if (b) {
    i = prev_;
  }
  else {
    i = this->locator_->locate(x);
    prev_ = i;
    xold_ = x;
  }

  // do not perform extrapolation
  if (i == 0) {
    return this->y_[i];
  }
  else if (i == this->n_) {
    return this->y_[i-1];
  }

  // linear interpolation
  if (!b)
    factor_ = (x - this->x_[i-1]) / (this->x_[i] - this->x_[i-1]);
  U y = this->y_[i-1] + (this->y_[i] - this->y_[i-1]) * factor_;
  reusable_ = true;
  return y;
}

}

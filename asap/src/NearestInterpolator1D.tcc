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

#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>

#include "NearestInterpolator1D.h"

using namespace casa;

namespace asap {

template <class T, class U> 
NearestInterpolator1D<T, U>::NearestInterpolator1D()
  : Interpolator1D<T, U>()
{}

template <class T, class U> 
NearestInterpolator1D<T, U>::~NearestInterpolator1D()
{}

template <class T, class U> U NearestInterpolator1D<T, U>::interpolate(T x)
{
  //assert(this->isready());
  casa::assert_<casa::AipsError>(this->isready(),"object is not ready to process.");
  if (this->n_ == 1)
    return this->y_[0];

  unsigned int i = this->locator_->locate(x);
  if (i == 0) {
    return this->y_[i];
  }
  else if (i == this->n_ || abs(x - this->x_[i]) > abs(x - this->x_[i-1])) {
    i--;
  }
  return this->y_[i];
}

}

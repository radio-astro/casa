//
// C++ Implementation: PolynomialInterpolator1D
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
#include <math.h>
#include <iostream>
using namespace std;

#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>

#include "PolynomialInterpolator1D.h"

namespace asap {

template <class T, class U>
PolynomialInterpolator1D<T, U>::PolynomialInterpolator1D()
  : Interpolator1D<T, U>()
{}

template <class T, class U>
PolynomialInterpolator1D<T, U>::~PolynomialInterpolator1D()
{}

template <class T, class U>
U PolynomialInterpolator1D<T, U>::interpolate(T x)
{
  //casa::AlwaysAssert((this->isready()),(casa::AipsError));
  casa::assert_<casa::AipsError>(this->isready(), "object is not ready to process.");
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

  // polynomial interpolation 
  U y;
  if (this->order_ >= this->n_ - 1) {
    // use full region
    y = dopoly(x, 0, this->n_);
  }
  else {
    // use sub-region
    int j = i - 1 - this->order_ / 2;
    unsigned int m = this->n_ - 1 - this->order_;
    unsigned int k = (unsigned int)((j > 0) ? j : 0);
    k = ((k > m) ? m : k);
    y = dopoly(x, k, this->order_ + 1);
  }

  return y;
}

template <class T, class U>
U PolynomialInterpolator1D<T, U>::dopoly(T x, unsigned int left,
                                         unsigned int n)
{
  T *xa = &this->x_[left];
  U *ya = &this->y_[left];

  // storage for C and D in Neville's algorithm
  U *c = new U[n];
  U *d = new U[n];
  for (unsigned int i = 0; i < n; i++) {
    c[i] = ya[i];
    d[i] = ya[i];
  }

  // Neville's algorithm
  U y = c[0];
  for (unsigned int m = 1; m < n; m++) {
    // Evaluate Cm1, Cm2, Cm3, ... Cm[n-m] and Dm1, Dm2, Dm3, ... Dm[n-m].
    // Those are stored to c[0], c[1], ..., c[n-m-1] and d[0], d[1], ..., 
    // d[n-m-1].
    for (unsigned int i = 0; i < n - m; i++) {
      U cd = c[i+1] - d[i];
      T dx = xa[i] - xa[i+m];
      try {
        cd /= (U)dx;
      }
      catch (...) {
        delete[] c;
        delete[] d;
        throw casa::AipsError("x_ has duplicate elements");
      }
      c[i] = (xa[i] - x) * cd;
      d[i] = (xa[i+m] - x) * cd;
    }

    // In each step, c[0] holds Cm1 which is a correction between 
    // P12...m and P12...[m+1]. Thus, the following repeated update 
    // corresponds to the route P1 -> P12 -> P123 ->...-> P123...n.
    y += c[0];
  }

  delete[] c;
  delete[] d;

  return y;
}
}

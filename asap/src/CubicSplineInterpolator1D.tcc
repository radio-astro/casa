//
// C++ Implementation: CubicSplineInterpolator1D
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

#include <iostream>
using namespace std;

#include "CubicSplineInterpolator1D.h"

namespace asap {

template <class T, class U>
CubicSplineInterpolator1D<T, U>::CubicSplineInterpolator1D()
  : Interpolator1D<T, U>(),
    y2_(0),
    ny2_(0),
    reusable_(false)
{}

template <class T, class U>
CubicSplineInterpolator1D<T, U>::~CubicSplineInterpolator1D()
{
  if (y2_) 
    delete[] y2_;
}

template <class T, class U>
void CubicSplineInterpolator1D<T, U>::setData(T *x, U *y, unsigned int n)
{
  Interpolator1D<T, U>::setData(x, y, n);
  reusable_ = false;
}

template <class T, class U>
void CubicSplineInterpolator1D<T, U>::setX(T *x, unsigned int n)
{
  Interpolator1D<T, U>::setX(x, n);
  reusable_ = false;
}

template <class T, class U>
void CubicSplineInterpolator1D<T, U>::setY(U *y, unsigned int n)
{
  Interpolator1D<T, U>::setY(y, n);
  reusable_ = false;
}

template <class T, class U>
U CubicSplineInterpolator1D<T, U>::interpolate(T x)
{
  assert(this->isready());
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

  // determine second derivative of each point
  if (!reusable_) {
    evaly2();
    reusable_ = true;
  }

  // cubic spline interpolation
  float y = dospline(x, i);
  return y;
}

template <class T, class U>
void CubicSplineInterpolator1D<T, U>::evaly2()
{
  if (this->n_ > ny2_) {
    if (y2_) 
      delete[] y2_;
    y2_ = new U[this->n_];
    ny2_ = this->n_;
  }

  U *u = new U[ny2_-1];

  // Natural cubic spline.
  y2_[0] = 0.0;
  y2_[ny2_-1] = 0.0;
  u[0] = 0.0;

  // Solve tridiagonal system.
  // Here, tridiagonal matrix is decomposed to upper triangular 
  // matrix. u stores upper triangular components while y2_ stores 
  // right-hand side vector. The diagonal elements are normalized 
  // to 1.
  T a1 = this->x_[1] - this->x_[0];
  T a2, bi;
  for (unsigned int i = 1; i < ny2_ - 1; i++) {
    a2 = this->x_[i+1] - this->x_[i];
    bi = 1.0 / (this->x_[i+1] - this->x_[i-1]);
    y2_[i] = 3.0 * bi * ((this->y_[i+1] - this->y_[i]) / a2 
                         - (this->y_[i] - this->y_[i-1]) / a1 
                         - y2_[i-1] * 0.5 * a1);
    a1 = 1.0 / (1.0 - u[i-1] * 0.5 * a1 * bi);
    y2_[i] *= a1;
    u[i] = 0.5 * a2 * bi * a1;
    a1 = a2;
  }
  
  // Then, solve the system by backsubstitution and store solution 
  // vector to y2_.
  for (int k = ny2_ - 2; k >= 0; k--)
    y2_[k] -= u[k] * y2_[k+1];

  delete[] u;
}

template <class T, class U>
U CubicSplineInterpolator1D<T, U>::dospline(T x, unsigned int i)
{
  unsigned int j = i - 1;
  T h = this->x_[i] - this->x_[j];
  T a = (this->x_[i] - x) / h;
  T b = (x - this->x_[j]) / h;
  U y = a * this->y_[j] + b * this->y_[i] + 
    ((a * a * a - a) * y2_[j] + (b * b * b - b) * y2_[i]) * (h * h) / 6.0;
  return y;
}

}

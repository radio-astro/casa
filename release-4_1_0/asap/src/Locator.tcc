//
// C++ Implementation: Locator
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

#include "Locator.h"

namespace asap {
template <class T> Locator<T>::Locator()
  : x_(0),
    n_(0),
    ascending_(true),
    copy_(false)
{
}

template <class T> Locator<T>::Locator(T *v, unsigned int n, bool copystorage)
  : x_(0),
    n_(0),
    ascending_(true),
    copy_(false)
{
  set(v, n, copystorage);
}

template <class T> Locator<T>::~Locator()
{
  if (copy_ && x_)
    delete[] x_;
}

template <class T> void Locator<T>::set(T *v, unsigned int n, bool copystorage)
{
  if (copy_) {
    if (!copystorage || n > n_) {
      delete[] x_;
      x_ = 0;
    }
  }
  copy_ = copystorage;
  n_ = n;
  if (copy_) {
    if (!x_)
      x_ = new double[n_];
    for (unsigned int i = 0; i < n_; i++)
      x_[i] = v[i];
  }
  else {
    x_ = v;
  }
  ascending_ = (x_[0] <= x_[n_-1]);
}

template <class T> unsigned int Locator<T>::bisection(T x, unsigned int left, unsigned int right)
{
  unsigned int jl = left;
  unsigned int ju = right;

  if (ascending_) {
    // ascending order
    if (x <= x_[0])
      return 0;
    else if (x > x_[n_-1])
      return n_;

    unsigned int jm;
    while (ju - jl > 1) {
      jm = (ju + jl) / 2;
      if (x > x_[jm])
        jl = jm;
      else
        ju = jm;
    }
  }
  else {
    // descending order
    if (x >= x_[0])
      return 0;
    else if (x < x_[n_-1])
      return n_;

    unsigned int jm;
    while (ju - jl > 1) {
      jm = (ju + jl) / 2;
      if (x < x_[jm])
        jl = jm;
      else
        ju = jm;
    }
  }

  return ju;
}

}

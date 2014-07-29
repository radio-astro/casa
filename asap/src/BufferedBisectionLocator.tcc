//
// C++ Implementation: BufferedBisectionLocator
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

#include "BufferedBisectionLocator.h"

namespace asap {
template <class T> BufferedBisectionLocator<T>::BufferedBisectionLocator()
  : Locator<T>(),
    prev_(0)
{}

template <class T> 
BufferedBisectionLocator<T>::BufferedBisectionLocator(T *v, unsigned int n, 
                                                      bool copystorage)
  : Locator<T>(v, n, copystorage),
    prev_(0)
{}

template <class T> BufferedBisectionLocator<T>::~BufferedBisectionLocator()
{}

template <class T> unsigned int BufferedBisectionLocator<T>::locate(T x)
{
  if (this->n_ == 1)
    return 0;

  unsigned int jl = 0;
  unsigned int ju = this->n_;
  if (this->ascending_) {
    // ascending order
    if (x <= this->x_[0])
      return 0;
    else if (x > this->x_[this->n_-1])
      return this->n_;

    if (x < this->x_[prev_]) {
      ju = this->bisection(x, jl, prev_);
    }
    else {
      ju = this->bisection(x, prev_, ju);
    }

  }
  else {
    // descending order
    if (x >= this->x_[0])
      return 0;
    else if (x < this->x_[this->n_-1])
      return this->n_;

    if (x > this->x_[prev_]) {
      ju = this->bisection(x, jl, prev_);
    }
    else {
      ju = this->bisection(x, prev_, ju);
    }
  }

  prev_ = (ju > 0) ? ju - 1 : 0;
  return ju;    
}

}

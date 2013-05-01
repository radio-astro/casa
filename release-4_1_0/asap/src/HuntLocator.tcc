//
// C++ Implementation: HuntLocator
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

#include "HuntLocator.h"

namespace asap {

template <class T> HuntLocator<T>::HuntLocator()
  : Locator<T>(),
    prev_(0)
{}

template <class T> HuntLocator<T>::HuntLocator(T *v, unsigned int n, bool copystorage)
  : Locator<T>(v, n, copystorage),
    prev_(0)
{}

template <class T> HuntLocator<T>::~HuntLocator()
{}

template <class T> unsigned int HuntLocator<T>::locate(T x)
{
  if (this->n_ == 1)
    return 0;

  if (this->ascending_) {
    if (x <= this->x_[0])
      return 0;
    else if (x > this->x_[this->n_-1])
      return this->n_;
  }
  else {
    if (x > this->x_[0])
      return 0;
    else if (x <= this->x_[this->n_-1])
      return this->n_;
  }

  unsigned int jl = 0;
  unsigned int ju = this->n_;

  // hunt phase
  if (prev_ > 0 && prev_ < this->n_) {
    jl = prev_;
    hunt(x, jl, ju);
  }

  // final bisection phase
  unsigned int j = this->bisection(x, jl, ju);
  prev_ = (j > 0) ? j - 1 : 0;
  return j;
}

template <class T> void HuntLocator<T>::hunt(T x, unsigned int &left, unsigned int &right)
{
  unsigned int inc = 1;
  if (this->ascending_) {
    // ascending order
    if (x >= this->x_[left]) {
      // forward hunt
      if (left >= this->n_ - 1) {
        right = this->n_;
        return;
      }
      right = left + inc;
      while (x >= this->x_[right]) {
        left = right;
        inc *= 2;
        right = left + inc;
        if (right > this->n_ - 1) {
          right = this->n_;
          break;
        }
      }
    }
    else {
      // backward hunt
      if (left == 0) {
        right = 0;
        return;
      }
      right = left;
      left -= inc;
      while (x < this->x_[left]) {
        right = left;
        inc *= 2;
        if (inc >= right) {
          left = 0;
          break;
        }
        left = right - inc;
      }
    }
  }
  else {
    // descending order
    if (x < this->x_[left]) {
      // forward hunt
      if (left >= this->n_ - 1) {
        right = this->n_;
        return;
      }
      right = left + inc;
      while (x < this->x_[right]) {
        left = right;
        inc *= 2;
        right = left + inc;
        if (right > this->n_ - 1) {
          right = this->n_;
          break;
        }
      }
    }
    else {
      // backward hunt
      if (left == 0) 
        return;
      right = left;
      left -= inc;
      while (x >= this->x_[left]) {
        right = left;
        inc *= 2;
        if (inc >= right) {
          left = 0;
          break;
        }
        left = right - inc;
      }
    }
  }
}
}

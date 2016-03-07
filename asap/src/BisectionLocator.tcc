//
// C++ Implementation: BisectionLocator
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

#include "BisectionLocator.h"

namespace asap {
template <class T> BisectionLocator<T>::BisectionLocator()
  : Locator<T>()
{
}


template <class T> BisectionLocator<T>::BisectionLocator(T *v, unsigned int n, bool copystorage)
  : Locator<T>(v, n, copystorage)
{}

template <class T> BisectionLocator<T>::~BisectionLocator()
{}

template <class T> unsigned int BisectionLocator<T>::locate(T x)
{
  if (this->n_ == 1)
    return 0;

  return this->bisection(x, 0, this->n_);
}

}

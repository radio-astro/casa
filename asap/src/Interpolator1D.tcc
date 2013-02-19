//
// C++ Implementation: Interpolator1D
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
#include <casa/Utilities/Assert.h>

#include "Interpolator1D.h"
#include "Locator.h"
#include "BisectionLocator.h"
#include "HuntLocator.h"

using namespace casa;

namespace asap {

template <class T, class U> Interpolator1D<T, U>::Interpolator1D()
  : order_(1),
    n_(0),
    x_(0),
    y_(0),
    locator_(0)
{
}

template <class T, class U> Interpolator1D<T, U>::~Interpolator1D()
{
  if (locator_)
    delete locator_;
}

template <class T, class U> 
void Interpolator1D<T, U>::setData(T *x, U *y, unsigned int n)
{
  x_ = x;
  y_ = y;
  n_ = n;
  createLocator();
  locator_->set(x, n);
}

template <class T, class U> 
void Interpolator1D<T, U>::setX(T *x, unsigned int n)
{
  //assert(n_ == 0 || n_ == n);
  casa::assert_<casa::AipsError>(n_ == 0 || n_ == n, "length mismatch in data.");
  x_ = x;
  n_ = n;
  createLocator();
  locator_->set(x, n);
}

template <class T, class U> 
void Interpolator1D<T, U>::setY(U *y, unsigned int n)
{
  //assert(n_ == 0 || n_ == n);
  casa::assert_<casa::AipsError>(n_ == 0 || n_ == n, "length mismatch in data.");
  y_ = y;
  n_ = n;
}

template <class T, class U> void Interpolator1D<T, U>::reset()
{
  n_ = 0;
  x_ = 0;
  y_ = 0;
  if (locator_) {
    delete locator_;
    locator_ = 0;
  }
}

template <class T, class U> bool Interpolator1D<T, U>::isready()
{
  return (n_ > 0 && x_ != 0 && y_ != 0);
}

template <class T, class U> unsigned int Interpolator1D<T, U>::locate(T x)
{
  return locator_->locate(x);
}

template <class T, class U> void Interpolator1D<T, U>::createLocator()
{
  if (!locator_) {
    if (n_ > 1000)
      locator_ = new HuntLocator<T>();
    else
      locator_ = new BisectionLocator<T>();
  }
}

}

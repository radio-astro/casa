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

#include "Interpolator1D.h"
#include "BisectionLocator.h"

using namespace casa;

namespace asap {

Interpolator1D::Interpolator1D()
  : order_(0),
    n_(0),
    x_(0),
    y_(0)
{
  locator_ = new BisectionLocator();
}

Interpolator1D::~Interpolator1D()
{
  if (locator_)
    delete locator_;
}

void Interpolator1D::setData(double *x, float *y, unsigned int n)
{
  x_ = x;
  y_ = y;
  n_ = n;
  locator_->set(x, n);
}

void Interpolator1D::setX(double *x, unsigned int n)
{
  assert(n_ == 0 || n_ == n);
  x_ = x;
  n_ = n;
  locator_->set(x, n);
}

void Interpolator1D::setY(float *y, unsigned int n)
{
  assert(n_ == 0 || n_ == n);
  y_ = y;
  n_ = n;
}

void Interpolator1D::reset()
{
  n_ = 0;
  x_ = 0;
  y_ = 0;
}

bool Interpolator1D::isready()
{
  return (n_ > 0 && x_ != 0 && y_ != 0);
}

int Interpolator1D::locate(double x)
{
  return locator_->locate(x);
}

}

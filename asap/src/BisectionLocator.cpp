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

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Exceptions/Error.h>

#include "BisectionLocator.h"

using namespace casa;

namespace asap {

BisectionLocator::BisectionLocator(double *v, unsigned int n)
  : Locator(v, n)
{}

BisectionLocator::~BisectionLocator()
{}

unsigned int BisectionLocator::locate(double x)
{
  if (n_ == 1)
    return 0;
  bool ascending = (x_[n_-1] >= x_[0]);
  if (ascending) {
    if (x <= x_[0])
      return 0;
    else if (x > x_[n_-1])
      return n_;
  }
  else {
    if (x > x_[0])
      return 0;
    else if (x <= x_[n_-1])
      return n_;
  }
  unsigned int jl = 0;
  unsigned int ju = n_;
  unsigned int jm;

  while (ju - jl > 1) {
    jm = (ju + jl) >> 1;
    if ((x >= x_[jm]) == ascending)
      jl = jm;
    else
      ju = jm;
  }
  return ju;
}

}

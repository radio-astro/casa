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

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>

#include "Locator.h"

using namespace casa;

namespace asap {

Locator::Locator(double *v, unsigned int n)
{
  set(v, n);
}

Locator::~Locator()
{}

void Locator::set(double *v, unsigned int n)
{
  x_ = v;
  n_ = n;
}

}

//
// C++ Interface: BisectionLocator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_BISECTION_LOCATOR_H
#define ASAP_BISECTION_LOCATOR_H

#include <memory>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include "Locator.h"

namespace asap {

/**
 * Implementation of locate operation by bisection search 
 * @author TakeshiNakazato
 */
class BisectionLocator : public Locator {
public:
  BisectionLocator() {;}
  BisectionLocator(double *v, unsigned int n);

  virtual ~BisectionLocator();

  unsigned int locate(double x);
};

}
#endif

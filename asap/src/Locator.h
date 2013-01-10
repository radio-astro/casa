//
// C++ Interface: Locator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_LOCATOR_H
#define ASAP_LOCATOR_H

#include <memory>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

namespace asap {

/**
 * Base class for locate operation 
 * @author TakeshiNakazato
 */
class Locator {
public:
  Locator() {;}
  Locator(double *v, unsigned int n);
  void set(double *v, unsigned int n);

  virtual ~Locator();

  // return right hand side index of location 
  // (return j+1 if x[j] < x <= x[j+1])
  // return value 0 or x.nelements() indicates out of range 
  virtual unsigned int locate(double x) = 0;

protected:
  double *x_;
  unsigned int n_;
};

}
#endif

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

#include "Locator.h"

namespace asap {

/**
 * Implementation of locate operation by bisection search 
 * @author TakeshiNakazato
 */
template <class T> class BisectionLocator : public Locator<T> {
public:
  // Default constructor.
  BisectionLocator();

  // Construct with data
  // @param[in] v pointer to input data.
  // @param[in] n length of the data.
  // @param[in] copystorage whether allocate internal memory or not.
  // @see Locator::set()
  BisectionLocator(T *v, unsigned int n, bool copystorage=true);

  // Destructor.
  virtual ~BisectionLocator();

  // Return right hand side index of location using bisection search.
  // @param[in] x input value to be located.
  // @return location as an index j.
  // @see Locator::locate()
  unsigned int locate(T x);
};

}

#include "BisectionLocator.tcc"

#endif

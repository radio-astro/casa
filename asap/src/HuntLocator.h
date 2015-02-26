//
// C++ Interface: HuntLocator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_HUNT_LOCATOR_H
#define ASAP_HUNT_LOCATOR_H

#include "Locator.h"

namespace asap {

/**
 * Implementation of locate operation by bisection search 
 * @author TakeshiNakazato
 */
template <class T> class HuntLocator : public Locator<T> {
public:
  // Default constructor.
  HuntLocator();

  // Construct with data
  // @param[in] v pointer to input data.
  // @param[in] n length of the data.
  // @param[in] copystorage whether allocate internal memory or not.
  // @see Locator::set()
  HuntLocator(T *v, unsigned int n, bool copystorage=true);

  // Destructor.
  virtual ~HuntLocator();

  // Return right hand side index of location using bisection search 
  // plus hunt algorithm.
  // @param[in] x input value to be located.
  // @return location as an index j.
  // @see Locator::locate()
  unsigned int locate(T x);
private:
  // Hunt algorithm
  // @param[in] x input value to be located.
  // @param[in,out] left input: the starting point for hunt.
  //                     output: the left index of hunted region.
  // @param[out] right the right index of hunted region.
  void hunt(T x, unsigned int &left, unsigned int &right);

  // Storage for previous result.
  unsigned int prev_;
};

}

#include "HuntLocator.tcc"

#endif

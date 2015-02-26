//
// C++ Interface: PolynomialInterpolator1D
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_POLYNOMIAL_INTERPOLATOR_1D_H
#define ASAP_POLYNOMIAL_INTERPOLATOR_1D_H

#include "Interpolator1D.h"

namespace asap {

/**
 * Implementation of polynomial interpolation.
 * @author TakeshiNakazato
 */
template <class T, class U>
class PolynomialInterpolator1D : public Interpolator1D<T, U> {
public:
  // Default constructor.
  PolynomialInterpolator1D();

  // Destructor.
  virtual ~PolynomialInterpolator1D();

  // Perform interpolation.
  // @param[in] x horizontal location where the value is evaluated 
  //              by interpolation.
  // @return interpolated value at x.
  U interpolate(T x);
private:
  // Perform polynomial interpolation. 
  // If (number of data points) >  (polynomial order + 1), polynomial 
  // interpolation must be done in the sub-region that contains x. 
  // This method takes arguments that specifies sub-region to be used. 
  // @param[in] x horizontal location where the value is evaluated 
  //              by interpolation.
  // @param[in] left the leftmost index of sub-region.
  // @param[in] n number of data points of sub-region.
  // @return interpolated value at x.
  U dopoly(T x, unsigned int left, unsigned int n);
};

}

#include "PolynomialInterpolator1D.tcc"

#endif

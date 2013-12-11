//
// C++ Interface: Interpolator1D
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_INTERPOLATOR_1D_H
#define ASAP_INTERPOLATOR_1D_H

#include "Locator.h"

namespace asap {

/**
 * Base class for interpolation operation 
 * @author TakeshiNakazato
 */
template <class T, class U> class Interpolator1D {
public:
  // Default constructor.
  Interpolator1D();

  // Destructor.
  virtual ~Interpolator1D();

  // Set horizontal (x) and vertical (y) data.
  // @param[in] x pointer to horizontal data.
  // @param[in] y pointer to vertical data.
  // @param[in] n number of data.
  virtual void setData(T *x, U *y, unsigned int n);

  // Set horizontal data (x).
  // @param[in] x pointer to horizontal data.
  // @param[in] n number of data.
  virtual void setX(T *x, unsigned int n);

  // Set vertical data (y).
  // @param[in] y pointer to vertical data.
  // @param[in] n number of data.
  virtual void setY(U *y, unsigned int n);

  // Reset object.
  void reset();

  // Set order for polynomial interpolation.
  // @param order order of the polynomial.
  // 
  // This method is effective only for polynomial interpolation.
  void setOrder(unsigned int order) {order_ = order;}

  // Perform interpolation.
  // @param[in] x horizontal location where the value is evaluated 
  //              by interpolation.
  // @return interpolated value at x.
  virtual U interpolate(T x) = 0;

protected:
  // Locate x.
  // @param[in] x horizontal location.
  // @return location as an index.
  // @see Locator::locate()
  unsigned int locate(T x);

  // Query function whether the object is ready to interpolate.
  // @return true if object is ready else false.
  bool isready();

  // Fuctory method to create appropriate Locator object.
  void createLocator();

  // Order of the polynomial (only effective for polynomial interpolation).
  unsigned int order_;

  // Number of data points.
  unsigned int n_;

  // Horizontal data.
  T *x_;

  // Vertical data.
  U *y_;

  // Pointer to the Locator object.
  Locator<T> *locator_;
};

}

#include "Interpolator1D.tcc"

#endif

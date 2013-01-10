//
// C++ Interface: NearestInterpolator1D
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_NEAREST_INTERPOLATOR_1D_H
#define ASAP_NEAREST_INTERPOLATOR_1D_H

#include <memory>
#include <vector>

#include <casa/aips.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include "Interpolator1D.h"

namespace asap {

/**
 * Base class for interpolation operation 
 * @author TakeshiNakazato
 */
class NearestInterpolator1D : public Interpolator1D {
public:
  NearestInterpolator1D();

  virtual ~NearestInterpolator1D();

  float interpolate(double x);
};

}
#endif

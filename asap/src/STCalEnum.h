//
// C++ Interface: STCalEnum
//
// Description:
//
// Apply any apply tables to target data.
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CAL_ENUM_H
#define ASAP_CAL_ENUM_H

#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

//#include "STApplyTable.h"

namespace asap {

/**
Apply any apply tables to target data

@author Takeshi Nakazato
@date $Date:$
@version $Revision:$
*/
class STCalEnum  {
public:
  enum InterpolationType {DefaultInterpolation = 0,
                          NearestInterpolation,
                          LinearInterpolation,
                          PolynomialInterpolation,
                          CubicSplineInterpolation};
  enum InterpolationAxis {TimeAxis = 0,
                          FrequencyAxis,
                          NumAxis};
  enum CalType {DefaultType = 0,
                CalTsys,
                CalPSAlma,
                CalPS,
                CalNod,
                CalFS,
                NoType};
};

}

#endif

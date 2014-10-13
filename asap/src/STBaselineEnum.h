//
// C++ Interface: STBaselineEnum
//
// Description:
//
// Apply any apply tables to target data.
//
// Author: Wataru Kawasaki <wataru.kawasaki@nao.ac.jp> (C) 2013
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_BASELINE_ENUM_H
#define ASAP_BASELINE_ENUM_H

#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

namespace asap {

/**
Apply any apply tables to target data

@author Wataru Kawasaki
@date $Date:$
@version $Revision:$
*/
class STBaselineFunc  {
public:
  enum FuncName {Polynomial = 1,
                 CSpline,
                 Sinusoid,
                 Chebyshev};
};

}

#endif

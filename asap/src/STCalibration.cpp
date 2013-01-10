//
// C++ Implementation: STCalibration
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "STCalibration.h"
#include "STCalSkyTable.h"

using namespace casa;

namespace asap {
STCalibration::STCalibration(CountedPtr<Scantable> &s)
  : scantable_(s)
{
}

}

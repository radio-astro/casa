//
// C++ Interface: STCalTsys
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALTSYS_H
#define ASAP_CALTSYS_H

#include <memory>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

#include "RowAccumulator.h"
#include "Scantable.h"
#include "STDefs.h"
#include "STApplyTable.h"
#include "STCalibration.h"
#include "STCalTsysTable.h"

namespace asap {

/**
 * Calibration operations on Scantable objects
 * @author TakeshiNakazato
 */
class STCalTsys : public STCalibration {
public:
  STCalTsys(casa::CountedPtr<Scantable> &s, vector<int> &iflist);

  ~STCalTsys() {;}
  
private:
  void setupSelector(const STSelector &sel);
  void fillCalTable();

  vector<int> iflist_;
};

}
#endif

//
// C++ Interface: STCalSkyPSAlma
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALSKY_PS_ALMA_H
#define ASAP_CALSKY_PS_ALMA_H

#include <memory>

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
#include "STCalSkyTable.h"


namespace asap {

/**
 * Calibration operations on Scantable objects
 * @author TakeshiNakazato
 */
class STCalSkyPSAlma : public STCalibration {
public:
  STCalSkyPSAlma(casa::CountedPtr<Scantable> &s);

  virtual void calibrate();

  ~STCalSkyPSAlma() {;}
private:
  void fillCalTable();
};

}
#endif

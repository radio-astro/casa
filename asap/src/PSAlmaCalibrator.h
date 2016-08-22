//
// C++ Interface: PSAlmaCalibrator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_PS_ALMA_CALIBRATOR_H
#define ASAP_PS_ALMA_CALIBRATOR_H

#include <memory>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include "Calibrator.h"

namespace asap {

/**
 * Calibration operation for position switch (ALMA)
 * @author TakeshiNakazato
 */
class PSAlmaCalibrator : public Calibrator {
public:
  PSAlmaCalibrator();
  PSAlmaCalibrator(unsigned int nchan);
  virtual ~PSAlmaCalibrator() {;}

  void calibrate();
};

}
#endif

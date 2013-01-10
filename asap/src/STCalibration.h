//
// C++ Interface: STCalibration
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTCALIBRATION_H
#define ASAPSTCALIBRATION_H

#include <memory>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

#include "Scantable.h"
#include "STDefs.h"
#include "STApplyTable.h"


namespace asap {

/**
 * Calibration operations on Scantable objects
 * @author TakeshiNakazato
 */
class STCalibration {
public:
  STCalibration(casa::CountedPtr<Scantable> &s);

  virtual void calibrate() = 0;

  virtual ~STCalibration() {;}

  void save(casa::String name) {applytable_->save(name);}
protected:
  casa::CountedPtr<Scantable> scantable_;
  casa::CountedPtr<STApplyTable> applytable_; 
};

}
#endif

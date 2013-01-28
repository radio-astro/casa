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
#include <casa/Logging/LogIO.h>

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

  void calibrate();

  virtual ~STCalibration() {;}

  void save(casa::String name) {applytable_->save(name);}
  //const STApplyTable &applytable() {return *applytable_;}
  const casa::CountedPtr<STApplyTable> applytable() {return applytable_;}
protected:
  virtual void setupSelector() = 0;
  virtual void fillCalTable() = 0;

  STSelector sel_;
  casa::CountedPtr<Scantable> scantable_;
  casa::CountedPtr<STApplyTable> applytable_; 
  casa::LogIO os_;
};

}
#endif

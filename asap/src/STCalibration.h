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

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Logging/LogIO.h>
#include <casa/Containers/Record.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

#include "Scantable.h"
#include "STDefs.h"
#include "STApplyTable.h"
#include "STSelector.h"

namespace asap {

/**
 * Calibration operations on Scantable objects
 * @author TakeshiNakazato
 */
class STCalibration {
public:
  STCalibration(casa::CountedPtr<Scantable> &s, const casa::String target_column);

  void calibrate();

  virtual ~STCalibration() {;}

  void save(casa::String name) {applytable_->save(name);}
  //const STApplyTable &applytable() {return *applytable_;}
  const casa::CountedPtr<STApplyTable> applytable() {return applytable_;}

  void setOption(casa::Record &rec) {options_ = rec;}

protected:
  virtual void setupSelector(const STSelector &sel) = 0;
  virtual void fillCalTable();
  virtual void appenddata(casa::uInt scanno, casa::uInt cycleno, 
			  casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
			  casa::uInt freqid, casa::Double time, casa::Float elevation, 
			  const casa::Vector<casa::Float> &any_data,
			  const casa::Vector<casa::uChar> &channel_flag) = 0;

  STSelector sel_;
  casa::CountedPtr<Scantable> scantable_;
  casa::CountedPtr<STApplyTable> applytable_; 
  casa::LogIO os_;
  casa::Record options_;
  const casa::String target_column_;
};
 
}
#endif

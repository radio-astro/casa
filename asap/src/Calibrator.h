//
// C++ Interface: Calibrator
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALIBRATOR_H
#define ASAP_CALIBRATOR_H

#include <memory>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

namespace asap {

/**
 * Base class for calibration operation 
 * @author TakeshiNakazato
 */
class Calibrator {
public:
  Calibrator();
  Calibrator(unsigned int nchan);

  virtual ~Calibrator();

  void setSource(casa::Vector<casa::Float> &v);
  void setReference(casa::Vector<casa::Float> &v);
  void setReference2(casa::Vector<casa::Float> &v);
  void setScaler(casa::Vector<casa::Float> &v);

  const casa::Vector<casa::Float> getCalibrated();

  virtual void calibrate() = 0;

protected:
  void initStorage();
  void freeStorage();
  void set(casa::Float *p, casa::Vector<casa::Float> &v);

  unsigned int nchan_;
  unsigned int nchanS_;

  casa::Float *source_;
  casa::Float *ref_;
  casa::Float *ref2_;
  casa::Float *scaler_;
  casa::Float *calibrated_;
};

}
#endif

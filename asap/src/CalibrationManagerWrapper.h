//
// C++ Interface: CalibrationManagerWrapper
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALIBRATION_MANAGER_WRAPPER_H
#define ASAP_CALIBRATION_MANAGER_WRAPPER_H

#include <string>
#include <vector>
#include <unistd.h>


#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Logging/LogIO.h>

#include "ScantableWrapper.h"
#include "CalibrationManager.h"
#include "GILHandler.h"

namespace asap {

/**
 * Class for calibration management. 
 * It also intends to be an interface for Python layer.
 * @author TakeshiNakazato
 */
class CalibrationManagerWrapper {
public:
  CalibrationManagerWrapper()
  : calibrationManagerImpl_()
  {}

  virtual ~CalibrationManagerWrapper()
  {}

  void setScantable(ScantableWrapper &s)
  {
    calibrationManagerImpl_.setScantable(s);
  }
  void setScantableByName(const std::string &s)
  {
    calibrationManagerImpl_.setScantableByName(s);
  }
  void addApplyTable(const std::string &c)
  {
    calibrationManagerImpl_.addApplyTable(c);
  }
  void addSkyTable(const std::string &c)
  {
    calibrationManagerImpl_.addSkyTable(c);
  }
  void addTsysTable(const std::string &c)
  {
    calibrationManagerImpl_.addTsysTable(c);
  }
  void setMode(const std::string &mode)
  {
    calibrationManagerImpl_.setMode(mode);
  }
  void setTimeInterpolation(const std::string &interp, int order=-1)
  {
    calibrationManagerImpl_.setTimeInterpolation(interp, order);
  }
  void setFrequencyInterpolation(const std::string &interp, int order=-1)
  {
    calibrationManagerImpl_.setFrequencyInterpolation(interp, order);
  }
  void setTsysSpw(const std::vector<int> &spwlist)
  {
    calibrationManagerImpl_.setTsysSpw(spwlist);
  }
  void setTsysSpwWithRange(const casa::Record &spwlist, bool average=false)
  {
    calibrationManagerImpl_.setTsysSpwWithRange(spwlist, average);
  }
  void setTsysTransfer(unsigned int from, 
                       const std::vector<unsigned int> &to)
  {
    calibrationManagerImpl_.setTsysTransfer(from, to);
  }
  void setCalibrationOptions(const casa::Record &options)
  {
    calibrationManagerImpl_.setCalibrationOptions(options);
  }
  void resetCalSetup()
  {
    calibrationManagerImpl_.resetCalSetup();
  }
  void reset()
  {
    calibrationManagerImpl_.reset();
  }
  
  void calibrate()
  {
    GILHandler scopedRelease;
    calibrationManagerImpl_.calibrate();
  }
  void apply(bool insitu=false, bool filltsys=true)
  {
    GILHandler scopedRelease;
    calibrationManagerImpl_.apply(insitu, filltsys);
  }
  void saveCaltable(const std::string &name)
  {
    calibrationManagerImpl_.saveCaltable(name);
  }
  void split(const std::string &name)
  {
    calibrationManagerImpl_.split(name);
  }
private:
  CalibrationManager calibrationManagerImpl_;
};

}
#endif

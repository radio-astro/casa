//
// C++ Interface: CalibrationManager
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALIBRATION_MANAGER_H
#define ASAP_CALIBRATION_MANAGER_H

#include <string>
#include <vector>

//#include <boost/scoped_ptr>

#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Logging/LogIO.h>

#include "ScantableWrapper.h"
#include "STApplyCal.h"
#include "STCalTsys.h"
#include "STCalibration.h"
#include "STCalEnum.h"

namespace asap {

/**
 * Class for calibration management. 
 * It also intends to be an interface for Python layer.
 * @author TakeshiNakazato
 */
class CalibrationManager {
public:
  CalibrationManager();

  virtual ~CalibrationManager();

  void setScantable(ScantableWrapper &s);
  void setScantableByName(const std::string &s);
  void addApplyTable(const std::string &c);
  void addSkyTable(const std::string &c);
  void addTsysTable(const std::string &c);
  void setMode(const std::string &mode);
  void setTimeInterpolation(const std::string &interp, int order=-1);
  void setFrequencyInterpolation(const std::string &interp, int order=-1);
  void setTsysSpw(const std::vector<int> &spwlist);
  void setTsysSpwWithRange(const casa::Record &spwlist, bool average=false);
  void setTsysTransfer(unsigned int from, 
                       const std::vector<unsigned int> &to);
  void setCalibrationOptions(const casa::Record &options) {options_ = options;}
  void resetCalSetup();
  void reset();
  
  void calibrate();
  void apply(bool insitu=false, bool filltsys=true);
  void saveCaltable(const std::string &name);
  void split(const std::string &name);
private:
  STCalEnum::InterpolationType stringToInterpolationEnum(const std::string &s);

  casa::Bool isAlmaAntenna();

  casa::CountedPtr<STApplyCal> applicator_;

  std::vector<casa::CountedPtr<STApplyTable> > skytables_;
  std::vector<casa::CountedPtr<STApplyTable> > tsystables_;

  casa::CountedPtr<Scantable> target_;

  casa::String calmode_;
  std::vector<int> spwlist_;
  casa::Record spwlist_withrange_;
  bool do_average_;

  casa::LogIO os_;

  casa::Record options_;
};

}
#endif

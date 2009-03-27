//
// C++ Interface: STFitEntry
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTFITENTRY_H
#define ASAPSTFITENTRY_H

#include <vector>
#include <string>

namespace asap {

/**
This is a helper object for transporting Fit parameters

@author Malte Marquarding
*/
class STFitEntry{
public:
  STFitEntry();
  STFitEntry(const STFitEntry& other);

  ~STFitEntry();

  void setFunctions(const std::vector<std::string>& f)
    { functions_ = f; }
  void setComponents(const std::vector<int>& c)
    { components_ = c; }
  void setParameters(const std::vector<double>& p)
    { parameters_ = p; }
  void setParmasks(const std::vector<bool>& m)
    { parmasks_ = m; }
  void setFrameinfo(const std::vector<std::string>& f)
    { frameinfo_ = f; }

  std::vector<std::string> getFunctions() const { return functions_; }
  std::vector<int> getComponents() const { return components_; }
  std::vector<double> getParameters() const { return parameters_; }
  std::vector<bool> getParmasks() const { return parmasks_; }
  std::vector<std::string> getFrameinfo() const { return frameinfo_; }

private:
  std::vector<std::string> functions_;
  std::vector<int> components_;
  std::vector<double> parameters_;
  std::vector<bool> parmasks_;
  std::vector<std::string> frameinfo_;
};

}

#endif

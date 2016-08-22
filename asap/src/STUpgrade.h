//
// C++ Interface: STUpgrade
//
// Description:
//   Seemlessly upgrade an old version of scantable files to the current
//   schema.
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTUPGRADE_H
#define ASAPSTUPGRADE_H

#include <casa/aips.h>
#include <string>


namespace asap {

/**
A class to upgrade Scantable schemas

@author Malte Marquarding
*/
class STUpgrade {
public:
  explicit STUpgrade(casa::uInt version) { version_ = version ;}
  virtual ~STUpgrade() {;}

  std::string two2three(const std::string& name);
  std::string three2four(const std::string& name);

  std::string upgrade(const std::string& name);
  
private:
  casa::uInt version_;
};

}

#endif

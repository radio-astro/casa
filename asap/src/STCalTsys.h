//
// C++ Interface: STCalTsys
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALTSYS_H
#define ASAP_CALTSYS_H

#include <memory>
#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/Record.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

#include "RowAccumulator.h"
#include "Scantable.h"
#include "STDefs.h"
#include "STApplyTable.h"
#include "STCalibration.h"
#include "STCalTsysTable.h"

namespace asap {

/**
 * Calibration operations on Scantable objects
 * @author TakeshiNakazato
 */
class STCalTsys : public STCalibration {
public:
  STCalTsys(casa::CountedPtr<Scantable> &s, vector<int> &iflist);
  STCalTsys(casa::CountedPtr<Scantable> &s, casa::Record &iflist, bool average=false);

  ~STCalTsys() {;}
  
private:
  void setupSelector(const STSelector &sel);
  virtual void appenddata(casa::uInt scanno, casa::uInt cycleno, 
			  casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
			  casa::uInt freqid, casa::Double time, casa::Float elevation, 
			  const casa::Vector<casa::Float> &any_data,
			  const casa::Vector<casa::uChar> &channel_flag);

  vector<int> iflist_;
  casa::Record tsysspw_;
  bool do_average_;
};

}
#endif

//
// C++ Interface: STApplyCal
//
// Description:
//
// Apply any apply tables to target data.
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp> (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_APPLY_CAL_H
#define ASAP_APPLY_CAL_H

#include <map>
#include <vector>

#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>

#include "Scantable.h"
#include "STSelector.h"
#include "STApplyTable.h"
#include "STCalEnum.h"
//#include "Calibrator.h"
//#include "Interpolator1D.h"
#include "STCalSkyTable.h"
#include "STCalTsysTable.h"

namespace asap {

template<class T, class U> class Interpolator1D;
class Calibrator;

/**
Apply any apply tables to target data

@author Takeshi Nakazato
@date $Date:$
@version $Revision:$
*/
class STApplyCal  {
public:
  STApplyCal();
  STApplyCal(casa::CountedPtr<Scantable> target);

  ~STApplyCal();

  // set data
  void setTarget(casa::CountedPtr<Scantable> target);
  void setTarget(const casa::String &name);

  // push new caltable 
  void push(STCalSkyTable *table);
  void push(STCalTsysTable *table);

  // set interpolation method
  //void setInterpolation(STCalEnum::InterpolationAxis axis, STCalEnum::InterpolationType itype, casa::Int order=-1);
  void setTimeInterpolation(STCalEnum::InterpolationType itype, casa::Int order=-1);
  void setFrequencyInterpolation(STCalEnum::InterpolationType itype, casa::Int order=-1);

  // set IF (spw) mapping for Tsys transfer
  void setTsysTransfer(casa::uInt from, casa::Vector<casa::uInt> to);

  // apply tables
  void apply(casa::Bool insitu=false, casa::Bool filltsys=true);

  // split target data and store it to disk
  void save(const casa::String &name);

  // reset all settings except target scantable
  void reset();

  // reset all settings
  void completeReset();
  
private:
  // initialization
  void init();

  // setup interpolator
  void initInterpolator();

  // single loop element in apply()
  void doapply(casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
               casa::Vector<casa::uInt> &rows,
               casa::Vector<casa::uInt> &skylist,
               casa::Bool filltsys=true);

  // get frequency information from FREQUENCIES subtable
  casa::Vector<casa::Double> getBaseFrequency(casa::uInt whichrow);

  // search spwmap_ to get IFNO for Tsys
  casa::uInt getIFForTsys(casa::uInt to);

  // target data
  casa::CountedPtr<Scantable> target_;

  // working data
  casa::CountedPtr<Scantable> work_;

  // calibrator
  casa::CountedPtr<Calibrator> calibrator_;

  // interpolation method
  STCalEnum::InterpolationType iTime_;
  STCalEnum::InterpolationType iFreq_;
  casa::Int order_;
  casa::CountedPtr<Interpolator1D<casa::Double, casa::Float> > interpolatorT_;
  casa::CountedPtr<Interpolator1D<casa::Double, casa::Float> > interpolatorF_;
  casa::CountedPtr<Interpolator1D<casa::Double, casa::Float> > interpolatorS_;

  // IF (spw) mapping for Tsys transfer
  map<casa::uInt, casa::Vector<casa::uInt> > spwmap_;

  // list of apply tables
  std::vector<STCalSkyTable*> skytable_;
  std::vector<STCalTsysTable*> tsystable_;

  // calibration type
  STCalEnum::CalType caltype_;
  casa::Bool doTsys_;

  // selector
  STSelector sel_;

  // logger
  casa::LogIO os_;
};

}

#endif

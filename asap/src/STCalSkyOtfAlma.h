//
// C++ Interface: STCalSkyOtfAlma
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_CALSKY_OTF_ALMA_H
#define ASAP_CALSKY_OTF_ALMA_H

#include <memory>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include <scimath/Mathematics/InterpolateArray1D.h>

#include "RowAccumulator.h"
#include "Scantable.h"
#include "STDefs.h"
#include "STApplyTable.h"
#include "STCalSkyPSAlma.h"

namespace asap {

/**
 * Calibration operations on Scantable objects
 * @author TakeshiNakazato
 */
class STCalSkyOtfAlma : public STCalSkyPSAlma {
public:
  STCalSkyOtfAlma(casa::CountedPtr<Scantable> &s, bool israster=false);

  virtual ~STCalSkyOtfAlma() {;}
  
protected:
  virtual void setupSelector(const STSelector &sel);
  //virtual void fillCalTable();

private:
  bool israster_;

  casa::Vector<casa::uInt> rowNumbers_;
};

}
#endif

//
// C++ Interface: STCalSky
//
// Description:
//
// ApplyTable for sky calibration.
//
// Author: Takeshi Nakazato 
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPCALSKY_H
#define ASAPCALSKY_H

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include "Scantable.h"
#include "STApplyTable.h"

namespace asap {

/**
ApplyTable for sky calibration

@author Takeshi Nakazato
@date $Date:$
@version $Revision:$
*/
class STCalSky : public STApplyTable {
public:
  STCalSky() {;}
  STCalSky(const Scantable& parent);

  virtual ~STCalSky();

  void setup();
  void attachOptionalColumns();

  const casa::String& name() const {return name_;}

  void setdata(casa::uInt irow, casa::uInt scannos, casa::uInt cycleno, 
               casa::uInt beamno, casa::uInt ifno, 
               casa::uInt polno, casa::Double time, casa::Float elevation, 
               casa::Vector<casa::Float> spectra);
  void appenddata(casa::uInt scanno, casa::uInt cycleno, casa::uInt beamno, 
                  casa::uInt ifno, casa::uInt polno, 
                  casa::Double time, casa::Float elevation, 
                  casa::Vector<casa::Float> spectra);
  
  casa::Vector<casa::Float> getElevation() {return elCol_.getColumn();}
  casa::Matrix<casa::Float> getSpectra() {return spectraCol_.getColumn();}

private:
  static const casa::String name_ ;
  casa::ArrayColumn<casa::Float> spectraCol_;
  casa::ScalarColumn<casa::Float> elCol_;
};

}

#endif

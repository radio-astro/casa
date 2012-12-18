//
// C++ Interface: STCalTsys
//
// Description:
//
// ApplyTable for Tsys calibration.
//
// Author: Takeshi Nakazato 
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPCALTSYS_H
#define ASAPCALTSYS_H

#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include "Scantable.h"
#include "STApplyTable.h"

namespace asap {

/**
ApplyTable for Tsys calibration

@author Takeshi Nakazato
@date $Date:$
@version $Revision:$
*/
class STCalTsys : public STApplyTable {
public:
  STCalTsys() {;}
  STCalTsys(const Scantable& parent);

  virtual ~STCalTsys();

  void setup();
  const casa::String& name() const {return name_;};

  void attachOptionalColumns();

  void setdata(casa::uInt irow, casa::uInt scanno, casa::uInt cycleno, 
               casa::uInt beamno, casa::uInt ifno, 
               casa::uInt polno, casa::Double time, casa::Float elevation, 
               casa::Vector<casa::Float> tsys);
  void appenddata(casa::uInt scanno, casa::uInt cycleno, 
                  casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
                  casa::Double time, casa::Float elevation, 
                  casa::Vector<casa::Float> tsys);
  
  casa::Vector<casa::Float> getElevation() {return elCol_.getColumn();}
  casa::Matrix<casa::Float> getTsys() {return tsysCol_.getColumn();}

private:
  static const casa::String name_ ;
  casa::ArrayColumn<casa::Float> tsysCol_;
  casa::ScalarColumn<casa::Float> elCol_;
};

}

#endif

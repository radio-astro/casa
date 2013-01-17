//
// C++ Interface: STBaselineParamTable
//
// Description:
//
// ApplyTable for baseline subtraction.
//
// Author: Wataru Kawasaki <wataru.kawasaki@nao.ac.jp> (C) 2013
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAP_BASELINEPARAM_TABLE_H
#define ASAP_BASELINEPARAM_TABLE_H

#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include "Scantable.h"
#include "STApplyTable.h"
#include "STBaselineEnum.h"

namespace asap {

/**
ApplyTable for baseline subtraction

@author Wataru Kawasaki
@date $Date:$
@version $Revision:$
*/
class STBaselineParamTable : public STApplyTable {
public:
  STBaselineParamTable() {;}
  STBaselineParamTable(const Scantable& parent);
  STBaselineParamTable(const casa::String &name);

  virtual ~STBaselineParamTable();

  void setup();
  const casa::String& name() const {return name_;};

  void attachOptionalColumns();

  void setdata(casa::uInt irow, casa::uInt scanno, casa::uInt cycleno, 
               casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
               casa::uInt freqid, casa::Double time, 
               casa::uInt blfunc, casa::uInt order, 
               casa::Vector<casa::Float> boundary, 
               casa::Vector<casa::Float> param);
  void appenddata(casa::uInt scanno, casa::uInt cycleno, 
                  casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
                  casa::uInt freqid, casa::Double time, 
                  casa::uInt blfunc, casa::uInt order, 
                  casa::Vector<casa::Float> boundary, 
                  casa::Vector<casa::Float> param);

  casa::Vector<casa::uInt> getBlfunc() {return blfuncCol_.getColumn();}
  //casa::Vector<STBaselineEnum::BaselineType> getBlfunc() {return blfuncCol_.getColumn();}
  casa::Vector<casa::uInt> getOrder() {return orderCol_.getColumn();}
  casa::Matrix<casa::Float> getBoundary() {return boundaryCol_.getColumn();}
  casa::Matrix<casa::Float> getParam() {return paramCol_.getColumn();}

private:
  static const casa::String name_ ;
  casa::ScalarColumn<casa::uInt> blfuncCol_;
  casa::ScalarColumn<casa::uInt> orderCol_;
  casa::ArrayColumn<casa::Float> boundaryCol_;
  casa::ArrayColumn<casa::Float> paramCol_;
};

}

#endif

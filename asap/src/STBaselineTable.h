//
// C++ Interface: STBaselineTable
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
class STBaselineTable : public STApplyTable {
public:
  STBaselineTable() {;}
  STBaselineTable(const Scantable& parent);
  STBaselineTable(const casa::String &name);

  virtual ~STBaselineTable();

  void setup();
  const casa::String& name() const {return name_;};

  void attachOptionalColumns();
  void save(const std::string &filename);
  void setdata(casa::uInt irow, casa::uInt scanno, casa::uInt cycleno, 
               casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
               casa::uInt freqid, casa::Double time, 
               casa::uInt nchan, 
               STBaselineFunc::FuncName ftype, 
	       casa::Vector<casa::uInt> fpar, 
	       casa::Vector<casa::Float> ffpar, 
               casa::uInt citer, 
	       casa::Float cthres,
               casa::Vector<casa::uInt> mask,
               casa::Vector<casa::Float> res,
               casa::Float rms);
  void appenddata(casa::uInt scanno, casa::uInt cycleno, 
                  casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
                  casa::uInt freqid, casa::Double time, 
		  casa::uInt nchan, 
		  STBaselineFunc::FuncName ftype, 
		  casa::Vector<casa::uInt> fpar, 
		  casa::Vector<casa::Float> ffpar, 
		  casa::uInt citer, 
		  casa::Float cthres,
		  casa::Vector<casa::uInt> mask,
		  casa::Vector<casa::Float> res,
		  casa::Float rms);
  casa::uInt nchan(casa::uInt ifno);
  casa::Vector<casa::uInt> getFunction() {return ftypeCol_.getColumn();}
  casa::Vector<STBaselineFunc::FuncName> getFunctionAsString();
  casa::Matrix<casa::uInt> getFuncParam() {return fparCol_.getColumn();}
  casa::Matrix<casa::Float> getFuncFParam() {return ffparCol_.getColumn();}
  casa::Vector<casa::uInt> getClipIteration() {return citerCol_.getColumn();}
  casa::Vector<casa::Float> getClipThreshold() {return cthresCol_.getColumn();}
  casa::Matrix<casa::uInt> getMaskList() {return maskCol_.getColumn();}
  casa::Matrix<casa::Float> getResult() {return resCol_.getColumn();}
  casa::Vector<casa::Float> getRms() {return rmsCol_.getColumn();}

private:
  static const casa::String name_ ;
  casa::ScalarColumn<casa::uInt> nchanCol_;
  casa::ScalarColumn<casa::uInt> ftypeCol_;
  casa::ArrayColumn<casa::uInt> fparCol_;
  casa::ArrayColumn<casa::Float> ffparCol_;
  casa::ScalarColumn<casa::uInt> citerCol_;
  casa::ScalarColumn<casa::Float> cthresCol_;
  casa::ArrayColumn<casa::uInt> maskCol_;
  casa::ArrayColumn<casa::Float> resCol_;
  casa::ScalarColumn<casa::Float> rmsCol_;
};

}

#endif

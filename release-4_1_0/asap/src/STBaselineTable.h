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
	       casa::Bool apply,
               STBaselineFunc::FuncName ftype, 
	       casa::Vector<casa::Int> fpar, 
	       casa::Vector<casa::Float> ffpar, 
               casa::Vector<casa::uInt> mask,
               casa::Vector<casa::Float> res,
               casa::Float rms, 
               casa::uInt nchan, 
	       casa::Float cthres,
               casa::uInt citer, 
	       casa::Float lfthres, 
	       casa::uInt lfavg, 
	       casa::Vector<casa::uInt> lfedge);
  void appenddata(int scanno, int cycleno, 
		  int beamno, int ifno, int polno, 
		  int freqid, casa::Double time, 
		  bool apply, 
		  STBaselineFunc::FuncName ftype, 
		  vector<int> fpar, 
		  vector<float> ffpar, 
		  casa::Vector<casa::uInt> mask,
		  vector<float> res,
		  float rms,
		  int nchan, 
		  float cthres,
		  int citer, 
		  float lfthres, 
		  int lfavg, 
		  vector<int> lfedge);
  void appenddata(int scanno, int cycleno, 
		  int beamno, int ifno, int polno, 
		  int freqid, casa::Double time, 
		  bool apply, 
		  STBaselineFunc::FuncName ftype, 
		  int fpar, 
		  vector<float> ffpar, 
		  casa::Vector<casa::uInt> mask,
		  vector<float> res,
		  float rms,
		  int nchan, 
		  float cthres,
		  int citer, 
		  float lfthres, 
		  int lfavg, 
		  vector<int> lfedge);
  void appenddata(casa::uInt scanno, casa::uInt cycleno, 
                  casa::uInt beamno, casa::uInt ifno, casa::uInt polno, 
                  casa::uInt freqid, casa::Double time, 
		  casa::Bool apply,
		  STBaselineFunc::FuncName ftype, 
		  casa::Vector<casa::Int> fpar, 
		  casa::Vector<casa::Float> ffpar, 
		  casa::Vector<casa::uInt> mask,
		  casa::Vector<casa::Float> res,
		  casa::Float rms, 
		  casa::uInt nchan, 
		  casa::Float cthres,
		  casa::uInt citer, 
		  casa::Float lfthres, 
		  casa::uInt lfavg, 
		  casa::Vector<casa::uInt> lfedge);
  void appendbasedata(int scanno, int cycleno, 
		      int beamno, int ifno, int polno, 
		      int freqid, casa::Double time);
  void setresult(casa::uInt irow, 
		 casa::Vector<casa::Float> res, 
		 casa::Float rms);
  casa::uInt nchan(casa::uInt ifno);
  casa::Vector<casa::Bool> getApply() {return applyCol_.getColumn();}
  bool getApply(int irow);
  casa::Vector<casa::uInt> getFunction() {return ftypeCol_.getColumn();}
  casa::Vector<STBaselineFunc::FuncName> getFunctionNames();
  STBaselineFunc::FuncName getFunctionName(int irow);
  casa::Matrix<casa::Int> getFuncParam() {return fparCol_.getColumn();}
  std::vector<int> getFuncParam(int irow);
  casa::Matrix<casa::Float> getFuncFParam() {return ffparCol_.getColumn();}
  casa::Matrix<casa::uInt> getMaskList() {return maskCol_.getColumn();}
  std::vector<bool> getMask(int irow);
  casa::Matrix<casa::Float> getResult() {return resCol_.getColumn();}
  casa::Vector<casa::Float> getRms() {return rmsCol_.getColumn();}
  casa::Vector<casa::uInt> getNChan() {return nchanCol_.getColumn();}
  casa::uInt getNChan(int irow);
  casa::Vector<casa::Float> getClipThreshold() {return cthresCol_.getColumn();}
  casa::Vector<casa::uInt> getClipIteration() {return citerCol_.getColumn();}
  casa::Vector<casa::Float> getLineFinderThreshold() {return lfthresCol_.getColumn();}
  casa::Vector<casa::uInt> getLineFinderChanAvg() {return lfavgCol_.getColumn();}
  casa::Matrix<casa::uInt> getLineFinderEdge() {return lfedgeCol_.getColumn();}
  void setApply(int irow, bool apply);

private:
  static const casa::String name_ ;
  casa::ScalarColumn<casa::Bool> applyCol_;
  casa::ScalarColumn<casa::uInt> ftypeCol_;
  casa::ArrayColumn<casa::Int> fparCol_;
  casa::ArrayColumn<casa::Float> ffparCol_;
  casa::ArrayColumn<casa::uInt> maskCol_;
  casa::ArrayColumn<casa::Float> resCol_;
  casa::ScalarColumn<casa::Float> rmsCol_;
  casa::ScalarColumn<casa::uInt> nchanCol_;
  casa::ScalarColumn<casa::Float> cthresCol_;
  casa::ScalarColumn<casa::uInt> citerCol_;
  casa::ScalarColumn<casa::Float> lfthresCol_;
  casa::ScalarColumn<casa::uInt> lfavgCol_;
  casa::ArrayColumn<casa::uInt> lfedgeCol_;
};

}

#endif

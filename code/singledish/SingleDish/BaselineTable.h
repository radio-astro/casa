//# BaselineTable.cc: this code defines baseline table
//# Copyright (C) 2015
//# National Astronomical Observatory of Japan
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#ifndef SD_BASELINETABLE_H
#define SD_BASELINETABLE_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Logging/LogIO.h>
#include <measures/Measures/MEpoch.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <libsakura/sakura.h>

namespace casa {

class BaselineTable {
public:
  BaselineTable() {;}
  BaselineTable(const MeasurementSet& parent);
  BaselineTable(const String &name);

  virtual ~BaselineTable();

  void setup();
  const String& name() const {return name_;};

  const Table& table() const { return table_; }
  Table table() { return table_; }
  void attach();
  void attachBaseColumns();
  void attachOptionalColumns();

  uInt nrow() {return table_.nrow();}

  uInt getScan(uInt irow) const {return scanCol_.get(irow);}
  uInt getBeam(uInt irow) const {return beamCol_.get(irow);}
  uInt getAntenna(uInt irow) const {return antCol_.get(irow);}
  uint getSpw(uInt irow) const {return static_cast<uint>(ifCol_.get(irow));}
  double getTime(uInt irow) const {return static_cast<double>(timeCol_.get(irow));}

  bool getApply(uInt irow, uInt ipol) const;
  uint getBaselineType(uInt irow, uInt ipol) const;
  int getFPar(uInt irow, uInt ipol) const;

  void setdata(uInt irow, uInt scanno, uInt beamno, 
	       uInt antno, uInt ifno, 
               uInt freqid, Double time, 
	       Array<Bool> apply,
               Array<uInt> ftype, 
	       Array<Int> fpar, 
	       Array<Float> ffpar, 
               Array<uInt> mask,
	       Array<Float> res,
               Array<Float> rms, 
               uInt nchan, 
	       Array<Float> cthres,
               Array<uInt> citer, 
	       Array<Bool> uself,
	       Array<Float> lfthres, 
	       Array<uInt> lfavg, 
	       Array<uInt> lfedge);
  void appenddata(uInt scanno, uInt beamno, 
		  uInt antno, uInt ifno, 
                  uInt freqid, Double time, 
		  Array<Bool> apply,
		  Array<uInt> ftype, 
		  Array<Int> fpar, 
		  Array<Float> ffpar, 
		  Array<uInt> mask,
		  Array<Float> res,
		  Array<Float> rms, 
		  uInt nchan, 
		  Array<Float> cthres,
		  Array<uInt> citer, 
		  Array<Bool> uself,
		  Array<Float> lfthres, 
		  Array<uInt> lfavg, 
		  Array<uInt> lfedge);
  void appendbasedata(int scanno, int beamno, int antno, int ifno, 
		      int freqid, Double time);
  void setresult(uInt irow, 
		 Vector<Float> res, 
		 Array<Float> rms);
  void save(const std::string &filename);
  uInt nchan(uInt ifno);

  Matrix<Bool> getApply() {return applyCol_.getColumn();}
  void setApply(int irow, int ipol, bool apply);
  Matrix<Int> getFuncParam() {return fparCol_.getColumn();}
  Matrix<Int> getFuncParam(uInt irow) {return fparCol_.get(irow);}
  Matrix<Float> getFuncFParam() {return ffparCol_.getColumn();}
  Matrix<Float> getFuncFParam(uInt irow) {return ffparCol_.get(irow);}
  Matrix<uInt> getMaskList() {return maskCol_.getColumn();}
  Matrix<Float> getResult() {return resCol_.getColumn();}
  Matrix<Float> getResult(uInt irow) {return resCol_.get(irow);}
  Matrix<Float> getRms() {return rmsCol_.getColumn();}
  Vector<uInt> getNChan() {return nchanCol_.getColumn();}
  uInt getNChan(int irow);
  Matrix<Float> getClipThreshold() {return cthresCol_.getColumn();}
  Matrix<uInt> getClipIteration() {return citerCol_.getColumn();}
  Matrix<Bool> getUseLineFinder() {return uselfCol_.getColumn();}
  Matrix<Float> getLineFinderThreshold() {return lfthresCol_.getColumn();}
  Matrix<uInt> getLineFinderChanAvg() {return lfavgCol_.getColumn();}
  Matrix<uInt> getLineFinderEdge() {return lfedgeCol_.getColumn();}

  std::vector<bool> getMaskFromMaskList(uInt const nchan, std::vector<int> const& masklist);

private:
  void setbasedata(uInt irow, uInt scanno, uInt beamno, uInt antno, 
		   uInt ifno, uInt freqid, Double time);
  Table table_, originaltable_;
  ScalarColumn<uInt> scanCol_, beamCol_, antCol_, ifCol_, freqidCol_;
  ScalarColumn<Double> timeCol_;
  MEpoch::ScalarColumn timeMeasCol_;
  static const String name_;

  ArrayColumn<Bool> applyCol_;
  ArrayColumn<uInt> ftypeCol_;
  ArrayColumn<Int> fparCol_;
  ArrayColumn<Float> ffparCol_;
  ArrayColumn<uInt> maskCol_;
  ArrayColumn<Float> resCol_;
  ArrayColumn<Float> rmsCol_;
  ScalarColumn<uInt> nchanCol_;
  ArrayColumn<Float> cthresCol_;
  ArrayColumn<uInt> citerCol_;
  ArrayColumn<Bool> uselfCol_;
  ArrayColumn<Float> lfthresCol_;
  ArrayColumn<uInt> lfavgCol_;
  ArrayColumn<uInt> lfedgeCol_;
};

}

#endif

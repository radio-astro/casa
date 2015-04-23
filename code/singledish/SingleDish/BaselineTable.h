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

  Vector<uInt> getScan() const {return scanCol_.getColumn();}
  Vector<uInt> getBeam() const {return beamCol_.getColumn();}
  Vector<uInt> getIF() const {return ifCol_.getColumn();}
  Vector<Double> getTime() const {return timeCol_.getColumn();}

  void save(const std::string &filename);
  void setdata(uInt irow, uInt scanno, 
               uInt beamno, uInt ifno, 
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
  void appenddata(uInt scanno, 
                  uInt beamno, uInt ifno, 
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
  void appendbasedata(int scanno, 
		      int beamno, int ifno, 
		      int freqid, Double time);
  void setresult(uInt irow, 
		 Vector<Float> res, 
		 Array<Float> rms);
  uInt nchan(uInt ifno);

  Matrix<Bool> getApply() {return applyCol_.getColumn();}
  ////bool getApply(int irow, int ipol);
  void setApply(int irow, int ipol, bool apply);
  ////Matrix<uInt> getFunction() {return ftypeCol_.getColumn();}
  ////Matrix<BaselineType> getFunctionNames();
  ////BaselineType getFunctionName(int irow, int ipol);
  Matrix<Int> getFuncParam() {return fparCol_.getColumn();}
  ////std::vector<int> getFuncParam(int irow, int ipol);
  Matrix<Float> getFuncFParam() {return ffparCol_.getColumn();}
  Matrix<uInt> getMaskList() {return maskCol_.getColumn();}
  ////std::vector<bool> getMask(int irow, int ipol);
  Matrix<Float> getResult() {return resCol_.getColumn();}
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
  void setbasedata(uInt irow, uInt scanno, 
                   uInt beamno, uInt ifno, 
                   uInt freqid, Double time);
  Table table_, originaltable_;
  ScalarColumn<uInt> scanCol_, beamCol_, ifCol_, freqidCol_;
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

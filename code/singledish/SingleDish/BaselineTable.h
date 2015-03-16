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

namespace casa {

typedef enum {
  BaselineType_Polynomial,
  BaselineType_Chebyshev,
  BaselineType_CubicSpline,
  BaselineType_Sinusoid,
  BaselineType_NumElements
} BaselineType;

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
  Vector<uInt> getCycle() const {return cycleCol_.getColumn();}
  Vector<uInt> getBeam() const {return beamCol_.getColumn();}
  Vector<uInt> getIF() const {return ifCol_.getColumn();}
  Vector<uInt> getPol() const {return polCol_.getColumn();}
  Vector<Double> getTime() const {return timeCol_.getColumn();}

  void save(const std::string &filename);
  void setdata(uInt irow, uInt scanno, uInt cycleno, 
               uInt beamno, uInt ifno, uInt polno, 
               uInt freqid, Double time, 
	       Bool apply,
               BaselineType ftype, 
	       Vector<Int> fpar, 
	       Vector<Float> ffpar, 
               Vector<uInt> mask,
               Vector<Float> res,
               Float rms, 
               uInt nchan, 
	       Float cthres,
               uInt citer, 
	       Float lfthres, 
	       uInt lfavg, 
	       Vector<uInt> lfedge);
  void appenddata(int scanno, int cycleno, 
		  int beamno, int ifno, int polno, 
		  int freqid, Double time, 
		  bool apply, 
		  BaselineType ftype, 
		  vector<int> fpar, 
		  vector<float> ffpar, 
		  Vector<uInt> mask,
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
		  int freqid, Double time, 
		  bool apply, 
		  BaselineType ftype, 
		  int fpar, 
		  vector<float> ffpar, 
		  Vector<uInt> mask,
		  vector<float> res,
		  float rms,
		  int nchan, 
		  float cthres,
		  int citer, 
		  float lfthres, 
		  int lfavg, 
		  vector<int> lfedge);
  void appenddata(uInt scanno, uInt cycleno, 
                  uInt beamno, uInt ifno, uInt polno, 
                  uInt freqid, Double time, 
		  Bool apply,
		  BaselineType ftype, 
		  Vector<Int> fpar, 
		  Vector<Float> ffpar, 
		  Vector<uInt> mask,
		  Vector<Float> res,
		  Float rms, 
		  uInt nchan, 
		  Float cthres,
		  uInt citer, 
		  Float lfthres, 
		  uInt lfavg, 
		  Vector<uInt> lfedge);
  void appendbasedata(int scanno, int cycleno, 
		      int beamno, int ifno, int polno, 
		      int freqid, Double time);
  void setresult(uInt irow, 
		 Vector<Float> res, 
		 Float rms);
  uInt nchan(uInt ifno);
  Vector<Bool> getApply() {return applyCol_.getColumn();}
  bool getApply(int irow);
  Vector<uInt> getFunction() {return ftypeCol_.getColumn();}
  Vector<BaselineType> getFunctionNames();
  BaselineType getFunctionName(int irow);
  Matrix<Int> getFuncParam() {return fparCol_.getColumn();}
  std::vector<int> getFuncParam(int irow);
  Matrix<Float> getFuncFParam() {return ffparCol_.getColumn();}
  Matrix<uInt> getMaskList() {return maskCol_.getColumn();}
  std::vector<bool> getMask(int irow);
  Matrix<Float> getResult() {return resCol_.getColumn();}
  Vector<Float> getRms() {return rmsCol_.getColumn();}
  Vector<uInt> getNChan() {return nchanCol_.getColumn();}
  uInt getNChan(int irow);
  Vector<Float> getClipThreshold() {return cthresCol_.getColumn();}
  Vector<uInt> getClipIteration() {return citerCol_.getColumn();}
  Vector<Float> getLineFinderThreshold() {return lfthresCol_.getColumn();}
  Vector<uInt> getLineFinderChanAvg() {return lfavgCol_.getColumn();}
  Matrix<uInt> getLineFinderEdge() {return lfedgeCol_.getColumn();}
  void setApply(int irow, bool apply);
  std::vector<bool> getMaskFromMaskList(uInt const nchan, std::vector<int> const& masklist);

private:
  void setbasedata(uInt irow, uInt scanno, uInt cycleno,
                   uInt beamno, uInt ifno, uInt polno, 
                   uInt freqid, Double time);
  Table table_, originaltable_;
  ScalarColumn<uInt> scanCol_, cycleCol_, beamCol_, ifCol_, polCol_, freqidCol_;
  ScalarColumn<Double> timeCol_;
  MEpoch::ScalarColumn timeMeasCol_;

  static const String name_ ;
  ScalarColumn<Bool> applyCol_;
  ScalarColumn<uInt> ftypeCol_;
  ArrayColumn<Int> fparCol_;
  ArrayColumn<Float> ffparCol_;
  ArrayColumn<uInt> maskCol_;
  ArrayColumn<Float> resCol_;
  ScalarColumn<Float> rmsCol_;
  ScalarColumn<uInt> nchanCol_;
  ScalarColumn<Float> cthresCol_;
  ScalarColumn<uInt> citerCol_;
  ScalarColumn<Float> lfthresCol_;
  ScalarColumn<uInt> lfavgCol_;
  ArrayColumn<uInt> lfedgeCol_;
};

}

#endif

/*
 * DataAccumulator.h
 *
 *  Created on: Jan 18, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_DATAACCUMULATOR_H_
#define SINGLEDISH_FILLER_DATAACCUMULATOR_H_

#include <singledish/Filler/DataRecord.h>
#include <singledish/Filler/FillerUtil.h>

#include <vector>
#include <cassert>
#include <memory>
#include <algorithm>

#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/casa/Containers/Record.h>

#include <casacore/measures/Measures/Stokes.h>

#include <casacore/tables/Tables/TableRecord.h>

using namespace casacore;

namespace {
template<class T>
inline void resizeTo(T &array, IPosition const &shape) {
  if (array.shape() != shape) {
    array.resize(shape, False);
  }
}

template<class T>
inline void setValue1(ssize_t n, T const *src, T *dst) {
  for (ssize_t i = 0; i < n; ++i) {
    dst[i] = src[i];
  }
}

template<class T>
inline void setValueToMatrixColumn(Vector<T> const &src, ssize_t icolumn,
    Matrix<T> &dst) {
  IPosition const &shape = dst.shape();
  ssize_t const nrow = shape[0];
  ssize_t const ncolumn = shape[1];
  if (icolumn >= ncolumn) {
    throw AipsError("Specified column doesn't exist.");
  }

  Bool b1, b2;
  T *dst_p = dst.getStorage(b1);
  T *work_p = dst_p + icolumn * nrow;
  T const *src_p = src.getStorage(b2);

  setValue1(nrow, src_p, work_p);

  src.freeStorage(src_p, b2);
  dst.putStorage(dst_p, b1);
}

template<class T, class Executor>
inline void transposeMatrix(ssize_t n, ssize_t offset_src, Matrix<T> const &src,
    Matrix<T> &dst) {
  Bool b1, b2;
  T const *src_p = src.getStorage(b1);
  T *dst_p = dst.getStorage(b2);
  T const *wsrc_p = src_p + offset_src * n;
  T *wdst_p = dst_p;

  Executor::execute(n, wsrc_p, wdst_p);

  src.freeStorage(src_p, b1);
  dst.putStorage(dst_p, b2);
}

struct ExecuteMatrix1 {
  template<class T>
  static void execute(ssize_t n, T const *src, T *dst) {
    setValue1(n, src, dst);
  }
};

struct ExecuteMatrix2 {
  template<class T>
  static void execute(ssize_t n, T const *src, T *dst) {
    T const *row0_p = src;
    T const *row1_p = src + n;
    for (ssize_t i = 0; i < n; ++i) {
      dst[2 * i] = row0_p[i];
      dst[2 * i + 1] = row1_p[i];
    }
  }
};

struct ExecuteMatrix4X {
  template<class T>
  static void execute(ssize_t n, T const *src, T *dst) {
    throw std::runtime_error("");
  }
};

template<>
inline void ExecuteMatrix4X::execute(ssize_t n, Bool const *src, Bool *dst) {
  Bool const *row0_p = src + 0 * n;
  Bool const *row1_p = src + 1 * n;
  Bool const *row2_p = src + 2 * n;
  Bool const *row3_p = src + 3 * n;
  for (ssize_t i = 0; i < n; ++i) {
    dst[4 * i + 0] = row0_p[i];
    Bool b = row2_p[i] || row3_p[i];
    dst[4 * i + 1] = b;
    dst[4 * i + 2] = b;
    dst[4 * i + 3] = row1_p[i];
  }
}

struct ExecuteMatrix4 {
  template<class T>
  static void execute(ssize_t n, T const *src, T *dst) {
    T const *row0_p = src + 0 * n;
    T const *row1_p = src + 1 * n;
    T const *row2_p = src + 2 * n;
    T const *row3_p = src + 3 * n;
    for (ssize_t i = 0; i < n; ++i) {
      dst[4 * i + 0] = row0_p[i];
      dst[4 * i + 1] = row1_p[i];
      dst[4 * i + 2] = row2_p[i];
      dst[4 * i + 3] = row3_p[i];
    }
  }
};

inline void transposeMatrix4F2C(ssize_t n, Matrix<Float> const &src,
    Matrix<Complex> &dst) {
  Bool b1, b2;
  Float const *src_p = src.getStorage(b1);
  Complex *dst_p = dst.getStorage(b2);

  Float const *row0_p = src_p + 0 * n;
  Float const *row1_p = src_p + 1 * n;
  Float const *row2_p = src_p + 2 * n;
  Float const *row3_p = src_p + 3 * n;
  for (ssize_t i = 0; i < n; ++i) {
    dst_p[4 * i].real(row0_p[i]);
    dst_p[4 * i].imag(0.0f);
    Float fr = row2_p[i];
    Float fi = row3_p[i];
    dst_p[4 * i + 1].real(fr);
    dst_p[4 * i + 1].imag(fi);
    dst_p[4 * i + 2].real(fr);
    dst_p[4 * i + 2].imag(-fi);
    dst_p[4 * i + 3].real(row1_p[i]);
    dst_p[4 * i + 3].imag(0.0f);
  }

  src.freeStorage(src_p, b1);
  dst.putStorage(dst_p, b2);
}
}

namespace casa { //# NAMESPACE CASA - BEGIN
namespace sdfiller { //# NAMESPACE SDFILLER - BEGIN

class DataAccumulator;

class DataChunk {
public:
  friend DataAccumulator;

  DataChunk(String const &poltype) :
      num_pol_max_(4), num_chan_(0), data_(), flag_(),
      flag_row_(num_pol_max_, False), tsys_(), tcal_(),
      weight_(num_pol_max_, 1.0f), sigma_(weight_), poltype_(poltype),
      corr_type_(), filled_(NoData()), get_chunk_(nullptr),
      get_num_pol_(nullptr) {
    POST_START;

    setPolType(poltype);

    POST_END;
  }

  virtual ~DataChunk() {
  }

  String getPolType() const {
    return poltype_;
  }

  void resetPolType(String const &poltype) {
    initialize(num_chan_);
    setPolType(poltype);
  }

  uInt getNumPol() const {
    return (*this.*get_num_pol_)();
  }

  void initialize(size_t num_chan) {
    num_chan_ = num_chan;
    IPosition const shape(2, num_chan_, num_pol_max_);
    ::resizeTo(data_, shape);
    ::resizeTo(flag_, shape);
    ::resizeTo(tsys_, shape);
    ::resizeTo(tcal_, shape);
    tsys_ = -1.0f;
    tcal_ = -1.0f;
    filled_ = NoData();
  }

  void clear() {
    num_chan_ = 0;
    filled_ = NoData();
  }

  bool readyToWrite() {
    return true;
  }

  bool accumulate(DataRecord const &record) {
    POST_START;

    if (!isValidRecord(record)) {
      return false;
    }

    uInt polid = record.polno;

    if (num_pol_max_ <= polid) {
      return false;
    }
    Vector<Float> const &data = record.data;
    if (num_chan_ == 0) {
      size_t num_chan = data.size();
      initialize(num_chan);
    }
    Vector<Bool> const &flag = record.flag;
    Bool flagrow = record.flag_row;

    if (data.shape() != flag.shape()) {
      return false;
    }

    Vector<Float> tsys;
    if (!record.tsys.empty()) {
//      std::cout << "tsys is not empty: " << record.tsys << std::endl;
      tsys.assign(record.tsys);
    }
    Vector<Float> tcal;
    if (!record.tcal.empty()) {
//      std::cout << "tcal is not empty: " << record.tcal << std::endl;
      tcal.assign(record.tcal);
    }

    if (data.nelements() != num_chan_) {
      return false;
    }

    //data_.column(polid) = data;
    ::setValueToMatrixColumn(data, polid, data_);
    //flag_.column(polid) = flag;
    ::setValueToMatrixColumn(flag, polid, flag_);
    flag_row_[polid] = flagrow;
    if (tsys.size() == num_chan_) {
      //tsys_.column(polid) = tsys;
      ::setValueToMatrixColumn(tsys, polid, tsys_);
    } else if (!tsys.empty()) {
      tsys_(0, polid) = tsys[0];
    }
    if (tcal.size() == num_chan_) {
      //tcal_.column(polid) = tcal;
      ::setValueToMatrixColumn(tcal, polid, tcal_);
    } else if (!tcal.empty()) {
      tcal_(0, polid) = tcal[0];
    }
    filled_ |= 0x01 << polid;

    return true;
  }

  bool get(MSDataRecord &record) {
    bool return_value = (*this.*get_chunk_)(record);
    return return_value;
  }

private:
  static constexpr unsigned char NoData() {
    return 0x00;
  }
  static constexpr unsigned char SinglePol0() {
    return 0x01;
  }
  static constexpr unsigned char SinglePol1() {
    return 0x01 << 1;
  }
  static constexpr unsigned char SinglePol2() {
    return 0x01 << 2;
  }
  static constexpr unsigned char SinglePol3() {
    return 0x01 << 3;
  }
  static constexpr unsigned char DualPol() {
    return SinglePol0() | SinglePol1();
  }
  static constexpr unsigned char FullPol() {
    return SinglePol0() | SinglePol1() | SinglePol2() | SinglePol3();
  }
  bool isFullPol() const {
    return judgePol(FullPol());
  }
  bool isDualPol() const {
    return judgePol(DualPol());
  }
  bool isSinglePol0() const {
    return judgePol(SinglePol0());
  }
  bool isSinglePol1() const {
    return judgePol(SinglePol1());
  }
  bool judgePol(unsigned char const pol) const {
    return (filled_ & pol) == pol;
  }
  bool isValidRecord(DataRecord const &record) {
    return !record.data.empty() && !record.flag.empty();
  }
  void setPolType(String const &poltype) {
    POST_START;

    poltype_ = poltype;
    if (poltype_ == "linear") {
      get_chunk_ = &DataChunk::getLinear;
      get_num_pol_ = &DataChunk::getNumPolLinear;
      corr_type_.resize(4);
      corr_type_[0] = Stokes::XX;
      corr_type_[1] = Stokes::XY;
      corr_type_[2] = Stokes::YX;
      corr_type_[3] = Stokes::YY;
    } else if (poltype_ == "circular") {
      get_chunk_ = &DataChunk::getCircular;
      get_num_pol_ = &DataChunk::getNumPolCircular;
      corr_type_.resize(4);
      corr_type_[0] = Stokes::RR;
      corr_type_[1] = Stokes::RL;
      corr_type_[2] = Stokes::LR;
      corr_type_[3] = Stokes::LL;
    } else if (poltype_ == "stokes") {
      get_chunk_ = &DataChunk::getStokes;
      get_num_pol_ = &DataChunk::getNumPolStokes;
      corr_type_.resize(4);
      corr_type_[0] = Stokes::I;
      corr_type_[1] = Stokes::Q;
      corr_type_[2] = Stokes::U;
      corr_type_[3] = Stokes::V;
    } else if (poltype_ == "linpol") {
      get_chunk_ = &DataChunk::getLinpol;
      get_num_pol_ = &DataChunk::getNumPolLinpol;
      corr_type_.resize(2);
      corr_type_[0] = Stokes::Plinear;
      corr_type_[1] = Stokes::Pangle;
    } else {
      throw AipsError(String("Invalid poltype") + poltype);
    }

    POST_END;
  }
  size_t const num_pol_max_;
  size_t num_chan_;
  Matrix<Float> data_;
  Matrix<Bool> flag_;
  Vector<Bool> flag_row_;
  Matrix<Float> tsys_;
  Matrix<Float> tcal_;
  Vector<Float> weight_;
  Vector<Float> sigma_;
  String poltype_;
  Vector<Int> corr_type_;
  unsigned char filled_;
  bool (DataChunk::*get_chunk_)(MSDataRecord &record);
  uInt (DataChunk::*get_num_pol_)() const;

  void setTsys2(MSDataRecord &record) {
    if (num_chan_ == 1) {
      record.setTsysSize(2, 1);
      record.tsys(0, 0) = tsys_(0, 0);
      record.tsys(1, 0) = tsys_(0, 1);
    } else {
      Float tsys00 = tsys_(0, 0);
      Float tsys01 = tsys_(0, 1);
      Float tsys10 = tsys_(1, 0);
      Float tsys11 = tsys_(1, 1);
      if ((tsys00 > 0.0f && tsys10 > 0.0f)
          || (tsys01 > 0.0f && tsys11 > 0.0f)) {
        record.setTsysSize(2, num_chan_);
        transposeMatrix<Float, ExecuteMatrix2>(num_chan_, 0, tsys_,
            record.tsys);
      } else if (tsys00 > 0.0f || tsys01 > 0.0f) {
        record.setTsysSize(2, 1);
        record.tsys(0, 0) = tsys_(0, 0);
        record.tsys(1, 0) = tsys_(0, 1);
      }
    }
  }

  void setTcal2(MSDataRecord &record) {
    if (num_chan_ == 1) {
      record.setTcalSize(2, 1);
      record.tcal(0, 0) = tcal_(0, 0);
      record.tcal(1, 0) = tcal_(0, 1);
    } else {
      Float tcal00 = tcal_(0, 0);
      Float tcal01 = tcal_(0, 1);
      Float tcal10 = tcal_(1, 0);
      Float tcal11 = tcal_(1, 1);
      if ((tcal00 > 0.0f && tcal10 > 0.0f)
          || (tcal01 > 0.0f && tcal11 > 0.0f)) {
        record.setTcalSize(2, num_chan_);
        transposeMatrix<Float, ExecuteMatrix2>(num_chan_, 0, tcal_,
            record.tcal);
      } else if (tcal00 > 0.0f || tcal01 > 0.0f) {
        record.setTcalSize(2, 1);
        record.tcal(0, 0) = tcal_(0, 0);
        record.tcal(1, 0) = tcal_(0, 1);
      }
    }
  }

  void setTsys1(ssize_t start_src, MSDataRecord &record) {
    if (num_chan_ == 1) {
      record.setTsysSize(1, 1);
      record.tsys(0, 0) = tsys_(0, start_src);
    } else if (tsys_(0, start_src) > 0.0f && tsys_(1, start_src) > 0.0f) {
      // should be spectral Tsys
      record.setTsysSize(1, num_chan_);
      //record.tsys = -1;
      transposeMatrix<Float, ExecuteMatrix1>(num_chan_, start_src, tsys_,
          record.tsys);
      //record.tsys.row(0) = tsys_.column(0);
    } else if (tsys_(0, start_src) > 0.0f) {
      // scalar Tsys
      record.setTsysSize(1, 1);
      record.tsys(0, 0) = tsys_(0, start_src);
    }
  }

  void setTcal1(ssize_t start_src, MSDataRecord &record) {
    if (num_chan_ == 1) {
      record.setTcalSize(1, 1);
      record.tcal(0, 0) = tcal_(0, start_src);
    } else if (tcal_(0, start_src) > 0.0f && tcal_(1, start_src) > 0.0f) {
      // should be spectral Tsys
      record.setTcalSize(1, num_chan_);
      //record.tsys = -1;
      transposeMatrix<Float, ExecuteMatrix1>(num_chan_, start_src, tcal_,
          record.tcal);
      //record.tsys.row(0) = tsys_.column(0);
    } else if (tcal_(0, start_src) > 0.0f) {
      // scalar Tsys
      record.setTcalSize(1, 1);
      record.tcal(0, 0) = tcal_(0, start_src);
    }
  }

  bool getLinear(MSDataRecord &record) {
    POST_START;

    Vector<Float> weight;
    Vector<Float> sigma;
    if (isFullPol()) {
      // POL 0, 1, 2, and 3
//      std::cout << "set data/flag" << std::endl;
      record.setComplex();
      record.setDataSize(4, num_chan_);
      transposeMatrix4F2C(num_chan_, data_, record.complex_data);
      transposeMatrix<Bool, ExecuteMatrix4X>(num_chan_, 0, flag_, record.flag);
      record.flag_row = anyEQ(flag_row_, True);
//      std::cout << "weight = " << record.weight << std::endl;

//      std::cout << "set tsys" << std::endl;
      setTsys2(record);

//      std::cout << "set tcal " << tcal_ << std::endl;
      setTcal2(record);

      record.num_pol = 4;
      record.corr_type = corr_type_;
    } else if (isDualPol()) {
      // POL 0 and 1
//      std::cout << "set data/flag" << std::endl;
      record.setFloat();
      record.setDataSize(2, num_chan_);
      transposeMatrix<Float, ExecuteMatrix2>(num_chan_, 0, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix2>(num_chan_, 0, flag_, record.flag);
      record.flag_row = flag_row_[0] || flag_row_[1];
//      std::cout << "weight = " << record.weight << std::endl;

//      std::cout << "set tsys" << std::endl;
      setTsys2(record);

//      std::cout << "set tcal " << tcal_ << std::endl;
      setTcal2(record);

      record.num_pol = 2;
      record.corr_type[0] = corr_type_[0];
      record.corr_type[1] = corr_type_[3];
    } else if (isSinglePol0()) {
      // only POL 0
//      std::cout << "set data/flag (pol 0)" << std::endl;
      record.setFloat();
      record.setDataSize(1, num_chan_);
      transposeMatrix<Float, ExecuteMatrix1>(num_chan_, 0, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix1>(num_chan_, 0, flag_, record.flag);
      record.flag_row = flag_row_(0);

      setTsys1(0, record);

//      std::cout << "set tcal " << tcal_ << std::endl;
      setTcal1(0, record);

      record.num_pol = 1;
      record.corr_type[0] = corr_type_[0];
    } else if (isSinglePol1()) {
      // only POL 1
//      std::cout << "set data/flag (pol 1)" << std::endl;
      record.setFloat();
      record.setDataSize(1, num_chan_);
      transposeMatrix<Float, ExecuteMatrix1>(num_chan_, 1, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix1>(num_chan_, 1, flag_, record.flag);
      record.flag_row = flag_row_(1);

      setTsys1(1, record);

//      std::cout << "set tcal " << tcal_ << std::endl;
      setTcal1(1, record);

      record.num_pol = 1;
      record.corr_type[0] = corr_type_[3];
    } else {
//      std::cout << "DataChunk is not ready for get" << std::endl;
      return false;
    }

    POST_END;
    return true;
  }

  bool getCircular(MSDataRecord &record) {
    return getLinear(record);
  }

  bool getStokes(MSDataRecord &record) {
    POST_START;

    record.setFloat();
    if (isFullPol()) {
      record.setDataSize(4, num_chan_);
      transposeMatrix<Float, ExecuteMatrix4>(num_chan_, 0, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix4>(num_chan_, 0, flag_, record.flag);
      record.flag_row = anyTrue(flag_row_);

      record.num_pol = 4;
      record.corr_type = corr_type_;
    } else if (isSinglePol0()) {
      record.setDataSize(1, num_chan_);
      transposeMatrix<Float, ExecuteMatrix1>(num_chan_, 0, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix1>(num_chan_, 0, flag_, record.flag);
      record.flag_row = flag_row_[0];

      record.num_pol = 1;
      record.corr_type[0] = corr_type_[0];
    } else {
      return false;
    }

    POST_END;
    return true;
  }

  bool getLinpol(MSDataRecord &record) {
    POST_START;

    record.setFloat();
    if (isDualPol()) {
      // POL 0 and 1
      record.setDataSize(2, num_chan_);
      transposeMatrix<Float, ExecuteMatrix2>(num_chan_, 0, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix2>(num_chan_, 0, flag_, record.flag);
      record.flag_row = flag_row_[0] || flag_row_[1];

      record.num_pol = 2;
      record.corr_type = corr_type_;
    } else if (isSinglePol0()) {
      record.setDataSize(1, num_chan_);
      transposeMatrix<Float, ExecuteMatrix1>(num_chan_, 0, data_,
          record.float_data);
      transposeMatrix<Bool, ExecuteMatrix1>(num_chan_, 0, flag_, record.flag);
      record.flag_row = flag_row_[0];

      record.num_pol = 1;
      record.corr_type[0] = corr_type_[0];
    } else {
      return false;
    }

    POST_END;
    return true;
  }

  uInt getNumPolLinear() const {
    if (isFullPol()) {
      return 4;
    } else if (isDualPol()) {
      return 2;
    } else if (isSinglePol0() || isSinglePol1()) {
      return 1;
    } else {
      return 0;
    }
  }

  uInt getNumPolCircular() const {
    return getNumPolLinear();
  }

  uInt getNumPolStokes() const {
    if (isFullPol()) {
      return 4;
    } else if (isSinglePol0()) {
      return 1;
    } else {
      return 0;
    }
  }

  uInt getNumPolLinpol() const {
    if (isDualPol()) {
      return 2;
    } else if (isSinglePol0()) {
      return 1;
    } else {
      return 0;
    }
  }
};

class DataAccumulator {
private:
  struct DataAccumulatorKey {
    Int antenna_id;
    Int field_id;
    Int feed_id;
    Int spw_id;
    String pol_type;
    String intent;

    template<class T, class C>
    bool comp(T const &a, T const &b, C const &c) const {
      if (a < b) {
        return true;
      } else if (a == b) {
        return c();
      } else {
        return false;
      }
    }

    bool operator()(DataAccumulatorKey const &lhs,
        DataAccumulatorKey const &rhs) const {
      return comp(lhs.antenna_id, rhs.antenna_id,
          [&]() {return comp(lhs.field_id, rhs.field_id,
                [&]() {return comp(lhs.feed_id, rhs.feed_id,
                      [&]() {return comp(lhs.spw_id, rhs.spw_id,
                            [&]() {return comp(lhs.pol_type, rhs.pol_type,
                                  [&]() {return comp(lhs.intent, rhs.intent,
                                        []() {return false;});});});});});});
    }
  };

public:
  DataAccumulator() :
      pool_(), antenna_id_(), spw_id_(), field_id_(), feed_id_(), scan_(),
      subscan_(), intent_(), direction_(), interval_(), indexer_(), time_(-1.0),
      is_free_() {
  }

  virtual ~DataAccumulator() {
    POST_START;

    for (auto iter = pool_.begin(); iter != pool_.end(); ++iter) {
      delete *iter;
    }

    POST_END;
  }

  size_t getNumberOfChunks() const {
    return pool_.size();
  }

  size_t getNumberOfActiveChunks() const {
    return std::count_if(pool_.begin(), pool_.end(), [](DataChunk * const &c) {
      return c->getNumPol() > 0;
    });
  }

  bool queryForGet(DataRecord const &record) const {
    Double const time = record.time;
    bool is_ready = (0.0 <= time_) && !(time_ == time);
    return is_ready;
  }

  bool queryForGet(Double const &time) const {
    bool is_ready = (0.0 <= time_) && !(time_ == time);
    return is_ready;
  }

  void clear() {
    for (auto iter = pool_.begin(); iter != pool_.end(); ++iter) {
      (*iter)->clear();
    }
    time_ = -1.0;
  }

  bool get(size_t ichunk, MSDataRecord &record) {
    POST_START;

    if (pool_.size() == 0) {
      return false;
    } else if (ichunk >= pool_.size()) {
      return false;
    }
    bool status = pool_[ichunk]->get(record);
//    std::cout << "get Chunk status = " << status << std::endl;
    if (!status) {
      record.clear();
      return status;
    }
    record.time = time_;
    record.pol_type = pool_[ichunk]->getPolType();
    record.antenna_id = antenna_id_[ichunk];
    record.spw_id = spw_id_[ichunk];
    record.field_id = field_id_[ichunk];
    record.feed_id = feed_id_[ichunk];
    record.scan = scan_[ichunk];
    record.subscan = subscan_[ichunk];
    record.intent = intent_[ichunk];
    record.direction = direction_[ichunk];
    record.interval = interval_[ichunk];
    record.temperature = temperature_[ichunk];
    record.pressure = pressure_[ichunk];
    record.rel_humidity = rel_humidity_[ichunk];
    record.wind_speed = wind_speed_[ichunk];
    record.wind_direction = wind_direction_[ichunk];

    POST_END;
    return status;
  }

  bool accumulate(DataRecord const &record) {
    POST_START;

    if (!isValidRecord(record)) {
//      std::cout << "record is not a valid one" << std::endl;
      return false;
    }

    Double time = record.time;
    if (time_ < 0.0) {
      time_ = time;
    }
    if (time_ != time) {
//      std::cout << "timestamp mismatch" << std::endl;
      return false;
    }
    Int antennaid = record.antenna_id;
    Int spwid = record.spw_id;
    Int fieldid = record.field_id;
    Int feedid = record.feed_id;
    Int scan = record.scan;
    Int subscan = record.subscan;
    String intent = record.intent;
    String poltype = record.pol_type;
    DataAccumulatorKey key;
    key.antenna_id = record.antenna_id;
    key.field_id = record.field_id;
    key.feed_id = record.feed_id;
    key.spw_id = record.spw_id;
    key.intent = record.intent;
    key.pol_type = record.pol_type;
    Matrix<Double> const &direction = record.direction;
    Double interval = record.interval;
    Float temperature = record.temperature;
    Float pressure = record.pressure;
    Float rel_humidity = record.rel_humidity;
    Float wind_speed = record.wind_speed;
    Float wind_direction = record.wind_direction;
    bool status = false;
    auto iter = indexer_.find(key);
    if (iter != indexer_.end()) {
      uInt index = iter->second;
      status = pool_[index]->accumulate(record);
      if (status) {
        antenna_id_[index] = antennaid;
        spw_id_[index] = spwid;
        field_id_[index] = fieldid;
        feed_id_[index] = feedid;
        scan_[index] = scan;
        subscan_[index] = subscan;
        intent_[index] = intent;
        direction_[index].assign(direction);
        interval_[index] = interval;
        temperature_[index] = temperature;
        pressure_[index] = pressure;
        rel_humidity_[index] = rel_humidity;
        wind_speed_[index] = wind_speed;
        wind_direction_[index] = wind_direction;
      }
    } else {
      pool_.push_back(new DataChunk(poltype));
      antenna_id_.push_back(-1);
      spw_id_.push_back(-1);
      field_id_.push_back(-1);
      feed_id_.push_back(-1);
      scan_.push_back(-1);
      subscan_.push_back(-1);
      intent_.push_back("");
      direction_.push_back(Matrix<Double>());
      interval_.push_back(-1.0);
      temperature_.push_back(0.0f);
      pressure_.push_back(0.0f);
      rel_humidity_.push_back(0.0f);
      wind_speed_.push_back(0.0f);
      wind_direction_.push_back(0.0f);
      uInt index = pool_.size() - 1;
      indexer_[key] = index;
      status = pool_[index]->accumulate(record);
      if (status) {
        antenna_id_[index] = antennaid;
        spw_id_[index] = spwid;
        field_id_[index] = fieldid;
        feed_id_[index] = feedid;
        scan_[index] = scan;
        subscan_[index] = subscan;
        intent_[index] = intent;
        direction_[index].assign(direction);
        interval_[index] = interval;
        temperature_[index] = temperature;
        pressure_[index] = pressure;
        rel_humidity_[index] = rel_humidity;
        wind_speed_[index] = wind_speed;
        wind_direction_[index] = wind_direction;
      }
    }

//    std::cout << "status = " << status << std::endl;
//    std::cout << "key " << key << "(index " << indexer_.asuInt(key)
//        << "): TIME=" << time_ << " INTERVAL=" << interval << std::endl;
    POST_END;
    return status;
  }

  String getPolType(size_t ichunk) const {
    assert(ichunk < pool_.size());
    return pool_[ichunk]->getPolType();
  }

  uInt getNumPol(size_t ichunk) const {
    assert(ichunk < pool_.size());
    return pool_[ichunk]->getNumPol();
  }

private:
  bool isValidRecord(DataRecord const &record) {
//    std::cout << record.time << " " << record.interval << " "
//        << record.antenna_id << " " << record.field_id << " " << record.feed_id
//        << " " << record.spw_id << " " << record.scan << " " << record.subscan
//        << " " << record.direction << std::endl;
    return record.time > 0.0 && record.interval > 0.0 && record.antenna_id >= 0
        && record.field_id >= 0 && record.feed_id >= 0 && record.spw_id >= 0
        && record.scan >= 0 && record.subscan >= 0 && !record.direction.empty();
  }

  std::vector<DataChunk *> pool_;
  std::vector<Int> antenna_id_;
  std::vector<Int> spw_id_;
  std::vector<Int> field_id_;
  std::vector<Int> feed_id_;
  std::vector<Int> scan_;
  std::vector<Int> subscan_;
  std::vector<String> intent_;
  std::vector<Matrix<Double> > direction_;
  std::vector<Double> interval_;
  std::vector<Float> temperature_;
  std::vector<Float> pressure_;
  std::vector<Float> rel_humidity_;
  std::vector<Float> wind_speed_;
  std::vector<Float> wind_direction_;
  std::map<DataAccumulatorKey, uInt, DataAccumulatorKey> indexer_;
  Double time_;
  std::vector<bool> is_free_;
};

} //# NAMESPACE SDFILLER - END
} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_DATAACCUMULATOR_H_ */

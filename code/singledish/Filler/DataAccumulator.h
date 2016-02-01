/*
 * DataAccumulator.h
 *
 *  Created on: Jan 18, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_DATAACCUMULATOR_H_
#define SINGLEDISH_FILLER_DATAACCUMULATOR_H_

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

//#define SINGLEDISHMSFILLER_DEBUG
#ifdef SINGLEDISHMSFILLER_DEBUG
#define POST_START std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl
#define POST_END std::cout << "End " << __PRETTY_FUNCTION__ << std::endl
#else
#define POST_START
#define POST_END
#endif

namespace casa { //# NAMESPACE CASA - BEGIN

class DataAccumulator;

class DataChunk {
public:
  friend DataAccumulator;

  DataChunk(String const &poltype) :
      num_pol_max_(4), num_pol_(0), num_chan_(0), data_(), flag_(), flag_row_(
          num_pol_max_, False), tsys_(), tcal_(), weight_(num_pol_max_, 1.0f), sigma_(
          weight_), poltype_(poltype), corr_type_(), filled_(NoData()), get_chunk_(
          nullptr), get_num_pol_(nullptr) {
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

  template<class T>
  void resizeTo(T &array, IPosition const &shape) {
    if (array.shape() != shape) {
      array.resize(shape);
    }
  }
  void initialize(size_t num_chan) {
    num_chan_ = num_chan;
    IPosition const shape(2, num_pol_max_, num_chan_);
    resizeTo(data_, shape);
    resizeTo(flag_, shape);
    resizeTo(tsys_, shape);
    resizeTo(tcal_, shape);
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

  bool accumulate(TableRecord const &record) {
    POST_START;

    if (!isValidRecord(record)) {
      return false;
    }

    uInt polid = record.asuInt("POLNO");

    if (num_pol_max_ <= polid) {
      return false;
    }
    Vector < Float > data = record.asArrayFloat("DATA");
    if (num_chan_ == 0) {
      size_t num_chan = data.size();
      initialize(num_chan);
    }
    Vector < Bool > flag = record.asArrayBool("FLAG");
    Bool flagrow = record.asBool("FLAG_ROW");

    if (data.shape() != flag.shape()) {
      return false;
    }

    Vector < Float > tsys;
    if (record.isDefined("TSYS")) {
      tsys.assign(record.asArrayFloat("TSYS"));
    }
    Vector < Float > tcal;
    if (record.isDefined("TCAL")) {
      tcal.assign(record.asArrayFloat("TCAL"));
    }

    if (data.nelements() != num_chan_) {
      return false;
    }

    data_.row(polid) = data;
    flag_.row(polid) = flag;
    flag_row_[polid] = flagrow;
    if (tsys.size() == num_chan_) {
      tsys_.row(polid) = tsys;
    } else if (!tsys.empty()){
      tsys_(polid, 0) = tsys[0];
    }
    if (tcal.size() == num_chan_) {
      tcal_.row(polid) = tcal;
    } else if (!tcal.empty()){
      tcal_(polid, 0) = tcal[0];
    }
    filled_ |= 0x01 << polid;

    return true;
  }

  bool get(TableRecord &record) {
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
  bool isValidRecord(TableRecord const &record) {
    bool is_valid = true;
    constexpr size_t num_required_keys = 4;
    constexpr const char *required_keys[] = { "POLNO", "DATA", "FLAG",
        "FLAG_ROW" };
    for (size_t i = 0; i < num_required_keys; ++i) {
      is_valid = is_valid && record.isDefined(required_keys[i]);
    }
    return is_valid;
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
  size_t num_pol_;
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
  bool (DataChunk::*get_chunk_)(TableRecord &record);
  uInt (DataChunk::*get_num_pol_)() const;

  bool getLinear(TableRecord &record) {
    POST_START;

    Vector < Float > weight;
    Vector < Float > sigma;
    if (isFullPol()) {
      // POL 0, 1, 2, and 3
      Matrix < Complex > data(4, num_chan_, Complex(0));
      Vector < Complex > complex0(data.row(0));
      setReal(complex0, data_.row(0));
      complex0.reference(data.row(3));
      setReal(complex0, data_.row(1));
      complex0.reference(data.row(1));
      setReal(complex0, data_.row(2));
      setImag(complex0, data_.row(3));
      data.row(2) = conj(data.row(1));
      Matrix < Bool > flag = flag_;
      flag.row(2) = flag.row(2) || flag.row(3);
      flag.row(3) = flag.row(1);
      flag.row(1) = flag.row(2);
      Bool flag_row = anyEQ(flag_row_, True);
      record.define("DATA", data);
      record.define("FLAG", flag);
      record.define("FLAG_ROW", flag_row);
      record.define("WEIGHT", weight_);
      record.define("SIGMA", sigma_);

      Matrix < Float > tsys;
      if (tsys_(0, 0) > 0.0f && tsys_(0, 1) > 0.0f) {
        // should be spectral Tsys
        tsys.resize(2, num_chan_);
        tsys = -1;
        tsys.row(0) = tsys_.row(0);
      } else if (tsys_(0, 0) > 0.0f) {
        // scalar Tsys
        tsys.resize(2, 1, -1.0f);
        tsys(0, 0) = tsys_(0, 0);
      }
      if (tsys_(1, 0) > 0.0f && tsys_(1, 1) > 0.0f) {
        tsys.resize(2, num_chan_, True);
        tsys.row(1) = tsys_.row(1);
      } else if (tsys_(1, 0) > 0.0f) {
        tsys.resize(2, 1, True);
        tsys.row(1) = -1.0f;
        tsys(1, 0) = tsys_(1, 0);
      }
      if (!tsys.empty() && anyNE(tsys, 1.0f)) {
        record.define("TSYS", tsys);
      }

      Matrix < Float > tcal;
      if (tcal_(0, 0) > 0.0f && tcal_(0, 1) > 0.0f) {
        // should be spectral Tcal
        tcal.resize(2, num_chan_);
        tcal = -1;
        tcal.row(0) = tcal_.row(0);
      } else if (tcal_(0, 0) > 0.0f) {
        // scalar Tcal
        tcal.resize(2, 1, -1.0f);
        tcal(0, 0) = tcal_(0, 0);
      }
      if (tcal_(1, 0) > 0.0f && tcal_(1, 1) > 0.0f) {
        if (tcal.ncolumn() < num_chan_) {
          tcal.resize(2, num_chan_, True);
        }
        tcal.row(1) = tcal_.row(1);
      } else if (tcal_(1, 0) > 0.0f) {
        tcal.row(1) = -1.0f;
        tcal(1, 0) = tcal_(1, 0);
      }
      if (!tcal.empty() && anyNE(tcal, 1.0f)) {
        record.define("TCAL", tcal);
      }

      record.define("NUM_POL", 4);
//      Vector<Int> corr_type(4);
//      corr_type[0] = Stokes::XX;
//      corr_type[1] = Stokes::XY;
//      corr_type[2] = Stokes::YX;
//      corr_type[3] = Stokes::YY;
      record.define("CORR_TYPE", corr_type_);
    } else if (isDualPol()) {
      // POL 0 and 1
      Matrix < Float > data = data_(IPosition(2, 0, 0),
          IPosition(2, 1, num_chan_ - 1));
      Matrix < Bool > flag = flag_(IPosition(2, 0, 0),
          IPosition(2, 1, num_chan_ - 1));
      Bool flag_row = flag_row_[0] || flag_row_[1];
      Vector < Float > weight = weight_(Slice(0, 2));
      record.define("FLOAT_DATA", data);
      record.define("FLAG", flag);
      record.define("FLAG_ROW", flag_row);
      record.define("WEIGHT", weight);
      record.define("SIGMA", weight);

      Matrix < Float > tsys;
      if (tsys_(0, 0) > 0.0f && tsys_(0, 1) > 0.0f) {
        // should be spectral Tsys
        tsys.resize(2, num_chan_);
        tsys = -1;
        tsys.row(0) = tsys_.row(0);
      } else if (tsys_(0, 0) > 0.0f) {
        // scalar Tsys
        tsys.resize(2, 1);
        tsys = -1.0f;
        tsys(0, 0) = tsys_(0, 0);
      }
      if (tsys_(1, 0) > 0.0f && tsys_(1, 1) > 0.0f) {
        tsys.resize(2, num_chan_, True);
        tsys.row(1) = tsys_.row(1);
      } else if (tsys_(1, 0) > 0.0f) {
        tsys.resize(2, 1, True);
        tsys.row(1) = -1.0f;
        tsys(1, 0) = tsys_(1, 0);
      }
      if (!tsys.empty() && anyNE(tsys, 1.0f)) {
        record.define("TSYS", tsys);
      }

      Matrix < Float > tcal;
      if (tcal_(0, 0) > 0.0f && tcal_(0, 1) > 0.0f) {
        // should be spectral Tcal
        tcal.resize(2, num_chan_);
        tcal = -1;
        tcal.row(0) = tcal_.row(0);
      } else if (tcal_(0, 0) > 0.0f) {
        // scalar Tcal
        tcal.resize(2, 1, -1.0f);
        tcal(0, 0) = tcal_(0, 0);
      }
      if (tcal_(1, 0) > 0.0f && tcal_(1, 1) > 0.0f) {
        tcal.resize(2, num_chan_, True);
        tcal.row(1) = tcal_.row(1);
      } else if (tcal_(1, 0) > 0.0f) {
        tcal.resize(2, 1, True);
        tcal.row(1) = -1.0f;
        tcal(1, 0) = tcal_(1, 0);
      }
      if (!tcal.empty() && anyNE(tcal, 1.0f)) {
        record.define("TCAL", tcal);
      }

      record.define("NUM_POL", 2);
      Vector < Int > corr_type(2);
      corr_type[0] = corr_type_[0];
      corr_type[1] = corr_type_[3];
      record.define("CORR_TYPE", corr_type);
    } else if (isSinglePol0()) {
      // only POL 0
      Slicer slicer(IPosition(2, 0, 0), IPosition(2, 1, num_chan_));
      Matrix < Float > data = data_(slicer);
      Matrix < Bool > flag = flag_(slicer);
      Bool flag_row = flag_row_(0);
      Vector < Float > weight = weight_(Slice(0, 1));
      record.define("FLOAT_DATA", data);
      record.define("FLAG", flag);
      record.define("FLAG_ROW", flag_row);
      record.define("WEIGHT", weight);
      record.define("SIGMA", weight);

      Matrix < Float > tsys;
      if (tsys_(0, 0) > 0.0f && tsys_(0, 1) > 0.0f) {
        // should be spectral Tsys
        tsys.resize(1, num_chan_);
        tsys = -1;
        tsys.row(0) = tsys_.row(0);
      } else if (tsys_(0, 0) > 0.0f) {
        // scalar Tsys
        tsys.resize(1, 1, -1.0f);
        tsys(0, 0) = tsys_(0, 0);
      }
      if (!tsys.empty() && anyNE(tsys, 1.0f)) {
        record.define("TSYS", tsys);
      }

      Matrix < Float > tcal;
      if (tcal_(0, 0) > 0.0f && tcal_(0, 1) > 0.0f) {
        // should be spectral Tcal
        tcal.resize(1, num_chan_);
        tcal = -1;
        tcal.row(0) = tcal_.row(0);
      } else if (tcal_(0, 0) > 0.0f) {
        // scalar Tcal
        tcal.resize(1, 1, -1.0f);
        tcal(0, 0) = tcal_(0, 0);
      }
      if (!tcal.empty() && anyNE(tcal, 1.0f)) {
        record.define("TCAL", tcal);
      }

      record.define("NUM_POL", 1);
      Vector < Int > corr_type(1, corr_type_[0]);
//      corr_type[0] = Stokes::XX;
      record.define("CORR_TYPE", corr_type);
    } else if (isSinglePol1()) {
      // only POL 1
      Slicer slicer(IPosition(2, 1, 0), IPosition(2, 1, num_chan_));
      Matrix < Float > data = data_(slicer);
      Matrix < Bool > flag = flag_(slicer);
      Bool flag_row = flag_row_(1);
      Vector < Float > weight = weight_(Slice(0, 1));
      record.define("FLOAT_DATA", data);
      record.define("FLAG", flag);
      record.define("FLAG_ROW", flag_row);
      record.define("WEIGHT", weight);
      record.define("SIGMA", weight);

      Matrix < Float > tsys;
      if (tsys_(1, 0) > 0.0f && tsys_(1, 1) > 0.0f) {
        // should be spectral Tsys
        tsys.resize(1, num_chan_);
        tsys = -1;
        tsys.row(0) = tsys_.row(1);
      } else if (tsys_(1, 0) > 0.0f) {
        // scalar Tsys
        tsys.resize(1, 1, -1.0f);
        tsys(0, 0) = tsys_(1, 0);
      }
      if (!tsys.empty() && anyNE(tsys, 1.0f)) {
        record.define("TSYS", tsys);
      }

      Matrix < Float > tcal;
      if (tcal_(1, 0) > 0.0f && tcal_(1, 1) > 0.0f) {
        // should be spectral Tcal
        tcal.resize(1, num_chan_);
        tcal = -1;
        tcal.row(0) = tcal_.row(1);
      } else if (tcal_(1, 0) > 0.0f) {
        // scalar Tcal
        tcal.resize(1, 1, -1.0f);
        tcal(0, 0) = tcal_(1, 0);
      }
      if (!tcal.empty() && anyNE(tcal, 1.0f)) {
        record.define("TCAL", tcal);
      }

      record.define("NUM_POL", 1);
      Vector < Int > corr_type(1, corr_type_[3]);
//      corr_type[0] = Stokes::YY;
      record.define("CORR_TYPE", corr_type);
    } else {
      return false;
    }

    POST_END;
    return true;
  }

  bool getCircular(TableRecord &record) {
    return getLinear(record);
  }

  bool getStokes(TableRecord &record) {
    POST_START;

    if (isFullPol()) {
      record.define("FLOAT_DATA", data_);
      record.define("FLAG", flag_);
      record.define("FLAG_ROW", anyTrue(flag_row_));
      record.define("SIGMA", sigma_);
      record.define("WEIGHT", weight_);

      record.define("NUM_POL", 4);
//      Vector<Int> corr_type(4);
//      corr_type[0] = Stokes::I;
//      corr_type[1] = Stokes::Q;
//      corr_type[2] = Stokes::U;
//      corr_type[3] = Stokes::V;
      record.define("CORR_TYPE", corr_type_);
    } else if (isSinglePol0()) {
      Slicer slicer(IPosition(2, 0, 0), IPosition(2, 1, num_chan_));
      record.define("FLOAT_DATA", data_(slicer));
      record.define("FLAG", flag_(slicer));
      record.define("FLAG_ROW", flag_row_[0]);
      Slice slice(0, 1);
      record.define("SIGMA", weight_(slice));
      record.define("WEIGHT", weight_(slice));

      record.define("NUM_POL", 1);
      Vector < Int > corr_type(1, corr_type_[0]);
//      corr_type[0] = Stokes::I;
      record.define("CORR_TYPE", corr_type);
    } else {
      return false;
    }

    POST_END;
    return true;
  }

  bool getLinpol(TableRecord &record) {
    POST_START;

    if (isDualPol()) {
      // POL 0 and 1
      Matrix < Float > data = data_(IPosition(2, 0, 0),
          IPosition(2, 1, num_chan_ - 1));
      Matrix < Bool > flag = flag_(IPosition(2, 0, 0),
          IPosition(2, 1, num_chan_ - 1));
      Bool flag_row = flag_row_[0] || flag_row_[1];
      Vector < Float > weight = weight_(Slice(0, 2));
      record.define("FLOAT_DATA", data);
      record.define("FLAG", flag);
      record.define("FLAG_ROW", flag_row);
      record.define("WEIGHT", weight);
      record.define("SIGMA", weight);

      record.define("NUM_POL", 2);
//      Vector<Int> corr_type(2);
//      corr_type[0] = Stokes::Plinear;
//      corr_type[1] = Stokes::Pangle;
      record.define("CORR_TYPE", corr_type_);
    } else if (isSinglePol0()) {
      Slicer slicer(IPosition(2, 0, 0), IPosition(2, 1, num_chan_));
      record.define("FLOAT_DATA", data_(slicer));
      record.define("FLAG", flag_(slicer));
      record.define("FLAG_ROW", flag_row_[0]);
      Slice slice(0, 1);
      record.define("SIGMA", weight_(slice));
      record.define("WEIGHT", weight_(slice));

      record.define("NUM_POL", 1);
      Vector < Int > corr_type(1, corr_type_[0]);
//      corr_type[0] = Stokes::Plinear;
      record.define("CORR_TYPE", corr_type);
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
public:
  DataAccumulator() :
      pool_(), antenna_id_(), spw_id_(), field_id_(), feed_id_(), scan_(), subscan_(), intent_(), direction_(), interval_(), indexer_(), time_(
          -1.0), is_ready_(false), is_free_() {
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
//    return std::count_if(pool_.begin(), pool_.end(), [](DataChunk * const &c){
//      return c->getNumPol() > 0;
//    });
  }

  size_t getNumberOfActiveChunks() const {
    return std::count_if(pool_.begin(), pool_.end(), [](DataChunk * const &c) {
      return c->getNumPol() > 0;
    });
  }

  bool queryForGet(TableRecord const &record) const {
    Double const time = record.asDouble("TIME");
    bool is_ready = (0.0 <= time_) && !(time_ == time);
    return is_ready;
  }

  void clear() {
    for (auto iter = pool_.begin(); iter != pool_.end(); ++iter) {
      (*iter)->clear();
    }
    time_ = -1.0;
  }

  bool get(size_t ichunk, TableRecord &record) {
    if (pool_.size() == 0) {
      return false;
    } else if (ichunk >= pool_.size()) {
      return false;
    }
    bool status = pool_[ichunk]->get(record);
    Int antennaid = -1;
    Int spwid = -1;
    Int fieldid = -1;
    Int feedid = -1;
    Int scan = -1;
    Int subscan = -1;
    String intent = "";
    Double time = -1.0;
    String poltype = "";
    Matrix < Double > direction;
    Double interval = -1.0;
    if (status) {
      poltype = pool_[ichunk]->getPolType();
      time = time_;
      antennaid = antenna_id_[ichunk];
      spwid = spw_id_[ichunk];
      fieldid = field_id_[ichunk];
      feedid = feed_id_[ichunk];
      scan = scan_[ichunk];
      subscan = subscan_[ichunk];
      intent = intent_[ichunk];
      direction.assign(direction_[ichunk]);
      interval = interval_[ichunk];
    }
    record.define("TIME", time);
    record.define("POL_TYPE", poltype);
    record.define("ANTENNA_ID", antennaid);
    record.define("SPECTRAL_WINDOW_ID", spwid);
    record.define("FIELD_ID", fieldid);
    record.define("FEED_ID", feedid);
    record.define("SCAN", scan);
    record.define("SUBSCAN", subscan);
    record.define("INTENT", intent);
    record.define("DIRECTION", direction);
    record.define("INTERVAL", interval);
    return status;
  }

  bool accumulate(TableRecord const &record) {
    POST_START;

    if (!isValidRecord(record)) {
      return false;
    }

    Double time = record.asDouble("TIME");
    if (time_ < 0.0) {
      time_ = time;
    }
    if (time_ != time) {
      return false;
    }
    Int antennaid = record.asInt("ANTENNA_ID");
    Int spwid = record.asInt("SPECTRAL_WINDOW_ID");
    Int fieldid = record.asInt("FIELD_ID");
    Int feedid = record.asInt("FEED_ID");
    Int scan = record.asInt("SCAN");
    Int subscan = record.asInt("SUBSCAN");
    String intent = record.asString("INTENT");
    String poltype = record.asString("POL_TYPE");
    String key = "ANTENNA" + String::toString(antennaid) + "SPW"
        + String::toString(spwid) + "FIELD" + String::toString(fieldid) + "FEED"
        + String::toString(feedid) + intent + poltype;
    Matrix < Double > direction = record.asArrayDouble("DIRECTION");
    Double interval = record.asDouble("INTERVAL");
    bool status = false;
    if (indexer_.isDefined(key)) {
      uInt index = indexer_.asuInt(key);
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
      direction_.push_back(Vector<Double>());
      interval_.push_back(-1.0);
      uInt index = pool_.size() - 1;
      indexer_.define(key, index);
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
  bool isValidRecord(TableRecord const &record) {
    constexpr size_t num_required_keys = 11;
    constexpr const char *required_keys[num_required_keys] = { "TIME",
        "ANTENNA_ID", "FIELD_ID", "SPECTRAL_WINDOW_ID", "FEED_ID", "SCAN",
        "SUBSCAN", "INTENT", "POL_TYPE", "DIRECTION", "INTERVAL" };
    bool is_valid = true;
    for (size_t i = 0; i < num_required_keys; ++i) {
      is_valid = is_valid && record.isDefined(required_keys[i]);
    }
    return is_valid;
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
  Record indexer_;
  Double time_;
  bool is_ready_;
  std::vector<bool> is_free_;
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_DATAACCUMULATOR_H_ */

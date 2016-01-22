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

#include <casacore/tables/Tables/TableRecord.h>

#define POST_START std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl
#define POST_END std::cout << "End " << __PRETTY_FUNCTION__ << std::endl

namespace casa { //# NAMESPACE CASA - BEGIN

class DataAccumulator;

class DataChunk {
public:
  friend DataAccumulator;

  DataChunk(String const &poltype) :
      num_pol_max_(4), num_pol_(0), num_chan_(0), data_(), flag_(), flag_row_(
          num_pol_max_, False), weight_(num_pol_max_, 1.0f), sigma_(weight_), poltype_(
          poltype), filled_(NoData()), get_chunk_(nullptr), get_num_pol_(
          nullptr) {
    POST_START;

    setPolType(poltype);
    std::cout << (unsigned int) SinglePol0() << " "
        << (unsigned int) SinglePol1() << " " << (unsigned int) DualPol() << " "
        << (unsigned int) FullPol() << std::endl;

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
    IPosition const shape(2, num_pol_max_, num_chan_);
    if (data_.shape() != shape) {
      data_.resize(IPosition(2, num_pol_max_, num_chan_));
    }
    if (flag_.shape() != shape) {
      flag_.resize(IPosition(2, num_pol_max_, num_chan_));
    }
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

    std::cout << "VALID" << std::endl;

    uInt polid = record.asuInt("POLNO");

    std::cout << "POLNO=" << polid << std::endl;

    if (num_pol_max_ <= polid) {
      return false;
      //AipsError("Invalid POLNO");
    }
    Vector < Float > data = record.asArrayFloat("DATA");
    if (num_chan_ == 0) {
      size_t num_chan = data.size();
      initialize(num_chan);
    }
    Vector < Bool > flag = record.asArrayBool("FLAG");
    Bool flagrow = record.asBool("FLAG_ROW");

    std::cout << "data=" << data << std::endl;
    std::cout << "flag=" << flag << std::endl;

    if (data.shape() != flag.shape()) {
      return false;
    }

    std::cout << "num_chan_ = " << num_chan_ << std::endl;

    if (data.nelements() != num_chan_) {
      return false;
    }

    data_.row(polid) = data;
    flag_.row(polid) = flag;
    flag_row_[polid] = flagrow;
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
      std::cout << "key " << required_keys[i] << " is_valid " << is_valid << std::endl;
    }
    return is_valid;
  }
  void setPolType(String const &poltype) {
    POST_START;

    std::cout << "poltype = " << poltype << std::endl;
    poltype_ = poltype;
    if (poltype_ == "linear") {
      get_chunk_ = &DataChunk::getLinear;
      get_num_pol_ = &DataChunk::getNumPolLinear;
    } else if (poltype_ == "circular") {
      get_chunk_ = &DataChunk::getCircular;
      get_num_pol_ = &DataChunk::getNumPolCircular;
    } else if (poltype_ == "stokes") {
      get_chunk_ = &DataChunk::getStokes;
      get_num_pol_ = &DataChunk::getNumPolStokes;
    } else if (poltype_ == "linpol") {
      get_chunk_ = &DataChunk::getLinpol;
      get_num_pol_ = &DataChunk::getNumPolLinpol;
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
  Vector<Float> weight_;
  Vector<Float> sigma_;
  String poltype_;
  unsigned char filled_;
  bool (DataChunk::*get_chunk_)(TableRecord &record);
  uInt (DataChunk::*get_num_pol_)() const;

  bool getLinear(TableRecord &record) {
    POST_START;

    std::cout << "data_.column(0).shape()=" << data_.column(0).shape()
        << std::endl;
    std::cout << "data_.row(0).shape()=" << data_.row(0).shape() << std::endl;

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
    } else if (isSinglePol0()) {
      // only POL 0
      Slicer slicer(IPosition(2, 0, 0), IPosition(2, 1, num_chan_));
      Matrix < Float > data = data_(slicer);
      Matrix < Bool > flag = flag_(slicer);
      std::cout << data << std::endl;
      std::cout << flag << std::endl;
      Bool flag_row = flag_row_(0);
      Vector < Float > weight = weight_(Slice(0, 1));
      std::cout << weight << std::endl;
      record.define("FLOAT_DATA", data);
      record.define("FLAG", flag);
      record.define("FLAG_ROW", flag_row);
      record.define("WEIGHT", weight);
      record.define("SIGMA", weight);
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
    } else if (isSinglePol0()) {
      Slicer slicer(IPosition(2, 0, 0), IPosition(2, 1, num_chan_));
      record.define("FLOAT_DATA", data_(slicer));
      record.define("FLAG", flag_(slicer));
      record.define("FLAG_ROW", flag_row_[0]);
      Slice slice(0, 1);
      record.define("SIGMA", weight_(slice));
      record.define("WEIGHT", weight_(slice));
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
    } else if (isSinglePol0()) {
      Slicer slicer(IPosition(2, 0, 0), IPosition(2, 1, num_chan_));
      record.define("FLOAT_DATA", data_(slicer));
      record.define("FLAG", flag_(slicer));
      record.define("FLAG_ROW", flag_row_[0]);
      Slice slice(0, 1);
      record.define("SIGMA", weight_(slice));
      record.define("WEIGHT", weight_(slice));
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
      pool_(), spw_id_(), field_id_(), feed_id_(), intent_(), indexer_(), time_(
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
      //delete *iter;
      (*iter)->clear();
    }
    //pool_.resize(0);
    time_ = -1.0;
    //indexer_ = Record();
  }

  bool get(size_t ichunk, TableRecord &record) {
    if (pool_.size() == 0) {
      return false;
    } else if (ichunk >= pool_.size()) {
      return false;
    }
    bool status = pool_[ichunk]->get(record);
    Int spwid = -1;
    Int fieldid = -1;
    Int feedid = -1;
    String intent = "";
    Double time = -1.0;
    String poltype = "";
    if (status) {
       poltype = pool_[ichunk]->getPolType();
       time = time_;
       spwid = spw_id_[ichunk];
       fieldid = field_id_[ichunk];
       feedid = feed_id_[ichunk];
       intent = intent_[ichunk];
    }
    record.define("TIME", time);
    record.define("POL_TYPE", poltype);
    record.define("SPECTRAL_WINDOW_ID", spwid);
    record.define("FIELD_ID", fieldid);
    record.define("FEED_ID", feedid);
    record.define("INTENT", intent);
    return status;
  }

  bool accumulate(TableRecord const &record) {
    POST_START;

    if (!isValidRecord(record)) {
      return false;
    }

    std::cout << "valid" << std::endl;

    Double time = record.asDouble("TIME");
    if (time_ < 0.0) {
      std::cout << "first record" << std::endl;
      time_ = time;
    }
    if (time_ != time) {
      std::cout << "not valid timestamp" << std::endl;
      return false;
      //AipsError("Invalid use of DataAccumulator");
    }
    Int spwid = record.asInt("SPECTRAL_WINDOW_ID");
    Int fieldid = record.asInt("FIELD_ID");
    Int feedid = record.asInt("FEED_ID");
    String intent = record.asString("INTENT");
    String poltype = record.asString("POL_TYPE");
    String key = "SPW" + String::toString(spwid) + "FIELD"
        + String::toString(fieldid) + "FEED" + String::toString(feedid) + intent
        + poltype;
    bool status = false;
    if (indexer_.isDefined(key)) {
      std::cout << "accumulate " << key << std::endl;
      uInt index = indexer_.asuInt(key);
      status = pool_[index]->accumulate(record);
      if (status) {
        spw_id_[index] = spwid;
        field_id_[index] = fieldid;
        feed_id_[index] = feedid;
        intent_[index] = intent;
      }
    } else {
      std::cout << "new entry " << key << std::endl;
      pool_.push_back(new DataChunk(poltype));
      spw_id_.push_back(-1);
      field_id_.push_back(-1);
      feed_id_.push_back(-1);
      intent_.push_back("");
      uInt index = pool_.size() - 1;
      indexer_.define(key, index);
      status = pool_[index]->accumulate(record);
      if (status) {
        spw_id_[index] = spwid;
        field_id_[index] = fieldid;
        feed_id_[index] = feedid;
        intent_[index] = intent;
      }
    }

    std::cout << "status = " << status << std::endl;

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
    constexpr size_t num_required_keys = 6;
    constexpr const char *required_keys[num_required_keys] = { "TIME",
        "FIELD_ID", "SPECTRAL_WINDOW_ID", "FEED_ID", "INTENT", "POL_TYPE" };
    bool is_valid = true;
    for (size_t i = 0; i < num_required_keys; ++i) {
      is_valid = is_valid && record.isDefined(required_keys[i]);
    }
    return is_valid;
  }
  std::vector<DataChunk *> pool_;
  std::vector<Int> spw_id_;
  std::vector<Int> field_id_;
  std::vector<Int> feed_id_;
  std::vector<String> intent_;
  Record indexer_;
  Double time_;
  bool is_ready_;
  std::vector<bool> is_free_;
};

}//# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_DATAACCUMULATOR_H_ */

/*
 * MSDataRecord.h
 *
 *  Created on: Jan 27, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_MSDATARECORD_H_
#define SINGLEDISH_FILLER_MSDATARECORD_H_

#include <casacore/casa/BasicSL/String.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSMainColumns.h>

namespace {
template<class T>
void copyStorage(size_t n, T const *src, T *dst) {
  for (size_t i = n; i < n; ++i) {
    dst[i] = src[i];
  }
}

}

namespace casa { //# NAMESPACE CASA - BEGIN

struct MSDataRecord {
  MSDataRecord() :
      block_size_(16384u), num_data_storage_(block_size_), num_tsys_storage_(
          block_size_), num_tcal_storage_(block_size_), data_shape_(1, 1), tsys_shape_(
          1, 1), tcal_shape_(1, 1), data_storage_(new Float[num_data_storage_]), flag_storage_(
          new Bool[num_data_storage_]), tsys_storage_(
          new Float[num_data_storage_]), tcal_storage_(
          new Float[num_data_storage_]), direction(2, 1, 0.0), direction_slice(
          direction.column(0)), data(data_shape_, data_storage_.get(), SHARE), flag(
          data_shape_, flag_storage_.get(), SHARE), tsys(tsys_shape_,
          tsys_storage_.get(), SHARE), tcal(tcal_shape_, tcal_storage_.get(),
          SHARE) {
  }

  ~MSDataRecord() {
  }

  // method
  void clear() {
    time = -1.0;
    interval = -1.0;
    antenna_id = -1;
    field_id = -1;
    spw_id = -1;
    feed_id = -1;
    scan = -1;
    subscan = -1;
    polno = 0;
    intent = "";
    pol_type = "";
    direction = 0.0;
    data_shape_[0] = 0;
    flag_row = True;
  }

  void setDataSize(size_t n) {
    Bool redirect = False;
    if (data_shape_[0] != n) {
//      std::cout << "resize data to " << n << std::endl;
      data_shape_[0] = n;
      redirect = True;
    }
    if (num_data_storage_ < n) {
      size_t new_num_storage = num_data_storage_ + block_size_;
      while (new_num_storage < n) {
        new_num_storage += block_size_;
      }
//      std::cout << "resize data storage to " << new_num_storage << std::endl;
      Float *new_data_storage = new Float[new_num_storage];
      copyStorage(num_data_storage_, data_storage_.get(), new_data_storage);
      data_storage_.reset(new_data_storage);
      Bool *new_flag_storage = new Bool[new_num_storage];
      copyStorage(num_data_storage_, flag_storage_.get(), new_flag_storage);
      flag_storage_.reset(new_flag_storage);
      num_data_storage_ = new_num_storage;
      redirect = True;
    }
    if (redirect) {
      data.reference(
          Vector < Float > (data_shape_, data_storage_.get(), SHARE));
      flag.reference(Vector < Bool > (data_shape_, flag_storage_.get(), SHARE));
    }
  }

  void setTsysSize(size_t n) {
    Bool redirect = False;
    if (tsys_shape_[0] != n) {
//      std::cout << "resize tsys to " << n << std::endl;
      tsys_shape_[0] = n;
      redirect = True;
    }
    if (num_data_storage_ < n) {
      size_t new_num_storage = num_tsys_storage_ + block_size_;
      while (new_num_storage < n) {
        new_num_storage += block_size_;
      }
//      std::cout << "resize tsys storage to " << new_num_storage << std::endl;
      Float *new_tsys_storage = new Float[new_num_storage];
      copyStorage(num_tsys_storage_, tsys_storage_.get(), new_tsys_storage);
      tsys_storage_.reset(new_tsys_storage);
      num_tsys_storage_ = new_num_storage;
      redirect = True;
    }
    if (redirect) {
      tsys.reference(
          Vector < Float > (tsys_shape_, tsys_storage_.get(), SHARE));
    }
  }

  void setTcalSize(size_t n) {
    Bool redirect = False;
    if (tcal_shape_[0] != n) {
//      std::cout << "resize tcal to " << n << std::endl;
      tcal_shape_[0] = n;
      redirect = True;
    }
    if (num_data_storage_ < n) {
      size_t new_num_storage = num_tcal_storage_ + block_size_;
      while (new_num_storage < n) {
        new_num_storage += block_size_;
      }
//      std::cout << "resize tcal storage to " << new_num_storage << std::endl;
      Float *new_tcal_storage = new Float[new_num_storage];
      copyStorage(num_tcal_storage_, tcal_storage_.get(), new_tcal_storage);
      tcal_storage_.reset(new_tcal_storage);
      num_tcal_storage_ = new_num_storage;
      redirect = True;
    }
    if (redirect) {
      tcal.reference(
          Vector < Float > (tcal_shape_, tcal_storage_.get(), SHARE));
    }
  }

  MSDataRecord(MSDataRecord const &other) :
      DataRecord() {
    *this = other;
  }
  MSDataRecord &operator=(MSDataRecord const &other) {
    time = other.time;
    interval = other.interval;
    antenna_id = other.antenna_id;
    field_id = other.field_id;
    spw_id = other.spw_id;
    feed_id = other.feed_id;
    scan = other.scan;
    subscan = other.subscan;
    polno = other.polno;
    intent = other.intent;
    pol_type = other.pol_type;
    direction = other.direction;
    flag_row = other.flag_row;
    setDataSize(other.data_shape_[0]);
    data = other.data;
    flag = other.flag;
    setTsysSize(other.tsys_shape_[0]);
    tsys = other.tsys;
    setTcalSize(other.tcal_shape_[0]);
    tcal = other.tcal;

    return *this;
  }

private:
  size_t const block_size_;
  size_t num_data_storage_;
  size_t num_tsys_storage_;
  size_t num_tcal_storage_;
  IPosition data_shape_;
  IPosition tsys_shape_;
  IPosition tcal_shape_;
  std::unique_ptr<Float[]> data_storage_;
  std::unique_ptr<Bool[]> flag_storage_;
  std::unique_ptr<Float[]> tsys_storage_;
  std::unique_ptr<Float[]> tcal_storage_;

public:
  // mandatory
  Double time;
  Double interval;
  Int antenna_id;
  Int field_id;
  Int spw_id;
  Int feed_id;
  Int scan;
  Int subscan;
  uInt polno;
  String intent;
  String pol_type;
  Matrix<Double> direction;
  Vector<Double> direction_slice;
  Vector<Float> data;
  Vector<Bool> flag;
  Bool flag_row;

  // optional
  Vector<Float> tsys;
  Vector<Float> tcal;

};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_MSDATARECORD_H_ */

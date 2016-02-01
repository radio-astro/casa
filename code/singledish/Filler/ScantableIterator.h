/*
 * ScantableIterator.h
 *
 *  Created on: Jan 28, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SCANTABLEITERATOR_H_
#define SINGLEDISH_FILLER_SCANTABLEITERATOR_H_

#include <singledish/Filler/FieldRecord.h>
#include <singledish/Filler/SourceRecord.h>
#include <singledish/Filler/SpectralWindowRecord.h>
#include <singledish/Filler/SysCalRecord.h>
#include <singledish/Filler/WeatherRecord.h>

#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Utilities/Compare.h>
#include <casacore/casa/Utilities/Sort.h>

#include <casacore/measures/Measures/MFrequency.h>

#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/ScalarColumn.h>
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/tables/TaQL/ExprNode.h>

using namespace casacore;

namespace casa { //# NAMESPACE CASA - BEGIN

class ScantableIteratorInterface {
public:
  ScantableIteratorInterface(Table const &table) :
      current_iter_(0), main_table_(table), num_iter_(0) {
  }
  virtual ~ScantableIteratorInterface() {
  }
  void initialize(size_t num_iter) {
    num_iter_ = num_iter;
    current_iter_ = 0;
  }
  bool moreData() const {
    return current_iter_ < num_iter_;
  }
  void next() {
    ++current_iter_;
  }

protected:
  size_t current_iter_;
  Table const main_table_;

private:
  size_t num_iter_;
};

class ScantableWeatherIterator: public ScantableIteratorInterface {
public:
  typedef void * Product;
  ScantableWeatherIterator(Table const &table) :
      ScantableIteratorInterface(table) {
    TableRecord const &header = main_table_.keywordSet();
    sub_table_ = header.asTable("WEATHER");
    size_t nrow = sub_table_.nrow();
    initialize(nrow);

    // attach columns
    id_column_.attach(sub_table_, "ID");
    temperature_column_.attach(sub_table_, "TEMPERATURE");
    pressure_column_.attach(sub_table_, "PRESSURE");
    humidity_column_.attach(sub_table_, "HUMIDITY");
    wind_speed_column_.attach(sub_table_, "WINDSPEED");
    wind_direction_column_.attach(sub_table_, "WINDAZ");
  }
  virtual ~ScantableWeatherIterator() {
  }

  void getEntry(WeatherRecord &record) {
    size_t const irow = current_iter_;
    uInt weather_id = id_column_(irow);
    Double time_min = 0.0;
    Double time_max = 0.0;
    Table subtable = main_table_(main_table_.col("WEATHER_ID") == weather_id);
    if (subtable.nrow() > 0) {
      ROScalarColumn < Double > column(subtable, "TIME");
      Vector < Double > time_list = column.getColumn();
      minMax(time_min, time_max, time_list);
    }

    record.antenna_id = 0;
    constexpr double kDay2Sec = 86400.0;
    record.time = 0.5 * (time_min + time_max) * kDay2Sec;
    record.interval = (time_max - time_min) * kDay2Sec;
    record.temperature = temperature_column_(irow);
    record.pressure = pressure_column_(irow);
    record.rel_humidity = humidity_column_(irow);
    record.wind_speed = wind_speed_column_(irow);
    record.wind_direction = wind_direction_column_(irow);
  }
  void getProduct(Product *p) {

  }

private:
  Table sub_table_;
  ScalarColumn<uInt> id_column_;
  ScalarColumn<Float> temperature_column_;
  ScalarColumn<Float> pressure_column_;
  ScalarColumn<Float> humidity_column_;
  ScalarColumn<Float> wind_speed_column_;
  ScalarColumn<Float> wind_direction_column_;
};

class ScantableFrequenciesIterator: public ScantableIteratorInterface {
public:
  typedef void * Product;
  ScantableFrequenciesIterator(Table const &table) :
      ScantableIteratorInterface(table) {
    TableRecord const &header = main_table_.keywordSet();
    sub_table_ = header.asTable("FREQUENCIES");
    //size_t nrow = sub_table_.nrow();
    ROScalarColumn < uInt > ifno_column(main_table_, "IFNO");
    Vector < uInt > ifno_list = ifno_column.getColumn();
    Sort sorter;
    sorter.sortKey(ifno_list.data(), TpUInt);
    Vector < uInt > index_vector;
    uInt n = sorter.sort(index_vector, ifno_list.nelements(),
        Sort::HeapSort | Sort::NoDuplicates);

    initialize(n);
    ifno_list_.resize(n);
    for (uInt i = 0; i < n; ++i) {
      ifno_list_[i] = ifno_list[index_vector[i]];
    }

    ROScalarColumn < uInt > freq_id_column(main_table_, "FREQ_ID");

    // attach columns
    id_column_.attach(sub_table_, "ID");
    refpix_column_.attach(sub_table_, "REFPIX");
    refval_column_.attach(sub_table_, "REFVAL");
    increment_column_.attach(sub_table_, "INCREMENT");
    id_list_ = id_column_.getColumn();
  }
  virtual ~ScantableFrequenciesIterator() {
  }

  void getEntry(SpectralWindowRecord &record) {
    size_t const irow = current_iter_;
    std::cout << "getEntry for row " << irow << std::endl;
    Int spw_id = ifno_list_[irow];
    Table subtable = main_table_(main_table_.col("IFNO") == (uInt) spw_id, 1);
    ROScalarColumn < uInt > freq_id_column(subtable, "FREQ_ID");
    uInt freq_id = freq_id_column(0);
    Int jrow = -1;
    for (uInt i = 0; i < id_list_.size(); ++i) {
      if (id_list_[i] == freq_id) {
        jrow = (Int) i;
        break;
      }
    }
    ROArrayColumn<uChar> flag_column(subtable, "FLAGTRA");
    Int num_chan = flag_column.shape(0)[0];
    String freq_frame = sub_table_.keywordSet().asString("BASEFRAME");
    MFrequency::Types frame_type;
    Bool status = MFrequency::getType(frame_type, freq_frame);
    if (!status) {
      frame_type = MFrequency::N_Types;
    }
    Double refpix = refpix_column_(jrow);
    Double refval = refval_column_(jrow);
    Double increment = increment_column_(jrow);
//    std::cout << "spw " << spw_id << " nchan " << num_chan << " mfr "
//        << (Int) frame_type << " (" << freq_frame << ") ref " << refpix << ", "
//        << refval << ", " << increment << std::endl;
    record.spw_id = spw_id;
    record.num_chan = num_chan;
    record.meas_freq_ref = frame_type;
    record.refpix = refpix;
    record.refval = refval;
    record.increment = increment;
  }
  virtual void getProduct(Product *p) {

  }

private:
  Table sub_table_;
  ScalarColumn<uInt> id_column_;
  ScalarColumn<Double> refpix_column_;
  ScalarColumn<Double> refval_column_;
  ScalarColumn<Double> increment_column_;
  Vector<uInt> ifno_list_;
  Vector<uInt> id_list_;
};

class ScantableFieldIterator: public ScantableIteratorInterface {
public:
  typedef std::map<String, Int> Product;
  ScantableFieldIterator(Table const &table) :
      ScantableIteratorInterface(table), row_list_(), is_reserved_(), field_column_(
          main_table_, "FIELDNAME"), source_column_(main_table_, "SRCNAME"), time_column_(
          main_table_, "TIME"), direction_column_(main_table_, "SRCDIRECTION"), scanrate_column_(
          main_table_, "SCANRATE"), direction_storage_(2, 2, 0.0) {
    Vector < String > field_name_list = field_column_.getColumn();
    Sort sorter;
    sorter.sortKey(field_name_list.data(), TpString);
    uInt n = sorter.sort(row_list_, field_name_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
    is_reserved_.resize(n);
    is_reserved_ = False;
    initialize(n);
  }

  virtual ~ScantableFieldIterator() {
  }

  void getEntry(FieldRecord &record) {
    uInt const irow = row_list_[current_iter_];
    String field_name_with_id = field_column_(irow);
    auto pos = field_name_with_id.find("__");
    auto defaultFieldId = [&]() {
      Int my_field_id = 0;
      while (is_reserved_[my_field_id] && my_field_id < is_reserved_.size()) {
        my_field_id++;
      }
      if (my_field_id >= is_reserved_.size()) {
        throw AipsError("Internal inconsistency in FIELD_ID numbering");
      }
      is_reserved_[my_field_id] = True;
      return my_field_id;
    };
    if (pos != String::npos) {
      record.name = field_name_with_id.substr(0, pos);
      Int field_id = String::toInt(field_name_with_id.substr(pos + 2));
      if (field_id < 0) {
        record.field_id = defaultFieldId();
      } else if (field_id >= is_reserved_.size() || !is_reserved_[field_id]) {
        record.field_id = field_id;
        is_reserved_[field_id] = True;
      } else {
        record.field_id = defaultFieldId();
      }
    } else {
      record.name = field_name_with_id;
      record.field_id = defaultFieldId();
    }
    record.time = time_column_(irow) * 86400.0;
    record.source_name = source_column_(irow);
    record.frame = MDirection::J2000;
    Matrix < Double
        > direction(direction_storage_(IPosition(2, 0, 0), IPosition(2, 1, 0)));
    std::cout << "direction = " << direction << " (shape " << direction.shape()
        << ")" << std::endl;
    direction_storage_.column(0) = direction_column_(irow);
    if (scanrate_column_.isDefined(irow)) {
      Vector < Double > scan_rate = scanrate_column_(irow);
      if (anyNE(scan_rate, 0.0)) {
        direction_storage_.column(1) = scan_rate;
        direction.reference(direction_storage_);
      }
    }
    std::cout << "direction = " << direction << " (shape " << direction.shape()
        << ")" << std::endl;
    record.direction = direction;

    // update product
    product_[field_name_with_id] = record.field_id;
  }
  virtual void getProduct(Product *p) {
    if (p) {
      for (auto iter = product_.begin(); iter != product_.end(); ++iter) {
        (*p)[iter->first] = iter->second;
      }
    }
  }

private:
  Vector<uInt> row_list_;
  Vector<Bool> is_reserved_;
  ROScalarColumn<String> field_column_;
  ROScalarColumn<String> source_column_;
  ROScalarColumn<Double> time_column_;
  ArrayColumn<Double> direction_column_;
  ArrayColumn<Double> scanrate_column_;
  Matrix<Double> direction_storage_;
  Product product_;
};

class ScantableSourceIterator: public ScantableIteratorInterface {
public:
  typedef void * Product;
  ScantableSourceIterator(Table const &table) :
      ScantableIteratorInterface(table), name_column_(main_table_, "SRCNAME"), direction_column_(
          main_table_, "SRCDIRECTION"), proper_motion_column_(main_table_,
          "SRCPROPERMOTION"), sysvel_column_(main_table_, "SRCVELOCITY"), molecule_id_column_(
          main_table_, "MOLECULE_ID"), ifno_column_(main_table_, "IFNO"), molecules_table_(), row_list_(), source_id_map_() {
    TableRecord const &header = main_table_.keywordSet();
    molecules_table_ = header.asTable("MOLECULES");
    Vector < String > source_name_list = name_column_.getColumn();
    Vector < uInt > ifno_list = ifno_column_.getColumn();
    Sort sorter;
    sorter.sortKey(source_name_list.data(), TpString);
    Vector < uInt > unique_vector;
    uInt num_unique = sorter.sort(unique_vector, source_name_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
    std::cout << "unique_vector = " << unique_vector << std::endl;
    for (uInt i = 0; i < num_unique; ++i) {
      //std::cout << "name " << name_column_(unique_vector[i]) << " ID " << i
      //    << std::endl;
      source_id_map_[name_column_(unique_vector[i])] = (Int) i;
    }
    Sort sorter2;
    sorter2.sortKey(source_name_list.data(), TpString);
    sorter2.sortKey(ifno_list.data(), TpUInt);
    unique_vector.resize();
    num_unique = sorter2.sort(row_list_, source_name_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
//    std::cout << "num_unique = " << num_unique << std::endl;
    for (uInt i = 0; i < num_unique; ++i) {
      std::cout << i << ": SRCNAME \"" << name_column_(row_list_[i])
          << "\" IFNO " << ifno_column_(row_list_[i]) << std::endl;
    }
    initialize(num_unique);
  }

  virtual ~ScantableSourceIterator() {
  }

  void getEntry(SourceRecord &record) {
    uInt const irow = row_list_[current_iter_];
    record.name = name_column_(irow);
    record.source_id = source_id_map_[record.name];
    record.spw_id = ifno_column_(irow);
    record.direction = MDirection(
        Quantum<Vector<Double> >(direction_column_(irow), "rad"),
        MDirection::J2000);
    record.proper_motion = proper_motion_column_(irow);
    uInt molecule_id = molecule_id_column_(irow);
    Table t = molecules_table_(molecules_table_.col("ID") == molecule_id, 1);
    Double sysvel = sysvel_column_(irow);
    if (t.nrow() == 1) {
      ArrayColumn<Double> rest_freq_column(t, "RESTFREQUENCY");
      ArrayColumn<String> molecule_name_column(t, "NAME");
      if (rest_freq_column.isDefined(0)) {
        record.rest_frequency = rest_freq_column(0);
      }
      if (molecule_name_column.isDefined(0)) {
        record.transition = molecule_name_column(0);
      }
    }
    record.num_lines = record.rest_frequency.size();
    record.sysvel = Vector < Double > (record.num_lines, sysvel);

    t = main_table_(
        main_table_.col("SRCNAME") == record.name
            && main_table_.col("IFNO") == record.spw_id);
    time_column_.attach(t, "TIME");
    Vector < Double > time_list = time_column_.getColumn();
    Sort sorter;
    sorter.sortKey(time_list.data(), TpDouble);
    Vector < uInt > index_vector;
    uInt n = sorter.sort(index_vector, time_list.size());
    interval_column_.attach(t, "INTERVAL");
    constexpr double kDay2Sec = 86400.0;
    Double time_min = time_list[index_vector[0]] * kDay2Sec
        - 0.5 * interval_column_(index_vector[0]);
    Double time_max = time_list[index_vector[n - 1]] * kDay2Sec
        + 0.5 * interval_column_(index_vector[n - 1]);
    record.time = 0.5 * (time_min + time_max);
    record.interval = (time_max - time_min);
  }
  virtual void getProduct(Product *p) {

  }

private:
  ROScalarColumn<String> name_column_;
  ArrayColumn<Double> direction_column_;
  ArrayColumn<Double> proper_motion_column_;
  ROScalarColumn<Double> sysvel_column_;
  ROScalarColumn<uInt> molecule_id_column_;
  ROScalarColumn<uInt> ifno_column_;
  ROScalarColumn<Double> time_column_;
  ROScalarColumn<Double> interval_column_;
  Table molecules_table_;
  Vector<uInt> row_list_;
  std::map<String, Int> source_id_map_;
};

class ScantableSysCalIterator: public ScantableIteratorInterface {
public:
  typedef void * Product;
  ScantableSysCalIterator(Table const &table) :
      ScantableIteratorInterface(table) {
    initialize(1);
  }

  virtual ~ScantableSysCalIterator() {
  }

  void getEntry(SysCalRecord &record) {
  }
  virtual void getProduct(Product *p) {

  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SCANTABLEITERATOR_H_ */

/*
 * ScantableIterator.h
 *
 *  Created on: Jan 28, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SCANTABLEITERATOR_H_
#define SINGLEDISH_FILLER_SCANTABLEITERATOR_H_

#include <casacore/casa/Arrays/Vector.h>
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
    Table subtable = main_table_(main_table_.col("WEATHER_ID") == weather_id);
    ROScalarColumn < Double > column(subtable, "TIME");
    Vector < Double > time_list = column.getColumn();
    Double time_min, time_max;
    minMax(time_min, time_max, time_list);

    record.antenna_id = 0;
    record.time = 0.5 * (time_min + time_max);
    record.interval = time_max - time_min;
    record.temperature = temperature_column_(irow);
    record.pressure = pressure_column_(irow);
    record.rel_humidity = humidity_column_(irow);
    record.wind_speed = wind_speed_column_(irow);
    record.wind_direction = wind_direction_column_(irow);
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

    initialize (n);
    ifno_list_.resize(n);
    for (uInt i = 0; i < n; ++i) {
      ifno_list_[i] = ifno_list[index_vector[i]];
    }

    ROScalarColumn<uInt> freq_id_column(main_table_, "FREQ_ID");

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
    Table subtable = main_table_(main_table_.col("IFNO") == (uInt)spw_id, 1);
    ROScalarColumn < uInt > freq_id_column(subtable, "FREQ_ID");
    uInt freq_id = freq_id_column(0);
    Int jrow = -1;
    for (uInt i = 0; i < id_list_.size(); ++i) {
      if (id_list_[i] == freq_id) {
        jrow = (Int)i;
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

private:
  Table sub_table_;
  ScalarColumn<uInt> id_column_;
  ScalarColumn<Double> refpix_column_;
  ScalarColumn<Double> refval_column_;
  ScalarColumn<Double> increment_column_;
  Vector<uInt> ifno_list_;
  Vector<uInt> id_list_;
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SCANTABLEITERATOR_H_ */

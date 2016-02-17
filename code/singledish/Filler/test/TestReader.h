/*
 * TestReader.h
 *
 *  Created on: Jan 15, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_TEST_TESTREADER_H_
#define SINGLEDISH_FILLER_TEST_TESTREADER_H_

#include <gtest/gtest.h>

#include <singledish/Filler/ReaderInterface.h>

#include <map>

#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/BasicSL/Constants.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/casa/Quanta/MVPosition.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/measures/Measures/MeasConvert.h>

using namespace casa;
using namespace casacore;

class FixedNumberIteratorInterface {
public:
  FixedNumberIteratorInterface(size_t num_iter) :
      num_iter_(num_iter), current_iter_(0) {
  }
  virtual ~FixedNumberIteratorInterface() {
  }

  bool moreData() const {
    return current_iter_ < num_iter_;
  }

  void next() {
    ++current_iter_;
  }

protected:
  size_t const num_iter_;
  size_t current_iter_;
};

// Iterator for SOURCE table
class SourceIterator: public FixedNumberIteratorInterface {
public:
  SourceIterator() :
      FixedNumberIteratorInterface(4), source_id_(num_iter_), time_(num_iter_), interval_(
          num_iter_), spectral_window_id_(num_iter_), num_lines_(num_iter_), name_(
          num_iter_), calibration_group_(num_iter_), code_(num_iter_), direction_(
          num_iter_, Vector < Double > (2)), proper_motion_(num_iter_,
          Vector < Double > (2)), transition_(num_iter_), rest_frequency_(
          num_iter_), sysvel_(num_iter_) {
    POST_START;

    source_id_[0] = 0;
    source_id_[1] = 0;
    source_id_[2] = 1;
    source_id_[3] = 1;
    name_[0] = "M78";
    name_[1] = "M78";
    name_[2] = "HLTau";
    name_[3] = "HLTau";
    Time t(2016, 1, 1);
    time_ = t.modifiedJulianDay() * 86400.0;
    time_[2] += 86400.0;
    time_[3] = time_[2];
    interval_ = 3600.0;
    calibration_group_ = -1;
    code_ = "";
    Vector < Double > direction(2, 0.0);
    direction_[0] = direction;
    direction_[1] = direction;
    direction[0] += C::pi;
    direction[1] -= C::pi;
    direction_[2] = direction;
    direction_[3] = direction;
    Vector < Double > proper_motion(direction);
    proper_motion = 0.0;
    proper_motion_[0] = proper_motion;
    proper_motion_[1] = proper_motion;
    proper_motion[0] = 0.1;
    proper_motion[1] = -0.05;
    proper_motion_[2] = proper_motion;
    proper_motion_[3] = proper_motion;
    spectral_window_id_[0] = 0;
    spectral_window_id_[1] = 1;
    spectral_window_id_[2] = 0;
    spectral_window_id_[3] = 1;
    num_lines_[0] = 0;
    num_lines_[1] = 2;
    num_lines_[2] = 0;
    num_lines_[3] = 1;
    transition_[0] = Vector<String>();
    rest_frequency_[0] = Vector<Double>();
    sysvel_[0] = Vector<Double>();
    transition_[1] = Vector < String > (1, "12CO(1-0)");
    rest_frequency_[1] = Vector < Double > (1, 115.271203e9);
    sysvel_[1] = Vector < Double > (1, 10.0);
    transition_[2] = transition_[0];
    rest_frequency_[2] = rest_frequency_[0];
    sysvel_[2] = sysvel_[0];
    transition_[3] = Vector < String > (2);
    transition_[3][0] = "12CO(1-0)";
    transition_[3][1] = "13CO(1-0)";
    rest_frequency_[3] = Vector < Double > (2);
    rest_frequency_[3][0] = 115.271203e9;
    rest_frequency_[3][1] = 110.20137e9;
    sysvel_[3] = Vector < Double > (2, 15.0);

    POST_END;
  }
  virtual ~SourceIterator() {
    POST_START;POST_END;
  }

  void getEntry(SourceRecord &record) {
    POST_START;
    if (moreData()) {
      size_t i = current_iter_;
      record.source_id = source_id_[i];
      record.name = name_[i];
      record.time = time_[i];
      record.interval = interval_[i];
      record.calibration_group = calibration_group_[i];
      record.code = code_[i];
      Quantum<Vector<Double> > qdirection(direction_[i], "rad");
      MDirection direction(qdirection, MDirection::J2000);
      record.direction = direction;
      record.proper_motion = proper_motion_[i];
      record.spw_id = spectral_window_id_[i];
      record.num_lines = num_lines_[i];
      record.transition = transition_[i];
      record.rest_frequency = rest_frequency_[i];
      record.sysvel = sysvel_[i];
    }POST_END;
  }

private:
  Vector<Int> source_id_;
  Vector<Double> time_;
  Vector<Double> interval_;
  Vector<Int> spectral_window_id_;
  Vector<Int> num_lines_;
  Vector<String> name_;
  Vector<Int> calibration_group_;
  Vector<String> code_;
  Vector<Vector<Double> > direction_;
  Vector<Vector<Double> > proper_motion_;
  Vector<Vector<String> > transition_;
  Vector<Vector<Double> > rest_frequency_;
  Vector<Vector<Double> > sysvel_;
};

// Iterator for FIELD table
class FieldIterator: public FixedNumberIteratorInterface {
public:
  FieldIterator() :
      FixedNumberIteratorInterface(4), field_id_(num_iter_), time_(num_iter_), interval_(
          num_iter_), name_(num_iter_), source_name_(num_iter_), code_(
          num_iter_), direction_(num_iter_, Vector < Double > (2)), scan_rate_(
          num_iter_, Vector < Double > (2)) {
    POST_START;

    field_id_[0] = 1;
    field_id_[1] = 0;
    field_id_[2] = 3;
    field_id_[3] = 4;
    source_name_[0] = "M78";
    source_name_[1] = "M78";
    source_name_[2] = "HLTau";
    source_name_[3] = "HLTau";
    name_[0] = source_name_[0] + "N";
    name_[1] = source_name_[1] + "S";
    name_[2] = source_name_[2] + "_E";
    name_[3] = source_name_[3] + "_W";
    source_name_[0] = "M78";
    Time t(2016, 1, 1);
    time_ = t.modifiedJulianDay() * 86400.0;
    time_[1] = time_[0] + 86400.0;
    time_[2] = time_[1] + 86400.0;
    time_[3] = time_[2] + 86400.0;
    interval_ = 3600.0;
    code_[0] = "Northern area of " + source_name_[0];
    code_[1] = "Southern area of " + source_name_[1];
    code_[2] = "East area of " + source_name_[2];
    code_[3] = "West area of " + source_name_[3];
    Vector < Double > direction(2, 0.0);
    direction_[0] = direction;
    direction_[0][1] += 0.1;
    direction_[1] = direction;
    direction_[1][1] -= 0.1;
    direction[0] += C::pi;
    direction[1] -= C::pi;
    direction_[2] = direction;
    direction_[2][0] -= 0.1;
    direction_[3] = direction;
    direction_[3][0] += 0.1;
    Vector < Double > scan_rate(2, 0.0);
    scan_rate_[0] = scan_rate;
    scan_rate_[1] = scan_rate;
    scan_rate[0] = 0.1;
    scan_rate[1] = -0.05;
    scan_rate_[2] = scan_rate;
    scan_rate_[3] = scan_rate;

    POST_END;
  }
  virtual ~FieldIterator() {
    POST_START;POST_END;
  }

  void getEntry(FieldRecord &record) {
    if (moreData()) {
      size_t i = current_iter_;
      record.field_id = field_id_[i];
      record.name = name_[i];
      record.source_name = source_name_[i];
      record.time = time_[i];
      record.code = code_[i];
      Matrix < Double > direction(direction_[i]);
      if (scan_rate_[i].nelements() > 0 && anyNE(scan_rate_[i], 0.0)) {
        direction.resize(2, 2, True);
        direction.column(1) = scan_rate_[i];
      }
      record.direction = direction;
      record.frame = MDirection::GALACTIC;
    }
  }

private:
  Vector<Int> field_id_;
  Vector<Double> time_;
  Vector<Double> interval_;
  Vector<String> name_;
  Vector<String> source_name_;
  Vector<String> code_;
  Vector<Vector<Double> > direction_;
  Vector<Vector<Double> > scan_rate_;
};

class SpwIterator: public FixedNumberIteratorInterface {
public:
  SpwIterator() :
      FixedNumberIteratorInterface(4), spw_id_(num_iter_), name_(num_iter_), refpix_(
          num_iter_), refval_(num_iter_), increment_(num_iter_), num_chan_(
          num_iter_), meas_freq_ref_(num_iter_) {
    spw_id_[0] = 0;
    spw_id_[1] = 1;
    spw_id_[2] = 4;
    spw_id_[3] = 5;
    for (size_t i = 0; i < num_iter_; ++i) {
      name_[i] = "SPW" + String::toString(spw_id_[i]);
    }
    num_chan_[0] = 4;
    num_chan_[1] = 8;
    num_chan_[2] = 64;
    num_chan_[3] = 32;
    meas_freq_ref_[0] = 0; // REST
    meas_freq_ref_[1] = 1; // LSRK
    meas_freq_ref_[2] = 5; // TOPO
    meas_freq_ref_[3] = 4; // GEO
    refpix_[0] = 0.0;
    refpix_[1] = 256.0;
    refpix_[2] = 32.0;
    refpix_[3] = -1.0;
    increment_[0] = 1.0e8;
    increment_[1] = 1.0e8;
    increment_[2] = -1.0e8;
    increment_[3] = -1.0e8;
    refval_[0] = 1.0e11;
    refval_[1] = 1.256e11;
    refval_[2] = 1.1e11;
    refval_[3] = 1.133e11;
  }

  virtual ~SpwIterator() {
  }

  void getEntry(SpectralWindowRecord &record) {
    if (moreData()) {
      size_t i = current_iter_;
      record.spw_id = spw_id_[i];
      record.name = name_[i];
      record.num_chan = num_chan_[i];
      record.meas_freq_ref = meas_freq_ref_[i];
      record.refpix = refpix_[i];
      record.refval = refval_[i];
      record.increment = increment_[i];
    }
  }

private:
  Vector<Int> spw_id_;
  Vector<String> name_;
  Vector<Double> refpix_;
  Vector<Double> refval_;
  Vector<Double> increment_;
  Vector<Int> num_chan_;
  Vector<Int> meas_freq_ref_;
};

// Mock object for Reader
struct FloatDataStorage {
  size_t const kNumRow = 24;
  Double const time_[4] = { 4.0e9, 4.1e9, 4.2e9, 4.3e9 };
  Int const antenna_[2] = { 0, 1 };
  Int const spw_[2] = { 0, 1 };
  Int const num_pol_[2] = { 2, 1 };
  Int const num_chan_[2] = { 4, 8 };
  Int const field_[2] = { 0, 1 };
  Int const feed_[2] = { 0, 1 };
  Int const scan_[2] = { 0, 2 };
  Int const subscan_[2] = { 0, 1 };
  Double const ra_[2] = { 0.25, 0.28 };
  Double const dec_[2] = { -1.48, -1.49 };
  Double const dra_[2] = { 0.0, 0.01 };
  Double const ddec_[2] = { 0.0, -0.07 };
  String const intent_[2] = { "OBSERVE_TARGET#ON_SOURCE",
      "OBSERVE_TARGET#OFF_SOURCE" };
  String const field_name_[2] = { "MyField", "AnotherField" };
  String const source_name_[2] = { "M78", "HLTau" };
  Float const temperature_[4] = { 100.0f, 150.0f, 200.0f, 250.0f };
  Bool getData(size_t irow, DataRecord &record) {
    POST_START;

    size_t const n = kNumRow;

    if (n <= irow) {
      return False;
    }

//    size_t index = irow / (n / 4);
//    std::cout << "ROW " << irow << std::endl;
//    std::cout << "    index for time " << index << std::endl;
//    if (4ul <= index)
//      return false;;
//
    size_t index = irow / (n / 2);
//    std::cout << "ROW " << irow << std::endl;
//    std::cout << "    index for time, state, and scan" << index << std::endl;
    if (2ul <= index)
      return false;;
    record.time = time_[index];
    record.intent = intent_[index];
    record.scan = scan_[index];
    record.subscan = subscan_[index];

    index = (irow / (n / 4)) % 2;
//    std::cout << "    index for field " << index << std::endl;
    if (2ul <= index)
      return false;;
    record.field_id = field_[index];
//    record.field_name = field_name_[index]);
//    record.define("SOURCE_NAME", source_name_[index]);
    record.antenna_id = antenna_[index];
    record.direction(0, 0) = ra_[index];
    record.direction(1, 0) = dec_[index];
    record.direction(0, 1) = dra_[index];
    record.direction(1, 1) = ddec_[index];
    record.temperature = temperature_[index];
    record.pressure = 0.0f;
    record.rel_humidity = 0.0f;
    record.wind_speed = 0.0f;
    record.wind_direction = 0.0f;
    Int &antenna_id = record.antenna_id;

    index = (irow / (n / 8)) % 2;
//    std::cout << "    index for feed " << index << std::endl;
    if (2ul <= index)
      return false;;
    record.feed_id = feed_[index];
    Int &feed_id = record.feed_id;

    if (irow % 3 == 2) {
      index = 1;
    } else {
      index = 0;
    }
//    std::cout << "    index for spw " << index << std::endl;
    if (2ul <= index)
      return false;;
    size_t const num_chan = num_chan_[index];
    record.spw_id = spw_[index];
    Int num_chan_tsys = 0;
    if (feed_id == 0) {
      num_chan_tsys = 1;
    } else {
      num_chan_tsys = num_chan_[index];
    }
    Float tsys_value = (Float)(spw_[index] + antenna_id + 1);

    if (irow % 3 == 1) {
      index = 1;
    } else {
      index = 0;
    }
//    std::cout << "    index for pol " << index << std::endl;
    if (2ul <= index)
      return false;;
    record.polno = index;
    tsys_value *= (Float)(index + 1) * 100.0f;

    record.pol_type = "linear";

    Bool flag_row = False;
    record.setDataSize(num_chan);
    record.data = (Float) irow;
    record.flag = False;
    record.flag_row = flag_row;
    record.interval = 10.0;

    Vector < Float > tsys(num_chan_tsys, tsys_value);
    record.setTsysSize(num_chan_tsys);
    record.tsys = tsys_value;

    POST_END;

    return True;
  }
  bool isFloatData() {
    return true;
  }
  size_t nrow() const {
    return kNumRow;
  }
};

struct ComplexDataStorage {

};

template<class DataStorage>
class TestReader: public casa::ReaderInterface {
public:
  TestReader(std::string const &name) :
      casa::ReaderInterface(name), source_iterator_(nullptr), get_observation_row_(
          &::TestReader<DataStorage>::getObservationRowImpl), get_antenna_row_(
          &::TestReader<DataStorage>::getAntennaRowImpl), get_processor_row_(
          &::TestReader<DataStorage>::getProcessorRowImpl), get_source_row_(
          &::TestReader<DataStorage>::getSourceRowWithInitImpl), get_field_row_(
          &::TestReader<DataStorage>::getFieldRowWithInitImpl), get_spw_row_(
          &::TestReader<DataStorage>::getSpwRowWithInitImpl), storage_() {
    POST_START;POST_END;
  }

  ~TestReader() {
    POST_START;POST_END;
  }

  // get number of rows
  virtual size_t getNumberOfRows() {
    return storage_.nrow();
  }

  // query for data type
  virtual Bool isFloatData() {
    return True;
  }

  // to get OBSERVATION table
  virtual Bool getObservationRow(ObservationRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_observation_row_)(record);

    POST_END;
    return return_value;
  }

  // to get ANTENNA table
  virtual Bool getAntennaRow(AntennaRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_antenna_row_)(record);

    POST_END;

    return return_value;
  }

  // to get PROCESSOR table
  virtual Bool getProcessorRow(ProcessorRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_processor_row_)(record);

    POST_END;
    return return_value;
  }

  // to get SOURCE table
  virtual Bool getSourceRow(SourceRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_source_row_)(record);

    POST_END;

    return return_value;
  }

  // to get FIELD table
  virtual Bool getFieldRow(FieldRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_field_row_)(record);

    POST_END;

    return return_value;
  }

  // to get SPECTRAL WINDOW table
  virtual Bool getSpectralWindowRow(SpectralWindowRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_spw_row_)(record);

    POST_END;

    return return_value;
  }

  // for DataAccumulator
  virtual Bool getData(size_t irow, DataRecord &record) {
    POST_START;

    Bool status = storage_.getData(irow, record);

    main_record_[irow] = record;

    POST_END;
    return status;
  }

  // for testing
  Record observation_record_;
  Record antenna_record_;
  Record processor_record_;
  std::map<uInt, SourceRecord> source_record_;
  std::map<uInt, FieldRecord> field_record_;
  std::map<uInt, SpectralWindowRecord> spw_record_;
  std::map<uInt, DataRecord> main_record_;

protected:
  void initializeSpecific() {
    POST_START;POST_END;
  }

  void finalizeSpecific() {
    POST_START;POST_END;
  }

private:
  std::unique_ptr<SourceIterator> source_iterator_;
  std::unique_ptr<FieldIterator> field_iterator_;
  std::unique_ptr<SpwIterator> spw_iterator_;
  Bool (::TestReader<DataStorage>::*get_observation_row_)(ObservationRecord &);
  Bool (::TestReader<DataStorage>::*get_antenna_row_)(AntennaRecord &);
  Bool (::TestReader<DataStorage>::*get_processor_row_)(ProcessorRecord &);
  Bool (::TestReader<DataStorage>::*get_source_row_)(SourceRecord &);
  Bool (::TestReader<DataStorage>::*get_field_row_)(FieldRecord &);
  Bool (::TestReader<DataStorage>::*get_spw_row_)(SpectralWindowRecord &);
  DataStorage storage_;

  Bool getObservationRowImpl(ObservationRecord &record) {
    POST_START;

    String telescope_name = "MyTelescope";
    record.telescope_name = telescope_name;
    observation_record_.define("TELESCOPE_NAME", telescope_name);
    Vector < Double > time_range(2);
    Time t(2016, 1, 1);
    time_range[0] = t.modifiedJulianDay() * 86400.0; // sec
    t = t + 3600.0;
    time_range[1] = t.modifiedJulianDay();
    record.time_range = time_range;
    observation_record_.define("TIME_RANGE", time_range);
    String observer = "MyName";
    record.observer = observer;
    observation_record_.define("OBSERVER", observer);
    String schedule_type = "ALMA-SD";
    record.schedule_type = schedule_type;
    observation_record_.define("SCHEDULE_TYPE", schedule_type);
    String project = "MyProject";
    record.project = project;
    observation_record_.define("PROJECT", project);
    t = t + 86400.0;
    Double release_date = t.modifiedJulianDay();
    record.release_date = release_date;
    observation_record_.define("RELEASE_DATE", release_date);
    Vector < String > strvec(1, "");
    record.log = strvec;
    observation_record_.define("LOG", strvec);
    strvec[0] = "NextSchedule";
    record.schedule = strvec;
    observation_record_.define("SCHEDULE", strvec);

    // redirect function pointer to noMoreRowImpl
    get_observation_row_ = &::TestReader < DataStorage
        > ::noMoreRowImplTemplate<ObservationRecord>;

    POST_END;

    return True;
  }

  Bool getAntennaRowImpl(AntennaRecord &record) {
    POST_START;

    String name = "PMXY";
    record.name = name;
    antenna_record_.define("NAME", name);
    String station = "A00";
    record.station = station;
    antenna_record_.define("STATION", station);
    String type = "GROUND-BASED";
    record.type = type;
    antenna_record_.define("TYPE", type);
    String mount = "ALT-AZ";
    record.mount = mount;
    antenna_record_.define("MOUNT", mount);
    MVPosition mvposition(Quantity(5056.8, "m"),
        Quantity(-1.1825465955049892, "rad"),
        Quantity(-0.4018251640113072, "rad"));
    MPosition position(mvposition, MPosition::WGS84);
    Quantum < Vector<Double> > position_val = MPosition::Convert(position,
        MPosition::ITRF)().get("m");
    MPosition position_itrf(position_val, MPosition::ITRF);
    record.position = position_itrf;
    antenna_record_.define("POSITION", position_val.getValue());
    MVPosition mvoffset(Quantity(-50.0, "m"), Quantity(0.01, "rad"),
        Quantity(-0.01, "rad"));
    MPosition offset(mvoffset, MPosition::ITRF);
    record.offset = offset;
    Quantum<Vector<Double> > offset_val = offset.get("m");
    antenna_record_.define("OFFSET", offset_val.getValue());
    Double dish_diameter = 12.0;
    record.dish_diameter = dish_diameter;
    antenna_record_.define("DISH_DIAMETER", dish_diameter);

    get_antenna_row_ = &::TestReader < DataStorage
        > ::noMoreRowImplTemplate<AntennaRecord>;

    POST_END;

    return True;
  }

  Bool getProcessorRowImpl(ProcessorRecord &record) {
    POST_START;

    String type = "CORRELATOR";
    record.type = type;
    processor_record_.define("TYPE", type);
    String sub_type = "ACA";
    record.sub_type = sub_type;
    processor_record_.define("SUB_TYPE", sub_type);
    Int type_id = -1;
    record.type_id = type_id;
    processor_record_.define("TYPE_ID", type_id);
    Int mode_id = 37;
    record.mode_id = mode_id;
    processor_record_.define("MODE_ID", mode_id);

    // redirect function pointer to noMoreRowImpl
    get_processor_row_ = &::TestReader < DataStorage
        > ::noMoreRowImplTemplate<ProcessorRecord>;

    POST_END;
    return True;
  }

  Bool getSourceRowWithInitImpl(SourceRecord &record) {
    POST_START;

    source_iterator_.reset(new ::SourceIterator());
    get_source_row_ = &::TestReader<DataStorage>::getSourceRowImpl;

    Bool more_rows = getSourceRowImpl(record);

    POST_END;

    return more_rows;
  }

  Bool getSourceRowImpl(SourceRecord &record) {
    return getRowImplTemplate2(source_iterator_, source_record_,
        get_source_row_, record);
  }

  Bool getFieldRowWithInitImpl(FieldRecord &record) {
    POST_START;

    field_iterator_.reset(new ::FieldIterator());
    get_field_row_ = &::TestReader<DataStorage>::getFieldRowImpl;

    Bool more_rows = getFieldRowImpl(record);

    POST_END;

    return more_rows;
  }

  Bool getFieldRowImpl(FieldRecord &record) {
    return getRowImplTemplate2(field_iterator_, field_record_, get_field_row_,
        record);
  }

  Bool getSpwRowWithInitImpl(SpectralWindowRecord &record) {
    POST_START;

    spw_iterator_.reset(new ::SpwIterator());
    get_spw_row_ = &::TestReader<DataStorage>::getSpwRowImpl;

    Bool more_rows = getSpwRowImpl(record);

    POST_END;

    return more_rows;
  }

  Bool getSpwRowImpl(SpectralWindowRecord &record) {
    return getRowImplTemplate2(spw_iterator_, spw_record_, get_spw_row_, record);
  }

  template<class _Iterator, class _InternalRecord, class _Func, class _Record>
  Bool getRowImplTemplate2(_Iterator &iter, _InternalRecord &internal_record,
      _Func &func, _Record &record) {
    POST_START;

    assert(iter);

    bool more_rows = iter->moreData();

    if (more_rows) {
      iter->getEntry(record);
      iter->next();

      // keep record contents
      uInt i = internal_record.size();
      internal_record[i] = record;
    } else {
      iter.reset(nullptr);
      func = &::TestReader < DataStorage > ::noMoreRowImplTemplate<_Record>;
    }

    POST_END;

    return more_rows;
  }

  template<class _Record>
  Bool noMoreRowImplTemplate(_Record &) {
    POST_START;POST_END;

    return False;
  }
};

#endif /* SINGLEDISH_FILLER_TEST_TESTREADER_H_ */

/*
 * Scantable2MSFiller.h
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_
#define SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_

#include <string>
#include <memory>
#include <map>

#include <singledish/Filler/DataAccumulator.h>
#include <singledish/Filler/SysCalRecord.h>

#include <casacore/casa/OS/File.h>
#include <casacore/casa/OS/Path.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSMainColumns.h>
#include <casacore/ms/MeasurementSets/MSSysCalColumns.h>
#include <casacore/tables/Tables/TableRow.h>
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/tables/Tables/ScalarColumn.h>

#define SINGLEDISHMSFILLER_DEBUG
#ifdef SINGLEDISHMSFILLER_DEBUG
#define POST_START std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl
#define POST_END std::cout << "End " << __PRETTY_FUNCTION__ << std::endl
#else
#define POST_START
#define POST_END
#endif

namespace casa { //# NAMESPACE CASA - BEGIN

class DataAccumulator;

template<typename Reader>
class SingleDishMSFiller {
public:
  SingleDishMSFiller(std::string const &name) :
      ms_(), ms_columns_(), reader_(new Reader(name)), is_float_(false), data_key_(), reference_feed_(
          -1), pointing_time_(), pointing_time_max_(), pointing_time_min_(), num_pointing_time_(), syscal_list_() {
  }

  ~SingleDishMSFiller() {
  }

  // access to reader object
  Reader const &getReader() const {
    return *reader_;
  }

  std::string const &getDataName() const {
    return reader_->getName();
  }

  // top level method to fill MS by reading input data
  void fill() {
    POST_START;

    // initialization
    initialize();

    // Fill tables that can be processed prior to main loop
    fillPreProcessTables();

    // main loop
    fillMain();

    // Fill tables that must be processed after main loop
    fillPostProcessTables();

    // finalization
    finalize();

    POST_END;
  }

  // save
  void save(std::string const &name);

private:
  // initialization
  void initialize() {
    POST_START;

    // initialize reader
    reader_->initialize();

    // query if the data is complex or float
    is_float_ = reader_->isFloatData();
    if (is_float_) {
      std::cout << "data column is FLOAT_DATA" << std::endl;
      data_key_ = "FLOAT_DATA";
    } else {
      std::cout << "data column is DATA" << std::endl;
      data_key_ = "DATA";
    }

    // setup MS
    setupMS();
    std::cout << "ms_->name() \"" << ms_->tableName() << "\"" << std::endl;

    // frame information
    MDirection::Types direction_frame = reader_->getDirectionFrame();
    auto mytable = ms_->pointing();
    ArrayColumn<Double> direction_column(mytable, "DIRECTION");
    TableRecord &record = direction_column.rwKeywordSet();
    Record meas_info = record.asRecord("MEASINFO");
    if (meas_info.dataType("Ref") == TpInt) {
      std::cout << "MEASINFO::Ref is integer" << std::endl;
      meas_info.define("Ref", direction_frame);
    } else {
      std::cout << "MEASINFO::Ref seems to be string" << std::endl;
      String ref_string = MDirection::showType(direction_frame);
      //ref_string = "GALACTIC";
      std::cout << "setting \"" << ref_string << "\" to direction frame"
          << std::endl;
      meas_info.define("Ref", ref_string);
    }
    record.defineRecord("MEASINFO", meas_info);

    POST_END;
  }

// finalization
  void finalize() {
    POST_START;

    // finalize reader
    reader_->finalize();

    POST_END;
  }

// setup MS as Scratch table
// The table will be non-Scratch when it is saved
  void setupMS();

// fill tables that can be processed prior to main loop
  void fillPreProcessTables() {
    POST_START;

    // fill OBSERVATION table
    fillObservation();

    // fill ANTENNA table
    fillAntenna();

    // fill PROCESSOR table
    fillProcessor();

    // fill SOURCE table
    fillSource();

    // fill FIELD table
    fillField();

    // fill SPECTRAL_WINDOW table
    fillSpectralWindow();

    // fill SYSCAL table
//    fillSysCal();

    // fill WEATHER table
    fillWeather();

    POST_END;
  }

// fill tables that must be processed after main loop
  void fillPostProcessTables() {
    POST_START;

    // fill HISTORY table
    fillHistory();

    // flush POINTING entry
    sortPointing();

    POST_END;
  }

// fill MAIN table
  void fillMain() {
    POST_START;

    size_t nrow = reader_->getNumberOfRows();
    DataAccumulator accumulator;
    TableRecord record;
    TableRecord previous_record;
    std::cout << "nrow = " << nrow << std::endl;
    for (size_t irow = 0; irow < nrow; ++irow) {
      Bool status = reader_->getData(irow, record);
      std::cout << "irow " << irow << " status " << status << std::endl;
      std::cout << "   TIME=" << record.asDouble("TIME") << " INTERVAL="
          << record.asDouble("INTERVAL") << std::endl;
      if (status) {
        Bool is_ready = accumulator.queryForGet(record);
        std::cout << "is_ready " << is_ready << std::endl;
        if (is_ready) {
          flush(previous_record, accumulator);
        }
        accumulator.accumulate(record);
        previous_record.merge(record, TableRecord::OverwriteDuplicates);
      }
    }

    flush(record, accumulator);

    POST_END;
  }

  void flush(TableRecord const &main_record, DataAccumulator &accumulator) {
    POST_START;

    std::cout << "main record" << std::endl;
    main_record.print(std::cout);

    size_t nchunk = accumulator.getNumberOfChunks();
    std::cout << "nchunk = " << nchunk << std::endl;

    if (nchunk == 0) {
      return;
    }

    //Int scan = main_record.asInt("SCAN");
    //Int subscan = main_record.asInt("SUBSCAN");
    //String obs_mode = main_record.asString("INTENT");
//    String field_name = main_record.asString("FIELD_NAME");
//    String source_name = main_record.asString("SOURCE_NAME");
    //Int field_id = main_record.asInt("FIELD_ID");
    Double time = main_record.asDouble("TIME");
    //Double interval = main_record.asDouble("INTERVAL");

    for (size_t ichunk = 0; ichunk < nchunk; ++ichunk) {
      TableRecord data_record;
      Bool status = accumulator.get(ichunk, data_record);
      std::cout << "ichunk " << ichunk << " status " << status << std::endl;
      if (status) {
        Int antenna_id = data_record.asInt("ANTENNA_ID");
        Int spw_id = data_record.asInt("SPECTRAL_WINDOW_ID");
        Int feed_id = data_record.asInt("FEED_ID");
        Int field_id = data_record.asInt("FIELD_ID");
        std::cout << "spw " << spw_id << std::endl;
        Int scan = data_record.asInt("SCAN");
        Int subscan = data_record.asInt("SUBSCAN");
        String pol_type = data_record.asString("POL_TYPE");
        String obs_mode = data_record.asString("INTENT");
        Int num_pol = data_record.asInt("NUM_POL");
        Vector < Int > corr_type = data_record.asArrayInt("CORR_TYPE");
        Int polarization_id = updatePolarization(corr_type, num_pol);
        updateFeed(feed_id, spw_id, pol_type);
        Int data_desc_id = updateDataDescription(polarization_id, spw_id);
        Int state_id = updateState(subscan, obs_mode);
        Matrix < Double > direction = data_record.asArrayDouble("DIRECTION");
        Double interval = data_record.asDouble("INTERVAL");
        std::cout << "BEFORE UPDATE: TIME=" << time << " INTERVAL=" << interval
            << std::endl;

        // updatePointing must be called after updateFeed
        updatePointing(antenna_id, feed_id, time, interval, direction);

        updateSysCal(antenna_id, feed_id, spw_id, time, interval, data_record);

        updateMain(antenna_id, field_id, feed_id, data_desc_id, state_id, scan,
            time,/*main_record,*/data_record);
      }
    }
    accumulator.clear();

    POST_START;
  }

  void sortPointing();

// Fill subtables
// fill ANTENNA table
  void fillAntenna();

// fill OBSERVATION table
  void fillObservation();

// fill PROCESSOR table
  void fillProcessor();

// fill SOURCE table
  void fillSource();

// fill SOURCE table
  void fillField();

// fill SPECTRAL_WINDOW table
  void fillSpectralWindow();

// fill SYSCAL table
  void fillSysCal();

// fill WEATHER table
  void fillWeather();

// fill HISTORY table
  void fillHistory() {
    POST_START;

    // HISTORY table should be filled by upper-level
    // application command (e.g. importscantable)
//    ms_->history().addRow(1, True);
//    Vector<String> cols(2);
//    cols[0] = "APP_PARAMS";
//    cols[1] = "CLI_COMMAND";
//    TableRow row(ms_->history(), cols, True);
//    // TODO: fill HISTORY row here
//    TableRecord record = row.record();
//    record.print(std::cout);
//    row.put(0, record);

    POST_END;
  }

//  // update Tables
//  // update SOURCE table
//  // @param[in] sourceSpec source specification
//  // @return source id
//  Int updateSource(Record sourceSpec);
//
//  // update FIELD table
//  // @param[in] sourceId source id
//  // @param[in] fieldSpec field specification except ids
//  // @return field id
//  Int updateField(Int sourceId, Record fieldSpec);
//
// update POLARIZATION table
// @param[in] corr_type polarization type list
// @param[in] num_pol number of polarization components
// @return polarization id
  Int updatePolarization(Vector<Int> const &corr_type, Int const &num_pol);

// update DATA_DESCRIPTION table
// @param[in] polarization_id polarization id
// @param[in] spw_id spectral window id
// @return data description id
  Int updateDataDescription(Int const &polarization_id, Int const &spw_id);

// update STATE table
// @param[in] subscan subscan number
// @param[in] obs_mode observing mode string
// @return state id
  Int updateState(Int const &subscan, String const &obs_mode);

// update FEED table
// @param[in] feed_id feed ID
// @param[in] spw_id spectral window ID
// @param[in] pol_type polarization type
// @return feed row number
  Int updateFeed(Int const &feed_id, Int const &spw_id, String const &pol_type);

// update POINTING table
// @param[in] antenna_id antenna id
// @param[in] time time stamp
// @param[in] direction pointing direction
  Int updatePointing(Int const &antenna_id, Int const &feed_id,
      Double const &time, Double const &interval,
      Matrix<Double> const &direction);

  Int updateSysCal(Int const &antenna_id, Int const &feed_id, Int const &spw_id,
      Double const &time, Double const &interval,
      TableRecord const &data_record) {
    POST_START;

    //SysCalTableRecord table_record(*ms_, antenna_id, feed_id, spw_id, record);
    SysCalRecord record;
    record.antenna_id = antenna_id;
    record.feed_id = feed_id;
    record.spw_id = spw_id;
    record.time = time;
    record.interval = interval;
    if (data_record.isDefined("TCAL")) {
      std::cout << "TCAL defined: " << data_record.asArrayFloat("TCAL") << std::endl;
      if (data_record.shape(data_key_) == data_record.shape("TCAL")) {
        record.tcal_spectrum.assign(data_record.asArrayFloat("TCAL"));
      } else {
        Matrix<Float> tcal = data_record.asArrayFloat("TCAL");
        if (!tcal.empty()) {
          record.tcal.assign(tcal.column(0));
        }
      }
    }
    if (data_record.isDefined("TSYS")) {
      std::cout << "TSYS defined: " << data_record.asArrayFloat("TSYS") << std::endl;
      if (data_record.shape(data_key_) == data_record.shape("TSYS")) {
        record.tsys_spectrum.assign(data_record.asArrayFloat("TSYS"));
      } else {
        Matrix<Float> tsys = data_record.asArrayFloat("TSYS");
        if (!tsys.empty()) {
          record.tsys.assign(tsys.column(0));
        }
      }
    }
    std::cout << "ANTENNA " << antenna_id << " FEED " << feed_id << " SPW "
        << spw_id << " TIME " << time << " TSYS " << record.tsys << std::endl;
    auto mytable = ms_->sysCal();
    MSSysCalColumns mycolumns(mytable);
    // set attr for SysCalRecord
    auto pos = std::find(syscal_list_.begin(), syscal_list_.end(), record);
    if (pos == syscal_list_.end()) {
      std::cout << "add new entry" << std::endl;
      uInt irow = mytable.nrow();
      mytable.addRow(1, True);
      record.fill(irow, mycolumns);
      syscal_list_.push_back(SysCalTableRecord(ms_.get(), irow, record));
    } else {
      auto irow = std::distance(syscal_list_.begin(), pos);
      std::cout << "update row " << irow << std::endl;
      updateSysCal(mycolumns, irow, record);
    }

    POST_END;
  }

  void updateSysCal(MSSysCalColumns &columns, uInt irow,
      SysCalRecord const &record) {
    // only update timestamp and interval
    Double time_org = columns.time()(irow);
    Double interval_org = columns.interval()(irow);

    Double time_min_org = time_org - interval_org / 2.0;
    Double time_max_org = time_org + interval_org / 2.0;

    Double time_min_in = record.time - record.interval / 2.0;
    Double time_max_in = record.time + record.interval / 2.0;

    Double time_min_new = min(time_min_org, time_min_in);
    Double time_max_new = max(time_max_org, time_max_in);

    std::cout << "time_org, interval_org=" << time_org << "," << interval_org << std::endl;
    std::cout << "time_min_in, time_max_in=" << time_min_in << "," << time_max_in << std::endl;
    std::cout << "time_min_new, time_max_new=" << time_min_new << "," << time_max_new << std::endl;

    if (time_min_new != time_min_org || time_max_new != time_max_org) {
      Double time_new = (time_min_new + time_max_new) / 2.0;
      Double interval_new = time_max_new - time_min_new;
      std::cout << "update timestamp: time, interval=" << time_new << "," << interval_new << std::endl;
      columns.time().put(irow, time_new);
      columns.interval().put(irow, interval_new);
    }
  }

// update MAIN table
// @param[in] fieldId field id
// @param[in] feedId feed id
// @param[in] dataDescriptionId data description id
// @param[in] stateId state id
// @param[in] mainSpec main table row specification except id
  void updateMain(Int const &antenna_id, Int field_id, Int feedId,
      Int dataDescriptionId, Int stateId, Int const &scan_number,
      Double const &time, /*TableRecord const &mainSpec,*/
      TableRecord const &dataRecord) {
    POST_START;

    // constant stuff
    Vector<Double> const uvw(3, 0.0);
    Array<Bool> flagCategory(IPosition(3, 0, 0, 0));

//    auto mytable = *ms_;
//    TableRow row(mytable);
//    TableRecord record = row.record();
    // target row id
    uInt irow = ms_->nrow();

    // add new row
    ms_->addRow(1, True);

    ms_columns_->uvw().put(irow, uvw);
    ms_columns_->flagCategory().put(irow, flagCategory);
    ms_columns_->antenna1().put(irow, antenna_id);
    ms_columns_->antenna2().put(irow, antenna_id);
    ms_columns_->fieldId().put(irow, field_id);
    ms_columns_->feed1().put(irow, feedId);
    ms_columns_->feed2().put(irow, feedId);
    ms_columns_->dataDescId().put(irow, dataDescriptionId);
    ms_columns_->stateId().put(irow, stateId);
    ms_columns_->scanNumber().put(irow, scan_number);
    //Double time = mainSpec.asDouble("TIME");
    ms_columns_->time().put(irow, time);
    ms_columns_->timeCentroid().put(irow, time);
    Double interval = dataRecord.asDouble("INTERVAL");
    ms_columns_->interval().put(irow, interval);
    ms_columns_->exposure().put(irow, interval);
//    record.define("UVW", uvw);
//    record.define("FLAG_CATEGORY", flagCategory);
//    record.define("ANTENNA1", antenna_id);
//    record.define("ANTENNA2", antenna_id);
//    record.define("FIELD_ID", fieldId);
//    record.define("FEED1", feedId);
//    record.define("FEED2", feedId);
//    record.define("DATA_DESC_ID", dataDescriptionId);
//    record.define("STATE_ID", stateId);
//    Double time = mainSpec.asDouble("TIME");
//    record.define("TIME", time);
//    record.define("TIME_CENTROID", time);
//    Double interval = mainSpec.asDouble("INTERVAL");
//    record.define("INTERVAL", interval);
//    record.define("EXPOSURE", interval);

    if (is_float_) {
      std::cout << "Fill Float data" << std::endl;
      Matrix < Float > floatData;
      if (dataRecord.isDefined("FLOAT_DATA")) {
        //record.define("FLOAT_DATA", dataRecord.asArrayFloat("FLOAT_DATA"));
        floatData.assign(dataRecord.asArrayFloat("FLOAT_DATA"));
      } else if (dataRecord.isDefined("DATA")) {
        //record.define("FLOAT_DATA", real(dataRecord.asArrayComplex("DATA")));
        floatData.assign(real(dataRecord.asArrayComplex("DATA")));
      }
      ms_columns_->floatData().put(irow, floatData);
    } else {
      std::cout << "Fill Complex data" << std::endl;
      Matrix < Complex > data;
      if (dataRecord.isDefined("FLOAT_DATA")) {
        std::cout << "FLOAT_DATA" << std::endl;
//        record.define("DATA",
//            makeComplex(dataRecord.asArrayFloat("FLOAT_DATA"),
//                Matrix < Float > (dataRecord.shape("FLOAT_DATA"), 0.0f)));
        data.assign(
            makeComplex(dataRecord.asArrayFloat("FLOAT_DATA"),
                Matrix < Float > (dataRecord.shape("FLOAT_DATA"), 0.0f)));
      } else if (dataRecord.isDefined("DATA")) {
        std::cout << "DATA" << std::endl;
//        record.define("DATA", dataRecord.asArrayComplex("DATA"));
        data.assign(dataRecord.asArrayComplex("DATA"));
      }
      ms_columns_->data().put(irow, data);
    }

    ms_columns_->flag().put(irow, dataRecord.asArrayBool("FLAG"));
    ms_columns_->flagRow().put(irow, dataRecord.asBool("FLAG_ROW"));
    ms_columns_->sigma().put(irow, dataRecord.asArrayFloat("SIGMA"));
    ms_columns_->weight().put(irow, dataRecord.asArrayFloat("WEIGHT"));
//    record.define("FLAG", dataRecord.asArrayBool("FLAG"));
//    record.define("FLAG_ROW", dataRecord.asBool("FLAG_ROW"));
//    record.define("SIGMA", dataRecord.asArrayFloat("SIGMA"));
//    record.define("WEIGHT", dataRecord.asArrayFloat("WEIGHT"));

//    uInt irow = mytable.nrow();
//    mytable.addRow(1, True);
//    row.put(irow, record);

    POST_END;
  }

//  std::string const &ms_name_;
  std::unique_ptr<casacore::MeasurementSet> ms_;
  std::unique_ptr<MSMainColumns> ms_columns_;
  std::unique_ptr<Reader> reader_;
  bool is_float_;
  String data_key_;

// for POINTING table
  Int reference_feed_;
  std::map<Int, Vector<Double>> pointing_time_;
  std::map<Int, Double> pointing_time_max_;
  std::map<Int, Double> pointing_time_min_;
  Vector<Int> num_pointing_time_;

  // for SYSCAL table
  std::vector<SysCalTableRecord> syscal_list_;
}
;

} //# NAMESPACE CASA - END

#include <singledish/Filler/SingleDishMSFiller.tcc>

#endif /* SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_ */

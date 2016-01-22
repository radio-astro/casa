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

#include <singledish/Filler/DataAccumulator.h>

#include <casacore/casa/OS/File.h>
#include <casacore/casa/OS/Path.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSMainColumns.h>
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
      ms_(), ms_columns_(), reader_(new Reader(name)), is_float_(false) {
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

    // setup MS
    setupMS();
    std::cout << "ms_->name() \"" << ms_->tableName() << "\"" << std::endl;

    // initialize reader
    reader_->initialize();

    // query if the data is complex or float
    is_float_ = reader_->isFloatData();

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

    // fill SPECTRAL_WINDOW table
    fillSpectralWindow();

    // fill SYSCAL table
    fillSyscal();

    // fill WEATHER table
    fillWeather();

    POST_END;
  }

  // fill tables that must be processed after main loop
  void fillPostProcessTables() {
    POST_START;

    // fill HISTORY table
    fillHistory();

    POST_END;
  }

  // fill MAIN table
  void fillMain() {
    POST_START;

    size_t nrow = reader_->getNumberOfRows();
    DataAccumulator accumulator;
    TableRecord record;
    std::cout << "nrow = " << nrow << std::endl;
    for (size_t irow = 0; irow < nrow; ++irow) {
      Bool status = reader_->getData(irow, record);
      std::cout << "irow " << irow << " status " << status << std::endl;
      if (status) {
        Bool is_ready = accumulator.queryForGet(record);
        std::cout << "is_ready " << is_ready << std::endl;
        if (is_ready) {
          flush(record, accumulator);
        }
      }
      accumulator.accumulate(record);
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
    for (size_t ichunk = 0; ichunk < nchunk; ++ichunk) {
//      Int sourceId = updateSource(main_record);
//      Int fieldId = updateField(sourceId, main_record);
//      Int polarizationId = updatePolarization(
//          accumulator.getPolType(ichunk), accumulator.getNumPol(ichunk));
//      Int spectralWindowId = updateSpectralWindow(main_record);
//      Int dataDescriptionId = updateDataDescription(polarizationId,
//          spectralWindowId);
//      Int stateId = updateState(main_record);
//      Int feedId = updateFeed(main_record);
//      updatePointing(main_record);
      TableRecord data_record;
      Bool status = accumulator.get(ichunk, data_record);
      std::cout << "ichunk " << ichunk << " status " << status << std::endl;
      if (status) {
        Int fieldId = 0;
        Int feedId = 0;
        Int dataDescriptionId = 0;
        Int stateId = 0;
        updateMain(fieldId, feedId, dataDescriptionId, stateId, main_record,
            data_record);
      }
    }
    accumulator.clear();

    POST_START;
  }

  // Fill subtables
  // fill ANTENNA table
  void fillAntenna();

  // fill OBSERVATION table
  void fillObservation();

  // fill PROCESSOR table
  void fillProcessor();

  // fill SOURCE table
  void fillSource();

  // fill SPECTRAL_WINDOW table
  void fillSpectralWindow();

  // fill SYSCAL table
  void fillSyscal();

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
//  // update POLARIZATION table
//  // @param[in] polarizationSpec polarization specification
//  // @return polarization id
//  Int updatePolarization(Record polarizationSpec);
//
//  // update SPECTRAL_WINDOW table
//  // @param[in] spectralWindowSpec spectral window specification
//  // @return spectral window id
//  Int updateSpectralWindow(Record spectralWindowSpec);
//
//  // update DATA_DESCRIPTION table
//  // @param[in] polarizationId polarization id
//  // @param[in] spectralWindowId spectral window id
//  // @return data description id
//  Int updateDataDescription(Int polarizationId, Int spectralWindowId);
//
//  // update STATE table
//  // @param[in] stateSpec state specification
//  // @return state id
//  Int updateState(Record stateSpec);
//
//  // update FEED table
//  // @param[in] feedSpec feed specification
//  // @return feed id
//  Int updateFeed(Record feedSpec);
//
//  // update POINTING table
//  // @param[in] pointingSpec pointing specification
//  void updatePointing(Record pointingSpec);
//
  // update MAIN table
  // @param[in] fieldId field id
  // @param[in] feedId feed id
  // @param[in] dataDescriptionId data description id
  // @param[in] stateId state id
  // @param[in] mainSpec main table row specification except id
  void updateMain(Int fieldId, Int feedId, Int dataDescriptionId, Int stateId,
      TableRecord const &mainSpec, TableRecord const &dataRecord) {
    // constant stuff
    Vector<Double> const uvw(3, 0.0);
    constexpr Int antennaId = 0;
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
    ms_columns_->antenna1().put(irow, antennaId);
    ms_columns_->antenna2().put(irow, antennaId);
    ms_columns_->fieldId().put(irow, fieldId);
    ms_columns_->feed1().put(irow, feedId);
    ms_columns_->feed2().put(irow, feedId);
    ms_columns_->dataDescId().put(irow, dataDescriptionId);
    ms_columns_->stateId().put(irow, stateId);
    Double time = mainSpec.asDouble("TIME");
    ms_columns_->time().put(irow, time);
    ms_columns_->timeCentroid().put(irow, time);
    Double interval = mainSpec.asDouble("INTERVAL");
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
      Matrix < Complex > data;
      if (dataRecord.isDefined("FLOAT_DATA")) {
//        record.define("DATA",
//            makeComplex(dataRecord.asArrayFloat("FLOAT_DATA"),
//                Matrix < Float > (dataRecord.shape("FLOAT_DATA"), 0.0f)));
        data.assign(
            makeComplex(dataRecord.asArrayFloat("FLOAT_DATA"),
                Matrix < Float > (dataRecord.shape("FLOAT_DATA"), 0.0f)));
      } else if (dataRecord.isDefined("DATA")) {
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
  }

//  std::string const &ms_name_;
  std::unique_ptr<casacore::MeasurementSet> ms_;
  std::unique_ptr<MSMainColumns> ms_columns_;
  std::unique_ptr<Reader> reader_;
  bool is_float_;
}
;

} //# NAMESPACE CASA - END

#include <singledish/Filler/SingleDishMSFiller.tcc>

#endif /* SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_ */

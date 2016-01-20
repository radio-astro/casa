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
      ms_(), reader_(new Reader(name)) {
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
    Bool is_ready = False;
    TableRecord record;
    for (size_t irow = 0; irow < nrow; ++irow) {
      Bool status = reader_->getData(irow, record);
      if (status) {
        Bool is_ready = accumulator.queryForGet(record);
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
//    size_t nchunk = accumulator.getNumberOfChunks();
//    for (size_t ichunk = 0; ichunk < nchunk; ++ichunk) {
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
//      TableRecord data_record;
//      Bool status = accumulator.get(ichunk, data_record);
//      updateMain(fieldId, feedId, dataDescriptionId, stateId,
//          data_record);
//    }
//    accumulator.clear();
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
//  // update MAIN table
//  // @param[in] fieldId field id
//  // @param[in] feedId feed id
//  // @param[in] dataDescriptionId data description id
//  // @param[in] stateId state id
//  // @param[in] mainSpec main table row specification except id
//  void updateMain(Int fieldId, Int feedId, Int dataDescriptionId,
//                  Int stateId, Record mainSpec);

//  std::string const &ms_name_;
  std::unique_ptr<casacore::MeasurementSet> ms_;
  std::unique_ptr<Reader> reader_;
}
;

} //# NAMESPACE CASA - END

#include <singledish/Filler/SingleDishMSFiller.tcc>

#endif /* SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_ */

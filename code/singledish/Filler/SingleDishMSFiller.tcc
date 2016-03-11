/*
 * SingleDishMSFiller.tcc
 *
 *  Created on: Jan 8, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SINGLEDISHMSFILLER_TCC_
#define SINGLEDISH_FILLER_SINGLEDISHMSFILLER_TCC_

#include <singledish/Filler/SingleDishMSFiller.h>
#include <singledish/Filler/FillerUtil.h>
#include <singledish/Filler/AntennaRecord.h>
#include <singledish/Filler/FieldRecord.h>
#include <singledish/Filler/ObservationRecord.h>
#include <singledish/Filler/ProcessorRecord.h>
#include <singledish/Filler/SourceRecord.h>
#include <singledish/Filler/SpectralWindowRecord.h>
#include <singledish/Filler/SysCalRecord.h>
#include <singledish/Filler/WeatherRecord.h>

#include <iostream>
#include <map>
#include <memory>

#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/casa/Utilities/Sort.h>

#include<casacore/measures/Measures/Stokes.h>

#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSDataDescColumns.h>
#include <casacore/ms/MeasurementSets/MSPolColumns.h>
#include <casacore/ms/MeasurementSets/MSPointingColumns.h>
#include <casacore/ms/MeasurementSets/MSStateColumns.h>
#include <casacore/ms/MeasurementSets/MSFeedColumns.h>
#include <casacore/ms/MeasurementSets/MSSourceColumns.h>

#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/ScalarColumn.h>
#include <casacore/tables/Tables/SetupNewTab.h>

using namespace casacore;
using namespace sdfiller;

#define ARRAY_BLOCK_SIZE 1024

namespace {

template<class _Table, class _Record, class _Reader>
inline void fillTable(_Table &table, _Record &record, _Reader const &reader) {
  POST_START;

  typename _Record::AssociatingColumns columns(table);

  size_t irow = 0;
  record.clear();
  for (Bool more_rows = reader(record); more_rows == True;
      more_rows = reader(record)) {
    record.add(table, columns);
    record.fill(irow, columns);
    ++irow;
    record.clear();
  }

  POST_END;
}

template<class _Table, class _Columns, class _Comparer, class _Updater>
inline Int updateTable(_Table &mytable, _Columns &mycolumns,
    _Comparer const &comparer, _Updater const &updater) {
  POST_START;

  Int id = -1;
  if (mycolumns.nrow() >= (uInt) INT_MAX) {
    throw AipsError("Too much row in table");
  }
  for (uInt i = 0; i < mycolumns.nrow(); ++i) {
    if (comparer(mycolumns, i)) {
      id = (Int) i;
    }
  }
  if (id < 0) {
    id = mycolumns.nrow();
    mytable.addRow(1, True);
    updater(mycolumns, id);
  }

  POST_END;
  return id;
}

template<class _Columns, class _Record>
inline void updateSubtable(_Columns &columns, uInt irow,
    _Record const &record) {
  // only update timestamp and interval
  Double time_org = columns.time()(irow);
  Double interval_org = columns.interval()(irow);

  Double time_min_org = time_org - interval_org / 2.0;
  Double time_max_org = time_org + interval_org / 2.0;

  Double time_min_in = record.time - record.interval / 2.0;
  Double time_max_in = record.time + record.interval / 2.0;

  Double time_min_new = min(time_min_org, time_min_in);
  Double time_max_new = max(time_max_org, time_max_in);

  if (time_min_new != time_min_org || time_max_new != time_max_org) {
    Double time_new = (time_min_new + time_max_new) / 2.0;
    Double interval_new = time_max_new - time_min_new;
    columns.time().put(irow, time_new);
    columns.interval().put(irow, interval_new);
  }
}

void makeSourceMap(MSSource const &table, Record &source_map) {
  POST_START;

  ROScalarColumn<String> name_column(table, "NAME");
  ROScalarColumn<Int> id_column(table, "SOURCE_ID");
  Vector<Int> id = id_column.getColumn();

  Sort sorter;
  sorter.sortKey(id.data(), TpInt);
  Vector<uInt> unique_vector;
  uInt num_id = sorter.sort(unique_vector, id.nelements(),
      Sort::HeapSort | Sort::NoDuplicates);
  for (uInt i = 0; i < num_id; ++i) {
    uInt irow = unique_vector[i];
    String const source_name = name_column(irow);
    Int const source_id = id[irow];
    source_map.define(source_name, source_id);
  }

  POST_END;
}

} // anonymous namespace

namespace casa { //# NAMESPACE CASA - BEGIN

static constexpr ssize_t CONTEXT_BUFFER_SIZE = 10;
static constexpr ssize_t FILLER_STORAGE_SIZE = CONTEXT_BUFFER_SIZE + 2;
typedef sdfiller::ProducerConsumerModelContext<ssize_t, CONTEXT_BUFFER_SIZE> PCMContext;

extern PCMContext *g_context_p;
extern DataRecord *g_storage_p;

template<class T>
void SingleDishMSFiller<T>::create_context() {
  static_assert(0 < FILLER_STORAGE_SIZE && FILLER_STORAGE_SIZE < SSIZE_MAX,
      "FILLER_STORAGE_SIZE is wrong value");
  static_assert(CONTEXT_BUFFER_SIZE + 2 <= FILLER_STORAGE_SIZE,
      "FILLER_STORAGE_SIZE < CONTEXT_BUFFER_SIZE + 2");

  constexpr ssize_t END_OF_PRODUCTION = -1;
  if (!g_context_p) {
    g_context_p = new PCMContext(END_OF_PRODUCTION);
  }
  if (!g_storage_p) {
    g_storage_p = new DataRecord[FILLER_STORAGE_SIZE];
  }
}

template<class T>
void SingleDishMSFiller<T>::destroy_context() {
  if (g_context_p) {
    delete g_context_p;
    g_context_p = nullptr;
  }
  if (g_storage_p) {
    delete[] g_storage_p;
    g_storage_p = nullptr;
  }
}

template<class T>
void *SingleDishMSFiller<T>::consume(void *arg) {
  POST_START;

  try {
    auto filler = reinterpret_cast<SingleDishMSFiller<T> *>(arg);
    auto reader = filler->reader_.get();

//      std::ostringstream oss;

    size_t nrow = reader->getNumberOfRows();
    assert(nrow < SIZE_MAX);
    DataAccumulator accumulator;

//      oss << "consume: nrow = " << nrow;
//      PCMContext::locked_print(oss.str(), g_context_p);

    ssize_t storage_index;
    for (size_t irow = 0; irow < nrow + 1; ++irow) {
//        oss.str("");
//        oss << "consume: start row " << irow;
//        PCMContext::locked_print(oss.str(), g_context_p);
      bool more_products = PCMContext::consume(g_context_p, &storage_index);
      assert(storage_index < FILLER_STORAGE_SIZE);

//        oss.str("");
//        oss << "consume index " << storage_index << " more_products = "
//            << more_products;
//        PCMContext::locked_print(oss.str(), g_context_p);

      if (more_products) {
        DataRecord *record = &g_storage_p[storage_index];

//          oss.str("");
//          oss << "Accumulate record: time = " << record->time << " interval = "
//              << record->interval;
//          PCMContext::locked_print(oss.str(), g_context_p);

        Bool is_ready = accumulator.queryForGet(record->time);
        if (is_ready) {
          filler->flush(accumulator);
        }
        Bool astatus = accumulator.accumulate(*record);
        (void) astatus;

//          oss.str("");
//          oss << "astatus = " << astatus;
//          PCMContext::locked_print(oss.str(), g_context_p);
      } else {
        break;
      }
    }

//      PCMContext::locked_print("Final flush", g_context_p);
    filler->flush(accumulator);
  } catch (std::runtime_error &e) {
    std::ostringstream oss;
    oss << "Exception: " << e.what();
    PCMContext::locked_print(oss.str(), g_context_p);
  } catch (...) {
    PCMContext::locked_print("Unknown exception", g_context_p);
  }

  POST_END;
  pthread_exit(0);
}

template<class T>
void *SingleDishMSFiller<T>::produce(void *arg) {
  POST_START;

  try {
    auto filler = reinterpret_cast<SingleDishMSFiller<T> *>(arg);
    auto reader = filler->reader_.get();

//      std::ostringstream oss;

    size_t nrow = reader->getNumberOfRows();

//      oss << "produce: nrow = " << nrow;
//      PCMContext::locked_print(oss.str(), g_context_p);

    ssize_t storage_index = 0;

    for (size_t irow = 0; irow < nrow; ++irow) {

      DataRecord *record = &g_storage_p[storage_index];
      Bool status = reader->getData(irow, *record);

//        oss.str("");
//        oss << "irow " << irow << " status " << status << std::endl;
//        oss << "   TIME=" << record->time << " INTERVAL=" << record->interval
//            << std::endl;
//        oss << "status = " << status;
//        PCMContext::locked_print(oss.str(), g_context_p);

      if (status) {
//          oss.str("");
//          oss << "produce index " << storage_index;
//          PCMContext::locked_print(oss.str(), g_context_p);

        PCMContext::produce(g_context_p, storage_index);
      } else {
        break;
      }

      storage_index++;
      storage_index %= FILLER_STORAGE_SIZE;
    }

//      PCMContext::locked_print("Done production", g_context_p);

    PCMContext::complete_production(g_context_p);

  } catch (std::runtime_error &e) {
    std::ostringstream oss;
    oss << "Exception: " << e.what();
    PCMContext::locked_print(oss.str(), g_context_p);

    PCMContext::complete_production(g_context_p);
  } catch (...) {
    PCMContext::locked_print("Unknown exception", g_context_p);

    PCMContext::complete_production(g_context_p);
  }

  POST_END;
  pthread_exit(0);
}

template<class T>
void SingleDishMSFiller<T>::fillMainMT(SingleDishMSFiller<T> *filler) {
  POST_START;

  pthread_t consumer_id;
  pthread_t producer_id;

  // create context
  SingleDishMSFiller<T>::create_context();

  try {
    sdfiller::create_thread(&consumer_id, NULL, SingleDishMSFiller<T>::consume,
        filler);
    sdfiller::create_thread(&producer_id, NULL, SingleDishMSFiller<T>::produce,
        filler);

    sdfiller::join_thread(&consumer_id, NULL);
    sdfiller::join_thread(&producer_id, NULL);
  } catch (...) {
    SingleDishMSFiller<T>::destroy_context();
    throw;
  }

  SingleDishMSFiller<T>::destroy_context();

  POST_END;
}

template<class T>
SingleDishMSFiller<T>::SingleDishMSFiller(std::string const &name) :
    ms_(), ms_columns_(), data_description_columns_(), feed_columns_(),
    pointing_columns_(), polarization_columns_(), syscal_columns_(),
    state_columns_(), weather_columns_(), reader_(new T(name)),
    is_float_(false), data_key_(), reference_feed_(-1), pointing_time_(),
    pointing_time_max_(), pointing_time_min_(), num_pointing_time_(),
    syscal_list_(), subscan_list_(), polarization_type_pool_(), weather_list_() {
}

template<class T>
SingleDishMSFiller<T>::~SingleDishMSFiller() {
}

template<class T>
void SingleDishMSFiller<T>::fill() {
  POST_START;

  // initialization
  initialize();

  // Fill tables that can be processed prior to main loop
  fillPreProcessTables();

  // main loop
#if 0
  SingleDishMSFiller<T>::fillMainMT(this);
#else
  fillMain();
#endif

  // Fill tables that must be processed after main loop
  fillPostProcessTables();

  // finalization
  finalize();

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::initialize() {
  POST_START;

  // initialize reader
  reader_->initialize();

  // query if the data is complex or float
  is_float_ = reader_->isFloatData();
  if (is_float_) {
//      std::cout << "data column is FLOAT_DATA" << std::endl;
    data_key_ = "FLOAT_DATA";
  } else {
//      std::cout << "data column is DATA" << std::endl;
    data_key_ = "DATA";
  }

  // setup MS
  setupMS();

  // frame information
  MDirection::Types direction_frame = reader_->getDirectionFrame();
  auto mytable = ms_->pointing();
  ArrayColumn<Double> direction_column(mytable, "DIRECTION");
  TableRecord &record = direction_column.rwKeywordSet();
  Record meas_info = record.asRecord("MEASINFO");
  String ref_string = MDirection::showType(direction_frame);
  meas_info.define("Ref", ref_string);
  record.defineRecord("MEASINFO", meas_info);

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::finalize() {
  POST_START;

  // finalize reader
  reader_->finalize();

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::save(std::string const &name) {
  POST_START;

#ifdef SINGLEDISHMSFILLER_DEBUG
  std::cout << "Saving MS as \"" << name << "\"" << std::endl;
  std::cout << "current working directory is \"" << Path().absoluteName()
  << "\"" << std::endl;
#endif

  ms_->deepCopy(name, Table::New);

#ifdef SINGLEDISHMSFILLER_DEBUG
  File file(name);
  Bool name_exists = file.exists();
  if (name_exists) {
    std::cout << "file successfully created" << std::endl;
  } else {
    std::cout << "failed to create file" << std::endl;
  }
#endif

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::setupMS() {
//  std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl;

//  String dunit = table_->getHeader().fluxunit ;

  TableDesc ms_main_description = MeasurementSet::requiredTableDesc();
  if (is_float_) {
    MeasurementSet::addColumnToDesc(ms_main_description,
        MSMainEnums::FLOAT_DATA, 2);
  } else {
    MeasurementSet::addColumnToDesc(ms_main_description, MSMainEnums::DATA, 2);
  }

  String const scratch_table_name = File::newUniqueName(".",
      "SingleDishMSFillerTemp").originalName();
  SetupNewTable newtab(scratch_table_name, ms_main_description, Table::Scratch);

  ms_.reset(new MeasurementSet(newtab));

  // create subtables
  TableDesc ms_antenna_description = MSAntenna::requiredTableDesc();
  SetupNewTable ms_antenna_table(ms_->antennaTableName(),
      ms_antenna_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::ANTENNA),
      Table(ms_antenna_table));

  TableDesc ms_data_desc_description = MSDataDescription::requiredTableDesc();
  SetupNewTable ms_data_desc_table(ms_->dataDescriptionTableName(),
      ms_data_desc_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::DATA_DESCRIPTION),
      Table(ms_data_desc_table));

  TableDesc ms_doppler_description = MSDoppler::requiredTableDesc();
  SetupNewTable ms_doppler_table(ms_->dopplerTableName(),
      ms_doppler_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::DOPPLER),
      Table(ms_doppler_table));

  TableDesc ms_feed_description = MSFeed::requiredTableDesc();
  SetupNewTable ms_feed_table(ms_->feedTableName(), ms_feed_description,
      Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::FEED), Table(ms_feed_table));

  TableDesc ms_field_description = MSField::requiredTableDesc();
  SetupNewTable ms_field_table(ms_->fieldTableName(), ms_field_description,
      Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::FIELD),
      Table(ms_field_table));

  TableDesc ms_flag_cmd_description = MSFlagCmd::requiredTableDesc();
  SetupNewTable ms_flag_cmd_table(ms_->flagCmdTableName(),
      ms_flag_cmd_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::FLAG_CMD),
      Table(ms_flag_cmd_table));

  TableDesc ms_freq_offset_description = MSFreqOffset::requiredTableDesc();
  SetupNewTable ms_freq_offset_table(ms_->freqOffsetTableName(),
      ms_freq_offset_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::FREQ_OFFSET),
      Table(ms_freq_offset_table));

  TableDesc ms_history_description = MSHistory::requiredTableDesc();
  SetupNewTable ms_history_table(ms_->historyTableName(),
      ms_history_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::HISTORY),
      Table(ms_history_table));

  TableDesc ms_observation_description = MSObservation::requiredTableDesc();
  SetupNewTable ms_observation_table(ms_->observationTableName(),
      ms_observation_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::OBSERVATION),
      Table(ms_observation_table));

  TableDesc ms_pointing_description = MSPointing::requiredTableDesc();
  SetupNewTable ms_pointing_table(ms_->pointingTableName(),
      ms_pointing_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::POINTING),
      Table(ms_pointing_table));

  TableDesc ms_polarization_description = MSPolarization::requiredTableDesc();
  SetupNewTable ms_polarization_table(ms_->polarizationTableName(),
      ms_polarization_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::POLARIZATION),
      Table(ms_polarization_table));

  TableDesc ms_processor_description = MSProcessor::requiredTableDesc();
  SetupNewTable ms_processor_table(ms_->processorTableName(),
      ms_processor_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::PROCESSOR),
      Table(ms_processor_table));

  TableDesc ms_source_description = MSSource::requiredTableDesc();
  MSSource::addColumnToDesc(ms_source_description, MSSourceEnums::TRANSITION,
      1);
  MSSource::addColumnToDesc(ms_source_description,
      MSSourceEnums::REST_FREQUENCY, 1);
  MSSource::addColumnToDesc(ms_source_description, MSSourceEnums::SYSVEL, 1);
  SetupNewTable ms_source_table(ms_->sourceTableName(), ms_source_description,
      Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::SOURCE),
      Table(ms_source_table));

  TableDesc ms_spectral_window_description =
      MSSpectralWindow::requiredTableDesc();
  SetupNewTable ms_spectral_window_table(ms_->spectralWindowTableName(),
      ms_spectral_window_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::SPECTRAL_WINDOW),
      Table(ms_spectral_window_table));

  TableDesc ms_state_description = MSState::requiredTableDesc();
  SetupNewTable ms_state_table(ms_->stateTableName(), ms_state_description,
      Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::STATE),
      Table(ms_state_table));

  TableDesc ms_syscal_description = MSSysCal::requiredTableDesc();
  MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TCAL_SPECTRUM,
      2);
  MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TCAL, 1);
  MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TSYS_SPECTRUM,
      2);
  MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TSYS, 1);
  SetupNewTable ms_syscal_table(ms_->sysCalTableName(), ms_syscal_description,
      Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::SYSCAL),
      Table(ms_syscal_table));

  TableDesc ms_weather_description = MSWeather::requiredTableDesc();
  MSWeather::addColumnToDesc(ms_weather_description,
      MSWeatherEnums::TEMPERATURE);
  MSWeather::addColumnToDesc(ms_weather_description, MSWeatherEnums::PRESSURE);
  MSWeather::addColumnToDesc(ms_weather_description,
      MSWeatherEnums::REL_HUMIDITY);
  MSWeather::addColumnToDesc(ms_weather_description,
      MSWeatherEnums::WIND_SPEED);
  MSWeather::addColumnToDesc(ms_weather_description,
      MSWeatherEnums::WIND_DIRECTION);
  SetupNewTable ms_weather_table(ms_->weatherTableName(),
      ms_weather_description, Table::Scratch);
  ms_->rwKeywordSet().defineTable(
      MeasurementSet::keywordName(MeasurementSet::WEATHER),
      Table(ms_weather_table));

  ms_->initRefs();

  // Set up MSMainColumns
  ms_columns_.reset(new MSMainColumns(*ms_));

  // Set up MSDataDescColumns
  data_description_columns_.reset(
      new MSDataDescColumns(ms_->dataDescription()));

  // Set up MSFeedColumns
  feed_columns_.reset(new MSFeedColumns(ms_->feed()));

  // Set up MSPointingColumns
  pointing_columns_.reset(new MSPointingColumns(ms_->pointing()));

  // Set up MSPolarizationColumns
  polarization_columns_.reset(new MSPolarizationColumns(ms_->polarization()));

  // Set up MSSysCalColumns
  syscal_columns_.reset(new MSSysCalColumns(ms_->sysCal()));

  // Set up MSStateColumns
  state_columns_.reset(new MSStateColumns(ms_->state()));

  // Set up MSWeatherColumns
  weather_columns_.reset(new MSWeatherColumns(ms_->weather()));

//  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
}

template<class T>
void SingleDishMSFiller<T>::fillPreProcessTables() {
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

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillPostProcessTables() {
  POST_START;

  // fill HISTORY table
  fillHistory();

  // flush POINTING entry
  sortPointing();

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillMain() {
  POST_START;

  size_t nrow = reader_->getNumberOfRows();
  DataAccumulator accumulator;
  DataRecord record;
//    std::cout << "nrow = " << nrow << std::endl;
  for (size_t irow = 0; irow < nrow; ++irow) {
    Bool status = reader_->getData(irow, record);
//      std::cout << "irow " << irow << " status " << status << std::endl;
//      std::cout << "   TIME=" << record.time << " INTERVAL=" << record.interval
//          << std::endl;
//      std::cout << "status = " << status << std::endl;
    if (status) {
      Bool is_ready = accumulator.queryForGet(record.time);
      if (is_ready) {
        flush(accumulator);
      }
      Bool astatus = accumulator.accumulate(record);
      (void) astatus;
//        std::cout << "astatus = " << astatus << std::endl;
    }
  }

  flush(accumulator);

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillAntenna() {
  POST_START;

  auto mytable = ms_->antenna();
  AntennaRecord record;
  ::fillTable(mytable, record,
      [&](AntennaRecord &record) {return reader_->getAntennaRow(record);});

  // initialize POINTING table related stuff
  uInt nrow = mytable.nrow();
  num_pointing_time_.resize(nrow);
  for (uInt i = 0; i < nrow; ++i) {
    pointing_time_[i] = Vector<Double>(ARRAY_BLOCK_SIZE, -1.0);
    pointing_time_min_[i] = -1.0;
    pointing_time_max_[i] = 1.0e30;
    num_pointing_time_[i] = 0;
  }

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillObservation() {
  POST_START;

  ObservationRecord record;
  ::fillTable(ms_->observation(), record,
      [&](ObservationRecord &record) {return reader_->getObservationRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillProcessor() {
  POST_START;

  ProcessorRecord record;
  ::fillTable(ms_->processor(), record,
      [&](ProcessorRecord &record) {return reader_->getProcessorRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillSource() {
  POST_START;

  SourceRecord record;
  ::fillTable(ms_->source(), record,
      [&](SourceRecord &record) {return reader_->getSourceRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillField() {
  POST_START;

  auto mytable = ms_->field();
  FieldRecord record;
  record.table = mytable;

  // make (name,id) map for SOURCE table
  ::makeSourceMap(ms_->source(), record.source_map);

  ::fillTable(mytable, record,
      [&](FieldRecord &record) {return reader_->getFieldRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillHistory() {
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

template<class T>
void SingleDishMSFiller<T>::fillSpectralWindow() {
  POST_START;

  auto mytable = ms_->spectralWindow();
  SpectralWindowRecord record;

  ::fillTable(mytable, record,
      [&](SpectralWindowRecord &record) {return reader_->getSpectralWindowRow(record);});

  POST_END;
}

template<class T>
Int SingleDishMSFiller<T>::updatePolarization(Vector<Int> const &corr_type,
    Int const &num_pol) {
  uInt num_corr = corr_type.size();
  if (num_pol < 1 || num_corr != (uInt) num_pol) {
    throw AipsError("Internal inconsistency in number of correlations");
  }
  MSPolarization &mytable = ms_->polarization();
  //MSPolarizationColumns mycolumns(mytable);
  Matrix<Int> const corr_product(2, num_pol, 0);
  auto comparer = [&](MSPolarizationColumns const &columns, uInt i) {
    Bool match = allEQ(columns.corrType()(i), corr_type);
    return match;
  };
  auto updater = [&](MSPolarizationColumns &columns, uInt i) {
    columns.numCorr().put(i, num_pol);
    columns.corrType().put(i, corr_type);
    columns.corrProduct().put(i, corr_product);
  };
  Int polarization_id = ::updateTable(mytable, *(polarization_columns_.get()),
      comparer, updater);
  return polarization_id;
}

template<class T>
Int SingleDishMSFiller<T>::updateDataDescription(Int const &polarization_id,
    Int const &spw_id) {
  if (polarization_id < 0 || spw_id < 0) {
    throw AipsError("Invalid ids for DATA_DESCRIPTION");
  }
  MSDataDescription &mytable = ms_->dataDescription();
  //MSDataDescColumns mycolumns(mytable);
  auto comparer = [&](MSDataDescColumns const &columns, uInt i) {
    Bool match = (columns.polarizationId()(i) == polarization_id)
    && (columns.spectralWindowId()(i) == spw_id);
    return match;
  };
  auto updater = [&](MSDataDescColumns &columns, uInt i) {
    columns.polarizationId().put(i, polarization_id);
    columns.spectralWindowId().put(i, spw_id);
  };
  Int data_desc_id = ::updateTable(mytable, *(data_description_columns_.get()),
      comparer, updater);

  return data_desc_id;
}

template<class T>
Int SingleDishMSFiller<T>::updateState(Int const &subscan,
    String const &obs_mode) {
  MSState &mytable = ms_->state();
  static Regex const regex("^OBSERVE_TARGET#ON_SOURCE");
  //static std::vector<Int> subscan_list;
  auto comparer =
      [&](MSStateColumns &columns, uInt i) {
        Bool match = (subscan == subscan_list_[i]) && (obs_mode == columns.obsMode()(i));
        return match;
      };
  auto updater = [&](MSStateColumns &columns, uInt i) {
    columns.subScan().put(i, subscan);
    columns.obsMode().put(i, obs_mode);
    Bool is_signal = obs_mode.matches(regex);
    columns.sig().put(i, is_signal);
    columns.ref().put(i, !is_signal);

    subscan_list_.push_back(subscan);
  };
  Int state_id = ::updateTable(mytable, *(state_columns_.get()), comparer,
      updater);
  return state_id;
}

template<class T>
Int SingleDishMSFiller<T>::updateFeed(Int const &feed_id, Int const &spw_id,
    String const &pol_type) {
  constexpr Int num_receptors = 2;
  static String const linear_type_arr[2] = { "X", "Y" };
  static Vector<String> linear_type(linear_type_arr, 2, SHARE);
  static String const circular_type_arr[2] = { "R", "L" };
  static Vector<String> circular_type(circular_type_arr, 2, SHARE);
  static Matrix<Complex> const pol_response(num_receptors, num_receptors,
      Complex(0));
  Vector<String> *polarization_type = nullptr;
  if (pol_type == "linear") {
    polarization_type = &linear_type;
  } else if (pol_type == "circular") {
    polarization_type = &circular_type;
  }
  //static std::vector< Vector<String> *> polarization_type_pool;

  String polarization_type_arr[2] = { "X", "Y" };
  Vector<String> polarization_type_storage(polarization_type_arr, 2, SHARE);
  Matrix<Double> const beam_offset(2, num_receptors, 0.0);
  Vector<Double> const receptor_angle(num_receptors, 0.0);
  Vector<Double> const position(3, 0.0);
  auto comparer = [&](MSFeedColumns &columns, uInt i) {
    Vector<String> *current_polarization_type = polarization_type_pool_[i];
    Bool match = allEQ(*polarization_type, *current_polarization_type) &&
    (feed_id == columns.feedId()(i)) &&
    (spw_id == columns.spectralWindowId()(i));
    return match;
  };
  auto updater = [&](MSFeedColumns &columns, uInt i) {
    // TODO: 2016/01/26 TN
    // Here I regard "multi-beam receiver" as multi-feed, single-beam receivers
    // in MS v2 notation. So, BEAM_ID is always 0 and FEED_ID is beam identifier
    // for "multi-beam" receivers so far. However, if we decide to import beams
    // as separated virtual antennas, FEED_ID must always be 0.
      columns.feedId().put(i, feed_id);
      columns.beamId().put(i, 0);
      columns.spectralWindowId().put(i, spw_id);
      columns.polarizationType().put(i, *polarization_type);
      columns.beamOffset().put(i, beam_offset);
      columns.receptorAngle().put(i, receptor_angle);
      columns.position().put(i, position);
      columns.polResponse().put(i, pol_response);

      polarization_type_pool_.push_back(polarization_type);
    };
  Int feed_row = ::updateTable(ms_->feed(), *(feed_columns_.get()), comparer,
      updater);

  // reference feed
  if (reference_feed_ < 0) {
    reference_feed_ = feed_id;
  }

  return feed_row;
}

template<class T>
Int SingleDishMSFiller<T>::updatePointing(Int const &antenna_id,
    Int const &feed_id, Double const &time, Double const &interval,
    Matrix<Double> const &direction) {
  POST_START;

  if (reference_feed_ != feed_id) {
    return -1;
  }

  auto &mytable = ms_->pointing();
  uInt nrow = mytable.nrow();

  uInt *n = &num_pointing_time_[antenna_id];
  Double *time_max = &pointing_time_max_[antenna_id];
  Double *time_min = &pointing_time_min_[antenna_id];
  Vector<Double> *time_list = &pointing_time_[antenna_id];

  auto addPointingRow =
      [&]() {
        mytable.addRow(1, True);
        pointing_columns_->time().put(nrow, time);
        pointing_columns_->interval().put(nrow, interval);
        pointing_columns_->antennaId().put(nrow, antenna_id);
        if (direction.ncolumn() == 1 || allEQ(direction.column(1), 0.0)) {
          pointing_columns_->numPoly().put(nrow, 0);
          pointing_columns_->direction().put(nrow, direction(IPosition(2,0,0), IPosition(2,1,0)));
        } else {
          pointing_columns_->direction().put(nrow, direction);
          Int num_poly = direction.shape()[1] - 1;
          pointing_columns_->numPoly().put(nrow, num_poly);
        }
        // add timestamp to the list
        uInt nelem = time_list->nelements();
        if (nelem <= *n) {
          time_list->resize(nelem + ARRAY_BLOCK_SIZE, True);
        }
        (*time_list)[*n] = time;
        // increment number of pointing entry
        *n += 1;
      };

  if (*n == 0) {
    addPointingRow();
    *time_min = time;
    *time_max = time;
  } else if (time < *time_min) {
    addPointingRow();
    *time_min = time;
  } else if (*time_max < time) {
    addPointingRow();
    *time_max = time;
  } else if (allNE(*time_list, time)) {
    addPointingRow();
  }

  POST_END;

  return -1;
}

template<class T>
void SingleDishMSFiller<T>::updateWeather(Int const &antenna_id,
    Double const &time, Double const &interval,
    MSDataRecord const &data_record) {
  WeatherRecord record;
  record.clear();
  record.antenna_id = antenna_id;
  record.time = time;
  record.interval = interval;
  record.temperature = data_record.temperature;
  record.pressure = data_record.pressure;
  record.rel_humidity = data_record.rel_humidity;
  record.wind_speed = data_record.wind_speed;
  record.wind_direction = data_record.wind_direction;
  auto &mytable = ms_->weather();
  auto pos = std::find(weather_list_.begin(), weather_list_.end(), record);
  if (pos == weather_list_.end()) {
    weather_list_.push_back(record);
    uInt irow = mytable.nrow();
    mytable.addRow(1, True);
    record.fill(irow, *(weather_columns_.get()));
  } else {
    auto irow = std::distance(weather_list_.begin(), pos);
    updateWeather(*(weather_columns_.get()), irow, record);
  }
}

template<class T>
void SingleDishMSFiller<T>::updateWeather(MSWeatherColumns &columns, uInt irow,
    WeatherRecord const &record) {
  ::updateSubtable(columns, irow, record);
}

template<class T>
void SingleDishMSFiller<T>::updateSysCal(Int const &antenna_id,
    Int const &feed_id, Int const &spw_id, Double const &time,
    Double const &interval, MSDataRecord const &data_record) {
  POST_START;

  SysCalRecord record;
  record.clear();
  record.antenna_id = antenna_id;
  record.feed_id = feed_id;
  record.spw_id = spw_id;
  record.time = time;
  record.interval = interval;

  //Bool tcal_empty = False;
  Bool tsys_empty = False;

  if (data_record.tcal.empty() || allEQ(data_record.tcal, 1.0f)
      || allEQ(data_record.tcal, 0.0f)) {
    //tcal_empty = True;
  } else {
//      std::cout << "tcal seems to be valid " << data_record.tcal << std::endl;
    if (data_record.float_data.shape() == data_record.tcal.shape()) {
      record.tcal_spectrum.assign(data_record.tcal);
    } else {
      Matrix<Float> tcal = data_record.tcal;
      if (!tcal.empty()) {
        record.tcal.assign(tcal.column(0));
      }
    }
  }
  if (data_record.tsys.empty() || allEQ(data_record.tsys, 1.0f)
      || allEQ(data_record.tsys, 0.0f)) {
    tsys_empty = True;
  } else {
    if (data_record.float_data.shape() == data_record.tsys.shape()) {
      record.tsys_spectrum.assign(data_record.tsys);
    } else {
      Matrix<Float> tsys = data_record.tsys;
      if (!tsys.empty()) {
        record.tsys.assign(tsys.column(0));
      }
    }
  }

  // do not add entry if Tsys is empty
  //if (tcal_empty && tsys_empty) {
  if (tsys_empty) {
    return;
  }

  auto &mytable = ms_->sysCal();
  auto pos = std::find(syscal_list_.begin(), syscal_list_.end(), record);
  if (pos == syscal_list_.end()) {
    uInt irow = mytable.nrow();
    mytable.addRow(1, True);
    record.fill(irow, *(syscal_columns_.get()));
    syscal_list_.push_back(SysCalTableRecord(ms_.get(), irow, record));
  } else {
    auto irow = std::distance(syscal_list_.begin(), pos);
    updateSysCal(*(syscal_columns_.get()), irow, record);
  }

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::updateSysCal(MSSysCalColumns &columns, uInt irow,
    SysCalRecord const &record) {
  ::updateSubtable(columns, irow, record);
}

template<class T>
void SingleDishMSFiller<T>::updateMain(Int const &antenna_id, Int field_id,
    Int feedId, Int dataDescriptionId, Int stateId, Int const &scan_number,
    Double const &time, MSDataRecord const &dataRecord) {
  POST_START;

  // constant stuff
  static Vector<Double> const uvw(3, 0.0);
  static Array<Bool> const flagCategory(IPosition(3, 0, 0, 0));

  // target row id
  uInt irow = ms_->nrow();

  // add new row
  //ms_->addRow(1, True);
  ms_->addRow(1, False);

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
  ms_columns_->time().put(irow, time);
  ms_columns_->timeCentroid().put(irow, time);
  Double const &interval = dataRecord.interval;
  ms_columns_->interval().put(irow, interval);
  ms_columns_->exposure().put(irow, interval);

  if (is_float_) {
    Matrix<Float> floatData;
    if (dataRecord.isFloat()) {
      floatData.reference(dataRecord.float_data);
    } else {
      floatData.assign(real(dataRecord.complex_data));
    }
    ms_columns_->floatData().put(irow, floatData);
  } else {
    Matrix<Complex> data;
    if (dataRecord.isFloat()) {
      data.assign(
          makeComplex(dataRecord.float_data,
              Matrix<Float>(dataRecord.float_data.shape(), 0.0f)));
    } else {
      data.reference(dataRecord.complex_data);
    }
    ms_columns_->data().put(irow, data);
  }

  ms_columns_->flag().put(irow, dataRecord.flag);
  ms_columns_->flagRow().put(irow, dataRecord.flag_row);
  ms_columns_->sigma().put(irow, dataRecord.sigma);
  ms_columns_->weight().put(irow, dataRecord.weight);

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::flush(DataAccumulator &accumulator) {
  POST_START;

  size_t nchunk = accumulator.getNumberOfChunks();
//    std::cout << "nchunk = " << nchunk << std::endl;

  if (nchunk == 0) {
    return;
  }

  for (size_t ichunk = 0; ichunk < nchunk; ++ichunk) {
    Bool status = accumulator.get(ichunk, record_);
//      std::cout << "accumulator status = " << std::endl;
    if (status) {
      Double time = record_.time;
      Int antenna_id = record_.antenna_id;
      Int spw_id = record_.spw_id;
      Int feed_id = record_.feed_id;
      Int field_id = record_.field_id;
      Int scan = record_.scan;
      Int subscan = record_.subscan;
      String pol_type = record_.pol_type;
      String obs_mode = record_.intent;
      Int num_pol = record_.num_pol;
      Vector<Int> &corr_type = record_.corr_type;
      Int polarization_id = updatePolarization(corr_type, num_pol);
      updateFeed(feed_id, spw_id, pol_type);
      Int data_desc_id = updateDataDescription(polarization_id, spw_id);
      Int state_id = updateState(subscan, obs_mode);
      Matrix<Double> &direction = record_.direction;
      Double interval = record_.interval;

      // updatePointing must be called after updateFeed
      updatePointing(antenna_id, feed_id, time, interval, direction);

      updateSysCal(antenna_id, feed_id, spw_id, time, interval, record_);

      updateWeather(antenna_id, time, interval, record_);

      updateMain(antenna_id, field_id, feed_id, data_desc_id, state_id, scan,
          time, record_);
    }
  }
//    std::cout << "clear accumulator" << std::endl;
  accumulator.clear();

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::sortPointing() {
  POST_START;

  // deallocate POINTING table related stuff
  pointing_time_.clear();
  pointing_time_min_.clear();
  pointing_time_max_.clear();
  num_pointing_time_.resize();

  auto mytable = ms_->pointing();
  MSPointingColumns mycolumns(mytable);
  uInt nrow = mytable.nrow();
  Vector<Int> antenna_id_list = mycolumns.antennaId().getColumn();
  Vector<Double> time_list = mycolumns.time().getColumn();
  Sort sorter;
  sorter.sortKey(antenna_id_list.data(), TpInt);
  sorter.sortKey(time_list.data(), TpDouble);
  Vector<uInt> index_vector;
  sorter.sort(index_vector, nrow);

  size_t storage_size = nrow * 2 * sizeof(Double);
  std::unique_ptr<void, sdfiller::Deleter> storage(malloc(storage_size));

  // sort TIME
  {
    Vector<Double> sorted(IPosition(1, nrow),
        reinterpret_cast<Double *>(storage.get()), SHARE);
    for (uInt i = 0; i < nrow; ++i) {
      sorted[i] = time_list[index_vector[i]];
    }
    mycolumns.time().putColumn(sorted);
  }
  // sort ANTENNA_ID
  {
    Vector<Int> sorted(IPosition(1, nrow),
        reinterpret_cast<Int *>(storage.get()), SHARE);
    for (uInt i = 0; i < nrow; ++i) {
      sorted[i] = antenna_id_list[index_vector[i]];
    }
    mycolumns.antennaId().putColumn(sorted);
  }

  // sort NUM_POLY
  {
    Vector<Int> num_poly_list(antenna_id_list);
    mycolumns.numPoly().getColumn(num_poly_list);
    Vector<Int> sorted(IPosition(1, nrow),
        reinterpret_cast<Int *>(storage.get()), SHARE);
    for (uInt i = 0; i < nrow; ++i) {
      sorted[i] = antenna_id_list[index_vector[i]];
    }
    mycolumns.numPoly().putColumn(sorted);
  }

  // sort DIRECTION
  {
    std::map<uInt, Matrix<Double> > direction;
    for (uInt i = 0; i < nrow; ++i) {
      direction[i] = mycolumns.direction()(i);
    }
    for (uInt i = 0; i < nrow; ++i) {
      mycolumns.direction().put(index_vector[i], direction[i]);
    }
  }

  POST_END;
}

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SINGLEDISHMSFILLER_TCC_ */

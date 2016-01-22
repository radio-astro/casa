/*
 * SingleDishMSFiller.tcc
 *
 *  Created on: Jan 8, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SINGLEDISHMSFILLER_TCC_
#define SINGLEDISH_FILLER_SINGLEDISHMSFILLER_TCC_

#include <singledish/Filler/SingleDishMSFiller.h>

#include <iostream>

#include <casacore/casa/OS/File.h>

#include <casacore/ms/MeasurementSets/MeasurementSet.h>

#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/SetupNewTab.h>

using namespace casacore;

namespace {

template<class _Table, class _Reader>
void fillTable(_Table &mytable, _Reader const &reader) {
  POST_START;

  TableRow row(mytable);
  TableRecord record = row.record();

  size_t irow = 0;
  for (Bool more_rows = reader(record); more_rows == True;
      more_rows = reader(record)) {
    mytable.addRow(1, True);
    row.put(irow, record);
    ++irow;
  }

  POST_END;
}

} // anonymous namespace

namespace casa { //# NAMESPACE CASA - BEGIN

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
  std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl;

//  String dunit = table_->getHeader().fluxunit ;

  TableDesc ms_main_description = MeasurementSet::requiredTableDesc();
//  if ( useFloatData_ )
//    MeasurementSet::addColumnToDesc( msDesc, MSMainEnums::FLOAT_DATA, 2 ) ;
//  else if ( useData_ )
//    MeasurementSet::addColumnToDesc( msDesc, MSMainEnums::DATA, 2 ) ;
  MeasurementSet::addColumnToDesc(ms_main_description, MSMainEnums::FLOAT_DATA,
      2);

  String const scratch_table_name = File::newUniqueName(".",
      "SingleDishMSFillerTemp").originalName();
  SetupNewTable newtab(scratch_table_name, ms_main_description, Table::Scratch);

  ms_.reset(new MeasurementSet(newtab));

//  TableColumn col;
//  if (useFloatData_)
//    col.attach(*ms_, "FLOAT_DATA");
//  else if (useData_)
//    col.attach(*ms_, "DATA");
//  col.rwKeywordSet().define("UNIT", dunit);

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
//  if (tcalSpec_)
//    MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TCAL_SPECTRUM, 2);
//  else
//    MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TCAL, 1);
//  if (tsysSpec_)
//    MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TSYS_SPECTRUM, 2);
//  else
//    MSSysCal::addColumnToDesc(ms_syscal_description, MSSysCalEnums::TSYS, 1);
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

  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
}

template<class T>
void SingleDishMSFiller<T>::fillAntenna() {
  POST_START;

  fillTable(ms_->antenna(),
      [&](TableRecord &record) {return reader_->getAntennaRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillObservation() {
  POST_START;

  fillTable(ms_->observation(),
      [&](TableRecord &record) {return reader_->getObservationRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillProcessor() {
  POST_START;

  fillTable(ms_->processor(),
      [&](TableRecord &record) {return reader_->getProcessorRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillSource() {
  POST_START;

  fillTable(ms_->source(),
      [&](TableRecord &record) {return reader_->getSourceRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillSpectralWindow() {
  POST_START;

  TableRecord record_proxy;
  fillTable(ms_->spectralWindow(), [&](TableRecord &record) {
    record.print(std::cout);
    Bool status = reader_->getSpectralWindowRow(record_proxy);
    if (status) {
      record.define("NUM_CHAN", record_proxy.asInt("NUM_CHAN"));
      record.define("NAME", record_proxy.asString("NAME"));
      record.define("MEAS_FREQ_REF", record_proxy.asInt("MEAS_FREQ_REF"));
      Double refpix = record_proxy.asDouble("REFPIX");
      Double refval = record_proxy.asDouble("REFVAL");
      Double increment = record_proxy.asDouble("INCREMENT");
      std::cout << "NUM_CHAN" << std::endl;
      Int num_chan = record.asInt("NUM_CHAN");

      Double tot_bw = num_chan * abs(increment);
      std::cout << "TOTAL_BANDWIDTH" << std::endl;
      record.define("TOTAL_BANDWIDTH", tot_bw);

      Double ref_frequency = refval - refpix * increment;
      std::cout << "REF_FREQUENCY" << std::endl;
      record.define("REF_FREQUENCY", ref_frequency);

      Vector<Double> freq(num_chan);
      indgen(freq, ref_frequency, increment);
      std::cout << "CHAN_FREQ" << std::endl;
      record.define("CHAN_FREQ", freq);

      freq = increment;
      std::cout << "CHAN_WIDTH" << std::endl;
      record.define("CHAN_WIDTH", freq);

      freq = abs(freq);
      std::cout << "EFFECTIVE_BW" << std::endl;
      record.define("EFFECTIVE_BW", freq);
      record.define("RESOLUTION", freq);

      Int net_sideband = 0;// USB
      if (increment < 0.0) {
        net_sideband = 1; // LSB
      }
      std::cout << "NET_SIDEBAND" << std::endl;
      record.define("NET_SIDEBAND", net_sideband);
    }
    return status;});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillSyscal() {
  POST_START;

  fillTable(ms_->sysCal(),
      [&](TableRecord &record) {return reader_->getSyscalRow(record);});

  POST_END;
}

template<class T>
void SingleDishMSFiller<T>::fillWeather() {
  POST_START;

  fillTable(ms_->weather(),
      [&](TableRecord &record) {return reader_->getWeatherRow(record);});

  POST_END;
}

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SINGLEDISHMSFILLER_TCC_ */

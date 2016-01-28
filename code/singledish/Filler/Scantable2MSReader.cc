/*
 * Scantable2MSReader.cc
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#include <singledish/Filler/Scantable2MSReader.h>

#include <iostream>
#include <string>

#include <casacore/casa/OS/File.h>
#include <casacore/casa/Containers/Record.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/tables/Tables/TableRecord.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/tables/Tables/ScalarColumn.h>

using namespace casacore;

namespace {
Double queryAntennaDiameter(String const &name) {
  String capitalized = name;
  capitalized.upcase();
  Double diameter = 0.0;
  std::cout << "name \"" << name << "\" capitalized \"" << capitalized << "\""
      << std::endl;
  if (capitalized.matches(Regex(".*(DV|DA|PM)[0-9]+$"))) {
    diameter = 12.0;
  } else if (capitalized.matches(Regex(".*CM[0-9]+$"))) {
    diameter = 7.0;
  } else if (capitalized.contains("GBT")) {
    diameter = 104.9;
  } else if (capitalized.contains("MOPRA")) {
    diameter = 22.0;
  } else if (capitalized.contains("PKS") || capitalized.contains("PARKS")) {
    diameter = 64.0;
  } else if (capitalized.contains("TIDBINBILLA")) {
    diameter = 70.0;
  } else if (capitalized.contains("CEDUNA")) {
    diameter = 30.0;
  } else if (capitalized.contains("HOBART")) {
    diameter = 26.0;
  } else if (capitalized.contains("APEX")) {
    diameter = 12.0;
  } else if (capitalized.contains("ASTE")) {
    diameter = 10.0;
  } else if (capitalized.contains("NRO")) {
    diameter = 45.0;
  }

  return diameter;
}

}

namespace casa { //# NAMESPACE CASA - BEGIN

Scantable2MSReader::Scantable2MSReader(std::string const &scantable_name) :
    ReaderInterface(scantable_name), main_table_(nullptr), get_observation_row_(
        &Scantable2MSReader::getObservationRowImpl), get_antenna_row_(
        &Scantable2MSReader::getAntennaRowImpl), get_processor_row_(
        &Scantable2MSReader::getProcessorRowImpl), get_spw_row_(
        &Scantable2MSReader::getSpectralWindowRowImpl), get_weather_row_(
        &Scantable2MSReader::getWeatherRowImpl), weather_iter_(nullptr), freq_iter_(
        nullptr) {
  std::cout << "Scantabl2MSReader::Scantable2MSReader" << std::endl;
}

Scantable2MSReader::~Scantable2MSReader() {
  std::cout << "Scantabl2MSReader::~Scantable2MSReader" << std::endl;
}

void Scantable2MSReader::initializeSpecific() {
  std::cout << "Scantabl2MSReader::initialize" << std::endl;
  File f(name_);
  if (f.exists() and f.isDirectory()) {
    main_table_.reset(new Table(name_, Table::Old));
  } else {
    throw AipsError("Input data doesn't exist or is invalid");
  }
}

void Scantable2MSReader::finalizeSpecific() {
  std::cout << "Scantabl2MSReader::finalize" << std::endl;
}

Bool Scantable2MSReader::getAntennaRowImpl(AntennaRecord &record) {
  std::cout << "Scantabl2MSReader::getAntennaRowImpl" << std::endl;
  TableRecord const &header = main_table_->keywordSet();

  // header antenna name is constructed as <ObservatoryName>//<AntennaName>@<StationName>
  String header_antenna_name = header.asString("AntennaName");
  String::size_type separator = header_antenna_name.find("//");
  String antenna_name;
  if (separator != String::npos) {
    antenna_name = header_antenna_name.substr(separator + 2);
  } else {
    antenna_name = header_antenna_name;
  }
  separator = antenna_name.find("@");
  if (separator != String::npos) {
    record.station = antenna_name.substr(separator + 1);
    record.name = antenna_name.substr(0, separator);
  } else {
    record.station = "";
    record.name = antenna_name;
  }
  std::cout << "name " << record.name << " station " << record.station
      << std::endl;
  record.dish_diameter = queryAntennaDiameter(record.name);
  std::cout << "diameter " << record.dish_diameter << std::endl;
  record.type = "GROUND-BASED";
  record.mount = "ALT-AZ";

  Vector < Double > antenna_position_value = header.asArrayDouble(
      "AntennaPosition");
  std::cout << "antenna_position " << antenna_position_value << std::endl;
  Vector < Quantity > antenna_position_quant(3);
  antenna_position_quant[0] = Quantity(antenna_position_value[0], "m");
  antenna_position_quant[1] = Quantity(antenna_position_value[1], "m");
  antenna_position_quant[2] = Quantity(antenna_position_value[2], "m");
  record.position = MPosition(MVPosition(antenna_position_quant),
      MPosition::ITRF);
  std::cout << "position";
  record.position.print(std::cout);
  std::cout << std::endl;

  // only one entry so redirect function pointer to noMoreRowImpl
  get_antenna_row_ = &Scantable2MSReader::noMoreRowImpl<AntennaRecord>;

  return True;
}

Bool Scantable2MSReader::getMainRecord(TableRecord &record) {
  std::cout << "Scantabl2MSReader::getMainRecord" << std::endl;

  return False;
}

Bool Scantable2MSReader::getObservationRowImpl(ObservationRecord &record) {
  std::cout << "Scantabl2MSReader::getObservationRowImpl" << std::endl;

  std::cout << "attach column" << std::endl;
  ROScalarColumn < Double > column(*main_table_, "TIME");
  std::cout << "get time list" << std::endl;
  Vector < Double > time_list = column.getColumn();
  if (record.time_range.size() != 2) {
    record.time_range.resize(2);
  }
  minMax(record.time_range[0], record.time_range[1], time_list);
  std::cout << "time_range = " << record.time_range << std::endl;
  TableRecord const &header = main_table_->keywordSet();
  record.observer = header.asString("Observer");
  record.project = header.asString("Project");
  // header antenna name is constructed as <ObservatoryName>//<AntennaName>@<StationName>
  String header_antenna_name = header.asString("AntennaName");
  String::size_type separator = header_antenna_name.find("//");
  if (separator != String::npos) {
    record.telescope_name = header_antenna_name.substr(0, separator);
  } else {
    separator = header_antenna_name.find("@");
    record.telescope_name = header_antenna_name.substr(0, separator);
  }

  // only one entry so redirect function pointer to noMoreRowImpl
  get_observation_row_ = &Scantable2MSReader::noMoreRowImpl<ObservationRecord>;

  return True;
}

Bool Scantable2MSReader::getProcessorRowImpl(ProcessorRecord &record) {
  std::cout << "Scantabl2MSReader::getProcessorRowImpl" << std::endl;

  // just add empty row once

  // only one entry so redirect function pointer to noMoreRowImpl
  get_processor_row_ = &Scantable2MSReader::noMoreRowImpl<ProcessorRecord>;

  return True;
}

Bool Scantable2MSReader::getSourceRow(SourceRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getFieldRow(FieldRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getSpectralWindowRowImpl(
    SpectralWindowRecord &record) {
  POST_START;

  if (!freq_iter_) {
    freq_iter_.reset(new ScantableFrequenciesIterator(*main_table_));
  }

  Bool more_data = freq_iter_->moreData();
  if (more_data) {
    freq_iter_->getEntry(record);
    freq_iter_->next();
  } else {
    // seems to be passed through all the table, deallocate iterator
    freq_iter_.reset(nullptr);
    // and then redirect function pointer to noMoreRowImpl
    get_spw_row_ = &Scantable2MSReader::noMoreRowImpl<SpectralWindowRecord>;
  }

  POST_END;

  return more_data;
}

Bool Scantable2MSReader::getSyscalRow(SysCalRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getWeatherRowImpl(WeatherRecord &record) {
  POST_START;

  if (!weather_iter_) {
    weather_iter_.reset(new ScantableWeatherIterator(*main_table_));
  }

  Bool more_data = weather_iter_->moreData();
  if (more_data) {
    weather_iter_->getEntry(record);
    weather_iter_->next();
  } else {
    // seems to be passed through all the table, deallocate iterator
    weather_iter_.reset(nullptr);
    // and then redirect function pointer to noMoreRowImpl
    get_weather_row_ = &Scantable2MSReader::noMoreRowImpl<WeatherRecord>;
  }

  POST_END;

  return more_data;
}

Bool Scantable2MSReader::getData(size_t irow, TableRecord &record) {
  std::cout << "Scantable2MSReader::getData" << std::endl;

  return False;
}

} //# NAMESPACE CASA - END

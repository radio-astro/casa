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

String getIntent(Int srctype) {
  static std::map<Int, String> intent_map;
  if (intent_map.size() == 0) {
    String sep1 = "#";
    String sep2 = ",";
    String target = "OBSERVE_TARGET";
    String atmcal = "CALIBRATE_ATMOSPHERE";
    String anycal = "CALIBRATE_SOMETHING";
    String onstr = "ON_SOURCE";
    String offstr = "OFF_SOURCE";
    String pswitch = "POSITION_SWITCH";
    String nod = "NOD";
    String fswitch = "FREQUENCY_SWITCH";
    String sigstr = "SIG";
    String refstr = "REF";
    String hot = "HOT";
    String warm = "WARM";
    String cold = "COLD";
    String unspecified = "UNSPECIFIED";
    String ftlow = "LOWER";
    String fthigh = "HIGHER";
    intent_map[0] = target + sep1 + onstr + sep2 + pswitch;
    intent_map[1] = target + sep1 + offstr + sep2 + pswitch;
    intent_map[2] = target + sep1 + onstr + sep2 + nod;
    intent_map[3] = target + sep1 + onstr + sep2 + fswitch + sep1 + sigstr;
    intent_map[4] = target + sep1 + onstr + sep2 + fswitch + sep1 + refstr;
    intent_map[6] = atmcal + sep1 + offstr + sep2 + unspecified;
    intent_map[7] = atmcal + sep1 + hot + sep2 + unspecified;
    intent_map[8] = atmcal + sep1 + warm + sep2 + unspecified;
    intent_map[9] = atmcal + sep1 + cold + sep2 + unspecified;
    intent_map[10] = atmcal + sep1 + onstr + sep2 + pswitch;
    intent_map[11] = atmcal + sep1 + offstr + sep2 + pswitch;
    intent_map[12] = atmcal + sep1 + onstr + sep2 + nod;
    intent_map[13] = atmcal + sep1 + onstr + sep2 + fswitch + sep1 + sigstr;
    intent_map[14] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + refstr;
    intent_map[20] = target + sep1 + onstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[21] = target + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[26] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[27] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[28] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[29] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[30] = target + sep1 + onstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[31] = target + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[36] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[37] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[38] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[39] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[90] = target + sep1 + onstr + sep2 + unspecified;
    intent_map[91] = target + sep1 + offstr + sep2 + unspecified;
    intent_map[92] = anycal + sep1 + offstr + sep2 + unspecified;
  }
  String stype = "UNKNOWN_INTENT";
  auto iter = intent_map.find(srctype);
  if (iter != intent_map.end()) {
    stype = iter->second;
  }
  return stype;
}
}

namespace casa { //# NAMESPACE CASA - BEGIN

Scantable2MSReader::Scantable2MSReader(std::string const &scantable_name) :
    ReaderInterface(scantable_name), main_table_(nullptr), scan_column_(), cycle_column_(), ifno_column_(), polno_column_(), beam_column_(), flagrow_column_(), time_column_(), interval_column_(), srctype_column_(), data_column_(), flag_column_(), direction_column_(), fieldname_column_(), tsys_column_(), tcal_id_column_(), sorted_rows_(), get_antenna_row_(
        &Scantable2MSReader::getAntennaRowImpl), get_field_row_(
        &Scantable2MSReader::getFieldRowImpl), get_observation_row_(
            &Scantable2MSReader::getObservationRowImpl), get_processor_row_(
        &Scantable2MSReader::getProcessorRowImpl), get_source_row_(
        &Scantable2MSReader::getSourceRowImpl), get_spw_row_(
        &Scantable2MSReader::getSpectralWindowRowImpl)//, get_syscal_row_(
        /*&Scantable2MSReader::getSysCalRowImpl)*/, get_weather_row_(
        &Scantable2MSReader::getWeatherRowImpl), field_iter_(nullptr), freq_iter_(
        nullptr), source_iter_(nullptr), /*syscal_iter_(nullptr),*/ weather_iter_(
        nullptr) {
//  std::cout << "Scantabl2MSReader::Scantable2MSReader" << std::endl;
}

Scantable2MSReader::~Scantable2MSReader() {
//  std::cout << "Scantabl2MSReader::~Scantable2MSReader" << std::endl;
}

void Scantable2MSReader::initializeSpecific() {
//  std::cout << "Scantabl2MSReader::initialize" << std::endl;
  File f(name_);
  if (f.exists() and f.isDirectory()) {
    main_table_.reset(new Table(name_, Table::Old));
  } else {
    throw AipsError("Input data doesn't exist or is invalid");
  }

  // attach columns
  scan_column_.attach(*main_table_, "SCANNO");
  cycle_column_.attach(*main_table_, "CYCLENO");
  ifno_column_.attach(*main_table_, "IFNO");
  polno_column_.attach(*main_table_, "POLNO");
  beam_column_.attach(*main_table_, "BEAMNO");
  flagrow_column_.attach(*main_table_, "FLAGROW");
  time_column_.attach(*main_table_, "TIME");
  interval_column_.attach(*main_table_, "INTERVAL");
  srctype_column_.attach(*main_table_, "SRCTYPE");
  data_column_.attach(*main_table_, "SPECTRA");
  flag_column_.attach(*main_table_, "FLAGTRA");
  direction_column_.attach(*main_table_, "DIRECTION");
  fieldname_column_.attach(*main_table_, "FIELDNAME");
  tsys_column_.attach(*main_table_, "TSYS");
  tcal_id_column_.attach(*main_table_, "TCAL_ID");

  // get sort index
  Sort s;
  Vector < Double > time_list = time_column_.getColumn();
  s.sortKey(time_list.data(), TpDouble, 0, Sort::Ascending);
  Vector < uInt > beamno_list = beam_column_.getColumn();
  s.sortKey(beamno_list.data(), TpUInt, 0, Sort::Ascending);
  Vector < uInt > ifno_list = ifno_column_.getColumn();
  s.sortKey(ifno_list.data(), TpUInt, 0, Sort::Ascending);
  Vector < uInt > polno_list = polno_column_.getColumn();
  s.sortKey(polno_list.data(), TpUInt, 0, Sort::Ascending);
  uInt n = s.sort(sorted_rows_, main_table_->nrow());
  if (n != main_table_->nrow()) {
    AipsError("Internal problem when sorting rows");
  }
//  for (uInt i = 0; i < n; ++i) {
//    uInt j = sorted_rows_[i];
//    printf("id %3u row %3u T %10.3f B %2u S %2u P %2u\n", i, j, time_list[j],
//        beamno_list[j], ifno_list[j], polno_list[j]);
//  }
}

void Scantable2MSReader::finalizeSpecific() {
//  std::cout << "Scantabl2MSReader::finalize" << std::endl;
}

Bool Scantable2MSReader::getAntennaRowImpl(AntennaRecord &record) {
//  std::cout << "Scantabl2MSReader::getAntennaRowImpl" << std::endl;
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
  record.dish_diameter = queryAntennaDiameter(record.name);
  record.type = "GROUND-BASED";
  record.mount = "ALT-AZ";

  Vector < Double > antenna_position_value = header.asArrayDouble(
      "AntennaPosition");
  Vector < Quantity > antenna_position_quant(3);
  antenna_position_quant[0] = Quantity(antenna_position_value[0], "m");
  antenna_position_quant[1] = Quantity(antenna_position_value[1], "m");
  antenna_position_quant[2] = Quantity(antenna_position_value[2], "m");
  record.position = MPosition(MVPosition(antenna_position_quant),
      MPosition::ITRF);

  // only one entry so redirect function pointer to noMoreRowImpl
  get_antenna_row_ = &Scantable2MSReader::noMoreRowImpl<AntennaRecord>;

  return True;
}

Bool Scantable2MSReader::getObservationRowImpl(ObservationRecord &record) {
//  std::cout << "Scantabl2MSReader::getObservationRowImpl" << std::endl;

  ROScalarColumn < Double > column(*main_table_, "TIME");
  Vector < Double > time_list = column.getColumn();
  if (record.time_range.size() != 2) {
    record.time_range.resize(2);
  }
  minMax(record.time_range[0], record.time_range[1], time_list);
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

Bool Scantable2MSReader::getProcessorRowImpl(ProcessorRecord &/*record*/) {
//  std::cout << "Scantabl2MSReader::getProcessorRowImpl" << std::endl;

  // just add empty row once

  // only one entry so redirect function pointer to noMoreRowImpl
  get_processor_row_ = &Scantable2MSReader::noMoreRowImpl<ProcessorRecord>;

  return True;
}

Bool Scantable2MSReader::getSourceRowImpl(SourceRecord &record) {
  return getRowImplTemplate(source_iter_, record, get_source_row_);
}

Bool Scantable2MSReader::getFieldRowImpl(FieldRecord &record) {
  return getRowImplTemplate(field_iter_, record, get_field_row_, &field_map_);
}

Bool Scantable2MSReader::getSpectralWindowRowImpl(
    SpectralWindowRecord &record) {
  return getRowImplTemplate(freq_iter_, record, get_spw_row_);
}

Bool Scantable2MSReader::getWeatherRowImpl(WeatherRecord &record) {
  return getRowImplTemplate(weather_iter_, record, get_weather_row_);
}

Bool Scantable2MSReader::getData(size_t irow, TableRecord &record) {
//  std::cout << "Scantable2MSReader::getData(irow=" << irow << ")" << std::endl;

  if (irow >= main_table_->nrow()) {
    return False;
  }

  constexpr double kDay2Sec = 86400.0;
  uInt index = sorted_rows_[irow];
//  std::cout << "Accessing row " << index << std::endl;

  record.define("TIME", time_column_(index) * kDay2Sec);
  record.define("INTERVAL", interval_column_(index));
//  std::cout << "TIME=" << record.asDouble("TIME") << " INTERVAL="
//      << record.asDouble("INTERVAL") << std::endl;
  String intent = getIntent(srctype_column_(index));
  record.define("INTENT", intent);
  record.define("SCAN", (Int) scan_column_(index));
  record.define("SUBSCAN", (Int) cycle_column_(index));
  String field_name = fieldname_column_(index);
  record.define("FIELD_ID", field_map_[field_name]);
  record.define("ANTENNA_ID", (Int) 0);
  record.define("DIRECTION", direction_column_(index));
  record.define("FEED_ID", beam_column_(index));
  record.define("SPECTRAL_WINDOW_ID", ifno_column_(index));
  record.define("POLNO", polno_column_(index));
  record.define("POL_TYPE", main_table_->keywordSet().asString("POLTYPE"));
  record.define("DATA", data_column_(index));
  Vector < uChar > flag = flag_column_(index);
  Vector < Bool > bflag(flag.shape(), False);
  convertArray(bflag, flag);
  record.define("FLAG", bflag);
  uInt flagrow = flagrow_column_(index);
  Bool bflagrow = (flagrow != 0);
  record.define("FLAG_ROW", bflagrow);

  if (tsys_column_.isDefined(index)) {
    Vector<Float> tsys = tsys_column_(index);
    if (!allEQ(tsys, 1.0f) || !allEQ(tsys, 0.0f)) {
      record.define("TSYS", tsys);
    }
  }

  Table const &tcal_table = main_table_->keywordSet().asTable("TCAL");
  Table const &t = tcal_table(tcal_table.col("ID") == tcal_id_column_(index), 1);
  if (t.nrow() > 0) {
    ArrayColumn<Float> tcal_column(t, "TCAL");
    Vector<Float> tcal = tcal_column(0);
    if (!allEQ(tcal, 1.0f) || !allEQ(tcal, 0.0f)) {
      record.define("TCAL", tcal);
    }
  }
  return True;
}

} //# NAMESPACE CASA - END
/*
 * TestReader.h
 *
 *  Created on: Jan 15, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SAMPLE_TESTREADER_H_
#define SINGLEDISH_FILLER_SAMPLE_TESTREADER_H_

#include <singledish/Filler/ReaderInterface.h>

#include <map>

#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/BasicSL/Constants.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/casa/Quanta/MVPosition.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/measures/Measures/MeasConvert.h>
#include <casacore/measures/Measures/MFrequency.h>
#include <casacore/casa/Logging/LogIO.h>

namespace {
void printSubtableMethodDesc(casacore::LogIO &os, casacore::String const &subtable_name) {
  casacore::String capital(subtable_name);
  capital.capitalize();
  casacore::String upcase(subtable_name);
  upcase.upcase();
  os << "get" << capital << "Row is responsible for filling "
      << capital << "Record\n"
    << "which is a struct representation of MS "
    << upcase << " row.\n"
    << "also, it should return true when more rows are available but\n"
    << "return false if no more rows." << casacore::LogIO::POST;
}
}

#define LOG os << casacore::LogOrigin(className_, __func__, WHERE)
#define FLUSH casacore::LogIO::POST

class SampleReader: public casa::ReaderInterface {
public:
  SampleReader(std::string const &name) :
      casa::ReaderInterface(name), className_("SampleReader"),
      direction_frame_(casacore::MDirection::J2000),
      num_main_row_(6),
      observation_row_counter_(0),
      antenna_row_counter_(0),
      processor_row_counter_(0),
      source_row_counter_(0),
      field_row_counter_(0),
      spectral_window_row_counter_(0),
      main_row_counter_(0) {
    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    LOG << "Construct a SampleReader object. Given input data name is \""
      << getName() << "\" (will be ignored)"
      << FLUSH;
  }

  virtual ~SampleReader() {
    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    LOG << "Destruct a SampleReader object." << FLUSH;
  }

  // get number of rows
  virtual size_t getNumberOfRows() {
    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    LOG << "getNumberOfRows should return number of data (scan records)\n"
        << "### return " << num_main_row_ << FLUSH;
    return num_main_row_;
  }

  // query for data type
  virtual casacore::Bool isFloatData() const {
    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    LOG << "isFloatData should return true if the data contains real data\n"
      << "while it should return false if the data is complex\n"
      << "### return true" << FLUSH;
    return true;
  }

  // specify direction frame for MS POINTING table
  virtual casacore::MDirection::Types getDirectionFrame() const {
    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    os << "getDirectionFrame should retun direction frame reference enum\n"
       << "for MS POINTING table" << FLUSH;
    os << "### return " << direction_frame_ << " ("
       << casacore::MDirection::showType(direction_frame_) << ")" << FLUSH;
    return direction_frame_;
  }

    // to get OBSERVATION table
  virtual casacore::Bool getObservationRow(casa::sdfiller::ObservationRecord &record) {
    // number of rows is 1
    const size_t num_rows = 1;
    bool ret = observation_row_counter_ < num_rows;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    if (ret) {
      // row contents: row 0
      // mandatory fields
      casacore::String telescope_name = "NRO45m";
      casacore::Vector<casacore::Double> time_range(2);
      time_range[0] = 1.e9;
      time_range[1] = 2.e9;
      casacore::String observer = "Anonymous";
      casacore::String project = "Merge2";

      // following fields are optional
      casacore::String schedule_type = "Standard";
      casacore::Double release_date = 0.5e9;
      casacore::Vector<casacore::String> schedule(1);
      schedule[0] = "Observe Target";
      casacore::Vector<casacore::String> olog(1);
      olog[0] = "Test Merge2";

      printSubtableMethodDesc(os, "OBSERVATION");
      LOG << "### Telescope Name: " << telescope_name << "\n"
        << "###     Time Range: " << time_range[0] << " ~ " << time_range[1]
        << " MJD sec\n"
        << "###       Observer: " << observer << "\n"
        << "###        Project: " << project << "\n"
        << "###  Schedule Type: " << schedule_type << "\n"
        << "###   Release Date: " << release_date << "\n"
        << "###       Schedule: " << schedule << "\n"
        << "###            Log: " << olog << FLUSH;
      record.telescope_name = telescope_name;
      record.time_range = time_range;
      record.observer = observer;
      record.project = project;
      record.schedule_type = schedule_type;
      record.release_date = release_date;
      record.schedule = schedule;
      record.log = olog;
    }
    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++observation_row_counter_;

    return ret;
  }

  // to get ANTENNA table
  virtual casacore::Bool getAntennaRow(casa::sdfiller::AntennaRecord &record) {
    // number of rows is 1
    const size_t num_rows = 1;
    bool ret = antenna_row_counter_ < num_rows;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    if (ret) {
      // row contents: row 0
      // mandatory fields
      casacore::String name = "NRO45m";
      // NRO 45m antenna position [m] in ITRF
      casacore::Double const posx = -3.8710235e6;
      casacore::Double const posy =  3.4281068e6;
      casacore::Double const posz =  3.7240395e6;
      casacore::MPosition position(casacore::MVPosition(posx, posy, posz),
          casacore::MPosition::ITRF);
      casacore::String antenna_type = "GROUND-BASED";
      casacore::String antenna_mount = "ALT-AZ";
      casacore::Double dish_diameter = 45.0;

      // following fields are optional
      casacore::String station = "Nobeyama";
      // dummy offset value
      casacore::Double const offx = 1.0e5;
      casacore::Double const offy = -2.3e4;
      casacore::Double const offz = 0.6e4;
      casacore::MPosition offset(casacore::MVPosition(offx, offy, offz),
          casacore::MPosition::ITRF);


      printSubtableMethodDesc(os, "ANTENNA");
      LOG << "###          Name: " << name << "\n"
        << "###      Position: " << position.get("m").getValue()
            << "(unit: [m, m, m] Ref: "
            << position.getRefString() << ") \n"
        << "###          Type: " << antenna_type << "\n"
        << "###         Mount: " << antenna_mount << "\n"
        << "### Dish Diameter: " << dish_diameter << "m \n"
        << "###       Station: " << station << "\n"
        << "###        Offset: " << offset.get("m").getValue()
            << "(Unit: [m, m, m] Ref: "
            << offset.getRefString() << ")" << FLUSH;
      record.name = name;
      record.position = position;
      record.type = antenna_type;
      record.mount = antenna_mount;
      record.dish_diameter = dish_diameter;
      record.station = station;
      record.offset = offset;
    }
    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++antenna_row_counter_;
    return ret;
  }

  // to get PROCESSOR table
  virtual casacore::Bool getProcessorRow(casa::sdfiller::ProcessorRecord &record) {
    // number of rows is 1
    const size_t num_rows = 1;
    bool ret = processor_row_counter_ < num_rows;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    if (ret) {
      // row contents: row 0
      // mandatory fields
      casacore::String processor_type = "SPECTROMETER";
      casacore::String processor_subtype = "SAM45";
      casacore::Int type_id = 0;
      casacore::Int mode_id = 8;

      printSubtableMethodDesc(os, "PROCESSOR");
      LOG << "###    Type: " << processor_type << "\n"
        << "### Subtype: " << processor_subtype << "\n"
        << "### Type Id: " << type_id << "\n"
        << "### Mode Id: " << mode_id << FLUSH;
      record.type = processor_type;
      record.sub_type = processor_subtype;
      record.type_id = type_id;
      record.mode_id = mode_id;
    }
    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++processor_row_counter_;
    return ret;
  }

  // to get SOURCE table
  virtual casacore::Bool getSourceRow(casa::sdfiller::SourceRecord &record) {
    // number of rows is 1
    const size_t num_rows = 2;
    bool ret = source_row_counter_ < num_rows;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    if (ret) {
      // mandatory fields
      casacore::Int source_id = -1;
      casacore::Int spw_id = -1;
      casacore::String name = "";
      casacore::Double time = 0.0;
      casacore::Double interval = 0.0;
      casacore::MDirection direction;
      casacore::Int num_lines = 0;

      // following fields are optional
      casacore::String code = "";
      casacore::Int calibration_group = -1;
      casacore::Vector<casacore::String> transition;
      casacore::Vector<casacore::Double> rest_frequency;
      casacore::Vector<casacore::Double> sysvel;
      casacore::Vector<casacore::Double> proper_motion(2);

      if (source_row_counter_ == 0u) {
        printSubtableMethodDesc(os, "SOURCE");

        // first row
        source_id = 0;
        spw_id = 0;
        name = "KUMAMON KL";
        time = 4.3e9;
        interval = 3600.0;
        casacore::Double dirx = 2.0;
        casacore::Double diry = -0.2;
        direction = casacore::MDirection(casacore::MVDirection(dirx, diry),
            casacore::MDirection::J2000);
        num_lines = 1;
        code = "YURU-CHARA";
        calibration_group = 1;
        transition.resize(num_lines);
        transition[0] = "CO(1-0)";
        rest_frequency.resize(num_lines);
        rest_frequency[0] = 1.15271202e11;
        sysvel.resize(num_lines);
        sysvel[0] = 3000.0;
        proper_motion[0] = 1.0e-9;
        proper_motion[1] = 2.3e-11;
      } else if (source_row_counter_ == 1u) {
        // second row
        source_id = 1;
        spw_id = 1;
        name = "Eze";
        time = 4.7e9;
        interval = 1800.0;
        casacore::Double dirx = -1.4;
        casacore::Double diry = 0.05;
        direction = casacore::MDirection(casacore::MVDirection(dirx, diry),
            casacore::MDirection::GALACTIC);
        num_lines = 0;
        code = "France";
        calibration_group = 1;
        transition.resize(num_lines);
        rest_frequency.resize(num_lines);
        sysvel.resize(num_lines);
        proper_motion[0] = 0;
        proper_motion[1] = 0;
      }

      LOG << "###         Source Id: " << source_id << "\n"
        << "###            SPW Id: " << spw_id << "\n"
        << "###              Name: " << name << ") \n"
        << "###              Time: " << time << " MJD sec\n"
        << "###          Interval: " << interval << " sec\n"
        << "###         Direction: " << direction.getAngle("rad").getValue()
        << " (Unit rad Ref: "
        << direction.getRefString() << ") \n"
        << "###              Code: " << code << "\n"
        << "### Calibration Group: " << calibration_group << "\n"
        << "###   Proper motion: " << proper_motion
        << " (Unit rad/s)\n"
        << "### Number of Lines: " << num_lines << FLUSH;
      if (num_lines > 0) {
        LOG << "###   Transition Name: " << transition << "\n"
          << "###    Rest Frequency: " << rest_frequency << " Hz\n"
          << "###   System Velocity: " << sysvel << " m/s" << FLUSH;
      }

      record.source_id = source_id;
      record.spw_id = spw_id;
      record.name = name;
      record.time = time;
      record.interval = interval;
      record.direction = direction;
      record.num_lines = num_lines;
      record.code = code;
      record.calibration_group = calibration_group;
      record.transition = transition;
      record.rest_frequency = rest_frequency;
      record.sysvel = sysvel;
      record.proper_motion = proper_motion;
    }
    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++source_row_counter_;
    return ret;
  }

  // to get FIELD table
  virtual casacore::Bool getFieldRow(casa::sdfiller::FieldRecord &record) {
    // number of rows is 1
    const size_t num_rows = 2;
    bool ret = field_row_counter_ < num_rows;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    if (ret) {
      // meta
      casacore::Int field_id = -1;
      casacore::String source_name = "";
      casacore::MDirection::Types frame = casacore::MDirection::N_Types;

      // mandatory fields
      casacore::String name = "";
      casacore::Double time = -1.0;
      casacore::Matrix<casacore::Double> direction; // can be set as a polynomial in time

      // following fields are optional
      casacore::String code = "";

      if (field_row_counter_ == 0u) {
        printSubtableMethodDesc(os, "FIELD");
        LOG << "*LIMITATION*: direction reference frame must be identical to all \n"
            << "              field entries." << FLUSH;

        // first row
        field_id = 0;
        source_name = "KUMAMON KL";
        frame = casacore::MDirection::J2000;
        name = "KUMAMOTO";
        time = 4.3e9;
        casacore::Double dirx = 2.0;
        casacore::Double diry = -0.2;
        casacore::Double pdirx = 1.0e-8;
        casacore::Double pdiry = -0.7e-8;
        direction.resize(2, 2);
        direction(0, 0) = dirx;
        direction(1, 0) = diry;
        direction(0, 1) = pdirx;
        direction(1, 1) = pdiry;
        code = "YURU-CHARA";
      } else if (field_row_counter_ == 1u) {
        // second row
        field_id = 1;
        source_name = "Eze";
        frame = casacore::MDirection::J2000;
        name = "Botanical Garden";
        time = 4.7e9;
        casacore::Double dirx = -1.4;
        casacore::Double diry = 0.05;
        direction.resize(2, 1);
        direction(0, 0) = dirx;
        direction(1, 0) = diry;
        code = "France";
      }

      LOG << "###    Field Id: " << field_id << "\n"
        << "### Source Name: " << source_name << "\n"
        << "###       Frame: " << casacore::MDirection::showType(frame) << "\n"
        << "###        Name: " << name << ") \n"
        << "###        Time: " << time << " MJD sec\n"
        << "###   Direction: " << direction << " (Unit rad)\n"
        << "###  Code: " << code << FLUSH;

      record.field_id = field_id;
      record.source_name = source_name;
      record.frame = frame;
      record.name = name;
      record.time = time;
      record.direction = direction;
      record.code = code;
    }
    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++field_row_counter_;
    return ret;
  }

  // to get SPECTRAL WINDOW table
  virtual casacore::Bool getSpectralWindowRow(casa::sdfiller::SpectralWindowRecord &record) {
    // number of rows is 1
    const size_t num_rows = 2;
    bool ret = spectral_window_row_counter_ < num_rows;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));
    if (ret) {
      // meta data
      casacore::Int spw_id = -1;

      // mandatory
      casacore::Int num_chan = 0; // number of channels
      casacore::Int meas_freq_ref = (casacore::Int)casacore::MFrequency::Undefined; // frequency reference frame (MFrequency::Types enum)
      casacore::Double refpix = 0; // reference pixel (channel)
      casacore::Double refval = 0; // frequency value at reference pixel
      casacore::Double increment = 0; // frequency increment (can be negative)

      // following fields are optional
      casacore::String name = "";

      if (spectral_window_row_counter_ == 0u) {
        printSubtableMethodDesc(os, "SPECTRAL_WINDOW");
        LOG << "*LIMITATION*: currently regularly spaced frequency setting is supported."
          << FLUSH;

        // first entry
        spw_id = 0;
        num_chan = 4;
        meas_freq_ref = (casacore::Int)casacore::MFrequency::LSRK;
        refpix = 0.0;
        refval = 1.0e9;
        increment = 1.0e8;
        name = "SPW0#LSRK";

      } else if (spectral_window_row_counter_ == 1u) {
        // second entry
        spw_id = 1;
        num_chan = 16;
        meas_freq_ref = (casacore::Int)casacore::MFrequency::TOPO;
        refpix = 7.5;
        refval = 2.0e9;
        increment = -1.0e8;
        name = "SPW1#TOPO";
      }

      LOG << "### SPW Id: " << spw_id << "\n"
          << "### Number of Channels: " << num_chan << ") \n"
          << "### Reference Pixel: " << refpix << "\n"
          << "### Reference Value: " << refval << "\n"
          << "### Increment: " << increment << "m \n"
          << "### Name: " << name << FLUSH;
      record.spw_id = spw_id;
      record.meas_freq_ref = meas_freq_ref;
      record.num_chan = num_chan;
      record.refpix = refpix;
      record.refval = refval;
      record.increment = increment;
      record.name = name;
    }
    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++spectral_window_row_counter_;
    return ret;
  }

  // for DataAccumulator
  virtual casacore::Bool getData(size_t irow, casa::sdfiller::DataRecord &record) {
    bool ret = main_row_counter_ < num_main_row_;

    casacore::LogIO os(casacore::LogOrigin(className_, __func__, WHERE));

    if (ret) {
      // mandatory
      casacore::Double time = -1.0;
      casacore::Double interval = -1.0;
      casacore::Int antenna_id = -1;
      casacore::Int field_id = -1;
      casacore::Int spw_id = -1;
      casacore::Int feed_id = -1;
      casacore::Int scan = -1;
      casacore::Int subscan = -1;
      casacore::Stokes::StokesTypes pol = casacore::Stokes::Undefined;
      casacore::String intent = "";
      casacore::String pol_type = "";
      casacore::Vector<casacore::Double> direction_vector(2);
      casacore::Vector<casacore::Double> scan_rate(2);
      casacore::Vector<casacore::Float> data;
      casacore::Vector<casacore::Bool> flag;
      casacore::Bool flag_row = casacore::False;

      // optional
      casacore::Vector<casacore::Float> tsys;
      casacore::Vector<casacore::Float> tcal;

      casacore::Float temperature = 0.0f;
      casacore::Float pressure = 0.0f;
      casacore::Float rel_humidity = 0.0f;
      casacore::Float wind_speed = 0.0f;
      casacore::Float wind_direction = 0.0f;

      if (irow == 0u) {
        LOG << "getData is responsible for constructing MS MAIN row. It should fill \n"
            << "spectral data as well as various kinds of associating information \n"
            << "including timestamp, Tsys, weather, reference to MS subtables, etc.\n"
            << "*NOTE*: record contents must be consistent with corresponding \n"
            << "        subtable row!!!\n"
            << "*NOTE*: boolean flag is marked as true if data is invalid.\n"
            << "*NOTE*: flag_row is obsolete."
            << FLUSH;

        // first row
        time = 4.0e9;
        interval = 1.0;
        antenna_id = 0;
        field_id = 0;
        spw_id = 0;
        feed_id = 0;
        scan = 3;
        subscan = 0;
        pol = casacore::Stokes::XX;
        intent = "ON_SOURCE";
        pol_type = "linear";
        direction_vector[0] = 2.0 + 1.e-4;
        direction_vector[1] = -0.4 + 1.e-4;
        scan_rate = 0.0;
        size_t num_chan = 4;
        record.setDataSize(num_chan);
        data.resize(num_chan);
        flag.resize(num_chan);
        data = 0.15;
        flag = casacore::False;
        flag_row = casacore::False;
        record.setTsysSize(1); // unchannelized Tsys
        tsys.resize(1);
        tsys = 300.0;
        temperature = 280.0f;
        pressure = 900.0f;
        rel_humidity = 0.2;
        wind_speed = 5.0f;
        wind_direction = casacore::C::pi_2; // PI/2
      } else if (irow == 1u) {
        // second row is a YY counterpart of the first row
        // so most of the meta data are identical to the first row
        time = 4.0e9;
        interval = 1.0;
        antenna_id = 0;
        field_id = 0;
        spw_id = 0;
        feed_id = 0;
        scan = 3;
        subscan = 0;
        pol = casacore::Stokes::YY;
        intent = "ON_SOURCE";
        pol_type = "linear";
        direction_vector[0] = 2.0 + 1.e-4;
        direction_vector[1] = -0.4 + 1.e-4;
        scan_rate = 0.0;
        size_t num_chan = 4;
        record.setDataSize(num_chan);
        data.resize(num_chan);
        flag.resize(num_chan);
        data = 0.18;
        flag = casacore::False;
        flag[2] = casacore::True;
        flag_row = casacore::False;
        record.setTsysSize(1); // unchannelized Tsys
        tsys.resize(1);
        tsys = 380.0;
        temperature = 280.0f;
        pressure = 900.0f;
        rel_humidity = 0.2;
        wind_speed = 5.0f;
        wind_direction = casacore::C::pi_2; // PI/2
      } else if (irow == 2u) {
        // third row has the same timestamp with first/second rows
        // but observed with different spw id
        time = 4.0e9;
        interval = 1.0;
        antenna_id = 0;
        field_id = 0;
        spw_id = 1;
        feed_id = 0;
        scan = 3;
        subscan = 0;
        pol = casacore::Stokes::XX;
        intent = "ON_SOURCE";
        pol_type = "linear";
        direction_vector[0] = 2.0 + 1.e-4;
        direction_vector[1] = -0.4 + 1.e-4;
        scan_rate = 0.0;
        size_t num_chan = 16;
        record.setDataSize(num_chan);
        data.resize(num_chan);
        flag.resize(num_chan);
        data = 0.18;
        flag = casacore::False;
        flag[2] = casacore::True;
        flag_row = casacore::False;
        record.setTsysSize(num_chan); // channelized Tsys
        tsys.resize(num_chan);
        tsys = 500.0;
        temperature = 280.0f;
        pressure = 900.0f;
        rel_humidity = 0.2;
        wind_speed = 5.0f;
        wind_direction = casacore::C::pi_2; // PI/2
      } else if (irow == 3u) {
        // fourth row goes to next timestamp
        time = 4.0e9 + 1.0;
        interval = 1.0;
        antenna_id = 0;
        field_id = 0;
        spw_id = 0;
        feed_id = 0;
        scan = 3;
        subscan = 1;
        pol = casacore::Stokes::XX;
        intent = "OFF_SOURCE";
        pol_type = "linear";
        direction_vector[0] = 2.5;
        direction_vector[1] = -0.4;
        scan_rate = 0.0;
        size_t num_chan = 4;
        record.setDataSize(num_chan);
        data.resize(num_chan);
        flag.resize(num_chan);
        data = 0.02;
        flag = casacore::False;
        flag_row = casacore::False;
        record.setTsysSize(1); // unchannelized Tsys
        tsys.resize(1);
        tsys = 303.0;
        temperature = 281.0f;
        pressure = 899.0f;
        rel_humidity = 0.201;
        wind_speed = 5.1f;
        wind_direction = casacore::C::pi_2 - 0.01;
      } else if (irow == 4u) {
        // fifth row is a YY counterpart of the fourth row
        time = 4.0e9 + 1.0;
        interval = 1.0;
        antenna_id = 0;
        field_id = 0;
        spw_id = 0;
        feed_id = 0;
        scan = 3;
        subscan = 1;
        pol = casacore::Stokes::YY;
        intent = "OFF_SOURCE";
        pol_type = "linear";
        direction_vector[0] = 2.5;
        direction_vector[1] = -0.4;
        scan_rate = 0.0;
        size_t num_chan = 4;
        record.setDataSize(num_chan);
        data.resize(num_chan);
        flag.resize(num_chan);
        data = 0.07;
        flag = casacore::False;
        flag_row = casacore::False;
        record.setTsysSize(1); // unchannelized Tsys
        tsys.resize(1);
        tsys = 313.0;
        temperature = 281.0f;
        pressure = 899.0f;
        rel_humidity = 0.201;
        wind_speed = 5.1f;
        wind_direction = casacore::C::pi_2 - 0.01;
      } else if (irow == 5u) {
        time = 4.0e9 + 1.0;
        interval = 1.0;
        antenna_id = 0;
        field_id = 0;
        spw_id = 1;
        feed_id = 0;
        scan = 3;
        subscan = 1;
        pol = casacore::Stokes::XX;
        intent = "OFF_SOURCE";
        pol_type = "linear";
        direction_vector[0] = 2.5;
        direction_vector[1] = -0.4;
        scan_rate = 0.0;
        size_t num_chan = 16;
        record.setDataSize(num_chan);
        data.resize(num_chan);
        flag.resize(num_chan);
        data = 0.09;
        flag = casacore::False;
        flag[2] = casacore::True;
        flag_row = casacore::False;
        record.setTsysSize(num_chan); // channelized Tsys
        tsys.resize(num_chan);
        tsys = 517.0;
        temperature = 281.0f;
        pressure = 899.0f;
        rel_humidity = 0.201;
        wind_speed = 5.1f;
        wind_direction = casacore::C::pi_2 - 0.01;
      }

      LOG << "###          Row: " << irow << "\n"
          << "###         Time: " << time << " MJD sec \n"
          << "###     Interval: " << interval << " sec \n"
          << "###   Antenna Id: " << antenna_id << "\n"
          << "###     Field Id: " << field_id << "\n"
          << "###       SPW Id: " << spw_id << "\n"
          << "###      Feed Id: " << feed_id << "\n"
          << "###         Scan: " << scan << "\n"
          << "###      Subscan: " << subscan << "\n"
          << "### Polarization: " << casacore::Stokes::name(pol) << "\n"
          << "###     Pol Type: " << pol_type << "\n"
          << "###       Intent: " << intent << "\n"
          << "###    Direction: " << direction_vector
          << " (Unit: rad Ref " << direction_frame_
          << ")\n"
          << "###    Scan Rate: " << scan_rate << " (Unit rad/sec)\n"
          << "###         Data: " << data << "\n"
          << "###         Flag: " << flag << "\n"
          << "###     Flag Row: " << flag_row << " (obsolete)\n"
          << "### Tsys: " << tsys << " (Unit: K)\n"
          << "### Temperature: " << temperature << " K\n"
          << "### Pressure: " << pressure << " hPa\n"
          << "### Rel Humidity: " << rel_humidity << "\n"
          << "### Wind Speed: " << wind_speed << " m/s\n"
          << "### Wind Direction: " << wind_direction << " rad\n"
          << FLUSH;


      record.time = time;
      record.interval = interval;
      record.antenna_id = antenna_id;
      record.field_id = field_id;
      record.spw_id = spw_id;
      record.feed_id = feed_id;
      record.scan = scan;
      record.subscan = subscan;
      record.pol = pol;
      record.intent = intent;
      record.pol_type = pol_type;
      record.direction_vector = direction_vector;
      record.scan_rate = scan_rate;
      record.data = data;
      record.flag = flag;
      record.flag_row = flag_row;
      record.tsys = tsys;
      record.temperature = temperature;
      record.pressure = pressure;
      record.rel_humidity = rel_humidity;
      record.wind_speed = wind_speed;
      record.wind_direction = wind_direction;
    }

    LOG << "### return " << ((ret) ? "true" : "false") << FLUSH;

    ++main_row_counter_;
    return ret;
  }

protected:
  void initializeSpecific() {
    casacore::LogIO os;
    LOG << "initializeSpecific is intended to execute any initialization\n"
      << "specific to the site, telescope, and so on. It can be empty\n"
      << "if no specific initialization is required."<< FLUSH;
  }

  void finalizeSpecific() {
    casacore::LogIO os;
    LOG << "finalizeSpecific is intended to execute any finalization\n"
      << "specific to the site, telescope, and so on. It can be empty\n"
      << "if no specific finalization is required." << FLUSH;
  }

private:
  std::string const className_;
  casacore::MDirection::Types const direction_frame_;
  size_t const num_main_row_;
  size_t observation_row_counter_;
  size_t antenna_row_counter_;
  size_t processor_row_counter_;
  size_t source_row_counter_;
  size_t field_row_counter_;
  size_t spectral_window_row_counter_;
  size_t main_row_counter_;
};

#endif /* SINGLEDISH_FILLER_SAMPLE_TESTREADER_H_ */

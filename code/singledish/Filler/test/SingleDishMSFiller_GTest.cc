/*
 * SingleDishMSFiller_Gtest.cc
 *
 *  Created on: Jan 8, 2016
 *      Author: nakazato
 */

#include <gtest/gtest.h>

#include <singledish/Filler/SingleDishMSFiller.h>
#include <singledish/Filler/ReaderInterface.h>
#include <singledish/Filler/Scantable2MSReader.h>
#include <singledish/Filler/test/TestReader.h>
#include <singledish/SingleDish/test/SingleDishTestUtil.h>

#include <string>
#include <iostream>
#include <memory>

#include <casacore/casa/OS/Time.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/OS/RegularFile.h>
#include <casacore/casa/OS/SymLink.h>
#include <casacore/casa/OS/Directory.h>
#include <casacore/casa/OS/DirectoryIterator.h>
#include <casacore/casa/Arrays/ArrayLogical.h>
#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Quanta/MVPosition.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/ms/MeasurementSets/MSObsColumns.h>
#include <casacore/ms/MeasurementSets/MSProcessorColumns.h>
#include <casacore/ms/MeasurementSets/MSAntennaColumns.h>
#include <casacore/ms/MeasurementSets/MSDataDescColumns.h>
#include <casacore/ms/MeasurementSets/MSFeedColumns.h>
#include <casacore/ms/MeasurementSets/MSFieldColumns.h>
#include <casacore/ms/MeasurementSets/MSMainColumns.h>
#include <casacore/ms/MeasurementSets/MSPolColumns.h>
#include <casacore/ms/MeasurementSets/MSSourceColumns.h>
#include <casacore/ms/MeasurementSets/MSSpWindowColumns.h>
#include <casacore/ms/MeasurementSets/MSStateColumns.h>
#include <casacore/ms/MeasurementSets/MSSysCalColumns.h>
#include <casacore/ms/MeasurementSets/MSWeatherColumns.h>

using namespace casa;
using namespace casacore;

#define POST_START std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl
#define POST_END std::cout << "End " << __PRETTY_FUNCTION__ << std::endl

#define CASA_EXPECT_STREQ(expected, actual) EXPECT_STREQ((expected).c_str(), (actual).c_str())
#define CASA_ASSERT_STREQ(expected, actual) ASSERT_STREQ((expected).c_str(), (actual).c_str())
#define CASA_EXPECT_ARRAYEQ(expected, actual) \
  EXPECT_TRUE((expected).conform((actual))); \
  if (!(expected).empty()) { \
    EXPECT_TRUE(allEQ((expected), (actual))); \
  }

class SingleDishMSFillerTestBase: public ::testing::Test {
public:
  virtual void SetUp() {
    std::cout << "this is SetUp" << std::endl;

    //my_ms_name_ = "mytest.ms";
    //my_data_name_ = "mytest.asap";
    my_ms_name_ = getMSName();
    my_data_name_ = getDataName();
    std::string const data_path = test_utility::GetCasaDataPath()
        + "/regression/unittest/sdsave/";

    copyData(data_path);
    ASSERT_TRUE(File(my_data_name_).exists());
    deleteTable(my_ms_name_);
  }

  virtual void TearDown() {
    std::cout << "this is TearDown" << std::endl;

    //cleanup();
  }

protected:
  template<class _Filler>
  void ExecuteFiller(_Filler &filler) {
    std::string const &data_name = filler.getDataName();

    EXPECT_STREQ(data_name.c_str(), my_data_name_.c_str());

    // fill MS
    filler.fill();

    // save MS
    filler.save(my_ms_name_);

    // file existence check
    ASSERT_PRED1([](std::string const &name) {
      File file(name);
      return file.exists();
    }, my_ms_name_);
  }

  std::string my_ms_name_;
  std::string my_data_name_;

  virtual std::string getDataName() = 0;
  virtual std::string getMSName() = 0;

private:
  void copyRegular(String const &src, String const &dst) {
//    std::cout << "copyRegular: src " << src << " dst " << dst << std::endl;
    RegularFile r(src);
    r.copy(dst);
  }
  void copySymLink(String const &src, String const &dst) {
    Path p = SymLink(src).followSymLink();
    String actual_src = p.absoluteName();
    File f(actual_src);
//    std::cout << "copySymLink: actual_src " << actual_src << " dst " << dst << std::endl;
    if (f.isRegular()) {
      copyRegular(actual_src, dst);
    } else if (f.isDirectory()) {
      copyDirectory(actual_src, dst);
    }
  }
  void copyDirectory(String const &src, String const &dst) {
    Directory dsrc(src);
    Directory ddst(dst);
    ddst.create();
    DirectoryIterator iter(dsrc);
    while (!iter.pastEnd()) {
      String name = iter.name();
      if (name.contains(".svn")) {
        iter++;
        continue;
      }
//      std::cout << "name " << name << std::endl;
      File f = iter.file();
      Path psrc(src);
      Path pdst(dst);
      psrc.append(name);
      String sub_src = psrc.absoluteName();
      pdst.append(name);
      String sub_dst = pdst.absoluteName();
//      std::cout << "copyDirectory: sub_src " << sub_src << " sub_dst " << sub_dst << std::endl;
      if (f.isSymLink()) {
        std::cout << name << " is symlink" << std::endl;
        copySymLink(sub_src, sub_dst);
      } else if (f.isRegular()) {
//        std::cout << name << " is regular file" << std::endl;
        copyRegular(sub_src, sub_dst);
      } else if (f.isDirectory()) {
//        std::cout << name << " is directory" << std::endl;
        copyDirectory(sub_src, sub_dst);
      }
      iter++;
    }
  }
  void copyData(std::string const &data_dir) {
    if (my_data_name_.size() > 0) {
//      std::cout << "Copying " << my_data_name_ << " from data repository"
//          << std::endl;
      std::string full_path = data_dir + my_data_name_;
//      std::cout << "full_path = " << full_path << std::endl;
      std::string work_path = my_data_name_;
      File f(full_path);
      ASSERT_TRUE(f.exists());
      if (f.isSymLink()) {
        copySymLink(full_path, work_path);
      } else if (f.isRegular()) {
        copyRegular(full_path, work_path);
      } else if (f.isDirectory()) {
        copyDirectory(full_path, work_path);
      }
    }
  }
  void cleanup() {
    File f(my_data_name_);
    if (f.isRegular()) {
      RegularFile r(my_data_name_);
      r.remove();
    } else if (f.isDirectory()) {
      Directory d(my_data_name_);
      d.removeRecursive();
    }
    deleteTable(my_ms_name_);
  }
  void deleteTable(std::string const &name) {
    File file(name);
    if (file.exists()) {
      std::cout << "Removing " << name << std::endl;
      Table::deleteTable(name, True);
    }
  }
};

class SingleDishMSFillerTestWithStub: public SingleDishMSFillerTestBase {
  virtual std::string getDataName() {
    return "";
  }
  virtual std::string getMSName() {
    return "mytest.ms";
  }
};

class SingleDishMSFillerTestFloat: public SingleDishMSFillerTestBase {
  virtual std::string getDataName() {
    return "data_selection.asap";
  }
  virtual std::string getMSName() {
    return "floatdata.ms";
  }
};

class SingleDishMSFillerTestComplex: public SingleDishMSFillerTestBase {
  virtual std::string getDataName() {
    return "crosspoltest.asap";
  }
  virtual std::string getMSName() {
    return "complexdata.ms";
  }
};

TEST_F(SingleDishMSFillerTestFloat, FillerTest) {
  // Create filler
  SingleDishMSFiller<Scantable2MSReader> filler(my_data_name_);

  // Run filler
  ExecuteFiller(filler);

  // verify table contents
  MeasurementSet myms(my_ms_name_);
  Table myscantable(my_data_name_, Table::Old);
  TableRecord const &scantable_header = myscantable.keywordSet();

  // Verify OBSERVATION table
  {
    std::cout << "Verify OBSERVATION table" << std::endl;
    auto const mytable = myms.observation();
    ASSERT_EQ(uInt(1), mytable.nrow());
    ROMSObservationColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(scantable_header.asString("Observer"),
        mycolumns.observer()(0));
    CASA_EXPECT_STREQ(scantable_header.asString("Project"),
        mycolumns.project()(0));
    String antenna_name = scantable_header.asString("AntennaName");
    String expected_telescope_name;
    String::size_type pos = antenna_name.find("//");
    if (pos != String::npos) {
      expected_telescope_name = antenna_name.substr(0, pos);
    } else {
      expected_telescope_name = antenna_name.substr(0, antenna_name.find("@"));
    }
    CASA_EXPECT_STREQ(expected_telescope_name, mycolumns.telescopeName()(0));
    ROScalarColumn < Double > column(myscantable, "TIME");
    Vector < Double > time_list = column.getColumn();
    Vector < Double > time_range = mycolumns.timeRange()(0);
    ASSERT_EQ(2u, time_range.nelements());
    EXPECT_EQ(min(time_list), time_range[0]);
    EXPECT_EQ(max(time_list), time_range[1]);
  }

  // verify PROCESSOR table
  {
    std::cout << "Verify PROCESSOR table" << std::endl;
    auto const mytable = myms.processor();
    // just to confirm it has only one row
    ASSERT_EQ(uInt(1), mytable.nrow());
  }

  // verify ANTENNA table
  {
    std::cout << "Verify ANTENNA table" << std::endl;
    auto const mytable = myms.antenna();
    ASSERT_EQ(uInt(1), mytable.nrow());
    ROMSAntennaColumns mycolumns(mytable);
    String header_antenna_name = scantable_header.asString("AntennaName");
    String antenna_name;
    String expected_name;
    String expected_station;
    String::size_type pos = header_antenna_name.find("//");
    if (pos != String::npos) {
      antenna_name = header_antenna_name.substr(pos + 2);
    } else {
      antenna_name = header_antenna_name;
    }
    pos = antenna_name.find("@");
    if (pos != String::npos) {
      expected_name = antenna_name.substr(0, pos);
      expected_station = antenna_name.substr(pos + 1);
    } else {
      expected_name = antenna_name;
      expected_station = "";
    }
    CASA_EXPECT_STREQ(expected_name, mycolumns.name()(0));
    CASA_EXPECT_STREQ(expected_station, mycolumns.station()(0));
    CASA_EXPECT_STREQ(String("GROUND-BASED"), mycolumns.type()(0));
    CASA_EXPECT_STREQ(String("ALT-AZ"), mycolumns.mount()(0));
    auto const position_meas_column = mycolumns.positionMeas();
    auto const position = position_meas_column(0);
    auto const position_val = position.get("m");
    Vector < Double > expected_position = scantable_header.asArrayDouble(
        "AntennaPosition");
    CASA_EXPECT_STREQ(String("ITRF"), position.getRefString());
    EXPECT_TRUE(allEQ(expected_position, position_val.getValue()));
    EXPECT_EQ(12.0, mycolumns.dishDiameter()(0));
  }

  // verify WEATHER table
  {
    std::cout << "Verify WEATHER table" << std::endl;
    auto const mytable = myms.weather();
    auto const weather_table = scantable_header.asTable("WEATHER");
    uInt expected_nrow = weather_table.nrow();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSWeatherColumns mycolumns(mytable);
    ROTableRow row(weather_table);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      TableRecord row_record = row.get(0);
      uInt weather_id = row_record.asuInt("ID");
      auto subtable = myscantable(myscantable.col("WEATHER_ID") == weather_id);
      ROScalarColumn < Double > col(subtable, "TIME");
      Double time_min, time_max;
      minMax(time_min, time_max, col.getColumn());
      EXPECT_EQ(0.5 * (time_min + time_max), mycolumns.time()(i));
      EXPECT_EQ(time_max - time_min, mycolumns.interval()(i));
      EXPECT_EQ(0, mycolumns.antennaId()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("TEMPERATURE"),
          mycolumns.temperature()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("PRESSURE"), mycolumns.pressure()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("HUMIDITY"),
          mycolumns.relHumidity()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("WINDSPEED"),
          mycolumns.windSpeed()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("WINDAZ"),
          mycolumns.windDirection()(i));
    }
  }
}

TEST_F(SingleDishMSFillerTestWithStub, FillerTest) {
  // Create filler
  SingleDishMSFiller<TestReader<FloatDataStorage> > filler(my_data_name_);

  // Run filler
  ExecuteFiller(filler);

  // verify table contents
  MeasurementSet myms(my_ms_name_);
  auto const &reader = filler.getReader();

  // verify OBSERVATION table
  {
    std::cout << "Verify OBSERVATION table" << std::endl;
    auto const mytable = myms.observation();
    ASSERT_EQ(uInt(1), mytable.nrow());
    TableRecord const &expected_record = reader.observation_record_;
    ROMSObservationColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(expected_record.asString("OBSERVER"),
        mycolumns.observer()(0));
    CASA_EXPECT_STREQ(expected_record.asString("PROJECT"),
        mycolumns.project()(0));
    CASA_EXPECT_STREQ(expected_record.asString("TELESCOPE_NAME"),
        mycolumns.telescopeName()(0));
    CASA_EXPECT_STREQ(expected_record.asString("OBSERVER"),
        mycolumns.observer()(0));
    CASA_EXPECT_STREQ(expected_record.asString("SCHEDULE_TYPE"),
        mycolumns.scheduleType()(0));
    EXPECT_EQ(expected_record.asDouble("RELEASE_DATE"),
        mycolumns.releaseDate()(0));
    EXPECT_TRUE(
        allEQ(expected_record.asArrayDouble("TIME_RANGE"),
            mycolumns.timeRange()(0)));
    Vector < String > mylog = mycolumns.log()(0);
    EXPECT_EQ(size_t(1), mylog.size());
    CASA_EXPECT_STREQ(expected_record.asArrayString("LOG")(IPosition(1, 0)),
        mylog[0]);
    Vector < String > myschedule = mycolumns.schedule()(0);
    EXPECT_EQ(size_t(1), myschedule.size());
    CASA_EXPECT_STREQ(
        expected_record.asArrayString("SCHEDULE")(IPosition(1, 0)),
        myschedule[0]);
  }

  // verify PROCESSOR table
  {
    std::cout << "Verify PROCESSOR table" << std::endl;
    auto const mytable = myms.processor();
    ASSERT_EQ(uInt(1), mytable.nrow());
    TableRecord const &expected_record = reader.processor_record_;
    ROMSProcessorColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(expected_record.asString("TYPE"), mycolumns.type()(0));
    CASA_EXPECT_STREQ(expected_record.asString("SUB_TYPE"),
        mycolumns.subType()(0));
    EXPECT_EQ(expected_record.asInt("TYPE_ID"), mycolumns.typeId()(0));
    EXPECT_EQ(expected_record.asInt("MODE_ID"), mycolumns.modeId()(0));
  }

  // verify ANTENNA table
  {
    std::cout << "Verify ANTENNA table" << std::endl;
    auto const mytable = myms.antenna();
    ASSERT_EQ(uInt(1), mytable.nrow());
    TableRecord const &expected_record = reader.antenna_record_;
    ROMSAntennaColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(expected_record.asString("NAME"), mycolumns.name()(0));
    CASA_EXPECT_STREQ(expected_record.asString("STATION"),
        mycolumns.station()(0));
    CASA_EXPECT_STREQ(expected_record.asString("TYPE"), mycolumns.type()(0));
    CASA_EXPECT_STREQ(expected_record.asString("MOUNT"), mycolumns.mount()(0));
    auto const position_meas_column = mycolumns.positionMeas();
    auto const position = position_meas_column(0);
    auto const position_val = position.get("m");
    CASA_EXPECT_STREQ(String("ITRF"), position.getRefString());
    EXPECT_TRUE(
        allEQ(Vector < Double > (expected_record.asArrayDouble("POSITION")),
            position_val.getValue()));
    auto const offset_meas_column = mycolumns.offsetMeas();
    auto const offset = offset_meas_column(0);
    auto const offset_val = offset.get("m");
    CASA_EXPECT_STREQ(String("ITRF"), offset.getRefString());
    EXPECT_TRUE(
        allEQ(expected_record.asArrayDouble("OFFSET"), offset_val.getValue()));
    EXPECT_EQ(expected_record.asDouble("DISH_DIAMETER"),
        mycolumns.dishDiameter()(0));
  }

  // verify SOURCE table
  Record source_map;
  {
    std::cout << "verify SOURCE table" << std::endl;
    auto const mytable = myms.source();
    std::map<uInt, SourceRecord> expected_record = reader.source_record_;
    uInt expected_nrow = expected_record.size();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSSourceColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      SourceRecord const row_record = expected_record[i];
      Int source_id = row_record.source_id;
      String source_name = row_record.name;
      EXPECT_EQ(source_id, mycolumns.sourceId()(i));
      CASA_EXPECT_STREQ(source_name, mycolumns.name()(i));
      if (!source_map.isDefined(source_name)) {
        source_map.define(source_name, source_id);
      }
      EXPECT_EQ(row_record.time, mycolumns.time()(i));
      EXPECT_EQ(row_record.interval, mycolumns.interval()(i));
      EXPECT_EQ(row_record.spw_id, mycolumns.spectralWindowId()(i));
      EXPECT_EQ(row_record.calibration_group, mycolumns.calibrationGroup()(i));
      CASA_EXPECT_STREQ(row_record.code, mycolumns.code()(i));
      EXPECT_EQ(row_record.num_lines, mycolumns.numLines()(i));
      MDirection expected_direction = row_record.direction;
      MDirection actual_direction = mycolumns.directionMeas()(i);

      expected_direction.print(std::cout);
      std::cout << " with ref " << expected_direction.getRefString()
          << std::endl;
      actual_direction.print(std::cout);
      std::cout << " with ref " << actual_direction.getRefString() << std::endl;
      Array<Double> expected_array = expected_direction.getAngle().getValue(
          "rad");
      Array<Double> array = mycolumns.direction()(i);
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.proper_motion);
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.properMotion().isDefined(i));
      } else {
        array.assign(mycolumns.properMotion()(i));
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
      expected_array.assign(row_record.rest_frequency);
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.restFrequency().isDefined(i));
      } else {
        array.assign(mycolumns.restFrequency()(i));
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
      expected_array.assign(row_record.sysvel);
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.sysvel().isDefined(i));
      } else {
        array.assign(mycolumns.sysvel()(i));
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
      Array<String> expected_transition = row_record.transition;
      if (expected_transition.size() == 0) {
        EXPECT_FALSE(mycolumns.transition().isDefined(i));
      } else {
        Array<String> transition = mycolumns.transition()(i);
        CASA_EXPECT_ARRAYEQ(expected_transition, transition);
      }
    }
  }

  // verify FIELD table
  {
    std::cout << "verify FIELD table" << std::endl;
    auto const mytable = myms.field();
    std::map<uInt, FieldRecord> expected_record = reader.field_record_;
    uInt num_records = expected_record.size();
    ROMSFieldColumns mycolumns(mytable);
    uInt expected_nrow = 0;
    std::vector<uInt> processed_rows;
    for (uInt i = 0; i < num_records; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      FieldRecord const row_record = expected_record[i];
      Int field_id = row_record.field_id;
      std::cout << "field " << field_id << std::endl;
      processed_rows.push_back((uInt) field_id);
      ASSERT_GE(field_id, 0);
      if ((uInt)(field_id + 1) > expected_nrow) {
        expected_nrow = field_id + 1;
      }
      EXPECT_EQ(row_record.time, mycolumns.time()(field_id));
      EXPECT_EQ(row_record.direction.shape()[1] - 1,
          mycolumns.numPoly()(field_id));
      Int source_id = -1;
      String source_name = row_record.source_name;
      if (source_map.isDefined(source_name)) {
        source_id = source_map.asInt(source_name);
      }
      ASSERT_GE(source_id, 0);
      EXPECT_EQ(source_id, mycolumns.sourceId()(field_id));
      CASA_EXPECT_STREQ(row_record.name, mycolumns.name()(field_id));
      CASA_EXPECT_STREQ(row_record.code, mycolumns.code()(field_id));
      Matrix<Double> const direction = row_record.direction;
      EXPECT_TRUE(allEQ(direction, mycolumns.delayDir()(field_id)));
      EXPECT_TRUE(allEQ(direction, mycolumns.phaseDir()(field_id)));
      EXPECT_TRUE(allEQ(direction, mycolumns.referenceDir()(field_id)));
    }
    ASSERT_EQ(expected_nrow, mytable.nrow());
    for (uInt i = 0; i < expected_nrow; ++i) {
      if (std::count(processed_rows.begin(), processed_rows.end(), i) == 0) {
        std::cout << "Verifying empty field " << i << std::endl;
        Int const num_poly = 0;
        Matrix<Double> const dummy(2, 1, 0.0);
        EXPECT_EQ(num_poly, mycolumns.numPoly()(i));
        EXPECT_TRUE(allEQ(dummy, mycolumns.delayDir()(i)));
        EXPECT_TRUE(allEQ(dummy, mycolumns.phaseDir()(i)));
        EXPECT_TRUE(allEQ(dummy, mycolumns.referenceDir()(i)));
      }
    }

  }

  // verify SPECTRAL_WINDOW table
  std::map<Int, Int> num_chan_map;
  {
    std::cout << "Verify SPECTRAL_WINDOW table" << std::endl;
    auto const mytable = myms.spectralWindow();
    map<uInt, SpectralWindowRecord> expected_record = reader.spw_record_;
    uInt num_records = expected_record.size();
    ROMSSpWindowColumns mycolumns(mytable);
    uInt expected_nrow = 0;
    std::vector<uInt> processed_rows;
    for (uInt i = 0; i < num_records; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      SpectralWindowRecord row_record = expected_record[i];
      Int spw_id = row_record.spw_id;
      std::cout << "spw " << spw_id << std::endl;
      processed_rows.push_back((uInt) spw_id);
      ASSERT_GE(spw_id, 0);
      if ((uInt)(spw_id + 1) > expected_nrow) {
        expected_nrow = spw_id + 1;
      }
      EXPECT_EQ(row_record.meas_freq_ref, mycolumns.measFreqRef()(spw_id));
      Int num_chan = row_record.num_chan;
      EXPECT_EQ(num_chan, mycolumns.numChan()(spw_id));
      num_chan_map[spw_id] = num_chan;
      CASA_EXPECT_STREQ(row_record.name, mycolumns.name()(spw_id));
      Double expected_refpix = row_record.refpix;
      Double expected_refval = row_record.refval;
      Double expected_increment = row_record.increment;
      Int expected_net_sideband = (expected_increment < 0.0) ? 1 : 0;
      EXPECT_EQ(expected_net_sideband, mycolumns.netSideband()(spw_id));
      Vector < Double > chan_freq = mycolumns.chanFreq()(spw_id);
      Double freq0 = chan_freq[0];
      Double chan0 = 0;
      Vector < Double > chan_width = mycolumns.chanWidth()(spw_id);
      Double incr0 = chan_width[0];
      Double refval = freq0 + expected_refpix * incr0;
      EXPECT_EQ(expected_refval, refval);
      EXPECT_TRUE(allEQ(chan_width, expected_increment));
      Vector < Double > resolution = mycolumns.resolution()(spw_id);
      EXPECT_TRUE(allEQ(resolution, abs(expected_increment)));
      Vector < Double > effective_bw = mycolumns.effectiveBW()(spw_id);
      EXPECT_TRUE(allEQ(effective_bw, abs(expected_increment)));
      Double ref_freq = mycolumns.refFrequency()(spw_id);
      EXPECT_EQ(chan_freq[0], ref_freq);
    }
    ASSERT_EQ(expected_nrow, mytable.nrow());
    for (uInt i = 0; i < expected_nrow; ++i) {
      if (std::count(processed_rows.begin(), processed_rows.end(), i) == 0) {
        std::cout << "Verifying empty spw " << i << std::endl;
        Int const one_chan = 1;
        Vector<Double> const dummy(1, 0.0);
        EXPECT_EQ(one_chan, mycolumns.numChan()(i));
        EXPECT_TRUE(allEQ(dummy, mycolumns.chanFreq()(i)));
        EXPECT_TRUE(allEQ(dummy, mycolumns.chanWidth()(i)));
        EXPECT_TRUE(allEQ(dummy, mycolumns.resolution()(i)));
        EXPECT_TRUE(allEQ(dummy, mycolumns.effectiveBW()(i)));
      }
    }
  }

  // verify SYSCAL table
  {
    std::cout << "Verify SYSCAL table" << std::endl;
    auto const mytable = myms.sysCal();
    map<uInt, SysCalRecord> expected_record = reader.syscal_record_;
    uInt expected_nrow = expected_record.size();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSSysCalColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      SysCalRecord row_record = expected_record[i];
      EXPECT_EQ(row_record.time, mycolumns.time()(i));
      EXPECT_EQ(row_record.interval, mycolumns.interval()(i));
      EXPECT_EQ(row_record.spw_id, mycolumns.spectralWindowId()(i));
      EXPECT_EQ(row_record.feed_id, mycolumns.feedId()(i));
      EXPECT_EQ(row_record.antenna_id, mycolumns.antennaId()(i));
      Array<Float> expected_array = row_record.tcal;
      Array<Float> array;
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.tcal().isDefined(i));
      } else {
        array = mycolumns.tcal()(i);
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
      expected_array.assign(row_record.tsys);
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.tsys().isDefined(i));
      } else {
        array.assign(mycolumns.tsys()(i));
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
      expected_array.assign(row_record.tcal_spectrum);
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.tcalSpectrum().isDefined(i));
      } else {
        array.assign(mycolumns.tcalSpectrum()(i));
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
      expected_array.assign(row_record.tsys_spectrum);
      if (expected_array.size() == 0) {
        EXPECT_FALSE(mycolumns.tsysSpectrum().isDefined(i));
      } else {
        array.assign(mycolumns.tsysSpectrum()(i));
        CASA_EXPECT_ARRAYEQ(expected_array, array);
      }
    }
  }

  // verify WEATHER table
  {
    std::cout << "Verify WEATHER table" << std::endl;
    auto const mytable = myms.weather();
    auto expected_record = reader.weather_record_;
    uInt expected_nrow = expected_record.size();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSWeatherColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      WeatherRecord row_record = expected_record[i];
      EXPECT_EQ(row_record.time, mycolumns.time()(i));
      EXPECT_EQ(row_record.interval, mycolumns.interval()(i));
      EXPECT_EQ(row_record.antenna_id, mycolumns.antennaId()(i));
      EXPECT_FLOAT_EQ(row_record.temperature, mycolumns.temperature()(i));
      EXPECT_FLOAT_EQ(row_record.pressure, mycolumns.pressure()(i));
      EXPECT_FLOAT_EQ(row_record.rel_humidity, mycolumns.relHumidity()(i));
      EXPECT_FLOAT_EQ(row_record.wind_speed, mycolumns.windSpeed()(i));
      EXPECT_FLOAT_EQ(row_record.wind_direction, mycolumns.windDirection()(i));
    }
  }

  // verify HISTORY table
  {
    std::cout << "Verify HISTORY table" << std::endl;
    auto const mytable = myms.history();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // verify FLAG_CMD table
  {
    std::cout << "Verify FLAG_CMD table" << std::endl;
    auto const mytable = myms.flagCmd();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // verify DOPPLER table
  {
    std::cout << "Verify DOPPLER table" << std::endl;
    auto const mytable = myms.doppler();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // verify FREQ_OFFSET table
  {
    std::cout << "Verify FREQ_OFFSET table" << std::endl;
    auto const mytable = myms.freqOffset();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // prepare for verifying DATA_DESCRIPTION table
  Vector < Int > dd_polarization_id;
  Vector < Int > dd_spw_id;
  {
    auto const mytable = myms.dataDescription();
    ROMSDataDescColumns const mycolumns(mytable);
    dd_polarization_id = mycolumns.polarizationId().getColumn();
    dd_spw_id = mycolumns.spectralWindowId().getColumn();
  }

  // prepare for verifying POLARIZATION table
  Vector < Int > num_pol_map;
  {
    auto const mytable = myms.polarization();
    ROMSPolarizationColumns const mycolumns(mytable);
    num_pol_map = mycolumns.numCorr().getColumn();
  }

  // prepare for verifying FEED table
  Matrix < Int > feed_id_matrix;
  {
    auto const mytable = myms.feed();
    ROMSFeedColumns const mycolumns(mytable);
    uInt nrow = mycolumns.nrow();
    feed_id_matrix.resize(nrow, 2);
    feed_id_matrix.column(0) = mycolumns.feedId().getColumn();
    feed_id_matrix.column(1) = mycolumns.spectralWindowId().getColumn();
  }

  // prepare for verifying STATE table
  Vector < Int > subscan_map;
  Vector < String > obs_mode_map;
  {
    auto const mytable = myms.state();
    ROMSStateColumns const mycolumns(mytable);
    subscan_map = mycolumns.subScan().getColumn();
    obs_mode_map = mycolumns.obsMode().getColumn();
  }

  // verify MAIN table with several subtables
  constexpr size_t kNumPointing = 4;
  size_t pointing_count = 0;
  Vector < Double > pointing_time(kNumPointing);
  Vector < Int > pointing_antenna(kNumPointing);
  Vector < Int > pointing_num_poly(kNumPointing);
  Cube<Double> pointing_direction(IPosition(3, 2, 1, kNumPointing));
  {
    std::cout << "Verify MAIN table" << std::endl;
    auto const mytable = myms;
    ROMSMainColumns const mycolumns(mytable);
    // expected record stores each spectrum with associating meta data
    // its order is as follows (See FloatDataStorage or ComplexDataStorage):
    // Time0 Intent0 Field0 Antenna0 Beam0 Spw0 Pol0 |
    // Time0 Intent0 Field0 Antenna0 Beam0 Spw0 Pol1 +--> ROW 0
    // Time0 Intent0 Field0 Antenna0 Beam0 Spw1 Pol0 ---> ROW 1
    // Time0 Intent0 Field0 Antenna0 Beam1 Spw0 Pol0 |
    // Time0 Intent0 Field0 Antenna0 Beam1 Spw0 Pol1 +--> ROW 2
    // Time0 Intent0 Field0 Antenna0 Beam1 Spw1 Pol0 ---> ROW 3
    // Time0 Intent0 Field1 Antenna1 Beam0 Spw0 Pol0 |
    // Time0 Intent0 Field1 Antenna1 Beam0 Spw0 Pol1 +--> ROW 4
    // Time0 Intent0 Field1 Antenna1 Beam0 Spw1 Pol0 ---> ROW 5
    // Time0 Intent0 Field1 Antenna1 Beam1 Spw0 Pol0 |
    // Time0 Intent0 Field1 Antenna1 Beam1 Spw0 Pol1 +--> ROW 6
    // Time0 Intent0 Field1 Antenna1 Beam1 Spw1 Pol0 ---> ROW 7
    // Time1 Intent1 Field0 Antenna0 Beam0 Spw0 Pol0 |
    // Time1 Intent1 Field0 Antenna0 Beam0 Spw0 Pol1 +--> ROW 8
    // Time1 Intent1 Field0 Antenna0 Beam0 Spw1 Pol0 ---> ROW 9
    // Time1 Intent1 Field0 Antenna0 Beam1 Spw0 Pol0 |
    // Time1 Intent1 Field0 Antenna0 Beam1 Spw0 Pol1 +--> ROW 10
    // Time1 Intent1 Field0 Antenna0 Beam1 Spw1 Pol0 ---> ROW 11
    // Time1 Intent1 Field1 Antenna1 Beam0 Spw0 Pol0 |
    // Time1 Intent1 Field1 Antenna1 Beam0 Spw0 Pol1 +--> ROW 12
    // Time1 Intent1 Field1 Antenna1 Beam0 Spw1 Pol0 ---> ROW 13
    // Time1 Intent1 Field1 Antenna1 Beam1 Spw0 Pol0 |
    // Time1 Intent1 Field1 Antenna1 Beam1 Spw0 Pol1 +--> ROW 14
    // Time1 Intent1 Field1 Antenna1 Beam1 Spw1 Pol0 ---> ROW 15
    TableRecord const &expected_record = reader.main_record_;
    constexpr size_t expected_nfield = 24;
    constexpr size_t expected_nrow = 16;
    ASSERT_EQ(expected_nfield, expected_record.nfields());
    ASSERT_EQ(expected_nrow, mycolumns.nrow());
    Vector<Double> const expected_uvw(3, 0.0);
    //Int const expected_antenna = 0;
    Int const expected_processor = 0;
    Int const expected_observation = 0;
    size_t record_index = 0;
    auto generateKey = [](size_t i) {return "ROW" + String::toString(i);};
    for (size_t i = 0; i < expected_nrow; i += 2) {
      size_t irow = i;
      std::cout << "Verifying row " << irow << " (record " << record_index
          << " and " << record_index + 1 << ")" << std::endl;
      String key = generateKey(record_index);
      TableRecord const row_record0 = expected_record.asRecord(key);
      Int expected_subscan = row_record0.asInt("SUBSCAN");
      String expected_intent = row_record0.asString("INTENT");
      auto verifyMeta =
          [&](TableRecord const &record, size_t krow) {
            Double expected_time = record.asDouble("TIME");
            std::cout << "irow " << krow << " time " << mycolumns.time()(krow) << " (expected " << expected_time << ")" << std::endl;
            Double expected_interval = record.asDouble("INTERVAL");
            Int expected_feed = record.asInt("FEED_ID");
            Int expected_field = record.asInt("FIELD_ID");
            Int expected_scan = record.asInt("SCAN");
            Int expected_antenna = record.asInt("ANTENNA_ID");
            EXPECT_EQ(expected_time, mycolumns.time()(krow));
            EXPECT_EQ(expected_time, mycolumns.timeCentroid()(krow));
            EXPECT_EQ(expected_interval, mycolumns.interval()(krow));
            EXPECT_EQ(expected_interval, mycolumns.exposure()(krow));
            EXPECT_TRUE(allEQ(expected_uvw, mycolumns.uvw()(krow)));
            EXPECT_EQ(expected_antenna, mycolumns.antenna1()(krow));
            EXPECT_EQ(expected_antenna, mycolumns.antenna2()(krow));
            EXPECT_EQ(expected_processor, mycolumns.processorId()(krow));
            EXPECT_EQ(expected_observation, mycolumns.observationId()(krow));
            EXPECT_EQ(expected_feed, mycolumns.feed1()(krow));
            EXPECT_EQ(expected_feed, mycolumns.feed2()(krow));
            EXPECT_EQ(expected_field, mycolumns.fieldId()(krow));
            EXPECT_EQ(expected_scan, mycolumns.scanNumber()(krow));
          };
      verifyMeta(row_record0, irow);
      ++record_index;
      key = generateKey(record_index);
      TableRecord const row_record1 = expected_record.asRecord(key);
      Bool expected_flag_row = row_record0.asBool("FLAG_ROW")
          || row_record1.asBool("FLAG_ROW");
      Int expected_num_pol = 2;
      Float expected_weight = 1.0;
      Int expected_spw_id = row_record0.asInt("SPECTRAL_WINDOW_ID");
      Int num_chan = -1;
      ASSERT_NO_THROW( {
        num_chan = num_chan_map[expected_spw_id]
        ;
      }
);
                                                                                                                            ASSERT_GT(num_chan, 0);
      Matrix < Bool > expected_flag(expected_num_pol, num_chan);
      Matrix < Float > expected_data(expected_num_pol, num_chan);
      std::cout << "expected_flag.shape = " << expected_flag.shape()
          << std::endl;
      Int pol_id0 = row_record0.asInt("POLNO");
      Int pol_id1 = row_record1.asInt("POLNO");
      ASSERT_LT(pol_id0, 2);
      ASSERT_GE(pol_id0, 0);
      ASSERT_LT(pol_id1, 2);
      ASSERT_GE(pol_id1, 0);
      ASSERT_NE(pol_id0, pol_id1);
      expected_flag.row(pol_id0) = row_record0.asArrayBool("FLAG");
      expected_flag.row(pol_id1) = row_record1.asArrayBool("FLAG");
      expected_data.row(pol_id0) = row_record0.asArrayFloat("DATA");
      expected_data.row(pol_id1) = row_record1.asArrayFloat("DATA");
      auto verifyData = [&](size_t krow) {
        EXPECT_EQ(expected_flag_row, mycolumns.flagRow()(krow));
        Vector<Float> sigma = mycolumns.sigma()(krow);
        IPosition sigma_shape = sigma.shape();
        EXPECT_EQ(1ul, sigma_shape.size());
        EXPECT_EQ(expected_num_pol, sigma_shape[0]);
        EXPECT_TRUE(allEQ(sigma, expected_weight));
        Vector<Float> weight = mycolumns.weight()(krow);
        EXPECT_TRUE(allEQ(sigma, weight));
        Matrix<Bool> flag = mycolumns.flag()(krow);
        EXPECT_EQ(expected_flag.shape(), flag.shape());
        EXPECT_TRUE(allEQ(expected_flag, flag));
        Matrix<Float> data = mycolumns.floatData()(krow);
        EXPECT_EQ(expected_data.shape(), data.shape());
        EXPECT_TRUE(allEQ(expected_data, data));
      };
      // verify data
      verifyData(irow);

      // verify subtables
      auto verifySubtables = [&](size_t krow) {
        // DATA_DESCRIPTION
          Int data_desc_id = mycolumns.dataDescId()(krow);
          Int spw_id = dd_spw_id[data_desc_id];
          Int pol_id = dd_polarization_id[data_desc_id];
          EXPECT_EQ(expected_spw_id, spw_id);

          // POLARIZATION
          Int num_pol = num_pol_map[pol_id];
          EXPECT_EQ(expected_num_pol, num_pol);

          // STATE
          Int state_id = mycolumns.stateId()(krow);
          Int subscan = subscan_map[state_id];
          String intent = obs_mode_map[state_id];
          EXPECT_EQ(expected_subscan, subscan);
          CASA_EXPECT_STREQ(expected_intent, intent);

          // FEED
          Int feed_id = mycolumns.feed1()(krow);
          Bool found = False;
          Int id_matrix_arr[2] = {feed_id, spw_id};
          Vector<Int> id_matrix(id_matrix_arr, 2, SHARE);
          for (uInt frow = 0; frow < feed_id_matrix.nrow(); ++frow) {
            if (allEQ(feed_id_matrix.row(frow), id_matrix)) {
              found = True;
              break;
            }
          }
          EXPECT_TRUE(found);
        };
      verifySubtables(irow);

      // for POINTING table verification
      auto addPointing =
          [&](size_t krow, TableRecord const &record) {
            if (mycolumns.feed1()(krow) == 0) {
              Int myantenna_id = mycolumns.antenna1()(krow);
              Double mytime = mycolumns.time()(krow);
              bool match = false;
              for (size_t i = 0; i < pointing_count; ++i) {
                if (pointing_time[i] == mytime && pointing_antenna[i] == myantenna_id) {
                  match = true;
                  break;
                }
              }
              if (!match) {
                pointing_time[pointing_count] = mytime;
                pointing_antenna[pointing_count] = myantenna_id;
                Matrix<Double> direction = record.asArrayDouble("DIRECTION");
                Int num_poly = direction.shape()[1] - 1;
                pointing_num_poly[pointing_count] = num_poly;
                pointing_direction.xyPlane(pointing_count) = direction;
                ++pointing_count;
                ASSERT_LE(pointing_count, kNumPointing);
              }
              std::cout << "irow " << irow << " time " << mytime << " antenna " << myantenna_id << " pointing_count " << pointing_count << std::endl;
              std::cout << "pointing_time=" << pointing_time << " pointing_antenna=" << pointing_antenna << std::endl;
            }
          };
      addPointing(irow, row_record0);

      // next row
      ++record_index;
      ++irow;
      std::cout << "Verifying row " << irow << " (record " << record_index
          << ")" << std::endl;
      key = generateKey(record_index);
      TableRecord const row_record2 = expected_record.asRecord(key);
      expected_subscan = row_record2.asInt("SUBSCAN");
      expected_intent = row_record2.asString("INTENT");
      verifyMeta(row_record2, irow);
      expected_flag_row = row_record2.asBool("FLAG_ROW");
      expected_num_pol = 1;
      expected_spw_id = row_record2.asInt("SPECTRAL_WINDOW_ID");
      ASSERT_NO_THROW( {
        num_chan = num_chan_map[expected_spw_id]
        ;
      }
);
                                                                                                                            ASSERT_GT(num_chan, 0);
      expected_flag.resize(expected_num_pol, num_chan);
      expected_data.resize(expected_flag.shape());
      Int pol_id2 = row_record2.asInt("POLNO");
      ASSERT_EQ(0, pol_id2);
      expected_flag.row(pol_id2) = row_record2.asArrayBool("FLAG");
      expected_data.row(pol_id2) = row_record2.asArrayFloat("DATA");
      // verify data
      verifyData(irow);

      // verify subtables
      verifySubtables(irow);

      // for POINTING table verifycation
      addPointing(irow, row_record2);

      ++record_index;
    }
  }

  // verify POINTING table
  {
    std::cout << "Verify POINTING table" << std::endl;
    auto const mytable = myms.pointing();
    ROMSPointingColumns const mycolumns(mytable);
    uInt nrow = mytable.nrow();
    ASSERT_EQ(pointing_count, kNumPointing);
    ASSERT_EQ(pointing_count, nrow);
    Sort sorter;
    sorter.sortKey(pointing_antenna.data(), TpInt);
    sorter.sortKey(pointing_time.data(), TpDouble);
    Vector < uInt > sort_index;
    sorter.sort(sort_index, pointing_count);
    for (size_t i = 0; i < nrow; ++i) {
      size_t j = sort_index[i];
      EXPECT_EQ(pointing_time[j], mycolumns.time()(i));
      EXPECT_EQ(pointing_antenna[j], mycolumns.antennaId()(i));
      EXPECT_EQ(pointing_num_poly[j], mycolumns.numPoly()(i));
      EXPECT_TRUE(
          allEQ(pointing_direction.xyPlane(j), mycolumns.direction()(i)));
    }
  }
}

namespace {
inline void TestKeyword(Vector<TableRecord> const &input_record,
    TableRecord const &output_record, String const &output_data_key) {
  constexpr size_t kNumInputKeys = 4;
  constexpr size_t kNumOutputKeys = 5;
  String const input_keys[kNumInputKeys] =
      { "DATA", "FLAG", "FLAG_ROW", "POLNO" };
  String output_keys[kNumOutputKeys] = { output_data_key, "FLAG", "FLAG_ROW",
      "SIGMA", "WEIGHT" };
  for (size_t i = 0; i < input_record.size(); ++i) {
    for (size_t j = 0; j < kNumInputKeys; ++j) {
      ASSERT_TRUE(input_record[i].isDefined(input_keys[j]));
    }
  }
  for (size_t j = 0; j < kNumOutputKeys; ++j) {
    ASSERT_TRUE(output_record.isDefined(output_keys[j]));
  }
}

inline void TestShape(size_t const num_pol, size_t const num_chan,
    TableRecord const &output_record, String const &output_data_key) {
  IPosition const expected_shape_matrix(2, num_pol, num_chan);
  IPosition const expected_shape_vector(1, num_pol);
  EXPECT_EQ(expected_shape_matrix, output_record.shape(output_data_key));
  EXPECT_EQ(expected_shape_matrix, output_record.shape("FLAG"));
  EXPECT_EQ(expected_shape_vector, output_record.shape("SIGMA"));
  EXPECT_EQ(expected_shape_vector, output_record.shape("WEIGHT"));
}

struct BasicPolarizationTester {
  static void Test(size_t const num_chan, String const &pol_type,
      Vector<uInt> const &polid_list, Vector<TableRecord> const &input_record,
      TableRecord const &output_record) {
    size_t const expected_num_pol = polid_list.size();
    size_t const num_pol = output_record.asInt("NUM_POL");
    ASSERT_GE(2ul, num_pol);
    ASSERT_EQ(input_record.size(), num_pol);
    ASSERT_EQ(expected_num_pol, num_pol);

    static Vector<Int> corr_type_list(2, Stokes::Undefined);
    if (pol_type == "linear") {
      corr_type_list[0] = Stokes::XX;
      corr_type_list[1] = Stokes::YY;
    } else if (pol_type == "circular") {
      corr_type_list[0] = Stokes::RR;
      corr_type_list[1] = Stokes::LL;
    } else if (pol_type == "stokes") {
      corr_type_list[0] = Stokes::I;
    } else if (pol_type == "linpol") {
      corr_type_list[0] = Stokes::Plinear;
      corr_type_list[1] = Stokes::Pangle;
    }
    if (num_pol == 2) {
      Vector < Int > expected_corr = corr_type_list;
      Vector < Int > corr = output_record.asArrayInt("CORR_TYPE");
      std::cout << "corr = " << corr << " (expected " << expected_corr << ")"
          << std::endl;
      ASSERT_TRUE(allEQ(expected_corr, corr));
    } else {
      Int corr_index = input_record[0].asInt("POLNO");
      Vector < Int > expected_corr(1, corr_type_list[corr_index]);
      Vector < Int > corr = output_record.asArrayInt("CORR_TYPE");
      std::cout << "corr = " << corr << " (expected " << expected_corr << ")"
          << std::endl;
      ASSERT_TRUE(allEQ(expected_corr, corr));
    }

    TestKeyword(input_record, output_record, "FLOAT_DATA");

    TestShape(num_pol, num_chan, output_record, "FLOAT_DATA");

    Matrix < Float > out_data = output_record.asArrayFloat("FLOAT_DATA");
    Matrix < Bool > out_flag = output_record.asArrayBool("FLAG");
    Bool out_flag_row = output_record.asBool("FLAG_ROW");
    Vector < Float > out_sigma = output_record.asArrayFloat("SIGMA");
    Vector < Float > out_weight = output_record.asArrayFloat("WEIGHT");
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      std::cout << "Verifying i " << i << " polid "
          << input_record[i].asuInt("POLNO") << std::endl;
      uInt polid = 0;
      if (num_pol == 2) {
        polid = input_record[i].asuInt("POLNO");
      }
      Vector < Float > data = input_record[i].asArrayFloat("DATA");
      Vector < Bool > flag = input_record[i].asArrayBool("FLAG");
      net_flag_row = net_flag_row || input_record[i].asBool("FLAG_ROW");
      std::cout << "out_data.shape() = " << out_data.shape() << std::endl;
      std::cout << "polid = " << polid << " (num_pol " << num_pol << ")"
          << std::endl;
      EXPECT_TRUE(allEQ(out_data.row(polid), data));
      EXPECT_TRUE(allEQ(out_flag.row(polid), flag));
    }
    EXPECT_EQ(net_flag_row, out_flag_row);
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }
};

struct FullPolarizationTester {
  static void Test(size_t const num_chan, String const &pol_type,
      Vector<uInt> const &polid_list, Vector<TableRecord> const &input_record,
      TableRecord const &output_record) {
    std::cout << "Full polarization test" << std::endl;
    size_t const expected_num_pol = polid_list.size();
    size_t const num_pol = output_record.asInt("NUM_POL");
    ASSERT_EQ(4ul, num_pol);
    ASSERT_EQ(input_record.size(), num_pol);
    ASSERT_EQ(expected_num_pol, num_pol);

    static Vector<Int> expected_corr(4, Stokes::Undefined);
    if (pol_type == "linear") {
      expected_corr[0] = Stokes::XX;
      expected_corr[1] = Stokes::XY;
      expected_corr[2] = Stokes::YX;
      expected_corr[3] = Stokes::YY;
    } else if (pol_type == "circular") {
      expected_corr[0] = Stokes::RR;
      expected_corr[1] = Stokes::RL;
      expected_corr[2] = Stokes::LR;
      expected_corr[3] = Stokes::LL;
    }
    Vector < Int > corr = output_record.asArrayInt("CORR_TYPE");
    std::cout << "corr = " << corr << " (expected " << expected_corr << ")"
        << std::endl;
    ASSERT_TRUE(allEQ(expected_corr, corr));

    TestKeyword(input_record, output_record, "DATA");

    TestShape(num_pol, num_chan, output_record, "DATA");

    Matrix < Float > data_real(IPosition(2, num_pol, num_chan), 0.0f);
    Matrix < Float > data_imag(data_real.shape(), 0.0f);
    Matrix < Bool > flag(data_real.shape(), False);
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      uInt polid = polid_list[i];
      ASSERT_LT(polid, num_pol);
      Vector < Float > input_data = input_record[i].asArrayFloat("DATA");
      Vector < Bool > input_flag = input_record[i].asArrayBool("FLAG");
      net_flag_row = net_flag_row || input_record[i].asBool("FLAG_ROW");
      if (polid == 0) {
        data_real.row(0) = input_data;
        flag.row(0) = input_flag;
      } else if (polid == 1) {
        data_real.row(3) = input_data;
        flag.row(3) = input_flag;
      } else if (polid == 2) {
        data_real.row(1) = input_data;
        data_real.row(2) = input_data;
        flag.row(1) = input_flag;
      } else if (polid == 3) {
        data_imag.row(1) = input_data;
        data_imag.row(2) = -input_data;
        flag.row(2) = input_flag;
      }
    }
    flag.row(1) = flag.row(1) || flag.row(2);
    flag.row(2) = flag.row(1);

    Matrix < Complex > out_data = output_record.asArrayComplex("DATA");
    EXPECT_TRUE(allEQ(data_real, real(out_data)));
    EXPECT_TRUE(allEQ(data_imag, imag(out_data)));
    Matrix < Bool > out_flag = output_record.asArrayBool("FLAG");
//    std::cout << "expected flag: " << flag << std::endl;
//    std::cout << "output flag  : " << out_flag << std::endl;
    EXPECT_TRUE(allEQ(flag, out_flag));
    Bool out_flag_row = output_record.asBool("FLAG_ROW");
    EXPECT_EQ(net_flag_row, out_flag_row);
    Vector < Float > out_sigma = output_record.asArrayFloat("SIGMA");
    Vector < Float > out_weight = output_record.asArrayFloat("WEIGHT");
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }

};

struct StokesFullPolarizationTester {
  static void Test(size_t const num_chan, String const &pol_type,
      Vector<uInt> const &polid_list, Vector<TableRecord> const &input_record,
      TableRecord const &output_record) {
    std::cout << "Stokes full polarization test" << std::endl;
    CASA_ASSERT_STREQ(String("stokes"), pol_type);

    size_t const expected_num_pol = polid_list.size();
    size_t const num_pol = output_record.asInt("NUM_POL");
    //ASSERT_LE(num_pol, 4ul);
    ASSERT_EQ(expected_num_pol, num_pol);
    ASSERT_EQ(input_record.size(), num_pol);
    ASSERT_EQ(4ul, num_pol);

    static Vector<Int> expected_corr(4, Stokes::Undefined);
    expected_corr[0] = Stokes::I;
    expected_corr[1] = Stokes::Q;
    expected_corr[2] = Stokes::U;
    expected_corr[3] = Stokes::V;
    Vector < Int > corr = output_record.asArrayInt("CORR_TYPE");
    std::cout << "corr = " << corr << " (expected " << expected_corr << ")"
        << std::endl;
    ASSERT_TRUE(allEQ(expected_corr, corr));

    TestKeyword(input_record, output_record, "FLOAT_DATA");

    TestShape(num_pol, num_chan, output_record, "FLOAT_DATA");

    Matrix < Float > out_data = output_record.asArrayFloat("FLOAT_DATA");
    Matrix < Bool > out_flag = output_record.asArrayBool("FLAG");
    Bool out_flag_row = output_record.asBool("FLAG_ROW");
    Vector < Float > out_sigma = output_record.asArrayFloat("SIGMA");
    Vector < Float > out_weight = output_record.asArrayFloat("WEIGHT");
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      uInt polid = polid_list[i];
      ASSERT_LT(polid, num_pol);
      Vector < Float > data = input_record[i].asArrayFloat("DATA");
      Vector < Bool > flag = input_record[i].asArrayBool("FLAG");
      Bool flag_row = input_record[i].asBool("FLAG_ROW");
      net_flag_row = net_flag_row || flag_row;
      EXPECT_TRUE(allEQ(data, out_data.row(polid)));
      EXPECT_TRUE(allEQ(flag, out_flag.row(polid)));
    }
    EXPECT_EQ(net_flag_row, out_flag_row);
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }

};

struct StandardInitializer {
  static void Initialize(size_t const num_chan, Vector<uInt> const &polid_list,
      Vector<TableRecord> &input_record, DataChunk &chunk) {
    chunk.initialize(num_chan);
    Matrix < Float > data(4, num_chan, 0.5);
    Matrix < Bool > flag(4, num_chan, False);
    Vector < Bool > flag_row(4, False);
    data(0, 16) = -0.5;
    flag(0, 16) = True;
    data(1, 8) = 1.e10;
    flag(1, 8) = True;
    data.row(2) = -1.e10;
    flag.row(2) = True;
    flag_row[2] = True;
    for (size_t i = 0; i < polid_list.size(); ++i) {
      TableRecord &input_record0 = input_record[i];
      uInt polid = polid_list[i];
      input_record0.define("POLNO", polid);
      input_record0.define("DATA", data.row(polid));
      input_record0.define("FLAG", flag.row(polid));
      input_record0.define("FLAG_ROW", flag_row(polid));
      chunk.accumulate(input_record0);
    }
  }
};

template<class Initializer = StandardInitializer,
    class Tester = BasicPolarizationTester>
void TestPolarization(String const &poltype, Vector<uInt> const &polid_list) {
  std::cout << "Test " << poltype << " polarization with pol " << polid_list
      << std::endl;

  DataChunk chunk(poltype);

  CASA_ASSERT_STREQ(poltype, chunk.getPolType());

  size_t num_chan = 32;
  Vector<TableRecord> input_record(polid_list.size());

  Initializer::Initialize(num_chan, polid_list, input_record, chunk);

  ASSERT_EQ(polid_list.size(), chunk.getNumPol());

  TableRecord output_record;
  ASSERT_TRUE(output_record.empty());

  chunk.get(output_record);

  Tester::Test(num_chan, poltype, polid_list, input_record, output_record);
}

void TestSinglePolarization(String const &poltype, uInt const polid) {
  Vector < uInt > polid_list(1, polid);
  TestPolarization(poltype, polid_list);
}

} // anonymous namespace

TEST(DataChunkTest, SinglePolarizationTest) {
// POL 0
  uInt polid = 0;

// Linear
  TestSinglePolarization("linear", polid);

// Circular
  TestSinglePolarization("circular", polid);

// Stokes
  TestSinglePolarization("stokes", polid);

// Linpol
  TestSinglePolarization("linpol", polid);

// POL 1
  polid = 1;

// Linear
  TestSinglePolarization("linear", polid);

// Circular
  TestSinglePolarization("circular", polid);

// Stokes
// it should cause error, see WhiteBoxTest

// Linpol
// it should cause error, see WhiteBoxTest
}

TEST(DataChunkTest, DualPolarizationTest) {
// POL 0 and 1
  Vector < uInt > polid_list(2);
  polid_list[0] = 0;
  polid_list[1] = 1;

// Linear
  TestPolarization("linear", polid_list);

// Circular
  TestPolarization("circular", polid_list);

// Stokes
// it should cause unexpected behavior, see WhiteBoxTest

// Linpol
  TestPolarization("linpol", polid_list);

// Reverse POL order
  polid_list[0] = 1;
  polid_list[1] = 0;

// Linear
  TestPolarization("linear", polid_list);

// Circular
  TestPolarization("circular", polid_list);

// Stokes
// it should cause unexpected behavior, see WhiteBoxTest

// Linpol
  TestPolarization("linpol", polid_list);

}

TEST(DataChunkTest, FullPolarizationTest) {
  Vector < uInt > polid_list(4);

// Usual accumulation order
  polid_list[0] = 0;
  polid_list[1] = 1;
  polid_list[2] = 2;
  polid_list[3] = 3;

// Linear
  TestPolarization<StandardInitializer, FullPolarizationTester>("linear",
      polid_list);

// Circular
  TestPolarization<StandardInitializer, FullPolarizationTester>("circular",
      polid_list);

// Stokes
  TestPolarization<StandardInitializer, StokesFullPolarizationTester>("stokes",
      polid_list);

// Linpol
// it should cause unexpected behavior, see WhiteBoxTest

// Different order
  polid_list[0] = 2;
  polid_list[2] = 0;

// Linear
  TestPolarization<StandardInitializer, FullPolarizationTester>("linear",
      polid_list);

// Circular
  TestPolarization<StandardInitializer, FullPolarizationTester>("circular",
      polid_list);

// Stokes
  TestPolarization<StandardInitializer, StokesFullPolarizationTester>("stokes",
      polid_list);

// Linpol
// it should cause unexpected behavior, see WhiteBoxTest
}

TEST(DataChunkTest, WhiteBoxTest) {
// Invalid poltype
  EXPECT_THROW(DataChunk("notype"), AipsError);

// Accumulate without initialization
  DataChunk chunk("linear");
  CASA_ASSERT_STREQ(String("linear"), chunk.getPolType());
  TableRecord record;
  Vector < Float > data1(1);
  Vector < Bool > flag1(1);
  record.define("POLNO", (uInt) 0);
  record.define("DATA", data1);
  record.define("FLAG", flag1);
  record.define("FLAG_ROW", False);
  ASSERT_EQ(0u, chunk.getNumPol());
  bool status = chunk.accumulate(record);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, chunk.getNumPol());
  TableRecord output_record;
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  EXPECT_EQ(IPosition(2, 1, 1),
      output_record.asArrayFloat("FLOAT_DATA").shape());

// clear
  chunk.clear();
  EXPECT_EQ(0u, chunk.getNumPol());

// Accumulate invalid record
  chunk.initialize(1);
  record.removeField("POLNO");
  ASSERT_EQ(0u, chunk.getNumPol());
  status = chunk.accumulate(record);
  ASSERT_FALSE(status);
  EXPECT_EQ(0u, chunk.getNumPol());

// Accumulate different shaped data
  Vector < Float > data2(2);
  Vector < Bool > flag2(2);
  chunk.initialize(1);
  record.define("POLNO", (uInt) 0);
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  constexpr bool expected_status = false;
  status = chunk.accumulate(record);
  EXPECT_EQ(expected_status, status);

// Shape mismatch between data and flag
  record.define("DATA", data1);
  status = chunk.accumulate(record);
  EXPECT_EQ(expected_status, status);

// Test number of polarization
  constexpr size_t num_seq = 4ul;
  constexpr size_t num_accum = 4ul;
  uInt expected_num_pol[num_seq][num_accum] = { { 1, 2, 2, 4 }, { 1, 2, 2, 4 },
      { 0, 0, 1, 4 }, { 1, 1, 2, 4 } };
  uInt polid_order[num_seq][num_accum] = { { 0, 1, 2, 3 }, { 1, 0, 2, 3 }, { 3,
      2, 1, 0 }, { 0, 3, 1, 2 } };
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  for (size_t j = 0; j < num_seq; ++j) {
    chunk.initialize(2);
    EXPECT_EQ(0u, chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.define("POLNO", polid_order[j][i]);
      status = chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol[j][i], chunk.getNumPol());
    }
  }

// accumulate data to same polarization twice
  data2 = 0.0;
  chunk.initialize(2);
  record.define("POLNO", 0u);
  record.define("DATA", data2);
  chunk.accumulate(record);
  data2 = 1.0;
  record.define("DATA", data2);
  status = chunk.accumulate(record);
  ASSERT_TRUE(status);
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > output_data = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 1, 2), output_data.shape());
  EXPECT_TRUE(allEQ(output_data, 1.0f));

// accumulate three polarization component
  chunk.initialize(2);
  data2 = 3.0;
  record.define("POLNO", 0u);
  record.define("DATA", data2);
  chunk.accumulate(record);
  record.define("POLNO", 1u);
  chunk.accumulate(record);
  record.define("POLNO", 2u);
  chunk.accumulate(record);
  EXPECT_EQ(2u, chunk.getNumPol());
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 2, 2), data.shape());
  EXPECT_TRUE(allEQ(data, 3.0f));

// reset poltype
  EXPECT_NE(0u, chunk.getNumPol());
  chunk.resetPolType("circular");
  CASA_ASSERT_STREQ(String("circular"), chunk.getPolType());
  ASSERT_EQ(0u, chunk.getNumPol());

// Stokes single pol 1 and dual pols 1 and 2 are invalid
  DataChunk stokes_chunk("stokes");

  stokes_chunk.initialize(2);
  data2 = 4.0;
  record.define("POLNO", 1u);
  record.define("DATA", data2);
  stokes_chunk.accumulate(record);
  EXPECT_EQ(0u, stokes_chunk.getNumPol());
  status = stokes_chunk.get(output_record);
  ASSERT_FALSE(status);
  record.define("POLNO", 0u);
  stokes_chunk.accumulate(record);
  EXPECT_EQ(1u, stokes_chunk.getNumPol());
  status = stokes_chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data_stokes = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 1, 2), data_stokes.shape());
  EXPECT_TRUE(allEQ(data_stokes, 4.0f));

// Test number of polarization
  uInt expected_num_pol_stokes[num_seq][num_accum] = { { 1, 1, 1, 4 }, { 0, 1,
      1, 4 }, { 0, 0, 0, 4 }, { 1, 1, 1, 4 } };
  uInt polid_order_stokes[num_seq][num_accum] = { { 0, 1, 2, 3 },
      { 1, 0, 2, 3 }, { 3, 2, 1, 0 }, { 0, 3, 1, 2 } };
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  for (size_t j = 0; j < num_seq; ++j) {
    stokes_chunk.initialize(2);
    EXPECT_EQ(0u, stokes_chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.define("POLNO", polid_order_stokes[j][i]);
      status = stokes_chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol_stokes[j][i], stokes_chunk.getNumPol());
    }
  }

// Linpol single pol 1 and full pols are invalid
  DataChunk linpol_chunk("linpol");

  linpol_chunk.initialize(2);
  data2 = 8.0;
  record.define("POLNO", 1u);
  record.define("DATA", data2);
  linpol_chunk.accumulate(record);
  EXPECT_EQ(0u, linpol_chunk.getNumPol());
  status = linpol_chunk.get(output_record);
  ASSERT_FALSE(status);
  record.define("POLNO", 0u);
  linpol_chunk.accumulate(record);
  record.define("POLNO", 2u);
  linpol_chunk.accumulate(record);
  record.define("POLNO", 4u);
  linpol_chunk.accumulate(record);
  EXPECT_EQ(2u, linpol_chunk.getNumPol());
  status = linpol_chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data_linpol = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 2, 2), data_linpol.shape());
  EXPECT_TRUE(allEQ(data_linpol, 8.0f));

// Test number of polarization
  uInt expected_num_pol_linpol[num_seq][num_accum] = { { 1, 2, 2, 2 }, { 0, 2,
      2, 2 }, { 0, 0, 0, 2 }, { 1, 1, 2, 2 } };
  uInt polid_order_linpol[num_seq][num_accum] = { { 0, 1, 2, 3 },
      { 1, 0, 2, 3 }, { 3, 2, 1, 0 }, { 0, 3, 1, 2 } };
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  for (size_t j = 0; j < num_seq; ++j) {
    linpol_chunk.initialize(2);
    EXPECT_EQ(0u, linpol_chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.define("POLNO", polid_order_linpol[j][i]);
      status = linpol_chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol_linpol[j][i], linpol_chunk.getNumPol());
    }
  }
}

TEST(DataAccumulatorTest, WhiteBoxTest) {
  constexpr size_t num_record_keys = 13;
  constexpr const char *output_record_keys[] = { "TIME", "POL_TYPE", "INTENT",
      "SPECTRAL_WINDOW_ID", "FIELD_ID", "FEED_ID", "SCAN", "SUBSCAN", "FLAG",
      "FLAG_ROW", "SIGMA", "WEIGHT", "DIRECTION" }; // and "DATA" or "FLOAT_DATA"
  auto IsValidOutputRecord = [&](TableRecord const &record) {
    for (size_t i = 0; i < num_record_keys; ++i) {
      ASSERT_TRUE(record.isDefined(output_record_keys[i]));
    }
    size_t const expected_num_pol = record.asInt("NUM_POL");
    Matrix<Bool> const flag = record.asArrayBool("FLAG");
    size_t const num_pol = flag.nrow();
    ASSERT_EQ(expected_num_pol, num_pol);
    String const poltype = record.asString("POL_TYPE");
    if (poltype == "linear" || poltype == "circular") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 2) || (num_pol == 4));
      if (num_pol < 4) {
        ASSERT_FALSE(record.isDefined("DATA"));
        ASSERT_TRUE(record.isDefined("FLOAT_DATA"));
      }
      else {
        ASSERT_FALSE(record.isDefined("FLOAT_DATA"));
        ASSERT_TRUE(record.isDefined("DATA"));
      }
    }
    else if (poltype == "stokes") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 4));
      ASSERT_FALSE(record.isDefined("DATA"));
      ASSERT_TRUE(record.isDefined("FLOAT_DATA"));
    }
    else if (poltype == "linpol") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 2));
      ASSERT_FALSE(record.isDefined("DATA"));
      ASSERT_TRUE(record.isDefined("FLOAT_DATA"));
    }
    else {
      FAIL();
    }
  };

  auto ClearRecord = [](TableRecord &record) {
    while (0 < record.nfields()) {
      record.removeField(0);
    }
    ASSERT_EQ(0u, record.nfields());
  };

  DataAccumulator a;

// number of chunks must be zero at initial state
  ASSERT_EQ(0ul, a.getNumberOfChunks());
  TableRecord output_record;
  bool status = a.get(0, output_record);
  ASSERT_FALSE(status);

  TableRecord r1;
  Time t(2016, 1, 1);
  Double time = t.modifiedJulianDay() * 86400.0;
  r1.define("TIME", time);

// accumulator should not be ready at initial state
  bool is_ready = a.queryForGet(r1);
  ASSERT_FALSE(is_ready);

// Invalid record cannot be accumulated
  status = a.accumulate(r1);
  ASSERT_FALSE(status);
  TableRecord r2;
  time += 1.0;
  r2.define("TIME", time);
  is_ready = a.queryForGet(r2);
  ASSERT_FALSE(is_ready);

// Accumulate one data
  Int antenna_id = 0;
  Int spw_id = 0;
  Int field_id = 1;
  Int feed_id = 2;
  Int scan = 29;
  Int subscan = 5;
  Matrix < Double > direction(2, 1, 0.0);
  String intent = "ON_SOURCE";
  String poltype = "linear";
  r1.define("ANTENNA_ID", antenna_id);
  r1.define("SPECTRAL_WINDOW_ID", spw_id);
  r1.define("FIELD_ID", field_id);
  r1.define("FEED_ID", feed_id);
  r1.define("SCAN", scan);
  r1.define("SUBSCAN", subscan);
  r1.define("INTENT", intent);
  r1.define("POL_TYPE", poltype);
  r1.define("DIRECTION", direction);
  size_t const num_chan = 4;
//  Vector < Float > data(num_chan, 0.0f);
//  Vector < Bool > flag(num_chan, False);
  Matrix < Float > data(IPosition(2, 4, num_chan), 0.0f);
  Matrix < Bool > flag(IPosition(2, 4, num_chan), False);
  data(0, 0) = 1.e10;
  flag(0, 0) = True;
  data(1, 1) = 1.e10;
  flag(1, 1) = True;
  data(2, 2) = 1.e10;
  flag(2, 2) = True;
  data(3, 3) = 1.e10;
  flag(3, 3) = True;
  Vector < Bool > flag_row(4, False);
  flag_row[3] = True;
  r1.define("POLNO", 0);
  r1.define("DATA", data.row(0));
  r1.define("FLAG", flag.row(0));
  r1.define("FLAG_ROW", flag_row[0]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  EXPECT_EQ(1u, a.getNumPol(0));

// Data with different timestamp cannot be accumulated
  r2.merge(r1, TableRecord::SkipDuplicates);
  status = a.accumulate(r2);
  ASSERT_FALSE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  EXPECT_EQ(1u, a.getNumPol(0));

// query
  is_ready = a.queryForGet(r1);
  ASSERT_FALSE(is_ready);

  is_ready = a.queryForGet(r2);
  ASSERT_TRUE(is_ready);

// Get
  ClearRecord(output_record);
  CASA_ASSERT_STREQ(poltype, a.getPolType(0));
  ASSERT_EQ(1u, a.getNumPol(0));
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Float > output_data = output_record.asArrayFloat("FLOAT_DATA");
  Matrix < Bool > output_flag = output_record.asArrayBool("FLAG");
  IPosition const expected_shape1(2, 1, num_chan);
  EXPECT_EQ(expected_shape1, output_data.shape());
  EXPECT_EQ(expected_shape1, output_flag.shape());
  EXPECT_TRUE(allEQ(output_data.row(0), data.row(0)));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_EQ(flag_row[0], output_record.asBool("FLAG_ROW"));
//  for (size_t i = 0; i < a.getNumberOfChunks(); ++i) {
//    CASA_ASSERT_STREQ(poltype, a.getPolType(i));
//    ASSERT_EQ(1u, a.getNumPol(i));
//    status = a.get(i, output_record);
//    ASSERT_TRUE(status);
//    IsValidOutputRecord(output_record);
//    Matrix < Float > output_data = output_record.asArrayFloat("FLOAT_DATA");
//    Matrix < Bool > output_flag = output_record.asArrayBool("FLAG");
//    IPosition const expected_shape(2, 1, num_chan);
//    EXPECT_EQ(expected_shape, output_data.shape());
//    EXPECT_EQ(expected_shape, output_flag.shape());
//    EXPECT_TRUE(allEQ(output_data.row(0), data.row(i)));
//    EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(i)));
//  }

// Accumulate more data with same meta data but different polno
  r1.define("POLNO", 1);
  r1.define("DATA", data.row(1));
  r1.define("FLAG", flag.row(1));
  r1.define("FLAG_ROW", flag_row[1]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());

// Get
  ClearRecord(output_record);
  CASA_ASSERT_STREQ(poltype, a.getPolType(0));
  ASSERT_EQ(2u, a.getNumPol(0));
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  output_data.assign(output_record.asArrayFloat("FLOAT_DATA"));
  output_flag.assign(output_record.asArrayBool("FLAG"));
  IPosition const expected_shape2(2, 2, num_chan);
  EXPECT_EQ(expected_shape2, output_data.shape());
  EXPECT_EQ(expected_shape2, output_flag.shape());
  EXPECT_TRUE(allEQ(output_data.row(0), data.row(0)));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_TRUE(allEQ(output_data.row(1), data.row(1)));
  EXPECT_TRUE(allEQ(output_flag.row(1), flag.row(1)));
  EXPECT_EQ(flag_row[0] || flag_row[1], output_record.asBool("FLAG_ROW"));

// Accumulate cross-pol data with same meta data
  r1.define("POLNO", 2);
  r1.define("DATA", data.row(2));
  r1.define("FLAG", flag.row(2));
  r1.define("FLAG_ROW", flag_row[2]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  r1.define("POLNO", 3);
  r1.define("DATA", data.row(3));
  r1.define("FLAG", flag.row(3));
  r1.define("FLAG_ROW", flag_row[3]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());

// Get
  ClearRecord(output_record);
  CASA_ASSERT_STREQ(poltype, a.getPolType(0));
  ASSERT_EQ(4u, a.getNumPol(0));
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Complex > output_cdata = output_record.asArrayComplex("DATA");
  output_flag.assign(output_record.asArrayBool("FLAG"));
  IPosition const expected_shape4(2, 4, num_chan);
  EXPECT_EQ(expected_shape4, output_cdata.shape());
  EXPECT_EQ(expected_shape4, output_flag.shape());
  EXPECT_TRUE(allEQ(real(output_cdata.row(0)), data.row(0)));
  EXPECT_TRUE(allEQ(imag(output_cdata.row(0)), 0.0f));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_TRUE(allEQ(real(output_cdata.row(1)), data.row(2)));
  EXPECT_TRUE(allEQ(imag(output_cdata.row(1)), data.row(3)));
  EXPECT_TRUE(allEQ(output_flag.row(1), (flag.row(2) || flag.row(3))));
  EXPECT_TRUE(allEQ(conj(output_cdata.row(1)), output_cdata.row(2)));
  EXPECT_TRUE(allEQ(output_flag.row(1), output_flag.row(2)));
  EXPECT_TRUE(allEQ(real(output_cdata.row(3)), data.row(1)));
  EXPECT_TRUE(allEQ(imag(output_cdata.row(3)), 0.0f));
  EXPECT_TRUE(allEQ(output_flag.row(3), flag.row(1)));
  EXPECT_EQ(anyTrue(flag_row), output_record.asBool("FLAG_ROW"));

// Accumulate data with another meta data
  String intent2 = "OFF_SOURCE";
  String poltype2 = "circular";
  r1.define("POLNO", 0);
  r1.define("SPECTRA_WINDOW_ID", spw_id + 1);
  r1.define("FIELD_ID", field_id + 1);
  r1.define("FEED_ID", feed_id + 1);
  r1.define("INTENT", intent2);
  r1.define("POL_TYPE", poltype2);
  Vector < Float > data2(8, -1.0f);
  Vector < Bool > flag2(8, False);
  r1.define("DATA", data2);
  r1.define("FLAG", flag2);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(4u, a.getNumPol(0));
  EXPECT_EQ(1u, a.getNumPol(1));
  CASA_EXPECT_STREQ(poltype, a.getPolType(0));
  CASA_EXPECT_STREQ(poltype2, a.getPolType(1));
  ClearRecord(output_record);
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Complex > output_cdata2 = output_record.asArrayComplex("DATA");
  EXPECT_EQ(output_cdata.shape(), output_cdata2.shape());
  EXPECT_TRUE(allEQ(output_cdata, output_cdata2));
  ClearRecord(output_record);
  status = a.get(1, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Float > output_data2 = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 1, 8), output_data2.shape());
  EXPECT_TRUE(allEQ(output_data2.row(0), data2));
  Matrix < Bool > output_flag2 = output_record.asArrayBool("FLAG");
  EXPECT_EQ(output_data2.shape(), output_flag2.shape());
  EXPECT_TRUE(allEQ(output_flag2.row(0), flag2));

// Accumulate another
  r1.define("POLNO", 1);
  data2 *= 2.0f;
  flag2 = True;
  r1.define("DATA", data2);
  r1.define("FLAG", flag2);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(2u, a.getNumPol(1));
  ClearRecord(output_record);
  status = a.get(1, output_record);
  IsValidOutputRecord(output_record);
  output_data2.assign(output_record.asArrayFloat("FLOAT_DATA"));
  output_flag2.assign(output_record.asArrayBool("FLAG"));
  EXPECT_EQ(IPosition(2, 2, 8), output_data2.shape());
  EXPECT_TRUE(allEQ(data2, output_data2.row(1)));
  EXPECT_EQ(output_data2.shape(), output_flag2.shape());
  EXPECT_TRUE(allEQ(flag2, output_flag2.row(1)));

// clear
  a.clear();
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(0u, a.getNumberOfActiveChunks());
  EXPECT_FALSE(a.queryForGet(r1));

// reuse underlying DataChunk object
}

int main(int nArgs, char * args[]) {
  ::testing::InitGoogleTest(&nArgs, args);
  std::cout << "SingleDishMSFiller test " << std::endl;
  return RUN_ALL_TESTS();
}

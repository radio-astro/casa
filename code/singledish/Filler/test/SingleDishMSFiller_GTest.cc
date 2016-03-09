/*
 * SingleDishMSFiller_Gtest.cc
 *
 *  Created on: Jan 8, 2016
 *      Author: nakazato
 */

#include <gtest/gtest.h>

#include <singledish/Filler/FillerUtil.h>
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
using namespace sdfiller;

#define CASA_EXPECT_STREQ(expected, actual) EXPECT_STREQ((expected).c_str(), (actual).c_str())
#define CASA_ASSERT_STREQ(expected, actual) ASSERT_STREQ((expected).c_str(), (actual).c_str())
#define CASA_EXPECT_ARRAYEQ(expected, actual) \
  EXPECT_TRUE((expected).conform((actual))); \
  if (!(expected).empty()) { \
    EXPECT_TRUE(allEQ((expected), (actual))); \
  }

namespace {
template<class T>
Vector<T> getScalarColumn(Table const &table, String const &name) {
  ROScalarColumn<T> col(table, name);
  return col.getColumn();
}
template<class T>
Array<T> getArrayColumn(Table const &table, String const &name) {
  ArrayColumn<T> col(table, name);
  return col.getColumn();
}
}

class SingleDishMSFillerTestBase: public ::testing::Test {
public:
  virtual void SetUp() {
    my_ms_name_ = getMSName();
    my_data_name_ = getDataName();
    std::string const data_path = test_utility::GetCasaDataPath()
        + "/regression/unittest/importasap/";

    copyData(data_path);
    ASSERT_TRUE(File(my_data_name_).exists());
    deleteTable(my_ms_name_);
  }

  virtual void TearDown() {
    cleanup();
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
    RegularFile r(src);
    r.copy(dst);
  }
  void copySymLink(String const &src, String const &dst) {
    Path p = SymLink(src).followSymLink();
    String actual_src = p.absoluteName();
    File f(actual_src);
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
      File f = iter.file();
      Path psrc(src);
      Path pdst(dst);
      psrc.append(name);
      String sub_src = psrc.absoluteName();
      pdst.append(name);
      String sub_dst = pdst.absoluteName();
      if (f.isSymLink()) {
        copySymLink(sub_src, sub_dst);
      } else if (f.isRegular()) {
        copyRegular(sub_src, sub_dst);
      } else if (f.isDirectory()) {
        copyDirectory(sub_src, sub_dst);
      }
      iter++;
    }
  }
  void copyData(std::string const &data_dir) {
    if (my_data_name_.size() > 0) {
      std::string full_path = data_dir + my_data_name_;
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
    if (my_data_name_.size() > 0) {
      File f(my_data_name_);
      if (f.isRegular()) {
        RegularFile r(my_data_name_);
        r.remove();
      } else if (f.isDirectory()) {
        Directory d(my_data_name_);
        d.removeRecursive();
      }
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
    return "uid___A002_X85c183_X36f.test.asap";
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

class DISABLED_SingleDishMSFillerTestPerformance: public SingleDishMSFillerTestBase {
  virtual std::string getDataName() {
    return "uid___A002_X85c183_X36f.PM04.asap";
  }
  virtual std::string getMSName() {
    return "performance.ms";
  }
};

TEST_F(SingleDishMSFillerTestFloat, FillerTest) {
  // Create filler
  std::cout << "create filler" << std::endl;
  SingleDishMSFiller<Scantable2MSReader> filler(my_data_name_);

  // Run filler
  std::cout << "run filler" << std::endl;
  ExecuteFiller(filler);

  // verify table contents
  MeasurementSet myms(my_ms_name_);
  Table myscantable(my_data_name_, Table::Old);
  TableRecord const &scantable_header = myscantable.keywordSet();
  constexpr double kDay2Sec = 86400.0;

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
    //uInt expected_nrow = weather_table.nrow();
    ROScalarColumn < uInt > weather_id_column(myscantable, "WEATHER_ID");
    Vector < uInt > weather_id_list = weather_id_column.getColumn();
    Sort sorter;
    sorter.sortKey(weather_id_list.data(), TpUInt, 0, Sort::Ascending);
    Vector < uInt > index_vector;
    uInt expected_nrow = sorter.sort(index_vector, weather_id_list.size(),
        Sort::QuickSort | Sort::NoDuplicates);
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSWeatherColumns mycolumns(mytable);
    ROTableRow row(weather_table);
    std::map<uInt, uInt> weather_id_map;
    weather_id_column.attach(weather_table, "ID");
    for (uInt i = 0; i < weather_id_column.nrow(); ++i) {
      weather_id_map[weather_id_column(i)] = i;
    }
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      uInt weather_id = weather_id_list[index_vector[i]];
      uInt irow_weather = weather_id_map[weather_id];
      TableRecord row_record = row.get(irow_weather);
      Double time_min = 1.0e30;
      Double time_max = 0.0;
      Double expected_time = 0.0;
      Double expected_interval = 0.0;
      auto subtable = myscantable(myscantable.col("WEATHER_ID") == weather_id);
      if (subtable.nrow() > 0) {
        ROScalarColumn < uInt > id_column(subtable, "IFNO");
        Vector < uInt > spw_list = id_column.getColumn();
        Sort ss;
        ss.sortKey(spw_list.data(), TpUInt, 0, Sort::Ascending);
        Vector < uInt > sidx;
        uInt num_spw = ss.sort(sidx, spw_list.size(),
            Sort::QuickSort | Sort::NoDuplicates);
        for (uInt ispw = 0; ispw < num_spw; ++ispw) {
          uInt spw_id = spw_list[sidx[ispw]];
          auto subsubtable = subtable(subtable.col("IFNO") == spw_id);
          ROScalarColumn < Double > tcol(subsubtable, "TIME");
          ROScalarColumn < Double > icol(subsubtable, "INTERVAL");
          Vector < Double > time_list = tcol.getColumn();
          Vector < Double > interval_list = icol.getColumn();
          Sort ts;
          ts.sortKey(time_list.data(), TpDouble, 0, Sort::Ascending);
          Vector < uInt > tidx;
          ts.sort(tidx, time_list.size());
          uInt id_min = tidx[0];
          uInt id_max = tidx[tidx.size() - 1];
          Double time_min_in = time_list[id_min] * kDay2Sec
              - 0.5 * interval_list[id_min];
          Double time_max_in = time_list[id_max] * kDay2Sec
              + 0.5 * interval_list[id_max];
          time_min = min(time_min, time_min_in);
          time_max = max(time_max, time_max_in);
          expected_time = (time_max + time_min) * 0.5;
          expected_interval = time_max - time_min;
        }
      }
//      EXPECT_EQ(expected_time, mycolumns.time()(i));
//      EXPECT_EQ(expected_interval, mycolumns.interval()(i));
      Double diff_time = abs(
          (mycolumns.time()(i) - expected_time) / expected_time);
      Double diff_interval = abs(
          (mycolumns.interval()(i) - expected_interval) / expected_interval);
      std::cout << "diff_time = " << diff_time << " diff_interval = "
          << diff_interval << std::endl;
      Double const eps = 1.0e-5;
      EXPECT_LT(diff_time, eps);
      EXPECT_LT(diff_interval, eps);
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

  // verify SOURCE table
  Record source_map;
  {
    std::cout << "verify SOURCE table" << std::endl;
    auto const mytable = myms.source();
    auto const molecules_table = scantable_header.asTable("MOLECULES");
    ROScalarColumn < String > srcname_column(myscantable, "SRCNAME");
    ROScalarColumn < uInt > ifno_column(myscantable, "IFNO");
    ROScalarColumn < uInt > molecule_id_column(myscantable, "MOLECULE_ID");
    ArrayColumn<Double> proper_motion_column(myscantable, "SRCPROPERMOTION");
    ROScalarColumn < Double > sysvel_column(myscantable, "SRCVELOCITY");
    Vector < String > srcname_list = srcname_column.getColumn();
    Vector < uInt > ifno_list = ifno_column.getColumn();
    Sort sorter;
    sorter.sortKey(srcname_list.data(), TpString);
    Vector < uInt > index_vector;
    uInt num_source = sorter.sort(index_vector, myscantable.nrow(),
        Sort::QuickSort | Sort::NoDuplicates);
    for (uInt i = 0; i < num_source; ++i) {
      source_map.define(srcname_list[index_vector[i]], (Int) i);
    }
    sorter.sortKey(ifno_list.data(), TpUInt);
    index_vector.resize();
    uInt expected_nrow = sorter.sort(index_vector, myscantable.nrow(),
        Sort::QuickSort | Sort::NoDuplicates);
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSSourceColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      uInt j = index_vector[i];
      String expected_name = srcname_list[j];
      uInt expected_spw_id = ifno_list[j];
      Int expected_source_id = source_map.asInt(expected_name);
      EXPECT_EQ((Int )expected_spw_id, mycolumns.spectralWindowId()(i));
      CASA_EXPECT_STREQ(expected_name, mycolumns.name()(i));
      EXPECT_EQ(expected_source_id, mycolumns.sourceId()(i));
      EXPECT_TRUE(allEQ(proper_motion_column(j), mycolumns.properMotion()(i)));
      uInt molecule_id = molecule_id_column(j);

      Table t = molecules_table(molecules_table.col("ID") == molecule_id, 1);
      ASSERT_EQ(1u, t.nrow());
      ArrayColumn<Double> rest_freq_column(t, "RESTFREQUENCY");
      ArrayColumn<String> transition_column(t, "NAME");
      Vector < Double > expected_rest_freq;
      Vector < String > expected_transition;
      if (rest_freq_column.isDefined(0)
          && rest_freq_column.shape(0).product() > 0) {
        Vector < Double > expected_rest_freq = rest_freq_column(0);
        Int expected_num_lines = expected_rest_freq.size();
        EXPECT_EQ(expected_num_lines, mycolumns.numLines()(i));
        EXPECT_TRUE(allEQ(expected_rest_freq, mycolumns.restFrequency()(i)));
        EXPECT_TRUE(allEQ(mycolumns.sysvel()(i), sysvel_column(j)));
      } else {
        EXPECT_EQ(0, mycolumns.numLines()(i));
        EXPECT_FALSE(mycolumns.restFrequency().isDefined(i));
        EXPECT_FALSE(mycolumns.sysvel().isDefined(i));
      }
      if (transition_column.isDefined(0)
          && transition_column.shape(0).product() > 0) {
        Vector < String > expected_transition = transition_column(0);
        EXPECT_TRUE(allEQ(expected_transition, mycolumns.transition()(i)));
      } else {
        EXPECT_FALSE(mycolumns.transition().isDefined(i));
      }

      t = myscantable(
          myscantable.col("SRCNAME") == expected_name
              && myscantable.col("IFNO") == expected_spw_id);
      ROScalarColumn < Double > time_column(t, "TIME");
      ROScalarColumn < Double > interval_column(t, "INTERVAL");
      Vector < Double > time_list = time_column.getColumn();
      Sort s;
      s.sortKey(time_list.data(), TpDouble);
      Vector < uInt > x;
      uInt m = s.sort(x, t.nrow());
      uInt imin = x[0];
      uInt imax = x[m - 1];
      Double tmin = time_list[imin] * kDay2Sec - 0.5 * interval_column(imin);
      Double tmax = time_list[imax] * kDay2Sec + 0.5 * interval_column(imax);
      Double expected_time = 0.5 * (tmin + tmax);
      Double expected_interval = (tmax - tmin);
      EXPECT_EQ(expected_time, mycolumns.time()(i));
      EXPECT_EQ(expected_interval, mycolumns.interval()(i));
    }
  }

  // verify FIELD table
  {
    std::cout << "verify FIELD table" << std::endl;
    auto const mytable = myms.field();
    ROScalarColumn < String > fieldname_column(myscantable, "FIELDNAME");
    ROScalarColumn < String > srcname_column(myscantable, "SRCNAME");
    ROScalarColumn < Double > time_column(myscantable, "TIME");
    ArrayColumn<Double> srcdir_column(myscantable, "SRCDIRECTION");
    ArrayColumn<Double> scanrate_column(myscantable, "SCANRATE");
    Vector < String > fieldname_list = fieldname_column.getColumn();
    Sort sorter;
    sorter.sortKey(fieldname_list.data(), TpString);
    Vector < uInt > index_vector;
    uInt num_records = sorter.sort(index_vector, myscantable.nrow(),
        Sort::QuickSort | Sort::NoDuplicates);
    ROMSFieldColumns mycolumns(mytable);
    std::vector<uInt> processed_rows;
    for (uInt i = 0; i < num_records; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      uInt j = index_vector[i];
      String field_name = fieldname_list(j);
      auto pos = field_name.find("__");
      String expected_name = fieldname_list(j);
      Int field_id = i;
      if (pos != String::npos) {
        expected_name = field_name.substr(0, pos);
        field_id = String::toInt(field_name.substr(pos + 2));
      }
      ASSERT_GE(field_id, 0);
      ASSERT_LT((uInt )field_id, mycolumns.nrow());
      CASA_EXPECT_STREQ(expected_name, mycolumns.name()(field_id));
      Int expected_source_id = source_map.asInt(srcname_column(j));
      EXPECT_EQ(expected_source_id, mycolumns.sourceId()(field_id));
      Double expected_time = time_column(j) * kDay2Sec;
      EXPECT_EQ(expected_time, mycolumns.time()(field_id));
      Vector < Double > expected_direction = srcdir_column(j);
      Vector < Double > expected_scan_rate = scanrate_column(j);
      Matrix < Double > direction = mycolumns.phaseDir()(field_id);
      if (allEQ(expected_scan_rate, 0.0)) {
        IPosition expected_shape(2, 2, 1);
        EXPECT_EQ(expected_shape, direction.shape());
        EXPECT_TRUE(allEQ(expected_direction, direction.column(0)));
      } else {
        IPosition expected_shape(2, 2, 2);
        EXPECT_EQ(expected_shape, direction.shape());
        EXPECT_TRUE(allEQ(expected_direction, direction.column(0)));
        EXPECT_TRUE(allEQ(expected_scan_rate, direction.column(1)));
      }
      CASA_EXPECT_STREQ(String("J2000"),
          mycolumns.phaseDir().keywordSet().asRecord("MEASINFO").asString(
              "Ref"));
    }
  }

  // Verify MAIN
  // The test data should contain
  //    - spws 0 (1 pol), 9, 10, 17, 18 (2 pols)
  //    - scans 0, 5, 6
  //    - cycles < 20
  // DD_ID 0 WVR (1 pol)
  // DD_ID > 0 non-WVR (2 pol)
  {
    std::cout << "Verifying MAIN and STATE" << std::endl;
    ASSERT_TRUE(myms.tableDesc().isColumn("FLOAT_DATA"));
    ROMSMainColumns mycolumns(myms);
    ROMSDataDescColumns data_desc_columns(myms.dataDescription());
    ROMSFieldColumns field_columns(myms.field());
    ROMSStateColumns state_columns(myms.state());
    ROMSPolarizationColumns pol_columns(myms.polarization());
    ROMSSpWindowColumns spw_columns(myms.spectralWindow());
    uInt const nrow = myms.nrow();
    Table t = myscantable(myscantable.col("IFNO") == (uInt) 0);
    uInt expected_nrow = t.nrow() + (myscantable.nrow() - t.nrow()) / 2;
    ASSERT_EQ(expected_nrow, nrow);
    // alias
    Table &s = myscantable;
    for (uInt i = 0; i < nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;

      // some trivial stuff
      EXPECT_EQ(0, mycolumns.antenna1()(i));
      EXPECT_EQ(mycolumns.antenna1()(i), mycolumns.antenna2()(i));
      EXPECT_EQ(IPosition(1, 3), mycolumns.uvw().shape(i));
      EXPECT_TRUE(allEQ(mycolumns.uvw()(i), 0.0));
      EXPECT_EQ(mycolumns.feed1()(i), mycolumns.feed2()(i));
      EXPECT_EQ(mycolumns.time()(i), mycolumns.timeCentroid()(i));
      EXPECT_EQ(mycolumns.interval()(i), mycolumns.exposure()(i));
      EXPECT_TRUE(allEQ(mycolumns.sigma()(i), 1.0f));
      EXPECT_TRUE(allEQ(mycolumns.sigma()(i), mycolumns.weight()(i)));

      // row matching
      Double time = mycolumns.time()(i);
      Double interval = mycolumns.interval()(i);
      Int field_id = mycolumns.fieldId()(i);
      Int feed_id = mycolumns.feed1()(i);
      Int data_desc_id = mycolumns.dataDescId()(i);
      Int state_id = mycolumns.stateId()(i);
      Int scan = mycolumns.scanNumber()(i);
      Bool flag_row = mycolumns.flagRow()(i);
      Int subscan = state_columns.subScan()(state_id);
      Int pol_id = data_desc_columns.polarizationId()(data_desc_id);
      Int spw_id = data_desc_columns.spectralWindowId()(data_desc_id);
      String field_name = field_columns.name()(field_id) + "__"
          + String::toString(field_id);
      Int num_corr = pol_columns.numCorr()(pol_id);
      Int num_chan = spw_columns.numChan()(spw_id);
      std::cout << "TIME " << time / kDay2Sec << " BEAM " << feed_id
          << " FEILD " << field_name << " SPW " << spw_id << std::endl;
      Table ss = s(
          s.col("TIME") * kDay2Sec == time && s.col("BEAMNO") == feed_id
              && s.col("FIELDNAME") == field_name && s.col("IFNO") == spw_id);
      Table so = ss.sort("POLNO", Sort::Ascending);
      EXPECT_TRUE(allEQ(getScalarColumn<uInt>(so, "SCANNO"), (uInt )scan));
      EXPECT_TRUE(allEQ(getScalarColumn<uInt>(so, "CYCLENO"), (uInt )subscan));
      Vector < Double > sc_interval = getScalarColumn<Double>(so, "INTERVAL");
      EXPECT_TRUE(allEQ(sc_interval, interval));
      EXPECT_EQ(anyGT(getScalarColumn<uInt>(so, "FLAGROW"), 0u), flag_row);
      EXPECT_EQ(so.nrow(), (uInt )num_corr);
      EXPECT_EQ(IPosition(1, num_corr), mycolumns.sigma().shape(i));
      IPosition expected_shape(2, num_corr, num_chan);
      EXPECT_EQ(expected_shape, mycolumns.floatData().shape(i));
      EXPECT_EQ(expected_shape, mycolumns.flag().shape(i));

      // flag and data
      Matrix < uChar > sc_flag(getArrayColumn<uChar>(so, "FLAGTRA"));
      Matrix < Bool > expected_flag(sc_flag.shape());
      convertArray(expected_flag, sc_flag);
      Matrix < Float > expected_data(getArrayColumn<Float>(so, "SPECTRA"));
      Matrix < Float > data = mycolumns.floatData()(i);
      Matrix < Bool > flag = mycolumns.flag()(i);
      if (spw_id == 0) {
        std::cout << "1 pol" << std::endl;
        EXPECT_EQ(1u, so.nrow());
        EXPECT_TRUE(allEQ(expected_flag.column(0), flag.row(0)));
        EXPECT_TRUE(allEQ(expected_data.column(0), data.row(0)));
      } else {
        std::cout << "2 pols" << std::endl;
        EXPECT_EQ(2u, so.nrow());
        EXPECT_TRUE(allEQ(expected_flag.column(0), flag.row(0)));
        EXPECT_TRUE(allEQ(expected_flag.column(1), flag.row(1)));
        EXPECT_TRUE(allEQ(expected_data.column(0), data.row(0)));
        EXPECT_TRUE(allEQ(expected_data.column(1), data.row(1)));
      }

    }
  }

  // Verify SPECTRAL_WINDOW
  {
    constexpr Int kExpectedNumRow = 19;
    Int const valid_spw_list[] = { 0, 9, 10, 17, 18 };
    Vector<Int> const valid_spw_vector(IPosition(1, 5),
        const_cast<Int *>(valid_spw_list), SHARE);
    std::cout << "verify SPECTRAL_WINDOW table" << std::endl;
    auto const mytable = myms.spectralWindow();
    ROMSSpWindowColumns mycolumns(mytable);
    Table frequencies_table = myscantable.keywordSet().asTable("FREQUENCIES");
    ASSERT_EQ((uInt )kExpectedNumRow, mytable.nrow());
    for (Int irow = 0; irow < kExpectedNumRow; ++irow) {
      std::cout << "Verifying row " << irow << std::endl;
      if (anyEQ(valid_spw_vector, irow)) {
        Table t = myscantable(myscantable.col("IFNO") == (uInt) irow, 1);
        ASSERT_EQ(1u, t.nrow());
        ROScalarColumn < uInt > freq_id_column(t, "FREQ_ID");
        ArrayColumn<uChar> flag_column(t, "FLAGTRA");
        Int expected_num_chan = flag_column.shape(0)[0];
        uInt freq_id = freq_id_column(0);
        t = frequencies_table(frequencies_table.col("ID") == freq_id, 1);
        ASSERT_EQ(1u, t.nrow());
        ROScalarColumn < Double > column(t, "REFPIX");
        Double refpix = column(0);
        column.attach(t, "REFVAL");
        Double refval = column(0);
        column.attach(t, "INCREMENT");
        Double increment = column(0);
        Int expected_sideband = (increment > 0.0) ? 0 : 1;
        Double expected_bandwidth = expected_num_chan * abs(increment);
        Vector < Double > expected_chan_freq(expected_num_chan);
        Double expected_ref_freq = refval - refpix * increment;
        indgen(expected_chan_freq, expected_ref_freq, increment);
        String freq_frame = frequencies_table.keywordSet().asString(
            "BASEFRAME");
        MFrequency::Types expected_type;
        Bool status = MFrequency::getType(expected_type, freq_frame);
        ASSERT_TRUE(status);

        EXPECT_EQ(expected_num_chan, mycolumns.numChan()(irow));
        EXPECT_EQ(expected_num_chan, mycolumns.chanFreq().shape(irow)[0]);
        EXPECT_EQ(expected_num_chan, mycolumns.chanWidth().shape(irow)[0]);
        EXPECT_EQ(expected_num_chan, mycolumns.effectiveBW().shape(irow)[0]);
        EXPECT_TRUE(allEQ(expected_chan_freq, mycolumns.chanFreq()(irow)));
        EXPECT_TRUE(allEQ(mycolumns.chanWidth()(irow), increment));
        EXPECT_TRUE(allEQ(mycolumns.effectiveBW()(irow), abs(increment)));
        EXPECT_EQ(expected_ref_freq, mycolumns.refFrequency()(irow));
        EXPECT_EQ(expected_sideband, mycolumns.netSideband()(irow));
        EXPECT_EQ(expected_bandwidth, mycolumns.totalBandwidth()(irow));
        EXPECT_EQ(expected_type, mycolumns.measFreqRef()(irow));
      } else {
        EXPECT_EQ(1, mycolumns.numChan()(irow));
        EXPECT_EQ(0.0, mycolumns.refFrequency()(irow));
        EXPECT_EQ(0.0, mycolumns.totalBandwidth()(irow));
        Vector < Double > chan_freq = mycolumns.chanFreq()(irow);
        EXPECT_EQ(1u, chan_freq.size());
        EXPECT_EQ(0.0, chan_freq[0]);
        Vector < Double > chan_width = mycolumns.chanWidth()(irow);
        EXPECT_EQ(1u, chan_width.size());
        EXPECT_EQ(0.0, chan_width[0]);
        Vector < Double > effbw = mycolumns.effectiveBW()(irow);
        EXPECT_EQ(1u, effbw.size());
        EXPECT_EQ(0.0, effbw[0]);
      }
      EXPECT_TRUE(
          allEQ(mycolumns.effectiveBW()(irow), mycolumns.resolution()(irow)));
    }
  }

  // Verify POLARIZATION
  // should have two rows that corresponds to WVR (1 pol) and others (2 pols)
  {
    std::cout << "Verify POLARIZATION table" << std::endl;
    auto const mytable = myms.polarization();
    ROMSPolarizationColumns const mycolumns(mytable);
    ASSERT_EQ(2u, mycolumns.nrow());
    Vector < Int > expected_corr_type(2);
    String poltype = myscantable.keywordSet().asString("POLTYPE");
    if (poltype == "linear") {
      expected_corr_type[0] = Stokes::XX;
      expected_corr_type[1] = Stokes::YY;
    } else if (poltype == "circular") {
      expected_corr_type[0] = Stokes::RR;
      expected_corr_type[1] = Stokes::LL;
    }
    EXPECT_EQ(1, mycolumns.numCorr()(0));
    Vector < Int > corr_type = mycolumns.corrType()(0);
    EXPECT_EQ(1u, corr_type.size());
    EXPECT_EQ(expected_corr_type[0], corr_type[0]);
    EXPECT_EQ(2, mycolumns.numCorr()(1));
    corr_type.assign(mycolumns.corrType()(1));
    EXPECT_EQ(2u, corr_type.size());
    EXPECT_EQ(expected_corr_type[0], corr_type[0]);
    EXPECT_EQ(expected_corr_type[1], corr_type[1]);
  }

  // Verify DATA_DESCRIPTION
  // expected configuration is
  // DD_ID POL_ID SPW_ID
  //   0      0      0
  //   1      1      9
  //   2      1     10
  //   3      1     17
  //   4      1     18
  {
    std::cout << "Verify DATA_DESCRIPTION table" << std::endl;
    auto const mytable = myms.dataDescription();
    ROMSDataDescColumns const mycolumns(mytable);
    ASSERT_EQ(5u, mytable.nrow());
    EXPECT_EQ(0, mycolumns.polarizationId()(0));
    EXPECT_EQ(0, mycolumns.spectralWindowId()(0));
    EXPECT_EQ(1, mycolumns.polarizationId()(1));
    EXPECT_EQ(9, mycolumns.spectralWindowId()(1));
    EXPECT_EQ(1, mycolumns.polarizationId()(2));
    EXPECT_EQ(10, mycolumns.spectralWindowId()(2));
    EXPECT_EQ(1, mycolumns.polarizationId()(3));
    EXPECT_EQ(17, mycolumns.spectralWindowId()(3));
    EXPECT_EQ(1, mycolumns.polarizationId()(4));
    EXPECT_EQ(18, mycolumns.spectralWindowId()(4));
  }

  // Verify POINTING
  {
    std::cout << "Verify POINTING table" << std::endl;
    auto const mytable = myms.pointing();
    ROMSPointingColumns const mycolumns(mytable);
    uInt nrow = mytable.nrow();
    Table t = myscantable;
    ROScalarColumn < Double > time_column(t, "TIME");
    ROScalarColumn < Double > interval_column(t, "INTERVAL");
    ArrayColumn<Double> direction_column(t, "DIRECTION");
    ArrayColumn<Double> scanrate_column(t, "SCANRATE");
    Vector < Double > time_list = time_column.getColumn();
    Sort s;
    s.sortKey(time_list.data(), TpDouble);
    Vector < uInt > index_vector;
    uInt expected_nrow = s.sort(index_vector, myscantable.nrow(),
        Sort::QuickSort | Sort::NoDuplicates);
    ASSERT_EQ(expected_nrow, nrow);
    for (uInt irow = 0; irow < nrow; ++irow) {
      std::cout << "Verifying row " << irow << std::endl;
      uInt index = index_vector[irow];
      EXPECT_EQ(0, mycolumns.antennaId()(irow));
      Double expected_time = time_list[index] * kDay2Sec;
      Table tsel = t(t.col("TIME") == time_list[index]);
      Table tsort = tsel.sort("IFNO");
      ROScalarColumn < Double > col(tsort, "INTERVAL");
      Double expected_interval = col(0);
      Vector < Double > expected_direction = direction_column(index);
      Vector < Double > expected_scanrate = scanrate_column(index);
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      Matrix < Double > direction = mycolumns.direction()(irow);
      if (allEQ(expected_scanrate, 0.0)) {
        EXPECT_EQ(IPosition(2, 2, 1), direction.shape());
        EXPECT_TRUE(allEQ(expected_direction, direction.column(0)));
      } else {
        EXPECT_EQ(IPosition(2, 2, 2), direction.shape());
        EXPECT_TRUE(allEQ(expected_direction, direction.column(0)));
        EXPECT_TRUE(allEQ(expected_scanrate, direction.column(1)));
      }
    }
  }

  // Verify FEED
  // expected configuration is
  // FEED_ID SPW_ID
  //     0      0
  //     0      9
  //     0     10
  //     0     17
  //     0     18
  {
    std::cout << "Verify FEED table" << std::endl;
    auto const mytable = myms.feed();
    ROMSFeedColumns const mycolumns(mytable);
    ASSERT_EQ(5u, mytable.nrow());
    EXPECT_EQ(0, mycolumns.feedId()(0));
    EXPECT_EQ(0, mycolumns.spectralWindowId()(0));
    EXPECT_EQ(0, mycolumns.feedId()(1));
    EXPECT_EQ(9, mycolumns.spectralWindowId()(1));
    EXPECT_EQ(0, mycolumns.feedId()(2));
    EXPECT_EQ(10, mycolumns.spectralWindowId()(2));
    EXPECT_EQ(0, mycolumns.feedId()(3));
    EXPECT_EQ(17, mycolumns.spectralWindowId()(3));
    EXPECT_EQ(0, mycolumns.feedId()(4));
    EXPECT_EQ(18, mycolumns.spectralWindowId()(4));
  }
}

TEST_F(SingleDishMSFillerTestComplex, FillerTest) {
  // Create filler
  SingleDishMSFiller<Scantable2MSReader> filler(my_data_name_);

  // Run filler
  ExecuteFiller(filler);

  // Here only data shape and values are examined since other information is
  // verified by other tests
  MeasurementSet myms(my_ms_name_);
  Table myscantable(my_data_name_, Table::Old);

  uInt expected_nrow = myscantable.nrow() / 4;
  ASSERT_EQ(expected_nrow, myms.nrow());

  Table t = myscantable.sort("POLNO");
  ArrayColumn<Float> c(t, "SPECTRA");
  Vector < Float > data0real = c(0);
  Vector < Float > data3real = c(1);
  Vector < Float > data1real = c(2);
  Vector < Float > data1imag = c(3);

  ASSERT_TRUE(myms.tableDesc().isColumn("DATA"));
  ArrayColumn<Complex> data_column(myms, "DATA");
  Matrix < Complex > data = data_column(0);
  EXPECT_TRUE(allEQ(data0real, real(data.row(0))));
  EXPECT_TRUE(allEQ(imag(data.row(0)), 0.0f));
  EXPECT_TRUE(allEQ(data1real, real(data.row(1))));
  EXPECT_TRUE(allEQ(data1imag, imag(data.row(1))));
  EXPECT_TRUE(allEQ(conj(data.row(1)), data.row(2)));
  EXPECT_TRUE(allEQ(data3real, real(data.row(3))));
  EXPECT_TRUE(allEQ(imag(data.row(3)), 0.0f));
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
      Vector < Double > chan_width = mycolumns.chanWidth()(spw_id);
      Double incr0 = chan_width[0];
      Double refval = freq0 + expected_refpix * incr0;
      Double bandwidth = abs(expected_increment) * num_chan;
      EXPECT_EQ(bandwidth, mycolumns.totalBandwidth()(spw_id));
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
  Cube<Double> pointing_direction(IPosition(3, 2, 2, kNumPointing));
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
    auto const &expected_record = reader.main_record_;
    constexpr size_t expected_nfield = 24;
    constexpr size_t expected_nrow = 16;
    ASSERT_EQ(expected_nfield, expected_record.size());
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
      DataRecord const row_record0 = expected_record.at(record_index);
      Int expected_subscan = row_record0.subscan;
      String expected_intent = row_record0.intent;
      auto verifyMeta =
          [&](DataRecord const &record, size_t krow) {
            Double expected_time = record.time;
            std::cout << "irow " << krow << " time " << mycolumns.time()(krow) << " (expected " << expected_time << ")" << std::endl;
            Double expected_interval = record.interval;
            Int expected_feed = record.feed_id;
            Int expected_field = record.field_id;
            Int expected_scan = record.scan;
            Int expected_antenna = record.antenna_id;
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
      DataRecord const row_record1 = expected_record.at(record_index);
      Bool expected_flag_row = row_record0.flag_row || row_record1.flag_row;
      Int expected_num_pol = 2;
      Float expected_weight = 1.0;
      Int expected_spw_id = row_record0.spw_id;
      Int num_chan = -1;
      ASSERT_NO_THROW( {
        num_chan = num_chan_map[expected_spw_id]
        ;
      }
);
                                                                                                                                                                                                    ASSERT_GT(num_chan, 0);
      Matrix < Bool > expected_flag(expected_num_pol, num_chan);
      Matrix < Float > expected_data(expected_num_pol, num_chan);
      uInt pol_id0 = row_record0.polno;
      uInt pol_id1 = row_record1.polno;
      ASSERT_LT(pol_id0, 2u);
      ASSERT_GE(pol_id0, 0u);
      ASSERT_LT(pol_id1, 2u);
      ASSERT_GE(pol_id1, 0u);
      ASSERT_NE(pol_id0, pol_id1);
      expected_flag.row(pol_id0) = row_record0.flag;
      expected_flag.row(pol_id1) = row_record1.flag;
      expected_data.row(pol_id0) = row_record0.data;
      expected_data.row(pol_id1) = row_record1.data;
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
          [&](size_t krow, DataRecord const &record) {
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
                Matrix<Double> direction = record.direction;
                Int num_poly = direction.shape()[1] - 1;
                if (allEQ(record.scan_rate, 0.0)) {
                  num_poly = 0;
                }
                pointing_num_poly[pointing_count] = num_poly;
                pointing_direction.xyPlane(pointing_count) = direction;
                ++pointing_count;
                ASSERT_LE(pointing_count, kNumPointing);
              }
            }
          };
      addPointing(irow, row_record0);

      // next row
      ++record_index;
      ++irow;
      std::cout << "Verifying row " << irow << " (record " << record_index
          << ")" << std::endl;
      key = generateKey(record_index);
      DataRecord const row_record2 = expected_record.at(record_index);
      expected_subscan = row_record2.subscan;
      expected_intent = row_record2.intent;
      verifyMeta(row_record2, irow);
      expected_flag_row = row_record2.flag_row;
      expected_num_pol = 1;
      expected_spw_id = row_record2.spw_id;
      ASSERT_NO_THROW( {
        num_chan = num_chan_map[expected_spw_id]
        ;
      }
);
                                                                                                                                                                                                    ASSERT_GT(num_chan, 0);
      expected_flag.resize(expected_num_pol, num_chan);
      expected_data.resize(expected_flag.shape());
      uInt pol_id2 = row_record2.polno;
      ASSERT_EQ(0u, pol_id2);
      expected_flag.row(pol_id2) = row_record2.flag;
      expected_data.row(pol_id2) = row_record2.data;
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
      Matrix < Double > expected_direction = pointing_direction.xyPlane(j);
      if (allEQ(expected_direction.column(1), 0.0)) {
        EXPECT_TRUE(
            allEQ(expected_direction(IPosition(2, 0, 0), IPosition(2, 1, 0)),
                mycolumns.direction()(i)));
      } else {
        EXPECT_TRUE(
            allEQ(pointing_direction.xyPlane(j), mycolumns.direction()(i)));
      }
    }

    // verify SYSCAL table
    // TSYS should be same per antenna, per feed, per spw
    // ANTENNA FEED SPW TSYS
    //    0      0   0  [100, 200] (scalar)
    //    0      0   1  [200] (scalar)
    //    0      1   0  [100, 200] (spectral)
    //    0      1   1  [200] (spectral)
    //    1      0   0  [200, 400] (scalar)
    //    1      0   1  [300] (scalar)
    //    1      1   0  [200, 400] (spectral)
    //    1      1   1  [300] (spectral)
    {
      std::cout << "Verify SYSCAL table" << std::endl;
      auto const mytable = myms.sysCal();
      //map<uInt, SysCalRecord> expected_record = reader.syscal_record_;
      uInt expected_nrow = 8;
      ASSERT_EQ(expected_nrow, mytable.nrow());
      ROMSSysCalColumns mycolumns(mytable);
      auto const &main_record = reader.main_record_;
      uInt n = main_record.size();
      Double time0 = main_record.at(0).time;
      Double interval0 = main_record.at(0).interval;
      Double time1 = main_record.at(n - 1).time;
      Double interval1 = main_record.at(n - 1).interval;
      Double expected_time = (time0 - interval0 / 2 + time1 + interval1 / 2)
          / 2;
      Double expected_interval = (time1 + interval1 / 2)
          - (time0 - interval0 / 2);
      // row 0
      uInt irow = 0;
      EXPECT_EQ(0, mycolumns.antennaId()(irow));
      EXPECT_EQ(0, mycolumns.feedId()(irow));
      EXPECT_EQ(0, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      Vector < Float > tsys_dualpol_scalar(2);
      tsys_dualpol_scalar[0] = 100.0;
      tsys_dualpol_scalar[1] = 200.0;
      EXPECT_TRUE(allEQ(tsys_dualpol_scalar, mycolumns.tsys()(irow)));

      // row 1
      ++irow;
      EXPECT_EQ(0, mycolumns.antennaId()(irow));
      EXPECT_EQ(0, mycolumns.feedId()(irow));
      EXPECT_EQ(1, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      Vector < Float > tsys_singlepol_scalar(1);
      tsys_singlepol_scalar[0] = 200.0;
      EXPECT_TRUE(allEQ(tsys_singlepol_scalar, mycolumns.tsys()(irow)));

      // row 2
      ++irow;
      EXPECT_EQ(0, mycolumns.antennaId()(irow));
      EXPECT_EQ(1, mycolumns.feedId()(irow));
      EXPECT_EQ(0, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      Matrix < Float > tsys_dualpol_spectral(2, num_chan_map[0]);
      tsys_dualpol_spectral.row(0) = 100.0;
      tsys_dualpol_spectral.row(1) = 200.0;
      EXPECT_TRUE(allEQ(tsys_dualpol_spectral, mycolumns.tsysSpectrum()(irow)));

      // row 3
      ++irow;
      EXPECT_EQ(0, mycolumns.antennaId()(irow));
      EXPECT_EQ(1, mycolumns.feedId()(irow));
      EXPECT_EQ(1, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      Matrix < Float > tsys_singlepol_spectral(1, num_chan_map[1]);
      tsys_singlepol_spectral.row(0) = 200.0;
      EXPECT_TRUE(
          allEQ(tsys_singlepol_spectral, mycolumns.tsysSpectrum()(irow)));

      // row 4
      ++irow;
      EXPECT_EQ(1, mycolumns.antennaId()(irow));
      EXPECT_EQ(0, mycolumns.feedId()(irow));
      EXPECT_EQ(0, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      tsys_dualpol_scalar[0] = 200.0;
      tsys_dualpol_scalar[1] = 400.0;
      EXPECT_TRUE(allEQ(tsys_dualpol_scalar, mycolumns.tsys()(irow)));

      // row 5
      ++irow;
      EXPECT_EQ(1, mycolumns.antennaId()(irow));
      EXPECT_EQ(0, mycolumns.feedId()(irow));
      EXPECT_EQ(1, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      tsys_singlepol_scalar[0] = 300.0;
      EXPECT_TRUE(allEQ(tsys_singlepol_scalar, mycolumns.tsys()(irow)));

      // row 6
      ++irow;
      EXPECT_EQ(1, mycolumns.antennaId()(irow));
      EXPECT_EQ(1, mycolumns.feedId()(irow));
      EXPECT_EQ(0, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      tsys_dualpol_spectral.row(0) = 200.0;
      tsys_dualpol_spectral.row(1) = 400.0;
      EXPECT_TRUE(allEQ(tsys_dualpol_spectral, mycolumns.tsysSpectrum()(irow)));

      // row 3
      ++irow;
      EXPECT_EQ(1, mycolumns.antennaId()(irow));
      EXPECT_EQ(1, mycolumns.feedId()(irow));
      EXPECT_EQ(1, mycolumns.spectralWindowId()(irow));
      EXPECT_EQ(expected_time, mycolumns.time()(irow));
      EXPECT_EQ(expected_interval, mycolumns.interval()(irow));
      tsys_singlepol_spectral.row(0) = 300.0;
      EXPECT_TRUE(
          allEQ(tsys_singlepol_spectral, mycolumns.tsysSpectrum()(irow)));

    }

    // verify WEATHER table
    {
      std::cout << "Verify WEATHER table" << std::endl;
      auto const mytable = myms.weather();
      constexpr uInt expected_nrow = 2;
      ASSERT_EQ(expected_nrow, mytable.nrow());
      ROMSWeatherColumns mycolumns(mytable);
      Float const expected_temperature[expected_nrow] = { 100.0f, 150.0f };
      Double const expected_time = 0.5 * (4.0e9 - 5.0 + 4.1e9 + 5.0);
      Double const expected_interval = 4.1e9 - 4.0e9 + 10.0;
      for (uInt i = 0; i < expected_nrow; ++i) {
        std::cout << "Verifying row " << i << std::endl;
        EXPECT_EQ((Int )i, mycolumns.antennaId()(i));
        EXPECT_EQ(expected_time, mycolumns.time()(i));
        EXPECT_EQ(expected_interval, mycolumns.interval()(i));
        EXPECT_EQ(expected_temperature[i], mycolumns.temperature()(i));
        EXPECT_EQ(0.0f, mycolumns.pressure()(i));
        EXPECT_EQ(0.0f, mycolumns.relHumidity()(i));
        EXPECT_EQ(0.0f, mycolumns.windSpeed()(i));
        EXPECT_EQ(0.0f, mycolumns.windDirection()(i));
      }
    }

  }

}

namespace {

inline void TestShape(size_t const num_pol, size_t const num_chan,
    MSDataRecord const &output_record, String const &output_data_key) {
  IPosition const expected_shape_matrix(2, num_pol, num_chan);
  IPosition const expected_shape_vector(1, num_pol);
  if (output_data_key == "FLOAT_DATA") {
    EXPECT_EQ(expected_shape_matrix, output_record.float_data.shape());
  } else {
    EXPECT_EQ(expected_shape_matrix, output_record.complex_data.shape());
  }
  EXPECT_EQ(expected_shape_matrix, output_record.flag.shape());
  EXPECT_EQ(expected_shape_vector, output_record.sigma.shape());
  EXPECT_EQ(expected_shape_vector, output_record.weight.shape());
}

struct BasicPolarizationTester {
  static void Test(size_t const num_chan, String const &pol_type,
      Vector<uInt> const &polid_list, Vector<DataRecord> const &input_record,
      MSDataRecord const &output_record) {
    size_t const expected_num_pol = polid_list.size();
    size_t const num_pol = output_record.num_pol;
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
      Vector < Int > corr = output_record.corr_type;
      ASSERT_TRUE(allEQ(expected_corr, corr));
    } else {
      Int corr_index = input_record[0].polno;
      Vector < Int > expected_corr(1, corr_type_list[corr_index]);
      Vector < Int > corr = output_record.corr_type;
      ASSERT_TRUE(allEQ(expected_corr, corr));
    }

    TestShape(num_pol, num_chan, output_record, "FLOAT_DATA");

    Matrix < Float > out_data = output_record.float_data;
    Matrix < Bool > out_flag = output_record.flag;
    Bool out_flag_row = output_record.flag_row;
    Vector < Float > out_sigma = output_record.sigma;
    Vector < Float > out_weight = output_record.weight;
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      std::cout << "Verifying i " << i << " polid " << input_record[i].polno
          << std::endl;
      uInt polid = 0;
      if (num_pol == 2) {
        polid = input_record[i].polno;
      }
      Vector < Float > data = input_record[i].data;
      Vector < Bool > flag = input_record[i].flag;
      net_flag_row = net_flag_row || input_record[i].flag_row;
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
      Vector<uInt> const &polid_list, Vector<DataRecord> const &input_record,
      MSDataRecord const &output_record) {
    std::cout << "Full polarization test" << std::endl;
    size_t const expected_num_pol = polid_list.size();
    size_t const num_pol = output_record.num_pol;
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
    Vector < Int > corr = output_record.corr_type;
    ASSERT_TRUE(allEQ(expected_corr, corr));

    TestShape(num_pol, num_chan, output_record, "DATA");

    Matrix < Float > data_real(IPosition(2, num_pol, num_chan), 0.0f);
    Matrix < Float > data_imag(data_real.shape(), 0.0f);
    Matrix < Bool > flag(data_real.shape(), False);
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      uInt polid = polid_list[i];
      ASSERT_LT(polid, num_pol);
      Vector < Float > input_data = input_record[i].data;
      Vector < Bool > input_flag = input_record[i].flag;
      net_flag_row = net_flag_row || input_record[i].flag_row;
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

    Matrix < Complex > out_data = output_record.complex_data;
    EXPECT_TRUE(allEQ(data_real, real(out_data)));
    EXPECT_TRUE(allEQ(data_imag, imag(out_data)));
    Matrix < Bool > out_flag = output_record.flag;
    EXPECT_TRUE(allEQ(flag, out_flag));
    Bool out_flag_row = output_record.flag_row;
    EXPECT_EQ(net_flag_row, out_flag_row);
    Vector < Float > out_sigma = output_record.sigma;
    Vector < Float > out_weight = output_record.weight;
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }

};

struct StokesFullPolarizationTester {
  static void Test(size_t const num_chan, String const &pol_type,
      Vector<uInt> const &polid_list, Vector<DataRecord> const &input_record,
      MSDataRecord const &output_record) {
    std::cout << "Stokes full polarization test" << std::endl;
    CASA_ASSERT_STREQ(String("stokes"), pol_type);

    size_t const expected_num_pol = polid_list.size();
    size_t const num_pol = output_record.num_pol;
    ASSERT_EQ(expected_num_pol, num_pol);
    ASSERT_EQ(input_record.size(), num_pol);
    ASSERT_EQ(4ul, num_pol);

    static Vector<Int> expected_corr(4, Stokes::Undefined);
    expected_corr[0] = Stokes::I;
    expected_corr[1] = Stokes::Q;
    expected_corr[2] = Stokes::U;
    expected_corr[3] = Stokes::V;
    Vector < Int > corr = output_record.corr_type;
    ASSERT_TRUE(allEQ(expected_corr, corr));

    TestShape(num_pol, num_chan, output_record, "FLOAT_DATA");

    Matrix < Float > out_data = output_record.float_data;
    Matrix < Bool > out_flag = output_record.flag;
    Bool out_flag_row = output_record.flag_row;
    Vector < Float > out_sigma = output_record.sigma;
    Vector < Float > out_weight = output_record.weight;
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      uInt polid = polid_list[i];
      ASSERT_LT(polid, num_pol);
      Vector < Float > data = input_record[i].data;
      Vector < Bool > flag = input_record[i].flag;
      Bool flag_row = input_record[i].flag_row;
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
      Vector<DataRecord> &input_record, DataChunk &chunk) {
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
      DataRecord &input_record0 = input_record[i];
      uInt polid = polid_list[i];
      input_record0.polno = polid;
      input_record0.data.assign(data.row(polid));
      input_record0.flag.assign(flag.row(polid));
      input_record0.flag_row = flag_row(polid);
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
  Vector<DataRecord> input_record(polid_list.size());

  Initializer::Initialize(num_chan, polid_list, input_record, chunk);

  ASSERT_EQ(polid_list.size(), chunk.getNumPol());

  MSDataRecord output_record;
  ASSERT_LT(output_record.time, 0.0);

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
  DataRecord record;
  Vector < Float > data1(1);
  Vector < Bool > flag1(1);
  record.polno = 0u;
  record.data.assign(data1);
  record.flag.assign(flag1);
  record.flag_row = False;
  ASSERT_EQ(0u, chunk.getNumPol());
  bool status = chunk.accumulate(record);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, chunk.getNumPol());
  MSDataRecord output_record;
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  EXPECT_EQ(IPosition(2, 1, 1), output_record.float_data.shape());

// clear
  chunk.clear();
  EXPECT_EQ(0u, chunk.getNumPol());

// Accumulate invalid record
  chunk.initialize(1);
  record.data.resize();
  ASSERT_TRUE(record.data.empty());
  status = chunk.accumulate(record);
  ASSERT_FALSE(status);
  EXPECT_EQ(0u, chunk.getNumPol());

// Accumulate different shaped data
  Vector < Float > data2(2);
  Vector < Bool > flag2(2);
  chunk.initialize(1);
  record.polno = 0u;
  record.data.assign(data2);
  record.flag.assign(flag2);
  record.flag_row = False;
  constexpr bool expected_status = false;
  status = chunk.accumulate(record);
  EXPECT_EQ(expected_status, status);

// Shape mismatch between data and flag
  record.data.assign(data1);
  status = chunk.accumulate(record);
  EXPECT_EQ(expected_status, status);

// Test number of polarization
  constexpr size_t num_seq = 4ul;
  constexpr size_t num_accum = 4ul;
  uInt expected_num_pol[num_seq][num_accum] = { { 1, 2, 2, 4 }, { 1, 2, 2, 4 },
      { 0, 0, 1, 4 }, { 1, 1, 2, 4 } };
  uInt polid_order[num_seq][num_accum] = { { 0, 1, 2, 3 }, { 1, 0, 2, 3 }, { 3,
      2, 1, 0 }, { 0, 3, 1, 2 } };
  record.data.assign(data2);
  record.flag.assign(flag2);
  record.flag_row = False;
  for (size_t j = 0; j < num_seq; ++j) {
    chunk.initialize(2);
    EXPECT_EQ(0u, chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.polno = polid_order[j][i];
      status = chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol[j][i], chunk.getNumPol());
    }
  }

// accumulate data to same polarization twice
  data2 = 0.0;
  chunk.initialize(2);
  record.polno = 0u;
  record.data.assign(data2);
  chunk.accumulate(record);
  data2 = 1.0;
  record.data.assign(data2);
  status = chunk.accumulate(record);
  ASSERT_TRUE(status);
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > output_data = output_record.float_data;
  EXPECT_EQ(IPosition(2, 1, 2), output_data.shape());
  EXPECT_TRUE(allEQ(output_data, 1.0f));

// accumulate three polarization component
  chunk.initialize(2);
  data2 = 3.0;
  record.polno = 0u;
  record.data.assign(data2);
  chunk.accumulate(record);
  record.polno = 1u;
  chunk.accumulate(record);
  record.polno = 2u;
  chunk.accumulate(record);
  EXPECT_EQ(2u, chunk.getNumPol());
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data = output_record.float_data;
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
  record.polno = 1u;
  record.data.assign(data2);
  stokes_chunk.accumulate(record);
  EXPECT_EQ(0u, stokes_chunk.getNumPol());
  status = stokes_chunk.get(output_record);
  ASSERT_FALSE(status);
  record.polno = 0u;
  stokes_chunk.accumulate(record);
  EXPECT_EQ(1u, stokes_chunk.getNumPol());
  status = stokes_chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data_stokes = output_record.float_data;
  EXPECT_EQ(IPosition(2, 1, 2), data_stokes.shape());
  EXPECT_TRUE(allEQ(data_stokes, 4.0f));

// Test number of polarization
  uInt expected_num_pol_stokes[num_seq][num_accum] = { { 1, 1, 1, 4 }, { 0, 1,
      1, 4 }, { 0, 0, 0, 4 }, { 1, 1, 1, 4 } };
  uInt polid_order_stokes[num_seq][num_accum] = { { 0, 1, 2, 3 },
      { 1, 0, 2, 3 }, { 3, 2, 1, 0 }, { 0, 3, 1, 2 } };
  record.data.assign(data2);
  record.flag.assign(flag2);
  record.flag_row = False;
  for (size_t j = 0; j < num_seq; ++j) {
    stokes_chunk.initialize(2);
    EXPECT_EQ(0u, stokes_chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.polno = polid_order_stokes[j][i];
      status = stokes_chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol_stokes[j][i], stokes_chunk.getNumPol());
    }
  }

// Linpol single pol 1 and full pols are invalid
  DataChunk linpol_chunk("linpol");

  linpol_chunk.initialize(2);
  data2 = 8.0;
  record.polno = 1u;
  record.data.assign(data2);
  linpol_chunk.accumulate(record);
  EXPECT_EQ(0u, linpol_chunk.getNumPol());
  status = linpol_chunk.get(output_record);
  ASSERT_FALSE(status);
  record.polno = 0u;
  linpol_chunk.accumulate(record);
  record.polno = 2u;
  linpol_chunk.accumulate(record);
  record.polno = 4u;
  linpol_chunk.accumulate(record);
  EXPECT_EQ(2u, linpol_chunk.getNumPol());
  status = linpol_chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data_linpol = output_record.float_data;
  EXPECT_EQ(IPosition(2, 2, 2), data_linpol.shape());
  EXPECT_TRUE(allEQ(data_linpol, 8.0f));

// Test number of polarization
  uInt expected_num_pol_linpol[num_seq][num_accum] = { { 1, 2, 2, 2 }, { 0, 2,
      2, 2 }, { 0, 0, 0, 2 }, { 1, 1, 2, 2 } };
  uInt polid_order_linpol[num_seq][num_accum] = { { 0, 1, 2, 3 },
      { 1, 0, 2, 3 }, { 3, 2, 1, 0 }, { 0, 3, 1, 2 } };
  record.data.assign(data2);
  record.flag.assign(flag2);
  record.flag_row = False;
  for (size_t j = 0; j < num_seq; ++j) {
    linpol_chunk.initialize(2);
    EXPECT_EQ(0u, linpol_chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.polno = polid_order_linpol[j][i];
      status = linpol_chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol_linpol[j][i], linpol_chunk.getNumPol());
    }
  }
}

TEST(DataAccumulatorTest, WhiteBoxTest) {
  auto IsValidOutputRecord = [&](MSDataRecord const &record) {
    size_t const expected_num_pol = record.num_pol;
    Matrix<Bool> const flag = record.flag;
    size_t const num_pol = flag.nrow();
    ASSERT_EQ(expected_num_pol, num_pol);
    String const poltype = record.pol_type;
    if (poltype == "linear" || poltype == "circular") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 2) || (num_pol == 4));
      if (num_pol < 4) {
        ASSERT_TRUE(record.isFloat());
      }
      else {
        ASSERT_FALSE(record.isFloat());
      }
    }
    else if (poltype == "stokes") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 4));
      ASSERT_TRUE(record.isFloat());
    }
    else if (poltype == "linpol") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 2));
      ASSERT_TRUE(record.isFloat());
    }
    else {
      FAIL();
    }
  };

  auto ClearRecord = [](MSDataRecord &record) {
    record.clear();
    ASSERT_LT(record.time, 0.0);
  };

  DataAccumulator a;

  // number of chunks must be zero at initial state
  ASSERT_EQ(0ul, a.getNumberOfChunks());
  MSDataRecord output_record;
  bool status = a.get(0, output_record);
  ASSERT_FALSE(status);

  DataRecord r1;
  r1.clear();
  Time t(2016, 1, 1);
  Double time = t.modifiedJulianDay() * 86400.0;
  r1.time = time;

  // accumulator should not be ready at initial state
  bool is_ready = a.queryForGet(r1);
  ASSERT_FALSE(is_ready);

  // Invalid record cannot be accumulated
  status = a.accumulate(r1);
  ASSERT_FALSE(status);
  DataRecord r2;
  r2.clear();
  time += 1.0;
  r2.time = time;
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
  Double interval = 0.48;
  String intent = "ON_SOURCE";
  String poltype = "linear";
  r1.antenna_id = antenna_id;
  r1.spw_id = spw_id;
  r1.field_id = field_id;
  r1.feed_id = feed_id;
  r1.scan = scan;
  r1.subscan = subscan;
  r1.intent = intent;
  r1.pol_type = poltype;
  //r1.direction.assign(direction);
  r1.direction_slice = direction;
  r1.interval = interval;
  size_t const num_chan = 4;
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
  r1.polno = 0u;
  r1.setDataSize(num_chan);
  r1.data = data.row(0);
  r1.flag = flag.row(0);
  r1.flag_row = flag_row[0];
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  EXPECT_EQ(1u, a.getNumPol(0));

  // Data with different timestamp cannot be accumulated
  Double t2 = r2.time;
  r2 = r1;
  r2.time = t2;
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
  Matrix < Float > output_data = output_record.float_data;
  Matrix < Bool > output_flag = output_record.flag;
  IPosition const expected_shape1(2, 1, num_chan);
  EXPECT_EQ(expected_shape1, output_data.shape());
  EXPECT_EQ(expected_shape1, output_flag.shape());
  EXPECT_TRUE(allEQ(output_data.row(0), data.row(0)));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_EQ(flag_row[0], output_record.flag_row);

  // Accumulate more data with same meta data but different polno
  r1.polno = 1u;
  r1.data = data.row(1);
  r1.flag = flag.row(1);
  r1.flag_row = flag_row[1];
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
  output_data.assign(output_record.float_data);
  output_flag.assign(output_record.flag);
  IPosition const expected_shape2(2, 2, num_chan);
  EXPECT_EQ(expected_shape2, output_data.shape());
  EXPECT_EQ(expected_shape2, output_flag.shape());
  EXPECT_TRUE(allEQ(output_data.row(0), data.row(0)));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_TRUE(allEQ(output_data.row(1), data.row(1)));
  EXPECT_TRUE(allEQ(output_flag.row(1), flag.row(1)));
  EXPECT_EQ(flag_row[0] || flag_row[1], output_record.flag_row);

// Accumulate cross-pol data with same meta data
  r1.polno = 2u;
  r1.data = data.row(2);
  r1.flag = flag.row(2);
  r1.flag_row = flag_row[2];
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  r1.polno = 3u;
  r1.data = data.row(3);
  r1.flag = flag.row(3);
  r1.flag_row = flag_row[3];
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
  Matrix < Complex > output_cdata = output_record.complex_data;
  output_flag.assign(output_record.flag);
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
  EXPECT_EQ(anyTrue(flag_row), output_record.flag_row);

// Accumulate data with another meta data
  String intent2 = "OFF_SOURCE";
  String poltype2 = "circular";
  r1.polno = 0u;
  r1.spw_id = spw_id + 1;
  r1.field_id = field_id + 1;
  r1.feed_id = feed_id + 1;
  r1.intent = intent2;
  r1.pol_type = poltype2;
  Vector < Float > data2(8, -1.0f);
  Vector < Bool > flag2(8, False);
  r1.setDataSize(8);
  r1.data = data2;
  r1.flag = flag2;
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
  Matrix < Complex > output_cdata2 = output_record.complex_data;
  EXPECT_EQ(output_cdata.shape(), output_cdata2.shape());
  EXPECT_TRUE(allEQ(output_cdata, output_cdata2));
  ClearRecord(output_record);
  status = a.get(1, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Float > output_data2 = output_record.float_data;
  EXPECT_EQ(IPosition(2, 1, 8), output_data2.shape());
  EXPECT_TRUE(allEQ(output_data2.row(0), data2));
  Matrix < Bool > output_flag2 = output_record.flag;
  EXPECT_EQ(output_data2.shape(), output_flag2.shape());
  EXPECT_TRUE(allEQ(output_flag2.row(0), flag2));

// Accumulate another
  r1.polno = 1u;
  data2 *= 2.0f;
  flag2 = True;
  r1.data = data2;
  r1.flag = flag2;
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(2u, a.getNumPol(1));
  ClearRecord(output_record);
  status = a.get(1, output_record);
  IsValidOutputRecord(output_record);
  output_data2.assign(output_record.float_data);
  output_flag2.assign(output_record.flag);
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

// This is a test for performance measurement that is disabled by default.
// To activate this test, please follow the instruction below:
//
//   1. put test data "uid___A002_X85c183_X36f.PM04.asap" to the directory unittest/importasap
//
//   2. run singledishMSFiller_GTest with the following option
//
//       --gtest_also_run_disabled_tests --gtest_filter="*Performance*"
//
TEST_F(DISABLED_SingleDishMSFillerTestPerformance, Test) {
  // Create filler
  SingleDishMSFiller<Scantable2MSReader> filler(my_data_name_);

  // Run filler
  ExecuteFiller(filler);
}

int main(int nArgs, char * args[]) {
  ::testing::InitGoogleTest(&nArgs, args);
  std::cout << "SingleDishMSFiller test " << std::endl;
  return RUN_ALL_TESTS();
}

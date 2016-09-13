/*
 * SDDoubleCircleGainCal_GTest.cc
 *
 *  Created on: May 31, 2016
 *      Author: nakazato
 */

#include <gtest/gtest.h>

#include <casacore/casa/BasicSL/Constants.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/casa/Quanta/Quantum.h>

#include <iostream>
#include <fstream>
#include <cmath>

#include "../SDDoubleCircleGainCalImpl.h"

using namespace casacore;
using namespace casa;
using namespace std;

#define TESTLOG cout << "TESTLOG::" << __FUNCTION__ << " "
#define SDD_TEST(name) TEST(SDDoubleCircleGainCalImplTest, name)

namespace {
inline Double convertUnit(Double const value, String const &from,
    String const &to) {
  return Quantity(value, from).getValue(to);
}

inline Double rad2arcsec(Double const value) {
  return convertUnit(value, "rad", "arcsec");
}

inline Double arcsec2rad(Double const value) {
  return convertUnit(value, "arcsec", "rad");
}

void createTestData(ssize_t const num_pol, ssize_t const num_chan,
    Cube<Float> &data, Matrix<Double> &direction, ssize_t const ncycle = 20,
    ssize_t const datapercycle = 20) {
  ssize_t const num_data = ncycle * datapercycle;

  direction.resize(2, num_data);
  data.resize(num_pol, num_chan, num_data);

  // set direction
  Double const scaler = arcsec2rad(10.0);
  for (ssize_t idata = 0; idata < num_data; ++idata) {
    Double angle = C::_2pi / static_cast<Double>(num_data)
        * static_cast<Double>(idata);
    Double px = cos(angle);
    Double py = sin(angle);

    ssize_t const k = idata % datapercycle;
    auto const halfcycle = datapercycle / 2;
    Double const halfcycle_asdouble = static_cast<Double>(datapercycle) / 2.0;
    Double r = 0.0;
    if (k == 0) {
      r = 0.0;
    } else if (k == halfcycle) {
      r = 1.0;
    } else if (k < halfcycle) {
      r = static_cast<Double>(k) / halfcycle_asdouble;
    } else {
      r = 1.0 - static_cast<Double>(k - halfcycle) / halfcycle_asdouble;
    }
    direction(0, idata) = scaler * r * px;
    direction(1, idata) = scaler * r * py;
  }

  ///
  // for testing
//  ofstream ofs("direction.dat", fstream::out);
//  string const delim = " ";
//  for (size_t i = 0; i < num_data; ++i) {
//    ofs << direction(0, i) << delim << direction(1, i) << endl;
//  }
//  ofs.close();
  ///

  // set data
  constexpr Float offset = 1.0;
  for (ssize_t idata = 0; idata < num_data; ++idata) {
    auto k = idata / datapercycle;
    data.xyPlane(idata) = static_cast<Float>(k) + offset;
  }

  ///
  // for testing
//  ofs.open("data.dat", fstream::out);
//  for (size_t i = 0; i < num_data; ++i) {
//    ofs << data(0, 0, i) << endl;
//  }
//  ofs.close();
  ///
}

Double getDefaultSmoothingSize(Double radius) {
  auto const radius_in_arcsec = rad2arcsec(radius);
  return 2 * static_cast<Int>(round(radius_in_arcsec / 1.5)) + 1;
}

inline size_t toUnsigned(ssize_t const v) {
  assert(v >= 0);
  return static_cast<size_t>(v);
}
}

template<class Config>
struct ConfigInterface {
  static void ConfigureData(Cube<Float> &data, Cube<Bool> &flag,
      Matrix<Double> &direction, ssize_t const num_cycle = 20,
      ssize_t const num_data_per_cycle = 20) {
    ssize_t const num_pol = 2;
    ssize_t const num_chan = 2;
    ssize_t const num_data = num_cycle * num_data_per_cycle;
    createTestData(num_pol, num_chan, data, direction, num_cycle,
        num_data_per_cycle);
    Config::ConfigureFlag(data, direction, flag);

    IPosition const data_shape = data.shape();
    IPosition const direction_shape = direction.shape();
    ASSERT_EQ(num_pol, data_shape[0]);
    ASSERT_EQ(num_chan, data_shape[1]);
    ASSERT_EQ(num_data, data_shape[2]);
    ASSERT_EQ(2, direction_shape[0]);
    ASSERT_EQ(num_data, direction_shape[1]);
  }

  /**
   * Configure radius for calibrator
   *
   * It receives radius in arcsec and return the calculated radius in rad.
   */
  static Double ConfigureRadius(SDDoubleCircleGainCalImpl &calibrator,
      Double const radius = 0.21, Bool const auto_radius = false) {
    Double myradius = radius;
    if (auto_radius) {
      Double const diameter = 12.0;
      Double const preliminary_frequency = 3.0e11;
      calibrator.setAntennaDiameter(diameter);
      calibrator.setObservingFrequency(preliminary_frequency);
      Double preliminary_size = calibrator.getPrimaryBeamSize();
      Double frequency = preliminary_frequency * preliminary_size
          / (arcsec2rad(radius) * 2.0);
      calibrator.setObservingFrequency(frequency);
      myradius = calibrator.getPrimaryBeamSize() / 2.0;
      if (radius < 0.0) {
        EXPECT_DOUBLE_EQ(0.0, myradius);
      } else {
        EXPECT_DOUBLE_EQ(radius, rad2arcsec(myradius));
      }
      Double const r = calibrator.getCentralRegion();
      TESTLOG << "central region: " << r << " myradius = " << myradius << endl;
    } else {
      // unit of the radius is assumed to be arcsec
      // calibrator expects the value in rad
      auto const radius_in_rad = arcsec2rad(radius);
      calibrator.setCentralRegion(radius_in_rad);
      myradius = radius_in_rad;
    }

    return myradius;
  }

  static Int ConfigureSmoothing(SDDoubleCircleGainCalImpl &calibrator,
      Bool const do_smooth = false, Int const smooth_size = -1,
      Bool const auto_size = false) {
    Int mysize = smooth_size;
    if (do_smooth) {
      if (auto_size) {
        Double radius = calibrator.getCentralRegion();
        Double myradius = radius;
        if (radius <= 0.0) {
          myradius = calibrator.getPrimaryBeamSize() / 2.0;
        }
        Int const expected_smooth_size = ::getDefaultSmoothingSize(myradius);
        Int const default_smooth_size = calibrator.getDefaultSmoothingSize();
        EXPECT_EQ(expected_smooth_size, default_smooth_size);
        TESTLOG << "auto calculated smoothing size: " << default_smooth_size
            << " (radius " << radius << " myradius " << myradius << ")" << endl;
        calibrator.setSmoothing(-1);
        mysize = default_smooth_size;
      } else {
        calibrator.setSmoothing(smooth_size);
      }
    } else {
      calibrator.unsetSmoothing();
      mysize = -1;
    }
    return mysize;
  }
};

struct StandardConfig: public ConfigInterface<StandardConfig> {
  static void ConfigureFlag(Cube<Float> const &/*data*/,
      Matrix<Double> const &/*direction*/, Cube<Bool> &flag) {
    if (!flag.empty()) {
      flag.resize();
    }
    ASSERT_TRUE(flag.empty());
  }
};

struct AllFlaggedConfig: public ConfigInterface<AllFlaggedConfig> {
  static void ConfigureFlag(Cube<Float> const &data,
      Matrix<Double> const &/*direction*/, Cube<Bool> &flag) {
    if (flag.shape() != data.shape()) {
      flag.resize(data.shape());
    }

    // all data are flagged
    flag = true;

    ASSERT_EQ(data.shape(), flag.shape());
  }
};

struct PartiallyFlaggedConfig: public ConfigInterface<PartiallyFlaggedConfig> {
  static void ConfigureFlag(Cube<Float> const &data,
      Matrix<Double> const &direction, Cube<Bool> &flag) {
    if (flag.shape() != data.shape()) {
      flag.resize(data.shape());
    }

    flag = false;

    ASSERT_GE(data.shape()[0], 0);
    ASSERT_GE(data.shape()[1], 0);
    ASSERT_GE(data.shape()[2], 0);
    size_t const num_data = data.shape()[2];
    for (size_t i = 0; i < num_data; ++i) {
      if (direction(0, i) == 0.0 && direction(1, i) == 0.0) {
        TESTLOG << "flag " << i << endl;
        flag.xyPlane(i) = true;
      }
    }

    ASSERT_EQ(data.shape(), flag.shape());
  }
};

struct ExceptionExecutor {
  static void Execute(SDDoubleCircleGainCalImpl &calibrator,
      Cube<Float> const &data, Cube<Bool> const &/*flag*/,
      Matrix<Double> const &direction, Double const /* radius */,
      Int const /* smooth_size */) {
    size_t const num_data = data.shape()[2];
    Vector<Double> time(num_data);
    indgen(time, 0., 1.0);

    Vector<Double> gain_time;
    Cube<Float> gain;
    EXPECT_THROW(calibrator.calibrate(data, time, direction, gain_time, gain),
        AipsError);
  }
};

struct StandardExecutor {
  static void Execute(SDDoubleCircleGainCalImpl &calibrator,
      Cube<Float> const &data, Cube<Bool> const &flag,
      Matrix<Double> const &direction, Double const radius,
      Int const smooth_size) {
    size_t const num_data = data.shape()[2];
    Vector<Double> time(num_data);
    indgen(time, 0., 1.0);

    Cube<Bool> flag_local;

    if (flag.empty()) {
      flag_local.resize(data.shape());
      flag_local = false; // all data are valid
    } else {
      flag_local.reference(flag);
    }

    Vector<Double> gain_time_with_flag;
    Cube<Float> gain_with_flag;
    Cube<Bool> gain_flag;
    calibrator.calibrate(data, flag_local, time, direction, gain_time_with_flag,
        gain_with_flag, gain_flag);

    Vector<size_t> data_index;
    GetDataIndex(radius, direction, data_index);

    //TESTLOG << "INPUT FLAG: " << flag_local << endl;
    //TESTLOG << "OUTPUT FLAG: " << gain_flag << endl;
    VerifyGainWithFlag(smooth_size, data_index, time, data, flag_local,
        gain_time_with_flag, gain_with_flag, gain_flag);

    if (flag.empty()) {
      // check consistency between calibrations with and without flag
      Vector<Double> gain_time;
      Cube<Float> gain;
      calibrator.calibrate(data, time, direction, gain_time, gain);

      Vector<size_t> data_index;
      GetDataIndex(radius, direction, data_index);

      VerifyGain(smooth_size, data_index, time, data, gain_time, gain);

      // result should be identical with no-flag version
//      TESTLOG << "gain " << gain << endl;
//      TESTLOG << "gain (with flag) " << gain_with_flag << endl;
      EXPECT_TRUE(allEQ(gain_time, gain_time_with_flag));
      EXPECT_TRUE(allEQ(gain, gain_with_flag));
      EXPECT_TRUE(allEQ(gain_flag, false));
    }
  }

private:
  static void GetDataIndex(Double const radius, Matrix<Double> const &direction,
      Vector<size_t> &data_index) {
    size_t const num_data = direction.shape()[1];
    size_t count = 0;
    data_index.resize(num_data);
    for (size_t i = 0; i < num_data; ++i) {
      auto const x = direction(0, i);
      auto const y = direction(1, i);
      auto const r = x * x + y * y;
      if (r <= radius * radius) {
        data_index[count] = i;
        count++;
      }
    }
    data_index.resize(count, true);
  }

  static void VerifyGain(Int const smooth_size,
      Vector<size_t> const &data_index, Vector<Double> const &time,
      Cube<Float> const &data, Vector<Double> &gain_time,
      Cube<Float> const &gain) {
    TESTLOG << endl;
    IPosition const data_shape = data.shape();
    ssize_t const num_pol_expected = data_shape[0];
    ssize_t const num_chan_expected = data_shape[1];
    ssize_t const num_data_expected = data_index.nelements();

    if (num_data_expected < 100) {
      // no calibration is executed, gain should be empty array
      ASSERT_TRUE(gain.empty());
    } else {
      // calibration is properly done
      IPosition const gain_shape = gain.shape();
      ASSERT_EQ(num_pol_expected, gain_shape[0]);
      ASSERT_EQ(num_chan_expected, gain_shape[1]);
      ASSERT_EQ(num_data_expected, gain_shape[2]);
      ASSERT_EQ(static_cast<size_t>(num_data_expected), gain_time.nelements());

      // verify timestamp
      for (ssize_t i = 0; i < num_data_expected; ++i) {
        auto const time_expected = time[data_index[i]];
        auto const time_actual = gain_time[i];
        EXPECT_EQ(time_expected, time_actual);
      }

      // do smoothing if necessary
      Cube<Float> smoothed_data(gain.shape());
      if (smooth_size > 1 && smooth_size < num_data_expected) {
        // do smoothing
        TESTLOG << "do smoothing" << endl;
        Cube<Float> unsmoothed_data(gain.shape());
        for (ssize_t i = 0; i < num_data_expected; ++i) {
          unsmoothed_data.xyPlane(i) = data.xyPlane(data_index[i]);
        }
//        TESTLOG << "unsmoothed_data" << unsmoothed_data << endl;
        SmoothData(unsmoothed_data, smooth_size, smoothed_data);
//        TESTLOG << "smoothed_data" << smoothed_data << endl;
      } else {
        // no smoothing
        for (ssize_t i = 0; i < num_data_expected; ++i) {
          smoothed_data.xyPlane(i) = data.xyPlane(data_index[i]);
        }
      }

      for (ssize_t ip = 0; ip < num_pol_expected; ++ip) {
        for (ssize_t ic = 0; ic < num_chan_expected; ++ic) {
          Double mean_data = 0.0;
          for (ssize_t i = 0; i < num_data_expected; ++i) {
            mean_data += smoothed_data(ip, ic, i);
          }
          mean_data /= num_data_expected;
          for (ssize_t ig = 0; ig < num_data_expected; ++ig) {
            auto const expected_gain = mean_data / smoothed_data(ip, ic, ig);
            auto const expected = 1.0 / sqrt(expected_gain);
            EXPECT_FLOAT_EQ(expected, gain(ip, ic, ig));
          }
        }
      }
    }
  }

  static void SmoothData(Cube<Float> const &data, Int const smooth_size,
      Cube<Float> &smoothed_data) {
    size_t const num_data = data.shape()[2];
    size_t const left_edge = (smooth_size + 1) / 2 - 1;
    size_t const right_edge = smooth_size / 2 + 1;
    TESTLOG << "num_data " << num_data << ", left_edge " << left_edge
        << ", right_edge " << right_edge << endl;
    TESTLOG << "smoothed_data.shape = " << smoothed_data.shape() << endl;
    for (size_t i = 0; i < left_edge; ++i) {
      size_t l = 0;
      size_t r = i + right_edge;
      smoothed_data.xyPlane(i) = AverageData(l, r, data);
    }
    for (size_t i = left_edge; i < num_data - right_edge; ++i) {
      size_t l = i - left_edge;
      size_t r = i + right_edge;
      smoothed_data.xyPlane(i) = AverageData(l, r, data);
    }
    for (size_t i = num_data - right_edge; i < num_data; ++i) {
      size_t l = i - left_edge;
      size_t r = num_data;
      smoothed_data.xyPlane(i) = AverageData(l, r, data);
    }
  }

  static void SmoothData(Cube<Float> const &data, Cube<Bool> const &flag,
      Int const smooth_size, Cube<Float> &smoothed_data) {
    size_t const num_data = data.shape()[2];
    size_t const left_edge = (smooth_size + 1) / 2 - 1;
    size_t const right_edge = smooth_size / 2 + 1;
    TESTLOG << "num_data " << num_data << ", left_edge " << left_edge
        << ", right_edge " << right_edge << endl;
    TESTLOG << "smoothed_data.shape = " << smoothed_data.shape() << endl;
    for (size_t i = 0; i < left_edge; ++i) {
      size_t l = 0;
      size_t r = i + right_edge;
      smoothed_data.xyPlane(i) = AverageData(i, l, r, data, flag);
    }
    for (size_t i = left_edge; i < num_data - right_edge; ++i) {
      size_t l = i - left_edge;
      size_t r = i + right_edge;
      smoothed_data.xyPlane(i) = AverageData(i, l, r, data, flag);
    }
    for (size_t i = num_data - right_edge; i < num_data; ++i) {
      size_t l = i - left_edge;
      size_t r = num_data;
      smoothed_data.xyPlane(i) = AverageData(i, l, r, data, flag);
    }
  }

  static Matrix<Float> AverageData(size_t const start, size_t const end,
      Cube<Float> const &data) {
    Matrix<Float> mean(data.shape().getFirst(2), 0.0f);
//    TESTLOG << "mean.shape=" << mean.shape() << endl;
//    TESTLOG << "data.shape " << data.shape() << ", start " << start << ", end "
//        << end << endl;
    for (size_t i = start; i < end; ++i) {
      mean += data.xyPlane(i);
    }
    mean /= static_cast<Float>(end - start);
    return mean;
  }

  static Matrix<Float> AverageData(size_t const middle, size_t const start,
      size_t const end, Cube<Float> const &data, Cube<Bool> const &flag) {
    Matrix<Float> mean(data.shape().getFirst(2), 0.0f);
    Matrix<size_t> count(mean.shape(), 0ul);
    auto const num_pol = mean.shape()[0];
    auto const num_chan = mean.shape()[1];
//    TESTLOG << "mean.shape=" << mean.shape() << endl;
//    TESTLOG << "data.shape " << data.shape() << ", start " << start << ", end "
//        << end << endl;
    for (ssize_t ip = 0; ip < num_pol; ++ip) {
      for (ssize_t ic = 0; ic < num_chan; ++ic) {
        for (size_t i = start; i < end; ++i) {
          if (flag(ip, ic, i) == false) {
            mean(ip, ic) += data(ip, ic, i);
            count(ip, ic) += 1;
          }
        }
        if (count(ip, ic) == 0) {
          mean(ip, ic) = data(ip, ic, middle);
        } else {
          mean(ip, ic) /= static_cast<Float>(count(ip, ic));
        }
      }
    }
    return mean;
  }

  static void VerifyGainWithFlag(Int const smooth_size,
      Vector<size_t> const &data_index, Vector<Double> const &time,
      Cube<Float> const &data, Cube<Bool> const &flag,
      Vector<Double> &gain_time, Cube<Float> const &gain,
      Cube<Bool> const &gain_flag) {
    TESTLOG << endl;
    IPosition const data_shape = data.shape();
    ssize_t const num_pol_expected = data_shape[0];
    ssize_t const num_chan_expected = data_shape[1];
    size_t const num_data_expected = data_index.nelements();
    ASSERT_LE(num_data_expected, static_cast<size_t>(SSIZE_MAX));

    //TESTLOG << "data_index=" << data_index << endl;
    if (num_data_expected < 100) {
      // no calibration is executed, gain should be empty array
      ASSERT_TRUE(gain.empty());
      ASSERT_TRUE(gain_flag.empty());
      ASSERT_TRUE(gain_time.empty());
    } else {
      // calibration is properly done
      IPosition const gain_shape = gain.shape();
      ASSERT_EQ(num_pol_expected, gain_shape[0]);
      ASSERT_EQ(num_chan_expected, gain_shape[1]);
      ASSERT_EQ(static_cast<ssize_t>(num_data_expected), gain_shape[2]);
      ASSERT_EQ(num_data_expected, gain_time.nelements());

      // verify timestamp
      for (size_t i = 0; i < num_data_expected; ++i) {
        auto const time_expected = time[data_index[i]];
        auto const time_actual = gain_time[i];
        EXPECT_EQ(time_expected, time_actual);
      }

      // do smoothing if necessary
      Cube<Float> smoothed_data(gain.shape());
      Cube<Bool> smoothed_flag(gain.shape());
      if (smooth_size > 1
          && static_cast<size_t>(smooth_size) < num_data_expected) {
        // do smoothing
        TESTLOG << "do smoothing" << endl;
        Cube<Float> unsmoothed_data(gain.shape());
        Cube<Bool> unsmoothed_flag(gain.shape());
        for (size_t i = 0; i < num_data_expected; ++i) {
          unsmoothed_data.xyPlane(i) = data.xyPlane(data_index[i]);
          unsmoothed_flag.xyPlane(i) = flag.xyPlane(data_index[i]);
        }
//        TESTLOG << "unsmoothed_data" << unsmoothed_data << endl;
        SmoothData(unsmoothed_data, unsmoothed_flag, smooth_size,
            smoothed_data);
        smoothed_flag = unsmoothed_flag;
//        TESTLOG << "smoothed_data" << smoothed_data << endl;
      } else {
        // no smoothing
        for (size_t i = 0; i < num_data_expected; ++i) {
          smoothed_data.xyPlane(i) = data.xyPlane(data_index[i]);
          smoothed_flag.xyPlane(i) = flag.xyPlane(data_index[i]);
        }
      }

      //TESTLOG << "smoothed_flag=" << smoothed_flag << endl;

      for (ssize_t ip = 0; ip < num_pol_expected; ++ip) {
        for (ssize_t ic = 0; ic < num_chan_expected; ++ic) {
          Double mean_data = 0.0;
          size_t count = 0;
          for (size_t ig = 0; ig < num_data_expected; ++ig) {
            if (gain_flag(ip, ic, ig) == false) {
              mean_data += smoothed_data(ip, ic, ig);
              count++;
            }
          }
          if (count > 0) {
            mean_data /= static_cast<Double>(count);
          } else {
            mean_data = 0.0;
          }
          for (size_t ig = 0; ig < num_data_expected; ++ig) {
            auto const expected_gain = mean_data / smoothed_data(ip, ic, ig);
            Float expected = 0.0f;
            if (mean_data != 0.0 && smoothed_data(ip, ic, ig) != 0.0) {
              expected = 1.0 / sqrt(expected_gain);
            }
            EXPECT_FLOAT_EQ(expected, gain(ip, ic, ig)) << ip << "," << ic
                << "," << ig;
            auto const expected_flag = smoothed_flag(ip, ic, ig);
            EXPECT_EQ(expected_flag, gain_flag(ip, ic, ig)) << ip << "," << ic
                << "," << ig;
          }
        }
      }
    }
  }
};

/**
 * RunTest
 *
 * Template function fur test execution.
 *
 * Unit of the radius must be arcsec.
 */
template<class Configurator, class Executor>
void RunTest(ssize_t const num_cycle, ssize_t const num_data_per_cycle,
    Double const radius, Bool const auto_radius, Bool const do_smooth,
    Int const smooth_size, Bool const auto_smooth_size) {
  TESTLOG << "=== START TEST === " << endl;
  TESTLOG << "Input parameter summary: " << "num_cycle = " << num_cycle
      << " num_data_per_cycle = " << num_data_per_cycle << " radius = "
      << radius << " auto_radius = " << ((auto_radius) ? "ON" : "OFF")
      << " smoothing = " << ((do_smooth) ? "ON" : "OFF") << " smoothing size = "
      << smooth_size << " auto smoothing_size = "
      << ((auto_smooth_size) ? "ON" : "OFF") << endl;
  SDDoubleCircleGainCalImpl calibrator;

  Cube<Float> data;
  Cube<Bool> flag;
  Matrix<Double> direction;
  Configurator::ConfigureData(data, flag, direction, num_cycle,
      num_data_per_cycle);

  auto const myradius = Configurator::ConfigureRadius(calibrator, radius,
      auto_radius);

  Int const mysmooth_size = Configurator::ConfigureSmoothing(calibrator,
      do_smooth, smooth_size, auto_smooth_size);
  //Bool do_smooth = (mysmooth_size >= 0);

  // calibrate without smoothing
  Executor::Execute(calibrator, data, flag, direction, myradius, mysmooth_size);

  TESTLOG << "=== END TEST === " << endl;
}

//TEST(SDDoubleCircleGainCalImplTest, BasicTest) {
SDD_TEST(BasicAPITest) {
  SDDoubleCircleGainCalImpl calibrator;

  // setter and getter tests
  Double const radius = 10.0;
  calibrator.setCentralRegion(radius);
  ASSERT_EQ(radius, calibrator.getCentralRegion());

  Double const frequency = 1.0e11;
  calibrator.setObservingFrequency(frequency);
  ASSERT_EQ(frequency, calibrator.getObservingFrequency());

  Double const diameter = 12.0;
  calibrator.setAntennaDiameter(diameter);
  ASSERT_EQ(diameter, calibrator.getAntennaDiameter());

  Int const smoothing_size = 3;
  calibrator.setSmoothing(smoothing_size);
  ASSERT_TRUE(calibrator.isSmoothingActive());
  ASSERT_EQ(smoothing_size, calibrator.getSmoothingSize());

  calibrator.unsetSmoothing();
  ASSERT_FALSE(calibrator.isSmoothingActive());
  ASSERT_EQ(-1, calibrator.getSmoothingSize());

  // primary beam size: ~19.4" at 300GHz for ALMA (12m diameter)
  Double const myfrequency = 3.0e11;
  Double const mydiameter = 12.0;
  Double const mywavelength = C::c / myfrequency;
  Double const rad2arcsec = 180.0 / C::pi * 3600.0;
  constexpr Double kFactorALMA = 1.13;
  calibrator.setObservingFrequency(myfrequency); // 300GHz
  calibrator.setAntennaDiameter(mydiameter); // 12m
  Double const beam_size = calibrator.getPrimaryBeamSize();
  Double const expected_size = kFactorALMA * mywavelength / mydiameter;
  TESTLOG << "beam_size = " << beam_size << " arcsec at frequency "
      << static_cast<uInt>(myfrequency / 1e9) << " GHz for ALMA "
      << static_cast<uInt>(mydiameter) << "m antenna (expected "
      << expected_size << " rad " << expected_size * rad2arcsec << " arcsec)"
      << endl;
  ASSERT_DOUBLE_EQ(expected_size, beam_size);

  // default smoothing size
  Double const radius_forsmooth = 20.0;
  auto const radius_forsmooth_in_rad = radius_forsmooth / rad2arcsec;
  calibrator.setCentralRegion(radius_forsmooth_in_rad);
  Int const expected_smooth_size = ::getDefaultSmoothingSize(
      radius_forsmooth_in_rad);
  Int default_smooth_size = calibrator.getDefaultSmoothingSize();
  TESTLOG << "central region " << radius_forsmooth
      << " arcsec, default smoothing size = " << default_smooth_size << endl;
  ASSERT_EQ(expected_smooth_size, default_smooth_size);
}

//TEST(SDDoubleCircleGainCalImplTest, FailedTest) {
SDD_TEST(FailedTest) {
  // too small central region, no calibration is done
  RunTest<StandardConfig, StandardExecutor>(20, 20, 0.01, false, false, -1,
      false);

  // invalid user-supplied radius
  RunTest<StandardConfig, ExceptionExecutor>(20, 20, -1.0, false, false, -1,
      false);

  // invalid auto calculated radius
  RunTest<StandardConfig, ExceptionExecutor>(20, 20, -1.0, true, false, -1,
      false);
}

//TEST(SDDoubleCircleGainCalImplTest, CalibrationTest) {
SDD_TEST(CalibrationNoSmoothingUserSuppliedRadius2) {
  // no smoothing, user-supplied radius
  TESTLOG << "no smoothing, user-supplied radius (2.1)" << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, false, false, -1,
      false);
}

SDD_TEST(CalibrationNoSmoothingUserSuppliedRadius3) {
  TESTLOG << "no smoothing, user-supplied radius (3.1)" << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 3.1, false, false, -1,
      false);
}

SDD_TEST(CalibrationNoSmoothingAutoRadius) {
  // no smoothing, auto radius based on primary beam size
  TESTLOG << "no smoothing, auto radius based on primary beam size" << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, true, false, -1,
      false);
}

SDD_TEST(CalibrationDoSmoothing2UserSuppliedRadius2) {
  // do smoothing with size 2, user-supplied radius
  TESTLOG << "do smoothing with size 2, user-supplied radius" << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, false, true, 2, false);
}

SDD_TEST(CalibrationDoSmoothing1UserSuppliedRadius2) {
  // do smoothing with size 1 (effectively no smoothing), user-supplied radius
  TESTLOG
      << "do smoothing with size 1 (effectively no smoothing), user-supplied radius"
      << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, False, True, 1, False);
}

SDD_TEST(CalibrationDoSmoothingLargeUserSuppliedRadius2) {
  // do smoothing with size 100000 (effectively no smoothing), user-supplied radius
  TESTLOG
      << "do smoothing with size 100000 (effectively no smoothing), user-supplied radius"
      << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, False, True, 100000,
      False);

}

SDD_TEST(CalibrationAutoSmoothingUserSuppliedRadius3) {
  // do smoothing with auto calculated size (which will be 5), user-supplied radius
  TESTLOG
      << "do smoothing with auto calculated size (which will be 5), user-supplied radius"
      << endl;
}

SDD_TEST(CalibrationAutoSmoothingAutoRadius) {
  // do smoothing with auto calculated size (which will be 5), auto radius based on primary beam size
  TESTLOG
      << "do smoothing with auto calculated size (which will be 5), auto radius based on primary beam size"
      << endl;
  RunTest<StandardConfig, StandardExecutor>(20, 20, 3.1, True, True, -1, True);
}

SDD_TEST(CalibrationNoSmoothingUserSuppliedRadius2AllFlagged) {
  // no smoothing, user-supplied radius, all data are flagged
  TESTLOG << "no smoothing, user-supplied radius, all data are flagged" << endl;
  RunTest<AllFlaggedConfig, StandardExecutor>(20, 20, 2.1, False, False, -1,
      False);
}

SDD_TEST(CalibrationDoSmoothing2UserSuppliedRadius2AllFlagged) {
  // do smoothing with size 2, user-supplied radius, all data are flagged
  TESTLOG
      << "do smoothing with size 2, user-supplied radius, all data are flagged"
      << endl;
  RunTest<AllFlaggedConfig, StandardExecutor>(20, 20, 2.1, False, True, 2,
      False);
}

SDD_TEST(CalibrationNoSmoothingUserSuppliedRadius2PartiallyFlagged) {
  // no smoothing, user-supplied radius, data partially flagged
  TESTLOG << "no smoothing, user-supplied radius, data partially flagged"
      << endl;
  RunTest<PartiallyFlaggedConfig, StandardExecutor>(20, 20, 2.1, False, False,
      -1, False);
}

SDD_TEST(CalibrationDoSmoothing2UserSuppliedRadius2PartiallyFlagged) {
  // do smoothing with size 2, user-supplied radius, data partially flagged
  TESTLOG
      << "do smoothing with size 2, user-supplied radius, data partially flagged"
      << endl;
  RunTest<PartiallyFlaggedConfig, StandardExecutor>(20, 20, 2.1, False, True, 2,
      False);
}

int main(int nArgs, char * args[]) {
  ::testing::InitGoogleTest(&nArgs, args);
  TESTLOG << "SDDoubleCircleGainCal test " << endl;
  return RUN_ALL_TESTS();
}

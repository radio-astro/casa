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

#include <iostream>
#include <fstream>
#include <cmath>

#include "../SDDoubleCircleGainCalImpl.h"

using namespace casa;
using namespace std;

#define TESTLOG cout << "TESTLOG::" << __FUNCTION__ << " "

namespace {
void createTestData(size_t const num_pol, size_t const num_chan,
    Cube<Float> &data, Matrix<Double> &direction, size_t const ncycle = 20,
    size_t const datapercycle = 20) {
  size_t const num_data = ncycle * datapercycle;

  direction.resize(2, num_data);
  data.resize(num_pol, num_chan, num_data);

  // set direction
  Double const scaler = 10.0;
  for (size_t idata = 0; idata < num_data; ++idata) {
    Double angle = C::_2pi / static_cast<Double>(num_data)
        * static_cast<Double>(idata);
    Double px = cos(angle);
    Double py = sin(angle);

    size_t const k = idata % datapercycle;
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
  for (size_t idata = 0; idata < num_data; ++idata) {
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
  return 2 * static_cast<Int>(round(radius / 1.5)) + 1;
}
}

struct StandardConfig {
  static void ConfigureData(Cube<Float> &data, Matrix<Double> &direction,
      size_t const num_cycle = 20, size_t const num_data_per_cycle = 20) {
    size_t const num_pol = 2;
    size_t const num_chan = 2;
    size_t const num_data = num_cycle * num_data_per_cycle;
    createTestData(num_pol, num_chan, data, direction, num_cycle,
        num_data_per_cycle);

    IPosition const data_shape = data.shape();
    IPosition const direction_shape = direction.shape();
    ASSERT_EQ(num_pol, data_shape[0]);
    ASSERT_EQ(num_chan, data_shape[1]);
    ASSERT_EQ(num_data, data_shape[2]);
    ASSERT_EQ(2, direction_shape[0]);
    ASSERT_EQ(num_data, direction_shape[1]);
  }

  static Double ConfigureRadius(SDDoubleCircleGainCalImpl &calibrator,
      Double const radius = 0.21, Bool const auto_radius = False) {
    Double myradius = radius;
    if (auto_radius) {
      Double const diameter = 12.0;
      Double const preliminary_frequency = 3.0e11;
      calibrator.setAntennaDiameter(diameter);
      calibrator.setObservingFrequency(preliminary_frequency);
      Double preliminary_size = calibrator.getPrimaryBeamSize();
      Double frequency = preliminary_frequency * preliminary_size
          / (radius * 2.0);
      calibrator.setObservingFrequency(frequency);
      myradius = calibrator.getPrimaryBeamSize() / 2.0;
      if (radius < 0.0) {
        EXPECT_DOUBLE_EQ(0.0, myradius);
      } else {
        EXPECT_DOUBLE_EQ(radius, myradius);
      }
      Double const r = calibrator.getCentralRegion();
      TESTLOG << "central region: " << r << " myradius = " << myradius << endl;
    } else {
      calibrator.setCentralRegion(radius);
    }

    return myradius;
  }

  static Int ConfigureSmoothing(SDDoubleCircleGainCalImpl &calibrator,
      Bool const do_smooth = False, Int const smooth_size = -1,
      Bool const auto_size = False) {
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

struct ExceptionExecutor {
  static void Execute(SDDoubleCircleGainCalImpl &calibrator,
      Cube<Float> const &data, Matrix<Double> const &direction,
      Double const radius, Int const smooth_size) {
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
      Cube<Float> const &data, Matrix<Double> const &direction,
      Double const radius, Int const smooth_size) {
    size_t const num_data = data.shape()[2];
    Vector<Double> time(num_data);
    indgen(time, 0., 1.0);

    Vector<Double> gain_time;
    Cube<Float> gain;
    calibrator.calibrate(data, time, direction, gain_time, gain);

    Vector<size_t> data_index;
    GetDataIndex(radius, direction, data_index);
    VerifyGain(smooth_size, data_index, time, data, gain_time, gain);
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
    data_index.resize(count, True);
  }

  static void VerifyGain(Int const smooth_size,
      Vector<size_t> const &data_index, Vector<Double> const &time,
      Cube<Float> const &data, Vector<Double> &gain_time,
      Cube<Float> const &gain) {
    TESTLOG << endl;
    IPosition const data_shape = data.shape();
    size_t const num_pol_expected = data_shape[0];
    size_t const num_chan_expected = data_shape[1];
    size_t const num_data_expected = data_index.nelements();

    if (num_data_expected < 100) {
      // no calibration is executed, gain should be empty array
      ASSERT_TRUE(gain.empty());
    } else {
      // calibration is properly done
      IPosition const gain_shape = gain.shape();
      ASSERT_EQ(num_pol_expected, gain_shape[0]);
      ASSERT_EQ(num_chan_expected, gain_shape[1]);
      ASSERT_EQ(num_data_expected, gain_shape[2]);
      ASSERT_EQ(num_data_expected, gain_time.nelements());

      // verify timestamp
      for (size_t i = 0; i < num_data_expected; ++i) {
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
        for (size_t i = 0; i < num_data_expected; ++i) {
          unsmoothed_data.xyPlane(i) = data.xyPlane(data_index[i]);
        }
//        TESTLOG << "unsmoothed_data" << unsmoothed_data << endl;
        SmoothData(unsmoothed_data, smooth_size, smoothed_data);
//        TESTLOG << "smoothed_data" << smoothed_data << endl;
      } else {
        // no smoothing
        for (size_t i = 0; i < num_data_expected; ++i) {
          smoothed_data.xyPlane(i) = data.xyPlane(data_index[i]);
        }
      }

      for (size_t ip = 0; ip < num_pol_expected; ++ip) {
        for (size_t ic = 0; ic < num_chan_expected; ++ic) {
          Double mean_data = 0.0;
          for (size_t i = 0; i < num_data_expected; ++i) {
            mean_data += smoothed_data(ip, ic, i);
          }
          mean_data /= num_data_expected;
          for (size_t ig = 0; ig < num_data_expected; ++ig) {
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
};

template<class Configurator, class Executor>
void RunTest(size_t const num_cycle, size_t const num_data_per_cycle,
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
  Matrix<Double> direction;
  Configurator::ConfigureData(data, direction, num_cycle, num_data_per_cycle);

  Double const myradius = Configurator::ConfigureRadius(calibrator, radius,
      auto_radius);

  Int const mysmooth_size = Configurator::ConfigureSmoothing(calibrator,
      do_smooth, smooth_size, auto_smooth_size);
  //Bool do_smooth = (mysmooth_size >= 0);

  // calibrate without smoothing
  Executor::Execute(calibrator, data, direction, myradius, mysmooth_size);

  TESTLOG << "=== END TEST === " << endl;
}

TEST(SDDoubleCircleGainCalImplTest, BasicTest) {
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
  Double const expected_size = kFactorALMA * rad2arcsec * mywavelength
      / mydiameter;
  TESTLOG << "beam_size = " << beam_size << " arcsec at frequency "
      << static_cast<uInt>(myfrequency / 1e9) << " GHz for ALMA "
      << static_cast<uInt>(mydiameter) << "m antenna (expected "
      << expected_size << ")" << endl;
  ASSERT_DOUBLE_EQ(expected_size, beam_size);

  // default smoothing size
  Double radius_forsmooth = 20.0;
  calibrator.setCentralRegion(radius_forsmooth);
  Int const expected_smooth_size = ::getDefaultSmoothingSize(radius_forsmooth);
  Int default_smooth_size = calibrator.getDefaultSmoothingSize();
  TESTLOG << "central region " << radius_forsmooth
      << " arcsec, default smoothing size = " << default_smooth_size << endl;
  ASSERT_EQ(expected_smooth_size, default_smooth_size);
}

TEST(SDDoubleCircleGainCalImplTest, FailedTest) {
  // too small central region, no calibration is done
  RunTest<StandardConfig, StandardExecutor>(20, 20, 0.01, False, False, -1,
      False);

  // invalid user-supplied radius
  RunTest<StandardConfig, ExceptionExecutor>(20, 20, -1.0, False, False, -1,
      False);

  // invalid auto calculated radius
  RunTest<StandardConfig, ExceptionExecutor>(20, 20, -1.0, True, False, -1,
      False);
}

TEST(SDDoubleCircleGainCalImplTest, CalibrationTest) {
  // no smoothing, user-supplied radius
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, False, False, -1,
      False);
  RunTest<StandardConfig, StandardExecutor>(20, 20, 3.1, False, False, -1,
      False);

  // no smoothing, auto radius based on primary beam size
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, True, False, -1,
      False);

  // do smoothing with size 2, user-supplied radius
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, False, True, 2, False);

  // do smoothing with size 1 (effectively no smoothing), user-supplied radius
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, False, True, 1, False);

  // do smoothing with size 100000 (effectively no smoothing), user-supplied radius
  RunTest<StandardConfig, StandardExecutor>(20, 20, 2.1, False, True, 100000,
      False);

  // do smoothing with auto calculated size (which will be 5), user-supplied radius
  RunTest<StandardConfig, StandardExecutor>(20, 20, 3.1, False, True, -1, True);

  // do smoothing with auto calculated size (which will be 5), auto radius based on primary beam size
  RunTest<StandardConfig, StandardExecutor>(20, 20, 3.1, True, True, -1, True);
}

int main(int nArgs, char * args[]) {
  ::testing::InitGoogleTest(&nArgs, args);
  TESTLOG << "SDDoubleCircleGainCal test " << endl;
  return RUN_ALL_TESTS();
}

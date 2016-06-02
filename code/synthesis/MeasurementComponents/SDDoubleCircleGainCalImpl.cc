/*
 * SDDoubleCircleGainCal.cpp
 *
 *  Created on: May 31, 2016
 *      Author: nakazato
 */

//#include "SDDoubleCircleGainCal.h"
#include "SDDoubleCircleGainCalImpl.h"

#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/scimath/Functionals/Interpolate1D.h>
#include <casacore/scimath/Functionals/ScalarSampledFunctional.h>
#include <casacore/casa/BasicSL/Constants.h>

#include <cassert>
#include <cmath>

using namespace casacore;

#define LOG logger_ << LogOrigin("SDDoubleCircleGainCal", __FUNCTION__, WHERE)

namespace { // anonymous namespace START
// primary beam size based on observing frequency and antenna diameter
inline Double getPrimaryBeamSize(Double const observing_frequency,
    Double const antenna_diameter) {
  Double beam_size = -1.0;
  constexpr Double kFactorALMA = 1.13; // measured factor for ALMA
  Double const speed_of_light = C::c;
  Double const rad2arcsec = 180.0 / C::pi * 3600.0;

  if (observing_frequency <= 0.0 || antenna_diameter <= 0.0) {
    beam_size = 0.0;
  } else {
    beam_size = kFactorALMA * rad2arcsec * speed_of_light
        / (antenna_diameter * observing_frequency);
  }
  return beam_size;
}

// default smoothing size based on the radius of central region
inline Int getDefaultSmoothingSize(Double const radius) {
  return 2 * static_cast<Int>(round(radius / 1.5)) + 1;
}

// average
inline Double average(size_t const index_from, size_t const index_to,
    Vector<Double> const &data) {
  Double sum = 0.0;
  //cout << "number of data to be averaged " << index_to - index_from << endl;
  for (size_t i = index_from; i < index_to; ++i) {
    sum += data[i];
  }
  return sum / static_cast<Double>(index_to - index_from);
}

// smoothing
inline void smooth(Int const smooth_size, Vector<Double> const &data,
    Vector<Double> &smoothed_data) {
  // TODO replace with sakura function
  assert(data.nelements() == smoothed_data.nelements());
  size_t num_data = data.nelements();
  if (smooth_size < 2 || static_cast<size_t>(smooth_size) >= num_data) {
    //cout << "no smoothing" << endl;
    smoothed_data = data;
  } else {
    size_t left_edge = (smooth_size + 1) / 2 - 1;
    size_t right_edge = smooth_size / 2 + 1;
    for (size_t i = 0; i < left_edge; ++i) {
      size_t l = 0;
      size_t r = i + right_edge;
      //cout << "i = " << i << ": l, r = " << l << "," << r << endl;
      smoothed_data[i] = average(l, r, data);
    }
    for (size_t i = left_edge; i < num_data - right_edge; ++i) {
      size_t l = i - left_edge;
      size_t r = i + right_edge;
      //cout << "i = " << i << ": l, r = " << l << "," << r << endl;
      smoothed_data[i] = average(l, r, data);
    }
    for (size_t i = num_data - right_edge; i < num_data; ++i) {
      size_t l = i - left_edge;
      size_t r = num_data;
      //cout << "i = " << i << ": l, r = " << l << "," << r << endl;
      smoothed_data[i] = average(l, r, data);
    }
  }
}

// interpolation
inline void interpolateLinear(Vector<Double> const &x0,
    Vector<Double> const &y0, Vector<Double> const &x1, Vector<Double> &y1) {
  // TODO replace with sakura function (need to add double precision version of interpolation function)
  Interpolate1D<Double, Double> interpolator(
      ScalarSampledFunctional<Double>(x0), ScalarSampledFunctional<Double>(y0),
      True, True);
  interpolator.setMethod(Interpolate1D<Double, Double>::linear);
  for (size_t i = 0; i < x1.nelements(); ++i) {
    y1[i] = interpolator(x1[i]);
  }
}
} // anonymous namespace END

namespace casa { // namespace casa START
SDDoubleCircleGainCalImpl::SDDoubleCircleGainCalImpl() :
    central_region_(-1.0), do_smooth_(False), smooth_size_(-1),
    observing_frequency_(0.0), antenna_diameter_(0.0), logger_() {
}

SDDoubleCircleGainCalImpl::~SDDoubleCircleGainCalImpl() {
}

Double SDDoubleCircleGainCalImpl::getPrimaryBeamSize() const {
  return ::getPrimaryBeamSize(observing_frequency_, antenna_diameter_);
}

Int SDDoubleCircleGainCalImpl::getDefaultSmoothingSize() const {
  Int default_size = -1;
  if (central_region_ > 0.0) {
    default_size = ::getDefaultSmoothingSize(central_region_);
  } else {
    default_size = ::getDefaultSmoothingSize(0.5 * getPrimaryBeamSize());
  }
  return default_size;
}

void SDDoubleCircleGainCalImpl::calibrate(Cube<Float> const &data,
    Vector<Double> const &time, Matrix<Double> const &direction,
    Vector<Double> &gain_time, Cube<casacore::Float> &gain) {

  // radius of the central region
  Double radius = central_region_;
  if (radius <= 0.0) {
    // use default value: primary beam size
    radius = 0.5 * getPrimaryBeamSize();
  }
  if (radius <= 0.0) {
    LOG << LogIO::SEVERE << "Size of central region is not properly set: "
        << radius << LogIO::EXCEPTION;
  }

  // select data within radius
  IPosition data_shape = data.shape();
  size_t const num_pol = data_shape[0];
  size_t const num_chan = data_shape[1];
  size_t const num_data = data_shape[2];
  assert(time.nelements() == num_data);
  assert(direction.shape()[1] == num_data);
  assert(direction.shape()[0] == 2);
  Vector<size_t> within_radius(num_data);
  size_t num_gain = 0;
  for (size_t i = 0; i < num_data; ++i) {
    Double x = direction(0, i);
    Double y = direction(1, i);
    Double r2 = x * x + y * y;
    if (r2 <= radius * radius) {
      within_radius[num_gain] = i;
      num_gain++;
    }
  }

//  LOG << "indices within radius: " << within_radius << LogIO::POST;

  if (num_gain < 100) {
    LOG << LogIO::WARN << "Probably not enough points for gain calibration: "
        << num_gain << endl << "Skipping..." << LogIO::POST;
    return;
  }

  // store data for calibration
  LOG << "num_gain = " << num_gain << LogIO::POST;
  gain_time.resize(num_gain);
  gain.resize(num_pol, num_chan, num_gain);
  for (size_t i = 0; i < num_gain; ++i) {
    size_t j = within_radius[i];
    gain_time[i] = time[j];
    gain.xyPlane(i) = data.xyPlane(j);
  }

  // for each spectral data
  Vector<Double> work_data(num_gain);
  Vector<Double> smoothed_data;
  if (do_smooth_) {
    smoothed_data.resize(num_gain);
  }
  for (size_t ipol = 0; ipol < num_pol; ++ipol) {
    for (size_t ichan = 0; ichan < num_chan; ++ichan) {
      for (size_t idata = 0; idata < num_gain; ++idata) {
        work_data[idata] = gain(ipol, ichan, idata);
      }

//      LOG << "work_data[" << ipol << "," << ichan << "]=" << work_data
//          << LogIO::POST;

      // smoothing if necessary
      if (do_smooth_) {
        LOG << "do smoothing with size " << smooth_size_ << LogIO::POST;
        Int smooth_size = smooth_size_;
        if (smooth_size_ < 0) {
          smooth_size = getDefaultSmoothingSize();
          LOG << "default smoothing size will be used: " << smooth_size
              << LogIO::POST;
        } else if (smooth_size_ < 2 || static_cast<size_t>(smooth_size_) >= num_gain) {
          LOG << LogIO::WARN
              << "data is not smoothed since smoothing size is invalid: "
              << smooth_size_ << " (number of data " << num_gain << ")"
              << LogIO::POST;
        }
        smooth(smooth_size, work_data, smoothed_data);
      } else {
        LOG << "no smoothing" << LogIO::POST;
        smoothed_data.reference(work_data);
      }

      LOG << LogIO::DEBUGGING << "smoothed_data[" << ipol << "," << ichan
          << "]=" << smoothed_data << LogIO::POST;

      LOG << LogIO::DEBUGGING << "mean value = " << mean(smoothed_data)
          << LogIO::POST;

      // derive gain factor: mean(smoothed_data) / smoothed_data
      work_data = mean(smoothed_data) / smoothed_data;

//      LOG << "gfactor[" << ipol << "," << ichan << "]=" << work_data
//          << LogIO::POST;

      // conversion for G type calibration
      work_data = 1.0 / sqrt(work_data);

//      LOG << "fparam[" << ipol << "," << ichan << "]=" << work_data
//          << LogIO::POST;

      for (size_t idata = 0; idata < num_gain; ++idata) {
        gain(ipol, ichan, idata) = work_data[idata];
      }
    }
  }
}

//void SDDoubleCircleGainCalImpl::apply(Vector<Double> const &gain_time,
//    Cube<Float> const &gain, Vector<Double> const &time, Cube<Float> &data) {
//  // TODO implement
//  // not necessary to implement? reuse G type application?
//}
} // namespace casa END

/*
 * SDDoubleCircleGainCal.h
 *
 *  Created on: May 31, 2016
 *      Author: nakazato
 */

#ifndef SYNTHESIS_MEASUREMENTCOMPONENTS_SDDOUBLECIRCLEGAINCALIMPL_H_
#define SYNTHESIS_MEASUREMENTCOMPONENTS_SDDOUBLECIRCLEGAINCALIMPL_H_

#include <casacore/casa/aipstype.h>
#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Logging/LogIO.h>

namespace casa { // namespace casa START

class SDDoubleCircleGainCalImpl {
public:
  SDDoubleCircleGainCalImpl();
  virtual ~SDDoubleCircleGainCalImpl();

  // getter
  casacore::Double getCentralRegion() const {
    return central_region_;
  }
  casacore::Bool isSmoothingActive() const {
    return do_smooth_;
  }
  casacore::Int getSmoothingSize() const {
    return smooth_size_;
  }
  casacore::Double getObservingFrequency() const {
    return observing_frequency_;
  }
  casacore::Double getAntennaDiameter() const {
    return antenna_diameter_;
  }
  casacore::Double getPrimaryBeamSize() const;
  casacore::Int getDefaultSmoothingSize() const;

  // setter
  void setCentralRegion(casacore::Double value) {
    central_region_ = value;
  }
  void setSmoothing(casacore::Int size) {
    do_smooth_ = True;
    smooth_size_ = size;
  }
  void unsetSmoothing() {
    do_smooth_ = False;
    smooth_size_ = -1;
  }
  void setObservingFrequency(casacore::Double value) {
    observing_frequency_ = value;
  }
  void setAntennaDiameter(casacore::Double value) {
    antenna_diameter_ = value;
  }

  // gain calibration
  void calibrate(casacore::Cube<casacore::Float> const &data,
      casacore::Vector<casacore::Double> const &time,
      casacore::Matrix<casacore::Double> const &direction,
      casacore::Vector<casacore::Double> &gain_time,
      casacore::Cube<casacore::Float> &gain);

  // apply gain factor
  void apply(casacore::Vector<casacore::Double> const &gain_time,
      casacore::Cube<casacore::Float> const &gain,
      casacore::Vector<casacore::Double> const &time,
      casacore::Cube<casacore::Float> &data);

private:
  // radius of the central region [arcsec]
  casacore::Double central_region_;

  // flag for smoothing
  casacore::Bool do_smooth_;

  // smoothing size [ch]
  casacore::Int smooth_size_;

  // parameter for primary beam size determination
  // observing frequency [Hz]
  casacore::Double observing_frequency_;
  // antenna diameter [m]
  casacore::Double antenna_diameter_;

  // logger
  casacore::LogIO logger_;
};

} // namespace casa END

#endif /* SYNTHESIS_MEASUREMENTCOMPONENTS_SDDOUBLECIRCLEGAINCALIMPL_H_ */

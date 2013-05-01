//#---------------------------------------------------------------------------
//# STAtmosphere.h: Model of atmospheric opacity
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# The code is based on the Fortran code written by Bob Sault for MIRIAD.
//# Converted to C++ by Max Voronkov. This code uses a simple model of the
//# atmosphere and Liebe's model (1985) of the complex refractive index of
//# air.
//# 
//# The model of the atmosphere is one with an exponential fall-off in
//# the water vapour content (scale height of 1540 m) and a temperature lapse
//# rate of 6.5 mK/m. Otherwise the atmosphere obeys the ideal gas equation
//# and hydrostatic equilibrium.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: STAtmosphere.h 1346 2007-04-26 03:24:41Z mar637 $
//#---------------------------------------------------------------------------

#ifndef STATMOSPHERE_H
#define STATMOSPHERE_H

// std includes
#include <vector>
#include <complex>

namespace asap {

/**
  * This class implements opacity/atmospheric brightness temperature model
  * equivalent to the model available in MIRIAD. The actual math is a 
  * convertion of the Fortran code written by Bob Sault for MIRIAD.
  * It implements a simple model of the atmosphere and Liebe's model (1985) 
  * of the complex refractive index of air.
  *
  * The model of the atmosphere is one with an exponential fall-off in
  * the water vapour content (scale height of 1540 m) and a temperature lapse
  * rate of 6.5 mK/m. Otherwise the atmosphere obeys the ideal gas equation
  * and hydrostatic equilibrium.
  *
  * Note, the model includes atmospheric lines up to 800 GHz, but was not 
  * rigorously tested above 100 GHz and for instruments located at 
  * a significant elevation. For high-elevation sites it may be necessary to
  * adjust scale height and lapse rate.
  * 
  * @brief The ASAP atmosphere opacity model
  * @author Max Voronkov
  * @date $Date: 2010-03-17 14:55:17 +1000 (Thu, 26 Apr 2007) $
  * @version
  */
class STAtmosphere {
public:
  /**
   * Default Constructor (apart from optional parameters).
   * The class set up this way will assume International Standard Atmosphere (ISA) conditions,
   * except for humidity. The latter is assumed to be 50%, which seems more realistic for 
   * Australian telescopes than 0%. 
   * @param[in] wvScale water vapour scale height (m), default is 1540m to match MIRIAD's model
   * @param[in] maxAlt maximum altitude of the model atmosphere (m), plane parallel layers are spread linearly up to
   *            this height, default is 10000m to match MIRIAD.
   * @param[in] nLayers number of plane parallel layers in the model (essentially for a numberical integration),
   *            default is 50 to match MIRIAD.
   **/
  explicit STAtmosphere(double wvScale = 1540., double maxAlt = 10000.0, size_t nLayers = 50);
   
  /**
   * Constructor with explicitly given parameters of the atmosphere 
   * @param[in] temperature air temperature at the observatory (K)
   * @param[in] pressure air pressure at the sea level if the observatory elevation 
   *            is set to non-zero value (note, by default is set to 200m) or at the 
   *            observatory ground level if the elevation is set to 0. (The value is in Pascals)
   * @param[in] pressure air pressure at the observatory (Pascals)
   * @param[in] humidity air humidity at the observatory (fraction)
   * @param[in] lapseRate temperature lapse rate (K/m), default is 0.0065 K/m to match MIRIAD and ISA 
   * @param[in] wvScale water vapour scale height (m), default is 1540m to match MIRIAD's model
   * @param[in] maxAlt maximum altitude of the model atmosphere (m), plane parallel layers are spread linearly up to
   *            this height, default is 10000m to match MIRIAD.
   * @param[in] nLayers number of plane parallel layers in the model (essentially for a numberical integration),
   *            default is 50 to match MIRIAD.
   **/
  STAtmosphere(double temperature, double pressure, double humidity, double lapseRate = 0.0065, 
               double wvScale = 1540., double maxAlt = 10000.0, size_t nLayers = 50);
   
  /**
   * Set the new weather station data, recompute the model 
   * @param[in] temperature air temperature at the observatory (K)
   * @param[in] pressure air pressure at the sea level if the observatory elevation 
   *            is set to non-zero value (note, by default is set to 200m) or at the 
   *            observatory ground level if the elevation is set to 0. (The value is in Pascals)
   * @param[in] humidity air humidity at the observatory (fraction)
   **/
  void setWeather(double temperature, double pressure, double humidity);

  /**
   * Set the elevation of the observatory (height above mean sea level)
   *
   * The observatory elevation affects only interpretation of the pressure supplied as part 
   * of the weather data, if this value is non-zero, the pressure (e.g. in setWeather or 
   * constructor) is that at mean sea level. If the observatory elevation is set to zero, 
   * regardless on real elevation, the pressure is that at the observatory ground level.
   *
   * By default, 200m is assumed.
   * @param[in] elev elevation in metres
   **/
  void setObservatoryElevation(double elev);

  /**
   * Calculate zenith opacity at the given frequency. This is a simplified version
   * of the routine implemented in MIRIAD, which calculates just zenith opacity and
   * nothing else. Note, that if the opacity is high, 1/sin(el) law is not correct 
   * even in the plane parallel case due to refraction. 
   * @param[in] freq frequency of interest in Hz
   * @return zenith opacity (nepers, i.e. dimensionless)
   **/
  double zenithOpacity(double freq) const;

  /**
   * Calculate zenith opacity for the range of frequencies. Same as zenithOpacity, but
   * for a vector of frequencies.
   * @param[in] freqs vector of frequencies in Hz
   * @return vector of zenith opacities, one value per frequency (nepers, i.e. dimensionless)
   **/
  std::vector<double> zenithOpacities(const std::vector<double> &freqs) const;
  
  /**
   * Calculate opacity at the given frequency and elevation. This is a simplified 
   * version of the routine implemented in MIRIAD, which calculates just the opacity and
   * nothing else. In contract to zenithOpacity, this method takes into account refraction
   * and is more accurate than if one assumes 1/sin(el) factor.
   * @param[in] freq frequency of interest in Hz
   * @param[in] el elevation in radians
   * @return zenith opacity (nepers, i.e. dimensionless)
   **/ 
  double opacity(double freq, double el) const;

  /**
   * Calculate opacities for the range of frequencies at the given elevation. Same as
   * opacity, but for a vector of frequencies.
   * @param[in] freqs vector of frequencies in Hz
   * @param[in] el elevation in radians
   * @return vector of opacities, one value per frequency (nepers, i.e. dimensionless)
   **/
  std::vector<double> opacities(const std::vector<double> &freqs, double el) const;
          
protected:
  /**
   * Build the atmosphere model based on exponential fall-off, ideal gas and hydrostatic
   * equilibrium. The model parameters are taken from the data members of this class.
   **/
  void recomputeAtmosphereModel();
  
  /**
   * Obtain the number of model layers, do consistency check that everything is
   * resized accordingly
   * @retrun number of model layers
   **/
  size_t nLayers() const;
  
  /**
   * Determine the saturation pressure of water vapour for the given temperature.
   *
   * Reference:
   * Waters, Refraction effects in the neutral atmosphere. Methods of
   * Experimental Physics, vol 12B, p 186-200 (1976).
   *   
   * @param[in] temperature temperature in K
   * @return vapour saturation pressure (Pascals) 
   **/
  static double wvSaturationPressure(double temperature);
   
  /**
   * Compute the complex refractivity of the dry components of the atmosphere
   * (oxygen lines) at the given frequency.
   * @param[in] freq frequency (Hz)
   * @param[in] temperature air temperature (K)
   * @param[in] pDry partial pressure of dry components (Pascals)
   * @param[in] pVapour partial pressure of water vapour (Pascals)
   * @return complex refractivity
   * 
   * Reference:
   * Liebe, An updated model for millimeter wave propogation in moist air,
   * Radio Science, 20, 1069-1089 (1985).
   **/
  static std::complex<double> dryRefractivity(double freq, double temperature, 
                     double pDry, double pVapour);
  
  /**
   * Compute the complex refractivity of the water vapour monomers
   * at the given frequency.
   * @param[in] freq frequency (Hz)
   * @param[in] temperature air temperature (K)
   * @param[in] pDry partial pressure of dry components (Pascals)
   * @param[in] pVapour partial pressure of water vapour (Pascals)
   * @return complex refractivity
   * 
   * Reference:
   * Liebe, An updated model for millimeter wave propogation in moist air,
   * Radio Science, 20, 1069-1089 (1985).
   **/
  static std::complex<double> vapourRefractivity(double freq, double temperature, 
                     double pDry, double pVapour);
      
private:
  
  // heights of all model layers
  std::vector<double> itsHeights;
  
  // temperatures of all model layers
  std::vector<double> itsTemperatures;
  
  // partial pressures of dry component for all model layers
  std::vector<double> itsDryPressures;
  
  // partial pressure of water vapour for all model layers
  std::vector<double> itsVapourPressures;
  
  /**
   *  Atmosphere parameters
   **/
  
  // ground level temperature (K)
  double itsGndTemperature;
  
  // sea level pressure (Pascals)
  double itsPressure;
  
  // ground level humidity (fraction)
  double itsGndHumidity;
  
  // lapse rate (K/m)
  double itsLapseRate;
  
  // water vapour scale height (m)
  double itsWVScale;
  
  // altitude of the highest layer of the model (m)
  double itsMaxAlt;
  
  // observatory elevation (m)
  double itsObsHeight;
};

} // namespace asap

#endif // #ifndef STATMOSPHERE_H


#ifndef _ATM_REFRACTIVEINDEX_H
#define _ATM_REFRACTIVEINDEX_H
/*******************************************************************************
 * ALMA - Atacama Large Millimiter Array
 * (c) Institut de Radioastronomie Millimetrique, 2009
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * "@(#) $Id: ATMRefractiveIndex.h,v 1.5 2010/02/19 01:42:24 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include "ATMCommon.h"
#include <complex>

using namespace std;

ATM_NAMESPACE_BEGIN

/*! \brief RefractiveIndex allows to extract absorption and Phase coefficient(s) at
 *  a given frequency and P/T/gas densities.
 *
 *  \anchor definitions Definitions to intepret the outputs:
 *    - Phase and amplitude response of a plane wave propagating a distance \f$z\f$ at frequency \f$\nu\f$:
 *      \f$E(z)=e^{ikz(1+N)}E(0)\f$
 *    - \f$k=2\pi\nu/c\f$ is the <b>free space wave number</b>.
 *    - \f$N=(n-1)=(N_r+iN_i)\f$ is the <b>Refractivity</b> (\f$n\f$ is the <b>Refraction Index</b>).
 *    - \f$N=N_{g1}+N_{g2}+N_{g3}+ ...\f$ (\f$gi\f$ are atmospheric gases or account for other mechanisms
 *      that modify the refractivity such as collision-induced absorption by \f$O_2-N_2\f$, \f$N_2-N_2\f$,
 *      \f$O_2-O_2\f$, \f$O_2-H_2O\f$ or \f$N_2-H_2O\f$ pairs.
 *    - For a given gas \f$g\f$:
 *       -# \f$N_g=(N_{rg}+iN_{ig})\f$ is the <b>Refractivity</b> of that gas.
 *       -# \f$N_g/\rho_g\f$, where \f$\rho_g\f$ is the <b>number density</b> of gas \f$g\f$, is the <b>Specific Refractivity</b> of that gas.
 *       -# \f$2\pi\nu N_{rg}/c=\phi_g\f$ \f$(rad\cdot m^{-1})\f$ is the <b>Phase Dispersion Coefficient</b> of gas \f$g\f$
 *       -# \f$2\pi\nu N_{ig}/c=\kappa_g\f$ \f$(m^{-1})\f$ is the <b>Absorption Coefficient</b> of gas \f$g\f$
 *       -# \f$2\pi\nu N_{rg}/(c\rho_g)=\phi_g/\rho_{g}\f$ \f$(rad\cdot m^2)\f$ is the <b>Specific Phase Dispersion Coefficient</b> of gas \f$g\f$
 *       -# \f$2\pi\nu N_{ig}/(c\rho_g)=\kappa_g/\rho_{g}\f$ \f$(m^2)\f$ is the <b>Specific Absorption Coefficient</b> of gas \f$g\f$.
 *       -# \f$(2\pi\nu/c)\cdot(N_{rg}+iN_{ig})\f$ \f$(rad\cdot m^{-1},m^{-1})\f$ is the <b>Meaningful Refractivity</b> of gas \f$g\f$.
 *       -# \f$[2\pi\nu/(c\rho_g)]\cdot(N_{rg}+iN_{ig})\f$ \f$(rad\cdot m^2,m^2)\f$ is the <b>Meaningful Specific Refractivity</b> of gas \f$g\f$.
 *    - Therefore in order to obtain Phase Dispersion and Absorption Coefficients:
 *	   -# Absorption Coefficient of gas (\f$m^{-1}\f$) = Imaginary part of the Meaningful Refractivity.
 *       -# Phase Dispersion Coefficient (\f$rad\cdot m^{-1}\f$) = Real part of the Meaningful Refractivity.
 *       -# Specific Absorption Coefficient (\f$m^2\f$) = Imaginary part of the Meaningful Specific Refractivity.
 *       -# Specific Phase Dispersion Coefficient (\f$rad\cdot m^2\f$) = Real part of the Meaningful Specific Refractivity.
 */
class RefractiveIndex
{
public:

  //@{

  /** The constructor has no arguments */
  RefractiveIndex();

  virtual ~RefractiveIndex();

  //@}

  //@{

  /** This returns a complex<double> that is the  Meaningful Refractivity
   (see \ref definitions) of all species of molecular oxygen together in \f$(rad\cdot m^{-1},m^{-1})\f$
   [input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb) and frequency (GHz)] */
  inline complex<double> getMeaningfulRefractivity_o2(double temperature,
                                                      double pressure,
                                                      double wvpressure,
                                                      double frequency)
  {
    return getRefractivity_o2(temperature, pressure, wvpressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  complex<double> getRefractivity_o2(double temperature,
                                     double pressure,
                                     double wvpressure,
                                     double frequency);

  /** This returns a complex<double> that is the  Meaningful Refractivity
   (see \ref definitions) of all species of water together in \f$(rad\cdot m^{-1},m^{-1})\f$
   [input parameters are of type double: temperature (K), pressure (mb), water vapor partial
   pressure (mb) and frequency (GHz)] */
  inline complex<double> getMeaningfulRefractivity_h2o(double temperature,
                                                       double pressure,
                                                       double wvpressure,
                                                       double frequency)
  {
    return getRefractivity_h2o(temperature, pressure, wvpressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex
   with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  complex<double> getRefractivity_h2o(double temperature,
                                      double pressure,
                                      double wvpressure,
                                      double frequency);

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity
   (see \ref definitions) of all species of ozone together in \f$(rad\cdot m^{2},m^{2})\f$
   [input parameters are of type double: temperature (K), pressure (mb), and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_o3(double temperature,
                                                              double pressure,
                                                              double frequency)
  {
    return getSpecificRefractivity_o3(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a
   complex with units \f$(rad\cdot m^{2},m^{2})\f$ */
  complex<double> getSpecificRefractivity_o3(double temperature,
                                             double pressure,
                                             double frequency);
  /** This returns a complex<double> that is the  Meaningful Specific Refractivity
   (see \ref definitions) of all species of ozone together in \f$(rad\cdot m^{-1},m^{-1})\f$
   [input parameters are of type double: temperature (K), pressure (mb), frequency (GHz),
   and \f$O_{3}\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_o3(double temperature,
                                                      double pressure,
                                                      double frequency,
                                                      double numberdensity)
  {
    return getRefractivity_o3(temperature, pressure, frequency, numberdensity);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a
   complex with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_o3(double temperature,
                                            double pressure,
                                            double frequency,
                                            double numberdensity)
  {
    return getSpecificRefractivity_o3(temperature, pressure, frequency)
        * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity
   (see \ref definitions)
   of \f$^{16}O_2\f$ \f$(rad\cdot m^2,m^2)\f$ [input parameters are of type double:
   temperature (K), pressure (mb), water vapor partial pressure (mb) and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o16o(double temperature,
                                                                  double pressure,
                                                                  double wvpressure,
                                                                  double frequency)
  {
    return mkSpecificRefractivity_16o16o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o16o(double temperature,
                                                        double pressure,
                                                        double wvpressure,
                                                        double frequency)
  {
    return mkSpecificRefractivity_16o16o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity
   (see \ref definitions)
   of \f$^{16}O_2\f$ \f$(rad\cdot m^{-1},m^{-1})\f$ [Input parameters are of type double:
   temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz), \f$^{16}O_2\f$ number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o(double temperature,
                                                          double pressure,
                                                          double wvpressure,
                                                          double frequency,
                                                          double numberdensity)
  {
    return mkSpecificRefractivity_16o16o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency) * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o16o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency,
                                                double numberdensity)
  {
    return mkSpecificRefractivity_16o16o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency) * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions)
   of \f$^{16}O_2\f$ \f$v=1\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o16o_vib(double temperature,
                                                                      double pressure,
                                                                      double wvpressure,
                                                                      double frequency)
  {
    return mkSpecificRefractivity_16o16o_vib(temperature,
                                             pressure,
                                             wvpressure,
                                             frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o16o_vib(double temperature,
                                                            double pressure,
                                                            double wvpressure,
                                                            double frequency)
  {
    return mkSpecificRefractivity_16o16o_vib(temperature,
                                             pressure,
                                             wvpressure,
                                             frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions)
   of \f$^{16}O_2\f$ \f$v=1\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and \f$^{16}O_2\f$ \f$v=1\f$  number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o_vib(double temperature,
                                                              double pressure,
                                                              double wvpressure,
                                                              double frequency,
                                                              double numberdensity)
  {
    return mkSpecificRefractivity_16o16o_vib(temperature,
                                             pressure,
                                             wvpressure,
                                             frequency) * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o16o_vib(double temperature,
                                                    double pressure,
                                                    double wvpressure,
                                                    double frequency,
                                                    double numberdensity)
  {
    return mkSpecificRefractivity_16o16o_vib(temperature,
                                             pressure,
                                             wvpressure,
                                             frequency) * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity
   (see \ref definitions) of \f$^{16}O$^{18}O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o18o(double temperature,
                                                                  double pressure,
                                                                  double wvpressure,
                                                                  double frequency)
  {
    return mkSpecificRefractivity_16o18o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o18o(double temperature,
                                                        double pressure,
                                                        double wvpressure,
                                                        double frequency)
  {
    return mkSpecificRefractivity_16o18o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity
   (see \ref definitions) of \f$^{16}O$^{18}O\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and \f$^{16}O$^{18}O\f$ \f$v=0\f$ number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_16o18o(double temperature,
                                                          double pressure,
                                                          double wvpressure,
                                                          double frequency,
                                                          double numberdensity)
  {
    return mkSpecificRefractivity_16o18o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency) * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o18o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency,
                                                double numberdensity)
  {
    return mkSpecificRefractivity_16o18o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency) * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity
   (see \ref definitions) of \f$^{16}O$^{17}O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o17o(double temperature,
                                                                  double pressure,
                                                                  double wvpressure,
                                                                  double frequency)
  {
    return mkSpecificRefractivity_16o17o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o17o(double temperature,
                                                        double pressure,
                                                        double wvpressure,
                                                        double frequency)
  {
    return mkSpecificRefractivity_16o17o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity
   (see \ref definitions) of \f$^{16}O$^{17}O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and \f$^{16}O$^{17}O\f$ \f$v=0\f$ number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_16o17o(double temperature,
                                                          double pressure,
                                                          double wvpressure,
                                                          double frequency,
                                                          double numberdensity)
  {
    return mkSpecificRefractivity_16o17o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency) * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o17o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency,
                                                double numberdensity)
  {
    return mkSpecificRefractivity_16o17o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency) * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$CO\f$ \f$v=0\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_co(double temperature,
                                                              double pressure,
                                                              double frequency)
  {
    return mkSpecificRefractivity_co(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_co(double temperature,
                                                    double pressure,
                                                    double frequency)
  {
    return mkSpecificRefractivity_co(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$CO\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and CO number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_co(double temperature,
                                                      double pressure,
                                                      double frequency,
                                                      double numberdensity)
  {
    return mkSpecificRefractivity_co(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_co(double temperature,
                                            double pressure,
                                            double frequency,
                                            double numberdensity)
  {
    return mkSpecificRefractivity_co(temperature, pressure, frequency)
        * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$N_2O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_n2o(double temperature,
                                                               double pressure,
                                                               double frequency)
  {
    return mkSpecificRefractivity_n2o(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_n2o(double temperature,
                                                     double pressure,
                                                     double frequency)
  {
    return mkSpecificRefractivity_n2o(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$N_2O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and \f$N_2O\f$ number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_n2o(double temperature,
                                                       double pressure,
                                                       double frequency,
                                                       double numberdensity)
  {
    return mkSpecificRefractivity_n2o(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_n2o(double temperature,
                                             double pressure,
                                             double frequency,
                                             double numberdensity)
  {
    return mkSpecificRefractivity_n2o(temperature, pressure, frequency)
        * numberdensity;
  }




 /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$NO_2\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_no2(double temperature,
                                                               double pressure,
                                                               double frequency)
  {
    return mkSpecificRefractivity_no2(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_no2(double temperature,
                                                     double pressure,
                                                     double frequency)
  {
    return mkSpecificRefractivity_no2(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$NO_2\f$ \f$v=0\f$\cdot \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and \f$N_2O\f$ number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_no2(double temperature,
                                                       double pressure,
                                                       double frequency,
                                                       double numberdensity)
  {
    return mkSpecificRefractivity_no2(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_no2(double temperature,
                                             double pressure,
                                             double frequency,
                                             double numberdensity)
  {
    return mkSpecificRefractivity_no2(temperature, pressure, frequency)
        * numberdensity;
  }




/** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$SO_2\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_so2(double temperature,
                                                               double pressure,
                                                               double frequency)
  {
    return mkSpecificRefractivity_so2(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_so2(double temperature,
                                                     double pressure,
                                                     double frequency)
  {
    return mkSpecificRefractivity_so2(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$SO_2\f$ \f$v=0\f$\cdot \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb),
   frequency (GHz), and \f$N_2O\f$ number density (\f$ m^{-3} \f$)] */
  inline complex<double> getMeaningfulRefractivity_so2(double temperature,
                                                       double pressure,
                                                       double frequency,
                                                       double numberdensity)
  {
    return mkSpecificRefractivity_so2(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_so2(double temperature,
                                             double pressure,
                                             double frequency,
                                             double numberdensity)
  {
    return mkSpecificRefractivity_so2(temperature, pressure, frequency)
        * numberdensity;
  }



  inline complex<double> getMeaningfulSpecificRefractivity_cnth2o(double temperature,
                                                                  double pressure,
                                                                  double wvpressure,
                                                                  double frequency)
  {
    return mkSpecificRefractivity_cnth2o(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }

  inline complex<double> getMeaningfulSpecificRefractivity_cntdry(double temperature,
                                                                  double pressure,
                                                                  double wvpressure,
                                                                  double frequency)
  {
    return mkSpecificRefractivity_cntdry(temperature,
                                         pressure,
                                         wvpressure,
                                         frequency);
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$H_2O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_hh16o(double temperature,
                                                                 double pressure,
                                                                 double wvpressure,
                                                                 double frequency)
  {
    return mkSpecificRefractivity_hh16o(temperature,
                                        pressure,
                                        wvpressure,
                                        frequency);
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$H_2O\f$ \f$v_2=1\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_hh16o_v2(double temperature,
                                                                    double pressure,
                                                                    double wvpressure,
                                                                    double frequency)
  {
    return mkSpecificRefractivity_hh16o_v2(temperature,
                                           pressure,
                                           wvpressure,
                                           frequency);
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$H_2^{18}O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_hh18o(double temperature,
                                                                 double pressure,
                                                                 double wvpressure,
                                                                 double frequency)
  {
    return mkSpecificRefractivity_hh18o(temperature,
                                        pressure,
                                        wvpressure,
                                        frequency);
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$H_2^{17}O\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_hh17o(double temperature,
                                                                 double pressure,
                                                                 double wvpressure,
                                                                 double frequency)
  {
    return mkSpecificRefractivity_hh17o(temperature,
                                        pressure,
                                        wvpressure,
                                        frequency);
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$HDO\f$ \f$v=0\f$\cdot \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb), water vapor partial pressure (mb)
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_hdo(double temperature,
                                                               double pressure,
                                                               double wvpressure,
                                                               double frequency)
  {
    return mkSpecificRefractivity_hdo(temperature,
                                      pressure,
                                      wvpressure,
                                      frequency);
  }








  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$v=0\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */

  inline complex<double> getMeaningfulSpecificRefractivity_16o16o16o(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o16o16o(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_16o16o16o(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o16o16o(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$O_{3}\f$ \f$v=0\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o16o(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_16o16o16o(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o(temperature, pressure, frequency)
        * numberdensity;
  }






  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$2=1\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */

  inline complex<double> getMeaningfulSpecificRefractivity_16o16o16o_v2(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o16o16o_v2(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_16o16o16o_v2(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o16o16o_v2(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$v2=1\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$O_{3}\f$ \f$v=0\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o16o_v2(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o_v2(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_16o16o16o_v2(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o_v2(temperature, pressure, frequency)
        * numberdensity;
  }





  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$v1=1\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */

  inline complex<double> getMeaningfulSpecificRefractivity_16o16o16o_v1(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o16o16o_v1(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_16o16o16o_v1(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o16o16o_v1(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$v1=1\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$O_{3}\f$ \f$v=0\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o16o_v1(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o_v1(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_16o16o16o_v1(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o_v1(temperature, pressure, frequency)
        * numberdensity;
  }





  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$3=1\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */

  inline complex<double> getMeaningfulSpecificRefractivity_16o16o16o_v3(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o16o16o_v3(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$  */
  inline complex<double> getSpecificRefractivity_16o16o16o_v3(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o16o16o_v3(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$O_{3}\f$ \f$v3=1\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$O_{3}\f$ \f$v=0\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o16o_v3(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o_v3(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_16o16o16o_v3(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o16o16o_v3(temperature, pressure, frequency)
        * numberdensity;
  }









  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$^{16}O^{16}O^{18}O\f$ \f$v=0\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o16o18o(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o16o18o(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o16o18o(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o16o18o(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$^{16}O^{16}O^{18}O\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz), and \f$^{16}O^{16}O^{18}O\f$ \f$v=0\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o18o(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o16o18o(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$  */
  inline complex<double> getRefractivity_16o16o18o(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o16o18o(temperature, pressure, frequency)
        * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$^{16}O^{16}O^{17}O\f$ \f$v=0\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o16o17o(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o16o17o(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o16o17o(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o16o17o(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$^{16}O^{16}O^{17}O\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$^{16}O^{16}O^{17}O\f$ \f$v=0\f$ number density (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o16o17o(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o16o17o(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o16o17o(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o16o17o(temperature, pressure, frequency)
        * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$^{16}O^{18}O^{16}O\f$ \f$v=0\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o18o16o(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o18o16o(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o18o16o(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o18o16o(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$^{16}O^{18}O^{16}O\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$^{16}O^{18}O^{16}O\f$ \f$v=0\f$ (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o18o16o(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o18o16o(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o18o16o(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o18o16o(temperature, pressure, frequency)
        * numberdensity;
  }

  /** This returns a complex<double> that is the  Meaningful Specific Refractivity (see \ref definitions) of
   \f$^{16}O^{17}O^{16}O\f$ \f$v=0\f$ \f$(rad\cdot m^2,m^2)\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   and frequency (GHz)] */
  inline complex<double> getMeaningfulSpecificRefractivity_16o17o16o(double temperature,
                                                                     double pressure,
                                                                     double frequency)
  {
    return mkSpecificRefractivity_16o17o16o(temperature, pressure, frequency);
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with units
   \f$(rad\cdot m^{2},m^{2})\f$ */
  inline complex<double> getSpecificRefractivity_16o17o16o(double temperature,
                                                           double pressure,
                                                           double frequency)
  {
    return mkSpecificRefractivity_16o17o16o(temperature, pressure, frequency);
  }
  /** This returns a complex<double> that is the  Meaningful Refractivity (see \ref definitions) of
   \f$^{16}O^{17}O^{16}O\f$ \f$v=0\f$ \f$(rad\cdot m^{-1},m^{-1})\f$
   [Input parameters are of type double: temperature (K), pressure (mb),
   frequency (GHz), and \f$^{16}O^{17}O^{16}O\f$ \f$v=0\f$ (\f$ m^{-3}\f$)] */
  inline complex<double> getMeaningfulRefractivity_16o17o16o(double temperature,
                                                             double pressure,
                                                             double frequency,
                                                             double numberdensity)
  {
    return mkSpecificRefractivity_16o17o16o(temperature, pressure, frequency)
        * numberdensity;
  }
  /** Same as the previous one but omitting "Meaningful" from the function's name. It returns a complex with
   units \f$(rad\cdot m^{-1},m^{-1})\f$ */
  inline complex<double> getRefractivity_16o17o16o(double temperature,
                                                   double pressure,
                                                   double frequency,
                                                   double numberdensity)
  {
    return mkSpecificRefractivity_16o17o16o(temperature, pressure, frequency)
        * numberdensity;
  }

  //@}

private:

  complex<double> mkSpecificRefractivity_16o16o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency);
  complex<double> mkSpecificRefractivity_16o16o_vib(double temperature,
                                                    double pressure,
                                                    double wvpressure,
                                                    double frequency);
  complex<double> mkSpecificRefractivity_16o18o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency);
  complex<double> mkSpecificRefractivity_16o17o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency);
  complex<double> mkSpecificRefractivity_co(double temperature,
                                            double pressure,
                                            double frequency);
  complex<double> mkSpecificRefractivity_n2o(double temperature,
                                             double pressure,
                                             double frequency);
  complex<double> mkSpecificRefractivity_no2(double temperature,
                                             double pressure,
                                             double frequency);
  complex<double> mkSpecificRefractivity_so2(double temperature,
                                             double pressure,
                                             double frequency);
  complex<double> mkSpecificRefractivity_cnth2o(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency);
  complex<double> mkSpecificRefractivity_cntdry(double temperature,
                                                double pressure,
                                                double wvpressure,
                                                double frequency);
  complex<double> mkSpecificRefractivity_hh16o(double temperature,
                                               double pressure,
                                               double wvpressure,
                                               double frequency);
  complex<double> mkSpecificRefractivity_hh16o_v2(double temperature,
                                                  double pressure,
                                                  double wvpressure,
                                                  double frequency);
  complex<double> mkSpecificRefractivity_hh18o(double temperature,
                                               double pressure,
                                               double wvpressure,
                                               double frequency);
  complex<double> mkSpecificRefractivity_hh17o(double temperature,
                                               double pressure,
                                               double wvpressure,
                                               double frequency);
  complex<double> mkSpecificRefractivity_hdo(double temperature,
                                             double pressure,
                                             double wvpressure,
                                             double frequency);
  complex<double> mkSpecificRefractivity_16o16o16o(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o16o16o_v2(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o16o16o_v1(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o16o16o_v3(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o16o18o(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o16o17o(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o18o16o(double temperature,
                                                   double pressure,
                                                   double frequency);
  complex<double> mkSpecificRefractivity_16o17o16o(double temperature,
                                                   double pressure,
                                                   double frequency);

  double linebroadening(double frequency,
                        double temperature,
                        double pressure,
                        double mmol,
                        double dv0_lines,
                        double texp_lines);
  double linebroadening_water(double frequency,
                              double temperature,
                              double pressure,
                              double wvpressure,
                              double ensanche1,
                              double ensanche2,
                              double ensanche3,
                              double ensanche4);
  double linebroadening_hh18o_hh17o(double temperature,
                                    double pressure,
                                    double ph2o,
                                    double dv0,
                                    double dvlm,
                                    double temp_exp);
  double linebroadening_o2(double frequency,
                           double temperature,
                           double pressure,
                           double ph2o,
                           double mmol,
                           double ensanche1,
                           double ensanche2);
  double interf_o2(double temperature,
                   double pressure,
                   double ensanche3,
                   double ensanche4);
  complex<double> lineshape(double frequency,
                            double linefreq,
                            double linebroad,
                            double interf);

}; // class RefractiveIndex

ATM_NAMESPACE_END

#endif /*!_ATM_REFRACTIVEINDEX_H*/

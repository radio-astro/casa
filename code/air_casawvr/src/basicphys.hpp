/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file basicphys.hpp

   Basic physics definitions
*/

#ifndef _LIBAIR_BASICPHYS_HPP__
#define _LIBAIR_BASICPHYS_HPP__

#include <cmath>

namespace LibAIR2 {

  /**
     Value taken from the "am" program. Units [K * cm^2 * GHz^3 /
     watt].
   */
  const double CSquared_On_TwoK = 1.0/3.0723613e-17;

  /** Conversion of mm of perciptible water vapour to molecules per cm
      squared.
   */
  const double pmw_mm_to_n = 3.3427e21;

  /** Molecular mass of the main isotope of water, in atomic mass
      units.
   */
  const double h2o_1_mass_amu = 18.0105646863;

  /**
     Atomic mass unit in terms of kilograms.
   */
  const double amu_kg = 1.660538782e-27;

  /**  Constants for standard dry air equation of state
   */
  namespace STP {
    /** Loschmidt constant [cm^-3] (IUPAP 1987) */
    const double N_STP = 2.686763e19;

    /** Standard temperature  [K] */
    const double T_STP = 273.15;
    
    /** Standard pressure  [mbar] */ 
    const double P_STP =	1013.25		; 
  }


  /**
     Computes:
     \f$
     \frac{2h}{c^2} \frac{\nu^3}{e^{\frac{h\nu}{kT}-1}}
     \f$
   */
  template<class fT>
  fT BPlanck( fT f, 
	      fT T)
  {
    const fT  TWOH_ON_CSQUARED = 1.4744992e-18;
    const fT  H_ON_K	       = 4.7992375e-2;

    return TWOH_ON_CSQUARED * std::pow( f, 3) / 
      ( exp( H_ON_K / T * f) -1 );

  }

  /**
     Converts water column c to extra path. c is in mm, path is mm.
     
     Equation taken from Alison Stirling's memo #496-1.
   */
  double SW_WaterToPath_Simplified( double c, 
				    double T);

  /** \brief Compute the absolute humidity from relative humdity,
     pressure and temperature

     The current implementation of this function is after the atm
     program.
     
     \param rh relative humidity as a fraction
     \param T  Temperature of air in K
     \param P  Pressure of air in mBar

     \return absolue humidity in g/m^3
   */
  double AbsHum(double rh,
		double T,
		double P);

}

#endif

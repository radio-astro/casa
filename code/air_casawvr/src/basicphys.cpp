/**
   \file basicphys.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Maintained by ESO since 2013.

*/

#include "basicphys.hpp"

namespace LibAIR2 {

  double SW_WaterToPath_Simplified( double c, 
				    double T)
  {
    return  c * ( pmw_mm_to_n *  h2o_1_mass_amu * amu_kg )* 1.742 * 1e7 / T;
  }

  double AbsHum(double rh,
		double T,
		double P)
  {

    // Saturation pressure of water vapour
    const double es = 6.105*exp(25.22 / T * (T - 273.0) - 5.31 * log(T / 273.0));

    double e = 1.0 - (1.0 - rh) * es / P;
    e = es * rh / e;

    double rwat0 = e * 216.502 / T;
    return rwat0;
  }


}



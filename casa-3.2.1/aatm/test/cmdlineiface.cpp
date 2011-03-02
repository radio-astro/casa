// Copyright (2009) Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
// 
// This file is part of AATM
//
// AATM is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AATM is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AATM.  If not, see <http://www.gnu.org/licenses/>.
//
// Comments regarding this example welcome at: 
// Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>


#include "cmdlineiface.hpp"

void addAtmoOptions(boost::program_options::options_description &desc)
{
  using namespace boost::program_options;

  desc.add_options()
    ("ghum", 
     value<double>(), 
     "Relative humidity at ground level (percent) (currently incompatible with --freq)")
    ("pwv", 
     value<double>(), 
     "Precipitable water vapour in mm (incompatible with --ghum)")
    ("altitude", 
     value<double>()->default_value(5000), 
     "Altitude of the site (m)")
    ("gpress", 
     value<double>()->default_value(560), 
     "Ground level pressure (mBar)")
    ("gtemp", 
     value<double>()->default_value(270), 
     "Ground level temperature (K)")
    ("wvl", 
     value<double>()->default_value(2), 
     "Water vapour scale height (km)")
    ("tlr", 
     value<double>()->default_value(-5.6), 
     "Tropospheric lapse rate (K/km)");
}

void addStdOutputOptions(boost::program_options::options_description &desc)
{
  using namespace boost::program_options;
  desc.add_options()
    ("help", "Produce this help message")
    ("freq", value<double>(), "Frequency at which to compute the absorption (GHz)")
    ("fmin", value<double>(), "The starting frequency (GHz) of a frequency grid (incompatible with --freq)")
    ("fmax", value<double>(), "The end frequency (GHz) of a frequency grid (incompatible with --freq)")
    ("fstep", value<double>(), "The frequency step (GHz) of a frequency grid (incompatible with --freq)")
    ;  
}


pAtmProf
simpleAOSAtmo(double ghum,
	      const boost::program_options::variables_map &vm)
{
  using namespace atm;

  unsigned int   atmType = 1; // Atmospheric type (to reproduce behavior above the tropopause)

  Temperature T(vm["gtemp"].as<double>(),
		"K" );

  Pressure         P(vm["gpress"].as<double>(),
		     "mb");     // Ground Pressure

  Humidity         H(  ghum,"%" );     // Ground Relative Humidity (indication)

  Length         Alt(vm["altitude"].as<double>(),
		     "m");     // Altitude of the site 

  Length WVL(vm["wvl"].as<double>(),
	     "km");
  
  double TLR=vm["tlr"].as<double>();

  Length      topAtm(  48.0,"km");     // Upper atm. boundary for calculations
  Pressure     Pstep(  10.0,"mb");     // Primary pressure step
  double   PstepFact=         1.2;     // Pressure step ratio between two consecutive layers

  return pAtmProf(new AtmProfile(Alt,
				 P,
				 T,
				 TLR,
				 H,
				 WVL,
				 Pstep,
				 PstepFact,
				 topAtm, 
				 atmType));
}

pAtmProf
AOSAtmo_pwv(const boost::program_options::variables_map &vm)
{
  using namespace atm;

  const double pwv=vm["pwv"].as<double>();
  
  double ghum_guess=10.0;
  pAtmProf p1= simpleAOSAtmo(ghum_guess,
			     vm);
  
  // Non-adaptive iteration is wasteful and a bit dangerous here, but
  // should be sufficient for almost all problems since generally
  // people are concerned with lower relative humidity regions
  for(size_t i=0; i<10; ++i)
  {
    ghum_guess= ghum_guess * pwv/p1->getGroundWH2O().get("mm");
    p1=simpleAOSAtmo(ghum_guess,
		     vm);
  }
  return p1;
			     
}


// Copyright (2008) Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
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
//

#include <boost/program_options.hpp>

#include "../config.h"
#include "atmincludes.hpp"


typedef boost::shared_ptr<atm::AtmProfile> pAtmProf;

pAtmProf
simpleAOSAtmo(double ghum)
{
  using namespace atm;

  Atmospheretype   atmType = tropical; // Atmospheric type (to reproduce behavior above the tropopause)
  Temperature      T( 270.0,"K" );     // Ground temperature
  Pressure         P( 560.0,"mb");     // Ground Pressure
  Humidity         H(  ghum,"%" );     // Ground Relative Humidity (indication)
  Length         Alt(  5000,"m" );     // Altitude of the site 
  Length         WVL(   2.0,"km");     // Water vapor scale height
  double         TLR=  -5.6      ;     // Tropospheric lapse rate (must be in K/km)
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

void absorption(double freq,
		double ghum)
{
  pAtmProf atmo(simpleAOSAtmo(ghum));

  atm::Frequency  afreq(freq,
			"GHz");

  atm::RefractiveIndexProfile rip(afreq,
				  *atmo);
  
  std::cout<<"Freq"
	   <<"\t"
	   <<"Dry Opac"
	   <<"\t\t"
	   <<"Wet Opac" 
	   <<std::endl;

  std::cout<<freq
	   <<"\t"
	   <<rip.getDryOpacity().get()
	   <<"\t\t"
  	   <<rip.getWetOpacity().get()
	   <<std::endl;
}

int main(int ac, char* av[])
{   
  using namespace boost::program_options;
  
  options_description desc("Allowed options");
  desc.add_options()
    ("help", "Produce this help message")
    ("freq", value<double>(), "Frequency at which to compute the dispersion (GHz)")
    ("ghum", value<double>(), "Relative humidity at ground level (percent)")
    ;

  variables_map vm;        
  store(parse_command_line(ac, av, desc), vm);
  notify(vm);    

  if (vm.count("help"))
  {
    std::cout<<"Compute absorption by the atmosphere"
	     <<std::endl
	     <<std::endl
	     <<"This program is part of AATM version " + std::string(VERSION)
	     <<std::endl
	     <<"GPL license -- you have the right to the source code. See COPYING"
	     <<std::endl
	     <<std::endl
	     <<desc;
  }
  else
  {
    absorption(vm["freq"].as<double>(),
	       vm["ghum"].as<double>()
	       );
  }
  
  return 0;
}  

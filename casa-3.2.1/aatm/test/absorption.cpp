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
#include <boost/format.hpp>

#include "../config.h"
#include "cmdlineiface.hpp"


void absorption(double freq,
		double ghum,
		const boost::program_options::variables_map &vm)
{
  pAtmProf atmo(simpleAOSAtmo(ghum,vm));

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

void absorption_fgrid(const boost::program_options::variables_map &vm)
{
  pAtmProf atmo(AOSAtmo_pwv(vm));
  
  const double fmin=vm["fmin"].as<double>();
  const double fmax=vm["fmax"].as<double>();
  const double fstep=vm["fstep"].as<double>();
  const size_t nc=(fmax-fmin)/fstep;
  const double pwv=vm["pwv"].as<double>();

  atm::SpectralGrid grid(nc,
			 0,
			 atm::Frequency(fmin,
					"GHz"),
			 atm::Frequency(fstep,
					"GHz"));

  atm::RefractiveIndexProfile rip(grid,
				  *atmo);
  atm::SkyStatus ss(rip);
  
  std::cout<<"Freq"
	   <<"\t"
	   <<"Dry Opac"
	   <<"\t\t"
	   <<"Wet Opac"
	   <<"\t\t"
	   <<"Sky Brightness"
	   <<std::endl;
  
  for(size_t i=0; i<nc; ++i)
  {
    std::cout<<boost::format("%g, \t%g, \t%g, \t%g") 
      % grid.getChanFreq(i).get()
      % rip.getDryOpacity(i).get()
      % rip.getWetOpacity(i).get()
      % ss.getTebbSky(i, atm::Length(pwv,"mm")).get()
	     <<std::endl;
  }
}

int main(int ac, char* av[])
{   
  using namespace boost::program_options;
  
  options_description desc("Allowed options");

  addStdOutputOptions(desc);  
  addAtmoOptions(desc);

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
  else if (vm.count("ghum") && vm.count("pwv"))
  {
    std::cout<<"Can't specify both ghum and pwv"
	     <<std::endl;
  }
  else
  {
    if(vm.count("fstep"))
    {
      absorption_fgrid(vm);
    }
    else
    {
      absorption(vm["freq"].as<double>(),
		 vm["ghum"].as<double>(),
		 vm
		 );
    }
  }
  
  return 0;
}  

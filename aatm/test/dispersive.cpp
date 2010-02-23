// Copyright (2008,2009) Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
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

#include <stdexcept>
#include <boost/program_options.hpp>


#include "../config.h"
#include "cmdlineiface.hpp"

void oHeader(std::ostream &os)
{
  os<<"Freq,"
    <<"\t"
    <<"Wet Non-Disp,"
    <<"\t\t"
    <<"Wet Disp," 
    <<"\t\t"
    <<"Dry Non-Disp"
    <<std::endl;
}

void oRow(std::ostream &os,
	  double freq,
	  double nd_h2o,
	  double d_h2o,
	  double nd_dry)
{
  os<<freq
    <<",\t"
    <<nd_h2o
    <<",\t\t"
    <<d_h2o
    <<",\t\t"
    <<nd_dry
    <<std::endl;    
}

void dispersive(const boost::program_options::variables_map &vm)
{
  pAtmProf atmo(AOSAtmo_pwv(vm));

  const double freq=vm["freq"].as<double>();
  atm::RefractiveIndexProfile rip(atm::Frequency(freq, "GHz"),
				  *atmo);  

  oHeader(std::cout);
  oRow(std::cout,
       freq,
       rip.getNonDispersiveH2OPathLength().get(),
       rip.getDispersiveH2OPathLength().get(),
       rip.getNonDispersiveDryPathLength().get());
}

void dispersive_fgrid(const boost::program_options::variables_map &vm)
{
  pAtmProf atmo;
  if (vm.count("pwv"))
  { 
    atmo=AOSAtmo_pwv(vm);
  }
  else if (vm.count("ghum"))
  {
    atmo=simpleAOSAtmo(vm["ghum"].as<double>(),
		       vm);
  }
  else
  {
    throw std::runtime_error("Must specify ghum (ground-level humidity)or pwv");
  }


  const double fmin=vm["fmin"].as<double>();
  const double fmax=vm["fmax"].as<double>();
  const double fstep=vm["fstep"].as<double>();
  const size_t nc=(fmax-fmin)/fstep;

  atm::SpectralGrid grid(nc,
			 0,
			 atm::Frequency(fmin,
					"GHz"),
			 atm::Frequency(fstep,
					"GHz"));

  atm::RefractiveIndexProfile rip(grid,
				  *atmo);

  std::cout.precision(14);
  oHeader(std::cout);
  for(size_t i=0; i<nc; ++i)
  {
    oRow(std::cout,
	 grid.getChanFreq(i).get(),
	 rip.getNonDispersiveH2OPathLength(i).get(),
	 rip.getDispersiveH2OPathLength(i).get(),
	 rip.getNonDispersiveDryPathLength(i).get());
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
    std::cout<<"Compute dispersive delay in the atmosphere"
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
    if(vm.count("fstep"))
    {
      dispersive_fgrid(vm);
    }
    else
    {
      dispersive(vm);
    }
  }
  
  return 0;
}  

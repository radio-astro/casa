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

/** \file cmdlineiface.hpp
    
    Routines for interfacing with the command line
*/
#ifndef _AATM_TEST_CMDLINEIFACE_HPP__
#define _AATM_TEST_CMDLINEIFACE_HPP__

#include <boost/program_options.hpp>

#include "atmincludes.hpp"


/** Add the options regarding the atmosphere and their defaults to the
    options available in the program.

 */
void addAtmoOptions(boost::program_options::options_description &desc);


/** Add the options regarding the output of the program
 */
void addStdOutputOptions(boost::program_options::options_description &desc);


/** Pointer to an atmospheric profiler
 */
typedef boost::shared_ptr<atm::AtmProfile> pAtmProf;

/**
   Create an atmosphere given a variable map "vm" which contains most
   of the parameters and the relative humidity separately as
   "ghum". The reason for separtion is that there is no obvious way of
   specify the total water vapour column when creating the atmosphere,
   hence ghum needs to be fiddled width.
 */
pAtmProf
simpleAOSAtmo(double ghum,
	      const boost::program_options::variables_map &vm);


/** Create an atmosphere with humidity specified as total PWV within
    the variable map structure

 */
pAtmProf
AOSAtmo_pwv(const boost::program_options::variables_map &vm);






#endif
    

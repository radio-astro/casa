/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version May 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file tbutils.hpp

   Utilities for dealing with brighhtness temperatures
*/

#ifndef _LIBAIR_TBUTILS_HPP__
#define _LIBAIR_TBUTILS_HPP__
namespace LibAIR2 {

  /** Convert Planck temperature at frequency f to an R-J temperature.
   */
  double PlanckToRJ(double f,
		    double TPlanck);
  
  

}
#endif

/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version August 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dispersion.hpp

   Routines for dealing with dispersion
*/
#ifndef _LIBAIR_DISPERSION_HPP__
#define _LIBAIR_DISPERSION_HPP__

#include <map>
#include <functional>

namespace LibAIR2 {

  class Dispersion:
    std::unary_function<double, double>
  {

  };

  class DispersionTab:
    public Dispersion,
    public std::map<double, double>
  {

  public:

    /** Return interpolated dispersion factor at frequency fnu
     */
    double operator() (double fnu);

  };

  /** Load contents of a dispersion table from a CSV file
   */
  void loadCSV(const char *fname,
	       DispersionTab &dt);


}

#endif


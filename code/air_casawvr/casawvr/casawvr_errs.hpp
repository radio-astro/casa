/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version March 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file casaswvr_errs.hpp

   Error hirerarchy for the casaWVR module
*/
#ifndef _LIBAIR_CASAWVR_CASAWVR_ERR_HPP__
#define _LIBAIR_CASAWVR_CASAWVR_ERR_HPP__

#include <stdexcept>

namespace LibAIR2 {

  class MSInputDataError: 
    public std::runtime_error
  {
  public:

    MSInputDataError(const std::string &s):
      std::runtime_error("There is an error in the supplied input data which prevents the calculation\
 of the calibration: " + s)
    {}
    
  };


}


#endif


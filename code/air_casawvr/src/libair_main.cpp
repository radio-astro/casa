/**
   \file libair_main.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   Initial version February 2008
   Maintained by ESO since 2013.

*/

#include <boost/preprocessor/stringize.hpp>

#include "libair_main.hpp"
#ifndef CMAKE_BUILD
#include "../config.h"
#endif

namespace LibAIR2 {
  
  const char * version(void)
  {
    #ifndef CMAKE_BUILD
    return PACKAGE_VERSION;
    #else
    return BOOST_PP_STRINGIZE(CMAKE_BUILD);
    #endif
  }
  

}





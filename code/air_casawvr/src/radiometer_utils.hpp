/**
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version March 2008
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file radiometer_utils.hpp

*/
#ifndef _LIBAIR_RADIOMETER_UTILS_HPP__
#define _LIBAIR_RADIOMETER_UTILS_HPP__

#include <vector>
#include <memory>

namespace LibAIR2 {

  // Forward Declarations
  class Radiometer;

  /** \brief Merge a set of radiometers into  one mult-channel
      radiometer

      Re-orders the frequency grid if necessary.
      
      A new radiometer object is constructed and no reference to
      supplied radiometers is taken.

   */
  std::auto_ptr<Radiometer> MergeRadiometers( std::vector<const Radiometer *> & vr);

}

#endif

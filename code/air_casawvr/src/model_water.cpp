// Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
// Initial version June 2009
// Maintained by ESO since 2013.

/** \file model_water.cpp

 */

#include "model_water.hpp"
#include "singlelayerwater.hpp"

namespace LibAIR2 {

  // Instnatiate the static class data members here because it looks
  // like GCC V4.0 has bugs prventing doing that correctly
  // automatically

  template<> const double WaterModel<ISingleLayerWater>::n_bump=0.001;
  template<> const double WaterModel<ISingleLayerWater>::tau_bump=0.001;

  template<> const double WaterModel<ICloudyWater>::n_bump=0.001;
  template<> const double WaterModel<ICloudyWater>::tau_bump=0.001;

  
  

}



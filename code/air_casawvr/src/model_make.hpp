/**
  Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
  Initial version July 2009
  Maintained by ESO since 2013.

  This file is part of LibAIR and is licensed under GNU Public
  License Version 2

  \file model_make.hpp
  
  Utilities to instantiate models
*/

#ifndef _LIBAIR_MODEL_MAKE_HPP__
#define _LIBAIR_MODEL_MAKE_HPP__

#include "model_water.hpp"

namespace LibAIR2 {

  // Forward declarations
  class ISingleLayerWater;
  class ICloudyWater;
  class ALMAWVRCharacter;

  /**
     Creates a single layer water vapour model
   */
  LibAIR2::WaterModel<ISingleLayerWater> *
  mkSingleLayerWater(RadiometerT radiot, 
		     PartitionTreatment t,
		     Continuum c,
		     double PDrop=0);    

  /**
     Creates a single layer water vapour model, but with arbitary
     filter centres/frequencies -- suitable for analysis of data with
     characterised rather than nominal WVR performance.
     
   */
  LibAIR2::WaterModel<ISingleLayerWater> *
  mkSingleLayerWater(const ALMAWVRCharacter &ac, 
		     PartitionTreatment t,
		     Continuum c,
		     double PDrop=0);    

  LibAIR2::WaterModel<ICloudyWater> *
  mkCloudy(RadiometerT radiot, 
	   PartitionTreatment t,
	   Continuum c,
	   double PDrop=0);    

  LibAIR2::WaterModel<ICloudyWater> *
  mkCloudy(const ALMAWVRCharacter &ac, 
	   PartitionTreatment t,
	   Continuum c,
	   double PDrop=0);    

  
  LibAIR2::WaterModel<ISingleLayerWater> *
  mkSimpleOffset(double cf,
		 double bw);

}


#endif

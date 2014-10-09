// Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
// Initial version July 2009
// Maintained by ESO since 2013.

/** \file model_make.cpp

 */

#include "model_make.hpp"
#include "model_water.hpp"
#include "singlelayerwater.hpp"
#include "models_basic.hpp"
#include "slice.hpp"
#include "layers.hpp"
#include "rtranfer.hpp"
#include "radiometermeasure.hpp"

namespace LibAIR2 {

  LibAIR2::WaterModel<ISingleLayerWater> *
  mkSingleLayerWater(RadiometerT radiot, 
		     PartitionTreatment t,
		     Continuum c,
		     double PDrop)
  {
    boost::shared_ptr<Radiometer> r(SwitchRadiometer(radiot));

    boost::shared_ptr<ISingleLayerWater> sl(new ISingleLayerWater (r->getFGrid(),
								   WaterData::L183,
								   t,
								   c,
								   PDrop));
    return new LibAIR2::WaterModel<ISingleLayerWater> (r,
						      sl);
    
  }

  LibAIR2::WaterModel<ISingleLayerWater> *
  mkSingleLayerWater(const ALMAWVRCharacter &ac, 
		     PartitionTreatment t,
		     Continuum c,
		     double PDrop)
  {
    boost::shared_ptr<Radiometer> r(MkALMAWVR(ac));

    boost::shared_ptr<ISingleLayerWater> sl(new ISingleLayerWater (r->getFGrid(),
								   WaterData::L183,
								   t,
								   c,
								   PDrop));
    return new LibAIR2::WaterModel<ISingleLayerWater> (r,
						      sl);

  }

  LibAIR2::WaterModel<ISingleLayerWater> *
  mkSimpleOffset(double cf,
		 double bw)
  {
    boost::shared_ptr<Radiometer> r(MkALMAWVR_offset(cf,bw));

    boost::shared_ptr<ISingleLayerWater> sl(new ISingleLayerWater (r->getFGrid(),
								   WaterData::L183,
								   PartTable,
								   AirCont,
								   0));
    return new LibAIR2::WaterModel<ISingleLayerWater> (r,
						      sl);

  }

  LibAIR2::WaterModel<ICloudyWater> *
  mkCloudy(RadiometerT radiot, 
	   PartitionTreatment t,
	   Continuum c,
	   double PDrop)
  {
    boost::shared_ptr<Radiometer> r(SwitchRadiometer(radiot));

    boost::shared_ptr<ICloudyWater> sl(new ICloudyWater (r->getFGrid(),
							 WaterData::L183,
							 t,
							 c,
							 PDrop));
    return new LibAIR2::WaterModel<ICloudyWater> (r,
						 sl);

  }

  LibAIR2::WaterModel<ICloudyWater> *
  mkCloudy(const ALMAWVRCharacter &ac, 
	   PartitionTreatment t,
	   Continuum c,
	   double PDrop)
  {
    boost::shared_ptr<Radiometer> r(MkALMAWVR(ac));

    boost::shared_ptr<ICloudyWater> sl(new ICloudyWater (r->getFGrid(),
							 WaterData::L183,
							 t,
							 c,
							 PDrop));
    return new LibAIR2::WaterModel<ICloudyWater> (r,
						 sl);

  }
  

}




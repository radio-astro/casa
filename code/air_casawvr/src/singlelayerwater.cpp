/**
   \file singlelayerwater.cpp

   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
   Initial version February 2009
   Maintained by ESO since 2013.
   
*/

#include "slice.hpp"
#include "columns.hpp"
#include "basicphys.hpp"
#include "partitionsum.hpp"
#include "partitionsum_testdata.hpp"
#include "rtranfer.hpp"
#include "layers.hpp"

#include "singlelayerwater.hpp"

namespace LibAIR2 {

  ISingleLayerWater::ISingleLayerWater(const std::vector<double> &fgrid,
				       WaterData::Lines l,
				       PartitionTreatment t,
				       Continuum c,
				       double PDrop):
    WaterData(l,t,c,0.0),
    n(0),
    T(0),
    P(0)    
  {
    if (PDrop == 0.0)
    {
      s.reset (new Slice(T, P));
      s->AddColumn(*wcol());
      if (wcont())
      {
	s->AddColumn(*wcont());
      }
      sr.reset(new SliceResult(*s,
			       fgrid));
    }
    else
    {
      // We have a layer!
      layer.reset(new IsoTLayer(T,P, PDrop));
      layer->AddColumn(*wcol());
      if (wcont())
      {
	layer->AddColumn(*wcont());
      }
      sr.reset(new LayerResult(*layer,
			       fgrid));
    }
    scratch.resize(fgrid.size());
    setBckgT(2.7);
  }

  const std::vector<double> & ISingleLayerWater::TbGrid(void)
  {
    updatePars();
    sr->UpdateI(*bckg);
    return sr->UpdateTb();
  }

  const std::vector<double> 
  &ISingleLayerWater::TbGrid(const SliceResult &background)
  {
    updatePars();
    sr->UpdateI(background);
    return sr->UpdateTb();
  }
    
  
  const SliceResult & ISingleLayerWater::getBckg(void) const
  {
    return *bckg;
  }

  void ISingleLayerWater::updatePars(void)
  {
    wcol()->setN( n * pmw_mm_to_n );
    if (wcont()) 
    {
      wcont()->setN( n * pmw_mm_to_n );
    }
    
    if (layer)
    {
      layer->setT(T);
      layer->setP(P);
    }
    else
    {
      s->setT(T);
      s->setP(P);
    }
  }

  void ISingleLayerWater::setBckgT(double Tbckg)
  {
    cmbslice.reset(new OpaqueSlice(Tbckg, 0));
    bckg.reset(new SliceResult(*cmbslice,
			       sr->f));
    bckg->UpdateI();
  }


  const std::vector<double> & ISingleLayerWater::getFGrid(void)
  {
    return sr->f;
  }
      
}



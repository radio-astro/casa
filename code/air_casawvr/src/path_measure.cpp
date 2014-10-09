/**
   \file path_measure.cpp
   
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
*/   

#include "path_measure.hpp"
#include "model_iface.hpp"
#include "numalgo.hpp"

namespace LibAIR2 {

  PathMeasure::PathMeasure(WVRAtmoQuantModel &model) :
    ALMAMeasure(model),
    Tb_obs(4),
    Tb_sigma(4),
    dT_dL(4),
    dT_dL_sigma(4)
  {
  }

  void PathMeasure::modelObs(void)
  {
    model.eval(Tb_obs);
    model.dTdL_ND(dT_dL);    
  }

  double PathMeasure::lLikely (void) const
  {
    double ltot=0;

    std::vector<double> res;
    model.eval(res);

    ltot+=GaussError( Tb_obs.begin(), res, Tb_sigma.thermNoise);
    
    for (size_t i =0 ; i < 4; ++i)
      res[i]=model.dTdL_ND(i);

    ltot+=GaussError( dT_dL.begin(), res, dT_dL_sigma.thermNoise);    

    return ltot;
  }

}


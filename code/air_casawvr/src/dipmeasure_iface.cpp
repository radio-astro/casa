/**
   \file dipmeasure_iface.cpp

   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>   
*/

#include "dipmeasure_iface.hpp"
#include "dipmodel_iface.hpp"
#include "numalgo.hpp"

namespace LibAIR2{
  
  DipNormMeasure::DipNormMeasure(PPDipModel & model):
    ALMAMeasure(model),
    NormalNoise(4),
    _model(model)
  {
  }

  void DipNormMeasure::addObs(double za,
			      const std::vector<double> & skyTb)
  {
    obs.push_back( boost::make_tuple(za, skyTb));
  }

  double DipNormMeasure::lLikely (void) const
  {
    std::vector<double> scratch;
    double res=0;
    for (size_t i = 0 ; i < obs.size() ; ++i)
    {
      double za;
      obs_t  skyT;
      boost::tie(za, skyT) = obs[i];

      _model.setZA(za);
      _model.eval(scratch);

      res+= GaussError( skyT.begin(), scratch, thermNoise);
    }

    return res;
  }
  
  

}


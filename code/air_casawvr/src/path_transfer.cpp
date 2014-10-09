/**
   \file path_transfer.cpp
   
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
*/   

#include "path_transfer.hpp"
#include "dipmodel_iface.hpp"
#include "numalgo.hpp"

namespace LibAIR2 {

  PathTransfer::PathTransfer(PPDipModel &model):
    ALMAMeasure(model),
    _model(model),
    Tb_q(4),
    Tb_s(4),
    Tb_sigma(4),
    dT_dL(4),
    dT_dL_sigma(4)
  {
  }

  double PathTransfer::lLikely(void) const
  {
    double ltot=0;
    std::vector<double> scratch(4);
    
    _model.setZA(za_q);
    _model.eval(scratch);
    
    ltot+=GaussError(Tb_q.begin(),
		     scratch,
		     Tb_sigma.thermNoise);

    for (size_t i =0 ; i < 4; ++i)
      scratch[i]=_model.dTdL_ND(i);
    
    ltot+=GaussError(dT_dL.begin(),
		     scratch,
		     dT_dL_sigma.thermNoise);    

    _model.setZA(za_s);
    _model.eval(scratch);
    
    ltot+=GaussError(Tb_s.begin(),
		     scratch,
		     Tb_sigma.thermNoise);
    return ltot;

  }
	   


}



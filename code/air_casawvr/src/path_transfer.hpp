/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file path_transfer.hpp
   
*/   
#ifndef _LIBAIR_PATH_TRANSFER_HPP__
#define _LIBAIR_PATH_TRANSFER_HPP__

#include "path_measure.hpp"

namespace LibAIR2 {

  class PPDipModel;

  /** \brief Measurement for inference of dT/dL to when transfering to
     a different elevation
     
     This class contains the full information about transfer: Tb and
     dT/dL in direction of quasar; and Tb in science direction.
     
   */
  class PathTransfer :
    public ALMAMeasure
  {

    /// A model that knows how to account for elevation change
    PPDipModel &_model;
    
  public:

    /// Sky brightness toward the quasar
    std::vector<double> Tb_q;

    /// Zenith angle toward the quasar
    double za_q;
    
    /// Sky brightness toward the science source
    std::vector<double> Tb_s;

    /// Zenith angle toward the science source
    double za_s;
    
    /// Noise on brightness measurement
    NormalNoise Tb_sigma;

    /// Observed dT/dLs (toward quasar of course)
    std::vector<double> dT_dL;

    /// Noise on dT_dL
    NormalNoise dT_dL_sigma;

    // ---------- Construction / Destruction --------------

    PathTransfer(PPDipModel &model);

    // ----------------Inherited from Minim::MLikelihood -------------------
    double lLikely(void) const;        
    

  };


}

#endif

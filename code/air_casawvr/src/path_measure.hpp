/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file path_measure.hpp
   
*/   
#ifndef _LIBAIR_PATH_MEASURE_HPP__
#define _LIBAIR_PATH_MEASURE_HPP__

#include "measure_iface.hpp"

namespace LibAIR2 {
  
  /** \brief Incorporate measurement of observed dT/dLs

     Calculate likelihood of data which include the differential of
     the brightness temperature wrt path as well as the brightness
     temperatures themselves.

   */
  class PathMeasure :
    public ALMAMeasure
  {

  public:
    
    // ---------- Public data -----------------------------

    /// Obsevation of brighntess temperatures
    std::vector<double> Tb_obs;
    /// Noise on brightness temerature measurement
    NormalNoise Tb_sigma;

    /** \brief Obsevation of the differential of brightness
	temperature wrt path

	Assume non-dispersive path.
     */
    std::vector<double> dT_dL;

    /**
       Noise on dT_dL;
     */
    NormalNoise dT_dL_sigma;


    // ---------- Construction / Destruction --------------

    /// Initialse from a model for atmospheric properties
    PathMeasure(WVRAtmoQuantModel &model);

    // ---------- Public interface   --------------    

    /// Set the observations memebers from the prediction from current
    /// model parameters
    void modelObs(void);

    // ----------------Inherited from Minim::MLikelihood -------------------
    double lLikely (void) const;        
        

  };
  

}

#endif

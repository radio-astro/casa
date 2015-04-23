/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file measure_iface.hpp

   Represetnation of WVR and related measurements
*/
#ifndef _LIBAIR_MEASURE_IFACE_HPP__
#define _LIBAIR_MEASURE_IFACE_HPP__

#include "bnmin1/src/minimmodel.hxx"

namespace LibAIR2 {

  class WVRAtmoQuantModel;

  /** \brief Can calculate the likelihood of an ALMA measurement given
     an atmospheric model.
     
   */
  class ALMAMeasure :
    public Minim::MLikelihood
  {

  protected:

    WVRAtmoQuantModel &model;

  public:

    // ---------- Construction / Destruction --------------
    
    ALMAMeasure(WVRAtmoQuantModel &model);

    // ---------- Public interface   --------------    

    // -------------- Inherited from Minim::Model --------------------------
    void AddParams ( std::vector< Minim::DParamCtr > &pars );    
    
  };

  /** \brief Base class for measurements with independent gaussian
      noise
   */
  struct NormalNoise
  {
    /// The thermal, gaussian random noise of measurement
    std::vector<double> thermNoise;

    /**
       \param n number of measurement channels
     */
    NormalNoise(size_t n);

  };

  /** \brief Likelihoods for a single absolute WVR measurement

      Here we assume independent normally distributed noise on each
      WVR channel.
   */
  class AbsNormMeasure :
    public ALMAMeasure,
    public NormalNoise
  {

  public:

    // ---------- Public data -----------------------------

    /// The observed data
    std::vector<double> obs;

    // ---------- Construction / Destruction --------------
    
    AbsNormMeasure(WVRAtmoQuantModel &model);

    // ---------- Public interface   --------------    

    /// Set the observation from current model
    void modelObs(void);

    // ----------------Inherited from Minim::MLikelihood -------------------
    double lLikely (void) const;        
    

  };

}
#endif


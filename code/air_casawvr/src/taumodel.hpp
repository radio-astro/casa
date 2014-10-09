/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file taumodel.hpp
   
*/
#ifndef _LIBAIR_TAUMODEL_HPP__
#define _LIBAIR_TAUMODEL_HPP__

#include "model_iface.hpp"

namespace LibAIR2 {

  /** \brief A simple fixed opacity per airmass model

      This can be used for testing of algorithms and to explore what
      information can be obtained from sky-dips. 
   */
  class TauModel:
    public WVRAtmoQuantModel
  {

  protected:
    
    /// The opacity per unit airmass of the model
    double n;

    /// The physical temperature of the medium
    double TPhy;

    /// CMB temperature
    const double TBack;

    /// CMB Temperature in R-J units
    const double TBackRJ;

  public:

    // ---------- Construction / Destruction --------------

    /** Default construction is OK since there isn't really much to
	initialise
     */
    TauModel(void);

    // ---------- Public interface ------------------------

    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const ;
    virtual void eval(std::vector<double> & res) const ;

    /** \bug This function is not relevant for TauModel, do not use
     */
    virtual double dTdc (size_t ch) const;

    /** \bug This function is not relevant for TauModel, do not use
     */
    virtual double dTdL_ND (size_t ch) const;    

    /** \bug This function is not relevant for TauModel, do not use
     */
    virtual void dTdL_ND(std::vector<double> &res) const;
    // Inherited from WVRAtmoModel
    void AddParams(std::vector<Minim::DParamCtr> &pars);        


  };

  /** \brief Tau model with a lapse rate
      
      See note 2009/4
   */
  class LapseTauModel:
    public TauModel
  {
    
  protected:

    /// The lapse rate, in units of K/m
    double Gamma;

    /// Normalised kappa, i.e., kappa divided by tau
    const double nkappa;

    /// Scale height
    const double hS;
    
  public:

    // ---------- Construction / Destruction --------------

    /**
       \param Gamma The lapse rate in units of K/m 

       \param hS Scale height of the absorbing species
       
       \param hT Truncation height of the absorbing species
     */
    LapseTauModel(double Gamma,
		  double hS,
		  double hT);
		  

    // ---------- Public interface ------------------------

    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const ;
    // Inherited from WVRAtmoModel
    void AddParams(std::vector<Minim::DParamCtr> &pars);        
    

  };

  
  /** \brief Lapse rate model with numerical integration
     
   */
  class NumLapseTau:
    public LapseTauModel
  {
    /// Number of steps in integration
    const size_t nstep;

    /* Return height as function of tau

       \param ntau is the normalised tau (i.e., tauprime/ total tau)

     */
    double htau(double ntau) const;

  public:

    // ---------- Construction / Destruction --------------

    NumLapseTau(double Gamma,
		double hS,
		double hT,
		size_t nstep);

    // ---------- Public interface ------------------------

    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const ;

  };

  /** \brief Include a parameter that does not vary with airmass

      \note Experimental class to work on explaining the channel 1 sky
      dips
   */
  class OffsetTauModel:
    public TauModel
  {

  protected:

    /// The opacity which does not scale with airmass
    double nfix;
    
  public:
    
    OffsetTauModel(double nfix);    

    // ---------- Public interface ------------------------

    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const;    

  };

  


}

#endif

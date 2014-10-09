/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dipmodel_iface.hpp
   
   Interface for dip models
*/
#ifndef _LIBAIR_DIPMODEL_IFACE_HPP__
#define _LIBAIR_DIPMODEL_IFACE_HPP__

#include <boost/scoped_ptr.hpp>

#include "model_iface.hpp"

namespace LibAIR2 {

  /** \brief A plane-parallel model of the atmosphere for measurement
      of dips
   */
  class PPDipModel:
    public WVRAtmoQuantModel
  {
    double _n;
    double _za;
    double *_nam;
    boost::scoped_ptr<WVRAtmoQuantModel> _am;
    
    void setPars(void) const;
    
  public:

    // ---------- Construction / Destruction --------------
    
    /**
       Construct from an underlying quant model. Note that a
       CouplingModel can be passed to here to represent non-perfect
       coupling to the sky.

       \param am The underlying model of zenith sky brightness.  We
       take ownership of this object

     */
    PPDipModel(WVRAtmoQuantModel * am);

    virtual ~PPDipModel();

    // ---------- Public interface   --------------    

    /** \brief Set the current zenith angle of the model
     */
    void setZA(double za);

    /** \brief Return the current zenith angle of the model
     */
    double getZA(void) const {return _za;};

    // Inherited from WVRAtmoQuants
    virtual double eval(size_t ch) const ;
    virtual void eval(std::vector<double> & res) const ;
    virtual double dTdc (size_t ch) const;
    virtual double dTdL_ND (size_t ch) const;    
    void dTdL_ND (std::vector<double> & res) const;
    // Inherited from WVRAtmoModel
    virtual void AddParams ( std::vector< Minim::DParamCtr > &pars );    

  };

}

#endif


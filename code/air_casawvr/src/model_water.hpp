/** 
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version June 2009
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file model_water.hpp

 */
#ifndef _LIBAIR_MODEL_WATER_HPP__
#define _LIBAIR_MODEL_WATER_HPP__

#include <boost/shared_ptr.hpp>

#include "model_iface.hpp"
#include "radiometermeasure.hpp"
#include "basicphys.hpp"
#include "cloudywater.hpp"

namespace LibAIR2 {

  // Forward decleration
  class Radiometer;
  
  template <class AM>
  class WaterModel:
    public WVRAtmoQuantModel
  {

    boost::shared_ptr<Radiometer> mr;
    boost::shared_ptr<AM> am;

  public:

    // -------------------- Public data ---------------------

    static const double n_bump;

    static const double tau_bump;

    // -------------------- Construction / Destruction ------


    WaterModel(boost::shared_ptr<Radiometer> mr,
	       boost::shared_ptr<AM> am):
      mr(mr),
      am(am)
    {
    }
    
    // Inherited from WVRAtmoQuants
    double eval(size_t ch) const
    {
      return mr->eval(am->TbGrid(), 
		      ch);
    }

    virtual void eval(std::vector<double> & res) const
    {
      const size_t nc = mr->nchannels();
      res.resize(nc);
      const std::vector<double> & 
	tbg = am->TbGrid();
      
      for (size_t i = 0 ; i < nc ; ++i)
	res[i]= mr->eval(tbg,i);
    }

    virtual double dTdc (size_t ch) const
    {
      const double invdelta = 0.5/n_bump;
      const double on=am->n;
      
      am->n=on+n_bump;
      const double fwdval =  eval(ch);
      am->n=on-n_bump;
      const double backval = eval(ch);
      am->n=on;
      
      return (fwdval - backval) * invdelta ;
    }

    void dTdc (std::vector<double> &res) const
    {
      const double invdelta = 0.5/ n_bump;
      const size_t nc = mr->nchannels();
      const double on=am->n;
      
      std::vector<double> fwdval(nc);
      am->n=on+n_bump;
      eval(fwdval);
      

      std::vector<double> backval(nc);
      am->n=on-n_bump;
      eval(backval);

      am->n=on;
      
      res.resize(nc);
      for(size_t  i=0 ; i < res.size() ; ++i)
	res[i] = (fwdval[i] - backval[i]) * invdelta ;
      
    }

    double dTdL_ND (size_t ch) const
    {
      return dTdc(ch) / SW_WaterToPath_Simplified(1.0, 
						  am->T);
    }

    void dTdL_ND (std::vector<double> & res) const
    {
      dTdc(res);
      const double conv = 1.0 / SW_WaterToPath_Simplified(1.0, 
							  am->T);
      
      for (size_t i =0 ; i < res.size() ; ++i)
	res[i] *= conv;
    }

    /** Compute the derivative of sky temperatures wvr the continuum
	opacity term
     */
    void dTdTau (std::vector<double> &res) const
    {
      throw std::runtime_error("This function requires specific impelmentation classes");
    }    

    // Inherited from WVRAtmoModel
    void AddParams ( std::vector< Minim::DParamCtr > &pars )
    {
      pars.push_back(Minim::DParamCtr ( &am->n ,      
					"n", 
					true     ,                       
					"Water column (mm)"
					));
      
      pars.push_back(Minim::DParamCtr ( &am->T ,      
					"T", 
					true     ,                       
					"Temperature (K)"
					));
      
      pars.push_back(Minim::DParamCtr ( &am->P ,      
					"P", 
					true     ,                       
					"Pressure (mBar)"
					));    

    }

  };

  template<>   inline 
  void WaterModel<ICloudyWater>::AddParams (std::vector< Minim::DParamCtr > &pars)
  {
      pars.push_back(Minim::DParamCtr ( &am->n ,      
					"n", 
					true     ,                       
					"Water column (mm)"
					));
      
      pars.push_back(Minim::DParamCtr ( &am->T ,      
					"T", 
					true     ,                       
					"Temperature (K)"
					));
      
      pars.push_back(Minim::DParamCtr ( &am->P ,      
					"P", 
					true     ,                       
					"Pressure (mBar)"
					));    

      pars.push_back(Minim::DParamCtr ( &am->tau183 ,      
					"tau183", 
					true     ,                       
					"Opacity at 183GHz"
					));    

  }

  template<>   inline 
  void WaterModel<ICloudyWater>::dTdTau (std::vector<double> &res) const
  {
    const double invdelta = 0.5/ tau_bump;
    const size_t nc = mr->nchannels();
    const double on=am->tau183;
    
    std::vector<double> fwdval(nc);
    am->tau183=on+tau_bump;
    eval(fwdval);
    
    
    std::vector<double> backval(nc);
    am->tau183=on-tau_bump;
    eval(backval);
    
    am->tau183=on;
    
    res.resize(nc);
    for(size_t  i=0 ; i < res.size() ; ++i)
      res[i] = (fwdval[i] - backval[i]) * invdelta ;
    
  }    

}



#endif

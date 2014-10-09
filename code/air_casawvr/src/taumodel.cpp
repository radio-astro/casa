/**
   \file taumodel.cpp
   
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
*/

#include <cmath>
#include "taumodel.hpp"
#include "tbutils.hpp"

namespace LibAIR2 {

  TauModel::TauModel(void):
    n(0.1),
    TPhy(275),
    TBack(2.7),
    TBackRJ(PlanckToRJ(183.3, TBack))
  {
  }

  double TauModel::eval(size_t ch) const
  {
    return TBackRJ * exp(-n)+ TPhy*(1-exp(-n));
  }

  void TauModel::eval(std::vector<double> & res) const
  {
    res.resize(1);
    res[0]=eval(0);
  }

  double TauModel::dTdc (size_t ch) const
  {
    return -1;
  }

  double TauModel::dTdL_ND (size_t ch) const
  {
    return -1;
  }

  void TauModel::dTdL_ND(std::vector<double> &res) const
  {
    res.resize(1);
    res[0]=-1;
  }
  
  void TauModel::AddParams(std::vector<Minim::DParamCtr> &pars)
  {
    pars.push_back(Minim::DParamCtr (&n ,      
				     "n", 
				     true     ,                       
				     "Opacity per unit airmass"
				      ));

    pars.push_back(Minim::DParamCtr (&TPhy ,      
				     "TPhy", 
				     true     ,                       
				     "Physical temperature"
				      ));
  }

  LapseTauModel::LapseTauModel(double Gamma,
			       double hS,
			       double hT):
    Gamma(Gamma),
    nkappa(1.0/(hS*(1-exp(-hT/hS)))),
    hS(hS)
  {
  }
  
  double LapseTauModel::eval(size_t ch) const
  {
    return TBackRJ * exp(-n) + 
      TPhy*(1-exp(-n)) +
      Gamma / ( nkappa * n) * ( 1 - (n+1)*exp(-n));
  }  

  void LapseTauModel::AddParams(std::vector<Minim::DParamCtr> &pars)
  {
    TauModel::AddParams(pars);
    
    pars.push_back(Minim::DParamCtr (&Gamma ,      
				     "gamma", 
				     false,                       
				     "Atmospheric lapse rate"
				      ));

  }


  NumLapseTau::NumLapseTau(double Gamma,
			   double hS,
			   double hT,
			   size_t nstep):
    LapseTauModel(Gamma, hS, hT),
    nstep(nstep)
  {
  }

  double NumLapseTau::htau(double ntau) const
  {
    return -hS * log( 1- ntau/nkappa/hS);
  }

  double NumLapseTau::eval(size_t ch) const
  {
    const double deltan = n/nstep;

    double Tt = TBackRJ;
    for (size_t i=0; i < nstep; ++i)
    {
      const double TLocal=TPhy + Gamma * htau(i*deltan/n);
      Tt= Tt*exp(-deltan) + TLocal*(1-exp(-deltan));
    }

    return Tt;
  }

  OffsetTauModel::OffsetTauModel(double nfix):
    nfix(nfix)
  {
  }
  
  double OffsetTauModel::eval(size_t ch) const
  {
    const double ntot=nfix+n;
    return TBackRJ * exp(-ntot)+ TPhy*(1-exp(-ntot));
  }

}



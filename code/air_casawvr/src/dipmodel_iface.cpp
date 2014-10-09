/**
   \file dipmodel_iface.cpp
   
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
*/

#include "dipmodel_iface.hpp"

#include <cmath>

namespace LibAIR2 {

  PPDipModel::PPDipModel(WVRAtmoQuantModel * am):
    _n(0),
    _za(0),
    _nam(NULL),
    _am(am)
  {
    std::vector< Minim::DParamCtr > pars;
    am->AddParams(pars);
    for( size_t i =0 ; i < pars.size() ; ++i)
      if(pars[i].name == "n") 
	_nam=pars[i].p;
  }

  PPDipModel::~PPDipModel()
  {
  }

  void PPDipModel::setPars(void) const
  {
    *_nam = ( _n / cos(_za) );
  }

  void PPDipModel::setZA(double za)
  {
    _za=za;
  }
  
  double PPDipModel::eval(size_t ch) const 
  {
    setPars();
    return _am->eval(ch);
  }

  void PPDipModel::eval(std::vector<double> & res) const 
  {
    setPars();
    _am->eval(res);
  }

  double PPDipModel::dTdc (size_t ch) const
  {
    setPars();
    return _am->dTdc(ch);
  }

  double PPDipModel::dTdL_ND (size_t ch) const
  {
    setPars();
    return _am->dTdL_ND(ch);
  }

  void PPDipModel::dTdL_ND (std::vector<double> & res) const
  {
    setPars();
    _am->dTdL_ND(res);
  }

  
  void PPDipModel::AddParams ( std::vector< Minim::DParamCtr > &pars )
  {
    _am->AddParams(pars);
    // Swap the pointer in the column density parameter of the model
    // to be to our variable _n; We will update the model depending on
    // _za;
    for( size_t i =0 ; i < pars.size() ; ++i)
      if(pars[i].name == "n") 
	pars[i].p= &_n;
  }

}




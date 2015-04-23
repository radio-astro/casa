/**
   \file model_iface.cpp
   
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
*/

#include "model_iface.hpp"

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <bnmin1/src/minim.hxx>

#include "models_basic.hpp"
#include "radiometermeasure.hpp"
#include "slice.hpp"
#include "columns.hpp"
#include "lineparams.hpp"
#include "basicphys.hpp"
#include "partitionsum.hpp"
#include "partitionsum_testdata.hpp"
#include "rtranfer.hpp"
#include "layers.hpp"
 
namespace LibAIR2 {

  WVRAtmoQuants::~WVRAtmoQuants(void)
  {
  }

  boost::array<double, 4>
  WVRAtmoQuantModel::evalFn(double x, const std::string &pname)
  {
    boost::array<double, 4> res;
    Minim::ModelDesc md(*this);
    double oldval=*md[pname]->p;
    *md[pname]->p=x;
    std::vector<double> Tb(4);
    eval(Tb);
    for(size_t i=0; i<4; ++i)
      res[i]=Tb[i];
    *md[pname]->p=oldval;
    return res;
  }

  CouplingModel::CouplingModel(WVRAtmoQuantModel * am):
    coupling(1.0),
    TTerm(1.0),
    _am(am)
  {
  }

  void CouplingModel::setSpill(double c, double T)
  {
    coupling=c;
    TTerm=T;
  }

  double CouplingModel::eval(size_t ch) const 
  {
    const double TSky=_am->eval(ch);
    return coupling*TSky + (1-coupling)* TTerm;
  }

  void CouplingModel::eval(std::vector<double> & res) const 
  {
    _am->eval(res);
    for (size_t i =0 ; i < res.size() ; ++i)
      res[i]= coupling * res[i] + (1-coupling)* TTerm;
  }

  double CouplingModel::dTdc (size_t ch) const
  {
    return coupling*_am->dTdc(ch); 
  }

  double CouplingModel::dTdL_ND (size_t ch) const
  {
    return coupling*_am->dTdL_ND(ch);
  }
  
  void CouplingModel::dTdL_ND (std::vector<double> & res) const
  {
    _am->dTdL_ND(res);

    BOOST_FOREACH( double &x, res)
      x*= coupling;
      
  }
  
  void   CouplingModel::AddParams ( std::vector< Minim::DParamCtr > &pars )
  {
    _am->AddParams(pars);

    pars.push_back(Minim::DParamCtr ( &coupling ,      
				      "coupling", 
				      true     ,                       
				      "Coupling to sky"
				      ));

    pars.push_back(Minim::DParamCtr ( &TTerm ,      
				      "TTerm", 
				      false,                       
				      "Termination temperature"
				      ));
  }

  AbsCalModel::AbsCalModel(WVRAtmoQuantModel * am,
			   size_t nchan):
    zero(nchan, 0.0),
    scale(nchan, 1.0),
    _am(am)
  {
  }

  double AbsCalModel::eval(size_t ch) const 
  {
    const double TSky=_am->eval(ch);
    return zero[ch]+scale[ch]*TSky;
  }

  void AbsCalModel::eval(std::vector<double> & res) const 
  {
    _am->eval(res);
    for (size_t i =0 ; i < res.size() ; ++i)
      res[i]= scale[i] * res[i] + zero[i];
  }

  double AbsCalModel::dTdc (size_t ch) const
  {
    return scale[ch]*_am->dTdc(ch); 
  }

  double AbsCalModel::dTdL_ND (size_t ch) const
  {
    return scale[ch]*_am->dTdL_ND(ch);
  }
  
  void AbsCalModel::dTdL_ND (std::vector<double> & res) const
  {
    _am->dTdL_ND(res);
    for(size_t i=0; i<res.size(); ++i)
      res[i]=scale[i]*res[i];
  }

  void AbsCalModel::AddParams(std::vector<Minim::DParamCtr> &pars)
  {
    using namespace boost;

    _am->AddParams(pars);

    for(size_t i=0; i<zero.size(); ++i)
    {
      pars.push_back(Minim::DParamCtr(&zero[i],      
				      (format("abszero%i") % (i+1)).str(), 
				      true,                       
				      (format("Absolute calibration zero of channel %i") % (i+1)).str()
				      ));

      pars.push_back(Minim::DParamCtr(&scale[i],      
				      (format("absscale%i") % (i+1)).str(), 
				      true,                       
				      (format("Absolute calibration scaling of channel %i") % (i+1)).str()
				      ));
    }
  }

  

  

}


   

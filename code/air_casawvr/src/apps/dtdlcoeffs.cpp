/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dtdlcoeffs.cpp

*/

#include <algorithm> 
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/algorithm.hpp>

#include <cmath>

#include "dtdlcoeffs.hpp"
#include "almaresults.hpp"

namespace LibAIR2 {

  dTdLCoeffsBase::dTdLCoeffsBase()
  {
    boost::array<double, 4> _chmask={{1,1,1,1}};
    chmask=_chmask;
  }

  dTdLCoeffsBase::~dTdLCoeffsBase()
  {
  }

  dTdLCoeffsSingle::dTdLCoeffsSingle(const std::vector<double> &c,
				     const std::vector<double> &e):
    c(c),
    c2(4, 0.0),
    e(e)
  {
  }

  dTdLCoeffsSingle::dTdLCoeffsSingle(const  ALMAResBase &r):
    c(4),
    c2(4, 0.0),
    e(4)
  {
    for(size_t i=0; i<4; ++i)
    {
      c[i]=r.dTdL[i];
      e[i]=r.dTdL_err[i];
    }
  }

  void dTdLCoeffsSingle::get(size_t i,
			     double time,
			     double el,
			     std::vector<double> &res,
			     std::vector<double> &res2) const
  {
    res.resize(c.size());
    std::copy(c.begin(), 
	      c.end(),
	      res.begin());

    for(size_t i=0; i<4; ++i)
      res[i]*=chmask[i];

    res2.resize(c2.size());
    std::copy(c2.begin(), 
	      c2.end(),
	      res2.begin());
	      
  }

  void dTdLCoeffsSingle::print(std::ostream &os)
  {
    os<<"Coefficients: ";
    for(size_t i=0; i<4; ++i)
      os<<c[i]<<", ";
    os<<std::endl;
  }

  void dTdLCoeffsSingle::repr(std::vector<double> &res,
			      std::vector<double> &err) const
  {
    res=c;
    err=e;
  }

  bool dTdLCoeffsSingle::isnan(void) const
  {
    return std::count_if(c.begin(), c.end(), std::isnan<double>);
  }

  dTdLCoeffsIndiv::dTdLCoeffsIndiv(const coeff_t &c):
    coeff(c)
  {
  }

  dTdLCoeffsIndiv::dTdLCoeffsIndiv(size_t nWVR):
    coeff(boost::extents[4][nWVR][3])
  {
  }

  void dTdLCoeffsIndiv::set(size_t i,
			    const std::vector<double> &c,
			    const std::vector<double> &e)
  {
    for(size_t k=0; k<4; ++k)
    {
      coeff[k][i][0]=c[k];
      coeff[k][i][1]=e[k];
    }
  }

  void dTdLCoeffsIndiv::get(size_t i,
			    double time,
			    double el,
			    std::vector<double> &res,
			    std::vector<double> &res2) const
  {
    const size_t nc=coeff.shape()[0];
    res.resize(nc);
    res2.resize(nc);
    for(size_t k=0; k<nc; ++k)
    {
      res[k]=coeff[k][i][0]*chmask[k];
      res2[k]=coeff[k][i][2];
    }
  }

  void dTdLCoeffsIndiv::print(std::ostream &os)
  {
    os<<"Coefficients: ";
    for(size_t i=0; i<4; ++i)
      os<<coeff[i][0][0]<<", ";
    os<<std::endl;
  }

  void dTdLCoeffsIndiv::repr(std::vector<double> &res,
			     std::vector<double> &err) const
  {
    const size_t nc=coeff.shape()[0];
    res.resize(nc);
    err.resize(nc);
    for(size_t k=0; k<nc; ++k)
    {
      res[k]=coeff[k][0][0];
      err[k]=coeff[k][0][1];
    }
  }

  bool dTdLCoeffsIndiv::isnan(void) const
  {
    return std::count_if(coeff.origin(), 
			 coeff.origin()+(coeff.shape()[0]*coeff.shape()[1]*coeff.shape()[2]), 
			 std::isnan<double>);
  }


  dTdLCoeffsSingleInterpolated::dTdLCoeffsSingleInterpolated()
  {
  }

  void dTdLCoeffsSingleInterpolated::insert(double time,
					    const boost::array<double, 4> &coeffs,
					    const boost::array<double, 4> &err)
  {
    ret_t t;
    t.time=time;
    t.coeffs=coeffs;
    t.err=err;
    retrievals.insert(t);
  }

  void dTdLCoeffsSingleInterpolated::get(size_t i,
					 double time,
					 double el,
					 std::vector<double> &res,
					 std::vector<double> &c2) const
  {
    if (retrievals.size() == 0) 
    {
      throw std::runtime_error("No retrievals have been enterred");
    }

    res.resize(4);
    c2.resize(4);
    if (time<retrievals.begin()->time)
    {
      // request is before the first retrieval
      std::copy(retrievals.begin()->coeffs.begin(), 
		retrievals.begin()->coeffs.end(),
		res.begin());      
      for(size_t i=0; i<4; ++i)
	res[i]*=chmask[i];
      std::copy(retrievals.begin()->c2.begin(), 
		retrievals.begin()->c2.end(),
		c2.begin());      
      return;
    }
    std::set<ret_t>::const_iterator prev;
    std::set<ret_t>::const_iterator next;

    for(next=retrievals.begin();
	next !=retrievals.end() and time >= next->time;
	++next)
    {
    }

    if (next ==retrievals.end())
    {
      // requested time is after the last retrieval
      std::set<ret_t>::reverse_iterator last=retrievals.rbegin();
      std::copy(last->coeffs.begin(), 
		last->coeffs.end(),
		res.begin());         
      for(size_t i=0; i<4; ++i)
	res[i]*=chmask[i];
      std::copy(last->c2.begin(), 
		last->c2.end(),
		c2.begin());            
      return;
    }

    prev=next;
    prev--;


    //Finally if not before beginning or after end, interpolate
    //linearly
    
    double c_f=(time-prev->time)/(next->time-prev->time);
    double c_s=(next->time-time)/(next->time-prev->time);
    
    for(size_t k=0; k<4; ++k)
    {
      res[k]= c_f*(prev->coeffs[k]) + c_s*(next->coeffs[k]);
      res[k]*= chmask[k];
      c2[k]= c_f*(prev->c2[k]) + c_s*(next->c2[k]);
    }
  }

  void dTdLCoeffsSingleInterpolated::print(std::ostream &os)
  {
    for(std::set<ret_t>::const_iterator i=retrievals.begin();
	i!=retrievals.end();
	++i)
    {
      os<<boost::format("Retrieval at %f is: [") % i->time;
      for(size_t j=0; j<4; ++j)
      {
	os<<i->coeffs[j]<<",";
      }
      os<<"]"
	<<std::endl;
    }
  }

  void dTdLCoeffsSingleInterpolated::repr(std::vector<double> &res,
					  std::vector<double> &err) const
  {
    std::set<ret_t>::const_iterator i=retrievals.begin();
    for(size_t j=0; j < retrievals.size()/2;++j)
    {
      ++i;
    };
    res.resize(4);
    err.resize(4);
    for(size_t j=0; j<4; ++j)
    {
      res[j]=i->coeffs[j];
      err[j]=i->err[j];
    }
  }

  bool dTdLCoeffsSingleInterpolated::isnan(void) const
  {
    ;
    for(std::set<ret_t>::const_iterator i=retrievals.begin(); 
	i != retrievals.end();
	++i)
    {
      for(size_t j=0; j<4; ++j)
      {
	if ( std::isnan(i->coeffs[j]) || (std::isnan(i->err[j])) )
	{
	  return true;
	}
      }
    }
    return false;
  }
}



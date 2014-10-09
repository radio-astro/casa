/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. Incorporating code originally in
   apps/almaabs_i.cpp.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file dtdltools.cpp
*/

#include <cmath>

#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "almawvr/nestedsampler.hxx"

#include "dtdltools.hpp"
#include "model_make.hpp"
#include "numalgo.hpp"




namespace LibAIR2 {

  void dTdLMom1(const std::list<Minim::WPPoint> &l,
		Minim::ModelDesc &md,
		const WVRAtmoQuants &model,
		double Z,
		double thresh,
		double *res)
  {
    std::vector<double> scratch(4, 0.0);
    const double Pthresh= Z*thresh;
    for(size_t i=0; i<4; ++i)
      res[i]=0;

    for(std::list<Minim::WPPoint>::const_iterator i=l.begin();
	i!= l.end();
	++i)
    {
      const double w=i->w *exp(- i->ll);
      if (w > Pthresh)
      {
	md.put(i->p);
	model.dTdL_ND(scratch);
	for(size_t j=0; j<scratch.size(); ++j)
	  res[j]+=(scratch[j]*w);
      }
    }
    for(size_t j=0; j<scratch.size(); ++j)
    res[j]/=Z;
  }

  void dTdLMom2(const std::list<Minim::WPPoint> &l,
		Minim::ModelDesc &md,
		const WVRAtmoQuants &model,
		const double *m1,
		double Z,
		double thresh,
		double *res
		)
  {
    std::vector<double> scratch(4, 0.0);
    const double Pthresh= Z*thresh;
    for(size_t i=0; i<4; ++i)
      res[i]=0;

    for(std::list<Minim::WPPoint>::const_iterator i=l.begin();
	i!= l.end();
	++i)
    {
      const double w=i->w *exp(- i->ll);
      if (w > Pthresh)
      {
	md.put(i->p);
	model.dTdL_ND(scratch);
	for(size_t j=0; j<scratch.size(); ++j)
	{
	  res[j]+=std::pow(scratch[j]-m1[j],2)*w;
	}
      }
    }
    for(size_t j=0; j<scratch.size(); ++j)
    res[j]/=Z;
  }

  void dTdL2_ND(WVRAtmoQuantModel &m,
		std::vector<double> &res)
  {
    using namespace boost::lambda;
    Minim::ModelDesc md(m);
    boost::array< boost::array<double, 4> ,3> r=
      CenFDV<3,4>(bind(&WVRAtmoQuantModel::evalFn, 
		       &m, 
		       _1, 
		       "n"),
		  *md["n"]->p,
		  0.001);
    const double cv=std::pow(SW_WaterToPath_Simplified(1.0, 
						       *md["T"]->p),
			     -2);
    res=std::vector<double>(r[2].begin(), r[2].end());
    BOOST_FOREACH(double &x, res)
      x*=cv;
		  
  }

  void dTdL3_ND(WVRAtmoQuantModel &m,
		std::vector<double> &res)
  {
    using namespace boost::lambda;
    Minim::ModelDesc md(m);
    boost::array< boost::array<double, 4> ,4> r=
      CenFDV<4,4>(bind(&WVRAtmoQuantModel::evalFn, 
		       &m, 
		       _1, 
		       "n"),
		  *md["n"]->p,
		  0.001);
    const double cv=std::pow(SW_WaterToPath_Simplified(1.0, 
						       *md["T"]->p),
			     -3);
    res=std::vector<double>(r[3].begin(), r[3].end());
    BOOST_FOREACH(double &x, res)
      x*=cv;
  }

  void dTdTAtm(WVRAtmoQuantModel &m,
	       std::vector<double> &res)
  {
    using namespace boost::lambda;
    Minim::ModelDesc md(m);
    boost::array< boost::array<double, 4> ,2> r=
      CenFDV<2,4>(bind(&WVRAtmoQuantModel::evalFn, 
		       &m, 
		       _1, 
		       "T"),
		  *md["T"]->p,
		  0.01);
    res=std::vector<double>(r[1].begin(), r[1].end());
  }

}



/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2009
   Maintained by ESO since 2013.

*/

#include "almaabs.hpp"

#include <iostream>
#include <stdio.h>            /*** for sprintf(...) ***/
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#include "almaabs_i.hpp"
#include "almaopts.hpp"
#include "arraydata.hpp"
#include "dtdlcoeffs.hpp"
#include "segmentation.hpp"
#include "../radiometermeasure.hpp"
#include "../dtdltools.hpp"

#include "../model_make.hpp"
#include "almawvr/nestedsampler.hxx"
#include "almawvr/nestedinitial.hxx"


#ifdef ELLIPSESAMPLE
#include "almawvr/prior_sampler.hxx"
#include "almawvr/mcmonitor.hxx"
#endif

namespace LibAIR2 {

  ALMAAbsRet::ALMAAbsRet(const std::vector<double> &TObs,
			 double el,
			 const ALMAWVRCharacter &WVRChar):
    i(new iALMAAbsRet(TObs,
		      el,
		      WVRChar))
  {
    i->sample();
  }

  ALMAAbsRet::~ALMAAbsRet()
  {
  }

  void ALMAAbsRet::g_Res(ALMAResBase &res)
  {
    i->g_Pars(res);
    i->g_Coeffs(res);
  }

  std::ostream & operator<<(std::ostream &os, 
			    const  ALMAAbsInput &i)
  {
    os<<i.time<<"\t"
      <<i.antno<<"\t"
      <<"["<<boost::format("%5.2f, ")% i.TObs[0]
           <<boost::format("%5.2f, ")% i.TObs[1]
           <<boost::format("%5.2f, ")% i.TObs[2]
           <<boost::format("%5.2f]")% i.TObs[3]
      <<"\t"
      <<i.el
      <<"\t"
      <<i.state;
    return os;
  }

  std::ostream & operator<<(std::ostream &os, 
			    const  ALMAAbsInpL &i)
  {
    os<<"     Input WVR data information:   "<<std::endl
      <<"-----------------------------------"<<std::endl;    
    os<<"Timestamp"<<"\t"
      <<"Ant. #"<<"\t"
      <<"["<<"Chn 0"<<", "
           <<"Chn 1"<<", "
           <<"Chn 2"<<", "
           <<"Chn 3"<<"]"
      <<"\t"
      <<"Elevation"
      <<"\t"
      <<"STATE_ID"<<std::endl;
    BOOST_FOREACH(const ALMAAbsInput &x, i)
      os<<x<<std::endl;
    return os;
  }

  /** \brief Check that the supplied temperatures are feasible
   */
  static void checkTObs(const std::vector<double>  &TObs)
  {
    int above = 0;
    int below = 0;
    for(size_t i=0; i<TObs.size(); ++i)
    {
      if (TObs[i]<2.7){
	below++;
      }
      else if (TObs[i]>350.){
	above++;
      }
    }
    if (above>0 || below>0){
      char tstr[120];                                                                                       
      sprintf(tstr, "Values out of range: Out of %d TObs values, %d were below 2.7 K and %d were above 350 K.", 
	      TObs.size(), below, above); 
      throw std::runtime_error(tstr);
    }
  }
  
  static void getMidPointData(const InterpArrayData &d,
			      std::vector<double>  &TObs,
			      double &el,
			      double &time,
			      size_t &state)
  {
    TObs.resize(4);
    const size_t midpoint=static_cast<size_t>(d.g_time().size() *0.5);
    for(size_t k=0; k<4; ++k)
    {
      TObs[k]=d.g_wvrdata()[midpoint][0][k];
    }
    checkTObs(TObs);    
    el=d.g_el()[midpoint];
    time=d.g_time()[midpoint];
    state=d.g_state()[midpoint];
  }

  // Convert to units of K/meter
  static void convertKm(std::vector<double> &dTdL)
  {
    for(size_t i=0; i<4; ++i)
    {
      dTdL[i]*=1e3;
    }
  }
  ALMAAbsInpL SimpleSingleI(const InterpArrayData &d)
  {
    ALMAAbsInpL res;
    ALMAAbsInput a;

    std::vector<double>  TObs(4);
    double el, time;
    size_t state;
    getMidPointData(d, TObs, 
		    el, time,
		    state);    

    a.antno=0;
    for(size_t i=0; i<4; ++i)
      a.TObs[i]=TObs[i];
    a.el=el;
    a.time=time;
    a.state=state;
    res.push_back(a);
    return res;
  }

  ALMAAbsInpL MultipleUniformI(const InterpArrayData &d,
			       size_t n,
			       const std::set<size_t> &states)
  {
    ALMAAbsInpL res;
    const size_t nrows=d.g_time().size();
    const size_t ndelta=nrows / (n+1);
    for(size_t i=0; i<n; ++i)
    {
      ALMAAbsInput a;
      size_t row=ndelta*(i+1);
      while (row<(nrows-1) and states.count(d.g_state()[row])==0)
      {
	++row;
      }
      if (states.count(d.g_state()[row]) == 0)
      {
	throw std::runtime_error("Could not find a row with a sky state");
      }      
      a.antno=0;
      for(size_t k=0; k<4; ++k)
	a.TObs[k]=d.g_wvrdata()[row][0][k];
      a.el=d.g_el()[row];
      a.time=d.g_time()[row];
      a.state=d.g_state()[row];
      res.push_back(a);
    }
    return res;
  }

  ALMAAbsInpL FieldMidPointI(const InterpArrayData &d,
			     const std::vector<double> &time,
			     const std::vector<std::pair<double, double> >  &fb,
			     const std::set<size_t> &states)
  {
    ALMAAbsInpL res;

    const size_t nrows=d.g_time().size();
    size_t row=0;
    for(size_t i=0; i<fb.size(); ++i)
    {
      ALMAAbsInput a;
      const double ftime=fb[i].first;
      const double ltime=fb[i].second;
      const double mtime=(ftime+ltime)/2;
      
      while (row<(nrows-1) and d.g_time()[row] < mtime)
      {
	++row;
      }

      // Now find the next row with a good state
      while (row<(nrows-1) and states.count(d.g_state()[row])==0)
      {
	++row;
      }
      if (states.count(d.g_state()[row]) == 0)
      {
	throw std::runtime_error("Could not find a row with a sky state");
      }
      a.antno=0;
      for(size_t k=0; k<4; ++k)
      {
	a.TObs[k]=d.g_wvrdata()[row][0][k];
      }
      a.el=d.g_el()[row];
      a.time=d.g_time()[row];
      a.state=d.g_state()[row];
      a.source=d.g_source()[row];
      res.push_back(a);
    }
    return res;
  }

  dTdLCoeffsBase * 
  SimpleSingle(const InterpArrayData &d)
  {
    std::vector<double>  TObs(4);
    double el, time;
    size_t state;
    getMidPointData(d, TObs, el, time, state);

    ALMAWVRCharacter wvrchar;
    ALMAAbsRet ar(TObs, 
		  M_PI/2.0,  
		  wvrchar);
    ALMAResBase res;
    ar.g_Res(res);

    // Convert to units of K/meter
    std::vector<double> dTdL(4);
    std::vector<double> dTdL_err(4);
    for(size_t i=0; i<4; ++i)
    {
      dTdL[i]=res.dTdL[i];
      dTdL_err[i]=res.dTdL_err[i];
    }

    convertKm(dTdL);
    convertKm(dTdL_err);

    return new dTdLCoeffsSingle(dTdL,
				dTdL_err
				);
    
  }

  // There is too much repetition here, need to merge with other
  // functions here
  dTdLCoeffsBase * 
  SimpleSingleCont(const InterpArrayData &d)
  {

    std::vector<double>  TObs(4);
    double el, time;
    size_t state;
    getMidPointData(d, TObs, el, time, state);

    LibAIR2::ALMARetOpts opts;
    LibAIR2::ALMAContRes res;
    ALMAWVRCharacter wvrchar;

    ALMAAbsContRetrieve(TObs,
			el,
			wvrchar,
			res,
			opts);

    // Convert to units of K/meter
    std::vector<double> dTdL(4);
    std::vector<double> dTdL_err(4);
    for(size_t i=0; i<4; ++i)
    {
      dTdL[i]=res.dTdL[i];
      dTdL_err[i]=res.dTdL_err[i];
    }

    convertKm(dTdL);
    convertKm(dTdL_err);

    return new dTdLCoeffsSingle(dTdL,
				dTdL_err);
    
  }

  boost::ptr_list<ALMAResBase> doALMAAbsRet(const ALMAAbsInpL &il, std::vector<int>& problemAnts)
  {

    problemAnts.resize(0);
    boost::ptr_list<ALMAResBase> res;

    BOOST_FOREACH(const ALMAAbsInput &x, il)
    {
      std::vector<double>  TObs(4);
      for(size_t i=0; i<4; ++i)
        TObs[i]=x.TObs[i];
      try {
	checkTObs(TObs);
      }
      catch(const std::runtime_error rE){
	std::cout << std::endl << "WARNING: problem with Tobs of antenna " << x.antno
		  << std::endl << "         LibAIR2::checkTObs: " << rE.what() << std::endl;
	std::cerr << std::endl << "WARNING: problem with Tobs of antenna " << x.antno
		  << std::endl << "         LibAIR2::checkTObs: " << rE.what() << std::endl;
	problemAnts.push_back(x.antno);
      }
      ALMAWVRCharacter wvrchar;
      ALMAAbsRet ar(TObs, 
		    x.el,  
		    wvrchar);
      ALMAResBase *ares=new ALMAResBase;      
      ar.g_Res(*ares);
      res.push_back(ares);
    }
    return res;
  }

  static void ALMAAbsRetP(ALMAResBase *i,
			  boost::array<double, 4> &dTdL,
			  boost::array<double, 4> &dTdL_err)
  {
    for(size_t k=0; k<4; ++k)
    {
      dTdL[k]= i->dTdL[k]*1e3;
      dTdL_err[k]= i->dTdL_err[k]*1e3;
    }
  }

  dTdLCoeffsBase *
  ALMAAbsProcessor(const ALMAAbsInpL &inp,
		   boost::ptr_list<ALMAResBase> &r)
  {
    std::auto_ptr<dTdLCoeffsBase> res;
    if (inp.size()==0)
    {
    }
    else
    {
      dTdLCoeffsSingleInterpolated *rr=new dTdLCoeffsSingleInterpolated();
      res=std::auto_ptr<dTdLCoeffsBase>(rr);

      for(ALMAAbsInpL::const_iterator i=inp.begin(); 
	  i!=inp.end(); 
	  ++i)
      {
	ALMAResBase *rp=r.release(r.begin()).release();
	boost::array<double, 4> dTdL;
	boost::array<double, 4> dTdL_err;
	ALMAAbsRetP(rp, dTdL, dTdL_err);
	rr->insert(i->time,
		   dTdL,
		   dTdL_err);
      }
      
    }
    return res.release();
  }


  dTdLCoeffsSingleInterpolated *
  SimpleMultiple(const InterpArrayData &d,
		 const std::vector<double> &time,
		 const std::vector<std::pair<double, double> > &fb,
		 boost::ptr_list<ALMAResBase> &r)
  {

    BOOST_ASSERT(fb.size()==r.size());
    
    std::auto_ptr<dTdLCoeffsSingleInterpolated> 
      res(new dTdLCoeffsSingleInterpolated());

    boost::ptr_list<ALMAResBase>::iterator ir=r.begin();
    for(size_t i=0; i<fb.size(); ++i)
    {
    
      // Convert to units of K/meter
      boost::array<double, 4> dTdL;
      boost::array<double, 4> dTdL_err;
      ALMAAbsRetP(&(*ir), dTdL, dTdL_err);
      res->insert(fb[i].first,
		  dTdL,
		  dTdL_err);
      res->insert(fb[i].second,
		  dTdL,
		  dTdL_err);
      ++ir;
    }
    
    return res.release();
    
  }

  void ALMAAbsContRetrieve(const std::vector<double> &TObs,
			   double el,
			   const ALMAWVRCharacter &WVRChar,
			   ALMAContRes &res,
			   const ALMARetOpts &opts)
  {
    CouplingModel *cm= new CouplingModel(mkCloudy(WVRChar,
						  PartTable,
						  AirCont));
    PPDipModel m(cm);
    AbsNormMeasure *ll=new AbsNormMeasure(m);
    Minim::IndependentFlatPriors pll(ll);
    
    cm->setSpill(0.98, 275);
    m.setZA(0.5 * M_PI -el);
    ll->obs=TObs;
    ll->thermNoise=std::vector<double>(TObs.size(),
				       1.0);
    
    if (opts.OSFPriors)
    {
      pll.AddPrior("n", 0, 15);
      pll.AddPrior("T", 250, 295);
      pll.AddPrior("P", 550, 750);
      pll.AddPrior("tau183", 0, 0.2);
    }
    else
    {
      pll.AddPrior("n", 0, 10);
      pll.AddPrior("T", 250, 295);
      pll.AddPrior("P", 300, 550);
      pll.AddPrior("tau183", 0, 0.2);
    }


    std::list<Minim::MCPoint> ss;
    startSetDirect(pll,
		   200,
		   ss);

    // Create the nested sampler
    boost::scoped_ptr<Minim::NestedS> ns;
    ns.reset(new Minim::NestedS(pll));
    (*ns)["coupling"]->dofit=false;

#ifdef ELLIPSESAMPLE    
    Minim::EllipsoidCPSampler *cps=new Minim::EllipsoidCPSampler(pll,
								 *ns);    
    cps->reshape_maxp=50;

    ns->reset(ss,
	      cps);
    cps->reshape();
#else
    ns->reset(ss);
#endif

#if 0
    Minim::SOutMCMon *pp=new Minim::SOutMCMon();
    ns->mon=pp;
    cps->mon=pp;
#endif 
    //ns->InitalS(new Minim::InitialRandom(200));

    double evidence=ns->sample(3000);
    /// The posterior 
    std::list<Minim::WPPoint> post;
    post=ns->g_post();

    if (post.size() < 10000 )
    {
      std::cout<<"Terminated after "<<post.size()
	       <<std::endl;
    }

    res.ev=evidence;

    std::vector<double> m1(4), m2(4);
    moment1(post,
	    evidence,
	    m1);

    moment2(post,
	    m1,
	    evidence,
	    m2);

    res.c=m1[0];
    res.c_err=std::pow(m2[0], 0.5);

    res.tau183=m1[3];
    res.tau183_err=std::pow(m2[3], 0.5);

    dTdLMom1(post,
	     *ns,
	     m,
	     evidence,
	     1e-10,
	     res.dTdL);

    dTdLMom2(post,
	     *ns,
	     m,
	     res.dTdL,
	     evidence,
	     1e-10,
	     res.dTdL_err);

  }

}



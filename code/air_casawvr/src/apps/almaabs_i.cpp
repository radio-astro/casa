/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version November 2009
   Maintained by ESO since 2013.

   \file almaabs_i.cpp
   
*/
#include <iostream>

#include "almaabs_i.hpp"

#include "../model_make.hpp"
#include "../dtdltools.hpp"
#include "bnmin1/src/nestedinitial.hxx"


namespace LibAIR2 {

  const double iALMAAbsRetLL::thermNoise=1.0;
  const size_t iALMAAbsRet::n_ss=200;

  iALMAAbsRetLL::iALMAAbsRetLL(const std::vector<double> &TObs,
			       double el,
			       const ALMAWVRCharacter &WVRChar):
    cm(new CouplingModel(mkSingleLayerWater(WVRChar,
					    PartTable,
					    AirCont))),
    m(cm),
    ll(new AbsNormMeasure(m))
  {
    cm->setSpill(0.98, 275);
    
    m.setZA(0.5 * M_PI -el);
    ll->obs=TObs;
    ll->thermNoise=std::vector<double>(TObs.size(),
				       thermNoise);

  }

  iALMAAbsRet::iALMAAbsRet(const std::vector<double> &TObs,
			   double el,
			   const ALMAWVRCharacter &WVRChar):
    ls(TObs, 
       el, 
       WVRChar),
    pll(ls.ll),
    evidence()
  {
    pll.AddPrior("n", 0, 10);
    pll.AddPrior("T", 250, 295);
    pll.AddPrior("P", 300, 550);
  }

  bool iALMAAbsRet::sample(void)
  {
    // Create starting set
    std::list<Minim::MCPoint> ss;
    startSetDirect(pll,
		   n_ss,
		   ss);

    // Create the nested sampler
    ns.reset(new Minim::NestedS(pll));
    (*ns)["coupling"]->dofit=false;
    ns->reset(ss);

    // So far not obvious it is necessary to enable this
    //ns->InitalS(new Minim::InitialRandom(n_ss));

    evidence=ns->sample(10000);
    post=ns->g_post();

    if (post.size() < 10000 )
    {
      std::cout<<"Terminated after "<<post.size()<<std::endl;
    }
    if(evidence == 0.){
      return false;
    }
    return true;

  }

  void iALMAAbsRet::g_Pars(ALMAResBase &res)
  {
    res.ev=evidence;

    std::vector<double> m1(3), m2(3);
    moment1(post,
	    evidence,
	    m1);

    moment2(post,
	    m1,
	    evidence,
	    m2);

    res.c=m1[0];
    res.c_err=std::pow(m2[0], 0.5);
  }

  void iALMAAbsRet::g_Coeffs(ALMAResBase &r)
  {
    r.ev=evidence;
    dTdLMom1(post,
	     *ns,
	     ls.m,
	     evidence,
	     1e-10,
	     r.dTdL);

    dTdLMom2(post,
	     *ns,
	     ls.m,
	     r.dTdL,
	     evidence,
	     1e-10,
	     r.dTdL_err);
  }


}




//
// CASA - Common Astronomy Software Applications
// Copyright (C) 2015
// Associated Universities, Inc. Washington DC, USA.
// Copyright by ESO (in the framework of the ALMA collaboration).
//
// This file is part of CASA.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//



#include <iostream>
#include <limits>
#include <set>

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include "../src/random_normal_distribution.hxx"
#include "../src/random_variate_generator.hxx"
#include <boost/random/uniform_real.hpp>

#include <casa/aips.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>

#include "../src/minimmodel.hxx"
#include "../src/tests/quadmodel.hpp"
#include "../src/apps/gaussmodel.hxx"
#include "../src/lmmin.hxx"
#include "../src/monitor.hxx"
#include "../src/minimio.hxx"
#include "../src/metropolis.hxx"
#include "../src/priors.hxx"
#include "../src/gradientminim.hxx"
#include "../src/robustline.hxx"
#include "../src/twoerrline_ml.hxx"
#include "../src/nestedsampler.hxx"
#include "../src/mcmonitor.hxx"

#include "../src/tests/nestedgauss.hpp"

#include "../config.h"

#include "../src/metro_propose.hxx"
#include "../src/pda.hxx"

#include "../src/mcpoint.hxx"
#include "../src/nestederr.hxx"
#include "../src/prior_sampler.hxx"
#include "../src/markovchain.hxx"


// Unit test for the bnmin1 module

using namespace casa;

QuadObs * mkStdObs(void)
{
  using namespace Minim;

  std::vector<double> x(3);
  x[0]=-1; x[1]=0; x[2]=1;
  
  std::vector<double> obs(3);
  QuadModel qm;
  qm.a=1; qm.b=2; qm.c=3;
  qm.eval(x, obs);
  
  return new QuadObs( x,obs);

}


void QuadT1()
{
  using namespace Minim;

  boost::scoped_ptr<QuadObs> qo(mkStdObs());

  std::vector<double> scratch(3);
  qo->residuals(scratch);

  LMMin minimiser(*qo);
  minimiser.ftol=minimiser.xtol=minimiser.gtol=1e-5;
  ChiSqMonitor mon;
  minimiser.AddMon(&mon);
  minimiser.solve();

  PrettyPrint(minimiser);
  AlwaysAssertExit( qo->qm.a == 1);

  AlwaysAssertExit(near( qo->qm.b,
	       2.0,
	       1e-12));

  AlwaysAssertExit( qo->qm.c == 3);

}

void RecPars( const QuadModel & qm,
	      std::vector<double> &x)
{
  x.resize(1);
  x[0]=qm.a;
}

void QuadMetro()
{
  using namespace Minim;

  const double params[]= { 1, 2, 3};

  std::vector<double> x(3);
  x[0]=-1; x[1]=0; x[2]=1;
  
  std::vector<double> obs(3);
  QuadModel qm;
  qm.a=params[0]; 
  qm.b=params[1]; 
  qm.c=params[2];
  qm.eval(x, obs);
  
  QuadObs qo ( x,obs);

  std::vector<double> scratch(3);
  qo.residuals(scratch);
  qo.sigma=0.01;

  std::vector<double> sigmas(3,0.1);

  MetropolisMCMC metro(qo,sigmas);
  metro.f=boost::bind( RecPars, qm, _1);
  
  boost::shared_ptr< std::list<Minim::MCPoint>  >
    res( metro.sample(10000)) ;

  for (size_t i = 0 ; i < sigmas.size(); ++i)
  {
    AlwaysAssertExit(near( res->back().p[i],
		 params[i],
		 1E-2));
  }

  AlwaysAssertExit(near( res->back().fval[0],
	       params[0],
	       1E-2));



}

void QuadMetro_Seq()
{
  using namespace Minim;

  const double params[]= { 1, 2, 3};

  std::vector<double> x(3);
  x[0]=-1; x[1]=0; x[2]=1;
  
  std::vector<double> obs(3);
  QuadModel qm;
  qm.a=params[0]; 
  qm.b=params[1]; 
  qm.c=params[2];
  qm.eval(x, obs);
  
  QuadObs qo ( x,obs);

  std::vector<double> scratch(3);
  qo.residuals(scratch);
  qo.sigma=0.01;

  std::vector<double> sigmas(3,0.1);

  MetropolisMCMC metro(qo,sigmas,33, MetropolisMCMC::Sequence);
  metro.f=boost::bind( RecPars, qm, _1);
  
  boost::shared_ptr< std::list<Minim::MCPoint>  >
    res( metro.sample(30000)) ;

  for (size_t i = 0 ; i < sigmas.size(); ++i)
  {
    AlwaysAssertExit(near( res->back().p[i],
		 params[i],
		 1E-2));
  }

  AlwaysAssertExit(near( res->back().fval[0],
	       params[0],
	       1E-2));



}

void GaussObs_Metro()
{
  using namespace Minim;
  
  GaussObs go(3);
  go.p[0]=10; 
  go.p[1]=0;
  go.p[2]=0;
  std::vector<double> sigmas(3,0.1);
  
  MetropolisMCMC metro(go,sigmas);  

  boost::shared_ptr< std::list<Minim::MCPoint>  >
    res( metro.sample(10000)) ;

  for (size_t i = 0; i < sigmas.size(); ++i)
  {
    AlwaysAssertExit(fabs(res->back().p[i]) < 3);
  }  
  


}
void Params_ByName()
{
  using namespace Minim;

  QuadModel qm;
  ModelDesc md(qm);
  
  AlwaysAssertExit( md["a"]->p == &qm.a);
  AlwaysAssertExit( md["c"]->p == &qm.c);
}

void QuadPrior()
{
  using namespace Minim;
  using namespace boost::assign;

  const double params[]= { 1, 2, 3};

  // Abcissa values at which the quardratic function is "observed"
  std::vector<double> x;
  x +=  -1, 0 , 1;

  // Corresponding "observation"
  std::vector<double> obs(3);

  QuadModel qm;
  qm.a=params[0]; 
  qm.b=params[1]; 
  qm.c=params[2];
  qm.eval(x, obs);
  
  std::auto_ptr<QuadObs> qo (new QuadObs( x,obs));

  std::vector<double> scratch(3);
  qo->residuals(scratch);
  qo->sigma=0.01;

  IndependentFlatPriors qifp( qo.release() );
  std::vector<double> sigmas(3,0.1);

  qifp.AddPrior("a", -10,10);
  qifp.AddPrior("b", -10,10);
  qifp.AddPrior("c", -10,2);

  MetropolisMCMC metro(qifp,sigmas);
  boost::shared_ptr< std::list<Minim::MCPoint>  >
    res( metro.sample(10000)) ;

  AlwaysAssertExit(near( res->back().p[2],
	       2.0,
	       1E-2));

}


void BFGS2Minim_QuadRes()
{
  using namespace Minim;

  std::vector<double> x(3);
  x[0]=-1; x[1]=0; x[2]=1;
  
  std::vector<double> obs(3);
  QuadModel qm;
  qm.a=1; qm.b=2; qm.c=3;
  qm.eval(x, obs);

  AlwaysAssertExit(obs[2] != 0);  

  std::vector<double> g(3);
  qm.grad(0, g);
  AlwaysAssertExit(g[0] == 1);
  AlwaysAssertExit(g[1] == 0);

  
  QuadGrad qg(x,obs);
  BFGS2Minim m(qg);

  AlwaysAssertExit(qg.lLikely() > 0);
  std::vector<double> scratch;
  qg.lGrd(scratch);


  AlwaysAssertExit(scratch.size() == 3);  
  AlwaysAssertExit(scratch[2] != 0);  
  AlwaysAssertExit(scratch[0] != 0);  
  AlwaysAssertExit(scratch[1] != 0);  

  m.solve();

  AlwaysAssertExit(near(qg.qm.a,
	      1.0,
	      1e-5));

  AlwaysAssertExit(near(qg.qm.b,
	      2.0,
	      1e-5));

  AlwaysAssertExit(near(qg.qm.c,
	      3.0,
	      1e-5));

}


void t_RobustLineObsMod()
{
  
  std::vector<double> xvals = boost::assign::list_of(1)(2)(3)(4)(5)(6)(7)(8)(9)(10).convert_to_container<std::vector<double> >();  
  std::vector<double> yvals = boost::assign::list_of(1)(2)(3)(4)(5)(6)(7)(80)(9)(10).convert_to_container<std::vector<double> >();  

  Minim::RobustLineObsMod rom(xvals, yvals);
  rom.a=0;
  rom.b=0;

  AlwaysAssertExit(near(rom.lLikely(),
	      127.0,
	      1e-12));  

  std::vector<double> grad;
  rom.lGrd(grad);
  AlwaysAssertExit(near(grad[1],
	      -10.0,
	      1e-12));  

  AlwaysAssertExit(near(grad[0],
	      -55.0,
	      1e-12));  
  

  for(size_t i=0; i<1; ++i)
  {
    rom.a=0;
    rom.b=0;  
    Minim::BFGS2Minim m(rom);
    m.solve();
  }

  
  AlwaysAssertExit(near(rom.a,
	      1.0,
	      5E-2));  

  AlwaysAssertExit(near(rom.b,
	      -0.04,
	      5E-2));  

  AlwaysAssertExit(near(rom.lLikely(),
	      72.0,
	      1E-2));  
  

}

void t_LineTwoErrML()
{

  /**
  std::vector<double> x(10), obs(10);
  for (size_t i=0; i <x.size(); ++i)
  {
    x[i]=i;
    obs[i]=i;
  }
  */
  std::vector<double> x=boost::assign::list_of(1)(2).convert_to_container<std::vector<double> >();
  std::vector<double> obs=boost::assign::list_of(1)(2).convert_to_container<std::vector<double> >();

  Minim::LineTwoErrML lml(x, obs,
			  1.0, 1.0);
  lml.a=1.0;
  lml.b=1.0;

  Minim::BFGS2Minim m(lml);
  m.solve();
  
  AlwaysAssertExit(near(lml.a, 1.0, 1e-4));  
  AlwaysAssertExit(near(1+lml.b, 1.0, 1e-4));  
  
}

void t_LineTwoErr_LavMarq()
{
  std::vector<double> x=boost::assign::list_of(1)(2).convert_to_container<std::vector<double> >();
  std::vector<double> obs=boost::assign::list_of(1)(2).convert_to_container<std::vector<double> >();

  Minim::LineTwoErr_LavMarq lml(x, obs,
				1.0, 1.0);
  Minim::LMMin m(lml);
  m.ftol=m.xtol=m.gtol=1e-5;
  Minim::ChiSqMonitor mon;
  m.AddMon(&mon);
  m.solve();

  const double ra=m.getbyname("a")->getp();
  AlwaysAssertExit(near(ra, 1.0, 1e-4));  
  AlwaysAssertExit(near(1+m.getbyname("b")->getp(), 1.0, 1e-4));  
  
}



void t_NestedSampling_Gauss()
{  
  const double l_sigma=1.0;

  const double res=getEvidence(l_sigma,
			       150,
			       false);

  AlwaysAssertExit(near(res, 
	      // Note pre-factor 8 cancels with 1/2 inside power
	      1.0/8* pow(erf(1.0/l_sigma/sqrt(2)),3) ,
	      0.1));
}

void t_NestedSampling()
{  
  using namespace Minim;
  IndependentFlatPriors obs(mkStdObs());
  
  std::list<Minim::MCPoint> startset;
  Minim::MCPoint p; 

  p.p=boost::assign::list_of(0)(0)(0).convert_to_container<std::vector<double> >();
  startset.push_back(p);

  p.p=boost::assign::list_of(2)(0)(0).convert_to_container<std::vector<double> >();
  startset.push_back(p);

  p.p=boost::assign::list_of(0)(1)(5).convert_to_container<std::vector<double> >();
  startset.push_back(p);

  p.p=boost::assign::list_of(3)(1)(10).convert_to_container<std::vector<double> >();
  startset.push_back(p);

  p.p=boost::assign::list_of(-3)(2)(10).convert_to_container<std::vector<double> >();
  startset.push_back(p);
  
  NestedS s(obs,
	    startset);
  
  const double res=s.sample(30);

  //AlwaysAssertExit(near(res,0.0, 0.1));
	  
}

void MetroPropose_raccept()
{
  
  std::vector<double> sigmas(0);
  //  sigmas= boost::assign::list_of(1)(1)(1)(1).convert_to_container<std::vector<double> >();
  Minim::MetroPropose mp(sigmas, 1);

  for (size_t i =0 ; i < 100 ; ++i)
  {
    AlwaysAssertExit( mp.raccept() >=0 );
    AlwaysAssertExit( mp.raccept() <=1 );
  }
}

void MetroProposeSeq_displace()
{
  
  std::vector<double> sigmas(4,1.0);
  {
    Minim::MetroPropose mp(sigmas, 1);
    std::vector<double> x(4,0);
    mp.displace(x); mp.displace(x); 
    AlwaysAssertExit( x[2] !=0);
    AlwaysAssertExit( x[3]!=0);
  }


  {
    Minim::MetroProposeSeq mps(sigmas, 1);
    std::vector<double> x(4,0);
    mps.displace(x); mps.displace(x); 
    AlwaysAssertExit( x[2] == 0);
    AlwaysAssertExit( x[3] == 0);
  }

}

void FlatPrior_NoParam()
{
  std::vector<double> dummy;
  Minim::IndependentFlatPriors pr(new QuadObs(dummy,dummy));

  AlwaysAssertExit(pr.llprob() == pr.lLikely());

  pr.AddPrior("c", 0,1);

  Minim::ModelDesc md(pr);
  md["c"]->setp(0.5);
  
  AlwaysAssertExit(pr.pprob() == 0);

  md["c"]->setp(2);
  
  AlwaysAssertExit(pr.pprob()> 1e5);
  
  Bool thrown = False;
  try{
    pr.AddPrior("d", 0,1);
  }
  catch(...){
    thrown = True;
  }
  AlwaysAssertExit(thrown);
  
}



void PDA_D1MACH()
{
  
#ifndef BNMIN1_NO_PDA  

  int i=3;
  double x = pda_d1mach__(&i);
  // Curiously this is a factor of two smaller than expected...
  AlwaysAssertExit(x*2 == std::numeric_limits<double>::epsilon());

#endif
		    
}


void BFGS2Minim_constructs()
{
  std::vector<double> x(5,1.0);
  std::vector<double> obs(5,2.0);
  
  QuadGrad qg(x,obs);
  Minim::BFGS2Minim m(qg);

}

void LineTwoErrML_basics()
{

  std::vector<double> x(5,1.0);
  std::vector<double> obs(5,2.0);
  
  Minim::LineTwoErrML lml(x, obs,
			  0.1, 0.1);

  lml.a=0; lml.b=2;
  AlwaysAssertExit(lml.lLikely() == 0);
  
}

void MCPoint_Less()
{
  Minim::MCPoint a,b;
  a.ll=10; b.ll=20;
  
  AlwaysAssertExit( a<b);
  a.ll=30;
  AlwaysAssertExit( b<a);

  std::set<Minim::MCPoint> ps;
  ps.insert(a);
  ps.insert(b);
  
  AlwaysAssertExit(ps.begin()->ll == 20);
}

void llPoint_Wrks()
{
  std::vector<double> dummy;
  Minim::IndependentFlatPriors pr(new QuadObs(dummy,
					      dummy));
  pr.AddPrior("c", 0,1);


  Minim::MCPoint p1; 
  p1.p=boost::assign::list_of(0.5)(0.5)(0.5).convert_to_container<std::vector<double> >();

  Minim::MCPoint p2; 
  p2.p=boost::assign::list_of(0.5)(0.5)(0.5).convert_to_container<std::vector<double> >();

  std::list<Minim::MCPoint> lp;
  lp.push_back(p1); 
  lp.push_back(p2);

  std::set<Minim::MCPoint> res;
  
  llPoint(pr, lp, res);

  AlwaysAssertExit(res.begin()->ll == 0);
  
}

void prior_sampler_constr()
{
  std::vector<double> dummy;
  Minim::IndependentFlatPriors pr(new QuadObs(dummy,
					      dummy));
  std::vector<double> sigs(3, 0.1);
  Minim::ModelDesc md(pr);
  Minim::CSPMetro a(pr, md,
		    sigs);
		
}

void SOutMCMon_tprint()
{
  Minim::SOutMCMon t;
  Minim::MCPoint p;
  p.p.push_back(10);
  t.accept(p);
}

void t_startSetDirect()
{
  std::vector<double> dummy;
  Minim::IndependentFlatPriors pr(new QuadObs(dummy,
					      dummy));
  pr.AddPrior("c", 0,1);
  pr.AddPrior("a", 0,1);
  pr.AddPrior("b", 0,1);
  
  std::list<Minim::MCPoint> s;

  startSetDirect(pr,
		 100,
		 s);

  for(std::list<Minim::MCPoint>::const_iterator ip=s.begin(); ip!=s.end(); ip++)
  {
    AlwaysAssertExit(ip->p[0] > 0 && ip->p[0] < 1 );
  }
		
}

void t_WPPoint_construct()
{
  std::vector<double> dummy;
  Minim::WPPoint pp(dummy, 2.0);

  AlwaysAssertExit(pp.w == 2.0);

  Minim::WPPoint pp2(pp, 3.0);

  AlwaysAssertExit(pp2.w == 3.0);
		
}

void metropolis_accept()
{
  using namespace Minim;
  MCPoint2 c,p;
  c.l=10;
  p.l=9;
  
  AlwaysAssertExit(metropolis(c,p) == 1.0);

  p.l=1000;
  AlwaysAssertExit(metropolis(c,p) == 0.0);

}

void normal() {
  unsigned seed = 42;
  boost::mt19937 rng(seed);
  bnmin1boost::normal_distribution<double> distro(0., 1.);
  bnmin1boost::variate_generator<boost::mt19937, bnmin1boost::normal_distribution<double> > norm(rng, distro);
  std::cout << "normal" << std::endl;
  std::cout << distro.mean() << " " << distro.sigma() << std::endl;
  for (int i=0; i<10; ++i) {
    std::cout << norm() << std::endl;
  }
}

void uniform() {
  unsigned seed = 42;
  boost::mt19937 rng(seed);
  boost::uniform_real<double> distro(0., 1.);
  std::cout << "uniform" << std::endl;
  for (int i=0; i<10; ++i) {
    std::cout << distro(rng) << std::endl;
  }
}

int main()
{
  using namespace Minim;
  Model m;

  // test random generation itself
  normal();
  uniform();

  //std::cout << "QuadT1" << std::endl;
  //QuadT1();
  std::cout << "QuadMetro" << std::endl;
  QuadMetro();
  std::cout << "QuadMetro_Seq" << std::endl;
  QuadMetro_Seq();
  std::cout << "QuadMetro_Seq" << std::endl;
  GaussObs_Metro();
  std::cout << "Params_ByName" << std::endl;
  Params_ByName();
  std::cout << "QuadPrior" << std::endl;
  QuadPrior();
  std::cout << "BFGS2Minim_QuadRes" << std::endl;
  BFGS2Minim_QuadRes();
  std::cout << "t_RobustLineObsMod" << std::endl;
  t_RobustLineObsMod();
  std::cout << "t_LineTwoErrML" << std::endl;
  t_LineTwoErrML();
  //std::cout << "t_LineTwoErr_LavMarq" << std::endl;
  //t_LineTwoErr_LavMarq();
  std::cout << "t_NestedSampling_Gauss" << std::endl;
  t_NestedSampling_Gauss();
  std::cout << "t_NestedSampling" << std::endl;
  t_NestedSampling();

  std::cout << "MetroPropose_raccept" << std::endl;
  MetroPropose_raccept();
  std::cout << "MetroProposeSeq_displace" << std::endl;
  MetroProposeSeq_displace();
  std::cout << "FlatPrior_NoParam" << std::endl;
  FlatPrior_NoParam();
  std::cout << "PDA_D1MACH" << std::endl;
  PDA_D1MACH();
  std::cout << "BFGS2Minim_constructs" << std::endl;
  BFGS2Minim_constructs();
  std::cout << "LineTwoErrML_basics" << std::endl;
  LineTwoErrML_basics();
  std::cout << "MCPoint_Less" << std::endl;
  MCPoint_Less();
  std::cout << "llPoint_Wrks" << std::endl;
  llPoint_Wrks();
  std::cout << "prior_sampler_constr" << std::endl;
  prior_sampler_constr();
  std::cout << "SOutMCMon_tprint(" << std::endl;
  SOutMCMon_tprint();
  std::cout << "t_startSetDirect" << std::endl;
  t_startSetDirect();
  std::cout << "t_WPPoint_construct" << std::endl;
  t_WPPoint_construct();
  std::cout << "metropolis_accept" << std::endl;
  metropolis_accept();

  std::cout << "Passed." << std::endl;

  return 0;

}


//# StandardVisCal.cc: Implementation of Standard VisCal types
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include <synthesis/MeasurementComponents/StandardVisCal.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <scimath/Fitting/LSQFit.h>

#include <tables/Tables/ExprNode.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

// math.h ?

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  PJones
//

PJones::PJones(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  VisJones(vs),
  pjonestype_(Jones::Diagonal),
  pa_()
{
  if (prtlev()>2) cout << "P::P(vs)" << endl;
}

PJones::~PJones() {
  if (prtlev()>2) cout << "P::~P()" << endl;
}

// We need to locally form the Jones according to 
//  the correlations we have

void PJones::syncJones(const Bool& doInv) {

  // Circulars
  if (vb().corrType()(0)==5)
    pjonestype_=Jones::Diagonal;

  // Linears
  else if (vb().corrType()(0)==9)
    pjonestype_=Jones::General;

  VisJones::syncJones(doInv);

}


void PJones::calcPar() {

  if (prtlev()>6) cout << "      VC::calcPar()" << endl;

  // Get parallactic angle from the vb:
  pa().resize(nAnt());
  pa() = vb().feed_pa(currTime());

  // Initialize parameter arrays
  currCPar().resize(1,1,nAnt());
  currParOK().resize(1,1,nAnt());
  currParOK()=True;

  // Fill currCPar() with exp(i*pa)
  Float* a=pa().data();
  Complex* cp=currCPar().data();
  Double ang(0.0);
  for (Int iant=0;iant<nAnt();++iant,++a,++cp) {
    ang=Double(*a);
    (*cp) = Complex(cos(ang),sin(ang));  // as a complex number
  }
  //  cout << "ang = " << ang << endl;

  // Pars now valid, matrices not
  validateP();
  invalidateJ();

}

// Calculate a single Jones matrix by some means from parameters
void PJones::calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk,
			  const Vector<Complex>& par, const Vector<Bool>& pOk ) {

  if (prtlev()>10) cout << "       P::calcOneJones()" << endl;

  //  TBD: handle linears too

  // Circular version:
  if (pOk(0)) {

    switch (jonesType()) {
    case Jones::Diagonal: {
      mat(0)=conj(par(0));  // exp(-ia)
      mat(1)=par(0);        // exp(ia)
      mOk=True;
      break;
    }
    case Jones::General: {
      Float a=arg(par(0));
      mat(0)=mat(3)=cos(a);
      mat(1)=sin(a);
      mat(2)=-mat(1);
      mOk=True;
      break;
    }
    default:
      throw(AipsError("PJones doesn't know if it is Diag (Circ) or General (Lin)"));
      break;

    }



  }
}



// **********************************************************
//  TJones Implementations
//

TJones::TJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs),    // immediate parent
  tcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "T::T(vs)" << endl;
}
TJones::TJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt),
  tcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "T::T(nAnt)" << endl;
}

TJones::~TJones() {
  if (prtlev()>2) cout << "T::~T()" << endl;
}

void TJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   T::guessPar(vb)" << endl;

  // Assumes:  1. corrs in canonical order
  //           2. vb has 1 channel (has been freq-averaged)

  // Make an antenna-based guess at T
  //  Correlation membership-dependence
  //  nCorr = 1: use icorr=0
  //  nCorr = 2: use icorr=[0,1]
  //  nCorr = 4: use icorr=[0,3]

  Int nCorr(1);
  Int nDataCorr(vb.visCube().shape()(0));
  Vector<Int> corridx(1,0);
  if (nDataCorr==2) {
    nCorr=2;
    corridx.resize(nCorr);
    corridx(0)=0;
    corridx(1)=1;
  } 
  else if (nDataCorr==4) {
    nCorr=2;
    corridx.resize(nCorr);
    corridx(0)=0;
    corridx(1)=3;
  }

  // Find out which ants are available
  // TBD: count nominal guessant rows, insist not much less than nAnt
  Vector<Bool> antok(nAnt(),False);
  Vector<Bool> rowok(vb.nRow(),False);
  for (Int irow=0;irow<vb.nRow();++irow) {
    // Is this row ok?
    rowok(irow)= (!vb.flagRow()(irow) &&
		  (vb.antenna1()(irow)!=vb.antenna2()(irow)) &&
		  nfalse(vb.flag().column(irow))> 0 );
    if (rowok(irow)) {
      antok(vb.antenna1()(irow))=True;
      antok(vb.antenna2()(irow))=True;
    }
  }

  // Assume refant is the target ant, for starters
  Int guessant(refant());

  // If no refant specified, or no data for refant
  //   base first guess on first good ant
  if (guessant<0 || !antok(guessant)) {
    guessant=0;
    while (!antok(guessant++));
  }

  AlwaysAssert(guessant>-1,AipsError);

  Cube<Complex>& V(vb.visCube());
  Float amp(0.0),ampave(0.0);
  Int namp(0);
  solveCPar()=Complex(0.0);
  for (Int irow=1;irow<vb.nRow();++irow) {  // why not row zero? copied from Calibrator

    if (rowok(irow)) {
      Int a1=vb.antenna1()(irow);
      Int a2=vb.antenna2()(irow);

      // If this row contains the guessant
      if (a1 == guessant || a2==guessant) {
      
	for (Int icorr=0;icorr<nCorr;icorr++) {
	  Complex& Vi(V(corridx(icorr),0,irow));
	  amp=abs(Vi);
	  if (amp>0.0f) {
	    if (a1 == guessant)
	      solveCPar()(0,0,a2)+=(conj(Vi)/amp/Float(nCorr));
	    //	      solveCPar()(0,0,a2)+=(conj(Vi)/Float(nCorr));
	    else
	      solveCPar()(0,0,a1)+=((Vi)/amp/Float(nCorr));
	    //	      solveCPar()(0,0,a1)+=((Vi)/Float(nCorr));
	    
	    ampave+=amp;
	    namp++;
	    //	cout << "          " << abs(Vi) << " " << arg(Vi)*180.0/C::pi << endl;
	  }
	}
      } // guessant in row
    } // rowok
  } // irow

  //  cout << "Guess:" << endl
  //   << "amp = " << amplitude(solveCPar())
  //     << endl;
 

  // Scale them by the mean amplitude
  ampave/=Float(namp);
  ampave=sqrt(ampave);

  //  cout << "ampave = " << ampave << endl;

  solveCPar()*=Complex(ampave);
  //  solveCPar()/=Complex(ampave);
  solveCPar()(0,0,guessant) = Complex(ampave);
  solveCPar()(LogicalArray(amplitude(solveCPar())==0.0f)) = Complex(ampave);
  solveParOK()=True;

  //  solveCPar()*=Complex(0.9);

  //  cout << "Guess:" << endl
  //       << "amp = " << amplitude(solveCPar())
  //       << "phase = " << phase(solveCPar())
  //       << endl;

}

// Fill the trivial DJ matrix elements
void TJones::initTrivDJ() {

  if (prtlev()>4) cout << "   T::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  // This is the unit matrix
  //  TBD: could we use a Jones::Unit type instead?
  diffJElem().resize(IPosition(4,1,1,1,1));
  diffJElem()=Complex(1.0);

}

  





//============================================================



Int TJones::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{
  if (prtlev()>2) cout << "   T::setupSim()" << endl;

  // This method only called in simulate context!
  AlwaysAssert((isSimulated()),AipsError);

  // this is done in sizeUpSim, and if VS goes away we'll probably need to 
  // be passing either the ms or the vi down from Calibrator and Simulator
  // into here and then on to suzeUpSim.
  // right now, I want vi to get vi.msColumns()
  // I'm assuming that there's only one ms attached to the VI *
  VisIter& vi(vs.iter());
  // VisBuffer vb(vi);
  
  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);
  
  // we can use the private access directly the corruptor
  tcorruptor_p = new TJonesCorruptor(nSim);
  // but set the public one 
  corruptor_p = tcorruptor_p;

  corruptor_p->startTime()=min(solTimes);
  corruptor_p->stopTime()=max(solTimes);
  corruptor_p->prtlev()=prtlev();

  Int Seed;
  if (simpar.isDefined("seed")) {    
    Seed=simpar.asInt("seed");
  }

  Float Beta(0.5); // exponent for generalized 1/f noise
  if (simpar.isDefined("beta")) {    
    Beta=simpar.asFloat("beta");
  }

  Float Scale(1.0); // scale of fluctuations rel to mean
  if (simpar.isDefined("scale")) {    
    Scale=simpar.asFloat("scale");
  }

  // initialize spw etc information in Corruptor
  // the "initialize" function that's corruptor-specific calculates the 
  // corruption values
  // this preparation is just to tell the corruptor about its VC - maybe 
  // the logic should be changed so the corruptor doesn't have to know 
  // all this?

  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();
 
  AlwaysAssert(nSpw()==spwcols.nrow(),AipsError);
  // things will break if spw mapping, ie not in same order as in vs
  corruptor_p->nSpw()=nSpw();
  corruptor_p->nAnt()=nAnt();
  corruptor_p->currAnt()=0;
  corruptor_p->currSpw()=0;
  corruptor_p->fRefFreq().resize(nSpw());
  corruptor_p->fnChan().resize(nSpw());
  corruptor_p->fWidth().resize(nSpw());

    for (Int irow=0;irow<nSpw();++irow) { 
      corruptor_p->fRefFreq()[irow]=spwcols.refFrequency()(irow);
      corruptor_p->fnChan()[irow]=spwcols.numChan()(irow);
      corruptor_p->fWidth()[irow]=spwcols.totalBandwidth()(irow); 
      // totalBandwidthQuant ?  in other places its assumed to be in Hz
    }
      // see MSsummary.cc for more info/examples

  if (simpar.isDefined("mean_pwv"))
    tcorruptor_p->mean_pwv() = simpar.asFloat("mean_pwv");

  if (tcorruptor_p->mean_pwv()<=0)
    throw(AipsError("TCorruptor attempted initialization with undefined PWV"));

  if (simpar.isDefined("mode")) {    
    if (simpar.asString("mode") == "test")
      tcorruptor_p->initialize();
    else if (simpar.asString("mode") == "individual") {
      tcorruptor_p->initialize(Seed,Beta,Scale);
    } else 
      throw(AipsError("Unknown Mode for TJonesCorruptor"));        
  } else {
      throw(AipsError("No Mode specified for TJones corruptor."));
  }

  return nSim;
}



Bool TJones::simPar(VisBuffGroupAcc& vbga) {
// void TJones::simPar() {

  LogIO os(LogOrigin("T", "simPar()", WHERE));

  if (prtlev()>4) cout << "   T::simPar()" << endl;

  // This method only called in simulate context!
  AlwaysAssert((isSimulated()),AipsError);
  
  // sizeUpSolve did this (Mueller):
  // solveCPar().resize(nPar(),nChanPar(),nBln());
  // or this (Jones):
  // solveCPar().resize(nPar(),1,nAnt());

  // loop through correlations or use corridx ?

  try {
    
    for (Int ivb=0;ivb<vbga.nBuf();++ivb) {
      CalVisBuffer& cvb(vbga(ivb));   // why the cast to a CalVisBuffer different from regular VB?
      Vector<Int>& a1(cvb.antenna1());
      Vector<Int>& a2(cvb.antenna2());
      for (Int irow=0;irow<cvb.nRow();++irow) {
	if ( !cvb.flagRow()(irow) &&
	     cvb.antenna1()(irow)!=cvb.antenna2()(irow) &&
	     nfalse(cvb.flag().column(irow))> 0 ) {	
	  
	  // outside row loop i.e. use all same Corruptor slot for this VB?
	  if (corruptor_p->curr_time()!=refTime()) {
	    
	    corruptor_p->curr_time()=refTime();
	    
	    // find new slot if required
	    Double dt(1e10),dt0(-1);
	    dt0 = abs(corruptor_p->slot_time() - refTime());

	    if (refTime()<corruptor_p->curr_time()) {
	      //throw(AipsError("T:simPar error: VB not monotonic in time!"));

	      for (Int newslot=corruptor_p->curr_slot()-1;newslot>=0;newslot--) {
		dt=abs(corruptor_p->slot_time(newslot) - refTime());
		if (dt<dt0) {
		  // use this check for on-depand corruptors to get new val.
		  corruptor_p->curr_slot()=newslot;
		  dt0 = dt;
		  if (prtlev()>4) 
		    cout << "    T:simPar retreating to time " << refTime() << endl;
		}
	      }	  
	    } else {	      
	      for (Int newslot=corruptor_p->curr_slot()+1;newslot<corruptor_p->nSim();newslot++) {
		dt=abs(corruptor_p->slot_time(newslot) - refTime());
		if (dt<dt0) {
		  // use this check for on-depand corruptors to get new val.
		  corruptor_p->curr_slot()=newslot;
		  dt0 = dt;
		  if (prtlev()>4) 
		    cout << "    T:simPar advancing to time " << refTime() << endl;
		}	  
	      }
	    }
	  }
	  
	  corruptor_p->currAnt()=a1(irow);
	  // solveCPar()(0,focusChan(),a1(irow))=tcorruptor_p->gain(cvb.frequency()(focusChan()));
	  solveCPar()(0,focusChan(),a1(irow))=tcorruptor_p->gain(focusChan());
	  // RI TODO if we keep the VBGA we need to actually do something 
	  // with all nBuf() in there!!!
	}
      }
    }
  } catch (AipsError x) {
    if (tcorruptor_p) delete tcorruptor_p;
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}





TJonesCorruptor::TJonesCorruptor(const Int nSim) : 
  CalCorruptor(nSim),  // parent
  mean_pwv_(-1.)
{}

TJonesCorruptor::~TJonesCorruptor() {
  if (prtlev()>2) cout << "TCorruptor::~TCorruptor()" << endl;
}


#ifdef RI_ATM

Vector<Float>* TJonesCorruptor::pwv() { 
  if (currAnt()<=pwv_p.nelements())
    return pwv_p[currAnt()];
  else
    return NULL;
};
 
Float& TJonesCorruptor::pwv(const Int islot) { 
  if (currAnt()<=pwv_p.nelements())
    return (*pwv_p[currAnt()])(islot);
  else
    throw(AipsError("TJonesCorruptor internal error accessing delay()"));;
};

Float TJonesCorruptor::delay(const Int islot, const Int ichan) { 
  if (currAnt()<=pwv_p.nelements()) {
    Float deltapwv = (*pwv_p[currAnt()])(islot);
//    if (prtlev()>2 and currAnt()<2 and islot<2) {
//      cout << islot << " | " << ichan << " | " << currAnt() << " : " << 
//	itsSkyStatus->getWetOpacity().get("rad") << 
//	" * " << deltapwv << " * " << mean_pwv() << endl;
//    }
    return itsSkyStatus->getDispersivePhaseDelay(currSpw(),ichan).get("rad") 
      * deltapwv * mean_pwv();
  } else
    throw(AipsError("TJonesCorruptor internal error accessing pwv()"));;
};

#else // ----------

Vector<Float>* TJonesCorruptor::delay() { 
  if (currAnt()<=delay_p.nelements())
    return delay_p[currAnt()];
  else
    return NULL;
};
 
Float TJonesCorruptor::delay(const Int islot, const Int ichan) { 
  if (currAnt()<=delay_p.nelements())
    return (*delay_p[currAnt()])(islot);
  else
    throw(AipsError("TJonesCorruptor internal error accessing delay()"));;
};

#endif






void TJonesCorruptor::initialize() {
  // for testing

  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

#ifdef RI_ATM
  initAtm();
  pwv_p.resize(1,False,True);
  pwv_p[0] = new Vector<Float>(nSim());
  // not really pwv, but this is a test mode
  for (Int i=0;i<nSim();++i) 
    (*pwv())(i) = Float(i)/Float(nSim())*mean_pwv();  
#else
  delay_p.resize(1,False,True);
  delay_p[0] = new Vector<Float>(nSim());
  for (Int i=0;i<nSim();++i) 
    (*delay())(i) = Float(i)/Float(nSim())*mean_pwv(); // pretends pwv is # radians
#endif

  initialized()=True;
  if (prtlev()>2) cout << "TCorruptor::init" << endl;
}




#ifdef RI_ATM
void TJonesCorruptor::initAtm() {

  atm::Temperature  T( 270.0,"K" );   // Ground temperature
  atm::Pressure     P( 560.0,"mb");   // Ground Pressure
  atm::Humidity     H(  20,"%" );     // Ground Relative Humidity (indication)
  atm::Length       Alt(  5000,"m" ); // Altitude of the site 
  atm::Length       WVL(   2.0,"km"); // Water vapor scale height

  double TLR = -5.6;     // Tropospheric lapse rate (must be in K/km)
  atm::Length  topAtm(  48.0,"km");   // Upper atm. boundary for calculations
  atm::Pressure Pstep(  10.0,"mb");   // Primary pressure step
  double PstepFact = 1.2; // Pressure step ratio between two consecutive layers
  atm::Atmospheretype atmType = atm::tropical;

  itsatm = new atm::AtmProfile(Alt, P, T, TLR, 
			       H, WVL, Pstep, PstepFact, 
			       topAtm, atmType);

  if (nSpw()<=0)
    throw(AipsError("TCorruptor::initAtm called before spw setup."));

  // does SpectralGrid constructor work with multiple spws?

  double fRes(fWidth()[0]/fnChan()[0]);
  itsSpecGrid = new atm::SpectralGrid(fnChan()[0],0, 
				      atm::Frequency(fRefFreq()[0],"Hz"),
				      atm::Frequency(fRes,"Hz"));

  itsrip = new atm::RefractiveIndexProfile(*itsSpecGrid,*itsatm);
  
  if (prtlev()>2) cout << "TCorruptor::getDispersiveWetPathLength = " 
		       << itsrip->getDispersiveWetPathLength().get("micron") 
		       << " microns" << endl;

  itsSkyStatus = new atm::SkyStatus(*itsrip);

  if (prtlev()>2) cout << "TCorruptor::getDispersivePhaseDelay = " 
		       << itsSkyStatus->getDispersivePhaseDelay(0,0).get("rad") 
		       << " rad" << endl;

}
#endif


void TJonesCorruptor::initialize(const Int Seed, const Float Beta, const Float scale) {
  // individual delays for each antenna

  fBM* myfbm = new fBM(nSim());

#ifdef RI_ATM
  initAtm();

  pwv_p.resize(nAnt(),False,True);
  for (Int iant=0;iant<nAnt();++iant){
    pwv_p[iant] = new Vector<Float>(nSim());
    myfbm->initialize(Seed+iant,Beta); // (re)initialize
    *(pwv_p[iant]) = myfbm->data(); // iAnt()=iant; delay() = myfbm->data();
    Float pmean = mean(*(pwv_p[iant]));
    Float rms = sqrt(mean( (*(pwv_p[iant])-pmean)*(*(pwv_p[iant])-pmean) ));
    if (prtlev()>2 and currAnt()<2) {
      cout << "RMS fBM fluctuation for antenna " << iant 
	   << " = " << rms << " ( " << pmean << " ) " << endl;      
    }
    // PWV = PWV_mean * 10.^ ( fBM - 0.5)
    // fluctuations are 1/scale to scale x pwv_mean
    for (Int islot=0;islot<nSim();++islot)
      (*(pwv_p[iant]))[islot] = pow(10., (*(pwv_p[iant]))[islot]/rms/3.0*scale );  
    if (prtlev()>2 and currAnt()<2) {
      Float pmean = mean(*(pwv_p[iant]));
      Float rms = sqrt(mean( (*(pwv_p[iant])-pmean)*(*(pwv_p[iant])-pmean) ));
      cout << "RMS pwv fluctuation for antenna " << iant 
	   << " = " << rms << endl;      
    }
    currAnt()=iant;
  }

#else

  delay_p.resize(nAnt(),False,True);
  for (Int iant=0;iant<nAnt();++iant){
    delay_p[iant] = new Vector<Float>(nSim());
    myfbm->initialize(Seed+iant,Beta); // (re)initialize
    *(delay_p[iant]) = myfbm->data(); // iAnt()=iant; delay() = myfbm->data();
    // RI TODO get scaling right with rms input from sm etc
    *(delay_p[iant]) = *(delay_p[iant]);
    currAnt()=iant;
    //if (prtlev()>2) {
    //  Float ave(mean(*(delay_p[iant])));
    //  cout << "Average delay for antenna " << iant 
    //	   << " = " << ave << endl;      
    //}
  }

#endif

  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

  initialized()=True;
  if (prtlev()>2) cout << "TCorruptor::init" << endl;

}


// RI TODO another initializer for the phase screen
//
// for phase screen we'll need to pass a mean time from the vb to 
// thisgain, as well as a direction.  initialize will need to know 
// the total scan length and wind speed to make the screen long 
// enough to blow over the array for the entire scan





Complex TJonesCorruptor::gain(const Int ichan) {

  // RI TODO different calculation if mode="screen"

  if (curr_slot()>=0 and curr_slot()<nSim()) {
    // delay() gets currAnt  so this works for mode=test
    // which only has one effective Ant, and mode=individual, as long as 
    // ant has been set before the call to gain()
    
    // Float freq = fRefFreq()[currSpw()] + 
    //   Float(ichan) * (fWidth()[currSpw()]/Float(fnChan()[currSpw()]));
    
#ifdef RI_ATM 
    Float phase = delay(curr_slot(),ichan); // DispersivePhaseDelay returns angle
#else
    Float phase = delay(curr_slot(),0); 
#endif

    return Complex(cos(phase),sin(phase));
  } else {
    cout << "TCorruptor::gain: slot " << curr_slot() << "out of range!" << endl;
    return Complex(1.);
  }
}



fBM::fBM(uInt i1) :    
  initialized_(False)
{ data_ = new Vector<Float>(i1); };

fBM::fBM(uInt i1, uInt i2) :
  initialized_(False)
{ data_ = new Matrix<Float>(i1,i2); };

fBM::fBM(uInt i1, uInt i2, uInt i3) :
  initialized_(False)
{ data_ = new Cube<Float>(i1,i2,i3); };

void fBM::initialize(const Int seed, const Float beta) {
  
  MLCG rndGen_p(seed,seed);
  Normal nDist_p(&rndGen_p, 0.0, 1.0); // sigma=1.
  Uniform uDist_p(&rndGen_p, 0.0, 1.0);
  IPosition s = data_->shape();
  uInt ndim = s.nelements();
  Float amp,phase,pi=3.14159265358979;
  uInt i0,j0;
  
  // FFTServer<Float,Complex> server;
  // takes a lot of thread thrashing to resize the server, so
  FFTServer<Float,Complex> server(IPosition(1,s(0)));
  // This class assumes that a Complex array is stored as
  // pairs of floating point numbers, with no intervening gaps, 
  // and with the real component first ie., 
  // <src>[re0,im0,re1,im1, ...]</src>. This means that the 
  // following type casts work,
  // <srcblock>
  // S * complexPtr;
  // T * realPtr = (T * ) complexPtr;
  // </srcblock>
  
  Matrix<Complex> F2(s(0)/2,s(0)/2);
  Matrix<Float> G2;
  Vector<Complex> F(s(0)/2);
  Vector<Float> G; // size zero to let FFTServer calc right size
  
  // FFTServer C,R assumes that the input is hermitian and only has 
  // half of the elements in each direction
  
  switch(ndim) {
  case 1:
    // beta = 1+2H = 5-2D
    for (uInt i=0; i<s(0)/2-1; i++) {
      // data_->operator()(IPosition(1,i))=5.;
      phase = 2.*pi*uDist_p(); 
      // RI TODO is this assuming the origin is at 0,0 in which case 
      // we should be using FFTServer::fft0 ? 
      amp = pow(Float(i+1), -0.5*beta) * nDist_p();
      F(i)=Complex(amp*cos(phase),amp*sin(phase));
      // F(s(0)-i)=Complex(amp*cos(phase),-amp*sin(phase));
    }
    server.fft(G,F,False);  // complex to real Xform
    // G comes out twice length of F 
    for (uInt i=0; i<s(0); i++)
      data_->operator()(IPosition(1,i)) = G(i); // there has to be a better way with strides or something.
    //    cout << endl << F(2) << " -> " << G(2) << " -> " 
    //	 << data_->operator()(IPosition(1,2)) << endl;
    break;
  case 2:
    // beta = 1+2H = 7-2D
    // I think the server will resize itself.
    // server.resize(IPosition(2,s(0),s(1)));
    F2.resize(s(0)/2,s(1)/2);
    for (uInt i=0; i<s(0)/2; i++)
      for (uInt j=0; j<s(1)/2; j++) {
	phase = 2.*pi*uDist_p(); 	  
	// ok to draw from the MLCG directly as well as through the Normal ?
	// RI TODO is this assuming the origin is at 0,0 in which case 
	// we should be using FFTServer::fft0 ? 
	if (i!=0 or j!=0) {
	  amp = pow(Float(i)*Float(i) + Float(j)*Float(j), 
		    -0.25*(beta+1)) * nDist_p();
	} else {
	  amp = 0.;
	}
	F2(i,j)=Complex(amp*cos(phase),amp*sin(phase));
	// if (i==0) {
	//   i0=0;
	// } else {
	//   i0=s(0)-i;
	// }
	// if (j==0) {
	//   j0=0;
	// } else {
	//   j0=s(1)-j;
	// }
	// F2(i0,j0)=Complex(amp*cos(phase),-amp*sin(phase));
      }
    // The complex to real transform does not check that the
    // imaginary component of the values where u=0 are zero
    F2(s(0)/2,0).imag()=0.;
    F2(0,s(1)/2).imag()=0.;
    F2(s(0)/2,s(1)/2).imag()=0.;
    // for (uInt i=0; i<s(0)/2; i++)
    // 	for (uInt j=0; j<s(1)/2; j++) {
    // 	  phase = 2.*pi*uDist_p();
    // 	  amp = pow(Double(i)*Double(i) + Double(j)*Double(j), 
    // 		    -0.25*(beta+1)) * nDist_p();
    // 	  F2(i,s(1)-j) = Complex(amp*cos(phase),amp*sin(phase));
    // 	  F2(s(0)-i,j) = Complex(amp*cos(phase),-amp*sin(phase));
    // 	}
    server.fft(G2,F2,False);  // complex to real Xform
    //data_->operator()(IPosition(2,i,j))=5.;
    //data_->operator()=InvFFT(F); //complex to real
    break;
  case 3:
    // beta = 1+2H = 9-2D
    throw(AipsError("no 3d fractional Brownian motion yet."));
    for (uInt i=0; i<s(0); i++)
      for (uInt j=0; j<s(1); j++)
	for (uInt k=0; j<s(3); k++)
	  data_->operator()(IPosition(3,i,j,k))=5.;     
  }
};






// **********************************************************
//  GJones Implementations
//

GJones::GJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs)    // immediate parent
{
  if (prtlev()>2) cout << "G::G(vs)" << endl;
}

GJones::GJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt)
{
  if (prtlev()>2) cout << "G::G(nAnt)" << endl;
}

GJones::~GJones() {
  if (prtlev()>2) cout << "G::~G()" << endl;
}

void GJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   G::guessPar(vb)" << endl;

  // Make a guess at antenna-based G
  //  Correlation membership-dependencexm
  //  assumes corrs in canonical order
  //  nCorr = 1: use icorr=0
  //  nCorr = 2: use icorr=[0,1]
  //  nCorr = 4: use icorr=[0,3]

  Int nCorr(2);
  Int nDataCorr(vb.visCube().shape()(0));
  Vector<Int> corridx(nCorr,0);
  if (nDataCorr==2) {
    corridx(0)=0;
    corridx(1)=1;
  } 
  else if (nDataCorr==4) {
    corridx(0)=0;
    corridx(1)=3;
  }

  Int guesschan(vb.visCube().shape()(1)-1);

  //  cout << "guesschan = " << guesschan << endl;
  //  cout << "nCorr = " << nCorr << endl;
  //  cout << "corridx = " << corridx << endl;


  // Find out which ants are available
  Vector<Int> antok(nAnt(),0);
  Vector<Bool> rowok(vb.nRow(),False);
  for (Int irow=0;irow<vb.nRow();++irow) {
    // Is this row ok
    rowok(irow)= (!vb.flagRow()(irow) &&
		  vb.antenna1()(irow)!=vb.antenna2()(irow) &&
		  nfalse(vb.flag().column(irow))> 0 );
    if (rowok(irow)) {
      antok(vb.antenna1()(irow))++;
      antok(vb.antenna2()(irow))++;
    }
  }

  // Assume refant is the target ant, for starters
  Int guessant(refant());
  //  Int guessant(-1);

  // If no refant specified, or no data for refant
  //   base first guess on first good ant
  if (guessant<0 || antok(guessant)<1) {
    guessant=0;
    while (antok(guessant)<1) guessant++;
  }

  //  cout << "antok = " << antok << endl;

  //  cout << "guessant = " << guessant << "  (" << currSpw() << ")" << endl;

  AlwaysAssert(guessant>-1,AipsError);

  Cube<Complex>& V(vb.visCube());
  Float amp(0.0),ampave(0.0);
  Int namp(0);
  solveCPar()=Complex(0.0);
  for (Int irow=1;irow<vb.nRow();++irow) {

    if (rowok(irow) && !vb.flag()(guesschan,irow)) {
      Int a1=vb.antenna1()(irow);
      Int a2=vb.antenna2()(irow);

      // If this row contains the guessant
      if (a1 == guessant || a2==guessant) {

	//	cout << a1 << " " << a2 << " " 
	//	     << vb.visCube().xyPlane(irow).column(guesschan) << " "
	//	     << amplitude(vb.visCube().xyPlane(irow).column(guesschan)) << " "
	//	     << endl;

	for (Int icorr=0;icorr<nCorr;icorr++) {
	  Complex& Vi(V(corridx(icorr),guesschan,irow));
	  amp=abs(Vi);
	  if (amp>0.0f) {
	    if (a1 == guessant)
	      solveCPar()(icorr,0,a2)=conj(Vi);
	    else
	      solveCPar()(icorr,0,a1)=(Vi);
	      
	    ampave+=amp;
	    namp++;
	  }
	}
      } // guessant
    } // rowok
  } // irow

  // Scale them by the mean amplitude

  if (namp>0) {
    ampave/=Float(namp);
    ampave=sqrt(ampave);
    //  solveCPar()*=Complex(ampave);
    solveCPar()/=Complex(ampave);
    solveCPar()(0,0,guessant)=solveCPar()(1,0,guessant)=Complex(ampave);
    solveCPar()(LogicalArray(amplitude(solveCPar())==0.0f)) = Complex(ampave);
  }
  else
    solveCPar()=Complex(0.3);

  solveParOK()=True;

  //For scalar data, Set "other" pol soln to zero
  if (nDataCorr == 1)
    solveCPar()(IPosition(3,1,0,0),IPosition(3,1,0,nAnt()-1))=Complex(0.0);

  //  cout << "Guess:" << endl;
  //  cout << "amplitude(solveCPar())   = " << amplitude(solveCPar()) << endl;
  //  cout << "phases       = " << phase(solveCPar())*180.0/C::pi << endl;
  //  cout << "solveParOK() = " << solveParOK() << endl;

}

// Fill the trivial DJ matrix elements
void GJones::initTrivDJ() {

  if (prtlev()>4) cout << "   G::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  //  1 0     0 0
  //  0 0  &  0 1
  // 
  if (diffJElem().nelements()==0) {
    diffJElem().resize(IPosition(4,2,2,1,1));
    diffJElem()=0.0;
    diffJElem()(IPosition(4,0,0,0,0))=Complex(1.0);
    diffJElem()(IPosition(4,1,1,0,0))=Complex(1.0);
  }

}


// **********************************************************
//  BJones Implementations
//

BJones::BJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  GJones(vs),             // immediate parent
  maxchangap_p(0)
{
  if (prtlev()>2) cout << "B::B(vs)" << endl;
}

BJones::BJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  GJones(nAnt),
  maxchangap_p(0)
{
  if (prtlev()>2) cout << "B::B(nAnt)" << endl;
}

BJones::~BJones() {
  if (prtlev()>2) cout << "B::~B()" << endl;
}

void BJones::setSolve(const Record& solve) {

  // call parent to get general stuff
  GJones::setSolve(solve);

  // get max chan gap from user
  maxchangap_p=0;
  if (solve.isDefined("maxgap"))
    maxchangap_p=solve.asInt("maxgap");

}

void BJones::normalize() {

  logSink() << "Normalizing solutions per spw, pol, ant, time." 
	    << LogIO::POST;

  // Iteration axes (norm per spw, pol, ant, timestamp)
  IPosition itax(3,0,2,3);
  
  for (Int ispw=0;ispw<nSpw();++ispw)
    if (cs().nTime(ispw)>0) {
      ArrayIterator<Complex> soliter(cs().par(ispw),itax,False);
      ArrayIterator<Bool> okiter(cs().parOK(ispw),itax,False);
      while (!soliter.pastEnd()) {
	normSolnArray(soliter.array(),okiter.array(),True);
	soliter.next();
	okiter.next();
      }
      
    }

}

void BJones::globalPostSolveTinker() {

  // Call parent to do more general things
  SolvableVisJones::globalPostSolveTinker();

  // Fill gaps in channels, if necessary
  if (maxchangap_p>0)
    fillChanGaps();

}

void BJones::fillChanGaps() {

  logSink() << "Filling in flagged solution channels by interpolation." 
	    << LogIO::POST;

  // Iteration axes (norm per spw, pol, ant, timestamp)
  IPosition itax(3,0,2,3);

  for (Int ispw=0;ispw<nSpw();++ispw)
    // Only if there are any solutions, and there are more than 2 channels
    if (cs().nTime(ispw)>0 && cs().nChan(ispw)>2) {
      // Iterate over time, pol, and ant
      ArrayIterator<Complex> soliter(cs().par(ispw),itax,False);
      ArrayIterator<Bool> okiter(cs().parOK(ispw),itax,False);
      while (!soliter.pastEnd()) {
	fillChanGapArray(soliter.array(),okiter.array());
	soliter.next();
	okiter.next();
      }
    }
}

void BJones::fillChanGapArray(Array<Complex>& sol,
			      Array<Bool>& solOK) {

  // Make the arrays 1D
  Vector<Complex> solv(sol.reform(IPosition(1,sol.nelements())));
  Vector<Bool> solOKv(solOK.reform(IPosition(1,solOK.nelements())));

  Int nChan(solv.nelements());
  Bool done(False);
  Int ich(0), ch1(-1), ch2(-1);
  Int dch(1);
  Float a0, da, a, p0, dp, p, fch;

  // Advance to first unflagged channel
  while(!solOKv(ich) && ich<nChan) ++ich;

  // Found no unflagged channels, so signal escape
  if (ich==nChan) done=True;

  // done turns True if we reach nChan, and nothing more to do
  while (!done) {

    // Advance to next flagged channel
    while(solOKv(ich) && ich<nChan) ++ich;

    if (ich<nChan) {

      // Left boundary of gap
      ch1=ich-1;     // (NB: above logic prevents ch1 < 0)

      // Find right boundary:
      ch2=ich+1;
      while (!solOKv(ch2) && ch2<nChan) ++ch2;

      if (ch2<nChan) {
	
	// The span of the interpolation (in channels)
	dch=ch2-ch1;

	//cout << ch1 << " " << ch2 << " " << dch << endl;

	// Interpolate only if gap is narrower than maxchangap_p
	if (dch<=maxchangap_p+1) {

	  // calculate interp params
	  a0=abs(solv(ch1));
	  da=abs(solv(ch2))-a0;
	  p0=arg(solv(ch1));
	  dp=arg(solv(ch2))-p0;
	  if (dp>C::pi) dp-=(2*C::pi);
	  if (dp<-C::pi) dp+=(2*C::pi);	

	  //cout << a0 << " " << da << " " << p0 << " " << dp << endl;


	  // interpolate the intervening channels
	  while (ich<ch2) {
	    fch=Float(ich-ch1)/Float(dch);
	    a=a0 + da*fch;
	    p=p0 + dp*fch;

	    // cout << " " << ich << " " << a << " " << p << endl;

	    solv(ich)=a*Complex(cos(p),sin(p));
	    solOKv(ich)=True;
	    ++ich;
	  }

	  // Begin looking for new gap on next round
	  ++ich;
	  
	}
	else
	  // we skipped a gap, look beyond it on next round
	  ich=ch2+1;
      }
      else
	// Reached nChan looking for ch2
	done=True;
    }
    else
      // Reach nChan looking for gaps 
      done=True;
  } // done

}

// **********************************************************
//  DJones Implementations
//

DJones::DJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs),   // immediate parent
  solvePol_(0)
{
  if (prtlev()>2) cout << "D::D(vs)" << endl;

}

DJones::DJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt),
  solvePol_(0)
{
  if (prtlev()>2) cout << "D::D(nAnt)" << endl;

}

DJones::~DJones() {
  if (prtlev()>2) cout << "D::~D()" << endl;
}

void DJones::setApply(const Record& apply) {

  SolvableVisJones::setApply(apply);

  // Force calwt to False for now
  calWt()=False;

}


void DJones::setSolve(const Record& solvepar) {

  // Call parent
  SolvableVisJones::setSolve(solvepar);

  // Determine if we are solving for source pol or not
  if (solvepar.isDefined("type")) {
    String type = solvepar.asString("type");
    if (type.contains("QU")) {
      solvePol_=2;
      logSink() << "Will solve for source polarization (Q,U)" << LogIO::POST;
    }
    else if (type.contains("X")) {
      solvePol_=1;
      logSink() << "Will solve for source polarization position angle correction" << LogIO::POST;
    }
    else
      solvePol_=0;
  }

  logSink() << "Using only cross-hand data for instrumental polarization solution." << LogIO::POST;
  
  // For D insist preavg is meaningful (5 minutes or user-supplied)
  if (preavg()<0.0)
    preavg()=300.0;

}


void DJones::calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk,
			  const Vector<Complex>& par, const Vector<Bool>& pOk) {

  if (prtlev()>10) cout << "   D::calcOneJones(vb)" << endl;

  // Only if both pars are good, form matrix
  if (allEQ(pOk,True)) {

    // On-diag = 1
    mat(0)=mat(3)=Complex(1.0);
    // Off-diag = par
    mat(1)=par(0);
    mat(2)=par(1);

    mOk=True;

  }
  else {
    mat=Complex(0.0);
    mOk=False;
  }

}

void DJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   D::guessPar(vb)" << endl;

  // TBD: Should we use a common wt for the X-hands??

  // First guess is zero D-terms
  solveCPar()=0.0;
  solveParOK()=True;

  if (jonesType()==Jones::GenLinear) {
    vb.weightMat().row(0)=0.0;
    vb.weightMat().row(3)=0.0;
  }

  if (solvePol()) {

    // solvePol() tells us how many source pol parameters
    srcPolPar().resize(solvePol());

    // The following assumes the MODEL_DATA has been
    //  corrupted by P 

    LSQFit fit(2,LSQComplex());
    Vector<Complex> ce(2);
    ce(0)=Complex(1.0);
    Complex d,md;
    Float wt,a(0.0);
    for (Int irow=0;irow<vb.nRow();++irow) {
      if (!vb.flagRow()(irow)  &&
	  vb.antenna1()(irow)!=vb.antenna2()(irow)) {
	for (Int ich=0;ich<vb.nChannel();++ich) {
	  if (!vb.flag()(ich,irow)) {
	    for (Int icorr=1;icorr<3;++icorr) {
	      md=vb.modelVisCube()(icorr,ich,irow);
	      if (icorr==2) md=conj(md);
	      a=abs(md);
	      if (a>0.0) {
		wt=Double(vb.weightMat()(icorr,irow));
		if (wt>0.0) {
		  d=vb.visCube()(icorr,ich,irow);
		  if (icorr==2) d=conj(d);
		  if (abs(d)>0.0) {

		    ce(1)=md;
		    fit.makeNorm(ce.data(),wt,d,LSQFit::COMPLEX);

		  } // abs(d)>0
		} // wt>0
	      } // a>0
	    } // icorr
	  } // !flag
	} // ich
      } // !flagRow
    } // row

    uInt rank;
    Bool ok = fit.invert(rank);

    Complex sol[2];
    if (ok)
      fit.solve(sol);
    else
      throw(AipsError("Source polarization solution is singular; try solving for D-terms only."));

    if (solvePol()==1 && a>0.0) 
      srcPolPar()(0)=Complex(arg(sol[1]));
    else if (solvePol()==2) {
      srcPolPar()(0)=Complex(real(sol[1]));
      srcPolPar()(1)=Complex(imag(sol[1]));
    }

    // Log source polarization solution
    reportSolvedQU();


  } // solvePol()?

}

void DJones::updatePar(const Vector<Complex> dCalPar,
		       const Vector<Complex> dSrcPar) {

  // Enforce no change in source parameters 
  //  before calling generic version
  Vector<Complex> dsrcpar(dSrcPar.shape());
  dsrcpar=Complex(0.0);
  SolvableVisJones::updatePar(dCalPar,dsrcpar);

}

void DJones::formSolveSNR() {

  solveParSNR()=0.0;

  for (Int iant=0;iant<nAnt();++iant)
    for (Int ipar=0;ipar<nPar();++ipar) {
      if (solveParOK()(ipar,0,iant) &&
	  solveParErr()(ipar,0,iant)>0.0f) {
	solveParSNR()(ipar,0,iant)=1.0f/solveParErr()(ipar,0,iant);
      }
      else
	// Ensure F if Err<=0  (OK?)
	solveParOK()(ipar,0,iant)=False;
    }
}

void DJones::globalPostSolveTinker() {

  // call parent
  SolvableVisJones::globalPostSolveTinker();

  // if not freqdep, report results to the logger
  logResults();

}

						       
void DJones::applyRefAnt() {

  if (refant()<0)
    throw(AipsError("No refant specified."));

  // Get the refant name from the nMS
  String refantName("none");
  MeasurementSet ms(msName());
  MSAntennaColumns msantcol(ms.antenna());
  refantName=msantcol.name()(refant());
  
  logSink() << "Applying refant: " << refantName
            << LogIO::POST;

  Vector<Int> altrefantlist(nAnt());
  indgen(altrefantlist);

  for (Int ispw=0;ispw<nSpw();++ispw) {

    currSpw()=ispw;

    if (cs().nTime(ispw)>0) {

      // References to ease access to solutions
      Array<Complex> sol(cs().par(ispw));
      Array<Bool> sok(cs().parOK(ispw));

      for (Int islot=0;islot<cs().nTime(ispw);++islot) {

	for (Int ich=0;ich<cs().nChan(ispw);++ich) {

	  IPosition ipr(4,0,ich,0,islot);

	  Complex refD1(0.0),refD2(0.0);

	  // Assume user's refant
	  ipr(2)=refant();

	  // If user's refant unavailable, so find another
	  if (!cs().parOK(ispw)(ipr)) {
	    ipr(2)=0;
	    while (ipr(2)<cs().nElem() &&
		   !cs().parOK(ispw)(ipr)) ++ipr(2);
	  }

	  // Found a refant, so use it
	  if (ipr(2)<nAnt()) {

	    refD1=cs().par(ispw)(ipr);
	    refD2=conj(refD1);

	    for (Int iant=0;iant<cs().nElem();++iant) {
	      ipr(2)=iant;
	      ipr(0)=0;
	      cs().par(ispw)(ipr)-=refD1;
	      ipr(0)=1;
	      cs().par(ispw)(ipr)+=refD2;
	    }
	  } // found refant
	} // ich
      } // islot
    } // nTime(ispw)>0
  } // ispw

}

void DJones::logResults() {

  // Don't bother, if the Ds are channelized
  if (freqDepPar()) return;

  Vector<String> rl(2);
  rl(0)="R: ";
  rl(1)="L: ";

  const ROMSAntennaColumns ac(MeasurementSet(msName()).antenna());
  Vector<String> antNames(ac.name().getColumn());

  Vector<uInt> ord;
  genSort(ord,antNames);

  logSink() << "The instrumental polarization solutions are: " << LogIO::POST;

  logSink().output().precision(4);

  for (Int ispw=0;ispw<nSpw();++ispw) {
    
    currSpw()=ispw;
    
    if (cs().nTime(ispw)>0) {

      if (cs().nTime(ispw)>1)
	logSink() << " Spw " << ispw << " has a time-dep D solution." << endl;
      else {
    
	logSink() << " Spw " << ispw << ":" << endl;
	
	// References to ease access to solutions
	Array<Complex> sol(cs().par(ispw));
	Array<Bool> sok(cs().parOK(ispw));
	
	IPosition ip(4,0,0,0,0);

	logSink().output().setf(ios::left, ios::adjustfield);

	for (Int iant=0;iant<sol.shape()(2);++iant) {
	  ip(2)=ord(iant);
	  logSink() << "  Ant=" << antNames(ord(iant)) << ": ";
	  for (Int ipar=0;ipar<2;++ipar) {
	    logSink() << rl(ipar);
	    ip(0)=ipar;
	    if (sok(ip)) {
	      logSink() << "A="; 
	      logSink().output().width(10);
	      logSink() << abs(sol(ip));
	      logSink() << " P=";
	      logSink().output().width(8);
	      logSink() << arg(sol(ip))*180.0/C::pi;
	      if (ipar==0) logSink() << " ; ";
	    }
	    else {
	      logSink().output().width(26);
	      logSink() << "(flagged)" << " ";
	    }
	  } // ipol
	  logSink() << endl;
	} // iant
	logSink() << LogIO::POST;
      } // nTime==1
    } // nTime>0
  } // ispw

}



// Fill the trivial DJ matrix elements
void DJones::initTrivDJ() {

  if (prtlev()>4) cout << "   D::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  //  0 1     0 0
  //  0 0  &  1 0

  if (jonesType()==Jones::General) {
    diffJElem().resize(IPosition(4,4,2,1,1));
    diffJElem()=0.0;
    diffJElem()(IPosition(4,1,0,0,0))=Complex(1.0);
    diffJElem()(IPosition(4,2,1,0,0))=Complex(1.0);
  }
  else {
    diffJElem().resize(IPosition(4,2,2,1,1));
    diffJElem()=0.0;
    diffJElem()(IPosition(4,0,0,0,0))=Complex(1.0);
    diffJElem()(IPosition(4,1,1,0,0))=Complex(1.0);
  }

}


// **********************************************************
//  DfJones Implementations
//

DfJones::DfJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  DJones(vs)              // immediate parent
{
  if (prtlev()>2) cout << "Df::Df(vs)" << endl;
}

DfJones::DfJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  DJones(nAnt)
{
  if (prtlev()>2) cout << "Df::Df(nAnt)" << endl;
}

DfJones::~DfJones() {
  if (prtlev()>2) cout << "Df::~Df()" << endl;
}

// **********************************************************
//  JJones Implementations
//

JJones::JJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs)    // immediate parent
{
  if (prtlev()>2) cout << "J::J(vs)" << endl;
}

JJones::JJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt)
{
  if (prtlev()>2) cout << "J::J(nAnt)" << endl;
}

JJones::~JJones() {
  if (prtlev()>2) cout << "J::~J()" << endl;
}

void JJones::setSolve(const Record& solvepar) {

  // Call parent
  SolvableVisJones::setSolve(solvepar);

  // For J insist preavg is meaningful (5 minutes or user-supplied)
  if (preavg()<0.0)
    preavg()=300.0;

}


void JJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   ::guessPar(vb)" << endl;

  // Make a guess at antenna-based J
  //  Correlation membership-dependence
  //  assumes corrs in canonical order
  //  nCorr = 1: use icorr=0
  //  nCorr = 2: use icorr=[0,1]
  //  nCorr = 4: use icorr=[0,3]

  // This method sets the off-diag = 0.0,
  //  and the on-diag as if this were G

  Int nCorr(2);
  Int nDataCorr(vb.visCube().shape()(0));
  Vector<Int> corridx(nCorr,0);
  if (nDataCorr==2) {
    corridx(0)=0;
    corridx(1)=1;
  } 
  else if (nDataCorr==4) {
    corridx(0)=0;
    corridx(1)=3;
  }

  Cube<Complex>& V(vb.visCube());
  Float amp(0.0),ampave(0.0);
  Int namp(0);
  solveCPar()=Complex(0.0);
  for (Int irow=1;irow<nAnt();++irow) {

    for (Int icorr=0;icorr<nCorr;icorr++) {
      Complex& Vi(V(corridx(icorr),0,irow));
      amp=abs(Vi);
      if (amp>0.0f) {
	solveCPar()(3*icorr,0,irow)=(conj(Vi)/amp);
	ampave+=amp;
	namp++;
	//	cout << "          " << abs(Vi) << " " << arg(Vi)*180.0/C::pi << endl;
      }
    }

  }

  // Scale them by the mean amplitude
  ampave/=Float(namp);
  ampave=sqrt(ampave);
  solveCPar()*=Complex(ampave);
  solveParOK()=True;

  //  cout << "post-guess:" << endl;
  //  cout << "solveCPar()   = " << solveCPar() << endl;
  //  cout << "phases       = " << phase(solveCPar())*180.0/C::pi << endl;
  //  cout << "solveParOK() = " << solveParOK() << endl;

}

// Fill the trivial DJ matrix elements
void JJones::initTrivDJ() {

  if (prtlev()>4) cout << "   J::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  //  1 0     0 1     0 0     0 0
  //  0 0     0 0     1 0     0 1

  diffJElem().resize(IPosition(4,4,4,1,1));
  diffJElem()=0.0;
  diffJElem()(IPosition(4,0,0,0,0))=Complex(1.0);
  diffJElem()(IPosition(4,1,1,0,0))=Complex(1.0);
  diffJElem()(IPosition(4,2,2,0,0))=Complex(1.0);
  diffJElem()(IPosition(4,3,3,0,0))=Complex(1.0);

}

// **********************************************************
//  MMueller: baseline-based (closure) solution
//

MMueller::MMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisMueller(vs)    // immediate parent
{
  if (prtlev()>2) cout << "M::M(vs)" << endl;
}

MMueller::MMueller(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisMueller(nAnt)
{
  if (prtlev()>2) cout << "M::M(nAnt)" << endl;
}

MMueller::~MMueller() {
  if (prtlev()>2) cout << "M::~M()" << endl;
}

void MMueller::setApply(const Record& apply) {

  SolvableVisCal::setApply(apply);

  // Force calwt to False for now
  calWt()=False;

}

void MMueller::newselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  // Inform logger/history
  logSink() << "Solving for " << typeName()
            << LogIO::POST;

  // Initialize the svc according to current VisSet
  //  (this counts intervals, sizes CalSet)
  Vector<Int> nChunkPerSol;
  Int nSol = sizeUpSolve(vs,nChunkPerSol);
  
  // The iterator, VisBuffer
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);

  //  cout << "nSol = " << nSol << endl;
  //  cout << "nChunkPerSol = " << nChunkPerSol << endl;

  Vector<Int> slotidx(vs.numberSpw(),-1);

  Int nGood(0);
  vi.originChunks();
  for (Int isol=0;isol<nSol && vi.moreChunks();++isol) {

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);
    
    for (Int ichunk=0;ichunk<nChunkPerSol(isol);++ichunk) {

      // Current _chunk_'s spw
      Int spw(vi.spectralWindow());

      // Abort if we encounter a spw for which a priori cal not available
      if (!ve.spwOK(spw))
        throw(AipsError("Pre-applied calibration not available for at least 1 spw. Check spw selection carefully."));


      // Collapse each timestamp in this chunk according to VisEq
      //  with calibration and averaging
      for (vi.origin(); vi.more(); vi++) {

        // Force read of the field Id
        vb.fieldId();

	// Apply the channel mask
	this->applyChanMask(vb);

        // This forces the data/model/wt I/O, and applies
        //   any prior calibrations
        ve.collapse(vb);

        // If permitted/required by solvable component, normalize
        if (normalizable())
          vb.normalize();

        // Accumulate collapsed vb in a time average
        vba.accumulate(vb);
      }
      // Advance the VisIter, if possible
      if (vi.moreChunks()) vi.nextChunk();

    }

    // Finalize the averged VisBuffer
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Make data amp- or phase-only
    enforceAPonData(svb);

    // Establish meta-data for this interval
    //  (some of this may be used _during_ solve)
    //  (this sets currSpw() in the SVC)
    Bool vbOk=syncSolveMeta(svb,-1);

    Int thisSpw=spwMap()(svb.spectralWindow());
    slotidx(thisSpw)++;

    // Fill solveCPar() with 1, nominally, and flagged
    solveCPar()=Complex(1.0);
    solveParOK()=False;
    
    if (vbOk && svb.nRow()>0) {

      // Insist that channel,row shapes match
      IPosition visshape(svb.visCube().shape());
      AlwaysAssert(solveCPar().shape().getLast(2)==visshape.getLast(2),AipsError);
      
      // Zero flagged data
      IPosition vblc(3,0,0,0);
      IPosition vtrc(visshape);  vtrc-=1;      
      Int nCorr(visshape(0));
      for (Int i=0;i<nCorr;++i) {
	vblc(0)=vtrc(0)=i;
	svb.visCube()(vblc,vtrc).reform(visshape.getLast(2))(svb.flag())=Complex(1.0);
      }
      
      // Form correct slice of visCube to copy to solveCPar
      IPosition vcblc(3,0,0,0);
      IPosition vctrc(svb.visCube().shape()); vctrc-=1;
      IPosition vcstr(3,1,1,1);

      IPosition spblc(3,0,0,0);
      IPosition sptrc(solveCPar().shape()); sptrc-=1;

      IPosition flshape(svb.flag().shape());
      
      switch (nCorr) {
      case 1: {
	// fill 1st par only
	spblc(0)=sptrc(0)=0; 
	solveCPar()(spblc,sptrc)=svb.visCube();
	// first pol flags
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	break;
      }
      case 2: {
	// shapes match
	solveCPar()=svb.visCube();
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      case 4: {
	// Slice visCube with stride
	vcstr(0)=3;
	solveCPar()=svb.visCube()(vcblc,vctrc,vcstr);
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      default:
	throw(AipsError("Problem in MMueller::selfSolve."));
	break;
      }

      nGood++;
    }

    keep(slotidx(thisSpw));
    
  }
  
  logSink() << "  Found good "
            << typeName() << " solutions in "
            << nGood << " intervals."
            << LogIO::POST;

  // Store whole of result in a caltable
  if (nGood==0)
    logSink() << "No output calibration table written."
              << LogIO::POST;
  else {

    // Do global post-solve tinkering (e.g., normalization)
    globalPostSolveTinker();

    // write the table
    store();
  }

}

void MMueller::oldselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  // Arrange for iteration over data
  Block<Int> columns;
  if (interval()==0.0) {
    // include scan iteration
    // avoid scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }
  vs.resetVisIter(columns,interval());

  // Initial the solve (sets shapes)
  initSolve(vs);

  // Solve each solution interval (chunk)
  Vector<Int> islot(nSpw(),0);
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

    Int spw(vi.spectralWindow());

    // Abort if we encounter a spw for which a priori cal not available
    if (!ve.spwOK(spw))
      throw(AipsError("Pre-applied calibration not available for at least 1 spw. Check spw selection carefully."));

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);

    // Collapse each timestamp in this chunk according to VisEq
    //  with calibration and averaging

    for (vi.origin(); vi.more(); vi++) {

      ve.collapse(vb);

      vb.normalize();

      // Accumulate collapsed vb in a time average
      vba.accumulate(vb);
     
    }
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Extract meta data from visBuffer
    syncSolveMeta(svb,vi.fieldId());

    // Fill solveCPar() with 1, nominally, and flagged
    solveCPar()=Complex(1.0);
    solveParOK()=False;

    if (svb.nRow()>0) {

      // Insist that channel,row shapes match
      IPosition visshape(svb.visCube().shape());
      AlwaysAssert(solveCPar().shape().getLast(2)==visshape.getLast(2),AipsError);
      
      // Zero flagged data
      IPosition vblc(3,0,0,0);
      IPosition vtrc(visshape);  vtrc-=1;      
      Int nCorr(visshape(0));
      for (Int i=0;i<nCorr;++i) {
	vblc(0)=vtrc(0)=i;
	svb.visCube()(vblc,vtrc).reform(visshape.getLast(2))(svb.flag())=Complex(1.0);
      }
      
      // Form correct slice of visCube to copy to solveCPar
      IPosition vcblc(3,0,0,0);
      IPosition vctrc(svb.visCube().shape()); vctrc-=1;
      IPosition vcstr(3,1,1,1);

      IPosition spblc(3,0,0,0);
      IPosition sptrc(solveCPar().shape()); sptrc-=1;

      IPosition flshape(svb.flag().shape());
      
      switch (nCorr) {
      case 1: {
	// fill 1st par only
	spblc(0)=sptrc(0)=0; 
	solveCPar()(spblc,sptrc)=svb.visCube();
	// first pol flags
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	break;
      }
      case 2: {
	// shapes match
	solveCPar()=svb.visCube();
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      case 4: {
	// Slice visCube with stride
	vcstr(0)=3;
	solveCPar()=svb.visCube()(vcblc,vctrc,vcstr);
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      default:
	throw(AipsError("Problem in MMueller::selfSolve."));
	break;
      }
      
    }

    keep(islot(spw));

    islot(spw)++;

  }
  
  // Store it.
  store();

}

void MMueller::globalPostSolveTinker() {

  // normalize, if requested
  if (solnorm()) normalize();

}


// File a solved solution (and meta-data) into a slot in the CalSet
void MMueller::keep(const Int& slot) {

  if (prtlev()>4) cout << " M::keep(i)" << endl;

  if (slot<cs().nTime(currSpw())) {
    // An available valid slot

   
    //    cout << "Result: solveCPar() = " << solveCPar() << endl;

    //    cout << "   Amp: " << amplitude(solveCPar()) << endl;
    //    cout << " Phase: " << phase(solveCPar()/solveCPar()(0,0,0))*180.0/C::pi << endl;

    //    cout << "Result: solveParOK() = " << solveParOK() << endl;

    cs().fieldId(currSpw())(slot)=currField();
    cs().time(currSpw())(slot)=refTime();

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(currSpw())(slot)=0.0;
    cs().stopTime(currSpw())(slot)=interval();

    // For now, just make these non-zero:
    cs().iFit(currSpw()).column(slot)=1.0;
    cs().iFitwt(currSpw()).column(slot)=1.0;
    cs().fit(currSpw())(slot)=1.0;
    cs().fitwt(currSpw())(slot)=1.0;

    IPosition blc4(4,0,       0,           0,        slot);
    IPosition trc4(4,nPar()-1,nChanPar()-1,nElem()-1,slot);
    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();
    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();

    cs().solutionOK(currSpw())(slot) = anyEQ(solveParOK(),True);

  }
  else
    throw(AipsError("MMueller::keep: Attempt to store solution in non-existent CalSet slot"));

}


// **********************************************************
//  MfMueller: freq-dep MMueller
//

MfMueller::MfMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  MMueller(vs)            // immediate parent
{
  if (prtlev()>2) cout << "Mf::Mf(vs)" << endl;
}

MfMueller::MfMueller(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  MMueller(nAnt)
{
  if (prtlev()>2) cout << "Mf::Mf(nAnt)" << endl;
}

MfMueller::~MfMueller() {
  if (prtlev()>2) cout << "Mf::~Mf()" << endl;
}

void MfMueller::normalize() {

  // This is just like BJones

  logSink() << "Normalizing solutions per spw, pol, baseline, time"
            << LogIO::POST;

  // Iteration axes (norm per spw, pol, ant, timestamp)
  //  (this normalizes each baseline spectrum)
  IPosition itax(3,0,2,3);

  for (Int ispw=0;ispw<nSpw();++ispw)
    if (cs().nTime(ispw)>0) {
      ArrayIterator<Complex> soliter(cs().par(ispw),itax,False);
      ArrayIterator<Bool> okiter(cs().parOK(ispw),itax,False);
      while (!soliter.pastEnd()) {
        normSolnArray(soliter.array(),okiter.array(),True);
        soliter.next();
        okiter.next();
      }

    }

}



// **********************************************************
//  TOpac
//

TOpac::TOpac(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  TJones(vs),
  za_()
{
  if (prtlev()>2) cout << "TOpac::TOpac(vs)" << endl;
}

TOpac::~TOpac() {
  if (prtlev()>2) cout << "TOpac::~TOpac()" << endl;
}

void TOpac::setApply(const Record& applypar) {
  
  // TBD: Handle opacity table case properly
  // TBD: Handle spwmap properly  (opacity is not spw-dep)

  // TBD: Call parent?
  //  T::setApply(applypar);

  // This version uses user-supplied opacity value for all ants

  //  Double opacity(0.0);
  if (applypar.isDefined("opacity"))
    opacity_=applypar.asFloat("opacity");  
 
  Int oldspw; oldspw=currSpw();
  for (Int ispw=0;ispw<nSpw();++ispw) {
    currSpw()=ispw;
    currRPar().resize(1,1,nAnt());
    currRPar()=opacity_;
    currParOK().resize(1,1,nAnt());
    currParOK()=True;
  }
  currSpw()=oldspw;

  // Resize za()
  za().resize(nAnt());

}

String TOpac::applyinfo() {

  ostringstream o;
  o << typeName()
    << ": opacity=" << opacity_
    << boolalpha
    << " calWt=" << calWt();
    //    << " t="      << interval();

  return String(o);

}


void TOpac::calcPar() {

  if (prtlev()>6) cout << "      TOpac::calcPar()" << endl;

  // NB: z.a. calc here because it is needed only 
  //   if we have a new timestamp...
  //  (opacity parameter is already ok)

  za().resize(nAnt());
  Vector<MDirection> antazel(vb().azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant,++a) 
    (*a)=C::pi_2 - antazel(iant).getAngle().getValue()(1);

  // Pars now valid, matrices not yet
  validateP();
  invalidateJ();

}


void TOpac::calcAllJones() {

  if (prtlev()>6) cout << "       TOpac::calcAllJones()" << endl;

  // Nominally no opacity
  currJElem()=Complex(1.0);
  currJElemOK()=currParOK();

  Complex* J=currJElem().data();
  Float*  op=currRPar().data();
  Bool*   opok=currParOK().data();
  Double* a=za().data();
  for (Int iant=0; iant<nAnt(); ++iant,++J,++op,++opok,++a) {
    if ((*opok) && (*a)<C::pi_2) 
      (*J) = Complex(sqrt(exp(-1.0 * Double(*op)/cos(*a))));
  }

}


// **********************************************************
//  XMueller: positiona angle for circulars
//

XMueller::XMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisMueller(vs)    // immediate parent
{
  if (prtlev()>2) cout << "X::X(vs)" << endl;
}

XMueller::XMueller(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisMueller(nAnt)
{
  if (prtlev()>2) cout << "X::X(nAnt)" << endl;
}

XMueller::~XMueller() {
  if (prtlev()>2) cout << "X::~X()" << endl;
}

void XMueller::setApply(const Record& apply) {

  SolvableVisCal::setApply(apply);

  // Force calwt to False 
  calWt()=False;

}


void XMueller::setSolve(const Record& solvepar) {

  SolvableVisCal::setSolve(solvepar);

  // Force calwt to False 
  calWt()=False;

  // For X insist preavg is meaningful (5 minutes or user-supplied)
  if (preavg()<0.0)
    preavg()=300.0;

}

void XMueller::newselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  MeasurementSet ms(msName());
  MSFieldColumns msfldcol(ms.field());

  // Inform logger/history
  logSink() << "Solving for " << typeName()
            << LogIO::POST;

  // Initialize the svc according to current VisSet
  //  (this counts intervals, sizes CalSet)
  Vector<Int> nChunkPerSol;
  Int nSol = sizeUpSolve(vs,nChunkPerSol);
  
  // The iterator, VisBuffer
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);

  //  cout << "nSol = " << nSol << endl;
  //  cout << "nChunkPerSol = " << nChunkPerSol << endl;

  Vector<Int> slotidx(vs.numberSpw(),-1);

  Int nGood(0);
  vi.originChunks();
  for (Int isol=0;isol<nSol && vi.moreChunks();++isol) {

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);
    
    for (Int ichunk=0;ichunk<nChunkPerSol(isol);++ichunk) {

      // Current _chunk_'s spw
      Int spw(vi.spectralWindow());

      // Abort if we encounter a spw for which a priori cal not available
      if (!ve.spwOK(spw))
        throw(AipsError("Pre-applied calibration not available for at least 1 spw. Check spw selection carefully."));


      // Collapse each timestamp in this chunk according to VisEq
      //  with calibration and averaging
      for (vi.origin(); vi.more(); vi++) {

        // Force read of the field Id
        vb.fieldId();

        // This forces the data/model/wt I/O, and applies
        //   any prior calibrations
        ve.collapse(vb);

        // If permitted/required by solvable component, normalize
        //if (normalizable())
	//          vb.normalize();

        // Accumulate collapsed vb in a time average
        vba.accumulate(vb);
      }
      // Advance the VisIter, if possible
      if (vi.moreChunks()) vi.nextChunk();

    }

    // Finalize the averged VisBuffer
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Establish meta-data for this interval
    //  (some of this may be used _during_ solve)
    //  (this sets currSpw() in the SVC)
    Bool vbOk=syncSolveMeta(svb,-1);

    Int thisSpw=spwMap()(svb.spectralWindow());
    slotidx(thisSpw)++;

    // Fill solveCPar() with 1, nominally, and flagged
    // TBD: drop unneeded basline-dependence    
    solveCPar()=Complex(1.0);
    solveParOK()=False;
    
    if (vbOk && svb.nRow()>0) {

      // solve for the R-L phase term in the current VB
      solveOneVB(svb);

      if (solveParOK()(0,0,0))
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") = "
		  << arg(solveCPar()(0,0,0))*180.0/C::pi/2.0
		  << " deg."
		  << LogIO::POST;
      else
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") "
		  << " was not determined (insufficient data)."
		  << LogIO::POST;
	
      nGood++;
    }

    keep(slotidx(thisSpw));
    
  }
  
  logSink() << "  Found good "
            << typeName() << " solutions in "
            << nGood << " intervals."
            << LogIO::POST;

  // Store whole of result in a caltable
  if (nGood==0)
    logSink() << "No output calibration table written."
              << LogIO::POST;
  else {

    // Do global post-solve tinkering (e.g., phase-only, normalization, etc.)
    //  TBD
    // globalPostSolveTinker();

    // write the table
    store();
  }

}

void XMueller::oldselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  MeasurementSet ms(msName());
  MSFieldColumns msfldcol(ms.field());

  // Arrange for iteration over data
  Block<Int> columns;
  if (interval()==0.0) {
    // include scan iteration
    // avoid scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }
  vs.resetVisIter(columns,interval());

  // Initial the solve (sets shapes)
  initSolve(vs);

  // Solve each solution interval (chunk)
  Vector<Int> islot(nSpw(),0);
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

    Int spw(vi.spectralWindow());
    //      cout << "Spw=" << spw << " slot=" << islot(spw) << " field="
    //           << vi.fieldId() << " " << MVTime(vb.time()(0)/86400.0) << " -------------------" << endl;

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);

    // Collapse each timestamp in this chunk according to VisEq
    //  with calibration and averaging

    for (vi.origin(); vi.more(); vi++) {

      ve.collapse(vb);

      //      vb.normalize();

      // Accumulate collapsed vb in a time average
      vba.accumulate(vb);
     
    }
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Extract meta data from visBuffer
    syncSolveMeta(svb,vi.fieldId());

    // Fill solveCPar() with 1, nominally, and flagged
    // TBD: drop unneeded basline-dependence    
    solveCPar()=Complex(1.0);
    solveParOK()=False;

    if (svb.nRow()>0) {

      // solve for the R-L phase term in the current VB
      solveOneVB(svb);

      if (solveParOK()(0,0,0))
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") = "
		  << arg(solveCPar()(0,0,0))*180.0/C::pi/2.0
		  << " deg."
		  << LogIO::POST;
      else
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") "
		  << " was not determined (insufficient data)."
		  << LogIO::POST;
	
    }

    keep(islot(spw));

    islot(spw)++;

  }
  
  // Store it.
  store();

}

// File a solved solution (and meta-data) into a slot in the CalSet
void XMueller::keep(const Int& slot) {

  if (prtlev()>4) cout << " M::keep(i)" << endl;

  if (slot<cs().nTime(currSpw())) {
    // An available valid slot

   
    //    cout << "Result: solveCPar() = " << solveCPar() << endl;

    //    cout << "   Amp: " << amplitude(solveCPar()) << endl;
    //    cout << " Phase: " << phase(solveCPar()/solveCPar()(0,0,0))*180.0/C::pi << endl;

    //    cout << "Result: solveParOK() = " << solveParOK() << endl;

    cs().fieldId(currSpw())(slot)=currField();
    cs().time(currSpw())(slot)=refTime();

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(currSpw())(slot)=0.0;
    cs().stopTime(currSpw())(slot)=interval();

    // For now, just make these non-zero:
    cs().iFit(currSpw()).column(slot)=1.0;
    cs().iFitwt(currSpw()).column(slot)=1.0;
    cs().fit(currSpw())(slot)=1.0;
    cs().fitwt(currSpw())(slot)=1.0;

    IPosition blc4(4,0,       0,           0,        slot);
    IPosition trc4(4,nPar()-1,nChanPar()-1,nElem()-1,slot);
    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();
    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();

    cs().solutionOK(currSpw())(slot) = anyEQ(solveParOK(),True);

  }
  else
    throw(AipsError("XMueller::keep: Attempt to store solution in non-existent CalSet slot"));

}

void XMueller::calcAllMueller() {

  //  cout << "currMElem().shape() = " << currMElem().shape() << endl;

  // Put the phase factor into the cross-hand diagonals
  //  (1,0) for the para-hands  
  IPosition blc(3,0,0,0);
  IPosition trc(3,0,nChanMat()-1,nElem()-1);
  currMElem()(blc,trc)=Complex(1.0);

  blc(0)=trc(0)=1;
  currMElem()(blc,trc)=currCPar()(0,0,0);
  blc(0)=trc(0)=2;
  currMElem()(blc,trc)=conj(currCPar()(0,0,0));

  blc(0)=trc(0)=3;
  currMElem()(blc,trc)=Complex(1.0);

  currMElemOK()=True;

}


void XMueller::solveOneVB(const VisBuffer& vb) {

  // This just a simple average of the cross-hand
  //  visbilities...

  Complex d,md;
  Float wt,a;
  DComplex rl(0.0),lr(0.0);
  Double sumwt(0.0);
  for (Int irow=0;irow<vb.nRow();++irow) {
    if (!vb.flagRow()(irow) &&
	vb.antenna1()(irow)!=vb.antenna2()(irow)) {

      for (Int ich=0;ich<vb.nChannel();++ich) {
	if (!vb.flag()(ich,irow)) {
	  
	  // A common weight for both crosshands
	  // TBD: we should probably consider this carefully...
	  //  (also in D::guessPar...)
	  wt=Double(vb.weightMat()(1,irow)+
		    vb.weightMat()(2,irow))/2.0;

	  // correct weight for model normalization
	  a=abs(vb.modelVisCube()(1,ich,irow));
	  wt*=(a*a);
	  
	  if (wt>0.0) {
	    // Cross-hands only
	    for (Int icorr=1;icorr<3;++icorr) {
	      md=vb.modelVisCube()(icorr,ich,irow);
	      d=vb.visCube()(icorr,ich,irow);
	      
	      if (abs(d)>0.0) {
		
		if (icorr==1) 
		  rl+=DComplex(Complex(wt)*d/md);
		else
		  lr+=DComplex(Complex(wt)*d/md);
		
		sumwt+=Double(wt);
		
	      } // abs(d)>0
	    } // icorr
	  } // wt>0
	} // !flag
      } // ich
    } // !flagRow
  } // row
  
/*
  cout << "spw = " << currSpw() << endl;
  cout << " rl = " << rl << " " << arg(rl)*180.0/C::pi << endl;
  cout << " lr = " << lr << " " << arg(lr)*180.0/C::pi << endl;
*/

    // combine lr with rl
  rl+=conj(lr);
  
  // Normalize to unit amplitude
  //  (note that the phase result is insensitive to sumwt)
  Double amp=abs(rl);
  if (sumwt>0 && amp>0.0) {
    rl/=DComplex(amp);
    
    solveCPar()=Complex(rl);
    solveParOK()=True;
  }
  
}


} //# NAMESPACE CASA - END

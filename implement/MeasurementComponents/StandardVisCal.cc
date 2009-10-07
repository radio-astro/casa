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
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
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
  prtlev()=4; // debug

  if (prtlev()>2) cout << "   T::setupSim()" << endl;

  // This method only called in simulate context!
  AlwaysAssert((isSimulated()),AipsError);

  // this is done in sizeUpSim, and if VS goes away we'll probably need to 
  // be passing either the ms or the vi down from Calibrator and Simulator
  // into here and then on to sizeUpSim.
  // right now, I want vi to get vi.msColumns()
  // I'm assuming that there's only one ms attached to the VI *
  VisIter& vi(vs.iter());
  // VisBuffer vb(vi);
  
  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);

  if (prtlev()>3) cout << " sized for Sim." << endl;

  // we can use the private access directly the corruptor
  tcorruptor_p = new TJonesCorruptor(nSim);
  // but set the public one 
  corruptor_p = tcorruptor_p;

  if (prtlev()>3) cout << " TCorruptor created." << endl;

  corruptor_p->startTime()=min(solTimes);
  corruptor_p->stopTime()=max(solTimes);
  corruptor_p->prtlev()=prtlev();

  Int Seed(1234);
  if (simpar.isDefined("seed")) {    
    Seed=simpar.asInt("seed");
  }

  Float Beta(1.1); // exponent for generalized 1/f noise
  if (simpar.isDefined("beta")) {    
    Beta=simpar.asFloat("beta");
  }
  
  if (simpar.isDefined("mean_pwv"))
    tcorruptor_p->mean_pwv() = simpar.asFloat("mean_pwv");
  
  if (tcorruptor_p->mean_pwv()<=0)
    throw(AipsError("TCorruptor attempted initialization with undefined PWV"));

  Float Scale(.15); // scale of fluctuations rel to mean
  if (simpar.isDefined("delta_pwv") and simpar.isDefined("mean_pwv")) {    
    Scale=simpar.asFloat("delta_pwv")/simpar.asFloat("mean_pwv");
    if (Scale>.5)
      Scale=.5;  // RI_TODO warn when doing this!!!
  }


  // initialize spw etc information in Corruptor
  // the "initialize" function that's corruptor-specific calculates the 
  // corruption values
  // this preparation is just to tell the corruptor about its VC - maybe 
  // the logic should be changed so the corruptor doesn't have to know 
  // all this?

  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();

  if (prtlev()>3) cout << " SpwCols accessed:" << endl;
  if (prtlev()>3) cout << "   nSpw()= " << nSpw() << endl;
  if (prtlev()>3) cout << "   spwcols.nrow()= " << nSpw() << endl;

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

  if (simpar.isDefined("mode")) {    
    if (prtlev()>2)
      cout << "initializing TCorruptor with mode " << simpar.asString("mode") << endl;

    if (simpar.asString("mode") == "test")
      tcorruptor_p->initialize();
    else if (simpar.asString("mode") == "individual") 
      tcorruptor_p->initialize(Seed,Beta,Scale);
    else if (simpar.asString("mode") == "screen") {
      const ROMSAntennaColumns& antcols(vi.msColumns().antenna());
      // moved into Corruptor:
      // const ROMSObservationColumns& obscols(vi.msColumns().observation());
      // const Float tracklength = max(obscols.timeRange().getColumn())-min(obscols.timeRange().getColumn());
      if (simpar.isDefined("windspeed")) {
	tcorruptor_p->windspeed()=simpar.asFloat("windspeed");
	tcorruptor_p->initialize(Seed,Beta,Scale,antcols);
      } else
	throw(AipsError("Unknown wind speed for TJonesCorruptor"));        
    } else 
      throw(AipsError("Unknown Mode for TJonesCorruptor"));        
  } else {
    throw(AipsError("No Mode specified for TJones corruptor."));
  }
  
  return nSim;
}



Bool TJones::simPar(VisIter& vi, const Int nChunks){

  LogIO os(LogOrigin("T", "simPar()", WHERE));

  if (prtlev()>4) cout << "   T::simPar()" << endl;

  AlwaysAssert((isSimulated()),AipsError);
  
  // sizeUpSolve did this (Mueller):
  // solveCPar().resize(nPar(),nChanPar(),nBln());
  // or this (Jones):
  // solveCPar().resize(nPar(),1,nAnt());

  // loop through correlations or use corridx ?

  try {
    
    VisBuffer cvb(vi);   
    Vector<Int>& a1(cvb.antenna1());
    Vector<Int>& a2(cvb.antenna2());
    corruptor_p->currSpw()=cvb.spectralWindow(); 
    
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
		if (prtlev()>3) 
		  cout << "    T:simPar retreating to time " << refTime() << endl;
	      }
	    }	  
	  } else {	      
	    for (Int newslot=corruptor_p->curr_slot()+1;newslot<corruptor_p->nSim();newslot++) {
	      dt=abs(corruptor_p->slot_time(newslot) - refTime());
	      if (dt<dt0) {
		// use this check for on-demand corruptors to get new val.
		corruptor_p->curr_slot()=newslot;
		dt0 = dt;
		if (prtlev()>4) 
		  cout << "    T:simPar advancing to time " << refTime() << endl;
	      }	  
	    }
	  }
	}
	
	corruptor_p->currAnt()=a1(irow);
	
	if(tcorruptor_p->mode()=="test" or tcorruptor_p->mode()=="1d")
	  solveCPar()(0,focusChan(),a1(irow))=tcorruptor_p->gain(focusChan());
	else if (tcorruptor_p->mode()=="2d") {
	  // RI_TODO modify x,y by tan(zenith angle)*(layer altitude)
	  Int ix(Int(tcorruptor_p->antx()[a1(irow)]));
	  Int iy(Int(tcorruptor_p->anty()[a1(irow)]));
	  if (prtlev()>4) 
	    cout << " getting gain for antenna ix,iy = " << ix << "," << iy << endl;  
	  solveCPar()(0,focusChan(),a1(irow))=tcorruptor_p->gain(ix,iy,focusChan());
	} else 
	  throw(AipsError("T: unknown corruptor mode "+tcorruptor_p->mode()));
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








void TJonesCorruptor::initialize() {
  // for testing only

  mode()="test";
  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

  initAtm();
  pwv_p.resize(nAnt(),False,True);
  for (Int ia=0;ia<nAnt();++ia) {
    pwv_p[ia] = new Vector<Float>(nSim());
    // not really pwv, but this is a test mode
    for (Int i=0;i<nSim();++i) 
      (*(pwv_p[ia]))(i) = (Float(i)/Float(nSim()) + Float(ia)/Float(nAnt()))*mean_pwv()*10;  
  }

  initialized()=True;
  if (prtlev()>2) cout << "TCorruptor::init [test]" << endl;
}




void TJonesCorruptor::initAtm() {

#ifndef CASA_STANDALONE
  atm::Temperature  T( 270.0,"K" );   // Ground temperature
  atm::Pressure     P( 560.0,"mb");   // Ground Pressure
  atm::Humidity     H(  20,"%" );     // Ground Relative Humidity (indication)
  atm::Length       Alt(  5000,"m" ); // Altitude of the site 
  atm::Length       WVL(   2.0,"km"); // Water vapor scale height

  // RI todo get Alt from observatory info in Simulator

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

  // first SpW:
  double fRes(fWidth()[0]/fnChan()[0]);
  itsSpecGrid = new atm::SpectralGrid(fnChan()[0],0, 
				      atm::Frequency(fRefFreq()[0],"Hz"),
				      atm::Frequency(fRes,"Hz"));
  // any more?
  for (Int ispw=1;ispw<nSpw();ispw++) {
    fRes = fWidth()[ispw]/fnChan()[ispw];
    itsSpecGrid->add(fnChan()[ispw],0,
		     atm::Frequency(fRefFreq()[ispw],"Hz"),
		     atm::Frequency(fRes,"Hz"));
  }

  itsRIP = new atm::RefractiveIndexProfile(*itsSpecGrid,*itsatm);
  
  if (prtlev()>2) cout << "TCorruptor::getDispersiveWetPathLength = " 
		       << itsRIP->getDispersiveWetPathLength().get("micron") 
		       << " microns at " 
		       << fRefFreq()[0]/1e9 << " GHz" << endl;

  //  itsSkyStatus = new atm::SkyStatus(*itsRIP);

#endif
}




void TJonesCorruptor::initialize(const Int Seed, const Float Beta, const Float scale) {
  // individual delays for each antenna

  initAtm();

  mode()="1d";
  fBM* myfbm = new fBM(nSim());
  pwv_p.resize(nAnt(),False,True);
  for (Int iant=0;iant<nAnt();++iant){
    pwv_p[iant] = new Vector<Float>(nSim());
    myfbm->initialize(Seed+iant,Beta); // (re)initialize
    *(pwv_p[iant]) = myfbm->data(); // iAnt()=iant; delay() = myfbm->data();
    Float pmean = mean(*(pwv_p[iant]));
    Float rms = sqrt(mean( (*(pwv_p[iant])-pmean)*(*(pwv_p[iant])-pmean) ));
    if (prtlev()>2 and currAnt()<2) {
      cout << "RMS fBM fluctuation for antenna " << iant 
	   << " = " << rms << " ( " << pmean << " ; beta = " << Beta << " ) " << endl;      
    }
    // scale is set above to delta/meanpwv
    // Float lscale = log(scale)/rms;
    for (Int islot=0;islot<nSim();++islot)
      (*(pwv_p[iant]))[islot] = (*(pwv_p[iant]))[islot]*scale/rms;  
    if (prtlev()>2 and currAnt()<2) {
      Float pmean = mean(*(pwv_p[iant]));
      Float rms = sqrt(mean( (*(pwv_p[iant])-pmean)*(*(pwv_p[iant])-pmean) ));
      cout << "RMS fractional fluctuation for antenna " << iant 
	   << " = " << rms << " ( " << pmean << " ) " 
	// << " lscale = " << lscale 
	   << endl;      
    }
    currAnt()=iant;
  }

  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

  initialized()=True;
  if (prtlev()>2) cout << "TCorruptor::init [1d]" << endl;

}



  
  void TJonesCorruptor::initialize(const Int Seed, const Float Beta, const Float scale, const ROMSAntennaColumns& antcols) {
  // 2d delay screen

  initAtm();

  mode()="2d";
  
  // RI_TODO calc xsize ysize from windspeed, track length, & array size
      
  // figure out where the antennas are, for blowing a phase screen over them
  // and how big they are, to set the pixel scale of the screen
  Float mindiam = min(antcols.dishDiameter().getColumn()); // units? dDQuant()?
  pixsize() = 0.5*mindiam; // RI_TODO temp compensate for lack of screen interpolation
  nAnt()=antcols.nrow();
  antx().resize(nAnt());
  anty().resize(nAnt());
  MVPosition ant;
  for (Int i=0;i<nAnt();i++) {	
    ant = antcols.positionMeas()(i).getValue();
    // have to convert to ENU or WGS84
    // ant = MPosition::Convert(ant,MPosition::WGS84)().getValue();
    // RI_TODO do this projection properly
    antx()[i] = ant.getLong()*6371000.;
    anty()[i] = ant.getLat()*6371000.; // m
  }     
  // from SDTableIterator
  //// but this expects ITRF XYZ, so make a Position and convert
  //obsPos = MPosition(Quantity(siteElev_.asdouble(thisRow_), "m"),
  //			 Quantity(siteLong_.asdouble(thisRow_), "deg"),
  //			 Quantity(siteLat_.asdouble(thisRow_), "deg"),
  //			 MPosition::WGS84);
  //obsPos = MPosition::Convert(obsPos, MPosition::ITRF)();
  Float meanlat=mean(anty())/6371000.;
  antx()=antx()*cos(meanlat);
  if (prtlev()>4) 
    cout << antx() << endl << anty() << endl;
  Int buffer(2); // # pix border
  //antx()=antx()-mean(antx());
  //anty()=anty()-mean(anty());
  antx()=antx()-min(antx());
  anty()=anty()-min(anty());
  antx()=antx()/pixsize();
  anty()=anty()/pixsize();
  if (prtlev()>3) 
    cout << antx() << endl << anty() << endl;

  Int ysize(Int(ceil(max(anty())+buffer)));

  const Float tracklength = stopTime()-startTime();    
  const Float blowlength = windspeed()*tracklength*1.05; // 5% margin
  if (prtlev()>2) 
    cout << "blowlength: " << blowlength << " track time = " << tracklength << endl;
  
  Int xsize(Int(ceil(max(antx())+buffer+blowlength/pixsize()))); 

  if (prtlev()>2) 
    cout << "xy screen size = " << xsize << "," << ysize << 
      " pixels (" << pixsize() << "m)" << endl;
  // if the array is too elongated, FFT sometimes gets upset;
  if (Float(xsize)/Float(ysize)>5) ysize=xsize/5;
  
  if (prtlev()>3) 
    cout << "new fBM of size " << xsize << "," << ysize << endl;
  fBM* myfbm = new fBM(xsize,ysize);
  screen_p = new Matrix<Float>(xsize,ysize);
  myfbm->initialize(Seed,Beta); 
  *screen_p=myfbm->data();

  Float pmean = mean(*screen_p);
  Float rms = sqrt(mean( ((*screen_p)-pmean)*((*screen_p)-pmean) ));
  // if (prtlev()>4) cout << (*screen_p)[10] << endl;
  if (prtlev()>2 and currAnt()<2) {
    cout << "RMS screen fluctuation " 
	 << " = " << rms << " ( " << pmean << " ; beta = " << Beta << " ) " << endl;
  }
  // scale is set above to delta/meanpwv
  *screen_p = myfbm->data() * scale/rms;

  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

  initialized()=True;
  if (prtlev()>2) cout << "TCorruptor::init [2d]" << endl;

}





Complex TJonesCorruptor::gain(const Int ix, const Int iy, const Int ichan) {
  // expects pixel positions in screen - already converted using the pixscale
  // of the screen, and modified for off-zenith pointing

  AlwaysAssert(mode()=="2d",AipsError);
  Float delay;
  ostringstream o; 
 
  if (curr_slot()>=0 and curr_slot()<nSim()) {
    // blow
    Int blown(Int(floor( (slot_time(curr_slot())-slot_time(0)) *
			 windspeed()/pixsize() ))); 
    if (prtlev()>4 and currAnt()<2) cout << "blown " << blown << endl;

    if ((ix+blown)>(screen_p->shape())[0]) {
      o << "Delay screen blown out of range (" << ix << "+" 
	<< blown << "," << iy << ") (" << screen_p->shape() << ")" << endl;
      throw(AipsError(o));
    }
    // RI TODO interpolate!
    Float deltapwv = (*screen_p)(ix+blown,iy);
    delay = itsRIP->getDispersiveWetPhaseDelay(currSpw(),ichan).get("rad") 
      * deltapwv / 57.2958; // convert from deg to rad
    return Complex(cos(delay),sin(delay));
  } else {    
    o << "TCorruptor::gain: slot " << curr_slot() << "out of range!" <<endl;
    throw(AipsError(o));
    return Complex(1.);
  }
}


Complex TJonesCorruptor::gain(const Int ichan) {
  AlwaysAssert(mode()=="1d" or mode()=="test",AipsError);
  Float delay;
  
  if (curr_slot()>=0 and curr_slot()<nSim()) {
    // Float freq = fRefFreq()[currSpw()] + 
    //   Float(ichan) * (fWidth()[currSpw()]/Float(fnChan()[currSpw()]));
    
    if (currAnt()<=pwv_p.nelements()) {
      Float deltapwv = (*pwv_p[currAnt()])(curr_slot());
      delay = itsRIP->getDispersiveWetPhaseDelay(currSpw(),ichan).get("rad") 
	* deltapwv / 57.2958; // convert from deg to rad
      //      cout << itsRIP->getDispersiveWetPhaseDelay(0,ichan).get("rad") << " "
      //	   << itsRIP->getDispersiveWetPhaseDelay(1,ichan).get("rad") << " "
      //	   << itsRIP->getDispersiveWetPhaseDelay(2,ichan).get("rad") << " "
      //	   << itsRIP->getDispersiveWetPhaseDelay(3,ichan).get("rad") << endl;
	
    } else
      throw(AipsError("TJonesCorruptor internal error accessing pwv()"));  
    return Complex(cos(delay),sin(delay));
  } else {
    cout << "TCorruptor::gain: slot " << curr_slot() << "out of range!" <<endl;
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
  // This class assumes that a Complex array is stored as
  // pairs of floating point numbers, with no intervening gaps, 
  // and with the real component first ie., 
  // <src>[re0,im0,re1,im1, ...]</src>. This means that the 
  // following type casts work,
  // <srcblock>
  // S * complexPtr;
  // T * realPtr = (T * ) complexPtr;
  // </srcblock>

  Int stemp(s(0));
  if (ndim>1)
    stemp=s(1);

  IPosition size(1,s(0));
  IPosition size2(2,s(0),stemp);
  // takes a lot of thread thrashing to resize the server but I can't
  // figure a great way around the scope issues to just define a 2d one
  // right off the bat
  FFTServer<Float,Complex> server(size);
  
  Vector<Complex> F(s(0)/2);
  Vector<Float> G; // size zero to let FFTServer calc right size  

  Matrix<Complex> F2(s(0)/2,stemp/2);
  Matrix<Float> G2;
  
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
    // Will the server resize itself?
    server.resize(size2);
    // RI_TODO make sure the hermitian business is correct - fftw only doubles
    // the first axis...
    // F2.resize(s(0)/2,s(1)/2);
    F2.resize(s(0)/2,s(1));
    for (uInt i=0; i<s(0)/2; i++)
      // for (uInt j=0; j<s(1)/2; j++) {
      for (uInt j=0; j<s(1); j++) {
	phase = 2.*pi*uDist_p(); 	  
	// RI TODO is this assuming the origin is at 0,0 in which case 
	// we should be using FFTServer::fft0 ? 
	if (i!=0 or j!=0) {
	  Float ij2 = sqrt(Float(i)*Float(i) + Float(j)*Float(j));
	  // RI_TODO still something not quite right with exponent
	  // amp = pow(ij2, -0.25*(beta+1)) * nDist_p();
	  amp = pow(ij2, -0.5*(beta+0.5) ) * nDist_p();
	} else {
	  amp = 0.;
	}
	F2(i,j)=Complex(amp*cos(phase),amp*sin(phase));
	// if (i==0) {

	//   i0=0;
	// } else {
	//   i0=s(0)-i;
	// }
	// do we need this ourselves in the second dimension?
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
    // cout << endl;
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
    // G2 comes out sized s(0),s(1)/2 i.e. only doubles the first dim.
    // cout << G2.shape() << endl;  
    // there has to be a better way
    for (uInt i=0; i<s(0); i++)
      for (uInt j=0; j<s(1); j++) 
	data_->operator()(IPosition(2,i,j)) = G2(i,j);       
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
  SolvableVisJones(vs),    // immediate parent
  gcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "G::G(vs)" << endl;
}

GJones::GJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt),
  gcorruptor_p(NULL)
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


Int GJones::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{
  prtlev()=4; // debug

  if (prtlev()>2) cout << "   G::setupSim()" << endl;
  AlwaysAssert((isSimulated()),AipsError);
  VisIter& vi(vs.iter());
  
  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);
  if (prtlev()>3) cout << " sized for Sim." << endl;

  gcorruptor_p = new GJonesCorruptor(nSim);
  corruptor_p = gcorruptor_p;
  if (prtlev()>3) cout << " GCorruptor created." << endl;

  //Vector<Int> corrType; vi.corrType(corrType);
  //uInt nCorr = corrType.nelements();
  corruptor_p->nCorr()=vi.nCorr();
  cout << "nCorr= " << vi.nCorr() <<endl;
      
  corruptor_p->startTime()=min(solTimes);
  corruptor_p->stopTime()=max(solTimes);
  corruptor_p->prtlev()=prtlev();

  Int Seed(1234);
  if (simpar.isDefined("seed")) {    
    Seed=simpar.asInt("seed");
  }

  Float Beta(1.1); // exponent for generalized 1/f noise
  if (simpar.isDefined("beta")) {    
    Beta=simpar.asFloat("beta");
  }
  
  if (simpar.isDefined("tsys")) {
    gcorruptor_p->tsys() = simpar.asFloat("tsys");
  } 
  
  Float Scale(.15); // scale of fluctuations rel to mean
  if (simpar.isDefined("scale")) {
    Scale=simpar.asFloat("scale");
    if (Scale>.5)
      Scale=.5;  // RI_TODO warn when doing this!!!
  }

  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();

  if (prtlev()>3) cout << " SpwCols accessed:" << endl;
  if (prtlev()>3) cout << "   nSpw()= " << nSpw() << endl;
  if (prtlev()>3) cout << "   spwcols.nrow()= " << nSpw() << endl;

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

  gcorruptor_p->initialize(Seed,Beta,Scale);
  
  if (prtlev()>2) cout << "   G::setupSim()  nSim = " << nSim << endl;
  return nSim;
}



Bool GJones::simPar(VisIter& vi, const Int nChunks){
  LogIO os(LogOrigin("G", "simPar()", WHERE));
  if (prtlev()>4) cout << "   G::simPar()" << endl;
  
  AlwaysAssert((isSimulated()),AipsError);
  try {
    VisBuffer cvb(vi);
    Vector<Int>& a1(cvb.antenna1());
    Vector<Int>& a2(cvb.antenna2());
    Vector<Int>& feed1(cvb.feed1());
    corruptor_p->currSpw()=cvb.spectralWindow(); 
    
    cout << " ant1=" << min(a1) << "-" << max(a1) << 
      " spw=" << cvb.spectralWindow() << 
      " feed=" << min(feed1) << "-" << max(feed1) << 
      " nrow=" << cvb.nRow() << 
      "(" << min(cvb.rowIds()) << "-" << max(cvb.rowIds()) << ") " << endl;
    
    for (Int irow=0;irow<cvb.nRow();++irow) {
      if (prtlev()>4 and !cvb.flagRow()(irow) and a1(irow)==49)
	cout << " G::simPar(): ant=" << cvb.antenna1()(irow) << " +" << cvb.antenna2()(irow) << 
	  " spw=" << cvb.spectralWindow() << 
	  " feed=" << cvb.feed1()(irow) << 
	  " row=" << irow << 
	  "(" << cvb.rowIds()(irow) << ") " <<
	  cvb.flagRow()(irow) << endl;
      
      if ( !cvb.flagRow()(irow) &&
	   cvb.antenna1()(irow)!=cvb.antenna2()(irow) &&
	   nfalse(cvb.flag().column(irow))> 0 ) { 
	
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
		if (prtlev()>3) 
		  cout << "    G:simPar retreating to time " << refTime() << endl;
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
		  cout << "    G:simPar advancing to time " << refTime() << endl;
	      }         
	    }
	  }
	}
	corruptor_p->currAnt()=a1(irow);
        
	for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) 
	  solveCPar()(icorr,focusChan(),a1(irow))=gcorruptor_p->gain(icorr,focusChan());
      }
    }
  } catch (AipsError x) {
    if (gcorruptor_p) delete gcorruptor_p;
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
  return True;
}




GJonesCorruptor::GJonesCorruptor(const Int nSim) : 
  CalCorruptor(nSim),  // parent
  tsys_(0.0)
{}

GJonesCorruptor::~GJonesCorruptor() {
  if (prtlev()>2) cout << "GCorruptor::~GCorruptor()" << endl;
}



Matrix<Complex>* GJonesCorruptor::drift() { 
  if (currAnt()<=drift_p.nelements())
    return drift_p[currAnt()];
  else
    return NULL;
};
 
//Complex& GJonesCorruptor::drift(const Int islot) { 
//  if (currAnt()<=drift_p.nelements())
//    return (*drift_p[currAnt()])(islot);
//  else
//    throw(AipsError("GJonesCorruptor internal error accessing drift()"));;
//};


void GJonesCorruptor::initialize() {
  // for testing only
  if (prtlev()>2) cout << "GCorruptor::init [test]" << endl;
}

void GJonesCorruptor::initialize(const Int Seed, const Float Beta, const Float scale) {
  // individual delays for each antenna

  fBM* myfbm = new fBM(nSim());
  drift_p.resize(nAnt(),False,True);
  for (Int iant=0;iant<nAnt();++iant) {
    drift_p[iant] = new Matrix<Complex>(nCorr(),nSim());
    for (Int icorr=0;icorr<nCorr();++icorr){
      myfbm->initialize(Seed+iant+icorr,Beta); // (re)initialize
      Float pmean = mean(myfbm->data());
      Float rms = sqrt(mean( ((myfbm->data())-pmean)*((myfbm->data())-pmean) ));
      if (prtlev()>2 and currAnt()<2) {
	cout << "RMS fBM fluctuation for antenna " << iant 
	     << " = " << rms << " ( " << pmean << " ; beta = " << Beta << " ) " << endl;      
      }
      // amp
      Vector<Float> amp = (myfbm->data()) * scale/rms;
      for (Int i=0;i<nSim();++i)  {
	(*(drift_p[iant]))(icorr,i) = Complex(1.+amp[i],0);
      }
      
      // phase
      myfbm->initialize((Seed+iant+icorr)*100,Beta); // (re)initialize
      pmean = mean(myfbm->data());
      rms = sqrt(mean( ((myfbm->data())-pmean)*((myfbm->data())-pmean) ));
      Vector<Float> angle = (myfbm->data()) * scale/rms * 3.141592; // *2 ?
      for (Int i=0;i<nSim();++i)  {
	(*(drift_p[iant]))(icorr,i) *= exp(Complex(0,angle[i]));
      }

      currAnt()=iant;
    }
  }

  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

  initialized()=True;
  if (prtlev()>2) cout << "GCorruptor::init" << endl;

}


Complex GJonesCorruptor::gain(const Int icorr,const Int ichan) {
  if (curr_slot()>=0 and curr_slot()<nSim() and icorr>=0 and icorr<nCorr()) {    
    if (currAnt()>drift_p.nelements())
      throw(AipsError("GJonesCorruptor internal error accessing drift()"));  
    Complex delta = (*drift_p[currAnt()])(icorr,curr_slot());    
    return delta;
  } else {
    cout << "GCorruptor::gain: slot " << curr_slot() << "out of range!" <<endl;
    return Complex(1.);
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


  ve.state();

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

	// If this solve not freqdep, and channels not averaged yet, do so
	if (!freqDepMat() && vb.nChannel()>1) 
	  vb.freqAveCubes();

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

      // If this solve not freqdep, and channels not averaged yet, do so
      if (!freqDepMat() && vb.nChannel()>1)
	vb.freqAveCubes();

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


Int MMueller::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{
  prtlev()=4; // debug

  if (prtlev()>2) cout << "   MM::setupSim()" << endl;

  // This method only called in simulate context!
  AlwaysAssert((isSimulated()),AipsError);

  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);

  if (prtlev()>3) cout << " sized for Sim." << endl;

  mcorruptor_p = new MMCorruptor();
  corruptor_p = mcorruptor_p;

  if (prtlev()>3) cout << " MMCorruptor created." << endl;

  Float Amp(1.0);
  if (simpar.isDefined("amplitude")) {
    Amp=simpar.asFloat("amplitude");
  }

  mcorruptor_p->initialize(Amp,simpar);

  String Mode("simple"); // simple means just multiply by amp()
  if (simpar.isDefined("mode")) {
    Mode=simpar.asString("mode");
  }

  mcorruptor_p->mode()=Mode;

  return nSim;
}

Bool MMueller::simPar(VisIter& vi,const Int nChunks) {
  if (prtlev()>3) cout << "  MM::simPar()" << endl;
  AlwaysAssert((isSimulated()),AipsError);
  try {
    
    Vector<Int> a1;
    vi.antenna1(a1);
    Vector<Int> a2;
    vi.antenna2(a2);
    Matrix<Bool> flags;  // matrix foreach row and chan - if want pol send cube.
    vi.flag(flags);
    Vector<Double> antDiams = vi.msColumns().antenna().dishDiameter().getColumn();
    Vector<Double> timevec;
    Double starttime,stoptime;
    
    // initSolvePar: solveCPar().resize(nPar(),nChanPar(),nBln());
    solveCPar()=Complex(0.0);
    IPosition cparshape=solveCPar().shape();

    // n good VI elements to average in each CPar entry
    Cube<Complex> nGood(cparshape);
    solveParOK()=False;
    
    // RI TODO MM::simPar needs channel loop **
    // RI TODO MM::simPar CPar and VI shape checks?

    starttime=vi.time(timevec)[0];

    for (Int ichunk=0;ichunk<nChunks;++ichunk) {
      Int spw(vi.spectralWindow());	
      corruptor_p->currSpw()=spw;
      currSpw()=spw;

      for (vi.origin(); vi.more(); vi++)
	for (Int irow=0;irow<vi.nRow();++irow)
	  if ( a1(irow)!=a2(irow) &&
	       nfalse(flags.column(irow)) > 0 ) {   
	    // RI TODO MM::simPar verify col not row in flags(irow)

	    // don't need loop to find the corruptor time slot here,
	    // since just drawing from random distribution.
	    
	    // RI TODO MM::simPar put temp vars outside of row loop
	    Complex factor(1.0);	
	    Vector<MDirection> antazel(vi.azel(refTime()));
	    
	    if (mcorruptor_p->mode() == "tsys") {
	      
	      Float tau0(0.);
	      if (mcorruptor_p->simpar().isDefined("tau0")) tau0=mcorruptor_p->simpar().asFloat("tau0");
	      // RI TODO MM::simPar *** ATM freqdep()  tau0 from pwv!
	      
	      Float el1 = antazel(a1(irow)).getAngle("rad").getValue()(1);
	      Float el2 = antazel(a2(irow)).getAngle("rad").getValue()(1);
	      
	      Float airmass1(1.0);   
	      Float airmass2(1.0);   
	      if (el1 > 0.0 && el2 > 0.0) {
		airmass1 = 1.0/ sin(el1);
		airmass2 = 1.0/ sin(el2);
	      }
	      
	      Float A(sqrt(exp(-tau0 * airmass1)) * sqrt(exp(-tau0 * airmass2)));
	      
	      // this is tsys at telescope 
	      // (use TOpac later in Simulator to get it above atm)
	      Float tsys = mcorruptor_p->simpar().asFloat("trx") + 
		mcorruptor_p->simpar().asFloat("spillefficiency")*mcorruptor_p->simpar().asFloat("tatmos")*(1.-A) + 
		(1.-mcorruptor_p->simpar().asFloat("spillefficiency"))*mcorruptor_p->simpar().asFloat("tground") + 
		mcorruptor_p->simpar().asFloat("tcmb")*A;
	      
	      // live dangerously: assume all vis have the same tint
	      Double tint = vi.msColumns().exposure()(0);  
	      Int iSpW = vi.spectralWindow();
	      Float deltaNu = 
		vi.msColumns().spectralWindow().totalBandwidth()(iSpW) / 
		Float(vi.msColumns().spectralWindow().numChan()(iSpW));	    
	      
	      // 1e-4 converts the Diam from cm to m
	      factor  = 4 * C::sqrt2 * 1.38062e-16 * 1e23 * 1e-4 * tsys / 
		antDiams(a1(irow)) / antDiams(a2(irow)) /
		sqrt( deltaNu * tint ) /  
		( mcorruptor_p->simpar().asFloat("antefficiency") * 
		  mcorruptor_p->simpar().asFloat("correfficiency") * C::pi );

	      Int scan(vi.scan(scntmp)[0]);
	      if (a1(irow)==5 and a2(irow)==6 and prtlev()>3 and scan<1) {
		cout << "[chunk " << ichunk << "], scan " << scan << ",time = " << vi.time(timtmp)[0]-4.84694e+09 << endl;		
		cout << "Tsys = " << tsys << " dnu = " << deltaNu << " factor = " << factor << endl; }
	    }
	    
	    // RI TODO MM::simPar switch (nCorr) ?  at least for the flags?
	    
	    // corr,chan,bln = xyz
	    solveCPar().xyPlane(irow) = solveCPar().xyPlane(irow) + factor;   
	    nGood.xyPlane(irow) = nGood.xyPlane(irow) + Complex(1.);
	    
	    solveParOK().xyPlane(irow) = True;
	    
	  } // if good row
      if (vi.moreChunks()) vi.nextChunk();
    } // nchunks loop
    
    vi.time(timevec);
    Int tvsize;
    timevec.shape(tvsize);
    stoptime=timevec[tvsize-1];
    
    // RI TODO MM::simPar  don't divide by zero **
    for (Int i=0;i<cparshape(2);i++)
      solveCPar().xyPlane(i) =  solveCPar().xyPlane(i) / nGood.xyPlane(i);
    
    // since we're not going to syncMeta to a VBA in Simulator, we need to set
    // various things here by hand so that keep() has them:
    // refTime(), currSpw(), currField(), and interval()
    refTime() = 0.5*(starttime+stoptime);
    interval() = (stoptime-starttime);
    currField() = vi.fieldId();
    
  } catch (AipsError x) {
    if (corruptor_p) delete corruptor_p;
    cout << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  }
  return True;
}





MMCorruptor::MMCorruptor(): CalCorruptor(0) {};

MMCorruptor::~MMCorruptor() {};











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

	// If this solve not freqdep, and channels not averaged yet, do so
	if (!freqDepMat() && vb.nChannel()>1)
	  vb.freqAveCubes();

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

      // If this solve not freqdep, and channels not averaged yet, do so
      if (!freqDepMat() && vb.nChannel()>1)
	vb.freqAveCubes();

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



// **********************************************************
//  KJones Implementations
//

KJones::KJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  GJones(vs)             // immediate parent
{
  if (prtlev()>2) cout << "K::K(vs)" << endl;

  // Extract per-spw ref Freq for phase(delay) calculation
  MSSpectralWindow msSpw(vs.msName()+"/SPECTRAL_WINDOW");
  MSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz

}

KJones::KJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  GJones(nAnt)
{
  if (prtlev()>2) cout << "K::K(nAnt)" << endl;
}

KJones::~KJones() {
  if (prtlev()>2) cout << "K::~K()" << endl;
}


void KJones::specify(const Record& specify) {


  LogMessage message(LogOrigin("SolvableVisCal","specify"));

  Vector<Int> spws;
  Vector<Int> antennas;
  Vector<Int> pols;
  Vector<Double> parameters;

  Int Nspw(1);
  Int Ntime(1);
  Int Nant(0);
  Int Npol(1);
  
  Bool repspw(False);
  
  IPosition ip0(4,0,0,0,0);
  IPosition ip1(4,0,0,0,0);

/*   Not yet supporting time....
  if (specify.isDefined("time")) {
    // TBD: the time label
    cout << "time = " << specify.asString("time") << endl;

    cout << "refTime() = " << refTime() << endl;


  }
*/

  if (specify.isDefined("spw")) {
    // TBD: the spws (in order) identifying the solutions
    spws=specify.asArrayInt("spw");
    cout << "spws = " << spws << endl;
    Nspw=spws.nelements();
    if (Nspw<1) {
      // None specified, so loop over all, repetitively
      //  (We need to optimize this...)
      cout << "Specified parameters repeated on all spws." << endl;
      repspw=True;
      Nspw=nSpw();
      spws.resize(Nspw);
      indgen(spws);
    }
  }


  if (specify.isDefined("antenna")) {
    // TBD: the antennas (in order) identifying the solutions
    antennas=specify.asArrayInt("antenna");
    cout << "antenna indices = " << antennas << endl;
    Nant=antennas.nelements();
    if (Nant<1) {
      // Use specified values for _all_ antennas implicitly
      Nant=1;   // For the antenna loop below
      ip0(2)=0;
      ip1(2)=nAnt()-1;
    }
    else {
      // Point to first antenna
      ip0(2)=antennas(0);
      ip1(2)=ip0(2);
    }
  }
  if (specify.isDefined("pol")) {
    // TBD: the pols (in order) identifying the solutions
    String polstr=specify.asString("pol");
    cout << "pol = " << polstr << endl;
    if (polstr=="R" || polstr=="X") 
      // Fill in only first pol
      pols=Vector<Int>(1,0);
    else if (polstr=="L" || polstr=="Y") 
      // Fill in only second pol
      pols=Vector<Int>(1,1);
    else if (polstr=="R,L" || polstr=="X,Y") {
      // Fill in both pols explicity
      pols=Vector<Int>(2,0);
      pols(1)=1;
    }
    else if (polstr=="L,R" || polstr=="Y,X") {
      // Fill in both pols explicity
      pols=Vector<Int>(2,0);
      pols(0)=1;
    }
    else if (polstr=="")
      // Fill in both pols implicitly
      pols=Vector<Int>();
    else
      throw(AipsError("Invalid pol specification"));
    
    Npol=pols.nelements();
    if (Npol<1) {
      // No pol axis specified
      Npol=1;
      ip0(0)=0;
      ip1(0)=nPar()-1;
    }
    else {
      // Point to the first polarization
      ip0(0)=pols(0);
      ip1(0)=ip0(0);
    }
  }
  if (specify.isDefined("parameter")) {
    // TBD: the actual cal values
    cout << "parameter = " << specify.asArrayDouble("parameter") << endl;

    parameters=specify.asArrayDouble("parameter");

  }
  
  cout << "Shapes = " << parameters.nelements() << " " 
       << (repspw ? (Ntime*Nant*Npol) : (Nspw*Ntime*Nant*Npol)) << endl;

  Int ipar(0);
  for (Int ispw=0;ispw<Nspw;++ispw) {
    // reset par index if we are repeating for all spws
    if (repspw) ipar=0;
    
    // Loop over specified timestamps
    for (Int itime=0;itime<Ntime;++itime) {
      ip1(3)=ip0(3)=itime;
      
      // Loop over specified antennas
      for (Int iant=0;iant<Nant;++iant) {
	if (Nant>1)
	  ip1(2)=ip0(2)=antennas(iant);
	
	// Loop over specified polarizations
	for (Int ipol=0;ipol<Npol;++ipol) {
	  if (Npol>1)
	    ip1(0)=ip0(0)=pols(ipol);
	  
	  Array<Complex> slice(cs().par(spws(ispw))(ip0,ip1));

	  // Acccumulation is addition for delays
	  slice+=Complex(parameters(ipar));
	  ++ipar;
	}
      }
    }
  }
}

void KJones::calcAllJones() {

  if (prtlev()>6) cout << "       VJ::calcAllJones()" << endl;

  if (False) {
    Vector<Complex> x(16,Complex(1.0));
    cout << "x = " << x << endl;
  
    ArrayLattice<Complex> arx(x);
  
    LatticeFFT::cfft(arx);
    cout << "x = " << x << endl;
  }

  // Should handle OK flags in this method, and only
  //  do Jones calc if OK

  Vector<Complex> oneJones;
  Vector<Bool> oneJOK;
  Vector<Complex> onePar;
  Vector<Bool> onePOK;

  ArrayIterator<Complex> Jiter(currJElem(),1);
  ArrayIterator<Bool>    JOKiter(currJElemOK(),1);
  ArrayIterator<Complex> Piter(currCPar(),1);
  ArrayIterator<Bool>    POKiter(currParOK(),1);

  Double phase(0.0);
  for (Int iant=0; iant<nAnt(); iant++) {

    for (Int ich=0; ich<nChanMat(); ich++) {
      
      oneJones.reference(Jiter.array());
      oneJOK.reference(JOKiter.array());
      onePar.reference(Piter.array());
      onePOK.reference(POKiter.array());

      for (Int ipar=0;ipar<nPar();++ipar) {
	if (onePOK(ipar)) { 
	  phase=2.0*C::pi*real(onePar(ipar))*(currFreq()(ich)-KrefFreqs_(currSpw()));
	  oneJones(ipar)=Complex(cos(phase),sin(phase));
	  oneJOK(ipar)=True;
	}
      }
      
      // Advance iterators
      Jiter.next();
      JOKiter.next();
      if (freqDepPar()) {
        Piter.next();
        POKiter.next();
      }

    }
    // Step to next antenns's pars if we didn't in channel loop
    if (!freqDepPar()) {
      Piter.next();
      POKiter.next();
    }
  }
}




} //# NAMESPACE CASA - END

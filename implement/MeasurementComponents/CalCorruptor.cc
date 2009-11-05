//# CalCorruptor.cc: Corruptions for simulated data
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

//#include <msvis/MSVis/VisBuffer.h>
//#include <msvis/MSVis/VisBuffAccumulator.h>
//#include <ms/MeasurementSets/MSColumns.h>
//#include <synthesis/MeasurementEquations/VisEquation.h>
//#include <scimath/Fitting/LSQFit.h>
//#include <lattices/Lattices/ArrayLattice.h>
//#include <lattices/Lattices/LatticeFFT.h>
//#include <tables/Tables/ExprNode.h>
//
//#include <casa/Arrays/ArrayMath.h>
//#include <casa/BasicSL/String.h>
//#include <casa/Utilities/Assert.h>
//#include <casa/Utilities/GenSort.h>
//#include <casa/Exceptions/Error.h>
//#include <casa/OS/Memory.h>
//#include <casa/System/Aipsrc.h>
//
//#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#define RI_DEBUG
#define PRTLEV 2

namespace casa { //# NAMESPACE CASA - BEGIN


CalCorruptor::CalCorruptor(const Int nSim) : 
  nSim_(nSim),
  initialized_(False),
  prtlev_(PRTLEV),
  curr_slot_(-1),
  curr_spw_(-1), nSpw_(0),
  curr_ant_(-1), nAnt_(0) 
{}

CalCorruptor::~CalCorruptor() {}





Array<Complex> ANoiseCorruptor::noise(const Int nrow,const Int ncol) {
  Matrix<Complex> foo(nrow,ncol);
  for (Int j=0;j<ncol;j++)
    for (Int i=0;i<nrow;i++) {
      //foo(i,j,k) = Complex((*nDist_p)(),(*nDist_p)())/amp();
      foo(i,j).real() = (*nDist_p)()*amp();
      foo(i,j).imag() = (*nDist_p)()*amp();
    }
  return foo;
}
ANoiseCorruptor::ANoiseCorruptor(): CalCorruptor(0) {};

ANoiseCorruptor::~ANoiseCorruptor() {};





//MMCorruptor::MMCorruptor(): CalCorruptor(0) {};
//
//MMCorruptor::~MMCorruptor() {};
//
//
//MfMCorruptor::MfMCorruptor(): MMCorruptor() {};
//
//MfMCorruptor::~MfMCorruptor() {};






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

  // RI TODO get Alt etc from observatory info in Simulator

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
  if (prtlev()>5) 
    cout << antx() << endl << anty() << endl;
  Int buffer(2); // # pix border
  //antx()=antx()-mean(antx());
  //anty()=anty()-mean(anty());
  antx()=antx()-min(antx());
  anty()=anty()-min(anty());
  antx()=antx()/pixsize();
  anty()=anty()/pixsize();
  if (prtlev()>4) 
    cout << antx() << endl << anty() << endl;

  Int ysize(Int(ceil(max(anty())+buffer)));

  const Float tracklength = stopTime()-startTime();    
  const Float blowlength = windspeed()*tracklength*1.05; // 5% margin
  if (prtlev()>3) 
    cout << "blowlength: " << blowlength << " track time = " << tracklength << endl;
  
  Int xsize(Int(ceil(max(antx())+buffer+blowlength/pixsize()))); 

  if (prtlev()>3) 
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
  if (prtlev()>3 and currAnt()<2) {
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
    // RI TODO Tcorr::gain  interpolate screen!
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
#ifdef RI_DEBUG      
      if (prtlev()>5) 
	cout << itsRIP->getDispersiveWetPhaseDelay(0,ichan).get("rad") << " "
	     << itsRIP->getDispersiveWetPhaseDelay(1,ichan).get("rad") << " "
	     << itsRIP->getDispersiveWetPhaseDelay(2,ichan).get("rad") << " "
	     << itsRIP->getDispersiveWetPhaseDelay(3,ichan).get("rad") << endl;
#endif	
    } else
      throw(AipsError("TJonesCorruptor internal error accessing pwv()"));  
    return Complex(cos(delay),sin(delay));
  } else {
    cout << "TCorruptor::gain: slot " << curr_slot() << "out of range!" <<endl;
    return Complex(1.);
  }
}









//################ atmcorruptor - this will replace Tcorr, 
// and also produce MfM and Topac corruptions


AtmosCorruptor::AtmosCorruptor() : 
  CalCorruptor(1),  // parent
  mean_pwv_(-1.)
{}

AtmosCorruptor::AtmosCorruptor(const Int nSim) : 
  CalCorruptor(nSim),  // parent
  mean_pwv_(-1.)
{}

AtmosCorruptor::~AtmosCorruptor() {
  if (prtlev()>2) cout << "AtmosCorruptor::~AtmosCorruptor()" << endl;
}



Vector<Float>* AtmosCorruptor::pwv() { 
  if (currAnt()<=pwv_p.nelements())
    return pwv_p[currAnt()];
  else
    return NULL;
};
 
Float& AtmosCorruptor::pwv(const Int islot) { 
  if (currAnt()<=pwv_p.nelements())
    return (*pwv_p[currAnt()])(islot);
  else
    throw(AipsError("AtmosCorruptor internal error accessing delay()"));;
};








void AtmosCorruptor::initialize() {
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
  if (prtlev()>2) cout << "AtmosCorruptor::init [test]" << endl;
}




void AtmosCorruptor::initAtm() {

#ifndef CASA_STANDALONE
  atm::Temperature  T( 270.0,"K" );   // Ground temperature
  atm::Pressure     P( 560.0,"mb");   // Ground Pressure
  atm::Humidity     H(  20,"%" );     // Ground Relative Humidity (indication)
  atm::Length       Alt(  5000,"m" ); // Altitude of the site 
  atm::Length       WVL(   2.0,"km"); // Water vapor scale height

  // RI TODO get Alt etc from observatory info in Simulator

  double TLR = -5.6;     // Tropospheric lapse rate (must be in K/km)
  atm::Length  topAtm(  48.0,"km");   // Upper atm. boundary for calculations
  atm::Pressure Pstep(  10.0,"mb");   // Primary pressure step
  double PstepFact = 1.2; // Pressure step ratio between two consecutive layers
  atm::Atmospheretype atmType = atm::tropical;

  itsatm = new atm::AtmProfile(Alt, P, T, TLR, 
			       H, WVL, Pstep, PstepFact, 
			       topAtm, atmType);

  if (nSpw()<=0)
    throw(AipsError("AtmosCorruptor::initAtm called before spw setup."));

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
  
  if (prtlev()>2) cout << "AtmosCorruptor::getDispersiveWetPathLength = " 
		       << itsRIP->getDispersiveWetPathLength().get("micron") 
		       << " microns at " 
		       << fRefFreq()[0]/1e9 << " GHz" << endl;

  //  itsSkyStatus = new atm::SkyStatus(*itsRIP);

#endif
}




void AtmosCorruptor::initialize(const Record& simpar) {
  simpar_=simpar;

  // start with amp defined as something
  amp()=1.0;
  if (simpar.isDefined("amplitude")) amp()=simpar.asFloat("amplitude");
  
  // simple means just multiply by amp()
  mode()="simple"; 
  if (simpar.isDefined("mode")) mode()=simpar.asString("mode");  

  if (mode()=="simple") {
    freqDep()=False; 
    if (prtlev()>2) cout << "AtmosCorruptor::init [simple scale by " << amp() << "]" << endl;

  } else {

    // use ATM but no time dependence - e.g. for B[Tsys]
    if (freqDep()) initAtm();
    
    // RI TODO AtmCorr::initialize catch other modes?  
    // if (mode()=="tsys") {

    // go with ATM straight up:
    tauscale() = 1.0;
    // user can override the ATM tau0 thusly - if its freqdep it'll be interpreted in the center of the band
    if (simpar.isDefined("tau0")) {
      Float tau0_atm(1.0);
      // find tau in center of band for current ATM parameters
      if (freqDep()) tau0_atm=opac(nChan()/2);
      tauscale()=simpar.asFloat("tau0")/tau0_atm;
    }
    
    // modified from mm::simPar:  
    // Tsys = Tsys0 + Tsys1*exp(+tau)
    tsys0() = simpar.asFloat("tcmb") - simpar.asFloat("spillefficiency")*simpar.asFloat("tatmos");
    tsys1() = simpar.asFloat("spillefficiency")*simpar.asFloat("tatmos") + 
      (1-simpar.asFloat("spillefficiency"))*simpar.asFloat("tground") +
      simpar.asFloat("trx");
    
    // conversion to Jy, when divided by D1D2
    amp() = 4 * C::sqrt2 * 1.38062e-16 * 1e23 * 1e-4 / 		
      ( simpar.asFloat("antefficiency") * 
	simpar.asFloat("correfficiency") * C::pi );

    if (prtlev()>2) cout << "AtmosCorruptor::init [tsys scale, freqDep=" << freqDep() << "]" << endl;
  }
  initialized()=True;    
}



// opacity - for screens, we'll need other versions of this like the different cphase calls
// that multiply wetopacity by the fluctuation in pwv
Float AtmosCorruptor::opac(const Int ichan) {
  Float opac = itsRIP->getDryOpacity(currSpw(),ichan).get() + 
    itsRIP->getWetOpacity(currSpw(),ichan).get()  ;
  return opac;
}






void AtmosCorruptor::initialize(const Int Seed, const Float Beta, const Float scale) {
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
  if (prtlev()>2) cout << "AtmosCorruptor::init [1d]" << endl;

}



  
  void AtmosCorruptor::initialize(const Int Seed, const Float Beta, const Float scale, const ROMSAntennaColumns& antcols) {
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
  if (prtlev()>5) 
    cout << antx() << endl << anty() << endl;
  Int buffer(2); // # pix border
  //antx()=antx()-mean(antx());
  //anty()=anty()-mean(anty());
  antx()=antx()-min(antx());
  anty()=anty()-min(anty());
  antx()=antx()/pixsize();
  anty()=anty()/pixsize();
  if (prtlev()>4) 
    cout << antx() << endl << anty() << endl;

  Int ysize(Int(ceil(max(anty())+buffer)));

  const Float tracklength = stopTime()-startTime();    
  const Float blowlength = windspeed()*tracklength*1.05; // 5% margin
  if (prtlev()>3) 
    cout << "blowlength: " << blowlength << " track time = " << tracklength << endl;
  
  Int xsize(Int(ceil(max(antx())+buffer+blowlength/pixsize()))); 

  if (prtlev()>3) 
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
  if (prtlev()>3 and currAnt()<2) {
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
  if (prtlev()>2) cout << "AtmosCorruptor::init [2d]" << endl;

}



Complex AtmosCorruptor::cphase(const Int ix, const Int iy, const Int ichan) {
  // this is the complex phase gain for a T
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
    // RI TODO Tcorr::gain  interpolate screen!
    Float deltapwv = (*screen_p)(ix+blown,iy);
    delay = itsRIP->getDispersiveWetPhaseDelay(currSpw(),ichan).get("rad") 
      * deltapwv / 57.2958; // convert from deg to rad
    return Complex(cos(delay),sin(delay));
  } else {    
    o << "atmosCorruptor::cphase: slot " << curr_slot() << "out of range!" <<endl;
    throw(AipsError(o));
    return Complex(1.);
  }
}


Complex AtmosCorruptor::cphase(const Int ichan) {
  AlwaysAssert(mode()=="1d" or mode()=="test",AipsError);
  Float delay;
  
  if (curr_slot()>=0 and curr_slot()<nSim()) {
    // Float freq = fRefFreq()[currSpw()] + 
    //   Float(ichan) * (fWidth()[currSpw()]/Float(fnChan()[currSpw()]));
    
    if (currAnt()<=pwv_p.nelements()) {
      Float deltapwv = (*pwv_p[currAnt()])(curr_slot());
      delay = itsRIP->getDispersiveWetPhaseDelay(currSpw(),ichan).get("rad") 
	* deltapwv / 57.2958; // convert from deg to rad
#ifdef RI_DEBUG      
      if (prtlev()>5) 
	cout << itsRIP->getDispersiveWetPhaseDelay(0,ichan).get("rad") << " "
	     << itsRIP->getDispersiveWetPhaseDelay(1,ichan).get("rad") << " "
	     << itsRIP->getDispersiveWetPhaseDelay(2,ichan).get("rad") << " "
	     << itsRIP->getDispersiveWetPhaseDelay(3,ichan).get("rad") << endl;
#endif	
    } else
      throw(AipsError("AtmosCorruptor internal error accessing pwv()"));  
    return Complex(cos(delay),sin(delay));
  } else {
    cout << "AtmosCorruptor::cphase: slot " << curr_slot() << "out of range!" <<endl;
    return Complex(1.);
  }
}











//###################################  fractional brownian motion


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
    server.resize(size2);
    // RI_TODO fBM::init make sure hermitian calc is correct - fftw only doubles first axis.
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



}

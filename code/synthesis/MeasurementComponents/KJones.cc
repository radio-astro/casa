//# KJones.cc: Implementation of KJones
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2011
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

#include <synthesis/MeasurementComponents/KJones.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisBuffAccumulator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>  // *
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <scimath/Mathematics/FFTServer.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>



namespace casa { //# NAMESPACE CASA - BEGIN



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
  //  TBD: these should be in the caltable!!
  MSSpectralWindow msSpw(vs.spectralWindowTableName());
  ROMSSpWindowColumns msCol(msSpw);
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

void KJones::setApply(const Record& apply) {

  // Call parent to do conventional things
  GJones::setApply(apply);

  if (calWt()) 
    logSink() << " (" << this->typeName() << ": Enforcing calWt()=False for phase/delay-like terms)" << LogIO::POST;

  // Enforce calWt() = False for delays
  calWt()=False;

  // Extract per-spw ref Freq for phase(delay) calculation
  //  from the CalTable
  MSSpectralWindow msSpw(ct_->spectralWindow());
  ROMSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz

  /// Re-assign KrefFreq_ according spwmap (if any)
  if (spwMap().nelements()>0) {
    Vector<Double> tmpfreqs;
    tmpfreqs.assign(KrefFreqs_);
    for (uInt ispw=0;ispw<spwMap().nelements();++ispw)
      if (spwMap()(ispw)>-1)
	KrefFreqs_(ispw)=tmpfreqs(spwMap()(ispw));
  }

    
}

void KJones::setSolve(const Record& solve) {

  // Call parent to do conventional things
  GJones::setSolve(solve);

  // Trap unspecified refant:
  if (refant()<0)
    throw(AipsError("Please specify a good reference antenna (refant) explicitly."));

}


void KJones::specify(const Record& specify) {

  return SolvableVisCal::specify(specify);

}

void KJones::calcAllJones() {

  if (prtlev()>6) cout << "       VJ::calcAllJones()" << endl;

  // Should handle OK flags in this method, and only
  //  do Jones calc if OK

  Vector<Complex> oneJones;
  Vector<Bool> oneJOK;
  Vector<Float> onePar;
  Vector<Bool> onePOK;

  ArrayIterator<Complex> Jiter(currJElem(),1);
  ArrayIterator<Bool>    JOKiter(currJElemOK(),1);
  ArrayIterator<Float>   Piter(currRPar(),1);
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
	  phase=2.0*C::pi*onePar(ipar)*(currFreq()(ich)-KrefFreqs_(currSpw()));
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

void KJones::selfSolveOne(VisBuffGroupAcc& vbga) {

  // We don't support combine on spw or field (yet),
  // so there should be only one VB in the vbga
  if (vbga.nBuf()!=1) 
    throw(AipsError("KJones can't process multi-VB vbga."));

  // call the single-VB solver with the first VB in the vbga
  this->solveOneVB(vbga(0));

}


// Do the FFTs
void KJones::solveOneVB(const VisBuffer& vb) {

  Int nChan=vb.nChannel();

  solveRPar()=0.0;
  solveParOK()=False;

  //  cout << "solveRPar().shape() = " << solveRPar().shape() << endl;
  //  cout << "vb.nCorr() = " << vb.nCorr() << endl;
  //  cout << "vb.corrType() = " << vb.corrType() << endl;

  // FFT parallel-hands only
  Int nCorr=vb.nCorr();
  Int nC= (nCorr>1 ? 2 : 1);  // number of parallel hands
  Int sC= (nCorr>2 ? 3 : 1);  // step by 3 for full pol data

  // I/O shapes
  Int fact(8);
  Int nPadChan=nChan*fact;

  IPosition ip0=vb.visCube().shape();
  IPosition ip1=ip0;
  ip1(0)=nC;    // the number of correlations to FFT 
  ip1(1)=nPadChan; // padded channel axis

  // I/O slicing
  Slicer sl0(Slice(0,nC,sC),Slice(),Slice());  
  Slicer sl1(Slice(),Slice(nChan*(fact-1)/2,nChan,1),Slice());

  // Fill the (padded) transform array
  //  TBD: only do ref baselines
  Cube<Complex> vpad(ip1);
  Cube<Complex> slvis=vb.visCube();
  vpad.set(Complex(0.0));
  vpad(sl1)=slvis(sl0);

  // We will only transform frequency axis of 3D array
  Vector<Bool> ax(3,False);
  ax(1)=True;
  
  // Do the FFT
  ArrayLattice<Complex> c(vpad);
  LatticeFFT::cfft(c,ax);        

  // Find peak in each FFT
  Int ipk=0;
  Float amax(0.0);
  Vector<Float> amp;

  //  cout << "Time=" << MVTime(refTime()/C::day).string(MVTime::YMD,7)
  //       << " Spw=" << currSpw() << ":" << endl;

  for (Int irow=0;irow<vb.nRow();++irow) {
    if (!vb.flagRow()(irow) &&
	vb.antenna1()(irow)!=vb.antenna2()(irow) &&
	(vb.antenna1()(irow)==refant() || 
	 vb.antenna2()(irow)==refant()) ) {

      for (Int icor=0;icor<ip1(0);++icor) {
	amp=amplitude(vpad(Slice(icor,1,1),Slice(),Slice(irow,1,1)));
	ipk=1;
	amax=0;
	for (Int ich=1;ich<nPadChan-1;++ich) {
	  if (amp(ich)>amax) {
	    ipk=ich;
	    amax=amp(ich);
	  }
	} // ich

       	// Derive refined peak (fractional) channel
	// via parabolic interpolation of peak and neighbor channels

	Vector<Float> amp3(amp(IPosition(1,ipk-1),IPosition(1,ipk+1)));
	Float denom(amp3(0)-2.0*amp3(1)+amp3(2));

	if (amax>0.0 && abs(denom)>0) {

	  Float fipk=Float(ipk)+0.5-(amp3(2)-amp3(1))/denom;
	    
	  // Handle FFT offset and scale
	  Float delay=(fipk-Float(nPadChan/2))/Float(nPadChan); // cycles/sample
	  
	  // Convert to cycles/Hz and then to nsec
	  Double df=vb.frequency()(1)-vb.frequency()(0);
	  delay/=df;
	  delay/=1.0e-9;
	  
	  //	  cout << " Antenna ID=";
	  if (vb.antenna1()(irow)==refant()) {
	    //	    cout << vb.antenna2()(irow) 
	    //		 << ", pol=" << icor << " delay(nsec)="<< -delay; 
	    solveRPar()(icor,0,vb.antenna2()(irow))=-delay;
	    solveParOK()(icor,0,vb.antenna2()(irow))=True;
	  }
	  else if (vb.antenna2()(irow)==refant()) {
	    //	    cout << vb.antenna1()(irow) 
	    //		 << ", pol=" << icor << " delay(nsec)="<< delay;
	    solveRPar()(icor,0,vb.antenna1()(irow))=delay;
	    solveParOK()(icor,0,vb.antenna1()(irow))=True;
	  }
	  //	  cout << " (refant ID=" << refant() << ")" << endl;

	  /*	  
	  cout << irow << " " 
	       << vb.antenna1()(irow) << " " 
	       << vb.antenna2()(irow) << " " 
	       << icor << " "
	       << ipk << " "
	       << fipk << " "
	       << delay << " "
	       << endl;
	  */
	} // amax > 0
    /*
	else {
	  cout << "No solution found for antenna ID= ";
	  if (vb.antenna1()(irow)==refant())
	    cout << vb.antenna2()(irow);
	  else if (vb.antenna2()(irow)==refant()) 
	    cout << vb.antenna1()(irow);
	  cout << " in polarization " << icor << endl;
	}
    */
	
      } // icor
    } // !flagrRow, etc.

  } // irow

  // Ensure refant has zero delay and is NOT flagged
  solveRPar()(Slice(),Slice(),Slice(refant(),1,1)) = 0.0;
  solveParOK()(Slice(),Slice(),Slice(refant(),1,1)) = True;

  /*  
  if (nfalse(solveParOK())>0) {
    cout << "NB: No delay solutions found for antenna IDs: ";
    Int nant=solveParOK().shape()(2);
    for (Int iant=0;iant<nant;++iant)
      if (!(solveParOK()(0,0,iant)))
	cout << iant << " ";
    cout << endl;
  }
  */

}

// **********************************************************
//  KcrossJones Implementations
//

KcrossJones::KcrossJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  KJones(vs)             // immediate parent
{
  if (prtlev()>2) cout << "Kx::Kx(vs)" << endl;

  // Extract per-spw ref Freq for phase(delay) calculation
  //  TBD: these should be in the caltable!!
  MSSpectralWindow msSpw(vs.spectralWindowTableName());
  ROMSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz

}

KcrossJones::KcrossJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  KJones(nAnt)
{
  if (prtlev()>2) cout << "Kx::Kx(nAnt)" << endl;
}

KcrossJones::~KcrossJones() {
  if (prtlev()>2) cout << "Kx::~Kx()" << endl;
}

// Do the FFTs
void KcrossJones::solveOneVB(const VisBuffer& vb) {

  solveRPar()=0.0;
  solveParOK()=False;

  Int fact(8);
  Int nChan=vb.nChannel();
  Int nPadChan=nChan*fact;

  // Collapse cross-hands over baseline
  Vector<Complex> sumvis(nPadChan);
  sumvis.set(Complex(0.0));
  Vector<Complex> slsumvis(sumvis(Slice(nChan*(fact-1)/2,nChan,1)));
  Vector<Float> sumwt(nChan);
  sumwt.set(0.0);
  for (Int irow=0;irow<vb.nRow();++irow) {
    if (!vb.flagRow()(irow) &&
	vb.antenna1()(irow)!=vb.antenna2()(irow)) {

      for (Int ich=0;ich<nChan;++ich) {

	if (!vb.flag()(ich,irow)) {
	  // 1st cross-hand
	  slsumvis(ich)+=(vb.visCube()(1,ich,irow)*vb.weightMat()(1,irow));
	  sumwt(ich)+=vb.weightMat()(1,irow);
	  // 2nd cross-hand
	  slsumvis(ich)+=conj(vb.visCube()(2,ich,irow)*vb.weightMat()(2,irow));
	  sumwt(ich)+=vb.weightMat()(2,irow);
	}
      }
    }
  }
  // Normalize the channelized sum
  for (int ich=0;ich<nChan;++ich)
    if (sumwt(ich)>0)
      slsumvis(ich)/=sumwt(ich);
    else
      slsumvis(ich)=Complex(0.0);
  
  // Do the FFT
  ArrayLattice<Complex> c(sumvis);
  LatticeFFT::cfft(c,True);        
      
  // Find peak in each FFT
  Vector<Float> amp=amplitude(sumvis);

  Int ipk=0;
  Float amax(0.0);
  for (Int ich=0;ich<nPadChan;++ich) {
    if (amp(ich)>amax) {
      ipk=ich;
      amax=amp(ich);
    }
  } // ich
	
  // Derive refined peak (fractional) channel
  // via parabolic interpolation of peak and neighbor channels
  Float fipk=ipk;
  // Interpolate the peak (except at edges!)
  if (ipk>0 && ipk<(nPadChan-1)) {
    Vector<Float> amp3(amp(IPosition(1,ipk-1),IPosition(1,ipk+1)));
    fipk=Float(ipk)+0.5-(amp3(2)-amp3(1))/(amp3(0)-2.0*amp3(1)+amp3(2));
    Vector<Float> pha3=phase(sumvis(IPosition(1,ipk-1),IPosition(1,ipk+1)));
  }

  // Handle FFT offset and scale
  Float delay=(fipk-Float(nPadChan/2))/Float(nPadChan); // cycles/sample

  // Convert to cycles/Hz and then to nsec
  Double df=vb.frequency()(1)-vb.frequency()(0);
  delay/=df;
  delay/=1.0e-9;

  solveRPar()(Slice(0,1,1),Slice(),Slice())=delay;
  solveParOK()=True;

  logSink() << " Time="<< MVTime(refTime()/C::day).string(MVTime::YMD,7)
	    << " Spw=" << currSpw()
	    << " Global cross-hand delay=" << delay << " nsec"
	    << LogIO::POST;
}

// **********************************************************
//  KMBDJones Implementations
//

KMBDJones::KMBDJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  KJones(vs)             // immediate parent
{
  if (prtlev()>2) cout << "Kmbd::Kmbd(vs)" << endl;

  // For MBD, the ref frequencies are zero
  //  TBD: these should be in the caltable!!
  KrefFreqs_.resize(nSpw());
  KrefFreqs_.set(0.0);
}

KMBDJones::KMBDJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  KJones(nAnt)
{

  if (prtlev()>2) cout << "Kmbd::Kmbd(nAnt)" << endl;
  // For MBD, the ref frequencies are zero
  //  TBD: these should be in the caltable!!
  KrefFreqs_.resize(nSpw());
  KrefFreqs_.set(0.0);

}

KMBDJones::~KMBDJones() {
  if (prtlev()>2) cout << "Kmbd::~Kmbd()" << endl;
}


void KMBDJones::setApply(const Record& apply) {
  if (prtlev()>2) cout << "Kmbd::setApply()" << endl;
  KJones::setApply(apply);
  KrefFreqs_.set(0.0);  // MBD is ALWAYS ref'd to zero freq
}


// **********************************************************
//  KAntPosJones Implementations
//

KAntPosJones::KAntPosJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  KJones(vs)             // immediate parent
{
  if (prtlev()>2) cout << "Kap::Kap(vs)" << endl;

  epochref_p="UTC";

}

KAntPosJones::KAntPosJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  KJones(nAnt)
{
  if (prtlev()>2) cout << "Kap::Kap(nAnt)" << endl;
}

KAntPosJones::~KAntPosJones() {
  if (prtlev()>2) cout << "Kap::~Kap()" << endl;
}

void KAntPosJones::setApply(const Record& apply) {

  //  TBD: Handle missing solutions in spw 0?

  // Force spwmap to all 0  (antpos is not spw-dep)
  //  NB: this is required before calling parents, because
  //   SVC::setApply sets up the CTPatchedInterp with spwMap()
  logSink() << " (" << this->typeName() 
	    << ": Overriding with spwmap=[0] since " << this->typeName() 
	    << " is not spw-dependent)"
	    << LogIO::POST;
  spwMap().assign(Vector<Int>(1,0));

  // Remove spwmap from record, and pass along to generic code
  Record newapply;
  newapply=apply;
  if (newapply.isDefined("spwmap"))
    newapply.removeField("spwmap");
  
  // Call parent to do conventional things
  KJones::setApply(newapply);

}


void KAntPosJones::specify(const Record& specify) {

  LogMessage message(LogOrigin("KAntPosJones","specify"));

  Vector<Int> spws;
  Vector<Int> antennas;
  Vector<Double> parameters;

  Int Nant(0);

  // Handle old VLA rotation, if necessary
  Bool doVLARot(False);
  Matrix<Double> vlaRot=Rot3D(0,0.0);
  if (specify.isDefined("caltype") ) {
    String caltype=upcase(specify.asString("caltype"));
    if (upcase(caltype).contains("VLA")) {
      doVLARot=True;
      MPosition vlaCenter;
      AlwaysAssert(MeasTable::Observatory(vlaCenter,"VLA"),AipsError);
      Double vlalong=vlaCenter.getValue().getLong();
      //      vlalong=-107.617722*C::pi/180.0;
      cout << "We will rotate specified offsets by VLA longitude = " 
	   << vlalong*180.0/C::pi << endl;
      vlaRot=Rot3D(2,vlalong);
    }
  }
  
  IPosition ip0(3,0,0,0);
  IPosition ip1(3,2,0,0);

  if (specify.isDefined("antenna")) {
    // TBD: the antennas (in order) identifying the solutions
    antennas=specify.asArrayInt("antenna");
    //    cout << "antenna indices = " << antennas << endl;
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

  if (specify.isDefined("parameter")) {
    // TBD: the actual cal values
    parameters=specify.asArrayDouble("parameter");
    //    cout << "parameters = ]" << parameters << "[" << endl;
  }

  Int npar=parameters.nelements();
  
  // Can't proceed of no parameters were specified
  if (npar==0)
    throw(AipsError("No antenna position corrections specified!"));

  // Must be a multiple of 3
  if (npar%3 != 0)
    throw(AipsError("For antenna position corrections, 3 parameters per antenna are required."));
  
  //  cout << "Shapes = " << parameters.nelements() << " " 
  //       << Nant*3 << endl;

  //  cout << "parameters = " << parameters << endl;

  // Always _ONLY_ spw=0 for antpos corrections
  currSpw()=0;

  // Loop over specified antennas
  Int ipar(0);
  for (Int iant=0;iant<Nant;++iant) {
    if (Nant>1)
      ip1(2)=ip0(2)=antennas(iant);

    // make sure ipar doesn't exceed specified list
    ipar=ipar%npar;
    
    // The current 3-vector of position corrections
    Vector<Double> apar(parameters(IPosition(1,ipar),IPosition(1,ipar+2)));

    // If old VLA, rotate them
    if (doVLARot) {
      cout << "id = " << antennas(iant) << " " << apar;
      apar = product(vlaRot,apar);
      cout << "--(rotation VLA to ITRF)-->" << apar << endl;
    }

    // Loop over 3 parameters, each antenna
    for (Int ipar0=0;ipar0<3;++ipar0) {
      ip1(0)=ip0(0)=ipar0;

      Array<Float> sl(solveAllRPar()(ip0,ip1));
    
      // Acccumulation is addition for ant pos corrections
      sl+=Float(apar(ipar0));
      ++ipar;
    }
  }

  // Store in the memory caltable
  //  (currSpw()=0 is the only one we need)
  keepNCT();

}

void KAntPosJones::calcAllJones() {

  if (prtlev()>6) cout << "       Kap::calcAllJones()" << endl;


  // The relevant direction for the delay offset calculation
  const MDirection& phasedir = vb().msColumns().field().phaseDirMeas(currField());

  // The relevant timestamp 
  MEpoch epoch(Quantity(currTime(),"s"));
  epoch.setRefString(epochref_p);

  //  cout << epoch.getValue() << ":" << endl;

  // The frame in which we convert our MBaseline from earth to sky and to uvw
  MeasFrame mframe(vb().msColumns().antenna().positionMeas()(0),epoch,phasedir);

  // template MBaseline, that will be used in calculations below
  MBaseline::Ref mbearthref(MBaseline::ITRF,mframe);
  MBaseline mb;
  MVBaseline mvb;
  mb.set(mvb,mbearthref); 

  // A converter that takes the MBaseline from earth to sky frame
  MBaseline::Ref mbskyref(MBaseline::fromDirType(MDirection::castType(phasedir.myType())));
  MBaseline::Convert mbcverter(mb,mbskyref);



  Double phase(0.0);
  for (Int iant=0; iant<nAnt(); iant++) {

    Vector<Float> rpars(currRPar().xyPlane(iant).column(0));
    Vector<Double> dpars(rpars.nelements());
    convertArray(dpars,rpars);

    // Only do complicated calculation if there 
    //   is a non-zero ant pos error
    if (max(abs(rpars))>0.0) {

      //      cout << iant << " ";
      //      cout << dpars << " ";
      //      cout << flush;

      // We need the w offset (in direction of source) implied
      //  by the antenna position correction
      Double dw(0.0);
      
      // The current antenna's error as an MBaseline (earth frame)
      mvb=MVBaseline(dpars);
      mb.set(mvb,mbearthref);
      
      // Convert to sky frame
      MBaseline mbdir = mbcverter(mb);

      // Get implied uvw
      MVuvw uvw(mbdir.getValue(),phasedir.getValue());

      // dw is third element
      dw=uvw.getVector()(2);

      // In time units 
      dw/=C::c;    // to sec
      dw*=1.0e9;   // to nsec

      //      cout << " " << dw << flush;

      // Form the complex corrections per chan (freq)
      for (Int ich=0; ich<nChanMat(); ++ich) {
        
	// NB: currFreq() is in GHz
	phase=2.0*C::pi*dw*currFreq()(ich);
	currJElem()(0,ich,iant)=Complex(cos(phase),sin(phase));
	currJElemOK()(0,ich,iant)=True;
	
	//	if (ich==0)
	//	  cout << " " << cos(phase) << " " << sin(phase) << endl;

      }
    }
    else {
      // No correction
      currJElem().xyPlane(iant)=Complex(1.0);
      currJElemOK().xyPlane(iant)=True;
    }

  }

}

} //# NAMESPACE CASA - END

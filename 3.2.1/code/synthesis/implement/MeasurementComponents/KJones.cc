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

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
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

void KJones::setApply(const Record& apply) {

  // Call parent to do conventional things
  GJones::setApply(apply);

  if (calWt()) 
    logSink() << " (" << this->typeName() << ": Enforcing calWt()=False for phase/delay-like terms)" << LogIO::POST;

  // Enforce calWt() = False for delays
  calWt()=False;

}

void KJones::setSolve(const Record& solve) {

  // Call parent to do conventional things
  GJones::setSolve(solve);

  // Trap unspecified refant:
  if (refant()<0)
    throw(AipsError("Please specify a good reference antenna (refant) explicitly."));

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

  Int nparam=parameters.nelements();

  // Test for correct number of specified parameters
  //  Either all params are enumerated, or one is specified
  //  for all, [TBD:or a polarization pair is specified for all]
  //  else throw
  if (nparam!=(repspw ? (Ntime*Nant*Npol) : (Nspw*Ntime*Nant*Npol)) && 
      nparam!=1 )                // one for all
    //      (Npol==2 && nparam%2!=0) )  // poln pair for all
    throw(AipsError("Inconsistent number of parameters specified."));

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

// Simple FFT search solver
void KJones::selfGatherAndSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   K::selfGatherAndSolve(ve)" << endl;

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

      // solve for the R-L phase term in the current VB
      solveOneVB(svb);

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

  solveCPar()=Complex(0.0);
  solveParOK()=False;

  //  cout << "solveCPar().shape() = " << solveCPar().shape() << endl;
  //  cout << "vb.nCorr() = " << vb.nCorr() << endl;
  //  cout << "vb.corrType() = " << vb.corrType() << endl;

  // FFT parallel-hands only
  Int nC= (vb.nCorr()>1 ? 2 : 1);  // number of parallel hands
  Int sC= (vb.nCorr()>2 ? 3 : 1);  // step by 3 for full pol data

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

  //  cout << "vpad.shape() = " << vpad.shape() << endl;
  //  cout << "vpad(sl1).shape() = " << vpad(sl1).shape() << endl;


  //  cout << "Starting ffts..." << flush;

  if (False) {
    Vector<Complex> testf(64,Complex(1.0));
    FFTServer<Float,Complex> ffts;
    cout << "FFTServer..." << flush;
    ffts.fft(testf,True);
    cout << "done." << endl;
    
    ArrayLattice<Complex> tf(testf);
    cout << "tf.isWritable() = " << boolalpha << tf.isWritable() << endl;
    
    LatticeFFT::cfft(tf,False);
    cout << "testf = " << testf << endl;
  }  

  // We will only transform frequency axis of 3D array
  Vector<Bool> ax(3,False);
  ax(1)=True;
  
  // Do the FFT
  ArrayLattice<Complex> c(vpad);
  //  cout << "c.shape() = " << c.shape() << endl;
  LatticeFFT::cfft(c,ax);        
  //LatticeFFT::cfft2d(c,False);   
      
  //  cout << "done." << endl;

  // Find peak in each FFT
  Int ipk=0;
  Float amax(0.0);
  Vector<Float> amp;
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
	  
	  cout << "Antenna ID=";
	  if (vb.antenna1()(irow)==refant()) {
	    cout << vb.antenna2()(irow) 
		 << ", spw=" << currSpw() 
		 << ", pol=" << icor << " delay(nsec)="<< -delay; 
	    solveCPar()(icor,0,vb.antenna2()(irow))=-Complex(delay);
	    solveParOK()(icor,0,vb.antenna2()(irow))=True;
	  }
	  else if (vb.antenna2()(irow)==refant()) {
	    cout << vb.antenna1()(irow) << ", pol=" << icor << " delay(nsec)="<< delay;
	    solveCPar()(icor,0,vb.antenna1()(irow))=Complex(delay);
	    solveParOK()(icor,0,vb.antenna1()(irow))=True;
	  }
	  cout << " (refant ID=" << refant() << ")" << endl;

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
	else {
	  cout << "No solution found for antenna ID= ";
	  if (vb.antenna1()(irow)==refant())
	    cout << vb.antenna2()(irow);
	  else if (vb.antenna2()(irow)==refant()) 
	    cout << vb.antenna1()(irow);
	  cout << " in polarization " << icor << endl;
	}
	
      } // icor
    } // !flagrRow, etc.

  } // irow

  // Ensure refant has zero delay and is NOT flagged
  solveCPar()(Slice(),Slice(),Slice(refant(),1,1)) = 0.0;
  solveParOK()(Slice(),Slice(),Slice(refant(),1,1)) = True;
  
  if (nfalse(solveParOK())>0) {
    cout << "NB: No delay solutions found for antenna IDs: ";
    Int nant=solveParOK().shape()(2);
    for (Int iant=0;iant<nant;++iant)
      if (!(solveParOK()(0,0,iant)))
	cout << iant << " ";
    cout << endl;
  }

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
  MSSpectralWindow msSpw(vs.msName()+"/SPECTRAL_WINDOW");
  MSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz

  cout << boolalpha 
       << " freqDepMat() = " << freqDepMat()
       << " freqDepPar() = " << freqDepPar() << endl;

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

  solveCPar()=Complex(0.0);
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
  
  cout << "Starting ffts..." << flush;

  // Do the FFT
  ArrayLattice<Complex> c(sumvis);
  cout << "c.shape() = " << c.shape() << endl;
  LatticeFFT::cfft(c,True);        
      
  cout << "done." << endl;

  // Find peak in each FFT
  Vector<Float> amp=amplitude(sumvis);
  cout << "amp = " << amp << endl;


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
    cout << "amp3 = " << amp3 << endl;
    cout << "pha3 = " << pha3 << endl;

  }

  // Handle FFT offset and scale
  Float delay=(fipk-Float(nPadChan/2))/Float(nPadChan); // cycles/sample

  // Convert to cycles/Hz and then to nsec
  Double df=vb.frequency()(1)-vb.frequency()(0);
  delay/=df;
  delay/=1.0e-9;

  solveCPar()(Slice(0,1,1),Slice(),Slice())=Complex(delay);
  solveParOK()=True;

  cout 	     << ipk << " "
	     << fipk << " "
	     << delay << " "
	     << endl;
  
}




// **********************************************************
//  KMBDJones Implementations
//

KMBDJones::KMBDJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  KJones(vs)             // immediate parent
{
  if (prtlev()>2) cout << "K::K(vs)" << endl;

  // For MBD, the ref frequencies are zero
  //  TBD: these should be in the caltable!!
  KrefFreqs_.resize(nSpw());
  KrefFreqs_.set(0.0);

  /*  
  MSSpectralWindow msSpw(vs.msName()+"/SPECTRAL_WINDOW");
  MSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz
  */

}

KMBDJones::KMBDJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  KJones(nAnt)
{

  if (prtlev()>2) cout << "K::K(nAnt)" << endl;
  // For MBD, the ref frequencies are zero
  //  TBD: these should be in the caltable!!
  KrefFreqs_.resize(nSpw());
  KrefFreqs_.set(0.0);

}

KMBDJones::~KMBDJones() {
  if (prtlev()>2) cout << "K::~K()" << endl;
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

  // Extract the FIELD phase direction measure column
  MSField msf(vs.msName()+"/FIELD");
  MSFieldColumns msfc(msf);
  dirmeas_p.reference(msfc.phaseDirMeasCol());

  //  epochref_p = MSMainColumns(MeasurementSet(vs.msName())).time().columnMeasureType(MSMainEnums::TIME);

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

  // Call parent to do conventional things
  KJones::setApply(apply);

  //  cout << "spwMap() = " << spwMap();

  // Reset spwmap to use spw 0 for all spws
  if (cs().spwOK()(0)) {
    spwMap() = Vector<Int>(nSpw(),0);
    ci().setSpwMap(spwMap());
  }
  else
    throw(AipsError("No KAntPos solutions available for spw 0"));

  //  cout << "->" << spwMap() << endl;

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
  
  IPosition ip0(4,0,0,0,0);
  IPosition ip1(4,2,0,0,0);

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

  }

  Int npar=parameters.nelements();
  
  if (npar%3 != 0)
    throw(AipsError("For antenna position corrections, 3 parameters per antenna are required."));

  
  //  cout << "Shapes = " << parameters.nelements() << " " 
  //       << Nant*3 << endl;

  //  cout << "parameters = " << parameters << endl;

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

      Array<Complex> slice(cs().par(0)(ip0,ip1));
    
      // Acccumulation is addition for ant pos corrections
      slice+=Complex(apar(ipar0),0.0);
      ++ipar;
    }
  }
  
  //  cout << "Ant pos: cs().par(0) = " << cs().par(0) << endl;


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

    Vector<Complex> pars(currCPar().xyPlane(iant).column(0));
    Vector<Float> rpars=real(pars);
    Vector<Double> dpars(rpars.nelements());
    convertArray(dpars,rpars);

    // Only do complicated calculation if there 
    //   is a non-zero ant pos error
    if (max(amplitude(pars))>0.0) {

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

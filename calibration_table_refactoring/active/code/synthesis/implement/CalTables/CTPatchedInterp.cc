//# CTPatchedInterp.cc: Implementation of CTPatchedInterp.h
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#include <synthesis/CalTables/CTPatchedInterp.h>
#include <synthesis/CalTables/CTIter.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
#include <casa/aips.h>

#define CTPATCHEDINTERPVERB False

//#include <casa/BasicSL/Constants.h>
//#include <casa/OS/File.h>
//#include <casa/Logging/LogMessage.h>
//#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Ctor
CTPatchedInterp::CTPatchedInterp(NewCalTable& ct,
				 Int nPar,
				 const String& timetype,
				 const String& freqtype) :
  ct_(ct),
  isCmplx_(False),
  nPar_(nPar),
  nFPar_(nPar),
  timeType_(timetype),
  freqType_(freqtype),
  nChanIn_(),
  freqIn_(),
  nFldOut_(1),  // for now, only one element on field axes...
  nSpwOut_(0),
  nAntOut_(0),
  nFldIn_(1),
  nSpwIn_(ct.spectralWindow().nrow()),
  nAntIn_(ct.antenna().nrow()),
  fldMap_(),
  spwMap_(),
  antMap_(),
  currTime_(),
  result_(),
  resFlag_(),
  tI_()
{
  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::CTPatchedInterp()" << endl;

  // Assume MS dimensions in spw,ant are same (for now)
  //  TBD: Supply something from the MS to discern this...
  nSpwOut_=nSpwIn_;
  nAntOut_=nAntIn_;

  freqIn_.resize(nSpwIn_);

  // Set default maps
  setDefFldMap();
  setDefSpwMap();
  setDefAntMap();

  // How many _Float_ parameters?
  if (isCmplx_=ct_.keywordSet().asString("ParType")=="Complex")  // Complex input
    nFPar_*=2;  // interpolating 2X as many Float values

  // Set channelization
  CTSpWindowColumns ctspw(ct_.spectralWindow());
  ctspw.numChan().getColumn(nChanIn_);
  for (uInt ispw=0;ispw<ctspw.nrow();++ispw) {
    //    freqIn_(ispw).resize(nChanIn_(ispw));
    ctspw.chanFreq().get(ispw,freqIn_(ispw),True);
  }

  // Initialize caltable slices
  sliceTable();

  // Setup mapped interpolators
  initialize();

}


// Destructor
CTPatchedInterp::~CTPatchedInterp() {

  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::~CTPatchedInterp()" << endl;

  IPosition ip(tI_.shape());
  for (Int k=0;k<ip(2);++k)
    for (Int j=0;j<ip(1);++j)
      for (Int i=0;i<ip(0);++i) {
	delete tI_(i,j,k);
	tI_(i,j,k)=NULL;
      }
}

Bool CTPatchedInterp::interpolate(Int fld, Int spw, Double time) {

  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::interpolate(...)" << endl;

  // Force to fld=0, for now  (only one element on fld axis of internal arrays)
  fld=0;

  // TBD: set currTime (also in freq-dep version)

  Bool newcal(False);
  // Loop over _output_ antennas
  for (Int iAntOut=0;iAntOut<nAntOut_;++iAntOut) {
    // Call fully _patched_ time-interpolator, keeping track of 'newness'
    //  fills timeResult_/timeResFlag_ implicitly
    newcal|=tI_(iAntOut,spw,fld)->interpolate(time);
  }

  // Whole result referred to time result:
  result_(spw,fld).reference(timeResult_(spw,fld));
  resFlag_(spw,fld).reference(timeResFlag_(spw,fld));

  return newcal;
}


Bool CTPatchedInterp::interpolate(Int fld, Int spw, Double time, const Vector<Double>& freq) {

  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::interpolate(...,freq)" << endl;

  // Force to fld=0, for now  (only one element on fld axis of internal arrays)
  fld=0;

  // Ensure freq result Array is properly sized
  //   (no-op if already ok)
  freqResult_(spw,fld).resize(nFPar_,freq.nelements(),nAntOut_);
  freqResFlag_(spw,fld).resize(nPar_,freq.nelements(),nAntOut_);

  Bool newcal(False);
  // Loop over _output_ antennas
  for (Int iAntOut=0;iAntOut<nAntOut_;++iAntOut) {

    // Call time interpolation calculation; resample in freq if new
    //   (fills timeResult_/timeResFlag_ implicitly)
    if (tI_(iAntOut,spw,fld)->interpolate(time)) {

      // Resample in frequency
      Matrix<Float> fR(freqResult_(spw,fld).xyPlane(iAntOut));
      Matrix<Bool> fRflg(freqResFlag_(spw,fld).xyPlane(iAntOut));
      Matrix<Float> tR(timeResult_(spw,fld).xyPlane(iAntOut));
      Matrix<Bool> tRflg(timeResFlag_(spw,fld).xyPlane(iAntOut));
      resampleInFreq(fR,fRflg,freq,tR,tRflg,freqIn_(spw));

      // Calibration is new
      newcal=True;
    }
  }

  // Whole result referred to freq result:
  result_(spw,fld).reference(freqResult_(spw,fld));
  resFlag_(spw,fld).reference(freqResFlag_(spw,fld));

  return newcal;
}


  // Report state
void CTPatchedInterp::state() {

  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::state()" << endl;

  cout << "-state--------" << endl;
  cout << boolalpha << "isCmplx_ = " << isCmplx_ << endl;
  cout << "nPar_ = " << nPar_ << endl;
  cout << "nFPar_ = " << nFPar_ << endl;
  cout << "nFldOut_ = " << nFldOut_ << endl;
  cout << "nFldIn_ = " << nFldIn_ << endl;
  cout << "nSpwOut_ = " << nSpwOut_ << endl;
  cout << "nSpwIn_ = " << nSpwIn_ << endl;
  cout << "nAntOut_ = " << nAntOut_ << endl;
  cout << "nAntIn_ = " << nAntIn_ << endl;
  cout << "fldMap_ = " << fldMap_ << endl;
  cout << "spwMap_ = " << spwMap_ << endl;
  cout << "antMap_ = " << antMap_ << endl;

}

void CTPatchedInterp::sliceTable() {

  if (CTPATCHEDINTERPVERB) cout << "  CTPatchedInterp::sliceTable()" << endl;

  // This method generates per-spw, per-antenna (and eventually per-field?)
  //   caltables.  

  // Ensure time sort of input table
  //  TBD (here or inside loop?)

  // Indexed by the spws, ants in the cal table (pre-mapped)
  ctSlices_.resize(nAntIn_,nSpwIn_);

  // Set up iterator
  //  TBD: handle baseline-based case!
  Block<String> sortcol(2);
  sortcol[0]="SPECTRAL_WINDOW_ID";
  sortcol[1]="ANTENNA1";
  ROCTIter ctiter(ct_,sortcol);

  while (!ctiter.pastEnd()) {
    Int ispw=ctiter.thisSpw();
    Int iant=ctiter.thisAntenna1();
    ctSlices_(iant,ispw)=ctiter.table();
    ctiter.next();
  }

}

// Initialize by iterating over the supplied table
void CTPatchedInterp::initialize() {

  if (CTPATCHEDINTERPVERB) cout << "  CTPatchedInterp::initialize()" << endl;

  // Resize working arrays
  result_.resize(nSpwOut_,nFldOut_);
  resFlag_.resize(nSpwOut_,nFldOut_);
  timeResult_.resize(nSpwOut_,nFldOut_);
  timeResFlag_.resize(nSpwOut_,nFldOut_);
  freqResult_.resize(nSpwOut_,nFldOut_);
  freqResFlag_.resize(nSpwOut_,nFldOut_);

  // Size/initialize interpolation engines
  tI_.resize(nAntOut_,nSpwOut_,nFldOut_);
  tI_.set(NULL);

  for (Int iFldOut=0;iFldOut<nFldOut_;++iFldOut) {
    for (Int iSpwOut=0;iSpwOut<nSpwOut_;++iSpwOut) { 
      // Size up the timeResult_ Cube (NB: channel shape matches Cal Table)
      if (timeResult_(iSpwOut,iFldOut).nelements()==0) {
	timeResult_(iSpwOut,iFldOut).resize(nFPar_,nChanIn_(spwMap_(iSpwOut)),nAntOut_);
	timeResFlag_(iSpwOut,iFldOut).resize(nPar_,nChanIn_(spwMap_(iSpwOut)),nAntOut_);
      }
      for (Int iAntOut=0;iAntOut<nAntOut_;++iAntOut) {

	// Realize the mapping (no field mapping yet!)
	NewCalTable& ict(ctSlices_(antMap_(iAntOut),spwMap_(iSpwOut)));
	if (!ict.isNull()) {
	  Matrix<Float> tR(timeResult_(iSpwOut,iFldOut).xyPlane(iAntOut));
	  Matrix<Bool> tRf(timeResFlag_(iSpwOut,iFldOut).xyPlane(iAntOut));
	  tI_(iAntOut,iSpwOut,iFldOut)=new CTTimeInterp1(ict,timeType_,tR,tRf);
	}
	else
	  cout << "Ant,Spw="<<iAntOut<<","<<iSpwOut<<" have no calibration mapping!!" << endl; 
      } // iAntOut
    } // iSpwOut
  } // iFldOut
}

// Resample in frequency
void CTPatchedInterp::resampleInFreq(Matrix<Float>& fres,Matrix<Bool>& fflg,const Vector<Double>& fout,
				     Matrix<Float>& tres,Matrix<Bool>& tflg,const Vector<Double>& fin) {

  if (CTPATCHEDINTERPVERB) cout << "  CTPatchedInterp::resampleInFreq(...)" << endl;

  // if no good solutions coming in, return flagged
  if (nfalse(tflg)==0) {
    fflg.set(True);
    return;
  }

  Int flparmod=nFPar_/nPar_;    // for indexing the flag Matrices on the par axis

  //cout << "nFPar_,nPar_,flparmod = " << nFPar_ << "," << nPar_ << "," << flparmod << endl;

  fres=0.0;

  for (Int ifpar=0;ifpar<nFPar_;++ifpar) {

    // Slice by par (each Matrix row)
    Vector<Float> fresi(fres.row(ifpar)), tresi(tres.row(ifpar));
    Vector<Bool> fflgi(fflg.row(ifpar/flparmod)), tflgi(tflg.row(ifpar/flparmod));

    // Mask time result by flags
    Vector<Double> mfin=fin(!tflgi).getCompressedArray();
    mfin/=1.0e9; // in GHz
    Vector<Float> mtresi=tresi(!tflgi).getCompressedArray();

    // Trap case of same in/out frequencies
    if (fout.nelements()==mfin.nelements() && allNear(fout,mfin,1.e-10)) {
      // Just copy
      fresi=mtresi;
      fflgi.set(False);  // none are flagged
      continue;
    }

    // TBD: ensure phases tracked on freq axis...

    // TBD: handle flags carefully!
    // For now,just unset them
    fflgi.set(False);
    
    // Always use nearest on edges
    // TBD: trap cases where frequencies don't overlap at all
    //     (fout(hi)<mfin(0) || fout(lo)> mfin(ihi))

    Int lo=0;
    while (fout(lo)<=mfin(0))
      fresi(lo++)=mtresi(0);
    Int hi=fresi.nelements()-1;
    Int ihi=mtresi.nelements()-1;
    while (fout(hi)>=mfin(ihi))
      fresi(hi--)=mtresi(ihi);

    //    cout << "lo, hi = " << lo << ","<<hi << endl;

    // Use InterpolateArray1D to fill in the middle
    IPosition blc(1,lo), trc(1,hi);
    Vector<Float> slfresi(fresi(blc,trc));
    Vector<Double> slfout(fout(blc,trc));

    InterpolateArray1D<Double,Float>::interpolate(slfresi,slfout,mfin,mtresi,1);

    // TBD: Handle _large_ flag ranges (i.e., exceeding user's freq interp reach)

  }


}




} //# NAMESPACE CASA - END

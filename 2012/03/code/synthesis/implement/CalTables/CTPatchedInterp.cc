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
				 VisCalEnum::MatrixType mtype,
				 Int nPar,
				 const String& timetype,
				 const String& freqtype,
				 Int nMSSpw,
				 Vector<Int> spwmap) :
  ct_(ct),
  mtype_(mtype),
  isCmplx_(False),
  nPar_(nPar),
  nFPar_(nPar),
  timeType_(timetype),
  freqType_(freqtype),
  nChanIn_(),
  freqIn_(),
  nFldOut_(1),  // for now, only one element on field axes...
  nMSSpw_(nMSSpw),
  nAntOut_(0),
  nFldIn_(1),
  nCTSpw_(ct.spectralWindow().nrow()),
  nAntIn_(ct.antenna().nrow()),
  nElemIn_(0),
  spwInOK_(),
  fldMap_(),
  spwMap_(),
  antMap_(),
  elemMap_(),
  conjTab_(),
  currTime_(),
  result_(),
  resFlag_(),
  tI_()
{
  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::CTPatchedInterp()" << endl;

  ia1dmethod_=ftype(freqType_);

  //  cout << "ia1dmethod_ = " << ia1dmethod_ << endl;

  // Assume MS dimensions in ant are same (for now)
  //  TBD: Supply something from the MS to discern this...
  nAntOut_=nAntIn_;

  switch(mtype_) {
  case VisCalEnum::GLOBAL: {
    throw(AipsError("CTPatchedInterp::ctor: No non-Mueller/Jones yet."));
    break;
  }
  case VisCalEnum::MUELLER: {
    nElemIn_=nAntIn_*(nAntIn_+1)/2;
    nElemOut_=nAntOut_*(nAntOut_+1)/2;
    break;
  }
  case VisCalEnum::JONES: {
    nElemIn_=nAntIn_;
    nElemOut_=nAntOut_;
    break;
  }
  }

  // How many _Float_ parameters?
  if (isCmplx_=ct_.keywordSet().asString("ParType")=="Complex")  // Complex input
    nFPar_*=2;  // interpolating 2X as many Float values

  // Set channel/freq info
  CTSpWindowColumns ctspw(ct_.spectralWindow());
  ctspw.numChan().getColumn(nChanIn_);
  freqIn_.resize(nCTSpw_);
  for (uInt iCTspw=0;iCTspw<ctspw.nrow();++iCTspw) 
    ctspw.chanFreq().get(iCTspw,freqIn_(iCTspw),True);

  // Initialize caltable slices
  sliceTable();

  // Set spwmap
  setSpwMap(spwmap);

  // Set default maps
  setDefFldMap();
  setDefAntMap();
  setElemMap();


  // Setup mapped interpolators
  // TBD: defer this to later, so that spwmap, etc. can be revised
  //   before committing to the interpolation engines
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

Bool CTPatchedInterp::interpolate(Int fld, Int spw, Double time, Double freq) {

  if (CTPATCHEDINTERPVERB) cout << "CTPatchedInterp::interpolate(...)" << endl;

  // Force to fld=0, for now  (only one element on fld axis of internal arrays)
  fld=0;

  // TBD: set currTime (also in freq-dep version)

  Bool newcal(False);
  // Loop over _output_ elements
  for (Int iElemOut=0;iElemOut<nElemOut_;++iElemOut) {
    // Call fully _patched_ time-interpolator, keeping track of 'newness'
    //  fills timeResult_/timeResFlag_ implicitly
    if (freq>0.0)
      newcal|=tI_(iElemOut,spw,fld)->interpolate(time,freq);
    else
      newcal|=tI_(iElemOut,spw,fld)->interpolate(time);
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
  freqResult_(spw,fld).resize(nFPar_,freq.nelements(),nElemOut_);
  freqResFlag_(spw,fld).resize(nPar_,freq.nelements(),nElemOut_);

  Bool newcal(False);
  // Loop over _output_ antennas
  for (Int iElemOut=0;iElemOut<nElemOut_;++iElemOut) {
    // Call time interpolation calculation; resample in freq if new
    //   (fills timeResult_/timeResFlag_ implicitly)
    if (tI_(iElemOut,spw,fld)->interpolate(time)) {

      // Resample in frequency
      Matrix<Float> fR(freqResult_(spw,fld).xyPlane(iElemOut));
      Matrix<Bool> fRflg(freqResFlag_(spw,fld).xyPlane(iElemOut));
      Matrix<Float> tR(timeResult_(spw,fld).xyPlane(iElemOut));
      Matrix<Bool> tRflg(timeResFlag_(spw,fld).xyPlane(iElemOut));
      resampleInFreq(fR,fRflg,freq,tR,tRflg,freqIn_(spwMap_(spw)));

      // Calibration is new
      newcal=True;
    }
  }

  // Whole result referred to freq result:
  result_(spw,fld).reference(freqResult_(spw,fld));
  resFlag_(spw,fld).reference(freqResFlag_(spw,fld));

  return newcal;
}

// spwOK info for users
Bool CTPatchedInterp::spwOK(Int spw) const {

  if (spw<Int(spwMap_.nelements()))
    return this->spwInOK(spwMap_(spw));

  // Something wrong...
  return False;

}
Bool CTPatchedInterp::spwInOK(Int spw) const {

  if (spw<Int(spwInOK_.nelements()))
    return spwInOK_(spw);

  // Something wrong
  return False;

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
  cout << "nMSSpw_ = " << nMSSpw_ << endl;
  cout << "nCTSpw_ = " << nCTSpw_ << endl;
  cout << "nAntOut_ = " << nAntOut_ << endl;
  cout << "nAntIn_ = " << nAntIn_ << endl;
  cout << "nElemOut_ = " << nElemOut_ << endl;
  cout << "nElemIn_ = " << nElemIn_ << endl;
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
  ctSlices_.resize(nElemIn_,nCTSpw_);

  // Initialize spwInOK_
  spwInOK_.resize(nCTSpw_);
  spwInOK_.set(False);

  // Set up iterator
  //  TBD: handle baseline-based case!
  Block<String> sortcol;
  switch(mtype_) {
  case VisCalEnum::GLOBAL: {
    throw(AipsError("CTPatchedInterp::sliceTable: No non-Mueller/Jones yet."));
    //    sortcol.resize(1);
    //    sortcol[0]="SPECTRAL_WINDOW_ID";
    break;
  }
  case VisCalEnum::MUELLER: {
    sortcol.resize(3);
    sortcol[0]="SPECTRAL_WINDOW_ID";
    sortcol[1]="ANTENNA1";
    sortcol[2]="ANTENNA2";
    ROCTIter ctiter(ct_,sortcol);
    while (!ctiter.pastEnd()) {
      Int ispw=ctiter.thisSpw();
      Int iant1=ctiter.thisAntenna1();
      Int iant2=ctiter.thisAntenna2();
      Int ibln=blnidx(iant1,iant2,nAntIn_);
      ctSlices_(ibln,ispw)=ctiter.table();
      spwInOK_(ispw)=(spwInOK_(ispw) || ctSlices_(ibln,ispw).nrow()>0);
      ctiter.next();
    }    
    break;
  }
  case VisCalEnum::JONES: {
    sortcol.resize(2);
    sortcol[0]="SPECTRAL_WINDOW_ID";
    sortcol[1]="ANTENNA1";
    ROCTIter ctiter(ct_,sortcol);
    while (!ctiter.pastEnd()) {
      Int iCTspw=ctiter.thisSpw();
      Int iant=ctiter.thisAntenna1();
      ctSlices_(iant,iCTspw)=ctiter.table();
      spwInOK_(iCTspw)=(spwInOK_(iCTspw) || ctSlices_(iant,iCTspw).nrow()>0);
      ctiter.next();
    }    
    break;
  }
  }

}

// Initialize by iterating over the supplied table
void CTPatchedInterp::initialize() {

  if (CTPATCHEDINTERPVERB) cout << "  CTPatchedInterp::initialize()" << endl;

  // Resize working arrays
  result_.resize(nMSSpw_,nFldOut_);
  resFlag_.resize(nMSSpw_,nFldOut_);
  timeResult_.resize(nMSSpw_,nFldOut_);
  timeResFlag_.resize(nMSSpw_,nFldOut_);
  freqResult_.resize(nMSSpw_,nFldOut_);
  freqResFlag_.resize(nMSSpw_,nFldOut_);

  // Size/initialize interpolation engines
  tI_.resize(nElemOut_,nMSSpw_,nFldOut_);
  tI_.set(NULL);


  Bool reportBadSpw(False);
  for (Int iFldOut=0;iFldOut<nFldOut_;++iFldOut) {
    for (Int iMSSpw=0;iMSSpw<nMSSpw_;++iMSSpw) { 

      // Only if the required CT spw is available
      if (this->spwOK(spwMap_(iMSSpw))) {
	
	// Size up the timeResult_ Cube (NB: channel shape matches Cal Table)
	if (timeResult_(iMSSpw,iFldOut).nelements()==0) {
	  timeResult_(iMSSpw,iFldOut).resize(nFPar_,nChanIn_(spwMap_(iMSSpw)),nElemOut_);
	  timeResFlag_(iMSSpw,iFldOut).resize(nPar_,nChanIn_(spwMap_(iMSSpw)),nElemOut_);
	}
	for (Int iElemOut=0;iElemOut<nElemOut_;++iElemOut) {
	  // Realize the mapping (no field mapping yet!)
	  NewCalTable& ict(ctSlices_(elemMap_(iElemOut),spwMap_(iMSSpw)));
	  if (!ict.isNull()) {
	    Matrix<Float> tR(timeResult_(iMSSpw,iFldOut).xyPlane(iElemOut));
	    Matrix<Bool> tRf(timeResFlag_(iMSSpw,iFldOut).xyPlane(iElemOut));
	    tI_(iElemOut,iMSSpw,iFldOut)=new CTTimeInterp1(ict,timeType_,tR,tRf);
	  }
	  else
	    cout << "Elem,Spw="<<iElemOut<<","<<iMSSpw<<" have no calibration mapping!!" << endl; 
	  
	} // iElemOut
      } // spwOK
      else
	reportBadSpw=True;
    } // iMSSpw
  } // iFldOut

  if (reportBadSpw) {
    cout << "The following MS spws have no corresponding cal spws: ";
    for (Int iMSSpw=0;iMSSpw<nMSSpw_;++iMSSpw)
      if (!this->spwOK(spwMap_(iMSSpw))) cout << iMSSpw << " ";
    cout << endl;
  }

}

void CTPatchedInterp::setSpwMap(Vector<Int>& spwmap) {

  // Set the default spwmap first, then we'll ammend it
  setDefSpwMap();

  Int nspec=spwmap.nelements();

  // Do nothing, if nothing specified (and rely on default)
  if (nspec==0) return;

  // Do nothing f a single -1 is specified
  if (nspec==1 && spwmap(0)==-1) return;


  // Alert user if too many spws specified
  //  TBD
  //  if (spwmap.nelements()>nMSSpw_)
    
  // Handle auto-fanout
  if (spwmap(0)==-999) {
    // Use first OK spw for all MS spws
    Int gspw(0);
    while (!spwInOK(gspw)) ++gspw;
    spwMap_.set(gspw);
  }
  else {
    // First trap out-of-range values
    if (anyLT(spwmap,0))
      throw(AipsError("Please specify positive indices in spwmap."));
    if (anyGE(spwmap,nCTSpw_)) {
      ostringstream o;
      o << "Please specify spw indices <= maximum available ("
	<< (nCTSpw_-1) << " in " << ct_.tableName() << ")";
      throw(AipsError(o.str()));
    }

    // Now fill from spwmap
    if (nspec==1)
      // Use one value for all
      spwMap_.set(spwmap(0));
    else {
      // set as many as are specified
      IPosition blc(1,0);
      IPosition trc(1,min(nspec-1,nMSSpw_-1));
      spwMap_(blc,trc)=spwmap(blc,trc);
    }
  }

  cout << "CTPatchedInterp::setSpwMap: Realized spwMap_ = " << spwMap_ << endl;

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

  Bool unWrapPhase=flparmod>1;

  //  cout << "nFPar_,nPar_,flparmod = " << nFPar_ << "," << nPar_ << "," << flparmod << endl;

  fres=0.0;

  for (Int ifpar=0;ifpar<nFPar_;++ifpar) {

    // Slice by par (each Matrix row)
    Vector<Float> fresi(fres.row(ifpar)), tresi(tres.row(ifpar));
    Vector<Bool> fflgi(fflg.row(ifpar/flparmod)), tflgi(tflg.row(ifpar/flparmod));

    // Mask time result by flags
    Vector<Double> mfin=fin(!tflgi).getCompressedArray();

    if (mfin.nelements()==0) {
      cout << ifpar << " All chans flagged!" << endl;
      // Everything flagged this par
      //  Just flag, zero and go on to the next one
      fflgi.set(True);
      fresi.set(0.0);
      continue;
    }

    mfin/=1.0e9; // in GHz
    Vector<Float> mtresi=tresi(!tflgi).getCompressedArray();

    // Trap case of same in/out frequencies
    if (fout.nelements()==mfin.nelements() && allNear(fout,mfin,1.e-10)) {
      // Just copy
      fresi=mtresi;
      fflgi.set(False);  // none are flagged
      continue;
    }

    if (ifpar%2==1 && unWrapPhase) {
      for (Int i=1;i<mtresi.nelements();++i) {
        while ( (mtresi(i)-mtresi(i-1))>C::pi ) mtresi(i)-=C::_2pi;
        while ( (mtresi(i)-mtresi(i-1))<-C::pi ) mtresi(i)+=C::_2pi;
      }
    }


    // TBD: ensure phases tracked on freq axis...

    // TBD: handle flags carefully!
    //      (i.e., flag gaps larger than user's "freach")
    // For now,just unset them
    fflgi.set(False);
    
    // Always use nearest on edges
    // TBD: trap cases where frequencies don't overlap at all
    //     (fout(hi)<mfin(0) || fout(lo)> mfin(ihi))..... already OK (lo>hi)?
    // TBD: optimize the following by forming Slices in the
    //     while loops and doing Array assignment once afterwords

    Int nfreq=fout.nelements();
    Int lo=0;
    Int hi=fresi.nelements()-1;
    Double inlo(mfin(0));
    Int ihi=mtresi.nelements()-1;
    Double inhi(mfin(ihi));

    // Handle 'nearest' extrapolation in sideband-dep way
    Bool inUSB(inhi>inlo);
    Bool outUSB(fout(hi)>fout(lo));
    if (inUSB) {
      if (outUSB) {
	while (lo<nfreq && fout(lo)<=inlo) fresi(lo++)=mtresi(0);
	while (hi>-1 && fout(hi)>=inhi) fresi(hi--)=mtresi(ihi);
      }
      else { // "outLSB"
	while (lo<nfreq && fout(lo)>=inhi) fresi(lo++)=mtresi(ihi);
	while (hi>-1 && fout(hi)<=inlo) fresi(hi--)=mtresi(0);
      }
    }
    else {  // "inLSB"
      if (outUSB) {
	while (lo<nfreq && fout(lo)<=inhi) fresi(lo++)=mtresi(ihi);
	while (hi>-1 && fout(hi)>=inlo) fresi(hi--)=mtresi(0);
      }
      else {  // "outLSB"
	while (lo<nfreq && fout(lo)>=inlo) fresi(lo++)=mtresi(0);
	while (hi>-1 && fout(hi)<=inhi) fresi(hi--)=mtresi(ihi);
      }
    }

    //    cout << "lo, hi = " << lo << ","<<hi << endl;

    if (lo>hi) return; // Frequencies didn't overlap, nearest was used

    // Use InterpolateArray1D to fill in the middle
    IPosition blc(1,lo), trc(1,hi);
    Vector<Float> slfresi(fresi(blc,trc));
    Vector<Double> slfout(fout(blc,trc));

    InterpolateArray1D<Double,Float>::interpolate(slfresi,slfout,mfin,mtresi,ia1dmethod_);

  }
}

void CTPatchedInterp::setElemMap() {
 
  // Ensure the antMap_ is set
  if (antMap_.nelements()!=uInt(nAntOut_))
    setDefAntMap();

  // Handle cases
  switch(mtype_) {
  case VisCalEnum::GLOBAL: {
    throw(AipsError("CTPatchedInterp::sliceTable: No non-Mueller/Jones yet."));
    break;
  }
  case VisCalEnum::MUELLER: {
    elemMap_.resize(nElemOut_);
    conjTab_.resize(nElemOut_);
    conjTab_.set(False);
    Int iElemOut(0),a1in(0),a2in(0);
    for (Int iAntOut=0;iAntOut<nAntOut_;++iAntOut) {
      a1in=antMap_(iAntOut);
      for (Int jAntOut=iAntOut;jAntOut<nAntOut_;++jAntOut) {
	a2in=antMap_(jAntOut);
	if (a1in<=a2in)
	  elemMap_(iElemOut)=blnidx(a1in,a2in,nAntOut_);
	else {
	  elemMap_(iElemOut)=blnidx(a2in,a1in,nAntOut_);
	  conjTab_(iElemOut)=True;  // we must conjugate Complex params!
	}
	++iElemOut;
      } // jAntOut
    } // iAntOut
    break;
  }    
  case VisCalEnum::JONES: {
    // Just reference the antMap_
    elemMap_.reference(antMap_);
    break;
  }
  } // switch
}


InterpolateArray1D<Double,Float>::InterpolationMethod CTPatchedInterp::ftype(String& strtype) {
  if (strtype=="nearest")
    return InterpolateArray1D<Double,Float>::nearestNeighbour;
  if (strtype=="linear")
    return InterpolateArray1D<Double,Float>::linear;
  if (strtype=="cubic")
    return InterpolateArray1D<Double,Float>::cubic;
  if (strtype=="spline")
    return InterpolateArray1D<Double,Float>::spline;

  cout << "Using linear for freq interpolation as last resort." << endl;
  return InterpolateArray1D<Double,Float>::linear;


}



} //# NAMESPACE CASA - END

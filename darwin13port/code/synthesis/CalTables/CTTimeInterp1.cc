//# CTTimeInterp1.cc: Implementation of CTTimeInterp1.h
//# Copyright (C) 2012                                     
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

#include <synthesis/CalTables/CTTimeInterp1.h>
#include <synthesis/CalTables/CTMainColumns.h>
#include <synthesis/CalTables/RIorAParray.h>
#include <casa/aips.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <scimath/Functionals/Interpolate1D.h>
#include <scimath/Functionals/ScalarSampledFunctional.h>
#include <scimath/Functionals/ArraySampledFunctional.h>

#define CTTIMEINTERPVERB1 False

namespace casa { //# NAMESPACE CASA - BEGIN                                                   

// Null ctor 
//CTTimeInterp1::CTTimeInterp1() {};  

// From NewCalTable
CTTimeInterp1::CTTimeInterp1(NewCalTable& ct,
			     const String& timetype,
			     Array<Float>& result,
			     Array<Bool>& rflag) :
  ct_(ct),
  mcols_p(NULL),
  timeType_(timetype),
  currTime_(-999.0),
  currIdx_(-1),
  lastWasExact_(False),
  timeRef_(0.0),
  timelist_(),
  domain_(2,0.0),
  flaglist_(),
  tInterpolator_p(NULL),
  cfreq_(-1.0),
  cycles_(),
  result_(),
  rflag_()
{

  if (CTTIMEINTERPVERB1) cout << "CTTimeInterp1::ctor()" << endl;

  // Access to main columns  (move to NewCalTable?)
  mcols_p = new ROCTMainColumns(ct_);

  // Flags
  mcols_p->flag().getColumn(flaglist_);

  // Record referenced timelist_
  //  TBD: handle flagged times
  Vector<Double> dtime;
  mcols_p->time().getColumn(dtime);
  domain_(0)=min(dtime);
  domain_(1)=max(dtime);
  timeRef_=86400.0*floor(dtime(0)/86400.0);
  dtime-=timeRef_;  // Relative to reference time
  timelist_.resize(dtime.shape());
  convertArray(timelist_,dtime);  // store referenced times as Floats

  // Create the _time_ interpolator
  //  TBD:  f(par) (because flags may be different for each par...)
  tInterpolator_p = new Interpolate1D<Float,Array<Float> >();
  setInterpType(timeType_);

  // Get fiducial frequency

  // Get cycles, if nec.
  if (timetype.contains("PD")) {
    Int spw=mcols_p->spwId()(0);
    MSSpWindowColumns spwcol(ct_.spectralWindow());
    Int nChan=spwcol.numChan()(spw);
    cfreq_=Vector<Double>(spwcol.chanFreq()(spw))(nChan/2);
    //cout << "cfreq_ = " << cfreq_ << endl;
    mcols_p->cycles(cycles_);
    //cout << "ant = " << mcols_p->antenna1()(0) << ":  cycles_ = " << cycles_ << endl;
  }

  // Reference supplied arrays for results
  //  Elsewhere, must always use strict (non-shape-changing) assignment for these!
  //  TBD: verify shapes..
  result_.reference(result);
  rflag_.reference(rflag);

}

// Destructor
CTTimeInterp1::~CTTimeInterp1() {
  if (tInterpolator_p)
    delete tInterpolator_p;
  if (mcols_p)
    delete mcols_p;
};

Bool CTTimeInterp1::interpolate(Double newtime) {

  if (CTTIMEINTERPVERB1) {cout.precision(12);cout << "CTTimeInterp1::interpolate("<<newtime<<"):" << endl;}

  // Don't work unnecessarily
  if (newtime==currTime_)
    return False;  // no change

  if (CTTIMEINTERPVERB1) cout << " newtime is new..." << endl;

  // A new time is specified, so some work may be required

  // Convert supplied time value to Float (referenced to timeRef_)
  Float fnewtime(newtime-timeRef_);

  // Establish registration in time
  Bool exact(False);
  Int newIdx(currIdx_);
  Bool newReg=findTimeRegistration(newIdx,exact,fnewtime);

  if (CTTIMEINTERPVERB1) 
    cout <<boolalpha<< " newReg="<<newReg<< " newIdx="<<newIdx<< " exact="<<exact
	 << " lastWasExact_=" << lastWasExact_ << endl;

  // Update interpolator coeffs if new registr. and not nearest
  if (newReg || (!exact && lastWasExact_)) {
    // Only bother if not 'nearest' nor exact...
    if (!timeType().contains("nearest") && !exact) { 
      ScalarSampledFunctional<Float> xf(timelist_(Slice(newIdx,2)));
      Vector<uInt> rows(2); indgen(rows); rows+=uInt(newIdx);
      Array<Float> ya(mcols_p->fparamArray("",rows));
      ArraySampledFunctional<Array<Float> > yf(ya);
      tInterpolator_p->setData(xf,yf,True);
    }
  }
  else
    // Escape if registration unchanged and 'nearest' or exact
    if (timeType().contains("nearest") || exact) return False;  // no change

  // Now calculate the interpolation result

  if (timeType().contains("nearest") || exact) {
    if (CTTIMEINTERPVERB1) cout << " nearest or exact" << endl;
    Cube<Float> r(mcols_p->fparamArray("",Vector<uInt>(1,newIdx)));
    result_=r.xyPlane(0);
    rflag_=flaglist_.xyPlane(newIdx);
  }
  else {
    if (CTTIMEINTERPVERB1) cout << " non-trivial non-nearest" << endl;
    // Delegate to the interpolator
    result_=(*tInterpolator_p)(fnewtime);
    rflag_=(flaglist_.xyPlane(newIdx) || flaglist_.xyPlane(newIdx+1));
  }

  // Now remember for next round
  currTime_=newtime;
  currIdx_=newIdx;
  lastWasExact_=exact;

  return True;
 
}

Bool CTTimeInterp1::interpolate(Double newtime, Double freq) {

  Bool newcal=this->interpolate(newtime);

  if (newcal && timeType().contains("PD"))
    applyPhaseDelay(freq);

  return newcal;

}

void CTTimeInterp1::state(Bool verbose) {

  cout << endl << "-state---------" << endl;
  cout.precision(12);
  cout << " timeType_= " << timeType_ << endl;
  cout << " ntime    = " << timelist_.nelements() << endl;
  cout << " currTime_= " << currTime_ << " (" << Float(currTime_-timeRef_) << ")" << endl;
  cout << " currIdx_ = " << currIdx_ << endl;
  cout << " timeRef_ = " << timeRef_ << endl;
  cout << " domain_  = " << domain_ << endl;
  if (verbose) {
    cout << " result_  = " << result_ << endl;
    cout << boolalpha;
    cout << " rflag_   = " << rflag_ << endl;
  }
  cout << "---------------" << endl;
}

void CTTimeInterp1::setInterpType(String strtype) {
  timeType_=strtype;
  currTime_=-999.0; // ensure force refreshed calculation
  if (strtype.contains("nearest")) {
    tInterpolator_p->setMethod(Interpolate1D<Float,Array<Float> >::nearestNeighbour);
    return;
  }
  if (strtype.contains("linear")) {
    tInterpolator_p->setMethod(Interpolate1D<Float,Array<Float> >::linear);
    return;
  }
  throw(AipsError("Unknown interp type: '"+strtype+"'!!"));
}

Bool CTTimeInterp1::findTimeRegistration(Int& idx,Bool& exact,Float newtime) {

  if (CTTIMEINTERPVERB1) cout << " CTTimeInterp1::findTimeRegistration()" << endl;

  Int ntime=timelist_.nelements();

  // If only one time in timelist, that's it, and its exact (behave as nearest)
  if (ntime==1) {
    idx=0;
    exact=True;
  } 
  else {
    // More than one element in timelist, find the right one:

    // Behave as nearest outside absolute range of available calibration   
    //   to avoid wild extrapolation, else search for the correct soln slot
    if (newtime<timelist_(0)) {
      // Before first timestamp, use first slot as nearest one
      idx=0;
      exact=True;
    }
    else if (newtime>timelist_(ntime-1)) {
      // After last timestamp, use last slot as nearest one
      idx=ntime-1;
      exact=True;
    }
    else
      // Find index in timelist where time would be:
      //  TBD: can we use last result to help this?
      idx=binarySearch(exact,timelist_,newtime,ntime,0);

    // If not (yet) an exact match...
    if ( !exact ) {

      // identify this time via index just prior
      if (idx>0) idx--;

      // If nearest, fine-tune slot to actual nearest:
      if ( timeType().contains("nearest") ) {
	//        exact=True;   // Nearest behaves like exact match
        if (idx!=(ntime-1) && 
            (timelist_(idx+1)-newtime) < (newtime-timelist_(idx)) )
          idx++;
      } else {
        // linear modes require one later slot
        if (idx==ntime-1) idx--;
      }
    }

  }
  // Return if new
  return (idx!=currIdx_);

}

void CTTimeInterp1::applyPhaseDelay(Double freq) {

  if (freq>0.0) {
    IPosition blc(2,1,0),trc(result_.shape()-1),stp(2,2,1);
    Matrix<Float> rph(result_(blc,trc,stp));
    if (cfreq_>0.0) {
      rph+=cycles_.xyPlane(currIdx_);
      rph*=Float(freq/cfreq_);
    }
  }
  else
    throw(AipsError("CTTimeInterp1::applyPhaseDelay: invalid frequency."));

}

} //# NAMESPACE CASA - END

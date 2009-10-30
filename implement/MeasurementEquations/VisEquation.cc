//# VisEquation:  Vis Equation
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
//# $Id: VisEquation.cc,v 19.18 2006/01/13 01:06:55 gmoellen Exp $


#include <synthesis/MeasurementEquations/VisEquation.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>

#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisBuffer.h>
//#include <casa/Quanta/MVTime.h>
#include <casa/iostream.h>

#include <casa/OS/Timer.h>

#define VISEQPRTLEV 0

namespace casa { //# NAMESPACE CASA - BEGIN

// ***************************************************************************
// ********************  Start of public member functions ********************
// ***************************************************************************

//----------------------------------------------------------------------
VisEquation::VisEquation() :
  vcpb_(NULL), 
  napp_(0),
  lfd_(-1),
  rfd_(9999),
  freqAveOK_(False),
  svc_(NULL),
  pivot_(VisCal::ALL),  // at the sky
  spwOK_(),
  prtlev_(VISEQPRTLEV)
{
  if (prtlev()>0) cout << "VE::VE()" << endl;
};

//----------------------------------------------------------------------
VisEquation::~VisEquation() {};

//---------------------------------------------------------------------- 
VisEquation& VisEquation::operator=(const VisEquation& other)
{
  if(this!=&other) {
    vcpb_=other.vcpb_;
    napp_=other.napp_;
    svc_=other.svc_;
  };
  return *this;
};

//----------------------------------------------------------------------
VisEquation::VisEquation(const VisEquation& other)
{
  operator=(other);
}


//----------------------------------------------------------------------
void VisEquation::setapply(PtrBlock<VisCal*>& vcin) {

  if (prtlev()>0) cout << "VE::setapply()" << endl;

  // Be sure internal pointer points to same PB
  vcpb_ = &vcin;

  // How many VisCals in list?
  napp_=vc().nelements();

  // only if at least one VisCal in list
  if (napp_>0) {

    // only sort if a non-trivial list
    if (napp_>1) {
      
      // A temporary local copy for sorting:
      PtrBlock<VisCal*> lvc;
      lvc.resize(napp_,True,True);
      lvc=vc();
      
      // Sorted index will go here
      Vector<uInt> order(napp_,0);
      
      // Fill in the sort key
      Vector<Int> key(napp_,0);
      for (Int i=0;i<napp_;i++)
	key(i)=Int(vc()[i]->type());
      
      // Do the sort
      {
	Sort sort;
	sort.sortKey(&key(0),TpInt);
	sort.sort(order,uInt(napp_));
      }
      
      // Assign VisCals in sorted order
      if (prtlev()>2) cout << "Sorted VisCals:" << endl;
      vc().set(NULL);
      for (Int i=0;i<napp_;i++) {
	vc()[i]=lvc[order(i)];
	
	if (prtlev()>2) cout << vc()[i]->typeName() << " (" << vc()[i]->type() << ")" << endl;
      }
      
    }
    
    // Maintain spwOK_ list
    // TBD: Should VisEquation know a priori how many spw here? Probably.
    spwOK_.resize();
    spwOK_ = (vc()[0])->spwOK();
    for (Int i=1;i<napp_;++i) 
      spwOK_ = spwOK_ && vc()[i]->spwOK();
    
  }

  // Set up freq-dependence of the Vis Equation
  //  (for currently specified solve/apply types)
  //  TBD: only needed in setsolve?
  //  setFreqDep();

}

//----------------------------------------------------------------------
void VisEquation::setsolve(SolvableVisCal& svc) {

  if (prtlev()>0) cout << "VE::setsolve()" << endl;
  
  // VE's own pointer to solvable component
  svc_=&svc;
  
  // Set up freq-dependence of the Vis Equation
  //  (for currently specified solve/apply components)
  setFreqDep();

}


//----------------------------------------------------------------------
void VisEquation::setPivot(VisCal::Type pivot) {

  if (prtlev()>0) cout << "VE::setPivot()" << endl;
  
  pivot_ = pivot;

}
  



//----------------------------------------------------------------------
// Correct in place the OBSERVED visibilities in a VisBuffer
void VisEquation::correct(VisBuffer& vb) {

  if (prtlev()>0) cout << "VE::correct()" << endl;

  AlwaysAssert(ok(),AipsError);

  if (napp_==0) throw(AipsError("Nothing to Apply"));

  // Ensure correlations in canonical order
  // (this is a no-op if no sort necessary)
  vb.sortCorr();

  // Apply each VisCal in left-to-right order 
  for (Int iapp=0;iapp<napp_;iapp++)
    vc()[iapp]->correct(vb);

  // Ensure correlations restored to original order
  // (this is a no-op if no sort necessary)
  vb.unSortCorr();
  
}

//----------------------------------------------------------------------
// Corrupt in place the MODEL visibilities in a VisBuffer
void VisEquation::corrupt(VisBuffer& vb) {

  if (prtlev()>0) cout << "VE::corrupt()" << endl;

  AlwaysAssert(ok(),AipsError);

  if (napp_==0) throw(AipsError("Nothing to Apply"));

  // Ensure correlations in canonical order
  // (this is a no-op if no sort necessary)
  vb.sortCorr();

  // Apply each VisCal in right-to-left order
  for (Int iapp=(napp_-1);iapp>-1;iapp--)
    vc()[iapp]->corrupt(vb);

  // Ensure correlations restored to original order
  // (this is a no-op if no sort necessary)
  vb.unSortCorr();

}

//----------------------------------------------------------------------
void VisEquation::collapse(VisBuffer& vb) {

  if (prtlev()>0) cout << "VE::collapse()" << endl;

  // Handle origin of model data here:

  /*
  if (doaltmodel)
    if (!skytype)
      vb.setModel(model);;
    // else skytype will originate model when encountered
  else
    // force I/O from MS
    vb.modelVisCube()
  */

  // Ensure required columns are present!
  vb.modelVisCube();
  vb.visCube();
  vb.weightMat();
  
  // Re-calculate weights from sigma column
  // TBD: somehow avoid if not necessary?
  vb.resetWeightMat();

  // Ensure correlations in canonical order
  // (this is a no-op if no sort necessary)
  // TBD: optimize in combo with model origination?
  vb.sortCorr();

  // If we are solving for the polarization:
  //  1. Normalize data and model by I model
  //  2. Set cross-hands to (1,0) so P factors multiplying them
  //     are propogated, and we can solve for pol factors
  if (svc().solvePol())
    svc().setUpForPolSolve(vb);

  // TBD: When warranted, do freqAve before setUpForPolSolve?
  
  // initialize LHS/RHS indices
  Int lidx=0;
  Int ridx=napp_-1;

  // If solve NOT freqDep, and data is, we want
  //  to freqAve as soon as possible before solve;
  //   apply any freqDep cal first
  if ( freqAveOK_ && !svc().freqDepMat() && vb.nChannel()>1 ) {
    
    // Correct OBSERVED data up to last freqDep term on LHS
    //  (type(lfd_) guaranteed < type(svc))
    while (lidx<napp_ && lidx <= lfd_) {
      vc()[lidx]->correct(vb);
      lidx++;
    }
    
    // Corrupt MODEL  data down to last freqDep term on RHS
    //  (type(rfd_) guaranteed >= type(svc))
    while (ridx>-1 && ridx >= rfd_) {
      vc()[ridx]->corrupt(vb);
      ridx--;
    }
    
    // All freq-dep cal has been applied, now freqAve
    vb.freqAveCubes();  // BOTH sides (if present)!
    
  }
  
  // Correct DATA up to solved-for term
  while (lidx<napp_ && vc()[lidx]->type() < svc().type()) {
    vc()[lidx]->correct(vb);
    lidx++;
  }
  
  // Corrupt MODEL down to solved-for term (incl. same type as solved-for term)
  while (ridx>-1    && vc()[ridx]->type() >= svc().type()) {
    vc()[ridx]->corrupt(vb);
    ridx--;
  }
  
}

//----------------------------------------------------------------------
void VisEquation::collapseForSim(VisBuffer& vb) {

  if (prtlev()>0) cout << "VE::collapse()" << endl;

  // Handle origin of model data here (?):

  // Ensure correlations in canonical order
  // (this is a no-op if no sort necessary)
  // TBD: optimize in combo with model origination?
  vb.sortCorr();

  // initialize LHS/RHS indices
  Int lidx=0;
  Int ridx=napp_-1;

  // copy data to model, to be corrupted in place there.
  // 20091030 RI changed skyequation to use Observed.  the below 
  // should not require scratch columns 
  vb.setModelVisCube(vb.visCube());

  // Corrupt Model down to (and including) the pivot
  while (ridx>-1    && vc()[ridx]->type() >= pivot_) {
    vc()[ridx]->corrupt(vb);
    ridx--;
  }
  
  // zero the data. correct will operate in place on data, so 
  // if we don't have an AMueller we don't get anything from this.  
  //vb.setVisCube(0.0);
  // RI KLUDGE FOR BROKEN ANOISE
  vb.setVisCube(Complex(0.0001,0.0));
  
  // Correct DATA up to pivot 
  while (lidx<napp_ && vc()[lidx]->type() < pivot_) {
    vc()[lidx]->correct(vb);
    lidx++;
  }

  // add corrected/scaled data (e.g. noise) to corrupted model
  // vb.modelVisCube()+=vb.visCube();

  // add corrupted Model to corrected/scaled data (i.e.. noise)
  vb.visCube()+=vb.modelVisCube();

  // Put correlations back in original order
  //  (~no-op if already canonical)
  vb.unSortCorr();

}

void VisEquation::state() {

  if (prtlev()>0) cout << "VE::state()" << endl;

  cout << "freqAveOK_ = " << freqAveOK_ << endl;


  // Order in which DATA is corrected
  cout << "Correct order:" << endl;
  if (napp_>0) {
    for (Int iapp=0;iapp<napp_;iapp++)
      cout << "  " << iapp << " " 
	   << vc()[iapp]->typeName() << " (" 
	   << vc()[iapp]->type() << ")" << endl;
  }
  else
    cout << " <none>" << endl;
  
  cout << endl;
  
  cout << "Corrupt order:" << endl;
  if (napp_>0) {
    for (Int iapp=(napp_-1);iapp>-1;iapp--)
      cout << "  " << iapp << " " 
	   << vc()[iapp]->typeName() << " (" 
	   << vc()[iapp]->type() << ")" << endl;
  }
  else
    cout << " <none>" << endl;
  
  cout << endl;
  cout << "Collapse order:" << endl;
  
  if (svc_) {
    Int lidx=0;
    Int ridx=napp_-1;

    if ( !svc().freqDepMat() ) {
      // Correct DATA up to last freqDep term on LHS
      cout << " LHS (pre-freqAve):" << endl;
      if (lidx <= lfd_) 
	while (lidx <= lfd_) {  // up to and including the lfd_th term
	  cout << "  " << lidx << " " 
	       << vc()[lidx]->typeName() << " (" 
	       << vc()[lidx]->type() << ")" 
	       << " (freqDep correct)"
	       << endl;
	  lidx++;
	}
      else
	cout << "  <none>" << endl;
      
      // Corrupt MODEL down to last freqDep term on RHS
      cout << " RHS (pre-freqAve):" << endl;
      if (ridx >= rfd_) 
	while (ridx >= rfd_) {  // down to and including the rfd_th term
	  cout << "  " << (ridx) << " " 
	       << vc()[ridx]->typeName() << " (" 
	       << vc()[ridx]->type() << ")" 
	       << " (freqDep corrupt)"
	       << endl;
	  ridx--;
	}
      else
	cout << "  <none>" << endl;
      
      cout << " Frequency average both sides" << endl;
    }
    
    cout << " LHS:" << endl;
    if (lidx<napp_ && vc()[lidx]->type() < svc().type()) 
      while (lidx<napp_ && vc()[lidx]->type() < svc().type()) {
	cout << "  " << (lidx) << " " 
	     << vc()[lidx]->typeName() << " (" 
	     << vc()[lidx]->type() << ")" 
	     << endl;
	lidx++;
      }
    else
      cout << "  <none>" << endl;
    
    cout << " RHS:" << endl;
    if (ridx>-1 && vc()[ridx]->type() >= svc().type())
      while (ridx>-1 && vc()[ridx]->type() >= svc().type()) {
	cout << "  " << (ridx) << " " 
	     << vc()[ridx]->typeName() << " (" 
	     << vc()[ridx]->type() << ")" 
	     << endl;
	ridx--;
      }
    else
      cout << "  <none>" << endl;

    
    cout << "Ready to solve for " << svc().typeName() << " (" << svc().type() << ")" << endl;

  }
  else
    cout << " <Nothing to solve for>" << endl;

}



//----------------------------------------------------------------------
// Determine residuals of VisBuffer data w.r.t. svc_
void VisEquation::residuals(VisBuffer& vb,
			    Cube<Complex>& R,
			    const Int& chan) {


  if (prtlev()>3) cout << "VE::residuals()" << endl;

  // Trap unspecified solvable term
  if (!svc_)
    throw(AipsError("No calibration term to differentiate."));

  // TBD: support for public non-in-place corruption and
  //        corrupt for specific channel

  throw(AipsError("Simple residual calculation NYI."));

}


// Calculate residuals and differentiated residuals
void VisEquation::diffResiduals(CalVisBuffer& cvb) {

  if (prtlev()>3) cout << "VE::diffResiduals(CVB)" << endl;

  // Trap unspecified solvable term
  if (!svc_)
    throw(AipsError("No calibration term to differentiate."));

  // Delegate to the SVC
  svc().differentiate(cvb);
  
}

// Calculate residuals and differentiated residuals
void VisEquation::diffResiduals(VisBuffer& vb, 
				Cube<Complex>& R, 
				Array<Complex>& dR,
				Matrix<Bool>& Rflg) {

  if (prtlev()>3) cout << "VE::diffResiduals()" << endl;

  // Trap unspecified solvable term
  if (!svc_)
    throw(AipsError("No calibration term to differentiate."));

  // Get trial model corrupt and differentiation from the SVC
  //   (R, dR, and Rflg will be sized by svc_)
  svc().differentiate(vb,R,dR,Rflg);

  // Now, subtract obs'd data from trial model corruption

  // Shape info
  Int nCorr(vb.corrType().nelements());
  Int& nChan(vb.nChannel());
  Int& nRow(vb.nRow());
  IPosition blc(3,0,0,0);
  IPosition trc(3,nCorr-1,nChan-1,nRow-1);

  // Slice if specific channel requested
  if (svc().freqDepPar()) 
    blc(1)=trc(1)=svc().focusChan();

  // If shapes match, subtract, else throw
  //  TBD: avoid subtraction in flagged channels?
  Cube<Complex> Vo(vb.visCube()(blc,trc));
  if (R.shape()==Vo.shape())
    R-=Vo;
  else
    throw(AipsError("Shape mismatch in residual calculation"));
  
}

void VisEquation::diffResiduals(VisBuffer& R,
                                VisBuffer& dR0,
                                VisBuffer& dR1,
                                Matrix<Bool>& Rflg)
{
  if (prtlev()>3) cout << "VE::diffResiduals()" << endl;

  // Trap unspecified solvable term
  if (!svc_)
    throw(AipsError("No calibration term to differentiate."));

  // Get trial model corrupt and differentiation from the SVC
  //   (R, dR, and Rflg will be sized by svc_)
  svc().differentiate(R,dR0,dR1, Rflg);

  // Now, subtract obs'd data from trial model corruption

  // Shape info
  Int nCorr(R.corrType().nelements());
  Int& nChan(R.nChannel());
  Int& nRow(R.nRow());
  IPosition blc(3,0,0,0);
  IPosition trc(3,nCorr-1,nChan-1,nRow-1);

  // Slice if specific channel requested
  if (svc().freqDepPar())
    blc(1)=trc(1)=svc().focusChan();
 // If shapes match, subtract, else throw
  //  TBD: avoid subtraction in flagged channels?
  //  Cube<Complex> Vo(R.visCube()(blc,trc));
//   cout << R.correctedVisCube().shape() << " "
//        << R.visCube().shape() << " "
//        << blc << " " << trc << endl;
//  Cube<Complex> Vo(R.correctedVisCube()(blc,trc));
  Cube<Complex> Vo(R.visCube()(blc,trc));
  Cube<Complex> Res;Res.reference(R.modelVisCube());
  /*
  for(Int i=0;i<Res.shape()(2);i++)
    {
      cout << i
           << " " << Res(0,0,i)
           << " " << Res(3,0,i)
           << " " << R.visCube()(0,0,i)
           << " " << R.visCube()(3,0,i)
           << " " << R.flag()(0,i)
           << " " << R.flag()(3,i)
           << " " << R.antenna1()(i)
           << " " << R.antenna2()(i)
           << " " << R.flagRow()(i)
           << endl;
    }
  exit(0);
  */
  if (Res.shape()==Vo.shape())
    Res-=Vo;
  else
    throw(AipsError("Shape mismatch in residual calculation"));
}


//----------------------------------------------------------------------
// ***************************************************************************
// ********************  Start of protected member functions *****************
// ***************************************************************************


void VisEquation::setFreqDep() {

  if (prtlev()>2) cout << "VE::setFreqDep()" << endl;

  // Nominal freq-dep is none on either side
  lfd_=-1;      // right-most freq-dep term on LHS
  rfd_=napp_;   // left-most freq-dep term on RHS

  // Nominally averaging in frequency before normalization is NOT OK
  //  (we will revise this when we can assert constant MODEL_DATA)
  freqAveOK_=False;

  // Only if there are both apply-able and solve-able terms
  if (svc_ && napp_>0) {

    // freqdep to LEFT of solvable type
    for (Int idx=0;         (idx<napp_ && vc()[idx]->type()<svc().type()); idx++)
      if (vc()[idx]->freqDepMat()) lfd_=idx;
    
    // freqdep to RIGHT of solvable type
    for (Int idx=(napp_-1); (idx>-1    && vc()[idx]->type()>=svc().type()); idx--)
      if (vc()[idx]->freqDepMat()) {
	rfd_=idx;
	// If we will corrupt the model with something freqdep, we can't
	//  frequency average in collapse
	freqAveOK_=False;
      }

  }
  

}

Bool VisEquation::ok() {

  if (napp_ != Int(vc().nelements()))
    return False;

  return(True);
}


} //# NAMESPACE CASA - END


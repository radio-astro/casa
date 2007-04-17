//# VisCalSolver.cc: Implementation of generic visibility solving
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

#include <synthesis/MeasurementComponents/VisCalSolver.h>

#include <msvis/MSVis/VisBuffer.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIter.h>
//#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicMath/Math.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/OS/Path.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#define VCS_PRTLEV 0

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  UVMod Implementations
//

VisCalSolver::VisCalSolver() :
  svb_(NULL),
  ve_(NULL),
  svc_(NULL),
  nTotalPar_(0),
  R_(),dR_(),Rflg_(),
  maxIter_(50),
  chiSq_(0.0),
  chiSqV_(4,0.0),
  lastChiSq_(0.0),dChiSq_(0.0),
  sumWt_(0.0),nWt_(0),
  cvrgcount_(0),
  par_(), parOK_(), lastPar_(), dpar_(),
  grad_(),hess_(),
  lambda_(2.0),
  optstep_(True),
  prtlev_(VCS_PRTLEV)
{
  if (prtlev()>0) cout << "VCS::VCS()" << endl;
}

VisCalSolver::~VisCalSolver() 
{  
  if (prtlev()>0) cout << "VCS::~VCS()" << endl;
}

Bool VisCalSolver::solve(VisEquation& ve, SolvableVisCal& svc, VisBuffer& svb) {

  if (prtlev()>1) cout << "VCS::solve(,,)" << endl;

  /*
  LogSink logsink;
  {
    LogMessage message(LogOrigin("VisCalSolver", "solve"));
    ostringstream o; o<<"Beginning solve...";
    message.message(o);
    logsink.post(message);
  }
  */
  // Pointers to local ve,svc,svb
  ve_=&ve;
  svc_=&svc;
  svb_=&svb;

  // Verify that VisEq has the correct svc:
  // TBD?

  // Initialize everything 
  initSolve();

  // Verify VisBuffer validity for solving
  //   (this sets parOK() on per-antenna basis (for focusChan)
  //    based on data weights and baseline participation)
  Bool oktosolve = svc_->verifyForSolve(*svb_);

  if (oktosolve) {
    
    if (prtlev()>1) cout << "First guess:" << endl
			 << "amp = " << amplitude(par()) << endl
			 << "pha = " << phase(par()) 
			 << endl;
    
    // Iterate solution
    Int iter(0);
    Bool done(False);
    while (!done) {
      
      if (prtlev()>2) cout << " Beginning iteration " << iter 
			   << "---------------------------------" << endl;
      
      // Differentiate the VB and get current Chi2
      differentiate();
      chiSquare();
      dChiSq() = chiSq()-lastChiSq();
      
      // Continuue if we haven't converged
      if (!converged()) {
	
	if (dChiSq()<0.0) {
	  // last step was good...
	  lastChiSq()=chiSq();
	  
	  // so accumulate new grad/hess...
	  accGradHess();
	  
	  //...and adjust lambda downward
	  //	lambda()/=2.0;
	  //	lambda()=0.8;
	  lambda()=1.0;
	}
	else {
	  // last step was bad, revert to previous 
	  revert();
	  //...with a larger lambda
	  //	lambda()*=4.0;
	  lambda()=1.0;
	}
	
	// Solve for the parameter step
	solveGradHess();
	
	// Remember curr pars
	lastPar()=par();
	
	// Refine the step size by exploring chi2 in the
	//  gradient direction
	if (optstep_) //  && cvrgcount_>=3)
	  optStepSize();
	
	// Update current parameters (saves a copy of them)
	updatePar();

      }
      else {
	// Convergence means we're done!
	done=True;
	
	// Accumulate final hessian for error estimation
	//	accGradHess();
	
	//	cout << "hess() = " << hess() << endl;

	if (prtlev()>0) {
	  cout << "Iterations =" << iter << endl;
	  //	cout << "par()=" << par() << endl;
	}

	// Return, signaling success
	return True;
	
      }
      
      // Escape iteration loop via iteration limit
      if (iter==maxIter()) {
	cout << "Reached iteration limit: " << iter << " iterations." << endl;
	done=True;
      }
      
      // Advance iteration counter
      iter++;
    }
    
  }
  else {
    cout << "Insufficient unflagged antennas to proceed with this solve." << endl;
  }

  return False;
    
}
  
void VisCalSolver::initSolve() {
    
  if (prtlev()>2) cout << " VCS::initSolve()" << endl;

  // Get total number of parameters from svc info
  nTotalPar()=svc().nTotalPar();

  if (prtlev()>2)
    cout << "  Total parameters in solve: " << nTotalPar() << endl;

  // Chi2 and weights
  chiSq()=0.0;
  lastChiSq()=DBL_MAX;
  dChiSq()=0.0;
    
  sumWt()=0.0;
  nWt()=0;

  // Link up svc's internal pars with local reference
  //   (only if shape is correct)

  if (svc().solveCPar().nelements()==uInt(nTotalPar())) {
    par().reference(svc().solveCPar().reform(IPosition(1,nTotalPar())));
    parOK().reference(svc().solveParOK().reform(IPosition(1,nTotalPar())));
  }
  else
    throw(AipsError("Solver and SVC cannot synchronize parameters."));

  // Pars
  lastPar().resize(nTotalPar());
  dpar().resize(nTotalPar());
  dpar()=0.0;

  // Gradient and Hessian
  grad().resize(nTotalPar());
  grad()=0.0;

  hess().resize(nTotalPar());
  hess()=0.0;

  // Levenberg-Marquardt factor
  lambda()=2.0;

  // Convergence anticipation
  cvrgcount_=0;

}

void VisCalSolver::residualate() {

  if (prtlev()>2) cout << "  VCS::residualate()" << endl;

  // Delegate to VisEquation
  //  ve().residuals(svb(),R(),Rflag());

  // For now, just use ve.diffResid, until we have
  //  implemented focuschan-aware trial corrupt in SVC
  //  (this will hurt performance a bit)
  ve().diffResiduals(svb(),R(),dR(),Rflg());

}

void VisCalSolver::differentiate() {

  if (prtlev()>2) cout << "  VCS::differentiate()" << endl;

  // Delegate to VisEquation
  ve().diffResiduals(svb(),R(),dR(),Rflg());

  if (prtlev()>6) {  // R, dR
    cout << "   R= " << R() << endl;
    cout << "   dR=" << dR() << endl;
  }

}

void VisCalSolver::chiSquare() {

  if (prtlev()>2) cout << "   VCS::chiSquare()" << endl;

  // NB: Assumes R() is up-to-date

  //  TBD: Review correctness of summing weights 
  //     inside the channel loop?

  // TBD: per-ant/bln chiSq?

  //  cout << "VCS::chiSquare: svc().focusChan() = " << svc().focusChan() << endl;

  chiSq()=0.0;
  chiSqV()=0.0;
  sumWt()=0.0;

  // Shapes for iteration
  IPosition shR(R().shape());
  Int nCorr=shR(0);
  Int nChan=shR(1);
  Int nRow=shR(2);

  ArrayIterator<Bool>  flag(svb().flag(),1);        // fl(chan) by (row)
  ArrayIterator<Float> wtMat(svb().weightMat(),1);  // wt(corr) by (row)
  ArrayIterator<Complex> Rit(R(),1);                // R(corr)  by (chan,row)

  Bool*  flR = svb().flagRow().data();
  Bool*  fl;
  Float* wt;
  Complex *Rp;

  for (Int irow=0;irow<nRow;++irow) { 
    if (!*flR) {
      // This row's wt(corr), flag(chan)
      wt = wtMat.array().data(); 
      //      wt[1]=wt[2]=0.0;
      fl = flag.array().data() + svc().focusChan();
      // Register R for this row, 0th channel
      for (Int ich=0;ich<nChan;++ich) {
	if (!*fl) { 
	  Rp = Rit.array().data();
	  for (Int icorr=0;icorr<nCorr;++icorr) {

	    chiSq()+=Double( (*wt)*real((*Rp)*conj(*Rp)) );
	    //	    chiSq()+=Double( real((*Rp)*conj(*Rp)) );
	    chiSqV()(icorr)+=Double( (*wt)*real((*Rp)*conj(*Rp)) );
	    sumWt()+=Double(*wt);   // for each channel?!
	    
	    //Advance to next corr
	    ++wt;
	    ++Rp;
	  }
	  // Use same wt(corr) vectors for each channel
	  wt-=nCorr;
	}
	// Advance to next channel
	++fl;
	Rit.next();
      }
    }
    // Advance to next row
    ++flR;
    flag.next();
    wtMat.next();
  }

  // Do this?
  chiSq()/=sumWt();

}


Bool VisCalSolver::converged() {

  if (prtlev()>2) cout << "    VCS::converged()" << endl;

  // Change in chi2
  dChiSq() = chiSq()-lastChiSq();
  Float fChiSq(dChiSq()/chiSq());

  //  if (prtlev()>0)
  //    cout << "chiSq: " << chiSq() << " " << chiSqV() << " " << lambda() << endl;

  // Consider convergence if chi2 decreases...
  //  if (dChiSq()<=0.0) {
  if (fChiSq<=0.001) {

    // ...and the change is small:
    if (abs(dChiSq()) < 0.1*chiSq()) {
      ++cvrgcount_;

      //      if (cvrgcount_==2) lambda()=2.0;

      // Four such steps we believe we have converged!
      if (cvrgcount_>3)
	return True;
    }

    if (prtlev()>0)
      cout << "     Good: chiSq=" << chiSq()
	   << " dChiSq=" << dChiSq()
	   << " fChiSq=" << dChiSq()/chiSq()
	   << " cvrgcnt=" << cvrgcount_
	   << " lambda=" << lambda()
	   << endl;
     
  }
  else {
    // (chi2 increased)

    // If a large increase, don't anticipate yet
    if (abs(dChiSq()) > 0.1*chiSq())
      cvrgcount_=0;
    else {
      // anticipate a little less if upward change is small
      //  TBD:  is this right?
      --cvrgcount_;
      cvrgcount_=max(cvrgcount_,0);  // never less than zero
    }

    if (prtlev()>0)
      cout << "     Bad:  chiSq=" << chiSq()
	   << " dChiSq=" << dChiSq()
	   << " fChiSq=" << dChiSq()/chiSq()
	   << " cvrgcnt=" << cvrgcount_
	   << " lambda=" << lambda()
	   << endl;


  }

  // Not yet converged
  return False;

}


void VisCalSolver::accGradHess() {

  if (prtlev()>2) cout << "     VCS::accGradHess()" << endl;

  grad()=0.0;
  hess()=0.0;

  IPosition dRip(dR().shape());

  Int& nRow(dRip(3));
  Int& nChan(dRip(2));
  Int& nPar(dRip(1));   // pars per antenna
  Int& nCorr(dRip(0));

  ArrayIterator<Bool>  flag(svb().flag(),1);        // fl(chan) by (row)
  ArrayIterator<Float> wtMat(svb().weightMat(),1);  // wt(corr) by (row)

  ArrayIterator<Complex> Rit(R(),1);       // R(corr)       by (chan,row)
  
  Array<Complex> dR0, dR1;
  {
    ArrayIterator<Complex> dRit(dR(),4);  // dR(corr,par,chan,row) by (ant)
    dR0.reference(dRit.array());
    dRit.next();
    dR1.reference(dRit.array());
  }
  ArrayIterator<Complex> dR0it(dR0,2);   // dR0(corr,par) by (chan,row)
  ArrayIterator<Complex> dR1it(dR1,2);   // dR1(corr,par) by (chan,row)

  Bool*  flR = svb().flagRow().data();
  Bool*  fl;
  Float* wt;
  Int*   a1 = svb().antenna1().data();
  Int*   a2 = svb().antenna2().data();
  Complex *Rp;
  Complex *dR0p;
  Complex *dR1p;
  DComplex *G1;
  DComplex *G2;
  Double *H1;
  Double *H2;

  //  cout << "flag.array().data() = " << flag.array().data() << endl;
  //  cout << "svc().focusChan()   = " << svc().focusChan() << endl;

  for (Int irow=0;irow<nRow;++irow) { 
    if (!*flR) {
      // Register grad, hess for ants in this baseline
      Int p1((*a1)*nPar);
      Int p2((*a2)*nPar);
      G1 = grad().data() + p1;
      G2 = grad().data() + p2;
      H1 = hess().data() + p1;
      H2 = hess().data() + p2;
      // This row's wt(corr), flag(chan)
      wt = wtMat.array().data(); 
      fl = flag.array().data() + svc().focusChan();
      for (Int ich=0;ich<nChan;++ich) {
	if (!*fl) { 
	  // Register R,dR for this channel, row
	  Rp = Rit.array().data();
	  dR0p = dR0it.array().data();
	  dR1p = dR1it.array().data();

   /*
	  cout << "irow=" << irow << endl;
	  cout << "R=" << Rit.array() << endl;
	  cout << "dR0=" << dR0it.array()
	       << "dR1=" << dR1it.array()
	       << endl;
   */

	  for (int ip=0;ip<nPar;++ip) {

	    for (Int icorr=0;icorr<nCorr;++icorr) {

	      (*G1) += DComplex( (*wt)*((*Rp)  *conj(*dR0p)) );
	      (*G2) += DComplex( (*wt)*((*dR1p)*conj(*Rp)) );
	      (*H1) +=   Double( (*wt)*real((*dR0p)*conj(*dR0p)) );
	      (*H2) +=   Double( (*wt)*real((*dR1p)*conj(*dR1p)) );
     /*
	      (*G1) += DComplex( (*Rp)  *conj(*dR0p) );
	      (*G2) += DComplex( (*dR1p)*conj(*Rp) );
	      (*H1) +=   Double( real((*dR0p)*conj(*dR0p)) );
	      (*H2) +=   Double( real((*dR1p)*conj(*dR1p)) );
     */
    

	      //Advance to next corr
	      ++wt;
	      ++Rp;
	      ++dR0p;++dR1p;
	    }
	    // Advance to next par
	    ++G1; ++G2;
	    ++H1; ++H2;
	    // Use same Rp(corr), wt(corr) vectors for each parameter
	    Rp-=nCorr;
	    wt-=nCorr;
	  }
	  // Accumulate to same grad(par) & hess(par) for each channel
	  G1-=nPar; G2-=nPar;
	  H1-=nPar; H2-=nPar;
	}
	// Advance to next channel
	++fl;
	Rit.next();
	dR0it.next();
	dR1it.next();
      }
    } // !*flgR
    else {
      // Advance over flagged row
      for (Int ich=0;ich<nChan;++ich) {
	Rit.next();
	dR0it.next();
	dR1it.next();
      }
    }
    // Advance to next row
    ++flR;
    ++a1;++a2;
    flag.next();
    wtMat.next();
  }

  if (prtlev()>4) {  // grad, hess
    cout << "      grad= " << grad() << endl;
    cout << "      hess= " << hess() << endl;
  }


/*  
  // This is the slower way with array indexing
  for (Int irow=0;irow<nRow;++irow,++flR,++a1,++a2) {
    if (!*flR) {
      par1=(*a1)*nPar;
      par2=(*a2)*nPar;
      for (Int ich=0;ich<nChan;++ich,++fl) {
	if (!*fl) {
	  for (int ip=0;ip<nPar;++ip) {
	    for (Int icorr=0;icorr<nCorr;++icorr) {
	      grad(par1+ip) += ( Double(wt(icorr,irow))*
				 Double( R()(icorr,ich,irow  )*
					 conj(dR()(icorr,ip,ich,irow,0)) ) );
	      grad(par2+ip) += ( Double(wt(icorr,irow))*
				 Double( dR()(icorr,ip,ich,irow,1)*
					 conj(R()(icorr,ich,irow  )) ) );
	      
	      hess(par1+ip) += ( Double(wt(icorr,irow))*
				 Double( dR()(icorr,ip,ich,irow,0)*
					 conj(dR()(icorr,ip,ich,irow,0)) ) );
	      hess(par2+ip) += ( Double(wt(icorr,irow))*
				 Double( dR()(icorr,ip,ich,irow,1)*
					 conj(dR()(icorr,ip,ich,irow,1)) ) );
	    }
	  }
	}
      }
    }
    else {
      // Advance over flagged row
      fl+=nChan;
    }

  }
*/


}

void VisCalSolver::revert() {

  if (prtlev()>2) cout << "     VCS::revert()" << endl;

  // Recall the last decent parameter set
  //  TBD: the OK flag?
  par()=lastPar();

}

void VisCalSolver::solveGradHess() {

  if (prtlev()>2) cout << "      VCS::solveGradHess()" << endl;

  // TBD: explicit option to avoid lmfact?
  // TBD: pointer (or MaskedArray?) optimization?

  Double lmfact(1.0+lambda());

  lmfact=2.0;

  dpar()=Complex(0.0);
  for (Int ipar=0; ipar<nTotalPar(); ipar++) {
    if (parOK()(ipar) && hess()(ipar)!=0.0) {
      // good hess for this par:
      dpar()(ipar) = grad()(ipar)/hess()(ipar);
      dpar()(ipar)/=lmfact;
    }
    else {
      dpar()(ipar)=0.0; 
      parOK()(ipar)=False;
    }
  }

  // Negate (so updatePar() can _add_)
  dpar()*=Complex(-1.0f);

}

void VisCalSolver::updatePar() {

  if (prtlev()>2) cout << "       VCS::updatePar()" << endl;

  //  if (prtlev()>4) cout << "        old =" << par() << endl;

  //  if (prtlev()>4) cout << "        dpar=" << dpar() << endl;

  // Tell svc to update the par 
  //   (permits svc() to condition the current solutions)
  svc().updatePar(dpar());

  if (prtlev()>4) 
    cout << "        new =" << endl
	 << "amp = " << amplitude(par()) << endl
	 << "pha = " << phase(par()) << endl;

}


void VisCalSolver::optStepSize() {

  Vector<Double> x2(3,-999.0);
  Float step(1.0);
  
  // Starting point is curr chiSq
  x2(0)=chiSq();

  // take nominal step
  par()+=dpar();  
  residualate();
  chiSquare();
  x2(1)=chiSq();

  // If nominal step is an improvement...
  if (x2(1)<x2(0)) {

    // ...double step size until x2 starts increasing
    par()=dpar(); par()*=Complex(2.0*step); par()+=lastPar();
    residualate();
    chiSquare();
    x2(2)=chiSq();
    //    cout <<   "  down:    " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;
    while (x2(2)<x2(1)) {    //  && step<4.0) {
      step*=2.0;
      par()=dpar(); par()*=Complex(2.0*step); par()+=lastPar();
      residualate();
      chiSquare();
      x2(1)=x2(2);
      x2(2)=chiSq();
      //      cout << "  stretch: " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;

    }
  }
  // else nominal step too big, so...
  else {

    // ... contract by halves until we bracket a minimum
    step*=0.5;
    par()=dpar(); par()*=Complex(step); par()+=lastPar();
    residualate();
    chiSquare();
    x2(2)=x2(1);
    x2(1)=chiSq();
    //    cout <<   "  up:       " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;
    while (x2(1)>x2(0)) { //  && step>0.125) {
      step*=0.5;
      par()=dpar(); par()*=Complex(step); par()+=lastPar();
      residualate();
      chiSquare();
      x2(2)=x2(1);
      x2(1)=chiSq();
      //      cout << "  contract: " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;
    }

  }

  // At this point   x2(0) > x2(1) < x2(2), so 
  //   calculate (quadratic) step optimization factor
  Double optfactor(0.0);
  Double optn(x2(2)-x2(1));
  Double optd(x2(0)-2*x2(1)+x2(2));
	      
  if (abs(optd)>0.0)
    optfactor=Double(step)*(1.5-optn/optd);
  
 /*
  cout << "Optimization: " 
       << step << " " 
       << optfactor << " "
       << x2 << " "
       << "(" << min(amplitude(lastPar())) << ") "
       << max(amplitude(dpar())/amplitude(lastPar()))*180.0/C::pi << " ";
 */  

  par()=lastPar();
  
  // Adjust step by the optfactor
  if (optfactor>0.0)
    dpar()*=Complex(optfactor);


  //  cout << max(amplitude(dpar())/amplitude(lastPar()))*180.0/C::pi
  //       << endl;

}


} //# NAMESPACE CASA - END


//# VisCalSolver2.cc: Implementation of generic visibility solving
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

#include <synthesis/MeasurementComponents/VisCalSolver2.h>

#include <msvis/MSVis/VisBuffer.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
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

#define VCS2_PRTLEV 0

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  VisCalSolver2 Implementations
//

VisCalSolver2::VisCalSolver2() :
  SDBs_(NULL),
  ve_(NULL),
  svc_(NULL),
  nPar_(0),
  maxIter_(50),
  chiSq_(0.0),
  chiSqV_(4,0.0),
  lastChiSq_(0.0),dChiSq_(0.0),
  sumWt_(0.0),sumWtV_(4,0.0),nWt_(0),
  cvrgcount_(0),
  par_(), parOK_(), parErr_(), lastPar_(),
  dpar_(), 
  grad_(),hess_(),
  lambda_(2.0),
  optstep_(True),
  prtlev_(VCS2_PRTLEV)
{
  if (prtlev()>0) cout << "VCS2::VCS2()" << endl;
}

VisCalSolver2::~VisCalSolver2() 
{  
  if (prtlev()>0) cout << "VCS2::~VCS2()" << endl;
}

// New VisBuffGroupAcc version
Bool VisCalSolver2::solve(VisEquation& ve, SolvableVisCal& svc, SDBList& sdbs) {

  if (prtlev()>1) cout << "VCS2::solve(,,VBGA)" << endl;

  /*
  LogSink logsink;
  {
    LogMessage message(LogOrigin("VisCalSolver2", "solve"));
    ostringstream o; o<<"Beginning solve...";
    message.message(o);
    logsink.post(message);
  }
  */
  // Pointers to local ve,svc
  ve_=&ve;
  svc_=&svc;
  SDBs_=&sdbs;

  // Verify that VisEq has the correct svc:
  // TBD?

  // Initialize everything 
  initSolve();

  Vector<Float> steplist(maxIter_+2,0.0);
  Vector<Float> rsteplist(maxIter_+2,0.0);

  // Verify Data's validity for solve w.r.t. baselines available
  //   (this sets parOK() on per-antenna basis (for focusChan)
  //    based on data weights and baseline participation)
  Bool oktosolve = svc_->verifyConstraints(*SDBs_);

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
      differentiate2();
      chiSquare2();
      if (chiSq()==0.0) {
	cout << "CHI2 IS SPURIOUSLY ZERO!*************************************" << endl;
	//cout << "R() = " << R() << endl;
	//	cout << "sum(wtmat) = " << sum(wtmat) << endl;
	return False;
      }

      dChiSq() = chiSq()-lastChiSq();

      //      cout << "chi2 = " << chiSq() << " " << dChiSq() << " " << dChiSq()/chiSq() << endl;
      
      // Continuue if we haven't converged
      if (!converged()) {
	
	if (dChiSq()<=0.0) {
	  // last step was good...
	  lastChiSq()=chiSq();
	  
	  // so accumulate new grad/hess...
	  accGradHess2();
	  
	  //...and adjust lambda downward
	  //	lambda()/=2.0;
	  //	lambda()=0.8;
	  lambda()=1.0;
	}
	else {
	  //	  cout << "reverting..." << chiSq() << " " << dChiSq() << " (" << iter << ")" << endl;
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
	  optStepSize2();
	
	// Update current parameters (saves a copy of them)
	updatePar();


	steplist(iter)=max(amplitude(dpar()));
	rsteplist(iter)=max(amplitude(dpar())/amplitude(par()));

      }
      else {
	// Convergence means we're done!
	done=True;

	  cout << "Iterations =" << iter << endl;
	if (prtlev()>0) {
	  cout << "par()=" << par() << endl;
	}


	/*
	cout << " good pars=" << ntrue(parOK())
	     << " iterations=" << iter << endl
	     << " steps=" << steplist(IPosition(1,0),IPosition(1,iter)) 
	     << endl
	     << " rsteps=" << rsteplist(IPosition(1,0),IPosition(1,iter)) 
	     << endl;
	*/   

	// Get parameter errors:
	accGradHess2();
	getErrors();

	// Return, signaling success if at least 1 good solution
	return (ntrue(parOK())>0);
	
      }
      
      // Escape iteration loop via iteration limit
      if (iter==maxIter()) {
	cout << "Reached iteration limit: " << iter << " iterations.  " << endl;
	//	cout << " good pars = " << ntrue(parOK())
	//	     << "  steps = " << steplist
	//	     << endl;
	done=True;
      }
      
      // Advance iteration counter
      iter++;
    }
    
  }
  else {
    cout << " Insufficient unflagged antennas to proceed with this solve." << endl;
  }

  return False;
    
}
  
void VisCalSolver2::initSolve() {
    
  if (prtlev()>2) cout << " VCS2::initSolve()" << endl;

  // Get total number of cal parameters from svc info
  nPar()=svc().nTotalPar();

  if (prtlev()>2)
    cout << "  Total parameters in solve: " << nPar() << endl;

  // Chi2 and weights
  chiSq()=0.0;
  lastChiSq()=DBL_MAX;
  dChiSq()=0.0;
    
  sumWt()=0.0;
  nWt()=0;

  // Link up svc's internal pars with local reference
  //   (only if shape is correct)

  if (svc().solveCPar().nelements()==uInt(nPar())) {
    par().reference(svc().solveCPar().reform(IPosition(1,nPar())));
    parOK().reference(svc().solveParOK().reform(IPosition(1,nPar())));
    parErr().reference(svc().solveParErr().reform(IPosition(1,nPar())));
  }
  else
    throw(AipsError("Solver and SVC cannot synchronize parameters."));

  // Pars

  dpar().resize(nPar());
  dpar()=0.0;

  lastPar().resize(nPar());

  // Gradient and Hessian
  grad().resize(nPar());
  grad()=0.0;

  hess().resize(nPar());
  hess()=0.0;

  // Levenberg-Marquardt factor
  lambda()=2.0;

  // Convergence anticipation
  cvrgcount_=0;

}

void VisCalSolver2::residualate2() {

  //  if (prtlev()>2) cout << "   VCS2::residualate()" << endl;

  // For now, just use ve.diffResid, until we have
  //  implemented focuschan-aware trial corrupt in SVC
  //  (this will hurt performance a bit)

  for (Int isdb=0;isdb<sdbs().nSDB();++isdb) 
    ve().differentiate(sdbs()(isdb));
}

void VisCalSolver2::differentiate2() {

  if (prtlev()>2) cout << "  VCS2::differentiate(SDB version)" << endl;

  // TBD:  Should this be packaged in the SolveDataBuffer such
  //  that is could be called there with a reference to the svc()?
  //  Eg:
  //  sdbs().differentiate(svc());  // an aggregate method in SDBList
  //   ...which then does:
  //      svc.differentiate(this)  (for each SDB)
  //
  //  also consider whether VE is in the loop here?
  //
  //  (don't wind this up in a way that makes it harder to extend....)

  // Delegate to VisEquation
  for (Int isdb=0;isdb<sdbs().nSDB();++isdb)
    ve().differentiate(sdbs()(isdb));

}

void VisCalSolver2::chiSquare2() {

  if (prtlev()>2) cout << "   VCS2::chiSquare(SDB version)" << endl;

  // TBD: per-ant/bln chiSq?

  chiSq()=0.0;
  chiSqV()=0.0;
  sumWt()=0.0;
  sumWtV()=0.0;
  nWt()=0;

  Cube<Complex> R;

  // Loop over SDBs
  for (Int isdb=0;isdb<sdbs().nSDB();++isdb) {

    // Current SDB
    SolveDataBuffer& sdb(sdbs()(isdb));

    R.reference(sdb.residuals());

    // Shapes for iteration
    IPosition shR(R.shape());
    Int nCorr=shR(0);
    Int nChan=shR(1);
    Int nRow=shR(2);

    // Simple indexed accumulation of chiSq
    //  TBD: optimize w.r.t. indexing?
    Double chisq0(0.0);
    for (Int irow=0;irow<nRow;++irow) { 
      if (!sdb.flagRow()(irow)) {
	for (Int ich=0;ich<nChan;++ich) {
	  for (Int icorr=0;icorr<nCorr;++icorr) {
	    if (!sdb.residFlagCube()(icorr,ich,irow)) { 
	      Float& wt(sdb.infocusWtSpec()(icorr,ich,irow));
	      if (wt>0.0) {
		Complex& Ri(R(icorr,ich,irow));

		// This element's contribution
		chisq0=Double(wt*real(Ri*conj(Ri)));  //  cf:  square(abs(R))?  

		// Accumulate per-corr
		chiSqV()(icorr)+=chisq0;
		sumWtV()(icorr)+=wt;
		nWt()++;
	      }	 // wt>0     
	    } // !flag
	  } // icorr
	} // ich
      } // !flagRow
    } // irow

  } // isdb

  // Totals over corrs
  chiSq()=sum(chiSqV());  
  sumWt()=sum(sumWtV());  

}


Bool VisCalSolver2::converged() {

  if (prtlev()>2) cout << "    VCS2::converged()" << endl;

  // Change in chi2
  dChiSq() = chiSq()-lastChiSq();
  Float fChiSq(dChiSq()/chiSq());

  // Consider convergence if chi2 decreases...
  //  if (dChiSq()<=0.0) {
  if (fChiSq<=0.001) {

    // ...and the change is small:
    if (abs(dChiSq()) < 0.1*chiSq()) {
      ++cvrgcount_;

      //      if (cvrgcount_==2) lambda()=2.0;

    }
    
    if (prtlev()>0)
      cout << "     Good: chiSq=" << chiSq()
	   << " dChiSq=" << dChiSq()
	   << " fChiSq=" << dChiSq()/chiSq()
	   << " cvrgcnt=" << cvrgcount_
	   << " lambda=" << lambda()
	   << endl;


    // Five such steps we believe we have converged!
    if (cvrgcount_>5)
      return True;
     
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

void VisCalSolver2::accGradHess2() {

  if (prtlev()>2) cout << "     VCS2::accGradHess(SDB version)" << endl;

  grad()=0.0;
  hess()=0.0;

  Cube<Complex> R;
  Array<Complex> dR;

  // Loop over SDBs
  for (Int isdb=0;isdb<sdbs().nSDB();++isdb) {

    // Current SDB
    SolveDataBuffer& sdb(sdbs()(isdb));

    R.reference(sdb.residuals());
    dR.reference(sdb.diffResiduals());
    
    IPosition dRip(dR.shape());
    
    Int nRow(dRip(3));
    Int nChan(dRip(2));
    Int nParPerAnt(dRip(1));   // pars per antenna
    Int nCorr(dRip(0));

    // Simple indexed accumulation
    for (Int irow=0;irow<nRow;++irow) {
      if (!sdb.flagRow()(irow)) {
	Int a1i= nParPerAnt*sdb.antenna1()(irow);
	Int a2i= nParPerAnt*sdb.antenna2()(irow);
	for (Int ichan=0;ichan<nChan;++ichan) {
	  for (int icorr=0;icorr<nCorr;++icorr) {
	    if (!sdb.residFlagCube()(icorr,ichan,irow)) {
	      Float& wt(sdb.infocusWtSpec()(icorr,ichan,irow));
	      if (wt>0.0) {
		Complex& Ri(R(icorr,ichan,irow));
		for (Int ipar=0;ipar<nParPerAnt;++ipar) {

		  // Accumulate grad and hess for this icorr,ichan,irow,ipar
		  // for a1:
		  Complex& dR1(dR(IPosition(5,icorr,ipar,ichan,irow,0)));
		  grad()(a1i+ipar)+= DComplex(wt*(Ri*conj(dR1)));
		  hess()(a1i+ipar)+= Double(wt*real(dR1*conj(dR1)));
		  // for a2:
		  Complex& dR2(dR(IPosition(5,icorr,ipar,ichan,irow,1)));
		  grad()(a2i+ipar)+= DComplex(wt*(dR2*conj(Ri)));
		  hess()(a2i+ipar)+= Double(wt*real(dR2*conj(dR2)));

		} // ipar
	      } // wt>0
	    } // !flag
	  } // icorr
	} // ichan
      } // !flagRow
    } // irow

  } // isdb

  if (prtlev()>4) {  // grad, hess
    cout << "      grad= " << grad() << endl;
    cout << "      hess= " << hess() << endl;
  }    

}

void VisCalSolver2::revert() {

  if (prtlev()>2) cout << "     VCS2::revert()" << endl;

  // Recall the last decent parameter set
  //  TBD: the OK flag?
  par()=lastPar();

}

void VisCalSolver2::solveGradHess() {

  if (prtlev()>2) cout << "      VCS2::solveGradHess()" << endl;

  // TBD: explicit option to avoid lmfact?
  // TBD: pointer (or MaskedArray?) optimization?

  Double lmfact(1.0+lambda());

  lmfact=2.0;

  dpar()=Complex(0.0);
  for (Int ipar=0; ipar<nPar(); ipar++) {
    if ( parOK()(ipar) && hess()(ipar)!=0.0) {
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

void VisCalSolver2::updatePar() {

  if (prtlev()>2) cout << "       VCS2::updatePar()" << endl;

  //  if (prtlev()>4) cout << "        old =" << par() << endl;

  //  if (prtlev()>4) cout << "        dpar=" << dpar() << endl;



  // Tell svc to update the par 
  //   (permits svc() to condition the current solutions)
  svc().updatePar(dpar());

  if (prtlev()>4) {
    cout << "        abs(dpar()) = " << amplitude(dpar()) << endl;
    cout << "        new amp = " << amplitude(par()) << endl
	 << "            pha = " << phase(par()) << endl;
  }

}


void VisCalSolver2::optStepSize2() {

  if (prtlev()>2) cout << "  VCS2::optStepSize2(SDB version)" << endl;

  Vector<Double> x2(3,-999.0);
  Float step(1.0);
  
  // Starting point is curr chiSq
  x2(0)=chiSq();

  // take nominal step
  par()+=dpar();  
  residualate2();
  chiSquare2();
  x2(1)=chiSq();

  // If nominal step is an improvement...
  if (x2(1)<x2(0)) {

    // ...double step size until x2 starts increasing
    par()=dpar(); par()*=Complex(2.0*step); par()+=lastPar();
    residualate2();
    chiSquare2();
    x2(2)=chiSq();
    if (prtlev()>4)
      cout <<   "  down:    " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;
    while (x2(2)<x2(1)) {    //  && step<4.0) {
      step*=2.0;
      par()=dpar(); par()*=Complex(2.0*step); par()+=lastPar();
      residualate2();
      chiSquare2();
      x2(1)=x2(2);
      x2(2)=chiSq();
      if (prtlev()>4)
	cout << "  stretch: " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;

    }
  }
  // else nominal step too big, so...
  else {

    // ... contract by halves until we bracket a minimum
    step*=0.5;
    par()=dpar(); par()*=Complex(step); par()+=lastPar();
    residualate2();
    chiSquare2();
    x2(2)=x2(1);
    x2(1)=chiSq();
    if (prtlev()>4)
      cout <<   "  up:       " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;
    while (x2(1)>x2(0)) { //  && step>0.125) {
      step*=0.5;
      par()=dpar(); par()*=Complex(step); par()+=lastPar();
      residualate2();
      chiSquare2();
      x2(2)=x2(1);
      x2(1)=chiSq();
      if (prtlev()>4)
	cout << "  contract: " << step << " " << x2-x2(0) << LogicalArray(x2>=x2(0)) <<endl;
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


  if (prtlev()>4) cout << "   optfactor=" << optfactor << endl;


  par()=lastPar();
  
  // Adjust step by the optfactor
  if (optfactor>0.0)
    dpar()*=Complex(optfactor);

  /*
  cout << max(amplitude(dpar())/amplitude(lastPar()))*180.0/C::pi
       << endl;
  */
}

void VisCalSolver2::getErrors() {

  // Number of *REAL* dof
  //  Int nDOF=2*(nWt()-ntrue(parOK()));  // !!!! this is zero for 3 antennas!
  Int nDOF=max(2*(nWt()-ntrue(parOK())), 1u);

  Double k2=chiSq()/Double(nDOF);

  parErr()=0.0;
  for (Int i=0;i<nPar();++i) 
    if (hess()(i)>0.0) {
      parErr()(i)=1.0/sqrt(hess()(i)/k2/2.0);   // 2 is from def of Hess!
    }


  if (prtlev()>4) {

    cout << "ChiSq  = " << chiSq() << endl;
    cout << "ChiSqV = " << chiSqV() << endl;
    cout << "sumWt  = " << sumWt() << endl;
    cout << "nWt    = " << nWt()
	 << "; nPar() = " << nPar() 
	 << "; nParOK = " << ntrue(parOK())
	 << "; nDOF = " << nDOF 
	 << endl;
    
    cout << "rChiSq = " << k2 << endl;
    cout << "max(dpar) = " << max(amplitude(dpar())) << endl;
    cout << "Amplitudes = " << amplitude(par()) << endl;
    cout << "Errors     = " << parErr() << endl;
    //    cout << "Errors = " << mean(parErr()(parOK())) << endl;
    
  }
}


} //# NAMESPACE CASA - END


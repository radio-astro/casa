//# VisCalSolver2.h: Default solver for calibration using visibilities
//# Copyright (C) 1996,1997,2000,2001,2002,2003
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

#ifndef SYNTHESIS_VISCALSOL2_H
#define SYNTHESIS_VISCALSOL2_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/SolveDataBuffer.h>
#include <synthesis/MeasurementEquations/VisEquation.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// VisCalSolver2: Default solver for calibration using visibility data
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// VisCal for visibility calibration (meaning solved from visibilities), Solver for
// solving. 
// </etymology>
//
// <synopsis> 
//
// VisCalSolver2 describes an interface for solving for calibration from visibility
// data in a VisBuffer.  It hosts the communication of visibility data with otherwise
// generic solving mechanims.  A Levenberg-Marquardt solver is supplied here by
// default, but it is intended that this class be the template for interfaces to
// third-party solving mechanisms.
//
// </synopsis> 
//
// <example>
// <srcblock>

// </srcblock>
// </example>
//
// <motivation>
// It is desirable to establish the distinct communicative boundary between generic
// solving mechanims and the particulars of visibility data and calibration
// component descriptions.  This class is intended to serve this purpose, by providing
// access to visibility data and calibration in terms of quantities necessary for
// least-squares (and other) style solvers.  
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

// **********************************************************
//  VisCalSolver2
//

class VisCalSolver2 {
public:

  // Constructor currently generic
  VisCalSolver2();
  
  // Destructor
  ~VisCalSolver2();

  // Do the solve
  Bool solve(VisEquation& viseq, SolvableVisCal& svc, SDBList& sdbs);

protected:

  // Access to fundamental external objects:
  inline SDBList&         sdbs() { return *SDBs_; };
  inline VisEquation&     ve()   { return *ve_; };
  inline SolvableVisCal&  svc()  { return *svc_; };

  // Access to maxIter_
  inline Int&    maxIter() { return maxIter_; };

  // Access to chi2
  inline Double& chiSq()     { return chiSq_; };
  inline Vector<Double>& chiSqV()     { return chiSqV_; };
  inline Double& lastChiSq() { return lastChiSq_; };
  inline Double& dChiSq()    { return dChiSq_; };
  inline Double& sumWt()     { return sumWt_; };
  inline Vector<Double>& sumWtV()     { return sumWtV_; };
  inline Int&    nWt()       { return nWt_; };

  // Access to parameters, & grad,hess,dp
  inline Int&              nPar() { return nPar_; };
  inline Vector<Complex>&  par()       { return par_; };
  inline Vector<Bool>&     parOK()     { return parOK_; };
  inline Vector<Float>&    parErr()    { return parErr_; };
  inline Vector<DComplex>& grad()      { return grad_; };
  inline Vector<Double>&   hess()      { return hess_; };
  inline Vector<Complex>&  dpar()      { return dpar_; };
  inline Vector<Complex>&  lastPar()   { return lastPar_; };

  inline Double&           lambda()    { return lambda_; };

  // Initialize solving data
  void initSolve();

  // Obtain trial residuals w.r.t svc's current pars
  void residualate2();

  // Differentiate w.r.t svc's pars
  void differentiate2();

  // Calculate residuals (incl. diff'd) and chi2 
  void chiSquare2();

  // Check for convergence
  Bool converged();

  // Internal solving methods
  void accGradHess2();
  void revert();
  void solveGradHess();
  void updatePar();

  // Optimize the step parabolically
  void optStepSize2();

  // Get and print par errors
  void getErrors();

  void printPar(const Int& iter);

private:
  
  // Diagnostic print level
  inline Int& prtlev() { return prtlev_; };

  // VisBuffer (from outside)
  SDBList* SDBs_;

  // VisEquation (from outside)
  VisEquation* ve_;

  // SVC (from outside)
  SolvableVisCal* svc_;

  // Total Number of parameters
  Int nPar_;

  // Maximum number of solve iterations to attempt
  Int maxIter_;

  // Chi2, sum wts
  Double chiSq_;
  Vector<Double> chiSqV_;
  Double lastChiSq_;
  Double dChiSq_;
  Double sumWt_;
  Vector<Double> sumWtV_;
  Int    nWt_;
  Int    cvrgcount_;

  // Parameter storage
  // (these are Complex to match the VisCal solvePar)
  Vector<Complex> par_;
  Vector<Bool>    parOK_;
  Vector<Float>   parErr_;
  Vector<Complex> lastPar_;

  // Parameter update
  Vector<Complex> dpar_;

  // Gradient, Hessian
  //  (these are Double for precision in accumulation
  Vector<DComplex> grad_;
  Vector<Double> hess_;

  // LM factor
  Double lambda_;

  // Step optimization toggle
  Bool optstep_;

  // Diagnostic print level
  Int prtlev_;

};

}
#endif

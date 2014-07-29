//# UVMod.h: Definitions of interface for UV-modelling in calibrater
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

#ifndef SYNTHESIS_UVMOD_H
#define SYNTHESIS_UVMOD_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <synthesis/MSVis/VisSet.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <components/ComponentModels/ComponentType.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/SkyComponent.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// VisCal: Model multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> module
//   <li> <linkto class="VisEquation">VisEquation</linkto> module
// </prerequisite>
//
// <etymology>
// VisCal describes an interface for multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. It is an Abstract Base Class: most methods
// must be defined in derived classes.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The following examples illustrate how an VisCal can be
// used:
// <ul>
// <li> Simulation: random multiplicative gains of a certain scaling
// can be applied: <linkto class="SimVisCal">SimVisCal</linkto>
// <li> Time variable multiplicative errors: <linkto class="TimeVarVisCal">TimeVarVisCal</linkto>
// <li> Solvable time variable multiplicatiove errors: <linkto class="SolvableVisCal">SolvableVisCal</linkto>
// </ul>
// </synopsis> 
//
// <example>
// <srcblock>
//      // Read the VisSet from disk
//      VisSet vs("3c84.MS");
//
//      VisEquation ve(vs);
//
//      // Make an VisCal
//      PhaseScreenVisCal coac(vs);
//
//      // Correct the visset
//      ve.correct();
// </srcblock>
// </example>
//
// <motivation>
// The properties of an multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

// **********************************************************
//  VisCal
//

class UVMod {
public:

  UVMod(VisSet& vs);
  
  virtual ~UVMod();

  inline Int& nPar() { return nPar_; };

  void setModel(const ComponentType::Shape type, 
		const Vector<Double> inpar,
		const Vector<Bool> invary);
  
  Bool modelfit(const Int& maxiter, const String file);

  inline Vector<Double>& par()  { return par_; };

protected:

  // Initial solving data
  void initSolve();

  // Calculate residuals (incl. diff'd) and chi2 
  virtual void residual();
  void chiSquare();

  // Internal solving methods
  void accGradHess();
  void solveGradHess(const Bool& doCovar=False);
  Bool updPar();
  Bool setCompPar();
  void printPar(const Int& iter);

  // Access to VisSet
  inline VisSet& vs() { return *vs_; };

  // Access to ComponentList
  inline ComponentList& cl() { return *cl_; };

  // Access to SkyComponents by id
  inline SkyComponent& skycomp(const uInt id) { return cl_->component(id); };

  // Access to current VisBuffer
  inline VisBuffer& svb() { return *svb_; };

  // Access to fieldId we are fitting for
  inline Int& fitfld() { return fitfld_; };
  inline MDirection& pc() { return pc_; };

  // Accessors to current svb's (differentiated) Residuals
  inline Cube<DComplex>&    R()  { return R_; };
  inline Array<DComplex>&   dR() { return dR_; };

  // Access to chi2
  inline Double& chiSq()     { return chiSq_; };
  inline Double& lastChiSq() { return lastChiSq_; };
  inline Double& sumWt()     { return sumWt_; };
  inline Int&    nWt()       { return nWt_; };
  inline Vector<Bool>& polWt()  { return polWt_; };

  // Access to parameters, & grad,hess,dp
  inline Double&         lamb() { return lamb_; };
  inline Vector<Double>& lastPar()  { return lastPar_; };
  inline Vector<Double>& grad() { return grad_; };
  inline Vector<Double>& lastGrad() { return lastGrad_; };
  inline Matrix<Double>& hess() { return hess_; };
  inline Matrix<Double>& lastHess() { return lastHess_; };
  inline Vector<Double>& dpar() { return dpar_; };
  inline Vector<Bool>&   vary() { return vary_; };
  inline Int&            nVary() { return nVary_; };


private:
  
  // Associated VisSet (LOCAL)
  VisSet* vs_;

  // ComponentList  (LOCAL)
  ComponentList* cl_;

  // VisBuffer (from outside)
  VisBuffer* svb_;

  // FieldId in fit
  Int fitfld_;

  // Phase center w.r.t. which modelfitting is done
  MDirection pc_;

  // Total Number of parameters
  Int nPar_;

  // Residual/Differentiation caches
  Cube<DComplex>  R_;
  Array<DComplex> dR_;

  // Chi2, sum wts
  Double chiSq_;
  Double lastChiSq_;
  Double sumWt_;
  Int    nWt_;

  Vector<Bool> polWt_;

  // Parameter storage
  Vector<Double> par_;
  Vector<Double> lastPar_;

  // Gradient, Hessian, par update
  Double         lamb_;
  Vector<Double> grad_;
  Vector<Double> lastGrad_;
  Matrix<Double> hess_;
  Matrix<Double> lastHess_;
  Vector<Double> dpar_;
  Vector<Bool>   vary_;
  Int            nVary_;

};

}
#endif

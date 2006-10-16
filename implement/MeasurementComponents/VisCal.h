//# VisCal.h: Definitions of interface for VisCal 
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
//# $Id$

#ifndef SYNTHESIS_VISCAL_H
#define SYNTHESIS_VISCAL_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <calibration/CalTables/CalSet.h>
#include <calibration/CalTables/CalInterp.h>
#include <msvis/MSVis/VisSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Need forward declaration for Solve in the Jones Matrices
class VisEquation;

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

typedef SquareMatrix<Complex,2> jMat;
typedef SquareMatrix<Complex,4> mMat;
typedef SquareMatrix<Float,2> jMatF;

// **********************************************************
//  VisCal
//

class VisCal {
public:

  // Allowed types of VisCal matrices
  enum Type{UVMOD,Mf,M,K,B,G,D,C,E,P,T,EP,F};

  VisCal();

  VisCal(VisSet& vs);
  
  virtual ~VisCal();

  // Return the type of this calibration matrix (actual type of derived class).
  virtual Type type() = 0;

  // Return type name as string
  inline virtual String typeName() { return "Unknown VisCal"; };

  // Frequency-dependent Parameters?  Nominally not.
  inline virtual Bool freqDepPar() { return False; };

  // Frequency-dependent Matrices?  Nominally same as freqDepPar.
  inline virtual Bool freqDepMat() { return freqDepPar(); };

  // Time-dependent Matrices (for a given set of parameters)
  inline virtual Bool& timeDepMat() { return timeDepMat_; };

  // Set the application parameters
  virtual void setApply(const Record& apply);

  // Is this calibration matrix to be applied?
  inline void setApplied(const Bool& flag) {applied_=flag;};
  inline Bool isApplied() {return applied_;};

  // Is this solveable? (never via this interface)
  inline virtual Bool isSolvable() {return False;};

  // Return the time interval over which this calibration is constant
  inline virtual Double& interval() {return interval_;}

  // Apply Mueller matrix to data (implemented in VisMueller)
  virtual VisBuffer& apply(VisBuffer& vb) = 0;
  virtual VisBuffer& applyInv(VisBuffer& vb) = 0;


protected:

  // Info
  inline Int& nSpw() { return nSpw_; };
  inline Int& nPar() { return nPar_; };
  inline Int& nAnt() { return nAnt_; };
  inline Int& nBln() { return nBln_; };
  virtual Int& nElem() { return nBln_; };

  // Current indices
  inline Int& currSpw() { return currSpw_; };

  inline Vector<Int>& nChanDat()     { return nChanDat_; };
  inline Vector<Int>& nChanPar()     { return nChanPar_; };
  inline Vector<Int>& nChanMat()     { return nChanMat_; };
  inline Vector<Int>& startChan() { return startChan_;}

  // Access to VisSet
  inline VisSet& vs() { return *vs_; };

private:
  
  // Associated VisSet
  VisSet* vs_;

  // Number of Spectral windows
  Int nSpw_;

  // Number of parameters describing this calibration component
  Int nPar_;

  // Number of antennas
  Int nAnt_;

  // Number of baselines
  Int nBln_;

  // Current synchronized spw
  Int currSpw_;

  // Channel counts
  Vector<Int> nChanDat_, nChanPar_, nChanMat_;
  Vector<Int> startChan_;

  // Time-dep (within VB) Matrices
  Bool timeDepMat_;

  // Solution timescale (context-dependent)
  Double interval_;

  // Application flag
  Bool applied_;

};


// **********************************************************
//  SolvableVisCal
//

class SolvableVisCal : virtual public VisCal {
public:

  SolvableVisCal(VisSet& vs);

  // Frequency-dependent Data required for solve?  
  //   Nominally same as freqDepMat.
  inline virtual Bool freqDepDat() { return freqDepMat(); };

  // Set the application parameters (apply from table)
  virtual void setApply(const Record& apply);

  // Set the solver parameters
  virtual void setSolve(const Record& solve);

  // Solve (pure virtual here)
  virtual Bool solve (VisEquation& ve) = 0;

  // Is this solveable? (always, via this interface)
  inline Bool isSolvable() {return True;};

  // Is this to be solved for?
  inline void setSolved(const Bool& flag) {solved_=flag;};
  inline Bool isSolved() {return solved_;};

  // Mode
  inline void setMode(const String m) {mode_=m;};
  inline const String mode() {return mode_;};

  inline Double&      preavg()         { return preavg_; };

protected:

  // Access to private data 
  inline String&      calTableName()   { return calTableName_; };
  inline String&      calTableSelect() { return calTableSelect_; };
  inline String&      tInterpType()    { return tInterpType_; };
  inline String&      fInterpType()    { return fInterpType_; };
  inline Vector<Int>& spwMap()         { return spwMap_; };
  inline Int&         refant()         { return refant_; };


private:

  // Cal table name
  String calTableName_;
  String calTableSelect_;

  // Interpolation types
  String tInterpType_;
  String fInterpType_;

  // Spw mapping
  Vector<Int> spwMap_;

  // Refant
  Int refant_;

  // Preavg interval
  Double preavg_;

  // Solved-for flag
  Bool solved_;

  // Solving mode
  String mode_;

};


// **********************************************************
//  VisMueller
//

class VisMueller : virtual public VisCal {

public:

  // Constructor
  VisMueller(VisSet& vs);

  virtual ~VisMueller();

  // Apply Mueller matrix
  virtual VisBuffer& apply(VisBuffer& vb);

  // Apply inverse of Mueller matrix
  virtual VisBuffer& applyInv(VisBuffer& vb);

  // Apply to VB
  virtual VisBuffer& applyCal(VisBuffer& vb, const Bool& doInv);

  // Synchronize "gains" with VisBuffer
  virtual void syncGain(const VisBuffer& vb,
                        const Bool& doInv=False);

  // Synchronize "gains" with data 
  virtual void syncGain(const Int& spw,
                        const Double& time,
                        const Int& field=-1,
                        const Bool& doInv=False);

  // Sync parameters for this spw by time/field, return reference to them
  virtual void syncPar(const Int& spw,
		       const Double& time,
		       const Int& field=-1);

  // Sync Mueller matrices for this spw by time/field, return reference to them
  virtual void syncMueller(const Int& spw,
			   const Double& time,
			   const Int& field=-1,
			   const Bool& doInv=False);

  // Access to solution parameters and matrices
  inline virtual Cube<Complex>& thisPar(const Int& spw)   {return (*thisPar_[spw]);};
  inline virtual Matrix<Bool>&  thisParOK(const Int& spw) {return (*thisParOK_[spw]);};
  inline Matrix<mMat>& thisMueller(const Int& spw)   {return (*thisMueller_[spw]);};
  inline Matrix<Bool>& thisMuellerOK(const Int& spw) {return (*thisMuellerOK_[spw]);};

  // Access to current Mueller by baseline index
  inline mMat& thisMueller(const Int& spw,
			   const Int& chn,
			   const Int& bln) { return thisMueller(spw)(chn,bln); };

  // Access to current Mueller by antenna indices
  inline mMat& thisMueller(const Int& spw,
			   const Int& chn,
			   const Int& a1,
			   const Int& a2) { return thisMueller(spw)(chn,blnidx(a1,a2)); };


  // Access to current MuellerOK by baseline index
  inline Bool& thisMuellerOK(const Int& spw,
			     const Int& chn,
			     const Int& bln) { return thisMuellerOK(spw)(chn,bln); };

  // Access to current MuellerOK by antenna indices
  inline Bool& thisMuellerOK(const Int& spw,
			     const Int& chn,
			     const Int& a1,
			     const Int& a2) { return thisMuellerOK(spw)(chn,blnidx(a1,a2)); };


protected:

  // Access to CalSet and CalInterp
  inline virtual CalSet<Complex>& cs() { return *cs_; };
  inline virtual CalInterp& ci() { return *cint_; };

  // Number of elements is nominally number of baselines for native Muellers
  inline virtual Int& nElem() { return nBln(); };

  // Current coords
  inline Double& lastTime(const Int& spw)  { return lastTime_(spw); };
  inline Double& lastTime()                { return lastTime_(currSpw()); };
  inline Double& currTime(const Int& spw)  { return currTime_(spw); };
  inline Double& currTime()                { return currTime(currSpw()); };
  inline Int&    currField(const Int& spw) { return currField_(spw); };
  inline Int&    currField()               { return currField(currSpw()); };
  inline Vector<Double>& currFreq()        { return currFreq_; };

  inline Double& refTime()                 { return refTime_; };
  inline Double& refFreq()                 { return refFreq_; };

  // Baseline index from antenna indices: (assumes a1<=a2 !!)
  inline Int blnidx(const Int& a1, 
		    const Int& a2) { return  a1*nAnt() - a1*(a1+1)/2 + a2; };

  // Validation of Mueller matrices
  inline void invalidateMM(const Int& spw) {MMValid_(spw)=False;};
  inline void validateMM(const Int& spw)   {MMValid_(spw)=True;};
  inline Bool MMValid(const Int& spw)      {return MMValid_(spw);};

  // Calculate a single Mueller parameters
  virtual void calcPar(const Int& spw, const Double& time, const Int& field);

  // Calculate a single Mueller matrix by some means
  virtual void calcMueller(mMat& mat, Vector<Complex>& par, 
			   const Int& baseline=-1,
			   const Int& channel=-1 );

  // Calculate a single Mueller matrix from Jones matrix
  virtual void calcMueller(mMat& mmat, jMat& jmat1, jMat& jmat2) {};

  // Invert Mueller matrices
  virtual void invMueller(const Int& spw);

  // Return true if polarization re-sequencing required
  virtual Bool polznSwitch(const VisBuffer& vb);

  // Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
  virtual void polznMap(CStokesVector& vis);

  // Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
  virtual void polznUnmap(CStokesVector& vis);

private:

  // Delete the this* caches
  void deleteMuellerCache();

  // Solution/Interpolation 
  CalSet<Complex> *cs_;
  CalInterp *cint_;

  // Current indices
  Vector<Double> currTime_;
  Vector<Int> currField_;
  Vector<Double> currFreq_;
  Vector<Double> lastTime_;
  Double refTime_;
  Double refFreq_;

  // Mueller validity
  Vector<Bool> MMValid_;

  // Current parameters
  PtrBlock<Cube<Complex>*>    thisPar_;        // [nSpw](nPar,nChanPar,nElm)
  PtrBlock<Matrix<Bool>*>     thisParOK_;      // [nSpw](nChanPar,nElm)
  PtrBlock<Matrix<mMat>*>     thisMueller_;    // [nSpw](nChanMat,nBln)
  PtrBlock<Matrix<Bool>*>     thisMuellerOK_;  // [nSpw](nChanMat,nBln)


};


// **********************************************************
//  SolvableVisMueller
//

class SolvableVisMueller : public SolvableVisCal, virtual public VisMueller
{

public:

  SolvableVisMueller(VisSet& vs);

  virtual ~SolvableVisMueller();

  // Set the application parameters (apply from table)
  virtual void setApply(const Record& apply);

  // Set the solver parameters and create empty CalSet
  virtual void setSolve(const Record& solve);

  // Solve (loops over solution intervals, calls type-specific per-vb solve mechanism)
  virtual Bool solve (VisEquation& ve);

  // Type-specific per-vb solve mechanism (pure virtual here)
  //  (shouldn't need slot, eventually)
  virtual Bool solvevb(const Int& slot)=0;

protected:

  inline virtual CalSet<Complex>& cs() { return *cs_; };
  inline virtual CalInterp& ci() { return *cint_; };

  inline virtual VisBuffer& svb() { return *svb_; };

  // Accessors to (differentiated) Residuals
  inline Matrix<CStokesVector>& R() { return R_; };
  inline Cube<CStokesVector>&  dR() { return dR_; };

  // Access to chi2
  inline Vector<Double>& chiSq() { return chiSq_; };
  inline Vector<Double>& lastChiSq() { return lastChiSq_; };
  inline Vector<Double>& sumWt() { return sumWt_; };

  // Access to grad,hess,dg
  inline virtual Int& nHess() { return nHess_; }; 
  inline Matrix<Complex>& grad() { return grad_; };
  //  inline Matrix<Complex>& hess() { return hess_; };
  inline Cube<Complex>& hess() { return hess_; };
  inline Matrix<Complex>& dg()   { return dg_; };

  // Calculate residuals
  virtual void residual();

  // Solving methods
  void calcGradHess();
  void solveGradHess();
  virtual void updgain();
  Bool converged(const Int& iter);

  Bool& almostconverged() { return almostconverged_; };
  Float& tolerance() { return tolerance_; };

  // Access to differentiated Muellers
  Cube<mMat>& diffMueller() { return diffMueller_; };

  // Synchronize the Muellers AND diffMuellers
  virtual void syncDiffMueller(const Int& row);

  // Calculate a diffMueller
  virtual void calcDiffMueller(Vector<mMat>& mat, Vector<Complex>& par, 
			       const Int& baseline=-1, const Int& channel=-1 ) {};

  // Initialize chi2, grad, hess
  void initSolve();

  // Calculate chi2
  void chiSquare();


  // Validation of diffMueller matrices
  inline void invalidateDM() {DMValid_=False;};
  inline void validateDM()   {DMValid_=True;};
  inline Bool DMValid()      {return DMValid_;};

  // Cal VisBuffer (pointers to VB internals)
  //  Bool*      flr_p;
  //  Bool*      fl_p;
  //  Int*       a1_p;
  //  Int*       a2_p;
  //  Double*    time_p;
  //  Float*     wt_p;
  //  Double*    freq_p;
  //  RigidVector<Double,3>* uvw_p;
  //  CStokesVector*         vis_p;
  //  CStokesVector*         mod_p;


private:

  // Solution/Interpolation 
  CalSet<Complex> *cs_;
  CalInterp *cint_;

  // The current collapsed Visbuffer in solve
  VisBuffer *svb_;

  // Residual/Differentiation caches
  Matrix<CStokesVector> R_;
  Cube<CStokesVector>  dR_;

  // diffMueller validation
  Bool DMValid_;

  // Differentiated Mueller matrices
  Cube<mMat> diffMueller_;    // (nPar,nChanMat,nBln)

  // Chi2, sum wts
  Vector<Double> chiSq_;
  Vector<Double> lastChiSq_;
  Vector<Double> sumWt_;

  // Gradient, Hessian, gain update
  Matrix<Complex> grad_;
  //  Matrix<Complex> hess_;
  Int nHess_;
  Cube<Complex> hess_;
  Matrix<Complex> dg_;

  Bool almostconverged_;
  Float tolerance_;

};


// **********************************************************
//  VisJones2
//

class VisJones2 : virtual public VisMueller {

public:

  // Constructor
  VisJones2(VisSet& vs);

  virtual ~VisJones2();

  // Synchronize current Mueller matrices via Jones matrices
  virtual void syncMueller(const Int& spw,
			   const Double& time,
			   const Int& field=-1,
			   const Bool& doInv=False);


  // Synchronize current Jones matrices
  virtual void syncJones(const Int& spw,
			 const Double& time,
			 const Int& field=-1,
			 const Bool& doInv=False);

  // Number of elements is number of antennas
  inline virtual Int& nElem() { return nAnt(); };

  // Access to solution parameters and matrices
  inline virtual Cube<Complex>& thisPar(const Int& spw)     {return (*thisPar_[spw]);};
  inline virtual Matrix<Bool>&  thisParOK(const Int& spw)   {return (*thisParOK_[spw]);};
  inline Matrix<jMat>& thisJones(const Int& spw)   {return (*thisJones_[spw]);};
  inline Matrix<jMat>& thisJonesConj(const Int& spw)   {return (*thisJonesConj_[spw]);};
  inline Matrix<Bool>& thisJonesOK(const Int& spw) {return (*thisJonesOK_[spw]);};

  // Access to current Jones by antenna index
  inline jMat& thisJones(const Int& spw,
			 const Int& chn,
			 const Int& ant) { return thisJones(spw)(chn,ant); };

  // Access to current Jones by antenna index
  inline jMat& thisJonesConj(const Int& spw,
			     const Int& chn,
			     const Int& ant) { return thisJonesConj(spw)(chn,ant); };

  // Access to current JonesOK by antenna index
  inline Bool& thisJonesOK(const Int& spw,
			   const Int& chn,
			   const Int& ant) { return thisJonesOK(spw)(chn,ant); };

protected:

  // Validation of Jones matrices
  inline void invalidateJM(const Int& spw) {JMValid_(spw)=False;};
  inline void validateJM(const Int& spw)   {JMValid_(spw)=True;};
  inline Bool JMValid(const Int& spw)      {return JMValid_(spw);};

  // Calculate a single Jones parameters
  virtual void calcPar(const Int& spw, const Double& time, const Int& field);

  // Calculate a single Jones matrix by some means from parameters
  virtual void calcJones(jMat& mat, Vector<Complex>& par );

  // Invert Jones matrices
  virtual void invJones(const Int& spw);

  // Form conjugated Jones matrices
  virtual void conjJones(const Int& spw);

  // Calculate a single Mueller matrix by some means
  virtual void calcMueller(mMat& mat, Vector<Complex>& par, 
			   const Int& baseline=-1,
			   const Int& channel=-1 ) {};

  // Calculate a single Mueller matrix from Jones matrix
  virtual void calcMueller(mMat& mmat, jMat& jmat1, jMat& jmat2);

private:

  // Delete the this* caches
  void deleteJonesCache();

  // Mueller validity
  Vector<Bool> JMValid_;

  // Current parameters
  PtrBlock<Cube<Complex>*>    thisPar_;        // [nSpw](nPar,nChanPar,nAnt)
  PtrBlock<Matrix<Bool>*>     thisParOK_;      // [nSpw](nChanPar,nAnt)
  PtrBlock<Matrix<jMat>*>     thisJones_;      // [nSpw](nChanMat,nAnt)
  PtrBlock<Matrix<jMat>*>     thisJonesConj_;  // [nSpw](nChanMat,nAnt)
  PtrBlock<Matrix<Bool>*>     thisJonesOK_;    // [nSpw](nChanMat,nAnt)


};

// **********************************************************
//  SolvableVisJones2
//

class SolvableVisJones2 : public SolvableVisMueller, public VisJones2 {

public:

  SolvableVisJones2(VisSet& vs);

  virtual ~SolvableVisJones2() {};

  // Solve (nominal antenna-based implementation here)
  virtual Bool solve (VisEquation& ve)=0;

  // Synchronize derivatives w.r.t. parameters 
  //  (here, no-op in generic version)
  virtual void differentiate(VisBuffer& vb);

  // Return reference to per-ant, per-chan derivatives w.r.t. parameters
  //  (here, in generic version, just return array/chan-constant matrix)
  virtual jMat& dJ(const Int& par, const Int& chan, const Int& ant) {
    return dJ_;
  }

  // Calculate the residual visibilities w.r.t. the current calibration params
  //  virtual void residual(VisBuffer& vb, 
  //			Matrix<CStokesVector>& R) ;

  // Form derivatives of the residuals w.r.t. the parameters
  //  virtual void applyDerivative(VisBuffer& vb, 
  //			       PtrBlock<Array<CStokesVector>*>& dR) ;


  virtual void addGain(const Cube<Complex> gaininc);


protected:

  // Validation of Jones matrix derivatives
  inline void invalidateDJ(const Int& spw) {DJValid_(spw)=False;};
  inline void validateDJ(const Int& spw)   {DJValid_(spw)=True;};
  inline Bool DJValid(const Int& spw)      {return DJValid_(spw);};



private:

  // Jones matrix derivatives
  jMat dJ_;

  // Validity of Jones matrix derivatives
  Vector<Bool> DJValid_;
  

};




// **********************************************************
//  MMueller
//

class MMueller : public SolvableVisMueller {

public:

  MMueller(VisSet& vs);

  // Set the solver parameters (enforce full preavg)
  virtual void setSolve(const Record& solve);
 
 inline virtual Type type() {return VisCal::M;};

  // Return type name as string
  inline virtual String typeName() { return "M Mueller"; };

  // MMueller-specific per-vb solve mechanism (copy data to soln)
  virtual Bool solvevb(const Int& slot);


};

// **********************************************************
//  MfMueller
//

class MfMueller : public MMueller {
    
public:
  
  MfMueller(VisSet& vs);

  virtual Bool freqDepPar() { return True; };

  virtual Type type() {return VisCal::Mf;};
	      
  // Return type name as string
  inline virtual String typeName() { return "MF Mueller"; };

};    


// **********************************************************
//  KMueller
//

class KMueller : public SolvableVisMueller {

public:

  KMueller(VisSet& vs);

  // Set the solver parameters (enforce no preavg)
  virtual void setSolve(const Record& solve);

  inline virtual Type type() {return VisCal::K;};

  // Return type name as string
  inline virtual String typeName() { return "K Mueller"; };

  // Frequency-dependent Matrices (parameters are NOT)
  inline virtual Bool freqDepMat() { return True; };

  // KMueller-specific per-vb solve mechanism
  virtual Bool solvevb(const Int& slot);

protected:

  virtual void calcMueller(mMat& mat, Vector<Complex>& par, 
			   const Int& baseline=-1,
			   const Int& channel=-1 );

  virtual void calcDiffMueller(Vector<mMat>& mat, Vector<Complex>& par, 
			       const Int& baseline=-1, const Int& channel=-1 );

  virtual void updgain();

};



// **********************************************************
//  UVpmod
//

class UVpMod : public SolvableVisMueller {

public:

  UVpMod(VisSet& vs);

  // Set the solver parameters (enforce no preavg)
  virtual void setSolve(const Record& solve);

  inline virtual Type type() {return VisCal::UVMOD;};

  // Return type name as string
  inline virtual String typeName() { return "UV point Model"; };

  // MMueller-specific per-vb solve mechanism (copy data to soln)
  virtual Bool solvevb(const Int& slot);

protected:

  inline virtual Int& nElem() { return nElem_; };

  Matrix<Double>& thisUV() { return thisUV_; };

  // Synchronize the Muellers AND diffMuellers
  virtual void syncDiffMueller(const Int& row);

  // Synchronize the UVW
  virtual void syncUVW(const Int& row);

  virtual void calcMueller(mMat& mat, Vector<Complex>& par, 
			   const Int& baseline=-1,
			   const Int& channel=-1 );

  virtual void calcDiffMueller(Vector<mMat>& mat, Vector<Complex>& par, 
			       const Int& baseline=-1, const Int& channel=-1 );


private:

  Int nElem_;
  Matrix<Double> thisUV_;

};

// **********************************************************
//  TJones (new)
//

class newTJones : public SolvableVisJones2 {

public:

  newTJones(VisSet& vs);

  inline virtual Type type() {return VisCal::T;};

  // Return type name as string
  inline virtual String typeName() { return "T Jones (new)"; };

};



} //# NAMESPACE CASA - END

#endif

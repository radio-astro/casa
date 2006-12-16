//# SolvableVisCal.h: Definitions of interface for SolvableVisCal 
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

#ifndef SYNTHESIS_SOLVABLEVISCAL_H
#define SYNTHESIS_SOLVABLEVISCAL_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/Mueller.h>
#include <synthesis/MeasurementComponents/Jones.h>
#include <synthesis/MeasurementComponents/VisVector.h>
#include <calibration/CalTables/CalSet.h>
#include <calibration/CalTables/CalInterp.h>
#include <msvis/MSVis/VisSet.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// **********************************************************
//  SolvableVisCal
//

// Forward
class VisEquation;

class SolvableVisCal : virtual public VisCal {
public:

  SolvableVisCal(VisSet& vs);
  
  SolvableVisCal(const Int& nAnt);

  virtual ~SolvableVisCal();

  // Access to user-supplied parameters
  inline String&      calTableName()   { return calTableName_; };
  inline String&      calTableSelect() { return calTableSelect_; };
  inline Bool&        append()         { return append_; };
  inline String&      tInterpType()    { return tInterpType_; };
  inline String&      fInterpType()    { return fInterpType_; };
  inline Vector<Int>& spwMap()         { return spwMap_; };
  inline Int&         refant()         { return refant_; };
  inline String&      mode()           { return mode_; };
  inline Double&      preavg()         { return preavg_; };

  // Total number of (complex) parameters per solve
  //  (specialize to jive with ant- or bln-basedness, etc.)
  virtual Int nTotalPar()=0;

  // Use standard VisCal solving mechanism?
  virtual Bool standardSolve() { return True; };

  // Does normalization by MODEL_DATA commute with this VisCal?
  //   (if so, permits pre-solve time-averaging)
  virtual Bool normalizable()=0;

  // Is this type capable of accumulation?  (nominally no)
  virtual Bool accumulatable() { return False; };

  // Is this type capable of smoothing?  (nominally no)
  virtual Bool smoothable() { return False; };

  // Access to focus channel
  inline Int&         focusChan()      { return focusChan_; };

  // Is this ready to solve?
  inline Bool isSolved() {return solved_;};

  // Is this solveable? (via this interface, nominally yes)
  virtual Bool isSolvable() {return True;};

  // Set the application parameters 
  virtual void setApply();
  virtual void setApply(const Record& apply);

  // Report apply info/params, e.g. for logging
  virtual String applyinfo();

  // Set the solving parameters
  virtual void setSolve();
  virtual void setSolve(const Record& solve);

  // Report solve info/params, e.g., for logging
  virtual String solveinfo();

  // Arrange for accumulation
  virtual void setAccumulate(VisSet& vs,
			     const String& table,
			     const String& select,
			     const Double& t,
			     const Int& refAnt=-1);

  // Initialize internal shapes for solving
  void initSolve(VisSet& vs);

  // Inflate the pristine CalSet (from VisSet info)
  void inflate(VisSet& vs, const Bool& fillMeta=False);

  // Inflate the pristine CalSet (generically)
  void inflate(const Vector<Int>& nChanDat,
	       const Vector<Int>& startChanDat,
	       const Vector<Int>& nSlot);

  // Hazard a guess at the parameters (solvePar) given the data
  virtual void guessPar(VisBuffer& vb)=0;

  // Access to current solution parameters and matrices
  inline virtual Cube<Complex>& solvePar()   {return (*solvePar_[currSpw()]);};
  inline virtual Matrix<Bool>&  solveParOK() {return (*solveParOK_[currSpw()]);};

  // Synchronize the meta data with a solvable VisBuffer
  //   (returns False if VisBuffer has no valid data)
  Bool syncSolveMeta(VisBuffer& vb, const Int& fieldId);

  // Self-solving mechanism
  virtual void selfSolve(VisSet& vs, VisEquation& ve);

  // Differentiate VB model w.r.t. Cal  parameters (no 2nd derivative yet)
  virtual void differentiate(VisBuffer& vb,        
			     Cube<Complex>& V,     
			     Array<Complex>& dV,
			     Matrix<Bool>& Vflg)=0;

  // Apply refant (implemented in SVJ)
  virtual void reReference()=0;

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* incr,
			  const Vector<Int>& fields)=0;

  virtual void smooth(Vector<Int>& fields,
		      const String& smtype,
		      const Double& smtime);

  // File the current solved solution into a slot in the CalSet
  void keep(const Int& slot);

  virtual void fluxscale(const Vector<Int>& refFieldIn,
			 const Vector<Int>& tranFieldIn,
			 const Vector<Int>& inRefSpwMap,
			 const Vector<String>& fldNames,
			 Matrix<Double>& fluxScaleFactor)=0;

  // Tell the CalSet to write a CalTable
  void store();
  void store(const String& tableName,const Bool& append);

  // Report state:
  inline virtual void state() { stateSVC(True); };

protected:

  // Set to-be-solved-for flag
  inline void setSolved(const Bool& flag) {solved_=flag;};

  // Access to CalSet/CalInterp
  inline virtual CalSet<Complex>& cs() { return *cs_; };
  inline virtual CalInterp& ci() { return *cint_; };

  // Initialize solve parameters (shape)
  virtual void initSolvePar()=0;

  // Invalidate diff cal matrices generically 
  inline virtual void invalidateDiffCalMat()=0;

  // Explicitly synchronize pars with a CalSet slot
  using VisCal::syncPar;
  void syncPar(const Int& spw, const Int& slot);

  // Set matrix channelization according to a VisSet
  void setSolveChannelization(VisSet& vs);

  // Fill CalSet meta-data according to a VisSet
  void fillMetaData(VisSet& vs);

  // Synchronize calibration for solve context
  void syncSolveCal();

  // Synchronize parameters for solve context
  void syncSolvePar();

  // Calculate parameters by some means (e.g., interpolation from a CalSet)
  virtual void calcPar();

  // Synchronize the differentiated calibration 
  virtual void syncDiffMat()=0;

  // Report the SVC-specific state, w/ option for VC::state()
  virtual void stateSVC(const Bool& doVC);

  // Logger
  LogIO& logSink() { return logsink_p; };

private:

  // Default ctor is private
  SolvableVisCal();

  // Initialize pointers, etc.
  void initSVC();

  // Delete pointers
  void deleteSVC();

  // Check if a cal table is appropriate
  void verifyCalTable(const String& caltablename);

  // Solution/Interpolation 
  CalSet<Complex> *cs_;
  CalInterp *cint_;

  // Cal table name
  String calTableName_;
  String calTableSelect_;
  Bool append_;

  // Interpolation types
  String tInterpType_;
  String fInterpType_;

  // Spw mapping
  Vector<Int> spwMap_;

  // Refant
  Int refant_;

  // Preavg interval
  //  Double preavg_;

  // Solved-for flag
  Bool solved_;

  // Solving mode
  String mode_;

  // Preavering interval
  Double preavg_;

  // In-focus channel for single-chan solves on multi-chan data
  Int focusChan_;

  // Solving meta-data
  Double dataInterval_;
  Double fitWt_;
  Double fit_;


  // Current parameters
  PtrBlock<Cube<Complex>*>    solvePar_;        // [nSpw](nPar,1,{1|nElem})
  PtrBlock<Matrix<Bool>*>     solveParOK_;      // [nSpw](1,{1|nElm})

  // LogIO
  LogIO logsink_p;

};



// **********************************************************
//  SolvableVisMueller
//

class SolvableVisMueller : public SolvableVisCal, virtual public VisMueller
{

public:

  SolvableVisMueller(VisSet& vs);

  SolvableVisMueller(const Int& nAnt);

  virtual ~SolvableVisMueller();

  // Total number of (complex) parameters per solve
  //  Mueller version:  just return number of per-Bln parameters
  virtual Int nTotalPar() { return nPar(); };

  // Does normalization by MODEL_DATA commute with this VisCal?
  virtual Bool normalizable() { return (this->muellerType() < Mueller::General); };

  // Hazard a guess at the parameters (solvePar) given the data
  virtual void guessPar(VisBuffer& vb) { throw(AipsError("NYI")); };

  // Differentiate VB model w.r.t. Mueller parameters (no 2nd derivative yet)
  virtual void differentiate(VisBuffer& vb,          // input data
			     Cube<Complex>& V,       // trial apply (nCorr,nChan,nRow)
			     Array<Complex>& dV,     // 1st deriv   (nCorr,nPar,nChan,nRow)
			     Matrix<Bool>& Vflg) { throw(AipsError("NYI")); };

  // Apply refant (no-op for Muellers)
  virtual void reReference() {};

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* incr,
			  const Vector<Int>& fields) { throw(AipsError("NYI")); };

  // Scale solutions
  virtual void fluxscale(const Vector<Int>& refFieldIn,
			 const Vector<Int>& tranFieldIn,
			 const Vector<Int>& inRefSpwMap,
			 const Vector<String>& fldNames,
			 Matrix<Double>& fluxScaleFactor) { throw(AipsError("NYI")); };

  // Report state:
  inline virtual void state() { stateSVM(True); };

protected:

  // Number of Cal Matrices to form on baseline axis
  //  (Mueller, solve context: 1)
  virtual Int nCalMat() { return isSolved() ? 1 : nBln(); };

  // Are differentiated M matrices constant in chan & bln?
  virtual Bool trivialDM() { return False; };

  // Initialize solve parameter shape
  //  Mueller version:  (nPar(),1,1)   (one chan, one baseline)
  virtual void initSolvePar();

  // Access to matrix rendering of dM (per par)
  inline Mueller& dM() { return *dM_; };

  // Access to differentiated Mueller elements
  inline Array<Complex>& diffMElem() {return diffMElem_;};

  // Invalidate diff cal matrices generically (at this level, just M, dM)
  inline virtual void invalidateDiffCalMat() { invalidateM(); invalidateDM(); };

  // Validation of diffMueller matrices
  inline void invalidateDM() {DMValid_=False;};
  inline void validateDM()   {DMValid_=True;};
  inline Bool DMValid()      {return DMValid_;};

  // Synchronize the differentiated calibration (specialization for Mueller);
  virtual void syncDiffMat();

  // Synchronize the Muellers AND diffMuellers
  virtual void syncDiffMueller();

  // Calculate the ensemble of diff'd Mueller Elements
  virtual void calcAllDiffMueller();

  // Calculate one diffMElem 
  virtual void calcOneDiffMueller(Matrix<Complex>& mat, const Vector<Complex>& par);

  // Create matrix renderers for dMs
  void createDiffMueller();

  // Initialize trivial diff'd Muellers
  virtual void initTrivDM();

  // SVM-specific state
  virtual void stateSVM(const Bool& doVC);

private:

  // Default ctor is private
  SolvableVisMueller();

  // Mueller wrapper for diffMElem_;
  Mueller *dM_;

  // Differentiated Mueller matrix elements
  Array<Complex> diffMElem_;    // (nDMEl,nPar,nChanMat,nBln)

  // diffMueller validation
  Bool DMValid_;


};

// **********************************************************
//  SolvableVisJones
//

class SolvableVisJones : public SolvableVisMueller, public VisJones {

public:

  SolvableVisJones(VisSet& vs);

  SolvableVisJones(const Int& nAnt);

  virtual ~SolvableVisJones();

  // Total number of (complex) parameters per solve
  //  Jones version:  nPar()*nAnt()
  virtual Int nTotalPar() { return nPar()*nAnt(); };

  // Does normalization by MODEL_DATA commute with this VisCal?
  virtual Bool normalizable() { return (this->jonesType() < Jones::General); };

  // Differentiate VB model w.r.t. Jones parameters
  virtual void differentiate(VisBuffer& vb,          // input data
			     Cube<Complex>& V,       // trial apply (nCorr,nChan,nRow)
			     Array<Complex>& dV,     // 1st deriv   (nCorr,nPar,nChan,nRow,2)
			     Matrix<Bool>& Vflg);

  // Apply refant
  virtual void reReference();

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* incr,
			  const Vector<Int>& fields);

  // Fluxscale is implemented here
  void fluxscale(const Vector<Int>& refFieldIn,
		 const Vector<Int>& tranFieldIn,
		 const Vector<Int>& inRefSpwMap,
		 const Vector<String>& fldNames,
		 Matrix<Double>& fluxScaleFactor);
			     
  // Report state:
  inline virtual void state() { stateSVJ(True); };

protected:
 
  // Number of Cal Matrices to form on baseline axis
  //  (Jones, all contexts: nAnt())
  virtual Int nCalMat() { return nAnt(); };

  // DM never trivial for SVJ
  virtual Bool trivialDM() { return False; };

  // Are differentiated J matrices constant in chan & ant?
  inline virtual Bool trivialDJ() { return False; };

  // Initialize solve parameter shape
  //  Jones version:  (nPar(),1,nAnt())   (one chan, all antennas)
  virtual void initSolvePar();

  // Access to matrix rendering of dJ1, dJ2
  inline Jones& dJ1() { return *dJ1_; };
  inline Jones& dJ2() { return *dJ2_; };

  // Access to differentiated Joness
  Array<Complex>& diffJElem() { return diffJElem_; };

  // Invalidate diff cal matrices generically (at this level, M, dM, J, dJ)
  inline virtual void invalidateDiffCalMat() { 
    SolvableVisMueller::invalidateDiffCalMat(); invalidateJ(); invalidateDJ(); };

  // Validation of Jones matrix derivatives
  inline void invalidateDJ() {DJValid_=False;};
  inline void validateDJ()   {DJValid_=True;};
  inline Bool DJValid()      {return DJValid_;};

  // Synchronize the differentiated calibration (specialization for Jones)
  virtual void syncDiffMat();

  // Synchronize the Jones AND diffJones matrices
  virtual void syncDiffJones();

  // Calculate the ensemble of diff'd Jones Elements
  virtual void calcAllDiffJones();

  // Calculate one ant/chan's diffJElem w.r.t. each par
  virtual void calcOneDiffJones(Matrix<Complex>& mat, const Vector<Complex>& par);

  // Create matrix renderers for dJs
  void createDiffJones();

  // Initialize trivial diff'd Jones
  virtual void initTrivDJ();

  virtual void stateSVJ(const Bool& doVC);

private:

  // Default ctor is private
  SolvableVisJones();

  // Jones wrappers for diffJElem_;
  Jones *dJ1_;
  Jones *dJ2_;

  // Differentiated Jones matrix elements
  Array<Complex> diffJElem_;    // (nJME,nPar,nChanMat,nAnt,2)

  // Validity of Jones matrix derivatives
  Bool DJValid_;
  
};

// Global methods

// Discern cal table type from the table itself
String calTableType(const String& tablename);





} //# NAMESPACE CASA - END

#endif

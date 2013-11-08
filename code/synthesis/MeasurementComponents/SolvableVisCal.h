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
#include <synthesis/MeasurementComponents/CalCorruptor.h>
#include <synthesis/MeasurementComponents/Mueller.h>
#include <synthesis/MeasurementComponents/Jones.h>
#include <synthesis/MeasurementComponents/VisVector.h>
#include <synthesis/TransformMachines/SynthesisError.h>
#include <synthesis/CalTables/NewCalTable.h>
#include <synthesis/CalTables/CLPatchPanel.h>
#include <synthesis/CalTables/CTPatchedInterp.h>
#include <synthesis/CalTables/CalSetMetaInfo.h>
#include <synthesis/CalTables/VisCalEnum.h>
#include <synthesis/MSVis/VisSet.h>
#include <synthesis/MSVis/CalVisBuffer.h>
#include <synthesis/MSVis/VisBuffGroupAcc.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/Timer.h>

#include <iostream>
#include <fstream>

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  SolvableVisCal
//

// Forward
class VisEquation;

class SolvableVisCal : virtual public VisCal {
public:

  typedef struct fluxScaleStruct {
    Matrix<Double> fd;
    Matrix<Double> fderr;
    Matrix<Int> numSol;
    Vector<Double> freq;
    Matrix<Double> spidx;
    Matrix<Double> spidxerr;
    Double fitfd;
    Double fitfderr;
    Double fitreffreq;
  } fluxScaleStruct;


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
  inline Int&         refant()         { return refantlist()(0); };
  inline Vector<Int>& refantlist()     { return urefantlist_; };
  inline Int&         minblperant()    { return minblperant_; };
  inline String&      apmode()         { return apmode_; };
  inline String&      solint()         { return solint_; };
  inline String&      fsolint()        { return fsolint_; };
  inline Double&      preavg()         { return preavg_; };
  inline Bool&        solnorm()        { return solnorm_;};
  inline Float&       minSNR()         { return minSNR_; };

  inline String&      combine()        { return combine_; };
  inline Bool         combspw()        { return upcase(combine_).contains("SPW"); };
  inline Bool         combfld()        { return upcase(combine_).contains("FIELD"); };
  inline Bool         combscan()       { return upcase(combine_).contains("SCAN"); };
  inline Bool         combobs()        { return upcase(combine_).contains("OBS"); };

  // Total number of (complex) parameters per solve
  //  (specialize to jive with ant- or bln-basedness, etc.)
  virtual Int nTotalPar()=0;

  // Report if calibration available for specified spw
  //  (if no CalInterp available, assume True)

  // Use generic data gathering mechanism for solve
  virtual Bool useGenericGatherForSolve() { return True; };

  // Use generic solution engine for a single solve
  //  (usually inside the generic gathering mechanism)
  virtual Bool useGenericSolveOne() { return useGenericGatherForSolve(); };

  // Solve for point-source X or Q,U?
  //  nominally no (0)
  virtual Int solvePol() { return 0; };

  // Does normalization by MODEL_DATA commute with this VisCal?
  //   (if so, permits pre-solve time-averaging)
  virtual Bool normalizable()=0;

  // Is this type capable of accumulation?  (nominally no)
  virtual Bool accumulatable() { return False; };

  // Is this type capable of smoothing?  (nominally no)
  virtual Bool smoothable() { return False; };

  // Should only parallel-hands be used in solving?
  //  (generally no (default=False), but GJones and related
  //   will override, and eventually this will be a user-set-able
  //   parameter)
  virtual Bool phandonly() { return False; }

  // Access to focus channel
  inline Int&         focusChan()      { return focusChan_; };

  // Is this ready to solve?
  inline Bool isSolved() {return solved_;};

  // Is this solveable? (via this interface, nominally yes)
  virtual Bool isSolvable() {return True;};

  // Set the application parameters 
  virtual void setApply();
  virtual void setApply(const Record& apply);
  virtual void setCallib(const Record& callib,const MeasurementSet& selms);

  virtual void setModel(const String& )
  {throw(SynthesisError("Internal error: setModel() not yet supported for non EPJones type."));};

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

  // Default value for parameters
  virtual Complex defaultPar() { return Complex(1.0); };
  virtual Float defaultRPar() { return Float(0.0); };
  virtual Complex defaultCPar() { return Complex(1.0); };

  // Arrange to build a cal table from specified values
  virtual void setSpecify(const Record& specify);

  // Fill a caltable with specified values
  virtual void specify(const Record& specify);

  // Size up the solving arrays, etc.  (supports combine)
  virtual Int sizeUpSolve(VisSet& vs, Vector<Int>& nChunkPerSol);


  // These inflate methods soon to deprecate (gmoellen, 20121212)
  // Inflate the pristine CalSet (from VisSet info)
  void inflate(VisSet& vs, const Bool& fillMeta=False);
  // Inflate the pristine CalSet (generically)
  virtual void inflate(const Vector<Int>& nChanDat,
		       const Vector<Int>& startChanDat,
		       const Vector<Int>& nSlot);


  // Hazard a guess at the parameters (solveCPar) given the data
  virtual void guessPar(VisBuffer& vb)=0;

  // Freq-dep solint values 
  inline Double& fintervalHz() { return fintervalHz_; };
  inline Double& fintervalCh() { return fintervalCh_(currSpw()); };  // for current Spw
  Matrix<Int> chanAveBounds()  { return chanAveBounds_(currSpw()); }; // for current Spw
  Matrix<Int> chanAveBounds(Int spw)  { return chanAveBounds_(spw); }; 

  // Access to current solution parameters and matrices
  inline virtual Cube<Complex>& solveCPar()   {return (*solveCPar_[currSpw()]);};
  inline virtual Cube<Float>&   solveRPar()   {return (*solveRPar_[currSpw()]);};
  inline virtual Cube<Bool>&    solveParOK()  {return (*solveParOK_[currSpw()]);};
  inline virtual Cube<Float> &  solveParErr() {return (*solveParErr_[currSpw()]);};
  inline virtual Cube<Float> &  solveParSNR() {return (*solveParSNR_[currSpw()]);};
  inline virtual Cube<Complex>& solveAllCPar()   {return (*solveAllCPar_[currSpw()]);};
  inline virtual Cube<Float>&   solveAllRPar()   {return (*solveAllRPar_[currSpw()]);};
  inline virtual Cube<Bool>&    solveAllParOK()  {return (*solveAllParOK_[currSpw()]);};
  inline virtual Cube<Float> &  solveAllParErr() {return (*solveAllParErr_[currSpw()]);};
  inline virtual Cube<Float> &  solveAllParSNR() {return (*solveAllParSNR_[currSpw()]);};

  // Access to source pol parameters
  inline Vector<Complex>& srcPolPar() { return srcPolPar_; };

  // Synchronize the meta data with a solvable VisBuffer
  //   (returns False if VisBuffer has no valid data)
  Bool syncSolveMeta(VisBuffer& vb, const Int& fieldId);
  Bool syncSolveMeta(VisBuffGroupAcc& vbga);

  // Provide for override of currScan and currObs
  void overrideObsScan(Int obs, Int scan);

  // If apmode() is "A", convert vb's visibilities to amp + 0i.
  // If it is "P", convert them to phase + 0i.
  // Otherwise (i.e. "AP"), leave them alone.
  virtual void enforceAPonData(VisBuffer& vb);

  // Verify VisBuffer data sufficient for solving (wts, etc.)
  virtual Bool verifyConstraints(VisBuffGroupAcc& vbag);
  virtual Bool verifyForSolve(VisBuffer& vb);
  
  // Self- gather and/or solve prototypes
  //  (triggered by useGenericGatherForSolve=F or useGenericSolveOne=F; 
  //   must be overridden in derived specializations)
  virtual void selfGatherAndSolve(VisSet& vs, VisEquation& ve);
  virtual void selfSolveOne(VisBuffGroupAcc& vs);

  // Set up data and model for pol solve
  void setUpForPolSolve(VisBuffer& vb);

  // Differentiate VB model w.r.t. Cal  parameters (no 2nd derivative yet)
  virtual void differentiate(CalVisBuffer& cvb)=0;
  virtual void differentiate(VisBuffer& vb,        
			     Cube<Complex>& V,     
			     Array<Complex>& dV,
			     Matrix<Bool>& Vflg)=0;
  virtual void differentiate(VisBuffer& ,          // vb.visCube() has the obs. data.  vb.modelVisCube() will receive the residuals
                             VisBuffer&   ,       // 1st. Derivative w.r.t. first parameter
                             VisBuffer& ,         // 1st. Derivative w.r.t. second parameter
                             Matrix<Bool>& ){ throw(AipsError("Invalid use of differentiate(vb,dV0,dv1)")); };


  // Differentiate VB model w.r.t. Source parameters
  virtual void diffSrc(VisBuffer& vb,        
		       Array<Complex>& dV)=0;

  // Update solve parameters incrementally (additive)
  virtual void updatePar(const Vector<Complex> dCalPar,const Vector<Complex> dSrcPar);

  // Form solution SNR
  virtual void formSolveSNR();

  // Apply SNR threshold
  virtual void applySNRThreshold();

  // Apply refant (implemented in SVJ)
  virtual void reReference()=0;

  // Retrieve the cal flag info as a record
  virtual Record actionRec();

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* incr,
			  const Vector<Int>& fields)=0;

  virtual void smooth(Vector<Int>& fields,
		      const String& smtype,
		      const Double& smtime);

  // Report solved-for QU
  virtual void reportSolvedQU();


  // New CalTable handling
  virtual void createMemCalTable();
  virtual void keep1(Int ichan);
  virtual void keepNCT();
  virtual void storeNCT();
  void storeNCT(const String& tableName,const Bool& append);

  virtual void loadMemCalTable(String ctname,String field="");

  // New spwOK
  virtual Bool spwOK(Int ispw);

  // Post solve tinkering (generic version)
  virtual void globalPostSolveTinker();

  // Divide all solutions by their amplitudes
  virtual void enforceAPonSoln();

  // Normalize a solution (generic implementation)
  virtual void normalize();

  // Determine and apply flux density scaling
  virtual void fluxscale(const String& outfile,
                         const Vector<Int>& refFieldIn,
			 const Vector<Int>& tranFieldIn,
			 const Vector<Int>& inRefSpwMap,
			 const Vector<String>& fldNames,
			 fluxScaleStruct& oFluxScaleStruct,
			 const String& oListFile,
                         const Bool& incremental,
                         const Int& fitorder)=0;

  // Report state:
  inline virtual void state() { stateSVC(True); };

  virtual VisCalEnum::VCParType setParType(VisCalEnum::VCParType type) 
  {parType_ = type;return (VisCalEnum::VCParType)parType_;};
  virtual void currMetaNote();

  virtual void listCal(const Vector<Int> ufldids, const Vector<Int> uantids,
		       const Matrix<Int> uchanids,  //const Int& spw, const Int& chan,
		       const String& listfile="",const Int& pagerows=50)=0;

  // Handle external channel mask
  inline void setChanMask(PtrBlock<Vector<Bool>*>& chanmask) { chanmask_=&chanmask; };
  inline void clearChanMask() { chanmask_=NULL; };
  void applyChanMask(VisBuffer& vb);
  // Log periodic solver activity
  virtual void printActivity(const Int nSlots, const Int slotNo, 
			     const Int fieldId, const Int spw, 
			     const Int nSolutions);
  virtual void markTimer() {timer_p.mark();};


  // -------------
  // Set the simulation parameters
  virtual void setSimulate(VisSet& vs, Record& simpar, Vector<Double>& solTimes);

  // make a corruptor in a VC-specific way
  virtual void createCorruptor(const VisIter& vi,const Record& simpar, const int nSim);

  // access to simulation variables that are general to all VisCals
  inline String& simint() { return simint_; };

  // Simulation info/params, suitable for logging
  virtual String siminfo();

  // Is this calibration simulated?
  inline Bool isSimulated() {return simulated_;};

  // object that can simulate the corruption terms
  CalCorruptor *corruptor_p;

  // calculate # required slots to simulate this SVC
  Int sizeUpSim(VisSet& vs, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes);

protected:

  // Set to-be-solved-for flag
  inline void setSolved(const Bool& flag) {solved_=flag;};

  // Initialize solve parameters (shape)
  virtual void initSolvePar()=0;

  // Invalidate diff cal matrices generically 
  inline virtual void invalidateDiffCalMat()=0;

  // Explicitly synchronize pars with a CalSet slot
  using VisCal::syncPar;

  // Set matrix channelization according to a VisSet
  virtual void setSolveChannelization(VisSet& vs);

  // Calculate chan averaging bounds
  virtual void setFracChanAve();

  // Inflate an NCT w/ meta-data according to a VisSet 
  //   (for accum)
  void inflateNCTwithMetaData(VisSet& vs);

  // Synchronize calibration for solve context
  void syncSolveCal();

  // Synchronize parameters for solve context
  void syncSolvePar();

  // Calculate parameters by some means (e.g., interpolation from a CalSet)
  virtual void calcPar();
  virtual void calcParByCLPP();

  // Synchronize the differentiated calibration 
  virtual void syncDiffMat()=0;

  // Report the SVC-specific state, w/ option for VC::state()
  virtual void stateSVC(const Bool& doVC);

  // Normalize a (complex) solution array (generic)
  void normSolnArray(Array<Complex>& sol,
		     const Array<Bool>& solOK,
		     const Bool doPhase=False);

  // Logger
  LogIO& logSink() { return logsink_p; };

  // Check if a cal table is appropriate
  void verifyCalTable(const String& caltablename);

  void sortVisSet(VisSet& vs, const Bool verbose=False);

  Int parType_;

  // New CalTable 
  NewCalTable *ct_;
  CTPatchedInterp *ci_;
  CLPatchPanel *cpp_;
  Vector<Bool> spwOK_;

  CalSetMetaInfo csmi;

  Double maxTimePerSolution_p, minTimePerSolution_p, avgTimePerSolution_p;
  Float userPrintActivityInterval_p, userPrintActivityFraction_p;
  uInt caiRC_p, cafRC_p;
  Timer timer_p;

  // Set state flag to simulate cal terms
  inline void setSimulated(const Bool& flag) {simulated_=flag;};

  // RI todo implement calcOneJones like calcAllMueller
  // calculate terms during apply, or up front during setSim?
  inline Bool& simOnTheFly() { 
    //    cout << "simOTF=" << onthefly_ << endl;
    return onthefly_; };



private:

  // Default ctor is private
  SolvableVisCal();

  // Initialize pointers, etc.
  void initSVC();

  // Delete pointers
  void deleteSVC();


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
  Vector<Int> urefantlist_;

  // Min baselines per ant for solve
  Int minblperant_;

  // Solved-for flag
  Bool solved_;

  // Signal apply by callib
  Bool byCallib_;

  // Solving mode
  String apmode_;

  // User-specified time-dep solint (string)
  String solint_;

  // User-specified freq-dep solint info
  String fsolint_;
  Double fintervalHz_;
  Vector<Double> fintervalCh_;   // (nSpw)

  // Channel averaging bounds
  Vector<Matrix<Int> > chanAveBounds_;  // (nSpw)(2,nOutChan)

  // Preavering interval
  Double preavg_;

  // Do solution normalization after a solve
  Bool solnorm_;

  // SNR threshold
  Float minSNR_;

  // axes to combine for solve
  String combine_;

  // In-focus channel for single-chan solves on multi-chan data
  Int focusChan_;

  // Solving meta-data
  Double dataInterval_;
  Double fitWt_;
  Double fit_;


  // Current parameters
  PtrBlock<Cube<Complex>*> solveCPar_;  // [nSpw](nPar,1,{1|nElem})
  PtrBlock<Cube<Float>*>   solveRPar_;  // [nSpw](nPar,1,{1|nElem})
  PtrBlock<Cube<Bool>*>    solveParOK_; // [nSpw](nPar,1,{1|nElm})
  PtrBlock<Cube<Float>*>   solveParErr_; // [nSpw](nPar,1,{1|nElm})
  PtrBlock<Cube<Float>*>   solveParSNR_; // [nSpw](nPar,1,{1|nElm})

  PtrBlock<Cube<Complex>*> solveAllCPar_;   // [nSpw](nPar,nChan,{1|nElem})
  PtrBlock<Cube<Float>*>   solveAllRPar_;   // [nSpw](nPar,nChan,{1|nElem})
  PtrBlock<Cube<Bool>*>    solveAllParOK_;  // [nSpw](nPar,nChan,{1|nElm})
  PtrBlock<Cube<Float>*>   solveAllParErr_; // [nSpw](nPar,nChan,{1|nElm})
  PtrBlock<Cube<Float>*>   solveAllParSNR_; // [nSpw](nPar,nChan,{1|nElm})

  Vector<Complex> srcPolPar_;

  // A _pointer_ to the external channel mask
  PtrBlock<Vector<Bool>*> *chanmask_;

  // LogIO
  LogIO logsink_p;

  // Simulation flag
  Bool simulated_;

  // simulation interval
  String simint_;

  Bool onthefly_;  

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
  virtual void guessPar(VisBuffer& ) { throw(AipsError("NYI")); };

  // Differentiate VB model w.r.t. Mueller parameters (no 2nd derivative yet)
  virtual void differentiate(CalVisBuffer& ) {throw(AipsError("NYI")); };
  virtual void differentiate(VisBuffer& ,          // input data
			     Cube<Complex>& ,       // trial apply (nCorr,nChan,nRow)
			     Array<Complex>& ,     // 1st deriv   (nCorr,nPar,nChan,nRow)
			     Matrix<Bool>& ) { throw(AipsError("NYI")); };
  using SolvableVisCal::differentiate;

  // Differentiate VB model w.r.t. Source parameters
  virtual void diffSrc(VisBuffer& ,
		       Array<Complex>& ) {throw(AipsError("NYI")); };

  // Apply refant (no-op for Muellers)
  virtual void reReference() {};

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* ,
			  const Vector<Int>& ) { throw(AipsError("NYI")); };

  // Scale solutions
  virtual void fluxscale(const String&,
                         const Vector<Int>& ,
			 const Vector<Int>& ,
			 const Vector<Int>& ,
			 const Vector<String>& ,
			 SolvableVisCal::fluxScaleStruct&,
			 const String&,
                         const Bool&,
                         const Int&)
	{ throw(AipsError("NYI")); };

  // SVM-specific write to caltable
  virtual void keepNCT();

  // Report state:
  inline virtual void state() { stateSVM(True); };

  // List calibration solutions in tabular form.
  virtual void listCal(const Vector<Int> ,
                       const Vector<Int> ,
                       const Matrix<Int> ,
   		               const String& ,
                       const Int& )
  { throw(AipsError(String("Calibration listing not supported for "+typeName()))); };

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

  // Override VM::setMatByOk in solve context
  void setMatByOk() { if (!isSolved()) VisMueller::setMatByOk(); };

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
  virtual Bool normalizable() { return (this->jonesType() < Jones::GenLinear); };

  // Differentiate VB model w.r.t. Jones parameters
  virtual void differentiate(CalVisBuffer& cvb);
  virtual void differentiate(VisBuffer& vb,          // input data
			     Cube<Complex>& V,       // trial apply (nCorr,nChan,nRow)
			     Array<Complex>& dV,     // 1st deriv   (nCorr,nPar,nChan,nRow,2)
			     Matrix<Bool>& Vflg);
  using SolvableVisMueller::differentiate;

  // Differentiate VB model w.r.t. Source parameters
  virtual void diffSrc(VisBuffer& vb,        
		       Array<Complex>& dV);

  // Apply refant
  virtual void reReference();

  // Accumulate another VisCal onto this one
  virtual void accumulate(SolvableVisCal* incr,
			  const Vector<Int>& fields);

  // Post solve tinkering (Jones version: includes refant application)
  virtual void globalPostSolveTinker();

  // Apply reference antenna (generic Jones version)
  virtual void applyRefAnt();

  // Fluxscale is implemented here
  void fluxscale(const String& outfile,
                 const Vector<Int>& refFieldIn,
		 const Vector<Int>& tranFieldIn,
		 const Vector<Int>& inRefSpwMap,
		 const Vector<String>& fldNames,
		 SolvableVisCal::fluxScaleStruct& oFluxScaleStruct,
		 const String& oListFile,
                 const Bool& incremental=False,
                 const Int& fitorder=1);

  // SVJ-specific write to caltable
  virtual void keepNCT();
			     
  // Report state:
  inline virtual void state() { stateSVJ(True); };

  // Write calibration solutions to the terminal
  virtual void listCal(const Vector<Int> ufldids, const Vector<Int> uantids,
                       const Matrix<Int> uchanids,
                       const String& listfile="",const Int& pagerows=50);

  // Write header for listCal output
  int writeHeader(const uInt numAntCols, 
                  const uInt numAnts,
                  const uInt iElem);

  virtual void nearest(const Double , Array<Float>& ) {};
  virtual void nearest(const Double , Array<Complex>& ) {};
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

  // Override VJ::setMatByOk in solve context
  void setMatByOk() { if (!isSolved()) VisJones::setMatByOk(); };

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

  // Column widths for listing
  uInt wTime_p,  wField_p, wChan_p, wAmp_p, 
       wPhase_p, wFlag_p,  wPol_p,  wAntCol_p, 
       wTotal_p, wPreAnt_p;
          
};

// Global methods

// Discern cal table type from the table itself
String calTableType(const String& tablename);

} //# NAMESPACE CASA - END

#endif

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
//# $Id: VisCal.h,v 1.10 2006/02/06 19:23:11 gmoellen Exp $

#ifndef SYNTHESIS_VISCAL_H
#define SYNTHESIS_VISCAL_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <synthesis/MeasurementComponents/Mueller.h>
#include <synthesis/MeasurementComponents/Jones.h>
#include <synthesis/CalTables/VisCalEnum.h>
#include <synthesis/MeasurementComponents/VisVector.h>
#include <synthesis/MSVis/VisSet.h>

#include <synthesis/MSVis/VisBuffGroupAcc.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  VisCal
//

class VisCal {

  friend class SolvableVisJones;

public:
 
  // Allowed types of VisCal matrices - 'correct' order
  //  enum Type{UVMOD,Mf,M,K,B,G,D,C,E,P,T,EP,F};
  enum Type{Test=0,ANoise,M,KAntPos,K,B,G,J,D,X,C,P,E,T,F,A,ALL};

  // Enumeration of parameter types (Complex, Real, or Both)
  enum ParType{Co,Re,CoRe};

  static String nameOfType(Type type) {
    switch (type) {
    case ANoise: return "ANoise";
    case M: return "M";
    case K: return "K";
    case B: return "B";
    case J: return "J";
    case D: return "D";
    case X: return "X";
    case C: return "C";
    case P: return "P";
    case E: return "E";
    case T: return "T";
    case F: return "F";
    case A: return "A";
    default: return "0";
    }
  }

  VisCal(VisSet& vs);
  
  VisCal(const Int& nAnt);
  
  virtual ~VisCal();

  // Return the type of this calibration matrix (actual type of derived class).
  //    (Must be implemented in specializations!)
  virtual Type type()=0;

  // Return type name as string
  virtual String typeName()     { return "Unknown VisCal"; };
  virtual String longTypeName() { return "Unknown VisCal"; };

  // Return Matrix type
  virtual VisCalEnum::MatrixType matrixType() { return VisCalEnum::GLOBAL; };

  // Return the parameter type (nominally complex)
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::COMPLEX; };

  // Number of pars per ant/bln
  //    (Must be implemented in specializations!)
  virtual Int nPar()=0;

  // Report calibration availability per spw
  //  (always True for non-tabular?)
  virtual Vector<Bool> spwOK() { return Vector<Bool>(nSpw(),True); };
  virtual Bool spwOK(Int) { return True; };

  // Frequency-dependent Parameters?  Nominally not.
  virtual Bool freqDepPar() { return False; };

  // Number of par channels in current spw
  inline const Int& nChanPar() const { return nChanPar_[currSpw_]; };
  
  // Frequency-dependent Matrices?  Nominally same as freqDepPar.
  virtual Bool freqDepMat() { return freqDepPar(); };

  // Matrices time-dependent per parameter set (nominally no)
  virtual Bool timeDepMat() { return False; };

  // Is this calibration matrix to be applied?
  inline Bool isApplied() {return applied_;};

  // Is this solveable? (never via this interface)
  virtual Bool isSolvable() {return False;};

  // Return the time interval over which this calibration is constant
  inline virtual Double& interval() {return interval_;}

  // Set the application parameters
  virtual void setApply();
  virtual void setApply(const Record& apply);
  virtual void setCallib(const Record& callib,const MeasurementSet& selms);

  // Apply info/params, suitable for logging
  virtual String applyinfo();

  // simulation params - for a VC, sim is apply; for a SVC this will get 
  // overriden
  inline virtual String siminfo() { return applyinfo(); };

  // Trigger calibration of weights
  inline Bool& calWt() { return calWt_; };

  // Apply calibration to data in VisBuffer (correct Data or corrupt Model)
  //  (in-place versions)
  virtual void correct(VisBuffer& vb, Bool trial=False);
  //  virtual void corrupt(VisBuffer& vb);
  virtual void corrupt(VisBuffer& vb);
  // Apply calibration to data in VisBuffer; 
  //  (alternate output versions)
  virtual void correct(VisBuffer& vb, Cube<Complex>& Vout,Bool trial=False);
  //  virtual void corrupt(VisBuffer& vb, Cube<Complex>& Mout);
  virtual void corrupt(VisBuffer& vb, Cube<Complex>& Mout);

  // Flag counting
  virtual void initCalFlagCount();
  virtual Record actionRec();

  // Report the state
  virtual void state();

  virtual void currMetaNote();

  // Set the print level
  inline void setPrtlev(const Int& prtlev) { prtlev_=prtlev; };

  // Baseline index from antenna indices: (assumes a1<=a2 !!)
  inline Int blnidx(const Int& a1, 
		    const Int& a2) { return  a1*nAnt() - a1*(a1+1)/2 + a2; };

  inline String& extraTag() { return extratag_; };
 
protected:

  // Set applied state flag
  inline void setApplied(const Bool& flag) {applied_=flag;};

  inline String& msName() { return msName_; };

  // General Shape Info 
  inline Int& nSpw() { return nSpw_; };
  inline Int& nAnt() { return nAnt_; };
  inline Int& nBln() { return nBln_; };

  // The number of sets of parameters under consideration
  virtual Int& nElem()=0;

  // Number of Calibration matrices on ant/bln axis
  virtual Int nCalMat()=0;

  // Access to current vb
  inline const VisBuffer& vb() { return *vb_; };

  // Current in-focus spw
  inline Int& currSpw() { return currSpw_; };

  // Current coords
  inline Double& lastTime()                { return lastTime_(currSpw()); };
  inline Double& currTime()                { return currTime_(currSpw()); };
  inline Int&    currScan()                { return currScan_(currSpw()); };
  inline Int&    currObs()                 { return currObs_(currSpw()); };
  inline Int&    currField()               { return currField_(currSpw()); };
  inline Vector<Double>& currFreq()        { return currFreq_; };

  inline Double& refTime()                 { return refTime_; };
  inline Double& refFreq()                 { return refFreq_; };

  // Current spectral shapes
  inline Int& nChanPar()              { return nChanPar_[currSpw_]; };
  inline Int& nChanMat()              { return nChanMat_[currSpw_]; };
  inline Int& startChan()             { return startChan_[currSpw_];};
  inline Vector<Int>& nChanParList()  { return nChanPar_; };
  inline Vector<Int>& nChanMatList()  { return nChanMat_; };
  inline Vector<Int>& startChanList() { return startChan_;};

  // Access to matrix renderings of Visibilities
  inline VisVector& V() { return (*V_[currSpw()]); };

  // Access to current solution parameters and matrices
  inline virtual Cube<Complex>& currCPar()  {return (*currCPar_[currSpw()]);};
  inline virtual Cube<Float>&   currRPar()  {return (*currRPar_[currSpw()]);};
  inline virtual Cube<Bool>&    currParOK() {return (*currParOK_[currSpw()]);};

  // Validation of calibration parameters
  inline void invalidateP() {PValid_(currSpw())=False;};
  inline void validateP()   {PValid_(currSpw())=True;};
  inline Bool PValid()      {return PValid_(currSpw());};

  // Invalidate cal matrices generically 
  virtual void invalidateCalMat()=0;

  // Access to weight-scaling factors
  inline Matrix<Float>& currWtScale() { return (*currWtScale_[currSpw()]); };

  // Flag counting
  virtual void countInFlag(const VisBuffer& vb);
  virtual void countOutFlag(const VisBuffer& vb);

  // Row-by-row apply to a Cube<Complex> (generic)
  virtual void applyCal(VisBuffer& vb, Cube<Complex>& Vout,Bool trial=False)=0;

  // Synchronize "gains" with a VisBuffer or another VisCal
  virtual void syncCal(const VisBuffer& vb,
		       const Bool& doInv=False);
  virtual void syncCal(VisCal& vc);

  // Set internal meta data from a VisBuffer or another VisCal
  void syncMeta(const VisBuffer& vb);
  void syncMeta(VisCal& vc);

  void syncMeta(const Int& spw,
		const Double& time,
		const Int& field,
		const Vector<Double>& freq,
		const Int& nchan);

  // Set the calibration matrix channelization
  void setCalChannelization(const Int& nChanDat);

  // Test for need of new calibration
  void checkCurrCal();

  // Synchronize "gains" with current meta-data
  virtual void syncCal(const Bool& doInv=False);

  // Sync parameters for current meta data
  virtual void syncPar();

  // Calculate Mueller parameters by some means 
  virtual void calcPar();

  // Sync matrices generically for current meta data 
  virtual void syncCalMat(const Bool& doInv=False)=0;

  // Return print (cout) level
  inline Int& prtlev() { return prtlev_; };

  // set current field index vector to given field id
  void setCurrField(const Int& ifld);

private:

  // Defalt ctor is private
  VisCal();

  // Initialize pointers, etc.
  void initVisCal();

  // Delete pointers
  void deleteVisCal();

  // Associated MS name
  String msName_;
  
  // Number of Spectral windows
  Int nSpw_;

  // Number of antennas
  Int nAnt_;

  // Number of baselines
  Int nBln_;

  // Pointer to the current vb
  const VisBuffer* vb_;

  // Current synchronized spw
  Int currSpw_;

  // Current indices
  Vector<Double> currTime_;
  Vector<Int> currScan_;
  Vector<Int> currObs_;
  Vector<Int> currField_;
  Vector<Double> currFreq_;
  Vector<Double> lastTime_;
  Double refTime_;
  Double refFreq_;

  // Channel counts
  Vector<Int> nChanPar_, nChanMat_;
  Vector<Int> startChan_;

  // Solution timescale (context-dependent)
  Double interval_;

  // Application flag
  Bool applied_;

  // In-focus channel for single-chan solves on multi-chan data
  Int focusChan_;

  // VisVector wrapper (per Spw)
  PtrBlock<VisVector*> V_;

  // Current parameters
  PtrBlock<Cube<Complex>*>  currCPar_;   // [nSpw](nPar,nChanPar,nElm)
  PtrBlock<Cube<Float>*>    currRPar_;   // [nSpw](nPar,nChanPar,nElm)
  PtrBlock<Cube<Bool>*>     currParOK_;  // [nSpw](nPar,nChanPar,nElm)

  // Paremeter validity
  Vector<Bool> PValid_;

  // Trigger calibration of weights
  Bool calWt_;

  // Weight scale factors
  PtrBlock<Matrix<Float>*> currWtScale_;  // [nSpw](nPar,nElm)

  // Flag counting
  Int64 ndataIn_, nflagIn_, nflagOut_;

  // Print level
  Int prtlev_;

  String extratag_;  // e.g. to tag as noise scale


};


// **********************************************************
//  VisMueller
//

class VisMueller : virtual public VisCal {

public:

  // Constructor
  VisMueller(VisSet& vs);

  VisMueller(const Int& nAnt);

  virtual ~VisMueller();

  // Return Matrix type
  virtual VisCalEnum::MatrixType matrixType() { return VisCalEnum::MUELLER; };

  // Mueller matrix type (must be implemented in Mueller specializations!)
  virtual Mueller::MuellerType muellerType()=0;  

  // Report the state
  virtual void state();

protected:

  // Total number of parameter sets required
  virtual Int& nElem() { return nBln(); };

  // Number of Cal Matrices to form on baseline axis
  //  (Mueller, apply context: nBln())
  virtual Int nCalMat() { return nBln(); };

  // Are the parameters the matrix elements? 
  //   (or is a non-trivial calculation required?)
  //    (Must be implemented in specializations!)
  virtual Bool trivialMuellerElem()=0;

  // Are we applying via Mueller multiplication?
  //   (necessarily True for native Muellers)
  virtual Bool applyByMueller() { return True; };

  // Access to matrix renderings of Muellers
  inline Mueller& M()   { return (*M_[currSpw()]); };

  // Access to current matrices
  inline Cube<Complex>& currMElem()   {return (*currMElem_[currSpw()]);};
  inline Cube<Bool>&    currMElemOK() {return (*currMElemOK_[currSpw()]);};

  // Invalidate cal matrices generically (at this level, just Mueller)
  inline virtual void invalidateCalMat() { invalidateM(); };

  // Validation of Mueller matrices (for currSpw)
  inline void invalidateM() {MValid_(currSpw())=False;};
  inline void validateM()   {MValid_(currSpw())=True;};
  inline Bool MValid()      {return MValid_(currSpw());};

  // Row-by-row apply to a Cube<Complex> (applyByMueller override)
  virtual void applyCal(VisBuffer& vb, Cube<Complex>& Vout,Bool trial=False);

  // Sync matrices for current meta data (Mueller override)
  virtual void syncCalMat(const Bool& doInv=False);

  // Sync Mueller matrix elements for current parameters
  virtual void syncMueller(const Bool& doInv=False);

  // Calculate an ensemble of Mueller matrices (all baselines, channels)
  virtual void calcAllMueller();

  // Calculate a single Mueller matrix by some means
  virtual void calcOneMueller(Vector<Complex>& mat, Vector<Bool>& mOk,
			      const Vector<Complex>& par, const Vector<Bool>& pOk);

  // Invert Mueller matrices
  virtual void invMueller();

  // Set matrix elements according to their ok flags
  //  (e.g., makes a unit matrix if everything flagged,
  //   so we don't have to make atomic ok checks in apply)
  virtual void setMatByOk();

  // Create Mueller matrix algebra interface
  void createMueller();
 
  // Synchronize weight scale factors
  virtual void syncWtScale();

  // Update the wt vector for a baseline
  virtual void updateWt(Vector<Float>& wt,const Int& a1,const Int& a2);


private:

  // Default ctor is private
  VisMueller();

  // Init VisMueller pointers, etc.
  void initVisMueller();

  // Delete the this* caches
  void deleteVisMueller();

  // Mueller algebra wrapper (per Spw)
  PtrBlock<Mueller*> M_;                

  // Current Mueller matrix elements
  PtrBlock<Cube<Complex>*> currMElem_;    // [nSpw]([1,2,4,16],nChanMat,nBln)
  PtrBlock<Cube<Bool>*>    currMElemOK_;  // [nSpw]([1,2,4,16],nChanMat,nBln)

  // Mueller validity
  Vector<Bool> MValid_;

};


// **********************************************************
//  VisJones
//

class VisJones : virtual public VisMueller {

  friend class SolvableVisJones;

public:

  // Constructor
  VisJones(VisSet& vs);

  VisJones(const Int& nAnt);

  virtual ~VisJones();

  // Return Matrix type
  virtual VisCalEnum::MatrixType matrixType() { return VisCalEnum::JONES; };

  // What kind of Mueller matrices should we use?  
  //   (A function of the jonesType and target data shape)
  virtual Mueller::MuellerType muellerType();

  // What kind of Jones matrix are we?  
  //    (Must be implemented in specializations!)
  virtual Jones::JonesType jonesType()=0; 

  // Report the state
  virtual void state();

protected:

  // Number of parameter sets is number of antennas
  inline virtual Int& nElem() { return nAnt(); };

  // Number of Cal Matrices to form on antenna axis
  //  (Jones, apply context: nAnt())
  virtual Int nCalMat() { return nAnt(); };

  // Jones matrices can never be trivial Muellers!
  virtual Bool trivialMuellerElem() { return False; };

  // Are the parameters the Jones matrix elements?
  //   (or is a non-trivial calculation required?)
  //    (Must be implemented in specializations!)
  virtual Bool trivialJonesElem()=0;

  // Are we applying via Mueller or Jones multiplication?
  //   (probably by Jones for native Jones?)
  virtual Bool applyByMueller() { return False; };
  virtual Bool applyByJones()   { return True; };

  // Access to matrix renderings of Jones matrices
  inline Jones& J1() { return *J1_[currSpw()]; };
  inline Jones& J2() { return *J2_[currSpw()]; };

  // Access to Jones matrix element array
  inline Cube<Complex>& currJElem() {return (*currJElem_[currSpw()]);};
  inline Cube<Bool>& currJElemOK()  {return (*currJElemOK_[currSpw()]);};

  // Invalidate cal matrices generically (at this level, both Mueller and Jones)
  inline virtual void invalidateCalMat() { invalidateM(); invalidateJ(); };

  // Validation of Jones matrices
  inline void invalidateJ() {JValid_(currSpw())=False;};
  inline void validateJ()   {JValid_(currSpw())=True;};
  inline Bool JValid()      {return JValid_(currSpw());};

  // Row-by-row apply to a Cube<Complex> (applyByJones override)
  virtual void applyCal(VisBuffer& vb, Cube<Complex>& Vout,Bool trial=False);

  // Sync matrices for current meta data (VisJones override)
  virtual void syncCalMat(const Bool& doInv=False);

  // Calculate an ensemble of Mueller matrices (all baselines, channels)
  //  (only meaningful if applyByMueller()=T)
  virtual void calcAllMueller();

  // Synchronize current Jones matrices
  virtual void syncJones(const Bool& doInv=False);

  // Calculate an ensemble of Jones matrices (all antennas, channels)
  virtual void calcAllJones();

  // Calculate a single Jones matrix by some means from parameters
  virtual void calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk, 
			    const Vector<Complex>& par, const Vector<Bool>& pOk );

  // Invert Jones matrices
  virtual void invJones();

  // Set matrix elements according to their ok flags
  //  (e.g., makes a unit matrix if everything flagged,
  //   so we don't have to make atomic ok checks in apply)
  virtual void setMatByOk();

  // Create Jones interface
  void createJones();

  // Synchronize weight scale factors
  virtual void syncWtScale();

  // Update the wt vector for a baseline
  virtual void updateWt(Vector<Float>& wt,const Int& a1,const Int& a2);

private:

  // Default ctor is private
  VisJones();

  // Init VisJones pointers
  void initVisJones();

  // Delete the this* caches
  void deleteVisJones();

  // Jones algebra wrapper (per Spw)
  PtrBlock<Jones*> J1_;
  PtrBlock<Jones*> J2_;

  // Current Jones matrix-element arrays
  PtrBlock<Cube<Complex>*> currJElem_;    // [nSpw](nJElem,nChanMat,nAnt)
  PtrBlock<Cube<Bool>*>    currJElemOK_;  // [nSpw](nJElem,nChanMat,nAnt)

  // Jones validity, per spw
  Vector<Bool> JValid_;


};

} //# NAMESPACE CASA - END

#endif

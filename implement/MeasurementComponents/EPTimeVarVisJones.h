//# EPTimeVarVisJones.h: Definition for EPTimeVarVisJones matrices
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$
#ifndef SYNTHESIS_EPTIMEVARVISJONES_H
#define SYNTHESIS_EPTIMEVARVISJONES_H
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MeasurementComponents/VisJones.h>
//#include <synthesis/MeasurementEquations/VisEquation.h>
#include <casa/OS/File.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

namespace casa {
// <summary> Model time-variable multiplicative errors for the VisEquation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
//   <li> <linkto class="VisJones">VisJones</linkto> class
// </prerequisite>
//
// <etymology>
// EPTimeVarVisJones describes an interface for time variable but not solvable
// multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// EPTimeVarVisJones performs caching of values to minimize computation.
//
// Values may be saved to a table.
// </synopsis> 
//
// <example>
// <srcblock>
//      // Read the VisSet from disk
//      VisSet vs("3c84.MS");
//
//      VisEquation ve(vs);
//
//      // Make an VisJones
//      PJones pj(vs);
//
//      ve.setVisJones(pj);
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

typedef SquareMatrix<Float,2> EPmjJones2;
typedef SquareMatrix<Float,4> EPmjJones4;
typedef SquareMatrix<Float,2> EPmjJones2F;

class EPTimeVarVisJones: public VisJones {
public:

  EPTimeVarVisJones() {}

  // From VisSet 
  EPTimeVarVisJones(VisSet& vs);

  // Copy
  EPTimeVarVisJones(const EPTimeVarVisJones& other);

  // Assignment
  //  virtual EPTimeVarVisJones& operator=(const EPTimeVarVisJones& other);

  // Destructor
  virtual ~EPTimeVarVisJones();

  // Make local VisSet
  virtual void makeLocalVisSet();
  // Setup apply context
  virtual void setInterpolation(const Record& interpolation);

  // Initialize interpolation parameters
  virtual void initInterp(const Record& interpolation);

  // Cleanup interpolation parameters
  virtual void deleteInterp();

  // Return the type of this Jones matrix (actual type of derived class).
  virtual Type type()=0;

  // Frequency-dependent ?
  virtual Bool freqDep() {return False;}

  // Is this Jones matrix to be solved for, applied or both ?
  virtual Bool isSolved() {return solved_;};
  virtual Bool isApplied() {return applied_;};

  // Return the interval over which this Jones matrix is constant
  virtual Double interval();

  // Return the pre-averaging interval (here equal to the interval).
  virtual Double preavg() {return interval();};

  // Apply Jones matrix 
  virtual VisBuffer& apply(VisBuffer& vb){return vb;}; 

  /*
  // Apply inverse of Jones matrix
  virtual VisBuffer& applyInv(VisBuffer& vb);
  */
  virtual VisBuffer & applyGradient(VisBuffer & vb,
				    const Vector<int> & v,
				    Int a1, Int a2, Int pos);

  virtual void addGradients(const VisBuffer& vb, Int row, const Antenna& a, 
			    const Vector<Float>& sumwt, 
			    const Vector<Float>& chisq,
                            const Vector<EPmjJones2>& c, 
			    const Vector<EPmjJones2F>& f);

  //  virtual Bool solve(class VisEquation &) {return False;}

  // Store the time-variable gains as a table.
  // Derived classes may need to redefine this if they do not use antGain_.
  virtual void store(const String& file, const Bool& append) {};

  // print out internals
  virtual void query();
  // Calculate the current antenna gain parameters by 
  //  some means (e.g., interpolation)
  virtual void calcAntGainPar(const VisBuffer& vb);
  // Get the current set of antenna & baseline gains 
  //  into the this* cache
  virtual void getThisGain(const VisBuffer& vb, 
			   const Bool& forceAntMat=True,
			   const Bool& doInverse=False,
			   const Bool& forceIntMat=True);




  // Retrieve the time-variable gains from a table.
  virtual void load(const String& file, const String& select,
		    const String& type) {};

  void initialize(VisSet& vs);

  void initMetaCache();
  void fillMetaCache();
  void deleteMetaCache();

  void initThisGain();
  void initAntGain();
  void deleteThisGain();
  void deleteAntGain();

protected:
  // Mark this Jones matrix as being solved for
  void setSolved(const Bool& flag) {solved_=flag;};

  // Mark this Jones matrix as being applied
  void setApplied(const Bool& flag) {applied_=flag;};

  // Calculate 2x2 antenna gain matrices (all chans, ants), using
  //  methods provided in concrete types
  virtual void fillAntGainMat();
  virtual void calcAntGainMat(EPmjJones2& mat, Vector<Float>& par );

  // Invert antenna gain matrices
  virtual void invAntGainMat();
  
  // Calculate 4x4 baseline gain matrices
  virtual void calcIntGainMat() {};

  void invalidateAGM(Int& spw) {thisAGMValid_(spw)=False;};
  void validateAGM(Int& spw)   {thisAGMValid_(spw)=True;};
  void invalidateIGM(Int& spw) {thisIGMValid_(spw)=False;};
  void validateIGM(Int& spw)   {thisIGMValid_(spw)=True;};
  Bool AGMValid(Int& spw)      {return thisAGMValid_(spw);};
  Bool IGMValid(Int& spw)      {return thisIGMValid_(spw);};

  // Return true if polarization re-sequencing required
  virtual Bool polznSwitch(const VisBuffer& vb);

  // Re-sequence to (XX,XY,YX,YY) or (RR,RL,LR,LL)
  virtual void polznMap(CStokesVector& vis);

  // Re-sequence to (XX,YY,XY,YX) or (RR,LL,RL,LR)
  virtual void polznUnmap(CStokesVector& vis);

  //
  // Keep reference to VisSet
  VisSet* vs_;
  Bool localVS_;  // True only if VisSet new'd within this object

  // Number of parameters describing this calibration component
  Int nPar_;

  // Parameter type (0=complex, 1=real)
  //  TODO: make this an enum
  Int parType_;

  // this* valid information
  Vector<Bool> thisAGMValid_;    // (numberSpw_)
  Vector<Bool> thisIGMValid_;    // (numberSpw_)
  Vector<Double> thisTimeStamp_;  // (numberSpw_)
  Vector<Double> thisSlot_;      // (numberSpw_)

  // Flags indicating whether the Jones matrix is to be applied,
  // solved for, or both.
  Bool solved_, applied_;

  // Interval of each slot
  Double interval_;

  // Precision of time comparison
  Double deltat_;

  // Status of cache
  Vector<Bool> cacheValid_;  // (numberSpw_)

  // Number of antennas
  Int numberAnt_;

  // Number of Spectral windows
  Int numberSpw_;

  // Current Spw
  Int currentSpw_;

  // Spw Mapping
  Vector<Int> spwMap_;
  Int currentCalSpw_;

  // Start and Stop time of current Slot
  Double currentStart_, currentStop_;

  // Remember last timestamp
  Double lastTimeStamp_;

  // Remember last Spw
  Int lastSpw_;

  // Jones matrix type (1="scalar", 2="diag", 3="general")
  // TODO: make this an enum
  Int jMatType_;

  LogSink logSink_p;
  LogSink& logSink() {return logSink_p;};

  //-----------------------------------------------------------
  // New PB versions

  Vector<Bool> newPB_;         // (numberSpw_) True if PB was made with new

  Vector<Int> numberSlot_;     // (numberSpw_) number of slots per spw
  Vector<Int> currentSlot_;    // (numberSpw_) current slot (per spw) in iteration
  Vector<Int> nSolnChan_;      // (numberSpw_) number of _soln_ channels per spw
  Vector<Int> startChan_;      // (numberSpw_) number of _soln_ channels per spw

  // Per spw, per slot Time information
  PtrBlock<Vector<Double>*> MJDStart_;       // (numberSpw_)(numberSlots_)
  PtrBlock<Vector<Double>*> MJDStop_;        // (numberSpw_)(numberSlots_)
  PtrBlock<Vector<Double>*> MJDTimeStamp_;   // (numberSpw_)(numberSlots_)
  
  // Per spw, per slot field/source information
  PtrBlock<Vector<Int>*> fieldId_;          // (numberSpw_)(numberSlots_)
  PtrBlock<Vector<String>*> fieldName_;     // (numberSpw_)(numberSlots_)
  PtrBlock<Vector<String>*> sourceName_;    // (numberSpw_)(numberSlots_)

  // Per spw, per channel, per antenna, per slot solution generic PARAMETER storage  
  PtrBlock<Array<Float>*> antGainPar_;  // (numberSpw_)(nPar_,nSolnChan_,numberAnt_,numberSlots_)
  //  PtrBlock<Array<Double>*>  antGainRPar_;  // (numberSpw_)(nPar_,nSolnChan_,numberAnt_,numberSlots_)
  PtrBlock<Cube<Bool>*> antGainParOK_;    // (numberSpw_)(nSolnChan_,numberAnt_,numberSlots_)

  // CURRENT, Per spw, per channel, per antenna ant and baseline solutions 
  PtrBlock<Cube<Float>*>    thisAntGainPar_;  // (numberSpw_)(nPar_,nSolnChan_,numberAnt_)
  //  PtrBlock<Cube<Double>*>     thisAntGainaRPar_; // (numberSpw_)(nPar_,nSolnChan_,numberAnt_)
  PtrBlock<Matrix<EPmjJones2>*> thisAntGainMat_;  // (numberSpw_)(nSolnChan_,numberAnt_)(2,2)
  PtrBlock<Vector<Bool>*>     thisAntGainOK_;   // (numberSpw_)(numberAnt_)
  PtrBlock<Cube<EPmjJones4>*>   thisIntGainMat_;  // (numberSpw_)(nSolnChan_,numberAnt_,numberAnt_)



  // Interpolation parameters (here temporarily)
  String interpType_;

  Vector<Double> timeRef_;
  Vector<Float> timeStep_;
  PtrBlock<Cube<Float>*> ampRef_, ampSlope_;
  PtrBlock<Cube<Float>*> phaRef_, phaSlope_;
  PtrBlock<Cube<Float>*> gainRef_, gainSlope_;

  Bool hasData;
};


inline VisBuffer & EPTimeVarVisJones::applyGradient(VisBuffer & vb,
						    const Vector<int> &,
						    Int, Int, Int) 
{return vb;}
inline void EPTimeVarVisJones::addGradients(const VisBuffer&, Int, const Antenna&,   
					       const Vector<Float>&, 
					       const Vector<Float>&,
					       const Vector<EPmjJones2>&, 
					       const Vector<EPmjJones2F>&) {}

  // A global function to get the time stamp of the VisBuffer
  Double getCurrentTimeStamp(const VisBuffer& vb);

}
#endif

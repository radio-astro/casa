//# KJones.h: Declaration of delay-like (geometry) calibration types
//# Copyright (C) 1996,1997,2000,2001,2002,2003,2011
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

#ifndef SYNTHESIS_KJONES_H
#define SYNTHESIS_KJONES_H

#include <casa/aips.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class DelayFFT {
public:
  // Construct from freq info and data-like Cube<Complex>
  //  (for generic testing w/out MS data)
  DelayFFT(Double f0, Double df, Double padBW, 
	   Cube<Complex> V);

  // Construct from freq info and shape, w/ initialization
  //   (for accumulation)
  DelayFFT(Double f0, Double df, Double padBW, 
	   Int nCorr, Int nElem, Int refant, Complex v0);

  // Construct from a VB
  DelayFFT(const VisBuffer& vb,Double padBW,Int refant);

  // Perform FFT
  void FFT();

  // Apply shift theorem
  void shift(Double f);

  // Accumulate another DelayFFT
  void add(const DelayFFT& other);

  // Find peaks
  void searchPeak();

  const Cube<Complex>& Vpad() const { return Vpad_; };

  // Access to results
  const Matrix<Float>& delay() const { return delay_; };
  const Matrix<Bool>& flag() const { return flag_; };

  // Report some stateinfo
  void state();

private:
  Double f0_, df_, padBW_;
  Int nCorr_, nPadChan_, nElem_;
  Int refant_;
  Cube<Complex> Vpad_;
  Matrix<Float> delay_;
  Matrix<Bool> flag_;

};






// Forward declarations

// K Jones provides support for SBD delays
class KJones : public GJones {
public:

  // Constructor
  KJones(VisSet& vs);
  KJones(const Int& nAnt);

  virtual ~KJones();

  // Local setApply to enforce calWt=F for delays
  virtual void setApply(const Record& apply);
  using GJones::setApply;

  // Local setSolve (traps lack of refant)
  virtual void setSolve(const Record& solve);
  using GJones::setSolve;

  // We have Float parameters
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::REAL; };

  // Return the type enum
  virtual Type type() { return VisCal::K; };

  // Return type name as string
  virtual String typeName()     { return ( (isSolved() && combspw()) ? 
					   "KMBD Jones" : 
					   "K Jones" ); };
  virtual String longTypeName() { return ( (isSolved() && combspw()) ? 
					   "KMBD Jones (multi-band delay)" : 
					   "K Jones (single-band delay)"  ); };

  // Type of Jones matrix according to nPar()
  virtual Jones::JonesType jonesType() { return Jones::Diagonal; };

  // Freq dependence (delays)
  virtual Bool freqDepPar() { return False; };
  virtual Bool freqDepMat() { return True; };

  // Default parameter value
  virtual Complex defaultPar() { return Complex(0.0); };

  // Type-specific specify
  virtual void specify(const Record& specify);

  // This type is not yet accumulatable
  virtual Bool accumulatable() { return False; };

  // This type is smoothable
  virtual Bool smoothable() { return True; };

  // Calculate phase(chan) from delay
  virtual void calcAllJones();

  // Delay to phase calculator
  //  virtual void calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk, 
  //                            const Vector<Complex>& par, const Vector<Bool>& pOk );


  // Hazard a guess at parameters (unneeded here)
  virtual void guessPar(VisBuffer& ) {};

  // K now uses generic gather, but solves for itself per solution
  virtual Bool useGenericGatherForSolve() { return True; };
  virtual Bool useGenericSolveOne() { return False; }

  // Override G here; nothing to do for K, for now
  virtual void globalPostSolveTinker() {};


protected:

  // K has two "real" parameters
  virtual Int nPar() { return 2; };

  // Jones matrix elements are trivial
  virtual Bool trivialJonesElem() { return False; };

  // dG/dp are trivial
  virtual Bool trivialDJ() { return False; };

  // Initialize trivial dJs
  virtual void initTrivDJ() {};

  // Local implementation of selfSolveOne (generalized signature)
  virtual void selfSolveOne(VisBuffGroupAcc& vbga);

  // FFT solver for one VB
  virtual void solveOneVB(const VisBuffer& vb);

  // FFT solver for multi-VB (MBD)
  virtual void solveOneVBmbd(VisBuffGroupAcc& vbga);

  // Reference frequencies
  Vector<Double> KrefFreqs_;

private:

  
};

// (sbd) K for cross-hand solve
class KcrossJones : public KJones {
public:

  // Constructor
  KcrossJones(VisSet& vs);
  KcrossJones(const Int& nAnt);

  virtual ~KcrossJones();

  // Return type name as string
  virtual String typeName()     { return "Kcross Jones"; };
  virtual String longTypeName() { return "Kcross Jones (single-band cross delay)"; };

  // By definition, we consider cross-hands
  virtual Bool phandonly() { return False; };

protected:

  // Local implementation of selfSolveOne 
  //   This traps combine='spw', which isn't supported yet
  virtual void selfSolveOne(VisBuffGroupAcc& vbga);

  // FFT solver for on VB, that collapses baselines and cross-hands first
  virtual void solveOneVB(const VisBuffer& vb);

};


// KMBD Jones provides support for multi-band delays
class KMBDJones : public KJones {
public:

  // Constructor
  KMBDJones(VisSet& vs);
  KMBDJones(const Int& nAnt);

  virtual ~KMBDJones();

  // Return the type enum
  virtual Type type() { return VisCal::K; };

  // Return type name as string
  virtual String typeName()     { return "KMBD Jones"; };
  virtual String longTypeName() { return "KMBD Jones (multi-band delay)"; };

  // Local setApply (to enforce KrefFreq_=0.0)
  virtual void setApply(const Record& apply);

 
};



class KAntPosJones : public KJones {
public:

  // Constructor
  KAntPosJones(VisSet& vs);
  KAntPosJones(const Int& nAnt);

  virtual ~KAntPosJones();

  // Return the type enum
  virtual Type type() { return VisCal::KAntPos; };

  // Return type name as string
  virtual String typeName()     { return "KAntPos Jones"; };
  virtual String longTypeName() { return "KAntPos Jones (antenna position errors)"; };

  // This is a scalar Jones matrix
  virtual Jones::JonesType jonesType() { return Jones::Scalar; };

  virtual Bool timeDepMat() { return True; };

  // This type is not smoothable
  virtual Bool smoothable() { return False; };

  // Local setApply to enforce spwmap=0 for all spw
  virtual void setApply(const Record& apply);
  using KJones::setApply;

  // Type-specific specify
  virtual void specify(const Record& specify);

  // Calculate phase(chan) from delay
  virtual void calcAllJones();

protected:

  // Detect phase direction and antpos for the current VB
  virtual void syncMeta(const VisBuffer& vb);
  virtual void syncMeta2(const vi::VisBuffer2& vb);

  // AntPos has three "real" parameters (dBx, dBy, dBz)
  virtual Int nPar() { return 3; };

  // Jones matrix elements are not trivial
  virtual Bool trivialJonesElem() { return False; };

  // dG/dp are not trivial
  virtual Bool trivialDJ() { return False; };

  // Initialize trivial dJs
  virtual void initTrivDJ() {};

private:
  
  // Geometry info for internal calculations (updated per VB)
  String epochref_p;
  MDirection phasedir_p;
  MPosition antpos0_p;

};

} //# NAMESPACE CASA - END

#endif

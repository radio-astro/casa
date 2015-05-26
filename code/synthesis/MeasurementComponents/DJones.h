//# StandardVisCal.h: Declaration of standard (Solvable)VisCal types
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

#ifndef SYNTHESIS_DJONES_H
#define SYNTHESIS_DJONES_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/CalCorruptor.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class VisEquation;

// **********************************************************
//  DJones
//

class DJones : public SolvableVisJones {
public:

  // Constructor
  DJones(VisSet& vs);
  DJones(String msname,Int MSnAnt,Int MSnSpw);
  DJones(const Int& nAnt);

  virtual ~DJones();

  // Local setapply
  using SolvableVisJones::setApply;
  virtual void setApply(const Record& apply);

  // D-specific solve setup
  using SolvableVisJones::setSolve;
  void setSolve(const Record& solvepar);

  // Return the type enum
  virtual Type type() { return VisCal::D; };

  // Return type name as string
  virtual String typeName()     { return "Dgen Jones"; };
  virtual String longTypeName() { return "Dgen Jones (instrumental polarization"; };

  // Type of Jones matrix according to nPar()
  //   Do GENERAL matrix algebra
  virtual Jones::JonesType jonesType() { return Jones::General; };

  // We can solve for polarization with D
  virtual Int solvePol() { return solvePol_; };

  // Hazard a guess at parameters
  virtual void guessPar(VisBuffer& vb);

  // Update the parameters from solving
  //  (in linear approx, we always set the source update to zero, for now!)
  virtual void updatePar(const Vector<Complex> dCalPar,
			 const Vector<Complex> dSrcPar);

  // SNR is 1/err for D-terms (?)
  virtual void formSolveSNR();

  // D-specific post-solve stuff
  virtual void globalPostSolveTinker();

  // D-specific reReference
  // TBD: non-triv impl
  virtual void reReference() { cout << "reReference!" << endl;};

  virtual void applyRefAnt();

  // Method to list the D results
  virtual void logResults();

  virtual void createCorruptor(const VisIter& vi, 
			       const Record& simpar, 
			       const Int nSim);
protected:

  // D has two Complex parameters
  virtual Int nPar() { return 2; };

  // Jones matrix elements are trivial?
  //  True if GenLinear, False if General
  virtual Bool trivialJonesElem() { return (jonesType()==Jones::GenLinear); };  
  // dD/dp are trivial
  virtual Bool trivialDJ() { return True; };

  // Non-trivial Jones matrix calculation
  virtual void calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk,
			    const Vector<Complex>& par, const Vector<Bool>& pOk);

  // Initialize trivial dJs
  virtual void initTrivDJ();

private:

  Int solvePol_;
  DJonesCorruptor *dcorruptor_p;
  

  
};

// **********************************************************
//  DfJones (freq-dep D)  (general)
//

class DfJones : public DJones {
public:

  // Constructor
  DfJones(VisSet& vs);
  DfJones(String msname,Int MSnAnt,Int MSnSpw);
  DfJones(const Int& nAnt);

  virtual ~DfJones();

  // Return type name as string
  virtual String typeName()     { return "Dfgen Jones"; };
  virtual String longTypeName() { return "Dfgen Jones (frequency-dependent instrumental polarization"; };

  // This is the freq-dep version of D 
  //   (this is the ONLY fundamental difference from D)
  virtual Bool freqDepPar() { return True; };
  
};



// **********************************************************
//  DlinJones   (linearized DJones)
//

class DlinJones : public DJones {
public:

  // Constructor
  DlinJones(VisSet& vs);
  DlinJones(String msname,Int MSnAnt,Int MSnSpw);
  DlinJones(const Int& nAnt);

  virtual ~DlinJones();

  // Return type name as string
  virtual String typeName()     { return "D Jones"; };
  virtual String longTypeName() { return "D Jones (instrumental polarization"; };

  // Type of Jones matrix according to nPar()
  //  Do linearized matrix algebra
  virtual Jones::JonesType jonesType() { return Jones::GenLinear; };

};

// **********************************************************
//  DflinJones (freq-dep, linearized DJones)
//

class DflinJones : public DlinJones {
public:

  // Constructor
  DflinJones(VisSet& vs);
  DflinJones(String msname,Int MSnAnt,Int MSnSpw);
  DflinJones(const Int& nAnt);

  virtual ~DflinJones();

  // Return type name as string
  virtual String typeName()     { return "Df Jones"; };
  virtual String longTypeName() { return "Df Jones (frequency-dependent instrumental polarization"; };

  // This is the freq-dep version of D 
  //   (this is the ONLY fundamental difference from D)
  virtual Bool freqDepPar() { return True; };

};

// **********************************************************
//  DllsJones   (LLS DJones solve, General apply)
//

class DllsJones : public DJones {
public:

  // Constructor
  DllsJones(VisSet& vs);
  DllsJones(String msname,Int MSnAnt,Int MSnSpw);
  DllsJones(const Int& nAnt);

  virtual ~DllsJones();

  // Return type name as string
  virtual String typeName()     { return "DLLS Jones"; };
  virtual String longTypeName() { return "DLLS Jones (instrumental polarization)"; };

  // Type of Jones matrix 
  virtual Jones::JonesType jonesType() { return Jones::General; };

  // Dlin now uses generic gather, but solves for itself per solution
  virtual Bool useGenericGatherForSolve() { return True; };
  virtual Bool useGenericSolveOne() { return False; }

protected:

  // Local implementation of selfSolveOne (generalized signature)
  //   call solveOneVB with the first (and only?) VB
  virtual void selfSolveOne(VisBuffGroupAcc& vbga) { this->solveOneVB(vbga(0)); };

  // LSQ Solver for 1 VB
  virtual void solveOneVB(const VisBuffer& vb);


};

// **********************************************************
//  DfllsJones (freq-dep, LLS DJones solve, General apply)
//

class DfllsJones : public DllsJones {
public:

  // Constructor
  DfllsJones(VisSet& vs);
  DfllsJones(String msname,Int MSnAnt,Int MSnSpw);
  DfllsJones(const Int& nAnt);

  virtual ~DfllsJones();

  // Return type name as string
  virtual String typeName()     { return "DfLLS Jones"; };
  virtual String longTypeName() { return "DfLLS Jones (frequency-dependent instrumental polarization"; };

  // This is the freq-dep version of D 
  //   (this is the ONLY fundamental difference from D)
  virtual Bool freqDepPar() { return True; };

};


// **********************************************************
//  X: position angle calibration (for circulars!)
//    (rendered as a Mueller for now)

class XMueller : public SolvableVisMueller {
public:

  // Constructor
  XMueller(VisSet& vs);
  XMueller(String msname,Int MSnAnt,Int MSnSpw);
  XMueller(const Int& nAnt);

  virtual ~XMueller();

  // Return the type enum
  virtual Type type() { return VisCal::X; };

  // Return type name as string
  virtual String typeName()     { return "X Mueller"; };
  virtual String longTypeName() { return "X Mueller (baseline-based)"; };

  // Type of Jones matrix according to nPar()
  virtual Mueller::MuellerType muellerType() { return Mueller::Diagonal; };

  // Local setApply
  using SolvableVisCal::setApply;
  virtual void setApply(const Record& apply);

  // Local setSolve
  using SolvableVisCal::setSolve;
  void setSolve(const Record& solvepar);

  // Turn off normalization by model....
  virtual Bool normalizable() { return False; };

  // X gathers/solves for itself
  virtual Bool useGenericGatherForSolve() { return False; };

  // X gathers/solves for itself 
  virtual void selfGatherAndSolve(VisSet& vs, VisEquation& ve) { newselfSolve(vs,ve); };
  virtual void newselfSolve(VisSet& vs, VisEquation& ve);  // new supports combine


protected:

  // X has just 1 complex parameter, storing a phase
  virtual Int nPar() { return 1; };

  // Jones matrix elements are trivial
  virtual Bool trivialMuellerElem() { return False; };

  // Calculate the X matrix for all ants
  virtual void calcAllMueller();

  // Solve in one VB for the position angle
  virtual void solveOneVB(const VisBuffer& vb);

private:

  // <nothing>

};



// **********************************************************
//  X: position angle calibration (for circulars!)
//
class XJones : public SolvableVisJones {
public:

  // Constructor
  XJones(VisSet& vs);
  XJones(String msname,Int MSnAnt,Int MSnSpw);
  XJones(const Int& nAnt);

  virtual ~XJones();

  // Return the type enum
  virtual Type type() { return VisCal::X; };

  // Return type name as string
  virtual String typeName()     { return "X Jones"; };
  virtual String longTypeName() { return "X Jones (antenna-based)"; };

  // Type of Jones matrix according to nPar()
  virtual Jones::JonesType jonesType() { return Jones::Diagonal; };

  // Local setApply
  using SolvableVisCal::setApply;
  virtual void setApply(const Record& apply);

  // Local setSolve
  using SolvableVisCal::setSolve;
  void setSolve(const Record& solvepar);

  // X is normalizable by the model
  virtual Bool normalizable() { return True; };

  // X generically gathers, but solves for itself per solution
  virtual Bool useGenericGatherForSolve() { return True; };
  virtual Bool useGenericSolveOne() { return False; }

  // X gathers/solves for itself 
  virtual void selfGatherAndSolve(VisSet& vs, VisEquation& ve) { newselfSolve(vs,ve); };
  virtual void newselfSolve(VisSet& vs, VisEquation& ve);  // new supports combine

  // When genericall gathering, solve using first VB only in VBGA
  virtual void selfSolveOne(VisBuffGroupAcc& vbga) { this->solveOneVB(vbga(0)); };


protected:

  // X has just 1 complex parameter, storing a phase
  virtual Int nPar() { return 1; };

  // Jones matrix elements are trivial
  virtual Bool trivialJonesElem() { return False; };

  // Calculate the X matrix for all ants
  virtual void calcAllJones();

  // Solve in one VB for the position angle
  virtual void solveOneVB(const VisBuffer& vb);

private:

  // <nothing>

};


// **********************************************************
//  Xf: position angle calibration (for circulars!)
//     (channel-dependent)
class XfJones : public XJones {
public:

  // Constructor
  XfJones(VisSet& vs);
  XfJones(String msname,Int MSnAnt,Int MSnSpw);
  XfJones(const Int& nAnt);

  virtual ~XfJones();

  // Return the type enum
  virtual Type type() { return VisCal::X; };

  // Return type name as string
  virtual String typeName()     { return "Xf Jones"; };
  virtual String longTypeName() { return "Xf Jones (antenna-based)"; };

  // This is the freq-dep version of X 
  //   (this is the ONLY fundamental difference from X)
  virtual Bool freqDepPar() { return True; };

protected:

  // Use nchan>=1 shaping
  //  (TBD: this should be generalized!)
  void initSolvePar();


};


// X-Y phase 
class GlinXphJones : public GJones {
public:

  // Constructor
  GlinXphJones(VisSet& vs);
  GlinXphJones(String msname,Int MSnAnt,Int MSnSpw);
  GlinXphJones(const Int& nAnt);

  virtual ~GlinXphJones();

  // Return the type enum  (position UPSTREAM of D, not downstream like G)
  virtual Type type() { return VisCal::X; };

  // Return type name as string
  virtual String typeName()     { return "GlinXph Jones"; };
  virtual String longTypeName() { return "GlinXph Jones (X-Y phase)"; };

  // Local setapply  (unsets calWt)
  using SolvableVisJones::setApply;
  virtual void setApply(const Record& apply);

  // NOT FreqDep
  virtual Bool freqDepPar() { return False; };

  // Though derived from GJones, this type actually uses the cross-hands
  virtual Bool phandonly() { return False; };

  // GlinXphJones specialization
  virtual Bool useGenericGatherForSolve() { return True; }; 
  virtual Bool useGenericSolveOne() { return False; }

  //  virtual Bool useGenericGatherForSolve() { return False; }; 
  virtual void selfGatherAndSolve(VisSet& vs, VisEquation& ve);

protected:

  // Handle trivial vbga generated by generic gather-for-solve
  virtual void selfSolveOne(VisBuffGroupAcc& vbga);

  // Solver for one VB, that collapses baselines and cross-hands first,
  //  then solves for XY-phase and QU
  virtual void solveOneVB(const VisBuffer& vb);

  // Write QU info into table keywords
  virtual void globalPostSolveTinker();

  // Derived QU_ info
  Matrix<Float> QU_;


};


// Freq-dep XY-phase
class GlinXphfJones : public GlinXphJones {
public:

  // Constructor
  GlinXphfJones(VisSet& vs);
  GlinXphfJones(String msname,Int MSnAnt,Int MSnSpw);
  GlinXphfJones(const Int& nAnt);

  virtual ~GlinXphfJones();
  // Return type name as string
  // Return type name as string
  virtual String typeName()     { return "GlinXphf Jones"; };
  virtual String longTypeName() { return "GlinXphf Jones (X-Y phase)"; };

  // This is the freq-dep version of GlinXphJones
  //   (this is the ONLY fundamental difference from GlinXphJones)
  virtual Bool freqDepPar() { return True; };


};




} //# NAMESPACE CASA - END

#endif


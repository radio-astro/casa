//# FringeJones.h: Declaration of fringe-fitting VisCal
//# Copyright (C) 1996,1997,2000,2001,2002,2003,2011,2016
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

#ifndef SYNTHESIS_FRINGEJONES_H
#define SYNTHESIS_FRINGEJONES_H

#include <casa/aips.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h> 

namespace casa { //# NAMESPACE CASA - BEGIN

// Fringe-fitting (parametrized phase) VisCal
class FringeJones : public SolvableVisJones {
public:

  // Constructor
  //  TBD:  MSMetaInfoForCal-aware version; deprecate older ones
  FringeJones(VisSet& vs);
  FringeJones(String msname,Int MSnAnt,Int MSnSpw);
  FringeJones(const MSMetaInfoForCal& msmc);
  FringeJones(const Int& nAnt);

  virtual ~FringeJones();

  // We have Float parameters
  virtual VisCalEnum::VCParType parType() { return VisCalEnum::REAL; };

  // Return the type enum
  virtual Type type() { return VisCal::K; };

  // Return type name as string
  virtual String typeName()     { return "Fringe Jones"; };
  virtual String longTypeName() { return "Fringe Jones (parametrized phase)"; };

  // Type of Jones matrix according to nPar()
  virtual Jones::JonesType jonesType() { return Jones::Diagonal; };

  virtual Bool timeDepMat() { return True; };

  // Freq dependence (delays)
  virtual Bool freqDepPar() { return False; };
  virtual Bool freqDepMat() { return True; };

  // Local setApply to enforce calWt=F for delays
  virtual void setApply(const Record& apply);
  using SolvableVisJones::setApply;
  virtual void setCallib(const Record& callib,
			 const MeasurementSet& selms);

  // Local setSolve (traps lack of refant)
  virtual void setSolve(const Record& solve);
  using SolvableVisJones::setSolve;

  // This type is not yet accumulatable
  //  Deprecate?
  virtual Bool accumulatable() { return False; };

  // This type is smoothable
  //  TBD?
  virtual Bool smoothable() { return True; };

  // Delay to phase calculator
  virtual void calcAllJones();

  // Hazard a guess at parameters (unneeded here)
  //  TBD?  Needed?
  virtual void guessPar(VisBuffer& ) {};

  // K now uses generic gather, but solves for itself per solution
  virtual Bool useGenericGatherForSolve() { return True; };
  virtual Bool useGenericSolveOne() { return False; }

  // Override G here; nothing to do for K, for now
  //   TBD: refant apply, etc.
  virtual void globalPostSolveTinker() {};


protected:

  // phase, delay, rate
  //  TBD:  Need to cater for parameter opt-out  (e.g., no rate solve, etc.)
  virtual Int nPar() { return 6; };

  // Jones matrix elements are NOT trivial
  virtual Bool trivialJonesElem() { return False; };

  // dJ/dp are trivial
  //  TBD: make this default in SVC?
  virtual Bool trivialDJ() { return False; };

  // Initialize trivial dJs
  //  TBD: make this default in SVC?
  virtual void initTrivDJ() {};

  // Local implementation of selfSolveOne (generalized signature)
  //  **TBD:  migration to SolveDataBuffer**
  virtual void selfSolveOne(VisBuffGroupAcc& vbga);

  // Reference frequencies
  Vector<Double> KrefFreqs_;

private:

  
};

} //# NAMESPACE CASA - END

#endif

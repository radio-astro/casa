//# AMueller.h:  Additive closure errors
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

#ifndef SYNTHESIS_AMUELLER_H
#define SYNTHESIS_AMUELLER_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/CalCorruptor.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//forward
//class ANoiseCorruptor;

class AMueller : public MMueller {
public:

  // Constructor
  AMueller(VisSet& vs);
  AMueller(const Int& nAnt);

  virtual ~AMueller();

  // Return the type enum
  virtual Type type() { return VisCal::A; };

  // Return type name as string
  virtual String typeName()     { return "A Mueller"; };
  virtual String longTypeName() { return "A Mueller (baseline-based)"; };

  // Algebraic type of Mueller matrix 
  //  (this is the key distinguishing characteristic)
  virtual Mueller::MuellerType muellerType() { return Mueller::AddDiag2; };

  // We do not normalize by the model, since we are estimating
  //  directly from the data  (we should optimize here by avoiding 
  //  the model I/O)
  virtual Bool normalizable() {return False;};

  // Specialize corrupt to pre-zero model for corruption
  virtual void corrupt(VisBuffer& vb);
  using VisMueller::corrupt;

};



// Additive noise
// In practice, this is not really solvable, but it
//   is a SVM because we need access to general simulation methods
class ANoise : public SolvableVisMueller {
public:

  // Constructor
  ANoise(VisSet& vs);
  ANoise(const Int& nAnt);

  virtual ~ANoise();

  // Return the type enum
  virtual Type type() { return VisCal::ANoise; };

  // Return type name as string
  virtual String typeName()     { return "A Noise"; };
  virtual String longTypeName() { return "A Noise (baseline-based)"; };

  // Algebraic type of Mueller matrix 
  //  (this is the key distinguishing characteristic)
  virtual Mueller::MuellerType muellerType() { return Mueller::AddDiag2; };

  // Overide solvability
  virtual Bool isSolvable() { return False; };

  // Insert noise generation method (per VisBuffer?) here, and
  //   implement it in AMueller.cc
  //   Is this simPar?

  virtual Bool simPar(VisIter& vi, const Int nChunks);

  // Also need setSimPar, etc., I think
  // use SVC::setSimulate for now
  
  Int setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes);

protected:
  // umm... 2 like an M, for each of parallel hands?
  virtual Int nPar() { return 2; };

  // Jones matrix elements are trivial
  virtual Bool trivialMuellerElem() { return True; };

private:
  ANoiseCorruptor *acorruptor_p;

};

} //# NAMESPACE CASA - END

#endif

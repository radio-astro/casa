//# VisJones.h: Definitions of interface for VisJones 
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

#ifndef SYNTHESIS_VISJONES_H
#define SYNTHESIS_VISJONES_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <casa/BasicSL/Constants.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <msvis/MSVis/VisSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Need forward declaration for Solve in the Jones Matrices
class VisEquation;

// <summary> 
// VisJones: Model multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
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
// VisJones describes an interface for multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. It is an Abstract Base Class: most methods
// must be defined in derived classes.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The following examples illustrate how an VisJones can be
// used:
// <ul>
// <li> Simulation: random multiplicative gains of a certain scaling
// can be applied: <linkto class="SimVisJones">SimVisJones</linkto>
// <li> Time variable multiplicative errors: <linkto class="TimeVarVisJones">TimeVarVisJones</linkto>
// <li> Solvable time variable multiplicatiove errors: <linkto class="SolvableVisJones">SolvableVisJones</linkto>
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
//      // Make an VisJones
//      PhaseScreenVisJones coac(vs);
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

typedef SquareMatrix<Complex,2> mjJones2;
typedef SquareMatrix<Complex,4> mjJones4;
typedef SquareMatrix<Float,2> mjJones2F;

class VisJones : public Iterate {
public:

  // Allowed types of VisJones matrices
  enum Type{UVMOD,Mf,M,K,B,G,D,C,E,P,T,EP,F};

  VisJones() {};
  
  virtual ~VisJones() {}

  // Set the solver parameters
  virtual void setSolver(const Record& solver) {};

  // Set the interpolation parameters
  virtual void setInterpolation(const Record& interpolation) {};
  
  // Apply actual Jones matrix and adjoint
  virtual VisBuffer& apply(VisBuffer& vb) = 0;
  virtual VisBuffer& applyInv(VisBuffer& vb) = 0;
 
  // Return the type of this Jones matrix (actual type of derived class).
  virtual Type type() = 0;

  // Frequency-dependent ?
  virtual Bool freqDep() = 0;

  // Is this solveable?
  virtual Bool isSolveable() {return False;};

  // Is this Jones matrix to be solved for, applied or both ?
  virtual Bool isSolved() {return False;};
  virtual Bool isApplied() {return False;};

  // Return the time interval over which this Jones matrix is constant
  virtual Double interval() {return DBL_MAX;}

  // Return the pre-averaging interval (applies only to solveable components)
  virtual Double preavg() {return DBL_MAX;}

  // Return the pre-normalization flag (applies only to solveable components)
  virtual Bool prenorm() {return False;};

  // Return the reference antenna if this component is solved for.
  virtual Int refant() {return -1;}

  // Apply Gradient
  virtual VisBuffer& applyGradient(VisBuffer& vb, 
				   const Vector<Int>& antenna, 
				   Int i, Int j, Int pos) = 0;

  // Initialize for gradient search
  virtual void initializeGradients() {};
  virtual void finalizeGradients()   {};
 
  // Add to Gradient Chisq
  virtual void addGradients(const VisBuffer& vb, Int row, const Antenna& a, 
			    const Vector<Float>& sumwt, 
			    const Vector<Float>& chisq,
                            const Vector<mjJones2>& c, 
			    const Vector<mjJones2F>& f) = 0;
 
  // Solve
  virtual Bool solve (VisEquation& ve) = 0;
 
protected:

  virtual String typeName() {
    switch(type()) {
    case VisJones::K: 
      return "K Jones";
    case VisJones::G: 
      return "G Jones";
    case VisJones::B: 
      return "B Jones";
    case VisJones::D: 
      return "D Jones";
    case VisJones::C: 
      return "C Jones";
    case VisJones::E: 
      return "E Jones";
    case VisJones::P: 
      return "P Jones";
    case VisJones::T: 
      return "T Jones";
    case VisJones::F:
      return "F Jones";
    default:
      return "Unknown Jones";
    }
  };

private:
  
};

} //# NAMESPACE CASA - END

#endif

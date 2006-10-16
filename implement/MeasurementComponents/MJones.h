//# MJones.h: Definitions of interface for MJones 
//# Copyright (C) 1996,1997,2000
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

#ifndef SYNTHESIS_MJONES_H
#define SYNTHESIS_MJONES_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <msvis/MSVis/VisSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Need forward declaration for Solve in the Jones Matrices
class VisEquation;

// <summary> 
// MJones: Model multiplicative errors for the <linkto class="VisEquation">VisEquation</linkto>
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
// </prerequisite>
//
// <etymology>
// MJones describes an interface for multiplicative errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. It is an Abstract Base Class: most methods
// must be defined in derived classes.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The following examples illustrate how an MJones can be
// used:
// <ul>
// <li> Time variable multiplicative errors: <linkto class="TimeVarMJones">TimeVarMJones</linkto>
// <li> Solvable time variable multiplicatiove errors: <linkto class="SolvableMJones">SolvableMJones</linkto>
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
//      // Make an MJones
//      CorrelatorMJones coac(vs);
//
//      // Correct the visset
//      ve.correct();
// </srcblock>
// </example>
//
// <motivation>
// The properties of a multiplicative component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>


class MJones : public Iterate {
public:

  MJones() {};
  
  // Apply actual Jones matrix and inverse
  // <group>
  virtual VisBuffer& apply(VisBuffer& vb) 
  {return vb;};
  virtual VisBuffer& applyInv(VisBuffer& vb) 
  {return vb;};
  // </group>
  
  // Is this solveable?
  virtual Bool isSolveable() {return False;};

  // Initialize for gradient search
  virtual void initializeGradients() {};
  virtual void finalizeGradients()   {};
 
  // Add to Gradient Chisq
  virtual void addGradients(const VisBuffer& vb, Int row, 
			    const Antenna& a1, const Antenna& a2,
			    const Vector<Float>& sumwt, 
			    const Vector<Float>& chisq,
                            const Cube<Complex>& c, const Cube<Float>& f) = 0;
 
  // Solve
  virtual Bool solve (VisEquation& ve) = 0;
 
protected:

  virtual String typeName() {
    return "M Jones";
  };
private:
};
 

} //# NAMESPACE CASA - END

#endif

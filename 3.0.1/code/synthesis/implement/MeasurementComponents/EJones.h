//# StandardVisCal.h: Declaration of standard (Solvable)VisCal types
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

#ifndef SYNTHESIS_EJONES_H
#define SYNTHESIS_EJONES_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declaration
class VisEquation;

// **********************************************************
//  EGainCurve
//

class EGainCurve : public SolvableVisJones {
public:

  // Constructor
  EGainCurve(VisSet& vs);
  //  EGainCurve(const Int& nAnt);  // NYI

  virtual ~EGainCurve();

  // Return the type enum
  virtual Type type() { return VisCal::E; };

  // Return type name as string
  virtual String typeName()     { return "EGainCurve"; };
  virtual String longTypeName() { return "EGainCurve (Gain(elev) corrections)"; };

  // Type of Jones matrix (
  Jones::JonesType jonesType() { return Jones::Diagonal; };

  // Local specialization of setApply to extract gain curves from table
  void setApply(const Record& applypar);

  // Guess (throws error because we don't yet solve for this)
  virtual void guessPar(VisBuffer& vb);

protected:

  // EGainCurve has eight Float pars per ant (4 per pol)
  virtual Int nPar() { return 8; };

  // Jones matrix elements are NOT trivial
  virtual Bool trivialJonesElem() { return False; };

  // Access to z.a. data
  Vector<Double>& za() { return za_; };

  // Calculate parameters (in this case, the z.a.)
  virtual void calcPar();

  // Calculate the EGainCurve matrix for all ants
  virtual void calcAllJones();

private:

  // avoid compiler warnings
  using SolvableVisJones::setApply;  

  // The observation start time (for selecting gain curves)
  Double obstime_;

  // The ant name list (for selecting gain curves)
  Vector<String> antnames_;

  // The spw ref frequencies (for selecting gain curves)
  Vector<Double> spwfreqs_;

  Vector<Double> za_;
  
};


// **********************************************************
//  EPJones (pointing errors)
//
/*
class EPJones : public SolvableVisJones {
public:

  // Constructor
  EPJones(VisSet& vs);
  //   EPJones(const Int& nAnt);  // NYI

  virtual ~EPJones();

  // Return the type enum
  virtual Type type() { return VisCal::E; };

  // Return type name as string
  virtual String typeName()     { return "EP Jones"; };
  virtual String longTypeName() { return "EP Jones (pointing errors)"; };

  // Type of Jones matrix according to nPar()
  Jones::JonesType jonesType() { return Jones::Diagonal; };

  // Specialized access to pointing parameters (no chan axis)
  Matrix<Float>& pointPar();

  // Arrange to apply (corrupt only)
  using SolvableVisCal::setApply;
  virtual void setApply(const Record& applypar);

  // Apply calibration to a VisBuffer 
  virtual void applyCal(VisBuffer& vb, 
			Cube<Complex>& Mout);

  // Differentiate a VisBuffer w.r.t. pointng parameters
  virtual void differentiate(VisBuffer& vb,
			     Cube<Complex>& Mout,
			     Array<Complex>& dMout,
			     Matrix<Bool>& Mflg);

protected:

  // EP has two real parameters
  virtual Int nPar() { return 2; };

  // Jones matrix elements are NOT trivial
  virtual Bool trivialJonesElem() { return False; };

private:

  // Local Matrix for referencing pointing pars in a convenient way
  Matrix<Float> pointPar_;

};
*/




} //# NAMESPACE CASA - END

#endif


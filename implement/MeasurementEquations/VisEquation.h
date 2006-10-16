//# VisEquation.h: Interface definition for Vis Equation
//# Copyright (C) 1996,1997,1999,2000,2001,2002,2003
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

#ifndef SYNTHESIS_VISEQUATION_H
#define SYNTHESIS_VISEQUATION_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <synthesis/MeasurementComponents/VisJones.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/XCorr.h>
#include <synthesis/MeasurementComponents/MJones.h>
#include <synthesis/MeasurementComponents/ACoh.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// Visibility Measurement Equation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
// </prerequisite>
//
// <etymology>
// Visibility Equation describes a model for measurements from a
// generic synthesis telescope
// </etymology>
//
// <synopsis> 
// This is responsible for the Measurement Equation of the Generic
// Interferometer due to Hamaker, Bregman and Sault and later extended
// by Noordam, and Cornwell.
//
// See <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// for more details of the form of the VisEquation.
//
// The following components can be plugged into VisEquation
// <ul>
// <li> Antenna-based terms: <linkto class="VisJones">VisJones</linkto>
// <li> Correlator-based multiplicative terms: <linkto class="MJones">MJones</linkto>
// <li> Correlator-based additive terms: <linkto class="ACoh">ACoh</linkto>
// <li> Non-linear functions: <linkto class="XCorr">XCorr</linkto>
// </ul>
// </synopsis> 
//
// <example>
// <srcblock>
//
//      // Read the VisSet from disk
//      VisSet vs("3c84.MS");
//
//      // Define a VisEquation
//      VisEquation ve(vs);
//
//      // Solve for calibration of G matrix every 5 minutes
//      GJones gj(vs, 5*60);
//
//      ve.setVisJones(gj);
//
//      ve.solve(gj);
//
//      // Solve for calibration of D matrix every 12 hours
//      DJones dj(vs, 12*60*60);
//
//      ve.setVisJones(dj);
//
//      ve.solve(dj);
//
// </srcblock>
// </example>
//
// <motivation>
// VisEquation is part of a framework of classes that are
// designed for synthesis and single dish imaging. The others are the 
// <linkto module=MeasurementComponents>MeasurementComponents</linkto>.
// </motivation>
//
// <todo asof="">
// </todo>

class VisEquation {
public:

  VisEquation(VisSet& vs);
  
  virtual ~VisEquation();
  
  VisEquation(const VisEquation& other);

  VisEquation& operator=(const VisEquation& other);

  void setVisSet(VisSet& vs);

  void setVisJones(VisJones& j);
  void unSetVisJones(VisJones& j);

  void setVisCal(VisCal& vc);
  void unSetVisCal(VisCal& vc);

  void setMJones(MJones& j);

  // Set Additive error coherence vector
  void setACoh(ACoh& c);

  // Set non-linear correlator function
  void setXCorr( XCorr& j);

  // Correct the measured "Vis" coherences to obtain an
  // estimate of the "Sky" visibilities
  virtual void correct();

  // Corrupt the measred "Vis" coherences to simulate
  // errors
  virtual void corrupt();

  // Initialize the gradient calculations, pre-computes & averages
  // lhs and rhs of the equation.
  void initChiSquare(VisJones& vj);

  void initChiSquare(SolvableVisCal& svc);

  void initChiSquare(const Int& type,
		     const Bool& freqdep,
		     const Bool& prenorm=False,
		     const Double& preavg=DBL_MAX);
  
  // Find sum of weights, Chi-squared per spectral window, and antenna
  virtual void chiSquared(Matrix<Float>& iChisq, Matrix<Float>& iSumwt);

  // Find sum of weights, Chi-squared per antenna
  virtual void chiSquared(Vector<Float>& iChisq, Vector<Float>& iSumwt);

  // VisJones: Find sum of weights, Chi-squared, and the first and second derivatives
  virtual void gradientsChiSquared(const Matrix<Bool>& required, VisJones& vj);

  // MJones: Find sum of weights, Chi-squared, and the first and second derivatives
  virtual void gradientsChiSquared(MJones& mj);

  // Solve for variables. 
  // <group>
  virtual Bool solve(MJones& mj);
  virtual Bool solve(ACoh& ac);
  virtual Bool solve(VisJones& vj);

  virtual Bool solve(SolvableVisCal& svc);

  // </group>

  // Accessor functions for the corrected and corrupted data
  const VisBuffer& corrected() {return correctedvb;};
  const VisBuffer& corrupted() {return corruptedvb;};

  // Apply actual Jones matrix and adjoint
  // <group>
  virtual VisBuffer& apply(VisBuffer& result);
  virtual VisBuffer& applyInv(VisBuffer& result);
  // </group>
 
protected:

  Bool ok();
 
  // Fix up the weights for the VisBuffer (use sigmas)
  void fixWeights(VisBuffer& vb);

  // Apply Gradient of VisJones
  virtual VisBuffer& applyGradient(VisBuffer& result, 
				   const Vector<Int>& antenna, Int i, Int j, 
				   Int pos);

  // Apply Gradient of MJones
  virtual VisBuffer& applyGradient(VisBuffer& result);

  // VisSet
  VisSet* vs_;
  
  // List of terms in left to right order
  // <group>
  XCorr* xc_;

  VisCal* mfm_;   // Freq-dep multiplicative closure
  VisCal* mm_;    // Multiplicative closure
  VisCal* km_;    // Fringe-fitting

  MJones* mj_;
  VisJones* kj_;
  VisJones* bj_;
  VisJones* gj_;
  VisJones* dj_;
  VisJones* cj_;
  VisJones* ej_;
  VisJones* pj_;
  VisJones* tj_;
  ACoh* ac_;
  // </group>

  // Buffers for gradient calculation
  VisBuffer corruptedvb,correctedvb;

  // VisJones still to be applied in gradient calculation
  VisJones* vj_;

  // VisCal with solving interface
  SolvableVisCal* svc_;

};

} //# NAMESPACE CASA - END

#endif

//# GJonesPoly.h: Electronic gain Jones matrix as polynomials over time
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
//# $Id: GJonesPoly.h,v 19.7 2004/11/30 17:50:48 ddebonis Exp $

#ifndef SYNTHESIS_GSPLINE_H
#define SYNTHESIS_GSPLINE_H

#include <synthesis/MeasurementComponents/SolvableVisCal.h>
#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/CalTables/GJonesMBuf.h>
#include <casa/Containers/SimOrdMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Electronic gain Jones matrix as spline polynomial over time
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="GJones">GJones</linkto> module
//   <li> <linkto module="GJonesPoly">GJonesPoly</linkto> module
// </prerequisite>
//
// <etymology>
// GJonesSpline describes the electronic gain visibility Jones matrix 
// parametrized as a spline polynomial over time.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of GJonesSpline are only non-zero on the diagonal.
//
// </synopsis> 
//
// <motivation>
// Support electronic gain calibration using splines polynomials over time.
// </motivation>
//
// <todo asof="02/02/01">
// i) Support solution and pre-averaging intervals
// </todo>

class VisEquation;

class GJonesSpline : public GJones {
public:

  // Construct from a visibility data set
  GJonesSpline (VisSet& vs);

  // Destructor
  virtual ~GJonesSpline();

  // Return the type enum
  virtual Type type() { return VisCal::G; };

  // Return type name as a string
  virtual String typeName()     { return "GSPLINE"; };
  virtual String longTypeName() { return "G Jones SPLINE (elec. gain)"; };


  // GSpline gathers/solves for itself
  virtual Bool useGenericGatherForSolve() { return False; };

  // Type of Jones matrix according to nPar()
  // TBD:
  virtual Jones::JonesType jonesType() { return Jones::Diagonal; };

  // Set the solver parameters
  using GJones::setSolve;
  virtual void setSolve(const Record& solvepar);

  // Set the interpolation parameters
  using GJones::setApply;
  virtual void setApply(const Record& applypar);

  // Solve
  virtual void selfGatherAndSolve (VisSet& vs, VisEquation& ve);

/*
  // Set raw phase transfer from another spw
  void setRawPhaseVisSet(VisSet& rawvs_p);

  // Set parameters for phase wrapping resolution
  void setPhaseWrapHelp(const Int& numpoi, const Double& phaseWrap);
*/

 protected:

  // GSPLINE has one trivial Complex parameter (single pol, for now)
  // TBD:
  virtual Int nPar() { return 2; };

  // Calc G pars from spline info
  virtual void calcPar();

 private:

  // The underlying VisSet
  VisSet* vs_p;

  // Private variables containing the solver parameters
  Bool solveAmp_p, solvePhase_p;
  Double splinetime_p;

  // Time for which the current calibration cache is valid
  Double cacheTimeValid_p;

  // Ptr to a calibration buffer associated with the calibration
  // solutions which are to be interpolated and applied to the data
  GJonesSplineMBuf* calBuffer_p;


  // Parameters for raw phase removal from another spw
  VisSet* rawvs_p;
  Bool rawPhaseRemoval_p;
  SimpleOrderedMap<String, Int> timeValueMap_p;
  Matrix<Double> rawPhase_p;

  // Solution timestamp
  Double solTimeStamp_p;


  // Create and fill an empty output calibration buffer
  void newCalBuffer (const Vector<Int>& fieldIdKeys, 
		     const Vector<Int>& antennaId);

  // Compute the number of spline knots required and their location
  Int getKnots (const Vector<Double>& times, Vector<Double>& knots);

  // Update the output calibration table
  virtual void updateCalTable (const Vector<Int>& fieldIdKeys,
			       const Vector<Int>& antennaId,
			       const Vector<String>& freqGrpName,
			       const Vector<String>& polyType,
			       const Vector<String>& polyMode,
			       const Vector<Complex>& scaleFactor,
			       const Matrix<Double>& polyCoeffAmp,
			       const Matrix<Double>& polyCoeffPhase,
			       const Vector<String>& phaseUnits,
			       const Vector<Double>& splineKnotsAmp,
			       const Vector<Double>& splineKnotsPhase,
			       const Vector<MFrequency>& refFreq,
			       const Vector<Int>& refAnt);
  
  // Compute a spline polynomial value
  Double getSplineVal (Double x, Vector<Double>& knots,
		       Vector<Double>& coeff);


/*
  // fill the raw phase buffer for usage
  void fillRawPhaseBuff();

  // return the rawphase of 
  Double getRawPhase(Int ant1, Int ant2, Double time);

*/

  //Plot solutions as compare with data
  void plotsolve(const Vector<Double>& x, 
		 const Matrix<Double>& yall, 
		 const Matrix<Double>& weightall, 
		 const Vector<Double>& errall, 
		 Matrix<Double>& coeff, Bool phasesoln);

  // Return all field id.'s in the underlying MS
  Vector<Int> fieldIdRange();

  //Logging solution and rms
  void writeAsciiLog(const String& filename, const Matrix<Double>& coeff, const Vector<Double>& rmsFit, Bool phasesoln);

  Int numpoint_p;
  Double phaseWrap_p;


};


} //# NAMESPACE CASA - END

#endif



//# TJonesPoly.h: Atmospheric gain Jones matrix as polynomials over time
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

#ifndef SYNTHESIS_TJONESPOLY_H
#define SYNTHESIS_TJONESPOLY_H

#include <synthesis/MeasurementComponents/SolvableVisJones.h>
#include <calibration/CalTables/TJonesMBuf.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Atmospheric gain Jones matrix as a polynomial over time
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="TJones">TJones</linkto> module
// </prerequisite>
//
// <etymology>
// TJonesPoly describes the atmospheric gain visibility Jones matrix 
// parametrized as a polynomial over time.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of TJonesSpline are only non-zero on the diagonal.
//
// </synopsis> 
//
// <motivation>
// Support atmospheric gain calibration using polynomials over time.
// </motivation>
//
// <todo asof="02/02/01">
// i) Splines supported at present; add general polynomial support
// </todo>

class TJonesPoly : public TJones {
public:

  // Construct from a visibility data set
  TJonesPoly (VisSet& vs) {};

  // Destructor
  virtual ~TJonesPoly() {};

  // Set the solver parameters
  virtual void setSolver (const Record& solver) {};

  // Set the interpolation parameters
  virtual void setInterpolation (const Record& interpolation) {};

  // Solve
  virtual Bool solve (VisEquation& me) {return True;};

 protected:

 private:

};

// <summary> Atmospheric gain Jones matrix as spline polynomial over time
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="TJones">TJones</linkto> module
//   <li> <linkto module="TJonesPoly">TJonesPoly</linkto> module
// </prerequisite>
//
// <etymology>
// TJonesSpline describes the atmospheric gain visibility Jones matrix 
// parametrized as a spline polynomial over time.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of TJonesSpline are only non-zero on the diagonal.
//
// </synopsis> 
//
// <motivation>
// Support atmospheric gain calibration using splines polynomials over time.
// </motivation>
//
// <todo asof="02/02/01">
// i) Support solution and pre-averaging intervals
// </todo>

class TJonesSpline : public TJones {
public:

  // Construct from a visibility data set
  TJonesSpline (VisSet& vs);

  // Destructor
  virtual ~TJonesSpline();

  // Set the solver parameters
  virtual void setSolver (const Record& solver);

  // Set the interpolation parameters
  virtual void setInterpolation (const Record& interpolation);

  // Solve
  virtual Bool solve (VisEquation& me);

 protected:
  // Check the validity of the antenna and baseline gain cache;
  // re-compute the spline polynomial corrections and refresh
  // the cache as necessary.
  virtual void checkCache(const VisBuffer& vb, Int spw, Double time);

 private:
  // Private variables containing the solver parameters
  String solveTable_p;
  Bool append_p;
  String mode_p;
  Bool solveAmp_p, solvePhase_p;
  Double interval_p, preavg_p;
  Int refant_p;

  // Private variables containing the interpolation parameters
  String applyTable_p, applySelect_p;
  Double applyInterval_p;

  // Ptr to a calibration buffer associated with the calibration
  // solutions which are to be interpolated and applied to the data
  TJonesSplineMBuf* calBuffer_p;

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

  //Plot solutions as compare with data
  void plotsolve(const Vector<Double>& x, 
		 const Matrix<Double>& yall, 
		 const Matrix<Double>& weightall, 
		 const Vector<Double>& errall, 
		 Matrix<Double>& coeff, Bool phasesoln);

  // Return all field id.'s in the underlying MS
  Vector<Int> fieldIdRange();
};


} //# NAMESPACE CASA - END

#endif

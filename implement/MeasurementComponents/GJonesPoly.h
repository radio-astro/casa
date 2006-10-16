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
//# $Id$

#ifndef SYNTHESIS_GJONESPOLY_H
#define SYNTHESIS_GJONESPOLY_H

#include <synthesis/MeasurementComponents/SolvableVisJones.h>
#include <calibration/CalTables/GJonesMBuf.h>
#include <casa/Containers/SimOrdMap.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Electronic gain Jones matrix as a polynomial over time
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="GJones">GJones</linkto> module
// </prerequisite>
//
// <etymology>
// GJonesPoly describes the electronic gain visibility Jones matrix 
// parametrized as a polynomial over time.
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
// Support electronic gain calibration using polynomials over time.
// </motivation>
//
// <todo asof="02/02/01">
// i) Splines supported at present; add general polynomial support
// </todo>

class GJonesPoly : public GJones {
public:

  // Construct from a visibility data set
  GJonesPoly (VisSet& vs) {};

  // Destructor
  virtual ~GJonesPoly() {};

  // Set the solver parameters
  virtual void setSolver (const Record& solver) {};

  // Set the interpolation parameters
  virtual void setInterpolation (const Record& interpolation) {};

  // Solve
  virtual Bool solve (VisEquation& me) {return True;};

 protected:

 private:

};

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

class GJonesSpline : public GJones {
public:

  // Construct from a visibility data set
  GJonesSpline (VisSet& vs);

  // Destructor
  virtual ~GJonesSpline();

  // Set the solver parameters
  virtual void setSolver (const Record& solver);

  // Set the interpolation parameters
  virtual void setInterpolation (const Record& interpolation);

  // Solve
  virtual Bool solve (VisEquation& me);

  // Set raw phase transfer from another spw
  void setRawPhaseVisSet(VisSet& rawvs_p);


  // Set parameters for phase wrapping resolution

  void setPhaseWrapHelp(const Int& numpoi, const Double& phaseWrap);

 protected:
  // Check the validity of the antenna and baseline gain cache;
  // re-compute the spline polynomial corrections and refresh
  // the cache as necessary.
  virtual void getThisGain(const VisBuffer& vb, 
			   const Bool& forceAntMat=True, 
			   const Bool& doInverse=False,
			   const Bool& forceIntMat=True);

 private:
  // Private variables containing the solver parameters
  String solveTable_p;
  Bool append_p;
  String mode_p;
  Bool solveAmp_p, solvePhase_p;
  Double interval_p, preavg_p, splinetime_p;
  Int refant_p;

  // Private variables containing the interpolation parameters
  String applyTable_p, applySelect_p;
  Double applyInterval_p;

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


  // fill the raw phase buffer for usage
  void fillRawPhaseBuff();

  // return the rawphase of 
  Double getRawPhase(Int ant1, Int ant2, Double time);

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



//# BJonesPoly.h: A bandpass visibility Jones matrix of polynomial form
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

#ifndef SYNTHESIS_BJONESPOLY_H
#define SYNTHESIS_BJONESPOLY_H

#include <synthesis/MeasurementComponents/SolvableVisJones.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> A bandpass visibility Jones matrix of polynomial form
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="BJones">BJones</linkto> module
// </prerequisite>
//
// <etymology>
// BJonesPoly describes a bandpass visibility Jones matrix of
// polynomial form.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of BJonesPoly are only non-zero on the diagonal.
//
// </synopsis> 
//
// <motivation>
// Support polynomial bandpass calibration.
// </motivation>
//
// <todo asof="02/02/01">
// i) Support solution intervals
// </todo>

class BJonesPoly : public BJones {
public:

  // Construct from a visibility data set
  BJonesPoly (VisSet& vs);

  // Destructor
  virtual ~BJonesPoly() {};

  // Set the solver parameters
  virtual void setSolver (const Record& solver);

  // Set the interpolation parameters
  virtual void setInterpolation (const Record& interpolation);

  // Return the pre-normalization flag 
  //  virtual Bool prenorm() {return visnorm_p;};  TURN OFF, FOR NOW (gmoellen 04Jan07)
  virtual Bool prenorm() {return False;};

  // Solve
  virtual Bool solve (VisEquation& me);

 protected:
  // Virtual function to check the bandpass correction cache validity
  // Null here to allow unmodified use of BJones::apply()/applyInv()
  virtual void checkCache (const VisBuffer& vb, const Int& spw, const Double& time) {};
  

 private:
  // Private variables containing the solver parameters
  String solveTable_p;
  Bool append_p;
  Double interval_p;
  Int degamp_p, degphase_p;
  Bool visnorm_p, bpnorm_p;
  Int maskcenter_p;
  Float maskedge_p;
  Bool phaseonly_p;
  Int refant_p;

  // Derived solver parameters
  // Center mask half width (in channels)
  Int maskcenterHalf_p;
  // Fractional edge mask
  Float maskedgeFrac_p;

  // Private variables containing the interpolation parameters
  String applyTable_p, applySelect_p;
  Double applyInterval_p;

  // Determine if a given channel is masked
  Bool maskedChannel (const Int& chan, const Int& nChan);

  // Update the output calibration table to include the
  // current solution parameters
  void updateCalTable (const String& freqGrpName, 
		       const Vector<Int>& antennaId,
		       const Vector<String>& polyType, 
		       const Vector<Complex>& scaleFactor,
		       const Matrix<Double>& validDomain,
		       const Matrix<Double>& polyCoeffAmp,
		       const Matrix<Double>& polyCoeffPhase,
		       const Vector<String>& phaseUnits,
		       const Vector<Complex>& sideBandRef,
		       const Vector<MFrequency>& refFreq, 
		       const Vector<Int>& refAnt);

  // Compute a Chebyshev polynomial using the CLIC library
  Double getChebVal (const Vector<Double>& coeff, const Double& xinit,
		     const Double& xfinal, const Double& x);

  // Load bandpass parameters from a calibration table and
  // pre-compute the corrections (and their inverse)
  void load (const String& applyTable);

  // Utility function to return the bandwidth-weighted average 
  // frequency for a set of spectral window id.'s
  Double meanFrequency (const Vector<Int>& spwids);

  // Utility function to return the frequency group name for a given spw. id.
  String freqGrpName (const Int& spwId);

  // Utility function to return the spw id.'s in a given freq. group
  Vector<Int> spwIdsInGroup (const String& freqGrpName);

  // Utility function to return the frequency axis for a given spw. id.
  Vector<Double> freqAxis (const Int& spwId);

  // Utility function to do a plot  baseline data (phase or amplitude and
  // compare with the solution of the polynomial fits  
  void plotsolve(const Vector<Double>& x, const Matrix<Double>& yall, 
		 const Matrix<Double>& weightall, 
		 const Vector<Double>& errall, Matrix<Double>& coeff, 
		 Bool phasesoln);

  void plotsolve2(const Vector<Double>& x, 
		  const Matrix<Double>& ampdata, 
		  const Matrix<Double>& phadata, 
		  const Matrix<Double>& wtdata, 
		  const Vector<Int>& ant1idx, const Vector<Int>& ant2idx, 
		  const Vector<Double>& amperr, Matrix<Double>& ampcoeff, 
		  const Vector<Double>& phaerr, Matrix<Double>& phacoeff) ;

};


} //# NAMESPACE CASA - END

#endif

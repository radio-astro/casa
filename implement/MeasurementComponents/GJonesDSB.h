//# GJonesDSB.h: A GJones matrix containing a sideband gain ratio
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#ifndef SYNTHESIS_GJONESDSB_H
#define SYNTHESIS_GJONESDSB_H

#include <synthesis/MeasurementComponents/SolvableVisJones.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> A GJones matrix containing a sideband gain ratio
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="GJones">GJones</linkto> module
// </prerequisite>
//
// <etymology>
// GJonesDSB describes an electronic visibility Jones matrix
// containing a sideband gain ratio.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The elements of GJonesDSB are only non-zero on the diagonal.
//
// </synopsis> 
//
// <motivation>
// Support sideband gain ratio corrections explicitly as a Jones matrix subtype
// </motivation>
//
// <todo asof="02/02/01">
// </todo>

class GJonesDSB : public GJones {
public:

  // Construct from a visibility data set
  GJonesDSB (VisSet& vs);

  // Destructor
  virtual ~GJonesDSB() {};

  // Set the solver parameters
  virtual void setSolver (const Record& solver);

  // Set the interpolation parameters
  virtual void setInterpolation (const Record& interpolation);

  // Solve
  virtual Bool solve (VisEquation& me);

 protected:
  // Virtual function to check the gain correction cache validity
  // Null here to allow unmodified use of TimeVarVisJones::apply()/applyInv()
  virtual void checkCache (const VisBuffer& vb, Int spw, Double time) {};
  

 private:
  // Private variables containing the solver parameters
  String solveTable_p;
  Bool append_p;
  Double interval_p;
  Int maskcenter_p;
  Float maskedge_p;
  String mode_p, refsideband_p;
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
  void updateCalTable (const Vector<String>& freqGrpName, 
		       const Vector<Int>& antennaId,
		       const Matrix<Complex>& sideBandRef,
		       const Vector<MFrequency>& refFreq, 
		       const Vector<Int>& refAnt);

  // Load the sideband gain rations from a calibration table and
  // cache the corrections (and their inverse)
  void load (const String& applyTable);

  // Utility function to return the bandwidth-weighted average 
  // frequency for a set of spectral window id.'s
  Double meanFrequency (const Vector<Int>& spwids);

  // Utility function to return the frequency group name for a given spw. id.
  String freqGrpName (const Int& spwId);

  // Utility function to return the frequency group id. associated with
  // a given frequency group name
  Int freqGrpId (const String& freqGrpName);

  // Utility to return the net sideband for a given spectral window id.
  Int sideBand (const Int& spwid);

  // Utility function to return the spw id.'s in a given freq. group
  Vector<Int> spwIdsInGroup (const String& freqGrpName);

  // Utility function to return the frequency axis for a given spw. id.
  Vector<Double> freqAxis (const Int& spwId);

};


} //# NAMESPACE CASA - END

#endif

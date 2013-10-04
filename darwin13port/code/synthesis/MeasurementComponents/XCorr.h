//# XCorr.h: Definitions of interface for XCorr 
//# Copyright (C) 1996,1997,1999,2000
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

#ifndef SYNTHESIS_XCORR_H
#define SYNTHESIS_XCORR_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <synthesis/MSVis/VisSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Model non-linear correlator function for the VisEquation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
// </prerequisite>
//
// <etymology>
// XCorr describes an interface for a non-linear function to be used in
// the <linkto class="VisEquation">VisEquation</linkto>. It is an Abstract Base Class: most methods
// must be defined in derived classes.
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation.
//
// The following examples illustrate how an XCorr can be
// used:
// <ul>
// <li> Simulation: van Vleck correction
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
//      // Make an XCorr
//      CorrelatorXCorr coxc(vs);
//
//      ve.SetXCorr(coxc);
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
// <li> There are no derived classes!
// </todo>

class XCorr {
public:

  XCorr();
  
  // Apply actual function and inverse
  // <group>
  virtual VisBuffer& apply(VisBuffer& vb) = 0;
  virtual VisBuffer& applyInv(VisBuffer& vb) = 0;
  // </group>
 
};

} //# NAMESPACE CASA - END

#endif

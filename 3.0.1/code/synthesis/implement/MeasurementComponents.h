//# MeasurementComponents.h: Definition for components of the Measurement Equation
//# Copyright (C) 1996,1997,1999
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

#ifndef SYNTHESIS_MEASUREMENTCOMPONENTS_H
#define SYNTHESIS_MEASUREMENTCOMPONENTS_H

#include <synthesis/MeasurementComponents/VisJones.h>
#include <synthesis/MeasurementComponents/SkyJones.h>
#include <synthesis/MeasurementComponents/XCorr.h>
#include <synthesis/MeasurementComponents/MJones.h>
#include <synthesis/MeasurementComponents/ACoh.h>
#include <synthesis/MeasurementComponents/FTMachine.h>
#include <synthesis/MeasurementComponents/SkyModel.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <module>

// <summary> 
// Measurement Components encapulsate synthesis measurement effects.
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// </prerequisite>
//
// <etymology>
// Measurement Components represent components of the 
// <linkto module="MeasurementEquations">Measurement Equation</linkto>
// </etymology>
//
// <synopsis> 
// The <linkto module="MeasurementEquations">Measurement Equation</linkto> is 
// expressed in terms of Measurement Components
// such as gain matrices, additive errors, Fourier transform machines, etc.
// MeasurementComponents typically take as arguments a 
// <linkto class="VisBuffer">VisBuffer</linkto> that holds
// the original, model and corrected visibility plus the information
// needed for the correction (e.g. time, uvw, antenna ids, etc)
// and returns the modified <linkto class="VisBuffer">VisBuffer</linkto>.
//
// Synthesis calibration and imaging is accomplished using the modules
// <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// to control the form of the equations to be solved, and
// <linkto module="MeasurementComponents">MeasurementComponents</linkto>
// to encapsulate different types of physical effects.
//
// An example may help. Standard antenna-based gain calibration
// is accomplished using a class <linkto class="GJones">GJones</linkto>
// (derived from <linkto class="VisJones">VisJones</linkto>)
// that applies antenna based gains indexed by time, antenna id (of course),
// and spectral window. All calibration can proceed using this class
// plugged into the
// <linkto module="MeasurementEquations">MeasurementEquations</linkto>.
// Solution for the parameters of the GJones class is accomplished
// using a generic solver that calls methods of the 
// <linkto module="MeasurementEquations">MeasurementEquations</linkto>.
// Suppose that we want to elaborate the model for the antenna
// gains by insisting that antenna phases be given by a polynomial
// function of position across the array (useful for compact
// synthesis arrays). We then derive another class e.g.
// SpatialGJones, that internally is parametrized in terms of antenna
// position. All calibration and imaging can proceed with the new
// class in place of GJones. Solution for the phase screen is
// accomplished using the same services of the
// <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// but the gradients of chi-squared are used internally by SpatialGJones
// to calculate coefficients of the polynomial. When the apply
// method of the SpatialGJones is called, it calculates the
// phase screen as appropriate.
//
// It is expected that some degree of caching will be implemented
// by most MeasurementComponents in order that efficient processing
// is possible.
//
// <ul>
// <li> <a href="MeasurementComponents/VisJones.html">VisJones</a> encapsulates pure 
// visibility-based gain effects as antenna gains</li>
// <li> <a href="MeasurementComponents/SkyJones.html">SkyJones</a> encapsulates pure 
// image plane-based gain effects such as the primary beam</li>
// <li> <a href="MeasurementComponents/MJones.html">MJones</a> encapsulates multiplicative 
// correlator-related visibility-based effects</li>
// <li> <a href="MeasurementComponents/ACoh.html">ACoh</a> encapsulates additive
// correlator-related visibility-based effects</li>
// <li> <a href="MeasurementComponents/XCorr.html">XCorr</a> encapsulates non-linear
// correlator-related visibility-based effects</li>
// <li> <a href="MeasurementComponents/FTMachine.html">FTMachine</a> encapsulates Fourier transform machines
// <li> <a href="MeasurementComponents/SkyModel.html">SkyModel</a> encapsulates models of the
// sky brightness such as images or discrete components
// </ul>
//
// </synopsis> 
//
// <example>
// <srcblock>
//    // Make a MeasurementComponent that applies antenna-based gain
//    // to a coherence sample
//    VisSet vs("3c84.MS");
//    GJones gain(vs, 30);
//
//    // Write original and corrupted coherences
//    ROVisIter vi(vs->iter());
//    VisBuffer vb(vi);
//    for (vi.origin(); vi.more(); vi++) {
//       cout<<"Original value "<<vb.visibility()<<" becomes "
//	     <<apply(vb).correctedVisibility()<<endl;
//    }
// </srcblock>
// </example>
//
// <motivation>
// To encapsulate behavior of components of the MeasurementEquation.
// </motivation>
//
// <todo asof="">
// </todo>

// </module>

} //# NAMESPACE CASA - END

#endif

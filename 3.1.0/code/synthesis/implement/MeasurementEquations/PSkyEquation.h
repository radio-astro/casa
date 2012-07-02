//# PSkyEquation.h: A parallellized version of class SkyEquation
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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
//# $Id$

#ifndef SYNTHESIS_PSKYEQUATION_H
#define SYNTHESIS_PSKYEQUATION_H

#include <synthesis/MeasurementEquations/SkyEquation.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// Parallelized version of class SkyEquation
//</summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//     <li> <linkto module="Applicator">Applicator</linkto> module
// </prerequisite>
//
// <etymology>
// From "parallel" , "sky" and "measurement equation"
// </etymology>
//
// <synopsis> 
// A parallelized version of class SkyEquation. This is provided
// as an inherited specialization. For information on SkyEquation,
// see <linkto class="SkyEquation">SkyEquation</linkto>
// </synopsis> 
//
// <example>
// <srcblock>
// See example in <linkto class="SkyEquation">SkyEquation</linkto>
// </srcblock>
// </example>
//
// <motivation>
// Parallelized version of SkyEquation; supports WF/MF parallel imaging
// </motivation>
//
// <todo asof="98/2/17">
// <li> Implement time-variable SkyJones support
// </todo>

// Forward declarations

class SkyModel;
class VisSet;
class FTMachine;
class ComponentFTMachine;

class PSkyEquation : public SkyEquation {
public:
   // Default constructor
   PSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, 
                ComponentFTMachine& cft);
   PSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, 
                FTMachine& ift, ComponentFTMachine& cft);

   // Parallelized version of the model coherence prediction method
   virtual  void predict(Bool incremental);

   // Parallel sum of weights, chi-squared, and first and second 
   // derivative computation, by transforming to the measured data.
   virtual  void gradientsChiSquared(Bool incremental);
 
   // Parallel solve for the sky model
   virtual Bool solveSkyModel();
   
   // A locking function for the underlying MS
   void lock();

   // An un-locking function for the underlying MS
   void unlock();

   // Return the name of the underlying MS
   String associatedMSName();

private:
};


} //# NAMESPACE CASA - END

#endif

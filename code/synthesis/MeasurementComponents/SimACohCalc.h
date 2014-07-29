//# SimACohCalc.h: Definition for Simulated additive errors, calculated from obs. parms
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

#ifndef SYNTHESIS_SIMACOHCALC_H
#define SYNTHESIS_SIMACOHCALC_H

#include <casa/aips.h>
#include <casa/BasicMath/Random.h>
#include <synthesis/MeasurementComponents/ACoh.h>
#include <casa/Quanta/Quantum.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// SimACohCalc: Model additive noise errors for the VisEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
//   <li> <linkto class="VisSet">VisSet</linkto> class
//   <li> <linkto class="VisEquation">VisEquation</linkto> class
//   <li> <linkto class="SimACoh">SimACoh</linkto> class
// </prerequisite>
//
// <etymology>
// SimACohCalc describes random additive errors to be used in
// the <linkto class="VisEquation">VisEquation</linkto>, calculated from parameters
// such as bandwidth, system temperature, integration time, dish diameter, and efficiencies. 
// </etymology>
//
// <synopsis> 
//
// See <linkto class="VisEquation">VisEquation</linkto> for definition of the
// Visibility Measurement Equation. See <linkto class="ACoh">ACoh</linkto>
// for how SimACohCalc is to be used.
// </synopsis> 
//
// <motivation>
// The properties of an additive component must be described
// for the <linkto class="VisEquation">VisEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// </todo>

class SimACohCalc  : public ACoh
{
public:
  // Construct from VisSet, seed and rms of additive noise (normal dist.)
  // NOTE: rms is for the tau = 0.0 case, and is augmented as
  // appropriate given tau, tatmos, trx (ie, all NON-atmospheric noise
  // contributions), and airmass= 1/sin(ELEVATION).
  // If tau = 0.0, this defaults to the SimACoh case.
  SimACohCalc(const Int seed=1, 
	      const Float antefficiency=0.80, 
	      const Float correfficiency=0.85,  
	      const Float spillefficiency=0.85,
	      const Float tau=0.0, 
	      const Quantity& trx=50,
	      const Quantity& tatmos=250,
	      const Quantity& tcmb=2.7);
 
  // virtual destructor
  virtual ~SimACohCalc();

  // Apply additive error (Inverse is minus)
  // <group>
  virtual VisBuffer& apply(VisBuffer& vb);
  virtual VisBuffer& applyInv(VisBuffer& vb);
  // </group>
private:
  Bool solve(VisEquation& /*ve*/) {return True;}
  SimACohCalc();

  MLCG rndGen_p;
  Normal noiseDist_p;  
  Float antefficiency_p;
  Float correfficiency_p;
  Float spillefficiency_p;
  Float tau_p;
  Quantity trx_p;
  Quantity tatmos_p;
  Quantity tcmb_p;



};


} //# NAMESPACE CASA - END

#endif

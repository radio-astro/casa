//# PBMath1DGauss.h: Definitions of 1-D Gauss PBMath objects
//# Copyright (C) 1996,1997,1998,2003
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

#ifndef SYNTHESIS_PBMATH1DGAUSS_H
#define SYNTHESIS_PBMATH1DGAUSS_H

#include <casa/aips.h>
#include <synthesis/TransformMachines/PBMath1D.h>
#include <measures/Measures.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward

// <summary> 
// PBMath1DGauss is a 1-D Gaussian Primary Beam
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="PBMathInterface">PBMathInterface</linkto> class
// <li> <linkto class="PBMath1D">PBMath1D</linkto> class
// </prerequisite>
//
// <etymology>
// PBMath1DGauss: derived from  PBMath1D, implements a gaussian PB and VP
// </etymology>
//
// <synopsis> 
// See PBMath1D for a general synopsis of the 1D PB types.
// Remember that we construct based on Voltage Patterns rather than
// primary beams.  The Voltage Pattern HWHM width will be sqrt(2)
// times larger than the Primary Beam HWHM.
//
// VP(x) = exp( - ( x/HWHM )**2 * log(2) ), out to maximumRadius
// </synopsis> 
//
//
// <example>
// <srcblock>
//
//    PBMath1DGauss myPB  (Quantity(1.0, "'"),
//                         Quantity(3.0, "'"),
//                         Quantity(1.0, "GHz"),
//                         BeamSquint(MDirection(Quantity(2.0, "\""),
//                                                         Quantity(0.0, "\""),
//                                                         MDirection::Ref(MDirection::AZEL)),
//                                    Quantity(2.0, "GHz")),
//                         False);
// </srcblock>
// </example>
//
// <motivation>
// All of the 1-D PB types have everything in common except for the
// details of their parameterization.  This lightweight class
// deals with those differences: construction, filling the PBArray
// from construction parameters, and flushing to disk.
// BIMA uses a Gaussian PB.
// </motivation>
//
// <todo asof="98/10/21">
// <li> constructor from a MS beam subtable
// <li> flush to MS beam subtable
// </todo>

 
class PBMath1DGauss : public PBMath1D {
public:

  PBMath1DGauss();

  // Instantiation from arguments; default = no squint
  // squint is the offset from pointing center if the Stokes R beam
  // useSymmetricBeam forces a fit to the squinted beam
  // width = Half-Width-Half-max
  // maxRad = half-width at zero intensity
  PBMath1DGauss( Quantity halfWidth, Quantity maxRad, Quantity refFreq, 
		 Bool isThisVP=False,
		 BeamSquint squint=BeamSquint(MDirection(Quantity(0.0, "deg"),
							 Quantity(0.0, "deg"),
							 MDirection::Ref(MDirection::AZEL)),
					      Quantity(1.0, "GHz")),
		 Bool useSymmetricBeam=False);

  // Instantiation from a row in the Beam subTable
  //  PBMath1DGauss(const Table& BeamSubTable, Int row, 
  //	Bool useSymmetricBeam=False);

  // Copy constructor
  // PBMath1DGauss(const PBMath1DGauss& other);

  // Assignment operator, by reference
  PBMath1DGauss& operator=(const PBMath1DGauss& other);

  // Clone the object
  //  CountedPtr<PBMathInterface> clone();

  // destructor
  ~PBMath1DGauss();  

  // Get the type of PB this is
  PBMathInterface::PBClass whichPBClass() { return PBMathInterface::GAUSS; }  
  
  // Flush the construction parameters to disk
  // Bool flushToTable(Table& beamSubTable, Int iRow);

  // Summarize the construction data for this primary beam
  void summary(Int nValues=0);

protected:

  // Fill in vp_p array from construction parameters
  void fillPBArray();

private:    

  Quantity halfWidth_p;

};


} //# NAMESPACE CASA - END

#endif

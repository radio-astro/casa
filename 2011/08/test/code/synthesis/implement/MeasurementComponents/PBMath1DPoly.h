//# PBMath1DPoly.h: Definitions of 1-D Polynomial PBMath objects
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

#ifndef SYNTHESIS_PBMATH1DPOLY_H
#define SYNTHESIS_PBMATH1DPOLY_H

#include <casa/aips.h>
#include <synthesis/MeasurementComponents/PBMath1D.h>
#include <measures/Measures.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward

// <summary> 
// PBMath1DPoly is a 1-D Polynomial Expansion for a Primary Beam
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="PBMathInterface">PBMathInterface</linkto> class
/// <li> <linkto class="PBMath1D">PBMath1D</linkto> class
// </prerequisite>
//
// <etymology>
// PBMath1DPoly: derived from  PBMath1D, implements an Polynomial PB and VP
// </etymology>
//
// <synopsis> 
// PBMath1DPoly: a voltage pattern expressed as a
// polynomial of even powers of x:
// 
// VP(x) = sum_i  coeff_i *  x ^{2i}
// out to maximumRadius (referenced to the reference frequency)
// 
// x is in arcminutes, referenced to the reference frequency
//
// This move may make me a bit unpopular: since the PBMath1D constructors
// construct a Voltage Pattern, the coeficienct for a polynomial type must
// be in terms of the VP, not the PB.  Hence, go back to the original PB
// data, take the square root, and fit the same type polynomial to the
// VP data.
// </synopsis> 
//
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// All of the 1-D PB types have everything in common except for the
// details of their parameterization.  This lightweight class
// deals with those differences: construction, filling the PBArray
// from construction parameters, and flushing to disk.  VLA and ATNF
// PBs are tabulated in terms of polynimials.
// </motivation>
//
// <todo asof="98/10/21">
// <li> constructor from a MS beam subtable
// <li> flush to MS beam subtable
// </todo>

 
class PBMath1DPoly : public PBMath1D {
public:

  PBMath1DPoly();

  // Instantiation from arguments; default = no squint
  // squint is the offset from pointing center if the Stokes R beam
  // useSymmetricBeam forces a fit to the squinted beam
  PBMath1DPoly(const Vector<Double>& coeff, Quantity maxRad, 
	       Quantity refFreq, 
	       Bool isThisVP=False,
	       BeamSquint squint=BeamSquint(MDirection(Quantity(0.0, "deg"),
						       Quantity(0.0, "deg"),
						       MDirection::Ref(MDirection::AZEL)),
					    Quantity(1.0, "GHz")),
	       Bool useSymmetricBeam=False);

  // Instantiation from a row in the Beam subTable
  //PBMath1DPoly(const Table& BeamSubTable, Int row,
  //	       Bool useSymmetricBeam=False);

  // Copy constructor
  // PBMath1DPoly(const PBMath1DPoly& other);

  // Assignment operator, by reference
  PBMath1DPoly& operator=(const PBMath1DPoly& other);

  // Clone the object
  //  CountedPtr<PBMathInterface> clone();

  // Destructor
  ~PBMath1DPoly();  

  // Get the type of PB this is
  PBMathInterface::PBClass whichPBClass() { return PBMathInterface::POLY; }  

  // Flush the construction parameters to disk
  // Bool flushToTable(Table& beamSubTable, Int iRow);

  // Summarize the construction data for this primary beam
  void summary(Int nValues=0);

protected:

  // Fill in vp_p array from construction parameters
  void fillPBArray();

private:    

  Vector<Double> coeff_p;

};


} //# NAMESPACE CASA - END

#endif

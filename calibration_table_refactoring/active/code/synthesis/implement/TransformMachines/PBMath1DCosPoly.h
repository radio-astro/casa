//# PBMath1DCosPoly.h: Definitions of 1-D CosPolynomial PBMath objects
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

#ifndef SYNTHESIS_PBMATH1DCOSPOLY_H
#define SYNTHESIS_PBMATH1DCOSPOLY_H

#include <casa/aips.h>
#include <synthesis/TransformMachines/PBMath1D.h>
#include <measures/Measures.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// PBMath1DCosPoly is a 1-D Polynomial Cosine Expansion for a Primary Beam
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="PBMathInterface">PBMathInterface</linkto> class
// <li> <linkto class="PBMath1D">PBMath1D</linkto> class
// </prerequisite>
//
// <etymology>
// PBMath1DCosPoly: derived from  PBMath1D, implements a polynomial of Cosines of different widths
// </etymology>
//
// <synopsis> 
// PBMath1DCosPoly: a voltage pattern expressed as a
// polynomial of cosines (all powers of them):
// 
// VP(x) = sum_i  coeff_i * cos( scale_i * x )^{i}
// 
// x is in arcminutes, referenced to 1GHz frequency;
// the argument of the cosines are in radians
// </synopsis> 
//
//
// <example>
// <srcblock>
//    Vector<Double> cosCoef(4);
//    Vector<Double> cosScale(4);
//    cosCoef.set(0.0);
//    cosScale.set(0.0);
//    cosCoef(3) = 1.0;
//    cosScale(3) = 0.01891; // 0.01891 = 0.065 * 1000(MHz/GHz) /(60(arcm/deg)) * 2pi/180
//    PBMath1DCosPoly cosPB(cosCoef, cosScale,
//                          Quantity(1.0, "deg"), 
//                          Quantity(1.0, "GHz")  );
//    cosPB.applyPB( im3, im4, pointingDir);
// </srcblock>
// </example>
//
// <motivation>
// All of the 1-D PB types have everything in common except for the
// details of their parameterization.  This lightweight class
// deals with those differences: construction, filling the PBArray
// from construction parameters, and flushing to disk.
// WSRT uses this model.
// </motivation>
//
// <todo asof="98/10/21">
// <li> constructor from a MS beam subtable
// <li> flush to MS beam subtable
// </todo>

 
class PBMath1DCosPoly : public PBMath1D {
public:

  PBMath1DCosPoly();

  // Instantiation from arguments; default = no squint
  // squint is the offset from pointing center if the Stokes R beam
  // useSymmetricBeam forces a fit to the squinted beam
  PBMath1DCosPoly(const Vector<Double>& coeff, 
		  const Vector<Double>& cosScale, Quantity maxRad, 
		  Quantity refFreq, 
		  Bool isThisVP=False,
		  BeamSquint squint=BeamSquint(MDirection(Quantity(0.0, "deg"),
							  Quantity(0.0, "deg"),
							  MDirection::Ref(MDirection::AZEL)),
					       Quantity(1.0, "GHz")),
		  Bool useSymmetricBeam=False);

  // Instantiation from a row in the Beam subTable
  // PBMath1DCosPoly(const Table& BeamSubTable, Int row,
  //	  Bool useSymmetricBeam=False);

  // Copy constructor
  // PBMath1DCosPoly(const PBMath1DCosPoly& other);

  // Assignment operator, by reference
  PBMath1DCosPoly& operator=(const PBMath1DCosPoly& other);

  // Clone the object
  //  CountedPtr<PBMathInterface> clone();

  // destructor
  ~PBMath1DCosPoly();

  // Get the type of PB this is
  PBMathInterface::PBClass whichPBClass() { return PBMathInterface::COSPOLY; }

  // Flush the construction parameters to disk
  // Bool flushToTable(Table& beamSubTable, Int iRow);

  // Summarize the construction data for this primary beam
  void summary(Int nValues=0);


protected:

  // Fill in vp_p array from construction parameters
  void fillPBArray();

private:    

  Vector<Double> coeff_p;
  Vector<Double> cosScale_p;

};


} //# NAMESPACE CASA - END

#endif

//# StokesImageUtil.h: Definitions for Stokes Image utilities
//# Copyright (C) 1996,1997,1998,1999
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

#ifndef SYNTHESIS_STOKESIMAGEUTIL_H
#define SYNTHESIS_STOKESIMAGEUTIL_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <images/Images/ImageInterface.h>
#include <casa/Quanta/Quantum.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MeasurementComponents/ImageSkyModel.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// </summary>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis> 
// </synopsis> 
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="">
// </todo>


class StokesImageUtil {
  
public:

  // Make a Gaussian PSF
  //<group>
  static void MakeGaussianPSF(ImageInterface<Float>& psf, Vector<Float>& beam,
			      Bool norm=False);
  static void MakeGaussianPSF(ImageInterface<Float>& psf,
			      Quantity& bmaj, Quantity& bmin, Quantity& bpa,
			      Bool norm=False);
  //</group>
  
  // Fit a Gaussian PSF
  //<group>
  static Bool FitGaussianPSF(ImageInterface<Float>& psf, Vector<Float>& beam);
  static Bool FitGaussianPSF(ImageInterface<Float>& psf,
			     Quantity& bmaj, Quantity& bmin, Quantity& bpa);
  //</group>
  // Locat peak of PSF return pos, peak and first plane that satisfies 
  // peak >0.9
  
  static void locatePeakPSF(ImageInterface<Float>& psf, Int& xpos, Int& ypos, 
			    Float& amp, Matrix<Float>& psfplane);

  // Convolve a Stokes Image in place
  //<group>
  static void Convolve(ImageInterface<Float>& image,
		       ImageInterface<Float>& psf);
  static void Convolve(ImageInterface<Float>& image, Float bmaj,
		       Float bmin, Float bpa, Bool normalizeVolume=False);
  static void Convolve(ImageInterface<Float>& image,
		       Quantity& bmaj, Quantity& bmin, Quantity& bpa,
		       Bool normalizeVolume=False);
  //</group>
  
  
  // Zero selected planes of a Stokes image
  static void Zero(ImageInterface<Float>& image, Vector<Bool>& mask);
  
  static void MaskFrom(ImageInterface<Float>& mask,
		       ImageInterface<Float>& image,
		       const Quantity& threshold);

  // Zero pixels where Stokes I < some value
  static void MaskOnStokesI(ImageInterface<Float>& image, const Quantity& threshold);

  // Make a box mask
  static void BoxMask(ImageInterface<Float>& mask, const IPosition& blc,
		      const IPosition& trc, const Float value);

  // Constrain a Stokes Image
  static void Constrain(ImageInterface<Float>& image);
  
  // Convert to Stokes Image
  static void From(ImageInterface<Complex>& out,
		   ImageInterface<Float>& in);
  
  // Convert to Stokes Image
  static void To(ImageInterface<Float>& out, ImageInterface<Complex>& in);

  // Direct copy from Float to Complex with 
  static void directCFromR(ImageInterface<Complex>& out,
		   ImageInterface<Float>& in);
  
  // Direct copy To Float...
  static void directCToR(ImageInterface<Float>& out, ImageInterface<Complex>& in);

  
  // Convert to Stokes PSF
  static void ToStokesPSF(ImageInterface<Float>& out, ImageInterface<Complex>& in);
  
  // Find the mapping from pixel on the Stokes Axis to I,Q,U,V
  static Int StokesPolMap(Vector<Int>& map, const CoordinateSystem& coord);
  
  // Find the mapping from pixel on the Stokes Axis to either XX,XY,YX,YY
  // of LL, LR, RL, RR. Return type as well: Linear:1, Circular 0
  static Int CStokesPolMap(Vector<Int>& map, SkyModel::PolRep& polframe,
			   const CoordinateSystem& coord);
  
  // Find all mappings from coordinate to axis number
  static Bool StokesMap(Vector<Int>& map, const CoordinateSystem& coord);
  
  //Create a CoordinateSystem from a MeasurementSet and other info
  static CoordinateSystem StokesCoordFromMS(const IPosition& shape,
					    Vector<Double>& deltas,
					    MeasurementSet& ms);
  
  // Create a CoordinateSystem from a MeasurementSet and other info
  static CoordinateSystem StokesCoordFromMS(const IPosition& shape,
					    Vector<Double>& deltas,
					    MeasurementSet& ms,
					    Vector<Int>& whichStokes,
					    Bool doCStokes=False,
					    Int fieldID=0, Int SPWID=0,
					    Int feedID=0);
  
  // Create a CoordinateSystem 
  static CoordinateSystem CStokesCoord(const IPosition& shape,
				       const CoordinateSystem& coord, 
				       Vector<Int>& whichStokes,
				       SkyModel::PolRep
				       polRep=SkyModel::CIRCULAR);
  static CoordinateSystem
  CStokesCoordFromImage(const ImageInterface<Complex>& image,
			Vector<Int>& whichStokes,
			SkyModel::PolRep polRep);

  // Change the stokes representation (but not the data!)
  static void changeCStokesRep(ImageInterface<Complex>& image,
			       SkyModel::PolRep polRep);

  // check to see if Image coordinates have the standard order:
  // Direction, Stokes, Spectral.  Returns false if this is not
  // the order, or if any are missing
  // <group>
  static Bool standardImageCoordinates(const CoordinateSystem& coords);
  static Bool standardImageCoordinates(const ImageInterface<Complex>& image);
  static Bool standardImageCoordinates(const ImageInterface<Float>& image);
  // </group>

};

} //# NAMESPACE CASA - END

#endif



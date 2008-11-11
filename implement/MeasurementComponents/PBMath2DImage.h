//# PBMath2DImage.h: Definitions of 2-D Image PBMath objects
//# Copyright (C) 1996,1997,1998,2000,2003
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

#ifndef SYNTHESIS_PBMATH2DIMAGE_H
#define SYNTHESIS_PBMATH2DIMAGE_H

#include <casa/aips.h>
#include <synthesis/MeasurementComponents/PBMath2D.h>
#include <images/Images/ImageInterface.h>
#include <measures/Measures.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward
template<class T> class TempImage;

typedef SquareMatrix<Complex,2> mjJones2;
typedef SquareMatrix<Complex,4> mjJones4;
typedef SquareMatrix<Float,2> mjJones2F;

// <summary> 2-D Image Primary Beam Model </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="PBMathInterface">PBMathInterface</linkto> class
// <li> <linkto class="PBMath2D">PBMath2D</linkto> class
// </prerequisite>
//
// <etymology>
//  PBMath2DImage: derived from  PBMath2D, implements a numeric PB and VP
// </etymology>
//
// <synopsis> 
// See PBMath2D for a general synopsis of the 2D PB types.
// 
// The user supplies a vector which is a numerical representation
// of a voltage [attern (hey, if you have a PB, just take the square
// root, and look out for sidelobes which could be negative).
// The first element in the vector needs to be 1.0, the center of the
// voltage pattern.  The last element of the vector is the value of the
// VP at the maximumRadius.  The maximumRadius and the reference frequency at
// which the tabulated VP is intended are also required for construction.
// The PBMath2DImage constructor proceeds by performing SINC interpolation
// on the input vector to generate the highly oversampled lookup vector.
// 
// </synopsis> 
//
//
// <example>
// <srcblock>
//    Vector<Float> vp(10);
//    vp(0) = 1.0f;
//    vp(1) = 0.932f;
//    vp(2) = 0.7462f;
//    vp(3) = 0.4914f;
//    vp(4) = 0.2308f;
//    vp(5) = 0.02183f;   // first null
//    vp(6) = -0.1005f;
//    vp(7) = -0.1318f;
//    vp(8) = -0.09458f;
//    vp(9) = -0.0269f;
//    Quantity maxRad(1.032,"deg");  
//    Quantity refFreq(1.414, "GHz");
//    PBMath2DImage numPB (vp, maxRad, refFreq);
//    numPB.applyPB( im1, im2, pointingDir);
// </srcblock>
// </example>
//
// <motivation>
// All of the 2-D PB types have everything in common except for the
// details of their parameterization.  This lightweight class
// deals with those differences: construction, filling the PBArray
// from construction parameters, and flushing to disk.
// The Image type is very handy: someone can take a sample
// illumination pattern, FT, and take a slice of the resulting voltage
// pattern and construct a  VP from that slice.
// </motivation>
//
// <todo asof="98/10/21">
// <li> constructor from a MS beam subtable
// <li> flush to MS beam subtable
// </todo>

 
class PBMath2DImage : public PBMath2D {
public:

  PBMath2DImage();

  // Instantiation from arguments; only an image is needed
  PBMath2DImage(ImageInterface<Float>& reJonesImage);
  PBMath2DImage(ImageInterface<Float>& reJonesImage,
		ImageInterface<Float>& imJonesImage);

  // Copy constructor
  // PBMath2DGImage(const PBMath2DImage& other);

  // Assignment operator, by reference
  PBMath2DImage& operator=(const PBMath2DImage& other);

  //destructor
 ~PBMath2DImage();  

  // Get the type of PB this is
  PBMathInterface::PBClass whichPBClass() { return PBMathInterface::IMAGE; }  
  
  // Summarize the construction data for this primary beam
  void summary(Int nValues=0);

protected:

  virtual ImageInterface<Complex>& apply(const ImageInterface<Complex>& in,
				 ImageInterface<Complex>& out,
				 const MDirection& sp,
				 const Quantity parAngle,	      
				 const BeamSquint::SquintType doSquint,
				 Bool inverse,
				 Bool conjugate,
				 Int ipower,  // ie, 1=VP, 2=PB, 4=PB^2
				 Float cutoff,
				 Bool forward); 

  virtual ImageInterface<Float>& apply(const ImageInterface<Float>& in,
			       ImageInterface<Float>& out,
			       const MDirection& sp,
			       const Quantity parAngle,	      
			       const BeamSquint::SquintType doSquint,
			       Float cutoff, Int ipower);

  virtual SkyComponent& apply(SkyComponent& in,
		      SkyComponent& out,
		      const MDirection& sp,
		      const Quantity frequency,	      
		      const Quantity parAngle,	      
		      const BeamSquint::SquintType doSquint,
		      Bool inverse,
		      Bool conjugate,
		      Int ipower,  // ie, 1=VP, 2=PB, 4=PB^2
		      Float cutoff,
		      Bool forward); 


private:    

  // Check for congruency
  void checkJonesCongruent(ImageInterface<Float>& reJones,
			   ImageInterface<Float>& imJones);
  void checkImageCongruent(ImageInterface<Float>& image);


  // Update the Jones Matrix
  void updateJones(const CoordinateSystem& coords,
		   const IPosition& shape,
		   const MDirection& pc,
		   const Quantity& paAngle);

  // Complex to Complex
  void applyJones(const Array<Float>* reJones,
		  const Array<Float>* imJones,
		  const Array<Complex>& in,
		  Array<Complex>& out,
		  Vector<Int>& polmap,
		  Bool inverse,
		  Bool conjugate,
		  Int ipower,  // ie, 1=VP, 2=PB
		  Float cutoff,
		  Bool circular=True,
		  Bool forward=True);


  // Float to Float
  void applyJones(const Array<Float>* reJones,
		  const Array<Float>* imJones,
		  const Array<Float>& in,
		  Array<Float>& out,
		  Vector<Int>& polmap,
		  Float cutoff,
		  Bool circular=True);


  TempImage<Float>* reJonesImage_p;
  TempImage<Float>* reRegridJonesImage_p;
  TempImage<Float>* imJonesImage_p;
  TempImage<Float>* imRegridJonesImage_p;

  Vector<Double>* incrementsReJones_p;
  Vector<Double>* incrementsImJones_p;

  Vector<Double>* referencePixelReJones_p;
  Vector<Double>* referencePixelImJones_p;

  Float pa_p;
};


} //# NAMESPACE CASA - END

#endif

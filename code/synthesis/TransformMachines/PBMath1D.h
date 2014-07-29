//# PBMath1D.h: Definitions of interface for 1-D PBMath objects
//# Copyright (C) 1996,1997,1998,1999,2000,2003
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

#ifndef SYNTHESIS_PBMATH1D_H
#define SYNTHESIS_PBMATH1D_H

#include <casa/aips.h>
#include <synthesis/TransformMachines/PBMathInterface.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward
class Table;
class SkyComponent;
class ImageRegion;
class CoordinateSystem;

// <summary> base class for 1D PBMath objects </summary>


// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="SkyJones">SkyJones</linkto> class
// <li> <linkto class="BeamSkyJones">BeamSkyJones</linkto> class
// <li> <linkto class="PBMathInterface">PBMathInterface</linkto> class
// </prerequisite>
//
// <etymology>
// PBMath types do the mathematical operations of the PB's or VP's.
// This is the base class for the 1D (ie, rotationally symmetric) PB's.
// </etymology>
//
// <synopsis> 
// PBMath1D,  the virtual base class for 1D PBMath objects, is
// derived from PBMathInterface.  Its cousin, PBMath2D, can deal with
// inherently 2D voltage patterns or primary beams.  PBMath1D can deal
// with beam squint, (ie, the offset of the LL and RR beams on opposite
// sides of the pointing center) which rotates on the sky with parallactic angle.
// 
// The 1D PB philosophy is to specify the Voltage pattern or Primary Beam
// via a small number of
// parameters via one of the derived types (PBMath1DGauss, for example).  The
// derived type knows how to instantiate itself from a row in a beam subTable,
// and how to convert itself into a lookup vector.  The lookup vector is
// fine enough that no interpolation need be done when finding the nearest
// PB or VP value for a particular pixel (currently, there are 1e+4 elements
// in the lookup vector, so on average, an error on order of 1e-4 is made
// when applying the primary beam).
//
// There are two ways of creating the derived PB types:
// 1) explicitly create one of the babies.  You have control
// over the details such as PB size and total extent, the reference
// frequency at which this size is true (the size scales inversely
// with wavelength), the squint orientation, and whether a mean
// symmetrized beam will be calculated from the squinted beam.
// (Nice defaults can reduce the arguments in most cases.)
// <example>
// <srcblock>
//  PBMath1DGauss myPB  (Quantity(1.0, "'"), Quantity(3.0, "'"), Quantity(1.0, "GHz"),
//                       False,   // these are PB parameters, not VP
//                       BeamSquint(MDirection(Quantity(2.0, "\""),
//                                                      Quantity(0.0, "\""),
//                                                      MDirection::Ref(MDirection::AZEL)),
//                                  Quantity(2.0, "GHz")),
//                       False);
//  PBMath1DGauss myPB2  (Quantity(1.0, "'"), Quantity(3.0, "'"), Quantity(1.0, "GHz"));
// 
// </srcblock>
// </example>
// 2) via the envelope class PBMath's enumerated CommonPB type.  This is much simpler,
// and will deal with a majority of the cases required:
// <example>
// <srcblock>
// PBMath wsrtPB(PBMath::WSRT);
// PBMath vla_LPB(PBMath::VLA_L);  // has L band squint built in
// </srcblock>
// </example>
//
// The main thing you want to do with a primary beam or voltage pattern is
// to apply it to an image.  The top level "apply" methods are defined in
// PBMathInterface.  They are applyPB, applyPB2, applyVP.  These top level
// apply's then call a lower level private polymorphic apply, which are defined
// in PBMath1D and in PBMath2D.  These two different apply's deal with the
// different details of 1D and 2D primary beam application.
// <example>
// <srcblock>
//
//    PagedImage<Float> in;
//    PagedImage<Complex> out;
//    MDirection pointingDir(Quantity(135.0, "deg"), Quantity(60.0, "deg"), 
//                           MDirection::Ref(MDirection::J2000));
//    Quantity parallacticAngle(26.5, "deg");
//    PBMath wsrtPB(PBMath::WSRT_LOW);
//    wsrtPB.applyPB(in, out, pointingDir);   // multiply by primary beam
//    wsrtPB.applyPB(in, out, pointingDir, parallacticAngle, BeamSquint::GOFIGURE, 
//                   True, 0.02); // divide by primary beam
//    wsrtPB.applyVP(in, out, pointingDir);   // multiply by voltage pattern
//
// </srcblock>
// </example>
// </synopsis> 
//
// <motivation>
// All of the 1-D PB types have everything in common except for the
// details of their parameterization.
// </motivation>
//
// lower level helping apply methods: reduce code by this bundling
// <thrown>
// <li> AipsError - in apply(Image...), if in and out images are 
//      inconsistent in shape or coordinates
// <li> AipsError - in  apply(SkyComponent...), if doSqiont==RR or LL
// </thrown>
// <todo asof="98/010/21">
// <li> SymmetrizeBeam doesn't do anything yet.  (It should calculate
//      the mean symmetric beam about the pointing center when
//      squint is present, slightly larger than the symmetric beam 
//      about the squint position.
// </todo>

 
class PBMath1D : public PBMathInterface {
public:

  // required so PBMath can see the protected "apply" method
  // Other derivatives of PBMathInterface, such as PBMath2D, will
  // also require friend class PBMath;
  friend class PBMath;  

  PBMath1D(Quantity maximumRadius,
	   Quantity refFreq,
	   Bool isThisVP,
	   BeamSquint squint,
	   Bool useSymmetricBeam);

  virtual ~PBMath1D() = 0;

  // Get the PB in a vector to look at
  // Concerning n_elements: they are evenly spaced between 0 and maxradius.
  // r is in units of arcminutes at 1 GHz
  void viewPB(Vector<Float>& r, Vector<Float>& PB, Int n_elements);
  
  // Summarize the Voltage Pattern;
  // For PBMath1D, list nValues worth of the VP array
  virtual void summary(Int nValues=0);

  // Is state of PBMath OK?
  virtual Bool ok();

  // Get the ImageRegion of the primary beam on an Image for a given pointing
  // Note: ImageRegion is not necesarily constrained to lie within the
  // image region (for example, if the pointing center is near the edge of the
  // image).  fPad: extra fractional padding, beyond Primary Beam support
  // (note: we do not properly treat squint yet, this will cover it for now)
  // iChan: frequency channel to take: lowest frequency channel is safe for all
  //
  // Potential problem: this ImageRegion includes all Stokes and Frequency Channels
  // present in the input image.
  ImageRegion* extent (const ImageInterface<Complex>& in, 
		       const MDirection& pointing,
		       const Int irow,	
		       const Float fPad,  
		       const Int iChan,  
		       const SkyJones::SizeType sizeType);
  ImageRegion* extent (const ImageInterface<Float>& in, 
		       const MDirection& pointing,
		       const Int irow,	
		       const Float fPad,  
		       const Int iChan,  
		       const SkyJones::SizeType sizeType);


  virtual Int support(const CoordinateSystem& cs);


protected:

  // Protect default constructor: this will do you no good
  PBMath1D();

  // calculate the limited box of the Primary Beam model's support,
  // return in blc and trc (which are NOT contrained to be inside
  // the image
  void extentguts (const CoordinateSystem& coords, const MDirection& pointing,
                    const Float fPad,  const Int iChan, Vector<Float> &blc, Vector<Float>& trc);

  // push blc lower, trc higher such that they define an image which is
  // a power of 2 in size.

  // Adjust blc and trc such that they are within the image and such that they
  // create an image with power of 2 (SkyJones::POWEROF2) shape or
  // composite number  (SkyJones::COMPOSITE) shape 
  void refineSize(Vector<Float>& blc, Vector<Float>& trc, 
		  const IPosition& shape, SkyJones::SizeType);


  ImageInterface<Complex>& apply(const ImageInterface<Complex>& in,
				 ImageInterface<Complex>& out,
				 const MDirection& sp,
				 const Quantity parAngle,	      
				 const BeamSquint::SquintType doSquint,
				 Bool inverse,
				 Bool conjugate,
				 Int ipower,  // ie, 1=VP, 2=PB, 4=PB^2
				 Float cutoff,
				 Bool forward); 


  ImageInterface<Float>& apply(const ImageInterface<Float>& in,
			       ImageInterface<Float>& out,
			       const MDirection& sp,
			       const Quantity parAngle,	      
			       const BeamSquint::SquintType doSquint,
			       Float cutoff, 
			       const Int ipower=4); //only 2 values allowed 2 and 4
                                              //PB and PB^2

  SkyComponent& apply(SkyComponent& in,
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


  // Fill in PB_p array from construction parameters, rescale construction
  // parameters to the 1 GHz internal reference frequency
  // Eventually: create it as its needed; we've got 4 arrays to fill;
  // only create and store as they are required
  // Right now: just construct all arrays
  virtual void fillPBArray()=0;

  // Helper method to fit a circularly symmetric beam to the
  // squinted RR + LL beam.  Called upon construction.  Build this later.
  // PB' = azimuthal fit to: ( VP(x+s)**2 + VP(x-s)**2 )/2
  // VP' = sqrt(PB')
  void symmetrizeSquintedBeam();
  
  void applyXLine(const Complex*& in, Complex*& out, Float*& rx2, Complex*& vp, const Float ry2, const Int ipower, const Bool conjugate, const Bool inverse, const Bool forward, const Int nx, const Int iy, const Double rmax2, const Double factor, const Double inverseIncrementRadius, const Float cutoff);
  // The parameterized representation is for the VP, not the PB.
  // Internally, a reference frequency of 1 GHz is used, and the
  // radius is in units of arcminutes.
  // That said, you can specify the voltage pattern in any
  // units, at any frequency, but they will be converted
  // into (1 GHz * arcminutes) for storage and internal use.
  // We fill in the lookup vectors VP, PB, esVP, esPB,
  // as they are asked for
  // <group>
  // Tabulated voltage pattern
  Vector<Complex> vp_p;

  // Tabulated effective az-symmetrical voltage pattern
  // ( optional, depending upon useSymmetric_p )
  Vector<Complex> esvp_p;
  // </group>

  // Maximum radius allowed in tabulated model
  Quantity maximumRadius_p;

  // reference frequency: used for squint and other
  // beam paramaters such as width, found in derived types.
  // Internally, we rescale everything
  // to a reference frequency of 1 GHz
  Quantity refFreq_p;

  // internal scaling from refFreq_p to 1GHz; used during construction
  Double fScale_p;

  // Increment in radius
  Double inverseIncrementRadius_p;

  // Scale to convert to tabulated units
  Double scale_p;

  // CompositeNumber (for beam application and the like)
  CompositeNumber composite_p;

private:    

};


} //# NAMESPACE CASA - END

#endif

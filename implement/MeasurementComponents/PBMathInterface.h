//# PBMathInterface.h: Definitions of interface for PBMath objects
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

#ifndef SYNTHESIS_PBMATHINTERFACE_H
#define SYNTHESIS_PBMATHINTERFACE_H

#include <casa/aips.h>
#include <synthesis/MeasurementComponents/BeamSquint.h>
#include <images/Images/ImageInterface.h>
#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/ComponentShape.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MeasurementComponents/SkyJones.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//Forward declaration
class Table;
class MDirection;
class SkyComponent;
class ImageRegion;
class String;

// <summary> Virtual base class defining the Primary Beam interface </summary>


// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
// <li> <linkto class="SkyJones">SkyJones</linkto> class
// <li> <linkto class="BeamSkyJones">BeamSkyJones</linkto> class
// </prerequisite>
//
// <etymology>
//  PBMathInterface defines the interface to PB Math objects, the
//  encapsulations of the PB mathematical functioning
// </etymology>
//
// <synopsis>
// PBMathInterface is a virtual base class that defines the
// PBMath interface.  The big thing that it does is to define
// the applyPB, applyVP, and applyPB2 methods, which call a polymorphic
// low level apply method depending upon whether the PBMath type is a
// 1D or 2D type.
//
// PBMathInterface - there are many forms the primary beam can
// take, each of which will be derived from this virtual class which
// defines the generic interface to the multitude of PBMath objects.
// The main part of that interface is application of the PB to
// images, which is defined by PBMathInterface.
//
// PBMathInterface is the virtual base class, and
// PBMath is an envelope type which is derived from
// PBMathInterface.  The two letter classes currently
// envisioned are PBMath1D and PBMath2D.
//
// PBMath can function independent of MeasurementSets.
// PBMath is used by the BeamSkyJones classes VPSkyJones and
// DBeamSkyJones.
// </synopsis>
//
//
// <example>
// <srcblock>

// </srcblock>
// </example>
//
// <motivation>
// The SkyJones classes are all tied up with the MS or VisBuffers.  This class
// encapsulates the raw mathematical functionality of the PB from the
// BeamSkyJones-derived classes which deal with the MS/VisBuff's.
// Also, several different BeamSkyJones objects and effects can be treated
// with the same PBMath heirarchy objects.
// </motivation>
//
// <todo asof="98/10/21">
// <li> summary()
// </todo>

 
class PBMathInterface {
public:

  // required so PBMath can see the protected "apply" method
  // Other derivatives of PBMathInterface, such as PBMath2D, will
  // also require friend class PBMath;
  friend class PBMath;  

  // Allowed PBMath Classes.
  // Currently, COMMONPB, Airy, Gauss, Poly, Ipoly, CosPoly, Numerical
  // are all implemented; Image and Zernike (2D PB's) are not.
  // This enumeration order becomes locked in when we get permanent
  // PB's in the MeasurementSet Tables.
  //
  // COMMPNPB is not really a PB Class, but is here for consistency with
  // glish code in vpmanager
  enum PBClass{NONE, COMMONPB, AIRY, GAUSS, POLY, IPOLY, COSPOLY,
	       NUMERIC, IMAGE, ZERNIKE};

  // Number of different classes in PBClass enumeration
  enum {
    NumberOfPBClasses=10
  };

  // Useful constructor
  PBMathInterface(Bool isThisVP,
		  BeamSquint squint, 
		  Bool useSymmetricBeam);

  // virtual destructor
  virtual ~PBMathInterface() = 0;

  // Apply Jones matrix to an image (and adjoint)
  // <group>

  ImageInterface<Complex>& applyVP(const ImageInterface<Complex>& in,
				   ImageInterface<Complex>& out,
				   const MDirection& sp,
				   const Quantity parAngle = Quantity(0.0,"deg"),
				   const BeamSquint::SquintType doSquint = BeamSquint::NONE,
				   Bool inverse = False,
				   Bool conjugate = False,
				   Float cutoff = 0.01,
				   Bool forward=True);

  ImageInterface<Complex>& applyPB(const ImageInterface<Complex>& in,
				   ImageInterface<Complex>& out,
				   const MDirection& sp,
				   const Quantity parAngle = Quantity(0.0,"deg"),
				   const BeamSquint::SquintType doSquint = BeamSquint::NONE,
				   Bool inverse = False,
				   Float cutoff = 0.01,
				   Bool forward=True);

  // This has a very specialized role (in SkyEquation) and should
  // not be used elsewhere
  ImageInterface<Float>& applyPB2(const ImageInterface<Float>& in,
				  ImageInterface<Float>& out,
				  const MDirection& sp,
				  const Quantity parAngle = Quantity(0.0,"deg"),
				  const BeamSquint::SquintType doSquint = BeamSquint::NONE,
				  Float cutoff=0.01);
  // </group>
  
  // Apply Jones matrix to a sky component (and adjoint)
  // <group>
  SkyComponent& applyVP(SkyComponent& in,
			SkyComponent& out,
			const MDirection& sp,
			const Quantity frequency,
			const Quantity parAngle = Quantity(0.0,"deg"),
			const BeamSquint::SquintType doSquint = BeamSquint::NONE,
			Bool inverse = False,
			Bool conjugate = False,
			Float cutoff = 0.01,
			Bool forward = True);
  SkyComponent& applyPB(SkyComponent& in,
			SkyComponent& out,
			const MDirection& sp,
			const Quantity frequency,
			const Quantity parAngle = Quantity(0.0,"deg"),
			const BeamSquint::SquintType doSquint = BeamSquint::NONE,
			Bool inverse = False,
			Float cutoff = 0.01,
			Bool forward = True);
  SkyComponent& applyPB2(SkyComponent& in,
			 SkyComponent& out,
			 const MDirection& sp,
			 const Quantity frequency,
			 const Quantity parAngle = Quantity(0.0,"deg"),
			 const BeamSquint::SquintType doSquint = BeamSquint::NONE);
  // </group>

  // Which PBClass is this one?
  virtual PBClass whichPBClass() = 0;
  
  // Convert PBClass enumeration into PBClass String
  void namePBClass(String & name);

  // Convert enumeration into a string
  static void namePBClass(const PBMathInterface::PBClass iPB,  String & str);

  // Fit an azimuthally symmetric beam to the squinted or otherwise 2-D beam
  // for Stokes I?
  virtual void setUseSymmetric(Bool useSym=True) { useSymmetricBeam_p = useSym; }

  // Get value of useSymmetric
  virtual Bool getUseSymmetric() { return useSymmetricBeam_p; }

  // Get the ImageRegion of the primary beam on an Image for a given pointing
  // Note: ImageRegion is not necesarily constrained to lie within the
  // image region (for example, if the pointing center is near the edge of the
  // image).  fPad: fractional padding for extra support beyond PB.
  // (note: we do not properly treat squint yet, this will cover it for now)
  // iChan: frequency channel to take: lowest frequency channel is safe for all
  //
  // Potential problem: this ImageRegion includes all Stokes and Frequency Channels
  // present in the input image.
  virtual ImageRegion*  extent (const ImageInterface<Complex>& im, const MDirection& pointing,
				const Int row, const Float fPad,  const Int iChan,  
				const SkyJones::SizeType) = 0;
  virtual ImageRegion*  extent (const ImageInterface<Float>& im, const MDirection& pointing,
				const Int row, const Float fPad,  const Int iChan,  
				const SkyJones::SizeType) = 0;


  // Summarize the Voltage Pattern;
  // For PBMath1D, list nValues worth of the PB array
  virtual void summary(Int nValues=0);

  // Is state of PBMath object OK?
  virtual Bool ok()=0;

protected:
 
  // Dumb constructor: protected, as it won't do you any good!
  PBMathInterface();

  // lower level helping apply methods
  // <group>

  virtual 
  ImageInterface<Complex>& 
  apply(const ImageInterface<Complex>& in,
	ImageInterface<Complex>& out,
	const MDirection& sp,
	const Quantity parAngle,       
	const BeamSquint::SquintType doSquint,
	Bool Inverse,
	Bool Conjugate,
	Int ipower,  // ie, 1=VP, 2=PB
	Float cutoff,
	Bool forward) =0;   

  virtual 
  ImageInterface<Float>& 
  apply(const ImageInterface<Float>& in,
	ImageInterface<Float>& out,
	const MDirection& sp,
	const Quantity parAngle,       
	const BeamSquint::SquintType doSquint,
	Float cutoff) =0;   

  virtual SkyComponent& 
  apply(SkyComponent& in,
	SkyComponent& out,
	const MDirection& sp,
	const Quantity frequency,
	const Quantity parAngle,          
	const BeamSquint::SquintType doSquint,
	Bool Inverse,
	Bool Conjugate,
	Int ipower,  // ie, 1=VP, 2=PB, 4=PB^2
	Float cutoff,
	Bool forward)  =0; 
  //  </group>
  

  // Functions to deal with conversions to Float
  // <group>
  static Float asFloat(const Complex & value) {return value.real(); }
  static Float asFloat(const DComplex & value){return (Float) value.real(); }
  static Float asFloat(const Float & value) {return value; }
  static Float asFloat(const Double & value) {return (Float) value; }
  // </group>

  
  
  // Are the parameters being provided in the derived class constructor
  // describing a Voltage Pattern or a Primary Beam?  The default is Voltage
  // Pattern, and we keep it internally as a voltage pattern
  Bool isThisVP_p;

  // Beam squint: convention is AZ-EL offset of the
  // Stokes RR beam w.r.t. pointing position 
  // Stokes LL will be at -squint_p
  // Southern observatories have the same AZ-EL and PA definitions as Northern
  BeamSquint squint_p;
  
  // Should we use the effective az-symmetrical primary beam?
  // Otherwise, use squinted beam; if no squint, no difference
  Bool useSymmetricBeam_p;

private:

};
 

} //# NAMESPACE CASA - END

#endif


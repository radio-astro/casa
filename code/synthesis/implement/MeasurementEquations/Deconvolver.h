//# Deconvolver: defines classes for deconvolver.
//# Copyright (C) 1996-2007
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id: DOdeconvolver.h,v 19.7 2005/03/18 01:43:31 kgolap Exp $

#ifndef SYNTHESIS_DECONVOLVER_H
#define SYNTHESIS_DECONVOLVER_H

#include <casa/aips.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MRadialVelocity.h>
#include <lattices/Lattices/LatticeCleaner.h>

#include <casa/namespace.h>
namespace casa { //# NAMESPACE CASA - BEGIN
template<class T> class Lattice;
template<class T> class PagedImage;
template<class T> class TempImage;
template<class T> class ImageInterface;
template<class T> class LatticeConvolver;
template<class T> class ResidualEquation;
template<class T> class SubImage;

class File;
class CEMemModel;
class ClarkCleanLatModel;
class PGPlotter;
class LatConvEquation;


// <summary> A simple deconvolver operating on images (no SkyEquation) </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="SkyEquation">SkyEquation</linkto>
//   <li> <linkto class="SkyModel">SkyModel</linkto>
// </prerequisite>
//
// <etymology>
// Undo convolution or at least try to do it !
// </etymology>
//
// <synopsis>
// This class is a container that allows many SkyComponents to be grouped
// together and manipulated as a group. In this respect this class is identical
// to the <linkto class="ComponentList">ComponentList</linkto> class. The user
// is encouraged to read the synopsis of that class for a general description
// of the capabilities of this class.
//
// This class is differs from the ComponentList class in the following ways:
// <ul>
// <li> All components are indexed starting at one. This means the first
//      component in this class is obtained by <src>component(1)</src> rather
//      than <src>component(0)</src> in the ComponentList class.
// <li> Copies of the components, rather than references, are returned to the
//      user. This means that this class needs a replace function whereas
//      ComponentList does not.
// <li> Components that have been removed from the list are stored in a
//      temporary place. In the ComponentList class once they are deleted they
//      are gone.
// <li> This class is derived from ApplicationObject and follows the AIPS++
//      conventions for "distributed objects". Hence the fuunctions in this
//      class can be made accessible from glish. 
// <li> This class can generate simulated components and add them to the list.
// </ul>
//
// There is a one-to-one correspondence between the functions in the glish
// componentlist object (see the AIPS++ User Reference manual) and functions in
// this class. This is make simplify the porting from glish to C++ of a glish
// script using the componentlist distributed object.
// </synopsis>
//
// <example>
// These examples are coded in the tDOcomponentlist.h file.
// <h4>Example 1:</h4>
// In this example a ComponentList object is created and used to calculate the
// ...
// <srcblock>
// </srcblock>
// </example>
//
// <motivation> 
// This class was written to make the componentlist classes usable from glish
// </motivation>
//
// <thrown>
// <li> AipsError - If an internal inconsistancy is detected, when compiled in 
// debug mode only.
// </thrown>
//
// <todo asof="1998/05/22">
//   <li> Nothing I hope. But I expect users will disagree.
// </todo>

class Deconvolver 
{
public:
  // "deconvolver" ctor
  Deconvolver();
  
  Deconvolver(const String& dirty, const String& psf);
  
  Deconvolver(const Deconvolver &other);
  Deconvolver &operator=(const Deconvolver &other);
  ~Deconvolver();

  // Open the given dirty image and psf
  // If warn is true,  print warnings about there being
  // no psf if one is not supplied.
  Bool open(const String& dirty, const String& psf, Bool warn=True);

  // After some cleaning, the dirty image is replaced with the
  // residual image in the deconvolver tool.  reopen reinstates
  // that dirty image; cannot be invoked before open has been
  // invoked
  Bool reopen();
  
  // Flush the ms to disk and detach from the ms file. All function
  // calls after this will be a no-op.
  Bool close();
  
  String dirtyname() const;
  String psfname() const;

  // Output a summary of the state of the object
  Bool summary() const;
  
  // Return the state of the object as a string
  String state() const;
  
  // Return the image shape
  IPosition imageshape() const;

  // Restore
  Bool restore(const String& model,
	       const String& image,
	       Quantity& bmaj, Quantity& bmin, Quantity& bpa);

  // Residual
  Bool residual(const String& model, 
	       const String& image);

  // Smooth
  Bool smooth(const String& model, 
	      const String& image,
	      Quantity& bmaj, Quantity& bmin, Quantity& bpa,
	      Bool normalizeVolume);

  // Clean algorithm
  Bool clean(const String& algorithm,
	     const Int niter, const Float gain, const Quantity& threshold, 
	     const Bool displayProgress,
             const String& model, const String& mask);

  // Clark Clean algorithm
  Bool clarkclean(const Int niter, 
		  const Float gain, const Quantity& threshold, 
		  const Bool displayProgress, 
		  const String& model, const String& mask,
		  const Int histBins, 
		  const Vector<Int>& psfPatchSize, const Float maxExtPsf,
		  const Float speedUp, Int maxNumPix,
		  const Int maxNumMajorCycles,
		  const Int maxNumMinorIterations);
  
  
  // MEM algorithm    add other inputs as required
  Bool mem(const String& algorithm,
	   const Int niter, const Quantity& sigma, 
	   const Quantity& targetFlux, 
	   Bool constrainTargetFlux, 
	   Bool displayprogress, 
	   const String& model, 
	   const String& prior = "",
	   const String& mask = "",
	   const Bool imagePlane = False);
  
  // make a prior image
  Bool makeprior(const String& prior,
		 const String& templateImage,
		 const Quantity& lowClipfrom, 
		 const Quantity& lowClipto, 
		 const Quantity& highClipfrom, 
		 const Quantity& highClipto, 
		 const Vector<Int>& blc,
		 const Vector<Int>& trc);
  
  // Set up scales: based on scaleMethod = "nscales" or "uservector",
  // we will create the scale sizes in pixels via a power law or
  // use the user specified scale sizes.
  Bool setscales(const String& scaleMethod, const Int nscales, 
		 const Vector<Float>& userScaleSizes);
  
  // NNLS algorithm
  Bool nnls(const String& algorithm, const Int niter, const Float tolerance,
	    const String& model, 
	    const String& fluxMask, const String& dataMask);

  // Fourier transform the model and componentlist
  Bool ft(const String& model, const String& transform);

  // Make an empty image
  Bool make(const String& model);

  // Make an empty image with just one Stokes pixel (ie, for a mask)
  Bool make1(const String& imagename);

  // Make an empty image modeled after templateImage
  Bool make(const String& model, ImageInterface<Float>& templateImage);

  // Make a Box Mask
  Bool boxmask(const String& boxmask,
	       const Vector<Int> blc,
	       const Vector<Int> trc,
	       const Quantity& fillValue=1.0,
	       const Quantity& externalValue=0.0);

  //make a mask image from regions
  Bool regionmask(const String& maskimage, Record* imageRegRec, 
		  Matrix<Quantity>& blctrcs, const Float& value=1.0);

  // Clip an image below some Stokes I threshold
  Bool clipimage(const String& clippedImage, const String& inputImage,
		 const Quantity& threshold);

  // Fit the psf
  Bool fitpsf(const String& psf, Quantity& mbmaj, Quantity& mbmin,
	      Quantity& mbpa);

  // Convolve one image with another
  Bool convolve(const String& convolvedmodel, 
		const String& model);

  // Make a Gaussian -- you might want to use it for convolution, etc
  Bool makegaussian(const String& gaussianimage, Quantity& mbmaj, Quantity& mbmin,
	      Quantity& mbpa, Bool normalizeVolume);

  void setPGPlotter(PGPlotter& thePlotter);
  

private:
  
  // Cut the inner quarter out of an image
  SubImage<Float>* innerQuarter(PagedImage<Float>& in);

  // Return full image as a SubImage
  SubImage<Float>* allQuarters(PagedImage<Float>& in);

  // Clone an image
  Bool clone(const String& imageName, const String& newImageName);
  

  //find which axes are the spectral and pol one
  void findAxes();

  //check mask

  void checkMask(ImageInterface<Float>& maskimage, Int& xbeg, Int& xend, 
		 Int& ybeg, Int& yend);


  // setup lattice cleaner
  Bool setupLatCleaner(const String& algorithm,
		       const Int niter, const Float gain, const Quantity& threshold, 
		       const Bool displayProgress);

  PagedImage<Float>* dirty_p;
  PagedImage<Float>* psf_p;

  LatticeConvolver<Float>* convolver_p;
  ResidualEquation<Lattice<Float> >* residEqn_p;
  LatConvEquation* latConvEqn_p;
  CountedPtr <LatticeCleaner<Float> > cleaner_p;

  Bool scalesValid_p;

  Int nx_p, ny_p, npol_p, nchan_p;
  Int chanAxis_p, polAxis_p;
  String mode_p;
  Quantity bmaj_p, bmin_p, bpa_p;
  Bool beamValid_p;
  String dirtyName_p;
  String psfName_p;
  Bool fullPlane_p;

  Vector<Float> itsTotalFluxScale;
  Float itsTotalFlux;
  Vector<Float> scaleSizes_p;


  PGPlotter* pgplotter_p;

  // Set the defaults
  void defaults();

  Bool removeTable(const String& tablename);
  
  // Prints an error message if the deconvolver DO is detached and returns True.
  Bool detached() const;

  String imageName() const;

  Bool valid() const;

  PGPlotter& getPGPlotter(Bool newPlotter=True);
};

} //# NAMESPACE CASA - END

#endif

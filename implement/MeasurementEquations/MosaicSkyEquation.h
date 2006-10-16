//# MosaicSkyEquation.h: MosaicSkyEquation definition
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
//#
//# $Id$

#ifndef SYNTHESIS_MOSAICSKYEQUATION_H
#define SYNTHESIS_MOSAICSKYEQUATION_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <synthesis/MeasurementComponents/FTMachine.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <images/Images/ImageRegion.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// forward decalaration
template <class T> class SubImage;


// <summary> Relate Sky brightness to the visibility, for the multi field case </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="MeasurementComponents">MeasurementComponents</linkto> module
// </prerequisite>
//
// <etymology>
// MF Sky Equation encapsulates the equation between the sky brightness
// and the visibility (or coherence) measured by a mosaicing interferometer
// </etymology>
//
// <synopsis> 
// This is responsible for the Sky-based part of Measurement Equation of the Generic
// Interferometer due to Hamaker, Bregman and Sault and later extended
// by Noordam, and Cornwell.
//
// See <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// for more details of the form of the SkyEquation.
//
// The principal use of SkyEquation is that, as described in 
// <linkto module="MeasurementEquations">MeasurementEquations</linkto>,
// the gradients of chi-squared may be calculated and returned
// as an image.
//
// The following components can be plugged into SkyEquation
// <ul>
// <li> Antenna-based direction-dependent terms: <linkto class="SkyJones">SkyJones</linkto>
// <li> Sky brightness model: <linkto class="SkyModel">SkyModel</linkto>
// <li> Fourier transform machine: <linkto class="FTMachine">FTMachine</linkto>
// </ul>
// </synopsis> 
//
// <example>
// <srcblock>
//
//      // Read the VisSet from disk
//      VisSet vs("3c84.MS");
//
//      PagedImage<Float> im("3c84.modelImage");
//
//      // Create an ImageSkyModel from an image on disk
//      ImageSkyModel ism(im);
//
//      // FTMachine
//      GridFT ft;
//
//      SkyEquation se(ism, vs, ft);
//      // Make a Clean Image and write it out
//      HogbomCleanImageSkyModel csm(ism);
//      if (csm.solveSkyModel()) {
//        PagedImage<Float> cleanImage=csm.getImage();
//        cleanImage.setName("3c84.cleanImage");
//      }
//
// </srcblock>
// </example>
//
// <motivation>
// There are various things we need to do differently for the
// Multi Field case: for example, we need to make a different approximate PSF.
// Later on we will need to do minimum sized FFT's.
// </motivation>
//
// <todo asof="98/2/17">
// <li> Implement SkyJones
// </todo>

// Forward declarations

class MosaicSkyEquation : public SkyEquation {
public:

  // Define a SkyEquation linking a VisSet vs with a SkyModel sm
  // using an FTMachine ft for transforms in both directions
  MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, const Float padding=1.2);
  
  // Define a SkyEquation linking a VisSet vs with a SkyModel sm
  // using an FTMachine ft for transforms in both directions and
  // a ComponentFTMachine for the component lists
  MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft,
	      ComponentFTMachine& cft, const Float padding=1.2);

  // Define a SkyEquation linking a VisSet vs with a SkyModel sm
  // using an FTMachine ft for Sky->Vis and ift for Vis->Sky
  MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, FTMachine& ift, 
		const Float padding=1.2);

  // Define a SkyEquation linking a VisSet vs with a SkyModel sm
  // using an FTMachine ft for Sky->Vis and ift for Vis->Sky and
  // a ComponentFTMachine for the component lists
  MosaicSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, FTMachine& ift,
	      ComponentFTMachine& cft, 
		const Float padding=1.2);

  // copy constructor
  MosaicSkyEquation(const MosaicSkyEquation& other);

  // assignment operator
  MosaicSkyEquation& operator=(const MosaicSkyEquation& other);

  // Destructor
  virtual ~MosaicSkyEquation();


  // Make an approximate PSF for each model. The PSF is approximate
  // in the sense that it is a shift-invariant approximation

  virtual void makeApproxPSF(Int model, ImageInterface<Float>& PSF);


 protected:

  MosaicSkyEquation() {};
  

  // Increment gradientsChiSquared. This version deals with the multiple
  // XFR nature 
  virtual void incrementGradientsChiSquared();


  // Puts for calculating the PSFs
  // <group>
  virtual void initializePutPSF(const VisBuffer &vb, Int model);
  virtual void putPSF(const VisBuffer& vb, Int model, Bool dopsf=True);
  virtual void finalizePutPSF(const VisBuffer& vb, Int Model);
  // </group>

  // We are overwriting these methods to provide support for minimum-sized
  // FFT's (ie, large enough to cover the entire primary beam)
  // in the predict phase
  // <group>
  virtual void initializeGet(const VisBuffer& vb, Int row, Int model,
			     Bool incremental);
  virtual void finalizeGet();
  // </group>


  // We are overwriting these methods to provide support for minimum-sized
  // FFT's (ie, large enough to cover the entire primary beam)
  // in the calculation of the generalized dirty image
  // (used by gradientsChiSquared())
  // <group>
  virtual void initializePut(const VisBuffer &vb, Int model);
  virtual void finalizePut(const VisBuffer& vb, Int Model);
  // </group>


  // We are overwriting these methods to provide support for minimum-sized
  // FFT's (ie, large enough to cover the entire primary beam)
  // in the calculation of residuals by convolution
  // (used by incrementGradientsChiSquared())
  // <group>
  virtual void initializePutXFR(const VisBuffer &vb, Int model, Int numXFR);
  // Just like SkyEquation's, but update nXFR_p
  virtual void MosaicSkyEquation::finalizePutXFR(const VisBuffer& vb, Int model, Int numXFR);
  // Just like SkyEquation's, but update nXFR_p
  virtual void finalizePutConvolve(const VisBuffer& vb, Int Model, Int numXFR);
  // </group>

  // Initialize a lot of stuff, like null counters and null pointers
  void init();


  // provide a cache of imageRegions so we don't have to continually make
  // them
  ImageRegion& imageRegion(const Int model=0, const Int numXFR=0);

  // set the cache of imageRegions
  void setImageRegion(ImageRegion * imgreg, const Int model=0, const Int numXFR=0);

  // Actual cache
  PtrBlock<ImageRegion * > imgreg_p;



  // nXFR and iXFR used for timing info for MF convolutions
  Int nXFR_p;

  // Only give MF <-> no VPSkyJones warning once!
  Int nWarnings;

  // Weakness: this parallels ImageSkyModel::maxNumXFR_p
  // doesn't have to equal it exactly, but we need to increase them
  // in parallel
  Int maxNumXFR_p;

  // Fractional padding beyond the primary beam support
  Float padding_p;

  // Our own work subImages (like sm_->cImage(model)), but
  // we own them all by ourselves, and we get rid of them
  // as we need to
  ImageInterface<Float> * fSubIm_p;
  ImageInterface<Complex> * cSubIm_p;

};

} //# NAMESPACE CASA - END

#endif

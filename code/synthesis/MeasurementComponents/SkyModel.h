//# SkyModel.h: Definition for SkyModel
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

#ifndef SYNTHESIS_SKYMODEL_H
#define SYNTHESIS_SKYMODEL_H

#include <casa/aips.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <images/Images/ImageInterface.h>
#include <components/ComponentModels/ComponentList.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//forward declarations
class SkyEquation;

// <summary> 
// Sky Model: Model the Sky Brightness for the SkyEquation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> Matrix module
//   <li> Vector module
//   <li> MeasurementComponents module
//   <li> VisSet module
// </prerequisite>
//
// <etymology>
// SkyModel describes an interface for Models to be used in
// the SkyEquation. It is an Abstract Base Class: most methods
// must be defined in derived classes.
// </etymology>
//
// <synopsis> 
// A SkyModel contains a number of separate models. The interface to
// SkyEquation is via an image per model. SkyEquation uses this image to
// calculate Fourier transforms, etc. Some (most) SkyModels are
// solvable: the SkyEquation can be used by the SkyModel to return
// gradients with respect to itself (via the image interface). Thus
// for a SkyModel to solve for itself, it calls the SkyEquation
// methods to get gradients of chi-squared with respect to the
// image pixel values (thus returning an image: basically a residual
// image). The SkyModel then uses these gradients as appropriate to
// update itself.
//
// The following examples illustrate how a SkyModel can  be
// used:
// <ul>
// <li> Simple cleaning: one model. The gradient gives the
// residual image. A special method gives a PSF.
// <li> Cleaning with visibility-based subtraction: one model. The
// gradient can be calculated as needed (using the SkyEquation)
// to produce the correct residual image.
// <li> Wide-field imaging: one model per patch of the sky
// that is to be imaged.
// <li> Non-coplanar baselines imaging: one model per facet of
// the polyhedron. At the end of processing all facets are combined
// into one overall image.
// <li> Mosaicing: one model per primary beam pointing. Each model
// is derived (as needed) by cutting out a patch from the full-field
// mosaic.
// </ul>
// </synopsis> 
//
// <example>
// <srcblock>
//      // Read the VisSet from disk
//      VisSet vs("3c84.MS");
//
//      // Create an ImageSkyModel from an image on disk
//      ImageSkyModel ism(PagedImage<Float>("3c84.modelImage"));
//
//      // Make an FTMachine: here we use a simple Grid and FT.
//      GridFT ft;
//
//      SkyEquation se(ism, vs, ft);
//
//      // Predict the visibility set
//      se.predict();
//
//      // Make a Clean Image and write it out
//      HogbomCleanImageSkyModel csm(ism);
//      if (csm.solve()) {
//        PagedImage<Float> cleanImage=csm.image(0);
//        cleanImage.setName("3c84.cleanImage");
//      }
//
// </srcblock>
// </example>
//
// <motivation>
// The properties of a model of the sky must be described
// for the SkyEquation.
// </motivation>
//
// <todo asof="97/10/01">
// <li> Multiple images in SkyModel
// <li> ComponentModel
// </todo>

class SkyModel : public Iterate {

public:
  
  enum PolRep {
    CIRCULAR=StokesImageUtil::CIRCULAR,
    LINEAR=StokesImageUtil::LINEAR
  };

  SkyModel() : itsAlgorithm(""), itsSubAlgorithm(""), imageRegion_p(0), isImageNormalized_p(False) { }  

  // Number of models contained
  virtual Int numberOfModels() = 0;

  // MFS : Number of taylor terms per model
  virtual Int numberOfTaylorTerms() = 0;

  // MFS : Reference Frequency
  virtual Double getReferenceFrequency() = 0;

  // MFS : Index of Taylor term in array of nmodels x ntaylorterms
  virtual Int getTaylorIndex(Int index=0) = 0;

  // Is this SkyModel solveable?
  virtual Bool isSolveable(Int model=0) = 0;

  // Is there a flux scale image associated with this model?
  virtual Bool doFluxScale(Int model=0) = 0;

  // Initialize for gradient search
  virtual void initializeGradients() = 0;

  // Finalize for gradient search
  virtual void finalizeGradients() = 0;

  // Return the component list
  virtual ComponentList& componentList() = 0;

  // Return the component list
  virtual Bool hasComponentList() = 0;

  // Image interface for this model (Stokes representation)
  virtual ImageInterface<Float>& image(Int model=0) = 0;

  // Increment in the image
  virtual ImageInterface<Float>& deltaImage(Int model=0) = 0;

  // Complex image (needed for e.g. RR,RL,LR,LL)
  virtual ImageInterface<Complex>& cImage(Int model=0) = 0;

  // Complex XFR
  virtual ImageInterface<Complex>& XFR(Int model=0, Int numXFR=0) = 0;
  virtual Bool hasXFR(Int model=0) = 0;

  // PSF 
  virtual ImageInterface<Float>& PSF(Int model=0) = 0;

  // Gradient of chi-squared wrt pixels
  virtual ImageInterface<Float>& gS(Int model=0) = 0;

  // Grad Grad chi-squared wrt pixels (diagonal elements only)
  virtual ImageInterface<Float>& ggS(Int model=0) = 0;

  // FluxScale image:   image * fluxScale => true brightness distribution
  virtual ImageInterface<Float>& fluxScale(Int model=0) = 0;
  
  // Work image
  virtual ImageInterface<Float>& work(Int model=0) = 0;

  // Add to Sum weights, Chi-Squared
  virtual void addStatistics(Float sumwt, Float chisq) = 0;

  // Weight per model (channels, polarizations)
  virtual Matrix<Float>& weight(Int model=0) = 0;

  // Solve for this SkyModel
  virtual Bool solve (SkyEquation& se) = 0;

  // Is this model empty
  virtual Bool isEmpty(Int model=0) = 0;

  virtual Int getModelIndex(uInt field=0, uInt taylor=0) = 0;

  //set Algorithm (e.g clean, mem, nnls)
  void setAlgorithm(const String& alg) {itsAlgorithm = alg;}
    
  // get  Algorithm
  const String getAlgorithm() { return itsAlgorithm; }

  // set Sub Algorithm
  void setSubAlgorithm(const String& alg) { itsSubAlgorithm = alg; }

  // get Sub Algorithm
  const String getSubAlgorithm() { return itsSubAlgorithm; }

  // Set the imageregion that will be used for the next XFR generation
  // <group>
  void setImageRegion( ImageRegion& ir ) { imageRegion_p = &ir; }
  // use the default shape
  void unsetImageRegion() { imageRegion_p = 0; }
  // </group>

  void setImageNormalization(Bool val) {isImageNormalized_p = val;};
  Bool isImageNormalized() {return isImageNormalized_p;};

  //set and get memory usage model
  virtual void setMemoryUse(Bool memuse)=0;
  virtual Bool getMemoryUse()=0;
  
protected:
  String itsAlgorithm;
  String itsSubAlgorithm;
  // this ImageRegion is used to suggest the shape for the
  // XFR.  If null, then just use the shape of image(model)
  ImageRegion *imageRegion_p;
  Bool isImageNormalized_p;

private:
};


} //# NAMESPACE CASA - END

#endif

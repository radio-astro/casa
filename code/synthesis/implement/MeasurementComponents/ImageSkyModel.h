//# ImageSkyModel.h: Definition for ImageSkyModel
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

#ifndef SYNTHESIS_IMAGESKYMODEL_H
#define SYNTHESIS_IMAGESKYMODEL_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/PGPlotter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// Image Sky Model: Image-based Model for the Sky Brightness
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=SkyModel>SkyModel</linkto> class
//   <li> <linkto class=SkyEquation>SkyEquation</linkto> class
//   <li> <linkto class=ImageInterface>ImageInterface</linkto> class
//   <li> <linkto class=PagedImage>PagedImage</linkto> class
//   <li> <linkto module=MeasurementComponents>MeasurementComponents</linkto> module
//   <li> <linkto class=VisSet>VisSet</linkto> class
// </prerequisite>
//
// <etymology>
// ImageSkyModel describes an interface for Models to be used in
// the SkyEquation. It is derived from <linkto class=SkyModel>SkyModel</linkto>.
// </etymology>
//
// <synopsis> 
// A ImageSkyModel contains a number of separate models. The interface to
// SkyEquation is via an image per model. <linkto class=SkyEquation>SkyEquation</linkto> uses this image to
// calculate Fourier transforms, etc. Some (most) SkyModels are
// solvable: the SkyEquation can be used by the SkyModel to return
// gradients with respect to itself (via the image interface). Thus
// for a SkyModel to solve for itself, it calls the SkyEquation
// methods to get gradients of chi-squared with respect to the
// image pixel values (thus returning an image: basically a residual
// image). The SkyModel then uses these gradients as appropriate to
// update itself.
// </synopsis> 
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// The properties of a model of the sky must be described
// for the <linkto class=SkyEquation>SkyEquation</linkto>.
// </motivation>
//
// <todo asof="97/10/01">
// <li> Multiple images in SkyModel
// <li> ComponentModel
// </todo>

class ImageSkyModel : public SkyModel {
public:

  // Empty constructor
  ImageSkyModel(const Int maxNumModels=1);

  void setMaxNumberModels(const Int maxNumModels);

  // Copy constructor
  ImageSkyModel(const ImageSkyModel& sm);

  // Add a componentlist
  virtual Bool add(ComponentList& compList);

  // Add an image. maxNumXfr is the maximum Number of transfer functions
  // that we might want to associate with this image.
  virtual Int add(ImageInterface<Float>& image, const Int maxNumXfr=100);
  
  // Add a residual image
  virtual Bool addResidual(Int image, ImageInterface<Float>& residual);

  // Destructor
  virtual ~ImageSkyModel();

  // Assignment operator
  ImageSkyModel& operator=(const ImageSkyModel& other);

  // Number of models contained
  virtual Int numberOfModels() {return nmodels_p;};

  // MFS : Number of taylor terms per model
  virtual Int numberOfTaylorTerms() {return 1;};

  // MFS : Reference Frequency
  virtual Double getReferenceFrequency(){return 0.0;}

  // MFS : Index of Taylor term in array of nmodels x ntaylorterms
  //virtual Int getTaylorIndex(Int index){return 0;}
  virtual Int getTaylorIndex(Int index){return (Int)(index/nfields_p);}

  // Is this model solveable?
  Bool isSolveable(Int model=0);

  // Free and fix the model (returns previous status). Free means that
  // it will be solved for in any solution.
  Bool free(Int model=0);
  Bool  fix(Int model=0);

  // Initialize for gradient search
  virtual void initializeGradients();

  // Finalize for gradient search
  virtual void finalizeGradients() {};

  // Does this have a component list?
  Bool hasComponentList();

  Bool isEmpty(Int model=0);

  // Return the component list
  virtual ComponentList& componentList();

  // Return actual images to be used by SkyEquation. 
  // <group>
  ImageInterface<Float>& image(Int model=0);
  ImageInterface<Complex>& cImage(Int model=0);
  ImageInterface<Complex>& XFR(Int model=0, Int numXFR=0);
  ImageInterface<Float>& PSF(Int model=0);
  ImageInterface<Float>& gS(Int model=0);
  ImageInterface<Float>& residual(Int model=0);
  ImageInterface<Float>& ggS(Int model=0);
  // if (doFluxScale(mod))  image(mod) *  fluxScale(mod)  
  // gives actual brightness distribution
  ImageInterface<Float>& fluxScale(Int model=0);
  ImageInterface<Float>& work(Int model=0);
  ImageInterface<Float>& deltaImage(Int model=0);
  // </group>

  // tells if this model needs to be multiplied by a flux scale image
  Bool doFluxScale(Int model=0);
  // require use of flux scale image
  void mandateFluxScale(Int model=0);

  Bool hasXFR(Int model=0);

  // Add to Sum weights, Chi-Squared
  void addStatistics(Float sumwt, Float chisq) {sumwt_p+=sumwt;chisq_p+=chisq;}

  // Weight per model (channels, polarizations)
  Matrix<Float>& weight(Int model=0);

  // Solve for this SkyModel: This replaces the image with
  // the residual image
  virtual Bool solve (SkyEquation& me);

  // Solve explicitly for the residuals: same as solve for
  // this class
  // modelToMs determines if predicted vis is put in the MODEL_DATA column
  Bool solveResiduals (SkyEquation& me, Bool modelToMS=False);

  // Make the approximate PSFs needed for each model
  virtual void makeApproxPSFs(SkyEquation& se);

  // Get current residual image: this is either that image specified via
  // addResidual, or a scratch image.
  ImageInterface<Float>& getResidual (Int model=0);

  // Return the fitted beam for each model
  Vector<Float>& beam(Int model=0);

  // Set PGPlotter to be used
  void setPGPlotter(PGPlotter& pgp) { pgplotter_p = &pgp; }

  // This is the factor by which you multiply the worst outer
  // sidelobe by to get the threshold for the current cycle
  void setCycleFactor(float x) { cycleFactor_p = x; }

  // Cycle threshold will double in this number of iterations
  // (ie, use a large number if you don't want cycle threshold
  // to inch up)
  void setCycleSpeedup(float x) { cycleSpeedup_p = x; }

  // Set the variable that switches on the progress display
  void setDisplayProgress (const Bool display ) {displayProgress_p = display; };

protected:

  // Make Newton Raphson step internally. This is really an implementation
  // detail: it is useful for derived classes.
  // The modelToMS parameter is for committing to MODEL_DATA column of the MS
  // the predicted visibilities.

  Bool makeNewtonRaphsonStep(SkyEquation& se, 
			     Bool incremental=False, Bool modelToMS=False);


  // Get PGPlotter to be used
  PGPlotter* getPGPlotter() { return pgplotter_p; }

  Int maxnmodels_p;
  Int nmodels_p;
  //MFS
  Int nfields_p;

  Int maxNumXFR_p;

  Float sumwt_p; 
  Float chisq_p; 

  // ComponentList
  ComponentList* componentList_p;

  // Images
  Vector<String> imageNames_p;
  // Everything here can be just interface
  PtrBlock<ImageInterface<Float> * > image_p;
  PtrBlock<ImageInterface<Float> * > residual_p;

  // We actually create these
  PtrBlock<TempImage<Complex> * > cimage_p;
  PtrBlock<TempImage<Complex> * > cxfr_p;
  PtrBlock<TempImage<Float> * > residualImage_p;
  PtrBlock<TempImage<Float> * > gS_p;
  PtrBlock<TempImage<Float> * > psf_p;
  PtrBlock<TempImage<Float> * > ggS_p;
  // if (doFluxScale_p), image_p * fluxScale_p gives the true brightness
  PtrBlock<TempImage<Float> * > fluxScale_p;
  PtrBlock<TempImage<Float> * > work_p;
  PtrBlock<TempImage<Float> * > deltaimage_p;
  Block<Bool> solve_p;
  Block<Bool> doFluxScale_p;

  PtrBlock<Matrix<Float> * > weight_p;

  PtrBlock<Vector<Float> * > beam_p;

  LogSink logSink_p;
  LogSink& logSink() {return logSink_p;};

  PGPlotter *pgplotter_p;
  Bool displayProgress_p;
  // This is the factor by which you multiply the worst outer
  // sidelobe by to get the threshold for the current cycle
  Float cycleFactor_p;
  // Cycle threshold will double in this number of iterations
  // (ie, use a large number if you don't want cycle threshold
  // to inch up)
  Float cycleSpeedup_p;
  // If PSF is done..should not redo it.
  Bool donePSF_p;
  // check if model has been modified especially for continuing
  // a deconvolution
  Bool modified_p;
};



} //# NAMESPACE CASA - END

#endif



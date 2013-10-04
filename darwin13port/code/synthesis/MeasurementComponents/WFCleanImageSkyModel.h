//# WFCleanImageSkyModel.h: Definition for WFCleanImageSkyModel
//# Copyright (C) 1996,1997,1998,1999,2002
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

#ifndef SYNTHESIS_WFCLEANIMAGESKYMODEL_H
#define SYNTHESIS_WFCLEANIMAGESKYMODEL_H

#include <synthesis/MeasurementComponents/MFCleanImageSkyModel.h>
namespace casa { //# NAMESPACE CASA - BEGIN

//forward
class SkyEquation;
template <class T> class SubImage;

// <summary> 
// WF Image Sky Model: Image Sky Model implementing the WF Clean algorithm
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=ImageSkyModel>ImageSkyModel</linkto> module
//   <li> <linkto class=MFCleanImageSkyModel>MFCleanImageSkyModel</linkto> module
// </prerequisite>
//
// <etymology>
// WFCleanImageSkyModel implements the WF Clean algorithm.
// It is derived from <linkto class=MFCleanImageSkyModel>MFCleanImageSkyModel</linkto>.
// </etymology>
//
// <synopsis> 
// The WF Clean is an FFT-based clean algorithm. Cleaning is
// split into major and minor cycles. In a minor cycle, the
// brightest pixels are cleaned using only the strongest sidelobes
// (and main lobe) of the PSF. In the major cycle, a fully correct
// subtraction of the PSF is done for all points accumulated in the
// minor cycle using an FFT-based convolution for speed.
//
// The WF Clean is implemented using the 
// <linkto class=MFCleanImageSkyModel>MFCleanImageSkyModel</linkto>
// class.
//
// Masking is optionally performed using a mask image: only points
// where the mask is non-zero are cleaned. If no mask is specified
// all points in the inner quarter of the image are cleaned.
// </synopsis> 
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="97/10/01">
// <ul> Allow specification of more control parameters
// </todo>

class WFCleanImageSkyModel : public MFCleanImageSkyModel {
public:

  WFCleanImageSkyModel();

  virtual ~WFCleanImageSkyModel();

  WFCleanImageSkyModel(const Int nfacets, Bool largeMemory=False);

  virtual Int add(ImageInterface<Float>& image, const Int maxNumXfr=100);

  virtual Bool addResidual(Int image, ImageInterface<Float>& residual);

  virtual Bool addMask(Int image, ImageInterface<Float>& mask);

  // Solve for this SkyModel
  virtual Bool solve (SkyEquation& me);

  //This will return for model 0 the main residual image i.e nxn facets
  virtual ImageInterface<Float>& getResidual(Int physImageID);

  //void makeApproxPSFs(SkyEquation& se);
  
  /*
  PtrBlock<TempImage<Complex> *>& cimageBlkPtr() { return cimage_p;};
  PtrBlock<Matrix<Float> * >& weightBlkPtr(){return weight_p; } ;
  */
protected:

  // Make a facet
  SubImage<Float>* makeFacet(Int facet, ImageInterface<Float>& image);

  // Make the slicers
  Bool makeSlicers(const Int facet, const IPosition& imageShape, 
		   IPosition& facetShape,
		   Slicer& imageSlicer);
  // Number of facets
  Int nfacets_p;


private:

  CountedPtr<ImageInterface<Float> > imageImage_p;
  CountedPtr<ImageInterface<Float> > residualImage_p;
  CountedPtr<ImageInterface<Float> > maskImage_p;

  Int facets_p;
  Bool largeMem_p;

  // Here we store the facets for the various images: Model, Mask
  // and Residual
  Block<CountedPtr<SubImage<Float> > > facetImages_p;
  Block<CountedPtr<SubImage<Float> > > facetMaskImages_p;
  Block<CountedPtr<SubImage<Float> > > facetResidualImages_p;
  
};


} //# NAMESPACE CASA - END

#endif



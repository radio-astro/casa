//# SIImageStore.h: Imager functionality sits here; 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//#
//# $Id$

#ifndef SYNTHESIS_SIIMAGESTORE_H
#define SYNTHESIS_SIIMAGESTORE_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <casa/BasicSL/Constants.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SIImageStore 
{
 public:
  // Default constructor

  SIImageStore();
  SIImageStore(String imagename);
  SIImageStore(String imagename, 
	       CoordinateSystem &imcoordsys, 
	       IPosition imshape, 
	       const Int nfacets=1, 
	       const Bool overwrite=False,
	       const Bool useweightimage=False);
  // Contructor by image objects necessary for facetted imaging (subimages 
  //can be passed in as residual and model etc). The caller has to make sure the 
  //images are similar.  the Pointers are taken over by CountedPtr...and will be deleted when reference 
  //count goes to 0
  SIImageStore(ImageInterface<Float>* modelim, ImageInterface<Float>* residim,
	       ImageInterface<Float>* psfim, ImageInterface<Float>* weightim, 
	       ImageInterface<Float>* restoredim, ImageInterface<Float>* maskim,
	       ImageInterface<Float>* sumwtim);
    

  virtual ~SIImageStore();


  IPosition getShape();
  String getName();

  virtual String getType(){return "default";}

  virtual CountedPtr<ImageInterface<Float> > psf(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > residual(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > weight(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > model(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > image(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > mask(uInt term=0);
  virtual CountedPtr<ImageInterface<Complex> > forwardGrid(uInt term=0);
  virtual CountedPtr<ImageInterface<Complex> > backwardGrid(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > sumwt(uInt term=0);

  virtual CountedPtr<ImageInterface<Float> > alpha(){throw(AipsError("No Alpha for 1 term"));};
  virtual CountedPtr<ImageInterface<Float> > beta(){throw(AipsError("No Beta for 1 term"));};

  virtual void setModelImage( String modelname );
  virtual Bool doesImageExist(String imagename);
  void setImageInfo(const Record miscinfo);

  virtual void resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight );
  virtual void addImages( CountedPtr<SIImageStore> imagestoadd, 
		  Bool addpsf, Bool addresidual, Bool addweight );

  virtual void dividePSFByWeight();
  virtual void divideResidualByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  virtual void divideModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  virtual void multiplyModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");

  //  virtual Bool isValid(){return itsValidity;}
  virtual Bool checkValidity(const Bool ipsf, const Bool iresidual, const Bool iweight, 
			     const Bool imodel, const Bool irestored, const Bool imask=False,
			     const Bool isumwt=True, const Bool ialpha=False, const Bool ibeta=False);

  virtual Bool releaseLocks();

  void getNSubImageStores(const Bool onechan, const Bool onepol, uInt& nSubChans, uInt& nSubPols);

  virtual Double getReferenceFrequency(){return 0.0;}

  virtual uInt getNTaylorTerms(Bool dopsf=False); //{return 1;};

  GaussianBeam getPSFGaussian();

  virtual GaussianBeam restorePlane();
  virtual void pbcorPlane();

  // Get a SIImageStore of a given facet..the caller has to delete it
  // The images internall will reference back to a given section of the main of this.
  //nfacets = nx_facets*ny_facets...assumption has been made  nx_facets==ny_facets
  virtual CountedPtr<SIImageStore> getFacetImageStore(const Int facet, const Int nfacets);
  virtual CountedPtr<SIImageStore> getSubImageStore(const Int chan, const Bool onechan, 
					    const Int pol, const Bool onepol);

  ///  Matrix<Float> sumWeights; // 2D for image pol/chan

  Bool getUseWeightImage(ImageInterface<Float>& target);

  virtual Bool hasSensitivity(){return !itsWeight.null();}
  virtual Bool hasMask(){return !itsMask.null(); }

protected:
// Can make this a utility function elsewhere...
//nfacets = nx_facets*ny_facets...assumption has been made  nx_facets==ny_facets
  SubImage<Float>* makeFacet(const Int facet, const Int nfacets,
			     ImageInterface<Float>& image);
  SubImage<Float>* makePlane(const Int chan, const Bool onechan, 
			     const Int pol, const Bool onepol,
			     ImageInterface<Float>& image);
  
  Double memoryBeforeLattice();
  IPosition tileShape();

  Matrix<Float> getSumWt(ImageInterface<Float>& target);
  void setSumWt(ImageInterface<Float>& target, Matrix<Float>& sumwt);
  //  Bool getUseWeightImage(ImageInterface<Float>& target);
  void setUseWeightImage(ImageInterface<Float>& target, Bool useweightimage);
  void addSumWts(ImageInterface<Float>& target, ImageInterface<Float>& toadd);

  Bool divideImageByWeightVal( ImageInterface<Float>& target );


  void checkRef( CountedPtr<ImageInterface<Float> > ptr, const String label );
  CountedPtr<ImageInterface<Float> > openImage(const String imagenamefull, 
					       const Bool overwrite, 
					       const Bool dosumwt=False);

  Double getPbMax();

  ///////////////////// Member Objects

  IPosition itsImageShape;
  String itsImageName;
  CoordinateSystem itsCoordSys;
  Int itsNFacets;
  Bool itsUseWeight;

  // Misc Information to go into the header. 
  Record itsMiscInfo;

  //  Bool itsWeightExists;

  //  Bool itsPsfNormed, itsResNormed;

  Bool itsValidity;

  CountedPtr<ImageInterface<Float> > itsMask; // mutliterm shares this...

  Double itsPBScaleFactor;

private:

  CountedPtr<ImageInterface<Float> > itsPsf, itsModel, itsResidual, itsWeight, itsImage, itsSumWt;
  CountedPtr<ImageInterface<Complex> > itsForwardGrid, itsBackwardGrid;


};


} //# NAMESPACE CASA - END

#endif

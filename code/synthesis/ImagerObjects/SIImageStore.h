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
#include <synthesis/TransformMachines/StokesImageUtil.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  template <class T>
  void openImage(const String& imagenamefull,CountedPtr<ImageInterface<T> >* img);
    

class SIImageStore 
{
 public:
  enum IMAGE_IDS {MASK=0,PSF,MODEL,RESIDUAL,WEIGHT,IMAGE,SUMWT,FORWARDGRID,BACKWARDGRID, MAX_IMAGE_IDS};
  // Default constructor

  SIImageStore();

  SIImageStore(String imagename,const Bool ignorefacets=False);

  SIImageStore(String imagename, 
	       CoordinateSystem &imcoordsys, 
	       IPosition imshape, 
	       //	       const Int nfacets=1, 
	       const Bool overwrite=False,
	       const Bool useweightimage=False);

  SIImageStore(CountedPtr<ImageInterface<Float> > modelim, 
	       CountedPtr<ImageInterface<Float> > residim,
	       CountedPtr<ImageInterface<Float> > psfim, 
	       CountedPtr<ImageInterface<Float> > weightim, 
	       CountedPtr<ImageInterface<Float> > restoredim, 
	       CountedPtr<ImageInterface<Float> > maskim,
	       CountedPtr<ImageInterface<Float> > sumwtim,
	       CoordinateSystem& csys, 
	       IPosition imshape, 
	       String imagename, 
	       const Int facet=0, const Int nfacets=1,
	       const Int chan=0, const Int nchanchunks=1,
	       const Int pol=0, const Int npolchunks=1);

  
    
  virtual void init();

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

  ///// Normalizers
  virtual void dividePSFByWeight();
  virtual void divideResidualByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  virtual void divideModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  virtual void multiplyModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");

  /// Other
  virtual Bool releaseLocks();
  virtual Double getReferenceFrequency(){return 0.0;}
  virtual uInt getNTaylorTerms(Bool dopsf=False); //{return 1;};
  GaussianBeam getPSFGaussian();
  virtual GaussianBeam restorePlane();
  virtual void restore(GaussianBeam& rbeam, String& usebeam,uInt term=0 );
  virtual void pbcorPlane();


  ////////// Restoring Beams
  virtual void makeImageBeamSet();
  ImageBeamSet getBeamSet();
  virtual void printBeamSet();
  GaussianBeam findGoodBeam();
  void lineFit(Vector<Float> &data, Vector<Bool> &flag, Vector<Float> &fit, uInt lim1, uInt lim2);
  Float calcMean(Vector<Float> &vect, Vector<Bool> &flag);
  Float calcStd(Vector<Float> &vect, Vector<Bool> &flag, Vector<Float> &fit);
  Float calcStd(Vector<Float> &vect, Vector<Bool> &flag, Float mean);

  // The images internall will reference back to a given section of the main of this.
  //nfacets = nx_facets*ny_facets...assumption has been made  nx_facets==ny_facets
  virtual CountedPtr<SIImageStore> getSubImageStore(const Int facet=0, const Int nfacets=1, 
						    const Int chan=0, const Int nchanchunks=1, 
						    const Int pol=0, const Int npolchunks=1);

  Bool getUseWeightImage(ImageInterface<Float>& target);

  virtual Bool hasSensitivity(){return !itsWeight.null();}
  virtual Bool hasMask(){return !itsMask.null(); }
  virtual Bool hasModel() {return !itsModel.null();}
  virtual Bool hasPsf() {return !itsPsf.null();}
  virtual Bool hasResidual() {return !itsResidual.null();}
  virtual Bool hasSumWt() {return !itsSumWt.null();}

  // Image Statistics....
  Float getPeakResidual();
  Float getPeakResidualWithinMask();
  Float getModelFlux();
  Float getPSFSidelobeLevel();
  void findMinMax(const Array<Float>& lattice,
		  const Array<Float>& mask,
		  Float& minVal, Float& maxVal,
		  Float& minValMask, Float& maxValMask);
  void printImageStats();
  Float getMaskSum();

  //
  //---------------------------------------------------------------
  //
  void makePersistent(String& fileName);
  void recreate(String& fileName);

  void validate();


  void setDataPolFrame(StokesImageUtil::PolRep datapolrep) {itsDataPolRep = datapolrep;};
  virtual void calcSensitivity();

protected:
  CountedPtr<ImageInterface<Float> > makeSubImage(const Int facet, const Int nfacets,
						  const Int chan, const Int nchanchunks,
						  const Int pol, const Int npolchunks,
						  ImageInterface<Float>& image);

  Double memoryBeforeLattice();
  IPosition tileShape();

  Matrix<Float> getSumWt(ImageInterface<Float>& target);
  void setSumWt(ImageInterface<Float>& target, Matrix<Float>& sumwt);
  void setUseWeightImage(ImageInterface<Float>& target, Bool useweightimage);

  Bool divideImageByWeightVal( ImageInterface<Float>& target );

  void accessImage( CountedPtr<ImageInterface<Float> > &ptr, 
		    CountedPtr<ImageInterface<Float> > &parentptr, 
		    const String label );

  CountedPtr<ImageInterface<Float> > openImage(const String imagenamefull, 
					       const Bool overwrite, 
					       const Bool dosumwt=False,
					       const Int nfacetsperside=1);

  Double getPbMax();


  ///////////////////// Member Objects

  IPosition itsImageShape, itsParentImageShape;
  String itsImageName;
  CoordinateSystem itsCoordSys;

  Bool itsUseWeight;
  Record itsMiscInfo;
  CountedPtr<ImageInterface<Float> > itsMask, itsParentMask; // mutliterm shares this...
  Double itsPBScaleFactor;

  Int itsNFacets, itsFacetId;
  Int itsNChanChunks, itsChanId;
  Int itsNPolChunks, itsPolId;

  StokesImageUtil::PolRep itsDataPolRep;

  ImageBeamSet itsPSFBeams;

  //
  //------------------------------------------
  // Non-persistent internal variables
  Vector<String> imageExts;

private:

  CountedPtr<ImageInterface<Float> > itsPsf, itsModel, itsResidual, itsWeight, itsImage, itsSumWt;
  CountedPtr<ImageInterface<Complex> > itsForwardGrid, itsBackwardGrid;

  CountedPtr<ImageInterface<Float> > itsParentPsf, itsParentModel, itsParentResidual, itsParentWeight, itsParentImage, itsParentSumWt;


};

} //# NAMESPACE CASA - END

#endif

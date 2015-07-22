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
  void openImage(const String& imagenamefull,SHARED_PTR<ImageInterface<T> >* img);
    

class SIImageStore 
{
 public:
  enum IMAGE_IDS {MASK=0,PSF,MODEL,RESIDUAL,WEIGHT,IMAGE,SUMWT,GRIDWT,PB,FORWARDGRID,BACKWARDGRID, MAX_IMAGE_IDS};
  // Default constructor

  SIImageStore();

  SIImageStore(String imagename,const Bool ignorefacets=False);

  SIImageStore(String imagename, 
	       CoordinateSystem &imcoordsys, 
	       IPosition imshape, 
	       //	       const Int nfacets=1, 
	       const Bool overwrite=False,
	       const Bool useweightimage=False);

  SIImageStore(SHARED_PTR<ImageInterface<Float> > modelim, 
	       SHARED_PTR<ImageInterface<Float> > residim,
	       SHARED_PTR<ImageInterface<Float> > psfim, 
	       SHARED_PTR<ImageInterface<Float> > weightim, 
	       SHARED_PTR<ImageInterface<Float> > restoredim, 
	       SHARED_PTR<ImageInterface<Float> > maskim,
	       SHARED_PTR<ImageInterface<Float> > sumwtim,
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

  virtual SHARED_PTR<ImageInterface<Float> > psf(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > residual(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > weight(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > model(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > image(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > mask(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Complex> > forwardGrid(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Complex> > backwardGrid(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > sumwt(uInt term=0);

  virtual SHARED_PTR<ImageInterface<Float> > alpha(){throw(AipsError("No Alpha for 1 term"));};
  virtual SHARED_PTR<ImageInterface<Float> > beta(){throw(AipsError("No Beta for 1 term"));};

  virtual SHARED_PTR<ImageInterface<Float> > gridwt(uInt term=0);
  virtual SHARED_PTR<ImageInterface<Float> > pb(uInt term=0);

  virtual void setModelImage( String modelname );
  virtual void setWeightDensity( SHARED_PTR<SIImageStore> imagetoset );
  virtual Bool doesImageExist(String imagename);
  void setImageInfo(const Record miscinfo);

  virtual void resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight );
  virtual void addImages( SHARED_PTR<SIImageStore> imagestoadd, 
			  Bool addpsf, Bool addresidual, Bool addweight, Bool adddensity );

  ///// Normalizers
  virtual void dividePSFByWeight(const Float pblimit=C::minfloat);
  virtual void divideResidualByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  virtual void divideModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  virtual void multiplyModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");

  /// Other
  virtual Bool releaseLocks();
  void releaseImage( SHARED_PTR<ImageInterface<Float> > im );
  virtual Double getReferenceFrequency(){return 0.0;}
  virtual uInt getNTaylorTerms(Bool dopsf=False); //{return 1;};
  GaussianBeam getPSFGaussian();
  //  virtual GaussianBeam restorePlane();
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
  virtual SHARED_PTR<SIImageStore> getSubImageStore(const Int facet=0, const Int nfacets=1, 
						    const Int chan=0, const Int nchanchunks=1, 
						    const Int pol=0, const Int npolchunks=1);

  Bool getUseWeightImage(ImageInterface<Float>& target);

  //  virtual Bool hasSensitivity(){return doesImageExist(itsImageName+imageExts(WEIGHT));}
  virtual Bool hasPB(){return doesImageExist(itsImageName+imageExts(PB));}

  virtual Bool hasSensitivity(){return (bool) itsWeight;}
  //virtual Bool hasPB(){return (bool) itsPB;}

  virtual Bool hasMask(){return (bool) itsMask; }
  virtual Bool hasModel() {return (bool) itsModel;}
  virtual Bool hasPsf() {return (bool) itsPsf;}
  virtual Bool hasResidual() {return (bool) itsResidual;}
  virtual Bool hasSumWt() {return (bool) itsSumWt;}

  // Image Statistics....
  Float getPeakResidual();
  Float getPeakResidualWithinMask();
  Float getModelFlux(uInt term=0);
  virtual Bool isModelEmpty();
  Float getPSFSidelobeLevel();
  void findMinMax(const Array<Float>& lattice,
		  const Array<Float>& mask,
		  Float& minVal, Float& maxVal,
		  Float& minValMask, Float& maxValMask);
  virtual void printImageStats();
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
  SHARED_PTR<ImageInterface<Float> > makeSubImage(const Int facet, const Int nfacets,
						  const Int chan, const Int nchanchunks,
						  const Int pol, const Int npolchunks,
						  ImageInterface<Float>& image);

  Double memoryBeforeLattice();
  IPosition tileShape();

  void regridToModelImage(ImageInterface<Float> &inputimage, Int term=0 );

  Matrix<Float> getSumWt(ImageInterface<Float>& target);
  void setSumWt(ImageInterface<Float>& target, Matrix<Float>& sumwt);
  void setUseWeightImage(ImageInterface<Float>& target, Bool useweightimage);

  void fillSumWt(Int term=0);
  Bool divideImageByWeightVal( ImageInterface<Float>& target );
  void normPSF(Int term=0);

  void makePBFromWeight(const Float pblimit);

  void accessImage( SHARED_PTR<ImageInterface<Float> > &ptr, 
		    SHARED_PTR<ImageInterface<Float> > &parentptr, 
		    const String label );

  SHARED_PTR<ImageInterface<Float> > openImage(const String imagenamefull, 
					       const Bool overwrite, 
					       const Bool dosumwt=False,
					       const Int nfacetsperside=1);

  Double getPbMax();
  Double getPbMax(Int pol, Int chan);

  Bool createMask(LatticeExpr<Bool> &lemask, CountedPtr<ImageInterface<Float> >outimage);
  Bool copyMask(CountedPtr<ImageInterface<Float> >inimage, CountedPtr<ImageInterface<Float> >outimage);

  ///////////////////// Member Objects

  IPosition itsImageShape, itsParentImageShape;
  String itsImageName;
  CoordinateSystem itsCoordSys;

  Bool itsUseWeight;
  Record itsMiscInfo;
  SHARED_PTR<ImageInterface<Float> > itsMask, itsParentMask, itsGridWt, itsPB; // mutliterm shares this...
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

  SHARED_PTR<ImageInterface<Float> > itsPsf, itsModel, itsResidual, itsWeight, itsImage, itsSumWt;
  SHARED_PTR<ImageInterface<Complex> > itsForwardGrid, itsBackwardGrid;

  SHARED_PTR<ImageInterface<Float> > itsParentPsf, itsParentModel, itsParentResidual, itsParentWeight, itsParentImage, itsParentSumWt, itsParentGridWt, itsParentPB;


};

} //# NAMESPACE CASA - END

#endif

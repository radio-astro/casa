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

#ifndef SYNTHESIS_SIIMAGESTOREMULTITERM_H
#define SYNTHESIS_SIIMAGESTOREMULTITERM_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <casa/BasicSL/Constants.h>

#include <synthesis/ImagerObjects/SIImageStore.h>

namespace casa { //# NAMESPACE CASA - BEGIN


  class SIImageStoreMultiTerm: public SIImageStore 
{
 public:
  // Default constructor

  SIImageStoreMultiTerm();
  SIImageStoreMultiTerm(casacore::String imagename, casacore::uInt ntaylorterms=1, const casacore::Bool ignorefacets=false);
  SIImageStoreMultiTerm(casacore::String imagename, casacore::CoordinateSystem &imcoordsys, 
			casacore::IPosition imshape, const int nfacets, 
			const casacore::Bool overwrite=false, casacore::uInt ntaylorterms=1,casacore::Bool useweightimage=false);

  /*
  SIImageStoreMultiTerm(casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >modelims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >residims,
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >psfims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >weightims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >restoredims,
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >sumwtims, 
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newmask,
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newalpha,
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newbeta);
  */

 SIImageStoreMultiTerm(casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >modelims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >residims,
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >psfims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >weightims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >restoredims,
			casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Float> > >sumwtims, 
			casacore::Block<SHARED_PTR<casacore::ImageInterface<float> > >pbims,
			casacore::Block<SHARED_PTR<casacore::ImageInterface<float> > >restoredpbcorims,
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newmask,
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newalpha,
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newbeta,
			SHARED_PTR<casacore::ImageInterface<casacore::Float> > newalphaerror,
			SHARED_PTR<casacore::ImageInterface<float> > newalphapbcor,
			SHARED_PTR<casacore::ImageInterface<float> > newbetapbcor,
		       casacore::CoordinateSystem& csys, 
		       casacore::IPosition imshape, 
		       casacore::String imagename, 
		       const casacore::Int facet=0, const casacore::Int nfacets=1,
		       const casacore::Int chan=0, const casacore::Int nchanchunks=1,
		       const casacore::Int pol=0, const casacore::Int npolchunks=1);
  
  virtual void init();

  virtual ~SIImageStoreMultiTerm();

  virtual casacore::String getType(){return "multiterm";}

  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > psf(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > residual(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > weight(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > model(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > image(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<float> > pb(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<float> > imagepbcor(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > sumwt(casacore::uInt term=0);
  ///   virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > mask(casacore::uInt term=0); // take from base class.
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Complex> > forwardGrid(casacore::uInt term=0);
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Complex> > backwardGrid(casacore::uInt term=0);

  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > alpha();
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > beta();
  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > alphaerror();
  virtual SHARED_PTR<casacore::ImageInterface<float> > alphapbcor();
  virtual SHARED_PTR<casacore::ImageInterface<float> > betapbcor();

  //  virtual SHARED_PTR<casacore::ImageInterface<casacore::Float> > gridwt(casacore::uInt term=0);

  void setModelImage( casacore::Vector<casacore::String> modelnames );
  casacore::Vector<casacore::String> getModelImageName();

  casacore::Bool doesImageExist(casacore::String imagename);

  void resetImages( casacore::Bool resetpsf, casacore::Bool resetresidual, casacore::Bool resetweight );
  void addImages( SHARED_PTR<SIImageStore> imagestoadd, 
		  casacore::Bool addpsf, casacore::Bool addresidual, casacore::Bool addweight, casacore::Bool adddensity);

  void dividePSFByWeight(const casacore::Float pblimit=casacore::C::minfloat);
  void normalizePrimaryBeam(const float pblimit=casacore::C::minfloat);

  void divideResidualByWeight(const casacore::Float pblimit=casacore::C::minfloat, const casacore::String normtype="flatnoise");
  //  void divideSensitivityPatternByWeight();
  void divideModelByWeight(const casacore::Float pblimit=casacore::C::minfloat, const casacore::String normtype="flatnoise");
  void multiplyModelByWeight(const casacore::Float pblimit=casacore::C::minfloat, const casacore::String normtype="flatnoise");

  /*
  casacore::Bool checkValidity(const casacore::Bool ipsf, const casacore::Bool iresidual, const casacore::Bool iweight, 
		     const casacore::Bool imodel, const casacore::Bool irestored, const casacore::Bool imask=false, 
		     const casacore::Bool isumwt=true, const casacore::Bool ialpha=false, const casacore::Bool ibeta=false);
  */

  casacore::Bool releaseLocks();
  casacore::Bool releaseComplexGrids();

  virtual casacore::Double getReferenceFrequency();

  casacore::uInt getNTaylorTerms(casacore::Bool dopsf=false);  // {return dopsf ? (2*itsNTerms-1) : itsNTerms;};

  void restore(casacore::GaussianBeam& rbeam, casacore::String& usebeam,casacore::uInt term=0 );
  void calculateAlphaBeta(casacore::String imtype);
  void pbcor();

  /*
  SHARED_PTR<SIImageStore> getFacetImageStore(const casacore::Int facet, const casacore::Int nfacets);
  SHARED_PTR<SIImageStore> getSubImageStoreOld(const casacore::Int chan, const casacore::Bool onechan, 
					    const casacore::Int pol, const casacore::Bool onepol);
  */

  SHARED_PTR<SIImageStore> getSubImageStore(const casacore::Int facet=0, const casacore::Int nfacets=1, 
					    const casacore::Int chan=0, const casacore::Int nchanchunks=1, 
					    const casacore::Int pol=0, const casacore::Int npolchunks=1);
  
  
  virtual casacore::Bool hasSensitivity(){return itsWeights.nelements()>0 && itsWeights[0];}
  //  virtual casacore::Bool hasModel() {return itsModels.nelements()>0 && itsModels[0];}
  virtual casacore::Bool hasModel() {return doesImageExist(itsImageName+imageExts(MODEL)+casacore::String(".tt0"));}
  virtual casacore::Bool hasPsf(){return itsPsfs.nelements()>0 && itsPsfs[0];}
  virtual casacore::Bool hasResidual() {return itsResiduals.nelements()>0 && itsResiduals[0];}
  virtual casacore::Bool hasSumWt() {return itsSumWts.nelements()>0 && itsSumWts[0];}
  

  //  virtual Bool hasSensitivity() {return doesImageExist(itsImageName+imageExts(WEIGHT)+String(".tt0"));}
  //  virtual Bool hasModel() {return doesImageExist(itsImageName+imageExts(MODEL)+String(".tt0"));}
  //  virtual Bool hasMask() {return doesImageExist(itsImageName+imageExts(MASK)+String(".tt0"));}
  //  virtual Bool hasPsf() {return doesImageExist(itsImageName+imageExts(PSF)+String(".tt0"));}
  virtual bool hasResidualImage() {return doesImageExist(itsImageName+imageExts(RESIDUAL)+casacore::String(".tt0"));}
  //  virtual Bool hasSumWt() {return doesImageExist(itsImageName+imageExts(SUMWT)+String(".tt0"));}
  virtual bool hasRestored() {return doesImageExist(itsImageName+imageExts(IMAGE)+casacore::String(".tt0"));}
  virtual bool hasPB() {return doesImageExist(itsImageName+imageExts(PB)+casacore::String(".tt0"));}

  //  {return ( itsParentSumWts.nelements()==0 || itsParentSumWts[0].null() ) ? false : getUseWeightImage( *(itsParentSumWts[0]) ); };

  void calcSensitivity();
  virtual casacore::Bool isModelEmpty();

  casacore::Double calcFractionalBandwidth();

protected:

  virtual void printImageStats();

private:

  casacore::uInt itsNTerms;

  casacore::Block<SHARED_PTR<casacore::ImageInterface<float> > > itsPsfs, itsModels, itsResiduals, itsWeights, itsImages, itsSumWts, itsImagePBcors, itsPBs;
  casacore::Block<SHARED_PTR<casacore::ImageInterface<casacore::Complex> > > itsForwardGrids, itsBackwardGrids;
  SHARED_PTR<casacore::ImageInterface<float> > itsAlpha, itsBeta, itsAlphaError, itsAlphaPBcor, itsBetaPBcor;

  casacore::Block<SHARED_PTR<casacore::ImageInterface<float> > > itsParentPsfs, itsParentModels, itsParentResiduals, itsParentWeights, itsParentImages, itsParentSumWts, itsParentPBs, itsParentImagePBcors;

};


} //# NAMESPACE CASA - END

#endif

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
  SIImageStoreMultiTerm(String imagename, uInt ntaylorterms=1, const Bool ignorefacets=False);
  SIImageStoreMultiTerm(String imagename, CoordinateSystem &imcoordsys, 
			IPosition imshape, const int nfacets, 
			const Bool overwrite=False, uInt ntaylorterms=1,Bool useweightimage=False);

  /*
  SIImageStoreMultiTerm(Block<CountedPtr<ImageInterface<Float> > >modelims, 
			Block<CountedPtr<ImageInterface<Float> > >residims,
			Block<CountedPtr<ImageInterface<Float> > >psfims, 
			Block<CountedPtr<ImageInterface<Float> > >weightims, 
			Block<CountedPtr<ImageInterface<Float> > >restoredims,
			Block<CountedPtr<ImageInterface<Float> > >sumwtims, 
			CountedPtr<ImageInterface<Float> > newmask,
			CountedPtr<ImageInterface<Float> > newalpha,
			CountedPtr<ImageInterface<Float> > newbeta);
  */

 SIImageStoreMultiTerm(Block<CountedPtr<ImageInterface<Float> > >modelims, 
			Block<CountedPtr<ImageInterface<Float> > >residims,
			Block<CountedPtr<ImageInterface<Float> > >psfims, 
			Block<CountedPtr<ImageInterface<Float> > >weightims, 
			Block<CountedPtr<ImageInterface<Float> > >restoredims,
			Block<CountedPtr<ImageInterface<Float> > >sumwtims, 
			CountedPtr<ImageInterface<Float> > newmask,
			CountedPtr<ImageInterface<Float> > newalpha,
			CountedPtr<ImageInterface<Float> > newbeta,
		       CoordinateSystem& csys, 
		       IPosition imshape, 
		       String imagename, 
		       const Int facet=0, const Int nfacets=1,
		       const Int chan=0, const Int nchanchunks=1,
		       const Int pol=0, const Int npolchunks=1);
  
  virtual void init();

  virtual ~SIImageStoreMultiTerm();

  virtual String getType(){return "multiterm";}

  virtual CountedPtr<ImageInterface<Float> > psf(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > residual(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > weight(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > model(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > image(uInt term=0);
  virtual CountedPtr<ImageInterface<Float> > sumwt(uInt term=0);
  ///   virtual CountedPtr<ImageInterface<Float> > mask(uInt term=0); // take from base class.
  virtual CountedPtr<ImageInterface<Complex> > forwardGrid(uInt term=0);
  virtual CountedPtr<ImageInterface<Complex> > backwardGrid(uInt term=0);

  virtual CountedPtr<ImageInterface<Float> > alpha();
  virtual CountedPtr<ImageInterface<Float> > beta();

  void setModelImage( String modelname );

  Bool doesImageExist(String imagename);

  void resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight );
  void addImages( CountedPtr<SIImageStore> imagestoadd, 
		  Bool addpsf, Bool addresidual, Bool addweight );

  void dividePSFByWeight();

  void divideResidualByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  //  void divideSensitivityPatternByWeight();
  void divideModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");
  void multiplyModelByWeight(const Float pblimit=C::minfloat, const String normtype="flatnoise");

  /*
  Bool checkValidity(const Bool ipsf, const Bool iresidual, const Bool iweight, 
		     const Bool imodel, const Bool irestored, const Bool imask=False, 
		     const Bool isumwt=True, const Bool ialpha=False, const Bool ibeta=False);
  */

  Bool releaseLocks();

  virtual Double getReferenceFrequency();

  uInt getNTaylorTerms(Bool dopsf=False);  // {return dopsf ? (2*itsNTerms-1) : itsNTerms;};

  void restore(GaussianBeam& rbeam, String& usebeam,uInt term=0 );
  GaussianBeam restorePlane();
  void pbcorPlane();

  /*
  CountedPtr<SIImageStore> getFacetImageStore(const Int facet, const Int nfacets);
  CountedPtr<SIImageStore> getSubImageStoreOld(const Int chan, const Bool onechan, 
					    const Int pol, const Bool onepol);
  */

  CountedPtr<SIImageStore> getSubImageStore(const Int facet=0, const Int nfacets=1, 
					    const Int chan=0, const Int nchanchunks=1, 
					    const Int pol=0, const Int npolchunks=1);
  

  virtual Bool hasSensitivity(){return itsWeights.nelements()>0 && !itsWeights[0].null();}
  virtual Bool hasModel() {return itsModels.nelements()>0 && !itsModels[0].null();}
  virtual Bool hasPsf(){return itsPsfs.nelements()>0 && !itsPsfs[0].null();}
  virtual Bool hasResidual() {return itsResiduals.nelements()>0 && !itsResiduals[0].null();}
  virtual Bool hasSumWt() {return itsSumWts.nelements()>0 && !itsSumWts[0].null();}

  //  virtual Bool getUseWeightImage();
  //  {return ( itsParentSumWts.nelements()==0 || itsParentSumWts[0].null() ) ? False : getUseWeightImage( *(itsParentSumWts[0]) ); };

  void calcSensitivity();

protected:

  Bool createMask(LatticeExpr<Bool> &lemask, CountedPtr<ImageInterface<Float> >outimage);

private:

  uInt itsNTerms;

  Block<CountedPtr<ImageInterface<Float> > > itsPsfs, itsModels, itsResiduals, itsWeights, itsImages, itsSumWts;
  Block<CountedPtr<ImageInterface<Complex> > > itsForwardGrids, itsBackwardGrids;
  CountedPtr<ImageInterface<Float> > itsAlpha, itsBeta;

  Block<CountedPtr<ImageInterface<Float> > > itsParentPsfs, itsParentModels, itsParentResiduals, itsParentWeights, itsParentImages, itsParentSumWts;

};


} //# NAMESPACE CASA - END

#endif

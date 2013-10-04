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
	       IPosition imshape, const Bool overwrite=False);
  // Contructor by image objects necessary for facetted imaging (subimages 
  //can be passed in as residual and model etc). The caller has to make sure the 
  //images are similar.  the Pointers are taken over by CountedPtr...and will be deleted when reference 
  //count goes to 0
  SIImageStore(ImageInterface<Float>* modelim, ImageInterface<Float>* residim,
	       ImageInterface<Float>* psfim, ImageInterface<Float>* weightim, ImageInterface<Float>* restoredim);
    

  virtual ~SIImageStore();


  IPosition getShape();
  String getName();

  CountedPtr<ImageInterface<Float> > psf();
  CountedPtr<ImageInterface<Float> > residual();
  CountedPtr<ImageInterface<Float> > weight();
  CountedPtr<ImageInterface<Float> > model();
  CountedPtr<ImageInterface<Float> > image();
  CountedPtr<ImageInterface<Complex> > forwardGrid();
  CountedPtr<ImageInterface<Complex> > backwardGrid();

  void setModelImage( String modelname );

  Bool hasWeight(){return itsWeightExists;};

  Bool doImagesExist();
  Bool doesImageExist(String imagename);

  void allocateRestoredImage();

  void resetImages( Bool resetpsf, Bool resetresidual, Bool resetweight );
  void addImages( CountedPtr<SIImageStore> imagestoadd, 
		  Bool addpsf, Bool addresidual, Bool addweight );

  void divideResidualByWeight(const Float weightlimit=C::minfloat);
  void dividePSFByWeight(const Float weightlimit=C::minfloat);
  void divideModelByWeight(const Float weightlimit=C::minfloat);

  Bool isValid(){return itsValidity;}

  Bool releaseLocks();

  // Get a SIImageStore of a given facet..the caller has to delete it
  // The images internall will reference back to a given section of the main of this.
  //nfacets = nx_facets*ny_facets...assumption has been made  nx_facets==ny_facets
  SIImageStore * getFacetImageStore(const Int facet, const Int nfacets);

protected:
// Can make this a utility function elsewhere...
//nfacets = nx_facets*ny_facets...assumption has been made  nx_facets==ny_facets
static SubImage<Float>* makeFacet(const Int facet, const Int nfacets, ImageInterface<Float>& image);
Double memoryBeforeLattice();
IPosition tileShape();
  ///////////////////// Member Objects

  IPosition itsImageShape;
  String itsImageName;
  CountedPtr<ImageInterface<Float> > itsPsf, itsModel, itsResidual, itsWeight, itsImage;
  CountedPtr<ImageInterface<Complex> > itsForwardGrid, itsBackwardGrid;
  Bool itsWeightExists;

  Bool itsPsfNormed, itsResNormed;

  Bool itsValidity;

};


} //# NAMESPACE CASA - END

#endif

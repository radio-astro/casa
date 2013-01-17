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
#define SYNTHESIS_SIMAGESTORE_H

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

namespace casa { //# NAMESPACE CASA - BEGIN

class SIImageStore 
{
 public:
  // Default constructor

  SIImageStore();
  SIImageStore(String imagename);
  SIImageStore(String imagename, 
	       CountedPtr<CoordinateSystem> imcoordsys, 
	       IPosition imshape);

  ~SIImageStore();


  IPosition getShape();
  String getName();

  CountedPtr<PagedImage<Float> > psf();
  CountedPtr<PagedImage<Float> > residual();
  CountedPtr<PagedImage<Float> > weight();
  CountedPtr<PagedImage<Float> > model();
  CountedPtr<PagedImage<Float> > image();

  Bool doImagesExist();
  Bool doesModelImageExist();

  void allocateRestoredImage();

  void normalizeByWeight();

protected:

  ///////////////////// Member Objects

  IPosition itsImageShape;
  String itsImageName;
  CountedPtr<PagedImage<Float> > itsPsf, itsModel, itsResidual, itsWeight, itsImage;
  

};


} //# NAMESPACE CASA - END

#endif

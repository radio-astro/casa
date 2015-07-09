//# SynthesisNormalizer.h: Gather and Scatter operations for parallel major cycles.
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

#ifndef SYNTHESIS_SYNTHESISPARSYNC_H
#define SYNTHESIS_SYNTHESISPARSYNC_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include<synthesis/ImagerObjects/SDAlgorithmBase.h>
#include<synthesis/ImagerObjects/SDAlgorithmHogbomClean.h>
#include<synthesis/ImagerObjects/SDMaskHandler.h>
#include <synthesis/ImagerObjects/SIMinorCycleController.h>

#include <synthesis/ImagerObjects/SIImageStore.h>
#include <synthesis/ImagerObjects/SIImageStoreMultiTerm.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class MeasurementSet;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class SynthesisNormalizer 
{
 public:
  // Default constructor

  SynthesisNormalizer();
  ~SynthesisNormalizer();

  // Copy constructor and assignment operator

  // make all pure-inputs const
  void setupNormalizer(Record normpars);

  // Gather all part images to the 'full' one
  void gatherImages(Bool dopsf, Bool doresidual, Bool dodensity);

  // Copy out model to all pieces. Currently a No-Op.
  void scatterModel();

  // Gather all part gridded weights and add them up.
  void gatherWeightDensity();
  // Scatter summed gridded weights to all parts
  void scatterWeightDensity();

  SHARED_PTR<SIImageStore> getImageStore();
  void setImageStore( SIImageStore* imstore );

  void divideResidualByWeight();
  void dividePSFByWeight();
  void divideModelByWeight();
  void multiplyModelByWeight();

protected:

 // Normalize. This can later change to be more general, i.e. used for PB-correction too...
  // Check if images exist on disk and are all the same shape
  Bool setupImagesOnDisk();
  Bool doImagesExist( String imagename );

  SHARED_PTR<SIImageStore> makeImageStore( String imagename );
  SHARED_PTR<SIImageStore> makeImageStore( String imagename, CoordinateSystem& csys, IPosition shp, Bool useweightimage );

  void setPsfFromOneFacet();

  /////////////// Member Objects

  SHARED_PTR<SIImageStore> itsImages;
  Vector<SHARED_PTR<SIImageStore> > itsPartImages;
  Block<SHARED_PTR<SIImageStore> > itsFacetImageStores;

  IPosition itsImageShape;
  
  String itsImageName;
  Vector<String> itsPartImageNames;
  String itsStartingModelName;
  Float itsPBLimit;

  String itsMapperType;
  uInt itsNTaylorTerms, itsNFacets;

  String itsNormType;

};


} //# NAMESPACE CASA - END

#endif

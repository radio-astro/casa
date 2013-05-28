//# SIMapper.h: Imager functionality sits here; 
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

#ifndef SYNTHESIS_SIMAPPERBASE_H
#define SYNTHESIS_SIMAPPERBASE_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/ImagerObjects/SIImageStore.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class SIMapperBase 
{
 public:
  // Default constructor

  SIMapperBase( CountedPtr<SIImageStore> imagestore, 
            CountedPtr<FTMachine> ftmachine, 
            Int mapperid);
  virtual ~SIMapperBase();

  ///// Major Cycle Functions

  // For KG : Need to add 'vb' coming into these functions.
  // Would be good to make these pure virtual, but need to keep base-class implementation for initial testing.
  virtual void initializeGrid();
  virtual void grid();
  virtual void finalizeGrid();

  virtual void initializeDegrid();
  virtual void degrid();
  virtual void finalizeDegrid();

  virtual Record getFTMRecord();

  String getImageName();

  Bool releaseImageLocks();

protected:
  //////////////////// Member Functions

  void initImages();
  
  ///////////////////// Member Objects

  CountedPtr<FTMachine> itsFTMachine, itsInverseFTMachine; 

  CountedPtr<CoordinateSystem> itsCoordSys;
  IPosition itsImageShape;

  CountedPtr<SIImageStore> itsImages;

  /*
  String itsImageName;
  CountedPtr<PagedImage<Float> > itsPsf, itsModel, itsResidual, itsWeight;
  */  

  // This is only for testing. In the real-world, this is the data....
  Array<Float> itsOriginalResidual;

  // These are supporting params
  Bool itsIsModelUpdated;
  Int itsMapperId;

  /////////////////// All input parameters

};


} //# NAMESPACE CASA - END

#endif

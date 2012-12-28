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

#ifndef SYNTHESIS_SIMAPPER_H
#define SYNTHESIS_SIMAPPER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/MeasurementComponents/SIDeconvolver.h>
#include <synthesis/MeasurementEquations/SIMaskHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
template<class T> class ImageInterface;
 class SISubIterBot;

// <summary> Class that contains functions needed for imager </summary>

class SIMapper 
{
 public:
  // Default constructor

  SIMapper( String imagename, CountedPtr<FTMachine> ftmachine, CountedPtr<SIDeconvolver> deconvolver, CountedPtr<CoordinateSystem> imcoordsys, IPosition imshape, CountedPtr<SIMaskHandler> maskhandler, Int mapperid);
  ~SIMapper();

  // Copy constructor and assignment operator
  //Imager(const Imager&);
  //Imager& operator=(const Imager&);

  ///// Minor Cycle Functions

  void getCopyOfResidualAndMask( TempImage<Float> &residual, TempImage<Float> &mask );
  void setMask( TempImage<Float> &mask );

  void deconvolve( SISubIterBot &loopcontrols );

  Float getPeakResidual();
  Float getPSFSidelobeLevel();
  Float getModelFlux();
  Bool isModelUpdated();

  void restore();

  ///// Major Cycle Functions

  void initializeGrid();
  void grid();
  void finalizeGrid();

  void initializeDegrid();
  void degrid();
  void finalizeDegrid();

  Record getFTMRecord();

protected:

  ///////////////////// Member Objects

  CountedPtr<SIDeconvolver> itsDeconvolver;
  CountedPtr<FTMachine> itsFTMachine; 
  CountedPtr<CoordinateSystem> itsCoordSys;
  IPosition itsImageShape;
  CountedPtr<SIMaskHandler> itsMaskHandler;

  // These are images
  String itsImageName;

  CountedPtr<PagedImage<Float> > itsImage, itsPsf, itsModel, itsResidual, itsWeight;
  Float itsBeam;
  Vector<Slicer> decSlices, ftmSlices; 
  
  // This is only for testing. In the real-world, this is the data....
  Array<Float> itsOriginalResidual;

  // These are supporting params
  Bool itsIsModelUpdated;
  Int itsMapperId;

  //////////////////// Member Functions
  
  void allocateImageMemory();

  // For the current deconvolver, decide how many sliced deconvolution calls to make
  void partitionImages();

  /////////////////// All input parameters

};


} //# NAMESPACE CASA - END

#endif

//# SynthesisDeconvolver.h: Imager functionality sits here; 
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

#ifndef SYNTHESIS_SYNTHESISDECONVOLVER_H
#define SYNTHESIS_SYNTHESISDECONVOLVER_H

#include <casa/aips.h>
#include <casa/OS/Timer.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>

#include<synthesis/ImagerObjects/SDAlgorithmBase.h>
#include<synthesis/ImagerObjects/SDAlgorithmHogbomClean.h>
#include<synthesis/ImagerObjects/SDAlgorithmClarkClean.h>
#include<synthesis/ImagerObjects/SDAlgorithmMSMFS.h>
#include<synthesis/ImagerObjects/SDAlgorithmMSClean.h>
#include<synthesis/ImagerObjects/SDAlgorithmMEM.h>

#include<synthesis/ImagerObjects/SDMaskHandler.h>
#include <synthesis/ImagerObjects/SIMinorCycleController.h>

#include<synthesis/ImagerObjects/SynthesisUtilMethods.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations
class MeasurementSet;
template<class T> class ImageInterface;

// <summary> Class that contains functions needed for imager </summary>

class SynthesisDeconvolver 
{
 public:
  // Default constructor

  SynthesisDeconvolver();
  ~SynthesisDeconvolver();

  // Copy constructor and assignment operator

  // make all pure-inputs const

  void setupDeconvolution(const SynthesisParamsDeconv& decpars);

  //  void setupDeconvolution(Record recpars);

  Record initMinorCycle();
  Record executeMinorCycle(Record& subIterBot);

  Record interactiveGUI(Record& iterRec);

  // Helpers
  /*
  Float getPeakResidual();
  Float getModelFlux();
  Float getPSFSidelobeLevel();
  */
  // Restoration (and post-restoration PB-correction)
  void restore();
  void pbcor();// maybe add a way to take in arbitrary PBs here.

  // For interaction
  void getCopyOfResidualAndMask( TempImage<Float> &/*residual*/, TempImage<Float>& /*mask*/ );
  void setMask( TempImage<Float> &/*mask*/ );

  void setStartingModel();

protected:

  CountedPtr<SIImageStore> makeImageStore( String imagename );
  /*
  void findMinMax(const Array<Float>& lattice,
					const Array<Float>& mask,
					Float& minVal, Float& maxVal,
					Float& minValMask, Float& maxValMask);

  void printImageStats();
  */

  // Gather all part images to the 'full' one
  //void gatherImages();
  //void scatterModel();


  // For the deconvolver, decide how many sliced deconvolution calls to make
  //  Vector<Slicer> partitionImages();

  // Check if images exist on disk and are all the same shape
  //Bool setupImagesOnDisk();
  // Bool doImagesExist( String imagename );

  /////////////// Member Objects

  CountedPtr<SDAlgorithmBase> itsDeconvolver;
  CountedPtr<SDMaskHandler> itsMaskHandler;

  CountedPtr<SIImageStore> itsImages;

  IPosition itsImageShape;
  
  String itsImageName;
  String itsStartingModelName;
  Bool itsAddedModel;


  Float itsBeam;

  SIMinorCycleController itsLoopController;

  /////////////// All input parameters

  uInt itsDeconvolverId;
  Vector<Float> itsScales;
 
};


} //# NAMESPACE CASA - END

#endif

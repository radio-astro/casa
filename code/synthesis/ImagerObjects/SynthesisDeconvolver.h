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
#include<synthesis/ImagerObjects/SDAlgorithmClarkClean2.h>
#include<synthesis/ImagerObjects/SDAlgorithmMSMFS.h>
#include<synthesis/ImagerObjects/SDAlgorithmMSClean.h>
#include<synthesis/ImagerObjects/SDAlgorithmMEM.h>
#include<synthesis/ImagerObjects/SDAlgorithmAAspClean.h>

#include<synthesis/ImagerObjects/SDMaskHandler.h>
#include <synthesis/ImagerObjects/SIMinorCycleController.h>

#include<synthesis/ImagerObjects/SynthesisUtilMethods.h>

namespace casacore{

class MeasurementSet;
template<class T> class ImageInterface;
}

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declarations

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

  //  void setupDeconvolution(casacore::Record recpars);

  casacore::Record initMinorCycle();
  casacore::Record executeMinorCycle(casacore::Record& subIterBot);

  casacore::Record interactiveGUI(casacore::Record& iterRec);

  // Helpers
  /*
  casacore::Float getPeakResidual();
  casacore::Float getModelFlux();
  casacore::Float getPSFSidelobeLevel();
  */
  // Restoration (and post-restoration PB-correction)
  void restore();
  void pbcor();// maybe add a way to take in arbitrary PBs here.

  // For interaction
  void getCopyOfResidualAndMask( casacore::TempImage<casacore::Float> &/*residual*/, casacore::TempImage<casacore::Float>& /*mask*/ );
  void setMask( casacore::TempImage<casacore::Float> &/*mask*/ );

  void setStartingModel();
  void setupMask();
  void setAutoMask();

protected:

  SHARED_PTR<SIImageStore> makeImageStore( casacore::String imagename );
  /*
  void findMinMax(const casacore::Array<casacore::Float>& lattice,
					const casacore::Array<casacore::Float>& mask,
					casacore::Float& minVal, casacore::Float& maxVal,
					casacore::Float& minValMask, casacore::Float& maxValMask);

  void printImageStats();
  */

  // Gather all part images to the 'full' one
  //void gatherImages();
  //void scatterModel();


  // For the deconvolver, decide how many sliced deconvolution calls to make
  //  casacore::Vector<casacore::Slicer> partitionImages();

  // Check if images exist on disk and are all the same shape
  //casacore::Bool setupImagesOnDisk();
  // casacore::Bool doImagesExist( casacore::String imagename );

  /////////////// Member Objects

  SHARED_PTR<SDAlgorithmBase> itsDeconvolver;
  SHARED_PTR<SDMaskHandler> itsMaskHandler;

  SHARED_PTR<SIImageStore> itsImages;

  casacore::IPosition itsImageShape;
  
  casacore::String itsImageName;
  casacore::Vector<casacore::String> itsStartingModelNames;
  casacore::Bool itsAddedModel;


  casacore::Float itsBeam;

  SIMinorCycleController itsLoopController;

  /////////////// All input parameters

  casacore::uInt itsDeconvolverId;
  casacore::Vector<casacore::Float> itsScales;

  casacore::String itsMaskType;
  casacore::Vector<casacore::String> itsMaskList;
  casacore::String itsMaskString;
  casacore::Float itsPBMask;
  casacore::String itsAutoMaskAlgorithm;
  casacore::String itsMaskThreshold;
  casacore::Float itsFracOfPeak;
  casacore::String itsMaskResolution;
  casacore::Float itsMaskResByBeam;
  casacore::Int itsNMask;
  casacore::Bool itsAutoAdjust;
  casacore::Bool itsIsMaskLoaded; // Try to get rid of this state variable ! 
  casacore::Bool itsIsInteractive;
 
};


} //# NAMESPACE CASA - END

#endif

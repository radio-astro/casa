//# SDAlgorithmBase.h: Definition for SDAlgorithmBase
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_SDALGORITHMBASE_H
#define SYNTHESIS_SDALGORITHMBASE_H

#include <ms/MeasurementSets/MeasurementSet.h>
//#include <synthesis/MeasurementComponents/SkyModel.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/PGPlotter.h>

#include <casa/aips.h>
#include <images/Images/ImageInterface.h>
#include <components/ComponentModels/ComponentList.h>
#include <casa/BasicSL/String.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>

#include<synthesis/ImagerObjects/SDMaskHandler.h>
#include<synthesis/ImagerObjects/SIImageStore.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  /* Forware Declaration */
  class SIMinorCycleController;


class SDAlgorithmBase {
public:

  // Empty constructor
  SDAlgorithmBase();
 virtual  ~SDAlgorithmBase();

  // Non virtual. Wrapper function implemented only in the base class. 
  void deconvolve( SIMinorCycleController& loopController,  
		   CountedPtr<SIImageStore> &imagestore,
		   Int deconvolverid);


  // Base Class contains standard restoration. Overload for more complex behaviour.
  void restore( CountedPtr<SIImageStore> imagestore );

protected:

  // Pure virtual functions to be implemented by various algorithm deconvolvers.
  virtual void takeOneStep( Float loopgain, Int cycleNiter, Float cycleThreshold, Float &peakresidual, Float &modelflux, Int& iterdone )=0;
  virtual void initializeDeconvolver( Float &peakresidual, Float &modelflux )=0;
  virtual void finalizeDeconvolver()=0;

  // Base Class implements the option of single-plane images for the minor cycle.
  virtual void queryDesiredShape(Bool &onechan, Bool &onepol); // , nImageFacets.
  virtual void restorePlane();

  // Non virtual. Implemented only in the base class.
  Bool checkStop( SIMinorCycleController &loopcontrols, Float currentresidual );
  void partitionImages( CountedPtr<SIImageStore> &imagestore );
  void initializeSubImages( CountedPtr<SIImageStore> &imagestore, uInt subim);
  Bool findMaxAbs(const Matrix<Float>& lattice,Float& maxAbs,IPosition& posMaxAbs);
  GaussianBeam getPSFGaussian();

  // Algorithm name
  String itsAlgorithmName;

  // For debugging.
  IPosition tmpPos_p;

  // Image Store
  //  CountedPtr<SIImageStore> itsImages;
  Vector<Slicer> itsDecSlices;
  SubImage<Float> itsResidual, itsPsf, itsModel, itsImage;
  Float itsComp;

};

} //# NAMESPACE CASA - END

#endif

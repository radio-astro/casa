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
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/PGPlotter.h>

#include<synthesis/MeasurementEquations/SDMaskHandler.h>
#include<synthesis/MeasurementEquations/SIImageStore.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  /* Forware Declaration */
  class SIMinorCycleController;


class SDAlgorithmBase {
public:

  // Empty constructor
  SDAlgorithmBase();
 virtual  ~SDAlgorithmBase();

  // In the base class. Non virtual.
  void deconvolve( SIMinorCycleController& loopController,  
		   CountedPtr<SIImageStore> &imagestore,
		   Int deconvolverid);


  virtual void restore( CountedPtr<SIImageStore> imagestore );

protected:

  // Local functions to be overloaded by various algorithm deconvolvers.
  virtual void findNextComponent( Float loopgain );
  virtual void updateModel();
  virtual void updateResidual();
  virtual Float getPeakResidual();
  virtual Float getIntegratedFlux();
  virtual void queryDesiredShape(Bool &onechan, Bool &onepol); // , nImageFacets.

  // Non virtual. Implemented only in the base class.
  Bool checkStop( SIMinorCycleController &loopcontrols, Float currentresidual );
  void partitionImages();
  void initializeSubImages(uInt subim);

  // For debugging.
  IPosition tmpPos_p;

  // Image Store
  CountedPtr<SIImageStore> itsImages;
  Vector<Slicer> itsDecSlices;
  SubImage<Float> itsResidual, itsPsf, itsModel;
  Float itsComp;

};

} //# NAMESPACE CASA - END

#endif

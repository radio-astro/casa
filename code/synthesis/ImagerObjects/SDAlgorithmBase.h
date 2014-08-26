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
#include<synthesis/ImagerObjects/SIImageStoreMultiTerm.h>

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

  void setRestoringBeam( GaussianBeam restbeam, String usebeam );
  //  void setMaskOptions( String maskstring );

  //  void loadMask();

  // Base Class contains standard restoration. Overload for more complex behaviour.
  virtual void restore( CountedPtr<SIImageStore> imagestore );
  virtual void pbcor( CountedPtr<SIImageStore> imagestore );

  virtual String getAlgorithmName(){return itsAlgorithmName;};

  virtual uInt getNTaylorTerms(){return 1;};

protected:

  // Pure virtual functions to be implemented by various algorithm deconvolvers.
  virtual void takeOneStep( Float loopgain, Int cycleNiter, Float cycleThreshold, 
			    Float &peakresidual, Float &modelflux, Int& iterdone )=0;
  virtual void initializeDeconvolver( Float &peakresidual, Float &modelflux )=0;
  virtual void finalizeDeconvolver()=0;

  // Base Class implements the option of single-plane images for the minor cycle.
  virtual void queryDesiredShape(Int &nchanchunks, Int& npolchunks, IPosition imshape);
  //  virtual void restorePlane();

  // Non virtual. Implemented only in the base class.
  Int checkStop( SIMinorCycleController &loopcontrols, Float currentresidual );
  Bool findMaxAbs(const Array<Float>& lattice,Float& maxAbs,IPosition& posMaxAbs);
  Bool findMaxAbsMask(const Array<Float>& lattice,const Array<Float>& mask,
		      Float& maxAbs,IPosition& posMaxAbs);

  // Algorithm name
  String itsAlgorithmName;

  CountedPtr<SIImageStore> itsImages; //sOriginalImages;

  //    Vector<Slicer> itsDecSlices;
  //   SubImage<Float> itsResidual, itsPsf, itsModel, itsImage;
  
  IPosition itsMaxPos;
  Float itsPeakResidual;
  Float itsModelFlux;

  //  SDMaskHandler itsMaskHandler;
  Array<Float> itsMatMask;

  GaussianBeam itsRestoringBeam;
  String itsUseBeam;
  //  String itsMaskString;
  //  Bool itsIsMaskLoaded; // Annoying state variable. Remove if possible. 

};

} //# NAMESPACE CASA - END

#endif

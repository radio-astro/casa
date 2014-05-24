//# SDAlgorithmMSClean.h: Definition for SDAlgorithmMSClean
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

#ifndef SYNTHESIS_SDALGORITHMMSCLEAN_H
#define SYNTHESIS_SDALGORITHMMSCLEAN_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/PGPlotter.h>

#include<synthesis/ImagerObjects/SDAlgorithmBase.h>
#include <synthesis/MeasurementEquations/MatrixCleaner.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  /* Forware Declaration */
  class SIMinorCycleController;


  class SDAlgorithmMSClean : public SDAlgorithmBase 
  {
  public:
    
    // Empty constructor
    SDAlgorithmMSClean(Vector<Float> scalesizes,
		       Float smallscalebias=0.6, 
		       // Int stoplargenegatives=-2, 
		       Int stoppointmode=-1 );
    virtual  ~SDAlgorithmMSClean();
    
    //    void restore( CountedPtr<SIImageStore> imagestore );
    
  protected:
    
    // Local functions to be overloaded by various algorithm deconvolvers.
    void takeOneStep( Float loopgain, Int cycleNiter, Float cycleThreshold, 
		      Float &peakresidual, Float &modelflux, Int &iterdone );
    void initializeDeconvolver( Float &peakresidual, Float &modelflux );
    void finalizeDeconvolver();

    Array<Float> itsMatPsf, itsMatResidual, itsMatModel;
    Array<Float> itsMatMask;  // Make an array if we eventually use multi-term masks...

    MatrixCleaner itsCleaner;
    Vector<Float> itsScaleSizes;
    Float itsSmallScaleBias;
    //Int itsStopLargeNegatives;
    Int itsStopPointMode;

  private:
    Bool itsMCsetup; 

  };

} //# NAMESPACE CASA - END

#endif

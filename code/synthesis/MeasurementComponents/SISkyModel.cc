//# SISkyModel.cc: Implementation of SISkyModel classes
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/HostInfo.h>
#include <synthesis/MeasurementComponents/SISkyModel.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <tables/Tables/TableLock.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN


SISkyModel::SISkyModel()
 {
   
 }

SISkyModel::~SISkyModel()
 {
   
 }

  void SISkyModel::init()
  {
    LogIO os( LogOrigin("SISkyModel","init",WHERE) );
    os << "Init SkyModel" << LogIO::POST;
  }


  void SISkyModel::runMinorCycle( SIMapperCollection &mappers, 
				  SIIterBot &loopcontrols )
  {
    LogIO os( LogOrigin("SISkyModel","runMinorCycle",WHERE) );
   
    Float peakResidual = mappers.findPeakResidual();

    loopcontrols.setMaxPsfSidelobe( mappers.findMaxPsfSidelobe() );
    loopcontrols.updateCycleThreshold(peakResidual);


    os << "Start Minor-Cycle iterations with peak residual = " << peakResidual;
    os << " and model flux = " << mappers.addIntegratedFlux() << LogIO::POST;

    os << " [ cyclethreshold = " << loopcontrols.getCycleThreshold() ;
    os << " max iter per field/chan/pol = " << loopcontrols.getCycleNiter() ;
    os << " loopgain = " << loopcontrols.getLoopGain() ;
    os << " ]" << LogIO::POST;



    if (loopcontrols.interactiveInputRequired(peakResidual)) {
      pauseForUserInteraction( mappers, loopcontrols );
    }

    Int startiter=0,stopiter=0;

    for(Int mp=0;mp<mappers.nMappers();mp++)
      {

	startiter = loopcontrols.getCompletedNiter();
	mappers.getMapper(mp)->deconvolve( loopcontrols );
        loopcontrols.resetCycleIter();
	stopiter = loopcontrols.getCompletedNiter();

	if( startiter != stopiter)
	  {
	    os << "Mapper " << mp << " : iterations " << startiter+1 
               << " to " << stopiter << LogIO::POST;
	  }
	else
	  {
	    os << "Mapper " << mp << " : No iterations " << LogIO::POST;
	  }

      }    


    // Get/sync peak residual and sum of flux over all fields.
    loopcontrols.setUpdatedModelFlag( mappers.anyUpdatedModel() );
    
    os << "Stopping at iteration " << loopcontrols.getCompletedNiter() 
       << " with peak residual = " << mappers.findPeakResidual() 
       << " and model flux = " << mappers.addIntegratedFlux() << LogIO::POST;  
  }


  void SISkyModel::restore( SIMapperCollection &mappers )
  {
    LogIO os( LogOrigin("SISkyModel","restore",WHERE) );

    Int nmappers = mappers.nMappers();

    os << "Restore images for all " << mappers.nMappers() << " mappers" << LogIO::POST;

    for(Int mp=0;mp<nmappers;mp++)
      {
	mappers.getMapper(mp)->restore();
      }

  }// end of restore

  void SISkyModel::pauseForUserInteraction( SIMapperCollection &mappers, 
                                            SIIterBot &loopcontrols )
  {
    LogIO os( LogOrigin("SISkyModel","pauseForUserInteraction",WHERE) );

    os << "Waiting for interactive clean feedback" << LogIO::POST;

    /* This call will make sure that the current values of loop control are
       available in the GUI and will not return until the user hits the
       button */
    loopcontrols.waitForInteractiveInput();
    
    Int nmappers = mappers.nMappers();
    for(Int mp=0;mp<nmappers;mp++)
      {
	TempImage<Float> dispresidual, dispmask;
	// Memory for these image copies are allocated inside the SIMapper
	mappers.getMapper(mp)->getCopyOfResidualAndMask( dispresidual, dispmask );

	///// Send dispresidual and dispmask to the GUI.
	///// Receive dispmask back from the GUI ( on click-to-set-mask for this field )

	mappers.getMapper(mp)->setMask( dispmask );
      }

    
  }// end of pauseForUserInteraction


} //# NAMESPACE CASA - END


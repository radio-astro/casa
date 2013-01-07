//# SynthesisIterBot.cc: Implementation of Imager.h
//# Copyright (C) 1997-2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <casa/OS/DirectoryIterator.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>

#include <casa/OS/HostInfo.h>

#include<synthesis/MeasurementEquations/SIIterBot.h>
#include <synthesis/MeasurementEquations/SynthesisIterBot.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  SynthesisIterBot::SynthesisIterBot() : itsLoopController()
  {
  }
  
  SynthesisIterBot::~SynthesisIterBot() 
  {
  }
  
  
  void SynthesisIterBot::setIterationDetails(Record iterpars)
  {
    LogIO os( LogOrigin("SynthesisIterBot","updateIterationDetails",WHERE) );
    try
      {
        itsLoopController.reset( new SIIterBot("SynthesisImage_"));
        itsLoopController->setControlsFromRecord(iterpars);
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in updating iteration parameters : " +
                         x.getMesg()) );
      }
  }

  Record SynthesisIterBot::getIterationDetails()
  {
    LogIO os( LogOrigin("SynthesisIterBot","getIterationDetails",WHERE) );
    Record returnRecord;
    try {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

      returnRecord = itsLoopController->getDetailsRecord();
    } catch(AipsError &x) {
      throw( AipsError("Error in retrieving iteration parameters : " +
                       x.getMesg()) );
    }
    return returnRecord;
  }

  Record SynthesisIterBot::getIterationSummary()
  {
    LogIO os( LogOrigin("SynthesisIterBot","getIterationSummary",WHERE) );
    Record returnRecord;
    try {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

      returnRecord = itsLoopController->getSummaryRecord();
    } catch(AipsError &x) {
      throw( AipsError("Error in retrieving iteration parameters : " +
                       x.getMesg()) );
    }
    return returnRecord;
  }


  void SynthesisIterBot::setupIteration(Record iterpars)
  {
    LogIO os( LogOrigin("SynthesisIterBot","setupIteration",WHERE) );
    os << "Set Iteration Control Options : Construct SISkyModel" << LogIO::POST;
     try
      {
        setIterationDetails(iterpars);
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing SkyModel : "+x.getMesg()) );
      }
  } //end of setupIteration
  
  void SynthesisIterBot::setInteractiveMode(Bool interactiveMode) 
  {
    LogIO os( LogOrigin("SynthesisIterBot","setupIteration",WHERE) );
    os << "Setting intractive mode to " 
       << ((interactiveMode) ? "Active" : "Inactive") << LogIO::POST;
    try {
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));
      itsLoopController->changeInteractiveMode(interactiveMode);
    } catch(AipsError &x) {
      throw( AipsError("Error Setting Interactive Mode : "+x.getMesg()) );
    }
  }
  
  
  bool SynthesisIterBot::cleanComplete() {
    bool returnValue;
    try {
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));
      Float peakResidual = itsLoopController->getPeakResidual(); // This should go..
      returnValue=itsLoopController->cleanComplete(peakResidual);
    } catch (AipsError &x) {
      throw( AipsError("Error checking clean complete state : "+x.getMesg()) );
    }
    return returnValue; 
  }

  /* /// Replaced by a direct call to 'restore' of the SynthesisDeconvolver
  void  SynthesisIterBot::endLoops()//SIIterBot& loopcontrol)
  {
    LogIO os( LogOrigin("SynthesisIterBot","endLoops",WHERE) );
    
    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

        if(itsLoopController->getUpdatedModelFlag() ==True)
	  {
	    os << "Restore Image (and normalize to Flat Sky) " << LogIO::POST;
	    itsSkyModel.restore( itsMappers );
	  }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing image coordinate system : "+
                         x.getMesg()) );
      }
  }// end of endLoops
  */


    /* This method makes all decisions about how the major cycle should 
       execute.  The output of this is passed to the runMajorCycle method
    */
  /// Not sure we need this anymore. Best to re-implement this logic in a better place.
  /*
  Record SynthesisIterBot::getMajorCycleControls()
  {
    LogIO os( LogOrigin("SynthesisIterBot","getMajorCycleControls",WHERE) );
    Record returnRecord;

    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

 	if(itsLoopController->getCompletedNiter()==0 &&
           startmodel_p.length()>1 )
 	  {
            itsLoopController->setUpdatedModelFlag(true);
 	  }
	
	if(itsLoopController->getMajorCycleCount() ==0)
	  {
	    os << "Make PSFs, weights and initial dirty/residual images. " ;
	  }
	else
	  {
	    if(! itsLoopController->getUpdatedModelFlag())
	      {
		os << "No new model. No need to update residuals in a major cycle." << LogIO::POST;
		return returnRecord;//With appropriate flag
	      }
	    os << "Update residual image in major cycle " << 
              String::toString(itsLoopController->getMajorCycleCount()) << ". ";
	  }
	
	if(itsLoopController->cleanComplete(itsMappers.findPeakResidual()) &&
           itsLoopController->getUpdatedModelFlag())
	  {
	    if(usescratch_p==True)
	      {
		os << "Save image model to MS in MODEL_DATA column on disk" 
                   << LogIO::POST;
	      }
	    else
	      {
		os << "Save image model to MS as a Record for on-the-fly prediction" << LogIO::POST;
	      }
	  }
	else
	  {
	    os << LogIO::POST;
	  }
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in setting up Major Cycle : "+x.getMesg()) );
      }
    return returnRecord;
  }
  */

  void SynthesisIterBot::endMajorCycle()
  {
    LogIO os( LogOrigin("SynthesisIterBot","endMajorCycle",WHERE) );

    try{
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));

      itsLoopController->incrementMajorCycleCount();
      itsLoopController->addSummaryMajor();
    } catch(AipsError &x) {
      throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
    }    
  }

  
  Record SynthesisIterBot::getSubIterBot()
  {
    Record returnRecord;

    LogIO os( LogOrigin("SynthesisIterBot","getSubIterBot",WHERE) );
    try
      {
        if (itsLoopController.get() == NULL) 
          throw( AipsError("Iteration Control un-initialized"));

	Float peakResidual = itsLoopController->getPeakResidual(); /// This can go
	Float integratedFlux = itsLoopController->getIntegratedFlux(); // This can go too.
	
	//// Commented out... this info should already be there, after the previous 'mergeSubIterBot' calls.
        //itsLoopController->setMaxPsfSidelobe(maxPsfSidelobe);
        //itsLoopController->updateCycleThreshold(peakResidual);

	/// Commented out temporarily. 

        os << "Start Minor-Cycle iterations with peak residual = " << peakResidual;
        os << " and model flux = " << integratedFlux << LogIO::POST;
        
        os << " [ cyclethreshold = " << itsLoopController->getCycleThreshold() ;
        os << " max iter per field/chan/pol = " << itsLoopController->getCycleNiter() ;
        os << " loopgain = " << itsLoopController->getLoopGain() ;
        os << " ]" << LogIO::POST;

        if (itsLoopController->interactiveInputRequired(peakResidual)) {
          pauseForUserInteraction();
        }

        returnRecord = itsLoopController->getSubIterBotRecord();
      }
    catch(AipsError &x)
      {
	throw( AipsError("Error in constructing SubIterBot : "+x.getMesg()) );
      }
    return returnRecord;
  }

  void SynthesisIterBot::endMinorCycle(Record& subIterBotRecord) {
    try {
      if (itsLoopController.get() == NULL) 
        throw( AipsError("Iteration Control un-initialized"));

      SISubIterBot loopController(subIterBotRecord);
      LogIO os( LogOrigin("SynthesisIterBot",__FUNCTION__,WHERE) );
      itsLoopController->mergeSubIterBot(loopController);
    } catch(AipsError &x) {
      throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
    }
  }

  void SynthesisIterBot::pauseForUserInteraction()
  {
    LogIO os( LogOrigin("SISkyModel","pauseForUserInteraction",WHERE) );

    os << "Waiting for interactive clean feedback" << LogIO::POST;

    /* This call will make sure that the current values of loop control are
       available in the GUI and will not return until the user hits the
       button */
    itsLoopController->waitForInteractiveInput();
    // UUU comment out the above line to test if plumbing around interaction is OK.

    /*    
    Int nmappers = itsMappers.nMappers();
    for(Int mp=0;mp<nmappers;mp++)
      {
	TempImage<Float> dispresidual, dispmask;
	// Memory for these image copies are allocated inside the SIMapper
	itsMappers.getMapper(mp)->getCopyOfResidualAndMask( dispresidual, dispmask );

	///// Send dispresidual and dispmask to the GUI.
	///// Receive dispmask back from the GUI ( on click-to-set-mask for this field )

	itsMappers.getMapper(mp)->setMask( dispmask );
      }
    */

  }// end of pauseForUserInteraction


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



} //# NAMESPACE CASA - END


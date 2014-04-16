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
#include <iomanip>


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

#include<synthesis/ImagerObjects/SIIterBot.h>
#include <synthesis/ImagerObjects/SynthesisIterBot.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/synthesis/ImagerControl.h>

#include <sys/types.h>
#include <unistd.h>
using namespace std;


namespace casa { //# NAMESPACE CASA - BEGIN
  
	SynthesisIterBot::SynthesisIterBot() : actionRequestSync(new SIIterBot_callback( )),
					       itsLoopController(new SIIterBot_state(actionRequestSync)),
                                               dbus_thread(NULL) {
	  //		fprintf( stderr, ">>>>>>\t\tSynthesisIterBot::~SynthesisIterBot(0x%p)\n", this );
	  //		fflush( stderr );
	}

	void SynthesisIterBot::openDBus( ) {
		if ( dbus_thread != NULL ) return;
		dbus_thread = new boost::thread(boost::bind(&SynthesisIterBot::dbus_thread_launch_pad,this));
	}

	void SynthesisIterBot::dbus_thread_launch_pad( ) {
		SIIterBot_adaptor dbus_adaptor(itsLoopController,ImagerControl::name( ),ImagerControl::object_path( ));
		casa::DBusSession::instance().dispatcher( ).enter( );
		std::cout << "Service Loop Exited: " << time(0) << std::endl;
	}

	SynthesisIterBot::~SynthesisIterBot() {
		if ( dbus_thread != NULL ) {
			casa::DBusSession::instance().dispatcher( ).leave( );
			dbus_thread->join( );
			delete dbus_thread;
			dbus_thread = NULL;
		}
		//		LogIO os( LogOrigin("SynthesisIterBot","destructor",WHERE) );
		//		os << "SynthesisIterBot destroyed" << LogIO::POST;
	}

	void SynthesisIterBot::setIterationDetails(Record iterpars) {
		LogIO os( LogOrigin("SynthesisIterBot","setIterationDetails",WHERE) );
		try {
			//itsLoopController.reset( new SIIterBot("SynthesisImage_"));
			if ( itsLoopController ) itsLoopController->setControlsFromRecord(iterpars);
		} catch(AipsError &x) {
			throw( AipsError("Error in updating iteration parameters : " + x.getMesg()) );
		}
	}

	Record SynthesisIterBot::getIterationDetails() {
		LogIO os( LogOrigin("SynthesisIterBot","getIterationDetails",WHERE) );
		Record returnRecord;
		try {
			if ( itsLoopController)
				returnRecord = itsLoopController->getDetailsRecord();
		} catch(AipsError &x) {
			throw( AipsError("Error in retrieving iteration parameters : " + x.getMesg()) );
		}
		return returnRecord;
	}

	Record SynthesisIterBot::getIterationSummary() {
		LogIO os( LogOrigin("SynthesisIterBot","getIterationSummary",WHERE) );
		Record returnRecord;
		try {
			if ( itsLoopController )
				returnRecord = itsLoopController->getSummaryRecord();
		} catch(AipsError &x) {
			throw( AipsError("Error in retrieving iteration parameters : " + x.getMesg()) );
		}
		return returnRecord;
	}


	void SynthesisIterBot::setupIteration(Record iterpars) {
		LogIO os( LogOrigin("SynthesisIterBot","setupIteration",WHERE) );
		os << "Set Iteration Control Options." << LogIO::POST;
		try {
			setIterationDetails(iterpars);
		} catch(AipsError &x) {
			throw( AipsError("Error in constructing SkyModel : "+x.getMesg()) );
		}
	} //end of setupIteration
  
	void SynthesisIterBot::setInteractiveMode(Bool interactiveMode) {
		LogIO os( LogOrigin("SynthesisIterBot","setupIteration",WHERE) );
		os << "Setting intractive mode to " 
		   << ((interactiveMode) ? "Active" : "Inactive") << LogIO::POST;
		try {
			if ( itsLoopController )
				itsLoopController->changeInteractiveMode(interactiveMode);
		} catch(AipsError &x) {
			throw( AipsError("Error Setting Interactive Mode : "+x.getMesg()) );
		}
	}
  
  
	bool SynthesisIterBot::cleanComplete() {
		bool returnValue=False;
		try {
			//Float peakResidual = itsLoopController.getPeakResidual(); // This should go..
			if ( itsLoopController )
				returnValue=itsLoopController->cleanComplete();
		} catch (AipsError &x) {
			throw( AipsError("Error checking clean complete state : "+x.getMesg()) );
		}
		return returnValue; 
	}

	void SynthesisIterBot::endMajorCycle() {
		LogIO os( LogOrigin("SynthesisIterBot","endMajorCycle",WHERE) );

		try {
			if ( itsLoopController ) {
				itsLoopController->incrementMajorCycleCount();
				itsLoopController->addSummaryMajor();
			}
		} catch(AipsError &x) {
			throw( AipsError("Error in running Major Cycle : "+x.getMesg()) );
		}    
	}

  
	Record SynthesisIterBot::getSubIterBot() {
		Record returnRecord;
		LogIO os( LogOrigin("SynthesisIterBot","getSubIterBot",WHERE) );

		try {
			if ( itsLoopController ) {
				if (itsLoopController->interactiveInputRequired()) {
					pauseForUserInteraction();
				}
				returnRecord = itsLoopController->getMinorCycleControls();
			}

		} catch(AipsError &x) {
			throw( AipsError("Error in constructing SubIterBot : "+x.getMesg()) );
		}
		return returnRecord;
	}

	void SynthesisIterBot::startMinorCycle(Record& initializationRecord) {
		try {
			LogIO os( LogOrigin("SynthesisIterBot",__FUNCTION__,WHERE) );
			if ( itsLoopController )
				itsLoopController->mergeCycleInitializationRecord(initializationRecord);
		} catch(AipsError &x) {
			throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
		}
	}

	void SynthesisIterBot::endMinorCycle(Record& executionRecord) {
		try {
			//SISubIterBot loopController(subIterBotRecord);
			LogIO os( LogOrigin("SynthesisIterBot",__FUNCTION__,WHERE) );
			if ( itsLoopController )
				itsLoopController->mergeCycleExecutionRecord(executionRecord);
		} catch(AipsError &x) {
			throw( AipsError("Error in running Minor Cycle : "+x.getMesg()) );
		}
	}

	void SynthesisIterBot::pauseForUserInteraction() {
		LogIO os( LogOrigin("SISkyModel","pauseForUserInteraction",WHERE) );
		os << "Waiting for interactive clean feedback" << LogIO::POST;

		/* This call will make sure that the current values of loop control are
		   available in the GUI and will not return until the user hits the
		   button */
		if ( itsLoopController ) {
			itsLoopController->waitForInteractiveInput();

		}// end of pauseForUserInteraction
	}



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////    Internal Functions start here.  These are not visible to the tool layer.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

} //# NAMESPACE CASA - END


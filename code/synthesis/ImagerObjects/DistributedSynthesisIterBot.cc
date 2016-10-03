//# DistributedSynthesisIterBot.cc: Parallel imaging iteration control
//# Copyright (C) 2016
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
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <iomanip>

#include <casa/Logging.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <synthesis/ImagerObjects/SIIterBot.h>
#include <synthesis/ImagerObjects/DistributedSynthesisIterBot.h>
#include <synthesis/ImagerObjects/DistSIIterBot.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/synthesis/ImagerControl.h>

#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <thread>

using namespace casacore;
namespace casa { //# NAMESPACE CASA - BEGIN

DistributedSynthesisIterBot::DistributedSynthesisIterBot(MPI_Comm comm)
	: actionRequestSync(new SIIterBot_callback())
	, itsLoopController(new DistSIIterBot_state(actionRequestSync, comm))
	, dbus_thread(NULL) {}

void 
DistributedSynthesisIterBot::openDBus() {
	if (itsLoopController->rank == 0) {
		if (dbus_thread != NULL) return;
		dbus_thread = new std::thread(
			std::bind(&DistributedSynthesisIterBot::dbus_thread_launch_pad,this));
	}
}

void
DistributedSynthesisIterBot::dbus_thread_launch_pad() {
	SIIterBot_adaptor dbus_adaptor(
		itsLoopController, ImagerControl::name(), ImagerControl::object_path());
	casa::DBusSession::instance().dispatcher().enter();
	std::cout << "Service Loop Exited: " << time(0) << std::endl;
}

DistributedSynthesisIterBot::~DistributedSynthesisIterBot() {
	if (dbus_thread != NULL) {
		casa::DBusSession::instance().dispatcher().leave();
		dbus_thread->join();
		delete dbus_thread;
		dbus_thread = NULL;
	}
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "destructor", WHERE));
	os << LogIO::DEBUG1
	   << "DistributedSynthesisIterBot destroyed"
	   << LogIO::POST;
}

void
DistributedSynthesisIterBot::setIterationDetails(Record iterpars) {
	LogIO os(LogOrigin("DistributedSynthesisIterBot",  "setIterationDetails",
	                   WHERE));
	try {
		itsLoopController->setControlsFromRecord(iterpars);
	} catch(AipsError &x) {
		throw AipsError("Error in updating iteration parameters : "
		                + x.getMesg()) ;
	}
}

Record
DistributedSynthesisIterBot::getIterationDetails() {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "getIterationDetails",
	                   WHERE));
	Record returnRecord;
	try {
		returnRecord = itsLoopController->getDetailsRecord();
	} catch(AipsError &x) {
		throw AipsError("Error in retrieving iteration parameters : "
		                + x.getMesg());
	}
	return returnRecord;
}

Record
DistributedSynthesisIterBot::getIterationSummary() {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "getIterationSummary",
	                   WHERE));
	Record returnRecord;
	try {
		returnRecord = itsLoopController->getSummaryRecord();
	} catch(AipsError &x) {
		throw AipsError("Error in retrieving iteration parameters : "
		                + x.getMesg());
	}
	return returnRecord;
}

void
DistributedSynthesisIterBot::setupIteration(Record iterpars) {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "setupIteration", WHERE));
	os << "Set Iteration Control Options" << LogIO::POST;
	try {
		setIterationDetails(iterpars);
	} catch(AipsError &x) {
		throw AipsError("Error in setting iteration parameters : "
		                + x.getMesg());
	}
}

void
DistributedSynthesisIterBot::setInteractiveMode(Bool interactiveMode) {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "setInteractiveMode", WHERE));
	os << "Setting intractive mode to "
	   << ((interactiveMode) ? "Active" : "Inactive")
	   << LogIO::POST;
	try {
		itsLoopController->changeInteractiveMode(interactiveMode);
	} catch(AipsError &x) {
		throw AipsError("Error Setting Interactive Mode : " + x.getMesg());
	}
}

int
DistributedSynthesisIterBot::cleanComplete() {
	int returnValue = 0;
	try {
		//Float peakResidual = itsLoopController.getPeakResidual(); // This should go..
		returnValue = itsLoopController->cleanComplete();
	} catch (AipsError &x) {
		throw AipsError("Error checking clean complete state : "+x.getMesg());
	}
	return returnValue;
}

void
DistributedSynthesisIterBot::endMajorCycle() {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "endMajorCycle", WHERE));

	try {
		itsLoopController->incrementMajorCycleCount();
		itsLoopController->addSummaryMajor();
	} catch(AipsError &x) {
		throw AipsError("Error in running Major Cycle : " + x.getMesg());
	}
}

Record
DistributedSynthesisIterBot::getSubIterBot() {
	Record returnRecord;
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "getSubIterBot", WHERE));

	try {
		if (itsLoopController->interactiveInputRequired())
			pauseForUserInteraction();
		returnRecord = itsLoopController->getMinorCycleControls();
	} catch(AipsError &x) {
		throw AipsError("Error in constructing Subtend : " + x.getMesg());
	}
	return returnRecord;
}

void
DistributedSynthesisIterBot::startMinorCycle(const Vector<Record>& initializationRecords) {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", __FUNCTION__, WHERE));
	try {
		itsLoopController->mergeCycleInitializationRecords(initializationRecords);
	} catch(AipsError &x) {
		throw AipsError("Error in running Minor Cycle : " + x.getMesg());
	}
}

void
DistributedSynthesisIterBot::endMinorCycle(const Vector<Record>& executionRecords) {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", __FUNCTION__, WHERE));
	try {
		itsLoopController->mergeCycleExecutionRecords(executionRecords);
	} catch(AipsError &x) {
		throw AipsError("Error in running Minor Cycle : " + x.getMesg());
	}
}

void
DistributedSynthesisIterBot::pauseForUserInteraction() {
	/* This call will make sure that the current values of loop control are
	   available in the GUI and will not return until the user hits the
	   button */
	itsLoopController->waitForInteractiveInput();
}

void
DistributedSynthesisIterBot::changeStopFlag( Bool stopflag ) {
	itsLoopController->changeStopFlag( stopflag );
}

////////////////////////////////////////////////////////////////////////////////
////    DistributedSynthesisIterBotWithOldGUI code starts.
////////////////////////////////////////////////////////////////////////////////

DistributedSynthesisIterBotWithOldGUI::DistributedSynthesisIterBotWithOldGUI(MPI_Comm comm)
	: DistributedSynthesisIterBot(comm)
	, itsInteractiveMasker() {
	int rank;
	MPI_Comm_rank(comm, &rank);
	is_root = rank == 0;
}

void
DistributedSynthesisIterBotWithOldGUI::setIterationDetails(Record iterpars) {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "setIterationDetails",
	                   WHERE));

	try {

		// TODO - What can be done about this? Is the need for this function
		// removed by the new design?

		////////////////////////////////////////////////////////////////////////
		///// START : code to get a list of image names for interactive masking

		// Setup interactive masking : list of image names.
		if (itsImageList.nelements() == 0 ) {
			if (iterpars.isDefined("allimages")) {
				Record allnames =
					iterpars.subRecord(RecordFieldId("allimages"));
				uInt nfields = allnames.nfields();
				itsImageList.resize(nfields);
				itsMultiTermList.resize(nfields);
				itsMultiTermList = false;
				for (uInt fld = 0; fld < nfields; fld++ ) {
					Record onename = allnames.subRecord(
						RecordFieldId(String::toString(fld)));
					if (onename.isDefined("imagename")
					    && onename.isDefined("multiterm")) {
						onename.get(RecordFieldId("imagename"),
						            itsImageList[fld]);
						onename.get(RecordFieldId("multiterm"),
						            itsMultiTermList[fld]);
					}
				}
				//cout << "Image List : " << itsImageList << " nterms : " << itsMultiTermList << endl;
			} else {
				throw AipsError(
					"Need image names and nterms in iteration parameter list");
			}
		}
		///// END : code to get a list of image names for interactive masking
		////////////////////////////////////////////////////////////////////////

		//itsLoopController.reset( new SIIterBot("SynthesisImage_"));
//		if (itsLoopController)
			itsLoopController->setControlsFromRecord(iterpars);

	} catch(AipsError &x) {
		throw AipsError("Error in updating iteration parameters : "
		                + x.getMesg());
	}
}

void
DistributedSynthesisIterBotWithOldGUI::pauseForUserInteraction() {
	LogIO os(LogOrigin("DistributedSynthesisIterBot", "pauseForUserInteraction",
	                   WHERE));

	if (is_root) {
		// This will launch it only once.
		if (itsInteractiveMasker.null())
			itsInteractiveMasker = new InteractiveMasking();

		// Get parameters to change.
		Record iterRec = getIterationDetails();

		Int niter = 0, cycleniter = 0, iterdone;
		Float threshold = 0.0, cyclethreshold = 0.0;
		if (iterRec.isDefined("niter") &&
		    iterRec.isDefined("cycleniter") &&
		    iterRec.isDefined("threshold") &&
		    iterRec.isDefined("cyclethreshold") &&
		    iterRec.isDefined("iterdone")) {
			iterRec.get("niter", niter);
			iterRec.get("cycleniter", cycleniter);
			iterRec.get("threshold", threshold);
			iterRec.get("cyclethreshold", cyclethreshold);
			iterRec.get("iterdone",iterdone);
		}
		else {
			throw AipsError("SI::interactiveGui() needs valid niter, " \
			                "cycleniter, threshold to start up.");
		}

		String strthresh = String::toString(threshold) + "Jy";
		String strcycthresh = String::toString(cyclethreshold) + "Jy";

		Int iterleft = niter - iterdone;
		if (iterleft < 0) iterleft = 0;

		uInt nIm = itsImageList.nelements();
		if (itsActionCodes.nelements() != nIm) {
			itsActionCodes.resize(nIm);
			itsActionCodes.set(0);
		}

		for (uInt ind=0; ind < nIm; ind++) {
			if (itsActionCodes[ind] == 0) {
				String imageName =
					itsImageList[ind] + ".residual"
					+ (itsMultiTermList[ind] ? ".tt0" : "");
				String maskName = itsImageList[ind] + ".mask";
				//cout << "Before interaction : niter : " << niter << " cycleniter : " << cycleniter << " thresh : " << strthresh << "  cyclethresh : " << strcycthresh << endl;
				itsActionCodes[ind] =
					itsInteractiveMasker->interactivemask(
						imageName, maskName, iterleft, cycleniter, strthresh,
						strcycthresh);
				//cout << "After interaction : niter : " << niter << " cycleniter : " << cycleniter << " thresh : " << strthresh << " cyclethresh : " << strcycthresh << "  ------ ret : " << itsActionCodes[ind] << endl;
			}
		}

		//cout << "ActionCodes : " << itsActionCodes << endl;

		Quantity qa;
		casacore::Quantity::read(qa, strthresh);
		threshold = qa.getValue(Unit("Jy"));
		casacore::Quantity::read(qa, strcycthresh);
		cyclethreshold = qa.getValue(Unit("Jy"));

		itsLoopController->changeNiter(iterdone + iterleft);
		itsLoopController->changeCycleNiter(cycleniter);
		itsLoopController->changeThreshold(threshold);
		itsLoopController->changeCycleThreshold(cyclethreshold);

		if (std::all_of(itsActionCodes.cbegin(), itsActionCodes.cend(),
		           [](Int c){return c == 2;}))
			changeStopFlag(true);
	}

	// TODO - The following was originally returned from this function. What's
	// the purpose? Is their purpose duplicated by calling changeStopFlag above?

	//              return itsActionCodes;

	// Record returnRec;
	// for (uInt ind = 0; ind < itsImageList.nelements(); ind++)
	//  returnRec.define(RecordFieldId(String::toString(ind)),
	//                   itsActionCodes[ind]);

	// return returnRec;
}


} //# NAMESPACE CASA - END

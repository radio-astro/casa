//# DistSIIterBot.cc: Parallel imaging iteration control
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
#include <synthesis/ImagerObjects/DistSIIterBot.h>
#include <synthesis/ImagerObjects/SIIterBot.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/Conversion.h>

/* Records Interface */
#include <casa/Containers/Record.h>

#include <cstddef>
#include <cstring>
#include <functional>
#include <algorithm>

namespace casa { //# NAMESPACE CASA - BEGIN

ExecRecord::ExecRecord(int iterDone, int maxCycleIterDone, float peakResidual,
                       bool updatedModelFlag)
	: peakResidual(peakResidual)
	, iterDone(iterDone)
	, maxCycleIterDone(maxCycleIterDone)
	, updatedModelFlag(updatedModelFlag) {}

ExecRecord::ExecRecord(const Record &rec)
	: peakResidual(
		rec.isDefined("peakresidual") ? rec.asFloat("peakresidual") : 0.0)
	, iterDone(
		rec.isDefined("iterdone") ? rec.asInt("iterdone") : 0)
	, maxCycleIterDone(
		rec.isDefined("maxcycleiterdone") ? rec.asInt("maxcycleiterdone") : 0)
	, updatedModelFlag(
		rec.isDefined("updatedmodelflag")
		? rec.asBool("updatedmodelflag")
		: false) {}

Record
ExecRecord::asRecord() {
	Record result;
	result.define("peakresidual", peakResidual);
	result.define("iterdone", iterDone);
	result.define("maxcycleiterdone", maxCycleIterDone);
	result.define("updatedmodelflag", updatedModelFlag);
	return result;
}

DetailsRecord::DetailsRecord(const Record &rec)
	: threshold(
		rec.isDefined("threshold") ? rec.asFloat("threshold") : 0.0f)
	, cycleThreshold(
		rec.isDefined("cyclethreshold") ? rec.asFloat("cyclethreshold") : 0.0f)
	, interactiveThreshold(
		rec.isDefined("interactivethreshold")
		? rec.asFloat("interactivethreshold")
		: 0.0f)
	, loopGain(
		rec.isDefined("loopgain") ? rec.asFloat("loopgain") : 0.0f)
	, cycleFactor(
		rec.isDefined("cyclefactor") ? rec.asFloat("cyclefactor") : 0.0f)
	, maxPsfSidelobe(
		rec.isDefined("maxpsfsidelobe") ? rec.asFloat("maxpsfsidelobe") : 0.0f)
	, maxPsfFraction(
		rec.isDefined("maxpsffraction") ? rec.asFloat("maxpsffraction") : 0.0f)
	, minPsfFraction(
		rec.isDefined("minpsffraction") ? rec.asFloat("minpsffraction") : 0.0f)
	, niter(
		rec.isDefined("niter") ? rec.asInt("niter") : 0)
	, cycleNiter(
		rec.isDefined("cycleniter") ? rec.asInt("cycleniter") : 0)
	, interactiveNiter(
		rec.isDefined("interactiveniter") ? rec.asInt("interactiveniter") : 0)
	, iterDone(
		rec.isDefined("iterdone") ? rec.asInt("iterdone") : 0)
	, maxCycleIterDone(
		rec.isDefined("cycleiterdone") ? rec.asInt("cycleiterdone") : 0)
	, interactiveIterDone(
		rec.isDefined("interactiveiterdone")
		? rec.asInt("interactiveiterdone")
		: 0)
	, majorDone(
		rec.isDefined("nmajordone") ? rec.asInt("nmajordone") : 0)
	, interactiveMode(
		rec.isDefined("interactivemode")
		? rec.asBool("interactivemode")
		: false) {
	String cs = (rec.isDefined("cleanstate") ? rec.asString("cleanstate") : "");
	if (cs == "stopped") cleanState = 0;
	else if (cs == "paused") cleanState = 1;
	else /* cs == "running" */ cleanState = 2;
}

Record
DetailsRecord::asRecord() {
	Record result;
	result.define("niter", niter);
	result.define("cycleniter", cycleNiter);
	result.define("interactiveniter", interactiveNiter);
	result.define("threshold", threshold);
	result.define("cyclethreshold", cycleThreshold);
	result.define("interactivethreshold", interactiveThreshold);
	result.define("loopgain", loopGain);
	result.define("cyclefactor", cycleFactor);
	result.define("iterdone", iterDone);
	result.define("cycleiterdone", maxCycleIterDone);
	result.define("interactiveiterdone", interactiveIterDone);
	result.define("nmajordone", majorDone);
	result.define("maxpsfsidelobe", maxPsfSidelobe);
	result.define("maxpsffraction", maxPsfFraction);
	result.define("minpsffraction", minPsfFraction);
	result.define("interactivemode", interactiveMode);
	result.define("cleanstate",
	              (cleanState == 0)
	              ? "stopped"
	              : ((cleanState == 1) ? "paused" : "running"));
	return result;
}

ControlRecord::ControlRecord(
	int niter, int cycleNiter, float threshold,
	float cycleThreshold, float loopGain,
	bool stopFlag)
	: threshold(threshold)
	, cycleThreshold(cycleThreshold)
	, loopGain(loopGain)
	, niter(niter)
	, cycleNiter(cycleNiter)
	, stopFlag(stopFlag) {};

ControlRecord::ControlRecord(const Record &rec)
	: threshold(rec.asFloat("threshold"))
	, cycleThreshold(rec.asFloat("cyclethreshold"))
	, loopGain(rec.asFloat("loopgain"))
	, niter(rec.asInt("niter"))
	, cycleNiter(rec.asInt("cycleniter"))
	, stopFlag(rec.asBool("stopFlag")) {}

Record
ControlRecord::asRecord() {
	Record result;
	result.define("niter", niter);
	result.define("cycleniter", cycleNiter);
	result.define("threshold", threshold);
	result.define("cyclethreshold", cycleThreshold);
	result.define("loopgain", loopGain);
	result.define("stopflag", stopFlag);
	return result;
}

// All DistSIIterBot_states must have 'type' and 'name' defined.
DistSIIterBot_state::DistSIIterBot_state(SHARED_PTR<SIIterBot_callback> cb,
                                         MPI_Comm comm)
	: SIIterBot_state(cb)
	, comm(comm)
{
	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));
	assert(comm != MPI_COMM_NULL);
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &commSize);
	execRecordDatatype = ExecRecord::datatype();
	MPI_Type_commit(&execRecordDatatype);
	execRecordReduceOp = ExecRecord::reduceOp();
	detailsRecordDatatype = DetailsRecord::datatype();
	MPI_Type_commit(&detailsRecordDatatype);
	controlRecordDatatype = ControlRecord::datatype();
	MPI_Type_commit(&controlRecordDatatype);
}

DistSIIterBot_state::~DistSIIterBot_state() {
	MPI_Type_free(&execRecordDatatype);
	MPI_Op_free(&execRecordReduceOp);
	MPI_Type_free(&detailsRecordDatatype);
	MPI_Type_free(&controlRecordDatatype);
}

bool
DistSIIterBot_state::interactiveInputRequired() {
	std::lock_guard<std::recursive_mutex> guard(recordMutex);
	MPI_BOOL_TYPE result =
		itsInteractiveMode
		&& (itsMaxCycleIterDone + itsInteractiveIterDone >= itsInteractiveNiter
		    || itsPeakResidual <= itsInteractiveThreshold
		    || itsPauseFlag);
	MPI_Bcast(&result, 1, MPI_BOOL, 0, comm);
	return result;
}

void
DistSIIterBot_state::waitForInteractiveInput() {
	std::unique_lock<std::mutex> lock(interactionMutex);
	if (rank == 0) {
		/* Check that we have at least one controller */
		if (getNumberOfControllers() == 0) {
			/* Spawn a Viewer set up for interactive */
		}
		cout << "UU : setup interaction" << endl;
		if (!interactionPending) {
			interactionPending = true;
			if (callback)
				callback->interactionRequired(true);
		}
		cout << "UU : about to wait" << endl;
		/* Wait on Condition variable */
		while (interactionPending)
			interactionCond.wait(lock);

		cout << "UU : returned from wait" << endl;
		if (updateNeeded) {
			updateNeeded = false;
			if (callback)
				callback->interactionRequired(false);
		}
	}
}

int
DistSIIterBot_state::cleanComplete() {
	std::lock_guard<std::recursive_mutex> guard(recordMutex);

	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));

	int stopCode = 0;
	if (itsMajorDone == 0 && itsIterDone == 0)
		stopCode = 0;
	else if (itsIterDone >= itsNiter)
		stopCode = 1;
	else if (itsPeakResidual <= itsThreshold)
		stopCode = 2;
	else if (itsStopFlag)
		stopCode = 3;
	else if (fabs(itsPrevPeakResidual - itsPeakResidual) < 1e-10)
		stopCode = 4;

#ifndef NDEBUG
	// Check that all ranks have the same value of stopCode. TODO: get rid of
	// this entirely.
	int stopCodes[commSize];
	stopCodes[0] = stopCode;
	MPI_Gather(((rank == 0) ? MPI_IN_PLACE : stopCodes), 1, MPI_BOOL,
	           stopCodes, 1, MPI_BOOL, 0, comm);
	if (rank == 0)
		for (int i = 1; i < commSize; ++i)
			assert(stopCodes[i] == stopCodes[0]);
#endif
	return stopCode;
}

Record
DistSIIterBot_state::getMinorCycleControls() {
	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));
	std::lock_guard<std::recursive_mutex> guard(recordMutex);

	// adjust cycle threshold if changed by user
	if (itsIsCycleThresholdAuto) updateCycleThreshold();

	// broadcast controls to all ranks
	ControlRecord control(itsNiter, itsCycleNiter,
	                      itsThreshold, itsCycleThreshold, 
	                      itsLoopGain, itsStopFlag);
	MPI_Bcast(&control, 1, controlRecordDatatype, 0, comm);
	itsNiter = control.niter;
	itsCycleNiter = control.cycleNiter;
	itsThreshold = control.threshold;
	itsCycleThreshold = control.cycleThreshold;
	itsLoopGain = control.loopGain;
	itsStopFlag = control.stopFlag;

	/* This returns a record suitable for initializing the minor cycle
	   controls. */
	Record returnRecord;

	/* The minor cycle will stop based on the cycle parameters. */
	Int maxCycleIterations = std::min(itsCycleNiter, itsNiter - itsIterDone);
	Float cycleThreshold = std::max(itsCycleThreshold, itsThreshold);
	// if (itsInteractiveMode) {
	//  maxCycleIterations = min(maxCycleIterations, itsInteractiveNiter);
	//  cycleThreshold = max(cycleThreshold, itsInteractiveThreshold);
	// }
	returnRecord.define("cycleniter", maxCycleIterations);
	returnRecord.define("cyclethreshold", cycleThreshold);
	returnRecord.define("loopgain", itsLoopGain);

	return returnRecord;
}

void
DistSIIterBot_state::mergeCycleInitializationRecord(Record &rec) {
	// Note that this method should not be called from inside a loop to merge
	// one record at a time; use mergeCycleInitializationRecords() method
	// instead. The problem with calling this method from such a loop is that
	// this method calls mergeCycleInitializationRecords(), which calls an MPI
	// collective routine; such code would fail when different ranks have
	// varying numbers of records to merge. Direct use of
	// mergeCycleInitializationRecords() is safe to use in these circumstances.
	Vector<Record> v(1);
	v[0] = rec;
	mergeCycleInitializationRecords(v);
}

void
DistSIIterBot_state::mergeCycleInitializationRecords(
	const Vector<Record> &initRecords) {

	std::lock_guard<std::recursive_mutex> guard(recordMutex);
	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));

	// reduce values locally first
	float values[2] = { itsPeakResidual, itsMaxPsfSidelobe };
	for (auto rec : initRecords) {
		if (rec.isDefined("peakresidual"))
			values[0] = std::max(values[0], rec.asFloat("peakresidual"));
		if (rec.isDefined("maxpsfsidelobe"))
			values[1] = std::max(values[1], rec.asFloat("maxpsfsidelobe"));
	}
	// reduce values globally
	MPI_Allreduce(MPI_IN_PLACE, values, 2, MPI_FLOAT, MPI_MAX, comm);
	itsPeakResidual = values[0];
	itsMaxPsfSidelobe = values[1];
	updateCycleThreshold();
	itsIsCycleThresholdAuto = true; // to prepare interactive process
}

void
DistSIIterBot_state::mergeCycleExecutionRecord(Record &rec) {
	Vector<Record> v(1);
	v[0] = rec;
	mergeCycleExecutionRecords(v);
}

void
DistSIIterBot_state::mergeCycleExecutionRecords(
	const Vector<Record> &execRecords) {
	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));

	std::lock_guard<std::recursive_mutex> guard(recordMutex);

	itsPrevPeakResidual = itsPeakResidual;
	itsPeakResidual = 0;

	ExecRecord acc(((rank == 0) ? itsIterDone : 0),
	               itsMaxCycleIterDone,
	               itsPeakResidual,
	               itsUpdatedModelFlag);
	for (auto rec : execRecords) {
		mergeMinorCycleSummary(rec.asArrayDouble("summaryminor"));
		ExecRecord exrec = ExecRecord(rec);
		acc.reduce(&exrec);
	}
	MPI_Allreduce(MPI_IN_PLACE, &acc, 1, execRecordDatatype,
	              execRecordReduceOp, comm);
	itsIterDone = acc.iterDone;
	itsMaxCycleIterDone = acc.maxCycleIterDone;
	itsPeakResidual = acc.peakResidual;
	itsUpdatedModelFlag = acc.updatedModelFlag;

	os << "Completed " << itsIterDone << " iterations." << LogIO::POST;
	//with peak residual "<< itsPeakResidual << LogIO::POST;
}

void
DistSIIterBot_state::mergeMinorCycleSummary(const Array<Double>& summary) {
	std::lock_guard<std::recursive_mutex> guard(recordMutex);

	IPosition accSummaryShape = itsSummaryMinor.shape();
	IPosition summaryShape = summary.shape();

	if (accSummaryShape.nelements() != 2
	    || accSummaryShape[0] != itsNSummaryFields
	    || summaryShape.nelements() != 2
	    || summaryShape[0] != itsNSummaryFields)
		throw AipsError(
			"Internal error in shape of global minor-cycle summary record");

	itsSummaryMinor.resize(
		IPosition(2, itsNSummaryFields, accSummaryShape[1] + summaryShape[1]),
		true);

	ArrayIterator<Double> summaryIter(summary, IPosition(1, 1), false);
	ArrayIterator<Double> accSummaryIter(itsSummaryMinor, IPosition(1, 1),
	                                     false);
	accSummaryIter.set(IPosition(1, accSummaryShape[1]));

	IPosition i0 = IPosition(1, 0);
	while (!summaryIter.pastEnd()) {
		accSummaryIter.array() = summaryIter.array();
		accSummaryIter.array()(i0) += itsIterDone;
		accSummaryIter.next();
		summaryIter.next();
	}
}

Record
DistSIIterBot_state::getSummaryRecord() {
	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));
	std::lock_guard<std::recursive_mutex> guard(recordMutex);

	// Gather summary records.

	// Note that we take measures to use MPI_IN_PLACE arguments below so as to
	// allow this code to compile without MPI, using only the macros in
	// MPIGlue.h. The code would be slightly cleaner without using MPI_IN_PLACE,
	// but then we'd have either MPI-only code, or alternative code sections in
	// this module based on the value of HAVE_MPI. The present implementation
	// seems more maintainable to me (MP).

    // First gather number of doubles in summary from every rank.
	int summarySize = itsSummaryMinor.size();
	std::vector<int> allSizes((rank == 0) ? commSize : 1);
	allSizes[0] = summarySize;
	MPI_Gather(((rank == 0) ? MPI_IN_PLACE : allSizes.data()), 1, MPI_INT,
	           allSizes.data(), 1, MPI_INT,
	           0, comm);

	// Prepare to receive summaries using MPI_Gatherv
	int totalSize = 0;
	std::vector<int> displs;
	for (auto sz : allSizes) {
		displs.push_back(totalSize);
		totalSize += sz;
	}
	assert(rank == 0 || totalSize == summarySize);
	std::vector<double> acc(totalSize);
	Bool delSummaryStore;
	const void *summaryStore = itsSummaryMinor.getVStorage(delSummaryStore);
	// To use MPI_IN_PLACE below, must copy data from summaryStore to acc
	std::memcpy(acc.data(), summaryStore, summarySize * sizeof(double));
	itsSummaryMinor.freeVStorage(summaryStore, delSummaryStore);

	// Gather summaries
	MPI_Gatherv(
		((rank == 0) ? MPI_IN_PLACE : acc.data()), summarySize, MPI_DOUBLE,
		acc.data(), allSizes.data(), displs.data(), MPI_DOUBLE,
		0, comm);

	// Create Array of expected shape from acc
	IPosition totalSummaryShape(2, itsNSummaryFields,
	                            totalSize / itsNSummaryFields);
	Array<Double> totalSummary(totalSummaryShape, acc.data(), SHARE);

	// Create return Record
	Record returnRecord = getDetailsRecord();
	returnRecord.define("interactiveiterdone",
	                    returnRecord.asInt("interactiveiterdone")
	                    + returnRecord.asInt("cycleiterdone"));
	returnRecord.define("summaryminor", totalSummary);
	returnRecord.define("summarymajor", itsSummaryMajor);
	return returnRecord;
}

// TODO: the following overrides SIIterBot_state::getDetailsRecord, but I'm not
// sure that it should
Record
DistSIIterBot_state::getDetailsRecord() {
	LogIO os(LogOrigin("DistSIIterBot_state", __FUNCTION__, WHERE));
	std::lock_guard<std::recursive_mutex> guard(recordMutex);
	Record returnRecord;

	/* Control Variables */
	returnRecord.define("niter", itsNiter);
	returnRecord.define("cycleniter", itsCycleNiter);
	returnRecord.define("interactiveniter",itsInteractiveNiter);

	returnRecord.define("threshold",  itsThreshold);

	returnRecord.define("cyclethreshold",itsCycleThreshold);
	returnRecord.define("interactivethreshold", itsInteractiveThreshold);

	returnRecord.define("loopgain",  itsLoopGain);
	returnRecord.define("cyclefactor", itsCycleFactor);

	/* Status Reporting Variables */
	returnRecord.define("iterdone",  itsIterDone);
	returnRecord.define("cycleiterdone",  itsMaxCycleIterDone);
	returnRecord.define("interactiveiterdone", itsInteractiveIterDone);

	returnRecord.define("nmajordone",  itsMajorDone);
	returnRecord.define("maxpsfsidelobe", itsMaxPsfSidelobe);
	returnRecord.define("maxpsffraction", itsMaxPsfFraction);
	returnRecord.define("minpsffraction", itsMinPsfFraction);
	returnRecord.define("interactivemode", itsInteractiveMode);

	returnRecord.define("stopcode", itsStopCode);

	/* report clean's state */
	returnRecord.define(
		"cleanstate",
		itsStopFlag ? "stopped" : (itsPauseFlag ? "paused" : "running"));
	return returnRecord;
}

} //# NAMESPACE CASA - END

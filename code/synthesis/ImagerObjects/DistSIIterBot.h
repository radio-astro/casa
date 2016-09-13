/* -*- mode: c++ -*- */
//# DistSIIterBot.h: Parallel imaging iteration control
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
#ifndef DIST_S_I_ITER_BOT_H_
#define DIST_S_I_ITER_BOT_H_

#include <casadbus/utilities/BusAccess.h>
// .casarc interface
#include <casa/System/AipsrcValue.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>
#if defined(DBUS_CPP)
#include <dbus-cpp/dbus.h> /*for DBus::Variant... probably can be removed with *_adaptor class*/
#else
#include <dbus-c++/dbus.h>
#endif

// Include files for the DBus Service
//#include <casadbus/interfaces/SynthImager.adaptor.h>

#ifdef INTERACTIVE_ITERATION
#include <casadbus/interfaces/SynthImager.adaptor.h>
#endif
#include <synthesis/ImagerObjects/SIIterBot.h>
#include <synthesis/ImagerObjects/MPIGlue.h>
#include <mutex>

namespace casa { //# NAMESPACE CASA - BEGIN

class DistSIIterBot_state
	: public SIIterBot_state {
private:
	// make DistSIIterBot_state uncopyable...
	DistSIIterBot_state(const DistSIIterBot_state &);
	DistSIIterBot_state &operator=(const DistSIIterBot_state &);

public:
	DistSIIterBot_state(SHARED_PTR<SIIterBot_callback>, MPI_Comm);
	~DistSIIterBot_state( );

	bool interactiveInputRequired();
	void waitForInteractiveInput(); 
	int cleanComplete();
	Record getMinorCycleControls();
	void mergeCycleInitializationRecord(Record&);
	void mergeCycleExecutionRecord(Record&);
	Record getSummaryRecord();
	Record getDetailsRecord();

	void mergeCycleInitializationRecords(const Vector<Record> &);
	void mergeCycleExecutionRecords(const Vector<Record> &);

	int rank;
	int commSize;

protected:
	void mergeMinorCycleSummary(const Array<Double>&);

	MPI_Comm comm;

	MPI_Datatype execRecordDatatype;
	MPI_Op execRecordReduceOp;
	MPI_Datatype detailsRecordDatatype;
	MPI_Datatype controlRecordDatatype;
};

struct ExecRecord {
	ExecRecord(int iterDone, int maxCycleIterDone, float peakResidual,
	           bool updatedModelFlag);

	ExecRecord(const Record &rec);

	Record asRecord();

	// Don't modify the following fields, including their order, without a
	// corresponding change in the computation of datatype()!
	float peakResidual;
	int iterDone;
	int maxCycleIterDone;
	MPI_BOOL_TYPE updatedModelFlag;

	void reduce(const ExecRecord *rec) {
		iterDone += rec->iterDone;
		maxCycleIterDone = std::max(maxCycleIterDone, rec->maxCycleIterDone);
		peakResidual = std::max(peakResidual, rec->peakResidual);
		updatedModelFlag |= rec->updatedModelFlag;
	};

	// It might be nice to have static class members to hold the MPI datatype
	// and reduction op, but since they can only be computed at run-time, that
	// would require a mutex and lock (the computation would have to be in a
	// critical section). In turn that would require acquiring the lock every
	// time the datatype is accessed, which is something that we'd like to
	// avoid.
	static MPI_Datatype datatype() {
		int blocklengths[3] UNUSED_WITHOUT_MPI = { 1, 2, 1 };
		MPI_Aint displacements[3] UNUSED_WITHOUT_MPI = {
			offsetof(struct ExecRecord, peakResidual),
			offsetof(struct ExecRecord, iterDone),
			offsetof(struct ExecRecord, updatedModelFlag) };
		MPI_Datatype types[3] UNUSED_WITHOUT_MPI =
			{ MPI_FLOAT, MPI_INT, MPI_BOOL };
		MPI_Datatype dt;
		MPI_Type_create_struct(3, blocklengths, displacements, types, &dt);
		MPI_Datatype result;
		// to capture padding for alignment of structures, we resize dt
		MPI_Type_create_resized(dt, 0, sizeof(struct ExecRecord),
		                        &result);
		MPI_Type_free(&dt);
		return result;
	};

	static MPI_Op reduceOp() {
		MPI_Op result;
		MPI_Op_create(
			static_cast<MPI_User_function *>(ExecRecord::reduceOpFun),
			true,
			&result);
		return result;
	};

	static void reduceOpFun(void *invec, void *inoutvec, int *len,
	                        MPI_Datatype *datatype __attribute__((unused))) {
		const ExecRecord *exin = static_cast<const ExecRecord *>(invec);
		ExecRecord *exinout = static_cast<ExecRecord *>(inoutvec);
		int cnt = *len; // OpenMPI at least doesn't like it if you directly
						// decrement the value pointed to by *len -- not sure
						// that the standard says anything about doing that
		while (cnt-- > 0) {
			exinout->reduce(exin);
			++exinout;
			++exin;
		}
	};
};

struct DetailsRecord {
	DetailsRecord(const Record &rec);

	Record asRecord();

	// Don't modify the following fields, including their order, without a
	// corresponding change in the computation of datatype()!
	float threshold;
	float cycleThreshold;
	float interactiveThreshold;
	float loopGain;
	float cycleFactor;
	float maxPsfSidelobe;
	float maxPsfFraction;
	float minPsfFraction;
	int niter;
	int cycleNiter;
	int interactiveNiter;
	int iterDone;
	int maxCycleIterDone;
	int interactiveIterDone;
	int majorDone;
	int cleanState; // 0: stopped, 1: paused, 2: running
	MPI_BOOL_TYPE interactiveMode;

	static MPI_Datatype datatype() {
		int blocklengths[3] UNUSED_WITHOUT_MPI = { 8, 8, 1 };
		MPI_Aint displacements[3] UNUSED_WITHOUT_MPI = {
			offsetof(struct DetailsRecord, threshold),
			offsetof(struct DetailsRecord, niter),
			offsetof(struct DetailsRecord, interactiveMode) };
		MPI_Datatype types[3] UNUSED_WITHOUT_MPI =
			{ MPI_FLOAT, MPI_INT, MPI_BOOL };
		MPI_Datatype dt;
		MPI_Type_create_struct(3, blocklengths, displacements, types, &dt);
		MPI_Datatype result;
		// to capture padding for alignment of structures, we resize dt
		MPI_Type_create_resized(dt, 0, sizeof(struct DetailsRecord),
		                        &result);
		MPI_Type_free(&dt);
		return result;
	};
};

struct ControlRecord {
	ControlRecord(int niter, int cycleNiter,
	              float threshold, float cycleThreshold,
	              float loopGain, bool stopFlag);

	ControlRecord(const Record &rec);

	Record asRecord();

	// Don't modify the following fields, including their order, without a
	// corresponding change in the computation of datatype()!
	float threshold;
	float cycleThreshold;
	float loopGain;
	int niter;
	int cycleNiter;
	MPI_BOOL_TYPE stopFlag;

	// It might be nice to have static class members to hold the MPI datatype
	// and reduction op, but since they can only be computed at run-time, that
	// would require a mutex and lock, since the computation would have to be in
	// a critical section. In turn that would require acquiring the lock every
	// time the datatype is accessed, which is something that we'd like to
	// avoid.
	static MPI_Datatype datatype() {
		int blocklengths[3] UNUSED_WITHOUT_MPI = { 3, 2, 1 };
		MPI_Aint displacements[3] UNUSED_WITHOUT_MPI = {
			offsetof(struct ControlRecord, threshold),
			offsetof(struct ControlRecord, niter),
			offsetof(struct ControlRecord, stopFlag)};
		MPI_Datatype types[3] UNUSED_WITHOUT_MPI =
			{ MPI_FLOAT, MPI_INT, MPI_BOOL };
		MPI_Datatype dt;
		MPI_Type_create_struct(3, blocklengths, displacements, types, &dt);
		MPI_Datatype result;
		// to capture padding for alignment of structures, we resize dt
		MPI_Type_create_resized(dt, 0, sizeof(struct ControlRecord),
		                        &result);
		MPI_Type_free(&dt);
		return result;
	};
};

} //# NAMESPACE CASA - END

#endif // DIST_S_I_ITER_BOT_H_

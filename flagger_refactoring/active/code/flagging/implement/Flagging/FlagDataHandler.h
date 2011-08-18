//# FlagDataHandler.h: This file contains the interface definition of the FlagDataHandler class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FLAGDATAHANDLER_H_
#define FLAGDATAHANDLER_H_


// Measurement Set selection
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>

// Async I/O infrastructure
#include <msvis/MSVis/VisibilityIteratorAsync.h>
#include <msvis/MSVis/VisBufferAsync.h>
#include <msvis/MSVis/VWBT.h>

// .casarc interface
#include <casa/System/AipsrcValue.h>

// Records interface
#include <casa/Containers/Record.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>

#define STARTCLOCK if (profiling_p) gettimeofday(&start_p,0);
#define STOPCLOCK if (profiling_p) \
	{\
		gettimeofday(&stop_p,0);\
		elapsedTime_p = (stop_p.tv_sec-start_p.tv_sec)*1000.0+(stop_p.tv_usec-start_p.tv_usec)/1000.0;\
		*logger_p << LogIO::DEBUG2 << "FlagDataHandler::" << __FUNCTION__ << " Executed in: " << elapsedTime_p << " ms, Memory free: " << HostInfo::memoryFree( )/1024.0 << " MB" << LogIO::POST;\
	}

namespace casa { //# NAMESPACE CASA - BEGIN

class FlagDataHandler
{

public:

	// Default constructor
	FlagDataHandler(uShort iterationApproach);

	// Default destructor
	~FlagDataHandler();

	// Open Measurement Set
	void open(string msname);

	// Close Measurement Set
	void close();

	// Set Data Selection parameters
	void setDataSelection(Record record);

	// Generate selected Measurement Set
	void selectData();

	// Generate Visibility Iterator
	void generateIterator();

	// Move to next chunk
	bool nextChunk();

	// Move to next buffer
	bool nextBuffer();

	// Write flag cube into MS
	void flushFlags();

	// As requested by Urvashi R.V. provide access to the original and modified flag cubes
	Cube<Bool> *getModifiedFlagCube();
	Cube<Bool> *getOriginalFlagCube();

	// Dummy processBuffer function (it has to be implemented in the agents)
	void processBuffer(bool write);

	// Set function to activate profiling
	void setProfiling(bool enable) {profiling_p = enable;}

	// Visibility Buffer
	// WARNING: The attach mechanism only works with pointers or
	// referenced variables. Otherwise the VisBuffer is created
	// and attached, but when it is assigned to the member it is
	// detached because of the dynamically called destructor
	VisBufferAutoPtr *visibilityBuffer_p;

	// RO Visibility Iterator
	VisibilityIterator *rwVisibilityIterator_p;
	ROVisibilityIterator *roVisibilityIterator_p;

	// Iteration counters
	uShort chunkNo;
	uShort bufferNo;

	enum iteration {

		SCAN_SUB_INTEGRATION=0,
		BASELINE_SCAN_TIME_SERIES
	};


protected:

private:

	// Measurement Set
	MeasurementSet *originalMeasurementSet_p;

	// Measurement Set Selection
	MSSelection *measurementSetSelection_p;

	// Selected Measurement Set
	MeasurementSet *selectedMeasurementSet_p;

	// Data Selection ranges
	casa::String arraySelection_p;
	casa::String fieldSelection_p;
	casa::String scanSelection_p;
	casa::String timeSelection_p;
	casa::String spwSelection_p;
	casa::String baselineSelection_p;
	casa::String uvwSelection_p;
	casa::String polarizationSelection_p;

	// Iteration parameters
	uShort iterationApproach_p;
	Block<int> sortOrder_p;
	Double timeInterval_p;
	bool groupTimeSteps_p;

	// Flag Cubes
	Cube<Bool> originalFlagCube_p;
	Cube<Bool> modifiedFlagCube_p;

	// Async I/O stuff
	VWBT * vwbt_p;
	bool asyncio_disabled_p;

	// Slurp flag
	bool slurp_p;

	// Iteration initialization parameters
	bool chunksInitialized_p;
	bool buffersInitialized_p;

	// Profiling clocks
	bool profiling_p;
	timeval start_p,stop_p;
	double elapsedTime_p;

	// Logger
	casa::LogIO *logger_p;

};

} //# NAMESPACE CASA - END

#endif /* FLAGDATAHANDLER_H_ */

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

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <casa/OS/HostInfo.h>
#include <sys/time.h>

#define STARTCLOCK if (profiling_p) gettimeofday(&start_p,0);
#define STOPCLOCK if (profiling_p) \
	{\
		gettimeofday(&stop_p,0);\
		elapsedTime_p = (stop_p.tv_sec-start_p.tv_sec)*1000.0+(stop_p.tv_usec-start_p.tv_usec)/1000.0;\
		cout << "FlagDataHandler::" << __FUNCTION__ << " Executed in: " << elapsedTime_p << " ms, Memory free: " << HostInfo::memoryFree( )/1024.0 << " MB" <<endl;\
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
	bool open(const std::string& msname = """");

	// Generate selected Measurement Set
	bool selectData();

	// Generate Visibility Iterator
	bool generateIterator();

	// Move to next chunk
	bool nextChunk();

	// Move to next buffer
	bool nextBuffer();

	// Visibility Buffer
	// WARNING: The attach mechanism only works with pointers or
	// referenced variables. Otherwise the VisBuffer is created
	// and attached, but when it is assigned to the member it is
	// detached because of the dynamically called destructor
	VisBuffer *visibilityBuffer_p;

	// Visibility Iterator
	VisibilityIterator *visibilityIterator_p;

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
	string arraySelection_p;
	string fieldSelection_p;
	string scanSelection_p;
	string timeSelection_p;
	string spwSelection_p;
	string baselineSelection_p;
	string uvwSelection_p;
	string polarizationSelection_p;

	// Iteration parameters
	uShort iterationApproach_p;
	Block<int> sortOrder_p;
	Double timeInterval_p;
	bool groupTimeSteps_p;

	// Iteration initialization parameters
	bool chunksInitialized_p;
	bool buffersInitialized_p;

	// Profiling clocks
	bool profiling_p;
	timeval start_p,stop_p;
	double elapsedTime_p;

	// Debug mode
	bool debug_p;

	// Logger
	casa::LogIO *logger_p;

};

} //# NAMESPACE CASA - END

#endif /* FLAGDATAHANDLER_H_ */

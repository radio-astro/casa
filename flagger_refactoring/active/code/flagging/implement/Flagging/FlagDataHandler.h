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

// Data mapping
#include <map>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>

#define STARTCLOCK timeval start,stop; double elapsedTime; if (profiling_p) gettimeofday(&start,0);
#define STOPCLOCK if (profiling_p) \
	{\
		gettimeofday(&stop,0);\
		elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;\
		*logger_p << LogIO::DEBUG2 << "FlagDataHandler::" << __FUNCTION__ << " Executed in: " << elapsedTime << " ms, Memory free: " << HostInfo::memoryFree( )/1024.0 << " MB" << LogIO::POST;\
	}

namespace casa { //# NAMESPACE CASA - BEGIN

// Type definitions
typedef std::map< std::pair<Int,Int>,std::vector<uInt> >::iterator antennaPairMapIterator;
typedef std::map< Double,std::vector<uInt> >::iterator subIntegrationMapIterator;
typedef std::map< std::pair<Int,Int>,std::vector<uInt> > antennaPairMap;
typedef std::map< Double,std::vector<uInt> > subIntegrationMap;

// We need to have the CubeView definition here because its type is used by FlagDataHandler class
template<class T> class CubeView
{

public:

	CubeView(Cube<T> *parentCube, std::vector<uInt> *rows = NULL)
	{
		parentCube_p = parentCube;

		if ((rows != NULL) and (rows->size() > 0))
		{
			rows_p = rows;
			IPosition baseCubeShape = parentCube_p->shape();
			reducedLength_p = IPosition(3);
			reducedLength_p(0) = baseCubeShape(0);
			reducedLength_p(1) = baseCubeShape(1);
			reducedLength_p(2) = rows_p->size();
		}
		else
		{
			IPosition baseCubeShape = parentCube_p->shape();
			reducedLength_p = IPosition(3);
			reducedLength_p(0) = baseCubeShape(0);
			reducedLength_p(1) = baseCubeShape(1);
			reducedLength_p(2) = baseCubeShape(2);

			rows_p = new vector<uInt>(reducedLength_p(2));
			rows_p->clear();
			for (Int i=0; i<reducedLength_p(2); i++ )
			{
				rows_p->push_back(i);
			}
		}
	}

    T &operator()(uInt i1, uInt i2, uInt i3)
    {
    	uInt i3_index = rows_p->at(i3);
    	return parentCube_p->at(i1,i2,i3_index);
    }

    const IPosition &shape() const
    {
    	return reducedLength_p;
    }

    void shape(Int &s1, Int &s2, Int &s3) const
    {
    	s1 = reducedLength_p(0);
    	s2 = reducedLength_p(1);
    	s3 = reducedLength_p(2);
    	return;
    }

protected:

private:
    Cube<T> *parentCube_p;
	std::vector<uInt> *rows_p;
	IPosition reducedLength_p;
};

// Flag Data Handler class definition
class FlagDataHandler
{

public:

	enum iteration {

		COMPLETE_SCAN_MAPPED=0,
		COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY,
		COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY,
		COMPLETE_SCAN_UNMAPPED,
		ANTENNA_PAIR,
		SUB_INTEGRATION
	};

	// Default constructor
	// NOTE: Time interval 0 groups all time steps together in one chunk.
	FlagDataHandler(string msname, uShort iterationApproach = SUB_INTEGRATION, Double timeInterval = 0);

	// Default destructor
	~FlagDataHandler();

	// Open Measurement Set
	bool open();

	// Close Measurement Set
	bool close();

	// Set Data Selection parameters
	bool setDataSelection(Record record);

	// Set Data Selection parameters
	void applyChannelSelection(ROVisibilityIterator *roVisIter);

	// Generate selected Measurement Set
	bool selectData();

	// Generate Visibility Iterator
	bool generateIterator();

	// Swap MS to check what is the maximum RAM memory needed
	void checkMaxMemory();

	// Move to next chunk
	bool nextChunk();

	// Move to next buffer
	bool nextBuffer();

	// Write flag cube into MS
	bool flushFlags();

	// As requested by Urvashi R.V. provide access to the original and modified flag cubes
	Cube<Bool> * getModifiedFlagCube();
	Cube<Bool> * getOriginalFlagCube();

	// Mapping functions as requested by Urvashi
	void generateAntennaPairMap();
	void generateSubIntegrationMap();
	antennaPairMap * getAntennaPairMap() {return antennaPairMap_p;}
	subIntegrationMap * getSubIntegrationMap() {return subIntegrationMap_p;}
	CubeView<Bool> * getFlagsView(Int antenna1, Int antenna2);
	CubeView<Bool> * getFlagsView(Double timestep);
	CubeView<Complex> * getVisibilitiesView(Int antenna1, Int antenna2);
	CubeView<Complex> * getVisibilitiesView(Double timestep);

	// Dummy processBuffer function (it has to be implemented in the agents)
	uShort processBuffer(bool write, uShort rotateMode, uShort rotateViews);
	void fillBuffer(CubeView<Bool> &flagCube, bool write, uShort processBuffer);

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

	// Measurement set section
	String msname_p;
	MeasurementSet *selectedMeasurementSet_p;
	MeasurementSet *originalMeasurementSet_p;
	MSSelection *measurementSetSelection_p;

	// Iteration counters
	uShort chunkNo;
	uShort bufferNo;


protected:

private:

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

	// Mapping members
	std::map< std::pair<Int,Int>,std::vector<uInt> > *antennaPairMap_p;
	std::map< Double,std::vector<uInt> > *subIntegrationMap_p;
	bool mapAntennaPairs_p;
	bool mapSubIntegrations_p;

	// Stats members
	bool stats_p;
	uLong cubeAccessCounter_p;
	double cubeAccessTime_p;
	uLong cubeAccessCounterTotal_p;
	double cubeAccessTimeTotal_p;

	// Slurp flag
	bool slurp_p;

	// Iteration initialization parameters
	bool chunksInitialized_p;
	bool buffersInitialized_p;
	bool iteratorGenerated_p;

	// Profiling
	bool profiling_p;

	// Logger
	casa::LogIO *logger_p;

};

} //# NAMESPACE CASA - END

#endif /* FLAGDATAHANDLER_H_ */

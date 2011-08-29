//# FlagDataHandler.h: This file contains the implementation of the FlagDataHandler class.
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

#include <flagging/Flagging/FlagDataHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
FlagDataHandler::FlagDataHandler(string msname, uShort iterationApproach, Double timeInterval):
		msname_p(msname), iterationApproach_p(iterationApproach), timeInterval_p(timeInterval)
{
	// Initialize logger
	logger_p = new LogIO();

	// Default verbosity
	profiling_p = false;

	// Check if async input is enabled
	asyncio_disabled_p = true;
	AipsrcValue<Bool>::find (asyncio_disabled_p,"ROVisibilityIteratorAsync.disabled", true);

	// Check if slurp is enabled
	if (asyncio_disabled_p)
	{
		slurp_p = true;
		AipsrcValue<Bool>::find (slurp_p,"ROVisibilityIteratorAsync.slurp:", true);
	}
	else
	{
		slurp_p = false;
		AipsrcValue<Bool>::find (slurp_p,"ROVisibilityIteratorAsync.slurp:", false);
	}


	// WARNING: By default the visibility iterator adds the following
	// default columns: ARRAY_ID and FIELD_ID,DATA_DESC_ID and TIME.
	// And they are needed for the correct operation of the VisibilityIterator
	// (it needs to know when any of these changes to be able to give
	// the correct coordinates with the data). If these columns are not
	// explicitly sorted on, they will be added BEFORE any others, with
	// unexpected iteration behaviour
	// See: MSIter::construct

	// WARNING: By specifying Time as a sort column we are not producing
	// chunks with only one time step, but just the other way around!
	// (all time steps are grouped in the same chunk). This is because TIME
	// is one of the 'implicit' sort columns, and as explained before it is
	// added BEFORE the ones specified by the user, resulting in chunks with
	// only one time step, thus invalidating the time interval effect.
	// See: MSIter.h

	switch (iterationApproach_p)
	{
		case COMPLETE_SCAN_MAPPED:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: COMPLETE_SCAN_MAPPED" << LogIO::POST;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = true;
			mapAntennaPairs_p = true;
			mapSubIntegrations_p = true;
			break;
		}
		case COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY" << LogIO::POST;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = true;
			mapAntennaPairs_p = false;
			mapSubIntegrations_p = true;
			break;
		}
		case COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY" << LogIO::POST;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = true;
			mapAntennaPairs_p = true;
			mapSubIntegrations_p = false;
			break;
		}
		case COMPLETE_SCAN_UNMAPPED:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: COMPLETE_SCAN_UNMAPPED" << LogIO::POST;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = true;
			mapAntennaPairs_p = false;
			mapSubIntegrations_p = false;
			break;
		}
		case ANTENNA_PAIR:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: ANTENNA_PAIR" << LogIO::POST;
			sortOrder_p = Block<int>(8);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			// NOTE: As requested by Urvashi, in this way we produce chunks
			// per antenna pair with all the time steps grouped instead
			// of chunks with entire integrations for a given time step.
			sortOrder_p[5] = MS::ANTENNA1;
			sortOrder_p[6] = MS::ANTENNA2;
			sortOrder_p[7] = MS::TIME;

			// NOTE: groupTimeSteps_p=true groups all time steps together in one buffer.
			groupTimeSteps_p = true;
			mapAntennaPairs_p = false;
			mapSubIntegrations_p = false;
			break;
		}
		case  SUB_INTEGRATION:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: SUB_INTEGRATION" << LogIO::POST;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = false;
			mapAntennaPairs_p = false;
			mapSubIntegrations_p = false;
			break;
		}
		default:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: COMPLETE_SCAN_MAPPED" << LogIO::POST;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = true;
			mapAntennaPairs_p = true;
			mapSubIntegrations_p = true;
			break;
		}
	}

	// Initialize selection ranges
	timeSelection_p = String("");
	baselineSelection_p = String("");
	fieldSelection_p = String("");
	spwSelection_p = String("*");
	uvwSelection_p = String("");
	scanSelection_p = String("");
	arraySelection_p = String("");

	// Initialize iteration parameters
	chunksInitialized_p = false;
	buffersInitialized_p = false;
	chunkNo = 0;
	bufferNo = 0;

	// Initialize stats
	stats_p = false;
	cubeAccessCounter_p = 0;

	// Set all the initialized pointers to NULL
	selectedMeasurementSet_p = NULL;
	measurementSetSelection_p = NULL;
	originalMeasurementSet_p = NULL;
	rwVisibilityIterator_p = NULL;
	roVisibilityIterator_p = NULL;
	visibilityBuffer_p = NULL;
	subIntegrationMap_p = NULL;
	antennaPairMap_p = NULL;
	vwbt_p = NULL;

	return;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
FlagDataHandler::~FlagDataHandler()
{
	// Destroy members
	if (vwbt_p) delete vwbt_p;
	if (antennaPairMap_p) delete antennaPairMap_p;
	if (subIntegrationMap_p) delete subIntegrationMap_p;
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	if (roVisibilityIterator_p) delete roVisibilityIterator_p;
	// Apparently there is a problem here, if we destroy the base RO iterator of a RW iterator the pointer is not set to NULL
	if ((!asyncio_disabled_p) and (rwVisibilityIterator_p)) delete rwVisibilityIterator_p;
	if (selectedMeasurementSet_p) delete selectedMeasurementSet_p;
	if (measurementSetSelection_p) delete measurementSetSelection_p;
	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	if (logger_p) delete logger_p;

	return;
}


// -----------------------------------------------------------------------
// Open Measurement Set
// -----------------------------------------------------------------------
void
FlagDataHandler::open()
{
	STARTCLOCK

	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	originalMeasurementSet_p = new MeasurementSet(msname_p,Table::Update);

	// Activate Memory Resident Sub-tables for everything but Pointing, Syscal and History
	originalMeasurementSet_p->setMemoryResidentSubtables (MrsEligibility::defaultEligible());

	STOPCLOCK
	return;
}


// -----------------------------------------------------------------------
// Close Measurement Set
// -----------------------------------------------------------------------
void
FlagDataHandler::close()
{
	STARTCLOCK

	if (selectedMeasurementSet_p)
	{
		// Wait until all the pending writing operations are done
		if (!asyncio_disabled_p)
		{
			while (vwbt_p->isWriting())
			{
				pthread_yield();
			}
		}

		// Flush and unlock MS
		selectedMeasurementSet_p->flush();
		selectedMeasurementSet_p->relinquishAutoLocks(True);
		selectedMeasurementSet_p->unlock();

		// Post stats
		if (stats_p)
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Total Flag Cube accesses: " <<  cubeAccessCounter_p << LogIO::POST;
		}
	}

	STOPCLOCK
	return;
}


// -----------------------------------------------------------------------
// Set Data Selection parameters
// -----------------------------------------------------------------------
void
FlagDataHandler::setDataSelection(Record record)
{
	STARTCLOCK

	int exists;

	exists = record.fieldNumber ("array");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("array"), arraySelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " array selection is " << arraySelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no array selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("field");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("field"), fieldSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " field selection is " << fieldSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no field selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("scan");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("scan"), scanSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " scan selection is " << scanSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no scan selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("time");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("time"), timeSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " time selection is " << timeSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no time selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("spw");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("spw"), spwSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " spw selection is " << spwSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no spw selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("baseline");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("baseline"), baselineSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " baselineSelection_p selection is " << baselineSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no baseline selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("uvw");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("uvw"), uvwSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " uvw selection is " << uvwSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no uvw selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("polarization");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("polarization"), polarizationSelection_p);
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " polarization selection is " << polarizationSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " no polarization selection" << LogIO::POST;
	}

	STOPCLOCK
	return;
}


// -----------------------------------------------------------------------
// Generate selected Measurement Set
// -----------------------------------------------------------------------
void
FlagDataHandler::selectData()
{
	STARTCLOCK

	// Create Measurement Selection object
	const String dummyExpr = String("");
	if (measurementSetSelection_p) delete measurementSetSelection_p;
	measurementSetSelection_p = new MSSelection(
			*originalMeasurementSet_p,
			MSSelection::PARSE_NOW,
			(const String)timeSelection_p,
			(const String)baselineSelection_p,
			(const String)fieldSelection_p,
			(const String)spwSelection_p,
			(const String)uvwSelection_p,
			dummyExpr, // taqlExpr
			dummyExpr, // polnExpr
			(const String)scanSelection_p,
			(const String)arraySelection_p,
			dummyExpr); // intent

	// Apply Measurement Selection to a copy of the original Measurement Set
	MeasurementSet auxMeasurementSet = MeasurementSet(*originalMeasurementSet_p);
	measurementSetSelection_p->getSelectedMS(auxMeasurementSet, String(""));
	if (selectedMeasurementSet_p) delete selectedMeasurementSet_p;
	selectedMeasurementSet_p = new MeasurementSet(auxMeasurementSet);

	// Check if selected MS has rows...
	if (selectedMeasurementSet_p->nrow() == 0)
	{
		*logger_p << LogIO::WARN << "Selected Measurement Set doesn't have any rows " << LogIO::POST;
	}

	// Some debugging information
	*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " Selected Measurement Set has " << measurementSetSelection_p->getSubArrayList() << " arrays" << LogIO::POST;
	*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " Selected Measurement Set has " << measurementSetSelection_p->getFieldList() << " fields" << LogIO::POST;
	*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " Selected Measurement Set has " << measurementSetSelection_p->getScanList() << " scans" << LogIO::POST;
	*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " Selected Measurement Set has " << measurementSetSelection_p->getSpwList() << " spws" << LogIO::POST;
	*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " Selected Measurement Set has " << measurementSetSelection_p->getChanList() << " channels" << LogIO::POST;
	*logger_p << LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__ << " Selected Measurement Set has " << selectedMeasurementSet_p->nrow() << " rows" << LogIO::POST;

	STOPCLOCK
	return;
}

// -----------------------------------------------------------------------
// Swap MS to check what is the maximum RAM memory needed
// -----------------------------------------------------------------------
void
FlagDataHandler::checkMaxMemory()
{
	STARTCLOCK

	double memoryNeeded = 0;
	double maxMemoryNeeded = 0;
	// visCube,flagCube
	double memoryPerVisFlagCubes = 65.0/(1024.0*1024.0);
	// ant1, ant2, corrType, feed1, feed2, fieldId, frequency, scan, spw, stateId, time, timeInterval, uvw
	double memoryPerRow = 32*15/(1024.0*1024.0);

	for (rwVisibilityIterator_p->originChunks(); rwVisibilityIterator_p->moreChunks();rwVisibilityIterator_p->nextChunk())
	{
		// Check if we have to group time steps
		if (groupTimeSteps_p)
		{
			rwVisibilityIterator_p->setRowBlocking(rwVisibilityIterator_p->nRowChunk());
		}

		// Iterate over vis buffers
		for (rwVisibilityIterator_p->origin(); rwVisibilityIterator_p->more();(*rwVisibilityIterator_p)++)
		{
			// Check total amount of memory needed for visibilities
			memoryNeeded = memoryPerVisFlagCubes*(rwVisibilityIterator_p->visibilityShape().product());

			// Add up memory needed for the rest of the columns
			memoryNeeded += memoryPerRow*(rwVisibilityIterator_p->nRow());

			if (memoryNeeded > maxMemoryNeeded) maxMemoryNeeded = memoryNeeded;
		}
	}

	Int buffers = 1;
	double memoryFree = HostInfo::memoryFree( )/1024.0;
	double memoryUsed = 100*maxMemoryNeeded/memoryFree;
	if (asyncio_disabled_p)
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " This process needs " << maxMemoryNeeded << " MB for loading visibility buffers ("
				<< memoryUsed << "%) of available free memory (" << memoryFree << " MB)"<< LogIO::POST;
	}
	else
	{
		AipsrcValue<Int>::find (buffers,"ROVisibilityIteratorAsync.nBuffers", 2);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " This process needs " << buffers << " (pre-fetched buffers in async mode) x " << maxMemoryNeeded << " MB for loading visibility buffers ("
				<< memoryUsed << "%) of available free memory (" << memoryFree << " MB)"<< LogIO::POST;
	}

	if (buffers*maxMemoryNeeded > memoryFree*0.90)
	{
		if (asyncio_disabled_p)
		{
			*logger_p << LogIO::SEVERE << "FlagDataHandler::" << __FUNCTION__ << " This process would need to consume more than 90% ("
					<< buffers*maxMemoryNeeded << " MB) of the available memory (" << memoryFree
					<< " MB) for loading vis buffers, aborting. Consider reducing the time interval."<< LogIO::POST;
		}
		else
		{
			*logger_p << LogIO::SEVERE << "FlagDataHandler::" << __FUNCTION__ << " This process would need to consume more than 90% ("
					<< buffers*maxMemoryNeeded << " MB) of the available memory (" << memoryFree
					<< " MB) for loading vis buffers, aborting. Consider reducing the time interval, or reducing the number of buffers pre-fetched by async I/O (" << buffers
					<< ") or even switch off async I/O." << LogIO::POST;
		}

		throw(AipsError("FlagDataHandler::checkMaxMemory() Not enough memory to process"));
	}

	STOPCLOCK
	return;
}


// -----------------------------------------------------------------------
// Generate Visibility Iterator with a given sort order and time interval
// -----------------------------------------------------------------------
void
FlagDataHandler::generateIterator()
{
	STARTCLOCK

	// First create and initialize RW iterator
	if (rwVisibilityIterator_p) delete rwVisibilityIterator_p;
	rwVisibilityIterator_p = new VisibilityIterator(*selectedMeasurementSet_p,sortOrder_p,true,timeInterval_p);

	checkMaxMemory();

	// If async I/O is enabled we create an async RO iterator for reading and a conventional RW iterator for writing
	// Both iterators share a mutex which is resident in the VLAT data (Visibility Look Ahead thread Data Object)
	// With this configuration the Visibility Buffer is attached to the RO async iterator
	if (asyncio_disabled_p)
	{
		// Cast RW conventional iterator into RO conventional iterator
		if (roVisibilityIterator_p) delete roVisibilityIterator_p;
		roVisibilityIterator_p = (ROVisibilityIterator*)rwVisibilityIterator_p;

		// Set the table data manager (ISM and SSM) cache size to the full column size, for
		// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
		if (slurp_p) roVisibilityIterator_p->slurp();

		// Finally attach Visibility Buffer to RO conventional iterator
		if (visibilityBuffer_p) delete visibilityBuffer_p;
		visibilityBuffer_p = new VisBufferAutoPtr(roVisibilityIterator_p);
	}
	else
	{
		// Determine columns to be pre-fetched
		ROVisibilityIteratorAsync::PrefetchColumns prefetchColumns = ROVisibilityIteratorAsync::prefetchColumns(casa::asyncio::Ant1,
															casa::asyncio::Ant2,
															casa::asyncio::ArrayId,
															casa::asyncio::CorrType,
															casa::asyncio::Feed1,
															casa::asyncio::Feed2,
															casa::asyncio::FieldId,
															casa::asyncio::Flag,
															casa::asyncio::FlagCube,
															casa::asyncio::FlagRow,
															casa::asyncio::ObservationId,
															casa::asyncio::ObservedCube,
															casa::asyncio::Freq,
															casa::asyncio::NChannel,
															casa::asyncio::NCorr,
															casa::asyncio::NRow,
															casa::asyncio::PhaseCenter,
															casa::asyncio::Scan,
															casa::asyncio::SpW,
															casa::asyncio::StateId,
															casa::asyncio::Time,
															casa::asyncio::TimeInterval,
															casa::asyncio::Uvw,-1);

		// Then create and initialize RO Async iterator
		if (roVisibilityIterator_p) delete roVisibilityIterator_p;
		roVisibilityIterator_p = ROVisibilityIteratorAsync::create(*selectedMeasurementSet_p,prefetchColumns,sortOrder_p,true,timeInterval_p,-1,groupTimeSteps_p);

		// Set the table data manager (ISM and SSM) cache size to the full column size, for
		// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
		if (slurp_p) roVisibilityIterator_p->slurp();

		// Attach Visibility Buffer to Visibility Iterator
		if (visibilityBuffer_p) delete visibilityBuffer_p;
		visibilityBuffer_p = new VisBufferAutoPtr(roVisibilityIterator_p);

		// Finally, initialize Visibility Write Behind Thread
		if (vwbt_p) delete vwbt_p;
		ROVisibilityIteratorAsync * roVisibilityIteratorAsync = (ROVisibilityIteratorAsync*) roVisibilityIterator_p;
		casa::async::Mutex * mutex = roVisibilityIteratorAsync->getMutex();
		vwbt_p = new VWBT(rwVisibilityIterator_p,mutex,groupTimeSteps_p);
		vwbt_p->start();

	}

	STOPCLOCK	
	return;
}


// -----------------------------------------------------------------------
// Move to next chunk
// -----------------------------------------------------------------------
bool
FlagDataHandler::nextChunk()
{
	STARTCLOCK

	bool moreChunks = false;
	if (!chunksInitialized_p)
	{
		generateIterator();
		roVisibilityIterator_p->originChunks();
		chunksInitialized_p = true;
		buffersInitialized_p = false;
		moreChunks = true;
		chunkNo++;
		bufferNo = 0;
	}
	else
	{
		roVisibilityIterator_p->nextChunk();

		if (roVisibilityIterator_p->moreChunks())
		{
			buffersInitialized_p = false;
			moreChunks = true;
			chunkNo++;
			bufferNo = 0;
		}
	}

	STOPCLOCK
	return moreChunks;
}


// -----------------------------------------------------------------------
// Move to next buffer
// -----------------------------------------------------------------------
bool
FlagDataHandler::nextBuffer()
{
	STARTCLOCK

	bool moreBuffers = false;
	if (!buffersInitialized_p)
	{
		// Group all the time stamps in one single buffer
		// NOTE: Otherwise we have to iterate over Visibility Buffers
		// that contain all the rows with the same time step.
		if ((groupTimeSteps_p) and (asyncio_disabled_p))
		{
			Int nRowChunk = roVisibilityIterator_p->nRowChunk();
			roVisibilityIterator_p->setRowBlocking(nRowChunk);
		}
		roVisibilityIterator_p->origin();
		buffersInitialized_p = true;
		if (mapAntennaPairs_p) generateAntennaPairMap();
		if (mapSubIntegrations_p) generateSubIntegrationMap();
		moreBuffers = true;
		bufferNo++;
	}
	else
	{
		// WARNING: ++ operator is defined for VisibilityIterator class ("advance" function)
		// but if you define a VisibilityIterator pointer, then  ++ operator does not call
		// the advance function but increments the pointers.
		(*roVisibilityIterator_p)++;

		// WARNING: We iterate and afterwards check if the iterator is valid
		if (roVisibilityIterator_p->more())
		{
			if (mapAntennaPairs_p) generateAntennaPairMap();
			if (mapSubIntegrations_p) generateSubIntegrationMap();
			moreBuffers = true;
			bufferNo++;
		}
	}

	// Set new common flag cube
	if (moreBuffers)
	{
		// WARNING: We have to modify the shape of the cube before re-assigning it
		Cube<Bool> flagCube= visibilityBuffer_p->get()->flagCube();
		modifiedFlagCube_p.resize(flagCube.shape());
		modifiedFlagCube_p = flagCube;
		originalFlagCube_p.resize(flagCube.shape());
		originalFlagCube_p = flagCube;

		//IPosition flagCubeShape = modifiedFlagCube_p.shape();
		//*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Current buffer shape: " << flagCubeShape  << " and indexing:" <<  modifiedFlagCube_p.printConfig() <<LogIO::POST;
	}

	STOPCLOCK
	return moreBuffers;
}


// -----------------------------------------------------------------------
// Flush flags to MS
// -----------------------------------------------------------------------
void
FlagDataHandler::flushFlags()
{
	STARTCLOCK

	if (asyncio_disabled_p)
	{
		rwVisibilityIterator_p->setFlag(modifiedFlagCube_p);
	}
	else
	{
		vwbt_p->setFlag(modifiedFlagCube_p);
	}

	STOPCLOCK
	return;
}


// -----------------------------------------------------------------------
// As requested by Urvashi R.V. provide access to the original and modified flag cubes
// -----------------------------------------------------------------------
Cube<Bool>*
FlagDataHandler::getModifiedFlagCube()
{
	return &modifiedFlagCube_p;
}

Cube<Bool>*
FlagDataHandler::getOriginalFlagCube()
{
	return &originalFlagCube_p;
}

// -----------------------------------------------------------------------
// Mapping functions as requested by Urvashi
// -----------------------------------------------------------------------
void
FlagDataHandler::generateAntennaPairMap()
{
	STARTCLOCK

	// Free previous map and create a new one
	if (antennaPairMap_p) delete antennaPairMap_p;
	antennaPairMap_p = new antennaPairMap();

	// Retrieve antenna vectors
	Vector<Int> antenna1Vector = visibilityBuffer_p->get()->antenna1();
	Vector<Int> antenna2Vector = visibilityBuffer_p->get()->antenna2();

	// Fill map
	Int ant1_i,ant2_i;
	uInt nRows = antenna1Vector.size();
	for (uInt row_idx=0;row_idx<nRows;row_idx++)
	{
		ant1_i = antenna1Vector[row_idx];
		ant2_i = antenna2Vector[row_idx];
		if (antennaPairMap_p->find(std::make_pair(ant1_i,ant2_i)) == antennaPairMap_p->end())
		{
			std::vector<uInt> newPair;
			newPair.push_back(row_idx);
			(*antennaPairMap_p)[std::make_pair(ant1_i,ant2_i)] = newPair;
		}
		else
		{
			(*antennaPairMap_p)[std::make_pair(ant1_i,ant2_i)].push_back(row_idx);
		}
	}
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ <<  " " << antennaPairMap_p->size() <<" Antenna pairs found in current buffer" << LogIO::POST;

	STOPCLOCK
	return;
}


void
FlagDataHandler::generateSubIntegrationMap()
{
	STARTCLOCK

	// Free previous map and create a new one
	if (subIntegrationMap_p) delete subIntegrationMap_p;
	subIntegrationMap_p = new subIntegrationMap();

	// Retrieve antenna vectors
	Vector<Double> timeVector = visibilityBuffer_p->get()->time();

	// Fill map
	uInt nRows = timeVector.size();
	for (uInt row_idx=0;row_idx<nRows;row_idx++)
	{
		if (subIntegrationMap_p->find(timeVector[row_idx]) == subIntegrationMap_p->end())
		{
			std::vector<uInt> newSubIntegration;
			newSubIntegration.push_back(row_idx);
			(*subIntegrationMap_p)[timeVector[row_idx]] = newSubIntegration;
		}
		else
		{
			(*subIntegrationMap_p)[timeVector[row_idx]].push_back(row_idx);
		}
	}
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ <<  " " << subIntegrationMap_p->size() <<" Sub-Integrations (time steps) found in current buffer" << LogIO::POST;

	STOPCLOCK
	return;
}



Cube<Bool> *
FlagDataHandler::getFlagsView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Bool> * cube= new CubeView<Bool>(&modifiedFlagCube_p,rows);
	return cube;
}

Cube<Bool> *
FlagDataHandler::getFlagsView(Double timestep)
{
	std::vector<uInt> *rows = &((*subIntegrationMap_p)[timestep]);
	CubeView<Bool> * cube= new CubeView<Bool>(&modifiedFlagCube_p,rows);
	return cube;
}

Cube<Complex> *
FlagDataHandler::getVisibilitiesView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Complex> * cube= new CubeView<Complex>(&(visibilityBuffer_p->get()->visCube()),rows);
	return cube;
}

Cube<Complex> *
FlagDataHandler::getVisibilitiesView(Double timestep)
{
	std::vector<uInt> *rows = &((*subIntegrationMap_p)[timestep]);
	CubeView<Complex> * cube= new CubeView<Complex>(&(visibilityBuffer_p->get()->visCube()),rows);
	return cube;
}


// -----------------------------------------------------------------------
// Dummy function to simulate processing
// -----------------------------------------------------------------------
uShort
FlagDataHandler::processBuffer(bool write, uShort rotateMode, uShort rotateViews)
{
	STARTCLOCK

	stats_p = true;

	antennaPairMapIterator 	myAntennaPairMapIterator;
	Double timestep;
	subIntegrationMapIterator mySubIntegrationMapIterator;
	std::pair<Int,Int> antennaPair;
	uShort processView = rotateViews;
	IPosition flagCubeShape;
	Cube<Bool> *flagCube;
	switch (rotateMode)
	{
		case 0:
			fillBuffer(modifiedFlagCube_p,write,0);
		break;

		case 1:
			for (myAntennaPairMapIterator=antennaPairMap_p->begin(); myAntennaPairMapIterator != antennaPairMap_p->end(); ++myAntennaPairMapIterator) {
				antennaPair = myAntennaPairMapIterator->first;
				flagCube = getFlagsView(antennaPair.first,antennaPair.second);
				flagCubeShape = flagCube->shape();
				*logger_p 	<< LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__
							<< " Flag cube for (" <<  antennaPair.first << "," << antennaPair.second << ") has shape ["
						    << flagCubeShape(0) << "," <<  flagCubeShape(1) << "," << flagCubeShape(2) << "]" << LogIO::POST;

				if (rotateViews == 0)
				{
					fillBuffer(*flagCube,write,0);
				}
				else
				{
					if (processView == 1)
					{
						fillBuffer(*flagCube,write,1);
						processView = 2;
					}
					else
					{
						fillBuffer(*flagCube,write,2);
						processView = 1;
					}
				}

				delete flagCube;
			}
		break;

		case 2:
			for (mySubIntegrationMapIterator=subIntegrationMap_p->begin(); mySubIntegrationMapIterator != subIntegrationMap_p->end(); ++mySubIntegrationMapIterator) {
				timestep = mySubIntegrationMapIterator->first;
				flagCube = getFlagsView(timestep);
				flagCubeShape = flagCube->shape();
				*logger_p 	<< LogIO::DEBUG1 << "FlagDataHandler::" << __FUNCTION__
							<< " Flag cube for (" <<  timestep << ") has shape ["
						    << flagCubeShape(0) << "," <<  flagCubeShape(1) << "," << flagCubeShape(2) << "]" << LogIO::POST;

				if (rotateViews == 0)
				{
					fillBuffer(*flagCube,write,0);
				}
				else
				{
					if (processView == 1)
					{
						fillBuffer(*flagCube,write,1);
						processView = 2;
					}
					else
					{
						fillBuffer(*flagCube,write,2);
						processView = 1;
					}
				}

				delete flagCube;
			}
		break;

		default:
		break;
	}

	STOPCLOCK
	return processView;
}

void
FlagDataHandler::fillBuffer(Cube<Bool> &flagCube,bool write, uShort processBuffer)
{
	bool processCondition = false;
	if (processBuffer == 0)
	{
		if (groupTimeSteps_p)
		{
			if (chunkNo % 2 == 0)
			{
				processCondition = true;
			}
			else
			{
				processCondition = false;
			}
		}
		else
		{
			if (bufferNo % 2 == 0)
			{
				processCondition = true;
			}
			else
			{
				processCondition = false;
			}
		}
	}
	else
	{
		if (processBuffer == 1)
		{
			processCondition = true;
		}
		else
		{
			processCondition = false;
		}
	}

	IPosition flagCubeShape = flagCube.shape();
	uInt nPolarizations = flagCubeShape(0);
	uInt nChannels = flagCubeShape(1);
	uInt nRows = flagCubeShape(2);
	bool flag;
	for (uInt row_k=0;row_k<nRows;row_k++) {
		for (uInt chan_j=0;chan_j<nChannels;chan_j++) {
			for (uInt pol_i=0;pol_i<nPolarizations;pol_i++) {
				// Flag each other chunks (i.e. even chunks)
				if (write)
				{
					if (processCondition) {
						flagCube(pol_i,chan_j,row_k) = True;
						cubeAccessCounter_p++;
					} else {
						flagCube(pol_i,chan_j,row_k) = False;
						cubeAccessCounter_p++;
					}
				}
				else
				{
					flag = flagCube(pol_i,chan_j,row_k);
					cubeAccessCounter_p++;

					if (processCondition) {
						if (flag != True)
						{
							*logger_p 	<< LogIO::SEVERE << "FlagDataHandler::" << __FUNCTION__
										<<" Wrong flag (False instead of True) in chunk " << chunkNo
										<< " buffer " << bufferNo << " polarization " << pol_i
										<< " channel " << nChannels << " row " << row_k << LogIO::POST;
						}
					} else {
						if (flag != False)
						{
							*logger_p 	<< LogIO::SEVERE << "FlagDataHandler::" << __FUNCTION__
										<<" Wrong flag (True instead of False) in chunk " << chunkNo
										<< " buffer " << bufferNo << " polarization " << pol_i
										<< " channel " << nChannels << " row " << row_k << LogIO::POST;
						}
					}
				}
			}
		}
	}

	return;
}



} //# NAMESPACE CASA - END


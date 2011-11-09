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

/////////////////////////////////////
/// FlagDataHandler implementation //
/////////////////////////////////////

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

	// Check if async i/o is enabled (double check for ROVisibilityIteratorAsync and FlagDataHandler config)
	asyncio_disabled_p = true;
	AipsrcValue<Bool>::find (asyncio_disabled_p,"ROVisibilityIteratorAsync.disabled", true);
	if (!asyncio_disabled_p)
	{
		// Check Flag Data Handler config
		Bool tmp = false;
		AipsrcValue<Bool>::find (tmp,"FlagDataHandler.asyncio", false);
		if (!tmp)
		{
			asyncio_disabled_p = true;
		}
	}

	// Check if slurp is enabled
	if (asyncio_disabled_p)
	{
		slurp_p = true;
		AipsrcValue<Bool>::find (slurp_p,"FlagDataHandler.slurp:", true);
	}
	else
	{
		slurp_p = false;
		AipsrcValue<Bool>::find (slurp_p,"FlagDataHandler.slurp:", false);
	}

	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ <<
			" Asyncio activated: " << !asyncio_disabled_p <<
			" Slurp activated: "<< slurp_p <<  LogIO::POST;


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

	// By default don't map polarizations
	mapPolarizations_p = false;

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
		case  ARRAY_FIELD:
		{
			*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Iteration mode: ARRAY_FIELD" << LogIO::POST;
			sortOrder_p = Block<int>(4);
			sortOrder_p[0] = MS::ARRAY_ID;
			sortOrder_p[1] = MS::FIELD_ID;
			sortOrder_p[2] = MS::DATA_DESC_ID;
			sortOrder_p[3] = MS::TIME;

			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = false;
			mapAntennaPairs_p = false;
			mapSubIntegrations_p = false;
			break;
		}
		default:
		{
			iterationApproach_p = SUB_INTEGRATION;

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
	}

	// Initialize selection ranges
	timeSelection_p = String("");
	baselineSelection_p = String("");
	fieldSelection_p = String("");
	spwSelection_p = String("*");
	uvwSelection_p = String("");
	polarizationSelection_p = String("");
	scanSelection_p = String("");
	arraySelection_p = String("");
	observationSelection_p = String("");
	scanIntentSelection_p = String("");

	// Initialize iteration parameters
	chunksInitialized_p = false;
	buffersInitialized_p = false;
	iteratorGenerated_p = false;
	chunkNo = 0;
	bufferNo = 0;

	// Initialize stats
	stats_p = false;
	cubeAccessCounter_p = 0;

	// Set all the initialized pointers to NULL
	selectedMeasurementSet_p = NULL;
	measurementSetSelection_p = NULL;
	originalMeasurementSet_p = NULL;
	antennaNames_p = NULL;
	rwVisibilityIterator_p = NULL;
	roVisibilityIterator_p = NULL;
	visibilityBuffer_p = NULL;
	subIntegrationMap_p = NULL;
	polarizationMap_p = NULL;
	polarizationIndexMap_p = NULL;
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
	if (polarizationMap_p) delete polarizationMap_p;
	if (polarizationIndexMap_p) delete polarizationIndexMap_p;
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	if (roVisibilityIterator_p) delete roVisibilityIterator_p;
	// Apparently there is a problem here, if we destroy the base RO iterator of a RW iterator the pointer is not set to NULL
	if ((!asyncio_disabled_p) and (rwVisibilityIterator_p)) delete rwVisibilityIterator_p;
	if (selectedMeasurementSet_p) delete selectedMeasurementSet_p;
	if (measurementSetSelection_p) delete measurementSetSelection_p;
	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	if (antennaNames_p) delete antennaNames_p;
	if (logger_p) delete logger_p;

	return;
}


// -----------------------------------------------------------------------
// Open Measurement Set
// -----------------------------------------------------------------------
bool
FlagDataHandler::open()
{
	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	originalMeasurementSet_p = new MeasurementSet(msname_p,Table::Update);

	// Activate Memory Resident Sub-tables for everything but Pointing, Syscal and History
	originalMeasurementSet_p->setMemoryResidentSubtables (MrsEligibility::defaultEligible());

	// Read antenna names from Antenna table
	const MSAntenna msant( originalMeasurementSet_p->antenna() );
	ROScalarColumn<String> names(msant,"NAME");
	if (antennaNames_p) delete antennaNames_p;
	antennaNames_p = new Vector<String>(names.getColumn());

	return true;
}


// -----------------------------------------------------------------------
// Close Measurement Set
// -----------------------------------------------------------------------
bool
FlagDataHandler::close()
{
	if (selectedMeasurementSet_p)
	{
		// Wait until all the pending writing operations are done
		if (!asyncio_disabled_p)
		{
			vwbt_p->terminate();
			vwbt_p->join();
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

	return true;
}


// -----------------------------------------------------------------------
// Set Data Selection parameters
// -----------------------------------------------------------------------
bool
FlagDataHandler::setDataSelection(Record record)
{
	int exists;

	exists = record.fieldNumber ("array");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("array"), arraySelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " array selection is " << arraySelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no array selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("field");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("field"), fieldSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " field selection is " << fieldSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no field selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("scan");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("scan"), scanSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " scan selection is " << scanSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no scan selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("timerange");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("timerange"), timeSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " timerange selection is " << timeSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no timerange selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("spw");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("spw"), spwSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " spw selection is " << spwSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no spw selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("antenna");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("antenna"), baselineSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " antenna selection is " << baselineSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no antenna selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("uvrange");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("uvrange"), uvwSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " uvrange selection is " << uvwSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no uvrange selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("correlation");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("correlation"), polarizationSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " correlation selection is " << polarizationSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no correlation selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("observation");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("observation"), observationSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " observationSelection selection is " << observationSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no observationSelection selection" << LogIO::POST;
	}

	exists = record.fieldNumber ("intent");
	if (exists >= 0)
	{
		record.get (record.fieldNumber ("intent"), scanIntentSelection_p);
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " scan intent selection is " << scanIntentSelection_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " no scan intent selection" << LogIO::POST;
	}

	return true;
}


// -----------------------------------------------------------------------
// Generate selected Measurement Set
// -----------------------------------------------------------------------
bool
FlagDataHandler::selectData()
{
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
			(const String)polarizationSelection_p,
			(const String)scanSelection_p,
			(const String)arraySelection_p,
			(const String)scanIntentSelection_p,
			(const String)observationSelection_p);

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

	// More debugging information from MS-Selection
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected array ids are " << measurementSetSelection_p->getSubArrayList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected observation ids are " << measurementSetSelection_p->getObservationList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected field ids are " << measurementSetSelection_p->getFieldList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected scan ids are " << measurementSetSelection_p->getScanList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected scan intent ids are " << measurementSetSelection_p->getStateObsModeList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected time range is " << measurementSetSelection_p->getTimeList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected spw-channels ids are " << measurementSetSelection_p->getChanList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected antenna1 ids are " << measurementSetSelection_p->getAntenna1List() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected antenna2 ids are " << measurementSetSelection_p->getAntenna2List() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected baselines are " << measurementSetSelection_p->getBaselineList() << LogIO::POST;
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected uv range is " << measurementSetSelection_p->getUVList() << LogIO::POST;

	ostringstream polarizationListToPrint (ios::in | ios::out);
	polarizationListToPrint << measurementSetSelection_p->getPolMap();
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Selected correlation ids are " << polarizationListToPrint.str() << LogIO::POST;

	return true;
}

// -----------------------------------------------------------------------
// Swap MS to check what is the maximum RAM memory needed
// -----------------------------------------------------------------------
void
FlagDataHandler::checkMaxMemory()
{
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

	return;
}


// -----------------------------------------------------------------------
// Generate Visibility Iterator with a given sort order and time interval
// -----------------------------------------------------------------------
bool
FlagDataHandler::generateIterator()
{
	// First create and initialize RW iterator
	if (rwVisibilityIterator_p) delete rwVisibilityIterator_p;
	rwVisibilityIterator_p = new VisibilityIterator(*selectedMeasurementSet_p,sortOrder_p,true,timeInterval_p);

	// Set the table data manager (ISM and SSM) cache size to the full column size, for
	// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
	if (slurp_p) rwVisibilityIterator_p->slurp();

	// Apply channel selection (Notice that is not necessary to do this again with the RO iterator un sync mode)
	applyChannelSelection(rwVisibilityIterator_p);

	checkMaxMemory();

	// If async I/O is enabled we create an async RO iterator for reading and a conventional RW iterator for writing
	// Both iterators share a mutex which is resident in the VLAT data (Visibility Look Ahead thread Data Object)
	// With this configuration the Visibility Buffer is attached to the RO async iterator
	if (asyncio_disabled_p)
	{
		// Cast RW conventional iterator into RO conventional iterator
		if (roVisibilityIterator_p) delete roVisibilityIterator_p;
		roVisibilityIterator_p = (ROVisibilityIterator*)rwVisibilityIterator_p;

		// Finally attach Visibility Buffer to RO conventional iterator
		if (visibilityBuffer_p) delete visibilityBuffer_p;
		visibilityBuffer_p = new VisBufferAutoPtr(roVisibilityIterator_p);
	}
	else
	{

	    throw AipsError ("FlagDataHandler.asyncio temporally disabled ", __FILE__, __LINE__);

		// Determine columns to be pre-fetched
//		ROVisibilityIteratorAsync::PrefetchColumns prefetchColumns = ROVisibilityIteratorAsync::prefetchColumns(casa::asyncio::Ant1,
//															casa::asyncio::Ant2,
//															casa::asyncio::ArrayId,
//															casa::asyncio::CorrType,
//															casa::asyncio::Feed1,
//															casa::asyncio::Feed2,
//															casa::asyncio::FieldId,
//															casa::asyncio::Flag,
//															casa::asyncio::FlagCube,
//															casa::asyncio::FlagRow,
//															casa::asyncio::ObservationId,
//															casa::asyncio::ObservedCube,
//															casa::asyncio::Freq,
//															casa::asyncio::NChannel,
//															casa::asyncio::NCorr,
//															casa::asyncio::NRow,
//															casa::asyncio::PhaseCenter,
//															casa::asyncio::Scan,
//															casa::asyncio::SpW,
//															casa::asyncio::StateId,
//															casa::asyncio::Time,
//															casa::asyncio::TimeInterval,
//															casa::asyncio::Uvw,-1);
//
//		// Then create and initialize RO Async iterator
//		if (roVisibilityIterator_p) delete roVisibilityIterator_p;
//		roVisibilityIterator_p = ROVisibilityIteratorAsync::create(*selectedMeasurementSet_p,prefetchColumns,sortOrder_p,true,timeInterval_p,-1,groupTimeSteps_p);
//
//		// Set the table data manager (ISM and SSM) cache size to the full column size, for
//		// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
//		if (slurp_p) roVisibilityIterator_p->slurp();
//
//		// Apply channel selection
//		applyChannelSelection(roVisibilityIterator_p);
//
//		// Attach Visibility Buffer to Visibility Iterator
//		if (visibilityBuffer_p) delete visibilityBuffer_p;
//		visibilityBuffer_p = new VisBufferAutoPtr(roVisibilityIterator_p);
//
//		// Reset ROVisibilityIteratorAsync in order to apply the channel selection
//		// NOTE: We have to do this before starting the flag agents,
//		// otherwise we have some seg. faults related with RegEx parser
//		// which is used when applying the ROVIA modifiers
//		roVisibilityIterator_p->originChunks();
//
//		// Finally, initialize Visibility Write Behind Thread
//		if (vwbt_p) delete vwbt_p;
//		ROVisibilityIteratorAsync * roVisibilityIteratorAsync = (ROVisibilityIteratorAsync*) roVisibilityIterator_p;
//		casa::async::Mutex * mutex = roVisibilityIteratorAsync->getMutex();
//		vwbt_p = new VWBT(rwVisibilityIterator_p,mutex,groupTimeSteps_p);
//		vwbt_p->start();

	}

	iteratorGenerated_p = true;

	return true;
}

void
FlagDataHandler::applyChannelSelection(ROVisibilityIterator *roVisIter)
{
	// Apply channel selection (in row selection cannot be done with MSSelection)
	// NOTE: Each row of the Matrix has the following elements: SpwID StartCh StopCh Step
	Matrix<Int> spwchan = measurementSetSelection_p->getChanList();
	Vector<Int> spwlist = measurementSetSelection_p->getSpwList();
	Int spw,channelStart,channelStop,channelStep,channelWidth;
	for(uInt spw_i=0;spw_i<spwlist.nelements();spw_i++ )
	{
		// NOTE: selectChannel needs channelStart,channelWidth,channelStep
		spw = spwlist[spw_i];
		channelStart = spwchan(spw_i,1);
		channelStop = spwchan(spw_i,2);
		channelStep = spwchan(spw_i,3);
		channelWidth = channelStop-channelStart+1;
		roVisIter->selectChannel(1,channelStart,channelWidth,channelStep,spw);
	}

	return;
}


// -----------------------------------------------------------------------
// Move to next chunk
// -----------------------------------------------------------------------
bool
FlagDataHandler::nextChunk()
{
	bool moreChunks = false;
	if (!chunksInitialized_p)
	{
		if (!iteratorGenerated_p) generateIterator();
		roVisibilityIterator_p->originChunks();
		chunksInitialized_p = true;
		buffersInitialized_p = false;
		chunkNo++;
		bufferNo = 0;
		moreChunks = true;
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

	return moreChunks;
}


// -----------------------------------------------------------------------
// Move to next buffer
// -----------------------------------------------------------------------
bool
FlagDataHandler::nextBuffer()
{
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
		if (mapPolarizations_p) generatePolarizationsMap();
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
			if (mapPolarizations_p) generatePolarizationsMap();
			moreBuffers = true;
			bufferNo++;
		}
	}

	// Set new common flag cube
	if (moreBuffers)
	{
		// WARNING: We have to modify the shape of the cube before re-assigning it
		Cube<Bool> modifiedflagCube= visibilityBuffer_p->get()->flagCube();
		modifiedFlagCube_p.resize(modifiedflagCube.shape());
		modifiedFlagCube_p = modifiedflagCube;
		const Cube<Bool> originalFlagCube= visibilityBuffer_p->get()->flagCube();
		originalFlagCube_p.resize(originalFlagCube.shape());
		originalFlagCube_p = originalFlagCube;

		//IPosition flagCubeShape = modifiedFlagCube_p.shape();
		//*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Current buffer shape: " << flagCubeShape  << " and indexing:" <<  modifiedFlagCube_p.printConfig() <<LogIO::POST;
	}

	return moreBuffers;
}


// -----------------------------------------------------------------------
// Flush flags to MS
// -----------------------------------------------------------------------
bool
FlagDataHandler::flushFlags()
{
	if (asyncio_disabled_p)
	{
		rwVisibilityIterator_p->setFlag(modifiedFlagCube_p);
	}
	else
	{
		vwbt_p->setFlag(modifiedFlagCube_p);
	}

	return true;
}


// -----------------------------------------------------------------------
// As requested by Urvashi R.V. provide access to the original and modified flag cubes
// -----------------------------------------------------------------------
Cube<Bool> *
FlagDataHandler::getModifiedFlagCube()
{
	return &modifiedFlagCube_p;
}

Cube<Bool> *
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

	return;
}


void
FlagDataHandler::generateSubIntegrationMap()
{
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

	return;
}


void
FlagDataHandler::generatePolarizationsMap()
{
	// Free previous map and create a new one
	if (polarizationMap_p) delete polarizationMap_p;
	polarizationMap_p = new polarizationMap();
	if (polarizationIndexMap_p) delete polarizationIndexMap_p;
	polarizationIndexMap_p = new polarizationIndexMap();

	uShort pos = 0;
	Vector<Int> corrTypes = visibilityBuffer_p->get()->corrType();
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Correlation type: " <<  corrTypes << LogIO::POST;

	for (Vector<Int>::iterator iter = corrTypes.begin(); iter != corrTypes.end();iter++)
	{
		switch (*iter)
		{
			case Stokes::I:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is I" << LogIO::POST;
				(*polarizationMap_p)[Stokes::I] = pos;
				(*polarizationIndexMap_p)[pos] = "I";
				break;
			}
			case Stokes::Q:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is Q" << LogIO::POST;
				(*polarizationMap_p)[Stokes::Q] = pos;
				(*polarizationIndexMap_p)[pos] = "Q";
				break;
			}
			case Stokes::U:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is U" << LogIO::POST;
				(*polarizationMap_p)[Stokes::U] = pos;
				(*polarizationIndexMap_p)[pos] = "U";
				break;
			}
			case Stokes::V:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is V" << LogIO::POST;
				(*polarizationMap_p)[Stokes::V] = pos;
				(*polarizationIndexMap_p)[pos] = "V";
				break;
			}
			case Stokes::XX:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is XX" << LogIO::POST;
				(*polarizationMap_p)[Stokes::XX] = pos;
				(*polarizationIndexMap_p)[pos] = "XX";
				break;
			}
			case Stokes::YY:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is YY" << LogIO::POST;
				(*polarizationMap_p)[Stokes::YY] = pos;
				(*polarizationIndexMap_p)[pos] = "YY";
				break;
			}
			case Stokes::XY:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is XY" << LogIO::POST;
				(*polarizationMap_p)[Stokes::XY] = pos;
				(*polarizationIndexMap_p)[pos] = "XY";
				break;
			}
			case Stokes::YX:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is YX" << LogIO::POST;
				(*polarizationMap_p)[Stokes::YX] = pos;
				(*polarizationIndexMap_p)[pos] = "YX";
				break;
			}
			case Stokes::RR:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is RR" << LogIO::POST;
				(*polarizationMap_p)[Stokes::RR] = pos;
				(*polarizationIndexMap_p)[pos] = "RR";
				break;
			}
			case Stokes::LL:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is LL" << LogIO::POST;
				(*polarizationMap_p)[Stokes::LL] = pos;
				(*polarizationIndexMap_p)[pos] = "LL";
				break;
			}
			case Stokes::RL:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is RL" << LogIO::POST;
				(*polarizationMap_p)[Stokes::RL] = pos;
				(*polarizationIndexMap_p)[pos] = "RL";
				break;
			}
			case Stokes::LR:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is LR" << LogIO::POST;
				(*polarizationMap_p)[Stokes::LR] = pos;
				(*polarizationIndexMap_p)[pos] = "LR";
				break;
			}
			default:
			{
				*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " The " << pos << " th correlation is unknown: " << *iter << LogIO::POST;
				break;
			}
		}
		pos++;
	}

	for (polarizationMap::iterator iter =polarizationMap_p->begin();iter != polarizationMap_p->end();iter++)
	{
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ << " Polarization map key: " << iter->first << " value: " << iter->second << LogIO::POST;
	}

	return;
}

CubeView<Bool> *
FlagDataHandler::getFlagsView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Bool> * cube= new CubeView<Bool>(&modifiedFlagCube_p,rows);
	return cube;
}

CubeView<Bool> *
FlagDataHandler::getFlagsView(Double timestep)
{
	std::vector<uInt> *rows = &((*subIntegrationMap_p)[timestep]);
	CubeView<Bool> * cube= new CubeView<Bool>(&modifiedFlagCube_p,rows);
	return cube;
}

CubeView<Complex> *
FlagDataHandler::getVisibilitiesView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Complex> * cube= new CubeView<Complex>(&(visibilityBuffer_p->get()->visCube()),rows);
	return cube;
}

CubeView<Complex> *
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
	stats_p = true;

	antennaPairMapIterator 	myAntennaPairMapIterator;
	Double timestep;
	subIntegrationMapIterator mySubIntegrationMapIterator;
	std::pair<Int,Int> antennaPair;
	uShort processView = rotateViews;
	IPosition flagCubeShape;
	CubeView<Bool> *flagCubeView;
	switch (rotateMode)
	{
		case 0:
			flagCubeView = new CubeView<Bool>(&modifiedFlagCube_p);
			fillBuffer(*flagCubeView,write,0);
			delete flagCubeView;
		break;

		case 1:
			for (myAntennaPairMapIterator=antennaPairMap_p->begin(); myAntennaPairMapIterator != antennaPairMap_p->end(); ++myAntennaPairMapIterator) {
				antennaPair = myAntennaPairMapIterator->first;
				flagCubeView = getFlagsView(antennaPair.first,antennaPair.second);
				flagCubeShape = flagCubeView->shape();
				*logger_p 	<< LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__
							<< " Flag cube for (" <<  antennaPair.first << "," << antennaPair.second << ") has shape ["
						    << flagCubeShape(0) << "," <<  flagCubeShape(1) << "," << flagCubeShape(2) << "]" << LogIO::POST;

				if (rotateViews == 0)
				{
					fillBuffer(*flagCubeView,write,0);
				}
				else
				{
					if (processView == 1)
					{
						fillBuffer(*flagCubeView,write,1);
						processView = 2;
					}
					else
					{
						fillBuffer(*flagCubeView,write,2);
						processView = 1;
					}
				}

				delete flagCubeView;
			}
		break;

		case 2:
			for (mySubIntegrationMapIterator=subIntegrationMap_p->begin(); mySubIntegrationMapIterator != subIntegrationMap_p->end(); ++mySubIntegrationMapIterator) {
				timestep = mySubIntegrationMapIterator->first;
				flagCubeView = getFlagsView(timestep);
				flagCubeShape = flagCubeView->shape();
				*logger_p 	<< LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__
							<< " Flag cube for (" <<  timestep << ") has shape ["
						    << flagCubeShape(0) << "," <<  flagCubeShape(1) << "," << flagCubeShape(2) << "]" << LogIO::POST;

				if (rotateViews == 0)
				{
					fillBuffer(*flagCubeView,write,0);
				}
				else
				{
					if (processView == 1)
					{
						fillBuffer(*flagCubeView,write,1);
						processView = 2;
					}
					else
					{
						fillBuffer(*flagCubeView,write,2);
						processView = 1;
					}
				}

				delete flagCubeView;
			}
		break;

		default:
		break;
	}

	return processView;
}

void
FlagDataHandler::fillBuffer(CubeView<Bool> &flagCube,bool write, uShort processBuffer)
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
										<< " channel " << chan_j << " row " << row_k << LogIO::POST;
						}
					} else {
						if (flag != False)
						{
							*logger_p 	<< LogIO::SEVERE << "FlagDataHandler::" << __FUNCTION__
										<<" Wrong flag (True instead of False) in chunk " << chunkNo
										<< " buffer " << bufferNo << " polarization " << pol_i
										<< " channel " << chan_j << " row " << row_k << LogIO::POST;
						}
					}
				}
			}
		}
	}

	return;
}

/////////////////////////////////////
/// VisMapper implementation ////////
/////////////////////////////////////
VisMapper::VisMapper(String expression,polarizationMap *polMap,CubeView<Complex> *leftVis,CubeView<Complex> *rightVis)
{
	rightVis_p = NULL;
	leftVis_p = NULL;
	setParentCubes(leftVis,rightVis);
	setExpressionMapping(expression,polMap);
}

VisMapper::VisMapper(String expression,polarizationMap *polMap)
{
	rightVis_p = NULL;
	leftVis_p = NULL;
	setExpressionMapping(expression,polMap);
}

void
VisMapper::setParentCubes(CubeView<Complex> *leftVis,CubeView<Complex> *rightVis)
{
	if (rightVis_p != NULL) delete rightVis_p;
	if (leftVis_p != NULL) delete leftVis_p;

	leftVis_p = leftVis;
	IPosition leftVisSize = leftVis->shape(); // pol,chan,row
	reducedLength_p = IPosition(2);
	reducedLength_p(0) = leftVisSize(1); // chan
	reducedLength_p(1) = leftVisSize(2); // row


	if (rightVis != NULL)
	{
		rightVis_p = rightVis;
		getVis_p = &VisMapper::diffVis;
	}
	else
	{
		rightVis_p = NULL;
		getVis_p = &VisMapper::leftVis;
	}
}

void
VisMapper::setExpressionMapping(String expression,polarizationMap *polMap)
{
	selectedCorrelations_p.clear();
	expression_p = expression;
	polMap_p = polMap;

	// Parse complex unitary function
	if (expression_p.find("real") != string::npos)
	{
		applyVisExpr_p = &VisMapper::real;
	}
	else if (expression_p.find("imag") != string::npos)
	{
		applyVisExpr_p = &VisMapper::imag;
	}
	else if (expression_p.find("arg") != string::npos)
	{
		applyVisExpr_p = &VisMapper::arg;
	}
	else if (expression_p.find("abs") != string::npos)
	{
		applyVisExpr_p = &VisMapper::abs;
	}
	else if (expression_p.find("norm") != string::npos)
	{
		applyVisExpr_p = &VisMapper::norm;
	}
	else
	{
		applyVisExpr_p = &VisMapper::abs;
	}

	// Parse polarization
	if (expression_p.find("XX") != string::npos)
	{
		if (polMap_p->find(Stokes::XX) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::XX]);
			getCorr_p = &VisMapper::linear_xx;
		}
		else
		{
			throw AipsError("Requested polarization parameter (XX) not available");
		}
	}
	else if (expression_p.find("YY") != string::npos)
	{
		if (polMap_p->find(Stokes::YY) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::YY]);
			getCorr_p = &VisMapper::linear_yy;
		}
		else
		{
			throw AipsError("Requested polarization parameter (YY) not available");
		}

	}
	else if (expression_p.find("XY") != string::npos)
	{
		if (polMap_p->find(Stokes::XY) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::XY]);
			getCorr_p = &VisMapper::linear_xy;
		}
		else
		{
			throw AipsError("Requested polarization parameter (XY) not available");
		}
	}
	else if (expression_p.find("YX") != string::npos)
	{
		if (polMap_p->find(Stokes::YX) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::YX]);
			getCorr_p = &VisMapper::linear_yx;
		}
		else
		{
			throw AipsError("Requested polarization parameter (YX) not available");
		}
	}
	else if (expression_p.find("RR") != string::npos)
	{
		if (polMap_p->find(Stokes::RR) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::RR]);
			getCorr_p = &VisMapper::circular_rr;
		}
		else
		{
			throw AipsError("Requested polarization parameter (RR) not available");
		}
	}
	else if (expression_p.find("LL") != string::npos)
	{
		if (polMap_p->find(Stokes::LL) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::LL]);
			getCorr_p = &VisMapper::circular_ll;
		}
		else
		{
			throw AipsError("Requested polarization parameter (LL) not available");
		}
	}
	else if (expression_p.find("LR") != string::npos)
	{
		if (polMap_p->find(Stokes::LR) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::LR]);
			getCorr_p = &VisMapper::circular_lr;
		}
		else
		{
			throw AipsError("Requested polarization parameter (LR) not available");
		}
	}
	else if (expression_p.find("RL") != string::npos)
	{
		if (polMap_p->find(Stokes::RL) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::RL]);
			getCorr_p = &VisMapper::circular_rl;
		}
		else
		{
			throw AipsError("Requested polarization parameter (RL) not available");
		}
	}
	else if (expression_p.find("I") != string::npos)
	{
		if (polMap_p->find(Stokes::I) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::I]);
			getCorr_p = &VisMapper::stokes_i;
		}
		else if ((polMap_p->find(Stokes::XX) != polMap_p->end()) and (polMap_p->find(Stokes::YY) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::XX]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::YY]);
			getCorr_p = &VisMapper::stokes_i_from_linear;
		}
		else if ((polMap_p->find(Stokes::RR) != polMap_p->end()) and (polMap_p->find(Stokes::LL) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::RR]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::LL]);
			getCorr_p = &VisMapper::stokes_i_from_circular;
		}
		else
		{
			throw AipsError("Requested Stokes parameter (I) cannot be computed from available polarizations");
		}
	}
	else if (expression_p.find("Q") != string::npos)
	{
		if (polMap_p->find(Stokes::Q) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::Q]);
			getCorr_p = &VisMapper::stokes_q;
		}
		else if ((polMap_p->find(Stokes::XX) != polMap_p->end()) and (polMap_p->find(Stokes::YY) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::XX]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::YY]);
			getCorr_p = &VisMapper::stokes_q_from_linear;
		}
		else if ((polMap_p->find(Stokes::RL) != polMap_p->end()) and (polMap_p->find(Stokes::LR) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::RL]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::LR]);
			getCorr_p = &VisMapper::stokes_q_from_circular;
		}
		else
		{
			throw AipsError("Requested Stokes parameter (Q) cannot be computed from available polarizations");
		}
	}
	else if (expression_p.find("U") != string::npos)
	{
		if (polMap_p->find(Stokes::U) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::U]);
			getCorr_p = &VisMapper::stokes_u;
		}
		else if ((polMap_p->find(Stokes::XY) != polMap_p->end()) and (polMap_p->find(Stokes::YX) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::XY]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::YX]);
			getCorr_p = &VisMapper::stokes_u_from_linear;
		}
		else if ((polMap_p->find(Stokes::RL) != polMap_p->end()) and (polMap_p->find(Stokes::LR) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::RL]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::LR]);
			getCorr_p = &VisMapper::stokes_u_from_circular;
		}
		else
		{
			throw AipsError("Requested Stokes parameter (U) cannot be computed from available polarizations");
		}
	}
	else if (expression_p.find("V") != string::npos)
	{
		if (polMap_p->find(Stokes::V) != polMap_p->end())
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::V]);
			getCorr_p = &VisMapper::stokes_v;
		}
		else if ((polMap_p->find(Stokes::XY) != polMap_p->end()) and (polMap_p->find(Stokes::YX) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::XY]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::YX]);
			getCorr_p = &VisMapper::stokes_v_from_linear;
		}
		else if ((polMap_p->find(Stokes::RR) != polMap_p->end()) and (polMap_p->find(Stokes::LL) != polMap_p->end()))
		{
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::RR]);
			selectedCorrelations_p.push_back((*polMap_p)[Stokes::LL]);
			getCorr_p = &VisMapper::stokes_v_from_circular;
		}
		else
		{
			throw AipsError("Requested Stokes parameter (V) cannot be computed from available polarizations");
		}
	}
	else
	{
		throw AipsError("Unknown polarization requested, (" + expression_p + ") supported types are: XX,YY,XY,YX,RR,LL,RL,LR,I,Q,U,V");
	}
}


VisMapper::~VisMapper()
{
	if (rightVis_p != NULL) delete rightVis_p;
	if (leftVis_p != NULL) delete leftVis_p;
}

Float
VisMapper::operator()(uInt chan, uInt row)
{
	Complex val = (*this.*getCorr_p)(chan,row);
	return (*this.*applyVisExpr_p)(val);
}

Complex
VisMapper::leftVis(uInt pol, uInt chan, uInt row)
{
	return leftVis_p->operator()(pol,chan,row);
}

Complex
VisMapper::diffVis(uInt pol, uInt chan, uInt row)
{
	return leftVis_p->operator()(pol,chan,row)-rightVis_p->operator()(pol,chan,row);
}

Complex
VisMapper::stokes_i(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::I],chan,row);
}

Complex
VisMapper::stokes_q(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::Q],chan,row);
}

Complex
VisMapper::stokes_u(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::U],chan,row);
}

Complex
VisMapper::stokes_v(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::V],chan,row);
}

Complex
VisMapper::linear_xx(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::XX],chan,row);
}

Complex
VisMapper::linear_yy(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::YY],chan,row);
}

Complex
VisMapper::linear_xy(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::XY],chan,row);
}

Complex
VisMapper::linear_yx(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::YX],chan,row);
}

Complex
VisMapper::circular_rr(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::RR],chan,row);
}

Complex
VisMapper::circular_ll(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::LL],chan,row);
}

Complex
VisMapper::circular_rl(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::RL],chan,row);
}

Complex
VisMapper::circular_lr(uInt chan, uInt row)
{
	return (*this.*getVis_p)((*polMap_p)[Stokes::LR],chan,row);
}

Complex
VisMapper::stokes_i_from_linear(uInt chan, uInt row)
{
	// I = (XX+YY)/2
	return ((*this.*getVis_p)((*polMap_p)[Stokes::XX],chan,row) + (*this.*getVis_p)((*polMap_p)[Stokes::YY],chan,row))/2;
}

Complex
VisMapper::stokes_q_from_linear(uInt chan, uInt row)
{
	// Q = (XX-YY)/2
	return ((*this.*getVis_p)((*polMap_p)[Stokes::XX],chan,row) - (*this.*getVis_p)((*polMap_p)[Stokes::YY],chan,row))/2;
}

Complex
VisMapper::stokes_u_from_linear(uInt chan, uInt row)
{
	// U = (XY-YX)/2
	return ((*this.*getVis_p)((*polMap_p)[Stokes::XY],chan,row) - (*this.*getVis_p)((*polMap_p)[Stokes::YX],chan,row))/2;
}

Complex
VisMapper::stokes_v_from_linear(uInt chan, uInt row)
{
	// V = (XY-YX)/2i
	return ((*this.*getVis_p)((*polMap_p)[Stokes::XY],chan,row) - (*this.*getVis_p)((*polMap_p)[Stokes::YX],chan,row))/(2*ImaginaryUnit);
}

Complex
VisMapper::stokes_i_from_circular(uInt chan, uInt row)
{
	// I = (RR+LL)/2
	return ((*this.*getVis_p)((*polMap_p)[Stokes::RR],chan,row) + (*this.*getVis_p)((*polMap_p)[Stokes::LL],chan,row))/2;
}

Complex
VisMapper::stokes_q_from_circular(uInt chan, uInt row)
{
	// Q = (RL-LR)/2
	return ((*this.*getVis_p)((*polMap_p)[Stokes::RL],chan,row) - (*this.*getVis_p)((*polMap_p)[Stokes::LR],chan,row))/2;
}

Complex
VisMapper::stokes_u_from_circular(uInt chan, uInt row)
{
	// U = (RL-LR)/2i
	return ((*this.*getVis_p)((*polMap_p)[Stokes::RL],chan,row) - (*this.*getVis_p)((*polMap_p)[Stokes::LR],chan,row))/(2*ImaginaryUnit);
}

Complex
VisMapper::stokes_v_from_circular(uInt chan, uInt row)
{
	// V = (RR-LL)/2
	return ((*this.*getVis_p)((*polMap_p)[Stokes::RR],chan,row) - (*this.*getVis_p)((*polMap_p)[Stokes::LL],chan,row))/2;
}

//////////////////////////////////////
/// FlagMapper implementation ////////
//////////////////////////////////////
FlagMapper::FlagMapper(Bool flag, vector<uInt> selectedCorrelations, CubeView<Bool> *commonFlagsView,CubeView<Bool> *privateFlagsView)
{
	commonFlagsView_p = NULL;
	privateFlagsView_p = NULL;
	setParentCubes(commonFlagsView,privateFlagsView);
	setExpressionMapping(selectedCorrelations);
	flag_p = flag;
}

FlagMapper::FlagMapper(Bool flag, vector<uInt> selectedCorrelations)
{
	commonFlagsView_p = NULL;
	privateFlagsView_p = NULL;
	setExpressionMapping(selectedCorrelations);
	flag_p = flag;
}

void
FlagMapper::setParentCubes(CubeView<Bool> *commonFlagsView,CubeView<Bool> *privateFlagsView)
{
	if (commonFlagsView_p != NULL) delete commonFlagsView_p;
	if (privateFlagsView_p != NULL) delete privateFlagsView_p;

	commonFlagsView_p = commonFlagsView;
	if (privateFlagsView != NULL)
	{
		privateFlagsView_p = privateFlagsView;
		applyFlag_p = &FlagMapper::applyPrivateFlags;
	}
	else
	{
		applyFlag_p = &FlagMapper::applyCommonFlags;
	}

	IPosition commonFlagCubeSize = commonFlagsView_p->shape(); // pol,chan,row
	reducedLength_p = IPosition(2);
	reducedLength_p(0) = commonFlagCubeSize(1); // chan
	reducedLength_p(1) = commonFlagCubeSize(2); // row
}

void
FlagMapper::setExpressionMapping(vector<uInt> selectedCorrelations)
{
	selectedCorrelations_p = selectedCorrelations;
	if (selectedCorrelations_p.empty())
	{
		throw AipsError("None of the correlations is eligible for flagging");
	}
}

FlagMapper::~FlagMapper()
{
	if (commonFlagsView_p != NULL) delete commonFlagsView_p;
	if (privateFlagsView_p != NULL) delete privateFlagsView_p;
}

Bool
FlagMapper::operator()(uInt channel, uInt row)
{
	Bool combinedFlag = False;
	for (vector<uInt>::iterator iter=selectedCorrelations_p.begin();iter!=selectedCorrelations_p.end();iter++)
	{
		combinedFlag = combinedFlag | commonFlagsView_p->operator ()(*iter,channel,row);
	}

	return combinedFlag;
}

Bool
FlagMapper::operator()(uInt pol, uInt channel, uInt row)
{
	return commonFlagsView_p->operator ()(pol,channel,row);
}

void
FlagMapper::applyFlag(uInt channel, uInt row)
{
	for (vector<uInt>::iterator iter=selectedCorrelations_p.begin();iter!=selectedCorrelations_p.end();iter++)
	{
		(*this.*applyFlag_p)(*iter,channel,row);
	}
}

void
FlagMapper::applyCommonFlags(uInt pol, uInt channel, uInt row)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagsView_p->operator()(pol,channel,row) = flag_p;
	return;
}

void
FlagMapper::applyPrivateFlags(uInt pol, uInt channel, uInt row)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagsView_p->operator()(pol,channel,row) = flag_p;
	privateFlagsView_p->operator()(pol,channel,row) = flag_p;
	return;
}

void
FlagMapper::checkCommonFlags(uInt pol, uInt channel, uInt row)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	if (commonFlagsView_p->operator()(pol,channel,row) != flag_p)
	{
		cerr << "FlagMapper::" << __FUNCTION__ <<  " Flag missmatch at pol=" << pol << ",channel=" << channel << ",row=" << row << endl;
	}
	return;
}

} //# NAMESPACE CASA - END


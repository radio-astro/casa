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
FlagDataHandler::FlagDataHandler(uShort iterationApproach): iterationApproach_p(iterationApproach)
{
	// Default verbosity
	profiling_p = true;

	// Check if async input is enabled
	asyncio_disabled_p = true;
	AipsrcValue<Bool>::find (asyncio_disabled_p,"ROVisibilityIteratorAsync.disabled", true);

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
		case SCAN_SUB_INTEGRATION:
		{
			cout << "SCAN_SUB_INTEGRATION" << endl;
			sortOrder_p = Block<int>(6);
			sortOrder_p[0] = MS::OBSERVATION_ID;
			sortOrder_p[1] = MS::ARRAY_ID;
			sortOrder_p[2] = MS::SCAN_NUMBER;
			sortOrder_p[3] = MS::FIELD_ID;
			sortOrder_p[4] = MS::DATA_DESC_ID;
			sortOrder_p[5] = MS::TIME;

			// NOTE: Time interval 0 groups all time steps together in one chunk.
			timeInterval_p = 0;
			// NOTE: groupTimeSteps_p=false selects only one time step per buffer
			groupTimeSteps_p = true;
			break;
		}
		case BASELINE_SCAN_TIME_SERIES:
		{
			cout << "BASELINE_SCAN_TIME_SERIES" << endl;
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

			// NOTE: Time interval 0 groups all time steps together in one chunk.
			timeInterval_p = 0;
			// NOTE: groupTimeSteps_p=true groups all time steps together in one buffer.
			groupTimeSteps_p = true;
			break;
		}
		default:
		{
			cout << "DEFAULT" << endl;
			sortOrder_p = Block<int>(4);
			sortOrder_p[0] = MS::ARRAY_ID;
			sortOrder_p[1] = MS::FIELD_ID;
			sortOrder_p[2] = MS::DATA_DESC_ID;
			sortOrder_p[3] = MS::TIME;
			timeInterval_p = 0;
			groupTimeSteps_p = false;
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

	// Initialize logger
	logger_p = new LogIO();

	// Set all the initialized pointers to NULL
	selectedMeasurementSet_p = NULL;
	measurementSetSelection_p = NULL;
	originalMeasurementSet_p = NULL;
	rwVisibilityIterator_p = NULL;
	roVisibilityIterator_p = NULL;
	visibilityBuffer_p = NULL;
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
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	if (roVisibilityIterator_p) delete roVisibilityIterator_p;
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
FlagDataHandler::open(string msname)
{
	STARTCLOCK

	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	originalMeasurementSet_p = new MeasurementSet(String(msname),Table::Update);

	// Activate Memory Resident Subtables for everything but Pointing, Syscal and History
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
		selectedMeasurementSet_p->flush();
		selectedMeasurementSet_p->relinquishAutoLocks(True);
		selectedMeasurementSet_p->unlock();
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
// Generate Visibility Iterator with a given sort order and time interval
// -----------------------------------------------------------------------
void
FlagDataHandler::generateIterator()
{
	STARTCLOCK

	// First create and initialize RW iterator
	if (rwVisibilityIterator_p) delete rwVisibilityIterator_p;
	rwVisibilityIterator_p = new VisibilityIterator(*selectedMeasurementSet_p,sortOrder_p,true,timeInterval_p);

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
// Dummy function to simulate processing
// -----------------------------------------------------------------------
void
FlagDataHandler::processBuffer(bool write)
{
	STARTCLOCK

	// Try to extract flagCube from RO async iterator and set it in RW async iterator
	IPosition flagCubeShape = modifiedFlagCube_p.shape();
	Int nPolarizations = flagCubeShape(0);
	Int nChannels = flagCubeShape(1);
	Int nRows = flagCubeShape(2);
	for (Int pol_i=0;pol_i<nPolarizations;pol_i++) {
		for (Int chan_j=0;chan_j<nChannels;chan_j++) {
			for (Int row_k=0;row_k<nRows;row_k++) {
				// Flag each other chunks (i.e. even chunks)
				if (write)
				{
					if (chunkNo % 2 == 0) {
						modifiedFlagCube_p(pol_i,chan_j,row_k) = True;
					} else {
						modifiedFlagCube_p(pol_i,chan_j,row_k) = False;
					}
				}
				else
				{
					if (chunkNo % 2 == 0) {
						if (modifiedFlagCube_p(pol_i,chan_j,row_k) != True)
						{
							*logger_p 	<< LogIO::SEVERE << "FlagDataHandler::" << __FUNCTION__
										<<" Wrong flag (False instead of True) in chunk " << chunkNo
										<< " buffer " << bufferNo << " polarization " << pol_i
										<< " channel " << nChannels << " row " << row_k << LogIO::POST;
						}
					} else {
						if (modifiedFlagCube_p(pol_i,chan_j,row_k) != False)
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

	STOPCLOCK
	return;
}


} //# NAMESPACE CASA - END


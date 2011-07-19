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
	profiling_p = false;
	debug_p = false;

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
			groupTimeSteps_p = false;
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
	visibilityIterator_p = NULL;
        selectedMeasurementSet_p = NULL;
        measurementSetSelection_p = NULL;
        originalMeasurementSet_p = NULL;
	visibilityBuffer_p = NULL;

	return;
}

// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
FlagDataHandler::~FlagDataHandler()
{
	// Destroy members
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	if (visibilityIterator_p) delete visibilityIterator_p;
	if (selectedMeasurementSet_p) delete selectedMeasurementSet_p;
	if (measurementSetSelection_p) delete measurementSetSelection_p;
	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	if (logger_p) delete logger_p;

	return;
}

// -----------------------------------------------------------------------
// Open Measurement Set
// -----------------------------------------------------------------------
bool
FlagDataHandler::open(const std::string& msname)
{
	STARTCLOCK

	originalMeasurementSet_p = new MeasurementSet(String(msname),Table::Update);

	STOPCLOCK
	return true;
}


// -----------------------------------------------------------------------
// Generate selected Measurement Set
// -----------------------------------------------------------------------
bool
FlagDataHandler::selectData()
{
	STARTCLOCK

	// Create Measurement Selection object
	const String dummyExpr = String("");
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
	selectedMeasurementSet_p = new MeasurementSet(auxMeasurementSet);

	// Check if selected MS has rows...
	if (selectedMeasurementSet_p->nrow() == 0)
	{
		*logger_p << LogIO::WARN << "Selected Measurement Set doesn't have any rows " << LogIO::POST;
	}

	// Some debugging information
	if (debug_p)
	{
		cout << "Selected Measurement Set has " << measurementSetSelection_p->getSubArrayList() << " arrays" << endl;
		cout << "Selected Measurement Set has " << measurementSetSelection_p->getFieldList() << " fields" << endl;
		cout << "Selected Measurement Set has " << measurementSetSelection_p->getScanList() << " scans" << endl;
		cout << "Selected Measurement Set has " << measurementSetSelection_p->getSpwList() << " spws" << endl;
		cout << "Selected Measurement Set has " << measurementSetSelection_p->getChanList() << " channels" << endl;
		cout << "Selected Measurement Set has " << selectedMeasurementSet_p->nrow() << " rows" << endl;
	}

	STOPCLOCK
	return true;
}

// -----------------------------------------------------------------------
// Generate Visibility Iterator with a given sort order and time interval
// -----------------------------------------------------------------------
bool
FlagDataHandler::generateIterator()
{
	STARTCLOCK

	// Delete previous Visibility Iterator
	if (visibilityIterator_p) delete visibilityIterator_p;

	// Create new Visibility Iterator
	// WARNING: The 3rd parameters refers to the default columns which are ARRAY_ID and FIELD_ID,DATA_DESC_ID and TIME
	visibilityIterator_p = new VisIter(*selectedMeasurementSet_p,sortOrder_p,true,timeInterval_p);

	// Set the table data manager (ISM and SSM) cache size to the full column size, for the columns
	// ANTENNA1, ANTENNA2, FvisibilityBuffer_p = VisBuffer(*visibilityIterator_p);
	visibilityIterator_p->slurp();

	// Attach Visibility Buffer to Visibility Iterator
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	visibilityBuffer_p = new VisBuffer(*visibilityIterator_p);

	STOPCLOCK	
	return true;
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
		visibilityIterator_p->originChunks();
		chunksInitialized_p = true;
		buffersInitialized_p = false;
		moreChunks = true;
		chunkNo++;
		bufferNo = 0;
	}
	else if (visibilityIterator_p->moreChunks())
	{
		visibilityIterator_p->nextChunk();
		buffersInitialized_p = false;
		moreChunks = true;
		chunkNo++;
		bufferNo = 0;
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
		if (groupTimeSteps_p)
		{
			Int nRowChunk = visibilityIterator_p->nRowChunk();
	    		visibilityIterator_p->setRowBlocking(nRowChunk);
		}
		visibilityIterator_p->origin();
		buffersInitialized_p = true;
		moreBuffers = true;
		bufferNo++;
	}
	else
	{
		// WARNING: ++ operator is defined for VisibilityIterator class ("advance" function)
		// but if you define a VisibilityIterator pointer, then  ++ operator does not call
		// the advance function but increments the pointers.
		(*visibilityIterator_p)++;

		// WARNING: We iterate and afterwards check if the iterator is valid
		if (visibilityIterator_p->more())
		{
			moreBuffers = true;
			bufferNo++;
		}
	}

	STOPCLOCK
	return moreBuffers;
}


} //# NAMESPACE CASA - END


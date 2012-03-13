//# FlagMSHandler.h: This file contains the implementation of the FlagMSHandler class.
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

#include <flagging/Flagging/FlagMSHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

////////////////////////////////////
/// FlagMSHandler implementation ///
////////////////////////////////////


// -----------------------------------------------------------------------
// Default constructor
// -----------------------------------------------------------------------
FlagMSHandler::FlagMSHandler(string msname, uShort iterationApproach, Double timeInterval):
		FlagDataHandler(msname,iterationApproach,timeInterval)
{
	selectedMeasurementSet_p = NULL;
	originalMeasurementSet_p = NULL;
	rwVisibilityIterator_p = NULL;
	roVisibilityIterator_p = NULL;
}


// -----------------------------------------------------------------------
// Default destructor
// -----------------------------------------------------------------------
FlagMSHandler::~FlagMSHandler()
{
	logger_p->origin(LogOrigin("FlagDataHandler",__FUNCTION__,WHERE));
	*logger_p << LogIO::DEBUG1 << "FlagMSHandler::~FlagMSHandler()" << LogIO::POST;

	// Delete MS objects
	if (selectedMeasurementSet_p) delete selectedMeasurementSet_p;
	if (originalMeasurementSet_p) delete originalMeasurementSet_p;

	// Delete VisBuffers and iterators
	if (visibilityBuffer_p) delete visibilityBuffer_p;
	// ROVisIter is in fact RWVisIter
	if (rwVisibilityIterator_p) delete rwVisibilityIterator_p;

	return;
}


// -----------------------------------------------------------------------
// Open Measurement Set
// -----------------------------------------------------------------------
bool
FlagMSHandler::open()
{
	logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

	if (originalMeasurementSet_p) delete originalMeasurementSet_p;
	originalMeasurementSet_p = new MeasurementSet(msname_p,Table::Update);

	// Activate Memory Resident Sub-tables for everything but Pointing, Syscal and History
	originalMeasurementSet_p->setMemoryResidentSubtables (MrsEligibility::defaultEligible());

	// Read antenna names and diameters from Antenna table
	ROMSAntennaColumns *antennaSubTable = new ROMSAntennaColumns(originalMeasurementSet_p->antenna());
	antennaNames_p = new Vector<String>(antennaSubTable->name().getColumn());
	antennaDiameters_p = new Vector<Double>(antennaSubTable->dishDiameter().getColumn());
	antennaPositions_p = new ROScalarMeasColumn<MPosition>(antennaSubTable->positionMeas());

	// Read field names
	ROMSFieldColumns *fieldSubTable = new ROMSFieldColumns(originalMeasurementSet_p->field());
	fieldNames_p = new Vector<String>(fieldSubTable->name().getColumn());

	// Read polarizations
	ROMSPolarizationColumns *polarizationSubTable = new ROMSPolarizationColumns(originalMeasurementSet_p->polarization());
	ROArrayColumn<Int> corrTypeColum = polarizationSubTable->corrType();
	corrProducts_p = new std::vector<String>();
	for (uInt polRow_idx=0;polRow_idx<corrTypeColum.nrow();polRow_idx++)
	{
		Array<Int> polRow = corrTypeColum(polRow_idx);
		for (uInt corr_i=0;corr_i<polRow.size();corr_i++)
		{
			switch (polRow(IPosition(1,corr_i)))
			{
				case Stokes::I:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product I found, which should correspond to ALMA WVR data - skipping" << LogIO::POST;
					// corrProducts_p->push_back("I");
					break;
				}
				case Stokes::Q:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product Q found" << LogIO::POST;
					corrProducts_p->push_back("Q");
					break;
				}
				case Stokes::U:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product U found" << LogIO::POST;
					corrProducts_p->push_back("U");
					break;
				}
				case Stokes::V:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product V found" << LogIO::POST;
					corrProducts_p->push_back("V");
					break;
				}
				case Stokes::XX:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product XX found" << LogIO::POST;
					corrProducts_p->push_back("XX");
					break;
				}
				case Stokes::YY:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product YY found" << LogIO::POST;
					corrProducts_p->push_back("YY");
					break;
				}
				case Stokes::XY:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product XY found" << LogIO::POST;
					corrProducts_p->push_back("XY");
					break;
				}
				case Stokes::YX:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product YX found" << LogIO::POST;
					corrProducts_p->push_back("YX");
					break;
				}
				case Stokes::RR:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product RR found" << LogIO::POST;
					corrProducts_p->push_back("RR");
					break;
				}
				case Stokes::LL:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product LL found" << LogIO::POST;
					corrProducts_p->push_back("LL");
					break;
				}
				case Stokes::RL:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product RL found" << LogIO::POST;
					corrProducts_p->push_back("RL");
					break;
				}
				case Stokes::LR:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product LR found" << LogIO::POST;
					corrProducts_p->push_back("LR");
					break;
				}
				default:
				{
					*logger_p << LogIO::WARN << " Correlation product unknown found: " << polRow(IPosition(1,corr_i)) << LogIO::POST;
					break;
				}
			}
		}
	}

	return true;
}


// -----------------------------------------------------------------------
// Close Measurement Set
// -----------------------------------------------------------------------
bool
FlagMSHandler::close()
{
	logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

	if (selectedMeasurementSet_p)
	{
		// Flush and unlock MS
		selectedMeasurementSet_p->flush();
		selectedMeasurementSet_p->relinquishAutoLocks(True);
		selectedMeasurementSet_p->unlock();

		// Post stats
		if (stats_p)
		{
			*logger_p << LogIO::NORMAL << " Total Flag Cube accesses: " <<  cubeAccessCounter_p << LogIO::POST;
		}
	}

	return true;
}


// -----------------------------------------------------------------------
// Generate selected Measurement Set
// -----------------------------------------------------------------------
bool
FlagMSHandler::selectData()
{
	logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

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
	if (!arraySelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected array ids are " << measurementSetSelection_p->getSubArrayList() << LogIO::POST;
	}

	if (!observationSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected observation ids are " << measurementSetSelection_p->getObservationList() << LogIO::POST;
	}

	if (!fieldSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected field ids are " << measurementSetSelection_p->getFieldList() << LogIO::POST;
	}

	if (!scanSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected scan ids are " << measurementSetSelection_p->getScanList() << LogIO::POST;
	}

	if (!scanIntentSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected scan intent ids are " << measurementSetSelection_p->getStateObsModeList() << LogIO::POST;
	}

	if (!timeSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected time range is " << measurementSetSelection_p->getTimeList() << LogIO::POST;
	}

	if (!spwSelection_p.empty())
	{
		*logger_p << LogIO::NORMAL << " Selected spw-channels ids are " << measurementSetSelection_p->getChanList() << LogIO::POST;
	}

	if (!baselineSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected antenna1 ids are " << measurementSetSelection_p->getAntenna1List() << LogIO::POST;
		*logger_p << LogIO::DEBUG1 << " Selected antenna2 ids are " << measurementSetSelection_p->getAntenna2List() << LogIO::POST;
		*logger_p << LogIO::DEBUG1 << " Selected baselines are " << measurementSetSelection_p->getBaselineList() << LogIO::POST;
	}

	if (!uvwSelection_p.empty())
	{
		*logger_p << LogIO::DEBUG1 << " Selected uv range is " << measurementSetSelection_p->getUVList() << LogIO::POST;
	}

	if (!polarizationSelection_p.empty())
	{
		ostringstream polarizationListToPrint (ios::in | ios::out);
		polarizationListToPrint << measurementSetSelection_p->getPolMap();
		*logger_p << LogIO::DEBUG1 << " Selected correlation ids are " << polarizationListToPrint.str() << LogIO::POST;
	}

	// There is a new selected MS so iterators have to be regenerated
	iteratorGenerated_p = false;
	chunksInitialized_p = false;
	buffersInitialized_p = false;
	stopIteration_p = false;

	return true;
}


// -----------------------------------------------------------------------
// Parse MSSelection expression
// -----------------------------------------------------------------------
void
FlagMSHandler::parseExpression(MSSelection &parser)
{
	parser.toTableExprNode(selectedMeasurementSet_p);
	return;
}


// -----------------------------------------------------------------------
// Swap MS to check what is the maximum RAM memory needed
// -----------------------------------------------------------------------
void
FlagMSHandler::checkMaxMemory()
{
	logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

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
			if (rwVisibilityIterator_p->nRowChunk() > maxChunkRows) maxChunkRows = rwVisibilityIterator_p->nRowChunk();
		}

		// Iterate over vis buffers
		for (rwVisibilityIterator_p->origin(); rwVisibilityIterator_p->more();(*rwVisibilityIterator_p)++)
		{
			// Check total amount of memory needed for visibilities
			memoryNeeded = memoryPerVisFlagCubes*(rwVisibilityIterator_p->visibilityShape().product());

			// Add up memory needed for the rest of the columns
			memoryNeeded += memoryPerRow*(rwVisibilityIterator_p->nRow());

			if (memoryNeeded > maxMemoryNeeded) maxMemoryNeeded = memoryNeeded;

			if (mapScanStartStop_p)
			{
				generateScanStartStopMap();
			}
		}
	}

	Int buffers = 1;
	double memoryFree = HostInfo::memoryFree( )/1024.0;
	double memoryUsed = 100*maxMemoryNeeded/memoryFree;
	if (asyncio_enabled_p)
	{
		AipsrcValue<Int>::find (buffers,"VisibilityIterator.async.nBuffers", 2);
		*logger_p << LogIO::NORMAL << " This process needs " << buffers << " (pre-fetched buffers in async mode) x " << maxMemoryNeeded << " MB for loading visibility buffers ("
				<< memoryUsed << "%) of available free memory (" << memoryFree << " MB)"<< LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << " This process needs " << maxMemoryNeeded << " MB for loading visibility buffers ("
				<< memoryUsed << "%) of available free memory (" << memoryFree << " MB)"<< LogIO::POST;
	}

	if (buffers*maxMemoryNeeded > memoryFree*0.90)
	{
		if (asyncio_enabled_p)
		{
			*logger_p << LogIO::SEVERE << " This process would need to consume more than 90% ("
					<< buffers*maxMemoryNeeded << " MB) of the available memory (" << memoryFree
					<< " MB) for loading vis buffers, aborting. Consider reducing the time interval, or reducing the number of buffers pre-fetched by async I/O (" << buffers
					<< ") or even switch off async I/O." << LogIO::POST;
		}
		else
		{
			*logger_p << LogIO::SEVERE << " This process would need to consume more than 90% ("
					<< buffers*maxMemoryNeeded << " MB) of the available memory (" << memoryFree
					<< " MB) for loading vis buffers, aborting. Consider reducing the time interval."<< LogIO::POST;
		}

		throw(AipsError("FlagMSHandler::checkMaxMemory() Not enough memory to process"));
	}

	if (mapScanStartStop_p)
	{
		*logger_p << LogIO::NORMAL <<  " " << scanStartStopMap_p->size() <<" Scans found in MS" << LogIO::POST;
	}

	return;
}


// -----------------------------------------------------------------------
// Generate Visibility Iterator with a given sort order and time interval
// -----------------------------------------------------------------------
bool
FlagMSHandler::generateIterator()
{
	if (!iteratorGenerated_p)
	{
		// Delete VisBuffer (this implies it self-detaches from VisIter, so we cannot delete VisIter before)
		if (visibilityBuffer_p) delete visibilityBuffer_p;

		// First create and initialize RW iterator
		if (rwVisibilityIterator_p) delete rwVisibilityIterator_p;
		rwVisibilityIterator_p = new VisibilityIterator(*selectedMeasurementSet_p,sortOrder_p,true,timeInterval_p);

		// Set the table data manager (ISM and SSM) cache size to the full column size, for
		// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
		if (slurp_p) rwVisibilityIterator_p->slurp();

		// Apply channel selection (Notice that is not necessary to do this again with the RO iterator in sync mode)
		applyChannelSelection(rwVisibilityIterator_p);

		if ((mapScanStartStop_p) or (groupTimeSteps_p and asyncio_enabled_p))
		{
			checkMaxMemory();
		}

		// If async I/O is enabled we create an async RO iterator for reading and a conventional RW iterator for writing
		// Both iterators share a mutex which is resident in the VLAT data (Visibility Look Ahead thread Data Object)
		// With this configuration the Visibility Buffer is attached to the RO async iterator
		if (asyncio_enabled_p)
		{
			// Set preFetchColumns
			prefetchColumns_p = casa::asyncio::PrefetchColumns::prefetchColumns(VisBufferComponents::FlagCube,
																				VisBufferComponents::FlagRow,
																				VisBufferComponents::NRow,
																				VisBufferComponents::FieldId);
			preFetchColumns();


			// Then create and initialize RO Async iterator
			if (rwVisibilityIterator_p) delete rwVisibilityIterator_p;
			rwVisibilityIterator_p = new VisibilityIterator(&prefetchColumns_p,*selectedMeasurementSet_p,sortOrder_p,true,timeInterval_p);

			// Cast RW conventional iterator into RO conventional iterator
			if (roVisibilityIterator_p) delete roVisibilityIterator_p;
			roVisibilityIterator_p = (ROVisibilityIterator*)rwVisibilityIterator_p;

			// Set the table data manager (ISM and SSM) cache size to the full column size, for
			// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
			if (slurp_p) roVisibilityIterator_p->slurp();

			// Apply channel selection
			applyChannelSelection(roVisibilityIterator_p);

			// Set row blocking to a huge number
			*logger_p << LogIO::NORMAL <<  "Setting row blocking to maximum number of rows in all the chunks swapped: " << maxChunkRows << LogIO::POST;
			if (groupTimeSteps_p) roVisibilityIterator_p->setRowBlocking(maxChunkRows);

			// Attach Visibility Buffer to Visibility Iterator
			visibilityBuffer_p = new VisBufferAutoPtr(roVisibilityIterator_p);
		}
		else
		{
			// Cast RW conventional iterator into RO conventional iterator
			if (roVisibilityIterator_p) delete roVisibilityIterator_p;
			roVisibilityIterator_p = (ROVisibilityIterator*)rwVisibilityIterator_p;

			// Finally attach Visibility Buffer to RO conventional iterator
			visibilityBuffer_p = new VisBufferAutoPtr(roVisibilityIterator_p);
		}

		iteratorGenerated_p = true;
		chunksInitialized_p = false;
		buffersInitialized_p = false;
		stopIteration_p = false;
	}
	else
	{
		chunksInitialized_p = false;
		buffersInitialized_p = false;
		stopIteration_p = false;
	}

	return true;
}


// -----------------------------------------------------------------------
// Apply channel selection for asyn or normal iterator
// NOTE (first implementation): We always have to do this, even if there is no SPW:channel selection
// NOTE (after Dic 2011 testing): As far as I know spw selection does not have to be *, it can be empty,
// therefore this step will in practice do nothing , because the spw and channel lists are empty too.
// -----------------------------------------------------------------------
void
FlagMSHandler::applyChannelSelection(ROVisibilityIterator *roVisIter)
{
	// Apply channel selection (in row selection cannot be done with MSSelection)
	// NOTE: Each row of the Matrix has the following elements: SpwID StartCh StopCh Step
	Matrix<Int> spwchan = measurementSetSelection_p->getChanList();
    IPosition shape = spwchan.shape();
    uInt nSelections = shape[0];
	Int spw,channelStart,channelStop,channelStep,channelWidth;
	for(uInt selection_i=0;selection_i<nSelections;selection_i++)
	{
		// NOTE: selectChannel needs channelStart,channelWidth,channelStep
		spw = spwchan(selection_i,0);
		channelStart = spwchan(selection_i,1);
		channelStop = spwchan(selection_i,2);
		channelStep = spwchan(selection_i,3);
		channelWidth = channelStop-channelStart+1;
		roVisIter->selectChannel(1,channelStart,channelWidth,channelStep,spw);
	}

	return;
}


// -----------------------------------------------------------------------
// Move to next chunk
// -----------------------------------------------------------------------
bool
FlagMSHandler::nextChunk()
{
	logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

	msCounts_p += chunkCounts_p;
	chunkCounts_p = 0;
	bool moreChunks = false;
	if (stopIteration_p)
	{
		moreChunks = false;
	}
	else
	{
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
	}

	if (!moreChunks)
	{
		*logger_p << LogIO::NORMAL << "==================================================================================== " << LogIO::POST;
	}

	return moreChunks;
}


// -----------------------------------------------------------------------
// Move to next buffer
// -----------------------------------------------------------------------
bool
FlagMSHandler::nextBuffer()
{
	bool moreBuffers = false;
	if (stopIteration_p)
	{
		moreBuffers = false;
	}
	else
	{
		if (!buffersInitialized_p)
		{
			// Group all the time stamps in one single buffer
			// NOTE: Otherwise we have to iterate over Visibility Buffers
			// that contain all the rows with the same time step.
			if ((groupTimeSteps_p) and (!asyncio_enabled_p))
			{
				Int nRowChunk = roVisibilityIterator_p->nRowChunk();
				roVisibilityIterator_p->setRowBlocking(nRowChunk);
			}
			roVisibilityIterator_p->origin();
			buffersInitialized_p = true;

			if (!asyncio_enabled_p) preFetchColumns();
			if (mapAntennaPairs_p) generateAntennaPairMap();
			if (mapSubIntegrations_p) generateSubIntegrationMap();
			if (mapPolarizations_p) generatePolarizationsMap();
			if (mapAntennaPointing_p) generateAntennaPointingMap();
			moreBuffers = true;
			flushFlags_p = false;
			flushFlagRow_p = false;
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
				if (!asyncio_enabled_p) preFetchColumns();
				if (mapAntennaPairs_p) generateAntennaPairMap();
				if (mapSubIntegrations_p) generateSubIntegrationMap();
				if (mapPolarizations_p) generatePolarizationsMap();
				if (mapAntennaPointing_p) generateAntennaPointingMap();
				moreBuffers = true;
				flushFlags_p = false;
				flushFlagRow_p = false;
				bufferNo++;
			}
		}
	}

	// Set new common flag cube
	if (moreBuffers)
	{
		logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

		// Get flag  (WARNING: We have to modify the shape of the cube before re-assigning it)
		Cube<Bool> curentFlagCube= visibilityBuffer_p->get()->flagCube();
		modifiedFlagCube_p.resize(curentFlagCube.shape());
		modifiedFlagCube_p = curentFlagCube;
		originalFlagCube_p.resize(curentFlagCube.shape());
		originalFlagCube_p = curentFlagCube;

		// Get flag row (WARNING: We have to modify the shape of the cube before re-assigning it)
		Vector<Bool> curentflagRow= visibilityBuffer_p->get()->flagRow();
		modifiedFlagRow_p.resize(curentflagRow.shape());
		modifiedFlagRow_p = curentflagRow;
		originalFlagRow_p.resize(curentflagRow.shape());
		originalFlagRow_p = curentflagRow;

		// Compute total number of flags per buffer to be used for generating the agents stats
		chunkCounts_p += curentFlagCube.shape().product();

		// Print chunk characteristics
		if (bufferNo == 1)
		{
			String corrs = "[ ";
			for (uInt corr_i=0;corr_i<visibilityBuffer_p->get()->nCorr();corr_i++)
			{
				corrs += (*polarizationIndexMap_p)[corr_i] + " ";
			}
			corrs += "]";

			*logger_p << LogIO::NORMAL << 
			  "------------------------------------------------------------------------------------ " << LogIO::POST;
			*logger_p << "Chunk = " << chunkNo <<
					", Observation = " << visibilityBuffer_p->get()->observationId()[0] <<
					", Array = " << visibilityBuffer_p->get()->arrayId() <<
					", Scan = " << visibilityBuffer_p->get()->scan0() <<
					", Field = " << visibilityBuffer_p->get()->fieldId() << " (" << fieldNames_p->operator()(visibilityBuffer_p->get()->fieldId()) << ")"
					", Spw = " << visibilityBuffer_p->get()->spectralWindow() <<
					", Channels = " << visibilityBuffer_p->get()->nChannel() <<
					", Corrs = " << corrs <<
					", Total Rows = " << visibilityBuffer_p->get()->nRowChunk() << LogIO::POST;
		}
	}

	return moreBuffers;
}

// -----------------------------------------------------------------------
// Generate scan start stop map
// -----------------------------------------------------------------------
void
FlagMSHandler::generateScanStartStopMap()
{

	Int scan;
	Double start,stop;
	Vector<Int> scans;
	Vector<Double> times;

	Cube<Bool> flags;
	uInt scanStartRow;
	uInt scanStopRow;
	uInt ncorrs,nchannels,nrows;
	Bool stopSearch;

	if (scanStartStopMap_p == NULL) scanStartStopMap_p = new scanStartStopMap();

	scans = rwVisibilityIterator_p->scan(scans);
	times = rwVisibilityIterator_p->time(times);

	// Check if anything is flagged in this buffer
	scanStartRow = 0;
	scanStopRow = times.size()-1;
	if (mapScanStartStopFlagged_p)
	{
		flags = rwVisibilityIterator_p->flag(flags);
		IPosition shape = flags.shape();
		ncorrs = shape[0];
		nchannels = shape[1];
		nrows = shape[2];

		// Look for effective scan start
		stopSearch = False;
		for (uInt row_i=0;row_i<nrows;row_i++)
		{
			if (stopSearch) break;

			for (uInt channel_i=0;channel_i<nchannels;channel_i++)
			{
				if (stopSearch) break;

				for (uInt corr_i=0;corr_i<ncorrs;corr_i++)
				{
					if (stopSearch) break;

					if (!flags(corr_i,channel_i,row_i))
					{
						scanStartRow = row_i;
						stopSearch = True;
					}
				}
			}
		}

		// If none of the rows were un-flagged we don't continue checking from the end
		// As a consequence of this some scans may not be present in the map, and have
		// to be skipped in the flagging process because they are already flagged.
		if (!stopSearch) return;

		// Look for effective scan stop
		stopSearch = False;
		for (uInt row_i=0;row_i<nrows;row_i++)
		{
			if (stopSearch) break;

			for (uInt channel_i=0;channel_i<nchannels;channel_i++)
			{
				if (stopSearch) break;

				for (uInt corr_i=0;corr_i<ncorrs;corr_i++)
				{
					if (stopSearch) break;

					if (!flags(corr_i,channel_i,nrows-1-row_i))
					{
						scanStopRow = nrows-1-row_i;
						stopSearch = True;
					}
				}
			}
		}
	}

	// Check scan start/stop times
	scan = scans[0];
	start = times[scanStartRow];
	stop = times[scanStopRow];

	if (scanStartStopMap_p->find(scan) == scanStartStopMap_p->end())
	{
		(*scanStartStopMap_p)[scan].push_back(start);
		(*scanStartStopMap_p)[scan].push_back(stop);
	}
	else
	{
		// Check if we have a better start time
		if ((*scanStartStopMap_p)[scan][0] > start)
		{
			(*scanStartStopMap_p)[scan][0] = start;
		}
		// Check if we have a better stop time
		if ((*scanStartStopMap_p)[scan][1] < stop)
		{
			(*scanStartStopMap_p)[scan][1] = stop;
		}
	}

	return;
}


// -----------------------------------------------------------------------
// Flush flags to MS
// -----------------------------------------------------------------------
bool
FlagMSHandler::flushFlags()
{
	if (flushFlags_p)
	{
		rwVisibilityIterator_p->setFlag(modifiedFlagCube_p);
		flushFlags_p = false;
	}

	if (flushFlagRow_p)
	{
		rwVisibilityIterator_p->setFlagRow(modifiedFlagRow_p);
		flushFlagRow_p = false;
	}

	return true;
}


// -----------------------------------------------------------------------
// Flush flags to MS
// -----------------------------------------------------------------------
String
FlagMSHandler::getTableName()
{
	return originalMeasurementSet_p->tableName();
}

} //# NAMESPACE CASA - END


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
FlagMSHandler::FlagMSHandler(string tablename, uShort iterationApproach, Double timeInterval):
		FlagDataHandler(tablename,iterationApproach,timeInterval)
{
	selectedMeasurementSet_p = NULL;
	originalMeasurementSet_p = NULL;
	visibilityIterator_p = NULL;
	tableTye_p = MEASUREMENT_SET;
	processorTableExist_p = false;
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

	// Delete VisibilityIterator
	if (visibilityIterator_p) delete visibilityIterator_p;

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
	originalMeasurementSet_p = new MeasurementSet(tablename_p,Table::Update);

	// Activate Memory Resident Sub-tables for everything but Pointing, Syscal and History
	originalMeasurementSet_p->setMemoryResidentSubtables (MrsEligibility::defaultEligible());

	// Read antenna names and diameters from Antenna table
	ROMSAntennaColumns *antennaSubTable = new ROMSAntennaColumns(originalMeasurementSet_p->antenna());
	antennaNames_p = new Vector<String>(antennaSubTable->name().getColumn());
	antennaDiameters_p = new Vector<Double>(antennaSubTable->dishDiameter().getColumn());
	antennaPositions_p = new ROScalarMeasColumn<MPosition>(antennaSubTable->positionMeas());

	// File the baseline to Ant1xAnt2 map
	String baseline;
	std::pair<Int,Int> ant1ant2;
	for (Int ant1Idx=0;ant1Idx < static_cast<Int>(antennaNames_p->size());ant1Idx++)
	{
		for (Int ant2Idx=ant1Idx+1;ant2Idx < static_cast<Int>(antennaNames_p->size());ant2Idx++)
		{
			ant1ant2.first = ant1Idx;
			ant1ant2.second = ant2Idx;
			baseline = antennaNames_p->operator()(ant1Idx) + "&&" + antennaNames_p->operator()(ant2Idx);
			baselineToAnt1Ant2_p[baseline] = ant1ant2;
			Ant1Ant2ToBaseline_p[ant1ant2] = baseline;
		}
	}

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
					// jagonzal (CAS-4234): We need this to Sanitize correlation expressions
					corrProducts_p->push_back("I");
					break;
				}
				case Stokes::Q:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product Q found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("Q")) == corrProducts_p->end()) corrProducts_p->push_back("Q");
					break;
				}
				case Stokes::U:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product U found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("U")) == corrProducts_p->end()) corrProducts_p->push_back("U");
					break;
				}
				case Stokes::V:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product V found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("V")) == corrProducts_p->end()) corrProducts_p->push_back("V");
					break;
				}
				case Stokes::XX:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product XX found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("XX")) == corrProducts_p->end()) corrProducts_p->push_back("XX");
					break;
				}
				case Stokes::YY:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product YY found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("YY")) == corrProducts_p->end()) corrProducts_p->push_back("YY");
					break;
				}
				case Stokes::XY:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product XY found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("XY")) == corrProducts_p->end()) corrProducts_p->push_back("XY");
					break;
				}
				case Stokes::YX:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product YX found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("YX")) == corrProducts_p->end()) corrProducts_p->push_back("YX");
					break;
				}
				case Stokes::RR:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product RR found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("RR")) == corrProducts_p->end()) corrProducts_p->push_back("RR");
					break;
				}
				case Stokes::LL:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product LL found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("LL")) == corrProducts_p->end()) corrProducts_p->push_back("LL");
					break;
				}
				case Stokes::RL:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product RL found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("RL")) == corrProducts_p->end()) corrProducts_p->push_back("RL");
					break;
				}
				case Stokes::LR:
				{
					*logger_p << LogIO::DEBUG1 << " Correlation product LR found" << LogIO::POST;
					if (find(corrProducts_p->begin(),corrProducts_p->end(),String("LR")) == corrProducts_p->end()) corrProducts_p->push_back("LR");
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

	// Read reference frequencies per SPW
	ROMSSpWindowColumns *spwSubTable = new ROMSSpWindowColumns(originalMeasurementSet_p->spectralWindow());
	ROScalarColumn<Double> refFrequencies = spwSubTable->refFrequency();
	lambdaMap_p = new lambdaMap();
	for (uInt spwidx=0;spwidx<refFrequencies.nrow();spwidx++)
	{
		(*lambdaMap_p)[spwidx]=C::c/refFrequencies(spwidx);
		*logger_p << LogIO::DEBUG1 << " spwidx: " << spwidx << " lambda: " << (*lambdaMap_p)[spwidx] << LogIO::POST;
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
	if (measurementSetSelection_p) {
		delete measurementSetSelection_p;
		measurementSetSelection_p = NULL;
	}

	// Set the MS Selection error handler to catch antenna names that are
    // not present in the MS in an expression that contains valid antenna names.
	// Catch also invalid intent values in a valid intent expression.
	// This will issue a WARNING and not fail.
    MSSelectionLogError mssLEA,mssLES;
    measurementSetSelection_p->setErrorHandler(MSSelection::ANTENNA_EXPR, &mssLEA);
    measurementSetSelection_p->setErrorHandler(MSSelection::STATE_EXPR, &mssLES);

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
		if (measurementSetSelection_p->getChanList().size())
		{
			inrowSelection_p = true;
		}
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
bool
FlagMSHandler::parseExpression(MSSelection &parser)
{
	parser.toTableExprNode(selectedMeasurementSet_p);
	return true;
}


// -----------------------------------------------------------------------
// Swap MS to check what is the maximum RAM memory needed
// -----------------------------------------------------------------------
void
FlagMSHandler::preSweep()
{
	logger_p->origin(LogOrigin("FlagMSHandler",__FUNCTION__,WHERE));

	for (visibilityIterator_p->originChunks(); visibilityIterator_p->moreChunks();visibilityIterator_p->nextChunk())
	{
		// Iterate over vis buffers
		for (visibilityIterator_p->origin(); visibilityIterator_p->more();visibilityIterator_p->next())
		{

			if (mapScanStartStop_p)
			{
				generateScanStartStopMap();
			}
		}
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
		// Do quack pre-sweep
		if (mapScanStartStop_p)
		{
			if (visibilityIterator_p) delete visibilityIterator_p;
			visibilityIterator_p = new vi::VisibilityIterator2(*selectedMeasurementSet_p,
			                                                   vi::SortColumns (sortOrder_p, true),
			                                                   true,NULL, timeInterval_p);
			preSweep();
		}

		if (asyncio_enabled_p)
		{
			// Set preFetchColumns
			prefetchColumns_p = new VisBufferComponents2();
			prefetchColumns_p->operator +=(vi::FlagCube);
			prefetchColumns_p->operator +=(vi::FlagRow);
			prefetchColumns_p->operator +=(vi::NRows);
			prefetchColumns_p->operator +=(vi::FieldId);

			preFetchColumns();

			// Then create and initialize RO Async iterator
			if (visibilityIterator_p) delete visibilityIterator_p;
			visibilityIterator_p = new vi::VisibilityIterator2(*selectedMeasurementSet_p,
			                                                   vi::SortColumns (sortOrder_p, true),
			                                                   true,prefetchColumns_p,timeInterval_p);
		}
		else if (!mapScanStartStop_p)
		{
			if (visibilityIterator_p) delete visibilityIterator_p;
			visibilityIterator_p = new vi::VisibilityIterator2(*selectedMeasurementSet_p,
			                                                   vi::SortColumns (sortOrder_p, true),
			                                                   true,NULL,timeInterval_p);
		}

		// Set the table data manager (ISM and SSM) cache size to the full column size, for
		// the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW
		if (slurp_p) visibilityIterator_p->slurp();

		// Apply channel selection
		// CAS-3959: Channel selection is now going to be handled at the FlagAgent level
		// applyChannelSelection(visibilityIterator_p);

		// Group all the time stamps in one single buffer
		// NOTE: Otherwise we have to iterate over Visibility Buffers
		// that contain all the rows with the same time step.
		if (groupTimeSteps_p)
		{
			// Set row blocking to a huge number
			uLong maxChunkRows = selectedMeasurementSet_p->nrow();
			visibilityIterator_p->setRowBlocking(maxChunkRows);

			*logger_p << LogIO::NORMAL <<  "Setting row blocking to number of row in selected table: " << maxChunkRows << LogIO::POST;
		}

		// Get Visibility Buffer reference from VisibilityIterator
		visibilityBuffer_p = visibilityIterator_p->getVisBuffer();

		// Get the TYPE column of the PROCESSOR sub-table
		if (loadProcessorTable_p){
			processorTable();
			loadProcessorTable_p = false;
		}

		iteratorGenerated_p = true;
		chunksInitialized_p = false;
		buffersInitialized_p = false;
		stopIteration_p = false;
		processedRows = 0;
	}
	else
	{
		chunksInitialized_p = false;
		buffersInitialized_p = false;
		stopIteration_p = false;
		processedRows = 0;
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
FlagMSHandler::applyChannelSelection(vi::VisibilityIterator2 * /*visIter*/)
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
		// jagonzal (TODO): Use the frequency selection capabilities of the new VI/VB framework
		// visIter->selectChannel(1,channelStart,channelWidth,channelStep,spw);
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
			visibilityIterator_p->originChunks();
			chunksInitialized_p = true;
			buffersInitialized_p = false;
			chunkNo++;
			bufferNo = 0;
			moreChunks = true;
}
		else
		{
			visibilityIterator_p->nextChunk();

			if (visibilityIterator_p->moreChunks())
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
			visibilityIterator_p->origin();
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
			visibilityIterator_p->next();

			// WARNING: We iterate and afterwards check if the iterator is valid
			if (visibilityIterator_p->more())
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
		// Get flag  (WARNING: We have to modify the shape of the cube before re-assigning it)
		Cube<Bool> curentFlagCube= visibilityBuffer_p->flagCube();
		modifiedFlagCube_p.resize(curentFlagCube.shape());
		modifiedFlagCube_p = curentFlagCube;
		originalFlagCube_p.resize(curentFlagCube.shape());
		originalFlagCube_p = curentFlagCube;

		// Get flag row (WARNING: We have to modify the shape of the cube before re-assigning it)
		Vector<Bool> curentflagRow= visibilityBuffer_p->flagRow();
		modifiedFlagRow_p.resize(curentflagRow.shape());
		modifiedFlagRow_p = curentflagRow;
		originalFlagRow_p.resize(curentflagRow.shape());
		originalFlagRow_p = curentflagRow;

		// Compute total number of flags per buffer to be used for generating the agents stats
		Int64 currentBufferCounts = curentFlagCube.shape().product();
		chunkCounts_p += currentBufferCounts;
		progressCounts_p += currentBufferCounts;
		msCounts_p += currentBufferCounts;

		// Print chunk characteristics
		if (bufferNo == 1)
		{
			processedRows += visibilityIterator_p->nRowsInChunk();
			if (printChunkSummary_p)
			{
				logger_p->origin(LogOrigin("FlagMSHandler",""));
				String corrs = "[ ";
				for (uInt corr_i=0;corr_i<(uInt) visibilityBuffer_p->nCorrelations();corr_i++)
				{
					corrs += (*polarizationIndexMap_p)[corr_i] + " ";
				}
				corrs += "]";

				Double progress  = 100.0* ((Double) processedRows / (Double) selectedMeasurementSet_p->nrow());

				*logger_p << LogIO::NORMAL <<
				  "------------------------------------------------------------------------------------ " << LogIO::POST;
				*logger_p << LogIO::NORMAL <<
						"Chunk = " << chunkNo << " [progress: " << (Int)progress << "%]"
						", Observation = " << visibilityBuffer_p->observationId()[0] <<
						", Array = " << visibilityBuffer_p->arrayId()[0] <<
						", Scan = " << visibilityBuffer_p->scan()[0] <<
						", Field = " << visibilityBuffer_p->fieldId()(0) << " (" << fieldNames_p->operator()(visibilityBuffer_p->fieldId()) << ")"
						", Spw = " << visibilityBuffer_p->spectralWindows()(0) <<
						", Channels = " << visibilityBuffer_p->nChannels() <<
						", Corrs = " << corrs <<
						", Total Rows = " << visibilityIterator_p->nRowsInChunk() << LogIO::POST;
			}
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

	scans = visibilityIterator_p->getVisBuffer()->scan();
	times = visibilityIterator_p->getVisBuffer()->time();

	// Check if anything is flagged in this buffer
	scanStartRow = 0;
	scanStopRow = times.size()-1;
	if (mapScanStartStopFlagged_p)
	{
		flags = visibilityIterator_p->getVisBuffer()->flagCube();
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
		visibilityIterator_p->writeFlag(modifiedFlagCube_p);
		flushFlags_p = false;
	}

	if ((flushFlagRow_p) and (!inrowSelection_p))
	{
		visibilityIterator_p->writeFlagRow(modifiedFlagRow_p);
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

// -----------------------------------------------------------------------
// Check what data columns exist
// -----------------------------------------------------------------------
bool
FlagMSHandler::checkIfColumnExists(String column)
{
	return originalMeasurementSet_p->tableDesc().isColumn(column);
}

// -----------------------------------------------------------------------
// Signal true when a progress summary has to be printed
// -----------------------------------------------------------------------
bool
FlagMSHandler::summarySignal()
{
	Double progress = 100.0* ((Double) processedRows / (Double) selectedMeasurementSet_p->nrow());
	if ((progress >= summaryThreshold_p) || (logger_p->priority() >= LogMessage::DEBUG1))
	{
		summaryThreshold_p += 10;
		printChunkSummary_p = true;
		return true;
	}
	else
	{
		printChunkSummary_p = false;
		return false;
	}
}

// -----------------------------------------------------------------------
// Get the PROCESSOR sub-table
// -----------------------------------------------------------------------
bool
FlagMSHandler::processorTable()
{
	MSProcessor msSubtable = selectedMeasurementSet_p->processor();

	// The rows in the PROCESSOR table correspond to the PROCESSOR_ID column
	// in main table
	if (msSubtable.nrow() == 0){
		*logger_p << LogIO::WARN << "PROCESSOR sub-table is empty. Assuming CORRELATOR type."
				<< LogIO::POST;
		processorTableExist_p = false;
	}
	else {
		MSProcessorColumns tableCols(msSubtable);
		ScalarColumn<String> typeCol = tableCols.type();
		processorTableExist_p = true;

		/* Create a look-up boolean column to tell if a row is of type CORRELATOR.
		   isCorrelatorType_p = True when PROCESSOR_ID is of TYPE CORRELATOR
		 */
		isCorrelatorType_p = tableCols.flagRow();

		// Assign True to row in look-up table that have TYPE==CORRELATOR
		for (uInt pid=0; pid<msSubtable.nrow(); pid++){

			String proc_type = typeCol.asString(pid);
			if (proc_type.compare("CORRELATOR") == 0){
				isCorrelatorType_p.put(pid, 1);
			}
			else
				isCorrelatorType_p.put(pid, 0);

		}
	}

	return true;

}

} //# NAMESPACE CASA - END


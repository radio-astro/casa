//# FlagAgentBase.h: This file contains the implementation of the FlagAgentBase class.
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

#include <flagging/Flagging/FlagAgentBase.h>

namespace casa { //# NAMESPACE CASA - BEGIN

////////////////////////////////////
/// FlagAgentBase implementation ///
////////////////////////////////////

// NOTE: We have to initialize the polarizationList_p here, which is a OrderedMap<Int, Vector<Int> >
// because otherwise the compiler complains because we are calling a theoretical default constructor
// OrderedMap() that does not exist.
FlagAgentBase::FlagAgentBase(FlagDataHandler *dh,Record config, Bool writePrivateFlagCube, Bool antennaMap, Bool flag): polarizationList_p(Vector<Int>(0))
{
	// Initialize logger
	logger_p = new LogIO();

	// Initialize members
	initialize();

	// Set flag/un-flag mode
	flag_p = flag;

	// Check if async processing is enabled
	parallel_processing_p = false;
	AipsrcValue<Bool>::find (parallel_processing_p,"FlagAgent.parallel", false);
	if (parallel_processing_p)
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " Parallel mode enabled" << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " Parallel mode enabled" << LogIO::POST;
	}

	// Set antenna map flag (for RFI mode)
	antennaMap_p = antennaMap;

	// Set pointer to apply flag function
	writePrivateFlagCube_p = writePrivateFlagCube;
	if (writePrivateFlagCube_p)
	{
		if (antennaMap_p)
		{
			applyFlag_p = &FlagAgentBase::applyPrivateFlagsView;
		}
		else
		{
			applyFlag_p = &FlagAgentBase::applyPrivateFlags;
		}
	}
	else
	{
		if (antennaMap_p)
		{
			applyFlag_p = &FlagAgentBase::applyCommonFlagsView;
		}
		else
		{
			applyFlag_p = &FlagAgentBase::applyCommonFlags;
		}
	}

	// Set flag data handler
	flagDataHandler_p = dh;

	// Set (pre-selected) MS
	selectedMeasurementSet_p = flagDataHandler_p->selectedMeasurementSet_p;

	// Set vis buffer
	visibilityBuffer_p = flagDataHandler_p->visibilityBuffer_p;

	// Set data selection
	setDataSelection(config);

	// Set agent parameters
	setAgentParameters(config);
}

FlagAgentBase::~FlagAgentBase()
{
	if (privateFlagCube_p) delete privateFlagCube_p;
}

void
FlagAgentBase::initialize()
{
   // Initialize members
   flagDataHandler_p = NULL;
   selectedMeasurementSet_p = NULL;
   visibilityBuffer_p = NULL;
   privateFlagCube_p = NULL;
   commonFlagCube_p = NULL;

   // Initialize selection ranges
   timeSelection_p = String("");
   baselineSelection_p = String("");
   fieldSelection_p = String("");
   // NOTE: According to MS selection syntax, spw must be at least * but since
   // we are parsing it only if it was provided it should not be a problem
   spwSelection_p = String("");
   uvwSelection_p = String("");
   scanSelection_p = String("");
   arraySelection_p = String("");
   observationSelection_p = String("");
   scanIntentSelection_p = String("");

   // Initialize filters
   filterChannels_p = false;
   filterRows_p = false;
   filterPols_p = false;

	// Initialize state
   terminationRequested_p = false;
   threadTerminated_p = false;
   processing_p = false;

   //// Initialize configuration ////

   /// Running config
   parallel_processing_p = true;
   /// Antenna maps config
   antennaPairMap_p = NULL;
   privateFlagsView_p = NULL;
   commonFlagsView_p = NULL;
   antennaMap_p = false;
   antennaNegation_p = false;
   /// Flag/Unflag config
   writePrivateFlagCube_p = false;
   applyFlag_p = NULL;
   flag_p = true;
   /// Profiling and testing config
   profiling_p = false;

   /////////////////////////////////

   return;
}

void *
FlagAgentBase::run ()
{
	while (!terminationRequested_p)
	{
		if (processing_p)
		{
			runCore();

			// Disable processing to enter in idle mode
			processing_p = false;
		}
		else
		{
			sched_yield();
		}
	}

	threadTerminated_p = true;

	return NULL;
}

void
FlagAgentBase::runCore()
{
	// Set pointer to common flag cube
	commonFlagCube_p = flagDataHandler_p->getModifiedFlagCube();

	// Generate indexes applying data selection filters
	generateAllIndex();

	if (!checkIfProcessBuffer())
	{
		// Disable processing to enter in idle mode
		processing_p = false;
	}
	else
	{
		// Set pointer to private flag cube
		if (writePrivateFlagCube_p)
		{
			if (privateFlagCube_p) delete privateFlagCube_p;
			privateFlagCube_p = new Cube<Bool>(commonFlagCube_p->shape(),!flag_p);
		}

		// Iterate trough (time,freq) maps per antenna pair
		if (antennaMap_p)
		{
			generateAntennaPairMap();
			iterateMaps();
		}
		// Iterate trough rows (i.e. timesteps)
		else
		{
			iterateRows();
		}
	}

	return;
}

void
FlagAgentBase::start()
{
	if (parallel_processing_p)
	{
		casa::async::Thread::startThread();
	}

	return;
}

void
FlagAgentBase::terminate ()
{
	if (parallel_processing_p)
	{
		terminationRequested_p = true;
		while (!threadTerminated_p)
		{
			sched_yield();
		}
		casa::async::Thread::terminate();
	}

	return;
}

void
FlagAgentBase::queueProcess()
{
	if (parallel_processing_p)
	{
		// Wait until we are done with previous buffer
		while (processing_p)
		{
			sched_yield();
		}

		// Enable processing to trigger flagging
		processing_p = true;
	}
	else
	{
		runCore();
	}

	return;
}

void
FlagAgentBase::completeProcess()
{
	if (parallel_processing_p)
	{
		// Wait until we are done with previous buffer
		while (processing_p)
		{
			sched_yield();
		}
	}

	return;
}

// -----------------------------------------------------------------------
// Set Data Selection parameters
// -----------------------------------------------------------------------
void
FlagAgentBase::setDataSelection(Record config)
{
	int exists;
	MSSelection parser;

	exists = config.fieldNumber ("array");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("array"), arraySelection_p);

		parser.setArrayExpr(arraySelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		arrayList_p=parser.getSubArrayList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " array selection is " << arraySelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " array ids are " << arrayList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no array selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("field");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("field"), fieldSelection_p);

		parser.setFieldExpr(fieldSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		fieldList_p=parser.getFieldList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " field selection is " << fieldSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " field ids are " << fieldList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no field selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("scan");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("scan"), scanSelection_p);

		parser.setScanExpr(scanSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		scanList_p=parser.getScanList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " scan selection is " << scanSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " scan ids are " << scanList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no scan selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("timerange");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("timerange"), timeSelection_p);

		parser.setTimeExpr(timeSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		timeList_p=parser.getTimeList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " timerange selection is " << timeSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " time ranges in MJD are " << timeList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no time selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("spw");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("spw"), spwSelection_p);

		parser.setSpwExpr(spwSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		spwList_p=parser.getSpwList();
		filterRows_p=true;

		channelList_p=parser.getChanList();
		filterChannels_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " Frequency selection is " << spwSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " Frequency ids are " << channelList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no spw selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("antenna");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("antenna"), baselineSelection_p);
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " antenna selection is " << baselineSelection_p << LogIO::POST;

		// Remove antenna negation operator (!) and set antenna negation flag
		size_t pos = baselineSelection_p.find(String("!"));
		while (pos != String::npos)
		{
			antennaNegation_p = true;
			baselineSelection_p.replace(pos,1,String(""));
			*logger_p << LogIO::DEBUG1 << "FlagAgentBase::" << __FUNCTION__ << " antenna selection is " << baselineSelection_p << LogIO::POST;
			pos = baselineSelection_p.find(String("!"));
		}

		parser.setAntennaExpr(baselineSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		antenna1List_p=parser.getAntenna1List();
		antenna2List_p=parser.getAntenna2List();
		baselineList_p=parser.getBaselineList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " selected baselines are " << baselineList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no baseline selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("uvrange");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("uvrange"), uvwSelection_p);

		parser.setUvDistExpr(uvwSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		uvwList_p=parser.getUVList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " uvrange selection is " << uvwSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " uvrange ids are " << uvwList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no uvw selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("correlation");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("correlation"), polarizationSelection_p);

		parser.setPolnExpr(polarizationSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		polarizationList_p=parser.getPolMap();
		filterPols_p=true;

		// NOTE: casa::LogIO does not support outstream from OrderedMap<Int, Vector<Int> > objects yet
		ostringstream polarizationListToPrint (ios::in | ios::out);
		polarizationListToPrint << polarizationList_p;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " correlation selection is " << polarizationSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " correlation ids are " << polarizationListToPrint.str() << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no polarization selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("observation");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("observation"), observationSelection_p);

		parser.setPolnExpr(observationSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		observationList_p=parser.getObservationList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " observation selection is " << observationList_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " observation ids are " << observationList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no observation selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("intent");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("intent"), observationSelection_p);

		parser.setPolnExpr(observationSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		scanIntentList_p=parser.getStateObsModeList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " scan intent selection is " << scanIntentList_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " scan intent ids are " << scanIntentList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no scan intent selection" << LogIO::POST;
	}

	return;
}

void
FlagAgentBase::setAgentParameters(Record config)
{
	// TODO: This class must be re-implemented in the derived classes
	return;
}

void
FlagAgentBase::generateAllIndex()
{
	STARTCLOCK

	Int nPolarizations,nChannels,nRows;
	commonFlagCube_p->shape(nPolarizations,nChannels,nRows);
	generateRowsIndex(nRows);
	generateChannelIndex(nChannels);
	generatePolarizationIndex(nPolarizations);

	STOPCLOCK
	return;
}

void
FlagAgentBase::generateRowsIndex(uInt nRows)
{
	STARTCLOCK

	// For uvw range filter
	Vector<casa::RigidVector<casa::Double, 3> > uvw;

	rowsIndex_p.clear();
	if (filterRows_p)
	{
		Double u,v,uvDistance;
		for (uInt row_i=0;row_i<nRows;row_i++)
		{
			// Check observation id
			if (observationList_p.size())
			{
				if (!find(observationList_p,visibilityBuffer_p->get()->observationId()[row_i])) continue;
			}

			// Check scan intent
			if (scanIntentList_p.size())
			{
				if (!find(scanIntentList_p,visibilityBuffer_p->get()->stateId()[row_i])) continue;
			}

			// Check scan id
			if (scanList_p.size())
			{
				if (!find(scanList_p,visibilityBuffer_p->get()->scan()[row_i])) continue;
			}

			// Check baseline
			if (baselineList_p.size())
			{
				if (!find(baselineList_p,visibilityBuffer_p->get()->antenna1()[row_i],visibilityBuffer_p->get()->antenna2()[row_i])) continue;
			}

			// Check time range
			if (timeList_p.size())
			{
				if (!find(timeList_p,visibilityBuffer_p->get()->time()[row_i])) continue;
			}

			// Check uvw range
			if (uvwList_p.size())
			{
				// NOTE: uvw from vis buffer is in meters, so we only support uv distance
				// (MS Selection does not return normalized ranges)
				uvw = visibilityBuffer_p->get()->uvw();
				u = uvw(row_i)(0);
				v = uvw(row_i)(1);
				uvDistance = sqrt(u*u + v*v);
				if (!find(uvwList_p,uvDistance)) continue;
			}

			// If all the filters passed, add the row to the list
			rowsIndex_p.push_back(row_i);
		}
	}
	else
	{
		indigen(rowsIndex_p,nRows);
	}

	STOPCLOCK
	return;
}

void
FlagAgentBase::generateChannelIndex(uInt nChannels)
{
	STARTCLOCK

	channelIndex_p.clear();
	if (filterChannels_p)
	{
		// First find channel start and stop for this spw
		Int currentSpw = visibilityBuffer_p->get()->spectralWindow();
		Int nSpw,width;
		bool spwFound = false;
		uInt channelStart = 0,channelStop = UINT_MAX;
		channelList_p.shape(nSpw,width);
		for (uShort spw_i=0;spw_i<nSpw;spw_i++)
		{
			if (channelList_p(spw_i,0) == currentSpw)
			{
				channelStart = channelList_p(spw_i,1);
				channelStop = channelList_p(spw_i,2);
				spwFound = true;
			}
		}

		// If the spw is not found we return w/o adding any channels
		if (!spwFound) return;

		// Finally check what channels are within the range
		for (uInt channel_i=0;channel_i<nChannels;channel_i++)
		{
			if ((channel_i>=channelStart) and (channel_i<=channelStop)) channelIndex_p.push_back(channel_i);
		}
	}
	else
	{
		indigen(channelIndex_p,nChannels);
	}

	STOPCLOCK
	return;
}

void
FlagAgentBase::generatePolarizationIndex(uInt nPolarizations)
{
	STARTCLOCK

	polarizationIndex_p.clear();
	if (filterPols_p)
	{
		// NOTE: Polarization ID should be accessible from the visibility buffer
		// but this functionality is not implemented yet, therefore we are getting
		// it from the RW Visibility Iterator which is always a conventional one
		// (not asyn I/O which does not implement it)
		Int polId = flagDataHandler_p->rwVisibilityIterator_p->polarizationId();
		Vector<Int> polarizations = polarizationList_p(polId);

		// Get accepted polarizations
		for (uInt polarization_i=0;polarization_i<nPolarizations;polarization_i++)
		{
			if (!find(polarizations,polarization_i)) continue;
			polarizationIndex_p.push_back(polarization_i);
		}
	}
	else
	{
		indigen(polarizationIndex_p,nPolarizations);
	}

	STOPCLOCK
	return;
}

void
FlagAgentBase::indigen(vector<uInt> &index, uInt size)
{
	STARTCLOCK

	for (uInt i=0; i<size; i++ )
	{
		index.push_back(i);
	}

	STOPCLOCK
	return;
}

bool
FlagAgentBase::find(Matrix<Int> validPairs, Int element1, Int element2)
{
	Int x,y;
	validPairs.shape(x,y);

	for (Int i=0;i<x;i++)
	{
		if ((validPairs(i,0) == element1) and (validPairs(i,1) == element2)) return !antennaNegation_p;
		if ((validPairs(i,0) == element2) and (validPairs(i,1) == element1)) return !antennaNegation_p;
	}

	return antennaNegation_p;
}

bool
FlagAgentBase::find(Vector<Int> validRange, Int element)
{
	for (uShort idx=0;idx<validRange.size(); idx++)
	{
		if (element == validRange[idx]) return true;
	}
	return false;
}

bool
FlagAgentBase::find(Matrix<Double> validRange, Double element)
{
	if (element>=validRange(0,0) and element<=validRange(1,0)) return true;
	return false;
}

bool
FlagAgentBase::checkIfProcessBuffer()
{
	STARTCLOCK

	// array,filed and spw are common and unique in a given vis buffer,
	// so we can use them to discard all the rows in a vis buffer.
	if (arrayList_p.size())
	{
		if (!find(arrayList_p,visibilityBuffer_p->get()->arrayId())) return false;
	}
	if (fieldList_p.size())
	{
		if (!find(fieldList_p,visibilityBuffer_p->get()->fieldId())) return false;
	}
	if (spwList_p.size())
	{
		if (!find(spwList_p,visibilityBuffer_p->get()->spectralWindow())) return false;
	}

	if ((!rowsIndex_p.size()) || (!channelIndex_p.size()) || (!polarizationIndex_p.size())) return false;

	STOPCLOCK
	return true;
}

void
FlagAgentBase::iterateRows()
{
	STARTCLOCK

	if (!checkIfProcessBuffer()) return;

	*logger_p 	<< LogIO::NORMAL << "Going to process a buffer with: " <<
			rowsIndex_p.size() << " rows (" << rowsIndex_p[0] << "-" << rowsIndex_p[rowsIndex_p.size()-1] << ") " <<
			channelIndex_p.size() << " channels (" << channelIndex_p[0] << "-" << channelIndex_p[channelIndex_p.size()-1] << ") " <<
			polarizationIndex_p.size() << " polarizations (" << polarizationIndex_p[0] << "-" << polarizationIndex_p[polarizationIndex_p.size()-1] << ")" << LogIO::POST;

	Bool computedFlag;
	vector<uInt>::iterator rowIter;
	vector<uInt>::iterator channellIter;
	vector<uInt>::iterator polarizationIter;

	for (rowIter = rowsIndex_p.begin();rowIter != rowsIndex_p.end();rowIter++)
	{
		for (channellIter = channelIndex_p.begin();channellIter != channelIndex_p.end();channellIter++)
		{
			for (polarizationIter = polarizationIndex_p.begin();polarizationIter != polarizationIndex_p.end();polarizationIter++)
			{
				computedFlag = computeFlag(*rowIter,*channellIter,*polarizationIter);

				if (computedFlag == flag_p)
				{
					applyFlag(*rowIter,*channellIter,*polarizationIter);
				}
			}
		}
	}

	STOPCLOCK
	return;
}

Bool
FlagAgentBase::computeFlag(uInt row, uInt channel, uInt pol)
{
	// TODO: This class must be re-implemented in the derived classes
	return flag_p;
}

void
FlagAgentBase::generateAntennaPairMap()
{
	STARTCLOCK

	if (!checkIfProcessBuffer()) return;

	// Free previous map and create a new one
	if (antennaPairMap_p) delete antennaPairMap_p;
	antennaPairMap_p = new antennaPairMap();

	// Retrieve antenna vectors
	Vector<Int> antenna1Vector = visibilityBuffer_p->get()->antenna1();
	Vector<Int> antenna2Vector = visibilityBuffer_p->get()->antenna2();

	// Fill map
	Int ant1_i,ant2_i;
	vector<uInt>::iterator rowIter;
	for (rowIter = rowsIndex_p.begin();rowIter != rowsIndex_p.end();rowIter++)
	{
		ant1_i = antenna1Vector[*rowIter];
		ant2_i = antenna2Vector[*rowIter];
		if (antennaPairMap_p->find(std::make_pair(ant1_i,ant2_i)) == antennaPairMap_p->end())
		{
			std::vector<uInt> newPair;
			newPair.push_back(*rowIter);
			(*antennaPairMap_p)[std::make_pair(ant1_i,ant2_i)] = newPair;
		}
		else
		{
			(*antennaPairMap_p)[std::make_pair(ant1_i,ant2_i)].push_back(*rowIter);
		}
	}
	*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ <<  " " << antennaPairMap_p->size() <<" Antenna pairs found in current buffer" << LogIO::POST;

	STOPCLOCK
	return;
}

void
FlagAgentBase::iterateMaps()
{
	STARTCLOCK

	antennaPairMapIterator myAntennaPairMapIterator;
	std::pair<Int,Int> antennaPair;
	CubeView<Complex> *visCubeView;
	IPosition flagCubeShape;

	for (myAntennaPairMapIterator=antennaPairMap_p->begin(); myAntennaPairMapIterator != antennaPairMap_p->end(); ++myAntennaPairMapIterator)
	{
		// Get antenna pair from map
		antennaPair = myAntennaPairMapIterator->first;

		// Create CubeView for common flags
		commonFlagsView_p = getCommonFlagsView(antennaPair.first,antennaPair.second);

		// Some logging info
		*logger_p << LogIO::NORMAL << "FlagDataHandler::" << __FUNCTION__ <<  " Processing [pol,freq,time] data cube for baseline ("
				<< antennaPair.first << "," << antennaPair.second << ") with shape " << commonFlagsView_p->shape() << LogIO::POST;

		// Create CubeView for private flags
		if (writePrivateFlagCube_p) privateFlagsView_p = getPrivateFlagsView(antennaPair.first,antennaPair.second);

		// Create CubeView for Visibilities
		visCubeView = getVisibilitiesView(antennaPair.first,antennaPair.second);

		// Flag map
		flagMap(antennaPair.first,antennaPair.second,visCubeView);

		// Clean up Cube Views
		delete visCubeView;
		delete commonFlagsView_p;
		if (writePrivateFlagCube_p) delete privateFlagsView_p;
	}

	STOPCLOCK
	return;
}

CubeView<Bool> *
FlagAgentBase::getCommonFlagsView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Bool> *cube= new CubeView<Bool>(commonFlagCube_p,rows);
	return cube;
}

CubeView<Bool> *
FlagAgentBase::getPrivateFlagsView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Bool> *cube= new CubeView<Bool>(privateFlagCube_p,rows);
	return cube;
}

CubeView<Complex> *
FlagAgentBase::getVisibilitiesView(Int antenna1, Int antenna2)
{
	std::vector<uInt> *rows = &((*antennaPairMap_p)[std::make_pair(antenna1,antenna2)]);
	CubeView<Complex> *cube= new CubeView<Complex>(&(visibilityBuffer_p->get()->visCube()),rows);
	return cube;
}

void
FlagAgentBase::flagMap(Int antenna1,Int antenna2,CubeView<Complex> *visibilities)
{
	IPosition flagCubeShape = visibilities->shape();
	uInt nPolarizations,nChannels,nRows;
	nPolarizations = flagCubeShape(0);
	nChannels = flagCubeShape(1);
	nRows = flagCubeShape(2);
	for (uInt row_i=0;row_i<nRows;row_i++)
	{
		for (uInt chan_i=0;chan_i<nChannels;chan_i++)
		{
			for (uInt pol_i=0;pol_i<nPolarizations;pol_i++)
			{
				applyFlag(row_i,chan_i,pol_i);
			}
		}
	}

	return;
}

void
FlagAgentBase::applyCommonFlags(uInt row, uInt channel, uInt pol)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagCube_p->operator()(pol,channel,row) = flag_p;
	return;
}

void
FlagAgentBase::applyPrivateFlags(uInt row, uInt channel, uInt pol)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagCube_p->operator()(pol,channel,row) = flag_p;
	privateFlagCube_p->operator()(pol,channel,row) = flag_p;
	return;
}

void
FlagAgentBase::applyCommonFlagsView(uInt row, uInt channel, uInt pol)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagsView_p->operator()(pol,channel,row) = flag_p;
	return;
}

void
FlagAgentBase::applyPrivateFlagsView(uInt row, uInt channel, uInt pol)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagsView_p->operator()(pol,channel,row) = flag_p;
	privateFlagsView_p->operator()(pol,channel,row) = flag_p;
	return;
}

void FlagAgentBase::applyFlag(uInt row, uInt channel, uInt pol)
{
	(*this.*applyFlag_p)(row,channel,pol);
}

void
FlagAgentBase::checkFlags(uInt row, uInt channel, uInt pol)
{
	if (commonFlagCube_p->at(pol,channel,row) != flag_p)
	{
		*logger_p 	<< LogIO::SEVERE << "FlagAgentBase::" << __FUNCTION__
					<< " Wrong flag in chunk " << flagDataHandler_p->chunkNo
					<< " buffer " << flagDataHandler_p->bufferNo
					<< " polarization " << pol
					<< " channel " << channel
					<< " row " << row << LogIO::POST;
	}
	return;
}

FlagAgentBase *
FlagAgentBase::create (Record config)
{
	FlagAgentBase *ret = NULL;
	return ret;
}

////////////////////////////////////
/// FlagAgentlist implementation ///
////////////////////////////////////

FlagAgentList::FlagAgentList()
{
	container_p.clear();
}

FlagAgentList::~FlagAgentList()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		delete (*iterator_p);
	}
	container_p.clear();
}

void FlagAgentList::push_back(FlagAgentBase *agent_i)
{
	container_p.push_back(agent_i);
	return;
}

void FlagAgentList::pop_back()
{
	container_p.pop_back();
	return;
}

void FlagAgentList::start()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->start();
	}

	return;
}

void FlagAgentList::terminate()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->terminate();
	}

	return;
}

void FlagAgentList::join()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->join();
	}

	return;
}

void FlagAgentList::queueProcess()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->queueProcess();
	}

	return;
}

void FlagAgentList::completeProcess()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->completeProcess();
	}

	return;
}

void FlagAgentList::setProfiling(bool enable)
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->setProfiling(enable);
	}

	return;
}

void FlagAgentList::setCheckMode()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->setCheckMode();
	}

	return;
}


} //# NAMESPACE CASA - END



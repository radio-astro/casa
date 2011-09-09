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

// NOTE: We have to initialize the polarizationList_p here, which is a OrderedMap<Int, Vector<Int> >
// because otherwise the compiler complains because we are calling a theoretical default constructor
// OrderedMap() that does not exist.
FlagAgentBase::FlagAgentBase(FlagDataHandler *dh,Record config, Bool writePrivateFlagCube, Bool flag): polarizationList_p(Vector<Int>(0))
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

	// Set pointer to apply flag function
	writePrivateFlagCube_p = writePrivateFlagCube;
	if (writePrivateFlagCube_p)
	{
		applyFlag_p = &FlagAgentBase::applyPrivateFlags;
	}
	else
	{
		applyFlag_p = &FlagAgentBase::applyCommonFlags;
	}

	// Set flag data handler
	flagDataHandler_p = dh;

	// Set (pre-selected) MS
	selectedMeasurementSet_p = flagDataHandler_p->selectedMeasurementSet_p;

	// Set vis buffer
	visibilityBuffer_p = dh->visibilityBuffer_p;

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

   // Initialize filters
   filterChannels_p = false;
   filterRows_p = false;
   filterPols_p = false;

	// Initialize state
   terminationRequested_p = false;
   threadTerminated_p = false;
   processing_p = false;

   // Initialize config
   writePrivateFlagCube_p = false;
   applyFlag_p = NULL;
   profiling_p = false;
   flag_p = true;

   return;
}

void *
FlagAgentBase::run ()
{
	while (!terminationRequested_p)
	{
		if (processing_p)
		{
			// Set pointers to common and private flag cube
			commonFlagCube_p = flagDataHandler_p->getModifiedFlagCube();
			if (writePrivateFlagCube_p)
			{
				if (privateFlagCube_p) delete privateFlagCube_p;
				privateFlagCube_p = new Cube<Bool>(commonFlagCube_p->shape(),!flag_p);
			}

			// Generate indexes applying data selection filters
			generateAllIndex();

			// Compute flags
			iterateRows();

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
FlagAgentBase::start()
{
	casa::async::Thread::startThread();

	return;
}

void
FlagAgentBase::terminate ()
{
	terminationRequested_p = true;
	while (!threadTerminated_p)
	{
		sched_yield();
	}
	casa::async::Thread::terminate();

	return;
}

void
FlagAgentBase::queueProcess()
{
	// Wait until we are done with previous buffer
	while (processing_p)
	{
		sched_yield();
	}

	// Enable processing to trigger flagging
	processing_p = true;

	// If parallel processing mode is not activated we
	// wait until the current buffer has been processed
	if (!parallel_processing_p)
	{
		while (processing_p)
		{
			sched_yield();
		}
	}

	return;
}

void
FlagAgentBase::completeProcess()
{
	// Wait until we are done with previous buffer
	while (processing_p)
	{
		sched_yield();
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

	exists = config.fieldNumber ("time");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("time"), timeSelection_p);

		parser.setTimeExpr(timeSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		timeList_p=parser.getTimeList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " time selection is " << timeSelection_p << LogIO::POST;
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

	exists = config.fieldNumber ("baseline");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("baseline"), baselineSelection_p);

		parser.setAntennaExpr(baselineSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		antenna1List_p=parser.getAntenna1List();
		antenna2List_p=parser.getAntenna2List();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " baseline selection is " << baselineSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " antenna1 ids are " << antenna1List_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " antenna2 ids are " << antenna2List_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no baseline selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("uvw");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("uvw"), uvwSelection_p);

		parser.setUvDistExpr(uvwSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		uvwList_p=parser.getUVList();
		filterRows_p=true;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " uvw selection is " << uvwSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " uvw ids are " << uvwList_p << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no uvw selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("polarization");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("polarization"), polarizationSelection_p);

		parser.setPolnExpr(polarizationSelection_p);
		parser.toTableExprNode(selectedMeasurementSet_p);
		polarizationList_p=parser.getPolMap();
		filterPols_p=true;

		// NOTE: casa::LogIO does not support outstream from OrderedMap<Int, Vector<Int> > objects yet
		ostringstream polarizationListToPrint (ios::in | ios::out);
		polarizationListToPrint << polarizationList_p;

		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " polarization selection is " << polarizationSelection_p << LogIO::POST;
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " polarization ids are " << polarizationListToPrint.str() << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << "FlagAgentBase::" << __FUNCTION__ << " no polarization selection" << LogIO::POST;
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
			// Check scan id
			if (scanList_p.size())
			{
				if (!find(scanList_p,visibilityBuffer_p->get()->scan()[row_i])) continue;
			}

			// Check baseline (NOTE: The antenna expressions are actually saved in antenna1)
			if (antenna1List_p.size())
			{
				if (!find(antenna1List_p,visibilityBuffer_p->get()->antenna1()[row_i])) continue;
			}

			if (antenna2List_p.size())
			{
				if (!find(antenna2List_p,visibilityBuffer_p->get()->antenna2()[row_i])) continue;
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
		Int channelStart = -1,channelStop = -1;
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
			if ((channel_i>=channelStart) and (channelStart<=channelStop)) channelIndex_p.push_back(channel_i);
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
FlagAgentBase::find(Vector<Int> validRange, Int element)
{
	for (uShort idx=0;idx<validRange.size(); idx++)
	{
		if (element == validRange[idx]) return true;
		// NOTE: Antenna negation expressions return the antenna id with - sign
		if (element == -validRange[idx]) return false;
		if (validRange[idx]<0) return true;
	}
	return false;
}

bool
FlagAgentBase::find(Matrix<Double> validRange, Double element)
{
	if (element>=validRange(0,0) and element<=validRange(1,0))
	{
		return true;
	}

	return false;
}

void
FlagAgentBase::iterateRows()
{
	STARTCLOCK

	Bool computedFlag;
	vector<uInt>::iterator rowIter;
	vector<uInt>::iterator channellIter;
	vector<uInt>::iterator polarizationIter;

	// array,filed and spw are common and unique in a given vis buffer,
	// so we can use them to discard all the rows in a vis buffer.
	if (arrayList_p.size())
	{
		if (!find(arrayList_p,visibilityBuffer_p->get()->arrayId())) return;
	}
	if (fieldList_p.size())
	{
		if (!find(fieldList_p,visibilityBuffer_p->get()->fieldId())) return;
	}
	if (spwList_p.size())
	{
		if (!find(spwList_p,visibilityBuffer_p->get()->spectralWindow())) return;
	}

	cout << "Going to process a buffer with: " <<
			rowsIndex_p.size() << " rows, " <<
			channelIndex_p.size() << " channels, " <<
			polarizationIndex_p.size() << " polarizations" << endl;

	for (rowIter = rowsIndex_p.begin();rowIter != rowsIndex_p.end();rowIter++)
	{
		for (channellIter = channelIndex_p.begin();channellIter != channelIndex_p.end();channellIter++)
		{
			for (polarizationIter = polarizationIndex_p.begin();polarizationIter != polarizationIndex_p.end();polarizationIter++)
			{
				computedFlag = computeFlag(*rowIter,*channellIter,*polarizationIter);

				if (computedFlag == flag_p)
				{
					(*this.*applyFlag_p)(*rowIter,*channellIter,*polarizationIter);
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
FlagAgentBase::applyCommonFlags(uInt row, uInt channel, uInt pol)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagCube_p->at(pol,channel,row) = flag_p;
	return;
}

void
FlagAgentBase::applyPrivateFlags(uInt row, uInt channel, uInt pol)
{
	// NOTE: Notice that the position is pol,channel,row, not the other way around
	commonFlagCube_p->at(pol,channel,row) = flag_p;
	privateFlagCube_p->at(pol,channel,row) = flag_p;
	return;
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



} //# NAMESPACE CASA - END



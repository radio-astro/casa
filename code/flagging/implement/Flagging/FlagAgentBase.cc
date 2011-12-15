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

// Needed for the factory method (create)
#include <flagging/Flagging/FlagAgentTimeFreqCrop.h>
#include <flagging/Flagging/FlagAgentClipping.h>
#include <flagging/Flagging/FlagAgentSummary.h>
#include <flagging/Flagging/FlagAgentManual.h>
#include <flagging/Flagging/FlagAgentElevation.h>
#include <flagging/Flagging/FlagAgentQuack.h>
#include <flagging/Flagging/FlagAgentShadow.h>

namespace casa { //# NAMESPACE CASA - BEGIN

////////////////////////////////////
/// FlagAgentBase implementation ///
////////////////////////////////////

// NOTE: We have to initialize the polarizationList_p here, which is a OrderedMap<Int, Vector<Int> >
// because otherwise the compiler complains because we are calling a theoretical default constructor
// OrderedMap() that does not exist.
FlagAgentBase::FlagAgentBase(FlagDataHandler *dh, Record config, uShort iterationApproach, Bool writePrivateFlagCube, Bool flag): polarizationList_p(Vector<Int>(0))
{
	// Initialize logger
	logger_p = new LogIO();

	// Initialize members
	initialize();

	// Set iteration approach
	iterationApproach_p = iterationApproach;

	// Set private flag cube (needed for flag extension)
	writePrivateFlagCube_p = writePrivateFlagCube;

	// Set flag/un-flag mode
	flag_p = flag;

	// Set flag data handler
	flagDataHandler_p = dh;

	// Set (pre-selected) MS
	selectedMeasurementSet_p = flagDataHandler_p->selectedMeasurementSet_p;

	// Set vis buffer
	visibilityBuffer_p = flagDataHandler_p->visibilityBuffer_p;

	// Set agent parameters
	setAgentParameters(config);

	// Set data selection
	setDataSelection(config);

	// Check if async processing is enabled
	backgroundMode_p = false;
	AipsrcValue<Bool>::find (backgroundMode_p,"FlagAgent.background", false);
	if (backgroundMode_p)
	{
		*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " Background mode enabled" << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " Background mode disabled" << LogIO::POST;
	}

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
   originalFlagCube_p = NULL;

   // Initialize selection ranges
   timeSelection_p = String("");
   baselineSelection_p = String("");
   fieldSelection_p = String("");
   // NOTE (first implementation): According to MS selection syntax, spw must be at least *
   // but since we are parsing it only if it was provided it should not be a problem
   // NOTE (after Dic 2011 testing): As far as I know spw selection does not have to be *
   // (can be empty) and in fact applying a spw selection slows down the MSSelection class
   spwSelection_p = String("");
   uvwSelection_p = String("");
   polarizationSelection_p = String("");
   scanSelection_p = String("");
   arraySelection_p = String("");
   observationSelection_p = String("");
   scanIntentSelection_p = String("");

   // Clear up indexes
   rowsIndex_p.clear();
   channelIndex_p.clear();
   polarizationIndex_p.clear();

   // Initialize filters
   antennaNegation_p = false;
   filterChannels_p = false;
   filterRows_p = false;
   filterPols_p = false;

	// Initialize state
   terminationRequested_p = false;
   threadTerminated_p = false;
   processing_p = false;

   // Initialize counters
   chunkFlags_p = 0;
   chunkNaNs_p = 0;
   msFlags_p = 0;
   msNaNs_p = 0;
   visBufferFlags_p = 0;

   //// Initialize configuration ////

   /// Running config
   profiling_p = false;
   backgroundMode_p = false;
   iterationApproach_p = ROWS;
   multiThreading_p = false;
   nThreads_p = 0;
   threadId_p = 0;
   agentName_p = String("");
   /// Flag/Unflag config
   writePrivateFlagCube_p = false;
   flag_p = true;
   /// Mapping config
   dataColumn_p = "data";
   expression_p = "ABS 1";
   dataReference_p = DATA;
   /// Profiling and testing config
   profiling_p = false;
   checkFlags_p = false;

   /////////////////////////////////

   return;
}

FlagAgentBase *
FlagAgentBase::create (FlagDataHandler *dh,Record config)
{
	String mode;
	FlagAgentBase *ret = NULL;

	// Retrieve mode
	if (config.fieldNumber ("mode") >= 0)
	{
		mode = config.asString("mode");
	}
	else
	{
		cerr << "FlagAgentFactory::" << __FUNCTION__ << " Mode not provided" << endl;
		return ret;
	}

	// Write private flags only if extension is required
	bool writePrivateFlags = false;
	if ((config.fieldNumber ("extend")>=0) and (config.asBool("extend")==true))
	{
		writePrivateFlags = true;
	}

	// Manual mode
	if (mode.compare("manualflag")==0)
	{
		FlagAgentManual* agent = new FlagAgentManual(dh,config,writePrivateFlags,true);
		return agent;
	}

	// Unflag mode
	if (mode.compare("unflag")==0)
	{
		FlagAgentManual* agent = new FlagAgentManual(dh,config,writePrivateFlags,false);
		return agent;
	}

	// TimeFreqCrop
	if (mode.compare("tfcrop")==0)
	{
		FlagAgentTimeFreqCrop* agent = new FlagAgentTimeFreqCrop(dh,config,writePrivateFlags);
		return agent;
	}

	// Clip
	if (mode.compare("clip")==0)
	{
		FlagAgentClipping* agent = new FlagAgentClipping(dh,config,writePrivateFlags);
		return agent;
	}

	// Summary
	if (mode.compare("summary")==0)
	{
		FlagAgentSummary* agent = new FlagAgentSummary(dh,config);
		return agent;
	}

	// Elevation
	if (mode.compare("elevation")==0)
	{
		FlagAgentElevation* agent = new FlagAgentElevation(dh,config,writePrivateFlags);
		return agent;
	}

	// Quack
	if (mode.compare("quack")==0)
	{
		FlagAgentQuack* agent = new FlagAgentQuack(dh,config,writePrivateFlags);
		return agent;
	}

	// Shadow
	if (mode.compare("shadow")==0)
	{
		FlagAgentShadow* agent = new FlagAgentShadow(dh,config,writePrivateFlags);
		return agent;
	}

	return ret;
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
	if (backgroundMode_p)
	{
		terminationRequested_p = true;
		while (!threadTerminated_p)
		{
			sched_yield();
		}
	}

	casa::async::Thread::terminate();

	return;
}

void
FlagAgentBase::queueProcess()
{
	if (backgroundMode_p)
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
	if (backgroundMode_p)
	{
		// Wait until we are done with previous buffer
		while (processing_p)
		{
			sched_yield();
		}
	}

	return;
}

void *
FlagAgentBase::run ()
{
	if (backgroundMode_p)
	{
		while (!terminationRequested_p)
		{

			if (processing_p) // NOTE: This races with queueProcess but it is harmless
			{
				// Carry out processing
				runCore();

				// Disable processing to enter in idle mode
				processing_p = false;
			}
			else
			{
				sched_yield();
			}
		}
	}

	processing_p = false;
	threadTerminated_p = true;

	return NULL;
}

void
FlagAgentBase::runCore()
{
	// Set pointer to common flag cube
	commonFlagCube_p = flagDataHandler_p->getModifiedFlagCube();
	originalFlagCube_p = flagDataHandler_p->getOriginalFlagCube();

	// Set vis buffer
	visibilityBuffer_p = flagDataHandler_p->visibilityBuffer_p;

	// Reset VisBuffer flag counters
	visBufferFlags_p = 0;

	// Generate indexes applying data selection filters
	generateAllIndex();
	if (checkIfProcessBuffer())
	{
		// Set pointer to private flag cube
		if (writePrivateFlagCube_p)
		{
			if (privateFlagCube_p) delete privateFlagCube_p;
			privateFlagCube_p = new Cube<Bool>(commonFlagCube_p->shape(),!flag_p);
		}

		switch (iterationApproach_p)
		{
			// Iterate trough (time,freq) maps per antenna pair
			case ANTENNA_PAIRS:
			{
				iterateAntennaPairs();
				break;
			}
			// Iterate trough rows (i.e. baselines)
			case ROWS:
			{
				iterateRows();
				break;
			}
			// Iterate inside every row (i.e. channels) applying a mapping expression
			case IN_ROWS:
			{
				iterateInRows();
				break;
			}
			case ANTENNA_PAIRS_INTERACTIVE:
			{
				preProcessBuffer(*(flagDataHandler_p->visibilityBuffer_p->get()));
				iterateAntennaPairsInteractive(flagDataHandler_p->getAntennaPairMap());
				break;
			}
			// Iterate trough (time,freq) maps per antenna pair doing a common pre-processing before
			case ANTENNA_PAIRS_PREPROCESS_BUFFER:
			{
				preProcessBuffer(*(flagDataHandler_p->visibilityBuffer_p->get()));
				iterateAntennaPairs();
				break;
			}
			// Iterate trough rows (i.e. baselines) doing a common pre-processing before
			case ROWS_PREPROCESS_BUFFER:
			{
				preProcessBuffer(*(flagDataHandler_p->visibilityBuffer_p->get()));
				iterateRows();
				break;
			}
			// Iterate inside every row (i.e. channels) applying a mapping expression doing a common pre-processing before
			case IN_ROWS_PREPROCESS_BUFFER:
			{
				preProcessBuffer(*(flagDataHandler_p->visibilityBuffer_p->get()));
				iterateInRows();
				break;
			}
			default:
			{
				preProcessBuffer(*(flagDataHandler_p->visibilityBuffer_p->get()));
				iterateRows();
				break;
			}
		}
	}

	// If any flag was raised, then we have to flush the flagCube
	if (visBufferFlags_p>0) flagDataHandler_p->flushFlags_p = true;

	// Update chunk counter
	chunkFlags_p += visBufferFlags_p;

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

		if (arraySelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no array selection" << LogIO::POST;
		}
		else
		{

			parser.setArrayExpr(arraySelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			arrayList_p=parser.getSubArrayList();
			filterRows_p=true;

			// Request to pre-load ArrayId
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ArrayId);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " array selection is " << arraySelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " array ids are " << arrayList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no array selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("field");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("field"), fieldSelection_p);

		if (fieldSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no field selection" << LogIO::POST;
		}
		else
		{

			parser.setFieldExpr(fieldSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			fieldList_p=parser.getFieldList();
			filterRows_p=true;

			// Request to pre-load FieldId
			flagDataHandler_p->preLoadColumn(VisBufferComponents::FieldId);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " field selection is " << fieldSelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " field ids are " << fieldList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no field selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("scan");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("scan"), scanSelection_p);

		if (scanSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no scan selection" << LogIO::POST;
		}
		else
		{
			parser.setScanExpr(scanSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			scanList_p=parser.getScanList();
			filterRows_p=true;

			// Request to pre-load scan
			flagDataHandler_p->preLoadColumn(VisBufferComponents::Scan);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " scan selection is " << scanSelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " scan ids are " << scanList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no scan selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("timerange");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("timerange"), timeSelection_p);

		if (timeSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no time selection" << LogIO::POST;
		}
		else
		{
			parser.setTimeExpr(timeSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			timeList_p=parser.getTimeList();
			filterRows_p=true;

			// Request to pre-load time
			flagDataHandler_p->preLoadColumn(VisBufferComponents::Time);


			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " timerange selection is " << timeSelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " time ranges in MJD are " << timeList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no time selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("spw");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("spw"), spwSelection_p);

		if (spwSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no spw selection" << LogIO::POST;
		}
		else
		{
			parser.setSpwExpr(spwSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			spwList_p=parser.getSpwList();
			filterRows_p=true;

			channelList_p=parser.getChanList();
			filterChannels_p=true;

			// Request to pre-load spw
			flagDataHandler_p->preLoadColumn(VisBufferComponents::SpW);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " spw selection is " << spwSelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " channel selection are " << channelList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no spw selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("antenna");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("antenna"), baselineSelection_p);

		if (baselineSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no antenna selection" << LogIO::POST;
		}
		else
		{
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " antenna selection is " << baselineSelection_p << LogIO::POST;

			// Remove antenna negation operator (!) and set antenna negation flag
			size_t pos = baselineSelection_p.find(String("!"));
			while (pos != String::npos)
			{
				antennaNegation_p = true;
				baselineSelection_p.replace(pos,1,String(""));
				*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " antenna selection is the negation of " << baselineSelection_p << LogIO::POST;
				pos = baselineSelection_p.find(String("!"));
			}

			parser.setAntennaExpr(baselineSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			antenna1List_p=parser.getAntenna1List();
			antenna2List_p=parser.getAntenna2List();
			baselineList_p=parser.getBaselineList();
			filterRows_p=true;

			// Request to pre-load antenna1/2
			flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant1);
			flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant2);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " selected baselines are " << baselineList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no baseline selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("uvrange");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("uvrange"), uvwSelection_p);

		if (uvwSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no uvw selection" << LogIO::POST;
		}
		else
		{
			parser.setUvDistExpr(uvwSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			uvwList_p=parser.getUVList();
			filterRows_p=true;

			// Request to pre-load uvw
			flagDataHandler_p->preLoadColumn(VisBufferComponents::Uvw);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " uvrange selection is " << uvwSelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " uvrange ids are " << uvwList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no uvw selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("correlation");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("correlation"), polarizationSelection_p);

		if (polarizationSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no correlation selection" << LogIO::POST;
		}
		else
		{
			parser.setPolnExpr(polarizationSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			polarizationList_p=parser.getPolMap();
			filterPols_p=true;

			// Request to pre-load CorrType
			flagDataHandler_p->preLoadColumn(VisBufferComponents::CorrType);

			// NOTE: casa::LogIO does not support outstream from OrderedMap<Int, Vector<Int> > objects yet
			ostringstream polarizationListToPrint (ios::in | ios::out);
			polarizationListToPrint << polarizationList_p;

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " correlation selection is " << polarizationSelection_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " correlation ids are " << polarizationListToPrint.str() << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no polarization selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("observation");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("observation"), observationSelection_p);

		if (observationSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no observation selection" << LogIO::POST;
		}
		else
		{
			parser.setPolnExpr(observationSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			observationList_p=parser.getObservationList();
			filterRows_p=true;

			// Request to pre-load ObservationId
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ObservationId);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " observation selection is " << observationList_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " observation ids are " << observationList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no observation selection" << LogIO::POST;
	}

	exists = config.fieldNumber ("intent");
	if (exists >= 0)
	{
		config.get (config.fieldNumber ("intent"), scanIntentSelection_p);

		if (scanIntentSelection_p.empty())
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no intent selection" << LogIO::POST;
		}
		else
		{
			parser.setPolnExpr(scanIntentSelection_p);
			parser.toTableExprNode(selectedMeasurementSet_p);
			scanIntentList_p=parser.getStateObsModeList();
			filterRows_p=true;

			// Request to pre-load StateId
			flagDataHandler_p->preLoadColumn(VisBufferComponents::StateId);

			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " scan intent selection is " << scanIntentList_p << LogIO::POST;
			*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " scan intent ids are " << scanIntentList_p << LogIO::POST;
		}
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " no scan intent selection" << LogIO::POST;
	}

	return;
}

void
FlagAgentBase::setAgentParameters(Record config)
{
	// NOTE: This class must be re-implemented in the derived classes for
	// the specific parameters although here we handle the common ones

	int exists;

	exists = config.fieldNumber ("name");
	if (exists >= 0)
	{
		agentName_p = config.asString("name");
	}
	else if (agentName_p.empty())
	{
		agentName_p = "FlagAgentUnknown";
	}

	exists = config.fieldNumber ("nThreads");
	if (exists >= 0)
	{
		nThreads_p = atoi(config.asString("nThreads").c_str());
		*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " nThreads is " << nThreads_p << LogIO::POST;

		if (nThreads_p > 0)
		{
			multiThreading_p = true;
			exists = config.fieldNumber ("threadId");
			if (exists >= 0)
			{
				threadId_p = atoi(config.asString("threadId").c_str());
				*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " threadId is " << threadId_p << LogIO::POST;

				if (threadId_p < 0 or threadId_p>=nThreads_p)
				{
					*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ << " Thread Id range is [0,nThreads-1], disabling multithreading" << LogIO::POST;
				}
			}
			else
			{
				*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ << " Thread Id not provided, disabling multithreading" << LogIO::POST;
				multiThreading_p = false;
			}
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ << " Number of threads must be positive, disabling multithreading" << LogIO::POST;
			dataColumn_p = "data";
		}
	}


	if (	(iterationApproach_p == IN_ROWS) or
			(iterationApproach_p == ANTENNA_PAIRS) or
			(iterationApproach_p == ANTENNA_PAIRS_INTERACTIVE) or
			(iterationApproach_p == IN_ROWS_PREPROCESS_BUFFER) or
			(iterationApproach_p == ANTENNA_PAIRS_PREPROCESS_BUFFER))
	{
		exists = config.fieldNumber ("expression");
		if (exists >= 0)
		{
			expression_p = config.asString("expression");
		}
		else
		{
			expression_p = "ABS 1";
		}

		expression_p.upcase();

		// Request to pre-load spw and corrType
		flagDataHandler_p->preLoadColumn(VisBufferComponents::SpW);
		flagDataHandler_p->preLoadColumn(VisBufferComponents::CorrType);

		// Check if expression is one of the supported operators
		if ((expression_p.find("REAL") == string::npos) and
				(expression_p.find("IMAG") == string::npos) and
				(expression_p.find("ARG") == string::npos) and
				(expression_p.find("ABS") == string::npos) and
				(expression_p.find("NORM") == string::npos))
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<
					" Unsupported mapping expression: " <<
					expression_p << ", selecting ABS by default. Supported expressions: REAL,IMAG,ARG,ABS,NORM." << LogIO::POST;
			expression_p = "ABS 1";
		}

		// If expression is ABS 1 we flag the first correlation product
		if (expression_p.find("1") != string::npos)
		{
			expression_p = "ABS " + flagDataHandler_p->corrProducts_p->at(0);
		}

		*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " visibility expression is " << expression_p << LogIO::POST;

		exists = config.fieldNumber ("datacolumn");
		if (exists >= 0)
		{
			dataColumn_p = config.asString("datacolumn");
		}
		else
		{
			dataColumn_p = "data";
		}

		dataColumn_p.upcase();

		// Check if dataColumn_p is one of the supported columns (or residues)
		if (dataColumn_p.compare("DATA") == 0)
		{
			dataReference_p = DATA;

			// Request to pre-load ObservedCube
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ObservedCube);
		}
		else if (dataColumn_p.compare("CORRECTED") == 0)
		{
			dataReference_p = CORRECTED;

			// Request to pre-load CorrectedCube
			flagDataHandler_p->preLoadColumn(VisBufferComponents::CorrectedCube);
		}
		else if (dataColumn_p.compare("MODEL") == 0)
		{
			dataReference_p = MODEL;

			// Request to pre-load ModelCube
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ModelCube);
		}
		else if (dataColumn_p.compare("RESIDUAL") == 0)
		{
			dataReference_p = RESIDUAL;

			// Request to pre-load CorrectedCube and ModelCube
			flagDataHandler_p->preLoadColumn(VisBufferComponents::CorrectedCube);
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ModelCube);
		}
		else if (dataColumn_p.compare("RESIDUAL_DATA") == 0)
		{
			dataReference_p = RESIDUAL_DATA;

			// Request to pre-load ObservedCube and ModelCube
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ObservedCube);
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ModelCube);
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<
					" Unsupported data column: " <<
					dataColumn_p << ", using data by default. Supported columns: data,corrected,model,residual,residual_data" << LogIO::POST;
			dataColumn_p = "data";

			// Request to pre-load ObservedCube
			flagDataHandler_p->preLoadColumn(VisBufferComponents::ObservedCube);
		}

		*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ << " data column is " << dataColumn_p << LogIO::POST;
	}

	return;
}

void
FlagAgentBase::generateAllIndex()
{
	Int nPolarizations,nChannels,nRows;
	commonFlagCube_p->shape(nPolarizations,nChannels,nRows);
	generateRowsIndex(nRows);
	generateChannelIndex(nChannels);
	generatePolarizationIndex(nPolarizations);

	return;
}

void
FlagAgentBase::generateRowsIndex(uInt nRows)
{
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

	return;
}

void
FlagAgentBase::generateChannelIndex(uInt nChannels)
{
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

	return;
}

void
FlagAgentBase::generatePolarizationIndex(uInt nPolarizations)
{
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

	return;
}

std::vector<uInt> *
FlagAgentBase::generateAntennaPairRowsIndex(Int antenna1, Int antenna2)
{
	// Retrieve common rows for this antenna pair from FlagDataHandler
	std::vector<uInt> commonRows = (*flagDataHandler_p->getAntennaPairMap())[std::make_pair(antenna1,antenna2)];

	// Second step: Filter out unnecessary rows
	std::vector<uInt> *privateRows = NULL;
	if (filterRows_p)
	{
		privateRows = new std::vector<uInt>();
		for (std::vector<uInt>::iterator iter=commonRows.begin();iter!=commonRows.end();iter++)
		{
			if (std::find(rowsIndex_p.begin(),rowsIndex_p.end(),*iter) != rowsIndex_p.end())
			{
				privateRows->push_back(*iter);
			}
		}
	}
	else
	{
		privateRows = new std::vector<uInt>((*flagDataHandler_p->getAntennaPairMap())[std::make_pair(antenna1,antenna2)]);
	}

	return privateRows;
}

void
FlagAgentBase::indigen(vector<uInt> &index, uInt size)
{
	index.reserve(size);
	for (uInt i=0; i<size; i++ )
	{
		index.push_back(i);
	}

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
FlagAgentBase::isNaN(Float number)
{
	bool result = isnan(number);
	chunkNaNs_p += result;
	return result;
}

bool
FlagAgentBase::isNaN(Double number)
{
	bool result = isnan(number);
	chunkNaNs_p += result;
	return result;
}

void
FlagAgentBase::chunkSummary()
{
	// With this check we skip cases like summary or display
	if (chunkFlags_p > 0)
	{
		msFlags_p +=  chunkFlags_p;
		*logger_p << LogIO::NORMAL << "=> " << agentName_p.c_str()  << " Data flagged in this chunk: " <<  100.0*chunkFlags_p/flagDataHandler_p->chunkCounts_p<< "%" << LogIO::POST;
	}

	// Only the clipping agent is capable of detecting this, and besides in general
	// we should not have NaNs, so it is better not to print this log if possible
	if (chunkNaNs_p > 0)
	{
		msNaNs_p += chunkNaNs_p;
		*logger_p << LogIO::NORMAL << "=> " << agentName_p.c_str()  << " Number of NaNs detected in this chunk: " <<  (Double)chunkNaNs_p << LogIO::POST;
	}

	chunkFlags_p = 0;
	chunkNaNs_p = 0;
	visBufferFlags_p = 0;
	return;
}

void
FlagAgentBase::msSummary()
{
	// With this check we skip cases like summary or display
	if (msFlags_p > 0)
	{
		*logger_p << LogIO::NORMAL << "=> " << agentName_p.c_str()  << " Total data flagged in MS: " <<  100.0*msFlags_p/flagDataHandler_p->msCounts_p<< "%" << LogIO::POST;
	}

	if (msNaNs_p > 0)
	{
		*logger_p << LogIO::NORMAL << "=> " << agentName_p.c_str()  << " Total number NaNs detected in MS: " <<  (Double)msNaNs_p << LogIO::POST;
	}

	msFlags_p = 0;
	msNaNs_p = 0;
	return;
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

	return true;
}

void
FlagAgentBase::preProcessBuffer(const VisBuffer &visBuffer)
{
	return;
}

void
FlagAgentBase::iterateRows()
{
	// Create FlagMapper objects and parse the correlation selection
	FlagMapper flagsMap = FlagMapper(flag_p,polarizationIndex_p);

	// Set CubeViews in FlagMapper
	setFlagsMap(NULL,&flagsMap);

	// Activate check mode
	if (checkFlags_p) flagsMap.activateCheckMode();

	// Some log info
	if (multiThreading_p)
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__
				<<  " Thread Id " << threadId_p << ":" << nThreads_p
				<< " Will process every " << nThreads_p << " rows starting with row " << threadId_p << " from a total of " <<
				rowsIndex_p.size() << " rows (" << rowsIndex_p[0] << "-" << rowsIndex_p[rowsIndex_p.size()-1] << ") " <<
				channelIndex_p.size() << " channels (" << channelIndex_p[0] << "-" << channelIndex_p[channelIndex_p.size()-1] << ") " <<
				polarizationIndex_p.size() << " polarizations (" << polarizationIndex_p[0] << "-" << polarizationIndex_p[polarizationIndex_p.size()-1] << ")" << LogIO::POST;

	}
	else
	{
		// Some logging info
		*logger_p 	<< LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " Going to process a buffer with: " <<
				rowsIndex_p.size() << " rows (" << rowsIndex_p[0] << "-" << rowsIndex_p[rowsIndex_p.size()-1] << ") " <<
				channelIndex_p.size() << " channels (" << channelIndex_p[0] << "-" << channelIndex_p[channelIndex_p.size()-1] << ") " <<
				polarizationIndex_p.size() << " polarizations (" << polarizationIndex_p[0] << "-" << polarizationIndex_p[polarizationIndex_p.size()-1] << ")" << LogIO::POST;
	}

	// Loop trough selected rows
	Int rowIdx = 0;
	vector<uInt>::iterator rowIter;
	for (rowIter = rowsIndex_p.begin();rowIter != rowsIndex_p.end();rowIter++)
	{
		if (multiThreading_p and (rowIdx % nThreads_p != threadId_p))
		{
			// Increment row index
			rowIdx++;

			// Continue with next row
			continue;
		}

		// Compute flags for this row
		computeRowFlags(*(flagDataHandler_p->visibilityBuffer_p->get()), flagsMap,*rowIter);

		// Increment row index
		rowIdx++;
	}

	return;
}

void
FlagAgentBase::iterateInRows()
{
	// Check if the visibility expression is suitable for this spw
	if (!checkVisExpression(flagDataHandler_p->getPolarizationMap())) return;

	// Create VisMapper and FlagMapper objects and parse the polarization expression
	VisMapper visibilitiesMap = VisMapper(expression_p,flagDataHandler_p->getPolarizationMap());
	FlagMapper flagsMap = FlagMapper(flag_p,visibilitiesMap.getSelectedCorrelations());

	// Set CubeViews in VisMapper
	setVisibilitiesMap(NULL,&visibilitiesMap);

	// Set CubeViews in FlagMapper
	setFlagsMap(NULL,&flagsMap);

	// Activate check mode
	if (checkFlags_p) flagsMap.activateCheckMode();

	// Some log info
	if (multiThreading_p)
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__
				<<  " Thread Id " << threadId_p << ":" << nThreads_p
				<< " Will process every " << nThreads_p << " rows starting with row " << threadId_p
				<< " from a total of " << rowsIndex_p.size() << " rows with " << channelIndex_p.size() << " channels ("
				<< channelIndex_p[0] << "-" << channelIndex_p[channelIndex_p.size()-1] << ") each one" << LogIO::POST;
	}
	else
	{
		// Some logging info
		*logger_p 	<< LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ << " Going to process a buffer with: " <<
				rowsIndex_p.size() << " rows (" << rowsIndex_p[0] << "-" << rowsIndex_p[rowsIndex_p.size()-1] << ") " <<
				channelIndex_p.size() << " channels (" << channelIndex_p[0] << "-" << channelIndex_p[channelIndex_p.size()-1] << ") "<< LogIO::POST;
	}

	// Iterate trough rows
	Int rowIdx = 0;
	vector<uInt>::iterator rowIter;
	for (rowIter = rowsIndex_p.begin();rowIter != rowsIndex_p.end();rowIter++)
	{
		if (multiThreading_p and (rowIdx % nThreads_p != threadId_p))
		{
			// Increment row index
			rowIdx++;

			// Continue with next row
			continue;
		}

		// Compute flags for this row
		computeInRowFlags(*(flagDataHandler_p->visibilityBuffer_p->get()),visibilitiesMap,flagsMap,*rowIter);

		// Increment row index
		rowIdx++;
	}

	return;
}

void
FlagAgentBase::iterateAntennaPairs()
{
	// Check if the visibility expression is suitable for this spw
	if (!checkVisExpression(flagDataHandler_p->getPolarizationMap())) return;

	antennaPairMapIterator myAntennaPairMapIterator;
	std::pair<Int,Int> antennaPair;
	std::vector<uInt> *antennaRows = NULL;
	IPosition cubeShape;

	// Create VisMapper and FlagMapper objects and parse the polarization expression
	VisMapper visibilitiesMap = VisMapper(expression_p,flagDataHandler_p->getPolarizationMap());
	FlagMapper flagsMap = FlagMapper(flag_p,visibilitiesMap.getSelectedCorrelations());

	// Activate check mode
	if (checkFlags_p) flagsMap.activateCheckMode();

	// Some log info
	if (multiThreading_p)
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__
				<<  " Thread Id " << threadId_p << ":" << nThreads_p
				<< " Will process every " << nThreads_p << " baselines starting with baseline " << threadId_p
				<< " from a total of " << flagDataHandler_p->getAntennaPairMap()->size() << LogIO::POST;
	}
	else
	{
		*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Iterating trough " << flagDataHandler_p->getAntennaPairMap()->size() <<  " antenna pair maps " << LogIO::POST;
	}


	uShort antennaPairdIdx = 0;
	for (myAntennaPairMapIterator=flagDataHandler_p->getAntennaPairMap()->begin(); myAntennaPairMapIterator != flagDataHandler_p->getAntennaPairMap()->end(); ++myAntennaPairMapIterator)
	{
		if (multiThreading_p and (antennaPairdIdx % nThreads_p != threadId_p))
		{
			// Increment antenna pair index
			antennaPairdIdx++;

			// Continue with next antenna pair
			continue;
		}

		// Get antenna pair from map
		antennaPair = myAntennaPairMapIterator->first;

		// Check if antenna pair is in the baselines list of this agent
		if (baselineList_p.size()>0)
		{
			if (!find(baselineList_p,antennaPair.first,antennaPair.second)) continue;
		}

		// Get rows corresponding to this antenna pair
		antennaRows = generateAntennaPairRowsIndex(antennaPair.first,antennaPair.second);

		// If none of the antenna pair rows were eligible then go to next pair
		if (antennaRows->empty())
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested baseline (" << antennaPair.first << "," << antennaPair.second << ") does not have any rows in this chunk" << LogIO::POST;

			// Increment antenna pair index
			antennaPairdIdx++;

			// Delete antenna pair rows
			delete antennaRows;

			// Continue with next antenna pair
			continue;
		}

		// Set CubeViews in VisMapper
		setVisibilitiesMap(antennaRows,&visibilitiesMap);

		// Set CubeViews in FlagMapper
		setFlagsMap(antennaRows,&flagsMap);

		// Flag map
		computeAntennaPairFlags(*(flagDataHandler_p->visibilityBuffer_p->get()),visibilitiesMap,flagsMap,antennaPair.first,antennaPair.second,*antennaRows);

		// Increment antenna pair index
		antennaPairdIdx++;

		// Delete antenna pair rows
		delete antennaRows;
	}

	return;
}

void
FlagAgentBase::iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr)
{
	// Check if the visibility expression is suitable for this spw
	if (!checkVisExpression(flagDataHandler_p->getPolarizationMap())) return;

	// Iterate trough antenna pair map
	std::pair<Int,Int> antennaPair;
	antennaPairMapIterator myAntennaPairMapIterator;
	for (myAntennaPairMapIterator=antennaPairMap_ptr->begin(); myAntennaPairMapIterator != antennaPairMap_ptr->end(); ++myAntennaPairMapIterator)
	{
		// Get antenna pair from map
		antennaPair = myAntennaPairMapIterator->first;

		// Process antenna pair
		processAntennaPair(antennaPair.first,antennaPair.second);
	}

	return;
}

void
FlagAgentBase::processAntennaPair(Int antenna1,Int antenna2)
{
	std::pair<Int,Int> antennaPair = std::make_pair(antenna1,antenna2);
	antennaPairMapIterator index = flagDataHandler_p->getAntennaPairMap()->find(antennaPair);
	if (index != flagDataHandler_p->getAntennaPairMap()->end())
	{
		std::vector<uInt> *antennaRows = generateAntennaPairRowsIndex(antennaPair.first,antennaPair.second);
		if (antennaRows->empty())
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested baseline (" << antennaPair.first << "," << antennaPair.second << ") does not have any rows in this chunk" << LogIO::POST;
		}
		else
		{
			// Check if antenna pair is in the baselines list of this agent
			if ((baselineList_p.size()>0) and (!find(baselineList_p,antennaPair.first,antennaPair.second)))
			{
				*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ <<  "Requested baseline (" << antennaPair.first << "," << antennaPair.second << ") is not included in the selected baseline range" << LogIO::POST;
			}
			else
			{
				*logger_p << LogIO::NORMAL << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Going to process requested baseline (" << antennaPair.first << "," << antennaPair.second << ") " << LogIO::POST;

				// Create VisMapper and FlagMapper objects and parse the polarization expression
				VisMapper visibilitiesMap = VisMapper(expression_p,flagDataHandler_p->getPolarizationMap());
				FlagMapper flagsMap = FlagMapper(flag_p,visibilitiesMap.getSelectedCorrelations());

				// Set CubeViews in VisMapper
				setVisibilitiesMap(antennaRows,&visibilitiesMap);

				// Set CubeViews in FlagMapper
				setFlagsMap(antennaRows,&flagsMap);

				// Flag map
				computeAntennaPairFlags(*(flagDataHandler_p->visibilityBuffer_p->get()),visibilitiesMap,flagsMap,antennaPair.first,antennaPair.second,*antennaRows);
			}
		}
	}
	else
	{
		*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested baseline (" << antennaPair.first << "," << antennaPair.second << ") is not available in this chunk " << LogIO::POST;
	}

	return;
}

void
FlagAgentBase::setVisibilitiesMap(std::vector<uInt> *rows,VisMapper *visMap)
{
	// First step: Get reference visibility cubes
	Cube<Complex> *leftVisCube = NULL;
	Cube<Complex> *rightVisCube = NULL;
	switch (dataReference_p)
	{
		case DATA:
		{
			leftVisCube = &(visibilityBuffer_p->get()->visCube());
			break;
		}
		case CORRECTED:
		{
			leftVisCube = &(visibilityBuffer_p->get()->correctedVisCube());
			break;
		}
		case MODEL:
		{
			leftVisCube = &(visibilityBuffer_p->get()->modelVisCube());
			break;
		}
		case RESIDUAL:
		{
			leftVisCube = &(visibilityBuffer_p->get()->correctedVisCube());
			rightVisCube = &(visibilityBuffer_p->get()->modelVisCube());
			break;
		}
		case RESIDUAL_DATA:
		{
			leftVisCube = &(visibilityBuffer_p->get()->visCube());
			rightVisCube = &(visibilityBuffer_p->get()->modelVisCube());
			break;
		}
		default:
		{
			leftVisCube = &(visibilityBuffer_p->get()->visCube());
			break;
		}
	}

	// Second step create CubeViews from selected vis cubes
	CubeView<Complex> *leftVisCubeView = NULL;
	CubeView<Complex> *rightVisCubeView = NULL;
	leftVisCubeView = new CubeView<Complex>(leftVisCube,rows,&channelIndex_p);
	if (rightVisCube != NULL) rightVisCubeView = new CubeView<Complex>(rightVisCube,rows,&channelIndex_p);


	// Third step: Set CubeViews in mapper
	visMap->setParentCubes(leftVisCubeView,rightVisCubeView);

	return;
}

void
FlagAgentBase::setFlagsMap(std::vector<uInt> *rows,FlagMapper *flagMap)
{
	// First step create common/private CubeViews
	CubeView<Bool> *commonFlagCube = NULL;
	CubeView<Bool> *originalFlagCube = NULL;
	CubeView<Bool> *privateFlagCube = NULL;

	// Second step create CubeViews from selected vis cubes
	commonFlagCube= new CubeView<Bool>(commonFlagCube_p,rows,&channelIndex_p);
	originalFlagCube= new CubeView<Bool>(originalFlagCube_p,rows,&channelIndex_p);
	if (writePrivateFlagCube_p) privateFlagCube= new CubeView<Bool>(privateFlagCube_p,rows,&channelIndex_p);

	// Third step: Set CubeViews in mapper
	flagMap->setParentCubes(commonFlagCube,originalFlagCube,privateFlagCube);

	return;
}

Bool
FlagAgentBase::checkVisExpression(polarizationMap *polMap)
{
	// If we find I directly in the polarization map we assume is Water Vapor Radiometer data
	// And we only process it if the user requested WVR
	if (polMap->find(Stokes::I) != polMap->end())
	{
		if (expression_p.find("WVR") != string::npos)
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Detected Water Vapor data in spw (" <<
					visibilityBuffer_p->get()->spectralWindow() << "), will be flagged" << LogIO::POST;
			return True;
		}
		else
		{
			*logger_p << LogIO::DEBUG1 << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Detected Water Vapor data in spw (" <<
					visibilityBuffer_p->get()->spectralWindow() << "), won't be flagged" << LogIO::POST;
			return False;
		}
	}

	if (expression_p.find("XX") != string::npos)
	{
		if (polMap->find(Stokes::XX) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (XX) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("YY") != string::npos)
	{
		if (polMap->find(Stokes::YY) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (YY) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}

	}
	else if (expression_p.find("XY") != string::npos)
	{
		if (polMap->find(Stokes::XY) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (XY) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("YX") != string::npos)
	{
		if (polMap->find(Stokes::YX) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (YX) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("RR") != string::npos)
	{
		if (polMap->find(Stokes::RR) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (RR) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("LL") != string::npos)
	{
		if (polMap->find(Stokes::LL) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (LL) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("LR") != string::npos)
	{
		if (polMap->find(Stokes::LR) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (LR) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("RL") != string::npos)
	{
		if (polMap->find(Stokes::RL) != polMap->end())
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested correlation (RL) not available in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("I") != string::npos)
	{
		if (polMap->find(Stokes::I) != polMap->end())
		{
			return True;
		}
		else if ((polMap->find(Stokes::XX) != polMap->end()) and (polMap->find(Stokes::YY) != polMap->end()))
		{
			return True;
		}
		else if ((polMap->find(Stokes::RR) != polMap->end()) and (polMap->find(Stokes::LL) != polMap->end()))
		{

			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested Stokes parameter (I) cannot be computed from available polarizations in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("Q") != string::npos)
	{
		if (polMap->find(Stokes::Q) != polMap->end())
		{
			return True;
		}
		else if ((polMap->find(Stokes::XX) != polMap->end()) and (polMap->find(Stokes::YY) != polMap->end()))
		{
			return True;
		}
		else if ((polMap->find(Stokes::RL) != polMap->end()) and (polMap->find(Stokes::LR) != polMap->end()))
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested Stokes parameter (Q) cannot be computed from available polarizations in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("U") != string::npos)
	{
		if (polMap->find(Stokes::U) != polMap->end())
		{
			return True;
		}
		else if ((polMap->find(Stokes::XY) != polMap->end()) and (polMap->find(Stokes::YX) != polMap->end()))
		{
			return True;
		}
		else if ((polMap->find(Stokes::RL) != polMap->end()) and (polMap->find(Stokes::LR) != polMap->end()))
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested Stokes parameter (U) cannot be computed from available polarizations in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else if (expression_p.find("V") != string::npos)
	{
		if (polMap->find(Stokes::V) != polMap->end())
		{
			return True;
		}
		else if ((polMap->find(Stokes::XY) != polMap->end()) and (polMap->find(Stokes::YX) != polMap->end()))
		{
			return True;
		}
		else if ((polMap->find(Stokes::RR) != polMap->end()) and (polMap->find(Stokes::LL) != polMap->end()))
		{
			return True;
		}
		else
		{
			*logger_p << LogIO::WARN << agentName_p.c_str() << "::" << __FUNCTION__ <<  " Requested Stokes parameter (V) cannot be computed from available polarizations in current spectral window (" <<
					visibilityBuffer_p->get()->spectralWindow() << ") " << LogIO::POST;
			return False;
		}
	}
	else
	{
		throw AipsError("Unknown polarization requested, (" + expression_p + ") supported types are: XX,YY,XY,YX,RR,LL,RL,LR,I,Q,U,V");
		return False;
	}
	return False;
}

void
FlagAgentBase::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
{
	// TODO: This class must be re-implemented in the derived classes
	return;
}

void
FlagAgentBase::computeInRowFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags, uInt row)
{
	// TODO: This class must be re-implemented in the derived classes
	return;
}

void
FlagAgentBase::computeAntennaPairFlags(const VisBuffer &visBuffer, VisMapper &visibilities,FlagMapper &flags,Int antenna1,Int antenna2,vector<uInt> &rows)
{
	// TODO: This class must be re-implemented in the derived classes
	return;
}


////////////////////////////////////
/// FlagAgentList implementation ///
////////////////////////////////////

FlagAgentList::FlagAgentList()
{
	container_p.clear();
}

FlagAgentList::~FlagAgentList()
{
	clear();
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

void FlagAgentList::clear()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		delete (*iterator_p);
	}
	container_p.clear();
	return;
}

bool FlagAgentList::empty()
{

	return container_p.empty();
}

size_t FlagAgentList::size()
{
	return container_p.size();
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

void FlagAgentList::chunkSummary()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->chunkSummary();
	}

	return;
}

void FlagAgentList::msSummary()
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->msSummary();
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

void FlagAgentList::setCheckMode(bool enable)
{
	for (iterator_p = container_p.begin();iterator_p != container_p.end(); iterator_p++)
	{
		(*iterator_p)->setCheckMode(enable);
	}

	return;
}


} //# NAMESPACE CASA - END



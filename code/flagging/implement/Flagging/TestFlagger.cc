///# TestFlagger.cc: this defines TestFlagger
//# Copyright (C) 2000,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/HostInfo.h>
#include <flagging/Flagging/TestFlagger.h>
#include <flagging/Flagging/FlagDataHandler.h>
#include <flagging/Flagging/FlagAgentBase.h>
#include <flagging/Flagging/FlagAgentSummary.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>
#include <sstream>
#include <iostream>
#include <vector>

namespace casa {

const bool TestFlagger::dbg = true;

LogIO TestFlagger::os( LogOrigin("TestFlagger") );

// TODO: add wrapping functions to existing fg.setdata(), fg.setclearflags(), etc?!


// -----------------------------------------------------------------------
// Default Constructor
// -----------------------------------------------------------------------
TestFlagger::TestFlagger ()
{
	fdh_p = NULL;
	done();
}


TestFlagger::~TestFlagger ()
{
	done();
}

void
TestFlagger::done()
{
	if(fdh_p){
		delete fdh_p;
		fdh_p = NULL;
	}

	// Default values of parameters
	msname_p = "";
	iterationApproach_p = FlagDataHandler::SUB_INTEGRATION;
	timeInterval_p = 0.0;
	spw_p = "";
	scan_p = "";
	field_p = "";
	antenna_p = "";
	timerange_p = "";
	correlation_p = "";
	intent_p = "";
	feed_p = "";
	array_p = "";
	uvrange_p = "";
	observation_p = "";

	if (! dataselection_p.empty()) {
		Record temp;
		dataselection_p = temp;
	}

	if(summaryAgent_p){
		delete summaryAgent_p;
		summaryAgent_p = NULL;
	}

//	if (! flagsummary_p.empty()){
//		Record temp;
//		flagsummary_p = temp;
//	}

	mode_p = "";
	agents_config_list_p.clear();
	agents_list_p.clear();

	return;
}

// ---------------------------------------------------------------------
// TestFlagger::configTestFlagger
// Parse some parameters to the FlagDataHandler
// ---------------------------------------------------------------------
bool
TestFlagger::configTestFlagger(Record config)
{
	// msname, async, parallel
	LogIO os(LogOrigin("TestFlagger", "configTestFlagger()", WHERE));

	if (config.empty()){
		return false;
	}

	// get the parameters to parse
	// TODO: Check for the existence later....
	if(dbg){
		cout << "configTestFlagger: "<< endl;
		config.get("msname", msname_p);
		cout << "configTestFlagger: "<< msname_p << endl;
		config.get("ntime", timeInterval_p);
		cout << "configTestFlagger: "<< timeInterval_p << endl;
	}

	return true;
}

// ---------------------------------------------------------------------
// TestFlagger::parseDataSelection
// Parse union of parameters to the FlagDataHandler
// ---------------------------------------------------------------------
bool
TestFlagger::parseDataSelection(Record selrec)
{

	LogIO os(LogOrigin("TestFlagger", "parseDataSelection()", WHERE));

	if(selrec.empty()) {
		return false;
	}

	dataselection_p = selrec;

	// Get the selection parameters
	dataselection_p.get("spw", spw_p);
	dataselection_p.get("scan", scan_p);
	dataselection_p.get("field", field_p);
	dataselection_p.get("antenna", antenna_p);
	dataselection_p.get("timerange", timerange_p);
	dataselection_p.get("correlation", correlation_p);
	dataselection_p.get("intent", intent_p);
	dataselection_p.get("feed", feed_p);
	dataselection_p.get("array", array_p);
	dataselection_p.get("uvrange", uvrange_p);
	dataselection_p.get("observation", observation_p);

	return true;
}

// ---------------------------------------------------------------------
// TestFlagger::parseAgentParameters
// Create a vector of agents and parameters
// Each input record contains data selection parameters
// and agent's specific parameters
// ---------------------------------------------------------------------
bool
TestFlagger::parseAgentParameters(Record agent_params)
{
	LogIO os(LogOrigin("TestFlagger", "parseAgentParameters()", WHERE));

	if(agent_params.empty()){
		return false;
	}

	// TODO: should I verify the contents of the record?

	// If there is a tfcrop agent in the list, we should
	// change the iterationApproach to all the agents
	agent_params.get("mode", mode_p);
	if (mode_p == "tfcrop"){
		iterationApproach_p = FlagDataHandler::COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY;
	}
	else if (mode_p == "summary"){
		if(dbg)
			cout << "parseAgentParameters: mode_p="<< mode_p << endl;
		if(summaryAgent_p == NULL){
			summaryAgent_p = new FlagAgentSummary(fdh_p, agent_params);
			if(dbg)
				cout << "parseAgentParameters: created new summary agent" << endl;
		}
	}

	// add this agent to the list
	agents_config_list_p.push_back(agent_params);

	return true;
}


// ---------------------------------------------------------------------
// TestFlagger::initFlagDataHandler
// Initialize the FlagDataHandler
// ---------------------------------------------------------------------
bool
TestFlagger::initFlagDataHandler()
{

	bool ret_status = true;

	LogIO os(LogOrigin("TestFlagger", "initFlagDataHandler()", WHERE));

	if (msname_p.empty()) {
		os << LogIO::SEVERE << "No Measurement Set has been parsed"
				<< LogIO::POST;
		return False;
	}

	if(fdh_p) delete fdh_p;

	// create a FlagDataHandler object
	fdh_p = new FlagDataHandler(msname_p, iterationApproach_p, timeInterval_p);

	// Open the MS
	fdh_p->open();

	if (dataselection_p.empty()) {
		return false;
	}

	// Set the data selection
	ret_status = fdh_p->setDataSelection(dataselection_p);
	if (!ret_status) {
		os << LogIO::SEVERE << "Failed to set the data selection."
				<< LogIO::POST;
		return false;
	}

	// Select the data
	ret_status = fdh_p->selectData();
	if (!ret_status) {
		os << LogIO::SEVERE << "Failed to select the data."
				<< LogIO::POST;
		return false;
	}

	// Generate the iterators
	fdh_p->generateIterator();

	return true;
}


// ---------------------------------------------------------------------
// TestFlagger::initAgents
// Initialize the Agents
// It assumes that parseAgentParameters and initFlagDataHander have
// been called first
// ---------------------------------------------------------------------
bool
TestFlagger::initAgents()
{

	LogIO os(LogOrigin("TestFlagger", "initAgents()", WHERE));

	if (agents_config_list_p.empty()){
		return false;
	}

	// loop through the vector of agents
	for (size_t i=0; i < agents_config_list_p.size(); i++) {

		// get agent record
		Record agent_rec = agents_config_list_p[i];

		// TODO: should I check for fdh_p existence here?
		// Should it call initFlagDataHandler in case it doesn't exist?
		// call the factory method for each of the agent's records
		if(not fdh_p){
			os << LogIO::SEVERE << "FlagDataHandler has not been initialized."
					<< LogIO::POST;
			return false;
		}

		// TODO: Catch error, print a warning and continue to next agent.
		FlagAgentBase *fa = FlagAgentBase::create(fdh_p, agent_rec);

		// add to list of FlagAgentList
		agents_list_p.push_back(fa);

	}

	return true;
}


// ---------------------------------------------------------------------
// TestFlagger::run
// Run the agents
// It assumes that initAgents has been called first
// ---------------------------------------------------------------------
Record
TestFlagger::run()
{

	LogIO os(LogOrigin("TestFlagger", "run()", WHERE));

	if (agents_list_p.empty()) {
		return Record();
	}

	agents_list_p.start();

	// iterate over chunks
	while (fdh_p->nextChunk())
	{
		// iterate over visBuffers
		while (fdh_p->nextBuffer())
		{

			// Queue flagging process
			// cout << "Put flag process in queue " << endl;
			agents_list_p.queueProcess();

			// Wait for completion of flagging process
			// cout << "Wait for completion of flagging process " << endl;
			agents_list_p.completeProcess();

			// Flush flags to MS
			// cout << "Flush flags to MS " << endl;
			fdh_p->flushFlags();
		}
	}

	agents_list_p.terminate();
	agents_list_p.join();

	// Get the record with the summary if there was any summary agent in the list
	Record summary_stats = Record();
	if (summaryAgent_p != NULL){
		summary_stats = summaryAgent_p->getResult();
	}

	return summary_stats;
}




} //#end casa namespace

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
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/BasicSL/Complex.h>
#include <measures/Measures/Stokes.h>
#include <casa/Utilities/Regex.h>
#include <casa/OS/HostInfo.h>
#include <flagging/Flagging/TestFlagger.h>
#include <flagging/FlagDataHandler.h>
#include <flagging/Flagging/FlagAgentBase.h>
#include <flagging/Flagging/RFAFlagExaminer.h>
#include <flagging/Flagging/RFAMedianClip.h>
#include <flagging/Flagging/RFASpectralRej.h>
#include <flagging/Flagging/RFASelector.h>
#include <flagging/Flagging/RFAUVBinner.h>
#include <flagging/Flagging/RFATimeFreqCrop.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/System/ProgressMeter.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/SetupNewTab.h>

#include <tables/Tables/ExprNode.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <measures/Measures/Stokes.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <casa/Quanta/MVEpoch.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>

#include <flagging/Flagging/RFANewMedianClip.h>

#include <algorithm>

#include <sstream>
#include <iostream>
#include <vector>

namespace casa {

const bool TestFlagger::dbg = false;

LogIO TestFlagger::os( LogOrigin("TestFlagger") );


// -----------------------------------------------------------------------
// Default Constructor
// -----------------------------------------------------------------------
TestFlagger::TestFlagger ()
{

	msname_p = "";
	asyncio_p = "";
	parallel_p = "";
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
	dataselection_p = NULL;
	agents_config_list = NULL;
	agents_list = NULL;

}


TestFlagger::~TestFlagger ()
{
	done();

}

void
TestFlagger::done(void)
{
	if(fdh_p){
		delete fdh_p;
		fdh_p = NULL;
	}

	msname_p = "";
	asyncio_p = "";
	parallel_p = "";
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
	dataselection_p = NULL;
	agents_config_list_p = NULL;
	agents_list_p = NULL;
}

// ---------------------------------------------------------------------
// TestFlagger::configTestFlagger
// Parse some parameters to the FlagDataHandler
// ---------------------------------------------------------------------
void
TestFlagger::configTestFlagger(Record &config)
{
	// msname, async, parallel
	LogIO os(LogOrigin("TestFlagger", "configTestFlagger()", WHERE));

	if (!config or config.empty()){
		return false;
	}

	// get the parameters to parse
	// TODO: Check for the existence later....
	config.get("MSNAME", msname_p);
	config.get("ASYNC", asyncio_p);
	config.get("PARALLEL", parallel_p);

	return true;
}

// ---------------------------------------------------------------------
// TestFlagger::parseDataSelection
// Parse union of parameters to the FlagDataHandler
// ---------------------------------------------------------------------
bool
TestFlagger::parseDataSelection(Record &selrec)
{

	LogIO os(LogOrigin("TestFlagger", "parseDataSelection()", WHERE));

	if(!selrec or selrec.empty()) {
		return false;
	}

	dataselection_p = selrec;

	// FIXME: maybe this is not needed.
	record.get("spw", spw_p);
	record.get("scan", scan_p);
	record.get("field", field_p);
	record.get("antenna", antenna_p);
	record.get("timerange", timerange_p);
	record.get("correlation", correlation_p);
	record.get("intent", intent_p);
	record.get("feed", feed_p);
	record.get("array", array_p);
	record.get("uvrange", uvrange_p);

	return true;
}

// ---------------------------------------------------------------------
// TestFlagger::parseAgentParameters
// Create a vector of agents and parameters
// ---------------------------------------------------------------------
bool
TestFlagger::parseAgentParameters(Record& agent_params)
{
	LogIO os(LogOrigin("TestFlagger", "parseAgentParameters()", WHERE));

	if(!agent_params or agent_params.empty()){
		return false;
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
	fdh_p = new FlagDataHandler(msname_p);

	// Open the MS
	fdh_p.open();

	if (!dataselection_p) {
		return false;
	}

	// Set the data selection
	ret_status = fdh_p.setDataSelection(dataselection_p);
	if (!ret_status) {
		os << LogIO::SEVERE << "Failed to set the data selection."
				<< LogIO::POST;
		return false;
	}

	// Select the data
	ret_status = fdh_p.selectData();
	if (!ret_status) {
		os << LogIO::SEVERE << "Failed to select the data."
				<< LogIO::POST;
		return false;
	}

	// Generate the iterators
	fdh_p.generateIterator();

	return true;
}


// ---------------------------------------------------------------------
// TestFlagger::initAgents
// Initialize the Agents
// ---------------------------------------------------------------------
bool
TestFlagger::initAgents()
{

	LogIO os(LogOrigin("TestFlagger", "initAgents()", WHERE));

	if (agents_config_list_p.empty()){
		return false;
	}

	// loop through the vector of agents
	for (int i=0; i < agents_config_list_p.size(); i++) {

		// get agent record
		Record agent_rec = agents_config_list_p[i];

		// call the factory method for each one of them
		FlagAgentBase *fa = FlagAgentBase::create(agent_rec);

		// add to list of FlagAgentBase
		agents_list_p.push_back(fa);

	}


}


// ---------------------------------------------------------------------
// TestFlagger::run
// Run the agents
// ---------------------------------------------------------------------
bool
TestFlagger::run()
{

	LogIO os(LogOrigin("TestFlagger", "run()", WHERE));

	if (agents_list_p.empty()) {
		return false;
	}

	//loop through the agents list and run them
	for (int i = 0; i < agents_list_p.size(); i++) {

		FlagAgentBase *agent = agents_list_p[i];

	}
	String mode = agent_rec.get("mode",mode);

	switch(mode)
	{
	case "manualflag":
		//
		break;
	case "clip":
		//
		break;
	case "shadow":
		//
		break;
	case "quack":
		//
		break;
	case "rfi":
		//
		break;
	case "elevation":
		//
		break;
	case "autoflag":
		//
		break;
	default:
		// manualflag
		break;
	}

}

}



} //#end casa namespace

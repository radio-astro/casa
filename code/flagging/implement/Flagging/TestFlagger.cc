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

	// get the parameters to parse
	// TODO: Check for the existence later....
	config.get("MSNAME", msname_p);
	config.get("ASYNC", asyncio_p);
	config.get("PARALLEL", parallel_p);


}

// ---------------------------------------------------------------------
// TestFlagger::configDataSelection
// Parse union of parameters to the FlagDataHandler
// ---------------------------------------------------------------------
void
TestFlagger::configDataSelection(Record &record)
{

	LogIO os(LogOrigin("TestFlagger", "configDataSelection()", WHERE));

	dataselection_p = record;

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


}

// ---------------------------------------------------------------------
// TestFlagger::configAgentParameters
// Create a vector of agents
// ---------------------------------------------------------------------
bool
TestFlagger::configAgentParameters(Record& agent_rec)
{
	LogIO os(LogOrigin("TestFlagger", "configAgentParameters()", WHERE));

	// add this agent to the list
	// implement a push_back function to add the agent
	// agents_push_back(agent_rec)
	// the list is actually a Vector<Record> agents_config_list_p

}


// ---------------------------------------------------------------------
// TestFlagger::initFlagDataHandler
// Initialize the FlagDataHandler
// ---------------------------------------------------------------------
bool
TestFlagger::initFlagDataHandler()
{
	LogIO os(LogOrigin("TestFlagger", "initFlagDataHandler()", WHERE));

	if(fdh_p) delete fdh_p;

	// create a FlagDataHandler object
	fdh_p = new FlagDataHandler(msname_p);

	// Open the MS
	fdh_p.open();

	// Set the data selection
	fdh_p.setDataSelection(dataselection_p);

	// Select the data and generate iterators
	fdh_p.selectData();
}


// ---------------------------------------------------------------------
// TestFlagger::initAgents
// Initialize the Agents
// ---------------------------------------------------------------------
bool
TestFlagger::initAgents()
{

	// call the factory method
}


// ---------------------------------------------------------------------
// TestFlagger::run
// Run the agents
// ---------------------------------------------------------------------
bool
TestFlagger::run()
{

	//loop through the agents and run them
}



} //#end casa namespace

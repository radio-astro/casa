//# tTestFlagger.cc This file contains the unit tests of the TestFlagger class.
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

#include <flagging/Flagging/TestFlagger.h>
#include <iostream>

using namespace casa;

int main(int argc, char **argv)
{
	String msname = "";
	Bool async = False;
	Bool parallel = False;
	String spw = "";
	String scan = "";
	String field = "";
	String antenna = "";
	String timerange = "";
	String correlation = "";
	String intent = "";
	String feed = "";
	String array = "";
	String uvrange = "";
	String observation = "";

//	if (argc == 2) {
//		msname = argv[1];
//	}

	if (argc < 4) {
		cout << "usage: tTestFlagger.cc msname async parallel" << endl;
		cout << "options: async (True/False)\nparallel(True/False)" << endl;
		exit (0);
	}

	String msname = argv[1];
	Bool async = argv[2];
	Bool parallel = argv[3];
	TestFlagger *tf = new TestFlagger();

	// Create a record with msname, async and parallel parameters
	Record record;
	record.define("msname", msname);
	record.define("async", async);
	record.define("parallel", parallel);
	if(not tf->configTestFlagger(record)) {
		cout << "ERROR: Failed to run configTestFlagger" << endl;
	}

	// Parse default data selection to the FlagDataHandler
	record.define("spw", spw);
	record.define("scan", scan);
	record.define("field", field);
	record.define("antenna", antenna);
	record.define("timerange", timerange);
	record.define("correlation", correlation);
	record.define("intent", intent);
	record.define("feed", feed);
	record.define("array", array);
	record.define("uvrange", uvrange);
	record.define("observation", observation);
	if(not tf->parseDataSelection(record)) {
		cout << "ERROR: Failed to parse default data selection" << endl;
	}

	// Create a data selection record
/*	String spw = "0~4:100~1000,5,6,7:1~100";
	String scan = "0~200";
	String field = "";
	String antenna = "1~20";
	String timerange = "";
	String correlation = "RR";
	String intent = "*AMPLI*";
	String feed = "";
	String array = "";
	String uvrange = "";
	String observation = "0";

	record.define("spw", spw);
	record.define("scan", scan);
	record.define("field", field);
	record.define("antenna", antenna);
	record.define("timerange", timerange);
	record.define("correlation", correlation);
	record.define("intent", intent);
	record.define("feed", feed);
	record.define("array", array);
	record.define("uvrange", uvrange);
	record.define("observation", observation);
	if( not tf->parseDataSelection(record)) {
		cout << "ERROR: Failed to parse data selection" << endl;
	}*/

	// Parse agent parameters
	Record rec_agent;
	rec_agent.define("mode","manualflag");
	rec_agent.define("unflag", "True");
	if (not tf->parseAgentParameters(rec_agent)) {
		cout << "ERROR: Failed to parse agent parameters" << endl;
	}

	// Initialize the FlagDataHandler
	if (not tf->initFlagDataHandler()) {
		cout << "ERROR: Failed to initialize the FlagDataHandler" << endl;
	}

	// Initialize the agents
	if (not tf->initAgents()) {
		cout << "ERROR: Failed to initialize the agents" << endl;
	}

	// Run the tool
	tf->run();

	tf->done();
	delete tf;
	return (0);
}

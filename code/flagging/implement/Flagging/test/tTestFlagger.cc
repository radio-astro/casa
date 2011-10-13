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
	String mode = "manualflag";
	String parameter, value;
	unsigned short logLevel = 0;

//	if (argc == 2) {
//		msname = argv[1];
//	}

	if (argc < 2) {
		cout << "usage: tTestFlagger.cc msname -async -parallel <-...>" << endl;
		cout << "options:"<<endl;
		cout <<	"async (True/False)\nparallel(True/False)" << endl;
		cout << "observation, array, scan, field, antenna, feed, intent, spw, timerange, "
				"correlation, uvrange" << endl;
		cout << "mode (manualflag,clip,quack,tfcrop,shadow,elevation,unflag,summary)" << endl;

		exit (0);
	}

	cout << "Initializing test" << endl;

	// Retrieve log level
	for (unsigned char i=0;i<argc-1;i++)
	{
		parameter = String(argv[i]);
		value = String(argv[i+1]);
		if (parameter == String("-loglevel"))
		{
			logLevel = (unsigned short)atoi(value.c_str());
		}
	}

	if (logLevel >= 3) cout << "Reading command line parameters " << endl;

	// Create a record to configure the TestFlagger tool
	Record config;
	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = String(argv[i]);
		value = String(argv[i+1]);

		if (parameter == String("-msname"))
		{
			msname = value;
			config.define("msname", casa::String(value));
			if (logLevel >= 3) cout << "MS is: " << msname << endl;
		}
/*		else if (parameter == String("parallel"))
		{
			parallel = (Bool)atoi(value.c_str());
			config.define ("parallel", casa::String(value));
			if (logLevel >= 3) cout << "Parallel is: " << parallel << endl;
		}
		else if (parameter == String("async"))
		{
			async = (Bool)atoi(value.c_str());
			config.define ("async", casa::String(value));
			if (logLevel >= 3) cout << "Async is: " << async << endl;
		}*/
	}

	TestFlagger *tf = new TestFlagger();
	if(not tf->configTestFlagger(config)) {
		cout << "ERROR: Failed to run configTestFlagger" << endl;
	}


	// Create a record with the data selection parameters
	Record record;
	// Fill up with the defaults
	record.define("array", array);
	record.define("scan", scan);
	record.define("observation", observation);
	record.define("field", field);
	record.define("feed", feed);
	record.define("spw", spw);
	record.define("timerange", timerange);
	record.define("uvrange", uvrange);
	record.define("correlation", correlation);
	record.define("antenna", antenna);
	record.define("intent", intent);

	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = String(argv[i]);
		value = String(argv[i+1]);

		if (parameter == String("-array"))
		{
			array = value;
			record.define ("array", casa::String(value));
			if (logLevel >= 3) cout << "Array selection is: " << array << endl;
		}
		else if (parameter == String("-scan"))
		{
			scan = value;
			record.define ("scan", casa::String(value));
			if (logLevel >= 3) cout << "Scan selection is: " << scan << endl;
		}
		else if (parameter == String("-timerange"))
		{
			timerange = value;
			record.define ("timerange", casa::String(value));
			if (logLevel >= 3) cout << "Time range selection is: " << timerange << endl;
		}
		else if (parameter == String("-field"))
		{
			field = value;
			record.define ("field", casa::String(value));
			if (logLevel >= 3) cout << "Field selection is: " << field << endl;
		}
		else if (parameter == String("-spw"))
		{
			spw = value;
			record.define ("spw", casa::String(value));
			if (logLevel >= 3) cout << "SPW selection is: " << spw << endl;
		}
		else if (parameter == String("-antenna"))
		{
			antenna = value;
			record.define ("antenna", casa::String(value));
			if (logLevel >= 3) cout << "Antenna selection is: " << antenna << endl;
		}
		else if (parameter == String("-uvrange"))
		{
			uvrange = value;
			record.define ("uvrange", casa::String(value));
			if (logLevel >= 3) cout << "UV range selection is: " << uvrange << endl;
		}
		else if (parameter == String("-correlation"))
		{
			correlation = value;
			record.define ("correlation", casa::String(value));
			if (logLevel >= 3) cout << "Correlation selection is: " << correlation << endl;
		}
		else if (parameter == String("-observation"))
		{
			observation = value;
			record.define ("observation", casa::String(value));
			if (logLevel >= 3) cout << "Observation selection is: " << observation << endl;
		}
		else if (parameter == String("-intent"))
		{
			intent = value;
			record.define ("intent", casa::String(value));
			if (logLevel >= 3) cout << "Scan intention selection is: " << intent << endl;
		}
		else if (parameter == String("-feed"))
		{
			intent = value;
			record.define ("feed", casa::String(value));
			if (logLevel >= 3) cout << "Feed selection is: " << feed << endl;
		}
	}

	// Parse the data selection parameters
	if(not tf->parseDataSelection(record)) {
		cout << "ERROR: Failed to parse the data selection" << endl;
	}


	// Create a record with the agent's parameters
	Record arecord;
	// default mode
	arecord.define("mode", "manualflag");

	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = String(argv[i]);
		value = String(argv[i+1]);

		if (parameter == String("-mode"))
		{
			mode = value;
			arecord.define ("mode", casa::String(value));
			if (logLevel >= 3) cout << "Flag mode is: " << mode << endl;
		}
	}

	// Parse agent parameters
	if (not tf->parseAgentParameters(arecord)) {
		cout << "ERROR: Failed to parse agent parameters" << endl;
	}

	if (logLevel >= 3) cout << "Done with parsing agent parameters: "<< endl;

	// Initialize the FlagDataHandler
	if (not tf->initFlagDataHandler()) {
		cout << "ERROR: Failed to initialize the FlagDataHandler" << endl;
	}

	if (logLevel >= 3) cout << "Done with initializing the FlagDataHandler: "<< endl;

	// Initialize the agents
	if (not tf->initAgents()) {
		cout << "ERROR: Failed to initialize the agents" << endl;
	}

	if (logLevel >= 3) cout << "Done with initializing the agents: "<< endl;

	// Run the tool
	tf->run();

	if (logLevel >= 3) cout << "Done with running the tool: "<< endl;

	tf->done();
	delete tf;
	return (0);
}

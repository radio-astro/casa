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
#include <sstream>


using namespace casa;

String
get_agent_parameters(int argc, char **argv, Record *agent_record)
{
	String parameter, value;
	String mode = "unknown";
	Float clipmin,clipmax;
	Bool clipmin_set = false;
	Bool clipmax_set = false;


	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = String(argv[i]);
		value = String(argv[i+1]);

		if (parameter == String("-mode"))
		{
			mode = value;
		}
	}

	if (mode == ""){
		mode == "manualflag";
		agent_record->define("mode", "manualflag");
	}
	else if (mode == "manualflag"){
		agent_record->define("mode", "manualflag");
	}
	else if (mode == "unflag"){
		agent_record->define("mode", "unflag");
	}
	else if (mode == "clip"){
		agent_record->define("mode", "clip");

		for (unsigned short i=0;i<argc-1;i++)
		{
			parameter = String(argv[i]);
			value = String(argv[i+1]);
			if (parameter == String("-clipcolumn"))
			{
				agent_record->define("datacolumn", value);
			}
			else if (parameter == String("-clipexpr"))
			{
				agent_record->define("expression", value);
			}
			else if (parameter == String("-clipmin"))
			{
				clipmin = casa::Float(atof(value.c_str()));
				clipmin_set = true;
			}
			else if (parameter == String("-clipmax"))
			{
				clipmax = casa::Float(atof(value.c_str()));
				clipmax_set = true;
			}
			else if (parameter == String("-clipoutside"))
			{
				agent_record->define("clipoutside", value);
			}
			else if (parameter == String("-channelavg"))
			{
				agent_record->define("channelavg", value);
			}

		}
	}
	else if (mode == "summary"){
		agent_record->define("mode", "summary");

		for (unsigned short i=0;i<argc-1;i++)
		{
			parameter = String(argv[i]);
			value = String(argv[i+1]);
			if (parameter == String("-minrel"))
			{
				agent_record->define("minrel", value);
			}
			else if (parameter == String("-maxrel"))
			{
				agent_record->define("maxrel", value);
			}
			else if (parameter == String("-minabs"))
			{
				agent_record->define("minabs", value);
			}
			else if (parameter == String("-maxabs"))
			{
				agent_record->define("maxabs", value);
			}
		}
	}
	else if (mode == "tfcrop"){
		agent_record->define("mode", "tfcrop");

		for (unsigned short i=0;i<argc-1;i++)
		{
			parameter = String(argv[i]);
			value = String(argv[i+1]);
			if (parameter == String("-time_amp_cutoff"))
			{
				agent_record->define("time_amp_cutoff", value);
			}
			else if (parameter == String("-freq_amp_cutoff"))
			{
				agent_record->define("freq_amp_cutoff", value);
			}
			else if (parameter == String("-maxnpieces"))
			{
				agent_record->define("maxnpieces", value);
			}
			else if (parameter == String("-timefit"))
			{
				agent_record->define("timefit", value);
			}
			else if (parameter == String("-freqfit"))
			{
				agent_record->define("freqfit", value);
			}
			else if (parameter == String("-flagdimension"))
			{
				agent_record->define("flagdimension", value);
			}
			else if (parameter == String("-halfwin"))
			{
				agent_record->define("halfwin", value);
			}
			else if (parameter == String("-usewindowstats"))
			{
				agent_record->define("usewindowstats", value);
			}
		}
	}
	else if (mode == "quack"){
		agent_record->define("mode", "quack");

		for (unsigned short i=0;i<argc-1;i++)
		{
			parameter = String(argv[i]);
			value = String(argv[i+1]);
			if (parameter == String("-quackinterval"))
			{
				agent_record->define("quackinterval", value);
			}
			else if (parameter == String("-quackmode"))
			{
				agent_record->define("quackmode", value);
			}
			else if (parameter == String("-quackincrement"))
			{
				agent_record->define("quackincrement", value);
			}
		}
	}
	else if (mode == "elevation"){
		agent_record->define("mode", "elevation");
		Double lowerlimit,upperlimit;

		for (unsigned short i=0;i<argc-1;i++)
		{
			parameter = String(argv[i]);
			value = String(argv[i+1]);
			if (parameter == String("-lowerlimit"))
			{
				lowerlimit = atof(value.c_str());
				agent_record->define("lowerlimit", lowerlimit);
			}
			else if (parameter == String("-upperlimit"))
			{
				upperlimit = atof(value.c_str());
				agent_record->define("upperlimit", upperlimit);
			}
		}
	}

	if (clipmin_set && clipmax_set)
	{
		casa::IPosition size(1);
		size[0]=2;
		casa::Array<Float> cliprange(size);
		cliprange[0] = clipmin;
		cliprange[1] = clipmax;
		agent_record->define("clipminmax",cliprange);
	}


	return mode;
}


int main(int argc, char **argv)
{
	String msname = "";
	Double ntime = 0.0;
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
	String parameter, value;
	unsigned short logLevel = 0;

	Record results = Record();

//	if (argc == 2) {
//		msname = argv[1];
//	}

	if (argc < 2) {
		cout << "usage: tTestFlagger.cc -msname ngc5921.ms <-...>" << endl;
		cout << "options:"<<endl;
		cout << "ntime, observation, array, scan, field, antenna, feed, intent, spw, timerange, "
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

	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = String(argv[i]);
		value = String(argv[i+1]);

		if (parameter == String("-msname"))
		{
			msname = value;
			if (logLevel >= 3) cout << "MS is: " << msname << endl;
		}
		else if (parameter == String("-ntime"))
		{
			ntime = (Double)atof(value.c_str());
			if (logLevel >= 3) cout << "Time inteval is: " << ntime << endl;
		}
	}

	TestFlagger *tf = new TestFlagger();
	if(not tf->open(msname, ntime)) {
		cout << "ERROR: Failed to open the tool" << endl;
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
/*		else if (parameter == String("-correlation"))
		{
			correlation = value;
			record.define ("correlation", casa::String(value));
			if (logLevel >= 3) cout << "Correlation selection is: " << correlation << endl;
		}*/
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
	if(not tf->selectData(record)) {
		cout << "ERROR: Failed to select the data" << endl;
	}


	// Create a record with the agent's parameters
	Record arecord;
	String mode = get_agent_parameters(argc, argv, &arecord);
	if (mode == "unknown")
	{
		cout << "ERROR: Unknown mode requested " << endl;

	}

	cout << "Will parse parameters for mode="<< mode << endl;
	if (logLevel >= 3)
	{
		cout << "Parameters of mode " << mode << " are" << endl;
		ostringstream os;
		arecord.print(os);
		String str(os.str());
		cout << str << endl;
	}


//	if (mode == "clip"){
		// get clip parameters
//	}
	// TODO: do other modes
	// Parse agent data selection parameters
	if (not tf->parseAgentParameters(arecord)) {
		cout << "ERROR: Failed to parse agent parameters" << endl;
	}

	if (logLevel >= 3) cout << "Done with parsing agent parameters: "<< endl;


	// Initialize the agents
	if (not tf->initAgents()) {
		cout << "ERROR: Failed to initialize the agents" << endl;
	}

	if (logLevel >= 3) cout << "Done with initializing the agents "<< endl;

	// Run the tool
	results = tf->run();
/*	if(mode == "summary"){
		ostringstream os;
		results.print(os);
		String str(os.str());
		cout << str << endl;
	}*/

	if (logLevel >= 3) cout << "Done with running the tool: "<< endl;

	tf->done();

	return (0);
}

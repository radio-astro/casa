//# tFlagDataHandler.cc: This file contains the unit tests of the FlagDataHandler class.
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

#include <flagging/Flagging/FlagCalTableHandler.h>
#include <flagging/Flagging/FlagMSHandler.h>
#include <flagging/Flagging/FlagAgentBase.cc>
#include <tableplot/TablePlot/FlagVersion.h>
#include <iostream>

using namespace casa;

void unflag(string inputFile,uShort iterationMode)
{
	bool fillBuffer = true;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;

	timeval start,stop;
	double elapsedTime = 0;

	Table table(inputFile,TableLock(TableLock::AutoNoReadLocking));
	TableInfo& info = table.tableInfo();
	String type=info.type();
	table.flush();
	table.relinquishAutoLocks(True);
	table.unlock();
	FlagDataHandler *dh = NULL;
	if (type == "Measurement Set")
	{
		dh = new FlagMSHandler(inputFile,iterationMode);
	}
	else
	{
		dh = new FlagCalTableHandler(inputFile,iterationMode);
	}

	// Open Measurement Set
	dh->open();

	// Set data selection
	Record dhConfig;
	dh->setDataSelection(dhConfig);

	// Select data
	dh->selectData();

	// Create agent list
	Record agentConfig;
	agentConfig.define("name","FlagAgentUnflag");
	FlagAgentList agentList;
	FlagAgentManual *unflagAgent = new FlagAgentManual(dh,agentConfig,false,false);
	agentList.push_back(unflagAgent);

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// Iterates over chunks (constant values)
	while (dh->nextChunk())
	{
		// Iterates over buffers (time steps)
		while (dh->nextBuffer())
		{
			agentList.apply();
			dh->flushFlags();
		}

		// Print stats from each agent
		agentList.chunkSummary();
	}

	// Print total stats from each agent
	agentList.tableSummary();

	// Stop Flag Agent
	agentList.terminate();
	agentList.join();

	// Close MS
	dh->close();

	// Clear Flag Agent List
	agentList.clear();

	// Free data handler
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Reading Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;
}

void flag(string inputFile,uShort iterationMode,uShort testMode,String flagmode,Record record)
{
	bool fillBuffer = true;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;

	timeval start,stop;
	double elapsedTime = 0;

	Table table(inputFile,TableLock(TableLock::AutoNoReadLocking));
	TableInfo& info = table.tableInfo();
	String type=info.type();
	table.flush();
	table.relinquishAutoLocks(True);
	table.unlock();
	FlagDataHandler *dh = NULL;
	if (type == "Measurement Set")
	{
		dh = new FlagMSHandler(inputFile,iterationMode);
	}
	else
	{
		dh = new FlagCalTableHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY);
	}

	// Open Measurement Set
	dh->open();

	// Set data selection
	Record dhConfig;
	if (testMode==1) dhConfig = record;
	dh->setDataSelection(dhConfig);

	// Select data
	dh->selectData();

	// Create agent list
	Record agentConfig;
	if (testMode==2) agentConfig = record;
	agentConfig.define("name","FlagAgent-" + flagmode);
	agentConfig.define("mode",flagmode);

	if (flagmode == String("clip"))
	{
		casa::IPosition size(1);
		size[0]=2;
		casa::Array<Double> cliprange(size);
		cliprange[0] = 0.0001;
		cliprange[1] = 7.5;
		agentConfig.define("clipminmax",cliprange);
	}
	else if (flagmode == String("rflag"))
	{
		agentConfig.define("timedev",(Double)7.5);
		agentConfig.define("freqdev",(Double)0.5);
	}
	else if (flagmode == String("quack"))
	{
		agentConfig.define("quackinterval",(Double)20);
	}

	FlagAgentList agentList;
	FlagAgentBase *agent = FlagAgentBase::create(dh,agentConfig);
	agentList.push_back(agent);

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// Iterates over chunks (constant values)
	while (dh->nextChunk())
	{
		// Iterates over buffers (time steps)
		while (dh->nextBuffer())
		{
			agentList.apply();
			dh->flushFlags();
		}

		// Print stats from each agent
		agentList.chunkSummary();
	}

	// Print total stats from each agent
	agentList.tableSummary();

	// Stop Flag Agent
	agentList.terminate();
	agentList.join();

	// Test iterator re-generation
	dh->generateIterator();

	// Close MS
	dh->close();

	// Clear Flag Agent List
	agentList.clear();

	// Free data handler
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Reading Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;
}

void summary(string inputFile,uShort iterationMode)
{
	bool fillBuffer = true;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;

	timeval start,stop;
	double elapsedTime = 0;

	Table table(inputFile,TableLock(TableLock::AutoNoReadLocking));
	TableInfo& info = table.tableInfo();
	String type=info.type();
	table.flush();
	table.relinquishAutoLocks(True);
	table.unlock();
	FlagDataHandler *dh = NULL;
	if (type == "Measurement Set")
	{
		dh = new FlagMSHandler(inputFile,iterationMode);
	}
	else
	{
		dh = new FlagCalTableHandler(inputFile,iterationMode);
	}

	// Open Measurement Set
	dh->open();

	// Set data selection
	Record dhConfig;
	dh->setDataSelection(dhConfig);

	// Select data
	dh->selectData();

	// Create agent list
	Record agentConfig;
	agentConfig.define("name","FlagAgentSummary");
	FlagAgentList agentList;
	FlagAgentSummary *summaryAgent = new FlagAgentSummary(dh,agentConfig);
	agentList.push_back(summaryAgent);

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// Iterates over chunks (constant values)
	while (dh->nextChunk())
	{
		// Iterates over buffers (time steps)
		while (dh->nextBuffer())
		{
			agentList.apply();
			dh->flushFlags();
		}

		// Print stats from each agent
		agentList.chunkSummary();
	}

	// Print total stats from each agent
	agentList.tableSummary();

	// Stop Flag Agent
	agentList.terminate();
	agentList.join();

	// Get statistics
	Record result = summaryAgent->getResult();

	// Close MS
	dh->close();

	// Clear Flag Agent List
	agentList.clear();

	// Free data handler
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Reading Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;
}

void display(string inputFile,uShort iterationMode,uShort testMode,Record record)
{
	bool fillBuffer = true;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;

	timeval start,stop;
	double elapsedTime = 0;

	Table table(inputFile,TableLock(TableLock::AutoNoReadLocking));
	TableInfo& info = table.tableInfo();
	String type=info.type();
	table.flush();
	table.relinquishAutoLocks(True);
	table.unlock();
	FlagDataHandler *dh = NULL;
	if (type == "Measurement Set")
	{
		dh = new FlagMSHandler(inputFile,iterationMode);
	}
	else
	{
		dh = new FlagCalTableHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY);
	}

	// Open Measurement Set
	dh->open();

	// Set data selection
	Record dhConfig;
	if (testMode==1) dhConfig = record;
	dh->setDataSelection(dhConfig);

	// Select data
	dh->selectData();

	// Create agent list
	Record agentConfig;
	if (testMode==2) agentConfig = record;
	agentConfig.define("name","FlagAgent-display");
	agentConfig.define("mode","display");
	agentConfig.define("datadisplay",True);
	FlagAgentDisplay *dataDisplayAgent = new FlagAgentDisplay(dh,agentConfig);
	FlagAgentList agentList;
	agentList.push_back(dataDisplayAgent);

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// Iterates over chunks (constant values)
	while (dh->nextChunk())
	{
		// Iterates over buffers (time steps)
		while (dh->nextBuffer())
		{
			agentList.apply();
			dh->flushFlags();
		}

		// Print stats from each agent
		agentList.chunkSummary();
	}

	// Print total stats from each agent
	agentList.tableSummary();

	// Stop Flag Agent
	agentList.terminate();
	agentList.join();

	// Test iterator re-generation
	dh->generateIterator();

	// Close MS
	dh->close();

	// Clear Flag Agent List
	agentList.clear();

	// Free data handler
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Reading Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;
}

int main(int argc, char **argv)
{
	// Variables declaration
	string parameter, value;
	string inputFile, array, time, scan, field, spw, baseline, uvw;
	string correlation, observation, intent;
	uShort iterationMode = FlagDataHandler::SUB_INTEGRATION;
	uShort testMode = 1;
	String flagMode = "manual";
	Bool disp=False;
	Bool unflg=True;


	// Parse input parameters
	Record record;
	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-file"))
		{
			inputFile = value;
			cout << "File is: " << inputFile << endl;
		}
		else if (parameter == string("-array"))
		{
			array = value;
			record.define ("array", casa::String(value));
			cout << "Array selection is: " << array << endl;
		}
		else if (parameter == string("-scan"))
		{
			scan = value;
			record.define ("scan", casa::String(value));
			cout << "Scan selection is: " << scan << endl;
		}
		else if (parameter == string("-timerange"))
		{
			time = value;
			record.define ("timerange", casa::String(value));
			cout << "Time range selection is: " << time << endl;
		}
		else if (parameter == string("-field"))
		{
			field = value;
			record.define ("field", casa::String(value));
			cout << "Field selection is: " << field << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = value;
			record.define ("spw", casa::String(value));
			cout << "SPW selection is: " << spw << endl;
		}
		else if (parameter == string("-antenna"))
		{
			baseline = value;
			record.define ("antenna", casa::String(value));
			cout << "Antenna selection is: " << baseline << endl;
		}
		else if (parameter == string("-uvrange"))
		{
			uvw = value;
			record.define ("uvrange", casa::String(value));
			cout << "UV range selection is: " << uvw << endl;
		}
		else if (parameter == string("-correlation"))
		{
			correlation = value;
			record.define ("correlation", casa::String(value));
			cout << "Correlation selection is: " << correlation << endl;
		}
		else if (parameter == string("-observation"))
		{
			observation = value;
			record.define ("observation", casa::String(value));
			cout << "Observation selection is: " << observation << endl;
		}
		else if (parameter == string("-intent"))
		{
			intent = value;
			record.define ("intent", casa::String(value));
			cout << "Scan intention selection is: " << intent << endl;
		}
		else if (parameter == string("-iteration"))
		{
			iterationMode = (uShort)atoi(value.c_str());
			cout << "Iteration approach is: " << iterationMode << endl;
		}
		else if (parameter == string("-flagMode"))
		{
			flagMode = String(value.c_str());
			cout << "Flagmode approach is: " << flagMode << endl;
		}
		else if (parameter == string("-testMode"))
		{
			testMode = (uShort)atoi(value.c_str());
			cout << "Testmode approach is: " << testMode << endl;
		}
		else if (parameter == string("-display"))
		{
			disp = (Bool)atoi(value.c_str());
			cout << "Display mode activated: " << disp << endl;
		}
		else if (parameter == string("-unflag"))
		{
			unflg = (Bool)atoi(value.c_str());
			cout << "Unflag mode activated: " << unflg << endl;
		}
	}

	if (unflg) unflag(inputFile,iterationMode);
	flag(inputFile,iterationMode,testMode,flagMode,record);
	summary(inputFile,iterationMode);
	if (disp) display(inputFile,iterationMode,testMode,record);

	exit(-1);
}

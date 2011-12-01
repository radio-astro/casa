//# tFlagAgentManual.cc This file contains the unit tests of the FlagAgentBase class.
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

#include <flagging/Flagging/FlagAgentManual.h>
#include <flagging/Flagging/FlagAgentSummary.h>
#include <iostream>

using namespace casa;

void clearFlags(string inputFile,Bool flag)
{
	// Stats variables declaration
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);

	// Open MS
	dh->open();

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent list
	Record dummyConfig;
	dummyConfig.define("name","FlagAgentUnflag");
	FlagAgentList *agentList = new FlagAgentList();
	FlagAgentBase *agent_i = new FlagAgentManual(dh,dummyConfig,false,!flag);
	agentList->push_back(agent_i);

	// Start Flag Agent
	agentList->start();

	// Set cout precision
	cout.precision(20);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
			cout << "Chunk:" << dh->chunkNo << " " << "Buffer:" << dh->bufferNo << " ";
			nBuffers += 1;

			if (dh->visibilityBuffer_p->get()->observationId().nelements() > 1)
			{
				cout << "Observation:"
					 << dh->visibilityBuffer_p->get()->observationId()[0] << "~"
					 << dh->visibilityBuffer_p->get()->observationId()[dh->visibilityBuffer_p->get()->observationId().nelements()-1] << " ";
			}
			else
			{
				cout << "Observation:" << dh->visibilityBuffer_p->get()->observationId()[0] << " ";
			}

			cout << "Array:" << dh->visibilityBuffer_p->get()->arrayId() << " ";

			if (dh->visibilityBuffer_p->get()->scan().nelements() > 1)
			{
				cout << "Scan:"
					 << dh->visibilityBuffer_p->get()->scan()[0] << "~"
					 << dh->visibilityBuffer_p->get()->scan()[dh->visibilityBuffer_p->get()->scan().nelements()-1] << " ";
			}
			else
			{
				cout << "Scan:" << dh->visibilityBuffer_p->get()->scan()[0] << " ";
			}

			cout << "Field:" << dh->visibilityBuffer_p->get()->fieldId() << " " ;

			cout << "Spw:" << dh->visibilityBuffer_p->get()->spectralWindow() << " ";

			if (dh->visibilityBuffer_p->get()->time().nelements() > 1)
			{
				cout << "Time:"
					 << dh->visibilityBuffer_p->get()->time()[0] << "~"
					 << dh->visibilityBuffer_p->get()->time()[dh->visibilityBuffer_p->get()->time().nelements()-1] << " ";
			}
			else
			{
				cout << "Time:" << dh->visibilityBuffer_p->get()->time()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna1().nelements() > 1)
			{
				cout << "Antenna1:"
					 << dh->visibilityBuffer_p->get()->antenna1()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna1()[dh->visibilityBuffer_p->get()->antenna1().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna1:" << dh->visibilityBuffer_p->get()->antenna1()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna2().nelements() > 1)
			{
				cout << "Antenna2:"
					 << dh->visibilityBuffer_p->get()->antenna2()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna2()[dh->visibilityBuffer_p->get()->antenna2().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna2:" << dh->visibilityBuffer_p->get()->antenna2()[0] << " ";
			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// Queue flagging process
			agentList->queueProcess();

			// Wait for completion of flagging process
			agentList->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	agentList->terminate();
	agentList->join();

	// Close MS
	dh->close();

	// Delete Flag Agent
	delete agentList;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;
}

void writeFlags(string inputFile,vector<Record> recordList,Bool flag)
{
	// Stats variables declaration
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);

	// Open MS
	dh->open();

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent list
	FlagAgentBase *agent_i = NULL;
	FlagAgentList *agentList = new FlagAgentList();
	Int agentNumber = 1;
	for (vector<Record>::iterator iter = recordList.begin();iter != recordList.end(); iter++)
	{
		stringstream agentName;
		agentName << agentNumber;
		agent_i= new FlagAgentManual(dh,*iter,false,flag);
		agentList->push_back(agent_i);
		agentNumber++;
	}

	// Start Flag Agent
	agentList->start();

	// Set cout precision
	cout.precision(20);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
			cout << "Chunk:" << dh->chunkNo << " " << "Buffer:" << dh->bufferNo << " ";
			nBuffers += 1;

			if (dh->visibilityBuffer_p->get()->observationId().nelements() > 1)
			{
				cout << "Observation:"
					 << dh->visibilityBuffer_p->get()->observationId()[0] << "~"
					 << dh->visibilityBuffer_p->get()->observationId()[dh->visibilityBuffer_p->get()->observationId().nelements()-1] << " ";
			}
			else
			{
				cout << "Observation:" << dh->visibilityBuffer_p->get()->observationId()[0] << " ";
			}

			cout << "Array:" << dh->visibilityBuffer_p->get()->arrayId() << " ";

			if (dh->visibilityBuffer_p->get()->scan().nelements() > 1)
			{
				cout << "Scan:"
					 << dh->visibilityBuffer_p->get()->scan()[0] << "~"
					 << dh->visibilityBuffer_p->get()->scan()[dh->visibilityBuffer_p->get()->scan().nelements()-1] << " ";
			}
			else
			{
				cout << "Scan:" << dh->visibilityBuffer_p->get()->scan()[0] << " ";
			}

			cout << "Field:" << dh->visibilityBuffer_p->get()->fieldId() << " " ;

			cout << "Spw:" << dh->visibilityBuffer_p->get()->spectralWindow() << " ";

			if (dh->visibilityBuffer_p->get()->time().nelements() > 1)
			{
				cout << "Time:"
					 << dh->visibilityBuffer_p->get()->time()[0] << "~"
					 << dh->visibilityBuffer_p->get()->time()[dh->visibilityBuffer_p->get()->time().nelements()-1] << " ";
			}
			else
			{
				cout << "Time:" << dh->visibilityBuffer_p->get()->time()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna1().nelements() > 1)
			{
				cout << "Antenna1:"
					 << dh->visibilityBuffer_p->get()->antenna1()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna1()[dh->visibilityBuffer_p->get()->antenna1().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna1:" << dh->visibilityBuffer_p->get()->antenna1()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna2().nelements() > 1)
			{
				cout << "Antenna2:"
					 << dh->visibilityBuffer_p->get()->antenna2()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna2()[dh->visibilityBuffer_p->get()->antenna2().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna2:" << dh->visibilityBuffer_p->get()->antenna2()[0] << " ";
			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// Queue flagging process
			agentList->queueProcess();

			// Wait for completion of flagging process
			agentList->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	agentList->terminate();
	agentList->join();

	// Close MS
	dh->close();

	// Delete Flag Agent
	delete agentList;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

}

void checkFlags(string inputFile,Record dataSelection,Bool flag)
{
	// Stats variables declaration
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);
	dh->setDataSelection(dataSelection);

	// Open MS
	dh->open();

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent list
	Record dummyConfig;
	dummyConfig.define("name","FlagAgentCheck");
	FlagAgentList *agentList = new FlagAgentList();
	FlagAgentBase *agent_i = new FlagAgentManual(dh,dummyConfig,false,flag);
	agent_i->setCheckMode(true);
	agentList->push_back(agent_i);

	// Start Flag Agent
	agentList->start();

	// Set cout precision
	cout.precision(20);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
			cout << "Chunk:" << dh->chunkNo << " " << "Buffer:" << dh->bufferNo << " ";
			nBuffers += 1;

			if (dh->visibilityBuffer_p->get()->observationId().nelements() > 1)
			{
				cout << "Observation:"
					 << dh->visibilityBuffer_p->get()->observationId()[0] << "~"
					 << dh->visibilityBuffer_p->get()->observationId()[dh->visibilityBuffer_p->get()->observationId().nelements()-1] << " ";
			}
			else
			{
				cout << "Observation:" << dh->visibilityBuffer_p->get()->observationId()[0] << " ";
			}

			cout << "Array:" << dh->visibilityBuffer_p->get()->arrayId() << " ";

			if (dh->visibilityBuffer_p->get()->scan().nelements() > 1)
			{
				cout << "Scan:"
					 << dh->visibilityBuffer_p->get()->scan()[0] << "~"
					 << dh->visibilityBuffer_p->get()->scan()[dh->visibilityBuffer_p->get()->scan().nelements()-1] << " ";
			}
			else
			{
				cout << "Scan:" << dh->visibilityBuffer_p->get()->scan()[0] << " ";
			}

			cout << "Field:" << dh->visibilityBuffer_p->get()->fieldId() << " " ;

			cout << "Spw:" << dh->visibilityBuffer_p->get()->spectralWindow() << " ";

			if (dh->visibilityBuffer_p->get()->time().nelements() > 1)
			{
				cout << "Time:"
					 << dh->visibilityBuffer_p->get()->time()[0] << "~"
					 << dh->visibilityBuffer_p->get()->time()[dh->visibilityBuffer_p->get()->time().nelements()-1] << " ";
			}
			else
			{
				cout << "Time:" << dh->visibilityBuffer_p->get()->time()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna1().nelements() > 1)
			{
				cout << "Antenna1:"
					 << dh->visibilityBuffer_p->get()->antenna1()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna1()[dh->visibilityBuffer_p->get()->antenna1().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna1:" << dh->visibilityBuffer_p->get()->antenna1()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna2().nelements() > 1)
			{
				cout << "Antenna2:"
					 << dh->visibilityBuffer_p->get()->antenna2()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna2()[dh->visibilityBuffer_p->get()->antenna2().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna2:" << dh->visibilityBuffer_p->get()->antenna2()[0] << " ";
			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// Queue flagging process
			agentList->queueProcess();

			// Wait for completion of flagging process
			agentList->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	agentList->terminate();
	agentList->join();

	// Close MS
	dh->close();

	// Delete Flag Agent
	delete agentList;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

}

void summaryFlags(string inputFile)
{
	// Stats variables declaration
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);

	// Open MS
	dh->open();

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent list
	Record dummyConfig;
	FlagAgentList *agentList = new FlagAgentList();
	dummyConfig.define("name","FlagAgentSummary");
	FlagAgentSummary *summaryAgent = new FlagAgentSummary(dh,dummyConfig);
	agentList->push_back(summaryAgent);

	// Start Flag Agent
	agentList->start();

	// Set cout precision
	cout.precision(20);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
			cout << "Chunk:" << dh->chunkNo << " " << "Buffer:" << dh->bufferNo << " ";
			nBuffers += 1;

			if (dh->visibilityBuffer_p->get()->observationId().nelements() > 1)
			{
				cout << "Observation:"
					 << dh->visibilityBuffer_p->get()->observationId()[0] << "~"
					 << dh->visibilityBuffer_p->get()->observationId()[dh->visibilityBuffer_p->get()->observationId().nelements()-1] << " ";
			}
			else
			{
				cout << "Observation:" << dh->visibilityBuffer_p->get()->observationId()[0] << " ";
			}

			cout << "Array:" << dh->visibilityBuffer_p->get()->arrayId() << " ";

			if (dh->visibilityBuffer_p->get()->scan().nelements() > 1)
			{
				cout << "Scan:"
					 << dh->visibilityBuffer_p->get()->scan()[0] << "~"
					 << dh->visibilityBuffer_p->get()->scan()[dh->visibilityBuffer_p->get()->scan().nelements()-1] << " ";
			}
			else
			{
				cout << "Scan:" << dh->visibilityBuffer_p->get()->scan()[0] << " ";
			}

			cout << "Field:" << dh->visibilityBuffer_p->get()->fieldId() << " " ;

			cout << "Spw:" << dh->visibilityBuffer_p->get()->spectralWindow() << " ";

			if (dh->visibilityBuffer_p->get()->time().nelements() > 1)
			{
				cout << "Time:"
					 << dh->visibilityBuffer_p->get()->time()[0] << "~"
					 << dh->visibilityBuffer_p->get()->time()[dh->visibilityBuffer_p->get()->time().nelements()-1] << " ";
			}
			else
			{
				cout << "Time:" << dh->visibilityBuffer_p->get()->time()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna1().nelements() > 1)
			{
				cout << "Antenna1:"
					 << dh->visibilityBuffer_p->get()->antenna1()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna1()[dh->visibilityBuffer_p->get()->antenna1().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna1:" << dh->visibilityBuffer_p->get()->antenna1()[0] << " ";
			}

			if (dh->visibilityBuffer_p->get()->antenna2().nelements() > 1)
			{
				cout << "Antenna2:"
					 << dh->visibilityBuffer_p->get()->antenna2()[0] << "~"
					 << dh->visibilityBuffer_p->get()->antenna2()[dh->visibilityBuffer_p->get()->antenna2().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna2:" << dh->visibilityBuffer_p->get()->antenna2()[0] << " ";
			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// Queue flagging process
			agentList->queueProcess();

			// Wait for completion of flagging process
			agentList->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	agentList->terminate();
	agentList->join();

	// Get statistics
	summaryAgent->getResult();

	// Close MS
	dh->close();

	// Delete Flag Agent
	delete agentList;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

}


int main(int argc, char **argv)
{
	string inputFile;
	Bool flag = True;
	Int nThreads = 0;
	string array,scan,timerange,field,spw,antenna,uvrange,observation,intent;


	string parameter, value;
	Bool multipleAgents = false;
	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);
		if (parameter == string("-multipleAgents"))
		{
			multipleAgents = casa::Bool(value.c_str());
			cout << "multipleAgents is: " << multipleAgents << endl;
		}
	}


	Record dataSelection;
	Record agentParameters;
	vector<Record> recordList;
	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-inputFile"))
		{
			inputFile = value;
			cout << "Target file is: " << inputFile << endl;
		}
		else if (parameter == string("-array"))
		{
			array = casa::String(value);
			dataSelection.define ("array", array);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("array", array);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("array", array);
			}
			cout << "Array selection is: " << array << endl;
		}
		else if (parameter == string("-scan"))
		{
			scan = casa::String(value);
			dataSelection.define ("scan", scan);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("scan", scan);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("scan", scan);
			}
			cout << "Scan selection is: " << scan << endl;
		}
		else if (parameter == string("-timerange"))
		{
			timerange = casa::String(value);
			dataSelection.define ("timerange", timerange);
			cout << "Time range selection is: " << timerange << endl;
		}
		else if (parameter == string("-field"))
		{
			field = casa::String(value);
			dataSelection.define ("field", field);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("field", field);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("field", field);
			}
			cout << "Field selection is: " << field << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = casa::String(value);
			dataSelection.define ("spw", spw);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("spw", spw);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("spw", spw);
			}
			cout << "SPW selection is: " << spw << endl;
		}
		else if (parameter == string("-antenna"))
		{
			antenna = casa::String(value);
			dataSelection.define("antenna",antenna);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("antenna", antenna);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("antenna", antenna);
			}
			cout << "Antenna selection is: " << antenna << endl;
		}
		else if (parameter == string("-uvrange"))
		{
			uvrange = casa::String(value);
			dataSelection.define ("uvrange", uvrange);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("uvrange", uvrange);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("uvrange", uvrange);
			}
			cout << "UV range selection is: " << uvrange << endl;
		}
		else if (parameter == string("-observation"))
		{
			observation = casa::String(value);
			dataSelection.define ("observation", observation);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("observation", observation);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("observation", observation);
			}
			cout << "Observation selection is: " << observation << endl;
		}
		else if (parameter == string("-intent"))
		{
			intent = casa::String(value);
			dataSelection.define ("intent", intent);
			if (multipleAgents)
			{
				Record rec;
				rec.define ("intent", intent);
				recordList.push_back(rec);
			}
			else
			{
				agentParameters.define ("intent", intent);
			}
			cout << "Scan intention selection is: " << intent << endl;
		}
		else if (parameter == string("-flag"))
		{
			parameter = string(argv[i]);
			value = string(argv[i+1]);
			flag = casa::Bool(value.c_str());
			cout << "flag is: " << flag << endl;
		}
		else if (parameter == string("-nThreads"))
		{
			nThreads = atoi(value.c_str());
			agentParameters.define ("nThreads", nThreads);
			cout << "nThreads is: " << nThreads << endl;
		}
	}

	// MultiThreading and naming within each agent
	if (!multipleAgents)
	{
		if (nThreads>1)
		{
			stringstream nThreadsParam;
			nThreadsParam << nThreads;
			for (Int threadId=0;threadId<nThreads;threadId++)
			{
				Record agentParameters_i = agentParameters;
				stringstream threadIdParam;
				threadIdParam << threadId;
				agentParameters_i.define("threadId",threadIdParam.str());
				agentParameters_i.define("nThreads",nThreadsParam.str());
				agentParameters_i.define("name","FlagAgentManual_1_Thread_" + threadIdParam.str());
				recordList.push_back(agentParameters_i);
			}
		}
		else
		{
			agentParameters.define("name","FlagAgentManual");
			recordList.push_back(agentParameters);
		}
	}
	else
	{
		if (nThreads>1)
		{
			stringstream nThreadsParam;
			nThreadsParam << nThreads;
			Int agentNumber=1;
			vector<Record> recordListMulithreading;
			for (vector<Record>::iterator iter = recordList.begin();iter != recordList.end(); iter++)
			{
				stringstream agentNumberParam;
				agentNumberParam << agentNumber;

				for (Int threadId=0;threadId<nThreads;threadId++)
				{
					stringstream threadIdParam;
					threadIdParam << threadId;
					iter->define("threadId",threadIdParam.str());
					iter->define("nThreads",nThreadsParam.str());
					iter->define("name","FlagAgentManual_" + agentNumberParam.str() + "_Thread_" + threadIdParam.str());
					recordListMulithreading.push_back(*iter);
				}

				agentNumber++;
			}
			recordList = recordListMulithreading;
		}
		else
		{
			Int agentNumber=1;
			for (vector<Record>::iterator iter = recordList.begin();iter != recordList.end(); iter++)
			{
				stringstream agentNumberParam;
				agentNumberParam << agentNumber;
				iter->define("name","FlagAgentManual_" + agentNumberParam.str());
				agentNumber++;
			}
		}
	}

	clearFlags(inputFile,flag);

	writeFlags(inputFile,recordList,flag);

	if (!multipleAgents) checkFlags(inputFile,dataSelection,flag);

	summaryFlags(inputFile);
}

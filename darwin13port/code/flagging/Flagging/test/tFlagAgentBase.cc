//# tFlagAgentBase.cc This file contains the unit tests of the FlagAgentBase class.
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
#include <iostream>

using namespace casa;

void deleteFlags(string inputFile,Record record,vector<Record> recordList)
{
	// Stats variables declaration
	bool fillBuffer = false;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagMSHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);

	// Enable profiling in the Flag Data Handler
	dh->setProfiling(false);

	// Open MS
	dh->open();

	// Parse data selection to Flag Data Handler
	dh->setDataSelection(record);

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create a normal Flag Agent
	Record dummyRecord;
	FlagAgentBase *flaggingAgent = new FlagAgentBase(dh,dummyRecord,false,false,false);

	// Enable profiling in the Flag Agent
	flaggingAgent->setProfiling(false);

	// Start Flag Agent
	flaggingAgent->start();

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

			if (fillBuffer)
			{
                                dh->visibilityBuffer_p->get()->antenna1();
                                dh->visibilityBuffer_p->get()->antenna2();
                                dh->visibilityBuffer_p->get()->arrayId();
                                //dh->visibilityBuffer_p->get()->channel();
                                //dh->visibilityBuffer_p->get()->CJones();
                                dh->visibilityBuffer_p->get()->corrType();
                                //dh->visibilityBuffer_p->get()->direction1();
                                //dh->visibilityBuffer_p->get()->direction2();
                                //dh->visibilityBuffer_p->get()->exposure();
                                dh->visibilityBuffer_p->get()->feed1();
                                //dh->visibilityBuffer_p->get()->feed1_pa();
                                dh->visibilityBuffer_p->get()->feed2();
                                //dh->visibilityBuffer_p->get()->feed2_pa();
                                dh->visibilityBuffer_p->get()->fieldId();
                                //dh->visibilityBuffer_p->get()->flagCategory();
                                dh->visibilityBuffer_p->get()->flag();
                                dh->visibilityBuffer_p->get()->flagRow();
                                dh->visibilityBuffer_p->get()->flagCube();
                                //dh->visibilityBuffer_p->get()->floatDataCube();
                                dh->visibilityBuffer_p->get()->frequency();
                                dh->visibilityBuffer_p->get()->nChannel();
                                dh->visibilityBuffer_p->get()->nCorr();
                                dh->visibilityBuffer_p->get()->nRow();
                                //dh->visibilityBuffer_p->get()->observationId();
                                dh->visibilityBuffer_p->get()->phaseCenter();
                                //dh->visibilityBuffer_p->get()->polFrame();
                                //dh->visibilityBuffer_p->get()->processorId();
                                dh->visibilityBuffer_p->get()->scan();
                                //dh->visibilityBuffer_p->get()->sigma();
                                //dh->visibilityBuffer_p->get()->sigmaMat();
                                dh->visibilityBuffer_p->get()->spectralWindow();
                                dh->visibilityBuffer_p->get()->stateId();
                                dh->visibilityBuffer_p->get()->time();
                                //dh->visibilityBuffer_p->get()->timeCentroid();
                                dh->visibilityBuffer_p->get()->timeInterval();
                                dh->visibilityBuffer_p->get()->uvw();
                                //dh->visibilityBuffer_p->get()->uvwMat();
                                //dh->visibilityBuffer_p->get()->visibility();
                                dh->visibilityBuffer_p->get()->visCube();
                                //dh->visibilityBuffer_p->get()->weight();
                                //dh->visibilityBuffer_p->get()->weightMat();
                                //dh->visibilityBuffer_p->get()->weightSpectrum();

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
			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// cout << "Antenna1:" << dh->visibilityBuffer_p->get()->antenna1() << endl;
			// cout << "Antenna2:" << dh->visibilityBuffer_p->get()->antenna2() << endl;

			// Queue flagging process
			flaggingAgent->queueProcess();

			// Wait for completion of flagging process
			flaggingAgent->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	flaggingAgent->terminate();
	flaggingAgent->join();

	// Close MS
	dh->close();

	// Delete Flag Agent
	delete flaggingAgent;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

}

void writeFlags(string inputFile,Record record,vector<Record> recordList)
{
	// Stats variables declaration
	bool fillBuffer = false;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagMSHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);

	// Enable profiling in the Flag Data Handler
	dh->setProfiling(false);

	// Open MS
	dh->open();

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent list
	int antenna;
	FlagAgentList *agentList = new FlagAgentList();
	for (vector<Record>::iterator iter = recordList.begin();iter != recordList.end(); iter++)
	{
		antenna = (*iter).fieldNumber ("antenna");
		FlagAgentBase *agent_i;
		if (antenna >= 0)
		{
			agent_i= new FlagAgentBase(dh,*iter,false,true);
		}
		else
		{
			agent_i = new FlagAgentBase(dh,*iter,false,false);
		}
		agentList->push_back(agent_i);
	}

	agentList->setProfiling(false);

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

			if (fillBuffer)
			{
                                dh->visibilityBuffer_p->get()->antenna1();
                                dh->visibilityBuffer_p->get()->antenna2();
                                dh->visibilityBuffer_p->get()->arrayId();
                                //dh->visibilityBuffer_p->get()->channel();
                                //dh->visibilityBuffer_p->get()->CJones();
                                dh->visibilityBuffer_p->get()->corrType();
                                //dh->visibilityBuffer_p->get()->direction1();
                                //dh->visibilityBuffer_p->get()->direction2();
                                //dh->visibilityBuffer_p->get()->exposure();
                                dh->visibilityBuffer_p->get()->feed1();
                                //dh->visibilityBuffer_p->get()->feed1_pa();
                                dh->visibilityBuffer_p->get()->feed2();
                                //dh->visibilityBuffer_p->get()->feed2_pa();
                                dh->visibilityBuffer_p->get()->fieldId();
                                //dh->visibilityBuffer_p->get()->flagCategory();
                                dh->visibilityBuffer_p->get()->flag();
                                dh->visibilityBuffer_p->get()->flagRow();
                                dh->visibilityBuffer_p->get()->flagCube();
                                //dh->visibilityBuffer_p->get()->floatDataCube();
                                dh->visibilityBuffer_p->get()->frequency();
                                dh->visibilityBuffer_p->get()->nChannel();
                                dh->visibilityBuffer_p->get()->nCorr();
                                dh->visibilityBuffer_p->get()->nRow();
                                //dh->visibilityBuffer_p->get()->observationId();
                                dh->visibilityBuffer_p->get()->phaseCenter();
                                //dh->visibilityBuffer_p->get()->polFrame();
                                //dh->visibilityBuffer_p->get()->processorId();
                                dh->visibilityBuffer_p->get()->scan();
                                //dh->visibilityBuffer_p->get()->sigma();
                                //dh->visibilityBuffer_p->get()->sigmaMat();
                                dh->visibilityBuffer_p->get()->spectralWindow();
                                dh->visibilityBuffer_p->get()->stateId();
                                dh->visibilityBuffer_p->get()->time();
                                //dh->visibilityBuffer_p->get()->timeCentroid();
                                dh->visibilityBuffer_p->get()->timeInterval();
                                dh->visibilityBuffer_p->get()->uvw();
                                //dh->visibilityBuffer_p->get()->uvwMat();
                                //dh->visibilityBuffer_p->get()->visibility();
                                dh->visibilityBuffer_p->get()->visCube();
                                //dh->visibilityBuffer_p->get()->weight();
                                //dh->visibilityBuffer_p->get()->weightMat();
                                //dh->visibilityBuffer_p->get()->weightSpectrum();

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
			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// Apply flags
			agentList->apply();

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

void checkFlags(string inputFile,Record record,vector<Record> recordList)
{
	// Stats variables declaration
	bool fillBuffer = false;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Start clock
	gettimeofday(&start,0);

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagMSHandler(inputFile,FlagDataHandler::COMPLETE_SCAN_UNMAPPED);

	// Enable profiling in the Flag Data Handler
	dh->setProfiling(false);

	// Open MS
	dh->open();

	// Parse data selection to Flag Data Handler
	dh->setDataSelection(record);

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent list
	int antenna;
	FlagAgentList *agentList = new FlagAgentList();
	for (vector<Record>::iterator iter = recordList.begin();iter != recordList.end(); iter++)
	{
		antenna = (*iter).fieldNumber ("antenna");
		FlagAgentBase *agent_i;
		if (antenna >= 0)
		{
			agent_i= new FlagAgentBase(dh,*iter,false,true);
		}
		else
		{
			agent_i = new FlagAgentBase(dh,*iter,false,false);
		}
		agentList->push_back(agent_i);
	}

	// Enable profiling in the Flag Agent
	agentList->setProfiling(false);

	// Enable check mode in the Flag Agent
	agentList->setCheckMode(true);

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

			if (fillBuffer)
			{
                                dh->visibilityBuffer_p->get()->antenna1();
                                dh->visibilityBuffer_p->get()->antenna2();
                                dh->visibilityBuffer_p->get()->arrayId();
                                //dh->visibilityBuffer_p->get()->channel();
                                //dh->visibilityBuffer_p->get()->CJones();
                                dh->visibilityBuffer_p->get()->corrType();
                                //dh->visibilityBuffer_p->get()->direction1();
                                //dh->visibilityBuffer_p->get()->direction2();
                                //dh->visibilityBuffer_p->get()->exposure();
                                dh->visibilityBuffer_p->get()->feed1();
                                //dh->visibilityBuffer_p->get()->feed1_pa();
                                dh->visibilityBuffer_p->get()->feed2();
                                //dh->visibilityBuffer_p->get()->feed2_pa();
                                dh->visibilityBuffer_p->get()->fieldId();
                                //dh->visibilityBuffer_p->get()->flagCategory();
                                dh->visibilityBuffer_p->get()->flag();
                                dh->visibilityBuffer_p->get()->flagRow();
                                dh->visibilityBuffer_p->get()->flagCube();
                                //dh->visibilityBuffer_p->get()->floatDataCube();
                                dh->visibilityBuffer_p->get()->frequency();
                                dh->visibilityBuffer_p->get()->nChannel();
                                dh->visibilityBuffer_p->get()->nCorr();
                                dh->visibilityBuffer_p->get()->nRow();
                                //dh->visibilityBuffer_p->get()->observationId();
                                dh->visibilityBuffer_p->get()->phaseCenter();
                                //dh->visibilityBuffer_p->get()->polFrame();
                                //dh->visibilityBuffer_p->get()->processorId();
                                dh->visibilityBuffer_p->get()->scan();
                                //dh->visibilityBuffer_p->get()->sigma();
                                //dh->visibilityBuffer_p->get()->sigmaMat();
                                dh->visibilityBuffer_p->get()->spectralWindow();
                                dh->visibilityBuffer_p->get()->stateId();
                                dh->visibilityBuffer_p->get()->time();
                                //dh->visibilityBuffer_p->get()->timeCentroid();
                                dh->visibilityBuffer_p->get()->timeInterval();
                                dh->visibilityBuffer_p->get()->uvw();
                                //dh->visibilityBuffer_p->get()->uvwMat();
                                //dh->visibilityBuffer_p->get()->visibility();
                                dh->visibilityBuffer_p->get()->visCube();
                                //dh->visibilityBuffer_p->get()->weight();
                                //dh->visibilityBuffer_p->get()->weightMat();
                                //dh->visibilityBuffer_p->get()->weightSpectrum();

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

			}

			cout << "nRows:" << dh->visibilityBuffer_p->get()->nRow() <<endl;
			cumRows += dh->visibilityBuffer_p->get()->nRow();

			// Apply flags
			agentList->apply();

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

int main(int argc, char **argv)
{
	// Parsing variables declaration
	string parameter, value;
	string inputFile, array, scan, time;
	string field, spw, baseline, uvw, polarization, observation, intent;
	unsigned short testMode = 0;
	unsigned short logLevel = 0;


	// Retrieve log level
	for (unsigned char i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-logLevel"))
		{
			logLevel = (unsigned short)atoi(value.c_str());
		}
	}


	// Parse input parameters
	vector<Record> recordList;
	Record record;
	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-file"))
		{
			inputFile = value;
			if (logLevel >= 3) cout << "File is: " << inputFile << endl;
		}
		else if (parameter == string("-array"))
		{
			array = value;
			Record rec;
			rec.define ("array", casa::String(value));
			record.define ("array", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Array selection is: " << array << endl;
		}
		else if (parameter == string("-scan"))
		{
			scan = value;
			Record rec;
			rec.define ("scan", casa::String(value));
			record.define ("scan", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Scan selection is: " << scan << endl;
		}
		else if (parameter == string("-timerange"))
		{
			time = value;
			Record rec;
			rec.define ("timerange", casa::String(value));
			record.define ("timerange", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Time range selection is: " << time << endl;
		}
		else if (parameter == string("-field"))
		{
			field = value;
			Record rec;
			rec.define ("field", casa::String(value));
			record.define ("field", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Field selection is: " << field << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = value;
			Record rec;
			rec.define ("spw", casa::String(value));
			record.define ("spw", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "SPW selection is: " << spw << endl;
		}
		else if (parameter == string("-antenna"))
		{
			baseline = value;
			Record rec;
			rec.define ("antenna", casa::String(value));
			record.define ("antenna", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Antenna selection is: " << baseline << endl;
		}
		else if (parameter == string("-uvrange"))
		{
			uvw = value;
			Record rec;
			rec.define ("uvrange", casa::String(value));
			record.define ("uvrange", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "UV range selection is: " << uvw << endl;
		}
		else if (parameter == string("-correlation"))
		{
			polarization = value;
			Record rec;
			rec.define ("correlation", casa::String(value));
			record.define ("correlation", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Correlation selection is: " << polarization << endl;
		}
		else if (parameter == string("-observation"))
		{
			observation = value;
			Record rec;
			rec.define ("observation", casa::String(value));
			record.define ("observation", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Observation selection is: " << observation << endl;
		}
		else if (parameter == string("-intent"))
		{
			intent = value;
			Record rec;
			rec.define ("intent", casa::String(value));
			record.define ("intent", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Scan intention selection is: " << intent << endl;
		}
		else if (parameter == string("-mode"))
		{
			testMode = (unsigned short)atoi(value.c_str());
			Record rec;
			rec.define ("mode", casa::String(value));
			record.define ("mode", casa::String(value));
			recordList.push_back(rec);
			if (logLevel >= 3) cout << "Test mode is: " << testMode << endl;
		}
	}

	// To test one single agent
	recordList.clear();
	recordList.push_back(record);

	deleteFlags(inputFile,record,recordList);
	writeFlags(inputFile,record,recordList);
	checkFlags(inputFile,record,recordList);

	exit(-1);
}

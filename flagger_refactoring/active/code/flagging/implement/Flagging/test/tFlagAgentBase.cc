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

void deleteFlags(string inputFile,Record record)
{
	// Stats variables declaration
	bool fillBuffer = false;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile);

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

	// Now we can create the Flag Agent
	Record dummyRecord;
	FlagAgentBase *fa = new FlagAgentBase(dh,dummyRecord,false,false);

	// Enable profiling in the Flag Agent
	fa->setProfiling(false);

	// Start Flag Agent
	fa->start();

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
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

			}

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

			// cout << "Antenna1:" << dh->visibilityBuffer_p->get()->antenna1() << endl;
			// cout << "Antenna2:" << dh->visibilityBuffer_p->get()->antenna2() << endl;

			// Queue flagging process
			fa->queueProcess();

			// Wait for completion of flagging process
			fa->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	fa->terminate();
	fa->join();

	// Close MS
	dh->close();

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

	// Delete Flag Agent
	delete fa;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

}

void writeFlags(string inputFile,Record record)
{
	// Stats variables declaration
	bool fillBuffer = false;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile);

	// Enable profiling in the Flag Data Handler
	dh->setProfiling(false);

	// Open MS
	dh->open();

	// Select data (creating selected MS)
	dh->selectData();

	// Generate iterators and vis buffers
	dh->generateIterator();

	// Now we can create the Flag Agent
	FlagAgentBase *fa = new FlagAgentBase(dh,record);

	// Enable profiling in the Flag Agent
	fa->setProfiling(false);

	// Start Flag Agent
	fa->start();

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
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

			}

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
			// cout << "Put flag process in queue " << endl;
			fa->queueProcess();

			// Wait for completion of flagging process
			// cout << "Wait for completion of flagging process " << endl;
			fa->completeProcess();

			// Flush flags to MS
			// cout << "Flush flags to MS " << endl;
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	fa->terminate();
	fa->join();

	// Close MS
	dh->close();

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

	// Delete Flag Agent
	delete fa;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

}

void checkFlags(string inputFile,Record record)
{
	// Stats variables declaration
	bool fillBuffer = false;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;
	double elapsedTime = 0;

	// Create Flag Data Handler
	FlagDataHandler *dh = new FlagDataHandler(inputFile);

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

	// Now we can create the Flag Agent
	Record dummyRecord;
	FlagAgentBase *fa = new FlagAgentBase(dh,dummyRecord);

	// Enable profiling in the Flag Agent
	fa->setProfiling(false);

	// Enable check mode in the Flag Agent
	fa->setCheckMode();

	// Start Flag Agent
	fa->start();

	// Set cout precision
	cout.precision(20);

	// Start clock
	gettimeofday(&start,0);

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
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

			}

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
			fa->queueProcess();

			// Wait for completion of flagging process
			fa->completeProcess();

			// Flush flags to MS
			dh->flushFlags();
		}
	}

	// Stop Flag Agent
	fa->terminate();
	fa->join();

	// Close MS
	dh->close();

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

	// Delete Flag Agent
	delete fa;

	// Delete Flag Data Handler (delete VisBuffer, therefore stop VLAT)
	delete dh;

}

int main(int argc, char **argv)
{
	// Parsing variables declaration
	string parameter, value;
	string inputFile, observation, array, scan, time;
	string field, spw, baseline, uvw, polarization;
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
		else if (parameter == string("-observation"))
		{
			observation = value;
			record.define ("observation", casa::String(value));
			if (logLevel >= 3) cout << "Observation selection is: " << observation << endl;
		}
		else if (parameter == string("-array"))
		{
			array = value;
			record.define ("array", casa::String(value));
			if (logLevel >= 3) cout << "Array selection is: " << array << endl;
		}
		else if (parameter == string("-scan"))
		{
			scan = value;
			record.define ("scan", casa::String(value));
			if (logLevel >= 3) cout << "Scan selection is: " << scan << endl;
		}
		else if (parameter == string("-time"))
		{
			time = value;
			record.define ("time", casa::String(value));
			if (logLevel >= 3) cout << "Time selection is: " << time << endl;
		}
		else if (parameter == string("-field"))
		{
			field = value;
			record.define ("field", casa::String(value));
			if (logLevel >= 3) cout << "Field selection is: " << field << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = value;
			record.define ("spw", casa::String(value));
			if (logLevel >= 3) cout << "SPW selection is: " << spw << endl;
		}
		else if (parameter == string("-baseline"))
		{
			baseline = value;
			record.define ("baseline", casa::String(value));
			if (logLevel >= 3) cout << "Baseline selection is: " << baseline << endl;
		}
		else if (parameter == string("-uvw"))
		{
			uvw = value;
			record.define ("uvw", casa::String(value));
			if (logLevel >= 3) cout << "UVW selection is: " << uvw << endl;
		}
		else if (parameter == string("-polarization"))
		{
			polarization = value;
			record.define ("polarization", casa::String(value));
			if (logLevel >= 3) cout << "Polarization selection is: " << polarization << endl;
		}
		else if (parameter == string("-mode"))
		{
			testMode = (unsigned short)atoi(value.c_str());
			record.define ("mode", casa::String(value));
			if (logLevel >= 3) cout << "Test mode is: " << testMode << endl;
		}
	}

	deleteFlags(inputFile,record);
	writeFlags(inputFile,record);
	checkFlags(inputFile,record);

	exit(-1);
}

//# FlagDataHandler.h: This file contains the unit tests of the FlagDataHandler class.
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

#include <flagging/Flagging/FlagDataHandler.h>
#include <iostream>

using namespace casa;

void writeFlags(string input_file,unsigned short test_mode,Record record)
{
	bool fillBuffer = true;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;

	timeval start,stop;
	double elapsedTime = 0;

	unsigned short rotateViews, iterationMode, rotateMode;
	bool fixRotateMode = true;

	switch (test_mode)
	{
		case 1:
			iterationMode = FlagDataHandler::COMPLETE_SCAN_MAP_ANTENNA_PAIRS_ONLY;
			rotateViews = 2;
			rotateMode = 1;
			fixRotateMode = false;
		break;

		case 2:
			iterationMode = FlagDataHandler::COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY;
			rotateViews = 2;
			rotateMode = 2;
			fixRotateMode = true;
		break;

		case 3:
			iterationMode = FlagDataHandler::COMPLETE_SCAN_MAPPED;
			rotateViews = 2;
			rotateMode = 1;
			fixRotateMode = false;
		break;

		case 4:
			iterationMode = FlagDataHandler::COMPLETE_SCAN_MAPPED;
			rotateViews = 2;
			rotateMode = 2;
			fixRotateMode = true;
		break;

		default:
			iterationMode = FlagDataHandler::COMPLETE_SCAN_MAP_SUB_INTEGRATIONS_ONLY;
			rotateViews = 2;
			rotateMode = 2;
			fixRotateMode = true;
		break;
	}

	// Create Flag Data Handler object
	FlagDataHandler *dh = new FlagDataHandler(input_file,iterationMode);

	// Enable profiling
	dh->setProfiling(true);

	// Open Measurement Set
	dh->open();

	// Set data selection
	dh->setDataSelection(record);

	// Select data
	dh->selectData();

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

			// Process Visibility buffer (dummy)
			if (fixRotateMode)
			{
				dh->processBuffer(true,rotateMode,rotateViews);
			}
			else
			{
				rotateViews = dh->processBuffer(true,rotateMode,rotateViews);
			}


			// Write flag cube
			dh->flushFlags();
		}
	}

	dh->close();

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Reading Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

	// Free data handler
	delete dh;
}

void checkFlags(string input_file,unsigned short test_mode,Record record)
{
	bool fillBuffer = true;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;

	timeval start,stop;
	double elapsedTime = 0;

	unsigned short iterationMode;

	switch (test_mode)
	{
		case 1:
			iterationMode = FlagDataHandler::ANTENNA_PAIR;
		break;

		case 2:
			iterationMode = FlagDataHandler::SUB_INTEGRATION;
		break;

		case 3:
			iterationMode = FlagDataHandler::ANTENNA_PAIR;
		break;

		case 4:
			iterationMode = FlagDataHandler::SUB_INTEGRATION;
		break;

		default:
			iterationMode = FlagDataHandler::SUB_INTEGRATION;
		break;
	}

	// Create Flag Data Handler object
	FlagDataHandler *dh = new FlagDataHandler(input_file,iterationMode);

	// Enable profiling
	dh->setProfiling(true);

	// Open Measurement Set
	dh->open();

	// Set data selection
	dh->setDataSelection(record);

	// Select data
	dh->selectData();

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

			// Process Visibility buffer (dummy)
			dh->processBuffer(false,0,0);
		}
	}

	dh->close();

	// Stop clock
	gettimeofday(&stop,0);
	elapsedTime = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;

	// Report elapsed time
	cout << "Total Reading Time [s]:" << elapsedTime/1000.0 << " Total number of rows:" << cumRows <<" Total number of Buffers:" << nBuffers <<endl;

	// Free data handler
	delete dh;
}

Bool &access(Bool *storage,uInt x, uInt y, uInt z)
{

	return storage[x*1 + y*1000 + z*1000000];
}

void example()
{
	// Create a new flag data handler with (time,frequency) maps and sub-integration views
	FlagDataHandler *dh = new FlagDataHandler("test.ms",FlagDataHandler::COMPLETE_SCAN_MAPPED,10);

	// Open Ms
	dh->open();

	// Add data selection ranges, several ranges can be added separated by ; and the union will be selected
	Record record;
	record.define ("scan", "5~20;3~23");
	record.define ("spw", "70~80;85~");
	dh->setDataSelection(record);

	// Select data and generate iterators
	dh->selectData();

	// Declare variables for iterating over the views
	Double timestep;
	Cube<Bool> *flagCube;
	Cube<Complex> *visCube;
	std::pair<Int,Int> antennaPair;

	// iterate over chunks
	while (dh->nextChunk())
	{
		// iterate over visBuffers
		while (dh->nextBuffer())
		{
			// Access (time,frequency) maps per antenna pair
			antennaPairMap *antennaMap = dh->getAntennaPairMap();

			// Iterate over (time,frequency) maps per antenna pair
			for (antennaPairMapIterator antennaIter=antennaMap->begin();
					antennaIter != antennaMap->end();++antennaIter)
			{
				// Get antenna pair
				antennaPair = antennaIter->first;

				// Get visibility cube
				visCube = dh->getVisibilitiesView(antennaPair.first,antennaPair.second);

				// Get mapped flag cube (can be modified and the flags will be written in the correct positions of the total flag cube)
				flagCube = dh->getFlagsView(antennaPair.first,antennaPair.second);

				// Compute flags ....
			}

			// Access sub integrations map
			subIntegrationMap *subIntegrationMap  = dh->getSubIntegrationMap();

			// Iterate over sub integrations (time steps)
			for (subIntegrationMapIterator subIntegrationIter=subIntegrationMap->begin();
					subIntegrationIter != subIntegrationMap->end();++subIntegrationIter)
			{
				// Get time step
				timestep = subIntegrationIter->first;

				// Get visibility cube
				visCube = dh->getVisibilitiesView(timestep);

				// Get mapped flag cube (can be modified and the flags will be written in the correct positions of the total flag cube)
				flagCube = dh->getFlagsView(timestep);

				// Compute flags ....
			}
		}

		// Access modified flag cube
		flagCube = dh->getModifiedFlagCube();

		// Access original flag cube
		flagCube = dh->getOriginalFlagCube();

		// Write flag cube
		dh->flushFlags();
	}

	// Close MS
	dh->close();

	return;
}

int main(int argc, char **argv)
{
	// Variables declaration
	string parameter, value;
	string inputFile, observation, array, scan, field, spw;
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
		else if (parameter == string("-mode"))
		{
			testMode = (unsigned short)atoi(value.c_str());
			record.define ("mode", casa::String(value));
			if (logLevel >= 3) cout << "Test mode is: " << testMode << endl;
		}
	}

	writeFlags(inputFile,testMode,record);
	checkFlags(inputFile,testMode,record);

	/*// Profiling test requested by Brian

	// Initialize variables
	timeval start,stop;
	double elapsedTimeInMs;
	Cube<Bool> testCube(1000,1000,1000);
	cout.precision(20);
	Bool flag = true;
	Bool deleteIt = false;
	Bool *storage = testCube.getStorage(deleteIt);

	// First we try with normal access
	gettimeofday(&start,0);
	for (uInt z=0;z<1000;z++)
	{
		for (uInt y=0;y<1000;y++)
		{
			for (uInt x=0;x<1000;x++)
			{
				testCube(x,y,z) = flag;
			}
		}
	}
	gettimeofday(&stop,0);
	elapsedTimeInMs = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;
	cout << "Elapsed time: " << elapsedTimeInMs << " ms" << endl;


	// Now directly with the storage array
	gettimeofday(&start,0);
	for (uInt z=0;z<1000;z++)
	{
		for (uInt y=0;y<1000;y++)
		{
			for (uInt x=0;x<1000;x++)
			{
				storage[x*1 + y*1000 + z*1000000] = flag;
			}
		}
	}
	gettimeofday(&stop,0);
	elapsedTimeInMs = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;
	cout << "Elapsed time: " << elapsedTimeInMs << " ms" << endl;


	// Now with the storage array but calling an intermediate function
	gettimeofday(&start,0);
	for (uInt z=0;z<1000;z++)
	{
		for (uInt y=0;y<1000;y++)
		{
			for (uInt x=0;x<1000;x++)
			{
				access(storage,x,y,z) = flag;
			}
		}
	}
	gettimeofday(&stop,0);
	elapsedTimeInMs = (stop.tv_sec-start.tv_sec)*1000.0+(stop.tv_usec-start.tv_usec)/1000.0;
	cout << "Elapsed time: " << elapsedTimeInMs << " ms" << endl;

	*/

	exit(-1);
}

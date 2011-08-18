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

void writeFlags(string input_file,unsigned short iteration_mode,Record record)
{
	bool fillBuffer = true;
	double elapsedTime = 0;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;

	// Create Flag Data Handler object
	FlagDataHandler *dh = new FlagDataHandler(iteration_mode);

	// Enable profiling
	dh->setProfiling(true);

	// Open Measurement Set
	dh->open(input_file.c_str());

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
			dh->processBuffer(true);

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

	// Free sata handler
	delete dh;
}

void checkFlags(string input_file,unsigned short iteration_mode,Record record)
{
	bool fillBuffer = true;
	double elapsedTime = 0;
	unsigned long nBuffers = 0;
	unsigned long cumRows = 0;
	timeval start,stop;

	// Create Flag Data Handler object
	FlagDataHandler *dh = new FlagDataHandler(iteration_mode);

	// Enable profiling
	dh->setProfiling(true);

	// Open Measurement Set
	dh->open(input_file.c_str());

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
			dh->processBuffer(false);
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

int main(int argc, char **argv)
{
	// Variables declaration
	string parameter, value;
	string input_file, observation, array, scan, field, spw;
	unsigned short iteration_mode = 0;
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
			input_file = value;
			if (logLevel >= 3) cout << "File is: " << input_file << endl;
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
		else if (parameter == string("-iteration"))
		{
			iteration_mode = (unsigned short)atoi(value.c_str());
			record.define ("iteration", casa::String(value));
			if (logLevel >= 3) cout << "Iteration mode is: " << iteration_mode << endl;
		}
	}

	writeFlags(input_file,iteration_mode,record);

	checkFlags(input_file,iteration_mode,record);

	exit(-1);
}

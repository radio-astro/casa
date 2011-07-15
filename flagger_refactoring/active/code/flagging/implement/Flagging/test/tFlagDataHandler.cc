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

int main(unsigned char argc, char **argv)
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
			if (logLevel >= 3) cout << "Observation selection is: " << observation << endl;
		}
		else if (parameter == string("-array"))
		{
			array = value;
			if (logLevel >= 3) cout << "Array selection is: " << array << endl;
		}
		else if (parameter == string("-scan"))
		{
			scan = value;
			if (logLevel >= 3) cout << "Scan selection is: " << scan << endl;
		}
		else if (parameter == string("-field"))
		{
			field = value;
			if (logLevel >= 3) cout << "Field selection is: " << field << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = value;
			if (logLevel >= 3) cout << "SPW selection is: " << spw << endl;
		}
		else if (parameter == string("-iteration"))
		{
			iteration_mode = (unsigned short)atoi(value.c_str());
			if (logLevel >= 3) cout << "Iteration mode is: " << iteration_mode << endl;
		}
	}

	// Create Flag Data Handler object
	FlagDataHandler *dh = new FlagDataHandler(iteration_mode);

	// Open Measurement Set
	dh->open(input_file.c_str());

	// Select data
	dh->selectData();

	// Set cout precision
	cout.precision(20);

	// Iterates over chunks (constant values)
	while (dh->nextChunk())
	{
		// Iterates over buffers (time steps)
		while (dh->nextBuffer())
		{
			cout << "Chunk:" << dh->chunkNo << " " << "Buffer:" << dh->bufferNo << " ";

			if (dh->visibilityBuffer_p.observationId().nelements() > 1)
			{
				cout << "Observation:"
					 << dh->visibilityBuffer_p.observationId()[0] << "~"
					 << dh->visibilityBuffer_p.observationId()[dh->visibilityBuffer_p.observationId().nelements()-1] << " ";
			}
			else
			{
				cout << "Observation:" << dh->visibilityBuffer_p.observationId()[0] << " ";
			}

			cout << "Array:" << dh->visibilityBuffer_p.arrayId() << " ";

			if (dh->visibilityBuffer_p.scan().nelements() > 1)
			{
				cout << "Scan:"
					 << dh->visibilityBuffer_p.scan()[0] << "~"
					 << dh->visibilityBuffer_p.scan()[dh->visibilityBuffer_p.scan().nelements()-1] << " ";
			}
			else
			{
				cout << "Scan:" << dh->visibilityBuffer_p.scan()[0] << " ";
			}

			cout << "Field:" << dh->visibilityBuffer_p.fieldId() << " " ;

			cout << "Spw:" << dh->visibilityBuffer_p.spectralWindow() << " ";

			if (dh->visibilityBuffer_p.time().nelements() > 1)
			{
				cout << "Time:"
					 << dh->visibilityBuffer_p.time()[0] << "~"
					 << dh->visibilityBuffer_p.time()[dh->visibilityBuffer_p.time().nelements()-1] << " ";
			}
			else
			{
				cout << "Time:" << dh->visibilityBuffer_p.time()[0] << " ";
			}

			if (dh->visibilityBuffer_p.antenna1().nelements() > 1)
			{
				cout << "Antenna1:"
					 << dh->visibilityBuffer_p.antenna1()[0] << "~"
					 << dh->visibilityBuffer_p.antenna1()[dh->visibilityBuffer_p.antenna1().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna1:" << dh->visibilityBuffer_p.antenna1()[0] << " ";
			}

			if (dh->visibilityBuffer_p.antenna2().nelements() > 1)
			{
				cout << "Antenna2:"
					 << dh->visibilityBuffer_p.antenna2()[0] << "~"
					 << dh->visibilityBuffer_p.antenna2()[dh->visibilityBuffer_p.antenna2().nelements()-1] << " ";
			}
			else
			{
				cout << "Antenna2:" << dh->visibilityBuffer_p.antenna2()[0] << " ";
			}

			cout << "nRows:" << dh->visibilityBuffer_p.nRow() << endl;
		}
	}

	exit(-1);
}

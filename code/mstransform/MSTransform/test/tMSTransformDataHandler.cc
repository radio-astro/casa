//# tMSTransformDataHandlerr.cc: This file contains the unit tests of the MsTransformDataHandler class.
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

#include <mstransform/MSTransform/MSTransformDataHandler.h>

#include <string>
#include <iostream>

using namespace casa;
using namespace std;

int main(int argc, char **argv)
{
	// Parsing input parameters
	string parameter,value;
	Record configuration;

	// Data selection parameters
	String inputMS,outputMS,combine;
	String timerange,antenna,field,spw,uvrange,correlation,scan,array,intent,observation;
	Double timebin;

	// Parse input parameters
	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-inputms"))
		{
			inputMS = value;
			configuration.define ("inputms", inputMS);
			cout << "Input file is: " << inputMS << endl;
		}
		else if (parameter == string("-outputms"))
		{
			outputMS = value;
			configuration.define ("outputms", outputMS);
			cout << "Output file is: " << outputMS << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = value;
			configuration.define ("spw", spw);
			cout << "Spectral Window selection is: " << spw << endl;
		}
		else if (parameter == string("-timebin"))
		{
			timebin = strtod(value.c_str(),NULL);
			configuration.define ("timebin", timebin);
			cout << "Time interval is: " << timebin << endl;
		}
		else if (parameter == string("-combine"))
		{
			combine = value;
			configuration.define ("combine", combine);
			cout << "Combine axes are: " << combine << endl;
		}
	}


	// Set up data handler
	MSTransformDataHandler *tvdh = new MSTransformDataHandler(configuration);
	tvdh->open();
	tvdh->setup();

	vi::VisibilityIterator2 *visIter = tvdh->getVisIter();
	vi::VisBuffer2 *vb = visIter->getVisBuffer();
	visIter->originChunks();
	while (visIter->moreChunks())
	{
		visIter->origin();
		while (visIter->more())
		{
			tvdh->fillOutputMs(vb);
			visIter->next();
		}
		visIter->nextChunk();
	}

	tvdh->close();
	delete tvdh;

	exit(0);
}

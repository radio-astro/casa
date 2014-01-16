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
	String inputMS,outputMS, datacolumn;
	Bool combinespws, hanning, regridms, freqaverage, realmodelcol;
	String timerange,antenna,field,spw,uvrange,correlation,scan,array,intent,observation;

	Int nchan,freqbin,ddistart,nspw;
	String start, width, useweights;

	Bool mergeSpwSubTables = False;
	vector<String> submslist;

	vector<Int> tileshape;
	Bool tileshapeON = False;

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
		else if (parameter == string("-datacolumn"))
		{
			datacolumn = value;
			configuration.define ("datacolumn", datacolumn);
			cout << "Data column is: " << datacolumn << endl;
		}
		else if (parameter == string("-realmodelcol"))
		{
			realmodelcol = Bool(atoi(value.c_str()));
			configuration.define ("realmodelcol", Bool(realmodelcol));
			cout << "Real MODEL column is: " << realmodelcol << endl;
		}
		else if (parameter == string("-tileshape"))
		{
			tileshapeON = True;
			tileshape.push_back(atoi(value.c_str()));
			cout << "Tile shape is: " << tileshape.at(tileshape.size()-1) << endl;
		}
		else if (parameter == string("-combinespws"))
		{
			combinespws = Bool(atoi(value.c_str()));
			configuration.define ("combinespws", Bool(combinespws));
			cout << "Combine Spectral Windows is: " << combinespws << endl;
		}
		else if (parameter == string("-hanning"))
		{
			hanning = Bool(atoi(value.c_str()));
			configuration.define ("hanning", Bool(hanning));
			cout << "Hanning Smooth is: " << hanning << endl;
		}
		else if (parameter == string("-regridms"))
		{
			regridms = Bool(atoi(value.c_str()));
			configuration.define ("regridms", Bool(regridms));
			cout << "Regrid MS is: " << regridms << endl;
		}
		else if (parameter == string("-nchan"))
		{
			nchan = atoi(value.c_str());
			configuration.define ("nchan", nchan);
			cout << "nchan is: " << nchan << endl;
		}
		else if (parameter == string("-start"))
		{
			start = value;
			configuration.define ("start", start);
			cout << "Start is: " << start << endl;
		}
		else if (parameter == string("-width"))
		{
			width = value;
			configuration.define ("width", width);
			cout << "Width is: " << width << endl;
		}
		else if (parameter == string("-freqaverage"))
		{
			freqaverage = Bool(atoi(value.c_str()));
			configuration.define ("freqaverage", Bool(freqaverage));
			cout << "Frequency Average is: " << freqaverage << endl;
		}
		else if (parameter == string("-freqbin"))
		{
			freqbin = atoi(value.c_str());
			configuration.define ("freqbin", freqbin);
			cout << "Frequency bin is: " << freqbin << endl;
		}
		else if (parameter == string("-useweights"))
		{
			useweights = value;
			configuration.define ("useweights", useweights);
			cout << "Weights is: " << useweights << endl;
		}
		else if (parameter == string("-spw"))
		{
			spw = value;
			configuration.define ("spw", spw);
			cout << "Spectral Window selection is: " << spw << endl;
		}
		else if (parameter == string("-intent"))
		{
			intent = value;
			configuration.define ("intent", intent);
			cout << "Intent is: " << intent << endl;
		}
		else if (parameter == string("-field"))
		{
			field = value;
			configuration.define ("field", field);
			cout << "Field selection is: " << field << endl;
		}
		else if (parameter == string("-correlation"))
		{
			correlation = value;
			configuration.define ("correlation", correlation);
			cout << "Correlation selection is: " << correlation << endl;
		}
		else if (parameter == string("-ddistart"))
		{
			ddistart = atoi(value.c_str());
			configuration.define ("ddistart", ddistart);
			cout << "DDI start is: " << ddistart << endl;
		}
		else if (parameter == string("-nspw"))
		{
			nspw = atoi(value.c_str());
			configuration.define ("nspw", nspw);
			cout << "nspw is: " << nspw << endl;
		}
		else if (parameter == string("-subms"))
		{
			mergeSpwSubTables = True;
			submslist.push_back(value);
			cout << "Adding MS to list of SubMS to merge SPW subtable : " << value << endl;
		}
	}

	if (mergeSpwSubTables)
	{
		MSTransformDataHandler::mergeSpwSubTables(submslist);
	}
	else
	{
		if (tileshapeON)
		{
			Vector<Int> tileShapeCASA = tileshape;
			configuration.define ("tileshape", tileShapeCASA);
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
	}

	exit(0);
}

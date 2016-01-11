//# tChannelAverageTVI: This file contains the unit tests of the ChannelAverageTVI class.
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


#include <mstransform/TVI/test/TestUtilsTVI.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <mstransform/MSTransform/MSTransformIteratorFactory.h>

using namespace std;
using namespace casa;
using namespace casa::vi;

Record parseConfiguration(int argc, char **argv)
{
	string parameter,value;
	Record configuration;
	Bool autoMode = True;

	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-vis"))
		{
			configuration.define ("inputms", value);
			autoMode = False;
		}
		else if (parameter == string("-spw"))
		{
			configuration.define ("spw", value);
		}
		else if (parameter == string("-chanbin"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("chanbin", tmp);
		}
	}

	if (autoMode)
	{
		char* pathChar = getenv ("CASAPATH");
		if (pathChar != NULL)
		{
			// Get base path
			String pathStr(pathChar);
			string res[2];
			casa::split(pathChar,res,2,string(" "));

			// Generate filename
			string filename(res[0]);
			filename += string("/data/regression/unittest/flagdata/Four_ants_3C286.ms");

			// Check if file exists
			string command = string ("cp -r ") + filename + string(" .");
			Int exists = system(command.c_str());
			if (exists == 0)
			{
				configuration.define ("inputms", String("Four_ants_3C286.ms"));
				configuration.define ("chanaverage", True);
				configuration.define ("chanbin", 8);
				configuration.define ("datacolumn", string("ALL"));
				configuration.define ("reindex", False);
				configuration.define ("spw", "1");
			}
			else
			{
				cout << RED;
				cout << "AUTOMATIC MODE DOES NOT FIND TESTING FILE: " << filename << endl;
				cout << RESET;
				exit(1);
			}

		}
		else
		{
			cout << RED;
			cout << "AUTOMATIC MODE DOES NOT FIND ENV. VARIABLE CASAPATH" << endl;
			cout << RESET;
			exit(1);
		}
	}

	return configuration;
}

Bool test_compareVsMSTransformIterator(Record configuration)
{
	// Declare working variables
	Bool res;
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Generate MSTransformIterator and associated buffer
	MSTransformIteratorFactory refFactory(configuration);
	VisibilityIterator2 refTVI(refFactory);

	// Generate input VisbilityIterator (use selected MS from MSTransformIterator)
	// NOTE: Declare as a pointer so that destructor is not invoked
	//       (~VisibilityIterator2 deletes its ViImplementation2 and
	//        ~TransformingVi2 deletes the input ViImplementation2)
	VisibilityIterator2 *inputVI = new vi::VisibilityIterator2(refFactory.getSelectedMS());

	// Generate TVI to test
	ChannelAverageTVIFactory testFactory(configuration,inputVI->getImpl());
	VisibilityIterator2 testTVI(testFactory);

	// Determine columns to check
	VisBufferComponents2 columns;
	columns += NRows;
	columns += NChannels;
	columns += NCorrelations;
	columns += FlagRow;
	columns += FlagCube;
	columns += VisibilityCubeObserved;
	columns += VisibilityCubeCorrected;
	columns += VisibilityCubeModel;
	columns += VisibilityCubeFloat;
	columns += WeightSpectrum;
	columns += SigmaSpectrum;
	columns += Weight;
	columns += Sigma;

	// Compare
	res = compareVisibilityIterators(testTVI,refTVI,columns,tolerance);

	return res;
}

int main(int argc, char **argv)
{
	// Read configuration
	Record configuration = parseConfiguration(argc, argv);

	// Run test
	Bool result = test_compareVsMSTransformIterator(configuration);

	// Exit code
	if (result)
	{
		exit(0);
	}
	else
	{
		exit(1);
	}
}

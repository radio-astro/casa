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
	string parameter,value,inputms;
	Record configuration;
	Bool autoMode = True;

	for (unsigned short i=0;i<argc-1;i++)
	{
		parameter = string(argv[i]);
		value = string(argv[i+1]);

		if (parameter == string("-vis"))
		{
			inputms = value;
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

			// Remove any previously existing copy
			inputms = string("Four_ants_3C286.ms");
			string rm_command = string ("rm -r ") + inputms + string("*");
			system(rm_command.c_str());

			// Make a copy of the MS in the working directory
			string cp_command = string ("cp -r ") + filename + string(" .");
			Int ret = system(cp_command.c_str());
			if (ret == 0)
			{
				configuration.define ("inputms", String("Four_ants_3C286.ms"));
				configuration.define ("spw", "1");
				configuration.define ("chanbin", 8);
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

	// Generic settings necessary for MSTransformIterator
	configuration.define ("reindex", False);
	configuration.define ("chanaverage", True);
	configuration.define ("datacolumn", string("ALL"));

	// Make a second copy of the MS to serve as reference value
	string cp_command = string ("cp -r ") + inputms + string(" ") + inputms + string(".ref");
	system(cp_command.c_str());

	return configuration;
}

Bool test_compareTransformedData(Record configuration)
{
	// Declare working variables
	Bool res;
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Get original inputms name
	String inputms;
	configuration.get (configuration.fieldNumber ("inputms"), inputms);

	// Create MSTransformIterator pointing to reference file
	configuration.define("inputms", inputms + String(".ref"));
	MSTransformIteratorFactory refFactory(configuration);
	VisibilityIterator2 refTVI(refFactory);

	// Use MSTransformFactory to create a plain input VII
	configuration.define("inputms", inputms);
	MSTransformIteratorFactory plainVIFactory(configuration);
	ViImplementation2 *inputVI = plainVIFactory.getInputVI()->getImpl();

	// Generate TVI to test
	ChannelAverageTVIFactory testFactory(configuration,inputVI);
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
	columns += Frecuencies;

	// Compare
	res = compareVisibilityIterators(testTVI,refTVI,columns,tolerance);

	return res;
}

void propagateFlags(Record configuration)
{
	// Get original inputms name
	String inputms;
	configuration.get (configuration.fieldNumber ("inputms"), inputms);

	// Create MSTransformIterator pointing to reference file
	configuration.define("inputms", inputms + String(".ref"));
	MSTransformIteratorFactory refFactory(configuration);
	VisibilityIterator2 refTVI(refFactory);

	// Use MSTransformFactory to create a plain input VII
	configuration.define("inputms", inputms);
	MSTransformIteratorFactory plainVIFactory(configuration);
	ViImplementation2 *inputVI = plainVIFactory.getInputVI()->getImpl();

	// Generate TVI to test
	ChannelAverageTVIFactory testFactory(configuration,inputVI);
	VisibilityIterator2 testTVI(testFactory);

	// Propagate flags with MSTransformIterator
	propagateFlags(refTVI);

	// Propagate flags with TVI to test
	propagateFlags(testTVI);

	return;
}


Bool test_comparePropagatedFlags(Record configuration)
{
	// Declare working variables
	Bool res;
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Propagate flags
	propagateFlags(configuration);

	// Get original inputms name
	String inputms;
	configuration.get (configuration.fieldNumber ("inputms"), inputms);

	// Use MSTransformIteratorFactory to create a plain input VI pointing to the test file
	configuration.define("inputms", inputms);
	MSTransformIteratorFactory testFactory(configuration);
	VisibilityIterator2 *testTVI = testFactory.getInputVI();

	// Use MSTransformIteratorFactory to create a plain input VI pointing to the reference file
	configuration.define("inputms", inputms + String(".ref"));
	MSTransformIteratorFactory refFactory(configuration);
	VisibilityIterator2 *refTVI = refFactory.getInputVI();

	// Determine columns to check
	VisBufferComponents2 columns;
	columns += FlagCube;

	// Compare
	res = compareVisibilityIterators(*testTVI,*refTVI,columns,tolerance);

	return res;
}


int main(int argc, char **argv)
{
	// Read configuration
	Record configuration = parseConfiguration(argc, argv);

	// Run test
	Bool result = True;
	result &= test_compareTransformedData(configuration);
	result &= test_comparePropagatedFlags(configuration);

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

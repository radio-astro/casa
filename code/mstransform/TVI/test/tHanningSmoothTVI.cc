//# tHanningSmoothTVI: This file contains the unit tests of the HanningSmoothTVI class.
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


#include <mstransform/TVI/test/tHanningSmoothTVI.h>

using namespace std;
using namespace casa;
using namespace casa::vi;


//////////////////////////////////////////////////////////////////////////
// HanningSmoothTVITest class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void HanningSmoothTVITest::generateTestFile()
{
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,testFile_p));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void HanningSmoothTVITest::generateReferenceFile()
{
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,referenceFile_p));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void HanningSmoothTVITest::initTestConfiguration(Record &configuration)
{
	testConfiguration_p = configuration;
	testConfiguration_p.define ("inputms", testFile_p);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void HanningSmoothTVITest::initReferenceConfiguration(Record &configuration)
{
	refConfiguration_p = configuration;
	refConfiguration_p.define ("inputms", referenceFile_p);
	refConfiguration_p.define ("reindex", False);
	refConfiguration_p.define ("hanning", True);
	refConfiguration_p.define ("datacolumn", String("ALL"));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
HanningSmoothTVITest::HanningSmoothTVITest(): FreqAxisTVITest ()
{
	inpFile_p = String("Four_ants_3C286.ms");
    testFile_p = String("Four_ants_3C286.ms.test");
    referenceFile_p = String("Four_ants_3C286.ms.ref");

    Record configuration;
    configuration.define ("spw", "1");

	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
HanningSmoothTVITest::HanningSmoothTVITest(Record configuration): FreqAxisTVITest(configuration)
{
	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void HanningSmoothTVITest::TestBody()
{
	SetUp();
	testCompareTransformedData();
	TearDown();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void HanningSmoothTVITest::testCompareTransformedData()
{
	// Declare working variables
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Create MSTransformIterator pointing to reference file
	MSTransformIteratorFactory refFactory(refConfiguration_p);
	VisibilityIterator2 refTVI(refFactory);

	// Use MSTransformFactory to create a plain input VII
	MSTransformIteratorFactory plainVIFactory(testConfiguration_p);
	ViImplementation2 *inputVI = plainVIFactory.getInputVI()->getImpl();

	// Generate TVI to test
	HanningSmoothTVIFactory testFactory(testConfiguration_p,inputVI);
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
	Bool res = compareVisibilityIterators(testTVI,refTVI,columns,tolerance);

	// Store result
	if (not res) testResult_p = res;

	// Trigger google test macro
	ASSERT_TRUE(res);

	return;
}

//////////////////////////////////////////////////////////////////////////
// Googletest macros
//////////////////////////////////////////////////////////////////////////
TEST_F(HanningSmoothTVITest, testCompareTransformedData)
{
	testCompareTransformedData();
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int ret;
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
	}

	if (autoMode)
	{
		::testing::InitGoogleTest(&argc, argv);
		ret = RUN_ALL_TESTS();
	}
	else
	{
		HanningSmoothTVITest test(configuration);
		test.TestBody();
		if (test.getTestResult()) ret = 0;
		else ret = 1;
	}

	return ret;
}

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


#include <mstransform/TVI/test/tChannelAverageTVI.h>

using namespace std;
using namespace casa;
using namespace casa::vi;


//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVITest class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::generateTestFile()
{
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,testFile_p));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::generateReferenceFile()
{
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,referenceFile_p));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::initTestConfiguration(Record &configuration)
{
	testConfiguration_p = configuration;
	testConfiguration_p.define ("inputms", testFile_p);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::initReferenceConfiguration(Record &configuration)
{
	refConfiguration_p = configuration;
	refConfiguration_p.define ("inputms", referenceFile_p);
	refConfiguration_p.define ("reindex", False);
	refConfiguration_p.define ("chanaverage", True);
	refConfiguration_p.define ("datacolumn", String("ALL"));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ChannelAverageTVITest::ChannelAverageTVITest(): FreqAxisTVITest ()
{
	inpFile_p = String("Four_ants_3C286.ms");
    testFile_p = String("Four_ants_3C286.ms.test");
    referenceFile_p = String("Four_ants_3C286.ms.ref");

    Record configuration;
    configuration.define ("spw", "1");
    configuration.define ("chanbin", 8);

	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
ChannelAverageTVITest::ChannelAverageTVITest(Record configuration): FreqAxisTVITest(configuration)
{
	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::TestBody()
{
	SetUp();
	testCompareTransformedData();
	TearDown();

	SetUp();
	testComparePropagatedFlags();
	TearDown();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::testCompareTransformedData()
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
	ChannelAverageTVIFactory testFactory(testConfiguration_p,inputVI);
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
	Bool res = compareVisibilityIterators(testTVI,refTVI,columns,tolerance);

	// Store result
	if (not res) testResult_p = res;

	// Trigger google test macro
	ASSERT_TRUE(res);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::testComparePropagatedFlags()
{
	// Declare working variables
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Propagate flags
	propagateFlags();

	// Use MSTransformIteratorFactory to create a plain input VI pointing to the test file
	MSTransformIteratorFactory testFactory(testConfiguration_p);
	VisibilityIterator2 *testTVI = testFactory.getInputVI();

	// Use MSTransformIteratorFactory to create a plain input VI pointing to the reference file
	MSTransformIteratorFactory refFactory(refConfiguration_p);
	VisibilityIterator2 *refTVI = refFactory.getInputVI();

	// Determine columns to check
	VisBufferComponents2 columns;
	columns += FlagCube;

	// Compare
	Bool res = compareVisibilityIterators(*testTVI,*refTVI,columns,tolerance);

	// Store result
	if (not res) testResult_p = res;

	// Trigger google test macro
	ASSERT_TRUE(res);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void ChannelAverageTVITest::propagateFlags()
{
	// Create MSTransformIterator pointing to reference file
	MSTransformIteratorFactory refFactory(refConfiguration_p);
	VisibilityIterator2 refTVI(refFactory);

	// Use MSTransformFactory to create a plain input VII
	MSTransformIteratorFactory plainVIFactory(testConfiguration_p);
	ViImplementation2 *inputVI = plainVIFactory.getInputVI()->getImpl();

	// Generate TVI to test
	ChannelAverageTVIFactory testFactory(testConfiguration_p,inputVI);
	VisibilityIterator2 testTVI(testFactory);

	// Propagate flags with MSTransformIterator
	flagEachOtherChannel(refTVI);

	// Propagate flags with TVI to test
	flagEachOtherChannel(testTVI);

	return;
}

//////////////////////////////////////////////////////////////////////////
// Googletest macros
//////////////////////////////////////////////////////////////////////////
TEST_F(ChannelAverageTVITest, testCompareTransformedData)
{
	testCompareTransformedData();
}

TEST_F(ChannelAverageTVITest, testComparePropagatedFlags)
{
	testComparePropagatedFlags();
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
		else if (parameter == string("-chanbin"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("chanbin", tmp);
		}
	}

	if (autoMode)
	{
		::testing::InitGoogleTest(&argc, argv);
		ret = RUN_ALL_TESTS();
	}
	else
	{
		ChannelAverageTVITest test(configuration);
		test.TestBody();
		if (test.getTestResult()) ret = 0;
		else ret = 1;
	}

	return ret;
}

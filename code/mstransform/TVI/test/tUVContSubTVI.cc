//# tUVContSubTVI: This file contains the unit tests of the UVContSubTVI class.
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


#include <mstransform/TVI/test/tUVContSubTVI.h>

using namespace std;
using namespace casa;
using namespace casa::vi;


//////////////////////////////////////////////////////////////////////////
// UVContSubTVITest class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVITest::generateTestFile()
{
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,testFile_p));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVITest::generateReferenceFile()
{
	// Set path
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,inpFile_p));

	// Get parameters
	uInt fitorder;
	String datacolumn;
	String fitspw;
	Bool want_cont;
	refConfiguration_p.get (refConfiguration_p.fieldNumber ("fitorder"), fitorder);
	refConfiguration_p.get (refConfiguration_p.fieldNumber ("datacolumn"), datacolumn);
	refConfiguration_p.get (refConfiguration_p.fieldNumber ("fitspw"), fitspw);
	refConfiguration_p.get (refConfiguration_p.fieldNumber ("want_cont"), want_cont);

	// Run uvcontsub (uvcontsub3 implementation)
	try
	{
		MeasurementSet ms(inpFile_p);
		SubMS subtractor(ms);
		subtractor.setFitOrder(fitorder);
		subtractor.setFitSpw(fitspw);
		subtractor.setWantCont(want_cont);
		subtractor.setmsselect();
		subtractor.setTVIDebug(True);
		subtractor.makeSubMS(referenceFile_p, datacolumn);
	}
	catch (AipsError ex)
	{
		LogIO logger;
		logger 	<< LogIO::SEVERE
					<< "Exception comparing visibility iterators: " << ex.getMesg() << endl
					<< "Stack Trace: " << ex.getStackTrace()
					<< LogIO::POST;
		RETHROW(ex);
	}

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVITest::initTestConfiguration(Record &configuration)
{
	testConfiguration_p = configuration;
	testConfiguration_p.define ("inputms", testFile_p);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVITest::initReferenceConfiguration(Record &configuration)
{
	refConfiguration_p = configuration;
	refConfiguration_p.define ("inputms", referenceFile_p);

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
UVContSubTVITest::UVContSubTVITest(): FreqAxisTVITest ()
{
	inpFile_p = String("Four_ants_3C286.ms");
    testFile_p = String("Four_ants_3C286.test");
    referenceFile_p = String("Four_ants_3C286.ms.ref");

    Record configuration;
    //configuration.define ("spw", "0");
    //configuration.define ("scan", "30");
    configuration.define ("fitorder", 1);
    configuration.define ("want_cont", False);
    configuration.define ("fitspw", "*:0~30");
    configuration.define ("datacolumn", String("CORRECTED_DATA"));

	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
UVContSubTVITest::UVContSubTVITest(Record configuration): FreqAxisTVITest(configuration)
{
	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVITest::TestBody()
{
	SetUp();
	testCompareTransformedData();
	TearDown();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void UVContSubTVITest::testCompareTransformedData()
{
	// Declare working variables
	Float tolerance = 1E-4; // FLT_EPSILON is 1.19209290e-7F

	// Generate configuration record for reference file
    Record refConfiguration;
    refConfiguration.define ("datacolumn", String("DATA"));
    refConfiguration.define ("inputms", referenceFile_p);

    // Prepare datacolmap
    dataColMap datacolmap;
    datacolmap[MS::CORRECTED_DATA] =  MS::DATA;

	// Use MSTransformFactory to create a plain input VII
	MSTransformIteratorFactory refFactory(refConfiguration);
	VisibilityIterator2 refTVI(refFactory);

	// Generate TVI to test
	MSTransformIteratorFactory testPlainVIFactory(testConfiguration_p);
	ViImplementation2 *testInputVI = testPlainVIFactory.getInputVI()->getImpl();
	UVContSubTVIFactory testFactory(testConfiguration_p,testInputVI);
	VisibilityIterator2 testTVI(testFactory);

	// Determine columns to check
	VisBufferComponents2 columns;
	columns += VisBufferComponent2::VisibilityCubeCorrected;
	columns += VisBufferComponent2::FlagCube;

	// Compare
	Bool res = compareVisibilityIterators(testTVI,refTVI,columns,tolerance,&datacolmap);

	// Store result
	if (not res) testResult_p = res;

	// Trigger google test macro
	ASSERT_TRUE(res);

	return;
}

//////////////////////////////////////////////////////////////////////////
// Googletest macros
//////////////////////////////////////////////////////////////////////////
TEST_F(UVContSubTVITest, testCompareTransformedData)
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
		UVContSubTVITest test(configuration);
		test.TestBody();
		if (test.getTestResult()) ret = 0;
		else ret = 1;
	}

	return ret;
}

//# tStatWtTVI: This file contains the unit tests of the StatWtTVI class.
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


#include <mstransform/TVI/test/tStatWtTVI.h>

#include <mstransform/MSTransform/MSTransformIteratorFactory.h>
#include <mstransform/TVI/StatWtTVIFactory.h>
#include <mstransform/TVI/StatWtTVILayerFactory.h>

using namespace std;
using namespace casa;
using namespace casacore;
using namespace casa::vi;

StatWtTVITest::StatWtTVITest(): FreqAxisTVITest () {
    inpFile_p = "ngc5921.split.ms";
    testFile_p = "ngc5921.split.ms.test";
    referenceFile_p = "ngc5921.split.ms.ref";
    Record configuration;
	init(configuration);
}

StatWtTVITest::StatWtTVITest(Record configuration): FreqAxisTVITest(configuration) {
	init(configuration);
}

void StatWtTVITest::generateTestFile() {
    String path = autoMode_p ? "/data/regression/unittest/simplecluster" : "";
    ASSERT_TRUE(copyTestFile(path, inpFile_p, testFile_p));
}

void StatWtTVITest::generateReferenceFile() {
    String path = autoMode_p ? "/data/regression/unittest/statwt" : "";
    ASSERT_TRUE(copyTestFile(path, referenceFile_p, referenceFile_p));
}

void StatWtTVITest::initTestConfiguration(Record &configuration) {
    testConfiguration_p = configuration;
    testConfiguration_p.define("inputms", testFile_p);
}

void StatWtTVITest::initReferenceConfiguration(Record &configuration) {
    refConfiguration_p = configuration;
    refConfiguration_p.define ("inputms", referenceFile_p);
}

void StatWtTVITest::TestBody() {
    SetUp();
    testCompareTransformedData();
    TearDown();
}

void StatWtTVITest::testCompareTransformedData() {
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Create MSTransformIterator pointing to reference file
	/*
	refConfiguration_p.define("factory",False);
	MSTransformIteratorFactory refFactory(refConfiguration_p);
	VisibilityIterator2 refTVI(refFactory);
	*/
    /*
	// Use MSTransformFactory to create a plain input VII
	testConfiguration_p.define("factory",True);
    testConfiguration_p.define ("timebin", "1s");
    MSTransformIteratorFactory plainVIFactory(testConfiguration_p);
	ViImplementation2 *inputVI = plainVIFactory.getInputVI()->getImpl();

    // Generate TVI to test
	StatWtTVIFactory testFactory(testConfiguration_p, inputVI);
    VisibilityIterator2 testTVI(testFactory);
    */
    // no binning
    vi::IteratingParameters ipar(0.001);
	MeasurementSet msref(referenceFile_p);
	vi::VisIterImpl2LayerFactory dataRef(&msref, ipar, True);
	Vector<vi::ViiLayerFactory*> factsRef(1, &dataRef);
	vi::VisibilityIterator2 refTVI(factsRef);

    MeasurementSet mstest(testFile_p);
    vi::VisIterImpl2LayerFactory data(&mstest, ipar, True);
    Record config;
    vi::StatWtTVILayerFactory statWtLayerFactory(config);
    Vector<vi::ViiLayerFactory*> factsTest(2);
    factsTest[0] = &data;
    factsTest[1] = &statWtLayerFactory;
    vi::VisibilityIterator2 testTVI(factsTest);
	// Determine columns to check
	VisBufferComponents2 columns;
	columns += VisBufferComponent2::NRows;
	columns += VisBufferComponent2::FlagRow;
	columns += VisBufferComponent2::FlagCube;
	columns += VisBufferComponent2::WeightSpectrum;
	columns += VisBufferComponent2::Weight;
    // Compare
	Bool res = compareVisibilityIterators(testTVI,refTVI,columns,tolerance);

	// Store result
	if (not res) {
        testResult_p = res;
    }

	// Trigger google test macro
	ASSERT_TRUE(res);
}

//////////////////////////////////////////////////////////////////////////
// Googletest macros
//////////////////////////////////////////////////////////////////////////
TEST_F(StatWtTVITest, testCompareTransformedData)
{
	testCompareTransformedData();
}

//////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
	int ret;
	string parameter, value;
	Record configuration;
	Bool autoMode = true;

	for (unsigned short i=0; i<argc-1; ++i) {
		parameter = string(argv[i]);
		value = string(argv[i+1]);
		if (parameter == string("-vis")) {
			configuration.define ("inputms", value);
			autoMode = false;
		}
		else if (parameter == string("-spw")) {
			configuration.define ("spw", value);
		}
	}
	if (autoMode) {
		::testing::InitGoogleTest(&argc, argv);
		ret = RUN_ALL_TESTS();
	}
	else {
        StatWtTVITest test(configuration);
        test.TestBody();
        ret = test.getTestResult() ? 0: 1;
	}
	return ret;
}

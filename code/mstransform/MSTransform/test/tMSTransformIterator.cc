//# tMSTransformIterator: This file contains the unit tests of the MSTransformIterator class.
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

#include <mstransform/MSTransform/test/tMSTransformIterator.h>

using namespace std;
using namespace casa;
using namespace casa::vi;

//////////////////////////////////////////////////////////////////////////
// MSTransformIteratorTest class
//////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorTest::generateTestFile()
{
	String path("");
	if (autoMode_p) path = String("/data/regression/unittest/flagdata/");
	ASSERT_TRUE(copyTestFile(path,inpFile_p,testFile_p));

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorTest::generateReferenceFile()
{
	if (autoMode_p)
	{
		String path("/data/regression/unittest/flagdata/");
		ASSERT_TRUE(copyTestFile(path,inpFile_p,inpFile_p));
	}

	MSTransformManager manager(refConfiguration_p);
	manager.open();
	manager.setup();

	vi::VisibilityIterator2 *visIter = manager.getVisIter();
	vi::VisBuffer2 *vb = visIter->getVisBuffer();
	visIter->originChunks();
	while (visIter->moreChunks())
	{
		visIter->origin();
		while (visIter->more())
		{
			manager.fillOutputMs(vb);
			visIter->next();
		}

		visIter->nextChunk();
	}

	manager.close();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorTest::initTestConfiguration(Record &configuration)
{
	testConfiguration_p = configuration;
	testConfiguration_p.define("inputms", testFile_p);
	testConfiguration_p.define("reindex", False);
	testConfiguration_p.define("datacolumn", string("ALL"));;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorTest::initReferenceConfiguration(Record &configuration)
{
	refConfiguration_p = configuration;
	refConfiguration_p.define("inputms",inpFile_p);
	refConfiguration_p.define("outputms",referenceFile_p);
	refConfiguration_p.define("reindex", False);
	refConfiguration_p.define("realmodelcol",True);
	refConfiguration_p.define("usewtspectrum",True);
	refConfiguration_p.define("datacolumn", string("ALL"));;

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIteratorTest::MSTransformIteratorTest(): FreqAxisTVITest ()
{
	inpFile_p = String("Four_ants_3C286.ms");
    testFile_p = String("Four_ants_3C286.ms.test");
    referenceFile_p = String("Four_ants_3C286.ms.ref");

    Record configuration;
	configuration.define ("spw", "1");
	configuration.define ("correlation", "RR,LL");
	configuration.define ("timeaverage", True);
	configuration.define ("timebin", "4s");
	configuration.define ("chanaverage", True);
	configuration.define ("chanbin", 8);

	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIteratorTest::MSTransformIteratorTest(Record configuration): FreqAxisTVITest(configuration)
{
	init(configuration);
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
MSTransformIteratorTest::~MSTransformIteratorTest()
{
	if (autoMode_p)
	{
		String rm_command;
		rm_command = String ("rm -rf ") + inpFile_p;
		system(rm_command.c_str());
	}
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorTest::TestBody()
{
	SetUp();
	testCompareTransformedData();
	TearDown();

	return;
}

// -----------------------------------------------------------------------
//
// -----------------------------------------------------------------------
void MSTransformIteratorTest::testCompareTransformedData()
{
	// Declare working variables
	Float tolerance = 1E-5; // FLT_EPSILON is 1.19209290e-7F

	// Create a plain iterator pointing to reference file
	Block<Int> sortCols(7);
	sortCols[0] = MS::OBSERVATION_ID;
	sortCols[1] = MS::ARRAY_ID;
	sortCols[2] = MS::SCAN_NUMBER;
	sortCols[3] = MS::STATE_ID;
	sortCols[4] = MS::FIELD_ID;
	sortCols[5] = MS::DATA_DESC_ID;
	sortCols[6] = MS::TIME;
	MeasurementSet ms(referenceFile_p,Table::Old);
	vi::VisibilityIterator2 refTVI(ms,vi::SortColumns (sortCols,false));

	// Get MSTransformIterator
	MSTransformIteratorFactory testFactory(testConfiguration_p);
	vi::VisibilityIterator2 testTVI(testFactory);

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

//////////////////////////////////////////////////////////////////////////
// Googletest macros
//////////////////////////////////////////////////////////////////////////
TEST_F(MSTransformIteratorTest, testCompareTransformedData)
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
		else if (parameter == string("-field"))
		{
			configuration.define ("field", value);
		}
		else if (parameter == string("-spw"))
		{
			configuration.define ("spw", value);
		}
		else if (parameter == string("-scan"))
		{
			configuration.define ("scan", value);
		}
		else if (parameter == string("-antenna"))
		{
			configuration.define ("antenna", value);
		}
		else if (parameter == string("-correlation"))
		{
			configuration.define ("correlation", value);
		}
		else if (parameter == string("-timerange"))
		{
			configuration.define ("timerange", value);
		}
		else if (parameter == string("-intent"))
		{
			configuration.define ("intent", value);
		}
		else if (parameter == string("-array"))
		{
			configuration.define ("array", value);
		}
		else if (parameter == string("-uvrange"))
		{
			configuration.define ("uvrange", value);
		}
		else if (parameter == string("-observation"))
		{
			configuration.define ("observation", value);
		}
		else if (parameter == string("-feed"))
		{
			configuration.define ("feed", value);
		}
		else if (parameter == string("-datacolumn"))
		{
			configuration.define ("datacolumn", value);
		}
		else if (parameter == string("-combinespws"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("combinespws", tmp);
		}
		else if (parameter == string("-chanaverage"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("chanaverage", tmp);
		}
		else if (parameter == string("-chanbin"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("chanbin", tmp);
		}
		else if (parameter == string("-useweights"))
		{
			configuration.define ("useweights", value);
		}
		else if (parameter == string("-hanning"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("hanning", tmp);
		}
		else if (parameter == string("-regridms"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("regridms", tmp);
		}
		else if (parameter == string("-mode"))
		{
			configuration.define ("mode", value);
		}
		else if (parameter == string("-nchan"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("nchan", tmp);
		}
		else if (parameter == string("-start"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("start", tmp);
		}
		else if (parameter == string("-width"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("width", tmp);
		}
		else if (parameter == string("-nspw"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("nspw", tmp);
		}
		else if (parameter == string("-interpolation"))
		{
			configuration.define ("interpolation", value);
		}
		else if (parameter == string("-phasecenter"))
		{
			configuration.define ("phasecenter", value);
		}
		else if (parameter == string("-restfreq"))
		{
			configuration.define ("restfreq", value);
		}
		else if (parameter == string("-outframe"))
		{
			configuration.define ("outframe", value);
		}
		else if (parameter == string("-veltype"))
		{
			configuration.define ("veltype", value);
		}
		else if (parameter == string("-timeaverage"))
		{
			Bool tmp = Bool(atoi(value.c_str()));
			configuration.define ("timeaverage", tmp);
		}
		else if (parameter == string("-timebin"))
		{
			configuration.define ("timebin", value);
		}
		else if (parameter == string("-timespan"))
		{
			configuration.define ("timespan", value);
		}
		else if (parameter == string("-maxuvwdistance"))
		{
			Double tmp = Double(atof(value.c_str()));
			configuration.define ("maxuvwdistance", tmp);
		}
		else if (parameter == string("-minbaselines"))
		{
			Int tmp = Int(atoi(value.c_str()));
			configuration.define ("minbaselines", tmp);
		}
		else if (parameter == string("-callib"))
		{
			Float calfactor = Float(atof(value.c_str()));
			Record calrec;
			calrec.define("calfactor",calfactor);
			configuration.defineRecord ("callib", calrec);
		}

	}

	if (autoMode)
	{
		::testing::InitGoogleTest(&argc, argv);
		ret = RUN_ALL_TESTS();
	}
	else
	{
		MSTransformIteratorTest test(configuration);
		test.TestBody();
		if (test.getTestResult()) ret = 0;
		else ret = 1;
	}

	return ret;
}

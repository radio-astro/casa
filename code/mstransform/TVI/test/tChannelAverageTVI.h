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

#ifndef ChannelAverageTVITest_H_
#define ChannelAverageTVITest_H_

#include <gtest/gtest.h>
#include <mstransform/TVI/test/TestUtilsTVI.h>
#include <mstransform/TVI/ChannelAverageTVI.h>
#include <mstransform/MSTransform/MSTransformIteratorFactory.h>

using namespace std;
using namespace casa;
using namespace casa::vi;


//////////////////////////////////////////////////////////////////////////
// FreqAxisTVITest class
//////////////////////////////////////////////////////////////////////////
class FreqAxisTVITest: public ::testing::Test {

public:

	FreqAxisTVITest();
	FreqAxisTVITest(Record configuration);
    virtual ~FreqAxisTVITest();

    void SetUp();
    void TearDown();
    Bool getTestResult() {return testResult_p;}

protected:

    void autoInit();
    void customInit(Record configuration);

    virtual void generateTestFile() = 0;
    virtual void initTestConfiguration() = 0;
    virtual void initReferenceConfiguration() = 0;


    Bool autoMode_p;
    Bool testResult_p;
    String inputMS_p;
    Record refConfiguration_p;
    Record testConfiguration_p;
};

//////////////////////////////////////////////////////////////////////////
// ChannelAverageTVITest class
//////////////////////////////////////////////////////////////////////////
class ChannelAverageTVITest: public FreqAxisTVITest {

public:

	ChannelAverageTVITest();
	ChannelAverageTVITest(Record configuration);

    void TestBody();
    void testCompareTransformedData();
    void testComparePropagatedFlags();

protected:

    void propagateFlags();
    void generateTestFile();
    void initTestConfiguration();
    void initReferenceConfiguration();
};

#endif /* ChannelAverageTVITest_H_ */

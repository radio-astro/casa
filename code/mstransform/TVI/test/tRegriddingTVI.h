//# tRegriddingTVI: This file contains the unit tests of the RegriddingTVI class.
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

#ifndef RegriddingTVITest_H_
#define RegriddingTVITest_H_

#include <mstransform/TVI/test/TestUtilsTVI.h>
#include <mstransform/TVI/RegriddingTVI.h>
#include <mstransform/MSTransform/MSTransformIteratorFactory.h>

using namespace std;
using namespace casa;
using namespace casa::vi;


/////////////////////////////////////////////////////////////////////////
// RegriddingTVITest class
/////////////////////////////////////////////////////////////////////////
class RegriddingTVITest: public FreqAxisTVITest {

public:

	RegriddingTVITest();
	RegriddingTVITest(Record configuration);

    void TestBody();
    void testCompareTransformedData();

protected:

    void propagateFlags();
    void generateTestFile();
    void generateReferenceFile();
    void initTestConfiguration(Record &configuration);
    void initReferenceConfiguration(Record &configuration);
};

#endif /* RegriddingTVITest_H_ */

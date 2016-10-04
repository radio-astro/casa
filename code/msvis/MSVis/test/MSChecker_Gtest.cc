#include <gtest/gtest.h>

#include <msvis/MSVis/MSChecker.h>

#include <casacore/casa/Containers/ValueHolder.h>
#include <casacore/tables/Tables/TableProxy.h>
#include <iostream>
#include <msvis/MSVis/test/MSCheckerTest.h>

using namespace std;

using namespace casacore;
using namespace casa;

MeasurementSet _testMS;

namespace test {


TEST_F(MSCheckerTest, CheckIntegrityTestClean) {
	MSChecker checker(*testMS);
	// not throwing an exception indicates this passes
	checker.checkReferentialIntegrity();
}

TEST_F(MSCheckerTest, CheckIntegrityTestBadDDID) {
	TableProxy proxy(*testMS);
	proxy.putCell("DATA_DESC_ID", Vector<Int>(1, 20), ValueHolder(9));
	MSChecker checker(*testMS);
	EXPECT_THROW(checker.checkReferentialIntegrity(), AipsError);
}

TEST_F(MSCheckerTest, CheckIntegrityTestBadFieldID) {
	TableProxy proxy(*testMS);
	proxy.putCell("FIELD_ID", Vector<Int>(1, 20), ValueHolder(9));
	MSChecker checker(*testMS);
	EXPECT_THROW(checker.checkReferentialIntegrity(), AipsError);
}

TEST_F(MSCheckerTest, CheckIntegrityTestBadAntenna1) {
	TableProxy proxy(*testMS);
	proxy.putCell("ANTENNA1", Vector<Int>(1, 20), ValueHolder(9));
	MSChecker checker(*testMS);
	EXPECT_THROW(checker.checkReferentialIntegrity(), AipsError);
}

TEST_F(MSCheckerTest, CheckIntegrityTestBadAntenna2) {
	TableProxy proxy(*testMS);
	proxy.putCell("ANTENNA2", Vector<Int>(1, 20), ValueHolder(9));
	MSChecker checker(*testMS);
	EXPECT_THROW(checker.checkReferentialIntegrity(), AipsError);
}
}

int main (int nArgs, char * args []) {
    ::testing::InitGoogleTest(& nArgs, args);
    cout << "MSChecker test " << endl;
    return RUN_ALL_TESTS();
}

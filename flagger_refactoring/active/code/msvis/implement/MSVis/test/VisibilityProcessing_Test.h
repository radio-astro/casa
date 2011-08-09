/*
 * VisibilityProcessing_Test.h
 *
 *  Created on: May 3, 2011
 *      Author: jjacobs
 */

#ifndef VISIBILITYPROCESSING_TEST_H_
#define VISIBILITYPROCESSING_TEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/TestResult.h>

using namespace CppUnit;

namespace casa {

namespace vpf {

class VpTestListener : public TextTestProgressListener {
public:

    VpTestListener (bool verbose = False);
    void addFailure (const TestFailure & failure);
    void endTestRun (Test * test, TestResult * eventManager);
    void startTest (Test * test);

private:

    bool errorsOccurred_p;
    bool verbose_p;
};

//class VisibilityProcessing_Test : public CppUnit::TestFixture {
//
//public:
//
//    CPPUNIT_TEST_SUITE (VisibilityProcessing_Test);
//
//    CPPUNIT_TEST (test_something);
//
//    CPPUNIT_TEST_SUITE_END ();
//
//    void
//    setup ()
//    {
//    }
//
//    void
//    teardown ()
//    {
//    }
//
//    void
//    test_something ()
//    {
//    }
//
//};

class SubChunkIndex_Test : public CppUnit::TestFixture {


    CPPUNIT_TEST_SUITE (SubChunkIndex_Test);

    CPPUNIT_TEST (testConstruction);
    CPPUNIT_TEST (testComparisons);

    CPPUNIT_TEST_SUITE_END ();

public:

    void testComparisons ();
    void testConstruction ();
};


class VisibilityProcessor_Test: public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE (VisibilityProcessor_Test);

    CPPUNIT_TEST (testConstruction);

    CPPUNIT_TEST_SUITE_END ();

public:

    void testConstruction ();

};


//class VpContainer_Test : public CppUnit::TestFixture {
//
//public:
//
//    CPPUNIT_TEST_SUITE (VisibilityProcessing_Test);
//
//    //CPPUNIT_TEST ();
//
//    CPPUNIT_TEST_SUITE_END ();
//
//};

class VpData_Test : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE (VpData_Test);

    CPPUNIT_TEST (testConstruction);
    CPPUNIT_TEST (testMethods);

    CPPUNIT_TEST_SUITE_END ();

public:

    void testConstruction ();
    void testMethods ();

};

class VpNoop : public VisibilityProcessor {

public:

    VpNoop (const String & name,
            const vector<String> & inputNames,
            const vector<String> & outputNames)
    : VisibilityProcessor (name, inputNames, outputNames)
    {}

    ~VpNoop () {}

    ProcessingResult doProcessing (ProcessingType /* processingType */,
                                   VpData & /* inputData */,
                                   const SubChunkIndex & /* subChunkIndex */)
    {return ProcessingResult();}

    void validate (const VpPorts & /* inputs */, const VpPorts & /* outputs */)
    {}
};


class  VpSimpleNoop : public SimpleVp {

protected:

    SimpleResult doProcessing (ProcessingType processType,
                               VisBuffer * vb,
                               const SubChunkIndex & subChunkIndex);
};

class VpEngine_Test : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE (VpEngine_Test);

    //CPPUNIT_TEST (testConstruction);

    CPPUNIT_TEST_SUITE_END ();

public:


};

class VpPort_Test : public CppUnit::TestFixture {

public:

    CPPUNIT_TEST_SUITE (VpPort_Test);

    CPPUNIT_TEST (testConstruction);
    CPPUNIT_TEST (testComparisons);
    CPPUNIT_TEST (testConnection);

    CPPUNIT_TEST_SUITE_END ();

public:

    void testComparisons();
    void testConnection();
    void testConstruction();

};

class VpPorts_Test : public CppUnit::TestFixture {

public:

    CPPUNIT_TEST_SUITE (VpPorts_Test);

    CPPUNIT_TEST (testMethods);

    CPPUNIT_TEST_SUITE_END ();

public:

    void
    testMethods();


};

class VbPtr_Test : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE (VbPtr_Test);

    CPPUNIT_TEST (testConstruction);
    CPPUNIT_TEST (testAssignment);
    CPPUNIT_TEST (testDestruction);
    CPPUNIT_TEST (testOperators);

    CPPUNIT_TEST_SUITE_END ();

public:

    void setup ();
    void tearDown ();

    void testAssignment ();
    void testDestruction ();
    void testConstruction ();
    void testOperators ();

private:

    VisBuffer * vb_p;

};

} // end namespace vpf

} // end namespace casa


#endif /* VISIBILITYPROCESSING_TEST_H_ */

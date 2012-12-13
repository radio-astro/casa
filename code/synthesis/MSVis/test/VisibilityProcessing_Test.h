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
#include <boost/program_options.hpp>
#include <memory>


namespace po = boost::program_options;


using namespace CppUnit;

int main (int argc, char * args []);

namespace casa {

namespace vpf {

class ErrorTest {
public:

    void reset ();

private:

    static bool x;
};


class VpTests {

    friend int ::main (int argc, char * args []);

public:


    const po::variables_map & getArguments () const;

    static VpTests & singleton ();

    static const casa::String Visibility;

protected:

    int parseArguments (int argc, char * args []);
    int run (int argc, char * args []);


private:

    VpTests ();

    std::auto_ptr<po::variables_map> vm_p;

};

class VpTestListener : public TextTestProgressListener {
public:

    VpTestListener (bool verbose = False);
    void addFailure (const TestFailure & failure);
    void endTest (Test * test);
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

class SubchunkIndex_Test : public CppUnit::TestFixture {


    CPPUNIT_TEST_SUITE (SubchunkIndex_Test);

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


class VpContainer_Test : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE (VpContainer_Test);

    CPPUNIT_TEST (testSimpleSweep);
    CPPUNIT_TEST (testDoubleSweep);

    CPPUNIT_TEST_SUITE_END ();

public:

    void testDoubleSweep ();
    void testSimpleSweep ();

protected:

    void testSweep (Int nChunkSweeps);
};

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
            const vector<String> & outputNames,
            Int nChunkSweeps)
    : VisibilityProcessor (name, inputNames, outputNames),
      nChunkSweeps_p (nChunkSweeps)
    {}

    ~VpNoop () {}

    ProcessingResult doProcessingImpl (ProcessingType processingType,
                                       VpData & /* inputData */,
                                       const SubchunkIndex & subchunkIndex )
    {
        cout << "VpNoop::doProcessing: " << processingType << " on subchunk: " << subchunkIndex.toString() << endl;


        if (processingType ==  EndOfChunk && subchunkIndex.getIteration () < nChunkSweeps_p - 1){
            return ProcessingResult (RepeatChunk, VpData ());
        }
        else{
            return ProcessingResult();
        }
    }

    void validateImpl ()
    {}

private:

    Int nChunkSweeps_p;
};


//class  VpSimpleNoop : public SimpleVp {
//
//protected:
//
//    SimpleResult doProcessing (ProcessingType processType,
//                               VisBuffer * vb,
//                               const SubchunkIndex & subchunkIndex);
//};

class VpEngine_Test : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE (VpEngine_Test);

    CPPUNIT_TEST (testNoopProcessor);

    CPPUNIT_TEST_SUITE_END ();

public:

    void testNoopProcessor ();

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

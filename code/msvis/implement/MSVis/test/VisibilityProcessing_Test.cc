/*
 * VisibilityProcessing_Test.cc
 *
 *  Created on: May 3, 2011
 *      Author: jjacobs
 */

#include "../VisibilityProcessing.h"
#include "VisibilityProcessing_Test.h"

#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/ui/text/TestRunner.h>
#include <stdarg.h>
#include <iterator>
#include <msvis/MSVis/UtilJ.h>

using namespace std;
using namespace casa::vpf;

int
main (int /*argc*/, char * /*args*/ [])
{
    CppUnit::TextUi::TestRunner runner;

    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

    runner.addTest (registry.makeTest());

    CppUnit::TestResult controller;
    VpTestListener listener (true);
    controller.addListener (& listener);

    CppUnit::TestResultCollector collector;
    controller.addListener( & collector);

    runner.run(controller);

    CppUnit::CompilerOutputter outputter (& collector, std::cout);
    outputter.write();

    return 0;
}

namespace casa {

namespace vpf {

CPPUNIT_TEST_SUITE_REGISTRATION (SubChunkIndex_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpPort_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpPorts_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VbPtr_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpData_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VisibilityProcessor_Test);

void
SubChunkIndex_Test::testConstruction ()
{
    {
        SubChunkIndex sci;
        CPPUNIT_ASSERT (sci.getChunkNumber () == SubChunkIndex::Invalid);
        CPPUNIT_ASSERT (sci.getSubChunkNumber () == SubChunkIndex::Invalid);
        CPPUNIT_ASSERT (sci.getIteration () == SubChunkIndex::Invalid);
    }

    {
        SubChunkIndex sci (1);
        CPPUNIT_ASSERT (sci.getChunkNumber () == 1);
        CPPUNIT_ASSERT (sci.getSubChunkNumber () == SubChunkIndex::Invalid);
        CPPUNIT_ASSERT (sci.getIteration () == SubChunkIndex::Invalid);
    }

    {
        SubChunkIndex sci (1,2);
        CPPUNIT_ASSERT (sci.getChunkNumber () == 1);
        CPPUNIT_ASSERT (sci.getSubChunkNumber () == 2);
        CPPUNIT_ASSERT (sci.getIteration () == SubChunkIndex::Invalid);
    }

    {
        SubChunkIndex sci (1,2,3);
        CPPUNIT_ASSERT (sci.getChunkNumber () == 1);
        CPPUNIT_ASSERT (sci.getSubChunkNumber () == 2);
        CPPUNIT_ASSERT (sci.getIteration () == 3);
        CPPUNIT_ASSERT (sci.toString() == "(1,2,3)");
    }
}

void
SubChunkIndex_Test::testComparisons ()
{
    SubChunkIndex sci1 (1,2,3);
    SubChunkIndex sci2 (1,2,3);
    CPPUNIT_ASSERT (sci1 == sci2);
    CPPUNIT_ASSERT (! (sci1 != sci2));
    CPPUNIT_ASSERT (! (sci1 < sci2));
    CPPUNIT_ASSERT (! (sci2 < sci1));

    CPPUNIT_ASSERT (SubChunkIndex (1,10,20) < SubChunkIndex (2,1,2));
    CPPUNIT_ASSERT (SubChunkIndex (1,10,20) < SubChunkIndex (1,11,2));
    CPPUNIT_ASSERT (SubChunkIndex (1,10,20) < SubChunkIndex (1,10,21));
}

void
VpData_Test::testConstruction ()
{
    VpData dataEmpty;
    CPPUNIT_ASSERT (dataEmpty.empty());

    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("vp");
    VpPort port1 (vp1, "In", VpPort::Input);
    VisBuffer * vb1 = new VisBuffer();
    VpData * data = new VpData (port1, vb1);

    CPPUNIT_ASSERT (utilj::containsKey (port1, * data));

    CPPUNIT_ASSERT_THROW (data->add (port1, vb1), AipsError);

    VpPort port2 (vp1, "Out", VpPort::Output);

    VisBuffer * vb2 = new VisBuffer ();
    data->add (port2, vb2);
    CPPUNIT_ASSERT (utilj::containsKey (port1, * data));
    CPPUNIT_ASSERT (utilj::containsKey (port2, * data));

    CPPUNIT_ASSERT_NO_THROW (delete data);

    delete vp1;
}

void
VpData_Test::testMethods ()
{
    // Put three ports with associated data into the VpData object data

    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("vp");

    VpPort port1 (vp1, "In", VpPort::Input);
    VisBuffer * vb1 = new VisBuffer();
    VpData * data = new VpData (port1, vb1);
    CPPUNIT_ASSERT_THROW (data->add (port1, vb1), AipsError);

    VpPort port2 (vp1, "Out", VpPort::Output);
    VisBuffer * vb2 = new VisBuffer ();
    CPPUNIT_ASSERT_NO_THROW (data->add (port2, vb2));

    VisBuffer * vb3 = new VisBuffer ();
    VpPort port3 (vp1, "InOut", VpPort::InOutput);
    CPPUNIT_ASSERT_NO_THROW (data->add (port3, vb3));

    // Pull out an empty selection

    VpPorts portList;
    VpData * dataSelection = new VpData ();
    CPPUNIT_ASSERT_NO_THROW (* dataSelection = data->getSelection (portList));
    CPPUNIT_ASSERT (dataSelection->empty());

    CPPUNIT_ASSERT_NO_THROW (delete dataSelection);

    // Pull out a selection containing port1

    dataSelection = new VpData ();
    portList.push_back(port1);
    CPPUNIT_ASSERT_NO_THROW (* dataSelection = data->getSelection (portList));
    CPPUNIT_ASSERT (dataSelection->size() == 1);
    CPPUNIT_ASSERT (utilj::containsKey (port1, * dataSelection));

    CPPUNIT_ASSERT_NO_THROW (delete dataSelection);

    // Pull out a selection containing port1 and port3

    dataSelection = new VpData ();
    portList.push_back(port3);
    CPPUNIT_ASSERT_NO_THROW (* dataSelection = data->getSelection (portList));
    CPPUNIT_ASSERT (dataSelection->size() == 2);
    CPPUNIT_ASSERT (utilj::containsKey (port1, * dataSelection));
    CPPUNIT_ASSERT (utilj::containsKey (port3, * dataSelection));

    CPPUNIT_ASSERT_NO_THROW (delete dataSelection);

    CPPUNIT_ASSERT_NO_THROW (delete data);

    delete vp1;
}

void
VbPtr_Test::setup ()
{
}

void
VbPtr_Test::tearDown ()
{
}


void
VbPtr_Test::testAssignment ()
{
    VisBuffer * vb = new VisBuffer();

    VbPtr vbPtr (vb, False);

    VbPtr vbPtr2;
    vbPtr2 = vbPtr;

    CPPUNIT_ASSERT (& * vbPtr2 == & * vbPtr);
    CPPUNIT_ASSERT (vbPtr.getNRefs() == 2);
    CPPUNIT_ASSERT (vbPtr2.getNRefs() == 2);

    CPPUNIT_ASSERT_NO_THROW (delete vb);

}

void
VbPtr_Test::testConstruction ()
{
    VisBuffer * vb = new VisBuffer();

    VbPtr vbPtr;
    CPPUNIT_ASSERT (vbPtr.null());

    VbPtr vbPtr2 (vb, False);
    CPPUNIT_ASSERT (& * vbPtr2 == vb);

    Int nRefs = vbPtr2.getNRefs();
    VbPtr vbPtr3 (vbPtr2);
    CPPUNIT_ASSERT (& * vbPtr2 == vb);
    CPPUNIT_ASSERT (& * vbPtr2 == & * vbPtr3);
    CPPUNIT_ASSERT (vbPtr2.getNRefs() == nRefs + 1);

    CPPUNIT_ASSERT_NO_THROW (delete vb);
}

void
VbPtr_Test::testDestruction ()
{
    VbPtr * vp1 = new VbPtr (new VisBuffer ());

    VbPtr * vp2 = new VbPtr (* vp1);
    CPPUNIT_ASSERT (vp2->getNRefs () == 2);

    CPPUNIT_ASSERT_NO_THROW (delete vp2);

    CPPUNIT_ASSERT (vp1->getNRefs () == 1);

    CPPUNIT_ASSERT_NO_THROW (delete vp1);

}

void
VbPtr_Test::testOperators ()
{
    VisBuffer * vb = new VisBuffer();

    VbPtr vbPtr (vb, False);

    CPPUNIT_ASSERT (vbPtr.operator->() == vb);
    CPPUNIT_ASSERT (& vbPtr.operator* () == vb);

    CPPUNIT_ASSERT_NO_THROW (delete vb);
}

void
VpPort_Test::testComparisons()
{
    VpPort port1;
    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("Vp1");
    VpPort port2 (vp1, "In", VpPort::Input);

    CPPUNIT_ASSERT (port2 == port2);
    CPPUNIT_ASSERT (! (port2 < port2));
    CPPUNIT_ASSERT (! (port1 == port2));
    CPPUNIT_ASSERT (! (port2 == port1));
    CPPUNIT_ASSERT (port1 < port2 || port2 < port1);
    CPPUNIT_ASSERT (! (port1 < port2 && port2 < port1));

    delete vp1;
}

void
VpPort_Test::testConnection ()
{
    VpPort port1;

    CPPUNIT_ASSERT_THROW (port1.setConnectedInput(), AipsError);
    CPPUNIT_ASSERT_THROW (port1.setConnectedOutput(), AipsError);

    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("Vp1");
    VpPort port2 (vp1, "In", VpPort::Input);

    port2.setConnectedInput ();
    CPPUNIT_ASSERT (port2.isConnectedInput() && ! port2.isConnectedOutput());

    // It is illegal to doubly connect a port and also to connect an input
    // as an output

    CPPUNIT_ASSERT_THROW (port2.setConnectedInput (), AipsError);
    CPPUNIT_ASSERT_THROW (port2.setConnectedOutput (), AipsError);


    VpPort port3 (vp1, "InOut", VpPort::InOutput);
    port3.setConnectedInput();
    CPPUNIT_ASSERT (port3.isConnectedInput() && ! port3.isConnectedOutput());

    port3.setConnectedOutput();
    CPPUNIT_ASSERT (port3.isConnectedInput() && port3.isConnectedOutput());

    delete vp1;

}

void
VpPort_Test::testConstruction ()
{
    VpPort port1;
    CPPUNIT_ASSERT (port1.empty());

    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("Vp1");

    VpPort port2 (vp1, "In", VpPort::Input);

    CPPUNIT_ASSERT (! port2.empty());
    CPPUNIT_ASSERT (port2.getType() == VpPort::Input);
    CPPUNIT_ASSERT (port2.isType (VpPort::Input));
    CPPUNIT_ASSERT (! port2.isType (VpPort::Output));
    CPPUNIT_ASSERT (port2.getName() == "In");
    CPPUNIT_ASSERT (port2.getVp() == vp1);

    const VpPort * port2Const = & port2;
    CPPUNIT_ASSERT (port2Const->getVp() == vp1); // test const version

    CPPUNIT_ASSERT (! port2.isConnectedInput());       // initially not connected
    CPPUNIT_ASSERT (! port2.isConnectedOutput());

    VpPort port3 (vp1, "InOut", VpPort::InOutput);
    CPPUNIT_ASSERT (port3.getType () == VpPort::InOutput);
    CPPUNIT_ASSERT (port3.isType (VpPort::Input));      // should be of both basic types
    CPPUNIT_ASSERT (port3.isType (VpPort::Output));

    VpPort port4 (vp1, "Out", VpPort::Output);
    CPPUNIT_ASSERT (port4.isType (VpPort::Output));
    CPPUNIT_ASSERT (! port4.isType (VpPort::Input));

    delete vp1;
}

void
VpPorts_Test::testMethods()
{

    VisibilityProcessor * vp1 = new VisibilityProcessorStub ("Stub1");

    VpPorts ports;

    VpPort p1 (vp1, "Vp1", VpPort::Input);
    VpPort p2 (vp1, "Vp2", VpPort::Input);
    VpPort p3 (vp1, "Vp3", VpPort::Input);
    VpPort p4 (vp1, "Vp4", VpPort::Input);

    ports.push_back (p1);
    ports.push_back (p2);
    ports.push_back (p3);

    CPPUNIT_ASSERT (ports.contains ("Vp1"));
    CPPUNIT_ASSERT (ports.contains ("Vp2"));
    CPPUNIT_ASSERT (ports.contains ("Vp3"));
    CPPUNIT_ASSERT (! ports.contains ("XYZ"));

    CPPUNIT_ASSERT (ports.contains (p1));
    CPPUNIT_ASSERT (ports.contains (p2));
    CPPUNIT_ASSERT (ports.contains (p3));
    CPPUNIT_ASSERT (! ports.contains (p4));

    VpPort pp = ports.get ("Vp1");
    CPPUNIT_ASSERT (pp == p1);

    VpPort & pRef = ports.getRef ("Vp1");
    CPPUNIT_ASSERT (pRef == p1);

    delete vp1;
}

void
VisibilityProcessor_Test::testConstruction ()
{
    vector<String> inputs = utilj::fillContainer<vector<String> > ("", "in1", "in2", "");
    vector<String> outputs = utilj::fillContainer<vector<String> > ("", "out1", "out2", "out3", "");
    String name = "vp1";
    VpNoop vp1 (name, inputs, outputs);

    CPPUNIT_ASSERT (vp1.getName() == name);
    CPPUNIT_ASSERT (vp1.getFullName() == name);

    {

        VpPorts inPorts = vp1.getInputs ();

        CPPUNIT_ASSERT (inPorts.size() == inputs.size());

        VpPorts::const_iterator i;
        vector<String>::const_iterator j;
        for (i = inPorts.begin(), j = inputs.begin();
             i != inPorts.end(), j != inputs.end();
             ++ i, ++ j){
            CPPUNIT_ASSERT (i->getName() == * j);
            CPPUNIT_ASSERT (i->getFullName() == name + ":" + *j);
            CPPUNIT_ASSERT (i->isType (VpPort::Input));
        }
    }

    {

        VpPorts outPorts = vp1.getOutputs ();

        CPPUNIT_ASSERT (outPorts.size() == outputs.size());

        VpPorts::const_iterator i;
        vector<String>::const_iterator j;
        for (i = outPorts.begin(), j = outputs.begin();
             i != outPorts.end(), j != outputs.end();
             ++ i, ++ j){
            CPPUNIT_ASSERT (i->getName() == * j);
            CPPUNIT_ASSERT (i->getFullName() == name + ":" + *j);
            CPPUNIT_ASSERT (i->isType (VpPort::Output));
        }
    }
}

VpTestListener::VpTestListener (bool verbose)
: errorsOccurred_p (false),
  verbose_p (verbose)
{
}

void
VpTestListener::addFailure (const TestFailure &failure)
{
    errorsOccurred_p = true;

    cout << "*** Test failure for " << failure.failedTestName() << ":" << endl;
    cout << "    at " << failure.sourceLine().fileName() << ":"
         << failure.sourceLine().lineNumber() << endl;

    Exception * e = failure.thrownException ();

    if (e != NULL){

        String what = e->what();
        what.gsub ("\n", "\n          ");

        cout << "    What: " << what << endl;

    }

}

void
VpTestListener::endTestRun (Test * /*test*/, TestResult * /*eventManager*/)
{
    if (verbose_p){

        cout << endl;

        if (errorsOccurred_p){
            cout << "--> Unsuccessfully completed test run. ;-(" << endl;
        }
        else{
            cout << "--> Successfully completed test run. ;-)" << endl;
        }
        cout << endl;
    }
}

void
VpTestListener::startTest (Test * test)
{
    if (verbose_p){

        cout << "Starting test " << test->getName() << "..." << endl;

    }
}


} // end namespace vpf
} // end namespace casa

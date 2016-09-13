/*
 * VisibilityProcessing_Test.cc
 *
 *  Created on: May 3, 2011
 *      Author: jjacobs
 */

#include "../VisibilityProcessing.h"
#include "VisibilityProcessing_Test.h"

#include <casa/BasicSL/String.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/ui/text/TestRunner.h>
#include <stdarg.h>
#include <iterator>
#include <msvis/MSVis/UtilJ.h>
#include <msvis/MSVis/VisibilityIterator.h>

using namespace std;
using namespace casa;
using namespace casa::vpf;
namespace po = boost::program_options;

int
main (int argc, char * args [])
{
    return casa::vpf::VpTests::singleton().run (argc, args);
}

namespace casa {

namespace vpf {

CPPUNIT_TEST_SUITE_REGISTRATION (SubchunkIndex_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpPort_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpPorts_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VbPtr_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpData_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VisibilityProcessor_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpEngine_Test);
CPPUNIT_TEST_SUITE_REGISTRATION (VpContainer_Test);

const casa::String VpTests::Visibility = "visibility";

void
reset () {
    ErrorTest::x = true;
}

VpTests::VpTests ()
: vm_p (new po::variables_map())
{}

const po::variables_map &
VpTests::getArguments () const
{
    return * vm_p;
}

int
VpTests::parseArguments (int argc, char * args [])
{
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Help me, help me!")
            (String (Visibility + ",v").c_str(), po::value<string>()->default_value(""),
             "visibility input file")
            ;

    try{
        po::store(po::command_line_parser(argc, args).
                  options(desc).run(),
                  * vm_p);
    }
    catch (po::error e){
        cerr << e.what() << endl;

        return 1;
    }

    po::notify(* vm_p);

    if (vm_p->count ("help")){
        cout << desc << endl;

        return 1;
    }

    return 0;
}

int
VpTests::run (int argc, char * args [])
{
    int parseResult = parseArguments (argc, args);
    if (parseResult != 0){
        return parseResult;
    }

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

VpTests &
VpTests::singleton ()
{
    static VpTests theSingleton;

    return theSingleton;
}

void
SubchunkIndex_Test::testConstruction ()
{
    {
        SubchunkIndex sci;
        CPPUNIT_ASSERT (sci.getChunkNumber () == SubchunkIndex::Invalid);
        CPPUNIT_ASSERT (sci.getSubchunkNumber () == SubchunkIndex::Invalid);
        CPPUNIT_ASSERT (sci.getIteration () == SubchunkIndex::Invalid);
    }

    {
        SubchunkIndex sci (1);
        CPPUNIT_ASSERT (sci.getChunkNumber () == 1);
        CPPUNIT_ASSERT (sci.getSubchunkNumber () == SubchunkIndex::Invalid);
        CPPUNIT_ASSERT (sci.getIteration () == SubchunkIndex::Invalid);
    }

    {
        SubchunkIndex sci (1,2);
        CPPUNIT_ASSERT (sci.getChunkNumber () == 1);
        CPPUNIT_ASSERT (sci.getSubchunkNumber () == 2);
        CPPUNIT_ASSERT (sci.getIteration () == SubchunkIndex::Invalid);
    }

    {
        SubchunkIndex sci (1,2,3);
        CPPUNIT_ASSERT (sci.getChunkNumber () == 1);
        CPPUNIT_ASSERT (sci.getSubchunkNumber () == 2);
        CPPUNIT_ASSERT (sci.getIteration () == 3);
        CPPUNIT_ASSERT (sci.toString() == "(1,2,3)");
    }
}

void
SubchunkIndex_Test::testComparisons ()
{
    SubchunkIndex sci1 (1,2,3);
    SubchunkIndex sci2 (1,2,3);
    CPPUNIT_ASSERT (sci1 == sci2);
    CPPUNIT_ASSERT (! (sci1 != sci2));
    CPPUNIT_ASSERT (! (sci1 < sci2));
    CPPUNIT_ASSERT (! (sci2 < sci1));

    CPPUNIT_ASSERT (SubchunkIndex (1,10,20) < SubchunkIndex (2,1,2));
    CPPUNIT_ASSERT (SubchunkIndex (1,10,20) < SubchunkIndex (1,11,2));
    CPPUNIT_ASSERT (SubchunkIndex (1,10,20) < SubchunkIndex (1,10,21));
}

void
VpData_Test::testConstruction ()
{
    VpData dataEmpty;
    CPPUNIT_ASSERT (dataEmpty.empty());

    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("vp");
    VpPort port1 (vp1, "In", VpPort::Input);
    VbPtr vb1 (new VisBuffer());
    VpData * data = new VpData (port1, vb1);

    CPPUNIT_ASSERT (utilj::containsKey (port1, * data));

    CPPUNIT_ASSERT_THROW (data->add (port1, vb1), AipsError);

    VpPort port2 (vp1, "Out", VpPort::Output);

    VbPtr vb2 (new VisBuffer ());
    data->add (port2, vb2);
    CPPUNIT_ASSERT (utilj::containsKey (port1, * data));
    CPPUNIT_ASSERT (utilj::containsKey (port2, * data));

    CPPUNIT_ASSERT_NO_THROW (delete data);

    delete vp1;
}

void
VpContainer_Test::testSweep (Int nRepeats)
{
    const po::variables_map & vm = VpTests::singleton().getArguments();

    if (vm.count(VpTests::Visibility) != 1 || vm[VpTests::Visibility].as<string>().empty()){
        CPPUNIT_ASSERT_MESSAGE ("No input file specified.", false);
        return;
    }

    VpContainer vpContainer ("TheContainer",
                             utilj::fillContainer<vector<String> > ("", "ContainerIn", ""),
                             vector<String> ());

    SplitterVp splitter ("Splitter",
                         "In1",
                         utilj::fillContainer<vector<String> > ("", "Out1", ""));

    vector<String> inputs = utilj::fillContainer<vector<String> > ("", "In2", "");
    vector<String> outputs;
    VpNoop noop ("Noop", inputs, outputs, nRepeats);

    vpContainer.add (& noop);
    vpContainer.add (& splitter);

    vpContainer.connect (& splitter, "Out1", & noop, "In2");
    vpContainer.connect ("ContainerIn", & splitter, "In1");

    VpEngine vpEngine;

    String inputFile = vm [VpTests::Visibility].as<string> ();
    MeasurementSet theMs;
    CPPUNIT_ASSERT_NO_THROW (theMs = MeasurementSet (inputFile, Table::Old));
    ROVisibilityIterator vi (theMs, Block<Int> ());

    CPPUNIT_ASSERT_NO_THROW (vpEngine.process (vpContainer, vi));

    CPPUNIT_ASSERT (noop.getNSubchunksProcessed () == splitter.getNSubchunksProcessed ());
    CPPUNIT_ASSERT (vpContainer.getNSubchunksProcessed () == splitter.getNSubchunksProcessed ());

    CPPUNIT_ASSERT (noop.getNSubchunksUniqueProcessed () == splitter.getNSubchunksUniqueProcessed ());
    CPPUNIT_ASSERT (vpContainer.getNSubchunksUniqueProcessed () == splitter.getNSubchunksUniqueProcessed ());

    CPPUNIT_ASSERT (noop.getNSubchunksProcessed () == noop.getNSubchunksUniqueProcessed () * nRepeats);

}

void
VpContainer_Test::testDoubleSweep ()
{
    testSweep (2);
}

void
VpContainer_Test::testSimpleSweep ()
{
    testSweep (1);
}

void
VpData_Test::testMethods ()
{
    // Put three ports with associated data into the VpData object data

    VisibilityProcessorStub * vp1 = new VisibilityProcessorStub ("vp");

    VpPort port1 (vp1, "In", VpPort::Input);
    VbPtr vb1 (new VisBuffer());
    VpData * data = new VpData (port1, vb1);
    CPPUNIT_ASSERT_THROW (data->add (port1, vb1), AipsError);

    VpPort port2 (vp1, "Out", VpPort::Output);
    VbPtr vb2 (new VisBuffer ());
    CPPUNIT_ASSERT_NO_THROW (data->add (port2, vb2));

    VbPtr vb3 (new VisBuffer ());
    VpPort port3 (vp1, "InOut", VpPort::InOut);
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
    VbPtr vbPtr (new VisBuffer());

    VbPtr vbPtr2;
    vbPtr2 = vbPtr;

    CPPUNIT_ASSERT (& * vbPtr2 == & * vbPtr);
    CPPUNIT_ASSERT (vbPtr.use_count() == 2);
    CPPUNIT_ASSERT (vbPtr2.use_count() == 2);

    {
        VbPtr vbPtr3 (vbPtr2);
        CPPUNIT_ASSERT (vbPtr.use_count() == 3);
    }

    vbPtr2.reset();

    CPPUNIT_ASSERT (vbPtr.use_count() == 1);

}

void
VbPtr_Test::testConstruction ()
{
    VisBuffer * vb = new VisBuffer();

    VbPtr vbPtr;
    CPPUNIT_ASSERT (! vbPtr && true);

    VbPtr vbPtr2 (vb);
    CPPUNIT_ASSERT (& * vbPtr2 == vb);

    Int nRefs = vbPtr2.use_count();
    VbPtr vbPtr3 (vbPtr2);
    CPPUNIT_ASSERT (& * vbPtr2 == vb);
    CPPUNIT_ASSERT (& * vbPtr2 == & * vbPtr3);
    CPPUNIT_ASSERT (vbPtr2.use_count() == nRefs + 1);
}

void
VbPtr_Test::testDestruction ()
{
    VbPtr * vp1 = new VbPtr (new VisBuffer ());

    VbPtr * vp2 = new VbPtr (* vp1);
    CPPUNIT_ASSERT (vp2->use_count () == 2);

    CPPUNIT_ASSERT_NO_THROW (delete vp2);

    CPPUNIT_ASSERT (vp1->use_count () == 1);

    CPPUNIT_ASSERT_NO_THROW (delete vp1);

}

void
VbPtr_Test::testOperators ()
{
    VisBuffer * vb = new VisBuffer();

    VbPtr vbPtr (vb);

    CPPUNIT_ASSERT (vbPtr.operator->() == vb);
    CPPUNIT_ASSERT (& vbPtr.operator* () == vb);
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


    VpPort port3 (vp1, "InOut", VpPort::InOut);
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

    VpPort port3 (vp1, "InOut", VpPort::InOut);
    CPPUNIT_ASSERT (port3.getType () == VpPort::InOut);
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
    VpNoop vp1 (name, inputs, outputs, 1);

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

void
VpEngine_Test::testNoopProcessor ()
{
    const po::variables_map & vm = VpTests::singleton().getArguments();

    if (vm.count(VpTests::Visibility) != 1 || vm[VpTests::Visibility].as<string>().empty()){
        CPPUNIT_ASSERT_MESSAGE ("No input file specified.", false);
        return;
    }

    String inputFile = vm [VpTests::Visibility].as<string> ();
    VpEngine engine;

    vector<String> inputs = utilj::fillContainer<vector<String> > ("", "in1", "");
    vector<String> outputs;
    String name = "vp1";
    VpNoop vp1 (name, inputs, outputs, 1);

    MeasurementSet theMs;
    CPPUNIT_ASSERT_NO_THROW (theMs = MeasurementSet (inputFile, Table::Old));
    ROVisibilityIterator vi (theMs, Block<Int> ());

    CPPUNIT_ASSERT_NO_THROW (engine.process (vp1, vi, vp1.getInput ("in1")));
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
VpTestListener::endTest (Test * test)
{
    if (verbose_p){

        cout << endl << "... completed test " << test->getName() << endl;

    }
}


void
VpTestListener::endTestRun (Test * /*test*/, TestResult * /*eventManager*/)
{
    if (verbose_p){

        cout << endl;

        if (errorsOccurred_p){

            cout << endl
                 << "**********************************************" << endl
                 << "* --> UNSUCCESSFULLY completed test run. ;-( *" << endl
                 << "**********************************************" << endl;
        }
        else{
            cout << endl
                 << "+------------------------------------------+" << endl
                 << "| --> Successfully completed test run. ;-) |" << endl
                 << "+------------------------------------------+" << endl;
        }
        cout << endl;
    }
}

void
VpTestListener::startTest (Test * test)
{
    if (verbose_p){

        cout << endl << "=============================================" << endl
             << "Starting test " << test->getName() << "..." << endl << endl;

    }
}


} // end namespace vpf
} // end namespace casa

/*
 * VisibilityProcessing.cc
 *
 *  Created on: Apr 20, 2011
 *      Author: jjacobs
 */

#include "VisibilityProcessing.h"
#include "VisBufferAsync.h"
#include "VisibilityIteratorAsync.h"
#include "UtilJ.h"

#include <casa/System/AipsrcValue.h>

#include <boost/lambda/lambda.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <list>
#include <stdarg.h>
#include <limits>
#include <memory>
#include <numeric>

using namespace casa::asyncio;
using namespace casa::utilj;
using namespace std;

namespace casa {

namespace vpf {

#define Log(level, ...) \
    {if (level <= VpEngine::getLogLevel()) \
         VpEngine::log (__VA_ARGS__);}


SubChunkIndex::SubChunkIndex (Int chunkNumber, Int subChunkNumber, Int iteration)
: chunkNumber_p (chunkNumber), iteration_p (iteration), subChunkNumber_p (subChunkNumber)
{}

Bool
SubChunkIndex::operator< (const SubChunkIndex & other) const
{
    Bool result = boost::make_tuple (chunkNumber_p, subChunkNumber_p, iteration_p) <
                  boost::make_tuple (other.chunkNumber_p, other.subChunkNumber_p, other.iteration_p);

    return result;
}

Int
SubChunkIndex::getChunkNumber () const
{
    return chunkNumber_p;
}

Int
SubChunkIndex::getIteration () const
{
    return iteration_p;
}

Int
SubChunkIndex::getSubChunkNumber () const
{
    return subChunkNumber_p;
}

String
SubChunkIndex::toString() const
{
    return format ("(%d,%d,%d)", chunkNumber_p, subChunkNumber_p, iteration_p);
}

VbPtr::VbPtr ()
: vb_p (new CtdPtr ())
{}


VbPtr::VbPtr (VisBuffer * vb, Bool destroyIt)
: vb_p (new CtdPtr (vb, destroyIt))
{}

VbPtr::VbPtr (const VbPtr & other)
{
    vb_p = new CtdPtr ();

    // operator= handles the locking

    * this = other;
}

VbPtr::~VbPtr ()
{
    {
        MutexLocker ml();

        delete vb_p;
    }
}

VbPtr &
VbPtr::operator= (const VbPtr & other)
{
    if (this != & other){

        MutexLocker ml ();

        * vb_p = * other.vb_p;
    }

    return * this;
}

VisBuffer &
VbPtr::operator* ()
{
    return vb_p->CtdPtr::operator* ();
}

VisBuffer *
VbPtr::operator-> ()
{
    return vb_p->CtdPtr::operator->();
}

void
VbPtr::clear ()
{
    MutexLocker ml ();
    * vb_p = NULL;
}

Int
VbPtr::getNRefs () const
{
    return vb_p->nrefs();
}

Bool
VbPtr::null () const
{
    return vb_p->null();
}



//VisBuffer *
//VbPtr::get ()
//{
//    return vb_p; // use with caution
//}
//

//VisBuffer *
//VbPtr::release ()
//{
//    freeOnDelete_p = False;
//    VisBuffer * vb = vb_p;
//    vb_p = NULL;
//
//    return vb;
//}



VisibilityProcessor::VisibilityProcessor (const String & name,
                                          const vector<String> & inputNames,
                                          const vector<String> & outputNames)
: container_p (NULL),
  name_p (name)
{
    vpInputs_p = definePorts (inputNames, VpPort::Input, "input");
    vpOutputs_p = definePorts (outputNames, VpPort::Output, "output");
}

VpPorts
VisibilityProcessor::definePorts (const vector<String> & portNames, VpPort::Type type, const String & typeName)
{

    VpPorts vpPorts;

    for (vector<String>::const_iterator portName = portNames.begin();
         portName != portNames.end();
         portName ++){

        ThrowIf (vpPorts.contains (* portName),
                 format ("VisibilityProcessor %s already has an %s port '%s'",
                         getName().c_str(), typeName.c_str(), portName->c_str()));

        vpPorts.push_back (VpPort (this, * portName, type));
    }

    return vpPorts;
}


String
VisibilityProcessor::getFullName () const
{

    list<String> names;
    names.push_front (getName());

    const VisibilityProcessor * parent = getContainer();

    while (parent != NULL){

        names.push_front (parent->getName());
        parent = parent->getContainer();
    }

    String fullName = utilj::join (names, ".");

    return fullName;
}

VpPorts
VisibilityProcessor::getInputs (Bool connectedOnly) const
{
    VpPorts result;

    if (connectedOnly){

        // Copy over the inputs that are input connected.
        // Need to negate predicate to get the right results --STL is strange sometimes.

        remove_copy_if (vpInputs_p.begin(), vpInputs_p.end(), back_inserter (result),
                        not1 (mem_fun_ref (& VpPort::isConnectedInput)));
    }
    else{
        result = vpInputs_p;
    }

    return result;
}

VpPort
VisibilityProcessor::getInput (const String & name) const
{
    ThrowIf (! vpInputs_p.contains (name),
             format ("No such input port '%s'", name.c_str()));

    return vpInputs_p.get (name);
}

VpPort &
VisibilityProcessor::getInputRef (const String & name)
{
    ThrowIf (! vpInputs_p.contains (name),
             format ("No such input port '%s'", name.c_str()));

    return vpInputs_p.getRef (name);
}

String
VisibilityProcessor::getName () const
{
    return name_p;
}

VpPort
VisibilityProcessor::getOutput (const String & name) const
{
    ThrowIf (! vpInputs_p.contains (name),
             format ("No such input port '%s'", name.c_str()));

    return vpInputs_p.get (name);
}

VpPort &
VisibilityProcessor::getOutputRef (const String & name)
{
    ThrowIf (! vpOutputs_p.contains (name),
             format ("No such output port '%s'", name.c_str()));

    return vpOutputs_p.getRef (name);
}

VpPorts
VisibilityProcessor::getOutputs () const
{
    return vpOutputs_p;
}

PrefetchColumns
VisibilityProcessor::getPrefetchColumns () const
{
    return PrefetchColumns ();
}

void
VisibilityProcessor::setContainer (const VpContainer * container)
{
    assert (container != NULL);

    ThrowIf (container_p != NULL,
             format ("Attempting to add VisibiltyProcessor '%s' into '%s'; previously added to '%s'",
                     getName().c_str(), container->getFullName().c_str(), container_p->getFullName().c_str()));

    container_p = container;
}

void
VpContainer::add (VisibilityProcessor * vp)
{
    ThrowIf (contains (vp),
             utilj::format ("Visibility processor %s already in container %s",
                            vp->getName().c_str(), getName().c_str()));

    vp->setContainer (this);
    vps_p.push_back (vp);
}

void
VpContainer::connect (VpPort & output, VpPort & input)
{
    // Validate the requested connection
    // =================================

    VisibilityProcessor * inputVp = input.getVp();
    VisibilityProcessor * outputVp = output.getVp();

    // Do they refer to a VP in this container?

    ThrowIf (! contains (outputVp),
             format ("No such visibility processor %s in %s.",
                     outputVp->getName().c_str(), getName().c_str()));
    ThrowIf (! contains (inputVp),
             format ("No such visibility processor %s in %s.",
                     inputVp->getName().c_str(), getName().c_str()));

    // Does the owning VP really support these ports?

    ThrowIf (! outputVp->getOutputs ().contains (output),
             format ("Visibility processor %s in %s does not have output %s",
                     outputVp->getName().c_str(), getName().c_str(),
                     output.getName().c_str()));

    ThrowIf (! inputVp->getInputs ().contains (input),
             format ("Visibility processor %s in %s does not have input %s",
                     inputVp->getName().c_str(), getName().c_str(),
                     input.getName().c_str()));

    // Are the ports already in use?

    ThrowIf (utilj::containsKey (output, network_p),
             format ("Output %s already in use for visibility processor %s in %s",
                     output.getName().c_str(), outputVp->getName().c_str(), getName().c_str()));

    ThrowIf (utilj::containsKey (input, networkReverse_p),
             format ("Input %s already in use for visibility processor %s in %s",
                     input.getName().c_str(), inputVp->getName().c_str(), getName().c_str()));

    // See if this is a connection to the container inputs or outputs or
    // a normal connection between VPs

    Bool containerConnect = (output.getType() == input.getType()) &&
                            ((output.isType(VpPort::Input) && outputVp == this) ||
                             (input.isType(VpPort::Output) && inputVp == this));

    Bool normalConnect = output.isType (VpPort::Output) && input.isType (VpPort::Input);

    Bool selfConnect = outputVp == inputVp; // detects loop back

    ThrowIf (! (normalConnect ||  containerConnect) || selfConnect,
             format ("Cannot connect %s:%s to %s:%s in %s", outputVp->getName ().c_str(),
                     output.getName ().c_str (), inputVp->getName().c_str (),
                     input.getName ().c_str (), getName().c_str()));

    // The validation is over, so actually do the connection.

    network_p [output] = input;
    networkReverse_p.insert (input);

    // Inform the real ports (i.e., not the copies) that they are connected

    outputVp->getOutputRef (output.getName()).setConnectedOutput ();
    inputVp->getInputRef (input.getName()).setConnectedInput ();
}

Bool
VpContainer::contains (const VisibilityProcessor * vp) const
{
    Bool foundIt = find (vps_p.begin(), vps_p.end(), vp) != vps_p.end();

    return foundIt;
}

VisibilityProcessor::ProcessingResult
VpContainer::doProcessing (ProcessingType processingType, VpData & data, const SubChunkIndex & sci)
{

    VpSet vpsWaiting (vps_p.begin(), vps_p.end()); // Set of pending VPs
    ChunkCode overallChunkCode = Normal;           // container result for this data
    VisibilityProcessor * vp;                      // Currently executing VP
    VpData vpInputs;                               // Inputs to be fed to current Vp

    remapPorts (data, this);

    Log (3, "VpContainer::doProcessing: '%s' starting execution with inputs {%s}.\n",
         getName().c_str(), data.getNames().c_str());

    do {

        // Find a VP which can compute given the current set of inputs

        boost::tie (vp, vpInputs) = findReadyVp (vpsWaiting, data);

        if (vp != NULL){

            Log (3, "VpContainer::doProcessing: '%s' starting execution of %s.\n",
                 getName().c_str(), vp->getName().c_str());

            // Have the ready VP process its inputs and
            // potentially produce more outputs

            ChunkCode chunkCode;
            VpData outputs;

            boost::tie (chunkCode, outputs) =
                vp->doProcessing (processingType, vpInputs, sci);

            Log (3, "VpContainer::doProcessing: execution of %s output {%s}.\n",
                 vp->getName().c_str(), outputs.getNames().c_str());

            if (processingType == EndOfChunk && chunkCode == RepeatChunk){

                // If any VP in this iteration requests a chunk repeat,
                // then that's the overall result.

                chunkCode = RepeatChunk;
            }

            // Remove the VP from the set of pending VPs, remove
            // the data this VP consumed as inputs and add any outputs
            // it produced to the set of available data.

            vpsWaiting.erase (vp);
            data.erase (vpInputs.begin(), vpInputs.end());
            remapPorts (outputs, vp);
            data.insert (outputs.begin(), outputs.end());

        }

    } while (vp != NULL);

    if (vpsWaiting.empty()){
        Log (3, "VpContainer::doProcessing: '%s' executed all VPs.\n", getName().c_str());
    }
    else{
        Log (3, "VpContainer::doProcessing: '%s' did not execute VPs: {%s}.\n",
             getName().c_str(), vpsWaiting.getNames().c_str());
    }

    return ProcessingResult (overallChunkCode, data);
}

void
VpContainer::fillWithSequence (VisibilityProcessor * first, ...)
{
    ThrowIf (! vps_p.empty (),
             format ("fillWithSequence performed on non-empty container %s", getName().c_str()));

	va_list vaList;

	VisibilityProcessor * vp = first;

	va_start (vaList, first);

	while (vp != NULL){

	    add (vp);

	    vp = va_arg (vaList, VisibilityProcessor *);
	}

	va_end (vaList);

	for (VPs::iterator vp = vps_p.begin ();
	     vp != vps_p.end();
	     vp ++){

	    VPs::iterator vp2 = vp + 1;
	    if (vp2 == vps_p.end()){
	        break;
	    }

	    ThrowIf ((* vp)->getOutputs().empty(),
	             format ("Visibility processor %s has no outputs.", (* vp)->getName().c_str()));
	    ThrowIf ((* vp2)->getInputs().empty(),
	             format ("Visibility processor %s has no inputs.", (* vp2)->getName().c_str()));

	    connect ((* vp)->getOutputs().front(), (* vp2)->getInputs().front());

	}

	// Connect up containers input to the input of the first VP

	ThrowIf (vps_p.front()->getInputs().empty(),
	         format ("First node in sequence, %s, has no inputs",
	                 vps_p.front()->getName().c_str()));

	connect (getInputs().front(), vps_p.front()->getInputs().front());

	if (! getOutputs().empty()  && ! vps_p.back()->getOutputs().empty()){

	    // Connect up output of last node with output of container

	    connect (getOutputs().front(), vps_p.back()->getOutputs().front());

	}
}

boost::tuple<VisibilityProcessor *, VpData>
VpContainer::findReadyVp (VpSet & vps, VpData & data) const
{
    boost::tuple<VisibilityProcessor *, VpData> result (NULL, VpData());

    set<VpPort> dataPorts;
    for (VpData::const_iterator d = data.begin(); d != data.end(); d++){
        dataPorts.insert (d->first);
    }

    for (VpSet::const_iterator vp = vps.begin(); vp != vps.end(); vp ++){

        VpPorts connectedInputList = (* vp)->getInputs (True);

        set<VpPort> connectedInputSet (connectedInputList.begin(), connectedInputList.end());

        // Subtract from the needed input ports, the set of available data ports.
        // When the comes up empty then the VP can execute.

        set<VpPort> diff;
        set_difference (connectedInputSet.begin(), connectedInputSet.end(),
                        dataPorts.begin(), dataPorts.end(),
                        inserter (diff, diff.begin()));

        if (diff.empty()){

            result = boost::tuple<VisibilityProcessor *, VpData> (* vp, data.getSelection (connectedInputList));

            break;
        }

    }

    return result;
}

void
VpContainer::remapPorts (VpData & data, const VisibilityProcessor * vp)
{
    vector<VpPort> oldPorts = mapKeys (data);

    for (vector<VpPort>::const_iterator oldPort = oldPorts.begin();
         oldPort != oldPorts.end();
         oldPort ++){

        Network::const_iterator newPortItr = network_p.find (* oldPort);

        if (newPortItr != network_p.end()){

            VpPort newPort = newPortItr->second;

            assert (! utilj::containsKey (newPort, data));

            data [newPort] = data [* oldPort];
            data.erase (* oldPort);

        }
        else{
            ThrowIf (true,
                     format ("Vp '%s' produced unused output '%s'",
                             vp->getFullName().c_str(), oldPort->getName().c_str()));
        }
    }
}

String
VpContainer::VpSet::getNames () const
{
    String nameList = utilj::join (begin(), end(), mem_fun (& VisibilityProcessor::getName), ",");

    return nameList;
}

VpData::VpData ()
{}

VpData::VpData (const VpPort & port, VisBuffer * vb, Bool deleteIt)
{
    add (port, vb, deleteIt);
}

void
VpData::add (const VpPort & port, VisBuffer * vb, Bool deleteIt)
{
    ThrowIf (utilj::containsKey (port, * this),
            format ("VpData::add: data already present for port %s.", port.getFullName ().c_str()));

    VbPtr vbPtr (vb, deleteIt);
    (* this) [port] = vbPtr;
}

String
VpData::getNames () const
{
    string names = join (begin(), end(),
                     compose (mem_fun_ref (& VpPort::getName),
                              casa::utilj::firstFunctor<VpPort, VbPtr>()),
                     ",");

    return names;
}

VpData
VpData::getSelection (const VpPorts & ports) const
{
    VpData result;

    for (VpPorts::const_iterator port = ports.begin(); port != ports.end(); port ++){
        result [* port] = find (* port)->second;
    }

    return result;
}


Int VpEngine::logLevel_p = std::numeric_limits<int>::min();
LogIO * VpEngine::logIo_p = NULL;
Bool VpEngine::loggingInitialized_p = VpEngine::initializeLogging ();

Bool
VpEngine::initializeLogging()
{
    AipsrcValue<Int>::find (logLevel_p, getAipsRcBase () + ".debug.logLevel",
                            std::numeric_limits<int>::min());

    if (logLevel_p >= 0){

        logIo_p = new LogIO (LogOrigin ("VisibilityProdessing"));
        * logIo_p << "VisibilityProcessing logging enabled; level=" << logLevel_p << endl << LogIO::POST;

    }

    return True;
}

String
VpEngine::getAipsRcBase ()
{
    return "VpFramework";
}

Int
VpEngine::getLogLevel ()
{
    return logLevel_p;
}

void
VpEngine::log (const String & formatString, ...)
{
	va_list vaList;

	va_start (vaList, formatString);

	String result = formatV (formatString.c_str(), vaList);

	va_end (vaList);

	(* logIo_p) << result << endl << LogIO::POST;
}


void
VpEngine::process (VisibilityProcessor & processor,
                   ROVisibilityIterator & vi,
                   VpPort inputPort)
{
    Log (1, "VpEngine::process starting on processor '%s'", processor.getName().c_str());

    VisBufferAutoPtr vb (vi);

    if (inputPort.empty()){ // Take single input to VP as default if not specified

        VpPorts inputs = processor.getInputs ();

        ThrowIf (inputs.size() != 1,
                 format ("Vp '%s' must have exactly one input or an input must be specified explicitly",
                         processor.getName().c_str()));

        inputPort = inputs.front();
    }

    // connect up input and then validate

    processor.processingStart ();

    Int chunkNumber = 0;

    for (vi.originChunks ();
            vi.moreChunks();
            vi.nextChunk (), chunkNumber ++){

        Int iteration = 0;
        VisibilityProcessor::ChunkCode chunkCode = VisibilityProcessor::Normal;

        do {  // The VP can request repeating a chunk

            Log (2, "VpEngine::process: Starting chunk %d (iteration=%s)\n",
                 chunkNumber, iteration);

            processor.chunkStart (SubChunkIndex (chunkNumber, SubChunkIndex::Invalid, iteration));

            Int subChunkNumber = 0;

            for (vi.origin (); vi.more (); ++ vi, subChunkNumber ++){

                VisibilityProcessor::ProcessingResult ignored;

                SubChunkIndex sci (chunkNumber, subChunkNumber);

                Log (2, "VpEngine::process: Starting SubChunk %s (iteration=%s)\n",
                     sci.toString ().c_str(), iteration);

                VpData data (inputPort, vb.get());
                ignored = processor.doProcessing (VisibilityProcessor::SubChunk,
                                                  data,
                                                  sci);
            }

            VpData noData;
            VpData ignored;
            boost::tie (chunkCode, ignored) =
                    processor.doProcessing (VisibilityProcessor::EndOfChunk,
                                            noData,
                                            SubChunkIndex (chunkNumber));

        } while (chunkCode == VisibilityProcessor::RepeatChunk);
    }

    VisibilityProcessor::ProcessingResult ignored;
    VpData noData;

    ignored = processor.doProcessing (VisibilityProcessor::EndOfData,
                                      noData,
                                      SubChunkIndex ());

    Log (1, "VpEngine::process completed for processor '%s'", processor.getName().c_str());

}

VpPort::VpPort (VisibilityProcessor * vp, const String & name, VpPort::Type type)
: connectedInput_p (False),
  connectedOutput_p (False),
  name_p (name),
  visibilityProcessor_p (vp),
  type_p (type)
{}

Bool
VpPort::operator== (const VpPort & other) const
{
    Bool result = other.getVp() == getVp() && other.getName() == getName();

    return result;
}

Bool
VpPort::operator< (const VpPort & other) const
{
    Bool result = other.getVp() < getVp() ||
                  (other.getVp() == getVp() && other.getName() < getName());

    return result;
}


Bool
VpPort::empty () const
{
    return visibilityProcessor_p == NULL;
}

String
VpPort::getFullName () const
{
    String vpName = "*NULL*";
    if (getVp() != NULL){
        vpName = getVp()->getFullName();
    }

    return format ("%s:%s", vpName.c_str(), getName().c_str());
}

String
VpPort::getName () const
{
    return name_p;
}

VpPort::Type
VpPort::getType () const
{
    return type_p;
}

VisibilityProcessor *
VpPort::getVp ()
{
    return visibilityProcessor_p;
}

const VisibilityProcessor *
VpPort::getVp () const
{
    return visibilityProcessor_p;
}

Bool
VpPort::isConnectedInput () const
{
    return connectedInput_p;
}

Bool
VpPort::isConnectedOutput () const
{
    return connectedOutput_p;
}

Bool
VpPort::isType (Type t) const
{
    return (type_p & t) != 0;
}

void
VpPort::setConnectedInput ()
{
    AssertAlways (! empty() && ! connectedInput_p && isType (Input));

    connectedInput_p = True;
}

void
VpPort::setConnectedOutput ()
{
    AssertAlways (! empty() && ! connectedOutput_p && isType (Output));

    connectedOutput_p = True;
}

Bool
VpPorts::contains (const String & name) const
{
    Bool foundIt = find (name, begin(), end()) != end();

    return foundIt;
}

Bool
VpPorts::contains (const VpPort & port) const
{
    Bool foundIt = std::find (begin(), end(), port) != end();

    return foundIt;
}

VpPort
VpPorts::get (const String & name) const
{
    const_iterator i = find (name, begin(), end());
    ThrowIf (i == end(), "No such port '" + name + "'");

    return * i;
}

VpPort &
VpPorts::getRef (const String & name)
{
    iterator i = find (name, begin(), end());
    ThrowIf (i == end(), "No such port '" + name + "'");

    return * i;
}


} // end namespace vpu

} // end namespace casa

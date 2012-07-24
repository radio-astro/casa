/*
 * VisibilityProcessing.h
 *
 *  Created on: Feb 8, 2011
 *      Author: jjacobs
 */

#ifndef VISIBILITYPROCESSING_H_
#define VISIBILITYPROCESSING_H_

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include "VisBuffer.h"
#include "VisibilityIterator.h"
#include "UtilJ.h"

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <map>
#include <set>
#include <vector>


/*

Visibility Processing Framework Class Summary
=============================================

SubchunkIndex - Index of a subchunk.  Consists of the chunk number,
                the subchunk number and the iteration number.  All three
                are zero-based.  The interation number is nonzero if a chunk
                is reprocessed. A subchunk is used to identify a VisBuffer
                relative to the VisibilityIterator managed by the VpEngine.
VbPtr - Smart pointer of a VisBuffer.
VisibilityProcessor - A visibility processing node in data flow graph
    VisibilityProcessorStub - A do-nothing node used for unit testing
    VpContainer - A VP which contains a graph of VPs.  It handles moving
                  data between its input and output ports to the appropriate
                  input and output ports of the data flow graph it contains.
    SplitterVp - Has a single input port and outputs identical copies of it
                 through its output ports.
    WriterVp - Takes an input and writes it out to the VisibilityIterator provided
               when it was constructed.  Optionally passes the input data to its
               output port.
VpEngine - Object that executes a data flow graph of VisibilityProcessors on data
           accessed via a VisibilityIterator.

VpPort - A data port into or out of (or both) a VisibiltyProcessor
VpPorts - A collection of VpPort objects
VpData - A collection of visibility data; it works like an associative array
         pairing a VpPort with a VisBuffer.

*/

namespace casa {

namespace asyncio {
    class PrefetchColumns;
};

namespace vpf {

class VisibilityProcessor;
class VpContainer;
class VpEngine;

class SubchunkIndex {

    friend class SubchunkIndex_Test;

public:

    enum {Invalid = -1};

    SubchunkIndex (Int chunkNumber = Invalid, Int subChunkNumber = Invalid, Int iteration = Invalid);

    // Comparison Operators
    //
    // Comparison is in lexicographic order by chunk, subchunk and iteration.

    Bool operator< (const SubchunkIndex & other) const;
    Bool operator== (const SubchunkIndex & other) const { return ! (* this < other || other < * this);}
    Bool operator!= (const SubchunkIndex & other) const { return ! (* this == other);}

    Int getChunkNumber () const;
    Int getIteration () const;
    Int getSubchunkNumber () const;

    String toString () const;

private:

    Int chunkNumber_p;        // -1 for invalid
    Int iteration_p;          // -1 for invalid
    Int subChunkNumber_p;
};

class VbPtr : public boost::shared_ptr<casa::VisBuffer> {

public:

    VbPtr () : boost::shared_ptr<casa::VisBuffer> () {}
    explicit VbPtr (casa::VisBuffer * vb) : boost::shared_ptr<casa::VisBuffer> (vb) {}

    // Assignment operator setting VbPtr to a normal pointer.  Ownership is passed to the
    // VbPtr so caller must ensure that delete is not called on the VisBuffer.

    VbPtr & operator= (casa::VisBuffer * vb)
    {
        boost::shared_ptr<casa::VisBuffer>::operator= (VbPtr (vb));
        return * this;
    }
};

class VpPort {

    friend class VpContainer;
    friend class VpPort_Test;

public:

    // Normally ports are either input or output ports.  However, the ports
    // of a VpContainer do double duty serving as an input to the container and
    // an outputted to the input of a contained VP, or vice versa.

    typedef enum {Unknown, Input = 1, Output = 2, InOut = Input | Output} Type;

    VpPort ();
    VpPort (VisibilityProcessor * vp, const String & name, Type type);
    ~VpPort () {}

    Bool operator< (const VpPort & other) const;
    Bool operator== (const VpPort & other) const;

    Bool empty () const;
    String getFullName () const; // returns Vp0.Vp1...VpN.portName
    String getName () const; // returns portName
    Type getType () const; // Returns the port's type as something from the Type enum
    Bool isConnectedInput () const; // True if port has been connected up as an input
    Bool isConnectedOutput () const; // True if port has been connected up as an output

    // Used to check the type of the port as defined in the Type enum.  InOut ports
    // return true for both Input and Output types.

    bool isType (Type t) const;

    //String toString() const;

protected:

    const VisibilityProcessor * getVp () const;
    VisibilityProcessor * getVp ();
    void setConnectedInput ();
    void setConnectedOutput ();

private:

    Bool connectedInput_p;
    Bool connectedOutput_p;
    String name_p;
    VisibilityProcessor * visibilityProcessor_p; // [use]
    Type type_p;

};

class VpPorts : public std::vector<VpPort> {

    friend class VisibilityProcessor;
    friend class VpPorts_Test;

public:

    Bool contains (const String & name) const;
    Bool contains (const VpPort & port) const;
    VpPort get (const String & name) const;
    String toString () const;

protected:

    VpPort & getRef (const String & name);

    template <typename Itr>
    static
    Itr
    find(const String & name, Itr begin, Itr end)
    {
        Itr i;

        for (i = begin; i != end; i++){
            if (i->getName() == name){
                break;
            }
        }

        return i;
    }

};

namespace asyncio {
    class PrefetchColumns;
}


class VpData: public std::map<VpPort, VbPtr> {

    friend class VpData_Test;

public:

    VpData ();
    VpData (const VpPort & port, VbPtr);

    void add (const VpPort & port, VbPtr); // Adds a (port,VbPtr) to the collection

    // Returns the (port,VbPtr) pairs for the requested set of ports.  An execption
    // is thrown if a requested port is not present unless missingIsOk is set to True.

    VpData getSelection (const VpPorts &, bool missingIsOk = False) const;
    String getNames () const; // Returns a comma-separated list of the port names.
};


class VisibilityProcessor : boost::noncopyable {

    friend class VpContainer;
    friend class WriterVp;

public:

    typedef enum {
        Normal,
        RepeatChunk
    } ChunkCode;

    typedef enum {
        Subchunk,    // Normal processing of a subchunk
        EndOfChunk,  // Called after all subchunks of a chunk have been processed
        EndOfData    // Called after all chunks have been processed
    } ProcessingType;

    typedef boost::tuple <ChunkCode, VpData> ProcessingResult;

    VisibilityProcessor ();
    VisibilityProcessor (const String & name,
                         const vector<String> & inputNames,
                         const vector<String> & outputNames = vector<String>(),
                         Bool makeIoPorts = False);
    virtual ~VisibilityProcessor () {}

    // chunkStart is called to inform the VP that a new chunk is starting.

    void chunkStart (const SubchunkIndex &);

    // Called to cause the VP to process the provided inputs.  It will be called
    // in three different contexts as indicated by the ProcessingType.

    ProcessingResult doProcessing (ProcessingType processingType,
                                   VpData & inputData,
                                   VpEngine * vpEngine,
                                   const SubchunkIndex & subChunkIndex);

    // Returns a pointer to the containing VP or NULL if this VP is top-level.

    const VpContainer * getContainer () const { return NULL;}

    // The full name of a VP is a dotted list of the names of all the containing
    // VPs ending with the name of this VP (e.g., vp0.vp1...vpN.thisVp).

    String getFullName () const;

    // Returns the input port having the specified name.  Exception if port is undefined.

    VpPort getInput (const String & name) const;

    // Returns a collection of the input ports for this VP; optionally only the
    // connected ports are returned.

    VpPorts getInputs (Bool connectedOnly = False) const;

    // Returns the name of this VP

    String getName () const;

    // Returns the number of Subchunks processed (mainly for testing)

    Int getNSubchunksProcessed () const;

    // Returns the number of unique Subchunks (i.e., iteration ignored) processed.
    // (mainly for testing)

    Int getNSubchunksUniqueProcessed () const;

    // Returns the output port having the specified name.  Exception if port is undefined.

    VpPort getOutput (const String & name) const;

    // Returns a collection of the output ports for this VP; optionally only the
    // connected ports are returned.

    VpPorts getOutputs (Bool connectedOnly = False) const;

    // Returns the collection of columns that need to be prefetched if this node
    // is used with async I/O.

    virtual casa::asyncio::PrefetchColumns getPrefetchColumns () const;

    // Called by the framework when the processing is about to begin (i.e., prior
    // to the first VisBuffer being fed into the graph.

    void processingStart ();

    // Called to ask the VP to check its validity (i.e., are all needed inputs connected,
    // etc.).

    void validate ();

protected:

    // The public API contains many methods that are not virtual.  However, where subclass-specific
    // behavior is potentially useful, a corresponding xxxImpl method is provided.  This allows the
    // framework to perform certain required housekeeping options while allowing the subclass
    // to perform custom operations.

    // Called on the object when a new chunk is about to be started.

    virtual void chunkStartImpl (const SubchunkIndex &) {}


    // Defines the set of possible input ports for this VP

    VpPorts definePorts (const vector<String> & portNames, VpPort::Type type, const String & typeName);

    // Requests processing of the provided (possibly empty) input data.  This is called on each
    // subchunk (then inputData will be nonempty) and at the end of a chunk and the end of the
    // entire data set.  These last two call types allow the VP to output any data that it might have
    // been accumulating across multiple subchunks, etc.

    virtual ProcessingResult doProcessingImpl (ProcessingType processingType,
                                               VpData & inputData,
                                               const SubchunkIndex & subChunkIndex) = 0;

    // Returns a collection of the ports that are not connected using the provided connection
    // method; some ports may also be excluded from this list by name.

    VpPorts portsUnconnected (const VpPorts & ports, Bool (VpPort::* isConnected) () const,
                              const vector<String> & except = vector<String> ()) const;

    // Called when data processing is about to beging; this allows the VP to perform any
    // initialization that it desires now that it is completely connected into the graph.

    virtual void processingStartImpl () {}

    // Methods to ease the validation process.

    void throwIfAnyInputsUnconnected (const vector<String> & exceptThese = vector<String> ()) const;
    void throwIfAnyInputsUnconnectedExcept (const String & exceptThisOne) const;
    void throwIfAnyOutputsUnconnected (const vector<String> & exceptThese = vector<String> ()) const;
    void throwIfAnyOutputsUnconnectedExcept (const String & exceptThisOne) const;
    void throwIfAnyPortsUnconnected () const;

    // Called to allow the node to validate its initial state.  An AipsError should be thrown if
    // this node decides that it is invalid.

    virtual void validateImpl () = 0;

private:

    VpPort & getInputRef (const String & name);
    VpPort & getOutputRef (const String & name);
    void setContainer (const VpContainer *);

    // Prevent copying of existing objects

    VisibilityProcessor (const VisibilityProcessor & other); // do not define
    VisibilityProcessor & operator=(const VisibilityProcessor & other); // do not define

    ROVisibilityIterator * getVi (); // returns the VI used for this data set
    VpEngine * getVpEngine(); // returns the engine executing this VP

    const VpContainer * container_p; // [use]
    String name_p; // name of this VP
    Int nSubchunks_p; // number of subchunks processed
    Int nSubchunksUnique_p; // number of unique subchunks processed
    VpEngine * vpEngine_p; // pointer to VpEngine processing this VP (can be null)
    VpPorts vpInputs_p; // collection of input ports
    VpPorts vpOutputs_p; // collection of output ports
};

ostream & operator<< (ostream & os, const VisibilityProcessor::ProcessingType & processingType);
String toString (VisibilityProcessor::ProcessingType p);

class VisibilityProcessorStub : public VisibilityProcessor {

    // Used to allow definition of a VP variable for use in testing.
    // Should never be actually operated on.

public:

    VisibilityProcessorStub (const String & name)
    : VisibilityProcessor (name, utilj::Strings(), utilj::Strings())
    {}

    ProcessingResult doProcessingImpl (ProcessingType /*processingType*/,
                                       VpData & /*inputData*/,
                                       const SubchunkIndex & /*subChunkIndex*/);

    void validateImpl ();


};

//class SimpleVp: public VisibilityProcessor {
//
//public:
//
//    SimpleVp (const String & name, const String & input = "In", const String & output = "");
//    virtual ~SimpleVp ();
//
//protected:
//
//    class SimpleResult : public boost::tuple<ChunkCode, VisBuffer *> {};
//
//    virtual ProcessingResult doProcessingImpl (ProcessingType processingType,
//                                               VpData & inputData,
//                                               const SubchunkIndex & subChunkIndex);
//    virtual void validateImpl ();
//
//private:
//
//};

class SplitterVp : public VisibilityProcessor {

public:

    SplitterVp (const String & name,
                const String & inputName,
                const vector<String> & outputNames);

    ~SplitterVp () {}

protected:

    ProcessingResult doProcessingImpl (ProcessingType processingType ,
                                       VpData & inputData,
                                       const SubchunkIndex & subChunkIndex);

    void validateImpl ();
};

class WriterVp: public VisibilityProcessor {

public:

    // Creates a WriterVp node.  If the vi argument is NULL then the
    // flow graph's VI is used.  The advanceVi argument is used to
    // direct the node to advance the VI after each write (i.e., perform
    // a vi++ operation); advancing the flow graph's VI will cause a
    // run time exception.

    WriterVp (const String & name,
              VisibilityIterator * vi = NULL,
              Bool advanceVi = False,
              const String & input = "In",
              const String & output = "Out");

    // This paradoxical method allows the user to create a single data flow graph
    // and then programmatically decide at run time whether data should be actually
    // output on this particular run.

    Bool setDisableOutput (Bool disableIt);

protected:

    ProcessingResult doProcessingImpl (ProcessingType processingType,
                                       VpData & inputData,
                                       const SubchunkIndex & subChunkIndex);

    void validateImpl ();

private:

    Bool advanceVi_p; // true is VI is to be advanced after each write.
                      // N.B., advancing the flow graphs VI is prohibited
    Bool disableOutput_p; // true if output is disabled.
    VisibilityIterator * vi_p; // VI to use for output.
};

class VpContainer : public VisibilityProcessor {

    friend class VisibilityProcessing;

public:

    // Creates a VpContainer object providing the specified inputs and outputs.
    // These inputs and outputs will potentially be connected to the inputs and
    // outputs of the VPs that are contained in the container.

    VpContainer (const String & name,
                 const vector<String> & inputs = vector<String> (1, "In"),
                 const vector<String> & outputs = vector<String> ());

    virtual ~VpContainer () {}

    // Adds a VP to the container.  Exception if VP is already in the container.

    virtual void add (VisibilityProcessor * processor);

    // Connects the specified output to the specified input.  The VP pointer may be
    // omitted if the port belongs to the container.

    virtual void connect (VisibilityProcessor * sourceVp, const String &  sourcePortName,
                          VisibilityProcessor * sinkVp, const String &  sinkPortName);
    virtual void connect (const String &  sourcePortName,
                          VisibilityProcessor * sinkVp, const String &  sinkPortName);
    virtual void connect (VisibilityProcessor * sourceVp, const String &  sourcePortName,
                          const String &  sinkPortName);

    virtual void chunkStart (const SubchunkIndex & sci);

    // Fills the container with the specified set of VPs.  The container must be
    // empty prior to this call.

    virtual void fillWithSequence (VisibilityProcessor * first, ...); // Last one NULL

    // Returns the columns that are required to be prefetched if async I/O is used.

    virtual casa::asyncio::PrefetchColumns getPrefetchColumns () const;

protected:

    typedef vector<VisibilityProcessor *> VPs; // VPs are used (not owned)
    typedef boost::indirect_iterator <VPs::const_iterator> const_iterator;
    typedef boost::indirect_iterator <VPs::iterator> iterator;

    iterator begin();
    const_iterator begin() const;

    Bool contains (const VisibilityProcessor *) const;
    virtual ProcessingResult doProcessingImpl (ProcessingType processingType,
                                               VpData & inputData,
                                               const SubchunkIndex & subChunkIndex);
    Bool empty () const;
    iterator end();
    const_iterator end() const;
    virtual void processingStartImpl ();
    size_t size() const;
    virtual void validateImpl ();

private:

    typedef std::map<VpPort, VpPort> Network;
    typedef std::set<VpPort> NetworkReverse;
    typedef boost::tuple<VisibilityProcessor *, VpData> ReadyVpAndData;

    class VpSet : public std::set<VisibilityProcessor *> {
    public:

        template <typename In>
        VpSet (In begin, In end) : std::set<VisibilityProcessor *> (begin, end) {}
        String getNames () const;
    };

    Network network_p; // connections between the ports of the connected nodes
    NetworkReverse networkReverse_p; // connections of contets except indexed in
                                     // backwards order.
    VPs vps_p; // the VPs contained by this container.

    ReadyVpAndData findReadyVp (VpSet & vpsWaiting, VpData & inputs, bool flushing) const;
    ReadyVpAndData findReadyVpFlushing (VpSet & vpsWaiting, VpData & inputs) const;
    ReadyVpAndData findReadyVpNormal (VpSet & vpsWaiting, VpData & inputs) const;
    bool follows (const VisibilityProcessor * a, const VisibilityProcessor * b) const;
    bool followsSet (const VisibilityProcessor * a, const VpSet & vpSet) const;
    void orderContents ();
    void remapPorts (VpData & data, const VisibilityProcessor *);
    pair<VpPort,VpPort> validateConnectionPorts (VisibilityProcessor * sourceVp,
                                                 const String &  sourcePortName,
                                                 VisibilityProcessor * sinkVp,
                                                 const String &  sinkPortName);
};

class VpEngine {

    friend class VisibilityProcessor;

public:

    VpEngine () : vi_p (NULL) {}

    // Process the data set swept by the VisibilityIterator using the
    // VisibilityProcessor provided with the optionally specified port
    // as the input.

    void process (VisibilityProcessor & processor,
                  ROVisibilityIterator & vi,
                  const String & inputPortName);

    void process (VisibilityProcessor & processor,
                  ROVisibilityIterator & vi,
                  const VpPort & inputPort = VpPort ());

    static Int getLogLevel ();
    static void log (const String & format, ...);
    static String getAipsRcBase ();

private:

    ROVisibilityIterator * vi_p; // [use]

    static Int logLevel_p;
    static LogIO * logIo_p;
    static Bool loggingInitialized_p;
    static LogSink * logSink_p;

    static Bool initializeLogging ();

    ROVisibilityIterator * getVi ();

};

} // end namespace vpu

} // end namespace casa


/*

    VisibilityProcessor vp1;
    VisibilityProcessor vp2;
    VpuContainer vpc1;

    vpc1.add (vp1);
    vpc1.add (vp2);

    vpc1.connect (vp1.getOutput (Out), vp2.getInput (In));
    vpc1.connect (vpc1.getInput (In), vp1.getInput (In));
    vpc1.connect (vp2.getOutput (Out), vpc1.getOutput (Out));

    VpuContainer vpc2;
    VpuContainer vpc0;

    vpc0.add (vpc1, vpc2);
    vpc0.connect (vpc1.getOutput (Out), vpc2.getOutput (In));
    vpc0.connect (vpc0.getOutput (In), vpc1.getInput (In));
    vpc0.connect (vpc1.getOutput (Out), vpc0.getOutput (Out));

    vpc0.validate ();

 */



#endif /* VISIBILITYPROCESSING_H_ */

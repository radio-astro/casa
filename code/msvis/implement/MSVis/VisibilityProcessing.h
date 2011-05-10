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
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <map>
#include <set>
#include <vector>

namespace casa {

namespace asyncio {
    class PrefetchColumns;
};

namespace vpf {

class VisibilityProcessor;
class VpContainer;

class SubChunkIndex {

    friend class SubChunkIndex_Test;

public:

    enum {Invalid = -1};

    SubChunkIndex (Int chunkNumber = Invalid, Int subChunkNumber = Invalid, Int iteration = Invalid);

    Bool operator< (const SubChunkIndex & other) const;
    Bool operator== (const SubChunkIndex & other) const { return ! (* this < other || other < * this);}
    Bool operator!= (const SubChunkIndex & other) const { return ! (* this == other);}

    Int getChunkNumber () const;
    Int getIteration () const;
    Int getSubChunkNumber () const;

    String toString () const;

private:

    Int chunkNumber_p;        // -1 for invalid
    Int iteration_p;          // -1 for invalid
    Int subChunkNumber_p;
};

class VbPtr {

    friend class VbPtr_Test;

public:

    VbPtr ();
    explicit VbPtr (VisBuffer * vb, Bool destroyIt = True);
    VbPtr (const VbPtr & other);
    ~VbPtr ();

    VbPtr & operator= (const VbPtr & other);
    VisBuffer & operator* ();
    VisBuffer * operator-> ();

    void clear ();
    Bool null () const;

//    VisBuffer * get ();
//    VisBuffer * release ();

private:

    class MutexLocker {
    public:

        MutexLocker () {} // no op for now
    };

    typedef CountedPtr <VisBuffer>  CtdPtr;

    CtdPtr * vb_p;

    Int getNRefs () const; // for testing only
};

class VpPort {

    friend class VpContainer;
    friend class VpPort_Test;

public:

    typedef enum {Input = 1, Output = 2, InOutput = Input | Output} Type;

    VpPort () : visibilityProcessor_p (NULL) {}
    VpPort (VisibilityProcessor * vp, const String & name, Type type);
    ~VpPort () {}

    Bool operator< (const VpPort & other) const;
    Bool operator== (const VpPort & other) const;

    Bool empty () const;
    String getFullName () const;
    String getName () const;
    Type getType () const;
    Bool isConnectedInput () const;
    Bool isConnectedOutput () const;
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
    VpData (const VpPort & port, VisBuffer *, Bool deleteIt = True);

    void add (const VpPort & port, VisBuffer *, Bool deleteIt = True);
    VpData getSelection (const VpPorts &) const;
    String getNames () const;
};


class VisibilityProcessor : boost::noncopyable {

    friend class VpContainer;

public:

    typedef enum {
        Normal,
        RepeatChunk
    } ChunkCode;

    typedef enum {
        SubChunk,
        EndOfChunk,
        EndOfData
    } ProcessingType;

//    typedef enum {
//        Nominal,
//        Averager,
//        Outputter,
//        MultiOut
//    } Type;

    typedef boost::tuple <ChunkCode, VpData> ProcessingResult;

    VisibilityProcessor (const String & name,
                         const vector<String> & inputNames,
                         const vector<String> & outputNames = vector<String>());
    virtual ~VisibilityProcessor () {}

    virtual void chunkStart (const SubChunkIndex &) {}
    virtual ProcessingResult doProcessing (ProcessingType processingType,
                                           VpData & inputData,
                                           const SubChunkIndex & subChunkIndex) = 0;
    const VpContainer * getContainer () const { return NULL;}
    String getFullName () const;
    VpPort getInput (const String & name) const;
    VpPorts getInputs (Bool connectedOnly = False) const;
    String getName () const;
    VpPort getOutput (const String & name) const;
    VpPorts getOutputs () const;
    virtual casa::asyncio::PrefetchColumns getPrefetchColumns () const;
    virtual void processingStart () {}
    virtual void validate (const VpPorts & inputs, const VpPorts & outputs) = 0;

protected:

    VpPorts definePorts (const vector<String> & portNames, VpPort::Type type, const String & typeName);

private:

    //VisibilityProcessor (Type t);

    VpPort & getInputRef (const String & name);
    VpPort & getOutputRef (const String & name);
    void setContainer (const VpContainer *);

    // Prevent copying of existing objects

    VisibilityProcessor (const VisibilityProcessor & other); // do not define
    VisibilityProcessor & operator=(const VisibilityProcessor & other); // do not define

    const VpContainer * container_p; // [use]
    String name_p;
//    Type type_p;
    VpPorts vpInputs_p;
    VpPorts vpOutputs_p;
};

class VisibilityProcessorStub : public VisibilityProcessor {

public:

    VisibilityProcessorStub (const String & name)
    : VisibilityProcessor (name, utilj::Strings(), utilj::Strings())
    {}

    ProcessingResult doProcessing (ProcessingType /*processingType*/,
                                   VpData & /*inputData*/,
                                   const SubChunkIndex & /*subChunkIndex*/)
    {
        return ProcessingResult (Normal, VpData());
    }

    void validate (const VpPorts & /*inputs */, const VpPorts & /*outputs*/) {}


};

class SimpleVp: public VisibilityProcessor {

public:

    SimpleVp (const String & input = "In", const string & output = "");
    virtual ~SimpleVp ();

    ProcessingResult doProcessing (ProcessingType processintType,
                                   VpData & inputData,
                                   const SubChunkIndex & subChunkIndex);
    ProcessingResult processingComplete ();

    void validate (const VpPorts & inputs, const VpPorts & outputs);

protected:

    class SimpleResult : public boost::tuple<ChunkCode, VisBuffer *> {};

    virtual SimpleResult doProcessing (ProcessingType processType,
                                       VisBuffer * vb,
                                       const SubChunkIndex & subChunkIndex) = 0;
private:

};

//class VisibilityAverager : public VisibilityProcessor {
//public:
//
//    VisibilityAverager () : VisibilityProcessor (Averager) {}
//    virtual ~VisibilityAverager ();
//
//    Result doProcessing (VisibilityBuffer & vb, Bool isNewChunk, Bool isNewSubChunk);
//
//};

class VpContainer : public VisibilityProcessor {

    friend class VisibilityProcessing;

public:

    VpContainer (); // create with one input, In and no outputs
    VpContainer (const vector<String> & inputs, const vector<String> & outputs = vector<String> ());

    virtual ~VpContainer ();

    virtual void add (VisibilityProcessor * processor);
    virtual void connect (VpPort & sourcePort,
                          VpPort & sinkPort);
    virtual void chunkStart (const SubChunkIndex & sci);
    virtual ProcessingResult doProcessing (ProcessingType processingType,
                                           VpData & inputData,
                                           const SubChunkIndex & subChunkIndex);
    virtual void fillWithSequence (VisibilityProcessor * first, ...); // Last one NULL
    virtual void processingStart ();
    virtual void validate (const VpPorts & inputs, const VpPorts & outputs);

protected:

    typedef vector<VisibilityProcessor *> VPs;
    typedef boost::indirect_iterator <VPs::const_iterator> const_iterator;
    typedef boost::indirect_iterator <VPs::iterator> iterator;

    iterator begin();
    const_iterator begin() const;
    Bool contains (const VisibilityProcessor *) const;
    Bool empty () const;
    iterator end();
    const_iterator end() const;
    size_t size() const;

private:

    typedef std::map<VpPort, VpPort> Network;
    typedef std::set<VpPort> NetworkReverse;
    class VpSet : public std::set<VisibilityProcessor *> {
    public:

        template <typename In>
        VpSet (In begin, In end) : std::set<VisibilityProcessor *> (begin, end) {}
        String getNames () const;
    };

    Network network_p;
    NetworkReverse networkReverse_p;
    VPs vps_p;

    boost::tuple<VisibilityProcessor *, VpData>
    findReadyVp (VpSet & vpsWaiting, VpData & inputs) const;
    void remapPorts (VpData & data, const VisibilityProcessor *);
};

class VpEngine {

public:

    void process (VisibilityProcessor & processor, ROVisibilityIterator & vi, VpPort inputPort = VpPort ());

    static Int getLogLevel ();
    static void log (const String & format, ...);
    static String getAipsRcBase ();

private:


    static Int logLevel_p;
    static LogIO * logIo_p;
    static Bool loggingInitialized_p;

    static Bool initializeLogging ();

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

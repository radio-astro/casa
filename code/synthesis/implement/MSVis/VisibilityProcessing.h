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

    typedef enum {Unknown, Input = 1, Output = 2, InOutput = Input | Output} Type;

    VpPort ();
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

    void add (const VpPort & port, VbPtr);
    VpData getSelection (const VpPorts &, bool missingIsOk = False) const;
    String getNames () const;
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
        Subchunk,
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

    VisibilityProcessor ();
    VisibilityProcessor (const String & name,
                         const vector<String> & inputNames,
                         const vector<String> & outputNames = vector<String>(),
                         Bool makeIoPorts = False);
    virtual ~VisibilityProcessor () {}

    void chunkStart (const SubchunkIndex &);
    ProcessingResult doProcessing (ProcessingType processingType,
                                   VpData & inputData,
                                   VpEngine * vpEngine,
                                   const SubchunkIndex & subChunkIndex);
    const VpContainer * getContainer () const { return NULL;}
    String getFullName () const;
    VpPort getInput (const String & name) const;
    VpPort & getInputRef (const String & name);
    VpPorts getInputs (Bool connectedOnly = False) const;
    String getName () const;
    Int getNSubchunksProcessed () const;
    Int getNSubchunksUniqueProcessed () const;
    VpPort getOutput (const String & name) const;
    VpPort & getOutputRef (const String & name);
    VpPorts getOutputs (Bool connectedOnly = False) const;
    virtual casa::asyncio::PrefetchColumns getPrefetchColumns () const;
    void processingStart ();
    void validate ();

protected:


    virtual void chunkStartImpl (const SubchunkIndex &) {}
    VpPorts definePorts (const vector<String> & portNames, VpPort::Type type, const String & typeName);
    virtual ProcessingResult doProcessingImpl (ProcessingType processingType,
                                               VpData & inputData,
                                               const SubchunkIndex & subChunkIndex) = 0;
    VpPorts portsUnconnected (const VpPorts & ports, Bool (VpPort::* isConnected) () const,
                              const vector<String> & except = vector<String> ()) const;
    virtual void processingStartImpl () {}
    void throwIfAnyInputsUnconnected (const vector<String> & exceptThese = vector<String> ()) const;
    void throwIfAnyInputsUnconnectedExcept (const String & exceptThisOne) const;
    void throwIfAnyOutputsUnconnected (const vector<String> & exceptThese = vector<String> ()) const;
    void throwIfAnyOutputsUnconnectedExcept (const String & exceptThisOne) const;
    void throwIfAnyPortsUnconnected () const;
    virtual void validateImpl () = 0;

private:

    //VisibilityProcessor (Type t);

    void setContainer (const VpContainer *);

    // Prevent copying of existing objects

    VisibilityProcessor (const VisibilityProcessor & other); // do not define
    VisibilityProcessor & operator=(const VisibilityProcessor & other); // do not define

    ROVisibilityIterator * getVi ();
    VpEngine * getVpEngine();

    const VpContainer * container_p; // [use]
    String name_p;
    Int nSubchunks_p;
    Int nSubchunksUnique_p;
//    Type type_p;
    VpEngine * vpEngine_p;
    VpPorts vpInputs_p;
    VpPorts vpOutputs_p;
};

ostream & operator<< (ostream & os, const VisibilityProcessor::ProcessingType & processingType);
String toString (VisibilityProcessor::ProcessingType p);

class VisibilityProcessorStub : public VisibilityProcessor {

public:

    VisibilityProcessorStub (const String & name)
    : VisibilityProcessor (name, utilj::Strings(), utilj::Strings())
    {}

    ProcessingResult doProcessingImpl (ProcessingType /*processingType*/,
                                       VpData & /*inputData*/,
                                       const SubchunkIndex & /*subChunkIndex*/)
    {
        return ProcessingResult (Normal, VpData());
    }

    void validateImpl () {}


};

class SimpleVp: public VisibilityProcessor {

public:

    SimpleVp (const String & input = "In", const string & output = "");
    virtual ~SimpleVp ();


    void validateImpl ();

protected:

    class SimpleResult : public boost::tuple<ChunkCode, VisBuffer *> {};

    ProcessingResult doProcessingImpl (ProcessingType processingType,
                                       VpData & inputData,
                                       const SubchunkIndex & subChunkIndex);

private:

};

class SplitterVp : public VisibilityProcessor {

public:

    SplitterVp (const String & name,
                const vector<String> & inputNames,
                const vector<String> & outputNames);

    ~SplitterVp () {}

protected:

    ProcessingResult doProcessingImpl (ProcessingType processingType ,
                                       VpData & inputData,
                                       const SubchunkIndex & subChunkIndex);

    void validateImpl ()
    {}
};

class WriterVp: public VisibilityProcessor {

public:

    WriterVp (const String & name,
              VisibilityIterator * vi = NULL,
              Bool advanceVi = False,
              const String & input = "In",
              const String & output = "Out");

    Bool setDisableOutput (Bool disableIt);

protected:

    ProcessingResult doProcessingImpl (ProcessingType processingType,
                                       VpData & inputData,
                                       const SubchunkIndex & subChunkIndex);

    void validateImpl ();

private:

    Bool advanceVi_p;
    Bool disableOutput_p;
    VisibilityIterator * vi_p;
};

//class VisibilityAverager : public VisibilityProcessor {
//public:
//
//    VisibilityAverager () : VisibilityProcessor (Averager) {}
//    virtual ~VisibilityAverager ();
//
//    Result doProcessing (VisibilityBuffer & vb, Bool isNewChunk, Bool isNewSubchunk);
//
//};

class VpContainer : public VisibilityProcessor {

    friend class VisibilityProcessing;

public:

    VpContainer (const String & name, const vector<String> & inputs = vector<String> (1, "In"), const vector<String> & outputs = vector<String> ());

    virtual ~VpContainer () {}

    virtual void add (VisibilityProcessor * processor);
    virtual void connect (VpPort & sourcePort,
                          VpPort & sinkPort);
    virtual void chunkStart (const SubchunkIndex & sci);
    virtual void fillWithSequence (VisibilityProcessor * first, ...); // Last one NULL
    virtual casa::asyncio::PrefetchColumns getPrefetchColumns () const;

protected:

    typedef vector<VisibilityProcessor *> VPs;
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
    class VpSet : public std::set<VisibilityProcessor *> {
    public:

        template <typename In>
        VpSet (In begin, In end) : std::set<VisibilityProcessor *> (begin, end) {}
        String getNames () const;
    };

    typedef boost::tuple<VisibilityProcessor *, VpData> ReadyVpAndData;

    Network network_p;
    NetworkReverse networkReverse_p;
    VPs vps_p;

    ReadyVpAndData findReadyVp (VpSet & vpsWaiting, VpData & inputs, bool flushing) const;
    ReadyVpAndData findReadyVpFlushing (VpSet & vpsWaiting, VpData & inputs) const;
    ReadyVpAndData findReadyVpNormal (VpSet & vpsWaiting, VpData & inputs) const;
    bool follows (const VisibilityProcessor * a, const VisibilityProcessor * b) const;
    bool followsSet (const VisibilityProcessor * a, const VpSet & vpSet) const;
    void orderContents ();
    void remapPorts (VpData & data, const VisibilityProcessor *);
};

class VpEngine {

    friend class VisibilityProcessor;

public:

    VpEngine () : vi_p (NULL) {}

    void process (VisibilityProcessor & processor, ROVisibilityIterator & vi, const VpPort & inputPort = VpPort ());

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

/*
 * VlaData.h
 *
 *  Created on: Sep 21, 2011
 *      Author: jjacobs
 */

#ifndef ASYNCHRONOUS_INTERFACE_H_
#define ASYNCHRONOUS_INTERFACE_H_

#include "AsynchronousTools.h"
#include "UtilJ.h"

using casa::utilj::ThreadTimes;
using casa::utilj::DeltaThreadTimes;

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Quanta/MVRadialVelocity.h>
#include <measures/Measures/MRadialVelocity.h>
#include <measures/Measures/MDoppler.h>
#include <synthesis/MSVis/VisBufferAsync.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisibilityIteratorImpl.h>

#include <boost/noncopyable.hpp>
///#pragma GCC diagnostic ignored "-Wno-missing-field-initializers"
///#warning "Disabling GCC diagnostic to handle spurious boost warnings"
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
///#pragma GCC diagnostic warning "-Wno-missing-field-initializers"
#include <memory>
#include <queue>
#include <vector>

namespace casa {

class ROVisibilityIterator;

namespace asyncio {

class RoviaModifier {
public:

    friend std::ostream & operator<< (std::ostream & o, const RoviaModifier & m);

    virtual ~RoviaModifier () {}
    virtual void apply (ROVisibilityIterator *) const = 0;

private:

    virtual void print (std::ostream & o) const = 0;

};

class ChannelSelection {

public:

    ChannelSelection () {}

    ChannelSelection (const Block< Vector<Int> > & blockNGroup,
                      const Block< Vector<Int> > & blockStart,
                      const Block< Vector<Int> > & blockWidth,
                      const Block< Vector<Int> > & blockIncr,
                      const Block< Vector<Int> > & blockSpw);

    ChannelSelection (const ChannelSelection & other);
    ChannelSelection & operator= (const ChannelSelection & other);


    void
    get (Block< Vector<Int> > & blockNGroup,
         Block< Vector<Int> > & blockStart,
         Block< Vector<Int> > & blockWidth,
         Block< Vector<Int> > & blockIncr,
         Block< Vector<Int> > & blockSpw) const;

protected:

    void copyBlock (const Block <Vector<Int> > & src,
                    Block <Vector<Int> > & to) const;

private:

    Block< Vector<Int> > blockNGroup_p;
    Block< Vector<Int> > blockStart_p;
    Block< Vector<Int> > blockWidth_p;
    Block< Vector<Int> > blockIncr_p;
    Block< Vector<Int> > blockSpw_p;
};


class SelectChannelModifier : public RoviaModifier {

public:

    SelectChannelModifier (Int nGroup, Int start, Int width, Int increment, Int spectralWindow);
    SelectChannelModifier (const Block< Vector<Int> > & blockNGroup,
                           const Block< Vector<Int> > & blockStart,
                           const Block< Vector<Int> > & blockWidth,
                           const Block< Vector<Int> > & blockIncr,
                           const Block< Vector<Int> > & blockSpw);

    void apply (ROVisibilityIterator *) const;

private:

    Bool channelBlocks_p;
    ChannelSelection channelSelection_p;
    Int increment_p;
    Int nGroup_p;
    Int spectralWindow_p;
    Int start_p;
    Int width_p;

    void print (std::ostream & o) const;
    String toCsv (const Block< Vector<Int> > & bv) const;
    String toCsv (const Vector<Int> & v) const;

};

class SetIntervalModifier : public RoviaModifier {

public:

    SetIntervalModifier  (Double timeInterval);
    void apply (ROVisibilityIterator *) const;

private:

    Double timeInterval_p;

    void print (std::ostream & o) const;
};


class SetRowBlockingModifier : public RoviaModifier {

public:

    SetRowBlockingModifier (Int nRows);
    void apply (ROVisibilityIterator *) const;

private:

    Int nRows_p;
    Int nGroup_p;
    Int spectralWindow_p;
    Int start_p;
    Int width_p;

    void print (std::ostream & o) const;
};

class RoviaModifiers {

public:

    ~RoviaModifiers ();

    void add (RoviaModifier *);
    void apply (ROVisibilityIterator *);
    void clearAndFree ();
    RoviaModifiers transferModifiers ();

private:

    typedef std::vector<RoviaModifier *> Data;
    Data data_p;

};

class SelectVelocityModifier : public RoviaModifier {

public:

    SelectVelocityModifier (Int nChan, const MVRadialVelocity& vStart, const MVRadialVelocity& vInc,
                            MRadialVelocity::Types rvType, MDoppler::Types dType, Bool precise);
    void apply (ROVisibilityIterator *) const;

private:

    MDoppler::Types dType_p;
    Int nChan_p;
    Bool precise_p;
    MRadialVelocity::Types rvType_p;
    MVRadialVelocity vInc_p;
    MVRadialVelocity vStart_p;

    virtual void print (std::ostream & o) const;

};


// <summary>
//    VlaDatum is a single elemement in the VlaDatum buffer ring used to support the
//    ROVisibilityIteratorAsync.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> VisBuffer
//   <li> VisBufferAsync
//   <li> ROVisibilityIteratorAsync
//   <li> VlaData
//   <li> VLAT
// </prerequisite>
//
// <etymology>
//    VlaDatum is the quantum of data associated with a single position of the visibility
//    look-ahead scheme.
// </etymology>
//
// <synopsis>
//    VlaDatum is a single buffer for data produced by the VLAT thread and consumed by the
//    main thread.  A collection of VlaDatum objects is organized as a buffer ring in a
//    VlaData object.
//
//    A VlaDatum object is responsible for maintaining its state as well as containing the set
//    of data accessed from a single position of a ROVisibilityIterator.  It contains a
//    VisibilityBufferAsync object to hold the data that will be used by the main thread; other
//    data is maintained in member variables.
//
//    VlaDatum has no concurrency mechanisms built in it; that is handled by the VlaData object.
//    It does support a set of states that indicate its current use:
//        Empty -> Filling -> Full -> Reading -> Empty.
//    Changing state is accomplished by the methods fillStart, fillComplete, readStart and readComplete.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li>AipsError for unhandleable errors
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>


class VlaDatum {

public:

    typedef enum {Empty, Filling, Full, Reading} State;

    VlaDatum (SubChunkPair);
    ~VlaDatum ();

    SubChunkPair  getSubChunkPair () const;
    VisBufferAsync * getVisBuffer ();
    //const VisBufferAsync * getVisBuffer () const;
    Bool isSubChunk (SubChunkPair) const;

    VisBufferAsync * releaseVisBufferAsync ();
    void reset ();

protected:

private:

    SubChunkPair     subchunk_p;
    VisBufferAsync * visBuffer_p;

    // Illegal operations

    VlaDatum & operator= (const VlaDatum & other);

};

class VLAT;

// <summary>
//    The VlaData class is a buffer ring used to support the communication between
//    the visiblity lookahead thread (VLAT) and the main application thread.  It
//    implements the required concurrency control to support this communication.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> VisBuffer
//   <li> VisBufferAsync
//   <li> ROVisibilityIteratorAsync
//   <li> VlaData
//   <li> VLAT
// </prerequisite>
//
// <etymology>
//    VlaData is the entire collection of visibility look-ahead data currently (or potentially)
//    shared between the lookahead and main threads.
// </etymology>
//
// <synopsis>
//    The VlaData object supports the sharing of information between the VLAT look ahead thread and
//    the main thread.  It contains a buffer ring of VlaDatum objects which each hold all of the
//    data that is normally access by a position of a ROVisibiltyIterator.  Other data that is shared
//    or communicated between the two threads is also managed by VlaData.
//
//    A single mutex (member variable mutex_p) is used to protect data that is shared between the
//    two threads.  In addition there is a single PThreads condition variable, vlaDataChanged_p used
//    to allow either thread to wait for the other to change the state of VlaData object.
//
//    Buffer ring concurrency has two levels: the mutex protecting VlaData and the state of the
//    VlaDatum object.  Whenever a free buffer (VlaDatum object) is available the VLAT thread will
//    fill it with the data from the next position of the ROVisibilityIterator; a buffer is free for
//    filling when it is in the Empty state.  Before the VLAT fills a buffer it must call fillStart;
//    this method will block the caller until the next buffer in the ring becomes free; as a side effect
//    the buffer's state becomes Filling.  After fillStart is complete, the VLAT owns the buffer.
//    When the VLAT is done with the buffer it calls fillComplete to relinquish the buffer; this causes
//    the buffer state to change from Filling to Full.
//
//    The main thread calls readStart to get the next filled buffer; the main thread is blocked until
//    the a filled buffer is available.  When the full buffer is ready its state is changed to Reading
//    and readStart returns.  The VLAT thread will not access the buffer while the main thread is reading.
//    The read operation is terminated by calling readComplete; this changes the buffer state to Empty and
//    does not block the main thread except potentially to acquire the mutex.
//
//    The concurrency scheme is fairly sound except for the possibility of low-level data sharing through
//    CASA data structures.  Some of the CASA containers (e.g., Array<T>) can potentially share storage
//    although it appears that normal operation they do not.  Some problems have been encountered with
//    objects that share data via reference-counted pointers.  For instance, objects derived from
//    MeasBase<T,U> (e.g., MDirection, MPosition, MEpoch, etc.) share the object that serves as the
//    frame of reference for the measurement; only by converting the object to text and back again can
//    a user easily obtain a copy which does not share values with another.  It is possible that other
//    objects deep many layers down a complex object may still be waiting to trip up VlaData's
//    concurrency scheme.
//
//    On unusual interaction mediated by VlaData occurs when it is necessary to reset the visibility
//    iterator back to the start of a MeasurementSet.  This usually happens either at the start of the MS
//    sweep (e.g., to reset the row blocking factor of the iterator) or at the end (e.g., to make an
//    additional pass through the MS).  The main thread requests a reset of the VI and then is blocked
//    until the VI is reset.  The sweepTerminationRequested_p variable is set to true; when the VLAT
//    discovers that this variable is true it resets the buffer ring, repositions its VI to the start
//    of the MS and then informs the blocked main thread by setting viResetComplete to true and
//    signalling vlaDataChanged_p.
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li> AipsError
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class AsynchronousInterface;
class InterfaceController;

class VlaData {

public:

    VlaData (Int maxNBuffers, async::Mutex & mutex);
    ~VlaData ();

    Bool fillCanStart () const;
    void fillComplete (VlaDatum * datum);
    VlaDatum * fillStart (SubChunkPair, const ThreadTimes & fillStartTime);
    asyncio::ChannelSelection getChannelSelection () const;
    void initialize (const AsynchronousInterface *);
    void insertValidChunk (Int chunkNumber);
    void insertValidSubChunk (SubChunkPair);
    Bool isValidChunk (Int chunkNumber) const;
    Bool isValidSubChunk (SubChunkPair) const;
    void readComplete (SubChunkPair);
    VisBufferAsync * readStart (SubChunkPair);
    void resetBufferData ();
    void setNoMoreData ();
    void storeChannelSelection (const asyncio::ChannelSelection & channelSelection);

    //static void debugBlock ();
    //static void debugUnblock ();
    //static Bool logThis (Int level);

    //static Bool loggingInitialized_p;
    //static Int logLevel_p;

protected:

private:

    typedef std::queue<VlaDatum *> Data;
    typedef std::queue<Int> ValidChunks;
    typedef std::queue<SubChunkPair> ValidSubChunks;

    class Timing {
    public:
        ThreadTimes      fill1_p;
        ThreadTimes      fill2_p;
        ThreadTimes      fill3_p;
        DeltaThreadTimes fillCycle_p;
        DeltaThreadTimes fillOperate_p;
        DeltaThreadTimes fillWait_p;
        ThreadTimes      read1_p;
        ThreadTimes      read2_p;
        ThreadTimes      read3_p;
        DeltaThreadTimes readCycle_p;
        DeltaThreadTimes readOperate_p;
        DeltaThreadTimes readWait_p;
        ThreadTimes      timeStart_p;
        ThreadTimes      timeStop_p;
    };

    asyncio::ChannelSelection     channelSelection_p; // last channels selected for the VI in use
    Data                          data_p;             // Buffer queue
    const AsynchronousInterface * interface_p;
    const Int                     MaxNBuffers_p;
    async::Mutex &                mutex_p; // provided by Asynchronous interface
    Timing                        timing_p;
    mutable ValidChunks           validChunks_p;       // Queue of valid chunk numbers
    mutable ValidSubChunks        validSubChunks_p; // Queue of valid subchunk pairs


    Int clock (Int arg, Int base);
    String makeReport ();

    Bool statsEnabled () const;
    void terminateSweep ();

    //// static Semaphore debugBlockSemaphore_p; // used to block a thread for debugging

    static Bool initializeLogging ();

    // Illegal operations

    VlaData (const VlaData & other);
    VlaData & operator= (const VlaData & other);
};

class WriteData {

public:

    WriteData (const SubChunkPair & subchunkPair) : subchunkPair_p (subchunkPair) {}

    virtual ~WriteData () {}

    SubChunkPair getSubChunkPair () const { return subchunkPair_p;}
    virtual void write (VisibilityIterator * vi) = 0;

private:

    SubChunkPair subchunkPair_p;

};

template <typename Data>
class WriteDataImpl : public WriteData {
public:

    typedef void (VisibilityIterator::* Setter) (const Data &);

    WriteDataImpl (const SubChunkPair & subchunkPair,
                   const Data & data,
                   Setter setter)
    : WriteData (subchunkPair),
      data_p (),
      setter_p (setter)
    {
        data_p.assign (data); // Make a pure copy
    }

    void
    write (VisibilityIterator * vi)
    {
        (vi ->* setter_p) (data_p);
    }

private:

    Data   data_p;
    Setter setter_p;

};

template <typename Data>
WriteData *
createWriteData (const SubChunkPair & subchunkPair,
                 const Data & data,
                 void (VisibilityIterator::* setter) (const Data &))
{
    return new WriteDataImpl<Data> (subchunkPair, data, setter);
}

template <typename Data>
class WriteDataImpl2 : public WriteData {
public:

    typedef ROVisibilityIterator::DataColumn DataColumn;
    typedef void (VisibilityIterator::* Setter) (const Data &, DataColumn);

    WriteDataImpl2 (const SubChunkPair & subchunkPair,
                    const Data & data,
                    DataColumn dataColumn,
                    Setter setter)
    : WriteData (subchunkPair),
      data_p (),
      dataColumn_p (dataColumn),
      setter_p (setter)
    {
        data_p.assign (data); // Make a pure copy
    }

    void
    write (VisibilityIterator * vi)
    {
        (vi ->* setter_p) (data_p, dataColumn_p);
    }

private:

    Data       data_p;
    DataColumn dataColumn_p;
    Setter     setter_p;
};

template <typename Data>
WriteData *
createWriteData (const SubChunkPair & subchunkPair,
                 const Data & data,
                 ROVisibilityIterator::DataColumn dataColumn,
                 void (VisibilityIterator::* setter) (const Data &, ROVisibilityIterator::DataColumn))
{
    return new WriteDataImpl2 <Data> (subchunkPair, data, dataColumn, setter);
}

class AsynchronousInterface;

class WriteQueue {

public:

    WriteQueue ();
    ~WriteQueue ();

    WriteData * dequeue ();
    Bool empty (Bool alreadyLocked = False);
    void enqueue (WriteData * writeData);

    void initialize (const AsynchronousInterface *);

    void write (VisibilityIterator * vi);

private:

    const AsynchronousInterface * interface_p;
    async::Mutex mutex_p;
    std::queue<WriteData *> queue_p;
};


class AsynchronousInterface : private boost::noncopyable {

    //friend class InterfaceController;

public:

    AsynchronousInterface (int maxNBuffers);
    ~AsynchronousInterface ();

    void addModifier (asyncio::RoviaModifier * modifier);
    async::Mutex & getMutex () const;
    //async::LockGuard getLockGuard () const;
    VlaData * getVlaData ();
    VLAT * getVlat ();
    WriteQueue & getWriteQueue ();
    void initialize ();
    Bool isSweepTerminationRequested () const;
    Bool isLookaheadTerminationRequested () const;
    void notifyAllInterfaceChanged () const;
    void requestViReset ();
    pair<Bool, RoviaModifiers> resetVi ();
    void terminate ();
    void terminateLookahead ();
    void terminateSweep ();
    RoviaModifiers transferRoviaModifiers ();
    void viResetComplete ();
    Bool viResetRequested ();
    void waitForInterfaceChange (async::UniqueLock & uniqueLock) const;

    static Bool initializeLogging ();
    static Bool logThis (Int level);

private:

    mutable async::Condition  interfaceDataChanged_p; // Signals interface data has changed
                                                      // o VisBuffer consumed
                                                      // o Write data queued
                                                      // o Sweep or thread termination requested
    volatile Bool             lookaheadTerminationRequested_p; // True to request thread termination
    mutable async::Mutex      mutex_p;                // Mutex protecting access to concurrent data
    asyncio::RoviaModifiers   roviaModifiers_p;
    volatile Bool             sweepTerminationRequested_p;     // True to request sweep termination
                                                               // (e.g., prior to rewinding
    volatile Bool             viResetComplete_p; // VI reset process has completed
    volatile Bool             viResetRequested_p; // VI reset has been requested
    VlaData                   vlaData_p;          // Lookahead data
    VLAT *                    vlat_p;             // Lookahead thread
    WriteQueue                writeQueue_p;       // Data to be written (writable VIs only)

    static Bool loggingInitialized_p;
    static Int logLevel_p;
};

} // end namespace asyncio

} // end namespace casa

#endif /* ASYNCHRONOUS_INTERFACE_H_ */

/*
 * VlaData.h
 *
 *  Created on: Sep 21, 2011
 *      Author: jjacobs
 */

#ifndef ASYNCHRONOUS_INTERFACE2_H_
#define ASYNCHRONOUS_INTERFACE2_H_

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
#include <msvis/MSVis/VisBufferImplAsync2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisibilityIteratorImpl2.h>

///#pragma GCC diagnostic warning "-Wno-missing-field-initializers"
#include <memory>
#include <queue>
#include <vector>
#include <sstream>

namespace casa {

namespace vi {

class VisibilityIterator2;

class RoviaModifier {
public:

    virtual ~RoviaModifier () {}
    virtual void apply (VisibilityIterator2 *) const = 0;
    inline operator std::string( ) const {
        std::stringstream ss;
        print(ss);
        return ss.str( );
    }

private:

    virtual void print (std::ostream & o) const = 0;

};

class ChannelSelection {

public:

    ChannelSelection () {}

    ChannelSelection (const casacore::Block< casacore::Vector<casacore::Int> > & blockNGroup,
                      const casacore::Block< casacore::Vector<casacore::Int> > & blockStart,
                      const casacore::Block< casacore::Vector<casacore::Int> > & blockWidth,
                      const casacore::Block< casacore::Vector<casacore::Int> > & blockIncr,
                      const casacore::Block< casacore::Vector<casacore::Int> > & blockSpw);

    ChannelSelection (const ChannelSelection & other);
    ChannelSelection & operator= (const ChannelSelection & other);


    void
    get (casacore::Block< casacore::Vector<casacore::Int> > & blockNGroup,
         casacore::Block< casacore::Vector<casacore::Int> > & blockStart,
         casacore::Block< casacore::Vector<casacore::Int> > & blockWidth,
         casacore::Block< casacore::Vector<casacore::Int> > & blockIncr,
         casacore::Block< casacore::Vector<casacore::Int> > & blockSpw) const;

protected:

    void copyBlock (const casacore::Block <casacore::Vector<casacore::Int> > & src,
                    casacore::Block <casacore::Vector<casacore::Int> > & to) const;

private:

    casacore::Block< casacore::Vector<casacore::Int> > blockNGroup_p;
    casacore::Block< casacore::Vector<casacore::Int> > blockStart_p;
    casacore::Block< casacore::Vector<casacore::Int> > blockWidth_p;
    casacore::Block< casacore::Vector<casacore::Int> > blockIncr_p;
    casacore::Block< casacore::Vector<casacore::Int> > blockSpw_p;
};


class SelectChannelModifier : public RoviaModifier {

public:

    SelectChannelModifier (casacore::Int nGroup, casacore::Int start, casacore::Int width, casacore::Int increment, casacore::Int spectralWindow);
    SelectChannelModifier (const casacore::Block< casacore::Vector<casacore::Int> > & blockNGroup,
                           const casacore::Block< casacore::Vector<casacore::Int> > & blockStart,
                           const casacore::Block< casacore::Vector<casacore::Int> > & blockWidth,
                           const casacore::Block< casacore::Vector<casacore::Int> > & blockIncr,
                           const casacore::Block< casacore::Vector<casacore::Int> > & blockSpw);

    void apply (VisibilityIterator2 *) const;

private:

    casacore::Bool channelBlocks_p;
    ChannelSelection channelSelection_p;
    casacore::Int increment_p;
    casacore::Int nGroup_p;
    casacore::Int spectralWindow_p;
    casacore::Int start_p;
    casacore::Int width_p;

    void print (std::ostream & o) const;
    casacore::String toCsv (const casacore::Block< casacore::Vector<casacore::Int> > & bv) const;
    casacore::String toCsv (const casacore::Vector<casacore::Int> & v) const;

};

class SetIntervalModifier : public RoviaModifier {

public:

    SetIntervalModifier  (casacore::Double timeInterval);
    void apply (VisibilityIterator2 *) const;

private:

    casacore::Double timeInterval_p;

    void print (std::ostream & o) const;
};


class SetRowBlockingModifier : public RoviaModifier {

public:

    SetRowBlockingModifier (casacore::Int nRows);
    void apply (VisibilityIterator2 *) const;

private:

    casacore::Int nRows_p;
    casacore::Int nGroup_p;
    casacore::Int spectralWindow_p;
    casacore::Int start_p;
    casacore::Int width_p;

    void print (std::ostream & o) const;
};

class RoviaModifiers {

public:

    ~RoviaModifiers ();

    void add (RoviaModifier *);
    void apply (VisibilityIterator2 *);
    void clearAndFree ();
    RoviaModifiers transferModifiers ();

private:

    typedef std::vector<RoviaModifier *> casacore::Data;
    casacore::Data data_p;

};

class SelectVelocityModifier : public RoviaModifier {

public:

    SelectVelocityModifier (casacore::Int nChan, const casacore::MVRadialVelocity& vStart, const casacore::MVRadialVelocity& vInc,
                            casacore::MRadialVelocity::Types rvType, casacore::MDoppler::Types dType, casacore::Bool precise);
    void apply (VisibilityIterator2 *) const;

private:

    casacore::MDoppler::Types dType_p;
    casacore::Int nChan_p;
    casacore::Bool precise_p;
    casacore::MRadialVelocity::Types rvType_p;
    casacore::MVRadialVelocity vInc_p;
    casacore::MVRadialVelocity vStart_p;

    virtual void print (std::ostream & o) const;

};


// <summary>
//    VlaDatum is a single elemement in the VlaDatum buffer ring used to support the
//    VisibilityIterator2Async.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> VisBuffer
//   <li> VisBufferAsync2
//   <li> VisibilityIterator2Async
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
//    of data accessed from a single position of a VisibilityIterator2.  It contains a
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
//    <li>casacore::AipsError for unhandleable errors
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>


class VlaDatum {

public:

    typedef enum {Empty, Filling, Full, Reading} State;

    VlaDatum (Subchunk);
    ~VlaDatum ();

    Subchunk  getSubchunk () const;
    VisBufferImplAsync2 * getVisBuffer ();
    //const VisBufferAsync2 * getVisBuffer () const;
    casacore::Bool isSubchunk (Subchunk) const;

    VisBufferImplAsync2 * releaseVisBufferAsync2 ();
    void reset ();

protected:

private:

    Subchunk     subchunk_p;
    VisBufferImplAsync2 * visBuffer_p;

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
//   <li> VisBufferImplAsync2
//   <li> VisibilityIterator2Async
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
//    data that is normally access by a position of a VisibiltyIterator.  Other data that is shared
//    or communicated between the two threads is also managed by VlaData.
//
//    A single mutex (member variable mutex_p) is used to protect data that is shared between the
//    two threads.  In addition there is a single PThreads condition variable, vlaDataChanged_p used
//    to allow either thread to wait for the other to change the state of VlaData object.
//
//    Buffer ring concurrency has two levels: the mutex protecting VlaData and the state of the
//    VlaDatum object.  Whenever a free buffer (VlaDatum object) is available the VLAT thread will
//    fill it with the data from the next position of the VisibilityIterator2; a buffer is free for
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
//    CASA data structures.  Some of the CASA containers (e.g., casacore::Array<T>) can potentially share storage
//    although it appears that normal operation they do not.  Some problems have been encountered with
//    objects that share data via reference-counted pointers.  For instance, objects derived from
//    casacore::MeasBase<T,U> (e.g., casacore::MDirection, casacore::MPosition, casacore::MEpoch, etc.) share the object that serves as the
//    frame of reference for the measurement; only by converting the object to text and back again can
//    a user easily obtain a copy which does not share values with another.  It is possible that other
//    objects deep many layers down a complex object may still be waiting to trip up VlaData's
//    concurrency scheme.
//
//    On unusual interaction mediated by VlaData occurs when it is necessary to reset the visibility
//    iterator back to the start of a MeasurementSet.  This usually happens either at the start of the MS
//    sweep (e.g., to reset the row blocking factor of the iterator) or at the end (e.g., to make an
//    additional pass through the casacore::MS).  The main thread requests a reset of the VI and then is blocked
//    until the VI is reset.  The sweepTerminationRequested_p variable is set to true; when the VLAT
//    discovers that this variable is true it resets the buffer ring, repositions its VI to the start
//    of the casacore::MS and then informs the blocked main thread by setting viResetComplete to true and
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

    VlaData (casacore::Int maxNBuffers, async::Mutex & mutex);
    ~VlaData ();

    casacore::Bool fillCanStart () const;
    void fillComplete (VlaDatum * datum);
    VlaDatum * fillStart (Subchunk, const ThreadTimes & fillStartTime);
    ChannelSelection getChannelSelection () const;
    void initialize (const AsynchronousInterface *);
    void insertValidChunk (casacore::Int chunkNumber);
    void insertValidSubChunk (Subchunk);
    casacore::Bool isValidChunk (casacore::Int chunkNumber) const;
    casacore::Bool isValidSubChunk (Subchunk) const;
    void readComplete (Subchunk);
    VisBufferImplAsync2 * readStart (Subchunk);
    void resetBufferData ();
    void setNoMoreData ();
    void storeChannelSelection (const ChannelSelection & channelSelection);

    //static void debugBlock ();
    //static void debugUnblock ();
    //static casacore::Bool logThis (casacore::Int level);

    //static casacore::Bool loggingInitialized_p;
    //static casacore::Int logLevel_p;

protected:

private:

    typedef std::queue<VlaDatum *> casacore::Data;
    typedef std::queue<casacore::Int> ValidChunks;
    typedef std::queue<Subchunk> ValidSubChunks;

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

    ChannelSelection              channelSelection_p; // last channels selected for the VI in use
    casacore::Data                          data_p;             // Buffer queue
    const AsynchronousInterface * interface_p;
    const casacore::Int                     MaxNBuffers_p;
    async::Mutex &                mutex_p; // provided by Asynchronous interface
    Timing                        timing_p;
    mutable ValidChunks           validChunks_p;       // casacore::Queue of valid chunk numbers
    mutable ValidSubChunks        validSubChunks_p; // casacore::Queue of valid subchunk pairs


    casacore::Int clock (casacore::Int arg, casacore::Int base);
    casacore::String makeReport ();

    casacore::Bool statsEnabled () const;
    void terminateSweep ();

    //// static Semaphore debugBlockSemaphore_p; // used to block a thread for debugging

    static casacore::Bool initializeLogging ();

    // Illegal operations

    VlaData (const VlaData & other);
    VlaData & operator= (const VlaData & other);
};

class WriteData {

public:

    WriteData (const Subchunk & subchunkPair) : subchunkPair_p (subchunkPair) {}

    virtual ~WriteData () {}

    Subchunk getsubchunk () const { return subchunkPair_p;}
    virtual void write (VisibilityIterator2 * vi) = 0;

private:

    Subchunk subchunkPair_p;

};

template <typename casacore::Data>
class WriteDataImpl : public WriteData {
public:

    typedef void (VisibilityIterator2::* Setter) (const casacore::Data &);

    WriteDataImpl (const Subchunk & subchunkPair,
                   const casacore::Data & data,
                   Setter setter)
    : WriteData (subchunkPair),
      data_p (),
      setter_p (setter)
    {
        data_p.assign (data); // Make a pure copy
    }

    void
    write (VisibilityIterator2 * vi)
    {
        (vi ->* setter_p) (data_p);
    }

private:

    casacore::Data   data_p;
    Setter setter_p;

};

template <typename casacore::Data>
WriteData *
createWriteData (const Subchunk & subchunkPair,
                 const casacore::Data & data,
                 void (VisibilityIterator2::* setter) (const casacore::Data &))
{
    return new WriteDataImpl<casacore::Data> (subchunkPair, data, setter);
}

template <typename casacore::Data>
class WriteDataImpl2 : public WriteData {
public:

    typedef VisibilityIterator2::DataColumn DataColumn;
    typedef void (VisibilityIterator2::* Setter) (const casacore::Data &, DataColumn);

    WriteDataImpl2 (const Subchunk & subchunkPair,
                    const casacore::Data & data,
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
    write (VisibilityIterator2 * vi)
    {
        (vi ->* setter_p) (data_p, dataColumn_p);
    }

private:

    casacore::Data       data_p;
    DataColumn dataColumn_p;
    Setter     setter_p;
};

template <typename casacore::Data>
WriteData *
createWriteData (const Subchunk & subchunkPair,
                 const casacore::Data & data,
                 VisibilityIterator2::DataColumn dataColumn,
                 void (VisibilityIterator2::* setter) (const casacore::Data &, VisibilityIterator2::DataColumn))
{
    return new WriteDataImpl2 <casacore::Data> (subchunkPair, data, dataColumn, setter);
}

class AsynchronousInterface;

class WriteQueue {

public:

    WriteQueue ();
    ~WriteQueue ();

    WriteData * dequeue ();
    casacore::Bool empty (casacore::Bool alreadyLocked = false);
    void enqueue (WriteData * writeData);

    void initialize (const AsynchronousInterface *);

    void write (VisibilityIterator2 * vi);

private:

    const AsynchronousInterface * interface_p;
    async::Mutex mutex_p;
    std::queue<WriteData *> queue_p;
};


class AsynchronousInterface {

    //friend class InterfaceController;

public:

    AsynchronousInterface( const AsynchronousInterface& ) = delete;
    AsynchronousInterface& operator=( const AsynchronousInterface& ) = delete;
    AsynchronousInterface (int maxNBuffers);
    ~AsynchronousInterface ();

    void addModifier (RoviaModifier * modifier);
    async::Mutex & getMutex () const;
    //async::LockGuard getLockGuard () const;
    VlaData * getVlaData ();
    VLAT * getVlat ();
    WriteQueue & getWriteQueue ();
    void initialize ();
    casacore::Bool isSweepTerminationRequested () const;
    casacore::Bool isLookaheadTerminationRequested () const;
    void notifyAllInterfaceChanged () const;
    void requestViReset ();
    pair<casacore::Bool, RoviaModifiers> resetVi ();
    void terminate ();
    void terminateLookahead ();
    void terminateSweep ();
    RoviaModifiers transferRoviaModifiers ();
    void viResetComplete ();
    casacore::Bool viResetRequested ();
    void waitForInterfaceChange (async::UniqueLock & uniqueLock) const;

    static casacore::Bool initializeLogging ();
    static casacore::Bool logThis (casacore::Int level);

private:

    mutable async::Condition  interfaceDataChanged_p; // Signals interface data has changed
                                                      // o VisBuffer consumed
                                                      // o Write data queued
                                                      // o Sweep or thread termination requested
    volatile casacore::Bool             lookaheadTerminationRequested_p; // true to request thread termination
    mutable async::Mutex      mutex_p;                // casacore::Mutex protecting access to concurrent data
    RoviaModifiers            roviaModifiers_p;
    volatile casacore::Bool             sweepTerminationRequested_p;     // true to request sweep termination
                                                               // (e.g., prior to rewinding
    volatile casacore::Bool             viResetComplete_p; // VI reset process has completed
    volatile casacore::Bool             viResetRequested_p; // VI reset has been requested
    VlaData                   vlaData_p;          // Lookahead data
    VLAT *                    vlat_p;             // Lookahead thread
    WriteQueue                writeQueue_p;       // casacore::Data to be written (writable VIs only)

    static casacore::Bool loggingInitialized_p;
    static casacore::Int logLevel_p;
};

} // end namespace vi

} // end namespace casa

#endif /* ASYNCHRONOUS_INTERFACE_H_ */

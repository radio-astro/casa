//# VLAT.h: Visibility lookahead concurrency definitions (classes VlaDatum, VlaData, VLAT)
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning CASA should be addressed as follows:
//#        Internet email: CASA-request@nrao.edu.
//#        Postal address: CASA Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef VLAT_H_
#define VLAT_H_

#include "AsynchronousTools.h"
#include "VisBuffer.h"
#include "VisibilityIterator.h"
#include "VisibilityIteratorAsync.h"
using casa::asyncio::RoviaModifiers;
#include "VisBufferAsync.h"
#include <boost/tuple/tuple.hpp>

#include <memory>
using std::pair;

#include <queue>
using std::queue;

using namespace casa::async;

namespace casa {

class VisBuffer;

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

class SubChunkPair : public pair<Int, Int>{
public:

    SubChunkPair () : pair<Int,Int> (-1, -1) {}
    SubChunkPair (Int a, Int b) : pair<Int,Int> (a,b) {}

    Bool operator== (const SubChunkPair & other){
        return first == other.first && second == other.second;
    }

    Bool operator< (const SubChunkPair & other){
        return first < other.first ||
               (first == other.first && second < other.second);
    }

    String toString () const
    {
        return utilj::format ("(%d,%d)", first, second);
    }

};

class VlaDatum {

public:

	typedef enum {Empty, Filling, Full, Reading} State;

	VlaDatum (Int chunkNumber, Int subChunkNumber);
	~VlaDatum ();

	SubChunkPair  getSubChunkPair () const;
    VisBufferAsync * getVisBuffer ();
    //const VisBufferAsync * getVisBuffer () const;
    Bool isSubChunk (Int chunkNumber, Int subchunkNumber) const;

	VisBufferAsync * releaseVisBufferAsync ();
	void reset ();

protected:

private:

	// Add: --> Cache of values normally obtained from MSIter

	Int              chunkNumber_p;
	Int              subChunkNumber_p;
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

class VlaData {

public:

	VlaData (Int maxNBuffers);
	~VlaData ();

	void addModifier (asyncio::RoviaModifier * modifier);
	void fillComplete (VlaDatum * datum);
	VlaDatum * fillStart (Int chunkNumber, Int subChunkNumber);
	asyncio::ChannelSelection getChannelSelection () const;
	void initialize ();
	void insertValidChunk (Int chunkNumber);
	void insertValidSubChunk (Int chunkNumber, Int subChunkNumber);
	Bool isSweepTerminationRequested () const;
	Bool isValidChunk (Int chunkNumber) const;
	Bool isValidSubChunk (Int chunkNumber, Int subChunkNumber) const;
	void readComplete (Int chunkNumber, Int subChunkNumber);
	VisBufferAsync * readStart (Int chunkNumber, Int subChunkNumber);
    void requestViReset ();
	void setNoMoreData ();
	void storeChannelSelection (const asyncio::ChannelSelection & channelSelection);
    void terminateLookahead ();
	pair<Bool,RoviaModifiers> waitForViReset ();

	static void debugBlock ();
	static void debugUnblock ();

	static Bool loggingInitialized_p;
	static Int logLevel_p;

protected:

private:

	class Stats {
	public:

	    typedef enum {Request=1, Begin=2, End=4, Fill=8} Type;
        enum {Wait,Operate,Cycle} ;

	    Stats ();

	    void addEvent (Int e);
	    Bool isEnabled () const { return enabled_p;}
	    String makeReport ();
	    void reserve (Int size);

	private:

	    typedef boost::tuple <Int,Double> Event;
	    typedef vector<Event> Events;

	    class OpStats {
	    public:

	        OpStats ();

	        Double & operator[] (Int i) { return events_p [i];}

	        void accumulate (Double wait, Double operate, Double cycle);
	        Double getAvg (Int i) { return (n_p != 0) ? sum_p [i] / n_p : 0;}
	        Int getN () const { return n_p;}
	        String format (Int component);
            void update (Int type, Double t);

            static Double dmax (Double a, Double b) { return max (a, b);}
            static Double dmin (Double a, Double b) { return min (a, b);}

	    private:

	        vector<Double> events_p;
	        vector<Double> max_p;
	        vector<Double> min_p;
	        Int n_p;
	        vector<Double> ssq_p;
	        vector<Double> sum_p;
	    };

	    Bool enabled_p;
	    Events events_p;

	};


    typedef queue<VlaDatum *> Data;
    typedef queue<Int> ValidChunks;
    typedef queue<SubChunkPair> ValidSubChunks;

    asyncio::ChannelSelection channelSelection_p; // last channels selected for the VI in use
	Data    data_p;       // Buffer queue
    volatile Bool lookaheadTerminationRequested_p;
    const Int MaxNBuffers_p;
	asyncio::RoviaModifiers roviaModifiers_p;
	Stats   stats_p;
    volatile Bool sweepTerminationRequested_p;
	volatile Bool viResetRequested_p;
    volatile Bool viResetComplete_p;
	mutable ValidChunks validChunks_p;       // Queue of valid chunk numbers
	mutable ValidSubChunks validSubChunks_p; // Queue of valid subchunk pairs
	mutable Condition vlaDataChanged_p;
	mutable Mutex vlaDataMutex_p;

    Int clock (Int arg, Int base);
    void resetBufferData ();
    Bool statsEnabled () const;
	void terminateSweep ();

	//// static Semaphore debugBlockSemaphore_p; // used to block a thread for debugging

	static Bool initializeLogging ();

	// Illegal operations

	VlaData (const VlaData & other);
	VlaData & operator= (const VlaData & other);


};

class MeasurementSet;
template<typename T> class Block;

// VlatFunctor is an abstract class for functor objects used to encapsulate the various
// filling methods (e.g., fillVis, fillAnt1, etc.).  This allows the various functions
// to be put into a list of fill methods that are used by the VLAT everytime the VLAT's
// visibliity iterator is advanced.  There are two subclasses VlatFunctor0 and VlatFunctor1
// which support nullar and unary fill methods.  The fillers for visibility-related data
// (e.g., fillVis and fillVisCube) take a parameter to indicate which sort of visibility
// (e.g., actual, model, corrected) is to be filled.

class VlatFunctor {

public:

	VlatFunctor (Int precedence = 0)
	: id_p (ROVisibilityIteratorAsync::N_PrefetchColumnIds), precedence_p (precedence)
	{}
	virtual ~VlatFunctor () {}

	virtual void operator() (VisBuffer *) { throw AipsError ("Illegal Vlat Functor");}
	virtual VlatFunctor * clone () { return new VlatFunctor (* this);}

	ROVisibilityIteratorAsync::PrefetchColumnIds getId () const { return id_p;}
	void setId (ROVisibilityIteratorAsync::PrefetchColumnIds id) { id_p = id;}
	void setPrecedence (Int precedence) { precedence_p = precedence; }

	static Bool byDecreasingPrecedence (const VlatFunctor * a, const VlatFunctor * b)
	{   // First by increasing precedence and then by decreasing id (make deterministic)
	    Bool result = (a->precedence_p > b->precedence_p) ||
	                  (a->precedence_p == b->precedence_p && a->id_p < b->id_p);
	    return result;
	}
private:

	ROVisibilityIteratorAsync::PrefetchColumnIds id_p;
	Int precedence_p;

};

template <typename Ret>
class VlatFunctor0 : public VlatFunctor {

public:

	typedef Ret (VisBuffer::* Nullary) ();

	VlatFunctor0 (Nullary nullary, Int precedence = 0) : VlatFunctor (precedence), f_p (nullary) {}
	virtual ~VlatFunctor0 () {}

	void operator() (VisBuffer * c) { (c->*f_p)(); }
	virtual VlatFunctor * clone () { return new VlatFunctor0 (* this); }

private:

	Nullary f_p;
};

template <typename Ret>
VlatFunctor0<Ret> * vlatFunctor0 (Ret (VisBuffer::* f) ())
{ return new VlatFunctor0<Ret> (f);}

template <typename Ret, typename Arg>
class VlatFunctor1 : public VlatFunctor {

public:

	typedef Ret (VisBuffer::* Unary) (Arg);

	VlatFunctor1 (Unary unary, Arg arg, Int precedence = 0) : VlatFunctor (precedence) { f_p = unary; arg_p = arg;}
	virtual ~VlatFunctor1 () {}

	void operator() (VisBuffer * c) { (c->*f_p)(arg_p); }
	virtual VlatFunctor * clone () { return new VlatFunctor1 (* this); }

private:

	Unary f_p;
	Arg arg_p;
};

template <typename Ret, typename Arg>
VlatFunctor1<Ret, Arg> * vlatFunctor1 (Ret (VisBuffer::* f) (Arg), Arg i)
{ return new VlatFunctor1<Ret, Arg> (f, i);}

// <summary>
// VLAT is the Visibility LookAhead Thread.  This thread advances a visibility iterator
// and fills the data indicated by the visibility iterator into the VlaData buffer ring.
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
//    VLAT is the Visibility LookAhead Thread.  It is not related to the more common NRAO
//    acronym VLA.
// </etymology>
//
// <synopsis>
//
//    The VLAT is a thread object that buffers up data from successive visibility iterator positions
//    in a MeasurementSet.  It is part of the backend to a ROVisibilityIteratorAsync (ROVIA)
//    object used by the main thread to replace the normal, synchronous ROVisibilityIterator.
//    When the user creates a ROVIA object the information normally used to create a ROVisibilityIterator
//    is passed to the VLAT which uses it to create a ROVisibilityIterator local to itself.  The VLAT then
//    uses this ROVisibilityIterator to fill buffers in the VlaData-managed buffer ring (this interaction
//    is described in VlaData).  Filling consists of attaching VLAT's ROVisibilityIterator to the
//    VisBufferAsync object associated with a buffer and then calling the fill operations for the data
//    items (e.g., visCube, Ant1, etc.) which the user has requested be prefetched.  The fill operations
//    will likely result in synchronous I/O operations being performed by the column access methods
//    related to the Table system (memory-resident tables, columns, etc., may be able to satisfy a fill
//    operation without performing I/O).
//
//    The thread may be terminated by calling the terminate method.  This will cause the VLAT to terminate
//    when it notices the termination request.  The termination may not be immediate since the VLAT may
//    be engaged in a syncrhonous I/O operation and is uanble to detect the termination request until
//    that I/O has completed.
//
//    Normally the VLAT sweeps the VI to the end of the measurement set and then awaits further instructions.
//    The main thread may stop the sweep early by calling VlaData::terminateSweep which will eventually be
//    detected by the VLAT and result in a coordinated reset of the sweep.  When the sweep reset is applied
//    the VLAT will also detect visibility iterator modification requests (e.g., setRowBlocking, selectChannel,
//    setInterval, etc.) that were queued up in VlaData; for the set of available VI modification requests
//    supported see ROVisibilityIteratorAsync.
//
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
//    <li> AipsError for unrecoverable errors.  These will not be caught (in C++ anyway) and cause
//         application termination.
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class VLAT : public casa::async::Thread {

public:

	VLAT (VlaData * vlaData);
	~VLAT ();

	void clearFillTerminationRequest ();
	void initialize (const ROVisibilityIterator & rovi);
	void initialize (const MeasurementSet & ms,
			         const Block<Int> & sortColumns,
                     Double timeInterval=0);
	void initialize (const MeasurementSet & ms,
			         const Block<Int> & sortColumns,
	                 const Bool addDefaultSortCols,
	                 Double timeInterval=0);
	void initialize (const Block<MeasurementSet> & mss,
	                 const Block<Int> & sortColumns,
	                 Double timeInterval=0);
	void initialize (const Block<MeasurementSet> & mss,
	                 const Block<Int> & sortColumns,
	                 const Bool addDefaultSortCols,
	                 Double timeInterval=0);
	Bool isTerminated () const;
	void setModifiers (asyncio::RoviaModifiers & modifiers);
	void setPrefetchColumns (const ROVisibilityIteratorAsync::PrefetchColumns & prefetchColumns);
	void requestSweepTermination ();
	void terminate ();

protected:

	class FillerDictionary : public map<ROVisibilityIteratorAsync::PrefetchColumnIds, VlatFunctor *> {

	public:
	    void add (ROVisibilityIteratorAsync::PrefetchColumnIds id, VlatFunctor * f)
	    {
	        f->setId (id);
	        assert (find(id) == end()); // shouldn't already have one for this ID
	        (* this)[id] =  f;
	    }

	    //void setPrecedences (const FillerDependencies & dependencies);
	};
	typedef vector<VlatFunctor *> Fillers;

    void applyModifiers (ROVisibilityIterator * rovi);
	void createFillerDictionary ();
    void fillDatum (VlaDatum * datum);
    void fillDatumMiscellanyAfter (VlaDatum * datum);
    void fillDatumMiscellanyBefore (VlaDatum * datum);
    void fillLsrInfo (VlaDatum * datum);
    void * run ();
    Bool sweepTerminationRequested () const;
    void sweepVi ();

private:

    //FillerDependencies fillerDependencies_p;
	FillerDictionary fillerDictionary_p;
	Fillers fillers_p;
	asyncio::RoviaModifiers roviaModifiers_p;
	volatile Bool sweepTerminationRequested_p;
	Bool threadTerminated_p;
	ROVisibilityIterator * visibilityIterator_p; // [own]
	VlaData * vlaData_p; // [use]

};

class ROVisibilityIteratorAsyncImpl {

	friend class ROVisibilityIteratorAsync;

public:

protected:

	ROVisibilityIteratorAsyncImpl ();
	~ROVisibilityIteratorAsyncImpl (){}

private:

	VlaData * vlaData_p;
	VLAT * vlat_p;

};

} // end namespace casa



#endif /* VLAT_H_ */

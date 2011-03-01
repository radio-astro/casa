/*
 * VLAT.h
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

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

class VlaDatum {

public:

	typedef enum {Empty, Filling, Full, Reading} State;

	VlaDatum (int id);
	~VlaDatum ();

	void fillComplete ();
	Bool fillStart (Int chunkNumber, Int subChunkNumber);

	Int getChunkNumber () const;
	Int getSubChunkNumber () const;
	Int getId () const;
	//String getStatus () const;
    VisBufferAsync * getVisBuffer (Bool bypassAssert = False);
    const VisBufferAsync * getVisBuffer () const;
    void initialize ();
    Bool isChunk (Int chunkNumber, Int subchunkNumber) const;

	void readComplete ();
	Bool readStart ();
	void reset ();

protected:

private:

	// Add: --> Cache of values normally obtained from MSIter

	Int              chunkNumber_p;
	Int              id_p;
	State            state_p;
	Int              subChunkNumber_p;
	VisBufferAsync * visBuffer_p;

	// Illegal operations

	VlaDatum & operator= (const VlaDatum & other);

};

class VLAT;

class VlaData {

public:


	VlaData (Int nBuffers);
	~VlaData ();

	void addModifier (asyncio::RoviaModifier * modifier);
	void fillComplete ();
	VlaDatum * fillStart (Int chunkNumber, Int subChunkNumber);
	asyncio::ChannelSelection getChannelSelection () const;
	void initialize ();
	void insertValidChunk (Int chunkNumber);
	void insertValidSubChunk (Int chunkNumber, Int subChunkNumber);
	Bool isSweepTerminationRequested () const;
	Bool isValidChunk (Int chunkNumber) const;
	Bool isValidSubChunk (Int chunkNumber, Int subChunkNumber) const;
	void readComplete ();
	const VlaDatum * readStart (Int chunkNumber, Int subChunkNumber);
    void requestViReset ();
	void setNoMoreData ();
	void storeChannelSelection (const asyncio::ChannelSelection & channelSelection);
    void terminateLookahead ();
	pair<Bool,RoviaModifiers> waitForViReset ();

	static void debugBlock ();
	static void debugUnblock ();

	static const Bool loggingInitialized_p;
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

    };

    typedef vector<VlaDatum *> Data;
    typedef queue<Int> ValidChunks;
    typedef queue<SubChunkPair> ValidSubChunks;

    asyncio::ChannelSelection channelSelection_p; // last channels selected for the VI in use
	Data    data_p;       // Buffer ring
	Int     fillIndex_p;  // index of buffer to be filled
    volatile Bool lookaheadTerminationRequested_p;
	Int     readIndex_p;  // index of buffer to be read
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
    VlaDatum * getNextDatum (Int & index);
    void resetBufferRing ();
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

	//class FillerDictionary;

//	class FillerDependencies : public map<ROVisibilityIteratorAsync::PrefetchColumnIds,
//	                                      set<ROVisibilityIteratorAsync::PrefetchColumnIds> >{
//
//    public:
//
//	    void add (ROVisibilityIteratorAsync::PrefetchColumnIds column,
//	              ROVisibilityIteratorAsync::PrefetchColumnIds requiresColumn1, ...);
//        void checkForCircularDependence ();
//        Bool isRequiredBy (ROVisibilityIteratorAsync::PrefetchColumnIds requiree,
//                           ROVisibilityIteratorAsync::PrefetchColumnIds requirer) const;
//        Bool isRequiredByAny (ROVisibilityIteratorAsync::PrefetchColumnIds requiree,
//                              const set<ROVisibilityIteratorAsync::PrefetchColumnIds> & requirers) const;
//	    void performTransitiveClosure (FillerDictionary &);
//	};

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

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
#include "UtilJ.h"

#include <boost/tuple/tuple.hpp>
#include <msvis/MSVis/AsynchronousInterface.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBufferAsync.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisibilityIteratorImplAsync.h>

#include <map>

//using namespace casa::async;
using casa::asyncio::RoviaModifiers;
using casa::utilj::ThreadTimes;
using casa::utilj::DeltaThreadTimes;

namespace casa {

template<typename T> class Block;
class MeasurementSet;
class VisBuffer;

namespace asyncio {

class AsynchronousInterface;
class InterfaceController;

// VlatFunctor is an abstract class for functor objects used to encapsulate the various
// filling methods (e.g., fillVis, fillAnt1, etc.).  This allows the various functions
// to be put into a list of fill methods that are used by the VLAT everytime the VLAT's
// visibliity iterator is advanced.  There are two subclasses VlatFunctor0 and VlatFunctor1
// which support nullar and unary fill methods.  The fillers for visibility-related data
// (e.g., fillVis and fillVisCube) take a parameter to indicate which sort of visibility
// (e.g., actual, model, corrected) is to be filled.

class VlatFunctor {

public:


    VlatFunctor (const String & name, Int precedence = 0)
    : id_p (VisBufferComponents::N_VisBufferComponents), name_p (name), precedence_p (precedence)
    {}
    VlatFunctor (Int precedence = 0)
    : id_p (VisBufferComponents::N_VisBufferComponents), name_p ("NotSpecified"), precedence_p (precedence)
    {}
    virtual ~VlatFunctor () {}

    virtual void operator() (VisBuffer *); // Throws an error if not overridden
    virtual VlatFunctor * clone () { return new VlatFunctor (* this);}

    VisBufferComponents::EnumType getId () const { return id_p;}
    void setId (VisBufferComponents::EnumType id) { id_p = id;}
    void setPrecedence (Int precedence) { precedence_p = precedence; }

    static Bool byDecreasingPrecedence (const VlatFunctor * a, const VlatFunctor * b)
    {   // First by increasing precedence and then by decreasing id (make deterministic)
        Bool result = (a->precedence_p > b->precedence_p) ||
                      (a->precedence_p == b->precedence_p && a->id_p < b->id_p);
        return result;
    }
private:

    VisBufferComponents::EnumType id_p;
    String name_p;
    Int precedence_p;

};

template <typename Ret, typename VbType>
class VlatFunctor0 : public VlatFunctor {

public:

    typedef Ret (VbType::* Nullary) ();

    VlatFunctor0 (Nullary nullary, Int precedence = 0) : VlatFunctor (precedence), f_p (nullary) {}
    virtual ~VlatFunctor0 () {}

    void operator() (VisBuffer * c) { (dynamic_cast<VbType *> (c)->*f_p)(); }
    virtual VlatFunctor * clone () { return new VlatFunctor0 (* this); }

private:

    Nullary f_p;
};

template <typename Ret, typename VbType>
VlatFunctor0<Ret, VbType> * vlatFunctor0 (Ret (VbType::* f) ())
{ return new VlatFunctor0<Ret, VbType> (f);}

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

    VLAT (AsynchronousInterface *);
    ~VLAT ();

    void clearFillTerminationRequest ();
    void initialize (const ROVisibilityIterator & rovi);
    void initialize (const Block<MeasurementSet> & mss,
                     const Block<Int> & sortColumns,
                     Bool addDefaultSortCols,
                     Double timeInterval,
                     Bool writable);
    Bool isTerminated () const;
    void setModifiers (RoviaModifiers & modifiers);
    void setPrefetchColumns (const PrefetchColumns & prefetchColumns);
    void requestSweepTermination ();
    void terminate ();

protected:

    class FillerDictionary : public std::map<VisBufferComponents::EnumType, VlatFunctor *> {

    public:

    void add (VisBufferComponents::EnumType id, VlatFunctor * f)
    {
        f->setId (id);
        assert (find(id) == end()); // shouldn't already have one for this ID
        (* this)[id] =  f;
    }

        //void setPrecedences (const FillerDependencies & dependencies);
    };
    typedef vector<VlatFunctor *> Fillers;

    void applyModifiers (ROVisibilityIterator * rovi, VisibilityIterator * vi);
    void alignWriteIterator (SubChunkPair subchunk);
    void checkFiller (VisBufferComponents::EnumType id);
    void createFillerDictionary ();
    void fillDatum (VlaDatum * datum);
    void fillDatumMiscellanyAfter (VlaDatum * datum);
    void fillDatumMiscellanyBefore (VlaDatum * datum);
    void fillLsrInfo (VlaDatum * datum);
    void flushWrittenData ();
    void handleWrite ();
    void * run ();
    Bool sweepTerminationRequested () const;
    void sweepVi ();
    void throwIfSweepTerminated ();
    Bool waitForViReset ();
    void waitUntilFillCanStart ();

private:

    class SweepTerminated : public std::exception {};

//    class NullaryPredicate {
//    public:
//
//        virtual ~NullaryPredicate () {}
//        virtual Bool operator () () const = 0;
//    };
//
//    class FillCanStartOrSweepTermination : public NullaryPredicate {
//
//    public:
//
//        FillCanStartOrSweepTermination (VlaData * vlaData, AsynchronousInterface * interface)
//        : interface_p (interface),
//          vlaData_p (vlaData)
//        {}
//
//        Bool operator() () const
//        {
//            return vlaData_p->fillCanStart () || interface_p->isSweepTerminationRequested ();
//        }
//
//    private:
//
//        AsynchronousInterface * interface_p;
//        VlaData * vlaData_p;
//    };
//
//    class ViResetOrLookaheadTermination : public NullaryPredicate {
//
//    public:
//
//        ViResetOrLookaheadTermination (AsynchronousInterface * interface)
//        : interface_p (interface)
//        {}
//
//        Bool operator() () const
//        {
//            Bool viWasReset_p = interface_p->viResetRequested;
//
//            return viWasReset_p || interface_p->isLookaheadTerminationRequested ();
//        }
//
//    private:
//
//        AsynchronousInterface * interface_p;
//    };

    const InterfaceController * controller_p; // [use]
    FillerDictionary            fillerDictionary_p;
    Fillers                     fillers_p;
    AsynchronousInterface *     interface_p; // [use]
    Block<MeasurementSet>       measurementSets_p;
    SubChunkPair                readSubchunk_p;
    RoviaModifiers              roviaModifiers_p;
    volatile Bool               sweepTerminationRequested_p;
    Bool                        threadTerminated_p;
    ROVisibilityIterator *      visibilityIterator_p; // [own]
    VlaData *                   vlaData_p; // [use]
    VisibilityIterator *        writeIterator_p; // [own]
    SubChunkPair                writeSubchunk_p;

};

class VlatAndData {

    friend class ViReadImplAsync;

public:

protected:

    VlatAndData ();
    ~VlatAndData (){}

private:

    VlaData * vlaData_p;
    VLAT * vlat_p;
};

} // end namespace asyncio

} // end namespace casa



#endif /* VLAT_H_ */

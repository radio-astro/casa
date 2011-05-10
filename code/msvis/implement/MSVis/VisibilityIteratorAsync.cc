/*
 * VisibilityIteratorAsync.cc
 *
 *  Created on: Nov 2, 2010
 *      Author: jjacobs
 */

#include "VisibilityIteratorAsync.h"
#include "VisBufferAsync.h"
#include "VisBufferAsyncWrapper.h"
#include "VLAT.h"

#include <ms/MeasurementSets/MSColumns.h>
#include <casa/System/AipsrcValue.h>

#include <algorithm>
#include <cstdarg>
#include <ostream>
#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;

#include "UtilJ.h"
using namespace casa::utilj;

using namespace casa::asyncio;

#define Log(level, ...) \
    {if (VlaData::loggingInitialized_p && level <= VlaData::logLevel_p) \
         Logger::get()->log (__VA_ARGS__);};

#define NotImplemented     Throw ("ROVisibilityIteratorAsync: Method not implemented!");

namespace casa {

// The base of the AIPS RC keyword to use for asynchronous I/O keywords

ROVisibilityIteratorAsync::ROVisibilityIteratorAsync (const MeasurementSet & ms,
                                                      const PrefetchColumns & prefetchColumns,
                                                      const Block<Int> & sortColumns,
                                                      Double timeInterval,
                                                      Int nReadAheadBuffers)
: impl_p (NULL),
  visBufferAsync_p (NULL)
{
    construct (prefetchColumns, nReadAheadBuffers);

    measurementSets_p.clear();
    measurementSets_p.push_back (& ms);

    impl_p->vlat_p->initialize (ms, sortColumns, timeInterval);

    startVlat ();
}

ROVisibilityIteratorAsync::ROVisibilityIteratorAsync (const ROVisibilityIterator & rovi,
                                                      const PrefetchColumns & prefetchColumns,
                                                      Int nReadAheadBuffers)
: impl_p (NULL),
  visBufferAsync_p (NULL)
{
    construct (prefetchColumns, nReadAheadBuffers);

    measurementSets_p = rovi.getMeasurementSets ();

    impl_p->vlat_p->initialize (rovi);

    startVlat ();
}


ROVisibilityIteratorAsync::ROVisibilityIteratorAsync (const MeasurementSet & ms,
                                                      const PrefetchColumns & prefetchColumns,
                                                      const Block<Int> & sortColumns,
                                                      const Bool addDefaultSortCols,
                                                      Double timeInterval,
                                                      Int nReadAheadBuffers)
: impl_p (NULL),
  visBufferAsync_p (NULL)
{
    construct (prefetchColumns, nReadAheadBuffers);

    measurementSets_p.clear();
    measurementSets_p.push_back (& ms);

    impl_p->vlat_p->initialize (ms, sortColumns, addDefaultSortCols, timeInterval);

    startVlat ();


}

ROVisibilityIteratorAsync::ROVisibilityIteratorAsync (const Block<MeasurementSet> & mss,
                                                      const PrefetchColumns & prefetchColumns,
                                                      const Block<Int> & sortColumns,
                                                      Double timeInterval,
                                                      Int nReadAheadBuffers)
: impl_p (NULL),
  visBufferAsync_p (NULL)
{
    construct (prefetchColumns, nReadAheadBuffers);

    saveMss (mss);

    impl_p->vlat_p->initialize (mss, sortColumns, timeInterval);

    startVlat ();


}

ROVisibilityIteratorAsync::ROVisibilityIteratorAsync (const Block<MeasurementSet> & mss,
                                                      const PrefetchColumns & prefetchColumns,
                                                      const Block<Int> & sortColumns,
                                                      const Bool addDefaultSortCols,
                                                      Double timeInterval,
                                                      Int nReadAheadBuffers)
: impl_p (NULL),
  visBufferAsync_p (NULL)
{
    construct (prefetchColumns, nReadAheadBuffers);

    measurementSets_p.clear();
    for (uint i = 0; i < mss.size(); i++){
        measurementSets_p.push_back (& mss [i]);
    }

    impl_p->vlat_p->initialize (mss, sortColumns, addDefaultSortCols, timeInterval);

    impl_p->vlat_p->startThread ();

}

ROVisibilityIteratorAsync::~ROVisibilityIteratorAsync ()
{
    if (! vbaWrapperStack_p.empty()){
        VisBufferAsync * vba = vbaWrapperStack_p.top()->releaseVba ();
        assert (vba == visBufferAsync_p);
        delete visBufferAsync_p;
    }
    else if (visBufferAsync_p != NULL){
        delete visBufferAsync_p;
    }

    // Destroy the VLAT

    impl_p->vlat_p->terminate(); // request termination
    impl_p->vlat_p->join();      // wait for it to terminate
    delete impl_p->vlat_p;               // free its storage

    // Destroy the lookahead buffers

    delete impl_p->vlaData_p;

    // Destroy the implementation object

    delete impl_p;
}



ROVisibilityIteratorAsync &
ROVisibilityIteratorAsync::operator++ ()
{
    advance ();

    return * this;
}

ROVisibilityIteratorAsync &
ROVisibilityIteratorAsync::operator++ (int)
{
    advance ();

    return * this;
}

void
ROVisibilityIteratorAsync::advance ()
{
    //readComplete (); // complete any pending read

    subChunkNumber_p ++;

    fillVisBuffer ();

    if (linkedVisibilityIterator_p != NULL){
        ++ (* linkedVisibilityIterator_p);
    }

}

void
ROVisibilityIteratorAsync::attachVisBuffer (VisBuffer & vb0)
{
    VisBufferAsyncWrapper * vb = dynamic_cast<VisBufferAsyncWrapper *> (& vb0);
    ThrowIf (vb == NULL, "Attempt to attach other than VisBufferAsyncWrapper");

    if (! vbaWrapperStack_p.empty()){
        vbaWrapperStack_p.top () -> releaseVba ();
    }

    vbaWrapperStack_p.push (vb);

    if (visBufferAsync_p != NULL){
        vb->wrap (visBufferAsync_p);
    }
}

void
ROVisibilityIteratorAsync::construct (const PrefetchColumns & prefetchColumns, Int nReadAheadBuffers)
{
    if (VlaData::loggingInitialized_p){
        Logger::get()->registerName ("Main");
    }

    chunkNumber_p = 0;
    linkedVisibilityIterator_p = NULL;
    subChunkNumber_p = -1;
    visBufferAsync_p = NULL;

    impl_p = new ROVisibilityIteratorAsyncImpl ();

    if (nReadAheadBuffers <= 0){

         nReadAheadBuffers = getDefaultNBuffers ();
    }

    // Create and initialize the shared data object VlaData

    impl_p->vlaData_p = new VlaData (nReadAheadBuffers);
    impl_p->vlaData_p->initialize ();

    // Create the lookahead thread object

    impl_p->vlat_p = new VLAT (impl_p->vlaData_p);

    // Augment the list of prefetch columns with any that are implied
    // by the others.  N.B., be wary of reordering these.

    prefetchColumns_p = prefetchColumns;

    if (contains (Direction1, prefetchColumns_p)){
        prefetchColumns_p.insert (Feed1_pa);
    }

    if (contains (Direction2, prefetchColumns_p)){
        prefetchColumns_p.insert (Feed2_pa);
    }

    if (contains (Feed1_pa, prefetchColumns_p)){
        prefetchColumns_p.insert (Feed1);
        prefetchColumns_p.insert (Ant1);
        prefetchColumns_p.insert (casa::asyncio::Time);
    }

    if (contains (Feed2_pa, prefetchColumns_p)){
        prefetchColumns_p.insert (Feed2);
        prefetchColumns_p.insert (Ant2);
        prefetchColumns_p.insert (casa::asyncio::Time);
    }

    impl_p->vlat_p->setPrefetchColumns (prefetchColumns_p);
}

ROVisibilityIterator *
ROVisibilityIteratorAsync::create (const ROVisibilityIterator & rovi,
                                   const PrefetchColumns & prefetchColumns,
                                   Int nReadAheadBuffers)
{
    ROVisibilityIterator * result = NULL;

    if (isAsynchronousIoEnabled()){

        result = new ROVisibilityIteratorAsync (rovi,
                                                prefetchColumns,
                                                nReadAheadBuffers);
    }
    else {
        result = new ROVisibilityIterator (rovi);
    }

    Log (2, "Created ROVisibilityIterator%s (1)\n", isAsynchronousIoEnabled() ? "Async" : "");
    //printBacktrace (cerr, "ROVI creation");

    return result;
}





ROVisibilityIterator *
ROVisibilityIteratorAsync::create (const MeasurementSet & ms,
                                   const PrefetchColumns & prefetchColumns,
                                   const Block<Int> & sortColumns,
                                   Double timeInterval,
                                   Int nReadAheadBuffers)
{
    ROVisibilityIterator * result = NULL;

    if (isAsynchronousIoEnabled()){

        result = new ROVisibilityIteratorAsync (ms,
                                                prefetchColumns,
                                                sortColumns,
                                                timeInterval,
                                                nReadAheadBuffers);
    }
    else {
        result = new ROVisibilityIterator (ms,
                                           sortColumns,
                                           timeInterval);
    }

    Log (2, "Created ROVisibilityIterator%s (2)\n", isAsynchronousIoEnabled() ? "Async" : "");
    //printBacktrace (cerr, "ROVI creation");

    return result;
}

ROVisibilityIterator *
ROVisibilityIteratorAsync::create (const MeasurementSet & ms,
                                   const PrefetchColumns & prefetchColumns,
                                   const Block<Int> & sortColumns,
                                   const Bool addDefaultSortCols,
                                   Double timeInterval,
                                   Int nReadAheadBuffers)
{
    ROVisibilityIterator * result = NULL;

    if (isAsynchronousIoEnabled()){

        result = new ROVisibilityIteratorAsync (ms,
                                                prefetchColumns,
                                                sortColumns,
                                                addDefaultSortCols,
                                                timeInterval,
                                                nReadAheadBuffers);
    }
    else {
        result = new ROVisibilityIterator (ms,
                                           sortColumns,
                                           addDefaultSortCols,
                                           timeInterval);
    }

    Log (2, "Created ROVisibilityIterator%s (3)\n", isAsynchronousIoEnabled() ? "Async" : "");

    return result;
}

ROVisibilityIterator *
ROVisibilityIteratorAsync::create (const Block<MeasurementSet> & mss,
                                   const PrefetchColumns & prefetchColumns,
                                   const Block<Int> & sortColumns,
                                   Double timeInterval,
                                   Int nReadAheadBuffers)
{
    ROVisibilityIterator * result = NULL;

    if (isAsynchronousIoEnabled()){

        result = new ROVisibilityIteratorAsync (mss,
                                                prefetchColumns,
                                                sortColumns,
                                                timeInterval,
                                                nReadAheadBuffers);
    }
    else {
        result = new ROVisibilityIterator (mss,
                                           sortColumns,
                                           timeInterval);
    }

    Log (2, "Created ROVisibilityIterator%s (4)\n", isAsynchronousIoEnabled() ? "Async" : "");

    return result;
}

ROVisibilityIterator *
ROVisibilityIteratorAsync::create (const Block<MeasurementSet> & mss,
                                   const PrefetchColumns & prefetchColumns,
                                   const Block<Int> & sortColumns,
                                   const Bool addDefaultSortCols,
                                   Double timeInterval,
                                   Int nReadAheadBuffers)
{
    ROVisibilityIterator * result = NULL;

    if (isAsynchronousIoEnabled()){

        result = new ROVisibilityIteratorAsync (mss,
                                                prefetchColumns,
                                                sortColumns,
                                                addDefaultSortCols,
                                                timeInterval,
                                                nReadAheadBuffers);
    }
    else {
        result = new ROVisibilityIterator (mss,
                                           sortColumns,
                                           addDefaultSortCols,
                                           timeInterval);
    }

    Log (2, "Created ROVisibilityIterator%s (5)\n", isAsynchronousIoEnabled() ? "Async" : "");

    return result;
}


void
ROVisibilityIteratorAsync::detachVisBuffer (VisBuffer & vb0)
{
    VisBufferAsyncWrapper * vb = dynamic_cast<VisBufferAsyncWrapper *> (& vb0);

    ThrowIf (vb == NULL, "Attempt to detach other than a VisBufferAsyncWrapper");

    if (vb == vbaWrapperStack_p.top()){

        // Get rid of the old buffer

        VisBufferAsync * vba = vb->releaseVba ();
        Assert (vba == visBufferAsync_p);

        vbaWrapperStack_p.pop ();

        // If there is still a VB attached either fill it with the
        // current values for the VI position or clear it.

        if (! vbaWrapperStack_p.empty()){

            if (visBufferAsync_p != NULL){
                vbaWrapperStack_p.top() -> wrap (visBufferAsync_p);
            }
        }

    } else {
        Throw ("ROVisibilityIteratorAsync::detachVisBuffer: VisBufferAsync not attached ");
    }
}

void
ROVisibilityIteratorAsync::dumpPrefetchColumns () const
{
    int i = 0;
    for (PrefetchColumns::const_iterator c = prefetchColumns_p.begin();
         c != prefetchColumns_p.end();
         c ++){

        cerr << prefetchColumnName (*c) << ", ";
        i ++;
        if (i == 10){
            cerr << endl;
            i = 0;
        }

    }

    if (i != 0)
        cerr << endl;

}


void
ROVisibilityIteratorAsync::fillVisBuffer()
{
    // Get the next buffer from the lookahead buffer ring.
    // This could block if the next buffer is not ready.
    // Before doing the fill check to see that there's more data.

    if (more()){

        readComplete ();

        visBufferAsync_p = impl_p->vlaData_p->readStart (chunkNumber_p, subChunkNumber_p);

        Assert (visBufferAsync_p != NULL);

        // If a VisBufferAsync is attached, then copy the prefetched VisBuffer into it.

        if (! vbaWrapperStack_p.empty ()){

            vbaWrapperStack_p.top() -> wrap (visBufferAsync_p);
        }
    }
}

//Vector<MDirection>
//ROVisibilityIteratorAsync::fillAzel(Double time) const
//{
//    NotImplemented;
//}




//void
//ROVisibilityIteratorAsync::fillVisBuffer ()
//{
//    vlaDatum_p = impl_p->getVlaData()->getDatum (chunkNumber_p, subChunkNumber_p);
//
//    ThrowIf (datum == NULL, format("Failed to get datum for subchunk (%d, %d)", chunkNumber_p, subChunkNumber_p));
//
//    if (visBuffer_p != NULL){
//        visBuffer_p->setRealBuffer (vlaDatum_p->getVisBuffer());
//    }
//}

void
ROVisibilityIteratorAsync::getChannelSelection(Block< Vector<Int> >& blockNGroup,
                                               Block< Vector<Int> >& blockStart,
                                               Block< Vector<Int> >& blockWidth,
                                               Block< Vector<Int> >& blockIncr,
                                               Block< Vector<Int> >& blockSpw){

    asyncio::ChannelSelection channelSelection = impl_p->vlaData_p->getChannelSelection ();
    channelSelection.get (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
}

int
ROVisibilityIteratorAsync::getDefaultNBuffers ()
{
    int nBuffers;
    AipsrcValue<Int>::find (nBuffers, getAipsRcBase () + ".nBuffers", 2);

    return nBuffers;
}

//ROVisibilityIteratorAsync::PrefetchColumns
//ROVisibilityIteratorAsync::getPrefetchColumns () const
//{
//    return prefetchColumns_p;
//}


String
ROVisibilityIteratorAsync::getAipsRcBase ()
{
    // Get the top-level name(s) for the async i/o related AipsRc variables

    return "ROVisibilityIteratorAsync";
}

VisBuffer *
ROVisibilityIteratorAsync::getVisBuffer ()
{
    // Returns the currently attached VisBufferAsync or NULL if none attached

    VisBuffer * vb = (! vbaWrapperStack_p.empty()) ? vbaWrapperStack_p.top() : NULL;
    return vb;
}

Bool
ROVisibilityIteratorAsync::isAsynchronousIoEnabled()
{
    // Determines whether asynchronous I/O is enabled by looking for the
    // expected AipsRc value.  If not found then async i/o is disabled.

    Bool isDisabled;
    AipsrcValue<Bool>::find (isDisabled, getAipsRcBase () + ".disabled", True);

    return ! isDisabled;
}

void
ROVisibilityIteratorAsync::linkWithRovi (ROVisibilityIterator * rovi)
{
    linkedVisibilityIterator_p = rovi;
}


Bool
ROVisibilityIteratorAsync::more () const
{
    // Returns true if the lookahead data structure has the next subchunk.

    Bool b = impl_p->vlaData_p->isValidSubChunk (chunkNumber_p, subChunkNumber_p);

    return b;
}

Bool
ROVisibilityIteratorAsync::moreChunks () const
{
    // Returns true if the looahead data structure has the first subchunk of the
    // next chunk.

    Bool b = impl_p->vlaData_p->isValidChunk (chunkNumber_p);

    return b;
}

ROVisibilityIteratorAsync &
ROVisibilityIteratorAsync::nextChunk ()
{
    // Terminates the current read and advances the state of this
    // object to expect to access the first subchunk of the next
    // chunk

    readComplete (); // complete any pending read

    chunkNumber_p ++;
    subChunkNumber_p = 0;

    if (linkedVisibilityIterator_p != NULL){
        linkedVisibilityIterator_p->nextChunk();
    }

    return * this;
}

void
ROVisibilityIteratorAsync::origin ()
{
    // Terminates the current read and
    readComplete (); // complete any pending read

    subChunkNumber_p = 0;

    fillVisBuffer ();

    if (linkedVisibilityIterator_p != NULL){
        linkedVisibilityIterator_p->origin();
    }
}

void
ROVisibilityIteratorAsync::originChunks ()
{
    readComplete (); // complete any pending read

    Bool atOrigin = chunkNumber_p == 0 && subChunkNumber_p == -1;

    if (! atOrigin){

        chunkNumber_p = 0;
        subChunkNumber_p = -1;

        impl_p->vlaData_p->requestViReset ();
    }

    if (linkedVisibilityIterator_p != NULL){
        linkedVisibilityIterator_p->originChunks ();
    }
}

ROVisibilityIteratorAsync::PrefetchColumns
ROVisibilityIteratorAsync::prefetchColumnsAll ()
{
    // Create the set of all columns

    PrefetchColumns pc;

    for (int i = 0; i < N_PrefetchColumnIds; ++ i){
        pc.insert ((PrefetchColumnIds) i);
    }

    return pc;
}

ROVisibilityIteratorAsync::PrefetchColumns
ROVisibilityIteratorAsync::prefetchAllColumnsExcept (Int firstColumn, ...)
{
    // Build a set of Prefetch columns named except.
    // The last arg must be either negative or
    // greater than or equal to N_PrefetchColumnIds

    va_list vaList;

    va_start (vaList, firstColumn);

    int id = firstColumn;
    PrefetchColumns except;

    while (id >= 0 && id < N_PrefetchColumnIds){
        except.insert ((PrefetchColumnIds)id);
        id = va_arg (vaList, int);
    }

    va_end (vaList);

    // Get the set of all columns and then subtract off the
    // caller specified columns.  Return the result

    PrefetchColumns allColumns = prefetchColumnsAll();
    PrefetchColumns result;

    set_difference (allColumns.begin(), allColumns.end(),
                     except.begin(), except.end(),
                    inserter (result, result.begin()));

    return result;

}

String
ROVisibilityIteratorAsync::prefetchColumnName (Int id)
{
    assert (id >= 0 && id < N_PrefetchColumnIds);

    static String names [] =
    {"Ant1",
     "Ant2",
     "ArrayId",
     "Channel",
     "Cjones",
     "CorrType",
     "Corrected",
     "CorrectedCube",
     "Direction1",
     "Direction2",
     "Feed1",
     "Feed1_pa",
     "Feed2",
     "Feed2_pa",
     "FieldId",
     "Flag",
     "FlagCube",
     "FlagRow",
     "Freq",
     "ImagingWeight",
     "LSRFreq",
     "Model",
     "ModelCube",
     "NChannel",
     "NCorr",
     "NRow",
     "Observed",
     "ObservedCube",
     "PhaseCenter",
     "PolFrame",
     "Scan",
     "Sigma",
     "SigmaMat",
     "SpW",
     "Time",
     "TimeInterval",
     "Weight",
     "WeightMat",
     "WeightSpectrum",
     "Uvw",
     "UvwMat"
    };

    return names [id];
}


ROVisibilityIteratorAsync::PrefetchColumns
ROVisibilityIteratorAsync::prefetchColumns (Int firstColumn, ...)
{
    // Returns a set of Prefetch columns.  The last arg must be either negative or
    // greater than or equal to N_PrefetchColumnIds

    va_list vaList;

    va_start (vaList, firstColumn);

    Int id = firstColumn;
    PrefetchColumns pc;

    while (id >= 0 && id < N_PrefetchColumnIds){
        pc.insert ((PrefetchColumnIds) id);
        id = va_arg (vaList, Int);
    }

    va_end (vaList);

    return pc;
}

void
ROVisibilityIteratorAsync::readComplete()
{
    if (visBufferAsync_p != NULL){

        // A buffer in the buffer ring was in use: clean up

        if (! vbaWrapperStack_p.empty()){

            // Break connection between our VisBufferAsync and
            // the shared data

            vbaWrapperStack_p.top()->releaseVba ();
        }

        // Clear the pointer to the shared buffer to indicate
        // internally that the read is complete

        delete visBufferAsync_p;
        visBufferAsync_p = NULL;

        // Inform the buffer ring that the read is complete.

        impl_p->vlaData_p->readComplete(chunkNumber_p, subChunkNumber_p);
    }
}

void
ROVisibilityIteratorAsync::saveMss (const Block<MeasurementSet> & mss)
{
    measurementSets_p.clear();
    for (uint i = 0; i < mss.size(); i++){
        measurementSets_p.push_back (& mss [i]);
    }
}

void
ROVisibilityIteratorAsync::saveMss (const MeasurementSet & ms)
{
    measurementSets_p.clear();
    measurementSets_p.push_back (& ms);
}

ROVisibilityIterator&
ROVisibilityIteratorAsync::selectVelocity (Int nChan,
                                           const MVRadialVelocity& vStart,
                                           const MVRadialVelocity& vInc,
                                           MRadialVelocity::Types rvType,
                                           MDoppler::Types dType,
                                           Bool precise)
{
    SelectVelocityModifier * svm = new SelectVelocityModifier (nChan, vStart, vInc, rvType, dType, precise);

    impl_p->vlaData_p->addModifier (svm); // ownership is transferred by this call

    originChunks ();

    return * this;
}

ROVisibilityIterator&
ROVisibilityIteratorAsync::selectChannel(Int nGroup,
                                         Int start,
                                         Int width,
                                         Int increment,
                                         Int spectralWindow)
{
    SelectChannelModifier * scm = new SelectChannelModifier (nGroup, start, width, increment, spectralWindow);

    impl_p->vlaData_p->addModifier (scm);
    // ownership is transferred by this call

    return * this;
}

ROVisibilityIterator&
ROVisibilityIteratorAsync::selectChannel(Block< Vector<Int> >& blockNGroup,
                                        Block< Vector<Int> >& blockStart,
                                        Block< Vector<Int> >& blockWidth,
                                        Block< Vector<Int> >& blockIncr,
                                        Block< Vector<Int> >& blockSpw)
{
    SelectChannelModifier * scm = new SelectChannelModifier (blockNGroup, blockStart, blockWidth,
                                                             blockIncr, blockSpw);

    impl_p->vlaData_p->addModifier (scm);
    // ownership is transferred by this call

    return * this;
}

void
ROVisibilityIteratorAsync::setInterval (Double timeInterval)
{
    SetIntervalModifier * sim = new SetIntervalModifier (timeInterval);

    impl_p->vlaData_p->addModifier (sim);
}


void
ROVisibilityIteratorAsync::setRowBlocking(Int nRow)
{
    SetRowBlockingModifier * srbm = new SetRowBlockingModifier (nRow);

    impl_p->vlaData_p->addModifier (srbm);
}

void
ROVisibilityIteratorAsync::startVlat ()
{
    impl_p->vlat_p->startThread ();
}

namespace asyncio {

ChannelSelection::ChannelSelection (const Block< Vector<Int> > & blockNGroup,
                                                               const Block< Vector<Int> > & blockStart,
                                                               const Block< Vector<Int> > & blockWidth,
                                                               const Block< Vector<Int> > & blockIncr,
                                                               const Block< Vector<Int> > & blockSpw)
{
    blockNGroup_p = blockNGroup;
    blockStart_p = blockStart;
    blockWidth_p = blockWidth;
    blockIncr_p = blockIncr;
    blockSpw_p = blockSpw;
}

ChannelSelection::ChannelSelection (const ChannelSelection & other)
{
    * this = other;
}

ChannelSelection &
ChannelSelection::operator= (const ChannelSelection & other)
{
    if (this != & other){

        blockNGroup_p = other.blockNGroup_p;
        blockStart_p = other.blockStart_p;
        blockWidth_p = other.blockWidth_p;
        blockIncr_p = other.blockIncr_p;
        blockSpw_p = other.blockSpw_p;
    }

    return * this;
}


void
ChannelSelection::get (Block< Vector<Int> > & blockNGroup,
                       Block< Vector<Int> > & blockStart,
                       Block< Vector<Int> > & blockWidth,
                       Block< Vector<Int> > & blockIncr,
                       Block< Vector<Int> > & blockSpw) const
{
    blockNGroup=blockNGroup_p;
    blockStart=blockStart_p;
    blockWidth=blockWidth_p;
    blockIncr=blockIncr_p;
    blockSpw=blockSpw_p;
}



//void
//ROVisibilityIteratorAsync::setPrefetchColumns (const PrefetchColumns & columns) const
//{
//
//}


//ROVisibilityIterator&
//ROVisibilityIteratorAsync::selectChannel(Int nGroup=1,
//                                        Int start=0,
//                                        Int width=0,
//                                        Int increment=1,
//                                        Int spectralWindow=-1)
//{
//    ROVisibilityIterator & rovi = VisibilityIterator::selectChannel (nGroup, start, width, increment, spectralWindow);
//
//    ThrowIf (visBuffer_p == NULL, "No VisBufferAsync attached");
//
//    visBuffer_p->setBlockSpw (blockSpw_p);
//
//    return rovi;
//}

//ROVisibilityIterator&
//ROVisibilityIteratorAsync::selectChannel(Block< Vector<Int> >& blockNGroup,
//                                        Block< Vector<Int> >& blockStart,
//                                        Block< Vector<Int> >& blockWidth,
//                                        Block< Vector<Int> >& blockIncr,
//                                        Block< Vector<Int> >& blockSpw)
//{
//    ROVisibilityIterator & rovi = VisibilityIterator::selectChannel (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
//
//    ThrowIf (visBuffer_p == NULL, "No VisBufferAsync attached");
//
//    visBuffer_p->setBlockSpw (blockSpw_p);
//
//    return rovi;
//}

std::ostream &
operator<< (std::ostream & o, const RoviaModifier & m)
{
    m.print (o);

    return o;
}


RoviaModifiers::~RoviaModifiers ()
{
    // Free the objects owned by the vector

    for (Data::iterator i = data_p.begin(); i != data_p.end(); i++){
        delete (* i);
    }
}

void
RoviaModifiers::add (RoviaModifier * modifier)
{
    data_p.push_back (modifier);
}

void
RoviaModifiers::apply (ROVisibilityIterator * rovi)
{
    // Free the objects owned by the vector

    for (Data::iterator i = data_p.begin(); i != data_p.end(); i++){
        Log (1, "Applying vi modifier: %s\n", lexical_cast<String> (** i).c_str());
        (* i) -> apply (rovi);
    }

}

void
RoviaModifiers::clear ()
{
    for (Data::iterator i = data_p.begin(); i != data_p.end(); i++){
        delete (* i);
    }

    data_p.clear();
}

void
RoviaModifiers::transfer (RoviaModifiers & other)
{
    for (Data::iterator i = other.data_p.begin(); i != other.data_p.end(); i++){
        data_p.push_back (* i);
    }

    other.data_p.clear();
}

SelectChannelModifier::SelectChannelModifier (Int nGroup, Int start, Int width, Int increment, Int spectralWindow)
: channelBlocks_p (False),
  increment_p (increment),
  nGroup_p (nGroup),
  spectralWindow_p (spectralWindow),
  start_p (start),
  width_p (width)
{}

SelectChannelModifier::SelectChannelModifier (const Block< Vector<Int> > & blockNGroup,
                                              const Block< Vector<Int> > & blockStart,
                                              const Block< Vector<Int> > & blockWidth,
                                              const Block< Vector<Int> > & blockIncr,
                                              const Block< Vector<Int> > & blockSpw)
: channelBlocks_p (True),
  channelSelection_p (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw)
{}

void
SelectChannelModifier::apply (ROVisibilityIterator * rovi) const
{
    if (! channelBlocks_p){
        rovi->selectChannel (nGroup_p, start_p, width_p, increment_p, spectralWindow_p);
    }
    else{
        Block< Vector<Int> > blockNGroup;
        Block< Vector<Int> > blockStart;
        Block< Vector<Int> > blockWidth;
        Block< Vector<Int> > blockIncr;
        Block< Vector<Int> > blockSpw;

        channelSelection_p.get (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
        rovi->selectChannel (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
    }
}

void
SelectChannelModifier::print (ostream & os) const
{
    os << "SelectChannel::{";

    if (channelBlocks_p){
        Block< Vector<Int> > blockNGroup;
        Block< Vector<Int> > blockStart;
        Block< Vector<Int> > blockWidth;
        Block< Vector<Int> > blockIncr;
        Block< Vector<Int> > blockSpw;

        channelSelection_p.get (blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);

        os << "nGroup=" << toCsv (blockNGroup)
           << ", start=" << toCsv (blockStart)
           << ", width=" << toCsv (blockWidth)
           << ", increment=" << toCsv (blockIncr)
           << ", spw=" << toCsv (blockSpw);
    }
    else {
        os << "nGroup=" << nGroup_p
           << ", start=" << start_p
           << ", width=" << width_p
           << ", increment=" << increment_p
           << ", spw=" << spectralWindow_p;
    }
    os << "}";
}

String
SelectChannelModifier::toCsv (const Block< Vector<Int> > & bv) const
{
    String result = "{";

    for (Block<Vector<Int> >::const_iterator v = bv.begin(); v != bv.end(); ++ v){
        if (result.size() != 1)
            result += ",";

        result += "{" + toCsv (* v) + "}";

    }

    result += "}";

    return result;

}

String
SelectChannelModifier::toCsv (const Vector<Int> & v) const
{
    String result = "";
    for (Vector<Int>::const_iterator i = v.begin(); i != v.end(); ++ i){
        if (! result.empty())
            result += ",";
        result +=  String::toString (* i);
    }

    return result;
}


SelectVelocityModifier::SelectVelocityModifier (Int nChan, const MVRadialVelocity& vStart, const MVRadialVelocity& vInc,
                                                MRadialVelocity::Types rvType, MDoppler::Types dType, Bool precise)

: dType_p (dType),
  nChan_p (nChan),
  precise_p (precise),
  rvType_p (rvType),
  vInc_p (vInc),
  vStart_p (vStart)
{}

void
SelectVelocityModifier::apply (ROVisibilityIterator * rovi) const
{
    rovi-> selectVelocity (nChan_p, vStart_p, vInc_p, rvType_p, dType_p, precise_p);
}

void
SelectVelocityModifier::print (std::ostream & os) const
{
    os << "SelectVelocity::{"

       << "dType=" << dType_p
       << ",nChan=" << nChan_p
       << ",precise=" << precise_p
       << ",rvType=" << rvType_p
       << ",vInc=" << vInc_p
       << ",vStart=" << vStart_p

       << "}";
}

SetIntervalModifier::SetIntervalModifier  (Double timeInterval)
: timeInterval_p (timeInterval)
{}

void
SetIntervalModifier::apply (ROVisibilityIterator * rovi) const
{
    rovi -> setInterval (timeInterval_p);
}

void
SetIntervalModifier::print (std::ostream & os) const
{
    os << "SetInterval::{" << timeInterval_p << "}";
}



SetRowBlockingModifier::SetRowBlockingModifier (Int nRows)
: nRows_p (nRows)
{}

void
SetRowBlockingModifier::apply (ROVisibilityIterator * rovi) const
{
    rovi->setRowBlocking (nRows_p);
}

void
SetRowBlockingModifier::print (std::ostream & os) const
{
    os << "SetRowBlocking::{"

       << "nRows=" << nRows_p
       << ",nGroup=" << nGroup_p
       << ",spectralWindow=" << spectralWindow_p
       << ",start=" << start_p
       << ",width=" << width_p

       << "}";
}

} // end namespace asyncio

} // end namespace casa

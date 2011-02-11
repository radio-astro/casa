/*
 * VisibilityIteratorAsync.cc
 *
 *  Created on: Nov 2, 2010
 *      Author: jjacobs
 */

#include "VisibilityIteratorAsync.h"
#include "VisBufferAsync.h"
#include "VLAT.h"

#include <ms/MeasurementSets/MSColumns.h>
#include <casa/System/AipsrcValue.h>

#include <algorithm>
#include <cstdarg>

using namespace std;

#include "UtilJ.h"
using namespace casa::utilj;

using namespace casa::asyncio;

#define Log(level, ...) \
    {if (VlaData::loggingInitialized_p && level <= VlaData::logLevel_p) \
         Logger::log (__VA_ARGS__);};

#define NotImplemented     Throw ("ROVisibilityIteratorAsync: Method not implemented!");

namespace casa {

// The base of the AIPS RC keyword to use for asynchronous I/O keywords

ROVisibilityIteratorAsync::ROVisibilityIteratorAsync (const MeasurementSet & ms,
                                                      const PrefetchColumns & prefetchColumns,
                                                      const Block<Int> & sortColumns,
                                                      Double timeInterval,
                                                      Int nReadAheadBuffers)
: impl_p (NULL),
  vlaDatum_p (NULL)
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
  vlaDatum_p (NULL)
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
  vlaDatum_p (NULL)
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
  vlaDatum_p (NULL)
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
  vlaDatum_p (NULL)
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

}

void
ROVisibilityIteratorAsync::attachVisBuffer (VisBuffer & vb0)
{
    VisBufferAsync * vb = dynamic_cast<VisBufferAsync *> (& vb0);
    ThrowIf (vb == NULL, "Attempt to attach other than VisBufferAsync");

    if (! visBufferAsyncStack_p.empty()){
        visBufferAsyncStack_p.top () -> clear();
    }

    visBufferAsyncStack_p.push (vb);

    if (vlaDatum_p != NULL){
        vb->fillFrom (* vlaDatum_p -> getVisBuffer());
    }
}

void
ROVisibilityIteratorAsync::construct (const PrefetchColumns & prefetchColumns, Int nReadAheadBuffers)
{
    if (VlaData::loggingInitialized_p){
        Logger::registerName ("Main");
    }

    chunkNumber_p = 0;
    subChunkNumber_p = -1;
    vlaDatum_p = NULL;

    impl_p = new ROVisibilityIteratorAsyncImpl ();

    if (nReadAheadBuffers <= 0){

         nReadAheadBuffers = getDefaultNBuffers ();
    }

    impl_p->vlaData_p = new VlaData (nReadAheadBuffers);
    impl_p->vlaData_p->initialize ();

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
        prefetchColumns_p.insert (Time);
    }

    if (contains (Feed2_pa, prefetchColumns_p)){
        prefetchColumns_p.insert (Feed2);
        prefetchColumns_p.insert (Ant2);
        prefetchColumns_p.insert (Time);
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
    VisBufferAsync * vb = dynamic_cast<VisBufferAsync *> (& vb0);

    if (vb == NULL){
        Throw ("Attempt to detach synchronous VisBuffer");
    }
    else if (vb == visBufferAsyncStack_p.top()){

        // Get rid of the old buffer

        vb->clear();
        visBufferAsyncStack_p.pop ();

        // If there is still a VB attached either fill it with the
        // current values for the VI position or clear it.

        if (! visBufferAsyncStack_p.empty()){

            if (vlaDatum_p != NULL){
                visBufferAsyncStack_p.top() -> fillFrom (* vlaDatum_p -> getVisBuffer());
            }
            else{
                visBufferAsyncStack_p.top()->clear();
            }
        }

    } else {
        ThrowIf (true, "ROVisibilityIteratorAsync::detachVisBuffer: VisBufferAsync not attached ");
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

        vlaDatum_p = impl_p->vlaData_p->readStart ();

        Assert (vlaDatum_p != NULL);
        ThrowIf (! vlaDatum_p->isChunk (chunkNumber_p, subChunkNumber_p),
                 format ("Expected chunk (%d,%d) but got (%d,%d", chunkNumber_p, subChunkNumber_p,
                         vlaDatum_p->getChunkNumber(), vlaDatum_p->getSubChunkNumber()));

        // If a VisBufferAsync is attached, then copy the prefetched VisBuffer into it.

        if (! visBufferAsyncStack_p.empty ()){

            visBufferAsyncStack_p.top() -> clear (); // out with the old

            visBufferAsyncStack_p.top() -> fillFrom (* vlaDatum_p->getVisBuffer()); // in with the new
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
    AipsrcValue<Int>::find (nBuffers, getRcBase () + ".nBuffers", 2);

    return nBuffers;
}

//ROVisibilityIteratorAsync::PrefetchColumns
//ROVisibilityIteratorAsync::getPrefetchColumns () const
//{
//    return prefetchColumns_p;
//}


String
ROVisibilityIteratorAsync::getRcBase ()
{
    static const String RcBase ("ROVisibilityIteratorAsync");
    return RcBase;
}

VisBuffer *
ROVisibilityIteratorAsync::getVisBuffer ()
{
    VisBuffer * vb = (! visBufferAsyncStack_p.empty()) ? visBufferAsyncStack_p.top() : NULL;
    return vb;
}

Bool
ROVisibilityIteratorAsync::isAsynchronousIoEnabled()
{
    Bool isDisabled;
    AipsrcValue<Bool>::find (isDisabled, getRcBase () + ".disabled", True);

    return ! isDisabled;
}

Bool
ROVisibilityIteratorAsync::more () const
{
    Bool b = impl_p->vlaData_p->isValidSubChunk (chunkNumber_p, subChunkNumber_p);

    return b;
}

Bool
ROVisibilityIteratorAsync::moreChunks () const
{
    Bool b = impl_p->vlaData_p->isValidChunk (chunkNumber_p);

    return b;
}

ROVisibilityIteratorAsync &
ROVisibilityIteratorAsync::nextChunk ()
{
    readComplete (); // complete any pending read

    chunkNumber_p ++;
    subChunkNumber_p = 0;

    return * this;
}

void
ROVisibilityIteratorAsync::origin ()
{
    readComplete (); // complete any pending read

    subChunkNumber_p = 0;

    fillVisBuffer ();
}

void
ROVisibilityIteratorAsync::originChunks ()
{
    readComplete (); // complete any pending read

    Bool atOrigin = chunkNumber_p == 0 && subChunkNumber_p == -1;

    if (! atOrigin){

        chunkNumber_p = 0;
        subChunkNumber_p = -1;

        impl_p->vlaData_p->requestViReset (impl_p->vlat_p);
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
ROVisibilityIteratorAsync::prefetchColumnName (PrefetchColumnIds id)
{
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
    if (vlaDatum_p != NULL){

        // A buffer in the buffer ring was in use: clean up

        if (! visBufferAsyncStack_p.empty()){

            // Break connection between our VisBufferAsync and
            // the shared data

            visBufferAsyncStack_p.top()->clear();
        }

        // Clear the pointer to the shared buffer to indicate
        // internally that the read is complete

        vlaDatum_p = NULL;

        // Inform the buffer ring that the read is complete.
        //

        impl_p->vlaData_p->readComplete();

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
    blockNGroup_p.resize(0, True, False);
    blockNGroup_p = blockNGroup;
    blockStart_p.resize(0, True, False);
    blockStart_p = blockStart;
    blockWidth_p.resize(0, True, False);
    blockWidth_p = blockWidth;
    blockIncr_p.resize(0, True, False);
    blockIncr_p = blockIncr;
    blockSpw_p.resize(0, True, False);
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

        blockNGroup_p.resize(0, True, False);
        blockNGroup_p = other.blockNGroup_p;

        blockStart_p.resize(0, True, False);
        blockStart_p = other.blockStart_p;

        blockWidth_p.resize(0, True, False);
        blockWidth_p = other.blockWidth_p;

        blockIncr_p.resize(0, True, False);
        blockIncr_p = other.blockIncr_p;

        blockSpw_p.resize(0, True, False);
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
    blockNGroup.resize(0, True, False);
    blockNGroup=blockNGroup_p;
    blockStart.resize(0, True, False);
    blockStart=blockStart_p;
    blockWidth.resize(0, True, False);
    blockWidth=blockWidth_p;
    blockIncr.resize(0, True, False);
    blockIncr=blockIncr_p;
    blockSpw.resize(0, True, False);
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

SetIntervalModifier::SetIntervalModifier  (Double timeInterval)
: timeInterval_p (timeInterval)
{}

void
SetIntervalModifier::apply (ROVisibilityIterator * rovi) const
{
    rovi -> setInterval (timeInterval_p);
}


SetRowBlockingModifier::SetRowBlockingModifier (Int nRows)
: nRows_p (nRows)
{}

void SetRowBlockingModifier::apply (ROVisibilityIterator * rovi) const
{
    rovi->setRowBlocking (nRows_p);
}

} // end namespace asyncio

} // end namespace casa

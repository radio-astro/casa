//# VisibilityIterator2.cc: Step through MeasurementEquation by visibility
//# Copyright (C) 1996-2012
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the !GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: VisibilityIterator2.cc,v 19.15 2006/02/01 01:25:14 kgolap Exp $

#include <boost/tuple/tuple.hpp>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Exceptions/Error.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Sort.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSpwIndex.h>
#include <msvis/MSVis/MSUtil.h>
#include <msvis/MSVis/UtilJ.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisBufferComponents2.h>
#include <msvis/MSVis/VisBufferImplAsync2.h>
#include <msvis/MSVis/VisBufferAsyncWrapper2.h>
#include <msvis/MSVis/VisibilityIteratorImplAsync2.h>
#include <synthesis/TransformMachines/VisModelData.h>

#include <cassert>
#include <algorithm>
#include <limits>
#include <memory>
#include <map>

using std::make_pair;
using namespace casa::vi;
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

#define ThrowNotPrefetchedException() \
    Throw ("Data not prefetched; consider adjusting prefetch columns.");

#define ThrowIllegalOperationException() \
    Throw ("Operation not permitted in async i/o.");

VisibilityIteratorImplAsync2::VisibilityIteratorImplAsync2 (VisibilityIterator2 * rovi,
                                                            const Block<MeasurementSet> &mss,
                                                            const Block<Int> & sortColumns,
                                                            Bool addDefaultSort,
                                                            Double timeInterval,
                                                            Bool createVb,
                                                            Bool isWritable)
: VisibilityIteratorImpl2 ()
{
    // Make sure the pointer to the containing ROVI (rovi_p) is NULL when calling initialize
    // otherwise the call back to the VI can result in it trying to use an uninitialized pointer
    // to this object (since it is in the process or being constructed).

    initialize (mss);

    rovi_p = rovi;

    if (createVb){

        vb_p = VisBuffer2::factory (rovi, VisBuffer2::Asynchronous, isWritable);
    }
}

VisibilityIteratorImplAsync2::~VisibilityIteratorImplAsync2 ()
{
    interface_p->terminate ();
    delete interface_p;
}




void
VisibilityIteratorImplAsync2::initialize (const Block<MeasurementSet> &mss)
{
    cache_p.flush();

    msIndex_p = 0;

    Int numMS = mss.nelements ();
    measurementSets_p.clear ();

    for (Int k = 0; k < numMS; ++k) {

        measurementSets_p.push_back (mss [k]);
    }

    // Install default frequency selections.  This will select all
    // channels in all windows.

    frequencySelections_p = new FrequencySelections ();
}

void
VisibilityIteratorImplAsync2::advance ()
{
    //readComplete (); // complete any pending read

    subchunk_p.incrementSubChunk ();

    fillVisBuffer ();
}

void
VisibilityIteratorImplAsync2::construct(const Block<MeasurementSet> & mss,
                                        const PrefetchColumns & prefetchColumns,
                                        const Block<Int> & sortColumns,
                                        const Bool addDefaultSortCols,
                                        Double timeInterval,
                                        Bool writable)
{
    AsynchronousInterface::initializeLogging();

    casa::async::Logger::get()->registerName ("Main");

    for (uint i = 0; i < mss.size(); i++){
        measurementSets_p.push_back (mss [i]);
    }

    msId_p = -1;

    // Create and initialize the Asynchronous Interface

    Int nReadAheadBuffers = getDefaultNBuffers ();

    interface_p = new AsynchronousInterface (nReadAheadBuffers);
    interface_p->initialize();

    vlaData_p = interface_p->getVlaData ();
    vlat_p = interface_p->getVlat ();

    // Augment the list of prefetch columns with any that are implied
    // by the others.  N.B., be wary of reordering these.

    prefetchColumns_p = augmentPrefetchColumns (prefetchColumns);

    // Get the VLAT going

    vlat_p->setPrefetchColumns (prefetchColumns_p);

    // If this is a writable VI then let the write implementation handle the
    // initialization of the VLAT.

    vlat_p->initialize (mss, sortColumns, addDefaultSortCols, timeInterval, writable);
    vlat_p->startThread ();
}

void
VisibilityIteratorImplAsync2::fillVisBuffer()
{
    // Get the next buffer from the lookahead buffer ring.
    // This could block if the next buffer is not ready.
    // Before doing the fill check to see that there's more data.

    if (more()){

        readComplete ();

        VisBufferAsyncImpl2 * vba  = vlaData_p->readStart (subchunk_p);

        Assert (a != NULL);

        getVisBufferWrapper()->wrap (vba);

        msId_p = visBufferAsync_p->msId ();

    }
}

VisibilityIteratorImplAsync2 &
VisibilityIteratorImplAsync2::nextChunk ()
{
    // Terminates the current read and advances the state of this
    // object to expect to access the first subchunk of the next
    // chunk

    subchunk_p.incrementChunk ();

    if (moreChunks()){

        readComplete (); // complete any pending read

    }

    return * this;
}

void
VisibilityIteratorImplAsync2::origin ()
{
    ThrowIf (rowBounds_p.chunkNRows_p < 0,
             "Call to origin without first initializing chunk");

    throwIfPendingChanges ();

    // Terminates the current read and

    readComplete (); // complete any pending read

    subchunk_p.resetSubChunk();

    fillVisBuffer ();

    updateMsd ();
}

void
VisibilityIteratorImplAsync2::originChunks ()
{
    readComplete (); // complete any pending read

    subchunk_p.resetToOrigin ();

#warning "Handle pending changes"

    applyPendingChanges ();

    interface_p->requestViReset ();
}

Bool
VisibilityIteratorImplAsync2::more () const
{
    // Returns true if the lookahead data structure has the next subchunk.

    Bool b = vlaData_p->isValidSubChunk (subchunk_p);

    return b;
}

Bool
VisibilityIteratorImplAsync2::moreChunks () const
{
    // Returns true if the looahead data structure has the first subchunk of the
    // next chunk.

    Bool b = vlaData_p->isValidChunk (subchunk_p.chunk());

    return b;
}



void
VisibilityIteratorImplAsync2::readComplete()
{
    if (visBufferAsync_p != NULL){

        // A buffer in the buffer ring was in use: clean up

        getVisBuffer()->releaseVba ();

        // Clear the pointer to the shared buffer to indicate
        // internally that the read is complete

        delete visBufferAsync_p;
        visBufferAsync_p = NULL;

        // Inform the buffer ring that the read is complete.

        vlaData_p->readComplete (subchunk_p);
    }
}



const Cube<RigidVector<Double, 2> > &
VisibilityIteratorImplAsync2::getBeamOffsets () const
{
    return getMsIterInfo().getBeamOffsets ();
}

Vector<Double>
VisibilityIteratorImplAsync2::getFrequencies (Double time, Int frameOfReference) const
{
    // Get the channel information object (basically contains info from the
    // spectral window subtable).

    Int msId = this->msId ();
    Int spectralWindowId = spectralWindow();

    const SpectralWindowChannels & spectralWindowChannels =
        getSpectralWindowChannels (msId, spectralWindowId);

    // Get the channel numbers selected for this time (the spectral window and MS index are
    // assumed to be the same as those currently pointed to by the MSIter).

    Vector<Int> channels = getChannels (time, frameOfReference);

    Vector<Double> frequencies (channels.nelements());
    MFrequency::Types observatoryFrequencyType = getObservatoryFrequencyType ();

    if (frameOfReference == observatoryFrequencyType){

        // Since the observed frequency is being asked for, no conversion is necessary.
        // Just copy each selected channel's observed frequency over to the result.

        for (Int i = 0; i < (int) channels.nelements(); i ++){

            Int channelNumber = channels [i];

            frequencies [i] = spectralWindowChannels [channelNumber].getFrequency ();
        }

        return frequencies;
    }

    // Get the converter from the observed to the requested frame.

    MFrequency::Convert fromObserved = makeFrequencyConverter (time, frameOfReference, False);

    // For each selected channel, get its observed frequency and convert it into
    // the frequency in the requested frame.  Put the result into the
    // corresponding slot of "frequencies".

    for (Int i = 0; i < (int) channels.nelements(); i ++){

        Int channelNumber = channels [i];

        Double fO = spectralWindowChannels [channelNumber].getFrequency ();
            // Observed frequency

        Double fF = fromObserved (Quantity (fO, "Hz")).get ("Hz").getValue();
            // Frame frequency

        frequencies [i] = fF;
    }

    return frequencies;
}

Vector<Int>
VisibilityIteratorImplAsync2::getChannels (Double /*time*/, Int /*frameOfReference*/) const
{
    assert (channelSelector_p != 0);

    return channelSelector_p->getChannels ();
}

VisBufferImplAsync2 *
VisibilityIteratorImplAsync2::getVisBufferAsync () const
{
    VisBufferAsyncWrapper2 * vbaw = getVisBufferWrapper ();

    VisBufferImplAsync2 * vba = dynamic_cast<VisBufferImplAsync2 *> (vbaw->getWrappedVisBuffer ());

    Assert (vba != 0);

    return vba;
}


VisBufferAsyncWrapper2 *
VisibilityIteratorImplAsync2::getVisBufferWrapper () const
{
    VisBuffer2 * vb = getVisBuffer ();

    VisBufferAsyncWrapper2 * vba = dynamic_cast <VisBufferAsyncWrapper2 *> (vb);

    ThrowIf (vba == 0, "Attached VisBuffer is not VisBufferAsyncWrapper");

    return vba;
}


const MsIterInfo &
VisibilityIteratorImplAsync2::getMsIterInfo () const
{
    return getVisBufferAsync()->getMsIterInfo ();
}

Bool
VisibilityIteratorImplAsync2::isNewArrayId () const
{
    return getMsIterInfo().newArray();
}

Bool
VisibilityIteratorImplAsync2::isNewFieldId () const
{
    return getMsIterInfo().newField ();
}

Bool
VisibilityIteratorImplAsync2::isNewMs () const
{
    return getMsIterInfo().newMS ();
}

Bool
VisibilityIteratorImplAsync2::isNewSpectralWindow () const
{
    return getMsIterInfo().newSpectralWindow ();
}

Bool
VisibilityIteratorImplAsync2::allBeamOffsetsZero () const
{
    return getMsIterInfo().allBeamOffsetsZero ();
}

Int
VisibilityIteratorImplAsync2::nRows () const
{
    return rowBounds_p.subchunkNRows_p;
}

Int VisibilityIteratorImplAsync2::nRowsInChunk () const
{
    return getMsIterInfo().table ().nrow ();
}

Bool
VisibilityIteratorImplAsync2::more () const
{
    return more_p;
}

Bool
VisibilityIteratorImplAsync2::moreChunks () const
{
    return getMsIterInfo().more ();
}

Int
VisibilityIteratorImplAsync2::msId () const
{
    return getMsIterInfo().msId ();
}

const MeasurementSet &
VisibilityIteratorImplAsync2::ms () const
{
    return getMsIterInfo().ms ();
}

Int
VisibilityIteratorImplAsync2::fieldId () const
{
    return getMsIterInfo().fieldId ();
}

// Return the current ArrayId
Int
VisibilityIteratorImplAsync2::arrayId () const
{
    return getMsIterInfo().arrayId ();
}

// Return the current Field Name
String
VisibilityIteratorImplAsync2::fieldName () const
{
    return getMsIterInfo().fieldName ();
}

// Return the current Source Name
String
VisibilityIteratorImplAsync2::sourceName () const
{
    return getMsIterInfo().sourceName ();
}

const Vector<String> &
VisibilityIteratorImplAsync2::antennaMounts () const
{
    return getMsIterInfo().antennaMounts ();
}

void
VisibilityIteratorImplAsync2::setInterval (Double timeInterval)
{
    pendingChanges_p.setInterval (timeInterval);
}

void
VisibilityIteratorImplAsync2::setRowBlocking (Int nRow)
{
    pendingChanges_p.setNRowBlocking (nRow);
}

const MDirection &
VisibilityIteratorImplAsync2::phaseCenter () const
{
        return getMsIterInfo().phaseCenter ();
}

Int
VisibilityIteratorImplAsync2::polFrame () const
{
        return getMsIterInfo().polFrame ();
}

Int
VisibilityIteratorImplAsync2::spectralWindow () const
{
    return getMsIterInfo().spectralWindowId ();
}

// Return current Polarization Id
Int
VisibilityIteratorImplAsync2::polarizationId () const
{
    return getMsIterInfo().polarizationId ();
}

// Return current DataDescription Id
Int
VisibilityIteratorImplAsync2::dataDescriptionId () const
{
    return getMsIterInfo().dataDescriptionId ();
}

Bool
VisibilityIteratorImplAsync2::newFieldId () const
{
    return (rowBounds_p.subchunkBegin_p == 0 && getMsIterInfo().newField ());
}

Bool
VisibilityIteratorImplAsync2::newArrayId () const
{
    return (rowBounds_p.subchunkBegin_p == 0 && getMsIterInfo().newArray ());
}

Bool
VisibilityIteratorImplAsync2::newSpectralWindow () const
{
    return (rowBounds_p.subchunkBegin_p == 0 && getMsIterInfo().newSpectralWindow ());
}

Int
VisibilityIteratorImplAsync2::nPolarizations () const
{
    return nPolarizations_p;
}



Bool
VisibilityIteratorImplAsync2::existsColumn (VisBufferComponent2 id) const
{
    Bool result;
    switch (id){

    case VisibilityCorrected:
    case VisibilityCubeCorrected:

        result = ! columns_p.corrVis_p.isNull();
        break;

    case VisibilityModel:
    case VisibilityCubeModel:

        result = ! columns_p.modelVis_p.isNull();
        break;

    case VisibilityObserved:
    case VisibilityCubeObserved:

        result = ! (columns_p.vis_p.isNull() && columns_p.floatVis_p.isNull());
        break;

    // RR: I can't tell if the other columns should checked for here or not.
    //     It's not true that all the other columns are required.
    //     existsFlagCategory uses caching anyway.
    // case FlagCategory:
    //   result = False;
    //   if(!columns_p.flagCategory().isNull() &&
    //      columns_p.flagCategory().isDefined(0)){
    //     IPosition fcshape(columns_p.flagCategory().shape(0));
    //     IPosition fshape(columns_p.flag().shape(0));

    //     result = fcshape(0) == fshape(0) && fcshape(1) == fshape(1);
    //   }

    default:
        result = True; // required columns
        break;
    }

    return result;
}

const SubtableColumns &
VisibilityIteratorImplAsync2::subtableColumns () const
{
    return * subtableColumns_p;
}

void
VisibilityIteratorImplAsync2::origin ()
{
    ThrowIf (rowBounds_p.chunkNRows_p < 0,
             "Call to origin without first initializing chunk");

    throwIfPendingChanges ();

    rowBounds_p.subchunkBegin_p = 0; // begin at the beginning
    more_p = True;
    subchunk_p.resetSubChunk ();

    configureNewSubchunk ();
}

void
VisibilityIteratorImplAsync2::originChunks ()
{
    originChunks (False);
}

void
VisibilityIteratorImplAsync2::applyPendingChanges ()
{
    if (! pendingChanges_p.empty()){

        // Send a copy of the changes over to the VLAT

        interface_p->setPendingChanges (pendingChanges_p.clone ());

        // Apply the changes which simply means making a copy of them
        // available for readonly use.

        Bool exists;

        // Handle a pending frequency selection if it exists.

        FrequencySelections * newSelection;
        boost::tie (exists, newSelection) = pendingChanges_p.popFrequencySelections ();

        if (exists){

            delete frequencySelections_p; // out with the old

            frequencySelections_p = newSelection; // in with the new
        }

        // Handle any pending interval change

        Double newInterval;
        boost::tie (exists, newInterval) = pendingChanges_p.popInterval ();

        if (exists){

            msIter_p.setInterval(newInterval);
            timeInterval_p = newInterval;
        }

        // Handle any row-blocking change

        Int newBlocking;
        boost::tie (exists, newBlocking) = pendingChanges_p.popNRowBlocking ();

        if (exists){

            nRowBlocking_p = newBlocking;

        }

    }
}

void
VisibilityIteratorImplAsync2::advance ()
{
    ThrowIf (! more_p, "Attempt to advance subchunk past end of chunk");

    throwIfPendingChanges (); // throw if unapplied changes exist

    // Attempt to advance to the next subchunk

    rowBounds_p.subchunkBegin_p = rowBounds_p.subchunkEnd_p + 1;

    more_p = rowBounds_p.subchunkBegin_p < rowBounds_p.chunkNRows_p;

    if (more_p) {

        subchunk_p.incrementSubChunk();

        configureNewSubchunk ();
    }
}

SubChunkPair2
VisibilityIteratorImplAsync2::getSubchunkId () const
{
    return subchunk_p;
}

const Block<Int>&
VisibilityIteratorImplAsync2::getSortColumns() const
{
  return sortColumns_p;
}

VisibilityIterator2 *
VisibilityIteratorImplAsync2::getViP () const
{
    return rovi_p;
}

void
VisibilityIteratorImplAsync2::throwIfPendingChanges ()
{
    // Throw an exception if there are any pending changes to the
    // operation of the visibility iterator.  The user must call
    // originChunks to cause the changes to take effect; it is an
    // error to try to advance the VI if there are unapplied changes
    // pending.

    ThrowIf (! pendingChanges_p.empty(),
             "Call to originChunks required after applying frequencySelection");

}

Bool
VisibilityIteratorImplAsync2::isInASelectedSpectralWindow () const
{
    return frequencySelections_p->isSpectralWindowSelected(getMsIterInfo().msId(),
                                                           getMsIterInfo().spectralWindowId ());
}

VisibilityIteratorImplAsync2 &
VisibilityIteratorImplAsync2::nextChunk ()
{
    ThrowIf (! getMsIterInfo().more (),
             "Attempt to advance past end of data using nextChunk");

    throwIfPendingChanges (); // error if unapplied changes exist

    if (getMsIterInfo().more ()) {

        // Advance the MS Iterator until either there's no
        // more data or it points to a selected spectral window.

        msIter_p++;

        positionMsIterToASelectedSpectralWindow ();

//        if (msIter_p.newMS ()) {
//            msIndex_p = msId ();
//            doChannelSelection ();
//        }

        msIterAtOrigin_p = False;
    }

    // If the MS Iterator was successfully advanced then
    // set up for a new chunk

    if (getMsIterInfo().more ()) {

        subchunk_p.incrementChunk();

        configureNewChunk ();

        vb_p->invalidate (); // flush the cache ??????????
    }

    more_p = getMsIterInfo().more ();

    return *this;
}

void
VisibilityIteratorImplAsync2::configureNewSubchunk ()
{

    // work out how many rows to return
    // for the moment we return all rows with the same value for time
    // unless row blocking is set, in which case we return more rows at once.

    if (nRowBlocking_p > 0) {

        rowBounds_p.subchunkEnd_p = rowBounds_p.subchunkBegin_p + nRowBlocking_p;

        if (rowBounds_p.subchunkEnd_p >= rowBounds_p.chunkNRows_p) {
            rowBounds_p.subchunkEnd_p = rowBounds_p.chunkNRows_p - 1;
        }

        // Scan the subchunk to see if the same channels are selected in each row.
        // End the subchunk when a row using different channels is encountered.

        Double previousRowTime = rowBounds_p.times_p (rowBounds_p.subchunkBegin_p);
        channelSelector_p = determineChannelSelection (previousRowTime);

        for (Int i = rowBounds_p.subchunkBegin_p + 1;
             i <= rowBounds_p.subchunkEnd_p;
             i++){

            Double rowTime = rowBounds_p.times_p (i);

            if (rowTime == previousRowTime){
                continue; // Same time means same rows.
            }

            // Compute the channel selector for this row so it can be compared with
            // the previous row's channel selector.

            const ChannelSelector * newSelector = determineChannelSelection (rowTime);

            if (newSelector != channelSelector_p){

                // This row uses different channels than the previous row and so it
                // cannot be included in this subchunk.  Make the previous row the end
                // of the subchunk.

                rowBounds_p.subchunkEnd_p = i - 1;

            }
        }
    }
    else {

        // The subchunk will consist of all rows in the chunk having the
        // same timestamp as the first row.

        Double subchunkTime = rowBounds_p.times_p (rowBounds_p.subchunkBegin_p);
        channelSelector_p = determineChannelSelection (subchunkTime);

        for (Int i = rowBounds_p.subchunkBegin_p;
             i < rowBounds_p.chunkNRows_p;
             i++){

            if (rowBounds_p.times_p (i) != subchunkTime){
                break;
            }

            rowBounds_p.subchunkEnd_p = i;
        }
    }

    rowBounds_p.subchunkNRows_p = rowBounds_p.subchunkEnd_p - rowBounds_p.subchunkBegin_p + 1;
    rowBounds_p.subchunkRows_p = RefRows (rowBounds_p.subchunkBegin_p, rowBounds_p.subchunkEnd_p);

    // Set flags for current subchunk

    nPolarizations_p = subtableColumns_p->polarization ().numCorr ()(getMsIterInfo().polarizationId ());

    String msName = ms().tableName ();
    vb_p->configureNewSubchunk (msId (), msName, isNewMs (), isNewArrayId (), isNewFieldId (),
                                isNewSpectralWindow (), subchunk_p, rowBounds_p.subchunkNRows_p,
                                channelSelector_p->getNFrequencies(), nPolarizations_p);

}

const ChannelSelector *
VisibilityIteratorImplAsync2::determineChannelSelection (Double time)
{
// --> The channels selected will need to be identical for all members of the
//     subchunk; otherwise the underlying fetch method won't work since it
//     takes a single Vector<Vector <Slice> > to specify the channels.

    assert (frequencySelections_p != 0);

    Int spectralWindowId = spectralWindow ();
    const FrequencySelection & selection = frequencySelections_p->get (msId ());
    Int frameOfReference = selection.getFrameOfReference ();

    // See if the appropriate channel selector is in the cache.

    const ChannelSelector * cachedSelector =  channelSelectorCache_p->find (time, msId (), frameOfReference,
                                                                            spectralWindowId);

    if (cachedSelector != 0){
        return cachedSelector;
    }

    // Create the appropriate frequency selection for the current
    // MS and Spectral Window

    selection.filterByWindow (spectralWindowId);

    // Find (or create) the appropriate channel selection.

    ChannelSelector * newSelector;

    if (selection.getFrameOfReference() == FrequencySelection::ByChannel){
        newSelector = makeChannelSelectorC (selection, time, msId (),
                                            spectralWindowId);
    }
    else{
        newSelector = makeChannelSelectorF (selection, time, msId (),
                                            spectralWindowId);
    }

    // Cache it for possible future use.  The cache may hold multiple equivalent
    // selectors, each having a different timestamp.  Since selectors are small
    // and there are not expected to be many equivalent selectors in the cache at
    // a time, this shouldn't be a problem (the special case of selection by
    // channel number is already handled).

    channelSelectorCache_p->add (newSelector, time, msId (), frameOfReference,
                                 spectralWindowId);

    return newSelector;
}

vi::ChannelSelector *
VisibilityIteratorImplAsync2::makeChannelSelectorC (const FrequencySelection & selectionIn,
                                                   Double time,
                                                   Int msId,
                                                   Int spectralWindowId)
{
    const FrequencySelectionUsingChannels & selection =
        dynamic_cast<const FrequencySelectionUsingChannels &> (selectionIn);

    vector<Slice> frequencySlices;

    // Iterate over all of the frequency selections for
    // the specified spectral window and collect them into
    // a vector of Slice objects.

    for (FrequencySelectionUsingChannels::const_iterator i = selection.begin();
         i != selection.end();
         i++){

        frequencySlices.push_back (i->getSlice());
    }

    if (frequencySlices.empty()){

        // And empty selection implies all channels

        Int nChannels = subtableColumns_p->spectralWindow().numChan()(spectralWindowId);
        frequencySlices.push_back (Slice (0, nChannels, 1));
    }

    // Convert the Slice collection built above into the needed
    // Vector<Vector<Slice> > structure.

    Vector <Vector <Slice> > slices (2);  // Cell array value is 2D: [nC,nF]
    slices [1].resize (frequencySlices.size());

    for (Int i = 0; i < (int) frequencySlices.size(); i++){
        slices [1][i] = frequencySlices [i];
    }

    // Package up the result and return it.

    ChannelSelector * result = new ChannelSelector (time, msId, spectralWindowId, slices);

    return result;
}

ChannelSelector *
VisibilityIteratorImplAsync2::makeChannelSelectorF (const FrequencySelection & selectionIn,
                                                   Double time, Int msId, Int spectralWindowId)
{
    // Make a ChannelSelector from a frame-based frequency selection.

    const FrequencySelectionUsingFrame & selection =
        dynamic_cast<const FrequencySelectionUsingFrame &> (selectionIn);

    vector<Slice> frequencySlices;

    selection.filterByWindow (spectralWindowId);

    // Set up frequency converter so that we can convert to the
    // measured frequency

    MFrequency::Convert convertToObservedFrame =
        makeFrequencyConverter (time, selection.getFrameOfReference(), True);

    // Convert each frequency selection into a Slice (interval) of channels.

    const SpectralWindowChannels & spectralWindowChannels =
        getSpectralWindowChannels (msId, spectralWindowId);

    for (FrequencySelectionUsingFrame::const_iterator i = selection.begin();
         i != selection.end();
         i++){

        Double f = i->getBeginFrequency();
        Double lowerFrequency = convertToObservedFrame (Quantity (f, "Hz")).get ("Hz").getValue();

        f = i->getEndFrequency();
        Double upperFrequency = convertToObservedFrame (Quantity (f, "Hz")).get ("Hz").getValue();

        Slice s = findChannelsInRange (lowerFrequency, upperFrequency, spectralWindowChannels);

        if (s.length () > 0){
            frequencySlices.push_back (s);
        }
    }

    // Convert the STL vector of slices into the expected Casa Vector<Vector <Slice>>
    // form. Element one of the Vector is empty indicating that the entire
    // correlations axis is desired.  The second element of the outer array specifies
    // different channel intervals along the channel axis.

    Vector <Vector <Slice> > slices (2);  // Cell array value is 2D: [nC,nF]
    slices [1].resize (frequencySlices.size());

    for (Int i = 0; i < (int) frequencySlices.size(); i++){
        slices [1][i] = frequencySlices [i];
    }

    // Package up result and return it.

    ChannelSelector * result = new ChannelSelector (time, msId, spectralWindowId, slices);

    return result;
}

MFrequency::Convert
VisibilityIteratorImplAsync2::makeFrequencyConverter (Double time, Int otherFrameOfReference,
                                                     Bool toObservedFrame) const
{
    MFrequency::Types observatoryFrequencyType = getObservatoryFrequencyType ();

    // Set up frequency converter so that we can convert to the
    // measured frequency

    MEpoch epoch (MVEpoch (Quantity (time, "s")));

    MPosition position = getObservatoryPosition ();
    MDirection direction = phaseCenter();

    MeasFrame measFrame (epoch, position, direction);

    MFrequency::Ref observedFrame (observatoryFrequencyType, measFrame);

    MFrequency::Types selectionFrame =
        static_cast<MFrequency::Types> (otherFrameOfReference);

    MFrequency::Convert result;

    if (toObservedFrame){

        result = MFrequency::Convert (selectionFrame, observedFrame);
    }
    else{

        result = MFrequency::Convert (observedFrame, selectionFrame);
    }

    return result;
}

Slice
VisibilityIteratorImplAsync2::findChannelsInRange (Double lowerFrequency, Double upperFrequency,
                                                  const vi::SpectralWindowChannels & spectralWindowChannels)
{
    ThrowIf (spectralWindowChannels.empty(),
             String::format ("No spectral window channel info for window=%d, ms=%d",
                            spectralWindow(), msId ()));

    typedef SpectralWindowChannels::const_iterator Iterator;

    Iterator first = spectralWindowChannels.begin();
    Iterator last = spectralWindowChannels.end() - 1;

    if (first->getFrequency() - 0.5 * first->getWidth() > upperFrequency ||
        last->getFrequency() + 0.5 * last->getWidth() < lowerFrequency){

        return Slice (0, 0); // value indicating no slice
    }

    // Find the first channel to include in the Slice.  The lower_bound method
    // returns the first channel >= to the target.

    Iterator lower = lower_bound (spectralWindowChannels.begin(),
                                  spectralWindowChannels.end(),
                                  lowerFrequency);

    Assert (lower != spectralWindowChannels.end());

    if (lower != spectralWindowChannels.begin()){

        Iterator previous = lower - 1;
        if (previous->getFrequency() + 0.5 * previous->getWidth() > lowerFrequency){

            lower = previous;

        }
    }

    // Find the ending channel to include in the Slice.  The upper_bound method
    // finds the first value > the target.

    Iterator upper = upper_bound (spectralWindowChannels.begin(),
                                  spectralWindowChannels.end(),
                                  upperFrequency);

    if (upper == spectralWindowChannels.end()){
        upper = spectralWindowChannels.end() - 1;
    }
    else{
        upper -= 1; // upper_bound finds value strictly > than target.
    }

    // Adjust the upper limit if the channel just outside the window overlaps
    // the upper frequency by 1/2 a channel width.

    Iterator next = upper + 1;
    if (next != spectralWindowChannels.end()){

        if (next->getFrequency() - 0.5 * next->getWidth() < upperFrequency){

            upper = next;

        }
    }

    // Create a slice.  If the frequencies are running high to low then
    // the increment needs to be -1; also length calculation needs to
    // adjust.

    Int increment = upper->getChannel() >= lower->getChannel() ? 1 : -1;
    Int length = increment * (upper->getChannel() - lower->getChannel()) + 1;
    Slice result (lower->getChannel(), length, increment);

    return result;
}

MFrequency::Types
VisibilityIteratorImplAsync2::getObservatoryFrequencyType () const
{
    const MFrequency & f0 = getMsIterInfo().frequency0();

    MFrequency::Types t = MFrequency::castType (f0.type());

    return t;
}

MPosition
VisibilityIteratorImplAsync2::getObservatoryPosition () const
{
    return getMsIterInfo().telescopePosition();
}

const SpectralWindowChannels &
VisibilityIteratorImplAsync2::getSpectralWindowChannels (Int msId, Int spectralWindowId) const
{
    const SpectralWindowChannels * cached =
            spectralWindowChannelsCache_p->find (msId, spectralWindowId);

    if (cached != 0){
        return * cached;
    }

    // Get the columns for the spectral window subtable and then select out the
    // frequency and width columns.  Use those columns to extract the frequency
    // and width lists for the specified spectral window.

    const ROMSSpWindowColumns& spectralWindow = subtableColumns_p->spectralWindow();

    const ROArrayColumn<Double>& frequenciesColumn = spectralWindow.chanFreq();
    Vector<Double> frequencies;
    frequenciesColumn.get (spectralWindowId, frequencies, True);

    const ROArrayColumn<Double>& widthsColumn = spectralWindow.chanWidth();
    Vector<Double> widths;
    widthsColumn.get (spectralWindowId, widths, True);

    Assert (! frequencies.empty());
    Assert (frequencies.size() == widths.size());

    // Use the frequencies and widths to fill out a vector of SpectralWindowChannel
    // objects. This array needs to be in order of increasing frequency.  N.B.: If
    // frequencies are in random order (i.e., rather than reverse order) then all
    // sorts of things will break elsewhere.

    SpectralWindowChannels * result = new SpectralWindowChannels (frequencies.size());
    bool inOrder = true;

    for (Int i = 0; i < (int) frequencies.nelements(); i++){
        (* result) [i] = SpectralWindowChannel (i, frequencies [i], widths [i]);
        inOrder = inOrder && (i == 0 || frequencies [i] > frequencies [i - 1]);
    }

    if (! inOrder){
        sort (result->begin(), result->end());
    }

    // Sanity check for frequencies that aren't monotonically increasing/decreasing.

    for (Int i = 1; i < (int) frequencies.nelements(); i++){
        ThrowIf (abs((* result) [i].getChannel() - (* result) [i-1].getChannel()) != 1,
                 String::format ("Spectral window %d in MS #%d has random ordered frequencies",
                                 spectralWindowId, msId));
    }

    spectralWindowChannelsCache_p->add (result, msId, spectralWindowId);

    return * result;
}

void
VisibilityIteratorImplAsync2::configureNewChunk ()
{
    rowBounds_p.chunkNRows_p = msIter_p.table ().nrow ();
    rowBounds_p.subchunkBegin_p = -1; // undefined value
    rowBounds_p.subchunkEnd_p = -1;   // will increment to 1st row

    cache_p.chunkRowIds_p.resize (0); // flush cached row number map.

    attachColumns (attachTable ());

    // Fetch all of the times in this chunk and get the min/max
    // of those times

    rowBounds_p.times_p.resize (rowBounds_p.chunkNRows_p);
    columns_p.time_p.getColumn (rowBounds_p.times_p);

    IPosition ignore1, ignore2;
    minMax (rowBounds_p.timeMin_p, rowBounds_p.timeMax_p, ignore1,
            ignore2, rowBounds_p.times_p);

    // If this is a new MeasurementSet then set up the antenna locations

    if (getMsIterInfo().newMS ()) {

        // Flush some cache flag values

        cache_p.flush ();

        msd_p.setAntennas (getMsIterInfo().msColumns ().antenna ());
    }

    if (getMsIterInfo().newField () || msIterAtOrigin_p) {
        msd_p.setFieldCenter (getMsIterInfo().phaseCenter ());
    }
}

const MSDerivedValues &
VisibilityIteratorImplAsync2::getMsd () const
{
    return msd_p;
}

void
VisibilityIteratorImplAsync2::setTileCache ()
{
    // This function sets the tile cache because of a feature in
    // sliced data access that grows memory dramatically in some cases
    //  if (useSlicer_p){

    if (! (getMsIterInfo().newDataDescriptionId () || getMsIterInfo().newMS ()) ) {
        return;
    }

    const MeasurementSet & theMs = getMsIterInfo().ms ();
    if (theMs.tableType () == Table::Memory) {
        return;
    }

    const ColumnDescSet & cds = theMs.tableDesc ().columnDescSet ();

    uInt startrow = getMsIterInfo().table ().rowNumbers ()(0); // Get the first row number for this DDID.

    if (tileCacheIsSet_p.nelements () != 8) {
        tileCacheIsSet_p.resize (8);
        tileCacheIsSet_p.set (False);
    }

    Vector<String> columns (8);
    columns (0) = MS::columnName (MS::DATA);            // complex
    columns (1) = MS::columnName (MS::CORRECTED_DATA);  // complex
    columns (2) = MS::columnName (MS::MODEL_DATA);      // complex
    columns (3) = MS::columnName (MS::FLAG);            // boolean
    columns (4) = MS::columnName (MS::WEIGHT_SPECTRUM); // float
    columns (5) = MS::columnName (MS::WEIGHT);          // float
    columns (6) = MS::columnName (MS::SIGMA);           // float
    columns (7) = MS::columnName (MS::UVW);             // double

    for (uInt k = 0; k < columns.nelements (); ++k) {

        if (! cds.isDefined (columns (k)) || ! usesTiledDataManager (columns[k], theMs)){
            continue;
        }

        try {
            //////////////////
            //////Temporary fix for virtual ms of multiple real ms's ...miracle of statics
            //////setting the cache size of hypercube at row 0 of each ms.
            ///will not work if each subms of a virtual ms has multi hypecube being
            ///accessed.
            if (theMs.tableInfo ().subType () == "CONCATENATED" &&
                msIterAtOrigin_p &&
                ! tileCacheIsSet_p[k]) {

                Block<String> refTables = theMs.getPartNames (True);

                for (uInt kk = 0; kk < refTables.nelements (); ++kk) {

                    MeasurementSet elms (refTables[kk]);
                    ROTiledStManAccessor tacc (elms, columns[k], True);
                    tacc.setCacheSize (0, 1);
                    tileCacheIsSet_p[k] = True;
                    //cerr << "set cache on kk " << kk << " vol " << columns[k] << "  " << refTables[kk] << endl;
                }
            }
            else {

                ROTiledStManAccessor tacc (theMs, columns[k], True);
                tacc.clearCaches (); //One tile only for now ...seems to work faster

                Bool setCache = True;

                for (uInt jj = 0 ; jj <  tacc.nhypercubes (); ++jj) {
                    if (tacc.getBucketSize (jj) == 0) {
                        setCache = False;
                    }
                }

                /// If some bucketSize is 0...there is trouble in setting cache
                /// but if slicer is used it gushes anyways if one does not set cache
                /// need to fix the 0 bucket size in the filler anyways...then this is not needed

                if (setCache) {
                    if (tacc.nhypercubes () == 1) {
                        tacc.setCacheSize (0, 1);
                    } else {
                        tacc.setCacheSize (startrow, 1);
                    }
                }
            }
        }
        catch (AipsError x) {
            //  cerr << "Data man type " << dataManType << "  " << dataManType.contains ("Tiled") << "  && " << (!String (cdesc.dataManagerGroup ()).empty ()) << endl;
            //  cerr << "Failed to set settilecache due to " << x.getMesg () << " column " << columns[k]  <<endl;
            //It failed so leave the caching as is
            continue;
        }
    }
}


Int
VisibilityIteratorImplAsync2::getDataDescriptionId () const
{
    return getMsIterInfo().dataDescriptionId ();
}

const MeasurementSet &
VisibilityIteratorImplAsync2::getMeasurementSet () const
{
    return getMsIterInfo().ms ();
}

Int
VisibilityIteratorImplAsync2::getMeasurementSetId () const
{
    return getMsIterInfo().msId ();
}

Int
VisibilityIteratorImplAsync2::getNAntennas () const
{
    Int nAntennas = getMsIterInfo().receptorAngle ().shape ()(1);

    return nAntennas;
}

MEpoch
VisibilityIteratorImplAsync2::getEpoch () const
{
    MEpoch mEpoch = getMsIterInfo().msColumns().timeMeas ()(0);

    return mEpoch;
}

Vector<Float>
VisibilityIteratorImplAsync2::getReceptor0Angle ()
{
    Int nAntennas = getNAntennas ();

    Vector<Float> receptor0Angle (nAntennas);

    for (int i = 0; i < nAntennas; i++) {
        receptor0Angle [i] = getMsIterInfo().receptorAngle ()(0, i);
    }

    return receptor0Angle;
}

void
VisibilityIteratorImplAsync2::getRowIds (Vector<uInt> & rowIds) const
{
    // Resize the rowIds vector and fill it with the row numbers contained
    // in the current subchunk.  These row numbers are relative to the reference
    // table used by MSIter to define a chunk.

    rowIds.resize (rowBounds_p.subchunkNRows_p);
    rowIds = rowBounds_p.subchunkRows_p.convert ();

    if (cache_p.chunkRowIds_p.nelements() == 0){

        // Create chunkRowIds_p as a map from chunk rows to MS rows.  This
        // needs to be created once per chunk since a new reference table is
        // created each time the MSIter moves to the next chunk.

        cache_p.chunkRowIds_p = getMsIterInfo().table ().rowNumbers (getMsIterInfo().ms ());

    }

    // Using chunkRowIds_p as a map from chunk rows to MS rows replace the
    // chunk-relative row numbers with the actual row number from the MS.

    for (uInt i = 0; i < rowIds.nelements (); i++) {

        rowIds (i) = cache_p.chunkRowIds_p (rowIds (i));
    }
}

void
VisibilityIteratorImplAsync2::antenna1(Vector<Int> & ant1) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::antenna2(Vector<Int> & ant2) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::feed1(Vector<Int> & fd1) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::feed2(Vector<Int> & fd2) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::corrType (Vector<Int> & corrTypes) const
{
#warning "Check this"
    Int polId = getMsIterInfo().polarizationId ();

    subtableColumns_p->polarization ().corrType ().get (polId, corrTypes, True);
}

void
VisibilityIteratorImplAsync2::flag (Cube<Bool> & flags) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::flag (Matrix<Bool> & flags) const
{
    ThrowNotPrefetchedException();
}

Bool
VisibilityIteratorImplAsync2::existsFlagCategory() const
{
#warning "Check this"
    if(getMsIterInfo().newMS()){ // Cache to avoid testing unnecessarily.
    try{
      cache_p.msHasFC_p = columns_p.flagCategory_p.hasContent();
    }
    catch (AipsError &){
      cache_p.msHasFC_p = False;
    }
  }
  return cache_p.msHasFC_p;
}

void
VisibilityIteratorImplAsync2::flagCategory (Array<Bool> & flagCategories) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::flagRow (Vector<Bool> & rowflags) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::observationId (Vector<Int> & obsIDs) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::processorId (Vector<Int> & procIDs) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::scan (Vector<Int> & scans) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::stateId (Vector<Int> & stateIds) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::time (Vector<Double> & t) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::timeCentroid (Vector<Double> & t) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::timeInterval (Vector<Double> & t) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::exposure (Vector<Double> & expo) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::visibilityCorrected (Cube<Complex> & vis) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::visibilityModel (Cube<Complex> & vis) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::visibilityObserved (Cube<Complex> & vis) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::floatData (Cube<Float> & fcube) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::visibilityCorrected (Matrix<CStokesVector> & vis) const
{
    ThrowNotPrefetchedException();
}
void
VisibilityIteratorImplAsync2::visibilityModel (Matrix<CStokesVector> & vis) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::visibilityObserved (Matrix<CStokesVector> & vis) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::getVisibilityAsStokes (Matrix<CStokesVector> & visibilityStokes,
                                                    const ROArrayColumn<Complex> & column) const
{
    // Read in the raw visibility data

    Cube<Complex> visibilityRaw;

    getColumnRows (column, visibilityRaw);

    // Resize the result (Matrix<CStokesVector>) to match up with the
    // shape of the data.

    Int nFrequencies = visibilityRaw.shape()(1);

    visibilityStokes.resize (nFrequencies, rowBounds_p.subchunkNRows_p);

    // Convert the polarizations into a Stokes Vector for each channel, row.
    // The cases where only 1 or 2 polarizations are present require some
    // assumptions/approximations be made to fill out the vector.
    //
    // The original implementation used a pointer based scheme to optimize
    // the performance.  If this routine turns out to be too sluggish then
    // an enhancement along those lines might be worthy the added opacity.

    if (nPolarizations_p == 4){

        for (Int row = 0; row < rowBounds_p.subchunkNRows_p; row ++) {
            for (Int frequency = 0; frequency < nFrequencies; frequency ++) {

                CStokesVector & stokesVector = visibilityStokes (frequency, row);

                for (Int polarization = 0; polarization < 4; polarization ++){

                    stokesVector (polarization) = visibilityRaw (polarization, frequency, row);

                }
            }
        }
    }
    else if (nPolarizations_p == 2){

        //  Assume XX,YY or RR,LL are provided.  Set cross terms to zero.

        for (Int row = 0; row < rowBounds_p.subchunkNRows_p; row ++) {
            for (Int frequency = 0; frequency < nFrequencies; frequency ++) {

                CStokesVector & stokesVector = visibilityStokes (frequency, row);

                stokesVector (0) = visibilityRaw (0, frequency, row);
                stokesVector (1) = 0;
                stokesVector (2) = 0;
                stokesVector (3) = visibilityRaw (1, frequency, row);
            }
        }
    }
    else if (nPolarizations_p == 1){

        // Assume provided polarization is an estimate of Stokes I (one of RR,LL,XX,YY).
        // Cross terms are then zero.

        for (Int row = 0; row < rowBounds_p.subchunkNRows_p; row ++) {
            for (Int frequency = 0; frequency < nFrequencies; frequency ++) {

                CStokesVector & stokesVector = visibilityStokes (frequency, row);

                stokesVector (0) = visibilityRaw (0, frequency, row);
                stokesVector (3) = stokesVector (0);
                stokesVector (1) = 0;
                stokesVector (2) = 0;
            }
        }

    }
    else{
        ThrowIf (True, String::format ("Unexpected number of polarizations: %d; should be 1, 2 or 4",
                                       nPolarizations_p));
    }
}

void
VisibilityIteratorImplAsync2::uvw (Matrix<Double> & uvwmat) const
{
    ThrowNotPrefetchedException();
}

// Fill in parallactic angle.
const Vector<Float> &
VisibilityIteratorImplAsync2::feed_pa (Double time) const
{
    //  LogMessage message (LogOrigin ("VisibilityIteratorImplAsync2","feed_pa"));

    // Absolute UT
    Double ut = time;

    if (ut != cache_p.feedpaTime_p) {

        // Set up the Epoch using the absolute MJD in seconds
        // get the Epoch reference from the column

        MEpoch mEpoch = getEpoch ();

        const Matrix<Double> & angles = receptorAngles ().xyPlane(0);
        Int nAnt = angles.shape ()(1);

        Vector<Float> receptor0Angle (nAnt, 0);

        for (int i = 0; i < nAnt; i++) {
            receptor0Angle [i] = angles (0, i);
        }

        cache_p.feedpa_p.assign (feed_paCalculate (time, msd_p, nAnt, mEpoch, receptor0Angle));

        cache_p.feedpaTime_p = ut;
    }
    return cache_p.feedpa_p;
}

// Fill in parallactic angle.
const Float &
VisibilityIteratorImplAsync2::parang0(Double time) const
{
    if (time != cache_p.parang0Time_p) {

        cache_p.parang0Time_p = time;

        // Set up the Epoch using the absolute MJD in seconds
        // get the Epoch reference from the column
        MEpoch mEpoch = getEpoch();
        cache_p.parang0_p = parang0Calculate (time, msd_p, mEpoch);
    }
    return cache_p.parang0_p;
}

// Fill in parallactic angle (NO FEED PA offset!).
const Vector<Float> &
VisibilityIteratorImplAsync2::parang (Double time) const
{
    if (time != cache_p.parangTime_p) {

        cache_p.parangTime_p = time;

        // Set up the Epoch using the absolute MJD in seconds
        // get the Epoch reference from the column

        MEpoch mEpoch = getEpoch();
        Int nAnt = getMsIterInfo().receptorAngle ().shape ()(1);

        cache_p.parang_p = parangCalculate (time, msd_p, nAnt, mEpoch);

    }
    return cache_p.parang_p;
}

// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
const Vector<MDirection> &
VisibilityIteratorImplAsync2::azel (Double ut) const
{
    if (ut != cache_p.azelTime_p) {

        cache_p.azelTime_p = ut;

        Int nAnt = getMsIterInfo().receptorAngle ().shape ()(1);

        MEpoch mEpoch = getEpoch();

        azelCalculate (ut, msd_p, cache_p.azel_p, nAnt, mEpoch);

    }
    return cache_p.azel_p;
}

// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
MDirection
VisibilityIteratorImplAsync2::azel0(Double time) const
{
    //  LogMessage message (LogOrigin ("VisibilityIteratorImplAsync2","azel0"));

    // Absolute UT
    Double ut = time;

    if (ut != cache_p.azel0Time_p) {

        cache_p.azel0Time_p = ut;

        MEpoch mEpoch = getEpoch();

        azel0Calculate (time, msd_p, cache_p.azel0_p, mEpoch);

    }
    return cache_p.azel0_p;
}

// Hour angle at specified time.
Double
VisibilityIteratorImplAsync2::hourang (Double time) const
{
    //  LogMessage message (LogOrigin ("VisibilityIteratorImplAsync2","azel"));

    // Absolute UT
    Double ut = time;

    if (ut != cache_p.hourangTime_p) {

        cache_p.hourangTime_p = ut;

        // Set up the Epoch using the absolute MJD in seconds
        // get the Epoch reference from the column keyword

        MEpoch mEpoch = getEpoch();

        cache_p.hourang_p = hourangCalculate (time, msd_p, mEpoch);

    }
    return cache_p.hourang_p;
}

void
VisibilityIteratorImplAsync2::sigma (Vector<Float> & sigmaVector) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::sigmaMat (Matrix<Float> & sigmat) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::weight (Vector<Float> & wt) const
{
    ThrowNotPrefetchedException();
}

void
VisibilityIteratorImplAsync2::weightMat (Matrix<Float> & wtmat) const
{
    ThrowNotPrefetchedException();
}

Bool
VisibilityIteratorImplAsync2::existsWeightSpectrum () const
{
    if (getMsIterInfo().newMS ()) { // Cache to avoid testing unnecessarily.
        try {
            cache_p.msHasWtSp_p = columns_p.weightSpectrum_p.hasContent ();
            // Comparing columns_p.weightSpectrum_p.shape (0) to
            // IPosition (2, nPolarizations_p, channelGroupSize ()) is too strict
            // when channel averaging might have changed
            // channelGroupSize () or weightSpectrum () out of sync.  Unfortunately the
            // right answer might not get cached soon enough.
            //
            //       columns_p.weightSpectrum_p.shape (0).isEqual (IPosition (2, nPolarizations_p,
            //                                                    channelGroupSize ())));
            // if (!msHasWtSp_p){
            //   cerr << "columns_p.weightSpectrum_p.shape (0): " << columns_p.weightSpectrum_p.shape (0) << endl;
            //   cerr << "(nPolarizations_p, channelGroupSize ()): " << nPolarizations_p
            //        << ", " << channelGroupSize () << endl;
            // }
        } catch (AipsError x) {
            cache_p.msHasWtSp_p = False;
        }
    }
    return cache_p.msHasWtSp_p;
}

void
VisibilityIteratorImplAsync2::weightSpectrum (Cube<Float> & spectrum) const
{
    ThrowNotPrefetchedException();
}

const VisImagingWeight &
VisibilityIteratorImplAsync2::getImagingWeightGenerator () const
{
    return imwgt_p;
}

vector<MeasurementSet>
VisibilityIteratorImplAsync2::getMeasurementSets () const
{
    return measurementSets_p;
}

Int
VisibilityIteratorImplAsync2::getReportingFrameOfReference () const
{
    Int frame;
    if (reportingFrame_p == VisBuffer2::FrameNotSpecified){

        if (frequencySelections_p != 0){

            frame = frequencySelections_p->getFrameOfReference();

            if (frame == FrequencySelection::ByChannel){

                // Since selection was done by channels, the frequencies
                // are native.

                frame = getObservatoryFrequencyType ();
            }
        }
        else{
            frame = VisBuffer2::FrameNotSpecified;
        }
    }
    else{
        frame = reportingFrame_p;
    }

    return frame;
}

void
VisibilityIteratorImplAsync2::setReportingFrameOfReference (Int frame)
{
    ThrowIf (frame < 0 || frame >= MFrequency::N_Types,
             String::format ("Unknown frame: id=%d", frame));

    reportingFrame_p = frame;
}

VisBuffer2 *
VisibilityIteratorImplAsync2::getVisBuffer ()
{
    return vb_p;
}

Int
VisibilityIteratorImplAsync2::numberAnt ()
{
    return subtableColumns_p->antenna ().nrow (); // for single (sub)array only..
}

Int
VisibilityIteratorImplAsync2::numberSpw ()
{
    return subtableColumns_p->spectralWindow ().nrow ();
}

Int
VisibilityIteratorImplAsync2::numberDDId ()
{
    return subtableColumns_p->dataDescription ().nrow ();
}

Int
VisibilityIteratorImplAsync2::numberPol ()
{
    return subtableColumns_p->polarization ().nrow ();
}

Int
VisibilityIteratorImplAsync2::numberCoh ()
{
    Int numcoh = 0;
    for (uInt k = 0; k < uInt (getMsIterInfo().numMS ()) ; ++k) {
        numcoh += getMsIterInfo().ms (k).nrow ();
    }
    return numcoh;

}

const Table
VisibilityIteratorImplAsync2::attachTable () const
{
    ThrowIllegalOperationException();
}

void
VisibilityIteratorImplAsync2::slurp () const
{
#warning "Check this"
    // Set the table data manager (ISM and SSM) cache size to the full column size, for
    //   the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW

    Record dmInfo (getMsIterInfo().ms ().dataManagerInfo ());

    RecordDesc desc = dmInfo.description ();

    for (unsigned i = 0; i < dmInfo.nfields (); i++) {

        if (desc.isSubRecord (i)) {

            Record sub = dmInfo.subRecord (i);

            if (sub.fieldNumber ("NAME") >= 0 &&
                sub.fieldNumber ("TYPE") >= 0 &&
                sub.fieldNumber ("COLUMNS") >= 0 &&
                sub.type (sub.fieldNumber ("NAME")) == TpString &&
                sub.type (sub.fieldNumber ("TYPE")) == TpString &&
                sub.type (sub.fieldNumber ("COLUMNS")) == TpArrayString) {

                Array<String> columns;
                dmInfo.subRecord (i).get ("COLUMNS", columns);

                bool match = false;

                for (unsigned j = 0; j < columns.nelements (); j++) {

                    String column = columns (IPosition (1, j));

                    match |= (column == MS::columnName (MS::ANTENNA1) ||
                              column == MS::columnName (MS::ANTENNA2) ||
                              column == MS::columnName (MS::FEED1) ||
                              column == MS::columnName (MS::FEED2) ||
                              column == MS::columnName (MS::TIME) ||
                              column == MS::columnName (MS::INTERVAL) ||
                              column == MS::columnName (MS::FLAG_ROW) ||
                              column == MS::columnName (MS::SCAN_NUMBER) ||
                              column == MS::columnName (MS::UVW));
                }

                if (match) {

                    String dm_name;
                    dmInfo.subRecord (i).get ("NAME", dm_name);

                    String dm_type;
                    dmInfo.subRecord (i).get ("TYPE", dm_type);

                    Bool can_exceed_nr_buckets = False;
                    uInt num_buckets = getMsIterInfo().ms ().nrow ();
                        // One bucket is at least one row, so this is enough

                    if (dm_type == "IncrementalStMan") {

                        ROIncrementalStManAccessor acc (getMsIterInfo().ms (), dm_name);
                        acc.setCacheSize (num_buckets, can_exceed_nr_buckets);

                    } else if (dm_type == "StandardStMan") {

                        ROStandardStManAccessor acc (getMsIterInfo().ms (), dm_name);
                        acc.setCacheSize (num_buckets, can_exceed_nr_buckets);
                    }
                    /* These are the only storage managers which use the BucketCache
                     (and therefore are slow for random access and small cache sizes)
                     */
                }
                else {

                    String dm_name;
                    dmInfo.subRecord (i).get ("NAME", dm_name);

                }
            } else {
                cerr << "Data manager info has unexpected shape! " << sub << endl;
            }
        }
    }
    return;
}

const Cube<Double> &
VisibilityIteratorImplAsync2::receptorAngles() const
{
#warning "Check this"
    return getMsIterInfo().receptorAngles ();
}

IPosition
VisibilityIteratorImplAsync2::visibilityShape() const
{
#warning "Check this"

    IPosition result (3,
                      nPolarizations_p,
                      channelSelector_p->getNFrequencies(),
                      rowBounds_p.subchunkNRows_p);

    return result;
}

void
VisibilityIteratorImplAsync2::setFrequencySelections (FrequencySelections const& frequencySelections)
{
#warning "Check this"
    pendingChanges_p.setFrequencySelections (frequencySelections.clone ());

    channelSelectorCache_p->flush();
}

void
VisibilityIteratorImplAsync2::jonesC(Vector<SquareMatrix<complex<float>, 2> >& cjones) const
{
    ThrowNotPrefetchedException();
}


void
VisibilityIteratorImplAsync2::writeFlag (const Matrix<Bool> & flag)
{
#warning "Check this"

    Int nFrequencies = channelSelector_p->getNFrequencies();
    Int nPolarizations = nPolarizations_p;
    Int nRows = this->nRows();

    // The flag matrix is expected to have dimensions [nF, nR].

    ThrowIf ((int) flag.nrow() != nFrequencies || (int) flag.ncolumn() != nRows,
             String::format ("Shape mismatch: got [%d,%d]; expected [%d,%d]",
                             flag.nrow(), flag.ncolumn(), nFrequencies, nRows));

    // Convert the flag matrix into the flag cube defined in the MS.  The flag
    // value for a (row,channel) will be copied into all of the polarizations.

    Cube<Bool> flagCube;

    flagCube.resize (nPolarizations, nFrequencies, nRows);

    for (Int row = 0; row < nRows; row++) {

        for (Int frequency = 0; frequency < nFrequencies; frequency ++) {

            Bool flagValue = flag (frequency, row); // same value for all polarizations

            for (Int polarization = 0; polarization < nPolarizations; polarization ++) {

                flagCube (polarization, frequency, row) = flagValue;
            }
        }
    }

    // Write the newly constructed flag cube into the MS.

    writeFlag (flagCube);
}

void
VisibilityIteratorImplAsync2::writeFlag (const Cube<Bool> & flags)
{
#warning "Check this"
    putColumnRows (columns_p.flag_p, flags);
}

void
VisibilityIteratorImplAsync2::writeFlagCategory(const Array<Bool>& flagCategory)
{
#warning "Check this"

    // Flag categories are [nC, nF, nCategories] and therefore must use a
    // different slicer which also prevents use of more usual putColumn method.

    RefRows & rows = rowBounds_p.subchunkRows_p;

    columns_p.flagCategory_p.putSliceFromRows (rows,
                                               channelSelector_p->getSlicerForFlagCategories(),
                                               flagCategory);
}

void
VisibilityIteratorImplAsync2::writeFlagRow (const Vector<Bool> & rowflags)
{
#warning "Check this"
    putColumnRows (columns_p.flagRow_p, rowflags);
}

void
VisibilityIteratorImplAsync2::writeVisCorrected (const Matrix<CStokesVector> & visibilityStokes)
{
#warning "Check this"
    Cube<Complex> visCube;

    convertVisFromStokes (visibilityStokes, visCube);

    writeVisCorrected (visCube);
}

void
VisibilityIteratorImplAsync2::writeVisModel (const Matrix<CStokesVector> & visibilityStokes)
{
#warning "Check this"
    Cube<Complex> visCube;

    convertVisFromStokes (visibilityStokes, visCube);

    writeVisModel (visCube);
}
                                                  void
VisibilityIteratorImplAsync2::writeVisObserved (const Matrix<CStokesVector> & visibilityStokes)
{
#warning "Check this"
    Cube<Complex> visCube;

    convertVisFromStokes (visibilityStokes, visCube);

    writeVisObserved (visCube);
}

void
VisibilityIteratorImplAsync2::convertVisFromStokes (const Matrix<CStokesVector> & visibilityStokes,
                                                    Cube<Complex> & visibilityCube)
{
#warning "Check this"
    Int nRows = rowBounds_p.subchunkNRows_p;
    Int nFrequencies = channelSelector_p->getNFrequencies();
    Int nPolarizations = this->nPolarizations();

    visibilityCube.resize (nPolarizations, nFrequencies, nRows);

    // The cube could be walked through using pointer operations rather than
    // using cube references.  If the straight-forward implementation below
    // proves to be too slow, then it can be reimplemented using pointer
    // operations.

    for (Int row = 0; row < nRows; row ++) {

        for (Int frequency = 0; frequency < nFrequencies; frequency ++) {

            const CStokesVector & v = visibilityStokes (frequency, row);

            if (nPolarizations == 4){

                visibilityCube (0, frequency, row) = v (0);
                visibilityCube (1, frequency, row) = v (1);
                visibilityCube (2, frequency, row) = v (2);
                visibilityCube (3, frequency, row) = v (3);
            }
            else if (nPolarizations == 2){

                visibilityCube (0, frequency, row) = v (0);
                visibilityCube (1, frequency, row) = v (3);
            }
            else if (nPolarizations == 1){

                visibilityCube (0, frequency, row) = (v (0) + v (3)) / 2;
            }
        }
    }
}

void
VisibilityIteratorImplAsync2::writeVisCorrected (const Cube<Complex> & vis)
{
#warning "Check this"
    putColumnRows (columns_p.corrVis_p, vis);
}

void
VisibilityIteratorImplAsync2::writeVisModel (const Cube<Complex> & vis)
{
#warning "Check this"
    putColumnRows (columns_p.modelVis_p, vis);
}

void
VisibilityIteratorImplAsync2::writeVisObserved (const Cube<Complex> & vis)
{
#warning "Check this"
    if (floatDataFound_p) {

        // This MS has float data; convert the cube to float
        // and right it out

        Cube<Float> dataFloat = real (vis);
        putColumnRows (columns_p.floatVis_p, dataFloat);

    }
    else {
        putColumnRows (columns_p.vis_p, vis);
    }

}

void
VisibilityIteratorImplAsync2::writeWeight (const Vector<Float> & weight)
{
#warning "Check this"
    Int nPolarizations = this->nPolarizations();
    Int nRows = this->nRows();

    ThrowIf ((int) weight.nelements() != nRows,
             String::format ("Dimension mismatch: got %d rows but expected %d rows",
                             weight.nelements(), nRows));

    Matrix<Float> weightMatrix;
    weightMatrix.resize (nPolarizations, nRows);

    // Weight is stored as a [nC] vector per row but the parameter provides
    // one value per row.  Spread the provided value acroos all correlations.

    for (Int i = 0; i < nPolarizations; i++) {
        Vector<Float> r = weightMatrix.row (i);
        r = weight;
    }

    writeWeightMat (weightMatrix);
}

void
VisibilityIteratorImplAsync2::writeWeightMat (const Matrix<Float> & weightMat)
{
#warning "Check this"
    putColumnRows (columns_p.weight_p, weightMat);
}

void
VisibilityIteratorImplAsync2::writeWeightSpectrum (const Cube<Float> & weightSpectrum)
{
#warning "Check this"
    if (! columns_p.weightSpectrum_p.isNull ()) {
        putColumnRows (columns_p.weightSpectrum_p, weightSpectrum);
    }
}

void
VisibilityIteratorImplAsync2::writeSigma (const Vector<Float> & sigma)
{
#warning "Check this"
    Int nPolarizations = this->nPolarizations();
    Int nRows = this->nRows();

    ThrowIf ((int) sigma.nelements() != nRows,
             String::format ("Dimension mismatch: got %d rows but expected %d rows",
                             sigma.nelements(), nRows));

    Matrix<Float> sigmaMatrix;
    sigmaMatrix.resize (nPolarizations, nRows);

    // Sigma is stored as a [nC] vector per row but the parameter provides
    // one value per row.  Spread the provided value acroos all correlations.

    for (Int i = 0; i < nPolarizations; i++) {
        Vector<Float> r = sigmaMatrix.row (i);
        r = sigma;
    }

    writeSigmaMat (sigmaMatrix);
}

void
VisibilityIteratorImplAsync2::writeSigmaMat (const Matrix<Float> & sigMat)
{
#warning "Check this"
    putColumnRows (columns_p.sigma_p, sigMat);
}

void
VisibilityIteratorImplAsync2::putModel(const RecordInterface& rec, Bool iscomponentlist, Bool incremental)
{

#warning "--> Reimplement putModel(const RecordInterface& rec, Bool iscomponentlist, Bool incremental)"

//  Vector<Int> fields = subtableColumns().fieldId().getColumn();
//
//  const Int option = Sort::HeapSort | Sort::NoDuplicates;
//  const Sort::Order order = Sort::Ascending;
//
//  Int nfields = GenSort<Int>::sort (fields, order, option);
//
//  // Make sure  we have the right size
//
//  fields.resize(nfields, True);
//  Int msid = msId();
//
//  Vector<Int> spws =  subtableColumns().spw_p[msid];
//  Vector<Int> starts = subtableColumns().start_p[msid];
//  Vector<Int> nchan = subtableColumns().width_p[msid];
//  Vector<Int> incr = subtableColumns().inc_p[msid];
//
//  VisModelData::putModel(ms(), rec, fields, spws, starts, nchan, incr,
//                         iscomponentlist, incremental);
    
}

void
VisibilityIteratorImplAsync2::writeBackChanges (VisBuffer2 * vb)
{
#warning "Check this"
    if (backWriters_p.empty ()) {
        initializeBackWriters ();
    }

    VisBufferComponents2 dirtyComponents = vb->dirtyComponentsGet ();

    for (VisBufferComponents2::const_iterator dirtyComponent = dirtyComponents.begin ();
         dirtyComponent != dirtyComponents.end ();
         dirtyComponent ++) {

        ThrowIf (backWriters_p.find (* dirtyComponent) == backWriters_p.end (),
                 String::format ("No writer defined for VisBuffer component %d", * dirtyComponent));
        BackWriter * backWriter = backWriters_p [ * dirtyComponent];

        try {
            (* backWriter) (this, vb);
        } catch (AipsError & e) {
            Rethrow (e, String::format ("Error while writing back VisBuffer component %d", * dirtyComponent));
        }
    }
}

void
VisibilityIteratorImplAsync2::initializeBackWriters ()
{
    backWriters_p [Flag] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeFlag, & VisBuffer2::flag);
    backWriters_p [FlagCube] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeFlag, & VisBuffer2::flagCube);
    backWriters_p [FlagRow] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeFlagRow, & VisBuffer2::flagRow);
    backWriters_p [FlagCategory] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeFlagCategory, & VisBuffer2::flagCategory);
    backWriters_p [Sigma] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeSigma, & VisBuffer2::sigma);
    backWriters_p [SigmaMat] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeSigmaMat, & VisBuffer2::sigmaMat);
    backWriters_p [Weight] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeWeight, & VisBuffer2::weight);
    backWriters_p [WeightMat] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeWeightMat, & VisBuffer2::weightMat);
    backWriters_p [WeightSpectrum] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeWeightSpectrum, & VisBuffer2::weightSpectrum);

    // Now do the visibilities.

    backWriters_p [VisibilityObserved] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeVisObserved, & VisBuffer2::vis);
    backWriters_p [VisibilityCorrected] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeVisCorrected, & VisBuffer2::visCorrected);
    backWriters_p [VisibilityModel] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeVisModel, & VisBuffer2::visModel);

    backWriters_p [VisibilityCubeObserved] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeVisObserved, & VisBuffer2::visCube);
    backWriters_p [VisibilityCubeCorrected] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeVisCorrected, & VisBuffer2::visCubeCorrected);
    backWriters_p [VisibilityCubeModel] =
        makeBackWriter (& VisibilityIteratorImplAsync2::writeVisModel, & VisBuffer2::visCubeModel);

}

VisibilityIteratorImplAsync2::Columns &
VisibilityIteratorImplAsync2::Columns::operator= (const VisibilityIteratorImplAsync2::Columns & other)
{
    flag_p.reference (other.flag_p);
    flagCategory_p.reference (other.flagCategory_p);
    flagRow_p.reference (other.flagRow_p);
    vis_p.reference (other.vis_p);
    floatVis_p.reference (other.floatVis_p);
    modelVis_p.reference (other.modelVis_p);
    corrVis_p.reference (other.corrVis_p);
    weight_p.reference (other.weight_p);
    weightSpectrum_p.reference (other.weightSpectrum_p);
    sigma_p.reference (other.sigma_p);

    return * this;
}

VisibilityIteratorImplAsync2::PendingChanges::PendingChanges ()
: frequencySelections_p (0),
  frequencySelectionsPending_p (False),
  interval_p (Empty),
  nRowBlocking_p (Empty)
{}

VisibilityIteratorImplAsync2::PendingChanges::~PendingChanges ()
{
    delete frequencySelections_p;
}

VisibilityIteratorImplAsync2::PendingChanges *
VisibilityIteratorImplAsync2::PendingChanges::clone () const
{
    PendingChanges * theClone = new PendingChanges ();

    theClone->frequencySelections_p = new FrequencySelections (* frequencySelections_p);
    theClone->frequencySelectionsPending_p = frequencySelectionsPending_p;
    theClone->interval_p = interval_p;
    theClone->nRowBlocking_p = nRowBlocking_p;

    return theClone;
}


Bool
VisibilityIteratorImplAsync2::PendingChanges::empty () const
{
    Bool result = frequencySelections_p == 0 &&
                  interval_p == Empty &&
                  nRowBlocking_p == Empty;

    return result;
}

pair<Bool, FrequencySelections *>
VisibilityIteratorImplAsync2::PendingChanges::popFrequencySelections () // yields ownershi
{
    FrequencySelections * result = frequencySelections_p;
    Bool wasPending = frequencySelectionsPending_p;

    frequencySelections_p = 0;
    frequencySelectionsPending_p = False;

    return make_pair (wasPending, result);
}

pair<Bool, Double>
VisibilityIteratorImplAsync2::PendingChanges::popInterval ()
{
    pair<Bool,Double> result = make_pair (interval_p != Empty, interval_p);

    interval_p = Empty;

    return result;
}

pair<Bool, Int>
VisibilityIteratorImplAsync2::PendingChanges::popNRowBlocking ()
{
    pair<Bool,Int> result = make_pair (nRowBlocking_p != Empty, nRowBlocking_p);

    nRowBlocking_p = Empty;

    return result;
}

void
VisibilityIteratorImplAsync2::PendingChanges::setFrequencySelections (FrequencySelections * fs) // takes ownershi
{
    Assert (! frequencySelectionsPending_p);

    frequencySelections_p = fs;
    frequencySelectionsPending_p = True;
}

void
VisibilityIteratorImplAsync2::PendingChanges::setInterval (Double interval)
{
    Assert (interval_p == Empty);

    interval_p = interval;
}

void
VisibilityIteratorImplAsync2::PendingChanges::setNRowBlocking (Int nRowBlocking)
{
    Assert (nRowBlocking_p == Empty);

    nRowBlocking_p = nRowBlocking;
}

} // end namespace vi

} //# NAMESPACE CASA - END


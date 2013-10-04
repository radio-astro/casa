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
#include <casa/Arrays.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities.h>
#include <ms/MeasurementSets.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSSpwIndex.h>
#include <scimath/Mathematics/InterpolateArray1D.h>
//#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/MeasurementSet2.h>
#include <synthesis/MSVis/MSUtil.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/SpectralWindow.h>
#include <synthesis/MSVis/ViFrequencySelection.h>
#include <synthesis/MSVis/VisBuffer2.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisibilityIteratorImpl2.h>
#include <synthesis/TransformMachines/VisModelData.h>
#include <tables/Tables/ColDescSet.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/IncrStManAccessor.h>
#include <tables/Tables/StandardStManAccessor.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TiledStManAccessor.h>

#include <cassert>
#include <algorithm>
#include <limits>
#include <memory>
#include <map>
#include <vector>

using std::make_pair;
using namespace casa::vi;
using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

Bool
operator!= (const Slice & a, const Slice & b)
{
    Bool result = a.start () != b.start ()||
                  a.length () != b.length () ||
                  a.inc () != b.inc();

    return result;
}

// ChannelSubslicer - represents a single selection from a DATA matrix
//
// Typically the ChannelSubslicer uses selects elements from the DATA
// matrix in two dimensions: correlation and channel.  Two slice objects
// are used, the first one for correlation and the other for channel.  The slice
// object is a triple of starting channel, increment and the number of
// channels.

class ChannelSubslicer {

public:

    enum {Correlation, Channel};

    ChannelSubslicer ()
    : subslicer_p ()
    {}

    ChannelSubslicer (Int n)
    : subslicer_p (n)
    {}

    ChannelSubslicer (const Vector<Slice> & axis)
    : subslicer_p (axis.nelements())
    {
        for (uInt i = 0; i < axis.nelements(); i++){
            subslicer_p [i] = axis (i);
        }
    }

    Bool
    operator== (const ChannelSubslicer & other) const
    {
        if (other.nelements() != nelements()){
            return False;
        }

        for (uInt i = 0; i < nelements(); i++){

            if (! slicesEqual (subslicer_p [i], other.subslicer_p [i])){
                return False;
            }
        }

        return True;
    }

    Bool
    operator!= (const ChannelSubslicer & other) const
    {
        return ! (* this == other);
    }

    const Slice &
    getSlice (Int i) const
    {
        return subslicer_p [i];
    }

    size_t nelements () const
    {
        return subslicer_p.size();
    }

    void
    setSlice (Int i, const Slice & slice)
    {
        subslicer_p [i] = slice;
    }

protected:

    static Bool
    slicesEqual (const Slice & a, const Slice & b){

        return a.start () == b.start () &&
               a.length () == b.length () &&
               a.inc () == b.inc ();

    }

private:

    vector<Slice> subslicer_p;
};

// ChannelSlicer - represents the channels selected in a window.
//
// The ChannelSlicer is a collection of channel and correlation
// selections.  Each selection, a ChannelSubslicer, specifies a
// slice for each dimension of the data cube: correlation,
// channel (row selection is not managed by this data structure).

class ChannelSlicer {

public:

    typedef vector<ChannelSubslicer> Rep;
    typedef Vector<Vector <Slice> > CoreRep;

    ChannelSlicer ()
    : slicer_p ()
    {}

    ChannelSlicer (Int nAxes)
    : slicer_p (nAxes)
    {}

    bool
    operator== (const ChannelSlicer & other) const
    {

        if (nelements () != other.nelements()){
            return false;
        }


        for (uInt i = 0; i < nelements(); i++){

            if (slicer_p [i] != other.slicer_p [i]){
                return false;
            }

        }

        return true;
    }

    void
    appendDimension ()
    {
        slicer_p.push_back (ChannelSubslicer());
    }

    CoreRep
    getSlicerInCoreRep () const
    {
        // Convert to Vector<Vector <Slice> > for use by
        // casacore methods

        CoreRep rep (nelements());

        for (uInt i = 0; i < nelements(); i ++){

            const ChannelSubslicer & subslicer = slicer_p [i];

            rep [i] = Vector<Slice> (subslicer.nelements());

            for (uInt j = 0; j < subslicer.nelements(); j ++){

                rep [i][j] = subslicer.getSlice (j);
            }
        }

        return rep;
    }

    const ChannelSubslicer &
    getSubslicer (Int i) const
    {
        return slicer_p [i];
    }

    size_t nelements () const
    {
        return slicer_p.size();
    }

    void
    setSubslicer (Int i, const ChannelSubslicer & subslice)
    {
        slicer_p [i] = subslice;
    }


    String
    toString () const
    {
        String result = "{";

        for (Rep::const_iterator i = slicer_p.begin(); i != slicer_p.end(); i++){

            result += String ((i == slicer_p.begin()) ? "" : ", ") + "{ ";

            const ChannelSubslicer & subslicer = * i;

            for (uInt j = 0; j < subslicer.nelements(); j ++){

                const Slice & slice = subslicer.getSlice (j);
                result += String::format ("(st=%d, len=%d, inc=%d)",
                                          slice.start(), slice.length(), slice.inc());
            }

            result += " }";
        }

        result += " }";

        return result;
    }

private:

    Rep slicer_p;
};

// ChannelSelector - class that provides the channels that need to be extracted from
//                   a spectral window at a given time for a given MS.
//


class ChannelSelector {

public:

    ChannelSelector (Double time, Int msId, Int spectralWindowId, const ChannelSlicer & slicer)
    : msId_p (msId),
      slicer_p (slicer),
      spectralWindowId_p (spectralWindowId),
      timeStamp_p (time)
    {
        // Count up the number of frequencies selected

        const ChannelSubslicer & frequencySlicer = slicer_p.getSubslicer (1);

        nFrequencies_p = 0;

        for (Int i = 0; i < (int) frequencySlicer.nelements(); i ++){

            nFrequencies_p += frequencySlicer.getSlice(i).length();
        }

        // Create the slicer for FlagCategory data which can't use the normal slicer.

        createFlagCategorySlicer ();
    }

    Bool
    equals (const ChannelSelector & other) const
    {
        // To be equal the other ChannelSelector must be for the same
        // MS and spectral window

        Bool equal = msId_p == other.msId_p &&
                     spectralWindowId_p == other.spectralWindowId_p;

        // If the timestamps match, then they're equal

        if (timeStamp_p == other.timeStamp_p){
            return true;
        }

        // They differed on timestamps, but if they select the same channels
        // then they're equivalent.

        equal = equal && slicer_p == other.slicer_p;

        return equal;
    }

    void
    createFlagCategorySlicer ()
    {
        // The shape of the flag categories column cell is
        // [nC, nF, nCategories] whereas every other sliced
        // column cell is [nC, nF].  Use the normal slicer to
        // create the flag category slicer.

        slicerFlagCategories_p = slicer_p;
        slicerFlagCategories_p.appendDimension ();

    }

    // Returns a list of channels to be extracted.

    Vector<Int>
    getChannels () const
    {

        Vector<Int> frequencies (nFrequencies_p); // create result of appropriate size

        const ChannelSubslicer & channelSlices = slicer_p.getSubslicer (1); // get channel axis of slicer

        // Iterator over all of the slices contained in the channel portion of the slicer.
        // For each slice, use each channel number to fill in the appropriate index
        // of the result.  The result will be that frequencies [x] will contain the
        // actual channel number that resulted from the frequency selection process.

        int k = 0;

        for (int i = 0; i < (int) channelSlices.nelements (); i ++){

            const Slice & slice = channelSlices.getSlice (i);
            Int channel = slice.start();
            Int increment = slice.inc();
            Int nChannels = slice.length();

            assert (k + nChannels - 1 <= nFrequencies_p);

            for (int j = 0; j < (int) nChannels; j ++){

                frequencies [k ++] = channel;
                channel += increment;
            }

        }

        return frequencies;
    }

    Vector<Int>
    getCorrelations () const {

        const ChannelSubslicer & correlationAxis = slicer_p.getSubslicer (0);

        vector<Int> correlations;

        for (uInt i = 0; i < correlationAxis.nelements(); i ++){

            const Slice & slice = correlationAxis.getSlice (i);

            for (uInt j = 0;
                 j < slice.length();
                 j += slice.inc()){

                correlations.push_back (j);

            }
        }

        Vector<Int> result (correlations.size());

        for (uInt i = 0; i < correlations.size(); i++){
            result [i] = correlations [i];
        }

        return result;
    }

    Int
    getNFrequencies () const
    {
        return nFrequencies_p;
    }

    // Returns the ChannelSlicer object which contains the actual channelselection
    // for the current time, window and MS.

    const ChannelSlicer &
    getSlicer () const
    {
        return slicer_p;
    }

    const ChannelSlicer &
    getSlicerForFlagCategories () const
    {
        return slicerFlagCategories_p;
    }

private:

    Int msId_p;
    Int nFrequencies_p;
    ChannelSlicer slicer_p;
    ChannelSlicer slicerFlagCategories_p;
    Int spectralWindowId_p;
    Double timeStamp_p;
};

class ChannelSelectorCache {
public:

    ChannelSelectorCache (Int maxEntries = 20) : maxEntries_p (maxEntries) {}

    ~ChannelSelectorCache (){
        flush ();
    }

    void add (const ChannelSelector * entry, Double time, Int msId,
              Int frameOfReference, Int spectralWindowId)
    {
        if (msId != msId_p || frameOfReference != frameOfReference_p){

            // Cache only holds values for a single MS and a single
            // frame of reference at a time.

            flush ();

            msId_p = msId;
            frameOfReference_p = frameOfReference;
        }

        if (cache_p.size() >= maxEntries_p){

            // Boot the first entry out of the cache.  Should have the
            // lowest timestamp and lowest spectral window id.

            delete (cache_p.begin ()->second);
            cache_p.erase (cache_p.begin());
        }

        if (frameOfReference_p == FrequencySelection::ByChannel){
            time = -1; // channel selection is not function of time
        }
        cache_p [Key (time, spectralWindowId)] = entry; // take ownership of it
    }

    const ChannelSelector *
    find (Double time, Int msId, Int frameOfReference,
          Int spectralWindowId) const{

        const ChannelSelector * result = 0;

        if (msId == msId_p && frameOfReference == frameOfReference_p){

            if (frameOfReference_p == FrequencySelection::ByChannel){
                time = -1; // channel selection is not function of time
            }

            Cache::const_iterator i = cache_p.find (Key (time, spectralWindowId));

            if (i != cache_p.end()){
                result = i->second;
            }
        }

        return result;
    }

    void
    flush (){
        for (Cache::iterator i = cache_p.begin(); i != cache_p.end(); i ++){
            delete (i->second);
        }

        cache_p.clear();
    }

private:

    typedef pair<Double, Int> Key; // (time, spectralWindowId)
    typedef map <Key, const ChannelSelector *> Cache;

    Cache cache_p;          // the cache iteself
    Int frameOfReference_p; // cache's frame of reference
    const uInt maxEntries_p;   // max # of entries to keep in the cache
    Int msId_p;             // cache's MS ID

};

class SpectralWindowChannel {

public:

    SpectralWindowChannel () // for use of vector only
    : channel_p (-1),
      frequency_p (-1),
      width_p (-1)
    {}

    SpectralWindowChannel (Int channel, Double frequency, Double width)
    : channel_p (channel),
      frequency_p (frequency),
      width_p (width)
    {}

    Bool
    operator< (const SpectralWindowChannel other) const
    {
        return frequency_p < other.frequency_p;
    }

    Bool
    operator< (Double other) const
    {
        return frequency_p < other;
    }

    Int
    getChannel () const
    {
        return channel_p;
    }

    Double
    getFrequency () const
    {
        return frequency_p;
    }

    Double
    getWidth () const
    {
        return width_p;
    }

private:

    Int channel_p;
    Double frequency_p;
    Double width_p;
};

Bool
operator< (Double frequency, const SpectralWindowChannel & swc){
    return frequency < swc.getFrequency ();
}

Bool
operator> (Double frequency, const SpectralWindowChannel & swc){
    return frequency > swc.getFrequency ();
}

class SpectralWindowChannels : public std::vector <SpectralWindowChannel>{
public:

    SpectralWindowChannels (size_t size)
    : std::vector <SpectralWindowChannel> (size)
    {}

private:

};

class SpectralWindowChannelsCache {

public:

    SpectralWindowChannelsCache ()
    : msId_p (-1),
      nBytes_p (0)
    {}

    ~SpectralWindowChannelsCache ()
    {
        flush ();
    }

    void
    add (const SpectralWindowChannels * entry, Int msId, Int spectralWindowId)
    {
        if (msId != msId_p){
            flush ();
            nBytes_p = 0;
        }

        // If necessary, insert code here to put an upper limit on the size
        // of the cache.

        // Add the entry to the cache

        map_p [spectralWindowId] = entry;
        msId_p = msId;
        nBytes_p += entry->size() * sizeof (SpectralWindowChannel);
    }

    const SpectralWindowChannels *
    find (Int msId, Int spectralWindowId)
    {
        const SpectralWindowChannels * result = 0;

        if (msId == msId_p){

            Map::const_iterator i = map_p.find (spectralWindowId);

            if (i != map_p.end()){
                result = i->second;
            }
        }

        return result;
    }

private:

    typedef map<Int, const SpectralWindowChannels *> Map;
        // spectralWindowId->SpectralWindowChannels *

    Map map_p;
    Int msId_p;
    Int nBytes_p;

    void flush ()
    {
        // Delete all of the contained SpectralWindowChannels objects

        for (Map::iterator i = map_p.begin(); i != map_p.end(); i++){
            delete (i->second);
        }

        map_p.clear();
    }

};

Subchunk
Subchunk::noMoreData ()
{
    Int maxInt = std::numeric_limits<Int>::max ();
    return Subchunk (maxInt, maxInt);
}

String
Subchunk::toString () const
{
    return String::format ("(%d,%d)", first, second);
}

template <typename T>
void
VisibilityIteratorImpl2::getColumnRows (const ROArrayColumn<T> & column,
                                            Array<T> & array) const
{
    const ChannelSlicer & slicer = channelSelector_p->getSlicer();
    column.getSliceForRows (rowBounds_p.subchunkRows_p,
                            slicer.getSlicerInCoreRep(),
                            array);
}

template <typename T>
void
VisibilityIteratorImpl2::getColumnRowsMatrix (const ROArrayColumn<T> & column,
                                                  Matrix<T> & array) const
{
    column.getColumnCells (rowBounds_p.subchunkRows_p, array, True);
}



template <typename T>
void
VisibilityIteratorImpl2::getColumnRows (const ROScalarColumn<T> & column,
                                            Vector<T> & array) const
{
    column.getColumnCells (rowBounds_p.subchunkRows_p, array, True);
}

template <typename T>
void
VisibilityIteratorImpl2::putColumnRows (ArrayColumn<T> & column, const Array<T> & array)
{
    RefRows & rows = rowBounds_p.subchunkRows_p;

    const ChannelSlicer & slicer = channelSelector_p->getSlicer();
    column.putSliceFromRows (rows,
                             slicer.getSlicerInCoreRep(),
                             array);
}

template <typename T>
void
VisibilityIteratorImpl2::putColumnRows (ArrayColumn<T> & column, const Matrix<T> & array)
{
    RefRows & rows = rowBounds_p.subchunkRows_p;

    column.putColumnCells (rows, array);
}

template <typename T>
void
VisibilityIteratorImpl2::putColumnRows (ScalarColumn<T> & column, const Vector <T> & array)
{
    RefRows & rows = rowBounds_p.subchunkRows_p;

    column.putColumnCells(rows, array);
}

//VisibilityIteratorImpl2::VisibilityIteratorImpl2 ()
//: channelSelector_p (0),
//  frequencySelections_p (0),
//  frequencySelectionsPending_p (0),
//  reportingFrame_p (VisBuffer2::FrameNotSpecified),
//  vi_p (NULL),
//  spectralWindowChannelsCache_p (0),
//  tileCacheIsSet_p (0)
//{}

VisibilityIteratorImpl2::VisibilityIteratorImpl2 (VisibilityIterator2 * vi,
                                                  const Block<const MeasurementSet *> &mss,
                                                  const SortColumns & sortColumns,
                                                  Double timeInterval,
                                                  VisBufferType vbType,
                                                  Bool writable)
: ViImplementation2 (),
  channelSelector_p (0),
  channelSelectorCache_p (new ChannelSelectorCache ()),
  columns_p (),
  floatDataFound_p (False),
  frequencySelections_p (0),
  more_p (False),
  msIndex_p (0),
  msIterAtOrigin_p (False),
  msIter_p (0),
  nPolarizations_p (-1),
  nRowBlocking_p (0),
  reportingFrame_p (VisBuffer2::FrameNotSpecified),
  sortColumns_p (sortColumns),
  spectralWindowChannelsCache_p (new SpectralWindowChannelsCache ()),
  subtableColumns_p (0),
  tileCacheIsSet_p (0),
  timeInterval_p (timeInterval),
  vb_p (0),
  vi_p (0),
  weightScaling_p (0),
  writable_p (writable)
{
    // Make sure the pointer to the containing ROVI (vi_p) is NULL when calling initialize
    // otherwise the call back to the VI can result in it trying to use an uninitialized pointer
    // to this object (since it is in the process or being constructed).

    initialize (mss);

    vi_p = vi;

    VisBufferOptions options = isWritable () ? VbWritable : VbNoOptions;

    vb_p = VisBuffer2::factory (vi, vbType, options);
}

void
VisibilityIteratorImpl2::addDataSelection (const MeasurementSet & ms)
{
    const MeasurementSet2 * ms2 = dynamic_cast <const MeasurementSet2 *> (& ms);

    if (ms2 == 0){

        // A normal MS was used so there is no frequency selection attached to
        // it.  Simply add in an empty selection which will select everything.

        frequencySelections_p->add (FrequencySelectionUsingChannels ());

        return;

    }

    // Get the channel and correlation selectin.
    //
    // Channel slices are indexed by spectral window ID and correlation slices
    // by polarization ID

    MSSelection * msSelection = const_cast<MSSelection *> (ms2->getMSSelection());
        // *KLUGE* -- MSSelection is somewhat sloppy about making methods const
        // so simply getting the slices requires a non-const object ;-(

    Vector <Vector <Slice> > channelSlices;
    msSelection->getChanSlices(channelSlices, ms2);
    Vector <Vector <Slice> > correlationSlices;
    msSelection->getCorrSlices(correlationSlices, ms2);

    FrequencySelectionUsingChannels selection;

    for (uInt spectralWindow = 0;
         spectralWindow < channelSlices.nelements();
         spectralWindow ++){

        // Get the frequency slices for this spectral window

        Vector<Slice> & slices = channelSlices [spectralWindow];

        for (uInt s = 0; s < slices.nelements(); s ++){

            // Add each frequency slice to the selection for this spectral window

            Slice & slice = slices [s];

            selection.add (spectralWindow, slice.start(), slice.length(), slice.inc());
        }
    }

    selection.addCorrelationSlices (correlationSlices);

    frequencySelections_p->add (selection);
}


void
VisibilityIteratorImpl2::initialize (const Block<const MeasurementSet *> &mss)
{
    cache_p.flush();

    msIndex_p = 0;

    frequencySelections_p = new FrequencySelections ();

    Int nMs = mss.nelements ();
    measurementSets_p.resize (nMs);

    for (Int k = 0; k < nMs; ++k) {

        measurementSets_p [k] = * mss [k];

        addDataSelection (measurementSets_p [k]);
    }

   msIter_p = new MSIter (measurementSets_p,
                          sortColumns_p.getColumnIds(),
                          timeInterval_p,
                          sortColumns_p.shouldAddDefaultColumns());

   subtableColumns_p = new SubtableColumns (msIter_p);


    // Install default frequency selections.  This will select all
    // channels in all windows.

}

VisibilityIteratorImpl2::~VisibilityIteratorImpl2 ()
{
    delete channelSelectorCache_p;
    delete frequencySelections_p;
    delete spectralWindowChannelsCache_p;
    delete subtableColumns_p;
    delete vb_p;
}

VisibilityIteratorImpl2::Cache::Cache()
:
  azel0Time_p (-1),
  azelTime_p (-1),
  feedpaTime_p (-1),
  hourang_p (0),
  hourangTime_p (-1),
  msHasFlagCategory_p(False),
  msHasWeightSpectrum_p (False),
  parang0_p (0),
  parang0Time_p (-1),
  parangTime_p (-1)
{}

void
VisibilityIteratorImpl2::Cache::flush ()
{
    azel0Time_p = -1;
    azelTime_p = -1;
    feedpaTime_p = -1;
    hourangTime_p = -1;
    parangTime_p = -1;
    parang0Time_p = -1;
}

const Cube<RigidVector<Double, 2> > &
VisibilityIteratorImpl2::getBeamOffsets () const
{
    return msIter_p->getBeamOffsets ();
}

Vector<Double>
VisibilityIteratorImpl2::getFrequencies (Double time, Int frameOfReference) const
{

    // Get the channel information object (basically contains info from the
    // spectral window subtable).


    return getFrequencies (time, frameOfReference, this->spectralWindow(), this->msId());

}

Vector<Double>
VisibilityIteratorImpl2::getFrequencies (Double time, Int frameOfReference,
                                         Int spectralWindowId, Int msId) const
{
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
VisibilityIteratorImpl2::getChannels (Double /*time*/, Int /*frameOfReference*/) const
{
    assert (channelSelector_p != 0);

    return channelSelector_p->getChannels ();
}

Vector<Int>
VisibilityIteratorImpl2::getCorrelations () const
{
    assert (channelSelector_p != 0);

    return channelSelector_p->getCorrelations ();
}


Double
VisibilityIteratorImpl2::getInterval () const
{
    return timeInterval_p;
}

Bool
VisibilityIteratorImpl2::isNewArrayId () const
{
    return msIter_p->newArray();
}

Bool
VisibilityIteratorImpl2::isNewFieldId () const
{
    return msIter_p->newField ();
}

Bool
VisibilityIteratorImpl2::isNewMs () const
{
    return msIter_p->newMS ();
}

Bool
VisibilityIteratorImpl2::isNewSpectralWindow () const
{
    return msIter_p->newSpectralWindow ();
}

Bool
VisibilityIteratorImpl2::allBeamOffsetsZero () const
{
    return msIter_p->allBeamOffsetsZero ();
}

Int
VisibilityIteratorImpl2::nRows () const
{
    return rowBounds_p.subchunkNRows_p;
}

Int VisibilityIteratorImpl2::nRowsInChunk () const
{
    return msIter_p->table ().nrow ();
}

Bool
VisibilityIteratorImpl2::more () const
{
    return more_p;
}

Bool
VisibilityIteratorImpl2::moreChunks () const
{
    return msIter_p->more ();
}

const ROMSColumns *
VisibilityIteratorImpl2::msColumnsKluge () const
{
    return & msIter_p->msColumns();
}

Int
VisibilityIteratorImpl2::msId () const
{
    return msIter_p->msId ();
}

const MeasurementSet &
VisibilityIteratorImpl2::ms () const
{
    return msIter_p->ms ();
}

void
VisibilityIteratorImpl2::fieldIds (Vector<Int> & fieldIds) const
{
    getColumnRows (columns_p.field_p, fieldIds);
}

// Return the current ArrayId

void
VisibilityIteratorImpl2::arrayIds (Vector<Int> & arrayIds) const
{
    getColumnRows (columns_p.array_p, arrayIds);
}

// Return the current Field Name
String
VisibilityIteratorImpl2::fieldName () const
{
    return msIter_p->fieldName ();
}

// Return the current Source Name
String
VisibilityIteratorImpl2::sourceName () const
{
    return msIter_p->sourceName ();
}

const Vector<String> &
VisibilityIteratorImpl2::antennaMounts () const
{
    return msIter_p->antennaMounts ();
}

void
VisibilityIteratorImpl2::setInterval (Double timeInterval)
{
    pendingChanges_p.setInterval (timeInterval);
}

void
VisibilityIteratorImpl2::setRowBlocking (Int nRow)
{
    pendingChanges_p.setNRowBlocking (nRow);
}

const MDirection &
VisibilityIteratorImpl2::phaseCenter () const
{
        return msIter_p->phaseCenter ();
}

Int
VisibilityIteratorImpl2::polFrame () const
{
        return msIter_p->polFrame ();
}

Int
VisibilityIteratorImpl2::spectralWindow () const
{
    return msIter_p->spectralWindowId ();
}

void
VisibilityIteratorImpl2::spectralWindows (Vector<Int> & spws) const
{
    // Get's the list of spectral windows for each row in the VB window

    Vector<Int> ddis;
    dataDescriptionIds(ddis);
    spws.resize(ddis.size());

    for (uInt idx=0;idx<ddis.size();idx++)
    {
        spws(idx) = subtableColumns_p->dataDescription().spectralWindowId()(ddis(idx));
    }

    return;
}

// Return current Polarization Id
Int
VisibilityIteratorImpl2::polarizationId () const
{
    return msIter_p->polarizationId ();
}

// Return current DataDescription Id
Int
VisibilityIteratorImpl2::dataDescriptionId () const
{
    return msIter_p->dataDescriptionId ();
}

void
VisibilityIteratorImpl2::dataDescriptionIds (Vector<Int> & ddis) const
{
	getColumnRows (columns_p.dataDescription_p, ddis);
}

Bool
VisibilityIteratorImpl2::newFieldId () const
{
    return (rowBounds_p.subchunkBegin_p == 0 && msIter_p->newField ());
}

Bool
VisibilityIteratorImpl2::newArrayId () const
{
    return (rowBounds_p.subchunkBegin_p == 0 && msIter_p->newArray ());
}

Bool
VisibilityIteratorImpl2::newSpectralWindow () const
{
    return (rowBounds_p.subchunkBegin_p == 0 && msIter_p->newSpectralWindow ());
}

Bool
VisibilityIteratorImpl2::existsColumn (VisBufferComponent2 id) const
{
    Bool result;
    switch (id){

    case VisibilityCorrected:
    case VisibilityCubeCorrected:

        result = ! columns_p.corrVis_p.isNull() && columns_p.corrVis_p.isDefined(0);
        break;

    case VisibilityModel:
    case VisibilityCubeModel:

        result = ! columns_p.modelVis_p.isNull() && columns_p.modelVis_p.isDefined(0);
        break;

    case VisibilityObserved:
    case VisibilityCubeObserved:

        result = (! columns_p.vis_p.isNull() && columns_p.vis_p.isDefined(0)) ||
        		(columns_p.floatVis_p.isNull() && columns_p.floatVis_p.isNull());

        break;

    case WeightSpectrum:

        result = ! columns_p.weightSpectrum_p.isNull() && columns_p.weightSpectrum_p.isDefined(0);
        break;

    case WeightSpectrumCorrected:

    	// jagonzal (CAS-5265): Sometimes WEIGHT_SPECTRUM exists but it is not defined
        result = ! columns_p.weightSpectrumCorrected_p.isNull() && columns_p.weightSpectrumCorrected_p.isDefined(0);
        break;

    default:
        result = True; // required columns
        break;
    }

    return result;
}

const SubtableColumns &
VisibilityIteratorImpl2::subtableColumns () const
{
    return * subtableColumns_p;
}

void
VisibilityIteratorImpl2::allSpectralWindowsSelected (Vector<Int> & selectedWindows,
                                                     Vector<Int> & nChannels) const
{

  Vector<Int> firstChannels; // ignored
  Vector<Int> channelIncrement; // ignored

  boost::tie (selectedWindows, nChannels, firstChannels, channelIncrement) =
      getChannelInformation (False); // info generation should not use time as input
}

void
VisibilityIteratorImpl2::useImagingWeight (const VisImagingWeight & imWgt)
{
    imwgt_p = imWgt;
}

void
VisibilityIteratorImpl2::origin ()
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
VisibilityIteratorImpl2::originChunks ()
{
    originChunks (False);
}

void
VisibilityIteratorImpl2::applyPendingChanges ()
{
    if (! pendingChanges_p.empty()){

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

            msIter_p->setInterval(newInterval);
            timeInterval_p = newInterval;
        }

        // Handle any row-blocking change

        Int newBlocking;
        boost::tie (exists, newBlocking) = pendingChanges_p.popNRowBlocking ();

        if (exists){

            nRowBlocking_p = newBlocking;

        }

        msIterAtOrigin_p = False; // force rewind since window selections may have changed
    }
}

void
VisibilityIteratorImpl2::originChunks (Bool forceRewind)
{
    subchunk_p.resetToOrigin();

    applyPendingChanges ();

    if (! msIterAtOrigin_p || forceRewind) {

        msIter_p->origin ();
        msIterAtOrigin_p = True;

        positionMsIterToASelectedSpectralWindow ();

        msIndex_p = msId ();
    }

    configureNewChunk ();

    setTileCache ();
}

void
VisibilityIteratorImpl2::positionMsIterToASelectedSpectralWindow ()
{
    while (msIter_p->more() &&
           ! frequencySelections_p->isSpectralWindowSelected (msIter_p->msId(),
                                                              msIter_p->spectralWindowId())){

        (* msIter_p) ++;
    }
}

void
VisibilityIteratorImpl2::next ()
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

Subchunk
VisibilityIteratorImpl2::getSubchunkId () const
{
    return subchunk_p;
}

const SortColumns &
VisibilityIteratorImpl2::getSortColumns() const
{
  return sortColumns_p;
}

VisibilityIterator2 *
VisibilityIteratorImpl2::getViP () const
{
    return vi_p;
}

void
VisibilityIteratorImpl2::throwIfPendingChanges ()
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
VisibilityIteratorImpl2::isInASelectedSpectralWindow () const
{
    return frequencySelections_p->isSpectralWindowSelected(msIter_p->msId(),
                                                           msIter_p->spectralWindowId ());
}

Bool
VisibilityIteratorImpl2::isWritable () const
{
    return writable_p;
}

void
VisibilityIteratorImpl2::nextChunk ()
{
    ThrowIf (! msIter_p->more (),
             "Attempt to advance past end of data using nextChunk");

    throwIfPendingChanges (); // error if unapplied changes exist

    // Advance the MS Iterator until either there's no
    // more data or it points to a selected spectral window.

    (* msIter_p) ++;

    positionMsIterToASelectedSpectralWindow ();

    msIterAtOrigin_p = False;

    // If the MS Iterator was successfully advanced then
    // set up for a new chunk

    if (msIter_p->more ()) {

        subchunk_p.incrementChunk();

        configureNewChunk ();

        vb_p->invalidate (); // flush the cache ??????????
    }

    more_p = msIter_p->more ();
}

void
VisibilityIteratorImpl2::configureNewSubchunk ()
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

    nPolarizations_p = subtableColumns_p->polarization ().numCorr ()(msIter_p->polarizationId ());

    String msName = ms().tableName ();
    vb_p->configureNewSubchunk (msId (), msName, isNewMs (), isNewArrayId (), isNewFieldId (),
                                isNewSpectralWindow (), subchunk_p, rowBounds_p.subchunkNRows_p,
                                channelSelector_p->getNFrequencies(), nPolarizations_p,
                                channelSelector_p->getCorrelations(),
                                weightScaling_p);

}

const ChannelSelector *
VisibilityIteratorImpl2::determineChannelSelection (Double time, Int spectralWindowId) const
{
// --> The channels selected will need to be identical for all members of the
//     subchunk; otherwise the underlying fetch method won't work since it
//     takes a single Vector<Vector <Slice> > to specify the channels.

    assert (frequencySelections_p != 0);

    if (spectralWindowId < 0){
        spectralWindowId = spectralWindow ();
    }
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
                                            spectralWindowId, polarizationId());
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
VisibilityIteratorImpl2::makeChannelSelectorC (const FrequencySelection & selectionIn,
                                               Double time,
                                               Int msId,
                                               Int spectralWindowId,
                                               Int polarizationId) const
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

    ChannelSlicer slices (2);

    // Install the polarization selections

    slices.setSubslicer (0, ChannelSubslicer (selection.getCorrelationSlices (polarizationId)));

    // Create and install the frequency selections

    ChannelSubslicer frequencyAxis (frequencySlices.size());

    for (Int i = 0; i < (int) frequencySlices.size(); i++){
        frequencyAxis.setSlice (i, frequencySlices [i]);
    }

    slices.setSubslicer (1, frequencyAxis);

    // Package up the result and return it.

    ChannelSelector * result = new ChannelSelector (time, msId, spectralWindowId, slices);

    return result;
}

ChannelSelector *
VisibilityIteratorImpl2::makeChannelSelectorF (const FrequencySelection & selectionIn,
                                               Double time, Int msId, Int spectralWindowId) const
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

    ChannelSlicer slices (2);
    ChannelSubslicer frequencyAxis (frequencySlices.size());

    for (Int i = 0; i < (int) frequencySlices.size(); i++){
        frequencyAxis.setSlice (i, frequencySlices [i]);
    }

    slices.setSubslicer (1, frequencyAxis);

    // Package up result and return it.

    ChannelSelector * result = new ChannelSelector (time, msId, spectralWindowId, slices);

    return result;
}

MFrequency::Convert
VisibilityIteratorImpl2::makeFrequencyConverter (Double time, Int otherFrameOfReference,
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
VisibilityIteratorImpl2::findChannelsInRange (Double lowerFrequency, Double upperFrequency,
                                              const vi::SpectralWindowChannels & spectralWindowChannels) const
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

Int
VisibilityIteratorImpl2::getNMs () const
{
    return measurementSets_p.nelements();
}


MFrequency::Types
VisibilityIteratorImpl2::getObservatoryFrequencyType () const
{
    const MFrequency & f0 = msIter_p->frequency0();

    MFrequency::Types t = MFrequency::castType (f0.type());

    return t;
}

MPosition
VisibilityIteratorImpl2::getObservatoryPosition () const
{
    return msIter_p->telescopePosition();
}

const SpectralWindowChannels &
VisibilityIteratorImpl2::getSpectralWindowChannels (Int msId, Int spectralWindowId) const
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
VisibilityIteratorImpl2::configureNewChunk ()
{
    rowBounds_p.chunkNRows_p = msIter_p->table ().nrow ();
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

    if (msIter_p->newMS ()) {

        // Flush some cache flag values

        cache_p.flush ();

        msd_p.setAntennas (msIter_p->msColumns ().antenna ());
    }

    if (msIter_p->newField () || msIterAtOrigin_p) {
        msd_p.setFieldCenter (msIter_p->phaseCenter ());
    }
}

const MSDerivedValues &
VisibilityIteratorImpl2::getMsd () const
{
    return msd_p;
}

void
VisibilityIteratorImpl2::setTileCache ()
{
    // This function sets the tile cache because of a feature in
    // sliced data access that grows memory dramatically in some cases
    //  if (useSlicer_p){

    if (! (msIter_p->newDataDescriptionId () || msIter_p->newMS ()) ) {
        return;
    }

    const MeasurementSet & theMs = msIter_p->ms ();
    if (theMs.tableType () == Table::Memory) {
        return;
    }

    const ColumnDescSet & cds = theMs.tableDesc ().columnDescSet ();

    uInt startrow = msIter_p->table ().rowNumbers ()(0); // Get the first row number for this DDID.

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

Bool
VisibilityIteratorImpl2::usesTiledDataManager (const String & columnName,
                                                  const MeasurementSet & theMs) const
{
    Bool noData = False;

    // Have to do something special about weight_spectrum as it tend to exist but
    // has no valid data.

    noData = noData ||
             (columnName == MS::columnName (MS::WEIGHT_SPECTRUM) && ! weightSpectrumExists ());

    noData = noData ||
             (columnName == "CORRECTED_WEIGHT_SPECTRUM" && ! weightSpectrumCorrectedExists ());

    // Check to see if the column exist and have valid data

    noData = noData ||
             (columnName == MS::columnName (MS::DATA) &&
              (columns_p.vis_p.isNull () || ! columns_p.vis_p.isDefined (0)));

    noData = noData ||
             (columnName == MS::columnName (MS::MODEL_DATA) &&
              (columns_p.modelVis_p.isNull () || ! columns_p.modelVis_p.isDefined (0)));

    noData = noData ||
             (columnName == MS::columnName (MS::CORRECTED_DATA) &&
              (columns_p.corrVis_p.isNull () || ! columns_p.corrVis_p.isDefined (0)));

    noData = noData ||
             (columnName == MS::columnName (MS::FLAG) &&
              (columns_p.flag_p.isNull () || ! columns_p.flag_p.isDefined (0)));

    noData = noData ||
             (columnName == MS::columnName (MS::WEIGHT) &&
              (columns_p.weight_p.isNull () || ! columns_p.weight_p.isDefined (0)));

    noData = noData ||
             (columnName == MS::columnName (MS::SIGMA) &&
              (columns_p.sigma_p.isNull () || ! columns_p.sigma_p.isDefined (0)));

    noData = noData ||
             (columnName == MS::columnName (MS::UVW) &&
              (columns_p.uvw_p.isNull () || ! columns_p.uvw_p.isDefined (0)));

    Bool usesTiles = False;

    if (! noData){
        String dataManType = RODataManAccessor (theMs, columnName, True).dataManagerType ();

        usesTiles = dataManType.contains ("Tiled");
    }

    return usesTiles;
}

//void
//VisibilityIteratorImpl2::attachColumnsSafe (const Table & t)
//{
//    // Normally, the call to attachColumns is redirected back to the ROVI class.
//    // This allows writable VIs to attach columns in both the read and write impls.
//    // However, this referral to the ROVI doesn't work during construction of this
//    // class (VIRI) since there is as yet no pointer to the object under construction.
//    // In that case, simply perform it locally.
//
//    if (vi_p == NULL){
//        attachColumns (t);
//    }
//    else{
//        vi_p->attachColumns (t);
//    }
//}

void
VisibilityIteratorImpl2::attachColumns (const Table & t)
{
    columns_p.attachColumns (t);

    floatDataFound_p = columns_p.isFloatDataPresent();
}

MEpoch
VisibilityIteratorImpl2::getEpoch () const
{
    MEpoch mEpoch = msIter_p->msColumns().timeMeas ()(0);

    return mEpoch;
}

Vector<Float>
VisibilityIteratorImpl2::getReceptor0Angle ()
{
    Int nAntennas = this->nAntennas ();

    Vector<Float> receptor0Angle (nAntennas);

    for (int i = 0; i < nAntennas; i++) {
        receptor0Angle [i] = msIter_p->receptorAngle ()(0, i);
    }

    return receptor0Angle;
}

void
VisibilityIteratorImpl2::getRowIds (Vector<uInt> & rowIds) const
{
    // Resize the rowIds vector and fill it with the row numbers contained
    // in the current subchunk.  These row numbers are relative to the reference
    // table used by MSIter to define a chunk.

    rowIds.resize (rowBounds_p.subchunkNRows_p);
    rowIds = rowBounds_p.subchunkRows_p.convert ();

    if (cache_p.chunkRowIds_p.nelements() == 0){

        // Create chunkRowIds_p as a "map" from chunk rows to MS rows.  This
        // needs to be created once per chunk since a new reference table is
        // created each time the MSIter moves to the next chunk.

        cache_p.chunkRowIds_p = msIter_p->table ().rowNumbers (msIter_p->ms ());

    }

    // Using chunkRowIds_p as a map from chunk rows to MS rows replace the
    // chunk-relative row numbers with the actual row number from the MS.

    for (uInt i = 0; i < rowIds.nelements (); i++) {

        rowIds (i) = cache_p.chunkRowIds_p (rowIds (i));
    }
}

void
VisibilityIteratorImpl2::antenna1(Vector<Int> & ant1) const
{
    getColumnRows (columns_p.antenna1_p, ant1);
}

void
VisibilityIteratorImpl2::antenna2(Vector<Int> & ant2) const
{
    getColumnRows (columns_p.antenna2_p, ant2);
}

void
VisibilityIteratorImpl2::feed1(Vector<Int> & fd1) const
{
    getColumnRows (columns_p.feed1_p, fd1);
}

void
VisibilityIteratorImpl2::feed2(Vector<Int> & fd2) const
{
    getColumnRows (columns_p.feed2_p, fd2);
}

void
VisibilityIteratorImpl2::corrType (Vector<Int> & corrTypes) const
{
    Int polId = msIter_p->polarizationId ();

    subtableColumns_p->polarization ().corrType ().get (polId, corrTypes, True);
}

void
VisibilityIteratorImpl2::flag (Cube<Bool> & flags) const
{
    getColumnRows (columns_p.flag_p, flags);
}

void
VisibilityIteratorImpl2::flag (Matrix<Bool> & flags) const
{
    Cube<Bool> flagCube;

    flag (flagCube);

    getColumnRows (columns_p.flag_p, flagCube);

    uInt nChannels = flagCube.shape()(1);

    flags.resize (nChannels, rowBounds_p.subchunkNRows_p);

    for (Int row = 0; row < rowBounds_p.subchunkNRows_p; row++) {

        for (uInt channel = 0; channel < nChannels; channel ++) {

            Bool flagIt = flagCube (0, channel, row);

            for (Int correlation = 1;
                 correlation < nPolarizations_p && not flagIt;
                 correlation ++){

                flagIt = flagCube (correlation, channel, row);
            }

            flags (channel, row) = flagIt;
        }
    }
}

Bool
VisibilityIteratorImpl2::flagCategoryExists() const
{
  if(msIter_p->newMS()){ // Cache to avoid testing unnecessarily.
      cache_p.msHasFlagCategory_p = columns_p.flagCategory_p.hasContent();
  }
  return cache_p.msHasFlagCategory_p;
}

void
VisibilityIteratorImpl2::flagCategory (Array<Bool> & flagCategories) const
{
    if (columns_p.flagCategory_p.isNull () ||
        ! columns_p.flagCategory_p.isDefined (0)) { // It often is.

        flagCategories.resize ();    // Zap it.
    }
    else {

        // Since flag category is shaped [nC, nF, nCategories] it requires a
        // slightly different slicer and cannot use the usual getColumns method.

        const ChannelSlicer & channelSlicer = channelSelector_p->getSlicerForFlagCategories();

        columns_p.flagCategory_p.getSliceForRows (rowBounds_p.subchunkRows_p,
                                                  channelSlicer.getSlicerInCoreRep(),
                                                  flagCategories);
    }
}

void
VisibilityIteratorImpl2::flagRow (Vector<Bool> & rowflags) const
{
    getColumnRows (columns_p.flagRow_p, rowflags);
}

void
VisibilityIteratorImpl2::observationId (Vector<Int> & obsIDs) const
{
    getColumnRows (columns_p.observation_p, obsIDs);
}

void
VisibilityIteratorImpl2::processorId (Vector<Int> & procIDs) const
{
    getColumnRows (columns_p.processor_p, procIDs);
}

void
VisibilityIteratorImpl2::scan (Vector<Int> & scans) const
{
    getColumnRows (columns_p.scan_p, scans);
}

void
VisibilityIteratorImpl2::stateId (Vector<Int> & stateIds) const
{
    getColumnRows (columns_p.state_p, stateIds);
}

void
VisibilityIteratorImpl2::time (Vector<Double> & t) const
{
    getColumnRows (columns_p.time_p, t);
}

void
VisibilityIteratorImpl2::timeCentroid (Vector<Double> & t) const
{
    getColumnRows (columns_p.timeCentroid_p, t);
}

void
VisibilityIteratorImpl2::timeInterval (Vector<Double> & t) const
{
    getColumnRows (columns_p.timeInterval_p, t);
}

void
VisibilityIteratorImpl2::exposure (Vector<Double> & expo) const
{
    getColumnRows (columns_p.exposure_p, expo);
}

void
VisibilityIteratorImpl2::visibilityCorrected (Cube<Complex> & vis) const
{
    getColumnRows (columns_p.corrVis_p, vis);
}

void
VisibilityIteratorImpl2::visibilityModel (Cube<Complex> & vis) const
{
    getColumnRows (columns_p.modelVis_p, vis);
}

void
VisibilityIteratorImpl2::visibilityObserved (Cube<Complex> & vis) const
{
    if (floatDataFound_p) {

        // Since there is a floating data column, read that and convert it
        // into the expected Complex form.

        Cube<Float> dataFloat;

        getColumnRows (columns_p.floatVis_p, dataFloat);

        vis.resize (dataFloat.shape());

        convertArray (vis, dataFloat);
    }
    else {
        getColumnRows (columns_p.vis_p, vis);
    }
}

void
VisibilityIteratorImpl2::floatData (Cube<Float> & fcube) const
{
    if (floatDataFound_p) {
        getColumnRows (columns_p.floatVis_p, fcube);
    }
    else{
        fcube.resize();
    }
}

void
VisibilityIteratorImpl2::uvw (Matrix<Double> & uvwmat) const
{
    getColumnRowsMatrix (columns_p.uvw_p, uvwmat);
}

// Fill in parallactic angle.
const Vector<Float> &
VisibilityIteratorImpl2::feed_pa (Double time) const
{
    //  LogMessage message (LogOrigin ("VisibilityIteratorImpl2","feed_pa"));

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
VisibilityIteratorImpl2::parang0(Double time) const
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
VisibilityIteratorImpl2::parang (Double time) const
{
    if (time != cache_p.parangTime_p) {

        cache_p.parangTime_p = time;

        // Set up the Epoch using the absolute MJD in seconds
        // get the Epoch reference from the column

        MEpoch mEpoch = getEpoch();
        Int nAnt = msIter_p->receptorAngle ().shape ()(1);

        cache_p.parang_p = parangCalculate (time, msd_p, nAnt, mEpoch);

    }
    return cache_p.parang_p;
}


// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
const Vector<MDirection> &
VisibilityIteratorImpl2::azel (Double ut) const
{
    if (ut != cache_p.azelTime_p) {

        cache_p.azelTime_p = ut;

        Int nAnt = msIter_p->receptorAngle ().shape ()(1);

        MEpoch mEpoch = getEpoch();

        azelCalculate (ut, msd_p, cache_p.azel_p, nAnt, mEpoch);

    }
    return cache_p.azel_p;
}


// Fill in azimuth/elevation of the antennas.
// Cloned from feed_pa, we need to check that this is all correct!
MDirection
VisibilityIteratorImpl2::azel0(Double time) const
{
    //  LogMessage message (LogOrigin ("VisibilityIteratorImpl2","azel0"));

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
VisibilityIteratorImpl2::hourang (Double time) const
{
    //  LogMessage message (LogOrigin ("VisibilityIteratorImpl2","azel"));

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
VisibilityIteratorImpl2::sigma (Matrix<Float> & sigma) const
{
    getColumnRowsMatrix (columns_p.sigma_p, sigma);
}

void
VisibilityIteratorImpl2::weight (Matrix<Float> & wt) const
{
    getColumnRowsMatrix (columns_p.weight_p, wt);
}

Bool
VisibilityIteratorImpl2::weightSpectrumExists () const
{
    if (msIter_p->newMS ()) { // Cache to avoid testing unnecessarily.

        cache_p.msHasWeightSpectrum_p = columns_p.weightSpectrum_p.hasContent ();

    }

    return cache_p.msHasWeightSpectrum_p;
}

Bool
VisibilityIteratorImpl2::weightSpectrumCorrectedExists () const
{
    if (msIter_p->newMS ()) { // Cache to avoid testing unnecessarily.

        cache_p.msHasWeightSpectrumCorrected_p = columns_p.weightSpectrumCorrected_p.hasContent ();

    }

    return cache_p.msHasWeightSpectrumCorrected_p;
}

void
VisibilityIteratorImpl2::weightSpectrum (Cube<Float> & spectrum) const
{
    if (weightSpectrumExists ()) {

        getColumnRows (columns_p.weightSpectrum_p, spectrum);

    }
    else {
        spectrum.resize (0, 0, 0);
    }
}

void
VisibilityIteratorImpl2::weightSpectrumCorrected (Cube<Float> & spectrum) const
{
    if (weightSpectrumCorrectedExists ()) {

        getColumnRows (columns_p.weightSpectrumCorrected_p, spectrum);

    }
    else {
        spectrum.resize (0, 0, 0);
    }
}

void
VisibilityIteratorImpl2::setWeightScaling (CountedPtr<WeightScaling> weightScaling)
{
    weightScaling_p = weightScaling;
}

Bool
VisibilityIteratorImpl2::hasWeightScaling () const
{
    return ! weightScaling_p.null();
}

CountedPtr<WeightScaling>
VisibilityIteratorImpl2::getWeightScaling () const
{
    return weightScaling_p;
}


const VisImagingWeight &
VisibilityIteratorImpl2::getImagingWeightGenerator () const
{
    return imwgt_p;
}

Block<MeasurementSet>
VisibilityIteratorImpl2::getMeasurementSets () const
{
    return measurementSets_p;
}

Int
VisibilityIteratorImpl2::getReportingFrameOfReference () const
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
VisibilityIteratorImpl2::setReportingFrameOfReference (Int frame)
{
    ThrowIf (frame < 0 || frame >= MFrequency::N_Types,
             String::format ("Unknown frame: id=%d", frame));

    reportingFrame_p = frame;
}

VisBuffer2 *
VisibilityIteratorImpl2::getVisBuffer ()
{
    return vb_p;
}

Int
VisibilityIteratorImpl2::nAntennas () const
{
    return subtableColumns_p->antenna ().nrow (); // for single (sub)array only..
}

Int
VisibilityIteratorImpl2::nSpectralWindows () const
{
    return subtableColumns_p->spectralWindow ().nrow ();
}

Int
VisibilityIteratorImpl2::nDataDescriptionIds () const
{
    return subtableColumns_p->dataDescription ().nrow ();
}

Int
VisibilityIteratorImpl2::nPolarizations () const
{
    return subtableColumns_p->polarization ().nrow ();
}

Int
VisibilityIteratorImpl2::nRowsViWillSweep () const
{
    Int numcoh = 0;
    for (uInt k = 0; k < uInt (msIter_p->numMS ()) ; ++k) {
        numcoh += msIter_p->ms (k).nrow ();
    }
    return numcoh;

}

const Table
VisibilityIteratorImpl2::attachTable () const
{
    return msIter_p->table ();
}

void
VisibilityIteratorImpl2::slurp () const
{
    // Set the table data manager (ISM and SSM) cache size to the full column size, for
    //   the columns ANTENNA1, ANTENNA2, FEED1, FEED2, TIME, INTERVAL, FLAG_ROW, SCAN_NUMBER and UVW

    Record dmInfo (msIter_p->ms ().dataManagerInfo ());

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
                    uInt num_buckets = msIter_p->ms ().nrow ();
                        // One bucket is at least one row, so this is enough

                    if (dm_type == "IncrementalStMan") {

                        ROIncrementalStManAccessor acc (msIter_p->ms (), dm_name);
                        acc.setCacheSize (num_buckets, can_exceed_nr_buckets);

                    } else if (dm_type == "StandardStMan") {

                        ROStandardStManAccessor acc (msIter_p->ms (), dm_name);
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
VisibilityIteratorImpl2::receptorAngles() const
{
    return msIter_p->receptorAngles ();
}

IPosition
VisibilityIteratorImpl2::visibilityShape() const
{

    IPosition result (3,
                      nPolarizations_p,
                      channelSelector_p->getNFrequencies(),
                      rowBounds_p.subchunkNRows_p);

    return result;
}

void
VisibilityIteratorImpl2::setFrequencySelections (FrequencySelections const& frequencySelections)
{
    pendingChanges_p.setFrequencySelections (frequencySelections.clone ());

    channelSelectorCache_p->flush();
}

void
VisibilityIteratorImpl2::jonesC(Vector<SquareMatrix<complex<float>, 2> >& cjones) const
{
    cjones.resize (msIter_p->CJones ().nelements ());
    cjones = msIter_p->CJones ();
}

//void
//VisibilityIteratorImpl2::writeFlag (const Matrix<Bool> & flag)
//{
//
//    Int nFrequencies = channelSelector_p->getNFrequencies();
//    Int nPolarizations = nPolarizations_p;
//    Int nRows = this->nRows();
//
//    // The flag matrix is expected to have dimensions [nF, nR].
//
//    ThrowIf ((int) flag.nrow() != nFrequencies || (int) flag.ncolumn() != nRows,
//             String::format ("Shape mismatch: got [%d,%d]; expected [%d,%d]",
//                             flag.nrow(), flag.ncolumn(), nFrequencies, nRows));
//
//    // Convert the flag matrix into the flag cube defined in the MS.  The flag
//    // value for a (row,channel) will be copied into all of the polarizations.
//
//    Cube<Bool> flagCube;
//
//    flagCube.resize (nPolarizations, nFrequencies, nRows);
//
//    for (Int row = 0; row < nRows; row++) {
//
//        for (Int frequency = 0; frequency < nFrequencies; frequency ++) {
//
//            Bool flagValue = flag (frequency, row); // same value for all polarizations
//
//            for (Int polarization = 0; polarization < nPolarizations; polarization ++) {
//
//                flagCube (polarization, frequency, row) = flagValue;
//            }
//        }
//    }
//
//    // Write the newly constructed flag cube into the MS.
//
//    writeFlag (flagCube);
//}

void
VisibilityIteratorImpl2::writeFlag (const Cube<Bool> & flags)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    putColumnRows (columns_p.flag_p, flags);
}

void
VisibilityIteratorImpl2::writeFlagCategory(const Array<Bool>& flagCategory)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    // Flag categories are [nC, nF, nCategories] and therefore must use a
    // different slicer which also prevents use of more usual putColumn method.

    RefRows & rows = rowBounds_p.subchunkRows_p;
    const ChannelSlicer & channelSlicer = channelSelector_p->getSlicerForFlagCategories();

    columns_p.flagCategory_p.putSliceFromRows (rows,
                                               channelSlicer.getSlicerInCoreRep(),
                                               flagCategory);
}

void
VisibilityIteratorImpl2::writeFlagRow (const Vector<Bool> & rowflags)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    putColumnRows (columns_p.flagRow_p, rowflags);
}

void
VisibilityIteratorImpl2::writeVisCorrected (const Cube<Complex> & vis)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    putColumnRows (columns_p.corrVis_p, vis);
}

void
VisibilityIteratorImpl2::writeVisModel (const Cube<Complex> & vis)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    putColumnRows (columns_p.modelVis_p, vis);
}

void
VisibilityIteratorImpl2::writeVisObserved (const Cube<Complex> & vis)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

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
VisibilityIteratorImpl2::writeWeight (const Matrix<Float> & weight)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    putColumnRows (columns_p.weight_p, weight);
}

void
VisibilityIteratorImpl2::writeWeightSpectrum (const Cube<Float> & weightSpectrum)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    if (! columns_p.weightSpectrum_p.isNull ()) {
        putColumnRows (columns_p.weightSpectrum_p, weightSpectrum);
    }
}

void
VisibilityIteratorImpl2::writeWeightSpectrumCorrected (const Cube<Float> & weightSpectrumCorrected)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    if (! columns_p.weightSpectrumCorrected_p.isNull ()) {
        putColumnRows (columns_p.weightSpectrumCorrected_p, weightSpectrumCorrected);
    }
}

void
VisibilityIteratorImpl2::writeSigma (const Matrix<Float> & sigma)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

    putColumnRows (columns_p.sigma_p, sigma);
}

void
VisibilityIteratorImpl2::writeModel(const RecordInterface& rec, Bool iscomponentlist, Bool incremental)
{

  ThrowIf (! isWritable (), "This visibility iterator is not writable");

  Vector<Int> fields = columns_p.field_p.getColumn();

  const Int option = Sort::HeapSort | Sort::NoDuplicates;
  const Sort::Order order = Sort::Ascending;

  Int nFields = GenSort<Int>::sort (fields, order, option);

  // Make sure  we have the right size

  fields.resize(nFields, True);

  Vector<Int> selectedWindows;
  Vector<Int> nChannels;
  Vector<Int> firstChannels;
  Vector<Int> channelIncrement;

  boost::tie (selectedWindows, nChannels, firstChannels, channelIncrement) =
      getChannelInformation (True);

  VisModelData::putModel(ms(), rec, fields, selectedWindows, firstChannels, nChannels,
                         channelIncrement, iscomponentlist, incremental);

}

VisibilityIteratorImpl2::ChannelInfo
VisibilityIteratorImpl2::getChannelInformationUsingFrequency (Bool now) const
{
    const FrequencySelection & frequencySelection = frequencySelections_p->get (msId());
    set<Int> windows = frequencySelection.getSelectedWindows();

    Vector<Int> spectralWindow (windows.size());
    Vector<Int> nChannels (windows.size());
    Vector<Int> firstChannel (windows.size());
    Vector<Int> channelIncrement (windows.size());

    if (now){

        // Select the channels in use at the provided time.

        Vector<Double> t;
        time(t); // put the current time vector into t

        AssertOrWarn (abs(mean(t) - t(0)) <= 1.0, "Time not relatively constant in VisBuffer.");
            // time needs to be relatively constant over the VisBuffer for this
            // approach to be valid.

        Int nElements = 0;

        for (set<Int>::const_iterator window = windows.begin();
             window != windows.end(); window ++){

            // Create a channel selector for this window at the buffer's time.

            const ChannelSelector * selector = determineChannelSelection(t (0), * window);
            const ChannelSlicer channelSlicer = selector->getSlicer();

            for (Int i = 0; i < (int) channelSlicer.nelements(); i++){

                const ChannelSubslicer subslicer = channelSlicer.getSubslicer(i);

                const Slice & slice = subslicer.getSlice (ChannelSubslicer::Channel);

                spectralWindow (nElements) = * window;
                nChannels (nElements) = slice.length();
                firstChannel (nElements) = slice.start();
                channelIncrement (nElements) = slice.inc();

                nElements ++;
            }
        }
    }
    else{

        // Since the frequency selection was not specified using channels, return the
        // entire number of channels in the selected spectral windows.  This might be overkill
        // but it should not cause false results to the caller.

        casa::ms::SpectralWindows spectralWindows (& measurementSets_p [msId()]);

        Int i = 0;
        for (set<Int>::iterator j = windows.begin(); j != windows.end(); j++){

            spectralWindow [i] = * j;
            nChannels [i] = spectralWindows.get (* j).nChannels();
            firstChannel [i] = 0;
            channelIncrement = 0;
        }
    }

    return boost::make_tuple (spectralWindow, nChannels, firstChannel, channelIncrement);
}


VisibilityIteratorImpl2::ChannelInfo
VisibilityIteratorImpl2::getChannelInformation (Bool now) const
{
    const FrequencySelectionUsingChannels * frequencySelection =
        dynamic_cast <const FrequencySelectionUsingChannels *> (& frequencySelections_p->get (msId()));

    if (frequencySelection == 0){

        return getChannelInformationUsingFrequency (now);

    }

    Vector<Int> spectralWindow;
    Vector<Int> nChannels;
    Vector<Int> firstChannel;
    Vector<Int> channelIncrement;

    if (frequencySelection->empty()){

        // No explicit selection, so everything is selected.

        casa::ms::SpectralWindows spectralWindows (& measurementSets_p [msId()]);

        spectralWindow.resize (spectralWindows.size());
        nChannels.resize (spectralWindows.size());
        firstChannel.resize (spectralWindows.size());
        channelIncrement.resize (spectralWindows.size());

        Int i = 0;

        for (casa::ms::SpectralWindows::const_iterator s = spectralWindows.begin();
             s != spectralWindows.end();
             s ++){

            spectralWindow (i) = s->id ();
            nChannels (i) = s->nChannels ();
            firstChannel (i) = 0;
            channelIncrement (i) = 1;

            i++;
        }
    }
    else {

        // Use the explicit channel-based selection to compute the result.


        spectralWindow.resize (frequencySelection->size());
        nChannels.resize (frequencySelection->size());
        firstChannel.resize (frequencySelection->size());
        channelIncrement.resize (frequencySelection->size());

        Int i = 0;
        for (FrequencySelectionUsingChannels::const_iterator j = frequencySelection->begin();
             j != frequencySelection->end();
             ++ j){

            spectralWindow (i) = j->spectralWindow_p;
            nChannels (i) = j->nChannels_p;
            firstChannel (i) = j->firstChannel_p;
            channelIncrement (i) = j->increment_p;

            i ++;
        }
    }

    return boost::make_tuple (spectralWindow, nChannels, firstChannel, channelIncrement);
}

void
VisibilityIteratorImpl2::writeBackChanges (VisBuffer2 * vb)
{
    ThrowIf (! isWritable (), "This visibility iterator is not writable");

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
VisibilityIteratorImpl2::initializeBackWriters ()
{
//    backWriters_p [Flag] =
//        makeBackWriter (& VisibilityIteratorImpl2::writeFlag, & VisBuffer2::flag);
    backWriters_p [FlagCube] =
        makeBackWriter (& VisibilityIteratorImpl2::writeFlag, & VisBuffer2::flagCube);
    backWriters_p [FlagRow] =
        makeBackWriter (& VisibilityIteratorImpl2::writeFlagRow, & VisBuffer2::flagRow);
    backWriters_p [FlagCategory] =
        makeBackWriter (& VisibilityIteratorImpl2::writeFlagCategory, & VisBuffer2::flagCategory);
    backWriters_p [Sigma] =
        makeBackWriter (& VisibilityIteratorImpl2::writeSigma, & VisBuffer2::sigma);
//    backWriters_p [SigmaMat] =
//        makeBackWriter (& VisibilityIteratorImpl2::writeSigmaMat, & VisBuffer2::sigmaMat);
    backWriters_p [Weight] =
        makeBackWriter (& VisibilityIteratorImpl2::writeWeight, & VisBuffer2::weight);
//    backWriters_p [WeightMat] =
//        makeBackWriter (& VisibilityIteratorImpl2::writeWeightMat, & VisBuffer2::weightMat);
    backWriters_p [WeightSpectrum] =
        makeBackWriter (& VisibilityIteratorImpl2::writeWeightSpectrum, & VisBuffer2::weightSpectrum);

    // Now do the visibilities.

    backWriters_p [VisibilityCubeObserved] =
        makeBackWriter (& VisibilityIteratorImpl2::writeVisObserved, & VisBuffer2::visCube);
    backWriters_p [VisibilityCubeCorrected] =
        makeBackWriter (& VisibilityIteratorImpl2::writeVisCorrected, & VisBuffer2::visCubeCorrected);
    backWriters_p [VisibilityCubeModel] =
        makeBackWriter (& VisibilityIteratorImpl2::writeVisModel, & VisBuffer2::visCubeModel);

}

VisibilityIteratorImpl2::PendingChanges::PendingChanges ()
: frequencySelections_p (0),
  frequencySelectionsPending_p (False),
  interval_p (Empty),
  nRowBlocking_p (Empty)
{}

VisibilityIteratorImpl2::PendingChanges::~PendingChanges ()
{
    delete frequencySelections_p;
}

VisibilityIteratorImpl2::PendingChanges *
VisibilityIteratorImpl2::PendingChanges::clone () const
{
    PendingChanges * theClone = new PendingChanges ();

    theClone->frequencySelections_p = new FrequencySelections (* frequencySelections_p);
    theClone->frequencySelectionsPending_p = frequencySelectionsPending_p;
    theClone->interval_p = interval_p;
    theClone->nRowBlocking_p = nRowBlocking_p;

    return theClone;
}



Bool
VisibilityIteratorImpl2::PendingChanges::empty () const
{
    Bool result = frequencySelections_p == 0 &&
                  interval_p == Empty &&
                  nRowBlocking_p == Empty;

    return result;
}

pair<Bool, FrequencySelections *>
VisibilityIteratorImpl2::PendingChanges::popFrequencySelections () // yields ownershi
{
    FrequencySelections * result = frequencySelections_p;
    Bool wasPending = frequencySelectionsPending_p;

    frequencySelections_p = 0;
    frequencySelectionsPending_p = False;

    return make_pair (wasPending, result);
}

pair<Bool, Double>
VisibilityIteratorImpl2::PendingChanges::popInterval ()
{
    pair<Bool,Double> result = make_pair (interval_p != Empty, interval_p);

    interval_p = Empty;

    return result;
}

pair<Bool, Int>
VisibilityIteratorImpl2::PendingChanges::popNRowBlocking ()
{
    pair<Bool,Int> result = make_pair (nRowBlocking_p != Empty, nRowBlocking_p);

    nRowBlocking_p = Empty;

    return result;
}

void
VisibilityIteratorImpl2::PendingChanges::setFrequencySelections (FrequencySelections * fs) // takes ownershi
{
    Assert (! frequencySelectionsPending_p);

    frequencySelections_p = fs;
    frequencySelectionsPending_p = True;
}

void
VisibilityIteratorImpl2::PendingChanges::setInterval (Double interval)
{
    Assert (interval_p == Empty);

    interval_p = interval;
}

void
VisibilityIteratorImpl2::PendingChanges::setNRowBlocking (Int nRowBlocking)
{
    Assert (nRowBlocking_p == Empty);

    nRowBlocking_p = nRowBlocking;
}

} // end namespace vi

} //# NAMESPACE CASA - END


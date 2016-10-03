//# VisBuffer.h: buffer for iterating through casacore::MS in large blocks
//# Copyright (C) 1996,1997,1998,1999,2000,2002,2003
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#ifndef MSVIS_VISBUFFERIMPL_H
#define MSVIS_VISBUFFERIMPL_H

#include <casa/aips.h>

#warning "Remove next line later"
#    include <msvis/MSVis/VisBuffer.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <msvis/MSVis/VisBufferComponents.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace asyncio {
    class VLAT;
} // end namespace asyncio

class VbDirtyComponents;
class ROVisibilityIterator2;
class VisibilityIterator2;

namespace vi {

//#forward


// <summary>
// VbDirtyComponents allows marking portions of a VisBufferImpl as
// modified (aka dirty).  This feature is needed for the Visibility
// Processing Framework (VPF) which allows a sequence of data processing
// nodes to work as a bucket brigade operating sequentially on a
// VisBufferImpl.  A downstream output node needs to know what data,
// if any, needs to be written out.
//
// <prerequisite>
//   #<li><linkto class="VisBufferImpl">VisBufferImpl</linkto>
// </prerequisite>
//
// </summary>
//
// <synopsis>
//
// </synopsis>
// <example>
//
// <code>
//
// </code>
// </example>
//

#if 0 // Can't avoid the definition in VisBuffer for now

class VbDirtyComponents {

public:

    typedef std::set<VisBufferComponents::EnumType> Set;
    typedef Set::const_iterator const_iterator;

    VbDirtyComponents operator+ (const VbDirtyComponents & other) const;

    const_iterator begin () const;
    casacore::Bool contains (VisBufferComponents::EnumType component) const;
    const_iterator end () const;

    static VbDirtyComponents all ();
    static VbDirtyComponents exceptThese (VisBufferComponents::EnumType component, ...);
    static VbDirtyComponents none ();
    static VbDirtyComponents singleton (VisBufferComponents::EnumType component);
    static VbDirtyComponents these (VisBufferComponents::EnumType component, ...);

protected:

private:

    Set set_p;

    static const VbDirtyComponents all_p;

    static VbDirtyComponents initializeAll ();

};

#endif // commenting out


class VbCacheItemBase;
class VisBufferCache;
class VisBufferState;

using vi::VbCacheItemBase;
using vi::VisBufferCache;
using vi::VisBufferState;

//<summary>VisBufferImpls encapsulate one chunk of visibility data for processing.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisSet">VisSet</linkto>
//   <li> <linkto class="VisibilityIterator2">VisibilityIterator2</linkto>
//   <li> <linkto class="VisBufferImplAutoPtr">VisBufferImplAutoPtr</linkto>
//   <li> <linkto class="VbDirtyComponents">VbDirtyComponents</linkto>
// </prerequisite>
//
// <etymology>
// VisBufferImpl is a buffer for visibility data
// </etymology>
//
//<synopsis>
// This class contains 'one iteration' of the
// <linkto class="VisibilityIterator2">VisibilityIterator2</linkto>
// It is a modifiable
// buffer of values to which calibration and averaging can be applied.
// This allows processing of the data in larger blocks, avoiding some
// overheads for processing per visibility point or spectrum.
//
// See <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// for more details on how the VisBufferImpl is to be used.
//
// When the user intends to allow asynchronous I/O they will need to use
// the VisBufferImplAsync and VisBufferImplAsyncWrapper classes; these are
// publicly derived from VisBufferImpl.  Normally, the user should not
// explicitly use these classes but should use the helper class
// VisBufferImplAutoPtr which will ensure that the appropriate class is
// used.
//</synopsis>

//<todo>
// <li> reconcile vis/visCube usage: visCube, flagCube and weightMatrix
// are currently only correct when this VisBufferImpl got them from a
// VisIter, operations like -=, freqAverage() are only done for
// visibility() and flag().
//</todo>

class VisBufferImpl {

    friend class casa::asyncio::VLAT; // for async i/o
    friend class VbCacheItemBase;
    friend class VisBufferCache;
    friend class VisBufferState;
    friend class VisBufferImplAsync; // for async i/o
    friend class VisBufferImplAsyncWrapper; // for async i/o
    friend class ViReadImpl;
    friend class ViReadImplAsync; // for async I/O

public:
    // Create empty VisBufferImpl you can assign to or attach.
    VisBufferImpl();
    // Construct VisBufferImpl for a particular VisibilityIterator2
    // The buffer will remain synchronized with the iterator.
    VisBufferImpl(ROVisibilityIterator2 & iter);

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).
    VisBufferImpl(const VisBufferImpl & vb);

    // Destructor (detaches from VisIter)
    virtual ~VisBufferImpl();

    // Assignment, loses synchronization with iterator: only use buffer for
    // current iteration (or reattach)

    virtual VisBufferImpl & operator=(const VisBufferImpl & vb);

    // Assignment, optionally without copying the data across; with copy=true
    // this is identical to normal assignment operator

    virtual void assign(const VisBufferImpl & vb, casacore::Bool copy = true);

    virtual VisBufferImpl * clone () const;

    virtual const ROVisibilityIterator2 * getVi () const;

    virtual void invalidate();

    // subtraction: return the difference of the visibilities, flags of
    // this and other are or-ed. An exception is thrown if the number of
    // rows or channels differs, but no further checks are done.
    //-->Not called: VisBufferImpl & operator-=(const VisBufferImpl & vb);

    // Invalidate the cache

    // <group>
    // Access functions
    //
    //     return nCatOK_p ? nCat_p : fillnCat();
    // virtual void setNCat (const casacore::Int &))) {
    // }
    // virtual casacore::Int nCat() const {
    //     return This->nCat();
    // }

    ///Because of it being publicly exposed ...using nRow, nChannel, nCorr etc to
    /// determine the size of the buffer can be totally WRONG
    ///They MAY NOT  represent the shape
    /// of a buffer filled in this iteration.
    // Decide what is the right value (depending on where the vb is coming from)
    // for you for the size of the buffer. i.e (nCorr(), nChannel(), nRow()) or vb.visCube().shape()
    // The latter comes from the VisIter state ...the former be careful...
    /// For example VisBuffAccumulator changes these in an unconnected fashion;
    //without updating fillnrow
    /// datacube shape  etc etc.
    /// You are warned nrow_p etc are public variables effectively (despite being
    ///declared private) so it can be changed
    /// anywhere the code uses a vb, intentionally or accidentally.

    ////--> shouldn't be settable: virtual void setNRow (const casacore::Int &);

    /////--> provided by VI: virtual casacore::Int nRowChunk() const;

    virtual void dirtyComponentsAdd (const VbDirtyComponents & additionalDirtyComponents);
    virtual void dirtyComponentsAdd (VisBufferComponents::EnumType component);
    virtual void dirtyComponentsClear ();
    virtual VbDirtyComponents dirtyComponentsGet () const;
    virtual void dirtyComponentsSet (const VbDirtyComponents & dirtyComponents);
    virtual void dirtyComponentsSet (VisBufferComponents::EnumType component);

    //--> This needs to be removed: virtual casacore::Bool fetch(const asyncio::PrefetchColumns *pfc);

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBufferImpl only to benefit from caching of the feed and antenna IDs.

    // Note that feed_pa is a function instead of a cached value
    virtual casacore::Vector<casacore::Float> feed_pa(casacore::Double time) const;

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.


    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual casacore::Float parang0(casacore::Double time) const;
    virtual casacore::Vector<casacore::Float> parang(casacore::Double time) const;

    virtual casacore::MDirection azel0(casacore::Double time) const; // function rather than cached value
    virtual void azel0Vec(casacore::Double time, casacore::Vector<casacore::Double>& azelVec) const;
    virtual casacore::Vector<casacore::MDirection> azel(casacore::Double time) const;
    virtual void azelMat(casacore::Double time, casacore::Matrix<casacore::Double>& azelMat) const;

    // Hour angle for specified time
    virtual casacore::Double hourang(casacore::Double time) const;

//    virtual const casacore::Vector<casacore::Double>& lsrFrequency() const;
//    virtual void setLsrFrequency (const casacore::Vector<casacore::Double>&);


    //the following method is to convert the observed frequencies
    // This conversion may not be accurate for some frame
    // conversion like topo to lsr except if the spw is in the actual buffer

    //if ignoreconv=true..frequency is served as is in the data frame
    //virtual void lsrFrequency(const casacore::Int & spw, casacore::Vector<casacore::Double>& freq, casacore::Bool & convert, const casacore::Bool ignoreconv = false) const;

//    virtual const casacore::Matrix<casacore::Float>& imagingWeight() const;
//    virtual void setImagingWeight (const casacore::Matrix<casacore::Float>&);

    //</group>

    //<group>
    // Utility functions to provide coordinate or column ranges of the
    // data in the VisBufferImpl. Flagging is applied before computing the ranges.
    //
    // Generic accessor to column ranges of integer type, as specified by
    // enumerations defined in class MSCalEnums. Throws an exception
    // if the enum is not for a recognized integer column.
    //--> Low utility?: virtual casacore::Vector<casacore::Int> vecIntRange(const MSCalEnums::colDef & calEnum) const;

    // Antenna id. range (includes both ANTENNA1 and ANTENNA2 columns)
    //--> Low utility?: virtual casacore::Vector<casacore::Int> antIdRange() const;

    // casacore::Time range

    //--> Low utility?: virtual casacore::Bool timeRange(casacore::MEpoch & rTime, casacore::MVEpoch & rTimeEP, casacore::MVEpoch & rInterval) const;

    // Return the row Ids from the original ms. If the ms used is a subset of
    // another ms then rowIds() return the row ids of the original ms.

    //--> Belongs in VI: virtual void setRowIds (const casacore::Vector<casacore::uInt>&);
    //--> Belongs in VI: virtual const casacore::Vector<casacore::uInt>& rowIds() const;

    //</group>

    // Frequency average the buffer (visibility() column only)
    //--> Not called: virtual void freqAverage();

    // Frequency average the buffer (visCube and [if present] modelVisCube)
    void freqAveCubes();

    // Average channel axis according to chanavebounds, for whichever of DATA,
    // MODEL_DATA, CORRECTED_DATA, FLOAT_DATA, FLAG, and WEIGHT_SPECTRUM are
    // present.  It will only treat the first 5 as present if they have already
    // been loaded into the buffer!
    void averageChannels (const casacore::Matrix<casacore::Int>& chanavebounds);


    // Form casacore::Stokes parameters from correlations
    //  (these are preliminary versions)
    //--> Not called: virtual void formStokes();
    //--> Not called: virtual void formStokesWeightandFlag();
    //--> Not called: virtual void formStokes(casacore::Cube<casacore::Complex>& vis);
    //--> Not called: virtual void formStokes(casacore::Cube<casacore::Float>& fcube);    // Will throw up if asked to do all 4.

    // casacore::Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    virtual void sortCorr ();
    virtual void unSortCorr();

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    virtual void normalize(const casacore::Bool & phaseOnly = false);

    // Fill weightMat according to sigma column
    virtual void resetWeightsUsingSigma ();//virtual void resetWeightMat();

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)

    //--> Called by routine in FixVis but that routine is not called:
    // virtual void phaseCenterShift(const casacore::Vector<casacore::Double>& phase);
    // Rotate visibility phase for phase center offsets (arcsecs)
    //--> Not called: virtual void phaseCenterShift(casacore::Double dx, casacore::Double dy);

    // Update coordinate info - useful for copied VisBufferImpls that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows.
    // if dirDependent is set to false the expensive direction dependent calculation of parallactic or direction of
    // antenna from pointing table is avoided
    //Add more as needed.
    virtual void updateCoordInfo(const VisBufferImpl * vb = NULL, const casacore::Bool dirDependent = true);
    void copyCoordInfo(const VisBufferImpl & other, casacore::Bool force = false);

    //--> Not called: virtual void setCorrectedVisCube(const casacore::Cube<casacore::Complex>& vis);
    // Like the above, but for FLOAT_DATA, keeping it as real floats.

    //--> Not called: virtual void setFloatDataCube(const casacore::Cube<casacore::Float>& fcube);
    // Reference external model visibilities
    //--> Not called: virtual void refModelVis(const casacore::Matrix<CStokesVector>& mvis);

    // Remove scratch cols data from vb
    //--> Not called: virtual void removeScratchCols();

    // Access the current casacore::ROMSColumns object via VisIter
    //--> Access using VI: virtual const casacore::ROMSColumns & msColumns() const;

    //--> Access via VI: virtual  casacore::Int numberAnt () const;



    // Get all selected spectral windows not just the one in the actual buffer
    //--> Get from VI: virtual void allSelectedSpectralWindows(casacore::Vector<casacore::Int>& spws, casacore::Vector<casacore::Int>& nvischan);

    //--> Get from VI: virtual void getChannelSelection(casacore::Block< casacore::Vector<casacore::Int> >& blockNGroup,
    //				   casacore::Block< casacore::Vector<casacore::Int> >& blockStart,
    //			   casacore::Block< casacore::Vector<casacore::Int> >& blockWidth,
    //				   casacore::Block< casacore::Vector<casacore::Int> >& blockIncr,
    //				   casacore::Block< casacore::Vector<casacore::Int> >& blockSpw) const;;
    //--> Get from VI: void allSelectedSpectralWindows(casacore::Vector<casacore::Int>& spws, casacore::Vector<casacore::Int>& nvischan) const;

    virtual casacore::Bool isNewArrayId () const;
    virtual casacore::Bool isNewFieldId () const;
    virtual casacore::Bool isNewMs() const;
    virtual casacore::Bool isNewSpectralWindow () const;
    virtual casacore::Bool isWritable () const;
    virtual casacore::Int msId() const;
    virtual casacore::String msName (casacore::Bool stripPath = false) const;

    //////////////////////////////////////////////////////////////////////
    //
    //  casacore::Data accessors and setters (where appropriate)
    //
    //  There are the methods that allows access to the items cached in the
    //  VisBuffer.  The straight accessors provide read-only access to the
    //  item.  Where the item is allowed to be modified, one or more set
    //  methods are provided.

    virtual casacore::Vector<casacore::Int> antenna1 () const;
    virtual const casacore::Vector<casacore::Int> & antenna2 () const;
    virtual casacore::Int arrayId () const;
    virtual const casacore::Vector<casacore::Int> & channel () const;
    virtual const casacore::Vector<casacore::SquareMatrix<casacore::Complex, 2> > & cjones () const;
    virtual const casacore::Cube<casacore::Complex> & correctedVisCube () const;
    virtual void setCorrectedVisCube (const casacore::Cube<casacore::Complex> &);
    virtual const casacore::Matrix<CStokesVector> & correctedVisibility () const;
    virtual void setCorrectedVisibility (const casacore::Matrix<CStokesVector> &);
    virtual const casacore::Vector<casacore::Int> & corrType () const;
    virtual casacore::Int dataDescriptionId () const;
    virtual const casacore::Vector<casacore::MDirection> & direction1 () const;
    virtual const casacore::Vector<casacore::MDirection> & direction2 () const;
    virtual const casacore::Vector<casacore::Double> & exposure () const;
    virtual const casacore::Vector<casacore::Int> & feed1 () const;
    virtual const casacore::Vector<casacore::Float> & feed1_pa () const;
    virtual const casacore::Vector<casacore::Int> & feed2 () const;
    virtual const casacore::Vector<casacore::Float> & feed2_pa () const;
    virtual casacore::Int fieldId () const;
    virtual const casacore::Matrix<casacore::Bool> & flag () const;
    virtual void setFlag (const casacore::Matrix<casacore::Bool>&);
    virtual const casacore::Array<casacore::Bool> & flagCategory () const;
    virtual void setFlagCategory (const casacore::Array<casacore::Bool>&);
    virtual const casacore::Cube<casacore::Bool> & flagCube () const;
    virtual void setFlagCube (const casacore::Cube<casacore::Bool>&);
    virtual const casacore::Vector<casacore::Bool> & flagRow () const;
    virtual void setFlagRow (const casacore::Vector<casacore::Bool>&);
    virtual const casacore::Cube<casacore::Float> & floatDataCube () const;
    virtual void setFloatDataCube (const casacore::Cube<casacore::Float> &);
    virtual const casacore::Vector<casacore::Double> & frequency () const;
    virtual const casacore::Matrix<casacore::Float> & imagingWeight () const;
    virtual const casacore::Cube<casacore::Complex> & modelVisCube () const;
    virtual void setModelVisCube(const casacore::Complex & c);
    virtual void setModelVisCube(const casacore::Cube<casacore::Complex>& vis);
    virtual void setModelVisCube(const casacore::Vector<casacore::Float>& stokes);
    virtual const casacore::Matrix<CStokesVector> & modelVisibility () const;
    virtual void setModelVisibility (casacore::Matrix<CStokesVector> &);
    virtual casacore::Int nChannel () const;
    virtual casacore::Int nCorr () const;
    virtual casacore::Int nRow () const;
    virtual const casacore::Vector<casacore::Int> & observationId () const;
    virtual const casacore::MDirection& phaseCenter () const;
    virtual casacore::Int polFrame () const;
    virtual const casacore::Vector<casacore::Int> & processorId () const;
    virtual const casacore::Vector<casacore::uInt> & rowIds () const;
    virtual const casacore::Vector<casacore::Int> & scan () const;
    virtual const casacore::Vector<casacore::Float> & sigma () const;
    virtual const casacore::Matrix<casacore::Float> & sigmaMat () const;
    virtual casacore::Int spectralWindow () const;
    virtual const casacore::Vector<casacore::Int> & stateId () const;
    virtual const casacore::Vector<casacore::Double> & time () const;
    virtual const casacore::Vector<casacore::Double> & timeCentroid () const;
    virtual const casacore::Vector<casacore::Double> & timeInterval () const;
    virtual const casacore::Vector<casacore::RigidVector<casacore::Double, 3> > & uvw () const;
    virtual const casacore::Matrix<casacore::Double> & uvwMat () const;
    virtual const casacore::Cube<casacore::Complex> & visCube () const;
    virtual void setVisCube(const casacore::Complex & c);
    virtual void setVisCube (const casacore::Cube<casacore::Complex> &);
    virtual const casacore::Matrix<CStokesVector> & visibility () const;
    virtual void setVisibility (casacore::Matrix<CStokesVector> &);
    virtual const casacore::Vector<casacore::Float> & weight () const;
    virtual void setWeight (const casacore::Vector<casacore::Float>&);
    virtual const casacore::Matrix<casacore::Float> & weightMat () const;
    virtual void setWeightMat (const casacore::Matrix<casacore::Float>&);
    virtual const casacore::Cube<casacore::Float> & weightSpectrum () const;
    virtual void setWeightSpectrum (const casacore::Cube<casacore::Float>&);

protected:


    // Attach to a VisIter. Detaches itself first if already attached
    // to a VisIter. Will remain synchronized with iterator.

    virtual void attachToVisibilityIterator2 (ROVisibilityIterator2 & iter);
    virtual void cacheCopy (const VisBufferImpl & other, casacore::Bool markAsCached);
    virtual void cacheClear (casacore::Bool markAsCached = false);

    void adjustWeightFactorsAndFlags (casacore::Matrix <casacore::Float> & rowWeightFactors,
                                      casacore::Bool useWeightSpectrum,
                                      casacore::Int nRows,
                                      casacore::Int nCorrelations,
                                      casacore::Int nChannelsOut);
    void adjustWeightAndSigmaMatrices (casacore::Int nChannelsAveraged, casacore::Int nChannelsOut, casacore::Int nRows,
                                       casacore::Int nCorrelations, casacore::Int nChannelsSelected,
                                       const casacore::Matrix <casacore::Float> & rowWeightFactors);

    casacore::Int averageChannelFrequencies (casacore::Int nChannelsOut,
                                   const casacore::Vector<casacore::Int> & channels,
                                   const casacore::Matrix<casacore::Int> & averagingBounds);

    void averageFlagInfoChannels (const casacore::Matrix<casacore::Int> & averagingBounds,
                                  casacore::Int nChannelsOut, casacore::Bool useWeightSpectrum);

    template<class T>
    void averageVisCubeChannels (T & dataCache,
                                 casacore::Int nChanOut,
                                 const casacore::Matrix<casacore::Int>& averagingbounds);

    virtual void checkVisIter (const char * func, const char * file, int line, const char * extra = "") const;
    void computeRowWeightFactors (casacore::Matrix <casacore::Float> & rowWeightFactors, casacore::Bool useWeightSpectrum);
    virtual void sortCorrelationsAux (casacore::Bool makeSorted);
    virtual void detachFromVisibilityIterator2 ();
    virtual ROVisibilityIterator2 * getViP () const; // protected, non-const access to VI
    void registerCacheItem (VbCacheItemBase *);
    void setIterationInfo (casacore::Int msId, const casacore::String & msName, casacore::Bool isNewMs,
                           casacore::Bool isNewArrayId, casacore::Bool isNewFieldId, casacore::Bool isNewSpectralWindow);
    virtual void stateCopy (const VisBufferImpl & other); // copy relevant noncached members

    template <typename Coord>
    void updateCoord (Coord & item, const Coord & otherItem);

private:

    virtual casacore::Bool areCorrelationsInCanonicalOrder () const;
    void checkVisIterBase (const char * func, const char * file, int line, const char * extra = "") const;
    void construct();
    void constructCache();
    virtual void validate();

    /////////////////////////////////////////
    //
    //  Fillers
    //
    // These methods fill the needed value into the cache.  This
    // usually means getting it from the attached VisibilityIterator2
    // but the needed value can be generated as well.
    //
    // The name of the filler should match up with the accessor method
    // (see above) as well as the member name in the cache (see
    // VisBufferCache class in VisBuffer.cc).


    virtual void fillAntenna1 (casacore::Vector<casacore::Int>& value) const;
    virtual void fillAntenna2 (casacore::Vector<casacore::Int>& value) const;
    virtual void fillArrayId (casacore::Int& value) const;
    virtual void fillChannel (casacore::Vector<casacore::Int>& value) const;
    virtual void fillCorrType (casacore::Vector<casacore::Int>& value) const;
    virtual void fillCubeCorrected (casacore::Cube <casacore::Complex> & value) const;
    virtual void fillCubeModel (casacore::Cube <casacore::Complex> & value) const;
    virtual void fillCubeObserved (casacore::Cube <casacore::Complex> & value) const;
    virtual void fillDataDescriptionId  (casacore::Int& value) const;
    virtual void fillDirection1 (casacore::Vector<casacore::MDirection>& value) const;
    virtual void fillDirection2 (casacore::Vector<casacore::MDirection>& value) const;
    virtual void fillDirectionAux (casacore::Vector<casacore::MDirection>& value,
                           const casacore::Vector<casacore::Int> & antenna,
                           const casacore::Vector<casacore::Int> &feed,
                           const casacore::Vector<casacore::Float> & feedPa) const;
    virtual void fillExposure (casacore::Vector<casacore::Double>& value) const;
    virtual void fillFeed1 (casacore::Vector<casacore::Int>& value) const;
    virtual void fillFeed2 (casacore::Vector<casacore::Int>& value) const;
    virtual void fillFeedPa1 (casacore::Vector <casacore::Float> & feedPa) const;
    virtual void fillFeedPa2 (casacore::Vector <casacore::Float> & feedPa) const;
    virtual void fillFeedPaAux (casacore::Vector <casacore::Float> & feedPa,
                        const casacore::Vector <casacore::Int> & antenna,
                        const casacore::Vector <casacore::Int> & feed) const;
    virtual void fillFieldId (casacore::Int& value) const;
    virtual void fillFlag (casacore::Matrix<casacore::Bool>& value) const;
    virtual void fillFlagCategory (casacore::Array<casacore::Bool>& value) const;
    virtual void fillFlagCube (casacore::Cube<casacore::Bool>& value) const;
    virtual void fillFlagRow (casacore::Vector<casacore::Bool>& value) const;
    virtual void fillFloatData (casacore::Cube<casacore::Float>& value) const;
    virtual void fillFrequency (casacore::Vector<casacore::Double>& value) const;
    virtual void fillImagingWeight (casacore::Matrix<casacore::Float> & value) const;
    virtual void fillJonesC (casacore::Vector<casacore::SquareMatrix<casacore::Complex, 2> >& value) const;
    virtual void fillNChannel (casacore::Int& value) const;
    virtual void fillNCorr (casacore::Int& value) const;
    virtual void fillNRow (casacore::Int& value) const;
    virtual void fillObservationId (casacore::Vector<casacore::Int>& value) const;
    virtual void fillPhaseCenter (casacore::MDirection& value) const;
    virtual void fillPolFrame (casacore::Int& value) const;
    virtual void fillProcessorId (casacore::Vector<casacore::Int>& value) const;
    virtual void fillScan (casacore::Vector<casacore::Int>& value) const;
    virtual void fillSigma (casacore::Vector<casacore::Float>& value) const;
    virtual void fillSigmaMat (casacore::Matrix<casacore::Float>& value) const;
    virtual void fillSpectralWindow (casacore::Int& value) const;
    virtual void fillStateId (casacore::Vector<casacore::Int>& value) const;
    virtual void fillTime (casacore::Vector<casacore::Double>& value) const;
    virtual void fillTimeCentroid (casacore::Vector<casacore::Double>& value) const;
    virtual void fillTimeInterval (casacore::Vector<casacore::Double>& value) const;
    virtual void fillUvw (casacore::Vector<casacore::RigidVector<casacore::Double, 3> >& value) const;
    virtual void fillUvwMat (casacore::Matrix<casacore::Double>& value) const;
    virtual void fillVisibilityCorrected (casacore::Matrix<CStokesVector>& value) const;
    virtual void fillVisibilityModel (casacore::Matrix<CStokesVector>& value) const;
    virtual void fillVisibilityObserved (casacore::Matrix<CStokesVector>& value) const;
    virtual void fillWeight (casacore::Vector<casacore::Float>& value) const;
    virtual void fillWeightMat (casacore::Matrix<casacore::Float>& value) const;
    virtual void fillWeightSpectrum (casacore::Cube<casacore::Float>& value) const;

    VisBufferCache * cache_p;
    VisBufferState * state_p;
};

} // end namespace vb

} // end namespace casa


#endif


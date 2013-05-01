//# VisBuffer.h: buffer for iterating through MS in large blocks
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
#    include <synthesis/MSVis/VisBuffer.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MSVis/VisBufferComponents.h>

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
    Bool contains (VisBufferComponents::EnumType component) const;
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

    // Assignment, optionally without copying the data across; with copy=True
    // this is identical to normal assignment operator

    virtual void assign(const VisBufferImpl & vb, Bool copy = True);

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
    // virtual void setNCat (const Int &))) {
    // }
    // virtual Int nCat() const {
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

    ////--> shouldn't be settable: virtual void setNRow (const Int &);

    /////--> provided by VI: virtual Int nRowChunk() const;

    virtual void dirtyComponentsAdd (const VbDirtyComponents & additionalDirtyComponents);
    virtual void dirtyComponentsAdd (VisBufferComponents::EnumType component);
    virtual void dirtyComponentsClear ();
    virtual VbDirtyComponents dirtyComponentsGet () const;
    virtual void dirtyComponentsSet (const VbDirtyComponents & dirtyComponents);
    virtual void dirtyComponentsSet (VisBufferComponents::EnumType component);

    //--> This needs to be removed: virtual Bool fetch(const asyncio::PrefetchColumns *pfc);

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBufferImpl only to benefit from caching of the feed and antenna IDs.

    // Note that feed_pa is a function instead of a cached value
    virtual Vector<Float> feed_pa(Double time) const;

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.


    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const;
    virtual Vector<Float> parang(Double time) const;

    virtual MDirection azel0(Double time) const; // function rather than cached value
    virtual void azel0Vec(Double time, Vector<Double>& azelVec) const;
    virtual Vector<MDirection> azel(Double time) const;
    virtual void azelMat(Double time, Matrix<Double>& azelMat) const;

    // Hour angle for specified time
    virtual Double hourang(Double time) const;

//    virtual const Vector<Double>& lsrFrequency() const;
//    virtual void setLsrFrequency (const Vector<Double>&);


    //the following method is to convert the observed frequencies
    // This conversion may not be accurate for some frame
    // conversion like topo to lsr except if the spw is in the actual buffer

    //if ignoreconv=True..frequency is served as is in the data frame
    //virtual void lsrFrequency(const Int & spw, Vector<Double>& freq, Bool & convert, const Bool ignoreconv = False) const;

//    virtual const Matrix<Float>& imagingWeight() const;
//    virtual void setImagingWeight (const Matrix<Float>&);

    //</group>

    //<group>
    // Utility functions to provide coordinate or column ranges of the
    // data in the VisBufferImpl. Flagging is applied before computing the ranges.
    //
    // Generic accessor to column ranges of integer type, as specified by
    // enumerations defined in class MSCalEnums. Throws an exception
    // if the enum is not for a recognized integer column.
    //--> Low utility?: virtual Vector<Int> vecIntRange(const MSCalEnums::colDef & calEnum) const;

    // Antenna id. range (includes both ANTENNA1 and ANTENNA2 columns)
    //--> Low utility?: virtual Vector<Int> antIdRange() const;

    // Time range

    //--> Low utility?: virtual Bool timeRange(MEpoch & rTime, MVEpoch & rTimeEP, MVEpoch & rInterval) const;

    // Return the row Ids from the original ms. If the ms used is a subset of
    // another ms then rowIds() return the row ids of the original ms.

    //--> Belongs in VI: virtual void setRowIds (const Vector<uInt>&);
    //--> Belongs in VI: virtual const Vector<uInt>& rowIds() const;

    //</group>

    // Frequency average the buffer (visibility() column only)
    //--> Not called: virtual void freqAverage();

    // Frequency average the buffer (visCube and [if present] modelVisCube)
    void freqAveCubes();

    // Average channel axis according to chanavebounds, for whichever of DATA,
    // MODEL_DATA, CORRECTED_DATA, FLOAT_DATA, FLAG, and WEIGHT_SPECTRUM are
    // present.  It will only treat the first 5 as present if they have already
    // been loaded into the buffer!
    void averageChannels (const Matrix<Int>& chanavebounds);


    // Form Stokes parameters from correlations
    //  (these are preliminary versions)
    //--> Not called: virtual void formStokes();
    //--> Not called: virtual void formStokesWeightandFlag();
    //--> Not called: virtual void formStokes(Cube<Complex>& vis);
    //--> Not called: virtual void formStokes(Cube<Float>& fcube);    // Will throw up if asked to do all 4.

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    virtual void sortCorr ();
    virtual void unSortCorr();

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    virtual void normalize(const Bool & phaseOnly = False);

    // Fill weightMat according to sigma column
    virtual void resetWeightsUsingSigma ();//virtual void resetWeightMat();

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)

    //--> Called by routine in FixVis but that routine is not called:
    // virtual void phaseCenterShift(const Vector<Double>& phase);
    // Rotate visibility phase for phase center offsets (arcsecs)
    //--> Not called: virtual void phaseCenterShift(Double dx, Double dy);

    // Update coordinate info - useful for copied VisBufferImpls that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows.
    // if dirDependent is set to False the expensive direction dependent calculation of parallactic or direction of
    // antenna from pointing table is avoided
    //Add more as needed.
    virtual void updateCoordInfo(const VisBufferImpl * vb = NULL, const Bool dirDependent = True);
    void copyCoordInfo(const VisBufferImpl & other, Bool force = False);

    //--> Not called: virtual void setCorrectedVisCube(const Cube<Complex>& vis);
    // Like the above, but for FLOAT_DATA, keeping it as real floats.

    //--> Not called: virtual void setFloatDataCube(const Cube<Float>& fcube);
    // Reference external model visibilities
    //--> Not called: virtual void refModelVis(const Matrix<CStokesVector>& mvis);

    // Remove scratch cols data from vb
    //--> Not called: virtual void removeScratchCols();

    // Access the current ROMSColumns object via VisIter
    //--> Access using VI: virtual const ROMSColumns & msColumns() const;

    //--> Access via VI: virtual  Int numberAnt () const;



    // Get all selected spectral windows not just the one in the actual buffer
    //--> Get from VI: virtual void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);

    //--> Get from VI: virtual void getChannelSelection(Block< Vector<Int> >& blockNGroup,
    //				   Block< Vector<Int> >& blockStart,
    //			   Block< Vector<Int> >& blockWidth,
    //				   Block< Vector<Int> >& blockIncr,
    //				   Block< Vector<Int> >& blockSpw) const;;
    //--> Get from VI: void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan) const;

    virtual Bool isNewArrayId () const;
    virtual Bool isNewFieldId () const;
    virtual Bool isNewMs() const;
    virtual Bool isNewSpectralWindow () const;
    virtual Bool isWritable () const;
    virtual Int msId() const;
    virtual String msName (Bool stripPath = False) const;

    //////////////////////////////////////////////////////////////////////
    //
    //  Data accessors and setters (where appropriate)
    //
    //  There are the methods that allows access to the items cached in the
    //  VisBuffer.  The straight accessors provide read-only access to the
    //  item.  Where the item is allowed to be modified, one or more set
    //  methods are provided.

    virtual Vector<Int> antenna1 () const;
    virtual const Vector<Int> & antenna2 () const;
    virtual Int arrayId () const;
    virtual const Vector<Int> & channel () const;
    virtual const Vector<SquareMatrix<Complex, 2> > & cjones () const;
    virtual const Cube<Complex> & correctedVisCube () const;
    virtual void setCorrectedVisCube (const Cube<Complex> &);
    virtual const Matrix<CStokesVector> & correctedVisibility () const;
    virtual void setCorrectedVisibility (const Matrix<CStokesVector> &);
    virtual const Vector<Int> & corrType () const;
    virtual Int dataDescriptionId () const;
    virtual const Vector<MDirection> & direction1 () const;
    virtual const Vector<MDirection> & direction2 () const;
    virtual const Vector<Double> & exposure () const;
    virtual const Vector<Int> & feed1 () const;
    virtual const Vector<Float> & feed1_pa () const;
    virtual const Vector<Int> & feed2 () const;
    virtual const Vector<Float> & feed2_pa () const;
    virtual Int fieldId () const;
    virtual const Matrix<Bool> & flag () const;
    virtual void setFlag (const Matrix<Bool>&);
    virtual const Array<Bool> & flagCategory () const;
    virtual void setFlagCategory (const Array<Bool>&);
    virtual const Cube<Bool> & flagCube () const;
    virtual void setFlagCube (const Cube<Bool>&);
    virtual const Vector<Bool> & flagRow () const;
    virtual void setFlagRow (const Vector<Bool>&);
    virtual const Cube<Float> & floatDataCube () const;
    virtual void setFloatDataCube (const Cube<Float> &);
    virtual const Vector<Double> & frequency () const;
    virtual const Matrix<Float> & imagingWeight () const;
    virtual const Cube<Complex> & modelVisCube () const;
    virtual void setModelVisCube(const Complex & c);
    virtual void setModelVisCube(const Cube<Complex>& vis);
    virtual void setModelVisCube(const Vector<Float>& stokes);
    virtual const Matrix<CStokesVector> & modelVisibility () const;
    virtual void setModelVisibility (Matrix<CStokesVector> &);
    virtual Int nChannel () const;
    virtual Int nCorr () const;
    virtual Int nRow () const;
    virtual const Vector<Int> & observationId () const;
    virtual const MDirection& phaseCenter () const;
    virtual Int polFrame () const;
    virtual const Vector<Int> & processorId () const;
    virtual const Vector<uInt> & rowIds () const;
    virtual const Vector<Int> & scan () const;
    virtual const Vector<Float> & sigma () const;
    virtual const Matrix<Float> & sigmaMat () const;
    virtual Int spectralWindow () const;
    virtual const Vector<Int> & stateId () const;
    virtual const Vector<Double> & time () const;
    virtual const Vector<Double> & timeCentroid () const;
    virtual const Vector<Double> & timeInterval () const;
    virtual const Vector<RigidVector<Double, 3> > & uvw () const;
    virtual const Matrix<Double> & uvwMat () const;
    virtual const Cube<Complex> & visCube () const;
    virtual void setVisCube(const Complex & c);
    virtual void setVisCube (const Cube<Complex> &);
    virtual const Matrix<CStokesVector> & visibility () const;
    virtual void setVisibility (Matrix<CStokesVector> &);
    virtual const Vector<Float> & weight () const;
    virtual void setWeight (const Vector<Float>&);
    virtual const Matrix<Float> & weightMat () const;
    virtual void setWeightMat (const Matrix<Float>&);
    virtual const Cube<Float> & weightSpectrum () const;
    virtual void setWeightSpectrum (const Cube<Float>&);

protected:


    // Attach to a VisIter. Detaches itself first if already attached
    // to a VisIter. Will remain synchronized with iterator.

    virtual void attachToVisibilityIterator2 (ROVisibilityIterator2 & iter);
    virtual void cacheCopy (const VisBufferImpl & other, Bool markAsCached);
    virtual void cacheClear (Bool markAsCached = False);

    void adjustWeightFactorsAndFlags (Matrix <Float> & rowWeightFactors,
                                      Bool useWeightSpectrum,
                                      Int nRows,
                                      Int nCorrelations,
                                      Int nChannelsOut);
    void adjustWeightAndSigmaMatrices (Int nChannelsAveraged, Int nChannelsOut, Int nRows,
                                       Int nCorrelations, Int nChannelsSelected,
                                       const Matrix <Float> & rowWeightFactors);

    Int averageChannelFrequencies (Int nChannelsOut,
                                   const Vector<Int> & channels,
                                   const Matrix<Int> & averagingBounds);

    void averageFlagInfoChannels (const Matrix<Int> & averagingBounds,
                                  Int nChannelsOut, Bool useWeightSpectrum);

    template<class T>
    void averageVisCubeChannels (T & dataCache,
                                 Int nChanOut,
                                 const Matrix<Int>& averagingbounds);

    virtual void checkVisIter (const char * func, const char * file, int line, const char * extra = "") const;
    void computeRowWeightFactors (Matrix <Float> & rowWeightFactors, Bool useWeightSpectrum);
    virtual void sortCorrelationsAux (Bool makeSorted);
    virtual void detachFromVisibilityIterator2 ();
    virtual ROVisibilityIterator2 * getViP () const; // protected, non-const access to VI
    void registerCacheItem (VbCacheItemBase *);
    void setIterationInfo (Int msId, const String & msName, Bool isNewMs,
                           Bool isNewArrayId, Bool isNewFieldId, Bool isNewSpectralWindow);
    virtual void stateCopy (const VisBufferImpl & other); // copy relevant noncached members

    template <typename Coord>
    void updateCoord (Coord & item, const Coord & otherItem);

private:

    virtual Bool areCorrelationsInCanonicalOrder () const;
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


    virtual void fillAntenna1 (Vector<Int>& value) const;
    virtual void fillAntenna2 (Vector<Int>& value) const;
    virtual void fillArrayId (Int& value) const;
    virtual void fillChannel (Vector<Int>& value) const;
    virtual void fillCorrType (Vector<Int>& value) const;
    virtual void fillCubeCorrected (Cube <Complex> & value) const;
    virtual void fillCubeModel (Cube <Complex> & value) const;
    virtual void fillCubeObserved (Cube <Complex> & value) const;
    virtual void fillDataDescriptionId  (Int& value) const;
    virtual void fillDirection1 (Vector<MDirection>& value) const;
    virtual void fillDirection2 (Vector<MDirection>& value) const;
    virtual void fillDirectionAux (Vector<MDirection>& value,
                           const Vector<Int> & antenna,
                           const Vector<Int> &feed,
                           const Vector<Float> & feedPa) const;
    virtual void fillExposure (Vector<Double>& value) const;
    virtual void fillFeed1 (Vector<Int>& value) const;
    virtual void fillFeed2 (Vector<Int>& value) const;
    virtual void fillFeedPa1 (Vector <Float> & feedPa) const;
    virtual void fillFeedPa2 (Vector <Float> & feedPa) const;
    virtual void fillFeedPaAux (Vector <Float> & feedPa,
                        const Vector <Int> & antenna,
                        const Vector <Int> & feed) const;
    virtual void fillFieldId (Int& value) const;
    virtual void fillFlag (Matrix<Bool>& value) const;
    virtual void fillFlagCategory (Array<Bool>& value) const;
    virtual void fillFlagCube (Cube<Bool>& value) const;
    virtual void fillFlagRow (Vector<Bool>& value) const;
    virtual void fillFloatData (Cube<Float>& value) const;
    virtual void fillFrequency (Vector<Double>& value) const;
    virtual void fillImagingWeight (Matrix<Float> & value) const;
    virtual void fillJonesC (Vector<SquareMatrix<Complex, 2> >& value) const;
    virtual void fillNChannel (Int& value) const;
    virtual void fillNCorr (Int& value) const;
    virtual void fillNRow (Int& value) const;
    virtual void fillObservationId (Vector<Int>& value) const;
    virtual void fillPhaseCenter (MDirection& value) const;
    virtual void fillPolFrame (Int& value) const;
    virtual void fillProcessorId (Vector<Int>& value) const;
    virtual void fillScan (Vector<Int>& value) const;
    virtual void fillSigma (Vector<Float>& value) const;
    virtual void fillSigmaMat (Matrix<Float>& value) const;
    virtual void fillSpectralWindow (Int& value) const;
    virtual void fillStateId (Vector<Int>& value) const;
    virtual void fillTime (Vector<Double>& value) const;
    virtual void fillTimeCentroid (Vector<Double>& value) const;
    virtual void fillTimeInterval (Vector<Double>& value) const;
    virtual void fillUvw (Vector<RigidVector<Double, 3> >& value) const;
    virtual void fillUvwMat (Matrix<Double>& value) const;
    virtual void fillVisibilityCorrected (Matrix<CStokesVector>& value) const;
    virtual void fillVisibilityModel (Matrix<CStokesVector>& value) const;
    virtual void fillVisibilityObserved (Matrix<CStokesVector>& value) const;
    virtual void fillWeight (Vector<Float>& value) const;
    virtual void fillWeightMat (Matrix<Float>& value) const;
    virtual void fillWeightSpectrum (Cube<Float>& value) const;

    VisBufferCache * cache_p;
    VisBufferState * state_p;
};

} // end namespace vb

} // end namespace casa


#endif


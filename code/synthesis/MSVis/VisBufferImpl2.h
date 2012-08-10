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

#ifndef MSVIS_VISBUFFERIMPL2_H
#define MSVIS_VISBUFFERIMPL2_H

#include <casa/aips.h>

//#warning "Remove next line later"
//#    include <synthesis/MSVis/VisBuffer.h>

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

class VisBufferImpl2 : public VisBufferBase2 {

    friend class casa::asyncio::VLAT; // for async i/o
    friend class VbCacheItemBase;
    friend class VisBufferCache;
    friend class VisBufferState;
    friend class VisBufferImpl2Async; // for async i/o
    friend class VisBufferImpl2AsyncWrapper; // for async i/o
    friend class ViReadImpl;
    friend class ViReadImplAsync; // for async I/O

public:

    // Create empty VisBufferImpl2 you can assign to or attach.
    VisBufferImpl2();
    // Construct VisBufferImpl2 for a particular VisibilityIterator2
    // The buffer will remain synchronized with the iterator.

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).

    VisBufferImpl2(const VisBufferImpl2 & vb);

    // Destructor (detaches from VisIter)

    virtual ~VisBufferImpl2();

    // Assignment, loses synchronization with iterator: only use buffer for
    // current iteration (or reattach)

    virtual VisBufferImpl2 & operator=(const VisBufferImpl2 & vb);

    // Assignment, optionally without copying the data across; with copy=True
    // this is identical to normal assignment operator

    virtual void assign(const VisBufferImpl2 & vb, Bool copy = True);
    virtual void associateWithVisibilityIterator2 (const ROVisibilityIterator2 & vi);

    virtual VisBufferImpl2 * clone () const;

    virtual const ROVisibilityIterator2 * getVi () const;

    virtual void invalidate();

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
    // VisBufferImpl2 only to benefit from caching of the feed and antenna IDs.

    // Note that feed_pa is a function instead of a cached value
    virtual Vector<Float> feed_pa(Double time) const;

    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const;
    virtual Vector<Float> parang(Double time) const;

    virtual MDirection azel0(Double time) const; // function rather than cached value
    virtual void azel0Vec(Double time, Vector<Double>& azelVec) const;
    virtual Vector<MDirection> azel(Double time) const;
    virtual void azelMat(Double time, Matrix<Double>& azelMat) const;

    // Hour angle for specified time
    virtual Double hourang(Double time) const;

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

    // Update coordinate info - useful for copied VisBufferImpl2s that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows.
    // if dirDependent is set to False the expensive direction dependent calculation of parallactic or direction of
    // antenna from pointing table is avoided
    //Add more as needed.

    virtual void updateCoordInfo(const VisBufferImpl2 * vb = NULL, const Bool dirDependent = True);
    void copyCoordInfo(const VisBufferImpl2 & other, Bool force = False);

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

    VisBufferImpl2(ROVisibilityIterator2 & iter);

    // Attach to a VisIter. Detaches itself first if already attached
    // to a VisIter. Will remain synchronized with iterator.

    virtual void attachToVisibilityIterator2 (ROVisibilityIterator2 & iter);
    virtual void cacheCopy (const VisBufferImpl2 & other, Bool markAsCached);
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
    virtual void stateCopy (const VisBufferImpl2 & other); // copy relevant noncached members

    template <typename Coord>
    void updateCoord (Coord & item, const Coord & otherItem);

private:

    virtual Bool areCorrelationsInCanonicalOrder () const;
    void checkVisIterBase (const char * func, const char * file, int line, const char * extra = "") const;
    void construct(ROVisibilityIterator2 * vi);
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


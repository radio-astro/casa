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
//#    include <msvis/MSVis/VisBuffer.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <msvis/MSVis/VisBufferComponents2.h>
#include <msvis/MSVis/VisBuffer2.h>

using casa::vi::VisBufferComponent2;
using casa::vi::VisBufferComponents2;

namespace casa { //# NAMESPACE CASA - BEGIN

class VisModelDataI;

namespace asyncio {
    class VLAT;
} // end namespace asyncio

namespace ms {
    class Vbi2MsRow;
}

namespace vi {

//#forward

class VbCacheItemBase;
class VisBufferCache;
class VisBufferState;
class VisBufferImpl2;
class VisibilityIterator2;
class VisibilityIterator2;

namespace avg {

    class VbAvg;
}



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
//
//</synopsis>

//<todo>
//</todo>

class VisBufferImpl2 : public VisBuffer2 {

    friend class VLAT; // for async i/o
    friend class casa::vi::avg::VbAvg;
    friend class AveragingTvi2;
    friend class VbCacheItemBase;
    friend class VisBufferCache;
    friend class VisBufferState;
    friend class VisBuffer2;
    friend class VisBufferImpl2Async; // for async i/o
    friend class VisBufferImpl2AsyncWrapper; // for async i/o
    friend class ViReadImpl;
    friend class ViReadImplAsync; // for async I/O
    friend class casa::ms::Vbi2MsRow;

public:


    // Destructor (detaches from VisIter)

    virtual ~VisBufferImpl2();

    void appendRow (ms::Vbi2MsRow * rowSrc, Int initialCapacity,
                    const VisBufferComponents2 & optionalComponentsToCopy);
    void appendRowsComplete ();
    Int appendSize () const;

    virtual void associateWithVisibilityIterator2 (const VisibilityIterator2 & vi);

    // Copies all of the components (or just the one in the cache) from
    // the specified VisBuffer into this one.

    virtual void copy (const VisBuffer2 & other, Bool fetchIfNeeded = True);

    // Copies the specified components (or just the one in the cache) from
    // the specified VisBuffer into this one.

    virtual void copyComponents (const VisBuffer2 & other,
				 const VisBufferComponents2 & components,
                                 Bool allowShapeChange = False,
				 Bool fetchIfNeeded = True);

    virtual void copyCoordinateInfo (const VisBuffer2 * vb, Bool dirDependent,
                                     Bool allowShapeChange = False, Bool fetchIfNeeded = True);

    virtual Vector<Int> getCorrelationTypes () const;
    virtual String getFillErrorMessage () const;
    virtual Double getFrequency (Int rowInBuffer, Int frequencyIndex, Int frame = FrameNotSpecified) const;
    virtual const Vector<Double> & getFrequencies (Int rowInBuffer,
                                                   Int frame = FrameNotSpecified) const;
    virtual Int getChannelNumber (Int rowInBuffer, Int frequencyIndex) const;
    virtual const Vector<Int> & getChannelNumbers (Int rowInBuffer) const;

    virtual const VisibilityIterator2 * getVi () const;

    ms::MsRow * getRow (Int row) const;
    ms::MsRow * getRowMutable (Int row);

    virtual void invalidate();

    virtual Bool isAttached () const;
    virtual Bool isFillable () const;
    virtual void setShape (Int nCorrelations, Int nChannels, Int nRows, Bool clearTheCache = True);
    virtual void validateShapes () const;

    virtual void writeChangesBack ();

    //--> This needs to be removed: virtual Bool fetch(const asyncio::PrefetchColumns *pfc);

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBufferImpl2 only to benefit from caching of the feed and antenna IDs.

    // Note that feed_pa is a function instead of a cached value
    virtual const Vector<Float> & feedPa (Double time) const;

    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const;
    virtual const Vector<Float> & parang(Double time) const;

    virtual MDirection azel0(Double time) const; // function rather than cached value
    virtual const Vector<MDirection> & azel(Double time) const;

    // Hour angle for specified time
    virtual Double hourang(Double time) const;

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    virtual void sortCorr ();
    virtual void unSortCorr();

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)

    virtual void normalize ();

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

    virtual Bool isNewArrayId () const;
    virtual Bool isNewFieldId () const;
    virtual Bool isNewMs() const;
    virtual Bool isNewSpectralWindow () const;
    virtual Bool isRekeyable () const;
    virtual Bool isWritable () const;
    virtual IPosition getShape () const;
    virtual Int msId() const;
    virtual String msName (Bool stripPath = False) const;
    virtual Subchunk getSubchunk () const;

    virtual Bool areCorrelationsSorted() const;
    virtual IPosition getValidShape (Int) const;
    virtual VisModelDataI * getVisModelData() const;
    virtual Bool modelDataIsVirtual () const;


    //////////////////////////////////////////////////////////////////////
    //
    //  Data accessors and setters (where appropriate)
    //
    //  There are the methods that allows access to the items cached in the
    //  VisBuffer.  The straight accessors provide read-only access to the
    //  item.  Where the item is allowed to be modified, one or more set
    //  methods are provided.  N.B.: the VB usually caches the information
    //  in the representation requested so that using a setter to modify
    //  a value (e.g., weight vs. weightMat) will not modify the cached value
    //  in a different representation.

    virtual const Vector<Int> & antenna1 () const;
    virtual void setAntenna1 (const Vector<Int> & value);
    virtual const Vector<Int> & antenna2 () const;
    virtual void setAntenna2 (const Vector<Int> & value);
    virtual const Vector<Int> & arrayId () const;
    virtual void setArrayId (const Vector<Int> &);
    virtual const Vector<SquareMatrix<Complex, 2> > & cjones () const;
    virtual const Vector<Int> & correlationTypes () const;
//    virtual Int dataDescriptionId () const;
//    virtual void setDataDescriptionId (Int value);
    virtual const Vector<Int> & dataDescriptionIds () const;
    virtual void setDataDescriptionIds (const Vector<Int> & ); // [nR]
    virtual const Vector<MDirection> & direction1 () const;
    virtual const Vector<MDirection> & direction2 () const;
    virtual const Vector<Double> & exposure () const;
    virtual void setExposure (const Vector<Double> & value);
    virtual const Vector<Int> & feed1 () const;
    virtual void setFeed1 (const Vector<Int> & value);
    virtual const Vector<Float> & feedPa1 () const;
    virtual const Vector<Int> & feed2 () const;
    virtual void setFeed2 (const Vector<Int> & value);
    virtual const Vector<Float> & feedPa2 () const;
    virtual const Vector<Int> & fieldId () const;
    virtual void setFieldId (const Vector<Int> &);
//    virtual const Matrix<Bool> & flag () const;
//    virtual void setFlag (const Matrix<Bool>&);
    virtual const Array<Bool> & flagCategory () const;
    virtual void setFlagCategory (const Array<Bool>&);
    virtual const Cube<Bool> & flagCube () const;
    virtual void setFlagCube (const Cube<Bool>&);
    virtual const Vector<Bool> & flagRow () const;
    virtual void setFlagRow (const Vector<Bool>&);
    virtual const Matrix<Float> & imagingWeight () const;
    virtual Int nAntennas () const;
    virtual Int nChannels () const;
    virtual Int nCorrelations () const;
    virtual Int nRows () const;
    virtual const Vector<Int> & observationId () const;
    virtual void setObservationId (const Vector<Int> & value);
    virtual const MDirection& phaseCenter () const;
    virtual Int polarizationFrame () const;
    virtual Int polarizationId () const;
    virtual const Vector<Int> & processorId () const;
    virtual void setProcessorId (const Vector<Int> & value);
    virtual const Vector<uInt> & rowIds () const;
    virtual const Vector<Int> & scan () const;
    virtual void setScan (const Vector<Int> & value);
    virtual const Matrix<Float> & sigma () const;
    virtual void setSigma (const Matrix<Float> &);
    //virtual const Matrix<Float> & sigmaMat () const;
    //virtual Int spectralWindow () const;
    virtual const Vector<Int> & spectralWindows () const;
    virtual void setSpectralWindows (const Vector<Int> & spectralWindows);
    virtual const Vector<Int> & stateId () const;
    virtual void setStateId (const Vector<Int> & value);
    virtual const Vector<Double> & time () const;
    virtual void setTime (const Vector<Double> & value);
    virtual const Vector<Double> & timeCentroid () const;
    virtual void setTimeCentroid (const Vector<Double> & value);
    virtual const Vector<Double> & timeInterval () const;
    virtual void setTimeInterval (const Vector<Double> & value);
    virtual const Matrix<Double> & uvw () const;
    virtual void setUvw (const Matrix<Double> & value);
    virtual const Cube<Complex> & visCubeCorrected () const;
    virtual void setVisCubeCorrected (const Cube<Complex> &);
//    virtual const Matrix<CStokesVector> & visCorrected () const;
//    virtual void setVisCorrected (const Matrix<CStokesVector> &);
    virtual const Cube<Float> & visCubeFloat () const;
    virtual void setVisCubeFloat (const Cube<Float> &);
    virtual const Cube<Complex> & visCubeModel () const;
    virtual void setVisCubeModel (const Complex & c);
    virtual void setVisCubeModel (const Cube<Complex>& vis);
//    virtual void setVisCubeModel(const Vector<Float>& stokes);
//    virtual const Matrix<CStokesVector> & visModel () const;
//    virtual void setVisModel (Matrix<CStokesVector> &);
    virtual const Cube<Complex> & visCube () const;
    virtual void setVisCube(const Complex & c);
    virtual void setVisCube (const Cube<Complex> &);
//    virtual const Matrix<CStokesVector> & vis () const;
//    virtual void setVis (Matrix<CStokesVector> &);
    virtual const Matrix<Float> & weight () const;
    virtual void setWeight (const Matrix<Float>&);
    virtual const Cube<Float> & weightSpectrum () const;
    virtual void setWeightSpectrum (const Cube<Float>&);
    virtual const Cube<Float> & weightSpectrumCorrected () const;
    virtual void setWeightSpectrumCorrected (const Cube<Float>& value);

protected:

    // Create empty VisBufferImpl2 you can assign to or attach.
    VisBufferImpl2 (VisBufferOptions options = VbNoOptions);
    // Construct VisBufferImpl2 for a particular VisibilityIterator2
    // The buffer will remain synchronized with the iterator.

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).

    VisBufferImpl2(VisibilityIterator2 * iter, VisBufferOptions options);

    // Attach to a VisIter. Detaches itself first if already attached
    // to a VisIter. Will remain synchronized with iterator.

    //virtual void cacheCopy (const VisBufferImpl2 & other, Bool markAsCached);

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

    template<typename T>
    void averageVisCubeChannels (T & dataCache,
                                 Int nChanOut,
                                 const Matrix<Int>& averagingbounds);

    virtual void cacheClear (Bool markAsCached = False);
    virtual void cacheResizeAndZero (const VisBufferComponents2 & exclusions = VisBufferComponents2());

    virtual void checkVisIter (const char * func, const char * file, int line, const char * extra = "") const;
    void computeRowWeightFactors (Matrix <Float> & rowWeightFactors, Bool useWeightSpectrum);
    virtual void configureNewSubchunk (Int msId, const String & msName, Bool isNewMs,
                                       Bool isNewArrayId, Bool isNewFieldId,
                                       Bool isNewSpectralWindow, const Subchunk & subchunk,
                                       Int nRows, Int nChannels, Int nCorrelations,
                                       const Vector<Int> & correlations,
                                       CountedPtr<WeightScaling> weightScaling);

    virtual void copyRow (Int sourceRow, Int destinationRow);
    virtual void deleteRows (const Vector<Int> & rowsToDelete);
    virtual void dirtyComponentsAdd (const VisBufferComponents2 & additionalDirtyComponents);
    virtual void dirtyComponentsAdd (VisBufferComponent2 component);
    virtual void dirtyComponentsClear ();
    virtual VisBufferComponents2 dirtyComponentsGet () const;
    virtual void dirtyComponentsSet (const VisBufferComponents2 & dirtyComponents);
    virtual void dirtyComponentsSet (VisBufferComponent2 component);
    virtual Bool hasShape () const;
    void normalizeRow (Int row, Int nCorrelations, const Cube<Bool> & flagged,
                       Cube<Complex> & visCube, Cube<Complex> & modelCube,
                        Matrix<Float> & weightMat);

    virtual void sortCorrelationsAux (Bool makeSorted);
    virtual VisibilityIterator2 * getViP () const; // protected, non-const access to VI
    void registerCacheItem (VbCacheItemBase *);
    virtual void resizeRows (Int newNRows);
    virtual void stateCopy (const VisBufferImpl2 & other); // copy relevant noncached members
    virtual void setFillable (Bool isFillable);
    virtual void setRekeyable (Bool isRekeyable);
    virtual void setupValidShapes ();
    template <typename Coord>
    void updateCoord (Coord & item, const Coord & otherItem);

    virtual Vector<Bool> & flagRowRef ();  // [nR]
    virtual Cube<Bool> & flagCubeRef ();  // [nC,nF,nR]
    virtual Cube<Complex> & visCubeRef (); // [nC,nF,nR]
    virtual Cube<Complex> & visCubeCorrectedRef (); // [nC,nF,nR]
    virtual Cube<Complex> & visCubeModelRef (); // [nC,nF,nR]
    virtual Cube<Float> & weightSpectrumRef (); // [nC,nF,nR]
    virtual Cube<Float> & weightSpectrumCorrectedRef (); // [nC,nF,nR]

    Float getWeightScaled (Int row) const;
    Float getWeightScaled (Int correlation, Int row) const;
    Float getWeightScaled (Int correlation, Int channel, Int row) const;
    virtual Float getWeightCorrectedScaled (Int correlation, Int channel, Int row) const;
    virtual CountedPtr<WeightScaling> getWeightScaling () const;
    void setIterationInfo (Int msId, const String & msName, Bool isNewMs,
                           Bool isNewArrayId, Bool isNewFieldId, Bool isNewSpectralWindow,
                           const Subchunk & subchunk, const Vector<Int> & correlations,
                           CountedPtr <WeightScaling> weightScaling);
    Bool weightSpectrumPresent () const;

private:

    virtual Bool areCorrelationsInCanonicalOrder () const;
    void checkVisIterBase (const char * func, const char * file, int line, const char * extra = "") const;
    void construct(VisibilityIterator2 * vi, VisBufferOptions options);
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
    virtual void fillArrayId (Vector<Int>& value) const;
    virtual void fillCorrType (Vector<Int>& value) const;
    virtual void fillCubeCorrected (Cube <Complex> & value) const;
    virtual void fillCubeModel (Cube <Complex> & value) const;
    virtual void fillCubeObserved (Cube <Complex> & value) const;
    virtual void fillDataDescriptionId  (Int& value) const;
    virtual void fillDataDescriptionIds  (Vector<Int>& value) const;
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
    virtual void fillFieldId (Vector<Int>& value) const;
    virtual void fillFlag (Matrix<Bool>& value) const;
    virtual void fillFlagCategory (Array<Bool>& value) const;
    virtual void fillFlagCube (Cube<Bool>& value) const;
    virtual void fillFlagRow (Vector<Bool>& value) const;
    virtual void fillFloatData (Cube<Float>& value) const;
    virtual void fillImagingWeight (Matrix<Float> & value) const;
    virtual void fillJonesC (Vector<SquareMatrix<Complex, 2> >& value) const;
    virtual void fillNAntennas (Int& value) const;
    virtual void fillNChannel (Int& value) const;
    virtual void fillNCorr (Int& value) const;
    virtual void fillNRow (Int& value) const;
    virtual void fillObservationId (Vector<Int>& value) const;
    virtual void fillPhaseCenter (MDirection& value) const;
    virtual void fillPolFrame (Int& value) const;
    virtual void fillPolarizationId (Int& value) const;
    virtual void fillProcessorId (Vector<Int>& value) const;
    virtual void fillRowIds (Vector<uInt>& value) const;
    virtual void fillScan (Vector<Int>& value) const;
    virtual void fillSigma (Matrix<Float>& value) const;
    //virtual void fillSigmaMat (Matrix<Float>& value) const;
    //virtual void fillSpectralWindow (Int& value) const;
    virtual void fillSpectralWindows  (Vector<Int>& value) const;
    virtual void fillStateId (Vector<Int>& value) const;
    virtual void fillTime (Vector<Double>& value) const;
    virtual void fillTimeCentroid (Vector<Double>& value) const;
    virtual void fillTimeInterval (Vector<Double>& value) const;
    virtual void fillUvw (Matrix<Double>& value) const;
//    virtual void fillVisibilityCorrected (Matrix<CStokesVector>& value) const;
//    virtual void fillVisibilityModel (Matrix<CStokesVector>& value) const;
//    virtual void fillVisibilityObserved (Matrix<CStokesVector>& value) const;
    virtual void fillWeight (Matrix<Float>& value) const;
    //virtual void fillWeightMat (Matrix<Float>& value) const;
    virtual void fillWeightSpectrum (Cube<Float>& value) const;
    virtual void fillWeightSpectrumCorrected (Cube<Float>& value) const;

    VisBufferCache * cache_p;
    VisBufferState * state_p;
};

} // end namespace vi

} // end namespace casa


#endif


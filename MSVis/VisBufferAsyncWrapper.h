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
//# $Id: VisBuffer.h,v 19.14 2006/02/28 04:48:58 mvoronko Exp $

#ifndef VISBUFFER_ASYNC_WRAPPER_H
#define VISBUFFER_ASYNC_WRAPPER_H

#include <casa/aips.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisBufferAsync.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/MSCalEnums.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward

//<summary>VisBuffers encapulsate one chunk of visibility data for processing.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisSet">VisSet</linkto>
//   <li> <linkto class="VisibilityIterator">VisibilityIterator</linkto>
// </prerequisite>
//
// <etymology>
// VisBuffer is a buffer for visibility data
// </etymology>
//
//<synopsis>
// This class contains 'one iteration' of the
// <linkto class="VisibilityIterator">VisibilityIterator</linkto>
// It is a modifiable
// buffer of values to which calibration and averaging can be applied.
// This allows processing of the data in larger blocks, avoiding some
// overheads for processing per visibility point or spectrum.
//
// See <linkto module="MeasurementEquations">MeasurementEquations</linkto>
// for more details on how the VisBuffer is to be used.
//</synopsis>

//<todo>
// <li> reconcile vis/visCube usage: visCube, flagCube and weightMatrix
// are currently only correct when this VisBuffer got them from a
// VisIter, operations like -=, freqAverage() are only done for
// visibility() and flag().
//</todo>
class VisBufferAsyncWrapper : public VisBufferAsync {

    friend class ViReadImplAsync;

public:
    // Create empty VisBuffer you can assign to or attach.
    VisBufferAsyncWrapper();
//    // Construct VisBuffer for a particular VisibilityIterator
//    // The buffer will remain synchronized with the iterator.
    VisBufferAsyncWrapper (ROVisibilityIterator & iter);
//
//    // Copy construct, looses synchronization with iterator: only use buffer for
//    // current iteration (or reattach).
    VisBufferAsyncWrapper (const VisBufferAsync & vb);

    // Destructor (detaches from VisIter)
    virtual ~VisBufferAsyncWrapper();

    // Assignment, loses synchronization with iterator: only use buffer for
    // current iteration (or reattach)

    virtual VisBufferAsyncWrapper & operator=(const VisBufferAsyncWrapper & vb);

    // Assignment, optionally without copying the data across; with copy=True
    // this is identical to normal assignment operator

    virtual VisBufferAsyncWrapper & assign(const VisBuffer & vb, Bool copy = True);

    // subtraction: return the difference of the visibilities, flags of
    // this and other are or-ed. An exception is thrown if the number of
    // rows or channels differs, but no further checks are done.
    VisBufferAsyncWrapper & operator-=(const VisBuffer & vb);

    // Attach to a VisIter. Detaches itself first if already attached
    // to a VisIter. Will remain synchronized with iterator.
    virtual void attachToVisIter(ROVisibilityIterator & iter);
    void attachToVisIterAsync (ROVisibilityIterator & iter);
    virtual VisBuffer * clone ();
    virtual void detachFromVisIter ();

    // Invalidate the cache
    virtual void invalidate();

    // <group>
    // Access functions
    //
    virtual Int & nCorr();
    Int nCorr() const;

    Int & nChannel();
    Int nChannel() const;

    Vector<Int>& channel();
    const Vector<Int>& channel() const;

    Bool newArrayId () const;
    Bool newFieldId () const;
    Bool newSpectralWindow () const;

    Int & nRow();
    Int nRow() const;

    Vector<Int>& antenna1();
    const Vector<Int>& antenna1() const;

    Vector<Int>& antenna2();
    const Vector<Int>& antenna2() const;

    Vector<Int>& feed1();
    const Vector<Int>& feed1() const;

    Vector<Int>& feed2();
    const Vector<Int>& feed2() const;

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBuffer only to benefit from caching of the feed and antenna IDs.
    Vector<Float>& feed1_pa();
    const Vector<Float>& feed1_pa() const;

    Vector<Float>& feed2_pa();
    const Vector<Float>& feed2_pa() const;

    Vector<SquareMatrix<Complex, 2> >& CJones();
    const Vector<SquareMatrix<Complex, 2> >& CJones() const;

    // Note that feed_pa is a function instead of a cached value
    virtual Vector<Float> feed_pa(Double time) const;

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.
    Vector<MDirection>& direction1();
    const Vector<MDirection>& direction1()  const;

    Vector<MDirection>& direction2();
    const Vector<MDirection>& direction2()  const;


    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const;
    virtual Vector<Float> parang(Double time) const;

    // Note that azel is a function instead of a cached value
    virtual MDirection azel0(Double time) const;
    Vector<Double>& azel0Vec(Double time, Vector<Double>& azelVec) const;
    virtual Vector<MDirection> azel(Double time) const;
    Matrix<Double>& azelMat(Double time, Matrix<Double>& azelMat) const;

    // Hour angle for specified time
    virtual Double hourang(Double time) const;

    Int fieldId() const;

    Int arrayId() const;

    Int polarizationId() const;

    // Return flag for each channel & row
    Matrix<Bool>& flag();
    const Matrix<Bool>& flag() const;

    // Return flag for each polarization, channel and row
    Cube<Bool>& flagCube();
    const Cube<Bool>& flagCube() const;

    Vector<Bool>& flagRow();
    const Vector<Bool>& flagRow() const;

    // Return flags for each polarization, channel, category, and row.
    Array<Bool>& flagCategory();
    const Array<Bool>& flagCategory() const;

    Vector<Int>& scan();
    const Vector<Int>& scan() const;

    // scalar version for convenience, when scan known constant for
    // entire iteration/buffer.
    Int scan0();

    Vector<Int>& processorId();
    const Vector<Int>& processorId() const;

    Vector<Int>& observationId();
    const Vector<Int>& observationId() const;

    Vector<Int>& stateId();
    const Vector<Int>& stateId() const;

    // Gets SPECTRAL_WINDOW/CHAN_FREQ (in Hz, acc. to the MS def'n v.2).
    Vector<Double>& frequency();
    const Vector<Double>& frequency() const;

//    Vector<Double>& lsrFrequency();
//    const Vector<Double>& lsrFrequency() const;


    //the following method is to convert the observed frequencies
    // This conversion may not be accurate for some frame
    // conversion like topo to lsr except if the spw is in the actual buffer

    virtual void lsrFrequency(const Int & spw, Vector<Double>& freq, Bool & convert) const;

    virtual Int numberCoh () const;

    MDirection & phaseCenter();
    MDirection phaseCenter() const;

    Int polFrame() const;

    Vector<Int>& corrType();
    const Vector<Int>& corrType() const;

    Vector<Float>& sigma();
    const Vector<Float>& sigma() const;

    Matrix<Float>& sigmaMat();
    const Matrix<Float>& sigmaMat() const;

    Int & spectralWindow();
    Int spectralWindow() const;
    virtual Int dataDescriptionId() const;
    Vector<Double>& time();
    const Vector<Double>& time() const;

    Vector<Double>& timeCentroid();
    const Vector<Double>& timeCentroid() const;

    Vector<Double>& timeInterval();
    const Vector<Double>& timeInterval() const;

    Vector<Double>& exposure();
    const Vector<Double>& exposure() const;

    Vector<RigidVector<Double, 3> >& uvw();
    const Vector<RigidVector<Double, 3> >& uvw() const;

    Matrix<Double>& uvwMat();
    const Matrix<Double>& uvwMat() const;

    Matrix<CStokesVector>& visibility();
    const Matrix<CStokesVector>& visibility() const;

    Matrix<CStokesVector>& modelVisibility();
    const Matrix<CStokesVector>& modelVisibility() const;

    Matrix<CStokesVector>& correctedVisibility();
    const Matrix<CStokesVector>& correctedVisibility() const;

    Cube<Complex>& visCube();
    const Cube<Complex>& visCube() const;

    Cube<Complex>& modelVisCube();

    Cube<Complex>& modelVisCube(const Bool & matchVisCubeShape);

    const Cube<Complex>& modelVisCube() const;

    Cube<Complex>& correctedVisCube();
    const Cube<Complex>& correctedVisCube() const;

    // Return visCube(), modelVisCube(), or correctedVisCube(),
    // according to whichcol.  They throw an exception if whichcol is
    // unsupported, but note the encouraged default.
    // TODO: Optionally return DATA if whichcol is unavailable.
    Cube<Complex>& dataCube(const MS::PredefinedColumns whichcol=MS::DATA);
    const Cube<Complex>& dataCube(const MS::PredefinedColumns
                                  whichcol=MS::DATA) const;

    Cube<Float>& floatDataCube();
    const Cube<Float>& floatDataCube() const;

    // Returns the weights for each row averaged over the parallel hand correlations.
    Vector<Float>& weight();
    const Vector<Float>& weight() const;

    // Returns the nPol_p x curNumRow_p weight matrix.
    Matrix<Float>& weightMat();
    const Matrix<Float>& weightMat() const;

    // Is a valid WEIGHT_SPECTRUM available?
    Bool existsWeightSpectrum() const;

    Cube<Float>& weightSpectrum();
    const Cube<Float>& weightSpectrum() const;

    const Matrix<Float>& imagingWeight() const;
    Matrix<Float> & imagingWeight ();

    Cube<Float>& weightCube();
    //</group>

    //<group>
    // Utility functions to provide coordinate or column ranges of the
    // data in the VisBuffer. Flagging is applied before computing the ranges.
    //
    // Generic accessor to column ranges of integer type, as specified by
    // enumerations defined in class MSCalEnums. Throws an exception
    // if the enum is not for a recognized integer column.
    Vector<Int> vecIntRange(const MSCalEnums::colDef & calEnum) const;

    // Antenna id. range (includes both ANTENNA1 and ANTENNA2 columns)
    Vector<Int> antIdRange() const;

    // Time range
    Bool timeRange(MEpoch & rTime, MVEpoch & rTimeEP, MVEpoch & rInterval) const;

    // Return the row Ids from the original ms. If the ms used is a subset of
    // another ms then rowIds() return the row ids of the original ms.
    virtual Vector<uInt>& rowIds();

    virtual const Vector<uInt>& rowIds() const;;

    //</group>

    // Frequency average the buffer (visibility() column only)
    void freqAverage();

    // Frequency average the buffer (visCube and [if present] modelVisCube)
    void freqAveCubes();

    // Average channel axis according to chanavebounds, for whichever of DATA,
    // MODEL_DATA, CORRECTED_DATA, FLOAT_DATA, FLAG, and WEIGHT_SPECTRUM are
    // present.  It will only treat the first 5 as present if they have already
    // been loaded into the buffer!
    void channelAve(const Matrix<Int>& chanavebounds);

    // Average channel axis by factor.
    template<class T> void chanAveVisCube(Cube<T>& data, Int nChanOut);

    // Accumulate channel axis by factor, without applying WEIGHT_SPECTRUM even
    // if it is present.
    // It is primarily intended for averaging WEIGHT_SPECTRUM itself.
    template<class T> void chanAccCube(Cube<T>& data, Int nChanOut);

    // This defaults to no conceptual side effects, but usually it is more
    // efficient to let it leave weightSpectrum() in a channel averaged state.
    // restoreWeightSpectrum has no effect if !existsWeightSpectrum().
    void chanAveFlagCube(Cube<Bool>& flagcube, const Int nChanOut,
                         const Bool restoreWeightSpectrum = True);

    // Form Stokes parameters from correlations
    //  (these are preliminary versions)
    void formStokes();
    void formStokesWeightandFlag();
    void formStokes(Cube<Complex>& vis);
    void formStokes(Cube<Float>& fcube);    // Will throw up if asked to do all 4.

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    void sortCorr();
    void unSortCorr();

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    void normalize(const Bool & phaseOnly = False);

    // Fill weightMat according to sigma column
    void resetWeightMat();

    // Rotate visibility phase for phase center offsets
    void phaseCenterShift(Double dx, Double dy);

    // Update coordinate info - useful for copied VisBuffers that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows. Add more as needed.
    virtual void updateCoordInfo(const VisBuffer * vb = NULL);

    // Set the visibility to a constant, note that this only changes the buffer,
    // no values are written back to tables from here.
    virtual void setVisCube(Complex c);
    virtual void setModelVisCube(Complex c);
    virtual void setCorrectedVisCube(Complex c);

    // Set the visibility, note that this only changes the buffer,
    // no values are written back to tables from here.
    void setVisCube(const Cube<Complex>& vis);
    void setModelVisCube(const Cube<Complex>& vis);
    void setCorrectedVisCube(const Cube<Complex>& vis);

    // Like the above, but for FLOAT_DATA, keeping it as real floats.
    void setFloatDataCube(const Cube<Float>& fcube);

    // Set model according to a Stokes vector
    void setModelVisCube(const Vector<Float>& stokes);

    // Reference external model visibilities
    void refModelVis(const Matrix<CStokesVector>& mvis);

    // Remove scratch cols data from vb
    void removeScratchCols();

    // Access the current ROMSColumns object via VisIter
    virtual const ROMSColumns & msColumns() const;

    virtual  Int numberAnt () const;



    // Get all selected spectral windows not just the one in the actual buffer
    virtual void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);

    void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan) const;

    // Return the actual msid, useful if using multiple ms to monitor which
    // ms in the  list is being dealt with
    virtual Int msId() const;

    //checked if the ms has changed since the last chunk processed
    virtual Bool newMS() const;

    void wrap (VisBufferAsync * vba);

protected:

    virtual Bool checkMSId();
    virtual void checkVisIter (const char * func, const char * file, int line) const;
    void copyCache (const VisBuffer & other, Bool force);
    const VisImagingWeight & getImagingWeightGenerator () const;
    Int getOldMsId () const;
    ROVisibilityIterator * getVisibilityIterator () const;
    VisBufferAsync * releaseVba ();

    // Create cache status accessors which relay the request to the wrapped
    // VBA.

#undef CacheStatus
#define CacheStatus(item)\
Bool item ## OK () const\
{\
    if (wrappedVba_p == NULL){\
        throw AipsError ("VisBufferAsyncWrapper: No attached VBA", __FILE__, __LINE__);\
    }\
    return wrappedVba_p->item ## OK_p;\
}

    CacheStatus (antenna1);
    CacheStatus (antenna2);
    CacheStatus (arrayId);
    CacheStatus (channel);
    CacheStatus (cjones);
    CacheStatus (correctedVisCube);
    CacheStatus (correctedVisibility);
    CacheStatus (corrType);
    CacheStatus (dataDescriptionId);
    CacheStatus (direction1);
    CacheStatus (direction2);
    CacheStatus (exposure);
    CacheStatus (feed1_pa);
    CacheStatus (feed1);
    CacheStatus (feed2_pa);
    CacheStatus (feed2);
    CacheStatus (fieldId);
    CacheStatus (flagCategory);
    CacheStatus (flagCube);
    CacheStatus (flag);
    CacheStatus (flagRow);
    CacheStatus (floatDataCube);
    CacheStatus (frequency);
    CacheStatus (imagingWeight);
    CacheStatus (modelVisCube);
    CacheStatus (modelVisibility);
    CacheStatus (ms);
    CacheStatus (nChannel);
    CacheStatus (nCorr);
    CacheStatus (nRow);
    CacheStatus (observationId);
    CacheStatus (phaseCenter);
    CacheStatus (polFrame);
    CacheStatus (processorId);
    CacheStatus (rowIds);
    CacheStatus (scan);
    CacheStatus (sigmaMat);
    CacheStatus (sigma);
    CacheStatus (spectralWindow);
    CacheStatus (stateId);
    CacheStatus (timeCentroid);
    CacheStatus (timeInterval);
    CacheStatus (time);
    CacheStatus (uvwMat);
    CacheStatus (uvw);
    CacheStatus (visCube);
    CacheStatus (visibility);
    CacheStatus (weightMat);
    CacheStatus (weight);
    CacheStatus (weightSpectrum);

private:

    VisBufferAsync * wrappedVba_p;
    ROVisibilityIterator * wrappedVisIterAsync_p;

    // validate the cache
    void validate();
    void attachWrappedVb (ROVisibilityIterator * iter);

    void setAllCacheStatuses (bool status);


    Bool nonCanonCorr(); // Are correlations in non-canonical order?

    // Filter index arrays for unique elements
    Vector<Int> unique(const Vector<Int>& indices) const;

    // +-------------------------------------------------+
    // | Cache Declarations (fillers, statuses and data) |
    // +-------------------------------------------------+
    //
    // Preferred naming convention:
    //
    // T thing () --- returns the value of thing (public method)
    // T fillThing () --- fills thing cache variable and sets thingOK_p to true (private)
    // Bool thingOK_p --- true if the value of thing is currently cached  (private)
    // T thing_p --- holds cached value of thing (if thingOK_p is true)  (private)
    //
    // Example: Vector<Int>& feed1(); Vector<Int>& fillFeed1();
    //          Bool feed1OK_P; Vector<Int> feed1_p;

    // Cache filling routines in alphabetical order
    //
    // The first line in a fill routine should be the macro CheckVisIter.  This
    // will cause an AipsError to be throw when there is no visibilty iterator
    // associated with the VisBuffer.  This is especially important because the
    // fillers may be called on a VisBufferAsync in two contexts: filling
    // where there will be a VI attached and in normal use where there is no VI
    // attached.  The filler must also set the cache status variable to True during
    // its execution.

    Vector<Int>& fillAnt1();
    Vector<Int>& fillAnt2();
    Int & fillArrayId();
    //Matrix<Int>& fillChanAveBounds();
    Vector<Int>& fillChannel();
    Vector<SquareMatrix<Complex, 2> >& fillCjones();
    Vector<Int>& fillCorrType();
    virtual Vector<MDirection>& fillDirection1();
    virtual Vector<MDirection>& fillDirection2();
    Vector<Double>& fillExposure();
    Vector<Int>& fillFeed1();
    Vector<Float>& fillFeed1_pa();
    Vector<Int>& fillFeed2();
    Vector<Float>& fillFeed2_pa();
    Int & fillFieldId();
    Matrix<Bool>& fillFlag();
    Array<Bool>& fillFlagCategory();
    Cube<Bool>& fillFlagCube();
    Vector<Bool> & fillFlagRow();
    Cube<Float>& fillFloatDataCube();
    Vector<Double>& fillFreq();         // Puts SPECTRAL_WINDOW/CHAN_FREQ in frequency_p.
    //Matrix<Float>& fillImagingWeight();
    //Vector<Double>& fillLSRFreq();
    Int & fillnChannel();
    Int & fillnCorr();
    Int & fillnRow();
    Vector<Int> & fillObservationId();
    virtual MDirection & fillPhaseCenter();
    Int & fillPolFrame();
    Vector<Int> & fillProcessorId();
    Vector<Int> & fillScan();
    Vector<Float>& fillSigma();
    Matrix<Float>& fillSigmaMat();
    Int & fillSpW();
    Vector<Int> & fillStateId();
    Vector<Double>& fillTime();
    Vector<Double>& fillTimeCentroid();
    Vector<Double>& fillTimeInterval();
    Vector<RigidVector<Double, 3> >& filluvw();
    Matrix<Double>& filluvwMat();
    Matrix<CStokesVector>& fillVis(VisibilityIterator::DataColumn whichOne);
    Cube<Complex>& fillVisCube(VisibilityIterator::DataColumn whichOne);
    Vector<Float>& fillWeight();
    Matrix<Float>& fillWeightMat();
    Cube<Float>& fillWeightSpectrum();

};

} //# NAMESPACE CASA - END

#endif

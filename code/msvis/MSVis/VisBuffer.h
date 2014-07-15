//# VisBuffer.h: buffer for iterating through MS in large blocks
//# Copyright (C) 1996-2014
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

#ifndef MSVIS_VISBUFFER_H
#define MSVIS_VISBUFFER_H

#include <casa/aips.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <msvis/MSVis/VisModelDataI.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBufferComponents.h>
#include <msvis/MSVis/MSCalEnums.h>
#include <set>

namespace casa { //# NAMESPACE CASA - BEGIN


//#forward
  class SIMapperCollection;

// <summary>
// VbDirtyComponents allows marking portions of a VisBuffer as
// modified (aka dirty).  This feature is needed for the Visibility
// Processing Framework (VPF) which allows a sequence of data processing
// nodes to work as a bucket brigade operating sequentially on a
// VisBuffer.  A downstream output node needs to know what data,
// if any, needs to be written out.
//
// <prerequisite>
//   #<li><linkto class="VisBuffer">VisBuffer</linkto>
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

namespace asyncio {
    class VLAT;
} // end namespace asyncio

//<summary>VisBuffers encapsulate one chunk of visibility data for processing.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisSet">VisSet</linkto>
//   <li> <linkto class="VisibilityIterator">VisibilityIterator</linkto>
//   <li> <linkto class="VisBufferAutoPtr">VisBufferAutoPtr</linkto>
//   <li> <linkto class="VbDirtyComponents">VbDirtyComponents</linkto>
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
//
// When the user intends to allow asynchronous I/O they will need to use
// the VisBufferAsync and VisBufferAsyncWrapper classes; these are
// publicly derived from VisBuffer.  Normally, the user should not
// explicitly use these classes but should use the helper class
// VisBufferAutoPtr which will ensure that the appropriate class is
// used.
//</synopsis>

//<todo>
// <li> reconcile vis/visCube usage: visCube, flagCube and weightMatrix
// are currently only correct when this VisBuffer got them from a
// VisIter, operations like -=, freqAverage() are only done for
// visibility() and flag().
//</todo>

class VisBuffer {

    friend class asyncio::VLAT; // for async i/o
    friend class VisBufferAsync; // for async i/o
    friend class VisBufferAsyncWrapper; // for async i/o
    friend class ViReadImplAsync; // for async I/O
    friend class SIMapperCollection; //for SIimager as we need access to MS object

public:
    // Create empty VisBuffer you can assign to or attach.
    VisBuffer();
    // Construct VisBuffer for a particular VisibilityIterator
    // The buffer will remain synchronized with the iterator.
    VisBuffer(ROVisibilityIterator & iter);

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).
    VisBuffer(const VisBuffer & vb);

    // Destructor (detaches from VisIter)
    virtual ~VisBuffer();

    // Assignment, loses synchronization with iterator: only use buffer for
    // current iteration (or reattach)

    virtual VisBuffer & operator=(const VisBuffer & vb);

    // Assignment, optionally without copying the data across; with copy=True
    // this is identical to normal assignment operator

    virtual VisBuffer & assign(const VisBuffer & vb, Bool copy = True);

    virtual VisBuffer * clone () const;

    // subtraction: return the difference of the visibilities, flags of
    // this and other are or-ed. An exception is thrown if the number of
    // rows or channels differs, but no further checks are done.
    VisBuffer & operator-=(const VisBuffer & vb);

    // Attach to a VisIter. Detaches itself first if already attached
    // to a VisIter. Will remain synchronized with iterator.
    virtual void attachToVisIter(ROVisibilityIterator & iter);
    virtual void detachFromVisIter ();

    // Invalidate the cache
    virtual void invalidate();

    // <group>
    // Access functions
    //
    virtual Int & nCorr() {
        return nCorrOK_p ? nCorr_p : fillnCorr();
    }
    virtual Int nCorr() const {
        return This->nCorr();
    }

    virtual Int & nChannel() {
        return nChannelOK_p ? nChannel_p : fillnChannel();
    }
    virtual Int nChannel() const {
        return This->nChannel();
    }

    virtual Vector<Int>& channel() {
        return channelOK_p ? channel_p : fillChannel();
    }
    virtual const Vector<Int>& channel() const {
        return This->channel();
    }

    // virtual Int & nCat() {
    //     return nCatOK_p ? nCat_p : fillnCat();
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
    virtual Int & nRow() {
        return nRowOK_p ? nRow_p : fillnRow();
    }
    virtual Int nRow() const {
        return This->nRow();
    }

    virtual Int nRowChunk() const;

    virtual Vector<Int>& antenna1() {
        return antenna1OK_p ? antenna1_p : fillAnt1();
    }
    virtual const Vector<Int>& antenna1() const {
        return This->antenna1();
    }

    virtual Vector<Int>& antenna2() {
        return antenna2OK_p ? antenna2_p : fillAnt2();
    }
    virtual const Vector<Int>& antenna2() const {
        return This->antenna2();
    }

    virtual Vector<Int>& feed1() {
        return feed1OK_p ? feed1_p : fillFeed1();
    }
    virtual const Vector<Int>& feed1() const {
        return This->feed1();
    }

    virtual Vector<Int>& feed2() {
        return feed2OK_p ? feed2_p : fillFeed2();
    }
    virtual const Vector<Int>& feed2() const {
        return This->feed2();
    }

    virtual void dirtyComponentsAdd (const VbDirtyComponents & additionalDirtyComponents);
    virtual void dirtyComponentsAdd (VisBufferComponents::EnumType component);
    virtual void dirtyComponentsClear ();
    virtual VbDirtyComponents dirtyComponentsGet () const;
    virtual void dirtyComponentsSet (const VbDirtyComponents & dirtyComponents);
    virtual void dirtyComponentsSet (VisBufferComponents::EnumType component);

    virtual Bool fetch(const asyncio::PrefetchColumns *pfc);

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBuffer only to benefit from caching of the feed and antenna IDs.
    virtual Vector<Float>& feed1_pa() {
        return feed1_paOK_p ? feed1_pa_p : fillFeed1_pa();
    }
    virtual const Vector<Float>& feed1_pa() const {
        return This->feed1_pa();
    }

    virtual Vector<Float>& feed2_pa() {
        return feed2_paOK_p ? feed2_pa_p : fillFeed2_pa();
    }
    virtual const Vector<Float>& feed2_pa() const {
        return This->feed2_pa();
    }

    virtual Vector<SquareMatrix<Complex, 2> >& CJones() {
        return cjonesOK_p ? cjones_p : fillCjones();
    }
    virtual const Vector<SquareMatrix<Complex, 2> >& CJones() const {
        return This->CJones();
    }

    // Note that feed_pa is a function instead of a cached value
    virtual Vector<Float> feed_pa(Double time) const;

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.
    virtual Vector<MDirection>& direction1() {
        return direction1OK_p ? direction1_p
               : fillDirection1();
    }
    virtual const Vector<MDirection>& direction1()  const {
        return This->direction1();
    }

    virtual Vector<MDirection>& direction2() {
        return direction2OK_p ? direction2_p :
               fillDirection2();
    }
    virtual const Vector<MDirection>& direction2()  const {
        return This->direction2();
    }
    virtual MDirection& firstDirection1() {
        return firstDirection1OK_p ? firstDirection1_p
               : fillFirstDirection1();
    }
    virtual const MDirection& firstDirection1()  const {
        return This->firstDirection1();
    }
    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const;
    virtual Vector<Float> parang(Double time) const;

    // Note that azel is a function instead of a cached value
    virtual MDirection azel0(Double time) const;
    virtual Vector<Double>& azel0Vec(Double time, Vector<Double>& azelVec) const;
    virtual Vector<MDirection> azel(Double time) const;
    virtual Matrix<Double>& azelMat(Double time, Matrix<Double>& azelMat) const;

    // Hour angle for specified time
    virtual Double hourang(Double time) const;

    virtual Int fieldId() const {
        return fieldIdOK_p ? fieldId_p : This->fillFieldId();
    }

    virtual Int& fieldIdRef() {
        return fieldIdOK_p ? fieldId_p : This->fillFieldId();
    }

    virtual Int& arrayIdRef() {
        return arrayIdOK_p ? arrayId_p : This->fillArrayId();
    }

    virtual Int arrayId() const {
        return arrayIdOK_p ? arrayId_p : This->fillArrayId();
    }

    // Return flag for each channel & row
    virtual Matrix<Bool>& flag() {
        return flagOK_p ? flag_p : fillFlag();
    }
    virtual const Matrix<Bool>& flag() const {
        return This->flag();
    }

    // Return flag for each polarization, channel and row
    virtual Cube<Bool>& flagCube() {
        return flagCubeOK_p ? flagCube_p : fillFlagCube();
    }
    virtual const Cube<Bool>& flagCube() const {
        return This->flagCube();
    }

    virtual Vector<Bool>& flagRow() {
        return flagRowOK_p ? flagRow_p : fillFlagRow();
    }
    virtual const Vector<Bool>& flagRow() const {
        return This->flagRow();
    }

    // Return flags for each polarization, channel, category, and row.
    virtual Array<Bool>& flagCategory() {
        return flagCategoryOK_p ? flagCategory_p : fillFlagCategory();
    }
    virtual const Array<Bool>& flagCategory() const {
        return This->flagCategory();
    }

    virtual Vector<Int>& scan() {
        return scanOK_p ? scan_p : fillScan();
    }
    virtual const Vector<Int>& scan() const {
        return This->scan();
    }

    // scalar version for convenience, when scan known constant for
    // entire iteration/buffer.
    virtual Int scan0() {
        return scan()(0);
    }

    virtual Vector<Int>& processorId() {
        return processorIdOK_p ? processorId_p : fillProcessorId();
    }
    virtual const Vector<Int>& processorId() const {
        return This->processorId();
    }

    virtual Vector<Int>& observationId() {
        return observationIdOK_p ? observationId_p : fillObservationId();
    }
    virtual const Vector<Int>& observationId() const {
        return This->observationId();
    }

    virtual Vector<Int>& stateId() {
        return stateIdOK_p ? stateId_p : fillStateId();
    }
    virtual const Vector<Int>& stateId() const {
        return This->stateId();
    }

    // Gets SPECTRAL_WINDOW/CHAN_FREQ (in Hz, acc. to the MS def'n v.2).
    virtual Vector<Double>& frequency() {
        return frequencyOK_p ? frequency_p : fillFreq();
    }
    virtual const Vector<Double>& frequency() const {
        return This->frequency();
    }

//    virtual Vector<Double>& lsrFrequency() {
//        return lsrFrequencyOK_p ? lsrFrequency_p : fillLSRFreq();
//    }
//    virtual const Vector<Double>& lsrFrequency() const {
//        return This->lsrFrequency();
//    }


    //the following method is to convert the observed frequencies
    // This conversion may not be accurate for some frame
    // conversion like topo to lsr except if the spw is in the actual buffer

    //if ignoreconv=True..frequency is served as is in the data frame
    virtual void lsrFrequency(const Int & spw, Vector<Double>& freq, Bool & convert, const Bool ignoreconv=False) const;

    virtual Int numberCoh () const;

    virtual MDirection & phaseCenter() {
        return phaseCenterOK_p ? phaseCenter_p : fillPhaseCenter();
    }
    virtual MDirection phaseCenter() const {
        return This->phaseCenter();
    }

    virtual Int polFrame() const {
        return polFrameOK_p ? polFrame_p : This->fillPolFrame();
    }

    virtual Vector<Int>& corrType() {
        return corrTypeOK_p ? corrType_p : fillCorrType();
    }
    virtual const Vector<Int>& corrType() const {
        return This->corrType();
    }

    virtual Vector<Float>& sigma() {
        return sigmaOK_p ? sigma_p : fillSigma();
    }
    virtual const Vector<Float>& sigma() const {
        return This->sigma();
    }

    virtual Matrix<Float>& sigmaMat() {
        return sigmaMatOK_p ? sigmaMat_p : fillSigmaMat();
    }
    virtual const Matrix<Float>& sigmaMat() const {
        return This->sigmaMat();
    }

    virtual Int & spectralWindow() {
        return spectralWindowOK_p ? spectralWindow_p : This->fillSpW();
    }
    virtual Int spectralWindow() const {
        return spectralWindowOK_p ? spectralWindow_p : This->fillSpW();
    }
    virtual Int polarizationId() const {
      return visIter_p->polarizationId();
    } 
    virtual Int& dataDescriptionIdRef() {
      return dataDescriptionIdOK_p ? dataDescriptionId_p : This->fillDataDescriptionId ();
    }
    virtual Int dataDescriptionId() const {
      return dataDescriptionIdOK_p ? dataDescriptionId_p : This->fillDataDescriptionId ();
    }
    virtual Vector<Double>& time() {
        return timeOK_p ? time_p : fillTime();
    }
    virtual const Vector<Double>& time() const {
        return This->time();
    }

    virtual Vector<Double>& timeCentroid() {
        return timeCentroidOK_p ? timeCentroid_p : fillTimeCentroid();
    }
    virtual const Vector<Double>& timeCentroid() const {
        return This->timeCentroid();
    }

    virtual Vector<Double>& timeInterval() {
        return timeIntervalOK_p ? timeInterval_p : fillTimeInterval();
    }
    virtual const Vector<Double>& timeInterval() const {
        return This->timeInterval();
    }

    virtual Vector<Double>& exposure() {
        return exposureOK_p ? exposure_p : fillExposure();
    }
    virtual const Vector<Double>& exposure() const {
        return This->exposure();
    }

    virtual Vector<RigidVector<Double, 3> >& uvw() {
        return uvwOK_p ? uvw_p : filluvw();
    }
    virtual const Vector<RigidVector<Double, 3> >& uvw() const {
        return This->uvw();
    }

    virtual Matrix<Double>& uvwMat() {
        return uvwMatOK_p ? uvwMat_p : filluvwMat();
    }
    virtual const Matrix<Double>& uvwMat() const {
        return This->uvwMat();
    }

    virtual Matrix<CStokesVector>& visibility() {
        return visibilityOK_p ? visibility_p : fillVis(VisibilityIterator::Observed);
    }
    virtual const Matrix<CStokesVector>& visibility() const {
        return This->visibility();
    }

    virtual Matrix<CStokesVector>& modelVisibility() {
        return modelVisibilityOK_p ? modelVisibility_p :
               fillVis(VisibilityIterator::Model);
    }
    virtual const Matrix<CStokesVector>& modelVisibility() const {
        return This->modelVisibility();
    }

    virtual Matrix<CStokesVector>& correctedVisibility() {
        return correctedVisibilityOK_p ? correctedVisibility_p :
               fillVis(VisibilityIterator::Corrected);
    }
    virtual const Matrix<CStokesVector>& correctedVisibility() const {
        return This->correctedVisibility();
    }

    virtual Cube<Complex>& visCube() {
        return visCubeOK_p ? visCube_p : fillVisCube(VisibilityIterator::Observed);
    }
    virtual const Cube<Complex>& visCube() const {
        return This->visCube();
    }

    virtual Cube<Complex>& modelVisCube() {
        return modelVisCubeOK_p ? modelVisCube_p :
               fillVisCube(VisibilityIterator::Model);
    }

    virtual Cube<Complex>& modelVisCube(const Bool & matchVisCubeShape) {
        // Avoids call to fillVisCube(VisIter::Model)
        modelVisCubeOK_p = True;
        if (matchVisCubeShape) {
            // shape it in memory like data
            modelVisCube_p.resize(visCube().shape());
            modelVisCube_p = Complex(0.0);
        }
        // ... and return it in the usual way
        return modelVisCube();
    }

    virtual const Cube<Complex>& modelVisCube() const {
        return This->modelVisCube();
    }

    virtual Cube<Complex>& correctedVisCube() {
        return correctedVisCubeOK_p ? correctedVisCube_p :
               fillVisCube(VisibilityIterator::Corrected);
    }
    virtual const Cube<Complex>& correctedVisCube() const {
        return This->correctedVisCube();
    }

    // Return visCube(), modelVisCube(), or correctedVisCube(),
    // according to whichcol.  They throw an exception if whichcol is
    // unsupported, but note the encouraged default.
    // TODO: Optionally return DATA if whichcol is unavailable.
    Cube<Complex>& dataCube(const MS::PredefinedColumns whichcol=MS::DATA);
    const Cube<Complex>& dataCube(const MS::PredefinedColumns
                                  whichcol=MS::DATA) const;

    virtual Cube<Float>& floatDataCube() {
        return floatDataCubeOK_p ? floatDataCube_p : fillFloatDataCube();
    }
    virtual const Cube<Float>& floatDataCube() const {
        return This->floatDataCube();
    }

    // Returns the weights for each row averaged over the parallel hand correlations.
    virtual Vector<Float>& weight() {
        return weightOK_p ? weight_p : fillWeight();
    }
    virtual const Vector<Float>& weight() const {
        return This->weight();
    }

    // Returns the nPol_p x curNumRow_p weight matrix.
    virtual Matrix<Float>& weightMat() {
        return weightMatOK_p ? weightMat_p : fillWeightMat();
    }
    virtual const Matrix<Float>& weightMat() const {
        return This->weightMat();
    }

    // Is a valid WEIGHT_SPECTRUM available?
    virtual Bool existsWeightSpectrum() const {
      return visIter_p ? visIter_p->existsWeightSpectrum() : false;
    }

    virtual Cube<Float>& weightSpectrum() {
        return weightSpectrumOK_p ? weightSpectrum_p : fillWeightSpectrum();
    }
    virtual const Cube<Float>& weightSpectrum() const {
        return This->weightSpectrum();
    }

    virtual Matrix<Float>& imagingWeight();
    virtual const Matrix<Float>& imagingWeight() const;


//    virtual Matrix<Float>& imagingWeight() {
//        return imagingWeightOK_p ? imagingWeight_p : fillImagingWeight();
//    }
//    virtual const Matrix<Float>& imagingWeight() const {
//        return This->imagingWeight();
//    }

    virtual Cube<Float>& weightCube() {
        return weightCube_p;
    }
    //</group>

    //<group>
    // Utility functions to provide coordinate or column ranges of the
    // data in the VisBuffer. Flagging is applied before computing the ranges.
    //
    // Generic accessor to column ranges of integer type, as specified by
    // enumerations defined in class MSCalEnums. Throws an exception
    // if the enum is not for a recognized integer column.
    virtual Vector<Int> vecIntRange(const MSCalEnums::colDef & calEnum) const;

    // Antenna id. range (includes both ANTENNA1 and ANTENNA2 columns)
    virtual Vector<Int> antIdRange() const;

    // Time range
    virtual Bool timeRange(MEpoch & rTime, MVEpoch & rTimeEP, MVEpoch & rInterval) const;

    // Return the row Ids from the original ms. If the ms used is a subset of
    // another ms then rowIds() return the row ids of the original ms.
    virtual Vector<uInt>& rowIds();

    virtual const Vector<uInt>& rowIds() const {
        return This->rowIds();
    };

    //</group>

    // Frequency average the buffer (visibility() column only)
    virtual void freqAverage();

    // Frequency average the buffer (visCube and [if present] modelVisCube)
    void freqAveCubes();

    // Average channel axis according to chanavebounds, for whichever of DATA,
    // MODEL_DATA, CORRECTED_DATA, FLOAT_DATA, FLAG, and WEIGHT_SPECTRUM are
    // present.  It will only treat the first 5 as present if they have already
    // been loaded into the buffer!
    void channelAve(const Matrix<Int>& chanavebounds,Bool calmode=True);

    // Average channel axis by factor.
    template<class T> void chanAveVisCube(Cube<T>& data, Int nChanOut);

    // Accumulate channel axis by factor, without applying WEIGHT_SPECTRUM even
    // if it is present.
    // It is primarily intended for averaging WEIGHT_SPECTRUM itself.
    template<class T> void chanAccCube(Cube<T>& data, Int nChanOut);

    // This defaults to no conceptual side effects, but usually it is more
    // efficient to let it leave weightSpectrum() in a channel averaged state.
    // restoreWeightSpectrum has no effect if !existsWeightSpectrum().
    virtual void chanAveFlagCube(Cube<Bool>& flagcube, const Int nChanOut,
                         const Bool restoreWeightSpectrum = True);

    // Doesn't do anything if flagcat is degenerate.
    void chanAveFlagCategory(Array<Bool>& flagcat, const Int nChanOut);

    // Form Stokes parameters from correlations
    //  (these are preliminary versions)
    virtual void formStokes();
    virtual void formStokesWeightandFlag();
    virtual void formStokes(Cube<Complex>& vis);
    virtual void formStokes(Cube<Float>& fcube);    // Will throw up if asked to do all 4.

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    virtual void sortCorr();
    virtual void unSortCorr();

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    virtual void normalize(const Bool & phaseOnly = False);

    // Fill weightMat according to sigma column
    virtual void resetWeightMat();

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)
    virtual void phaseCenterShift(const Vector<Double>& phase);
    // Rotate visibility phase for phase center offsets (arcsecs)
    virtual void phaseCenterShift(Double dx, Double dy);

    // Update coordinate info - useful for copied VisBuffers that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows. 
    // if dirDependent is set to False the expensive direction dependent calculation of parallactic or direction of
    // antenna from pointing table is avoided
    //Add more as needed.
    virtual void updateCoordInfo(const VisBuffer * vb = NULL, const Bool dirDependent=True);
    void copyCoordInfo(const VisBuffer& other, Bool force=False);

    // Set the visibility to a constant, note that this only changes the buffer,
    // no values are written back to tables from here.
    virtual void setVisCube(Complex c);
    virtual void setModelVisCube(Complex c);
    virtual void setCorrectedVisCube(Complex c);

    // Set the visibility, note that this only changes the buffer,
    // no values are written back to tables from here.
    virtual void setVisCube(const Cube<Complex>& vis);
    virtual void setModelVisCube(const Cube<Complex>& vis);
    virtual void setCorrectedVisCube(const Cube<Complex>& vis);

    // Like the above, but for FLOAT_DATA, keeping it as real floats.
    virtual void setFloatDataCube(const Cube<Float>& fcube);

    // Set model according to a Stokes vector
    virtual void setModelVisCube(const Vector<Float>& stokes);

    // Reference external model visibilities
    virtual void refModelVis(const Matrix<CStokesVector>& mvis);

    // Remove scratch cols data from vb
    virtual void removeScratchCols();

    // Access the current ROMSColumns object via VisIter
    virtual const ROMSColumns & msColumns() const {
        return visIter_p->msColumns();
    }

    virtual  Int numberAnt () const;



    // Get all selected spectral windows not just the one in the actual buffer
    virtual void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan) {
        visIter_p->allSelectedSpectralWindows(spws, nvischan);
    }

  virtual void getChannelSelection(Block< Vector<Int> >& blockNGroup,
				   Block< Vector<Int> >& blockStart,
				   Block< Vector<Int> >& blockWidth,
				   Block< Vector<Int> >& blockIncr,
				   Block< Vector<Int> >& blockSpw) const {
    visIter_p->getChannelSelection(blockNGroup, blockStart, blockWidth, blockIncr, blockSpw);
  };
    void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan) const {
        This->allSelectedSpectralWindows(spws, nvischan);
    }

    // Return the actual msid, useful if using multiple ms to monitor which
    // ms in the  list is being dealt with
    virtual Int msId() const {
        This->checkMSId();
        return oldMSId_p;
    }

    //checked if the ms has changed since the last chunk processed
    virtual Bool newMS() const {
        This->checkMSId();
        return newMS_p;
    }
    //get the name of the ms the buffer is at empty string if no visiter is attached
    virtual String msName(Bool stripPath=False) const;

    virtual Bool newArrayId () const;
    virtual Bool newFieldId () const;
    virtual Bool newSpectralWindow () const;

    //

protected:

    virtual Bool checkMSId();
    virtual void checkVisIter (const char * func, const char * file, int line, const char * extra = "") const;
    virtual void copyCache (const VisBuffer & other, Bool force);
    virtual void copyMsInfo (Int & msID, Bool & MsOk_p, Bool & newMs) const;
    virtual const Matrix<Float>& imagingWeight(const VisImagingWeight & weightGenerator) const;
    virtual Int getOldMsId () const;
    virtual ROVisibilityIterator * getVisibilityIterator () const;

    template <typename Coord>
    void updateCoord (const VisBuffer * other,
                      Bool otherOk,
                      const Coord & (VisBuffer::* getCoord) () const,
                      Coord & coord,
                      Bool & coordOk)
    {
        if (otherOk){
            coord.assign ((other ->* getCoord) ());
            coordOk = True;
        }
        else {
            ((this ->* getCoord) ());
        }
    }

    template <typename Scalar>
    void updateCoordS (const VisBuffer * other,
                       Bool otherOk,
                       Scalar (VisBuffer::* getCoord) () const,
                       Scalar & coord,
                       Bool & coordOk)
    {
        if (otherOk){
            coord = (other ->* getCoord) ();
            coordOk = True;
        }
        else {
            ((this ->* getCoord) ());
        }
    }


private:


    // validate the cache
    virtual void validate();

    void checkVisIterBase (const char * func, const char * file, int line, const char * extra = "") const;

    template<typename T>
    static void cacheCopyArray (Bool & newStatus,
                                Bool oldStatus,
                                T & newCache,
                                const VisBuffer & other,
                                const T &  (VisBuffer::* oldCache) () const,
                                Bool force) {

        // Leave things unchanged if the old status is false.  This will often
        // leave the value with an empty data structure and an OK status which
        // is needed in many cases.
        //
        // For copying Array<T> derived objects since the assign operator
        // doesn't work for these.

        newStatus = force || oldStatus;

        if (newStatus) {
            newCache.assign (((& other) ->* oldCache) ());
        }
    }

    template<typename T>
    static void cacheCopyNormal (Bool & newStatus,
                                 Bool oldStatus,
                                 T & newCache,
                                 const VisBuffer & other,
                                 T (VisBuffer::* oldCache) () const,
                                 Bool force) {

        // Leave things unchanged if the old status is false.  This will often
        // leave the value with an empty data structure and an OK status which
        // is needed in many cases.
        //
        // For copying "normal" cache status and values.  Normal means
        // the assign operator works (which it doesn't for classes based on Array<T>)

        newStatus = force || oldStatus;

        if (newStatus) {

            newCache = ((& other) ->* oldCache) ();
        }
    }

    virtual void setAllCacheStatuses (bool status);

    virtual Bool nonCanonCorr(); // Are correlations in non-canonical order?

    // Filter index arrays for unique elements
    virtual Vector<Int> unique(const Vector<Int>& indices) const;

    Bool corrSorted_p; // Have correlations been sorted by sortCorr?
    VbDirtyComponents dirtyComponents_p;
    Int lastPointTableRow_p;
    Int oldMSId_p;
    VisBuffer * This;
    Bool twoWayConnection_p;
    ROVisibilityIterator * visIter_p;
    CountedPtr <VisModelDataI> visModelData_p;

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

    virtual Vector<Int>& fillAnt1();
    virtual Vector<Int>& fillAnt2();
    virtual Int & fillArrayId();
    //virtual Matrix<Int>& fillChanAveBounds();
    virtual Vector<Int>& fillChannel();
    virtual Vector<SquareMatrix<Complex, 2> >& fillCjones();
    virtual Vector<Int>& fillCorrType();
    virtual Int & fillDataDescriptionId ();
    virtual Vector<MDirection>& fillDirection1();
    virtual MDirection& fillFirstDirection1();
    virtual Vector<MDirection>& fillDirection2();
    virtual Vector<Double>& fillExposure();
    virtual Vector<Int>& fillFeed1();
    virtual Vector<Float>& fillFeed1_pa();
    virtual Vector<Int>& fillFeed2();
    virtual Vector<Float>& fillFeed2_pa();
    virtual Int & fillFieldId();
    virtual Matrix<Bool>& fillFlag();
    virtual Array<Bool>& fillFlagCategory();
    virtual Cube<Bool>& fillFlagCube();
    virtual Vector<Bool> & fillFlagRow();
    virtual Cube<Float>& fillFloatDataCube();
    virtual Vector<Double>& fillFreq();         // Puts SPECTRAL_WINDOW/CHAN_FREQ in frequency_p.
    //virtual Matrix<Float>& fillImagingWeight();
    //virtual Vector<Double>& fillLSRFreq();
    virtual Int & fillnChannel();
    virtual Int & fillnCorr();
  //    virtual Int & fillnCat();
    virtual Int & fillnRow();
    virtual Vector<Int> & fillObservationId();
    virtual MDirection & fillPhaseCenter();
    virtual Int & fillPolFrame();
    virtual Vector<Int> & fillProcessorId();
    virtual Vector<Int> & fillScan();
    virtual Vector<Float>& fillSigma();
    virtual Matrix<Float>& fillSigmaMat();
    virtual Int & fillSpW();
    virtual Vector<Int> & fillStateId();
    virtual Vector<Double>& fillTime();
    virtual Vector<Double>& fillTimeCentroid();
    virtual Vector<Double>& fillTimeInterval();
    virtual Vector<RigidVector<Double, 3> >& filluvw();
    virtual Matrix<Double>& filluvwMat();
    virtual Matrix<CStokesVector>& fillVis(VisibilityIterator::DataColumn whichOne);
    virtual Cube<Complex>& fillVisCube(VisibilityIterator::DataColumn whichOne);
    virtual Vector<Float>& fillWeight();
    virtual Matrix<Float>& fillWeightMat();
    virtual Cube<Float>& fillWeightSpectrum();

    Bool newMS_p;

    // Variables to track validity of cache (alphabetical order)

#define CacheStatus(item) \
virtual Bool item ## OK () const\
{\
    return item ## OK_p;\
}\
Bool item ## OK_p;

    // Define the cache statuses
    //
    // For example, CacheStatus (antenna1) defines:
    //
    // virtual Bool antenna1OK () const { return antenna1OK_p;}
    //
    // and
    //
    // Bool antenna1OK_p;

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
    CacheStatus (firstDirection1);
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

    // Cached values (alphabetical order)

    Vector<Int> antenna1_p;
    Vector<Int> antenna2_p;
    Int arrayId_p;
    Matrix<Int> chanAveBounds_p;
    Vector<Int> channel_p;
    Vector<SquareMatrix<Complex, 2> > cjones_p;
    Cube<Complex> correctedVisCube_p;
    Matrix<CStokesVector> correctedVisibility_p;
    Vector<Int> corrType_p;
    Int dataDescriptionId_p;
    Vector<MDirection> direction1_p; //where the first antenna/feed is pointed to
    Vector<MDirection> direction2_p; //where the second antenna/feed is pointed to
    MDirection firstDirection1_p;
    Vector<Double> exposure_p;
    Vector<Int> feed1_p;
    Vector<Float> feed1_pa_p;
    Vector<Int> feed2_p;
    Vector<Float> feed2_pa_p;
    Int fieldId_p;
    Matrix<Bool> flag_p;
    Array<Bool> flagCategory_p;
    Cube<Bool> flagCube_p;
    Vector<Bool> flagRow_p;
    Cube<Float> floatDataCube_p;
    Vector<Double> frequency_p;
    mutable Matrix<Float> imagingWeight_p;
    //Vector<Double> lsrFrequency_p;
    Cube<Complex> modelVisCube_p;
    Matrix<CStokesVector> modelVisibility_p;
    Int nChannel_p;
    Int nCorr_p;
  //    Int nCat_p;
    Int nRow_p;
    Vector<Int> observationId_p;
    MDirection phaseCenter_p;
    Int polFrame_p;
    Vector<Int> processorId_p;
    Vector<uInt> rowIds_p;
    Vector<Int> scan_p;
    Vector<Float> sigma_p;
    Matrix<Float> sigmaMat_p;
    Int spectralWindow_p;
    Vector<Int> stateId_p;
    Vector<Double> time_p;
    Vector<Double> timeCentroid_p;
    Vector<Double> timeInterval_p;
    Vector<RigidVector<Double, 3> > uvw_p;
    Matrix<Double> uvwMat_p;
    Cube<Complex> visCube_p;
    Matrix<CStokesVector> visibility_p;
    Vector<Float> weight_p;
    Cube<Float> weightCube_p;
    Matrix<Float> weightMat_p;
    Cube<Float> weightSpectrum_p;

};

// <summary>
// A convenience class to assist in migrating code to potentially use
// asynchronous I/O.
//
// <prerequisite>
//   <li> <linkto class="VisBuffer">VisBuffer</linkto>
//   <li> <linkto class="VisBufferAsync">VisBufferAsync</linkto>
//   <li> <linkto class="ROVisibilityIterator">ROVisibilityIterator</linkto>
// </prerequisite>
//
// </summary>
//
// <synopsis>
//
// When existing code is modified to potentially use asynchronous I/O the current
// VisBuffer usage is probably using automatic (stack) storage which will have to
// be replaced to allow VisBufferAsync objects (which derive from VisBuffer) to be
// used with asynchronous I/O.  The goal of this class is to make that transition
// easier.  The user will replace their existing declaration of a VisBuffer object
// with a declaration of a VisBufferAutoPtr object.  Depending on the attributes
// of the VisBuffer reference/pointer or the ROVisibilityIterator provided in the
// VisBufferAutoPtr constructor, the appropriate type of VisBuffer will be created
// dynamically.  The VisBufferAutoPtr will also function somewhat like an auto_ptr
// and delete the underlying object when the VisBufferAutoPtr object is destroyed.
//
// Once the straight VisBuffer declaration is replaced, then the code in its scope
// will need to be modified to dereference the VisBufferAutoPtr or to delete use
// of the address-of operator, "&", applied to the previous VisBuffer variable.
// See the example below.
//
// </synopsis>
// <example>
//
// <code>
// // Before adding asynchronous I/O support
//
// VisBuffer vb (vi);
//
// doSomething (vb);        // void doSomething (VisBuffer &);
// doSomethingElse (& vb);  // void doSomethingElse (VisBuffer *);
//
// // After adding asynchronous I/O support
//
// VisBufferAutoPtr vb (vi);
//
// doSomething (* vb);
// doSomethingElse (vb.get());
//
// </code>
// </example>
//
class VisBufferAutoPtr {

public:

    VisBufferAutoPtr ();
    VisBufferAutoPtr (VisBufferAutoPtr & other);
    explicit VisBufferAutoPtr (VisBuffer &);
    explicit VisBufferAutoPtr (VisBuffer *);
    explicit VisBufferAutoPtr (ROVisibilityIterator * rovi);
    explicit VisBufferAutoPtr (ROVisibilityIterator & rovi);
    ~VisBufferAutoPtr ();

    VisBufferAutoPtr & operator= (VisBufferAutoPtr & other);
    VisBuffer & operator* () const;
    VisBuffer * operator-> () const;

    VisBuffer * get () const;
    VisBuffer * release ();
    void set (VisBuffer &);
    void set (VisBuffer *);
    void set (ROVisibilityIterator * rovi, Bool attachIt = False);
    void set (ROVisibilityIterator & rovi, Bool attachIt = False);

protected:

    void construct (ROVisibilityIterator * rovi, Bool attachVi);
    void constructVb (VisBuffer * rovi);

private:

    VisBuffer * visBuffer_p;

};

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <msvis/MSVis/VisBuffer.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif


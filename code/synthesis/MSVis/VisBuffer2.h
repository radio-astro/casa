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

#ifndef MSVIS_VISBUFFER2_H
#define MSVIS_VISBUFFER2_H

#include <casa/aips.h>

#warning "Remove next line later"
#    include <synthesis/MSVis/VisBuffer.h>

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>
#include <synthesis/MSVis/VisBufferComponents.h>
#include <synthesis/MSVis/VisibilityIterator2.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#forward

// <summary>
// VbDirtyComponents allows marking portions of a VisBuffer2 as
// modified (aka dirty).  This feature is needed for the Visibility
// Processing Framework (VPF) which allows a sequence of data processing
// nodes to work as a bucket brigade operating sequentially on a
// VisBuffer2.  A downstream output node needs to know what data,
// if any, needs to be written out.
//
// <prerequisite>
//   #<li><linkto class="VisBuffer2">VisBuffer2</linkto>
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

class VbDirtyComponents;
class ROVisibilityIterator2;
class VisibilityIterator2;

//<summary>VisBuffer2s encapsulate one chunk of visibility data for processing.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisSet">VisSet</linkto>
//   <li> <linkto class="VisibilityIterator">VisibilityIterator</linkto>
//   <li> <linkto class="VisBuffer2AutoPtr">VisBuffer2AutoPtr</linkto>
//   <li> <linkto class="VbDirtyComponents">VbDirtyComponents</linkto>
// </prerequisite>
//
// <etymology>
// VisBuffer2 is a buffer for visibility data
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
// for more details on how the VisBuffer2 is to be used.
//
// When the user intends to allow asynchronous I/O they will need to use
// the VisBuffer2Async and VisBuffer2AsyncWrapper classes; these are
// publicly derived from VisBuffer2.  Normally, the user should not
// explicitly use these classes but should use the helper class
// VisBuffer2AutoPtr which will ensure that the appropriate class is
// used.
//</synopsis>

//<todo>
// <li> reconcile vis/visCube usage: visCube, flagCube and weightMatrix
// are currently only correct when this VisBuffer2 got them from a
// VisIter, operations like -=, freqAverage() are only done for
// visibility() and flag().
//</todo>

class VisBuffer2 {

public:
    // Create empty VisBuffer2 you can assign to or attach.
    VisBuffer2();
    // Construct VisBuffer2 for a particular VisibilityIterator
    // The buffer will remain synchronized with the iterator.
    VisBuffer2(ROVisibilityIterator2 & iter);

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).
    VisBuffer2(const VisBuffer2 & vb);

    // Destructor (detaches from VisIter)
    ~VisBuffer2();

    // Assignment, loses synchronization with iterator: only use buffer for
    // current iteration (or reattach)

    VisBuffer2 & operator=(const VisBuffer2 & vb);

    // Assignment, optionally without copying the data across; with copy=True
    // this is identical to normal assignment operator

    void assign(const VisBuffer2 & vb, Bool copy = True);

    VisBuffer2 * clone () const;

    const ROVisibilityIterator2 * getVi () const;

    void invalidate();

    // subtraction: return the difference of the visibilities, flags of
    // this and other are or-ed. An exception is thrown if the number of
    // rows or channels differs, but no further checks are done.
    //-->Not called: VisBuffer2 & operator-=(const VisBuffer2 & vb);

    // Invalidate the cache

    // <group>
    // Access functions
    //
    //     return nCatOK_p ? nCat_p : fillnCat();
    // void setNCat (const Int &))) {
    // }
    // Int nCat() const {
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

    ////--> shouldn't be settable: void setNRow (const Int &);

    /////--> provided by VI: Int nRowChunk() const;

    void dirtyComponentsAdd (const VbDirtyComponents & additionalDirtyComponents);
    void dirtyComponentsAdd (VisBufferComponents::EnumType component);
    void dirtyComponentsClear ();
    VbDirtyComponents dirtyComponentsGet () const;
    void dirtyComponentsSet (const VbDirtyComponents & dirtyComponents);
    void dirtyComponentsSet (VisBufferComponents::EnumType component);

    //--> This needs to be removed: Bool fetch(const asyncio::PrefetchColumns *pfc);

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBuffer2 only to benefit from caching of the feed and antenna IDs.

    // Note that feed_pa is a function instead of a cached value
    Vector<Float> feed_pa(Double time) const;

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.


    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    Float parang0(Double time) const;
    Vector<Float> parang(Double time) const;

    MDirection azel0(Double time) const; // function rather than cached value
    void azel0Vec(Double time, Vector<Double>& azelVec) const;
    Vector<MDirection> azel(Double time) const;
    void azelMat(Double time, Matrix<Double>& azelMat) const;

    // Hour angle for specified time
    Double hourang(Double time) const;

//    const Vector<Double>& lsrFrequency() const;
//    void setLsrFrequency (const Vector<Double>&);


    //the following method is to convert the observed frequencies
    // This conversion may not be accurate for some frame
    // conversion like topo to lsr except if the spw is in the actual buffer

    //if ignoreconv=True..frequency is served as is in the data frame
    //void lsrFrequency(const Int & spw, Vector<Double>& freq, Bool & convert, const Bool ignoreconv = False) const;

//    const Matrix<Float>& imagingWeight() const;
//    void setImagingWeight (const Matrix<Float>&);

    //</group>

    //<group>
    // Utility functions to provide coordinate or column ranges of the
    // data in the VisBuffer2. Flagging is applied before computing the ranges.
    //
    // Generic accessor to column ranges of integer type, as specified by
    // enumerations defined in class MSCalEnums. Throws an exception
    // if the enum is not for a recognized integer column.
    //--> Low utility?: Vector<Int> vecIntRange(const MSCalEnums::colDef & calEnum) const;

    // Antenna id. range (includes both ANTENNA1 and ANTENNA2 columns)
    //--> Low utility?: Vector<Int> antIdRange() const;

    // Time range

    //--> Low utility?: Bool timeRange(MEpoch & rTime, MVEpoch & rTimeEP, MVEpoch & rInterval) const;

    // Return the row Ids from the original ms. If the ms used is a subset of
    // another ms then rowIds() return the row ids of the original ms.

    //--> Belongs in VI: void setRowIds (const Vector<uInt>&);
    //--> Belongs in VI: const Vector<uInt>& rowIds() const;

    //</group>

    // Frequency average the buffer (visibility() column only)
    //--> Not called: void freqAverage();

    // Frequency average the buffer (visCube and [if present] modelVisCube)
    void freqAveCubes();

    // Average channel axis according to chanavebounds, for whichever of DATA,
    // MODEL_DATA, CORRECTED_DATA, FLOAT_DATA, FLAG, and WEIGHT_SPECTRUM are
    // present.  It will only treat the first 5 as present if they have already
    // been loaded into the buffer!
    void averageChannels (const Matrix<Int>& chanavebounds);

    Double getFrequency (Int rowInBuffer, Int channelIndex,
                         FrequencySelection::FrameOfReference frame = FrequencySelection::AsSelected) const;
    Int getChannelNumber (Int rowInBuffer, Int channelIndex) const;

    // Form Stokes parameters from correlations
    //  (these are preliminary versions)
    //--> Not called: void formStokes();
    //--> Not called: void formStokesWeightandFlag();
    //--> Not called: void formStokes(Cube<Complex>& vis);
    //--> Not called: void formStokes(Cube<Float>& fcube);    // Will throw up if asked to do all 4.

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    void sortCorr ();
    void unSortCorr();

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    void normalize(Bool phaseOnly = False);

    // Fill weightMat according to sigma column
    void resetWeightsUsingSigma ();//void resetWeightMat();

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)

    //--> Called by routine in FixVis but that routine is not called:
    // void phaseCenterShift(const Vector<Double>& phase);
    // Rotate visibility phase for phase center offsets (arcsecs)
    //--> Not called: void phaseCenterShift(Double dx, Double dy);

    // Update coordinate info - useful for copied VisBuffer2s that need
    // to retain some state for later reference.
    // Presently this fills antenna, array, field and spectralWindow ids, time,
    // frequency and number of rows.
    // if dirDependent is set to False the expensive direction dependent calculation of parallactic or direction of
    // antenna from pointing table is avoided
    //Add more as needed.
    void updateCoordInfo(const VisBuffer2 * vb = NULL, const Bool dirDependent = True);
    void copyCoordInfo(const VisBuffer2 & other, Bool force = False);

    //--> Not called: void setCorrectedVisCube(const Cube<Complex>& vis);
    // Like the above, but for FLOAT_DATA, keeping it as real floats.

    //--> Not called: void setFloatDataCube(const Cube<Float>& fcube);
    // Reference external model visibilities
    //--> Not called: void refModelVis(const Matrix<CStokesVector>& mvis);

    // Remove scratch cols data from vb
    //--> Not called: void removeScratchCols();

    // Access the current ROMSColumns object via VisIter
    //--> Access using VI: const ROMSColumns & msColumns() const;

    //--> Access via VI:  Int numberAnt () const;



    // Get all selected spectral windows not just the one in the actual buffer
    //--> Get from VI: void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);

    //--> Get from VI: void getChannelSelection(Block< Vector<Int> >& blockNGroup,
    //				   Block< Vector<Int> >& blockStart,
    //			   Block< Vector<Int> >& blockWidth,
    //				   Block< Vector<Int> >& blockIncr,
    //				   Block< Vector<Int> >& blockSpw) const;;
    //--> Get from VI: void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan) const;

    Bool isNewArrayId () const;
    Bool isNewFieldId () const;
    Bool isNewMs() const;
    Bool isNewSpectralWindow () const;
    Bool isWritable () const;
    Int msId() const;
    String msName (Bool stripPath = False) const;

    //////////////////////////////////////////////////////////////////////
    //
    //  Data accessors and setters (where appropriate)
    //
    //  There are the methods that allows access to the items cached in the
    //  VisBuffer.  The straight accessors provide read-only access to the
    //  item.  Where the item is allowed to be modified, one or more set
    //  methods are provided.

    const Vector<Int> & antenna1 () const;
    const Vector<Int> & antenna2 () const;
    Int arrayId () const;
    const Vector<Int> & channel () const;
    const Vector<SquareMatrix<Complex, 2> > & cjones () const;
    const Cube<Complex> & correctedVisCube () const;
    void setCorrectedVisCube (const Cube<Complex> &);
    const Matrix<CStokesVector> & correctedVisibility () const;
    void setCorrectedVisibility (const Matrix<CStokesVector> &);
    const Vector<Int> & corrType () const;
    Int dataDescriptionId () const;
    const Vector<MDirection> & direction1 () const;
    const Vector<MDirection> & direction2 () const;
    const Vector<Double> & exposure () const;
    const Vector<Int> & feed1 () const;
    const Vector<Float> & feed1_pa () const;
    const Vector<Int> & feed2 () const;
    const Vector<Float> & feed2_pa () const;
    Int fieldId () const;
    const Matrix<Bool> & flag () const;
    void setFlag (const Matrix<Bool>&);
    const Array<Bool> & flagCategory () const;
    void setFlagCategory (const Array<Bool>&);
    const Cube<Bool> & flagCube () const;
    void setFlagCube (const Cube<Bool>&);
    const Vector<Bool> & flagRow () const;
    void setFlagRow (const Vector<Bool>&);
    const Cube<Float> & floatDataCube () const;
    void setFloatDataCube (const Cube<Float> &);
    const Vector<Double> & frequency () const;
    const Matrix<Float> & imagingWeight () const;
    const Cube<Complex> & modelVisCube () const;
    void setModelVisCube(const Complex & c);
    void setModelVisCube(const Cube<Complex>& vis);
    void setModelVisCube(const Vector<Float>& stokes);
    const Matrix<CStokesVector> & modelVisibility () const;
    void setModelVisibility (Matrix<CStokesVector> &);
    Int nChannel () const;
    Int nCorr () const;
    Int nRow () const;
    const Vector<Int> & observationId () const;
    const MDirection& phaseCenter () const;
    Int polFrame () const;
    const Vector<Int> & processorId () const;
    const Vector<uInt> & rowIds () const;
    const Vector<Int> & scan () const;
    const Vector<Float> & sigma () const;
    const Matrix<Float> & sigmaMat () const;
    Int spectralWindow () const;
    const Vector<Int> & stateId () const;
    const Vector<Double> & time () const;
    const Vector<Double> & timeCentroid () const;
    const Vector<Double> & timeInterval () const;
    const Vector<RigidVector<Double, 3> > & uvw () const;
    const Matrix<Double> & uvwMat () const;
    const Cube<Complex> & visCube () const;
    void setVisCube(const Complex & c);
    void setVisCube (const Cube<Complex> &);
    const Matrix<CStokesVector> & visibility () const;
    void setVisibility (Matrix<CStokesVector> &);
    const Vector<Float> & weight () const;
    void setWeight (const Vector<Float>&);
    const Matrix<Float> & weightMat () const;
    void setWeightMat (const Matrix<Float>&);
    const Cube<Float> & weightSpectrum () const;
    void setWeightSpectrum (const Cube<Float>&);

protected:

    VisBuffer2 * vb_p; // One of the implementation classes

};

} //# NAMESPACE CASA - END


#endif


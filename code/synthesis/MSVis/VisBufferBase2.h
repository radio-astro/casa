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

#ifndef MSVIS_VISBUFFERBASE2_H
#define MSVIS_VISBUFFERBASE_H

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

class CStokesVector;
class MDirection;
template <typename T, Int n> class SquareMatrix;
class SubChunkPair2;
template <typename T, Int n> class RigidVector;
class ROVisibilityIterator2;
class VisBufferBase2;
class VisibilityIterator2;

namespace vi {

//#forward


// <summary>
// VbDirtyComponents allows marking portions of a VisBufferBase as
// modified (aka dirty).  This feature is needed for the Visibility
// Processing Framework (VPF) which allows a sequence of data processing
// nodes to work as a bucket brigade operating sequentially on a
// VisBufferBase.  A downstream output node needs to know what data,
// if any, needs to be written out.
//
// <prerequisite>
//   #<li><linkto class="VisBufferBase">VisBufferBase</linkto>
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

//<summary>VisBufferBases encapsulate one chunk of visibility data for processing.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisSet">VisSet</linkto>
//   <li> <linkto class="VisibilityIterator2">VisibilityIterator2</linkto>
//   <li> <linkto class="VisBufferBaseAutoPtr">VisBufferBaseAutoPtr</linkto>
//   <li> <linkto class="VbDirtyComponents">VbDirtyComponents</linkto>
// </prerequisite>
//
// <etymology>
// VisBufferBase is a buffer for visibility data
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
// for more details on how the VisBufferBase is to be used.
//
// When the user intends to allow asynchronous I/O they will need to use
// the VisBufferBaseAsync and VisBufferBaseAsyncWrapper classes; these are
// publicly derived from VisBufferBase.  Normally, the user should not
// explicitly use these classes but should use the helper class
// VisBufferBaseAutoPtr which will ensure that the appropriate class is
// used.
//</synopsis>

//<todo>
// <li> reconcile vis/visCube usage: visCube, flagCube and weightMatrix
// are currently only correct when this VisBufferBase got them from a
// VisIter, operations like -=, freqAverage() are only done for
// visibility() and flag().
//</todo>

class VisBufferBase2 {

    friend class casa::asyncio::VLAT; // for async i/o
    friend class VbCacheItemBase;
    friend class VisBufferCache;
    friend class VisBufferState;
    friend class VisBufferBase2Async; // for async i/o
    friend class VisBufferBase2AsyncWrapper; // for async i/o
    friend class ViReadImpl;
    friend class ViReadImplAsync; // for async I/O

public:

    // Create empty VisBufferBase2 you can assign to or attach.
    VisBufferBase2();
    // Construct VisBufferBase2 for a particular VisibilityIterator2
    // The buffer will remain synchronized with the iterator.

    // Copy construct, looses synchronization with iterator: only use buffer for
    // current iteration (or reattach).
    VisBufferBase2(const VisBufferBase2 & vb);

    // Destructor (detaches from VisIter)

    virtual ~VisBufferBase2();

    virtual void associateWithVisibilityIterator2 (const ROVisibilityIterator2 & vi) = 0;

    virtual VisBufferBase2 * clone () const = 0;

    virtual const ROVisibilityIterator2 * getVi () const = 0;

    virtual void invalidate() = 0;

    virtual void dirtyComponentsAdd (const VisBufferComponents2 & additionalDirtyComponents) = 0;
    virtual void dirtyComponentsAdd (VisBufferComponent2 component) = 0;
    virtual void dirtyComponentsClear () = 0;
    virtual VisBufferComponents2 dirtyComponentsGet () const = 0;
    virtual void dirtyComponentsSet (const VisBufferComponents2 & dirtyComponents) = 0;
    virtual void dirtyComponentsSet (VisBufferComponent2 component) = 0;

    // feed1_pa() and feed2_pa() return an array of parallactic angles
    // (each corresponds to the first receptor of the feed) one for each
    // row in the current buffer. In contrast, feed_pa() calculates
    // the angles for each antenna. These methods are implemented for
    // VisBufferBase2 only to benefit from caching of the feed and antenna IDs.

    // Note that feed_pa is a function instead of a cached value
    virtual Vector<Float> feed_pa(Double time) const = 0;

    // direction1() and direction2() return arrays of directions where
    // the first and the second antenna/feed are pointed to. One value for
    // each row in the current buffer.


    // NOMINAL parallactic angle (feed p.a. offset NOT included)
    virtual Float parang0(Double time) const = 0;
    virtual Vector<Float> parang(Double time) const = 0;

    virtual MDirection azel0(Double time) const = 0; // function rather than cached value
    virtual void azel0Vec(Double time, Vector<Double>& azelVec) const = 0;
    virtual Vector<MDirection> azel(Double time) const = 0;
    virtual void azelMat(Double time, Matrix<Double>& azelMat) const = 0;

    // Hour angle for specified time
    virtual Double hourang(Double time) const = 0;


    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)
    virtual void sortCorr () = 0;
    virtual void unSortCorr() = 0;

    // Normalize the visCube by the modelVisCube
    //   (and optionally also divide visCube_p by its normalized amp)
    virtual void normalize(const Bool & phaseOnly = False) = 0;

    // Fill weightMat according to sigma column
    virtual void resetWeightsUsingSigma () = 0;//virtual void resetWeightMat();

    // Rotate visibility phase for given vector (dim = nrow of vb) of phases (metres)


    virtual void copyCoordinateInfo(const VisBufferBase2 * other, Bool includeDirectionCoordinates) = 0;

    virtual Bool isNewArrayId () const = 0;
    virtual Bool isNewFieldId () const = 0;
    virtual Bool isNewMs() const = 0;
    virtual Bool isNewSpectralWindow () const = 0;
    virtual Bool isWritable () const = 0;
    virtual Int msId() const = 0;
    virtual String msName (Bool stripPath = False) const = 0;

    //////////////////////////////////////////////////////////////////////
    //
    //  Data accessors and setters (where appropriate)
    //
    //  There are the methods that allows access to the items cached in the
    //  VisBuffer.  The straight accessors provide read-only access to the
    //  item.  Where the item is allowed to be modified, one or more set
    //  methods are provided.

    virtual const Vector<Int> & antenna1 () const = 0;
    virtual const Vector<Int> & antenna2 () const = 0;
    virtual Int arrayId () const = 0;
    virtual const Vector<SquareMatrix<Complex, 2> > & cjones () const = 0;
    virtual const Cube<Complex> & correctedVisCube () const = 0;
    virtual void setCorrectedVisCube (const Cube<Complex> &) = 0;
    virtual const Matrix<CStokesVector> & correctedVisibility () const = 0;
    virtual void setCorrectedVisibility (const Matrix<CStokesVector> &) = 0;
    virtual const Vector<Int> & corrType () const = 0;
    virtual Int dataDescriptionId () const = 0;
    virtual const Vector<MDirection> & direction1 () const = 0;
    virtual const Vector<MDirection> & direction2 () const = 0;
    virtual const Vector<Double> & exposure () const = 0;
    virtual const Vector<Int> & feed1 () const = 0;
    virtual const Vector<Float> & feed1_pa () const = 0;
    virtual const Vector<Int> & feed2 () const = 0;
    virtual const Vector<Float> & feed2_pa () const = 0;
    virtual Int fieldId () const = 0;
    virtual const Matrix<Bool> & flag () const = 0;
    virtual void setFlag (const Matrix<Bool>&) = 0;
    virtual const Array<Bool> & flagCategory () const = 0;
    virtual void setFlagCategory (const Array<Bool>&) = 0;
    virtual const Cube<Bool> & flagCube () const = 0;
    virtual void setFlagCube (const Cube<Bool>&) = 0;
    virtual const Vector<Bool> & flagRow () const = 0;
    virtual void setFlagRow (const Vector<Bool>&) = 0;
    virtual const Cube<Float> & floatDataCube () const = 0;
    virtual void setFloatDataCube (const Cube<Float> &) = 0;
    virtual const Vector<Double> & frequency () const = 0;
    virtual const Matrix<Float> & imagingWeight () const = 0;
    virtual const Cube<Complex> & modelVisCube () const = 0;
    virtual void setModelVisCube(const Complex & c) = 0;
    virtual void setModelVisCube(const Cube<Complex>& vis) = 0;
    virtual void setModelVisCube(const Vector<Float>& stokes) = 0;
    virtual const Matrix<CStokesVector> & modelVisibility () const = 0;
    virtual void setModelVisibility (Matrix<CStokesVector> &) = 0;
    virtual Int nChannel () const = 0;
    virtual Int nCorr () const = 0;
    virtual Int nRow () const = 0;
    virtual const Vector<Int> & observationId () const = 0;
    virtual const MDirection& phaseCenter () const = 0;
    virtual Int polFrame () const = 0;
    virtual const Vector<Int> & processorId () const = 0;
    virtual const Vector<uInt> & rowIds () const = 0;
    virtual const Vector<Int> & scan () const = 0;
    virtual const Vector<Float> & sigma () const = 0;
    virtual const Matrix<Float> & sigmaMat () const = 0;
    virtual Int spectralWindow () const = 0;
    virtual const Vector<Int> & stateId () const = 0;
    virtual const Vector<Double> & time () const = 0;
    virtual const Vector<Double> & timeCentroid () const = 0;
    virtual const Vector<Double> & timeInterval () const = 0;
    virtual const Vector<RigidVector<Double, 3> > & uvw () const = 0;
    virtual const Matrix<Double> & uvwMat () const = 0;
    virtual const Cube<Complex> & visCube () const = 0;
    virtual void setVisCube(const Complex & c) = 0;
    virtual void setVisCube (const Cube<Complex> &) = 0;
    virtual const Matrix<CStokesVector> & visibility () const = 0;
    virtual void setVisibility (Matrix<CStokesVector> &) = 0;
    virtual const Vector<Float> & weight () const = 0;
    virtual void setWeight (const Vector<Float>&) = 0;
    virtual const Matrix<Float> & weightMat () const = 0;
    virtual void setWeightMat (const Matrix<Float>&) = 0;
    virtual const Cube<Float> & weightSpectrum () const = 0;
    virtual void setWeightSpectrum (const Cube<Float>&) = 0;

protected:

    VisBufferBase2(ROVisibilityIterator2 & iter);

    virtual void setIterationInfo (Int msId, const String & msName, Bool isNewMs,
                                   Bool isNewArrayId, Bool isNewFieldId,
                                   Bool isNewSpectralWindow, const SubChunkPair2 & subchunk) = 0;

    template <typename Coord>
    void updateCoord (Coord & item, const Coord & otherItem) = 0;

private:

    virtual void validate() = 0;

};

} // end namespace vi

} // end namespace casa


#endif


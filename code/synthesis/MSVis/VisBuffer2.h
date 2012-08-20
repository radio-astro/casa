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
#include <boost/noncopyable.hpp>

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

class PrefetchColumns;
class VbDirtyComponents;
class ROVisibilityIterator2;
class VisibilityIterator2;

namespace vi {
    class VisBufferBase2;
};

//<summary>VisBuffer2s encapsulate one chunk of visibility data for processing.</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="VisibilityIterator">VisibilityIterator</linkto>
//   <li> <linkto class="VbDirtyComponents">VbDirtyComponents</linkto>
// </prerequisite>
//
// <etymology>
// VisBuffer2 is a buffer for visibility data
// </etymology>
//
//<synopsis>
// The VisBuffer is designed to contain a small amount of visibility-related
// data.  The VisBuffer can be used in two somewhat distinct ways.  The first
// is as an integral (or attached) part of the VisibilityIterator and the second
// is as a free or unattached object.
//
// Attached VisBuffer -- Each VisibilityIterator has a exactly one VisBuffer
// attached to it.  This VisBuffer is created and destroyed by the
// VisibilityIterator.  The role of an attached VB is to contain the data
// currently "pointed to" by the VI.  Because of this tight coupling between
// an attached VI and its VB the operations that can be applied to a VisBuffer
// are somewhat restricted in order to maintain the relationship between the
// MeasurementSet's data and the data i the VisBuffer (failure to do so allows
// obscure bugs to be created).  As such the functions for averaging in either
// the time or frequency axes is not permitted on an attached VB.
//
// Free VisBuffer -- A free VisBuffer is used to contain data that might not
// correspond to the data in the MeasurementSet.  The meaning of the data in
// a free VisBuffer will depend on the user.  Some obvious examples might be:
// a VisBuffer used to resample or average frequencies together; creation of
// "virtual" spectral windows which might reconnect frequencies that were
// split into separate spectral windows becasue of hardware limitation; and
// performing a time average of visibility data.  Because the free VB is not
// tightly coupled to a VI, it is the responsibility of the user to assign
// meaningful values to some of the fields in the VB.  For example, the user
// averaging across time will need to decide what values ought to be reported
// for the VisBuffer's timestamp, pointing angle, etc.
//
//</synopsis>
//
//<todo>
//</todo>

class VisBuffer2 : private boost::noncopyable {

    friend class VisibilityIterator;
    friend class VisibilityIteratorReadImpl2;

public:

    enum {FrameNotSpecified = -2};
    typedef enum {Plain, Asynchronous} Type;

    // Create empty VisBuffer2 you can assign to.

    VisBuffer2 (Type type);

    VisBuffer2(const VisBuffer2 & vb, Type type = Plain);

    // Destructor (detaches from VisIter)

    ~VisBuffer2();

    //---------------------------------------------------------------------
    //
    // Copying methods
    //
    // These methods allow copying portions of the data between two
    // VisBuffers.  Because of the complicated semantics of a VisBuffer the
    // assignment and copy-construction methods are not used as they are likely
    // to confuse VisBuffer users.


    // Copies all of the components (or just the one in the cache) from
    // the specified VisBuffer into this one.

    void copy (const VisBuffer2 & other, Bool copyCachedDataOnly = True);

    // Copies the specified components (or just the one in the cache) from
    // the specified VisBuffer into this one.

    void copyComponents (const VisBuffer2 & other,
                         const VisBufferComponentSet & components,
                         Bool copyCachedDataOnly = False);

    // Copies the coordinate components from the specified VisBuffer into this one.
    // Depending on includeDirections the directio related ones are copied or not.

    void copyCoordinateInfo(const VisBuffer2 & other, Bool includeDirections);

    // For attached VBs this returns the VI the VB is attached to.  For free
    // VBs this method returns False.

    const ROVisibilityIterator2 * getVi () const;

    //---------------------------------------------------------------------
    //
    //    Dirty component methods
    //
    //    The dirtyComponent methods support the data-flow approach to using
    //    VisBuffers (the Visibility Processing Framework).  In this approach
    //    a VisBuffer is passed to successive processing nodes (e.g., applycal,
    //    flagging, etc.) which operate  on it and pass it on to the next node
    //    in the algorithm. The dirtyComponents mechanism allows a processing
    //    node to mark portions of the VisBuffer as modified.  If the VisBuffer
    //    reaches an write-to-disk node then the modified portions of the
    //    VisBuffer will be written out.  The user can also explicitly direct
    //    that the changes be written out using the writeChangesBack method.

    void writeChangesBack ();

    void dirtyComponentsAdd (const VbDirtyComponents & additionalDirtyComponents);
    void dirtyComponentsAdd (VisBufferComponents::EnumType component);
    void dirtyComponentsClear ();
    VbDirtyComponents dirtyComponentsGet () const;
    void dirtyComponentsSet (const VbDirtyComponents & dirtyComponents);
    void dirtyComponentsSet (VisBufferComponents::EnumType component);

    // This method returns the imaging weights associated with the VisBuffer.
    // If an imaging weight generator has not been supplied to the associated
    // VisibilityIterator then this method will throw an exception.

    const Matrix<Float> & imagingWeight() const;

    //---------------------------------------------------------------------------
    //
    // Frequency reporting methods.
    //
    // These methods provide information about the frequencies returned in the
    // visibility cubes.  The information can returned as the channel numbers
    // (numbered as in the underlying MS data) or in a frame-based frequency.
    // This information reported need not be in the same frame of reference
    // used to select the data.  If the frame of reference is specified in the
    // call, then that is the frame that is used to calculate the frequencies.
    // If it is not specified, then the VisibilityIterator will be queried for
    // the reportingFrame; if the user has specified a reporting frame to the
    // VI then that frame will be used; otherwise the frame used to select
    // the frequencies will be used.  If the user provides no frequency selection
    // to the VI then the selection frame will TOPO.
    //
    // Both the channelNumber and frequency reporting methods come in two versions:
    // one returns a single frequency for the specified frequency index and row
    // while the other provides all of the frequencies for the specified row.
    // The frequency index is the zero-based index along the frequency axis of
    // a visibility cube.

    Double getFrequency (Int rowInBuffer, Int frequencyIndex, Int frame = FrameNotSpecified) const;
    const Vector<Double> getFrequencies (Int rowInBuffer,
                                         Int frame = FrameNotSpecified) const;
    Int getChannelNumber (Int rowInBuffer, Int frequencyIndex) const;
    const Vector<Int> & getChannelNumbers (Int rowInBuffer) const;

    // Sort/unsort the correlations, if necessary
    //  (Rudimentary handling of non-canonically sorted correlations--use with care!)

    void sortCorr ();
    void unSortCorr();

    // Normalize the visCube by the modelVisCube.

    void normalize(Bool phaseOnly = False);

    // Set the weight cube using the sigma cube.  Each weight will be
    // the reciprocal of the square of the corresponding element in the model
    // VisCube multiplied by the number of channels in the spectral window.
    // If an element in sigma is zero then the corresponding weight element
    // will also be set to zero.

    void resetWeightsUsingSigma ();//void resetWeightMat();

    //----------------------------------------------------------------------
    //
    //  Subhchunk information methods
    //
    //  These methods provide information related to the current subchunk.
    //  The isNewXXX methods return True if the XXX property of the subchunk
    //  differs from the previous subchunk.
    //
    //  The methods msId and msName provide information about the MS
    //  related to the current subchunk.  The msID is the zero-based index
    //  of the MS in the sequence of MSs being iterated over.
    //
    //  The isWritable method is True when the attached iterator is writable
    //  and False otherwise.

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
    //
    //  The dimensionality of the major object in in accessor/setter is
    //  shown in a trailing comment using the following abbreviations:
    //
    //  nA :== number of antennas
    //  nF :== number of frequencies (or channels)
    //  nC :== number of correlations
    //  nR :== number of table rows contained in the buffer
    //
    //  ** Caution **
    //
    //  Some of the methods below have in their argument lists a parameter of the
    //  form "Int row = -1".  These will return the requested value assuming the
    //  value requested is constant for all of the rows in the VisBuffer.  Otherwise
    //  it will throw an exception.


    //--------------------------------------------------------
    //
    // Accessors for data contained in the main MeasurementSet main table
    // The actual visibility data are at the end.

    const Vector<Int> & antenna1 () const; // [nR]
    const Vector<Int> & antenna2 () const; // [nR]
    Int arrayId (Int row = -1) const;
    Int dataDescriptionId (Int row = -1) const;
    const Vector<MDirection> & direction1 () const; // [nR]
    const Vector<MDirection> & direction2 () const; // [nR]
    const Vector<Double> & exposure () const; // [nR]
    const Vector<Int> & feed1 () const; // [nR]
    const Vector<Int> & feed2 () const; // [nR]
    Int fieldId (Int row = -1) const;
    const Matrix<Bool> & flag () const; // [nF,nR]
    void setFlag (const Matrix<Bool>&); // [nF,nR]
    const Array<Bool> & flagCategory () const; // [nC,nF,nCategories,nR]
    void setFlagCategory (const Array<Bool>&); // [nC,nF,nCategories,nR]
    const Cube<Bool> & flagCube () const; // [nC,nF,nR]
    void setFlagCube (const Cube<Bool>&); // [nC,nF,nR]
    const Vector<Bool> & flagRow () const; // [nR]
    void setFlagRow (const Vector<Bool>&); // [nR]
    const Vector<Int> & observationId () const; // [nR]
    const Vector<Int> & processorId () const; // [nR]
    const Vector<Int> & scan () const; // [nR]
    const Vector<Float> & sigma () const; // [nR]
    const Matrix<Float> & sigmaMat () const; // [nC,nR]
    const Vector<Int> & stateId () const; // [nR]
    const Vector<Double> & time () const; // [nR]
    const Vector<Double> & timeCentroid () const; // [nR]
    const Vector<Double> & timeInterval () const; // [nR]
    const Vector<RigidVector<Double, 3> > & uvw () const; // [nR]
    const Matrix<Double> & uvwMat () const; // [nR,3]
    const Vector<Float> & weight () const; // [nR]
    void setWeight (const Vector<Float>&); // [nR]
    const Matrix<Float> & weightMat () const; // [nC,nR]
    void setWeightMat (const Matrix<Float>&); // [nC,nR]
    const Cube<Float> & weightSpectrum () const; // [nC,nF,nR]
    void setWeightSpectrum (const Cube<Float>&); // [nC,nF,nR]

    // --------------------------------------------------------------
    // Visibility data accessors in order of observed, corrected,
    // float, & model

    const Cube<Complex> & visCube () const; // [nC,nF,nR]
    void setVisCube(const Complex & c);
    void setVisCube (const Cube<Complex> &); // [nC,nF,nR]
    const Matrix<CStokesVector> & vis () const; // [nF,nR]
    void setVis (Matrix<CStokesVector> &); // [nF,nR]

    const Cube<Complex> & visCubeCorrected () const; // [nC,nF,nR]
    void setVisCubeCorrected (const Cube<Complex> &); // [nC,nF,nR]
    const Matrix<CStokesVector> & visCorrected () const; // [nF,nR]
    void setVisCorrected (const Matrix<CStokesVector> &); // [nF,nR]

    const Cube<Float> & visCubeFloat () const; // [nC,nF,nR]
    void setVisCubeFloat (const Cube<Float> &); // [nC,nF,nR]

    const Cube<Complex> & visCubeModel () const; // [nC,nF,nR]
    void setVisCubeModel(const Complex & c);
    void setVisCubeModel(const Cube<Complex>& vis); // [nC,nF,nR]
    void setVisCubeModel(const Vector<Float>& stokes); // [1..4]
    const Matrix<CStokesVector> & visModel () const; // [nF,nR]
    void setVisModel (Matrix<CStokesVector> &); // [nF,nR]

    //--------------------------------------------------------
    //
    // Accessors for data derived from the MS main table data

    // Returns the pointing angle for the array as a whole at the
    // specified time.

    MDirection azel0 (Double time) const;

    // Returns the pointing angle for each antenna in the array
    // at the specified time.

    Vector<MDirection> azel(Double time) const; // [nA]

    // Returns the Jones C matrix for each antenna.

    const Vector<SquareMatrix<Complex, 2> > & cjones () const; // [nA]

    // Returns the correlation type of each correlation in the
    // VisCube.

    const Vector<Int> & correlationType (Int row = -1) const; // [nC]

    // Calcuates the parallactic angle for the first receptor of
    // each antenna at the specified time.

    Vector<Float> feed_pa(Double time) const; // [nR]

    // Calculates the parallactic angle for feed 0 of the
    // row's Antenna1.

    const Vector<Float> & feed1_pa () const; // [nR]

    // Calculates the parallactic angle for feed 0 of the
    // row's Antenna2.

    const Vector<Float> & feed2_pa () const; // [nR]

    // Returns the hour angle of the array at the specified time.

    Double hourang(Double time) const;

    // Returns the number of correlations along the visCube
    // correlation axis.

    Int nCorrelations (Int row = -1) const;

    // Returns the number of rows in this VisBuffer

    Int nRows () const;

    // Calculates the parallactic angle of the array as a whole
    // at the specified time.

    Float parang0(Double time) const;

    // Calculates the parallactic angle of each antenna in the
    // array at the specified time.

    Vector<Float> parang(Double time) const; // [nA]

    // Returns the phase center of the array for the specified
    // row.

    const MDirection& phaseCenter (Int row = -1) const;

    // Returns the polarization frame for the specified row.

    Int polarizationFrame (Int row = -1) const;

    // The returned Vector serves as a map between the rows in
    // the VisBuffer and the row IDs in the underlying MS main
    // table:  mainTableID [i] = rowIds () [ i];

    const Vector<uInt> & rowIds () const; // [nR]

    // Returns the spectral window ID for the specified row.

    Int spectralWindow (Int row = -1) const;

protected:

    VisBuffer2 (ROVisibilityIterator2 * vi, Type type);
    void construct (ROVisibilityIterator2 * vi, Type type);
    void invalidate();

    vi::VisBufferBase2 * vb_p; // One of the implementation classes
};

} //# NAMESPACE CASA - END


#endif


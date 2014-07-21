//# VisibilityIterator.h: Step through the MeasurementEquation by visibility
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the Implied warranty of MERCHANTABILITY or
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
//# $Id: VisibilityIterator2.h,v 19.14 2006/02/28 04:48:58 mvoronko Exp $

#ifndef MSVIS_VISIBILITYITERATORIMPLASYNC2_H
#define MSVIS_VISIBILITYITERATORIMPLASYNC2_H

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Stack.h>
#include <casa/Quanta/MVDoppler.h>
#include <casa/aips.h>
#include <measures/Measures/MCDoppler.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <ms/MeasurementSets/MSIter.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisBufferComponents2.h>
#include <msvis/MSVis/VisBufferImplAsync2.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

//# forward decl

class AsynchronousInterface;
class VisBuffer2;
class VisBufferAsyncWrapper2;
class VlaData;

// <summary>
// VisibilityIterator2 iterates through one or more readonly MeasurementSets
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MSIter">MSIter</linkto>
//   <li> <linkto class="MeasurementSet">MeasurementSet</linkto>
//   <li> <linkto class="VisSet">VisSet</linkto>
// </prerequisite>
//
// <etymology>
// The VisibilityIterator2 is a readonly iterator returning visibilities
// </etymology>
//
// <synopsis>
// VisibilityIterator2 provides iteration with various sort orders
// for one or more MSs. It has member functions to retrieve the fields
// commonly needed in synthesis calibration and imaging.
//
// One should use <linkto class="VisBuffer">VisBuffer</linkto>
// to access chunks of data.
// </synopsis>
//
// <example>
// <code>
// //
// </code>
// </example>
//
// <motivation>
// For imaging and calibration you need to access an MS in some consistent
// order (by field, spectralwindow, time interval etc.). This class provides
// that access.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="1997/05/30">
//   <li> cleanup the currently dual interface for visibilities and flags
//   <li> sort out what to do with weights when interpolating
// </todo>

class VisibilityIteratorImplAsync2 : public VisibilityIteratorImpl2 {

    friend class AsyncEnabler;
    friend class ViImplAsync2;
    friend class VisibilityIterator2;
    friend class VLAT; // allow VI lookahead thread class to access protected functions
    // VLAT should not access private parts, especially variables

public:

    //# the following is a copy of the enum in MSIter
    //# can't think of a way to get one that known to the outside world from here
    //  enum PolFrame {
    //    // Circular polarization
    //    Circular=0,
    //    // Linear polarization
    //    Linear=1
    //  };
    //
    //  enum DataColumn {
    //    // Observed data
    //    Observed=0,
    //    // Model data
    //    Model,
    //    // Corrected data
    //    Corrected
    //  };

    //    class AsyncEnabler {
    //    public:
    //        AsyncEnabler (VisibilityIterator2 &);
    //        AsyncEnabler (VisibilityIterator2 *);
    //        ~AsyncEnabler ();
    //
    //        void release ();
    //
    //    private:
    //
    //        Bool oldEnabledState_p;
    //        VisibilityIterator2 * roVisibilityIterator2_p;
    //    };

    // Default constructor - useful only to assign another iterator later
    ////VisibilityIteratorImplAsync2 ();

    // Construct from an MS and a Block of MS column enums specifying the
    // iteration order.  If no order is specified, it uses the default sort
    // order of MSIter, which is not necessarily the raw order of ms!
    // The default ordering is ARRAY_ID, FIELD_ID, DATA_DESC_ID,
    // and TIME, but check MSIter.h to be sure.
    // These columns will be added first if they are not specified.
    //
    // An optional timeInterval (in seconds) can be given to iterate through
    // chunks of time.  The default interval of 0 groups all times together.
    // Every 'chunk' of data contains all data within a certain time interval and
    // with identical values of the other iteration columns (e.g.  DATA_DESC_ID
    // and FIELD_ID).  Using selectChannel (), a number of groups of channels can
    // be requested.  At present the channel group iteration will always occur
    // before the interval iteration.
//    VisibilityIteratorImplAsync (const MeasurementSet & ms,
//                                const Block<Int> & sortColumns,
//                                Double timeInterval = 0);

    // Same as above, but with the option of using the raw order of ms
    // (addDefaultSortCols=false).
//    VisibilityIteratorImplAsync (const MeasurementSet & ms,
//                                const Block<Int> & sortColumns, const Bool addDefaultSortCols,
//                                Double timeInterval = 0);

    // Same as previous constructors, but with multiple MSs to iterate over.
//    VisibilityIteratorImplAsync (const Block<MeasurementSet> & mss,
//                                const Block<Int> & sortColumns,
//                                Double timeInterval = 0);

    VisibilityIteratorImplAsync2 (VisibilityIterator2 * rovi,
                                  const Block<MeasurementSet> & mss,
                                  const Block<Int> & sortColumns,
                                  Bool addDefaultSortCols,
                                  Double timeInterval,
                                  Bool createVb,
                                  Bool isWritable);

    // Copy construct. This calls the assigment operator.
    VisibilityIteratorImplAsync2 (const VisibilityIteratorImplAsync2 & other,
                                  VisibilityIterator2 * rovi);

    // Destructor
    virtual ~VisibilityIteratorImplAsync2 ();

        // Members

    // Reset iterator to origin/start of data (of current chunk)
    virtual void origin ();
    // Reset iterator to true start of data (first chunk)
    virtual void originChunks ();

    // Return the time interval (in seconds) used for iteration.
    // This is not the same as the INTERVAL column.

    // Set or reset the time interval (in seconds) to use for iteration.
    // You should call originChunks () to reset the iteration after
    // calling this.

    virtual void setInterval (Double timeInterval);

    // Set the 'blocking' size for returning data.
    // With the default (0) only a single integration is returned at a time, this
    // is what is currently required for the calibration software. With blocking
    // set, up to nRows can be returned in one go. The chunk
    // size determines the actual maximum.

    virtual void setRowBlocking (Int nRows = 0);

    virtual Bool existsColumn (VisBufferComponent2 id) const;

    // Return False if no more data (in current chunk)

    virtual Bool more () const;

    virtual Subchunk getSubchunkId () const;

    virtual void setFrequencySelections (const FrequencySelections & selection);

    // Return False if no more 'Chunks' of data left

    virtual Bool moreChunks () const;

    // Check if ms has change since last iteration

    virtual Bool isNewArrayId () const;
    virtual Bool isNewFieldId () const;
    virtual Bool isNewMs () const;
    virtual Bool isNewSpectralWindow () const;

    virtual Int msId () const;

    //reference to actual ms in interator
    virtual const MeasurementSet & ms () const;

    // Advance to the next Chunk of data
    virtual void nextChunk ();

    // Return antenna1
    virtual void antenna1 (Vector<Int> & ant1) const;

    // Return antenna2
    virtual void antenna2 (Vector<Int> & ant2) const;

    // Return feed1
    virtual void feed1 (Vector<Int> & fd1) const;

    // Return feed2
    virtual void feed2 (Vector<Int> & fd2) const;


    // Return feed configuration matrix for specified antenna
    void jonesC (Vector<SquareMatrix<Complex, 2> > & cjones) const;

    // Return receptor angles for all antennae and feeds
    // First axis of the cube is a receptor number,
    // 2nd is antennaId, 3rd is feedId
    // Note: the method is intended to provide an access to MSIter::receptorAngles
    // for VisBuffer in the multi-feed case. It may be worth to change the
    // interface of feed_pa to return the information for all feeds.
    virtual const Cube<Double> & receptorAngles () const;

    // return a string mount identifier for each antenna
    virtual const Vector<String> & antennaMounts () const;

    // Return a cube containing pairs of coordinate offsets for each
    // receptor of each feed (values are in radians, coordinate system is fixed
    // with antenna and is the same one as used to define the BEAM_OFFSET
    // parameter in the feed table). The cube axes are receptor, antenna, feed.
    virtual const Cube<RigidVector<Double, 2> > & getBeamOffsets () const;

    // Return the current FieldId

    virtual Int fieldId () const;

    // Return the current ArrayId
    virtual Int arrayId () const;

    // Return flag for each polarization, channel and row
    virtual void flag (Cube<Bool> & flags) const;

    // Return flag for each channel & row
    virtual void flag (Matrix<Bool> & flags) const;

    // Determine whether FLAG_CATEGORY is valid.
    Bool existsFlagCategory() const;

    // Return flags for each polarization, channel, category, and row.
    virtual void flagCategory (Array<Bool> & flagCategories) const;

    // Return row flag
    virtual void flagRow (Vector<Bool> & rowflags) const;

    // Return scan number
    virtual void scan (Vector<Int> & scans) const;

    // Return the OBSERVATION_IDs
    virtual void observationId (Vector<Int> & obsids) const;

    // Return the PROCESSOR_IDs
    virtual void processorId (Vector<Int> & procids) const;

    // Return the STATE_IDs
    virtual void stateId (Vector<Int> & stateids) const;

    // Return the current phase center as an MDirection
    virtual const MDirection & phaseCenter () const;

    // Return frame for polarization (returns PolFrame enum)
    virtual Int polFrame () const;

    // Return the correlation type (returns Stokes enums)
    virtual void corrType (Vector<Int> & corrTypes) const;

    // Return sigma
    virtual void sigma (Vector<Float> & sig) const;

    // Return sigma matrix (pol-dep)
    virtual void sigmaMat (Matrix<Float> & sigmat) const;

    // Return current SpectralWindow
    virtual Int spectralWindow () const;

    // Return current Polarization Id
    virtual Int polarizationId () const;

    // Return current DataDescription Id
    virtual Int dataDescriptionId () const;

    // Return MJD midpoint of interval.
    virtual void time (Vector<Double> & t) const;

    // Return MJD centroid of interval.
    virtual void timeCentroid (Vector<Double> & t) const;

    // Return nominal time interval
    virtual void timeInterval (Vector<Double> & ti) const;

    // Return actual time interval
    virtual void  exposure (Vector<Double> & expo) const;

    // Return the visibilities as found in the MS, Cube (npol,nchan,nrow).
    virtual void visibilityCorrected (Cube<Complex> & vis) const;
    virtual void visibilityModel (Cube<Complex> & vis) const;
    virtual void visibilityObserved (Cube<Complex> & vis) const;

    // Return FLOAT_DATA as a Cube (npol, nchan, nrow) if found in the MS.
    virtual void floatData (Cube<Float> & fcube) const;

    // Return the visibility 4-vector of polarizations for each channel.
    // If the MS doesn't contain all polarizations, it is assumed it
    // contains one or two parallel hand polarizations.
    virtual void visibilityCorrected (Matrix<CStokesVector> & vis) const;
    virtual void visibilityModel (Matrix<CStokesVector> & vis) const;
    virtual void visibilityObserved (Matrix<CStokesVector> & vis) const;

    // Return the shape of the visibility Cube
    virtual IPosition visibilityShape () const;

    // Return u,v and w (in meters)

    virtual void uvw (Matrix<Double> & uvwmat) const;

    // Return weight
    virtual void weight (Vector<Float> & wt) const;

    // Returns the nPol_p x curNumRow_p weight matrix
    virtual void weightMat (Matrix<Float> & wtmat) const;

    // Determine whether WEIGHT_SPECTRUM exists.
    Bool existsWeightSpectrum () const;

    // Return weightspectrum (a weight for each channel)
    virtual void weightSpectrum (Cube<Float> & wtsp) const;

    // Return imaging weight (a weight for each channel)
    //virtual Matrix<Float> & imagingWeight (Matrix<Float> & wt) const;
    const VisImagingWeight & getImagingWeightGenerator () const;

    // Return True if FieldId/Source has changed since last iteration
    virtual Bool newFieldId () const;

    // Return True if arrayID has changed since last iteration
    virtual Bool newArrayId () const;

    // Return True if SpectralWindow has changed since last iteration
    virtual Bool newSpectralWindow () const;

    // Return the number of correlations in the current iteration

    virtual Int nPolarizations () const;

    // Return the number of rows in the current iteration
    virtual Int nRows () const;

    // Return the row ids as from the original root table. This is useful
    // to find correspondance between a given row in this iteration to the
    // original ms row
    virtual void getRowIds (Vector<uInt> & rowids) const;

    // Return the numbers of rows in the current chunk

    virtual Int nRowsInChunk () const;

    // Return the number of sub-intervals in the current chunk

    //virtual Int nSubInterval () const;

    // Call to use the slurp i/o method for all scalar columns. This
    // will set the BucketCache cache size to the full column length
    // and cause the full column to be cached in memory, if
    // any value of the column is used. In case of out-of-memory,
    // it will automatically fall-back on the smaller cache size.
    // Slurping the column is to be considered as a work-around for the
    // Table i/o code, which uses BucketCache and performs extremely bad
    // for random access. Slurping is useful when iterating non-sequentially
    // an MS or parts of an MS, it is not tested with multiple MSs.
    virtual void slurp () const;

    // Get the spw, start  and nchan for all the ms's is this Visiter that
    // match the frequecy "freqstart-freqStep" and "freqEnd+freqStep" range
    // Can help in doing channel selection above..
    // freqFrame is the frame the caller frequency values are in (freqStart and freqEnd)
    // These will be converted to the frame of the selected spw to match

//    virtual void getSpwInFreqRange (Block<Vector<Int> > & spw,
//                                    Block<Vector<Int> > & start,
//                                    Block<Vector<Int> > & nchan,
//                                    Double freqStart, Double freqEnd,
//                                    Double freqStep, MFrequency::Types freqFrame = MFrequency::LSRK) const;

    // Get the range of frequency convered by the selected data in the frame requested

//    virtual void getFreqInSpwRange(Double& freqStart, Double& freqEnd, MFrequency::Types freqframe = MFrequency::LSRK) const;

    // Access the current ROMSColumns object in MSIter
    virtual const vi::SubtableColumns & subtableColumns () const;

    // get back the selected spectral windows and spectral channels for
    // current ms

    virtual const SpectralWindowChannels & getSpectralWindowChannels (Int msId, Int spectralWindowId) const;

    //assign a VisImagingWeight object to this iterator
    virtual void useImagingWeight (const VisImagingWeight & imWgt);
    //return number  of Ant
    virtual Int numberAnt ();
    //Return number of rows in all selected ms's
    virtual Int numberCoh ();

    // Return number of spws, polids, ddids
    virtual Int numberSpw ();
    virtual Int numberPol ();
    virtual Int numberDDId ();

    Int getDataDescriptionId () const;
    const MeasurementSet & getMeasurementSet () const;;
    Int getMeasurementSetId () const;
    Int getNAntennas () const;
    virtual MEpoch getEpoch () const;
    MFrequency::Types getObservatoryFrequencyType () const; //???
    MPosition getObservatoryPosition () const;
    Vector<Float> getReceptor0Angle ();

    // Write/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
    virtual void writeFlag (const Matrix<Bool> & flag);

    // Write/modify the flags in the data.
    // This writes the flags as found in the MS, Cube (npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow ()).
    virtual void writeFlag (const Cube<Bool> & flag);

    // Write/modify the flag row column; dimension Vector (nrow)
    virtual void writeFlagRow (const Vector<Bool> & rowflags);

    void writeFlagCategory(const Array<Bool>& fc);

    // Write/modify the visibilities.
    // This is possibly only for a 'reference' MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
    void writeVisCorrected (const Matrix<CStokesVector> & visibilityStokes);
    void writeVisModel (const Matrix<CStokesVector> & visibilityStokes);
    void writeVisObserved (const Matrix<CStokesVector> & visibilityStokes);

    // Write/modify the visibilities
    // This writes the data as found in the MS, Cube (npol,nchan,nrow).
    virtual void writeVisCorrected (const Cube<Complex> & vis);
    virtual void writeVisModel (const Cube<Complex> & vis);
    virtual void writeVisObserved (const Cube<Complex> & vis);

    // Write/modify the weights
    virtual void writeWeight (const Vector<Float> & wt);

    // Write/modify the weightMat
    virtual void writeWeightMat (const Matrix<Float> & wtmat);

    // Write/modify the weightSpectrum
    virtual void writeWeightSpectrum (const Cube<Float> & wtsp);

    // Write/modify the Sigma
    virtual void writeSigma (const Vector<Float> & sig);

    // Write/modify the ncorr x nrow SigmaMat.
    virtual void writeSigmaMat (const Matrix<Float> & sigmat);

    virtual void writeBackChanges (VisBuffer2 *);

protected:

    void attachColumnsSafe (const Table & t);

    // attach the column objects to the currently selected table

    virtual void attachColumns (const Table & t);

    // returns the table, to which columns are attached,
    // can be overridden in derived classes

    virtual const Table attachTable () const;

    // advance the iteration

    virtual void advance ();

    virtual void applyPendingChanges ();

    // set the iteration state

    virtual void configureNewChunk ();

    // set the currently selected table

    virtual void configureNewSubchunk ();

    const ChannelSelector *
    createDefaultChannelSelector (Double time, Int msId, Int spectralWindowId);

    virtual const ChannelSelector * determineChannelSelection (Double time);

    Slice findChannelsInRange (Double lowerFrequency, Double upperFrequency,
                               const SpectralWindowChannels & spectralWindowChannels);

    void fillVisBuffer ();

    // Methods to get the data out of a table column according to whatever selection
    // criteria (e.g., slicing) is in effect.

    template <typename T>
    void getColumnRows (const ROArrayColumn<T> & column, Array<T> & array) const;

    template <typename T>
    void
    getColumnRowsMatrix (const ROArrayColumn<T> & column, Matrix<T> & array) const;

    template <typename T>
    void getColumnRows (const ROScalarColumn<T> & column, Vector<T> & array) const;

    Vector<Double> getFrequencies (Double time, Int frameOfReference) const;
    Vector<Int> getChannels (Double time, Int frameOfReference) const;

    Int getReportingFrameOfReference () const;

    // Returns the MS objects that this VI is iterating over.

    std::vector<MeasurementSet> getMeasurementSets () const;

    // Provides access to the MS-derived values object

    const MSDerivedValues & getMsd () const; // for use by Async I/O *ONLY*

    const MsIterInfo & getMsIterInfo () const;

    // Get privileged (non-const) access to the containing ROVI

    VisBufferImplAsync2 * getVisBufferAsync () const;
    VisBufferAsyncWrapper2 * getVisBufferWrapper () const;

    VisibilityIterator2 * getViP () const;

    void getVisibilityAsStokes (Matrix<CStokesVector> & visibilityStokes,
                                const ROArrayColumn<Complex> & column) const;

    // Ctor auxiliary method

    virtual void initialize (const Block<MeasurementSet> & mss);

    // Returns true if MS Iterator is currently pointing to a selected
    // spectral window

    Bool isInASelectedSpectralWindow () const;

    // Creates a channel selection for the current subchunk based on the channel
    // or frequency selection made by the user.

    ChannelSelector *
    makeChannelSelectorC (const FrequencySelection & selection,
                          Double time, Int msId, Int spectralWindowId);

    ChannelSelector *
    makeChannelSelectorF (const FrequencySelection & selection,
                          Double time, Int msId, Int spectralWindowId);

    MFrequency::Convert makeFrequencyConverter (Double time, Int otherFrameOfReference,
                                                Bool toObservedFrame) const;


    // Method to reset the VI back to the start.  Unlike the public version
    // there is a parameter to allow forcing the rewind even if the
    // MS Iter is already at the origin.

    virtual void originChunks (Bool forceRewind);

    // Advances the MS Iterator until it points at a spectral window
    // that is part of the frequency selection.

    void positionMsIterToASelectedSpectralWindow ();

    void readComplete ();

    // Sets the default frequency reporting frame of reference.  This
    // affects the default frame for obtaining the frequencies in a
    // VisBuffer.

    void setReportingFrameOfReference (Int);

    // Adjusts the tile cache for some columns so that the cache size is
    // optimized for the current input state (e.g., a new data description).

    virtual void setTileCache ();

    // Throws exception if there is a pending (i.e., unapplied) change to
    // the VI's properties.  Called when the VI is advanced since the user
    // probably forgot to apply the changes.

    virtual void throwIfPendingChanges ();


    // Returns true if the named column uses a tiled data manager in the specified MS

    Bool usesTiledDataManager (const String & columnName, const MeasurementSet & ms) const;

private:

    AsynchronousInterface * interface_p;
    VlaData * vlaData_p;

};

} // end namespace vi

} //# NAMESPACE CASA - END

#endif


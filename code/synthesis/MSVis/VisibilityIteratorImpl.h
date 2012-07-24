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
//# $Id: VisibilityIterator.h,v 19.14 2006/02/28 04:48:58 mvoronko Exp $

#ifndef MSVIS_VISIBILITYITERATORIMPL_H
#define MSVIS_VISIBILITYITERATORIMPL_H

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Containers/Stack.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVDoppler.h>
#include <measures/Measures/MCDoppler.h>
#include <measures/Measures/MDoppler.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <scimath/Mathematics/RigidVector.h>

#include <ms/MeasurementSets/MSDerivedValues.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/MSVis/VisImagingWeight.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisBufferComponents.h>
#include <ms/MeasurementSets/MSIter.h>

#include <map>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward decl
class VisBuffer;



// <summary>
// ROVisibilityIterator iterates through one or more readonly MeasurementSets
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
// The ROVisibilityIterator is a readonly iterator returning visibilities
// </etymology>
//
// <synopsis>
// ROVisibilityIterator provides iteration with various sort orders
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

class VisibilityIteratorReadImpl {

    friend class AsyncEnabler;
    friend class ViReadImplAsync;
    friend class VisibilityIteratorWriteImpl;
    friend class ROVisibilityIterator;
    friend class ROVisIterator;
    friend class VisIterator;
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

    typedef ROVisibilityIterator::DataColumn DataColumn;

    //    class AsyncEnabler {
    //    public:
    //        AsyncEnabler (ROVisibilityIterator &);
    //        AsyncEnabler (ROVisibilityIterator *);
    //        ~AsyncEnabler ();
    //
    //        void release ();
    //
    //    private:
    //
    //        Bool oldEnabledState_p;
    //        ROVisibilityIterator * roVisibilityIterator_p;
    //    };

    // Default constructor - useful only to assign another iterator later
    VisibilityIteratorReadImpl ();

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
//    VisibilityIteratorReadImpl (const MeasurementSet & ms,
//                                const Block<Int> & sortColumns,
//                                Double timeInterval = 0);

    // Same as above, but with the option of using the raw order of ms
    // (addDefaultSortCols=false).
//    VisibilityIteratorReadImpl (const MeasurementSet & ms,
//                                const Block<Int> & sortColumns, const Bool addDefaultSortCols,
//                                Double timeInterval = 0);

    // Same as previous constructors, but with multiple MSs to iterate over.
//    VisibilityIteratorReadImpl (const Block<MeasurementSet> & mss,
//                                const Block<Int> & sortColumns,
//                                Double timeInterval = 0);

    VisibilityIteratorReadImpl (ROVisibilityIterator * rovi,
                                const Block<MeasurementSet> & mss,
                                const Block<Int> & sortColumns,
                                const Bool addDefaultSortCols,
                                Double timeInterval);

    // Copy construct. This calls the assigment operator.
    VisibilityIteratorReadImpl (const VisibilityIteratorReadImpl & other,
                                ROVisibilityIterator * rovi);

    // Assigment. Any attached VisBuffers are lost in the assign.
    VisibilityIteratorReadImpl & operator= (const VisibilityIteratorReadImpl & other);

    // Destructor
    virtual ~VisibilityIteratorReadImpl ();

    virtual VisibilityIteratorReadImpl * clone (ROVisibilityIterator * rovi) const;

    // Members

    Bool isAsyncEnabled () const;

    virtual Bool isWritable () const {
        return False;
    }

    // Reset iterator to origin/start of data (of current chunk)
    virtual void origin ();
    // Reset iterator to true start of data (first chunk)
    virtual void originChunks ();

    // Return the time interval (in seconds) used for iteration.
    // This is not the same as the INTERVAL column.
    virtual Double getInterval() const { return timeInterval_p; }

    // Set or reset the time interval (in seconds) to use for iteration.
    // You should call originChunks () to reset the iteration after
    // calling this.
    virtual void setInterval (Double timeInterval) {
        msIter_p.setInterval (timeInterval);
        timeInterval_p = timeInterval;
    }

    // Set the 'blocking' size for returning data.
    // With the default (0) only a single integration is returned at a time, this
    // is what is currently required for the calibration software. With blocking
    // set, up to nRows can be returned in one go. The chunk
    // size determines the actual maximum.
    virtual void setRowBlocking (Int nRows = 0);

    virtual Bool existsColumn (VisBufferComponents::EnumType id) const;

    // Return False if no more data (in current chunk)
    virtual Bool more () const;

    virtual SubChunkPair getSubchunkId () const;

    virtual const Block<Int>& getSortColumns() const;

    // Return False if no more 'Chunks' of data left
    virtual Bool moreChunks () const {
        return msIter_p.more ();
    }

    // Check if ms has change since last iteration
    virtual Bool newMS () const {
        return msIter_p.newMS ();
    }

    virtual Int msId () const {
        return msIter_p.msId ();
    }

    virtual VisBuffer * getVisBuffer ();

    //reference to actual ms in interator
    virtual const MeasurementSet & ms () const {
        return msIter_p.ms ();
    }
    // Advance to the next Chunk of data
    virtual VisibilityIteratorReadImpl & nextChunk ();

    // Return antenna1
    virtual Vector<Int> & antenna1 (Vector<Int> & ant1) const;

    // Return antenna2
    virtual Vector<Int> & antenna2 (Vector<Int> & ant2) const;

    // Return feed1
    virtual Vector<Int> & feed1 (Vector<Int> & fd1) const;

    // Return feed2
    virtual Vector<Int> & feed2 (Vector<Int> & fd2) const;

    // Return channel numbers in selected VisSet spectrum
    // (i.e. disregarding possible selection on the iterator, but
    //  including the selection set when creating the VisSet)
    virtual Vector<Int> & channel (Vector<Int> & chan) const;

    // Return feed configuration matrix for specified antenna
    virtual Vector<SquareMatrix<Complex, 2> > &
    CJones (Vector<SquareMatrix<Complex, 2> > & cjones) const;

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

    // True if all elements of the cube returned by getBeamOffsets are zero
    virtual Bool allBeamOffsetsZero () const;

    // Return feed parallactic angles Vector (nant) (1 feed/ant)
    virtual Vector<Float> feed_pa (Double time) const;
    static Vector<Float> feed_paCalculate (Double time, MSDerivedValues & msd,
                                           Int nAntennas, const MEpoch & mEpoch0,
                                           const Vector<Float> & receptor0Angle);

    // Return nominal parallactic angle at specified time
    // (does not include feed position angle offset--see feed_pa)
    // A global value for all antennas (e.g., small array)
    virtual const Float & parang0 (Double time) const;
    static Float parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & epoch0);

    // Per antenna:
    virtual Vector<Float> parang (Double time) const;
    static Vector<Float> parangCalculate (Double time, MSDerivedValues & msd,
                                          int nAntennas, const MEpoch mEpoch0);

    // Return the antenna AZ/EL Vector (nant)
    virtual MDirection azel0 (Double time) const;
    static void azel0Calculate (Double time, MSDerivedValues & msd,
                                MDirection & azel0, const MEpoch & mEpoch0);

    virtual Vector<MDirection> azel (Double time) const;
    static void azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
                               Int nAnt, const MEpoch & mEpoch0);


    // Return the hour angle for the specified time
    virtual Double hourang (Double time) const;
    static Double hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0);

    // Return the current FieldId
    virtual Int fieldId () const {
        return msIter_p.fieldId ();
    }

    // Return the current ArrayId
    virtual Int arrayId () const {
        return msIter_p.arrayId ();
    }

    // Return the current Field Name
    virtual String fieldName () const {
        return msIter_p.fieldName ();
    }

    // Return the current Source Name
    virtual String sourceName () const {
        return msIter_p.sourceName ();
    }

    // Return flag for each polarization, channel and row
    virtual Cube<Bool> & flag (Cube<Bool> & flags) const;

    // Return flag for each channel & row
    virtual Matrix<Bool> & flag (Matrix<Bool> & flags) const;

    // Determine whether FLAG_CATEGORY is valid.
    Bool existsFlagCategory() const;

    // Return flags for each polarization, channel, category, and row.
    virtual Array<Bool> & flagCategory (Array<Bool> & flagCategories) const;

    // Return row flag
    virtual Vector<Bool> & flagRow (Vector<Bool> & rowflags) const;

    // Return scan number
    virtual Vector<Int> & scan (Vector<Int> & scans) const;

    // Return the OBSERVATION_IDs
    virtual Vector<Int> & observationId (Vector<Int> & obsids) const;

    // Return the PROCESSOR_IDs
    virtual Vector<Int> & processorId (Vector<Int> & procids) const;

    // Return the STATE_IDs
    virtual Vector<Int> & stateId (Vector<Int> & stateids) const;

    // Return current frequencies (in Hz, acc. to the MS def'n v.2)
    virtual Vector<Double> & frequency (Vector<Double> & freq) const;

    // Return frequencies  (in Hz, acc. to the MS def'n v.2) in selected velocity frame,
    // returns the same as frequency () if there is no vel selection active.
    virtual Vector<Double> & lsrFrequency (Vector<Double> & freq) const;

    // Return the current phase center as an MDirection
    virtual const MDirection & phaseCenter () const {
        return msIter_p.phaseCenter ();
    }

    // Return frame for polarization (returns PolFrame enum)
    virtual Int polFrame () const {
        return msIter_p.polFrame ();
    }

    // Return the correlation type (returns Stokes enums)
    virtual Vector<Int> & corrType (Vector<Int> & corrTypes) const;

    // Return sigma
    virtual Vector<Float> & sigma (Vector<Float> & sig) const;

    // Return sigma matrix (pol-dep)
    virtual Matrix<Float> & sigmaMat (Matrix<Float> & sigmat) const;

    // Return current SpectralWindow
    virtual Int spectralWindow () const {
        return msIter_p.spectralWindowId ();
    }

    // Return current Polarization Id
    virtual Int polarizationId () const {
        return msIter_p.polarizationId ();
    }

    // Return current DataDescription Id
    virtual Int dataDescriptionId () const {
        return msIter_p.dataDescriptionId ();
    }

    // Return MJD midpoint of interval.
    virtual Vector<Double> & time (Vector<Double> & t) const;

    // Return MJD centroid of interval.
    virtual Vector<Double> & timeCentroid (Vector<Double> & t) const;

    // Return nominal time interval
    virtual Vector<Double> & timeInterval (Vector<Double> & ti) const;

    // Return actual time interval
    virtual Vector<Double> & exposure (Vector<Double> & expo) const;

    // Return the visibilities as found in the MS, Cube (npol,nchan,nrow).
    virtual Cube<Complex> & visibility (Cube<Complex> & vis,
                                        DataColumn whichOne) const;

    // Return FLOAT_DATA as a Cube (npol, nchan, nrow) if found in the MS.
    virtual Cube<Float> & floatData (Cube<Float> & fcube) const;

    // Return the visibility 4-vector of polarizations for each channel.
    // If the MS doesn't contain all polarizations, it is assumed it
    // contains one or two parallel hand polarizations.
    virtual Matrix<CStokesVector> & visibility (Matrix<CStokesVector> & vis,
                                                DataColumn whichOne) const;

    // Return the shape of the visibility Cube
    virtual IPosition visibilityShape () const;

    // Return u,v and w (in meters)
    virtual Vector<RigidVector<Double, 3> > & uvw (Vector<RigidVector<Double, 3> > & uvwvec) const;
    virtual Matrix<Double> & uvwMat (Matrix<Double> & uvwmat) const;

    // Return weight
    virtual Vector<Float> & weight (Vector<Float> & wt) const;

    // Returns the nPol_p x curNumRow_p weight matrix
    virtual Matrix<Float> & weightMat (Matrix<Float> & wtmat) const;

    // Determine whether WEIGHT_SPECTRUM exists.
    Bool existsWeightSpectrum () const;

    // Return weightspectrum (a weight for each channel)
    virtual Cube<Float> & weightSpectrum (Cube<Float> & wtsp) const;

    // Return imaging weight (a weight for each channel)
    //virtual Matrix<Float> & imagingWeight (Matrix<Float> & wt) const;
    const VisImagingWeight & getImagingWeightGenerator () const;

    // Return True if FieldId/Source has changed since last iteration
    virtual Bool newFieldId () const {
        return (curStartRow_p == 0 && msIter_p.newField ());
    }

    // Return True if arrayID has changed since last iteration
    virtual Bool newArrayId () const {
        return (curStartRow_p == 0 && msIter_p.newArray ());
    }

    // Return True if SpectralWindow has changed since last iteration
    virtual Bool newSpectralWindow () const {
        return (curStartRow_p == 0 && msIter_p.newSpectralWindow ());
    }

    // Return the index of the first channel of the current channel group
    // in the total (selected) spectrum.
    virtual Int channelIndex () const;

    // Return the width of the current group of channels, i.e.,
    // the number of channels returned by visibility () and frequency ().
    virtual Int channelGroupSize () const;

    // Return the number of correlations in the current iteration
    virtual Int nCorr () const {
        return nPol_p;
    };

    // Return the number of rows in the current iteration
    virtual Int nRow () const;

    // Return the row ids as from the original root table. This is useful
    // to find correspondance between a given row in this iteration to the
    // original ms row
    virtual Vector<uInt> & rowIds (Vector<uInt> & rowids) const;

    // Return the numbers of rows in the current chunk
    virtual Int nRowChunk () const;

    // Return the number of sub-intervals in the current chunk
    virtual Int nSubInterval () const;

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

    // Velocity selection - specify the output channels in velocity:
    // nChan - number of output channels, vStart - start velocity,
    // vInc - velocity increment. So channel i will have velocity
    // vStart + i*vInc (i=0,nChan-1).
    // Specify velocities as in e.g., MVRadialVelocity (Quantity (2001.,"km/s")).
    // The reference type and velocity definition are specified separately.
    // Note that no averaging is performed, the visibilities will be interpolated
    // and sampled at the specified velocities, it's up to you to choose a vInc
    // appropriate to the channel width.
    // The REST_FREQUENCY column in the SPECTRAL_WINDOW subtable is used to
    // determine the velocity-frequency conversion.
    // By default calculations are done for a single velocity with offsets
    // applied for the others (ok for non-rel velocities with RADIO defn),
    // set precise to True to do a full conversion for each output channel. (NYI)
    virtual VisibilityIteratorReadImpl &
    selectVelocity (Int nChan,
                    const MVRadialVelocity & vStart, const MVRadialVelocity & vInc,
                    MRadialVelocity::Types rvType = MRadialVelocity::LSR,
                    MDoppler::Types dType = MDoppler::RADIO, Bool precise = False);

    // Select the velocity interpolation scheme.
    // At present the choice is limited to : nearest and linear, linear
    // is the default.
    // TODO: add cubic, spline and possibly FFT
    virtual VisibilityIteratorReadImpl & velInterpolation (const String & /*type*/) {return * this; }

    // Channel selection - only the selected channels will be returned by the
    // access functions. The default spectralWindow is the current one (or 0)
    // This allows selection of the input channels, producing
    // nGroup groups of width output channels. Default is to return all channels
    // in a single group.
    virtual VisibilityIteratorReadImpl & selectChannel (Int nGroup = 1, Int start = 0, Int width = 0,
                                                        Int increment = 1, Int spectralWindow = -1);

    //Same as above except when multiple ms's are to be accessed

    virtual VisibilityIteratorReadImpl & selectChannel (const Block< Vector<Int> > & blockNGroup,
                                                        const Block< Vector<Int> > & blockStart,
                                                        const Block< Vector<Int> > & blockWidth,
                                                        const Block< Vector<Int> > & blockIncr,
                                                        const Block< Vector<Int> > & blockSpw);


    //get the channel selection ...the block over the number of ms's associated
    // with this iterator
    virtual void getChannelSelection (Block< Vector<Int> > & blockNGroup,
                                      Block< Vector<Int> > & blockStart,
                                      Block< Vector<Int> > & blockWidth,
                                      Block< Vector<Int> > & blockIncr,
                                      Block< Vector<Int> > & blockSpw);

    // Translate slicesv from the form returned by MSSelection::getChanSlices ()
    // to matv as used by setChanAveBounds ().  widthsv is the channel averaging
    // width for each _selected_ spw.
    void slicesToMatrices (Vector<Matrix<Int> > & matv,
                           const Vector<Vector<Slice> > & slicesv,
                           const Vector<Int> & widthsv) const;

    // Get the spw, start  and nchan for all the ms's is this Visiter that
    // match the frequecy "freqstart-freqStep" and "freqEnd+freqStep" range
    // Can help in doing channel selection above..
    // freqFrame is the frame the caller frequency values are in (freqStart and freqEnd)
    // These will be converted to the frame of the selected spw to match

    virtual void getSpwInFreqRange (Block<Vector<Int> > & spw,
                                    Block<Vector<Int> > & start,
                                    Block<Vector<Int> > & nchan,
                                    Double freqStart, Double freqEnd,
                                    Double freqStep, MFrequency::Types freqFrame = MFrequency::LSRK) const;

    // Get the range of frequency convered by the selected data in the frame requested

    virtual void getFreqInSpwRange(Double& freqStart, Double& freqEnd, MFrequency::Types freqframe = MFrequency::LSRK) const;

    // Attach a VisBuffer object.
    // Note that while more than one VisBuffer may be attached, only the
    // last one is actively updated. A Stack is kept internally, so after
    // a detach, the previous VisBuffer becomes active again.
    virtual void attachVisBuffer (VisBuffer & vb);

    // Detach a VisBuffer object.
    // If the object detached is not the last one attached an exception
    // is thrown.
    virtual void detachVisBuffer (VisBuffer & vb);

    // Access the current ROMSColumns object in MSIter
    virtual const ROMSColumns & msColumns () const {
        return msIter_p.msColumns ();
    }

    // get back the selected spectral windows and spectral channels for
    // current ms

    virtual void allSelectedSpectralWindows (Vector<Int> & spws, Vector<Int> & nvischan);

    // Convert the frequency from the observe frame to lsr frame.
    // Returns True in convert if given spw was not observed
    // in the LSRK frame
    // if ignoreconv=True then conversion from frame in data 
    //is ignored by request
    virtual void lsrFrequency (const Int & spw, Vector<Double> & freq, Bool & convert, const Bool ignoreconv=False);
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

    ROArrayColumn <Double> & getChannelFrequency () const;
    Block<Int> getChannelGroupNumber () const;
    Block<Int> getChannelIncrement () const;
    Block<Int> getChannelStart () const;
    Block<Int> getChannelWidth () const;
    Int getDataDescriptionId () const;
    const MeasurementSet & getMeasurementSet () const;;
    Int getMeasurementSetId () const;
    Int getNAntennas () const;
    virtual MEpoch getEpoch () const;
    MFrequency::Types getObservatoryFrequencyType () const; //???
    MPosition getObservatoryPosition () const;
    MDirection getPhaseCenter () const;
    Vector<Float> getReceptor0Angle ();
    Vector<uInt> getRowIds () const;

    static void lsrFrequency (const Int & spw,
                              Vector<Double> & freq,
                              Bool & convert,
                              const Block<Int> & chanStart,
                              const Block<Int> & chanWidth,
                              const Block<Int> & chanInc,
                              const Block<Int> & numChanGroup,
                              const ROArrayColumn <Double> & chanFreqs,
                              const ROScalarColumn<Int> & obsMFreqTypes,
                              const MEpoch & ep,
                              const MPosition & obsPos,
                              const MDirection & dir,
			      const Bool ignoreconv=False);

protected:

    void attachColumnsSafe (const Table & t);

    // advance the iteration
    virtual void advance ();
    // set the currently selected table
    virtual void setSelTable ();
    // set the iteration state
    virtual void setState ();
    // get the TOPO frequencies from the selected velocities and the obs. vel.
    virtual void getTopoFreqs ();
    virtual void getTopoFreqs (Vector<Double> & lsrFreq, Vector<Double> & selFreq); // for async i/o

    virtual void getLsrInfo (Block<Int> & channelGroupNumber,
                             Block<Int> & channelIncrement,
                             Block<Int> & channelStart,
                             Block<Int> & channelWidth,
                             MPosition & observatoryPositon,
                             MDirection & phaseCenter,
                             Bool & velocitySelection) const;

    vector<MeasurementSet> getMeasurementSets () const;

    const MSDerivedValues & getMSD () const; // for use by Async I/O *ONLY*

    // update the DATA slicer
    virtual void updateSlicer ();
    // attach the column objects to the currently selected table
    virtual void attachColumns (const Table & t);
    // returns the table, to which columns are attached,
    // can be overridden in derived classes
    virtual const Table attachTable () const;

    // get the (velocity selected) interpolated visibilities, flags and weights.
    // It is not really const at all (it seems to use This-> trickery so callers
    // like flag () can be declared const).

//    virtual void getInterpolatedVisFlagWeight (DataColumn whichOne) const;

    // get the (velocity selected) interpolated FLOAT_DATA (as real Floats),
    // flags and weights.

//    void getInterpolatedFloatDataFlagWeight () const;

    Bool usesTiledDataManager (const String & columnName, const MeasurementSet & ms) const;

    // get the visibility data (observed, corrected or model);
    // deals with Float and Complex observed data (DATA or FLOAT_DATA)
    virtual void getDataColumn (ROVisibilityIterator::DataColumn whichOne, const Slicer & slicer,
                                Cube<Complex> & data) const;
    virtual void getDataColumn (DataColumn whichOne, Cube<Complex> & data) const;

    // get FLOAT_DATA as real Floats.
    virtual void getFloatDataColumn (const Slicer & slicer, Cube<Float> & data) const;
    virtual void getFloatDataColumn (Cube<Float> & data) const;

    //constructor helpers
    virtual void initialize (const Block<MeasurementSet> & mss);

    virtual void originChunks (Bool forceRewind);

    //Re-Do the channel selection in multi ms case
    virtual void doChannelSelection ();
    //Set the tile cache size....when using slice access if tile cache size is
    // not set memory usage can go wild.  Specifically, the caching scheme is
    // ephemeral and lives for that instance of setting the caching scheme.
    //
    // If you don't set any then the defaults come into play and caches a few
    // tiles along every axis at the tile you requested...which is a waste when
    // say you know you want to proceed along the row axis for example...and in
    // fact now VisIter just reads one tile (thus the commenting in setTileCache)
    // and lets the OS do the caching rather than than having the table system
    // cache extra tiles.
    virtual void setTileCache ();
    //Check if spw is in selected SPW for actual ms
    virtual Bool isInSelectedSPW (const Int & spw);

    // Updates, if necessary, rowIds_p member for the current chunk
    virtual void update_rowIds () const;

    void setAsyncEnabled (Bool enable);

    template<class T>
    void getColScalar (const ROScalarColumn<T> & column, Vector<T> & array, Bool resize) const;

    template<class T>
    void getColArray (const ROArrayColumn<T> & column, Array<T> & array, Bool resize) const;

    // column access functions, can be overridden in derived classes
    virtual void getCol (const ROScalarColumn<Bool> & column, Vector<Bool> & array, Bool resize = False) const;
    virtual void getCol (const ROScalarColumn<Int> & column, Vector<Int> & array, Bool resize = False) const;
    virtual void getCol (const ROScalarColumn<Double> & column, Vector<Double> & array, Bool resize = False) const;

    virtual void getCol (const ROArrayColumn<Bool> & column, Array<Bool> & array, Bool resize = False) const;
    virtual void getCol (const ROArrayColumn<Float> & column, Array<Float> & array, Bool resize = False) const;
    virtual void getCol (const ROArrayColumn<Double> & column, Array<Double> & array, Bool resize = False) const;
    virtual void getCol (const ROArrayColumn<Complex> & column, Array<Complex> & array, Bool resize = False) const;

    virtual void getCol (const ROArrayColumn<Bool> & column, const Slicer & slicer, Array<Bool> & array, Bool resize = False) const;
    virtual void getCol (const ROArrayColumn<Float> & column, const Slicer & slicer, Array<Float> & array, Bool resize = False) const;
    virtual void getCol (const ROArrayColumn<Complex> & column, const Slicer & slicer, Array<Complex> & array, Bool resize = False) const;

    //  virtual void getCol (const String & colName, Array<Double> & array,
    //                      Array<Double> & all, Bool resize = False) const;
    //  virtual void getCol (const String & colName, Vector<Bool> & array,
    //                      Vector<Bool> & all, Bool resize = False) const;
    //  virtual void getCol (const String & colName, Vector<Int> & array,
    //                      Vector<Int> & all, Bool resize = False) const;
    //  virtual void getCol (const String & colName, Vector<Double> & array,
    //                      Vector<Double> & all, Bool resize = False) const;

    template<class T>
    void swapyz (Cube<T> & out, const Cube<T> & in) const;

    class Cache {

    public:

        Cache ();

        Cache & operator= (const Cache & other);

        MDirection         azel0_p;
        Vector<MDirection> azel_p;
        Vector<Float>      feedpa_p;
        Cube<Bool>         flagCube_p;
        Bool               flagOK_p;
        Bool               floatDataCubeOK_p;
        Cube<Float>        floatDataCube_p;
        Bool               freqCacheOK_p;
        Vector<Double>     frequency_p;
        Double             hourang_p;
        Matrix<Float>      imagingWeight_p;
        Double             lastParang0UT_p;
        Double             lastParangUT_p;
        Double             lastazelUT_p;
        Double             lastfeedpaUT_p;
        Bool               msHasFC_p;   // Does the current MS have a valid FLAG_CATEGORY?
        Bool               msHasWtSp_p; // Does the current MS have a valid WEIGHT_SPECTRUM?
        Float              parang0_p;
        Vector<Float>      parang_p;
        Vector<uInt>       rowIds_p;
        Matrix<Double>     uvwMat_p;
        Cube<Complex>      visCube_p;
        Block<Bool>        visOK_p;
        Bool               weightSpOK_p;
        Cube<Float>        wtSp_p;
    };

    class Channels { // channel selection

    public:

        Block<Int> inc_p;
        Block<Int> start_p;
        Block<Int> width_p;
        Block<Int> nGroups_p;
        Block<Int> preselectedChanStart_p;
        Block<Int> preselectednChan_p;

    };

    class Columns {

    public:

        Columns & operator= (const Columns & other);

        ROScalarColumn<Int>    antenna1_p;
        ROScalarColumn<Int>    antenna2_p;
        ROArrayColumn<Complex> corrVis_p;
        ROScalarColumn<Double> exposure_p;
        ROScalarColumn<Int>    feed1_p;
        ROScalarColumn<Int>    feed2_p;
        ROArrayColumn<Bool>    flagCategory_p;
        ROScalarColumn<Bool>   flagRow_p;
        ROArrayColumn<Bool>    flag_p;
        ROArrayColumn<Float>   floatVis_p;
        ROArrayColumn<Complex> modelVis_p;
        ROScalarColumn<Int>    observation_p;
        ROScalarColumn<Int>    processor_p;
        ROScalarColumn<Int>    scan_p;
        ROArrayColumn<Float>   sigma_p;
        ROScalarColumn<Int>    state_p;
        ROScalarColumn<Double> timeCentroid_p;
        ROScalarColumn<Double> timeInterval_p;
        ROScalarColumn<Double> time_p;
        ROArrayColumn<Double>  uvw_p;
        ROArrayColumn<Complex> vis_p;
        ROArrayColumn<Float>   weightSpectrum_p;
        ROArrayColumn<Float>   weight_p;

    };

    class MeasurementSetChannels {

    public:

        Block < Vector<Int> > inc_p;
        Block < Vector<Int> > nGroups_p;
        Block < Vector<Int> > start_p;
        Block < Vector<Int> > width_p;
        Block < Vector<Int> > spw_p;

    };

    class Velocity { // for velocity selection and conversion

    public:

        Velocity ();
        Velocity & operator= (const Velocity & other);

        MDoppler::Convert cFromBETA_p;
        Vector<Double>    lsrFreq_p;
        Int               nVelChan_p;
        Vector<Double>    selFreq_p;
        MDoppler::Types   vDef_p;
        MVRadialVelocity  vInc_p;
        String            vInterpolation_p;
        Bool              velSelection_p;
        Bool              vPrecise_p;
        MVRadialVelocity  vStart_p;

    };


    typedef std::vector<MeasurementSet> MeasurementSets;

    Bool                    addDefaultSort_p;
    Bool                    asyncEnabled_p; // Allows lower-level code to make an async "copy" of this VI.
    mutable Cache           cache_p;
    Int                     channelGroupSize_p;
    Channels                channels_p;
    Int                     chunkNumber_p;
    Columns                 columns_p;
    Int                     curChanGroup_p;
    Int                     curEndRow_p;
    Int                     curNGroups_p;
    uInt                    curNumRow_p;
    Int                     curStartRow_p;
    Int                     curTableNumRow_p;
    Bool                    floatDataFound_p;
    VisImagingWeight        imwgt_p;    // object to calculate imaging weight
    Bool                    initialized_p;
    Bool                    isMultiMS_p;
    MeasurementSets         measurementSets_p; // [use]
    Bool                    more_p;
    MeasurementSetChannels  msChannels_p;
    Int                     msCounter_p;
    Bool                    msIterAtOrigin_p;
    MSIter                  msIter_p;
    mutable MSDerivedValues msd_p;
    Int                     nAnt_p;
    Int                     nChan_p;
    Int                     nPol_p;
    Int                     nRowBlocking_p;
    Bool                    newChanGroup_p;
    ROVisibilityIterator *  rovi_p; // [use]
    RefRows                 selRows_p; // currently selected rows from msIter_p.table ()
    Slicer                  slicer_p;
    Block<Int>              sortColumns_p;
    Bool                    stateOk_p;
    SubChunkPair            subchunk_p;
    Vector<Bool>            tileCacheIsSet_p;
    Double                  timeInterval_p;
    Vector<Double>          time_p;
    Bool                    useSlicer_p;
    Stack<VisBuffer *>      vbStack_p;   // Stack of VisBuffer objects
    Velocity                velocity_p;
    Slicer                  weightSlicer_p;


};

inline Bool VisibilityIteratorReadImpl::more () const
{
    return more_p;
}

inline Vector<SquareMatrix<Complex, 2> > &
VisibilityIteratorReadImpl::CJones (Vector<SquareMatrix<Complex, 2> > & cjones) const
{
    cjones.resize (msIter_p.CJones ().nelements ());
    return cjones = msIter_p.CJones ();
}

inline const Cube<Double> & VisibilityIteratorReadImpl::receptorAngles () const
{
    return msIter_p.receptorAngles ();
}

inline const Vector<String> & VisibilityIteratorReadImpl::antennaMounts () const
{
    return msIter_p.antennaMounts ();
}

inline const Cube<RigidVector<Double, 2> > &
VisibilityIteratorReadImpl::getBeamOffsets () const
{
    return msIter_p.getBeamOffsets ();
}

inline Bool VisibilityIteratorReadImpl::allBeamOffsetsZero () const
{
    return msIter_p.allBeamOffsetsZero ();
}

inline Int VisibilityIteratorReadImpl::channelGroupSize () const
{
    return channels_p.width_p[msIter_p.spectralWindowId ()];
}
inline Int VisibilityIteratorReadImpl::channelIndex () const
{
    return channels_p.inc_p[msIter_p.spectralWindowId ()] * curChanGroup_p;
}
inline Int VisibilityIteratorReadImpl::nRow () const
{
    return curNumRow_p;
}
inline Int VisibilityIteratorReadImpl::nRowChunk () const
{
    return msIter_p.table ().nrow ();
}
//inline VisibilityIteratorReadImpl &
//VisibilityIteratorReadImpl::velInterpolation (const String & type)
//{
//    vInterpolation_p = type;
//    return *this;
//}
inline IPosition VisibilityIteratorReadImpl::visibilityShape () const
{
    return IPosition (3, nPol_p, channelGroupSize (), curNumRow_p);
}

// <summary>
// VisibilityIterator iterates through one or more writable MeasurementSets
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="VisibilityIteratorReadImpl">VisibilityIteratorReadImpl</linkto>
// </prerequisite>
//
// <etymology>
// The VisibilityIterator is a read/write iterator returning visibilities
// </etymology>
//
// <synopsis>
// VisibilityIterator provides iteration with various sort orders
// for one or more MSs. It has member functions to retrieve the fields
// commonly needed in synthesis calibration and imaging. It is
// derived from the read-only iterator
// <linkto class="VisibilityIteratorReadImpl">VisibilityIteratorReadImpl</linkto>.
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
// #<thrown>
//
// #</thrown>
//
// <todo asof="1997/05/30">
//   <li> cleanup the currently dual interface for visibilities and flags
//   <li> sort out what to do with weights when interpolating
// </todo>

class VisibilityIteratorWriteImpl {

    friend class VisibilityIterator;

public:

    typedef ROVisibilityIterator::DataColumn DataColumn;

    // Constructors.
    // Note: The VisibilityIterator is not initialized correctly by default, you
    // need to call origin () before using it to iterate.

    VisibilityIteratorWriteImpl (VisibilityIterator * vi);

    //VisibilityIteratorWriteImpl (VisibilityIterator * vi);

    VisibilityIteratorWriteImpl (const VisibilityIteratorWriteImpl & other);

    // Destructor

    virtual ~VisibilityIteratorWriteImpl ();

    // Members

    virtual VisibilityIteratorWriteImpl * clone (VisibilityIterator * vi) const;

    virtual Bool isWritable () const {
        return True;
    }

    // Set/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
    virtual void setFlag (const Matrix<Bool> & flag);

    // Set/modify the flags in the data.
    // This sets the flags as found in the MS, Cube (npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow ()).
    virtual void setFlag (const Cube<Bool> & flag);

    // Set/modify the flag row column; dimension Vector (nrow)
    virtual void setFlagRow (const Vector<Bool> & rowflags);

    void setFlagCategory(const Array<Bool>& fc);

    // Set/modify the visibilities.
    // This is possibly only for a 'reference' MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
    virtual void setVis (const Matrix<CStokesVector> & vis, DataColumn whichOne);

    // Set/modify the visibilities
    // This sets the data as found in the MS, Cube (npol,nchan,nrow).
    virtual void setVis (const Cube<Complex> & vis, DataColumn whichOne);

    // Set the visibility and flags, and interpolate from velocities if needed
    virtual void setVisAndFlag (const Cube<Complex> & vis, const Cube<Bool> & flag,
                        DataColumn whichOne);

    // Set/modify the weights
    virtual void setWeight (const Vector<Float> & wt);

    // Set/modify the weightMat
    virtual void setWeightMat (const Matrix<Float> & wtmat);

    // Set/modify the weightSpectrum
    virtual void setWeightSpectrum (const Cube<Float> & wtsp);

    // Set/modify the Sigma
    virtual void setSigma (const Vector<Float> & sig);

    // Set/modify the ncorr x nrow SigmaMat.
    virtual void setSigmaMat (const Matrix<Float> & sigmat);

    virtual void writeBack (VisBuffer *);

protected:

    // A BackWriter is a functor that will extract a piece of information out of its VisBuffer
    // argument and write it out using a "set" method on the supplied VisibilityIterator.
    class BackWriter {

    public:

        virtual ~BackWriter () {}

        virtual void operator () (VisibilityIteratorWriteImpl * vi, VisBuffer * vb) = 0;

    };

    // A simple BackWriterImpl uses a nullary accessor on a VisBuffer.
    template <typename Setter, typename Getter>
    class BackWriterImpl : public BackWriter {
    public:

        BackWriterImpl (Setter setter, Getter getter) : getter_p (getter), setter_p (setter) {}
        void operator () (VisibilityIteratorWriteImpl * vi, VisBuffer * vb) {
            (vi ->* setter_p) ((vb ->* getter_p) ());
        }

    private:

        Getter getter_p;
        Setter setter_p;
    };

    // BackWriterImpl2 is slightly more complicated in that it uses a unary accessor.  The argument
    // to the unary accessor is a member of the ROVisibilityIterator DataColumn enumeration which
    // specifies which visibilty or visCube type is wanted (e.g., observed, model or corrected).
    template <typename Setter, typename Getter>
    class BackWriterImpl2 : public BackWriter {
    public:

        typedef VisibilityIteratorReadImpl::DataColumn DataColumn;

        BackWriterImpl2 (Setter setter, Getter getter, DataColumn dc)
        : dataColumn_p (dc), getter_p (getter), setter_p (setter)
        {}
        void operator () (VisibilityIteratorWriteImpl * vi, VisBuffer * vb) {
            (vi ->* setter_p) ((vb ->* getter_p) (), dataColumn_p);
        }

    private:

        DataColumn dataColumn_p;
        Getter getter_p;
        Setter setter_p;
    };

    // Backwriter(2) creation methods.  These methods make it fairly straightforward to create
    // a BackWriter object.

    template <typename Ret>
    static
    BackWriter *
    makeBackWriter (void (VisibilityIteratorWriteImpl::* setter) (Ret), Ret (VisBuffer::* getter) () const) {
        return new BackWriterImpl <void (VisibilityIteratorWriteImpl:: *) (Ret),
                                   Ret (VisBuffer:: *) () const >
        (setter, getter);
    }

    template <typename Ret>
    static
    BackWriter *
    makeBackWriter2 (void (VisibilityIteratorWriteImpl::* setter) (Ret, VisibilityIteratorReadImpl::DataColumn),
                     Ret (VisBuffer::* getter) () const,
                     ROVisibilityIterator::DataColumn dc) {

        // Define the Getter and Setter types

        typedef void (VisibilityIteratorWriteImpl::* Setter) (Ret, VisibilityIteratorReadImpl::DataColumn);
        typedef Ret (VisBuffer::* Getter) () const;

        return new BackWriterImpl2 < Setter, Getter> (setter, getter, dc);
    }

    VisibilityIteratorReadImpl * getReadImpl ();

    void initializeBackWriters ();

    virtual void attachColumns (const Table & t);

//    void setInterpolatedVisFlag (const Cube<Complex> & vis,
//                                 const Cube<Bool> & flag);
//    void setInterpolatedWeight (const Matrix<Float> & wt);

    // Write the data column (observed, model or corrected);
    // deals with Float or Complex observed data (DATA and FLOAT_DATA).

    void putDataColumn (DataColumn whichOne, const Slicer & slicer,
                        const Cube<Complex> & data);
    void putDataColumn (DataColumn whichOne, const Cube<Complex> & data);

    // column access functions, can be overridden in derived classes
    virtual void putCol (ScalarColumn<Bool> & column, const Vector<Bool> & array);
    virtual void putCol (ArrayColumn<Bool> & column, const Array<Bool> & array);
    virtual void putCol (ArrayColumn<Float> & column, const Array<Float> & array);
    virtual void putCol (ArrayColumn<Complex> & column, const Array<Complex> & array);

    virtual void putCol (ArrayColumn<Bool> & column, const Slicer & slicer, const Array<Bool> & array);
    virtual void putCol (ArrayColumn<Float> & column, const Slicer & slicer, const Array<Float> & array);
    virtual void putCol (ArrayColumn<Complex> & column, const Slicer & slicer, const Array<Complex> & array);

    // non-virtual, no reason to template this function because Bool is the only type needed
    void putColScalar (ScalarColumn<Bool> & column, const Vector<Bool> & array);

    //This puts a model into the descriptor of the actual ms
    //Set iscomponentlist to True if the record represent a componentlist
    //if False then it is a FTMachine Record that holds the model image
    // a [-1] vector in validfields mean the model applies to all fields of the active ms 
    virtual void putModel(const RecordInterface& rec, Bool iscomponentlist=True, Bool incremental=False);





private:

    class Columns {

    public:

        Columns & operator= (const Columns & other);

        ArrayColumn<Complex> corrVis_p;
        ScalarColumn<Bool>   flagRow_p;
        ArrayColumn<Bool>    flag_p;
        ArrayColumn<Bool>    flagCategory_p;
        ArrayColumn<Float>   floatVis_p;
        ArrayColumn<Complex> modelVis_p;
        ArrayColumn<Float>   sigma_p;
        ArrayColumn<Complex> vis_p;
        ArrayColumn<Float>   weightSpectrum_p;
        ArrayColumn<Float>   weight_p;

    };

    std::map <VisBufferComponents::EnumType, BackWriter *> backWriters_p;
    Columns columns_p;
    VisibilityIterator * vi_p; // [use]
};

} //# NAMESPACE CASA - END

#endif


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

#ifndef MSVIS_VISIBILITYITERATORIMPL2_H
#define MSVIS_VISIBILITYITERATORIMPL2_H

#include <casa/aips.h>
#include <ms/MSOper/MSDerivedValues.h>
#include <msvis/MSVis/ViImplementation2.h>
#include <msvis/MSVis/ViColumns2.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <msvis/MSVis/VisBuffer2.h>

#include <tuple>
#include <map>
#include <vector>

namespace casacore{

template <typename T> class ArrayColumn;
template <typename T> class CountedPtr;
class VisModelDataI;
template <typename T> class ROArrayColumn;
template <typename T, Int N> class RigidVector;
template <typename T, Int N> class SquareMatrix;
}

namespace casa { //# NAMESPACE CASA - BEGIN

class MsIter;
class VisModelDataI;

namespace vi {

//# forward decl

class VisBuffer2;

class ChannelSelector;
class ChannelSelectorCache;
class PointingDirectionCache;
class PointingSource;
class SpectralWindowChannelsCache;
class SpectralWindowChannels;
class SubtableColumns;

// <summary>
// VisibilityIterator2 iterates through one or more readonly MeasurementSets
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MSIter">MSIter</linkto>
//   <li> <linkto class="casacore::MeasurementSet">casacore::MeasurementSet</linkto>
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
// For imaging and calibration you need to access an casacore::MS in some consistent
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

class VisibilityIteratorImpl2 : public ViImplementation2 {

    friend class AsyncEnabler;
    friend class ViImplAsync2;
    friend class VisibilityIterator2;
    friend class VLAT; // allow VI lookahead thread class to access protected functions
    friend class VisBuffer2Adapter;
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

    typedef VisibilityIterator2::DataColumn DataColumn;

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
    //        casacore::Bool oldEnabledState_p;
    //        VisibilityIterator2 * roVisibilityIterator2_p;
    //    };

    // Default constructor - useful only to assign another iterator later
    ////VisibilityIteratorImpl2 ();

    // Construct from an casacore::MS and a casacore::Block of casacore::MS column enums specifying the
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
//    VisibilityIteratorImpl (const casacore::MeasurementSet & ms,
//                                const casacore::Block<casacore::Int> & sortColumns,
//                                casacore::Double timeInterval = 0);

    // Same as above, but with the option of using the raw order of ms
    // (addDefaultSortCols=false).
//    VisibilityIteratorImpl (const casacore::MeasurementSet & ms,
//                                const casacore::Block<casacore::Int> & sortColumns, const casacore::Bool addDefaultSortCols,
//                                casacore::Double timeInterval = 0);

    // Same as previous constructors, but with multiple MSs to iterate over.
//    VisibilityIteratorImpl (const casacore::Block<casacore::MeasurementSet> & mss,
//                                const casacore::Block<casacore::Int> & sortColumns,
//                                casacore::Double timeInterval = 0);

    VisibilityIteratorImpl2 (const casacore::Block<const casacore::MeasurementSet *> & mss,
                             const SortColumns & sortColumns,
                             casacore::Double timeInterval,
                             VisBufferType vbType,
                             casacore::Bool isWritable,
			     casacore::Bool useMSIter2=false);

//    // Copy construct. This calls the assigment operator.
//    VisibilityIteratorImpl2 (const VisibilityIteratorImpl2 & other,
//                                  VisibilityIterator2 * rovi);

    // Destructor

    virtual ~VisibilityIteratorImpl2 ();

        // Members

    // Report the the ViImplementation type
    //  TBD:  indicate writable?
    virtual casacore::String ViiType() const { return casacore::String("DiskIO()"); };

    virtual casacore::Bool isWritable () const;


    // Reset iterator to origin/start of data (of current chunk)
    virtual void origin ();
    // Reset iterator to true start of data (first chunk)
    virtual void originChunks ();

    // Return the time interval (in seconds) used for iteration.
    // This is not the same as the INTERVAL column.

    virtual casacore::Double getInterval() const;

    // Set or reset the time interval (in seconds) to use for iteration.
    // You should call originChunks () to reset the iteration after
    // calling this.
    virtual void setInterval (casacore::Double timeInterval);


    // Set the 'blocking' size for returning data.
    // With the default (0) only a single integration is returned at a time, this
    // is what is currently required for the calibration software. With blocking
    // set, up to nRows can be returned in one go. The chunk
    // size determines the actual maximum.
    virtual void setRowBlocking (casacore::Int nRows = 0);

    virtual casacore::Bool existsColumn (VisBufferComponent2 id) const;

    // Return false if no more data (in current chunk)
    virtual casacore::Bool more () const;

    virtual Subchunk getSubchunkId () const;

    virtual const SortColumns & getSortColumns() const;

    virtual void setFrequencySelections (const FrequencySelections & selection);

    // Return false if no more 'Chunks' of data left
    virtual casacore::Bool moreChunks () const;

    // Check if ms has change since last iteration

    virtual casacore::Bool isNewArrayId () const;
    virtual casacore::Bool isNewFieldId () const;
    virtual casacore::Bool isNewMs () const;
    virtual casacore::Bool isNewSpectralWindow () const;

    virtual casacore::Int msId () const;
    virtual casacore::Int getNMs () const;

    virtual VisBuffer2 * getVisBuffer ();
    virtual VisBuffer2 * getVisBuffer (const VisibilityIterator2 *);

    //reference to actual ms in interator
    virtual const casacore::MeasurementSet & ms () const;

    // Name of the MS in the interator
    virtual casacore::String msName() const;

    // advance the iteration

    virtual void next ();

    // Advance to the next Chunk of data
    virtual void nextChunk ();

    // Report Name of slowest column that changes at end of current iteration
    virtual casacore::String keyChange() const { return msIter_p->keyChange(); };

    // Return antenna1
    virtual void antenna1 (casacore::Vector<casacore::Int> & ant1) const;

    // Return antenna2
    virtual void antenna2 (casacore::Vector<casacore::Int> & ant2) const;

    // Return feed1
    virtual void feed1 (casacore::Vector<casacore::Int> & fd1) const;

    // Return feed2
    virtual void feed2 (casacore::Vector<casacore::Int> & fd2) const;

    // Return feed configuration matrix for specified antenna
    void jonesC (casacore::Vector<casacore::SquareMatrix<casacore::Complex, 2> > & cjones) const;

    // Return receptor angles for all antennae and feeds
    // First axis of the cube is a receptor number,
    // 2nd is antennaId, 3rd is feedId
    // Note: the method is intended to provide an access to MSIter::receptorAngles
    // for VisBuffer in the multi-feed case. It may be worth to change the
    // interface of feed_pa to return the information for all feeds.
    virtual const casacore::Cube<casacore::Double> & receptorAngles () const;

    // return a string mount identifier for each antenna
    virtual const casacore::Vector<casacore::String> & antennaMounts () const;

    // Return a cube containing pairs of coordinate offsets for each
    // receptor of each feed (values are in radians, coordinate system is fixed
    // with antenna and is the same one as used to define the BEAM_OFFSET
    // parameter in the feed table). The cube axes are receptor, antenna, feed.
    virtual const casacore::Cube<casacore::RigidVector<casacore::Double, 2> > & getBeamOffsets () const;

    virtual std::pair<bool, casacore::MDirection> getPointingAngle (int antenna, double time) const;

    // true if all elements of the cube returned by getBeamOffsets are zero
    virtual casacore::Bool allBeamOffsetsZero () const;

    // Return feed parallactic angles casacore::Vector (nant) (1 feed/ant)
    virtual const casacore::Vector<casacore::Float> & feed_pa (casacore::Double time) const;

    // Return nominal parallactic angle at specified time
    // (does not include feed position angle offset--see feed_pa)
    // A global value for all antennas (e.g., small array)
    virtual const casacore::Float & parang0 (casacore::Double time) const;

    // Per antenna:
    virtual const casacore::Vector<casacore::Float> & parang (casacore::Double time) const;

    // Return the antenna AZ/EL casacore::Vector (nant)
    virtual casacore::MDirection azel0 (casacore::Double time) const;

    virtual const casacore::Vector<casacore::MDirection> & azel (casacore::Double time) const;

    // Return the hour angle for the specified time
    virtual casacore::Double hourang (casacore::Double time) const;

    // Return the current FieldId
    virtual void fieldIds (casacore::Vector<casacore::Int>&) const;

    // Return the current ArrayId
    virtual void arrayIds (casacore::Vector<casacore::Int>&) const;

    // Return the current Field Name
    virtual casacore::String fieldName () const;

    // Return the current Source Name
    virtual casacore::String sourceName () const;

    // Return flag for each polarization, channel and row
    virtual void flag (casacore::Cube<casacore::Bool> & flags) const;

    // Return flag for each channel & row
    virtual void flag (casacore::Matrix<casacore::Bool> & flags) const;

    // Determine whether FLAG_CATEGORY is valid.
    casacore::Bool flagCategoryExists () const;

    // Return flags for each polarization, channel, category, and row.
    virtual void flagCategory (casacore::Array<casacore::Bool> & flagCategories) const;

    // Return row flag
    virtual void flagRow (casacore::Vector<casacore::Bool> & rowflags) const;

    // Return scan number
    virtual void scan (casacore::Vector<casacore::Int> & scans) const;

    // Return the OBSERVATION_IDs
    virtual void observationId (casacore::Vector<casacore::Int> & obsids) const;

    // Return the PROCESSOR_IDs
    virtual void processorId (casacore::Vector<casacore::Int> & procids) const;

    // Return the STATE_IDs
    virtual void stateId (casacore::Vector<casacore::Int> & stateids) const;

    // Return the current phase center as an MDirection
    virtual const casacore::MDirection & phaseCenter () const;

    // Return frame for polarization (returns PolFrame enum)
    virtual casacore::Int polFrame () const;

    // Return the correlation type (returns casacore::Stokes enums)
    virtual void corrType (casacore::Vector<casacore::Int> & corrTypes) const;

    // Return sigma
    virtual void sigma (casacore::Matrix<casacore::Float> & sig) const;

    // Return current SpectralWindow
    virtual casacore::Int spectralWindow () const;

    virtual void spectralWindows (casacore::Vector<casacore::Int> & spws) const;

    // Return current Polarization Id
    virtual casacore::Int polarizationId () const;

    // Return current DataDescription Id
    virtual casacore::Int dataDescriptionId () const;

    virtual void dataDescriptionIds (casacore::Vector<casacore::Int> & ddis) const;

    // Return MJD midpoint of interval.
    virtual void time (casacore::Vector<casacore::Double> & t) const;

    // Return MJD centroid of interval.
    virtual void timeCentroid (casacore::Vector<casacore::Double> & t) const;

    // Return nominal time interval
    virtual void timeInterval (casacore::Vector<casacore::Double> & ti) const;

    // Return actual time interval
    virtual void  exposure (casacore::Vector<casacore::Double> & expo) const;

    // Return the visibilities as found in the casacore::MS, casacore::Cube (npol,nchan,nrow).
    virtual void visibilityCorrected (casacore::Cube<casacore::Complex> & vis) const;
    virtual void visibilityModel (casacore::Cube<casacore::Complex> & vis) const;
    virtual void visibilityObserved (casacore::Cube<casacore::Complex> & vis) const;

    // Return FLOAT_DATA as a casacore::Cube (npol, nchan, nrow) if found in the MS.
    virtual void floatData (casacore::Cube<casacore::Float> & fcube) const;

    // Return the visibility 4-vector of polarizations for each channel.
    // If the casacore::MS doesn't contain all polarizations, it is assumed it
    // contains one or two parallel hand polarizations.
//    virtual void visibilityCorrected (casacore::Matrix<CStokesVector> & vis) const;
//    virtual void visibilityModel (casacore::Matrix<CStokesVector> & vis) const;
//    virtual void visibilityObserved (casacore::Matrix<CStokesVector> & vis) const;

    // Return the shape of the visibility Cube
    virtual casacore::IPosition visibilityShape () const;

    // Return u,v and w (in meters)

    virtual void uvw (casacore::Matrix<casacore::Double> & uvwmat) const;

    // Return weight
    virtual void weight (casacore::Matrix<casacore::Float> & wt) const;

    // Determine whether WEIGHT_SPECTRUM exists.
    casacore::Bool weightSpectrumExists () const;

    // Determine whether SIGMA_SPECTRUM exists.
    casacore::Bool sigmaSpectrumExists () const;

    // Return weightspectrum (a weight for each channel)
    virtual void weightSpectrum (casacore::Cube<casacore::Float> & wtsp) const;

    // Return sigmaspectrum (a sigma for each channel)
    virtual void sigmaSpectrum (casacore::Cube<casacore::Float> & sigsp) const;


    virtual void setWeightScaling (casacore::CountedPtr<WeightScaling> weightscaling);
    virtual casacore::CountedPtr<WeightScaling> getWeightScaling () const;
    virtual casacore::Bool hasWeightScaling () const;

    // Return imaging weight (a weight for each channel)
    //virtual casacore::Matrix<casacore::Float> & imagingWeight (casacore::Matrix<casacore::Float> & wt) const;
    const VisImagingWeight & getImagingWeightGenerator () const;

    // Return true if FieldId/Source has changed since last iteration
    virtual casacore::Bool newFieldId () const;

    // Return true if arrayID has changed since last iteration
    virtual casacore::Bool newArrayId () const;

    // Return true if SpectralWindow has changed since last iteration
    virtual casacore::Bool newSpectralWindow () const;

    // Return the number of rows in the current iteration
    virtual casacore::Int nRows () const;

    // Return the row ids as from the original root table. This is useful
    // to find correspondance between a given row in this iteration to the
    // original ms row
    virtual void getRowIds (casacore::Vector<casacore::uInt> & rowids) const;

    // Return the numbers of rows in the current chunk

    virtual casacore::Int nRowsInChunk () const;

    // Return the number of sub-intervals in the current chunk

    //virtual casacore::Int nSubInterval () const;

    // Call to use the slurp i/o method for all scalar columns. This
    // will set the casacore::BucketCache cache size to the full column length
    // and cause the full column to be cached in memory, if
    // any value of the column is used. In case of out-of-memory,
    // it will automatically fall-back on the smaller cache size.
    // Slurping the column is to be considered as a work-around for the
    // casacore::Table i/o code, which uses casacore::BucketCache and performs extremely bad
    // for random access. Slurping is useful when iterating non-sequentially
    // an casacore::MS or parts of an casacore::MS, it is not tested with multiple MSs.
    virtual void slurp () const;

    // Get the spw, start  and nchan for all the ms's is this Visiter that
    // match the frequecy "freqstart-freqStep" and "freqEnd+freqStep" range
    // Can help in doing channel selection above..
    // freqFrame is the frame the caller frequency values are in (freqStart and freqEnd)
    // These will be converted to the frame of the selected spw to match

//    virtual void getSpwInFreqRange (casacore::Block<casacore::Vector<casacore::Int> > & spw,
//                                    casacore::Block<casacore::Vector<casacore::Int> > & start,
//                                    casacore::Block<casacore::Vector<casacore::Int> > & nchan,
//                                    casacore::Double freqStart, casacore::Double freqEnd,
//                                    casacore::Double freqStep, casacore::MFrequency::Types freqFrame = casacore::MFrequency::LSRK) const;

    // Get the range of frequency convered by the selected data in the frame requested

//    virtual void getFreqInSpwRange(casacore::Double& freqStart, casacore::Double& freqEnd, casacore::MFrequency::Types freqframe = casacore::MFrequency::LSRK) const;

    // Access the current casacore::ROMSColumns object in MSIter
    virtual const vi::SubtableColumns & subtableColumns () const;

    // get back the selected spectral windows and spectral channels for
    // current ms

    virtual const SpectralWindowChannels & getSpectralWindowChannels (casacore::Int msId, casacore::Int spectralWindowId) const;

    //assign a VisImagingWeight object to this iterator
    virtual void useImagingWeight (const VisImagingWeight & imWgt);
    //return number  of Ant
    virtual casacore::Int nAntennas () const;
    //Return number of rows in all selected ms's
    virtual casacore::Int nRowsViWillSweep () const;

    // Return number of spws, polids, ddids

    virtual casacore::Int nSpectralWindows () const;
    virtual casacore::Int nPolarizationIds () const;
    virtual casacore::Int nDataDescriptionIds () const;

    virtual casacore::MEpoch getEpoch () const;
    casacore::MFrequency::Types getObservatoryFrequencyType () const; //???
    casacore::MPosition getObservatoryPosition () const;
    casacore::Vector<casacore::Float> getReceptor0Angle ();

    // Write/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
//    virtual void writeFlag (const casacore::Matrix<casacore::Bool> & flag);

    // Write/modify the flags in the data.
    // This writes the flags as found in the casacore::MS, casacore::Cube (npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow ()).
    virtual void writeFlag (const casacore::Cube<casacore::Bool> & flag);

    // Write/modify the flag row column; dimension casacore::Vector (nrow)
    virtual void writeFlagRow (const casacore::Vector<casacore::Bool> & rowflags);

    void writeFlagCategory(const casacore::Array<casacore::Bool>& fc);

    // Write/modify the visibilities.
    // This is possibly only for a 'reference' casacore::MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the casacore::MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
//    void writeVisCorrected (const casacore::Matrix<CStokesVector> & visibilityStokes);
//    void writeVisModel (const casacore::Matrix<CStokesVector> & visibilityStokes);
//    void writeVisObserved (const casacore::Matrix<CStokesVector> & visibilityStokes);

    // Write/modify the visibilities
    // This writes the data as found in the casacore::MS, casacore::Cube (npol,nchan,nrow).
    virtual void writeVisCorrected (const casacore::Cube<casacore::Complex> & vis);
    virtual void writeVisModel (const casacore::Cube<casacore::Complex> & vis);
    virtual void writeVisObserved (const casacore::Cube<casacore::Complex> & vis);

    // Write/modify the weights
    virtual void writeWeight (const casacore::Matrix<casacore::Float> & wt);

    // Write/modify the weightMat
    //virtual void writeWeightMat (const casacore::Matrix<casacore::Float> & wtmat);

    // Write/modify the weightSpectrum
    virtual void writeWeightSpectrum (const casacore::Cube<casacore::Float> & wtsp);

    // Initialize the weightSpectrum column
    virtual void initWeightSpectrum (const casacore::Cube<casacore::Float> & weightSpectrum);

    // Write/modify the sigmaSpectrum
    virtual void writeSigmaSpectrum (const casacore::Cube<casacore::Float> & sigsp);

    // Initialize the sigmaSpectrum column
    virtual void initSigmaSpectrum (const casacore::Cube<casacore::Float> & sigmaSpectrum);

    // Write/modify the Sigma
    virtual void writeSigma (const casacore::Matrix<casacore::Float> & sig);

    // Write/modify the ncorr x nrow SigmaMat.
    //virtual void writeSigmaMat (const casacore::Matrix<casacore::Float> & sigmat);

    virtual void writeModel(const casacore::RecordInterface& rec, casacore::Bool iscomponentlist=true,
                            casacore::Bool incremental=false);

    virtual void writeBackChanges (VisBuffer2 *);

protected:

    typedef std::tuple <casacore::Vector<casacore::Int>, casacore::Vector<casacore::Int>, casacore::Vector<casacore::Int>, casacore::Vector<casacore::Int> > ChannelInfo;

    void addDataSelection (const casacore::MeasurementSet & ms);

    void allSpectralWindowsSelected (casacore::Vector<casacore::Int> & spectralWindows,
                                     casacore::Vector<casacore::Int> & nChannels) const; /*KLUGE*/

    void attachColumnsSafe (const casacore::Table & t);

    // attach the column objects to the currently selected table

    virtual void attachColumns (const casacore::Table & t);

    // returns the table, to which columns are attached,
    // can be overridden in derived classes

    virtual const casacore::Table attachTable () const;

    virtual void applyPendingChanges ();

    // set the iteration state

    virtual void configureNewChunk ();

    // set the currently selected table

    virtual void configureNewSubchunk ();

    const ChannelSelector *
    createDefaultChannelSelector (casacore::Double time, casacore::Int msId, casacore::Int spectralWindowId);

    virtual const vi::ChannelSelector * determineChannelSelection (casacore::Double time,
                                                                   casacore::Int spectralWindowId = -1,
                                                                   casacore::Int polarizationId = -1,
                                                                   casacore::Int msId = -1) const;

    bool fillFromVirtualModel (casacore::Cube <casacore::Complex> & value) const;
    casacore::Slice findChannelsInRange (casacore::Double lowerFrequency, casacore::Double upperFrequency,
                               const vi::SpectralWindowChannels & spectralWindowChannels) const;

    ChannelInfo
    getChannelInformation(casacore::Bool now) const;

    ChannelInfo
    getChannelInformationUsingFrequency (casacore::Bool now) const;

    // Methods to get the data out of a table column according to whatever selection
    // criteria (e.g., slicing) is in effect.

    template <typename T>
    void getColumnRows (const casacore::ROArrayColumn<T> & column, casacore::Array<T> & array) const;

    template <typename T>
    void
    getColumnRowsMatrix (const casacore::ROArrayColumn<T> & column, casacore::Matrix<T> & array,
                         casacore::Bool correlationSlicing) const;

    template <typename T>
    void getColumnRows (const casacore::ROScalarColumn<T> & column, casacore::Vector<T> & array) const;

    casacore::Vector<casacore::Double> getFrequencies (casacore::Double time, casacore::Int frameOfReference,
                                   casacore::Int spectralWindowId, casacore::Int msId) const; // helper method

    casacore::Vector<casacore::Int> getChannels (casacore::Double time, casacore::Int frameOfReference,
                             casacore::Int spectralWindowId, casacore::Int msId) const;
    casacore::Vector<casacore::Int> getCorrelations () const;
    casacore::Vector<casacore::Stokes::StokesTypes> getCorrelationTypesDefined () const;
    casacore::Vector<casacore::Stokes::StokesTypes> getCorrelationTypesSelected () const;

    int getMeasurementFrame (int spectralWindowId) const;

    casacore::Int getPolarizationId (casacore::Int spectralWindowId, casacore::Int msId) const;

    casacore::Int getReportingFrameOfReference () const;

    // Returns the casacore::MS objects that this VI is iterating over.

    casacore::Block <casacore::MeasurementSet> getMeasurementSets () const;

    // Provides access to the casacore::MS-derived values object

    const casacore::MSDerivedValues & getMsd () const; // for use by Async I/O *ONLY*

    // Get privileged (non-const) access to the containing ROVI

    VisibilityIterator2 * getViP () const;

//    void getVisibilityAsStokes (casacore::Matrix<CStokesVector> & visibilityStokes,
//                                const casacore::ROArrayColumn<casacore::Complex> & column) const;

    // Ctor auxiliary method

    virtual void initialize (const casacore::Block<const casacore::MeasurementSet *> & mss,
			     casacore::Bool useMSIter2=false);

    // Returns true if casacore::MS Iterator is currently pointing to a selected
    // spectral window

    casacore::Bool isInASelectedSpectralWindow () const;

    // Creates a channel selection for the current subchunk based on the channel
    // or frequency selection made by the user.

    vi::ChannelSelector *
    makeChannelSelectorC (const FrequencySelection & selection,
                          casacore::Double time, casacore::Int msId, casacore::Int spectralWindowId,
                          casacore::Int polarizationId) const;

    vi::ChannelSelector *
    makeChannelSelectorF (const FrequencySelection & selection,
                          casacore::Double time, casacore::Int msId, casacore::Int spectralWindowId,
                          casacore::Int polarizationId) const;

    casacore::MFrequency::Convert makeFrequencyConverter (casacore::Double time,
                                                int spectralWindowId,
                                                casacore::Int otherFrameOfReference,
                                                casacore::Bool toObservedFrame, casacore::Unit) const;

    // Allow access to the casacore::MSColumns object; for use by VisBuffer2Adapter *KLUGE*

    const casacore::ROMSColumns * msColumnsKluge () const;

    // Method to reset the VI back to the start.  Unlike the public version
    // there is a parameter to allow forcing the rewind even if the
    // casacore::MS Iter is already at the origin.

    virtual void originChunks (casacore::Bool forceRewind);

    // Advances the casacore::MS Iterator until it points at a spectral window
    // that is part of the frequency selection.

    void positionMsIterToASelectedSpectralWindow ();

    // Sets the default frequency reporting frame of reference.  This
    // affects the default frame for obtaining the frequencies in a
    // VisBuffer.

    void setReportingFrameOfReference (casacore::Int);

    // Adjusts the tile cache for some columns so that the cache size is
    // optimized for the current input state (e.g., a new data description).

    virtual void setTileCache ();
    void setMsCacheSizes (const casacore::MeasurementSet & ms,
                          vector<casacore::MSMainEnums::PredefinedColumns> columnIds);
    void setMsColumnCacheSizes (const casacore::MeasurementSet&, const string &);

    // Throws exception if there is a pending (i.e., unapplied) change to
    // the VI's properties.  Called when the VI is advanced since the user
    // probably forgot to apply the changes.

    virtual void throwIfPendingChanges ();

    // Returns true if the named column uses a tiled data manager in the specified MS

    casacore::Bool usesTiledDataManager (const casacore::String & columnName, const casacore::MeasurementSet & ms) const;


//    +========================|
//    |                        |
//    | Output Related Methods |
//    |                        |
//    +========================|

    // A BackWriter is a functor that will extract a piece of information out of its VisBuffer
    // argument and write it out using a "set" method on the supplied VisibilityIterator2.
    class BackWriter {

    public:

        virtual ~BackWriter () {}

        virtual void operator () (VisibilityIteratorImpl2 * vi, VisBuffer2 * vb) = 0;

    };

    // A simple BackWriterImpl2 uses a nullary accessor on a VisBuffer.
    template <typename Setter, typename Getter>
    class BackWriterImpl : public BackWriter {
    public:

        BackWriterImpl (Setter setter, Getter getter) : getter_p (getter), setter_p (setter) {}
        void operator () (VisibilityIteratorImpl2 * vi, VisBuffer2 * vb) {
            (vi ->* setter_p) ((vb ->* getter_p) ());
        }

    private:

        Getter getter_p;
        Setter setter_p;
    };

    // BackWriterImpl2 is slightly more complicated in that it uses a unary accessor.  The argument
    // to the unary accessor is a member of the VisibilityIterator2 DataColumn enumeration which
    // specifies which visibilty or visCube type is wanted (e.g., observed, model or corrected).
    template <typename Setter, typename Getter>
    class BackWriterImpl2 : public BackWriter {
    public:

        typedef VisibilityIteratorImpl2::DataColumn DataColumn;

        BackWriterImpl2 (Setter setter, Getter getter, DataColumn dc)
        : dataColumn_p (dc), getter_p (getter), setter_p (setter)
        {}
        void operator () (VisibilityIteratorImpl2 * vi, VisBuffer2 * vb) {
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
    makeBackWriter (void (VisibilityIteratorImpl2::* setter) (Ret), Ret (VisBuffer2::* getter) () const) {
        return new BackWriterImpl <void (VisibilityIteratorImpl2:: *) (Ret),
                                   Ret (VisBuffer2:: *) () const >
        (setter, getter);
    }

    template <typename Ret>
    static
    BackWriter *
    makeBackWriter2 (void (VisibilityIteratorImpl2::* setter) (Ret, VisibilityIteratorImpl2::DataColumn),
                     Ret (VisBuffer2::* getter) () const,
                     VisibilityIterator2::DataColumn dc) {

        // Define the Getter and Setter types

        typedef void (VisibilityIteratorImpl2::* Setter) (Ret, VisibilityIteratorImpl2::DataColumn);
        typedef Ret (VisBuffer2::* Getter) () const;

        return new BackWriterImpl2 < Setter, Getter> (setter, getter, dc);
    }

    void initializeBackWriters ();

    template <typename T>
    void putColumnRows (casacore::ArrayColumn<T> & column, const casacore::Array<T> & array);

    template <typename T>
    void putColumnRows (casacore::ArrayColumn<T> & column, const casacore::Matrix<T> & array);

    template <typename T>
    void putColumnRows (casacore::ScalarColumn<T> & column, const casacore::Vector <T> & array);

//    void convertVisFromStokes (const casacore::Matrix<CStokesVector> & visibilityStokes,
//                               casacore::Cube<casacore::Complex> & visCube);

    class Cache {

    public:

        Cache ();

        Cache & operator= (const Cache & other);

        void flush ();

        casacore::MDirection         azel0_p;
        casacore::Double             azel0Time_p;
        casacore::Vector<casacore::MDirection> azel_p;
        casacore::Double             azelTime_p;
        casacore::Vector<casacore::uInt>       chunkRowIds_p; // Row numbers of underlying casacore::MS; used to map
                                          // form chunk rows to casacore::MS rows.  See rowIds method.
        casacore::Vector<casacore::Float>      feedpa_p;
        casacore::Double             feedpaTime_p;
        casacore::Double             hourang_p;
        casacore::Double             hourangTime_p;
        casacore::Matrix<casacore::Float>      imagingWeight_p;
        casacore::Bool               msHasFlagCategory_p;   // Does the current casacore::MS have a valid FLAG_CATEGORY?
        casacore::Bool               msHasWeightSpectrum_p; // Does the current casacore::MS have a valid WEIGHT_SPECTRUM?
        casacore::Bool               msHasSigmaSpectrum_p; // Does the current casacore::MS have a valid SIGMA_SPECTRUM?
        casacore::Float              parang0_p;
        casacore::Double             parang0Time_p;
        casacore::Vector<casacore::Float>      parang_p;
        casacore::Double             parangTime_p;
    };

    class PendingChanges {

    public:

        PendingChanges ();
        ~PendingChanges ();

        PendingChanges * clone () const;
        casacore::Bool empty () const;

        pair<casacore::Bool, FrequencySelections *> popFrequencySelections (); // yields ownership
        pair<casacore::Bool, casacore::Double> popInterval ();
        pair<casacore::Bool, casacore::Int> popNRowBlocking ();

        void setFrequencySelections (FrequencySelections *); // takes ownership
        void setInterval (casacore::Double);
        void setNRowBlocking (casacore::Int);

    private:

        enum {Empty = -1};

        FrequencySelections * frequencySelections_p;
        casacore::Bool frequencySelectionsPending_p;
        casacore::Double interval_p;
        casacore::Int nRowBlocking_p;
    };

    typedef casacore::Block <casacore::MeasurementSet> MeasurementSets;

    class RowBounds {
    public:

        RowBounds () : chunkNRows_p (-1), subchunkBegin_p (-1),
                       subchunkEnd_p (-1), subchunkNRows_p (-1),
                       subchunkRows_p (0, 0),
                       timeMax_p (-1), timeMin_p (-1)
        {}

        casacore::Int chunkNRows_p;      // last row in current chunk
        casacore::Int subchunkBegin_p; // first row in current subchunk
        casacore::Int subchunkEnd_p;   // last row in current subchunk
        casacore::Int subchunkNRows_p; // # rows in subchunk
        casacore::RefRows subchunkRows_p; // subchunk's table row numbers
        casacore::Vector<casacore::Double> times_p; // times for each row in the chunk
        casacore::Double timeMax_p;       // max timestamp in the chunk
        casacore::Double timeMin_p;       // min timechunk in the chunk

    };

    casacore::Bool                          autoTileCacheSizing_p;
    std::map <VisBufferComponent2, BackWriter *> backWriters_p;
    mutable Cache                 cache_p; // general copllection of cached values
    const ChannelSelector *       channelSelector_p; // [use] current channel selector for this casacore::MS & Spw
    ChannelSelectorCache *        channelSelectorCache_p; // [own] cache of recently used channel selectors
    ViColumns2                    columns_p; // The main columns for the current MS
    casacore::Bool                          floatDataFound_p; // true if a float data column was found
    FrequencySelections *         frequencySelections_p; // [own] Current frequency selection
    VisImagingWeight              imwgt_p;    // object to calculate imaging weight
    mutable casacore::Int                   measurementFrame_p; // cached value of observatory type
    MeasurementSets               measurementSets_p; // [use]
    VisModelDataI *               modelDataGenerator_p; // [own]
    casacore::Bool                          more_p; // true if more data in this chunk
    casacore::Int                           msIndex_p; // array index of current MS
    casacore::Bool                          msIterAtOrigin_p; // true if casacore::MS Iter is a start of first MS
    casacore::CountedPtr<casacore::MSIter>            msIter_p; // casacore::MS Iter that underlies the VI (sweeps in chunks)
    mutable casacore::MSDerivedValues       msd_p; // Helper class holding casacore::MS derived values.
    casacore::Int                           nCorrelations_p;
    casacore::Int                           nRowBlocking_p; // suggested # of rows in a subchunk
    PendingChanges                pendingChanges_p; // holds pending changes to VI properties
    mutable std::unique_ptr<PointingDirectionCache>  pointingDirectionCache_p;
    mutable std::unique_ptr<PointingSource>  pointingSource_p;
    casacore::Int                           reportingFrame_p; // default frequency reporting (not selecting)
                                                    // frame of reference
    RowBounds                     rowBounds_p; // Subchunk row management object (see above)
    SortColumns                   sortColumns_p; // sort columns specified when creating VI
    mutable SpectralWindowChannelsCache * spectralWindowChannelsCache_p; // [own] Info about spectral windows
    Subchunk                      subchunk_p; // (chunkN #, subchunk #) pair
    SubtableColumns *             subtableColumns_p; // [own] Allows const access to casacore::MS's subtable columns
    casacore::Vector<casacore::Bool>                  tileCacheIsSet_p; // Flags indicating whether tile cache set for this column
    casacore::MeasRef<casacore::MEpoch>               timeFrameOfReference_p;
    casacore::Double                        timeInterval_p;
    VisBuffer2 *                  vb_p;  // [own] VisBuffer attached to this VI
    casacore::CountedPtr<WeightScaling>     weightScaling_p;
    casacore::Bool                          writable_p;
};

} // end namespace vi

} //# NAMESPACE CASA - END

#endif


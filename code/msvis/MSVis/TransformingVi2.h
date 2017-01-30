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

#if ! defined (MSVIS_TransformingVi2_H_121115_0950)
#define MSVIS_TransformingVi2_H_121115_0950

#include <casa/aips.h>
#include <msvis/MSVis/ViImplementation2.h>

#include <map>
#include <vector>

#include <measures/Measures/Stokes.h>

namespace casacore{

template <typename T> class Vector;
}

namespace casa { //# NAMESPACE CASA - BEGIN


namespace vi {

//# forward decl

class VisBuffer2;

class ChannelSelector;
class ChannelSelectorCache;
typedef casacore::Vector<casacore::Vector <casacore::Slice> > ChannelSlicer;
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

class TransformingVi2 : public ViImplementation2 {

public:

    // Destructor

    virtual ~TransformingVi2 ();

    // Report the the ViImplementation type
    //  (should be specialized in child classes)
    virtual casacore::String ViiType() const { return casacore::String("UnknownTrans( ")+getVii()->ViiType()+" )"; };

    //   +==================================+
    //   |                                  |
    //   | Iteration Control and Monitoring |
    //   |                                  |
    //   +==================================+


    // Methods to control and monitor subchunk iteration

    virtual void origin ();
    virtual casacore::Bool more () const;
    virtual void next ();
    virtual Subchunk getSubchunkId () const;

    // Methods to control chunk iterator

    virtual void originChunks (casacore::Bool forceRewind = false);
    virtual casacore::Bool moreChunks () const;
    virtual void nextChunk ();

    virtual casacore::Bool isWritable () const;

    // Return the time interval (in seconds) used for iteration.
    // This is not the same as the INTERVAL column.  Setting the
    // the interval requires calling origin chunks before performing
    // further iterator.

    virtual double getInterval() const;
    virtual void setInterval (double timeInterval);

    // Select the channels to be returned.  Requires calling originChunks before
    // performing additional iteration.

    virtual void setFrequencySelections (const FrequencySelections & selection);

    // Set the 'blocking' size for returning data.
    // With the default (0) only a single integration is returned at a time, this
    // is what is currently required for the calibration software. With blocking
    // set, up to nRows can be returned in one go. The chunk
    // size determines the actual maximum.

    virtual void setRowBlocking (casacore::Int nRows);

    virtual casacore::Bool existsColumn (VisBufferComponent2 id) const;

    virtual const SortColumns & getSortColumns() const;

    virtual casacore::Bool isNewArrayId () const;
    virtual casacore::Bool isNewFieldId () const;
    virtual casacore::Bool isNewMs () const;
    virtual casacore::Bool isNewSpectralWindow () const;

    // Return the number of rows in the current iteration

    virtual casacore::Int nRows () const;

    // Return the row ids as from the original root table. This is useful
    // to find correspondance between a given row in this iteration to the
    // original ms row

    virtual void getRowIds (casacore::Vector<casacore::uInt> & rowids) const;

    virtual VisBuffer2 * getVisBuffer (const VisibilityIterator2 *);
    virtual VisBuffer2 * getVisBuffer ();


    //   +=========================+
    //   |                         |
    //   | Subchunk casacore::Data Accessors |
    //   |                         |
    //   +=========================+

    // Return antenna1

    virtual void antenna1 (casacore::Vector<casacore::Int> & ant1) const;

    // Return antenna2

    virtual void antenna2 (casacore::Vector<casacore::Int> & ant2) const;

    // Return the correlation type (returns casacore::Stokes enums)

    virtual void corrType (casacore::Vector<casacore::Int> & corrTypes) const;

    // Return current DataDescription Id

    virtual casacore::Int dataDescriptionId () const;

    // Return actual time interval

    virtual void  exposure (casacore::Vector<double> & expo) const;

    // Return feed1

    virtual void feed1 (casacore::Vector<casacore::Int> & fd1) const;

    // Return feed2

    virtual void feed2 (casacore::Vector<casacore::Int> & fd2) const;

    // Return the current FieldId

    virtual void fieldIds (casacore::Vector<casacore::Int>&) const;


    // Return the current ArrayId

    virtual void arrayIds (casacore::Vector<casacore::Int>&) const;

    // Return the current Field Name

    virtual casacore::String fieldName () const;

    // Return flag for each polarization, channel and row

    virtual void flag (casacore::Cube<casacore::Bool> & flags) const;

    // Return flag for each channel & row

    virtual void flag (casacore::Matrix<casacore::Bool> & flags) const;

    // Determine whether FLAG_CATEGORY is valid.

    virtual casacore::Bool flagCategoryExists () const;

    // Return flags for each polarization, channel, category, and row.

    virtual void flagCategory (casacore::Array<casacore::Bool> & flagCategories) const;

    // Return row flag

    virtual void flagRow (casacore::Vector<casacore::Bool> & rowflags) const;

    // Return the OBSERVATION_IDs

    virtual void observationId (casacore::Vector<casacore::Int> & obsids) const;

    // Return current Polarization Id

    virtual casacore::Int polarizationId () const;

    // Return the PROCESSOR_IDs

    virtual void processorId (casacore::Vector<casacore::Int> & procids) const;

    // Return scan number

    virtual void scan (casacore::Vector<casacore::Int> & scans) const;

    // Return the current Source Name

    virtual casacore::String sourceName () const;

    // Return the STATE_IDs

    virtual void stateId (casacore::Vector<casacore::Int> & stateids) const;


    // Return feed configuration matrix for specified antenna

    virtual void jonesC (casacore::Vector<casacore::SquareMatrix<casacore::Complex, 2> > & cjones) const;

    // Return frame for polarization (returns PolFrame enum)

    virtual casacore::Int polFrame () const;

    // Return sigma

    virtual void sigma (casacore::Matrix<casacore::Float> & sigmat) const;

    // Return current SpectralWindow

    virtual casacore::Int spectralWindow () const;

    virtual void spectralWindows (casacore::Vector<casacore::Int> & spws) const;

    // Return MJD midpoint of interval.

    virtual void time (casacore::Vector<double> & t) const;

    // Return MJD centroid of interval.

    virtual void timeCentroid (casacore::Vector<double> & t) const;

    // Return nominal time interval

    virtual void timeInterval (casacore::Vector<double> & ti) const;

    // Return u,v and w (in meters)

    virtual void uvw (casacore::Matrix<double> & uvwmat) const;

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

    // Return weight

    virtual void weight (casacore::Matrix<casacore::Float> & wtmat) const;

    // Determine whether WEIGHT_SPECTRUM exists.

    virtual casacore::Bool weightSpectrumExists () const;
    virtual casacore::Bool sigmaSpectrumExists () const;

    // Return weightspectrum (a weight for each channel)

    virtual void weightSpectrum (casacore::Cube<casacore::Float> & wtsp) const;
    virtual void sigmaSpectrum (casacore::Cube<casacore::Float> & wtsp) const;

    // Return the number of sub-intervals in the current chunk

    //   +------------------------+
    //   |                        |
    //   | Angular casacore::Data Providers |
    //   |                        |
    //   +------------------------+

    // true if all elements of the cube returned by getBeamOffsets are zero

    virtual casacore::Bool allBeamOffsetsZero () const;

    virtual std::pair<bool, casacore::MDirection> getPointingAngle (int antenna, double time) const;

    // Return the antenna AZ/EL casacore::Vector (nant)

    virtual casacore::MDirection azel0 (double time) const;
//    static void azel0Calculate (double time, MSDerivedValues & msd,
//                                MDirection & azel0, const MEpoch & mEpoch0);

    virtual const casacore::Vector<casacore::MDirection> & azel (double time) const;
//    static void azelCalculate (double time, MSDerivedValues & msd, Vector<MDirection> & azel,
//                               Int nAnt, const MEpoch & mEpoch0);

    // Return feed parallactic angles casacore::Vector (nant) (1 feed/ant)

    virtual const casacore::Vector<casacore::Float> & feed_pa (double time) const;
//    static Vector<Float> feed_paCalculate (double time, MSDerivedValues & msd,
//                                           Int nAntennas, const MEpoch & mEpoch0,
//                                           const Vector<Float> & receptor0Angle);

    // Return a cube containing pairs of coordinate offsets for each
    // receptor of each feed (values are in radians, coordinate system is fixed
    // with antenna and is the same one as used to define the BEAM_OFFSET
    // parameter in the feed table). The cube axes are receptor, antenna, feed.

    virtual const casacore::Cube<casacore::RigidVector<double, 2> > & getBeamOffsets () const;

    // Return the hour angle for the specified time

    virtual double hourang (double time) const;
    static double hourangCalculate (double time, casacore::MSDerivedValues & msd, const casacore::MEpoch & mEpoch0);

    // Return nominal parallactic angle at specified time
    // (does not include feed position angle offset--see feed_pa)
    // A global value for all antennas (e.g., small array)

    virtual const casacore::Float & parang0 (double time) const;
    static casacore::Float parang0Calculate (double time, casacore::MSDerivedValues & msd, const casacore::MEpoch & epoch0);

    // Per antenna:

    virtual const casacore::Vector<casacore::Float> & parang (double time) const;
    static casacore::Vector<casacore::Float> parangCalculate (double time, casacore::MSDerivedValues & msd,
                                          int nAntennas, const casacore::MEpoch mEpoch0);

    // Return the current phase center as an MDirection

    virtual const casacore::MDirection & phaseCenter () const;

    // Return receptor angles for all antennae and feeds
    // First axis of the cube is a receptor number,
    // 2nd is antennaId, 3rd is feedId
    // Note: the method is intended to provide an access to MSIter::receptorAngles
    // for VisBuffer in the multi-feed case. It may be worth to change the
    // interface of feed_pa to return the information for all feeds.

    virtual const casacore::Cube<double> & receptorAngles () const;

    //   +=========================+
    //   |                         |
    //   | Chunk and casacore::MS Level casacore::Data |
    //   |                         |
    //   +=========================+

    // return a string mount identifier for each antenna

    virtual const casacore::Vector<casacore::String> & antennaMounts () const;

    virtual casacore::MEpoch getEpoch () const;

    // Return imaging weight (a weight for each channel)
    // virtual casacore::Matrix<casacore::Float> & imagingWeight (casacore::Matrix<casacore::Float> & wt) const;

    virtual const VisImagingWeight & getImagingWeightGenerator () const;

    virtual casacore::MFrequency::Types getObservatoryFrequencyType () const; //???
    virtual casacore::MPosition getObservatoryPosition () const;
    virtual casacore::Vector<casacore::Float> getReceptor0Angle ();

    virtual casacore::Int getReportingFrameOfReference () const;
    virtual void setReportingFrameOfReference (casacore::Int frame);

    virtual casacore::Vector<casacore::Int> getChannels (double time, casacore::Int frameOfReference,
                                     casacore::Int spectralWindowId, casacore::Int msId) const;
    virtual casacore::Vector<casacore::Int> getCorrelations () const;
    virtual casacore::Vector<casacore::Stokes::StokesTypes> getCorrelationTypesDefined () const;
    virtual casacore::Vector<casacore::Stokes::StokesTypes> getCorrelationTypesSelected () const;

    virtual casacore::Vector<double> getFrequencies (double time, casacore::Int frameOfReference,
                                           casacore::Int spectralWindowId, casacore::Int msId) const;
    virtual void dataDescriptionIds(casacore::Vector<casacore::Int> &) const;

    //reference to actual ms in interator

    virtual casacore::Int msId () const; // zero-based index of current casacore::MS in set of MSs
    virtual const casacore::MeasurementSet & ms () const;
    virtual casacore::Int getNMs () const;

    // Name of nominal MS (will typically be beneath several layers)
    virtual casacore::String msName () const;

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

    // Access the current casacore::ROMSColumns object in MSIter

    virtual const vi::SubtableColumns & subtableColumns () const;

    // get back the selected spectral windows and spectral channels for
    // current ms

    virtual const SpectralWindowChannels & getSpectralWindowChannels (casacore::Int msId, casacore::Int spectralWindowId) const;

    //assign a VisImagingWeight object to this iterator

    virtual void useImagingWeight (const VisImagingWeight & imWgt);

    // Return number of antennasm spws, polids, ddids

    virtual casacore::Int nAntennas () const;
    virtual casacore::Int nDataDescriptionIds () const;
    virtual casacore::Int nPolarizationIds () const;
    virtual casacore::Int nRowsInChunk () const; // number rows in current chunk
    virtual casacore::Int nRowsViWillSweep () const; // number of rows in all selected ms's
    virtual casacore::Int nSpectralWindows () const;

    //   +-------------------+
    //   |                   |
    //   | Writeback Methods |
    //   |                   |
    //   +-------------------+

    // This method writes back any changed (dirty) components of the provided
    // VisBuffer and is the preferred method for writing data out.

    virtual void writeBackChanges (VisBuffer2 * vb);

    // Write/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
    virtual void writeFlag (const casacore::Matrix<casacore::Bool> & flag);

    // Write/modify the flags in the data.
    // This writes the flags as found in the casacore::MS, casacore::Cube (npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow ()).
    virtual void writeFlag (const casacore::Cube<casacore::Bool> & flag);

    // Write/modify the flag row column; dimension casacore::Vector (nrow)
    virtual void writeFlagRow (const casacore::Vector<casacore::Bool> & rowflags);

    virtual void writeFlagCategory(const casacore::Array<casacore::Bool>& fc);

    // Write/modify the visibilities.
    // This is possibly only for a 'reference' casacore::MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the casacore::MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
//    virtual void writeVisCorrected (const casacore::Matrix<CStokesVector> & visibilityStokes);
//    virtual void writeVisModel (const casacore::Matrix<CStokesVector> & visibilityStokes);
//    virtual void writeVisObserved (const casacore::Matrix<CStokesVector> & visibilityStokes);

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
    virtual void writeSigmaSpectrum (const casacore::Cube<casacore::Float> & wtsp);

    // Write/modify the Sigma
    virtual void writeSigma (const casacore::Matrix<casacore::Float> & sig);

    // Write/modify the ncorr x nrow SigmaMat.
    //virtual void writeSigmaMat (const casacore::Matrix<casacore::Float> & sigmat);

    // Write the information needed to generate on-the-fly model visibilities.

    virtual void writeModel(const casacore::RecordInterface& rec, casacore::Bool iscomponentlist=true,
                            casacore::Bool incremental=false);

    virtual void setWeightScaling (casacore::CountedPtr <WeightScaling> weightscaling);
    virtual casacore::Bool hasWeightScaling () const;
    virtual casacore::CountedPtr<WeightScaling> getWeightScaling () const;

    // Library of static transformations available for all TVIs
    static void calculateFlagRowFromFlagCube (const casacore::Cube<casacore::Bool> &flagCube, casacore::Vector<casacore::Bool> &flagRow);

protected:

    //const VisBuffer2 * getVisBufferConst () const /*__attribute__((deprecated))*/ { throw casacore::AipsError ("Not implemented"); }
    const VisBuffer2 * getVisBufferConst () const {return vb_p;}
    TransformingVi2 (ViImplementation2 * inputVi);

    void configureNewSubchunk ();
    void configureNewSubchunk (casacore::Int msId, const casacore::String & msName, casacore::Bool isNewMs,
                               casacore::Bool isNewArrayId, casacore::Bool isNewFieldId,
                               casacore::Bool isNewSpectralWindow, const Subchunk & subchunk,
                               casacore::Int nRows, casacore::Int nChannels, casacore::Int nCorrelations,
                               const casacore::Vector<casacore::Int> & correlations,
                               const casacore::Vector<casacore::Stokes::StokesTypes> & correlationsDefined,
                               const casacore::Vector<casacore::Stokes::StokesTypes> & correlationsSelected,
                               casacore::CountedPtr<WeightScaling> weightScaling);

    VisibilityIterator2 * getVi () const;
    ViImplementation2 * getVii () const;
    void setVisBuffer (VisBuffer2 * vb);
    virtual VisBuffer2 * getVisBuffer () const {return vb_p;}

    // jagonzal (to be reviewed by jjacobs): I need to set inputVii_p to NULL from
    // MSTransformIterator destructor because MSTransformIteratorFactory is borrowing
    // the ViImplementation2 from the input iterator, which is deleted in the first
    // steps of the MSTransformIterator destructor.
    ViImplementation2 * inputVii_p;

private:

    VisBuffer2 * vb_p; // [own]
    casacore::CountedPtr<WeightScaling> weightScaling_p;
};

} // end namespace vi

} //# NAMESPACE CASA - END

#endif // ! defined (MSVIS_TransformingVi2_H_121115_0950)



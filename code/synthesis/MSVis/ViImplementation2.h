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

#if ! defined (MSVIS_ViImplementation2_H_121115_0950)
#define MSVIS_ViImplementation2_H_121115_0950

#include <casa/aips.h>
#include <casa/BasicSL.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <measures/Measures/MFrequency.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

template <typename T> class Array;
template <typename T> class Block;
//class CStokesVector;
template <typename T> class Cube;
template <typename T> class Matrix;
class MDirection;
class MeasurementSet;
class MEpoch;
class MPosition;
class MSDerivedValues;
class RecordInterface;
template <typename T, Int n> class RigidVector;
class Slice;
class String;
template <typename T, Int n> class SquareMatrix;
template <typename T> class Vector;
class VisImagingWeight;

namespace vi {


//# forward decl

class ChannelSelector;
class ChannelSelectorCache;
class FrequencySelections;
class SortColumns;
class SpectralWindowChannels;
class SpectralWindowChannelsCache;
class Subchunk;
class SubtableColumns;
class VisBuffer2;
class VisibilityIterator2;
class WeightScaling;

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

class ViImplementation2 : private boost::noncopyable {

    friend class VisibilityIterator2;

public:

    ViImplementation2 () {}

    // Destructor

    virtual ~ViImplementation2 () {}

    //   +==================================+
    //   |                                  |
    //   | Iteration Control and Monitoring |
    //   |                                  |
    //   +==================================+


    // Methods to control and monitor subchunk iteration

    virtual void origin () = 0;
    virtual Bool more () const = 0;
    virtual void next () = 0;
    virtual Subchunk getSubchunkId () const = 0;

    // Methods to control chunk iterator

    virtual void originChunks (Bool forceRewind = False) = 0;
    virtual Bool moreChunks () const = 0;
    virtual void nextChunk () = 0;

    virtual Bool isWritable () const = 0;

    // Return the time interval (in seconds) used for iteration.
    // This is not the same as the INTERVAL column.  Setting the
    // the interval requires calling origin chunks before performing
    // further iterator.

    virtual Double getInterval() const = 0;
    virtual void setInterval (Double timeInterval) = 0;

    // Select the channels to be returned.  Requires calling originChunks before
    // performing additional iteration.

    virtual void setFrequencySelections (const FrequencySelections & selection) = 0;

    // Set the 'blocking' size for returning data.
    // With the default (0) only a single integration is returned at a time, this
    // is what is currently required for the calibration software. With blocking
    // set, up to nRows can be returned in one go. The chunk
    // size determines the actual maximum.

    virtual void setRowBlocking (Int nRows = 0) = 0;

    virtual Bool existsColumn (VisBufferComponent2 id) const = 0;

    virtual const SortColumns & getSortColumns() const = 0;

    virtual Bool isNewArrayId () const = 0;
    virtual Bool isNewFieldId () const = 0;
    virtual Bool isNewMs () const = 0;
    virtual Bool isNewSpectralWindow () const = 0;

    // Return the number of rows in the current iteration

    virtual Int nRows () const = 0;

    // Return the row ids as from the original root table. This is useful
    // to find correspondance between a given row in this iteration to the
    // original ms row

    virtual void getRowIds (Vector<uInt> & rowids) const = 0;

    virtual VisBuffer2 * getVisBuffer () = 0;


    //   +=========================+
    //   |                         |
    //   | Subchunk Data Accessors |
    //   |                         |
    //   +=========================+

    // Return antenna1

    virtual void antenna1 (Vector<Int> & ant1) const = 0;

    // Return antenna2

    virtual void antenna2 (Vector<Int> & ant2) const = 0;

    // Return the correlation type (returns Stokes enums)

    virtual void corrType (Vector<Int> & corrTypes) const = 0;

    // Return current DataDescription Id

    virtual Int dataDescriptionId () const = 0;

    virtual void dataDescriptionIds (Vector<Int> & ddis) const = 0;

    // Return actual time interval

    virtual void  exposure (Vector<Double> & expo) const = 0;

    // Return feed1

    virtual void feed1 (Vector<Int> & fd1) const = 0;

    // Return feed2

    virtual void feed2 (Vector<Int> & fd2) const = 0;

    // Return the current FieldId

    virtual void fieldIds (Vector<Int>&) const = 0;

    // Return the current ArrayId

    virtual void arrayIds (Vector<Int>&) const = 0;

    // Return the current Field Name

    virtual String fieldName () const = 0;

    // Return flag for each polarization, channel and row

    virtual void flag (Cube<Bool> & flags) const = 0;

    // Return flag for each channel & row

    virtual void flag (Matrix<Bool> & flags) const = 0;

    // Determine whether FLAG_CATEGORY is valid.

    virtual Bool flagCategoryExists () const = 0;

    // Return flags for each polarization, channel, category, and row.

    virtual void flagCategory (Array<Bool> & flagCategories) const = 0;

    // Return row flag

    virtual void flagRow (Vector<Bool> & rowflags) const = 0;

    // Return the OBSERVATION_IDs

    virtual void observationId (Vector<Int> & obsids) const = 0;

    // Return current Polarization Id

    virtual Int polarizationId () const = 0;

    // Return the PROCESSOR_IDs

    virtual void processorId (Vector<Int> & procids) const = 0;

    // Return scan number

    virtual void scan (Vector<Int> & scans) const = 0;

    // Return the current Source Name

    virtual String sourceName () const = 0;

    // Return the STATE_IDs

    virtual void stateId (Vector<Int> & stateids) const = 0;


    // Return feed configuration matrix for specified antenna

    virtual void jonesC (Vector<SquareMatrix<Complex, 2> > & cjones) const = 0;

    // Return frame for polarization (returns PolFrame enum)

    virtual Int polFrame () const = 0;

    // Return sigma

    virtual void sigma (Matrix<Float> & sigmat) const = 0;

    // Return current SpectralWindow

    virtual Int spectralWindow () const = 0;

    virtual void spectralWindows (Vector<Int> & spws) const = 0;

    // Return MJD midpoint of interval.

    virtual void time (Vector<Double> & t) const = 0;

    // Return MJD centroid of interval.

    virtual void timeCentroid (Vector<Double> & t) const = 0;

    // Return nominal time interval

    virtual void timeInterval (Vector<Double> & ti) const = 0;

    // Return u,v and w (in meters)

    virtual void uvw (Matrix<Double> & uvwmat) const = 0;

    // Return the visibilities as found in the MS, Cube (npol,nchan,nrow).

    virtual void visibilityCorrected (Cube<Complex> & vis) const = 0;
    virtual void visibilityModel (Cube<Complex> & vis) const = 0;
    virtual void visibilityObserved (Cube<Complex> & vis) const = 0;

    // Return FLOAT_DATA as a Cube (npol, nchan, nrow) if found in the MS.

    virtual void floatData (Cube<Float> & fcube) const = 0;

    // Return the visibility 4-vector of polarizations for each channel.
    // If the MS doesn't contain all polarizations, it is assumed it
    // contains one or two parallel hand polarizations.

//    virtual void visibilityCorrected (Matrix<CStokesVector> & vis) const = 0;
//    virtual void visibilityModel (Matrix<CStokesVector> & vis) const = 0;
//    virtual void visibilityObserved (Matrix<CStokesVector> & vis) const = 0;

    // Return the shape of the visibility Cube

    virtual IPosition visibilityShape () const = 0;

    // Return weight

    virtual void weight (Matrix<Float> & wtmat) const = 0;

    // Determine whether WEIGHT_SPECTRUM exists.

    virtual Bool weightSpectrumExists () const = 0;
    virtual Bool weightSpectrumCorrectedExists () const = 0;

    // Return weightspectrum (a weight for each channel)

    virtual void weightSpectrum (Cube<Float> & wtsp) const = 0;
    virtual void weightSpectrumCorrected (Cube<Float> & wtsp) const = 0;


    virtual void setWeightScaling (CountedPtr<WeightScaling> weightscaling) = 0;
    virtual Bool hasWeightScaling () const = 0;
    virtual CountedPtr<WeightScaling> getWeightScaling () const = 0;

    // Return the number of sub-intervals in the current chunk

    //   +------------------------+
    //   |                        |
    //   | Angular Data Providers |
    //   |                        |
    //   +------------------------+

    // True if all elements of the cube returned by getBeamOffsets are zero

    virtual Bool allBeamOffsetsZero () const = 0;

    // Return the antenna AZ/EL Vector (nant)

    virtual MDirection azel0 (Double time) const = 0;
    static void azel0Calculate (Double time, MSDerivedValues & msd,
                                MDirection & azel0, const MEpoch & mEpoch0);

    virtual const Vector<MDirection> & azel (Double time) const = 0;
    static void azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
                               Int nAnt, const MEpoch & mEpoch0);

    // Return feed parallactic angles Vector (nant) (1 feed/ant)

    virtual const Vector<Float> & feed_pa (Double time) const = 0;
    static Vector<Float> feed_paCalculate (Double time, MSDerivedValues & msd,
                                           Int nAntennas, const MEpoch & mEpoch0,
                                           const Vector<Float> & receptor0Angle);

    // Return a cube containing pairs of coordinate offsets for each
    // receptor of each feed (values are in radians, coordinate system is fixed
    // with antenna and is the same one as used to define the BEAM_OFFSET
    // parameter in the feed table). The cube axes are receptor, antenna, feed.

    virtual const Cube<RigidVector<Double, 2> > & getBeamOffsets () const = 0;

    // Return the hour angle for the specified time

    virtual Double hourang (Double time) const = 0;
    static Double hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0);

    // Return nominal parallactic angle at specified time
    // (does not include feed position angle offset--see feed_pa)
    // A global value for all antennas (e.g., small array)

    virtual const Float & parang0 (Double time) const = 0;
    static Float parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & epoch0);

    // Per antenna:

    virtual const Vector<Float> & parang (Double time) const = 0;
    static Vector<Float> parangCalculate (Double time, MSDerivedValues & msd,
                                          int nAntennas, const MEpoch mEpoch0);

    // Return the current phase center as an MDirection

    virtual const MDirection & phaseCenter () const = 0;

    // Return receptor angles for all antennae and feeds
    // First axis of the cube is a receptor number,
    // 2nd is antennaId, 3rd is feedId
    // Note: the method is intended to provide an access to MSIter::receptorAngles
    // for VisBuffer in the multi-feed case. It may be worth to change the
    // interface of feed_pa to return the information for all feeds.

    virtual const Cube<Double> & receptorAngles () const = 0;

    //   +=========================+
    //   |                         |
    //   | Chunk and MS Level Data |
    //   |                         |
    //   +=========================+

    // return a string mount identifier for each antenna

    virtual const Vector<String> & antennaMounts () const = 0;

    virtual MEpoch getEpoch () const = 0;

    // Return imaging weight (a weight for each channel)
    //virtual Matrix<Float> & imagingWeight (Matrix<Float> & wt) const = 0;

    virtual const VisImagingWeight & getImagingWeightGenerator () const = 0;

    virtual MFrequency::Types getObservatoryFrequencyType () const = 0; //???
    virtual MPosition getObservatoryPosition () const = 0;
    virtual Vector<Float> getReceptor0Angle () = 0;

    virtual Int getReportingFrameOfReference () const = 0;
    virtual void setReportingFrameOfReference (Int frame) = 0;

    virtual Vector<Int> getCorrelations () const = 0;
    virtual Vector<Int> getChannels (Double time, Int frameOfReference) const = 0;
    virtual Vector<Double> getFrequencies (Double time, Int frameOfReference) const = 0;


    //reference to actual ms in interator

    virtual Int msId () const = 0; // zero-based index of current MS in set of MSs
    virtual const MeasurementSet & ms () const = 0;
    virtual Int getNMs () const = 0;

    // Call to use the slurp i/o method for all scalar columns. This
    // will set the BucketCache cache size to the full column length
    // and cause the full column to be cached in memory, if
    // any value of the column is used. In case of out-of-memory,
    // it will automatically fall-back on the smaller cache size.
    // Slurping the column is to be considered as a work-around for the
    // Table i/o code, which uses BucketCache and performs extremely bad
    // for random access. Slurping is useful when iterating non-sequentially
    // an MS or parts of an MS, it is not tested with multiple MSs.

    virtual void slurp () const = 0;

    // Access the current ROMSColumns object in MSIter

    virtual const vi::SubtableColumns & subtableColumns () const = 0;

    // get back the selected spectral windows and spectral channels for
    // current ms

    virtual const SpectralWindowChannels & getSpectralWindowChannels (Int msId, Int spectralWindowId) const = 0;

    //assign a VisImagingWeight object to this iterator

    virtual void useImagingWeight (const VisImagingWeight & imWgt) = 0;

    // Return number of antennasm spws, polids, ddids

    virtual Int nAntennas () const = 0;
    virtual Int nDataDescriptionIds () const = 0;
    virtual Int nPolarizationIds () const = 0;
    virtual Int nRowsInChunk () const = 0; // number rows in current chunk
    virtual Int nRowsViWillSweep () const = 0; // number of rows in all selected ms's
    virtual Int nSpectralWindows () const = 0;

    //   +-------------------+
    //   |                   |
    //   | Writeback Methods |
    //   |                   |
    //   +-------------------+

    // This method writes back any changed (dirty) components of the provided
    // VisBuffer and is the preferred method for writing data out.

    virtual void writeBackChanges (VisBuffer2 * vb) = 0;

    // Write/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
//    virtual void writeFlag (const Matrix<Bool> & flag) = 0;

    // Write/modify the flags in the data.
    // This writes the flags as found in the MS, Cube (npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow ()).
    virtual void writeFlag (const Cube<Bool> & flag) = 0;

    // Write/modify the flag row column = 0; dimension Vector (nrow)
    virtual void writeFlagRow (const Vector<Bool> & rowflags) = 0;

    virtual void writeFlagCategory(const Array<Bool>& fc) = 0;

    // Write/modify the visibilities.
    // This is possibly only for a 'reference' MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
//    virtual void writeVisCorrected (const Matrix<CStokesVector> & visibilityStokes) = 0;
//    virtual void writeVisModel (const Matrix<CStokesVector> & visibilityStokes) = 0;
//    virtual void writeVisObserved (const Matrix<CStokesVector> & visibilityStokes) = 0;

    // Write/modify the visibilities
    // This writes the data as found in the MS, Cube (npol,nchan,nrow).
    virtual void writeVisCorrected (const Cube<Complex> & vis) = 0;
    virtual void writeVisModel (const Cube<Complex> & vis) = 0;
    virtual void writeVisObserved (const Cube<Complex> & vis) = 0;

    // Write/modify the weights
    virtual void writeWeight (const Matrix<Float> & wt) = 0;

    // Write/modify the weightMat
    //virtual void writeWeightMat (const Matrix<Float> & wtmat) = 0;

    // Write/modify the weightSpectrum
    virtual void writeWeightSpectrum (const Cube<Float> & wtsp) = 0;

    // Write/modify the weightSpectrum
    virtual void writeWeightSpectrumCorrected (const Cube<Float> & wtsp) = 0;

    // Write/modify the Sigma
    virtual void writeSigma (const Matrix<Float> & sig) = 0;

    // Write/modify the ncorr x nrow SigmaMat.
    //virtual void writeSigmaMat (const Matrix<Float> & sigmat) = 0;

    // Write the information needed to generate on-the-fly model visibilities.

    virtual void writeModel(const RecordInterface& rec, Bool iscomponentlist=True,
                            Bool incremental=False) = 0;


protected:

    static void doWeightScaling (Bool hasWeightScaling,
                                 WeightScaling * scaling,
                                 const Array<Float>& unscaled,
                                 Array<Float>& scaled);

};

} // end namespace vi

} //# NAMESPACE CASA - END

#endif // ! defined (MSVIS_ViImplementation2_H_121115_0950)



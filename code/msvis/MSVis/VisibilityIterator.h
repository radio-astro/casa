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

#ifndef MSVIS_VISIBILITYITERATOR_H
#define MSVIS_VISIBILITYITERATOR_H

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Stack.h>
#include <casa/Containers/RecordInterface.h>
#include <casa/Quanta/MVDoppler.h>
#include <casa/aips.h>
#include <measures/Measures/MCDoppler.h>
#include <measures/Measures/MDoppler.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <ms/MeasurementSets/MSIter.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisBufferComponents.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>

#include <map>
#include <set>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward decl

namespace asyncio {

class VLAT;

// <summary>
// The PrefetchColumns class is used to specify a set of columns that
// can be prefetched when the (RO)VisibilityIterator is using asynchronous
// I/O.
// </summary>
//
// <synopsis>
// When creating an ROVisibilityIterator or VisibilityIterator that can
// potentially operate using asynchronous I/O it is necessary to specify
// which columns of the main table are to be prefetched by the lookahead
// thread.  This class provides the way to specify those columns.  The
// class is an STL set object of enum values defined in VisBufferComponents.h.
// These roughly correspond to the different components that can be access
// via a VisBuffer.
//
// </synopsis>
//
// <example>
// This is a simple example showing the construction of a PrefetchColumns
// and its use in a VisibilityIterator constructor.  For more complex cases
// (e.g., where the columns to be prefetched depend on other factors) the
// class provides some additional utility methods.
//
// Usually the file that creates the VI will include VisBuffer which will
// then include VisBufferComponents.h; if not then the user will also need
// to add "#include <msvis/MSVis/VisBufferComponents.h>" to their file.
//
// <code>
//    asyncio::PrefetchColumns prefetchColumns =
//            PrefetchColumns::prefetchColumns (VisBufferComponents::Ant1,
//                                              VisBufferComponents::Ant2,
//                                              VisBufferComponents::ArrayId,
//                                              VisBufferComponents::Direction1,
//                                              VisBufferComponents::Direction2,
//                                              VisBufferComponents::Feed1,
//                                              VisBufferComponents::Feed1_pa,
//                                              VisBufferComponents::Feed2,
//                                              VisBufferComponents::Feed2_pa,
//                                              VisBufferComponents::FieldId,
//                                              VisBufferComponents::FlagCube,
//                                              VisBufferComponents::Flag,
//                                              VisBufferComponents::FlagRow,
//                                              VisBufferComponents::Freq,
//                                              VisBufferComponents::NChannel,
//                                              VisBufferComponents::NCorr,
//                                              VisBufferComponents::NRow,
//                                              VisBufferComponents::ObservedCube,
//                                              VisBufferComponents::PhaseCenter,
//                                              VisBufferComponents::PolFrame,
//                                              VisBufferComponents::SpW,
//                                              VisBufferComponents::Time,
//                                              VisBufferComponents::Uvw,
//                                              VisBufferComponents::UvwMat,
//                                              VisBufferComponents::Weight,
//                                              -1);
//
//     wvi_p = new VisibilityIterator (& prefetchColumns, * wvi_p);
//
//
// </code>
//
// </example
class PrefetchColumns : public std::set<VisBufferComponents::EnumType>{

public:

    PrefetchColumns operator+ (const PrefetchColumns & other);

    static String columnName (Int id);
    static PrefetchColumns prefetchColumnsAll ();
    static PrefetchColumns prefetchAllColumnsExcept (Int firstColumn, ...);
    static PrefetchColumns prefetchColumns (Int firstColumn, ...);
};

} // end namespace asyncio

class VisBuffer;
class VisibilityIteratorReadImpl;
class VisibilityIteratorWriteImpl;

class SubChunkPair : public std::pair<Int, Int>{

public:

    // First component is Chunk and second is subchunk

    SubChunkPair () { resetToOrigin ();}
    SubChunkPair (Int a , Int b) : pair<Int,Int> (a,b) {}

    Bool operator== (const SubChunkPair & other){
        return first == other.first && second == other.second;
    }

    Bool operator< (const SubChunkPair & other){
        return first < other.first ||
               (first == other.first && second < other.second);
    }

    Bool atOrigin () const { return * this == SubChunkPair ();}
    Int chunk () const { return first;}
    void incrementSubChunk () { second ++;}
    void incrementChunk () { first ++; second = 0; }

    void resetSubChunk () { second = 0;} // position to start of chunk

    void resetToOrigin () { first = 0; second = 0; }
       // Make a subchunk pair that is positioned to the first subchunk of the
       // first chunk (i.e., 0,0)

    Int subchunk () const { return second;}
    String toString () const;

    static SubChunkPair noMoreData ();

private:


};


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
//   <li> <linkto class="PrefetchColumns">PrefetchColumns</linkto>
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
//
// ROVisibilityIterators can be either synchronous or asynchronous, depending
// on the constructor used to create them as well as the current value of
// a CASARC file setting.  A synchronous instance is works the same as
// this class ever worked; an asynchronous instance uses a second thread
// (the Visibility Lookahead Thread or VLAT) to fill the VisBuffers in
// advance of their use by the original thread.
//
// To create an asynchronous instance of ROVI you must use one of the two
// constructors which have a pointer to a PrefetchColumns object as the
// first argument.  This object specifies which table columns should be
// prefetched by the VLAT; accessing columns not specified in the PrefetchColumns
// object will result in an exception containing an error message indicating
// that a the VisBuffer does not contain the requested column.  In addition
// to using the appropriate constructor, the CASARC file setting
// VisibilityIterator.
//
// +-------------------+
// |                   |
// | *** Nota Bene *** |
// |                   |
// +-------------------+
//
// Because of the multithreaded nature of asynchronous I/O, the user
// needs to be a bit more careful in the use of the VI and it's attached VisBuffer.
// Data access operations need to be directed to the VisBuffer.  Additionally
// the user must not attempt to access the data using a separate VI since
// the underlying casacore objects are not threadsafe and bizarre errors
// will likely occur.
//
// CASARC Settings
// ===============
//
// Normal settings
// ---------------
//
// VisibilityIterator.async.enabled - Boolean value that enables or disables
//     async I/O.  The default value is currently False (i.e., disabled).
// VisibilityIterator.async.nBuffers - The number of lookahead buffers.  This
//     defaults to 2.
//
//
// Debug settings
// --------------
//
// VisibilityIterator.async.doStats: true
// VisibilityIterator.async.debug.logFile: stderr
// VisibilityIterator.async.debug.logLevel: 1
//

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
class ROVisibilityIterator
{
    friend class AsyncEnabler;
    friend class VisibilityIteratorReadImpl;
    friend class ViReadImplAsync;
    friend class asyncio::VLAT; // allow VI lookahead thread class to access protected functions
                                // VLAT should not access private parts, especially variables
public:

  class Factory {

  public:

      virtual ~Factory () {}

      virtual VisibilityIteratorReadImpl *
      operator() (const asyncio::PrefetchColumns * /*prefetchColumns*/,
                  const Block<MeasurementSet>& /*mss*/,
                  const Block<Int>& /*sortColumns*/,
                  const Bool /*addDefaultSortCols*/,
                  Double /*timeInterval*/) const
      {
          return NULL;
      }
  };

  //# the following is a copy of the enum in MSIter
  //# can't think of a way to get one that known to the outside world from here
  enum PolFrame {
    // Circular polarization
    Circular=0,
    // Linear polarization
    Linear=1
  };

  typedef enum DataColumn {
      Observed=0,  // Observed data
      Model,       // Model data
      Corrected    // Corrected data
  } DataColumn;

  class AsyncEnabler {
  public:
      AsyncEnabler (ROVisibilityIterator &);
      AsyncEnabler (ROVisibilityIterator *);
      ~AsyncEnabler ();
      void release ();
  private:
      Bool oldEnabledState_p;
      ROVisibilityIterator * roVisibilityIterator_p;
  };

  // Default constructor - useful only to assign another iterator later
  ROVisibilityIterator();
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
  // and FIELD_ID).  Using selectChannel(), a number of groups of channels can
  // be requested.  At present the channel group iteration will always occur
  // before the interval iteration.
  ROVisibilityIterator(const MeasurementSet& ms, 
		       const Block<Int>& sortColumns,
		       Double timeInterval=0,
		       const Factory & factory = Factory());
  // Same as above, but with the option of using the raw order of ms
  // (addDefaultSortCols=false).
  ROVisibilityIterator(const MeasurementSet& ms, 
		       const Block<Int>& sortColumns,
		       const Bool addDefaultSortCols,
		       Double timeInterval=0);
 
  // Same as previous constructors, but with multiple MSs to iterate over.
  ROVisibilityIterator(const Block<MeasurementSet>& mss,
		       const Block<Int>& sortColumns, 
		       Double timeInterval=0);

  ROVisibilityIterator(const Block<MeasurementSet>& mss,
                       const Block<Int>& sortColumns,
                       const Bool addDefaultSortCols,
                       Double timeInterval=0);

  ROVisibilityIterator(const asyncio::PrefetchColumns * prefetchColumns,
                       const MeasurementSet& ms,
		       const Block<Int>& sortColumns,
		       const Bool addDefaultSortCols = True,
		       Double timeInterval = 0);

  ROVisibilityIterator (const asyncio::PrefetchColumns * prefetchColumns,
                        const Block<MeasurementSet>& mss,
                        const Block<Int>& sortColumns,
                        const Bool addDefaultSortCols = True,
                        Double timeInterval = 0);

  // Copy construct. This calls the assigment operator.
  ROVisibilityIterator(const ROVisibilityIterator & other);
  ROVisibilityIterator(const asyncio::PrefetchColumns * prefetchColumns, const ROVisibilityIterator & other);
  // Assigment. Any attached VisBuffers are lost in the assign.
  ROVisibilityIterator & operator=(const ROVisibilityIterator &other);
  // Destructor
  virtual ~ROVisibilityIterator();
  
//  static ROVisibilityIterator * create (const MeasurementSet & ms,
//                                        const Block<Int>& sortColumns,
//                                        const asyncio::PrefetchColumns * prefetchColumns = NULL,
//                                        const Bool addDefaultSortCols = True,
//                                        Double timeInterval=0);
//
//  static ROVisibilityIterator * create (const Block<MeasurementSet>& mss,
//                                        const Block<Int>& sortColumns,
//                                        const asyncio::PrefetchColumns * prefetchColumns = NULL,
//                                        const Bool addDefaultSortCols = True,
//                                        Double timeInterval=0);
//
//  static ROVisibilityIterator * create (const ROVisibilityIterator & rovi,
//                                        const asyncio::PrefetchColumns * prefetchColumns,
//                                        Int nReadAheadBuffers = 2);

  // Members
  
  Bool isAsynchronous () const;
  static Bool isAsynchronousIoEnabled();

  Bool isAsyncEnabled () const;
  Bool isWritable () const;
  // Reset iterator to origin/start of data (of current chunk)
  void origin();
  // Reset iterator to true start of data (first chunk)
  void originChunks();
 
  // Return the time interval (in seconds) used for iteration.
  // This is not the same as the INTERVAL column.
  Double getInterval() const;
  // Set or reset the time interval (in seconds) to use for iteration.
  // You should call originChunks() to reset the iteration after 
  // calling this.
  void setInterval(Double timeInterval);

  // Set the 'blocking' size for returning data.
  // With the default (0) only a single integration is returned at a time, this
  // is what is currently required for the calibration software. With blocking
  // set, up to nRows can be returned in one go. The chunk 
  // size determines the actual maximum.
  void setRowBlocking(Int nRows=0);
  // Return False if no more data (in current chunk)
  Bool more() const;
  SubChunkPair getSubchunkId () const;
  // Advance iterator through data
  ROVisibilityIterator & operator++(int);
  ROVisibilityIterator & operator++();
  // Return False if no more 'Chunks' of data left
  Bool moreChunks() const;
  // Check if ms has change since last iteration
  Bool newMS() const;
  Int msId() const;
  VisBuffer * getVisBuffer ();
  //reference to actual ms in interator 
  const MeasurementSet& ms() const;
 // Advance to the next Chunk of data
  ROVisibilityIterator& nextChunk();
  // Return antenna1
  Vector<Int>& antenna1(Vector<Int>& ant1) const;
  // Return antenna2
  Vector<Int>& antenna2(Vector<Int>& ant2) const;
  // Return feed1
  Vector<Int>& feed1(Vector<Int>& fd1) const;
  // Return feed2
  Vector<Int>& feed2(Vector<Int>& fd2) const;
  // Return channel numbers in selected VisSet spectrum
  // (i.e. disregarding possible selection on the iterator, but
  //  including the selection set when creating the VisSet)
  virtual Vector<Int>& channel(Vector<Int>& chan) const;
  // Return feed configuration matrix for specified antenna
  Vector<SquareMatrix<Complex,2> >& 
  CJones(Vector<SquareMatrix<Complex,2> >& cjones) const;
  // Return receptor angles for all antennae and feeds
  // First axis of the cube is a receptor number,
  // 2nd is antennaId, 3rd is feedId
  // Note: the method is intended to provide an access to MSIter::receptorAngles
  // for VisBuffer in the multi-feed case. It may be worth to change the
  // interface of feed_pa to return the information for all feeds.
  const Cube<Double>& receptorAngles() const;
  // return a string mount identifier for each antenna
  const Vector<String>& antennaMounts() const;
  // Return a cube containing pairs of coordinate offsets for each
  // receptor of each feed (values are in radians, coordinate system is fixed
  // with antenna and is the same one as used to define the BEAM_OFFSET 
  // parameter in the feed table). The cube axes are receptor, antenna, feed.
  const Cube<RigidVector<Double, 2> >& getBeamOffsets() const;
  // True if all elements of the cube returned by getBeamOffsets are zero
  Bool allBeamOffsetsZero() const;
  // Return feed parallactic angles Vector(nant) (1 feed/ant)
  Vector<Float> feed_pa(Double time) const;
  static Vector<Float> feed_paCalculate(Double time, MSDerivedValues & msd,
  									    Int nAntennas, const MEpoch & mEpoch0,
									    const Vector<Float> & receptor0Angle);
  // Return nominal parallactic angle at specified time
  // (does not include feed position angle offset--see feed_pa)
  // A global value for all antennas (e.g., small array)
  const Float& parang0(Double time) const;
  static Float parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & epoch0);
  // Per antenna:
  Vector<Float> parang(Double time) const;
  static Vector<Float> parangCalculate (Double time, MSDerivedValues & msd,
		                                int nAntennas, const MEpoch mEpoch0);
  // Return the antenna AZ/EL Vector(nant) 
  MDirection azel0(Double time) const;
  static void azel0Calculate (Double time, MSDerivedValues & msd,
		                      MDirection & azel0, const MEpoch & mEpoch0);
  Vector<MDirection> azel(Double time) const;
  static void azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
		                     Int nAnt, const MEpoch & mEpoch0);
  // Return the hour angle for the specified time
  Double hourang(Double time) const;
  static Double hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0);
  // Return the current FieldId
  Int fieldId() const;
  // Return the current ArrayId
  Int arrayId() const;
  // Return the current Field Name
  String fieldName() const;
  // Return the current Source Name
  String sourceName() const;
  // Return flag for each polarization, channel and row
  virtual Cube<Bool>& flag(Cube<Bool>& flags) const;
  // Return flag for each channel & row
  Matrix<Bool>& flag(Matrix<Bool>& flags) const;

  // Determine whether FLAG_CATEGORY is valid.
  Bool existsFlagCategory() const;

  // Return flags for each polarization, channel, category, and row.
  Array<Bool>& flagCategory(Array<Bool>& flagCategories) const;
  // Return row flag
  Vector<Bool>& flagRow(Vector<Bool>& rowflags) const;
  // Return scan number
  Vector<Int>& scan(Vector<Int>& scans) const;
  // Return the OBSERVATION_IDs
  Vector<Int>& observationId(Vector<Int>& obsids) const;
  // Return the PROCESSOR_IDs
  Vector<Int>& processorId(Vector<Int>& procids) const;
  // Return the STATE_IDs
  Vector<Int>& stateId(Vector<Int>& stateids) const;
  // Return current frequencies (in Hz, acc. to the MS def'n v.2)
  virtual Vector<Double>& frequency(Vector<Double>& freq) const;
  // Return frequencies  (in Hz, acc. to the MS def'n v.2) in selected velocity frame,
  // returns the same as frequency() if there is no vel selection active.
  Vector<Double>& lsrFrequency(Vector<Double>& freq) const;
  // Return the current phase center as an MDirection
  const MDirection& phaseCenter() const;
  // Return frame for polarization (returns PolFrame enum)
  Int polFrame() const;
  // Return the correlation type (returns Stokes enums)
  virtual Vector<Int>& corrType(Vector<Int>& corrTypes) const;
  // Return sigma
  Vector<Float>& sigma(Vector<Float>& sig) const;
  // Return sigma matrix (pol-dep)
  virtual Matrix<Float>& sigmaMat(Matrix<Float>& sigmat) const;
  // Return current SpectralWindow
  Int spectralWindow() const;
  // Return current Polarization Id
  Int polarizationId() const;
  // Return current DataDescription Id
  Int dataDescriptionId() const;
  // Return MJD midpoint of interval.
  Vector<Double>& time(Vector<Double>& t) const;
  // Return MJD centroid of interval.
  Vector<Double>& timeCentroid(Vector<Double>& t) const;
  // Return nominal time interval
  Vector<Double>& timeInterval(Vector<Double>& ti) const;
  // Return actual time interval
  Vector<Double>& exposure(Vector<Double>& expo) const;
  // Return the visibilities as found in the MS, Cube(npol,nchan,nrow).
  virtual Cube<Complex>& visibility(Cube<Complex>& vis,
				    DataColumn whichOne) const;
  // Return FLOAT_DATA as a Cube(npol, nchan, nrow) if found in the MS.
  Cube<Float>& floatData(Cube<Float>& fcube) const;
  // Return the visibility 4-vector of polarizations for each channel.
  // If the MS doesn't contain all polarizations, it is assumed it
  // contains one or two parallel hand polarizations.
  Matrix<CStokesVector>& visibility(Matrix<CStokesVector>& vis, 
				    DataColumn whichOne) const;
  // Return the shape of the visibility Cube
  IPosition visibilityShape() const;
  // Return u,v and w (in meters)
  virtual Vector<RigidVector<Double,3> >& uvw(Vector<RigidVector<Double,3> >& uvwvec) const;
  Matrix<Double>& uvwMat(Matrix<Double>& uvwmat) const;
  // Return weight
  Vector<Float>& weight(Vector<Float>& wt) const;
  // Returns the nPol_p x curNumRow_p weight matrix
  virtual Matrix<Float>& weightMat(Matrix<Float>& wtmat) const;
  // Determine whether WEIGHT_SPECTRUM exists.
  Bool existsWeightSpectrum() const;
  // Return weightspectrum (a weight for each channel)
  Cube<Float>& weightSpectrum(Cube<Float>& wtsp) const;
  // Return imaging weight (a weight for each channel)
  //Matrix<Float>& imagingWeight(Matrix<Float>& wt) const;
  // Return True if FieldId/Source has changed since last iteration
  Bool newFieldId() const;
  // Return True if arrayID has changed since last iteration
  Bool newArrayId() const;
  // Return True if SpectralWindow has changed since last iteration
  Bool newSpectralWindow() const;
  // Return the index of the first channel of the current channel group 
  // in the total (selected) spectrum.
  Int channelIndex() const;
  // Return the width of the current group of channels, i.e.,
  // the number of channels returned by visibility() and frequency().
  Int channelGroupSize() const;
  
  // Return the number of correlations in the current iteration
  Int nCorr() const;;
  // Return the number of rows in the current iteration
  Int nRow() const;
  // Return the row ids as from the original root table. This is useful 
  // to find correspondance between a given row in this iteration to the 
  // original ms row
  virtual Vector<uInt>& rowIds(Vector<uInt>& rowids) const;
  // Return the numbers of rows in the current chunk
  Int nRowChunk() const;
  // Return the number of sub-intervals in the current chunk
  Int nSubInterval() const;
  // Call to use the slurp i/o method for all scalar columns. This
  // will set the BucketCache cache size to the full column length
  // and cause the full column to be cached in memory, if
  // any value of the column is used. In case of out-of-memory,
  // it will automatically fall-back on the smaller cache size.
  // Slurping the column is to be considered as a work-around for the
  // Table i/o code, which uses BucketCache and performs extremely bad
  // for random access. Slurping is useful when iterating non-sequentially
  // an MS or parts of an MS, it is not tested with multiple MSs.
  void slurp() const;
  // Velocity selection - specify the output channels in velocity:
  // nChan - number of output channels, vStart - start velocity,
  // vInc - velocity increment. So channel i will have velocity 
  // vStart + i*vInc (i=0,nChan-1).
  // Specify velocities as in e.g., MVRadialVelocity(Quantity(2001.,"km/s")).
  // The reference type and velocity definition are specified separately.
  // Note that no averaging is performed, the visibilities will be interpolated
  // and sampled at the specified velocities, it's up to you to choose a vInc
  // appropriate to the channel width.
  // The REST_FREQUENCY column in the SPECTRAL_WINDOW subtable is used to
  // determine the velocity-frequency conversion.
  // By default calculations are done for a single velocity with offsets 
  // applied for the others (ok for non-rel velocities with RADIO defn), 
  // set precise to True to do a full conversion for each output channel.(NYI)
  ROVisibilityIterator& 
  selectVelocity(Int nChan, 
		 const MVRadialVelocity& vStart, const MVRadialVelocity& vInc,
		 MRadialVelocity::Types rvType = MRadialVelocity::LSR,
		 MDoppler::Types dType = MDoppler::RADIO, Bool precise=False);
  // Select the velocity interpolation scheme.
  // At present the choice is limited to : nearest and linear, linear
  // is the default. 
  // TODO: add cubic, spline and possibly FFT
  ROVisibilityIterator& velInterpolation(const String& type);
  // Channel selection - only the selected channels will be returned by the
  // access functions. The default spectralWindow is the current one (or 0)
  // This allows selection of the input channels, producing
  // nGroup groups of width output channels. Default is to return all channels
  // in a single group.
  ROVisibilityIterator& selectChannel(Int nGroup=1, Int start=0, Int width=0, 
				      Int increment=1, Int spectralWindow=-1);
  //Same as above except when multiple ms's are to be accessed
  ROVisibilityIterator& selectChannel(Block< Vector<Int> >& blockNGroup,
				      Block< Vector<Int> >& blockStart,
				      Block< Vector<Int> >& blockWidth,
				      Block< Vector<Int> >& blockIncr,
				      Block< Vector<Int> >& blockSpw);
  //get the channel selection ...the block over the number of ms's associated
  // with this iterator
  void getChannelSelection(Block< Vector<Int> >& blockNGroup,
			   Block< Vector<Int> >& blockStart,
			   Block< Vector<Int> >& blockWidth,
			   Block< Vector<Int> >& blockIncr,
			   Block< Vector<Int> >& blockSpw);
  // Translate slicesv from the form returned by MSSelection::getChanSlices()
  // to matv as used by setChanAveBounds().  widthsv is the channel averaging
  // width for each _selected_ spw.
  void slicesToMatrices(Vector<Matrix<Int> >& matv,
                        const Vector<Vector<Slice> >& slicesv,
                        const Vector<Int>& widthsv) const;
  // Get the spw, start  and nchan for all the ms's is this Visiter that 
  // match the frequecy "freqstart-freqStep" and "freqEnd+freqStep" range
  // Can help in doing channel selection above..
  // freqFrame is the frame the caller frequency values are in (freqStart and freqEnd)
  // These will be converted to the frame of the selected spw to match
  void getSpwInFreqRange(Block<Vector<Int> >& spw, 
				 Block<Vector<Int> >& start, 
				 Block<Vector<Int> >& nchan, 
				 Double freqStart, Double freqEnd, 
				 Double freqStep, MFrequency::Types freqFrame=MFrequency::LSRK);

  //Get the frequency range of the data selection
  void getFreqInSpwRange(Double& freqStart, Double& freqEnd, MFrequency::Types freqframe = MFrequency::LSRK);

  // Attach a VisBuffer object.
  // Note that while more than one VisBuffer may be attached, only the
  // last one is actively updated. A Stack is kept internally, so after 
  // a detach, the previous VisBuffer becomes active again.
  void attachVisBuffer(VisBuffer& vb);
  // Detach a VisBuffer object.
  // If the object detached is not the last one attached an exception
  // is thrown.
  void detachVisBuffer(VisBuffer& vb);
  // Access the current ROMSColumns object in MSIter
  const ROMSColumns& msColumns() const;
  // get back the selected spectral windows and spectral channels for
  // current ms 
  void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);
  // Convert the frequency from the observe frame to lsr frame.
  // Returns True in convert if given spw was not observed 
  // in the LSRK frame
  //when ignoreconv=True...no conversion is done from data frame
  //frequency is served as is
  void lsrFrequency(const Int& spw, Vector<Double>& freq, Bool& convert, const  Bool ignoreconv=False);
  //assign a VisImagingWeight object to this iterator
  void useImagingWeight(const VisImagingWeight& imWgt);
  const VisImagingWeight & getImagingWeightGenerator () const;

  //return number  of Ant 
  Int numberAnt();
  //Return number of rows in all selected ms's
  Int numberCoh();
  // Return number of spws, polids, ddids
  Int numberSpw();
  Int numberPol();
  Int numberDDId();
//  ROArrayColumn <Double> & getChannelFrequency () const;
//  Block<Int> getChannelGroupNumber () const;
//  Block<Int> getChannelIncrement () const;
//  Block<Int> getChannelStart () const;
//  Block<Int> getChannelWidth () const;
  Int getDataDescriptionId () const;
  const MeasurementSet & getMeasurementSet() const;;
  Int getMeasurementSetId() const;
  Int getNAntennas () const;
  MEpoch getEpoch () const;
  ///////////////MFrequency::Types getObservatoryFrequencyType () const; //???
  ///////////////MPosition getObservatoryPosition () const;
  ///////////////MDirection getPhaseCenter () const;
  Vector<Float> getReceptor0Angle ();
  Vector<uInt> getRowIds () const;

  // Returns the columns that the VisibilityIterator is sorting by.
  const Block<Int>& getSortColumns() const;

  static void lsrFrequency (const Int& spw,
                            Vector<Double>& freq,
                            Bool & convert,
                            const Block<Int> & chanStart,
                            const Block<Int> & chanWidth,
                            const Block<Int> & chanInc,
                            const Block<Int> & numChanGroup,
                            const ROArrayColumn <Double> & chanFreqs,
                            const ROScalarColumn<Int> & obsMFreqTypes,
                            const MEpoch & ep,
                            const MPosition & obsPos,
                            const MDirection & dir);

  static String getAipsRcBase () { return "VisibilityIterator.async";}

protected:

  ROVisibilityIterator (const asyncio::PrefetchColumns * prefetchColumns,
                        const Block<MeasurementSet>& mss,
                        const Block<Int>& sortColumns,
                        const Bool addDefaultSortCols,
                        Double timeInterval,
                        Bool writable);

  void construct (const asyncio::PrefetchColumns * prefetchColumns,
                  const Block<MeasurementSet>& mss,
                  const Block<Int>& sortColumns,
                  const Bool addDefaultSortCols,
                  Double timeInterval,
                  Bool writable,
                  const Factory & factory);

  VisibilityIteratorReadImpl * getReadImpl() const;

  bool existsColumn (VisBufferComponents::EnumType id) const;
  // advance the iteration
  virtual void advance();
  // set the currently selected table
  virtual void setSelTable();
  // set the iteration state
  void setState();
  // get the TOPO frequencies from the selected velocities and the obs. vel.
  void getTopoFreqs();
  void getTopoFreqs(Vector<Double> & lsrFreq, Vector<Double> & selFreq); // for async i/o
  void getLsrInfo (Block<Int> & channelGroupNumber,
                           Block<Int> & channelIncrement,
                           Block<Int> & channelStart,
                           Block<Int> & channelWidth,
                           MPosition & observatoryPositon,
                           MDirection & phaseCenter,
                           Bool & velocitySelection) const;
  std::vector<MeasurementSet> getMeasurementSets () const;
  const MSDerivedValues & getMSD () const; // for use by Async I/O *ONLY*
  // update the DATA slicer
  virtual void updateSlicer();
  // attach the column objects to the currently selected table
  virtual void attachColumns(const Table &t);
  // returns the table, to which columns are attached, 
  // can be overridden in derived classes
  virtual const Table attachTable() const;
  // get the (velocity selected) interpolated visibilities, flags and weights.
  // It is not really const at all (it seems to use This-> trickery so callers
  // like flag() can be declared const).
//  void getInterpolatedVisFlagWeight(DataColumn whichOne) const;
  // get the (velocity selected) interpolated FLOAT_DATA (as real Floats),
  // flags and weights.
//  void getInterpolatedFloatDataFlagWeight() const;
  // get the visibility data (observed, corrected or model);
  // deals with Float and Complex observed data (DATA or FLOAT_DATA)
  void getDataColumn(DataColumn whichOne, const Slicer& slicer, 
			     Cube<Complex>& data) const;
  void getDataColumn(DataColumn whichOne, Cube<Complex>& data) const;
  // get FLOAT_DATA as real Floats.
  void getFloatDataColumn(const Slicer& slicer, Cube<Float>& data) const;
  void getFloatDataColumn(Cube<Float>& data) const;

  void originChunks(Bool forceRewind);
  //Re-Do the channel selection in multi ms case 
  void doChannelSelection();
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
  virtual void setTileCache();
  //Check if spw is in selected SPW for actual ms
  Bool isInSelectedSPW(const Int& spw);
  // Updates, if necessary, rowIds_p member for the current chunk
  void update_rowIds() const;
  void setAsyncEnabled (Bool enable);
  template<class T>
    void getColScalar(const ROScalarColumn<T> &column, Vector<T> &array, Bool resize) const;
  template<class T>
    void getColArray(const ROArrayColumn<T> &column, Array<T> &array, Bool resize) const;
  // column access functions, can be overridden in derived classes
  virtual void getCol(const ROScalarColumn<Bool> &column, Vector<Bool> &array, Bool resize = False) const;
  virtual void getCol(const ROScalarColumn<Int> &column, Vector<Int> &array, Bool resize = False) const;
  virtual void getCol(const ROScalarColumn<Double> &column, Vector<Double> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Bool> &column, Array<Bool> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Float> &column, Array<Float> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Double> &column, Array<Double> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Complex> &column, Array<Complex> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Bool> &column, const Slicer &slicer, Array<Bool> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Float> &column, const Slicer &slicer, Array<Float> &array, Bool resize = False) const;
  virtual void getCol(const ROArrayColumn<Complex> &column, const Slicer &slicer, Array<Complex> &array, Bool resize = False) const;
  //  void getCol(const String &colName, Array<Double> &array,
  //                      Array<Double> &all, Bool resize = False) const;
  //  void getCol(const String &colName, Vector<Bool> &array,
  //                      Vector<Bool> &all, Bool resize = False) const;
  //  void getCol(const String &colName, Vector<Int> &array,
  //                      Vector<Int> &all, Bool resize = False) const;
  //  void getCol(const String &colName, Vector<Double> &array,
  //                      Vector<Double> &all, Bool resize = False) const;
  template<class T>
  void swapyz(Cube<T>& out, const Cube<T>& in) const;

  VisibilityIteratorReadImpl * readImpl_p;
};


// <summary>
// VisibilityIterator iterates through one or more writable MeasurementSets
// </summary>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>
//   <li> <linkto class="ROVisibilityIterator">ROVisibilityIterator</linkto>
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
// <linkto class="ROVisibilityIterator">ROVisibilityIterator</linkto>.
//
// One should use <linkto class="VisBuffer">VisBuffer</linkto>
// to access chunks of data.
//
// As with an ROVI, a VI can be created to only utilize synchronous I/O
// (the "normal" way) or it can potentially use asynchronous I/O.  To create
// an asynchronous instance of VI requires using an appropriate constructor
// (i.e., one that has PrefetchColumns * as its first argument) and setting
// the CASARC setting "VisibilityIterator.async.disabled: False".
//
// When a write operation is performed using the various setX methods of an
// asynchronous instance of this class, the data is shipped over to the VLAT
// which writes out when it is not in the middle of performing a read ahead
// of a VisBuffer.
//
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
class VisibilityIterator : public ROVisibilityIterator
{

    friend class VisibilityIteratorWriteImpl;

public:
  // Constructors.
  // Note: The VisibilityIterator is not initialized correctly by default, you
  // need to call origin() before using it to iterate.
  VisibilityIterator();
  VisibilityIterator(MeasurementSet & ms, const Block<Int>& sortColumns, 
       Double timeInterval=0);
  VisibilityIterator(MeasurementSet & ms, const Block<Int>& sortColumns, 
		     const Bool addDefaultSortCols,
		     Double timeInterval=0);
  // Same as previous constructor, but with multiple MSs to iterate over.
  VisibilityIterator(const Block<MeasurementSet>& mss,
		       const Block<Int>& sortColumns, 
		       Double timeInterval=0);
  VisibilityIterator(const Block<MeasurementSet>& mss,
		     const Block<Int>& sortColumns, const Bool addDefaultSortCols, 
		       Double timeInterval=0);
  VisibilityIterator (const asyncio::PrefetchColumns * prefetchColumns,
                      const Block<MeasurementSet>& mss,
                      const Block<Int>& sortColumns,
                      const Bool addDefaultSortCols = True,
                      Double timeInterval = 0);
  VisibilityIterator (const asyncio::PrefetchColumns * prefetchColumns,
                      MeasurementSet & mss,
                      const Block<Int>& sortColumns,
                      const Bool addDefaultSortCols = True,
                      Double timeInterval = 0);
  VisibilityIterator(const VisibilityIterator & MSI);
  VisibilityIterator(const asyncio::PrefetchColumns * prefetchColumns, const VisibilityIterator & other);

  
  virtual ~VisibilityIterator();

  VisibilityIterator & operator=(const VisibilityIterator &MSI);


  // Members
  
  Bool isWritable () const;

  // Advance iterator through data
  VisibilityIterator & operator++(int);
  VisibilityIterator & operator++();
  // Set/modify the flags in the data.
  // This will flag all channels in the original data that contributed to
  // the output channel in the case of channel averaging.
  // All polarizations have the same flag value.
  void setFlag(const Matrix<Bool>& flag);
  // Set/modify the flags in the data.
  // This sets the flags as found in the MS, Cube(npol,nchan,nrow),
  // where nrow is the number of rows in the current iteration (given by
  // nRow()).
  virtual void setFlag(const Cube<Bool>& flag);
  // Set/modify the flag row column; dimension Vector(nrow)
  virtual void setFlagRow(const Vector<Bool>& rowflags);

  void setFlagCategory(const Array<Bool>& fc);

  // Set/modify the visibilities.
  // This is possibly only for a 'reference' MS which has a new DATA column.
  // The first axis of the matrix should equal the selected number of channels
  // in the original MS.
  // If the MS does not contain all polarizations, only the parallel
  // hand polarizations are used.
  void setVis(const Matrix<CStokesVector>& vis, DataColumn whichOne);
  // Set/modify the visibilities
  // This sets the data as found in the MS, Cube(npol,nchan,nrow).
  virtual void setVis(const Cube<Complex>& vis, DataColumn whichOne);
  // Set the visibility and flags, and interpolate from velocities if needed
  virtual void setVisAndFlag(const Cube<Complex>& vis, const Cube<Bool>& flag,
	   	             DataColumn whichOne);
  // Set/modify the weights
  void setWeight(const Vector<Float>& wt);
  // Set/modify the weightMat
  virtual void setWeightMat(const Matrix<Float>& wtmat);
  // Set/modify the weightSpectrum
  virtual void setWeightSpectrum(const Cube<Float>& wtsp);
  // Set/modify the Sigma
  void setSigma(const Vector<Float>& sig);
  // Set/modify the ncorr x nrow SigmaMat.
  void setSigmaMat(const Matrix<Float>& sigmat);
  
  //This puts a model into the descriptor of the current ms in the iterator
  //Set iscomponentlist to True if the record represent a componentlist
  //if False then it is a FTMachine Record that holds the model image 
  //note the spw and fields selected are going to be associated with this model
  //incremetal =True implies add the model to previous any existant model 
  //in the ms for the spw and fields 
  //false means any existant model will be replaces.
  void putModel(const RecordInterface& rec, Bool iscomponentlist=True, Bool incremental=False);


  void writeBack (VisBuffer *);

protected:

  virtual void attachColumns(const Table &t);

  void construct ();


  VisibilityIteratorWriteImpl * getWriteImpl() const;

  void initializeBackWriters ();

  VisibilityIteratorWriteImpl * writeImpl_p;
};

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <msvis/MSVis/VisibilityIterator.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif

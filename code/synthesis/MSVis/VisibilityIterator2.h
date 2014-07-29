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

#ifndef MSVIS_VISIBILITYITERATOR2_H
#define MSVIS_VISIBILITYITERATOR2_H

#include <casa/aips.h>

//#include <casa/Arrays/Cube.h>
//#include <casa/Arrays/Matrix.h>
//#include <casa/Arrays/Slicer.h>
//#include <casa/BasicSL/String.h>
//#include <casa/Containers/Stack.h>
//#include <casa/Quanta/MVDoppler.h>
//#include <measures/Measures/MCDoppler.h>
//#include <measures/Measures/MDoppler.h>
//#include <measures/Measures/MeasConvert.h>
//#include <measures/Measures/Stokes.h>
//#include <ms/MeasurementSets/MSDerivedValues.h>
#include <ms/MeasurementSets/MSIter.h>
#include <ms/MeasurementSets/MeasurementSet.h>
//#include <ms/MeasurementSets/MSSelection.h>
//#include <scimath/Mathematics/RigidVector.h>
//#include <scimath/Mathematics/SquareMatrix.h>
//#include <synthesis/MSVis/StokesVector.h>
//#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
//#include <synthesis/MSVis/ViFrequencySelection.h>
//#include <synthesis/MSVis/VisImagingWeight.h>
//#include <tables/Tables/ArrayColumn.h>
//#include <tables/Tables/ScalarColumn.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forward decl

//class CStokesVector;
class MDirection;
class MSDerivedValues;
class MSIter;
class RecordInterface;
class ROMSAntennaColumns;
class ROMSDataDescColumns;
class ROMSFeedColumns;
class ROMSFieldColumns;
class ROMSFlagCmdColumns;
class ROMSHistoryColumns;
class ROMSObservationColumns;
class ROMSPointingColumns;
class ROMSPolarizationColumns;
class ROMSProcessorColumns;
class ROMSSpWindowColumns;
class ROMSStateColumns;
class ROMSDopplerColumns;
class ROMSFreqOffsetColumns;
class ROMSSourceColumns;
class ROMSSysCalColumns;
class ROMSWeatherColumns;
template <typename T, Int N> class RigidVector;
template <typename T, Int N> class SquareMatrix;
class VisImagingWeight;

namespace asyncio {

class VLAT;

} // end namespace asyncio

// Forward declarations outside namespace vi
class MSTransformIteratorFactory;

namespace vi {

class FrequencySelection;
class FrequencySelections;
class VisBuffer2;
class VisBufferComponents2;
class VisBufferImpl2;
class ViImplementation2;

class SubtableColumns {

public:

    // Simple wrapper class to limit access to only the columns associated with the
    // current MS's subtables.  This prevents misuse of the main table data columns
    // which are provided as part of the MSColumns object returned by
    // MSIter::msColumns.

    SubtableColumns (CountedPtr <MSIter> msIter);

    const ROMSAntennaColumns& antenna() const;
    const ROMSDataDescColumns& dataDescription() const;
    const ROMSFeedColumns& feed() const;
    const ROMSFieldColumns& field() const;
    const ROMSFlagCmdColumns& flagCmd() const;
    const ROMSHistoryColumns& history() const;
    const ROMSObservationColumns& observation() const;
    const ROMSPointingColumns& pointing() const;
    const ROMSPolarizationColumns& polarization() const;
    const ROMSProcessorColumns& processor() const;
    const ROMSSpWindowColumns& spectralWindow() const;
    const ROMSStateColumns& state() const;
    const ROMSDopplerColumns& doppler() const;
    const ROMSFreqOffsetColumns& freqOffset() const;
    const ROMSSourceColumns& source() const;
    const ROMSSysCalColumns& sysCal() const;
    const ROMSWeatherColumns& weather() const;

private:

    CountedPtr <MSIter> msIter_p;
};

///////////////////////////////////////////////////////////////////
//
//    Subchunk class
//
//    Provides the ordered pair identifying the current subchunk.  The first
//    component is the chunk (goes from 0 to nChunks-1 during the outer loop
//    iteration).  The second component is the subchunk number which is
//    zero after VisibilityIterator::origin is called and is increment
//    until ROVI::more returns False.

class Subchunk : public std::pair<Int, Int>{

public:

    // First component is Chunk and second is Subchunk

    Subchunk () { resetToOrigin ();}
    Subchunk (Int a , Int b) : std::pair<Int,Int> (a,b) {}

    Bool operator== (const Subchunk & other){
        return first == other.first && second == other.second;
    }

    Bool operator< (const Subchunk & other){
        return first < other.first ||
               (first == other.first && second < other.second);
    }

    Bool atOrigin () const { return * this == Subchunk ();}
    Int chunk () const { return first;}
    void incrementSubChunk () { second ++;}
    void incrementChunk () { first ++; second = 0; }

    void resetSubChunk () { second = 0;} // position to start of chunk

    void resetToOrigin () { first = 0; second = 0; }
       // Make a subchunk pair that is positioned to the first subchunk of the
       // first chunk (i.e., 0,0)

    Int subchunk () const { return second;}
    String toString () const;

    static Subchunk noMoreData ();

private:

};



///////////////////////////////////////////////////////////
//
// Code to provide interface to weight function
//
// WeightScaling is essentially the interface to the function
// used for weight scaling while WeightScalingImpl is the class
// to actually use to create WeightScaling functors.  The use of
// a templated implementation allows the wrapping of both functors
// and functions while the base class allows polymorphic storage.

class WeightScaling {
public:

    virtual ~WeightScaling () {}
    Float operator() (Float x) { return apply (x);}

    static CountedPtr<WeightScaling> generateUnityWeightScaling ();
    static CountedPtr<WeightScaling> generateIdentityWeightScaling ();
    static CountedPtr<WeightScaling> generateSquareWeightScaling ();

protected:

    virtual Float apply (Float) = 0;
    static Float unity (Float);
    static Float identity (Float x);
    static Float square (Float x);
};

template<typename F>
class WeightScalingImpl : public WeightScaling {
public:

    // Provide either a unary function, Float (*) (Float), or
    // a functor class having a Float operator() (Float) method.

    WeightScalingImpl (F f) : function_p (f) {}

    Float apply (Float f) { return function_p (f);}

private:

    F function_p;
};


template<typename F>
CountedPtr <WeightScaling> generateWeightScaling (F f) { return new WeightScalingImpl<F> (f);}

class SortColumns {
public:

    explicit SortColumns (const Block<Int> & columnIds = Block<Int> (), Bool addDefaultColumns = True);

    Bool shouldAddDefaultColumns () const;
    const Block<Int> & getColumnIds () const;

private:

    Bool addDefaultColumns_p;
    Block<Int> columnIds_p;
};

class VisibilityIterator2;

//////////////////////////////////////////////////////////////////////
//
// Class ViFactory
//
// The ViFactory is a class that can be used to initialize the implementation of
// a VisibilityIterator2.  It is passed into VI2's constructor where it creates
// the needed ViImplementation object used by the VI2.  The first example of this
// factory is the AveragingVi2Factory which is used to create a VI2 which will
// return time-averaged data.

class ViFactory {

public:

    virtual ~ViFactory () {}

protected:

    friend class VisibilityIterator2;

    virtual ViImplementation2 * createVi (VisibilityIterator2 *) const = 0;
};

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
//   <li> <linkto class="PrefetchColumns">PrefetchColumns</linkto>
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
//
// VisibilityIterator2s can be either synchronous or asynchronous, depending
// on the constructor used to create them as well as the current value of
// a CASARC file setting.  A synchronous instance is works the same as
// this class ever worked; an asynchronous instance uses a second thread
// (the Visibility Lookahead Thread or VLAT) to fill the VisBuffers in
// advance of their use by the original thread.
//
// To create an asynchronous instance of ROVI you must use one of the two
// constructors which have a pointer to a PrefetchColumns object as the
// first argument.  This object specifies which VisBuffer components should be
// prefetched by the VLAT; accessing components not specified in the PrefetchColumns
// object will result in an exception containing an error message indicating
// that the VisBuffer does not contain the requested column.  In addition
// to using the appropriate constructor, the CASARC file setting
// VisibilityIterator2.async.enabled can be used to turn asynchronous I/O
// off globally; if it's globally enabled then it is still possible for the
// user to choose to enable/disable it on a VI by VI basis.
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
// VisibilityIterator2.async.enabled - Boolean value that enables or disables
//     async I/O.  The default value is currently False (i.e., disabled).
// VisibilityIterator2.async.nBuffers - The number of lookahead buffers.  This
//     defaults to 2.
//
//
// Debug settings
// --------------
//
// VisibilityIterator2.async.doStats: true
// VisibilityIterator2.async.debug.logFile: stderr
// VisibilityIterator2.async.debug.logLevel: 1
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
class VisibilityIterator2 : private boost::noncopyable
{
    // These classes are members of the VI/VB framework and need extra
    // access to the non-public methods of this class.  Any additions of
    // classes not in this framework is strongly discouraged.
    //
    // Friends of the class should limit themselves to accessing the protected
    // area of this class so that items in the private area can remeain de
    // facto private.

    friend class VisibilityIteratorImpl2;
    friend class VisBuffer2Adapter;
    friend class VisBufferImpl2;
    friend class VisBufferState;
    friend class asyncio::VLAT; // allow VI lookahead thread class to access protected
                                // functions VLAT should not access private parts,
                                // especially variables
    friend class casa::MSTransformIteratorFactory;

public:


  class Factory { // Interface for implementation creation factory

  public:

      virtual ~Factory () {}

      virtual ViImplementation2 *
      operator() (const VisBufferComponents2 * /*prefetchColumns*/,
                  const Block<MeasurementSet>& /*mss*/,
                  const Block<Int>& /*sortColumns*/,
                  const Bool /*addDefaultSortCols*/,
                  Double /*timeInterval*/) const
      {
          return NULL;
      }
  };

  typedef enum MSIter::PolFrame PolFrame;

  typedef enum DataColumn {
      Observed=0,  // Observed data
      Model,       // Model data
      Corrected    // Corrected data
  } DataColumn;

  // Construct from an MS and a Block of MS column enums specifying the
  // iteration order.  These can be specified as MS::ANTENNA1, MS::ARRAY_ID,
  // etc.; they are defined in MSMainEnums.h.
  // If no order is specified, it uses the default sort
  // order of MSIter, which is not necessarily the raw order of ms!
  // The default ordering is ARRAY_ID, FIELD_ID, DATA_DESC_ID,
  // and TIME, but check MSIter.h to be sure.
  // These columns will be added first if they are not specified.
  //
  // An optional timeInterval (in seconds) can be given to iterate through
  // chunks of time.  The default interval of 0 groups all times together.
  // Every 'chunk' of data contains all data within a certain time interval and
  // with identical values of the other iteration columns (e.g.  DATA_DESC_ID
  // and FIELD_ID).
  //
  // A set of prefetch columns can be specified for asynchronous I/O use.  These
  // roughly correspond to the fields accessible from the VisBuffer.  Specifying
  // prefetch columns will cause an asynchronous VI to be created if asynchronous
  // I/O is globally enabled.  If the user wishes to have application-specific
  // enable/disable of asynchronous I/O then they must implement logic that
  // either either provides prefetch columns (enables) or a null pointer (disables).

  VisibilityIterator2 (const MeasurementSet& ms,
                       const SortColumns & sortColumns = SortColumns (),
                       Bool isWritable = False,
                       const VisBufferComponents2 * prefetchColumns = 0,
                       Double timeInterval = 0);

  VisibilityIterator2 (const Block<const MeasurementSet *>& mss,
                       const SortColumns & sortColumns = SortColumns (),
                       Bool isWritable = False,
                       const VisBufferComponents2 * prefetchColumns = 0,
                       Double timeInterval = 0);

  VisibilityIterator2 (const ViFactory & factory);

  // Destructor

  virtual ~VisibilityIterator2();

  ///////////////////////////////////////////////////////////////////
  //
  // BEGIN Experimental Section
  //

  static VisibilityIterator2 * copyingViFactory (const MeasurementSet & srcMs,
                                                 MeasurementSet & dstMs);

  //
  // END Experimental Section
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  // Iteration movement methods
  //
  // The typical use case for VisibilityIteration looks like
  //
  // for (vi.originChunks(); vi.moreChunks(); nextChunk()){
  //     for (vi.origin (); vi.more(); vi++){
  //     }
  // }
  //
  // The outer loop is the "chunk" loop and the inner loop is the "subchunk"
  // loop.  A chunk contains all the rows having identical values for the
  // sort columns values except time; time can have an optional interval
  // value specified either in the constructor or via the setInterval
  // method.  If an interval is specified the set of rows will all have
  // timestamps that are within that interval of the first row in the
  // chunk; if no interval is specified then the chunk will contain only
  // rows having the identical timestamp.  If multiple MSs are specified
  // in the constructor then changing from one MS to the next will occur
  // on a chunk boundary (i.e., a chunk will never contain data from more
  // than one MS).
  //
  // A subchunk typically contains all the rows in the chunk having identical
  // timestamps.  The only exception is when the user calls setRowBlocking(N)
  // with a positive value.  In this case, the subchunk *may* contain up to
  // N rows; however, this is only a suggestion to the VI.  If there are
  // less than N rows remaining in the chunk then fewer than N rows will be
  // contained in the chunk.  The number of rows in a subchunk will also be
  // less than N if a larger set of rows would create an inconsistently
  // shaped visibility cube; this could happen if the number of channels
  // changes from one timestamp to another or the framed frequency
  // selection selects a different number of channels from one timestamp to
  // the next.
  //
  // origin () - positions VI to the start of the current chunk
  // operator++ - advance VI to next subchunk if it exists
  // more - returns True if the VI is pointing to a valid subchunk
  //
  // originChunks - Move to the first chunk of data.
  // operator++ - advance VI to the next chunk if it exists
  // moreChunks - returns True if the VI is pointing to a valid chunk.

  void origin(); // Reset to start of the chunk
  void next ();
  Bool more() const;

  void originChunks();
  void nextChunk();
  Bool moreChunks() const;

  // Returns the pair (chunk,subchunk) for the current position of the VI.  Only
  // valid after origin has been called.

  Subchunk getSubchunkId () const;

  ///////////////////////////////////////////////////////////////////
  //
  // Iterator operation methods
  //
  // These methods alter the way that the VI operates.  After applying any of these
  // (except slurp which should be called after originChunks) originChunks must be
  // called before the VI is advanced again; otherwise an exception will be thrown
  // (changing these values in the middle of a sweep can lead to unexpected behavior).
  //
  // Selecting ranges of frequencies involves initializing a
  // FrequencySelection(s) object and providing it to the VI.  Once
  // properly applied the VI will only return data for the selected
  // frequencies.  No interpolation is performed; the data returned is
  // simply the channels that currently map to the selected frequencies for
  // the specified frame of reference.

  void setFrequencySelection (const FrequencySelection &);  // for single MS
  void setFrequencySelection (const FrequencySelections &); // when using multiple MSs

  // RowBlocking is a suggestion to the iterator that it try to use subchunks
  // having the suggested number of rows.  The VI will attempt to honor the
  // suggestion except when there are not enough rows remaining in the chunk
  // or when putting the suggested number of rows in the subchunk cannot
  // be contained in a cube (e.g., there are sets of row with different
  // numbers of frequencies, etc.).  The latter case will mainly occur
  // when the row visibility shape changes from one row to the next.

  Int getRowBlocking() const;
  void setRowBlocking(Int nRows=0);

  // In this context the interval determines what rows will be contained
  // in a chunk.  A chunk is all the rows in an MS that have the same values
  // in the sort columns, except for time; all rows that otherwise have the
  // same values for the sort columns will be in the chunk if they are no
  // later than "interval" seconds after the first row in the chunk.
  //
  // *** This value is unrelated to the interval field of the MS main table.

  Double getInterval() const;
  void setInterval(Double timeInterval);

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

  ///////////////////////////////////////////////////////////////////
  //
  // Other methods
  //

  // Returns True if this is an asynchronous VI

  Bool isAsynchronous () const;

  // Returns True if async I/O is globally enabled.

  static Bool isAsynchronousIoEnabled();

  // Returns True if this VI is writable (always False for ROVI and
  // True for VI; see VisibilityIterator class).

  Bool isWritable () const;

  // Returns the columns that the VisibilityIterator2 is sorting by.  These are
  // defined in MSMainEnums.h.  These can be specified as MS::ANTENNA1,
  // MS::ARRAY_ID, etc.

  const SortColumns & getSortColumns() const;

  // Returns true if the specified column exists.

  bool existsColumn (VisBufferComponent2 id) const;

  // Returns the VisBuffer permanently attached to this VisibilityIterator.

  VisBuffer2 * getVisBuffer ();

  // Manages the weight function that can be used to process the weights
  // produced by the "scaled" variants of the weight accessors.  Use
  // generateWeightscaling to create a WeightScaling object.  This allow you
  // to use either a function (FLoat (Float)) or a functor (object having
  // method Float operator () (Float)).
  //
  // To revert to having no scaling function, call setWeightScaling with
  // 0 as the argument.  Any call to setWeightScaling needs to be followed
  // by an originChunks call before any further data access is performed
  // using the VI.
  //
  // The method hasWeightScaling will return false if either no weightScaling
  // object has installed or setWeightScaling (0) was called.  There is not
  // way for VI to know if the user has passed in the identity function;
  // doing so will still cause hasWeightScaling to return true.

  virtual void setWeightScaling (CountedPtr<WeightScaling> weightscaling);
  virtual Bool hasWeightScaling () const;

  // Return number of spws, polids, ddids for the current MS

  Int nSpectralWindows () const;
  Int nPolarizationIds () const; // number of different polarization configurations
                                 // (i.e., length of polarization subtable)
  Int nDataDescriptionIds () const;

  // Determine whether WEIGHT_SPECTRUM exists.

  Bool weightSpectrumExists() const;

  // Determine whether WEIGHT_SPECTRUM exists.

  Bool sigmaSpectrumExists() const;

  //reference to actual ms in interator

  const MeasurementSet& ms() const;

  const vi::SubtableColumns & subtableColumns () const;

  // The reporting frame of reference is the default frame of reference to be
  // used when the user requests the frequencies of the current data selection
  // in a VisBuffer.  This is useful when the user wishes to select the data
  // using one frame of reference but use the frequencies from another frame
  // of reference.  If not specified then the default will be to use the
  // frame of reference used to select the data.
  //
  // These *do not* change the selection in any way.

  Int getReportingFrameOfReference () const;
  void setReportingFrameOfReference (Int);

  // Return the numbers of rows in the current chunk

  Int nRowsInChunk() const;

  // Assign a VisImagingWeight object to this iterator.  This object is used
  // to generate imaging weights.

  void useImagingWeight(const VisImagingWeight& imWgt);
  const VisImagingWeight & getImagingWeightGenerator () const;

  // Write/modify the flags in the data.
  // This will flag all channels in the original data that contributed to
  // the output channel in the case of channel averaging.
  // All polarizations have the same flag value.

//  void writeFlag(const Matrix<Bool>& flag);

  // Write/modify the flags in the data.
  // This writes the flags as found in the MS, Cube(npol,nchan,nrow),
  // where nrow is the number of rows in the current iteration (given by
  // nRow()).

  virtual void writeFlag(const Cube<Bool>& flag);

  // Write/modify the flag row column; dimension Vector(nrow)

  virtual void writeFlagRow(const Vector<Bool>& rowflags);

  void writeFlagCategory(const Array<Bool>& fc);

  // Write/modify the visibilities.
  // This is possibly only for a 'reference' MS which has a new DATA column.
  // The first axis of the matrix should equal the selected number of channels
  // in the original MS.
  // If the MS does not contain all polarizations, only the parallel
  // hand polarizations are used.

//  void writeVisCorrected (const Matrix<CStokesVector>& vis);
//  void writeVisModel (const Matrix<CStokesVector>& vis);
//  void writeVisObserved (const Matrix<CStokesVector>& vis);

  // Write/modify the visibilities
  // This writes the data as found in the MS, Cube(npol,nchan,nrow).

  void writeVisCorrected (const Cube <Complex> & vis);
  void writeVisModel (const Cube <Complex> & vis);
  void writeVisObserved (const Cube <Complex> & vis);

  // Write/modify the weights

  void writeWeight(const Matrix<Float>& wt);

  // Write/modify the weightMat

  //virtual void writeWeightMat(const Matrix<Float>& wtmat);

  // Write/modify the weightSpectrum

  virtual void writeWeightSpectrum(const Cube<Float>& wtsp);

  // Write/modify the Sigma

  void writeSigma(const Matrix<Float>& sig);

  // Write/modify the ncorr x nrow SigmaMat.

  //void writeSigmaMat(const Matrix<Float>& sigmat);

  // This puts a model into the descriptor of the current ms in the iterator
  // Set iscomponentlist to True if the record represent a componentlist
  // if False then it is a FTMachine Record that holds the model image
  // note the spw and fields selected are going to be associated with this model
  // incremetal =True implies add the model to previous any existant model
  // in the ms for the spw and fields
  // false means any existant model will be replaces.

  void writeModel(const RecordInterface& rec, Bool iscomponentlist=True,
                  Bool incremental=False);

  // Requests that the modified VisBuffer2 be written back to the visibility
  // at the same spot that it came from.  The dirtyComponents feature of
  // VisBuffer is used to mark which portions of the VisBuffer actually need
  // to be written back out.

  void writeBackChanges (VisBuffer2 *);

//**********************************************************************
// Internal methods below this line
//**********************************************************************

protected:


//  VisibilityIterator2 (const VisBufferComponents2 * prefetchColumns,
//                         const Block<MeasurementSet>& mss,
//                         const Block<Int>& sortColumns,
//                         const Bool addDefaultSortCols,
//                         Double timeInterval,
//                         Bool writable);

  VisibilityIterator2();

  void construct (const VisBufferComponents2 * prefetchColumns,
                  const Block<const MeasurementSet *>& mss,
                  const SortColumns & sortColumns,
                  Double timeInterval,
                  Bool writable);

//     +------------------+
//     |                  |
//     |  Data Accessors  |
//     |                  |
//     +------------------+

  // These methods are protected and should only be used by subclasses or
  // befriended framework member classes.  Others should access these values
  // via the VisBuffer attached to the VI (see getVisBuffer()).

  // Return antenna1

  void antenna1(Vector<Int>& ant1) const;

  // Return antenna2

  void antenna2(Vector<Int>& ant2) const;

  // Return feed1

  void feed1(Vector<Int>& fd1) const;

  // Return feed2

  void feed2(Vector<Int>& fd2) const;

  // Returns True is this is a new MS from the last subchunk.

  Bool isNewMS() const;

  // Returns the zero-based index of the current MS in the collection
  // of MSs used to create the VI.

  Int msId() const;

  // Return number  of Ant

  Int nAntennas () const;

  // Return number of rows in all selected ms's

  Int nRowsViWillSweep () const;


  // Return the number of rows in the current iteration

  Int nRows () const;

  void jonesC (Vector<SquareMatrix<Complex,2> >& cjones) const;

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

  const Vector<Float> & feed_pa(Double time) const;

  static Vector<Float> feed_paCalculate(Double time, MSDerivedValues & msd,
                                        Int nAntennas, const MEpoch & mEpoch0,
                                        const Vector<Float> & receptor0Angle);

  // Return nominal parallactic angle at specified time
  // (does not include feed position angle offset--see feed_pa)
  // A global value for all antennas (e.g., small array)

  const Float& parang0(Double time) const;

  static Float parang0Calculate (Double time, MSDerivedValues & msd,
                                 const MEpoch & epoch0);

  // Per antenna:

  const Vector<Float> & parang(Double time) const;

  static Vector<Float> parangCalculate (Double time, MSDerivedValues & msd,
		                        int nAntennas, const MEpoch mEpoch0);

  // Return the antenna AZ/EL Vector(nant)

  MDirection azel0(Double time) const;

  static void azel0Calculate (Double time, MSDerivedValues & msd,
		                      MDirection & azel0, const MEpoch & mEpoch0);
  const Vector<MDirection> & azel(Double time) const;
  static void azelCalculate (Double time,
                             MSDerivedValues & msd,
                             Vector<MDirection> & azel,
                             Int nAnt,
                             const MEpoch & mEpoch0);

  // Return the hour angle for the specified time

  Double hourang(Double time) const;
  static Double hourangCalculate (Double time, MSDerivedValues & msd,
                                  const MEpoch & mEpoch0);

  // Return the current FieldId

  void fieldIds(Vector<Int>&) const;

  // Return the current ArrayId

  void arrayIds (Vector<Int>&) const;

  // Return the current Field Name

  String fieldName() const;

  // Return the current Source Name

  String sourceName() const;

  // Return flag for each polarization, channel and row

  virtual void flag(Cube<Bool>& flags) const;

  // Return flag for each channel & row

  virtual void flag(Matrix<Bool>& flags) const;

  // Determine whether FLAG_CATEGORY is valid.

  Bool existsFlagCategory() const;

  // Return flags for each polarization, channel, category, and row.

  virtual void flagCategory(Array<Bool>& flagCategories) const;

  // Return row flag

  virtual void flagRow(Vector<Bool>& rowflags) const;

  // Return scan number

  virtual void scan(Vector<Int>& scans) const;

  // Return the OBSERVATION_IDs

  virtual void observationId(Vector<Int>& obsids) const;

  // Return the PROCESSOR_IDs

  virtual void processorId(Vector<Int>& procids) const;

  // Return the STATE_IDs

  virtual void stateId(Vector<Int>& stateids) const;

  // Return the current phase center as an MDirection

  virtual const MDirection& phaseCenter() const;

  // Return frame for polarization (returns PolFrame enum)

  virtual Int polFrame() const;

  // Return the correlation type (returns Stokes enums)

  virtual void corrType(Vector<Int>& corrTypes) const;

  // Return sigma

  virtual void sigma(Matrix<Float>& sigmat) const;

  // Return current SpectralWindow

  virtual Int spectralWindow() const;

  virtual void spectralWindows(Vector<Int>& spws) const;

  // Return current Polarization Id

  virtual Int polarizationId() const;

  // Return current DataDescription Id

  virtual Int dataDescriptionId() const;

  virtual void dataDescriptionIds(Vector<Int>& ddis) const;

  // Return MJD midpoint of interval.

  virtual void time(Vector<Double>& t) const;

  // Return MJD centroid of interval.

  virtual void timeCentroid(Vector<Double>& t) const;

  // Return nominal time interval

  virtual void timeInterval(Vector<Double>& ti) const;

  // Return actual time interval

  virtual void exposure(Vector<Double>& expo) const;

  // Return the visibilities as found in the MS, Cube(npol,nchan,nrow).

  virtual void visibilityCorrected (Cube<Complex> & vis) const;
  virtual void visibilityModel (Cube<Complex> & vis) const;
  virtual void visibilityObserved (Cube<Complex> & vis) const;

  // Return FLOAT_DATA as a Cube (npol, nchan, nrow) if found in the MS.
  virtual void floatData (Cube<Float> & fcube) const;

  // Return the visibility 4-vector of polarizations for each channel.
  // If the MS doesn't contain all polarizations, it is assumed it
  // contains one or two parallel hand polarizations.

//  virtual void visibilityCorrected (Matrix<CStokesVector> & vis) const;
//  virtual void visibilityModel (Matrix<CStokesVector> & vis) const;
//  virtual void visibilityObserved (Matrix<CStokesVector> & vis) const;

  // Return the shape of the visibility Cube

  virtual IPosition visibilityShape() const;

  // Return u,v and w (in meters)

  virtual void uvw(Matrix<Double>& uvw) const;

  // Returns the nPol_p x curNumRow_p weight matrix.

  virtual void weight (Matrix<Float>& wt) const;

  // Return weightspectrum (a weight for each channel)

  virtual void weightSpectrum(Cube<Float>& wtsp) const;

  // Return sigmaspectrum (a sigma for each channel)

  virtual void sigmaSpectrum(Cube<Float>& sigsp) const;

  // Returns a vector of the correlation types that were selected
  // into the current subchunk's data.  If Vector<Int> c = vi.getCorrelations();
  // Then c[0] will return the correlation type (as defined in the polarization
  // table) of the first correlation part of a data item (e.g., for a visCube,
  // all elements having the first dimension index of zero will be c[0] type
  // correlations).

  Vector<Int> getCorrelations () const;

  Vector<Double> getFrequencies (Double time, Int frameOfReference) const;
  Vector<Int> getChannels (Double time, Int frameOfReference) const;

  // Convert the frequency from the observe frame to lsr frame.
  // Returns True in convert if given spw was not observed
  // in the LSRK frame
  //when ignoreconv=True...no conversion is done from data frame
  //frequency is served as is

  Int getDataDescriptionId () const;
  const MeasurementSet & getMeasurementSet() const;;
  Int getMeasurementSetId() const;
  Int getNAntennas () const;
  MEpoch getEpoch () const;
  Vector<Float> getReceptor0Angle ();

  // Return the row ids as from the original root table. This is useful
  // to find correspondance between a given row in this iteration to the
  // original ms row.

  void getRowIds (Vector<uInt> &) const;

  Bool newFieldId() const;

  // Return True if arrayID has changed since last iteration

  Bool newArrayId() const;

  // Return True if SpectralWindow has changed since last iteration

  Bool newSpectralWindow() const;

  static String getAipsRcBase () { return "VisibilityIterator2.async";}

//     +-------------------------------+
//     |                               |
//     |  Internal Management Methods  |
//     |                               |
//     +-------------------------------+

  void getSpwInFreqRange(Block<Vector<Int> >& spw,
                         Block<Vector<Int> >& start,
                         Block<Vector<Int> >& nchan,
                         Double freqStart,
                         Double freqEnd,
                         Double freqStep,
                         MFrequency::Types freqFrame=MFrequency::LSRK);

  //Get the frequency range of the data selection

  void getFreqInSpwRange(Double& freqStart,
                         Double& freqEnd,
                         MFrequency::Types freqframe = MFrequency::LSRK);

  // advance the iteration

  void originChunks(Bool forceRewind);

  ViImplementation2 * getImpl() const;

private:

  ViImplementation2 * impl_p;
};

} // end namespace vi

} //# NAMESPACE CASA - END

#endif

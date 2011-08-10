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
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <ms/MeasurementSets/MSIter.h>

// jagonzal: For MS access mutex
#include "msvis/MSVis/AsynchronousTools.h"

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

class ROVisibilityIterator
{

    friend class AsyncEnabler;
    friend class ROVisibilityIteratorAsync;
    friend class VLAT; // allow VI lookahead thread class to access protected functions
                       // VLAT should not access private parts, especially variables

public:

  //# the following is a copy of the enum in MSIter
  //# can't think of a way to get one that known to the outside world from here
  enum PolFrame {
    // Circular polarization
    Circular=0,
    // Linear polarization
    Linear=1
  };

  enum DataColumn {
    // Observed data
    Observed=0,
    // Model data
    Model,
    // Corrected data
    Corrected
  };

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
		       Double timeInterval=0);

  // Same as above, but with the option of using the raw order of ms
  // (addDefaultSortCols=false).
  ROVisibilityIterator(const MeasurementSet& ms, 
		       const Block<Int>& sortColumns, const Bool addDefaultSortCols, 
		       Double timeInterval=0);
 
  // Same as previous constructors, but with multiple MSs to iterate over.
  ROVisibilityIterator(const Block<MeasurementSet>& mss,
		       const Block<Int>& sortColumns, 
		       Double timeInterval=0);
  ROVisibilityIterator(const Block<MeasurementSet>& mss,
                       const Block<Int>& sortColumns, const Bool addDefaultSortCols, 
                       Double timeInterval=0);


  // Copy construct. This calls the assigment operator.
  ROVisibilityIterator(const ROVisibilityIterator & other);

  // Assigment. Any attached VisBuffers are lost in the assign.
  ROVisibilityIterator & operator=(const ROVisibilityIterator &other);

  // Destructor
  virtual ~ROVisibilityIterator();
  
  // Members
  
  Bool isAsyncEnabled () const;

  virtual Bool isWritable () const { return False;}

  // Reset iterator to origin/start of data (of current chunk)
  virtual void origin();
  // Reset iterator to true start of data (first chunk)
  virtual void originChunks();
 
  // Set or reset the time interval (in seconds) to use for iteration.
  // You should call originChunks() to reset the iteration after 
  // calling this.
  virtual void setInterval(Double timeInterval)
  { msIter_p.setInterval(timeInterval);}

  // Set the 'blocking' size for returning data.
  // With the default (0) only a single integration is returned at a time, this
  // is what is currently required for the calibration software. With blocking
  // set, up to nRows can be returned in one go. The chunk 
  // size determines the actual maximum.
  virtual void setRowBlocking(Int nRows=0);

  // Return False if no more data (in current chunk)
  virtual Bool more() const;

  // Advance iterator through data
  virtual ROVisibilityIterator & operator++(int);
  virtual ROVisibilityIterator & operator++();

  // Return False if no more 'Chunks' of data left
  virtual Bool moreChunks() const
  { return msIter_p.more();}

  // Check if ms has change since last iteration
  virtual Bool newMS() const
    { return msIter_p.newMS();}

  virtual Int msId() const
    { return msIter_p.msId();}

  virtual VisBuffer * getVisBuffer ();

  //reference to actual ms in interator 
  virtual const MeasurementSet& ms() const {
    return msIter_p.ms();
  }
 // Advance to the next Chunk of data
  virtual ROVisibilityIterator& nextChunk();

  // Return antenna1
  virtual Vector<Int>& antenna1(Vector<Int>& ant1) const;

  // Return antenna2
  virtual Vector<Int>& antenna2(Vector<Int>& ant2) const;

  // Return feed1
  virtual Vector<Int>& feed1(Vector<Int>& fd1) const;

  // Return feed2
  virtual Vector<Int>& feed2(Vector<Int>& fd2) const;

  // Return channel numbers in selected VisSet spectrum
  // (i.e. disregarding possible selection on the iterator, but
  //  including the selection set when creating the VisSet)
  virtual Vector<Int>& channel(Vector<Int>& chan) const;

  // Return feed configuration matrix for specified antenna
  virtual Vector<SquareMatrix<Complex,2> >& 
  CJones(Vector<SquareMatrix<Complex,2> >& cjones) const;

  // Return receptor angles for all antennae and feeds
  // First axis of the cube is a receptor number,
  // 2nd is antennaId, 3rd is feedId
  // Note: the method is intended to provide an access to MSIter::receptorAngles
  // for VisBuffer in the multi-feed case. It may be worth to change the
  // interface of feed_pa to return the information for all feeds.
  virtual const Cube<Double>& receptorAngles() const;

  // return a string mount identifier for each antenna
  virtual const Vector<String>& antennaMounts() const;

  // Return a cube containing pairs of coordinate offsets for each
  // receptor of each feed (values are in radians, coordinate system is fixed
  // with antenna and is the same one as used to define the BEAM_OFFSET 
  // parameter in the feed table). The cube axes are receptor, antenna, feed.
  virtual const Cube<RigidVector<Double, 2> >& getBeamOffsets() const;

  // True if all elements of the cube returned by getBeamOffsets are zero
  virtual Bool allBeamOffsetsZero() const;

  // Return feed parallactic angles Vector(nant) (1 feed/ant)
  virtual Vector<Float> feed_pa(Double time) const;
  static Vector<Float> feed_paCalculate(Double time, MSDerivedValues & msd,
  									    Int nAntennas, const MEpoch & mEpoch0,
									    const Vector<Float> & receptor0Angle);

  // Return nominal parallactic angle at specified time
  // (does not include feed position angle offset--see feed_pa)
  // A global value for all antennas (e.g., small array)
  virtual const Float& parang0(Double time) const;
  static Float parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & epoch0);

  // Per antenna:
  virtual Vector<Float> parang(Double time) const;
  static Vector<Float> parangCalculate (Double time, MSDerivedValues & msd,
		                                int nAntennas, const MEpoch mEpoch0);

  // Return the antenna AZ/EL Vector(nant) 
  virtual MDirection azel0(Double time) const;
  static void azel0Calculate (Double time, MSDerivedValues & msd,
		                      MDirection & azel0, const MEpoch & mEpoch0);

  virtual Vector<MDirection> azel(Double time) const;
  static void azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
		                     Int nAnt, const MEpoch & mEpoch0);


  // Return the hour angle for the specified time
  virtual Double hourang(Double time) const;
  static Double hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0);

  // Return the current FieldId
  virtual Int fieldId() const
    { return msIter_p.fieldId(); }

  // Return the current ArrayId
  virtual Int arrayId() const
    { return msIter_p.arrayId(); }

  // Return the current Field Name
  virtual String fieldName() const
    { return msIter_p.fieldName(); }

  // Return the current Source Name
  virtual String sourceName() const
    { return msIter_p.sourceName(); }

  // Return flag for each polarization, channel and row
  virtual Cube<Bool>& flag(Cube<Bool>& flags) const;

  // Return flag for each channel & row
  virtual Matrix<Bool>& flag(Matrix<Bool>& flags) const;

  // Return flags for each polarization, channel, category, and row.
  Array<Bool>& flagCategory(Array<Bool>& flagCategories) const;

  // Return row flag
  virtual Vector<Bool>& flagRow(Vector<Bool>& rowflags) const;

  // Return scan number
  virtual Vector<Int>& scan(Vector<Int>& scans) const;

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
  virtual Vector<Double>& lsrFrequency(Vector<Double>& freq) const;

  // Return the current phase center as an MDirection
  virtual const MDirection& phaseCenter() const
  {return msIter_p.phaseCenter(); }

  // Return frame for polarization (returns PolFrame enum)
  virtual Int polFrame() const
  { return msIter_p.polFrame(); }

  // Return the correlation type (returns Stokes enums)
  virtual Vector<Int>& corrType(Vector<Int>& corrTypes) const;

  // Return sigma
  virtual Vector<Float>& sigma(Vector<Float>& sig) const;

  // Return sigma matrix (pol-dep)
  virtual Matrix<Float>& sigmaMat(Matrix<Float>& sigmat) const;

  // Return current SpectralWindow
  virtual Int spectralWindow() const
  { return msIter_p.spectralWindowId(); }

  // Return current Polarization Id
  virtual Int polarizationId() const
  { return msIter_p.polarizationId(); }

  // Return current DataDescription Id
  virtual Int dataDescriptionId() const
  { return msIter_p.dataDescriptionId(); }

  // Return MJD midpoint of interval.
  virtual Vector<Double>& time(Vector<Double>& t) const;

  // Return MJD centroid of interval.
  Vector<Double>& timeCentroid(Vector<Double>& t) const;

  // Return nominal time interval
  virtual Vector<Double>& timeInterval(Vector<Double>& ti) const;

  // Return actual time interval
  virtual Vector<Double>& exposure(Vector<Double>& expo) const;

  // Return the visibilities as found in the MS, Cube(npol,nchan,nrow).
  virtual Cube<Complex>& visibility(Cube<Complex>& vis,
				    DataColumn whichOne) const;

  // Return FLOAT_DATA as a Cube(npol, nchan, nrow) if found in the MS.
  virtual Cube<Float>& floatData(Cube<Float>& fcube) const;

  // Return the visibility 4-vector of polarizations for each channel.
  // If the MS doesn't contain all polarizations, it is assumed it
  // contains one or two parallel hand polarizations.
  virtual Matrix<CStokesVector>& visibility(Matrix<CStokesVector>& vis, 
				    DataColumn whichOne) const;

  // Return the shape of the visibility Cube
  virtual IPosition visibilityShape() const;

  // Return u,v and w (in meters)
  virtual Vector<RigidVector<Double,3> >& uvw(Vector<RigidVector<Double,3> >& uvwvec) const;
  virtual Matrix<Double>& uvwMat(Matrix<Double>& uvwmat) const;

  // Return weight
  virtual Vector<Float>& weight(Vector<Float>& wt) const;

  // Returns the nPol_p x curNumRow_p weight matrix
  virtual Matrix<Float>& weightMat(Matrix<Float>& wtmat) const;

  // Determine whether WEIGHT_SPECTRUM exists.
  Bool existsWeightSpectrum();

  // Return weightspectrum (a weight for each channel)
  virtual Cube<Float>& weightSpectrum(Cube<Float>& wtsp) const;

  // Return imaging weight (a weight for each channel)
  virtual Matrix<Float>& imagingWeight(Matrix<Float>& wt) const;

  // Return True if FieldId/Source has changed since last iteration
  virtual Bool newFieldId() const
  { return (curStartRow_p==0 && msIter_p.newField()); }

  // Return True if arrayID has changed since last iteration
  virtual Bool newArrayId() const
  { return (curStartRow_p==0 && msIter_p.newArray()); }

  // Return True if SpectralWindow has changed since last iteration
  virtual Bool newSpectralWindow() const
  { return (curStartRow_p==0 && msIter_p.newSpectralWindow()); }

  // Return the index of the first channel of the current channel group 
  // in the total (selected) spectrum.
  virtual Int channelIndex() const;

  // Return the width of the current group of channels, i.e.,
  // the number of channels returned by visibility() and frequency().
  virtual Int channelGroupSize() const;
  
  // Return the number of correlations in the current iteration
  virtual Int nCorr() const {return nPol_p; };

  // Return the number of rows in the current iteration
  virtual Int nRow() const;

  // Return the row ids as from the original root table. This is useful 
  // to find correspondance between a given row in this iteration to the 
  // original ms row
  virtual Vector<uInt>& rowIds(Vector<uInt>& rowids) const; 

  // Return the numbers of rows in the current chunk
  virtual Int nRowChunk() const;

  // Return the number of sub-intervals in the current chunk
  virtual Int nSubInterval() const;

  // Call to use the slurp i/o method for all scalar columns. This
  // will set the BucketCache cache size to the full column length
  // and cause the full column to be cached in memory, if
  // any value of the column is used. In case of out-of-memory,
  // it will automatically fall-back on the smaller cache size.
  // Slurping the column is to be considered as a work-around for the
  // Table i/o code, which uses BucketCache and performs extremely bad
  // for random access. Slurping is useful when iterating non-sequentially
  // an MS or parts of an MS, it is not tested with multiple MSs.
  virtual void slurp() const;

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
  virtual ROVisibilityIterator& 
  selectVelocity(Int nChan, 
		 const MVRadialVelocity& vStart, const MVRadialVelocity& vInc,
		 MRadialVelocity::Types rvType = MRadialVelocity::LSR,
		 MDoppler::Types dType = MDoppler::RADIO, Bool precise=False);

  // Select the velocity interpolation scheme.
  // At present the choice is limited to : nearest and linear, linear
  // is the default. 
  // TODO: add cubic, spline and possibly FFT
  virtual ROVisibilityIterator& velInterpolation(const String& type);

  // Channel selection - only the selected channels will be returned by the
  // access functions. The default spectralWindow is the current one (or 0)
  // This allows selection of the input channels, producing
  // nGroup groups of width output channels. Default is to return all channels
  // in a single group.
  virtual ROVisibilityIterator& selectChannel(Int nGroup=1, Int start=0, Int width=0, 
				      Int increment=1, Int spectralWindow=-1);

  //Same as above except when multiple ms's are to be accessed

  virtual ROVisibilityIterator& selectChannel(Block< Vector<Int> >& blockNGroup,
				      Block< Vector<Int> >& blockStart,
				      Block< Vector<Int> >& blockWidth,
				      Block< Vector<Int> >& blockIncr,
				      Block< Vector<Int> >& blockSpw);


  //get the channel selection ...the block over the number of ms's associated
  // with this iterator
  virtual void getChannelSelection(Block< Vector<Int> >& blockNGroup,
			   Block< Vector<Int> >& blockStart,
			   Block< Vector<Int> >& blockWidth,
			   Block< Vector<Int> >& blockIncr,
			   Block< Vector<Int> >& blockSpw);

  // Get the spw, start  and nchan for all the ms's is this Visiter that 
  // match the frequecy "freqstart-freqStep" and "freqEnd+freqStep" range
  // Can help in doing channel selection above..
  // freqFrame is the frame the caller frequency values are in (freqStart and freqEnd)
  // These will be converted to the frame of the selected spw to match

  virtual void getSpwInFreqRange(Block<Vector<Int> >& spw, 
				 Block<Vector<Int> >& start, 
				 Block<Vector<Int> >& nchan, 
				 Double freqStart, Double freqEnd, 
				 Double freqStep, MFrequency::Types freqFrame=MFrequency::LSRK);

  // Attach a VisBuffer object.
  // Note that while more than one VisBuffer may be attached, only the
  // last one is actively updated. A Stack is kept internally, so after 
  // a detach, the previous VisBuffer becomes active again.
  virtual void attachVisBuffer(VisBuffer& vb);

  // Detach a VisBuffer object.
  // If the object detached is not the last one attached an exception
  // is thrown.
  virtual void detachVisBuffer(VisBuffer& vb);

  // Access the current ROMSColumns object in MSIter
  virtual const ROMSColumns& msColumns() const
    { return msIter_p.msColumns();}

  // get back the selected spectral windows and spectral channels for
  // current ms 

  virtual void allSelectedSpectralWindows(Vector<Int>& spws, Vector<Int>& nvischan);

  // Convert the frequency from the observe frame to lsr frame.
  // Returns True in convert if given spw was not observed 
  // in the LSRK frame
  virtual void lsrFrequency(const Int& spw, Vector<Double>& freq, Bool& convert);
  //assign a VisImagingWeight object to this iterator
  virtual void useImagingWeight(const VisImagingWeight& imWgt);
  //return number  of Ant 
  virtual Int numberAnt();
  //Return number of rows in all selected ms's
  virtual Int numberCoh();

  // Return number of spws, polids, ddids
  virtual Int numberSpw();
  virtual Int numberPol();
  virtual Int numberDDId();

  ROArrayColumn <Double> & getChannelFrequency () const;
  Block<Int> getChannelGroupNumber () const;
  Block<Int> getChannelIncrement () const;
  Block<Int> getChannelStart () const;
  Block<Int> getChannelWidth () const;
  Int getDataDescriptionId () const;
  const MeasurementSet & getMeasurementSet() const;;
  Int getMeasurementSetId() const;
  Int getNAntennas () const;
  MEpoch getMEpoch () const;
  MFrequency::Types getObservatoryFrequencyType () const; //???
  MPosition getObservatoryPosition () const;
  MDirection getPhaseCenter () const;
  Vector<Float> getReceptor0Angle ();
  Vector<uInt> getRowIds () const;

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



protected:
  // advance the iteration
  virtual void advance();
  // set the currently selected table
  virtual void setSelTable();
  // set the iteration state
  virtual void setState();
  // get the TOPO frequencies from the selected velocities and the obs. vel.
  virtual void getTopoFreqs();
  virtual void getTopoFreqs(Vector<Double> & lsrFreq, Vector<Double> & selFreq); // for async i/o

  virtual void getLsrInfo (Block<Int> & channelGroupNumber,
                           Block<Int> & channelIncrement,
                           Block<Int> & channelStart,
                           Block<Int> & channelWidth,
                           MPosition & observatoryPositon,
                           MDirection & phaseCenter,
                           Bool & velocitySelection) const;

  vector<const MeasurementSet *> getMeasurementSets () const;

  const MSDerivedValues & getMSD () const; // for use by Async I/O *ONLY*

  // update the DATA slicer
  virtual void updateSlicer();
  // attach the column objects to the currently selected table
  virtual void attachColumns(const Table &t);
  // returns the table, to which columns are attached, 
  // can be overridden in derived classes
  virtual const Table attachTable() const;
  // get the (velocity selected) interpolated visibilities, flags and weights
  virtual void getInterpolatedVisFlagWeight(DataColumn whichOne) const;

  // get the (velocity selected) interpolated FLOAT_DATA (as real Floats),
  // flags and weights.
  void getInterpolatedFloatDataFlagWeight() const;

  // get the visibility data (observed, corrected or model);
  // deals with Float and Complex observed data (DATA or FLOAT_DATA)
  virtual void getDataColumn(DataColumn whichOne, const Slicer& slicer, 
			     Cube<Complex>& data) const;
  virtual void getDataColumn(DataColumn whichOne, Cube<Complex>& data) const;

  // get FLOAT_DATA as real Floats.
  virtual void getFloatDataColumn(const Slicer& slicer, Cube<Float>& data) const;
  virtual void getFloatDataColumn(Cube<Float>& data) const;

  //constructor helpers
  virtual void initsinglems(const MeasurementSet & mss);
  virtual void initmultims(const Block<MeasurementSet>& mss);

  virtual void originChunks(Bool forceRewind);

  //Re-Do the channel selection in multi ms case 
  virtual void doChannelSelection();
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
  virtual Bool isInSelectedSPW(const Int& spw);

  // Updates, if necessary, rowIds_p member for the current chunk
  virtual void update_rowIds() const;

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

  //  virtual void getCol(const String &colName, Array<Double> &array,
  //                      Array<Double> &all, Bool resize = False) const;
  //  virtual void getCol(const String &colName, Vector<Bool> &array,
  //                      Vector<Bool> &all, Bool resize = False) const;
  //  virtual void getCol(const String &colName, Vector<Int> &array,
  //                      Vector<Int> &all, Bool resize = False) const;
  //  virtual void getCol(const String &colName, Vector<Double> &array,
  //                      Vector<Double> &all, Bool resize = False) const;

  template<class T>
  void swapyz(Cube<T>& out, const Cube<T>& in) const;

  ROVisibilityIterator* This;
  MSIter msIter_p;
  RefRows selRows_p; // currently selected rows from msIter_p.table()
  Int curChanGroup_p, curNumChanGroup_p, channelGroupSize_p, 
      curTableNumRow_p, curStartRow_p, curEndRow_p,
      nChan_p, nPol_p, nRowBlocking_p;
  uInt curNumRow_p;
  Bool more_p, newChanGroup_p, initialized_p, msIterAtOrigin_p, stateOk_p;

  // channel selection
  Block<Int> numChanGroup_p, chanStart_p, chanWidth_p, chanInc_p,
    preselectedChanStart_p,preselectednChan_p;
  
  Bool isMultiMS_p;
  Block< Vector<Int> >blockNumChanGroup_p, blockChanStart_p;
  Block< Vector<Int> > blockChanWidth_p, blockChanInc_p;
  Block<Vector<Int> > blockSpw_p;
  Int msCounter_p;
  // Stack of VisBuffer objects
  Stack<VisBuffer *> vbStack_p;
  vector<const MeasurementSet *> measurementSets_p; // [use]

  //cache for access functions
  Slicer slicer_p;
  Slicer weightSlicer_p;
  Bool useSlicer_p;
  Vector<Double> time_p;
  Vector<Double> timeInterval_p;
  Vector<Double> frequency_p;
  Bool freqCacheOK_p, flagOK_p, weightSpOK_p;
  Block<Bool> visOK_p;
  Bool floatDataCubeOK_p;
  Cube<Bool> flagCube_p;
  Cube<Complex> visCube_p;
  Cube<Float> floatDataCube_p;
  mutable Matrix<Double> uvwMat_p;
  Matrix<Float> imagingWeight_p;
  Vector<Float> feedpa_p;

  Float parang0_p;
  Vector<Float> parang_p;
  MDirection azel0_p;
  Vector<MDirection> azel_p;
  Double hourang_p;

  Bool floatDataFound_p;

  // Does the current MS have a valid WEIGHT_SPECTRUM?
  Bool msHasWtSp_p;     // make mutable so existsWeightSpectrum() can be const?

  // for PA/AZEL calculations
  MSDerivedValues msd_p;
  Double lastazelUT_p;
  Double lastfeedpaUT_p;
  Double lastParangUT_p;
  Double lastParang0UT_p;
  Int nAnt_p;

  // for velocity selection and conversion
  Bool velSelection_p, vPrecise_p;
  Int nVelChan_p;
  MVRadialVelocity vStart_p;
  MVRadialVelocity vInc_p;
  MDoppler::Convert cFromBETA_p;
  MDoppler::Types vDef_p;
  Vector<Double> selFreq_p;
  Vector<Double> lsrFreq_p;
  String vInterpolation_p;

  mutable Vector<uInt> rowIds_p;

  ROScalarColumn<Int> colAntenna1, colAntenna2;
  ROScalarColumn<Int> colFeed1, colFeed2;
  ROScalarColumn<Double> colTime;
  ROScalarColumn<Double> colTimeCentroid;
  ROScalarColumn<Double> colTimeInterval;
  ROScalarColumn<Double> colExposure;
  ROArrayColumn<Float> colWeight;
  ROArrayColumn<Float> colWeightSpectrum;
  ROArrayColumn<Complex> colVis;
  ROArrayColumn<Float> colFloatVis;
  ROArrayColumn<Complex> colModelVis;
  ROArrayColumn<Complex> colCorrVis;
  ROArrayColumn<Float> colSigma;
  ROArrayColumn<Bool> colFlag;
  ROArrayColumn<Bool> colFlagCategory;
  ROScalarColumn<Bool> colFlagRow;
  ROScalarColumn<Int> colObservation;
  ROScalarColumn<Int> colProcessor;
  ROScalarColumn<Int> colScan;
  ROScalarColumn<Int> colState;
  ROArrayColumn<Double> colUVW;

  //object to calculate imaging weight
  VisImagingWeight imwgt_p;

  Bool asyncEnabled_p; // Allows lower-level code to make an async "copy" of this VI.

  Vector<Bool> tileCacheIsSet_p;

};

inline Bool ROVisibilityIterator::more() const { return more_p;}

inline Vector<SquareMatrix<Complex,2> >& 
ROVisibilityIterator::CJones(Vector<SquareMatrix<Complex,2> >& cjones) const 
{cjones.resize(msIter_p.CJones().nelements());return cjones=msIter_p.CJones();}

inline const Cube<Double>& ROVisibilityIterator::receptorAngles() const
{return msIter_p.receptorAngles();}

inline const Vector<String>& ROVisibilityIterator::antennaMounts() const
{return msIter_p.antennaMounts();}

inline const Cube<RigidVector<Double, 2> >& 
       ROVisibilityIterator::getBeamOffsets() const
{return msIter_p.getBeamOffsets();}

inline Bool ROVisibilityIterator::allBeamOffsetsZero() const
{return msIter_p.allBeamOffsetsZero();}

inline Int ROVisibilityIterator::channelGroupSize() const
{ return velSelection_p ? nVelChan_p : chanWidth_p[msIter_p.spectralWindowId()]; }
inline Int ROVisibilityIterator::channelIndex() const
{ return chanInc_p[msIter_p.spectralWindowId()]*curChanGroup_p; }
inline Int ROVisibilityIterator::nRow() const
{ return curNumRow_p;}
inline Int ROVisibilityIterator::nRowChunk() const
{ return msIter_p.table().nrow();}                                            
inline ROVisibilityIterator& 
ROVisibilityIterator::velInterpolation(const String& type)
{ vInterpolation_p=type; return *this;}
inline IPosition ROVisibilityIterator::visibilityShape() const
{return IPosition(3,nPol_p,channelGroupSize(),curNumRow_p);}

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
  VisibilityIterator(Block<MeasurementSet>& mss,
		       const Block<Int>& sortColumns, 
		       Double timeInterval=0);
  VisibilityIterator(Block<MeasurementSet>& mss,
		     const Block<Int>& sortColumns, const Bool addDefaultSortCols, 
		       Double timeInterval=0);

  VisibilityIterator(const VisibilityIterator & MSI);

  VisibilityIterator & operator=(const VisibilityIterator &MSI);

  // Destructor

  virtual ~VisibilityIterator();
  
  // Members
  
  virtual Bool isWritable () const { return True;}

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
  void setFlag(const Cube<Bool>& flag);

  // Set/modify the flag row column; dimension Vector(nrow)
  void setFlagRow(const Vector<Bool>& rowflags);

  // Set/modify the visibilities.
  // This is possibly only for a 'reference' MS which has a new DATA column.
  // The first axis of the matrix should equal the selected number of channels
  // in the original MS.
  // If the MS does not contain all polarizations, only the parallel
  // hand polarizations are used.
  void setVis(const Matrix<CStokesVector>& vis, DataColumn whichOne);

  // Set/modify the visibilities
  // This sets the data as found in the MS, Cube(npol,nchan,nrow).
  void setVis(const Cube<Complex>& vis, DataColumn whichOne);

  // Set the visibility and flags, and interpolate from velocities if needed
  void setVisAndFlag(const Cube<Complex>& vis, const Cube<Bool>& flag,
		     DataColumn whichOne);

  // Set/modify the weights
  void setWeight(const Vector<Float>& wt);

  // Set/modify the weightMat
  void setWeightMat(const Matrix<Float>& wtmat);

  // Set/modify the weightSpectrum
  void setWeightSpectrum(const Cube<Float>& wtsp);

  // Set/modify the Sigma
  void setSigma(const Vector<Float>& sig);

  // Set/modify the ncorr x nrow SigmaMat.
  void setSigmaMat(const Matrix<Float>& sigmat);

  // jagonzal: Set MS access mutex
  void setupAsyncIO();
  void setMutex(casa::async::Mutex *msAccessMutex);
  bool getMutexSet() { return msAccessMutexSet_p;}

protected:
  virtual void attachColumns(const Table &t);
  void setInterpolatedVisFlag(const Cube<Complex>& vis, 
			      const Cube<Bool>& flag);
  void setInterpolatedWeight(const Matrix<Float>& wt); 
  // Write the data column (observed, model or corrected);
  // deals with Float or Complex observed data (DATA and FLOAT_DATA).
  void putDataColumn(DataColumn whichOne, const Slicer& slicer, 
		     const Cube<Complex>& data);
  void putDataColumn(DataColumn whichOne, const Cube<Complex>& data);

  // column access functions, can be overridden in derived classes
  virtual void putCol(ScalarColumn<Bool> &column, const Vector<Bool> &array);
  virtual void putCol(ArrayColumn<Bool> &column, const Array<Bool> &array);
  virtual void putCol(ArrayColumn<Float> &column, const Array<Float> &array);
  virtual void putCol(ArrayColumn<Complex> &column, const Array<Complex> &array);

  virtual void putCol(ArrayColumn<Bool> &column, const Slicer &slicer, const Array<Bool> &array);
  virtual void putCol(ArrayColumn<Float> &column, const Slicer &slicer, const Array<Float> &array);
  virtual void putCol(ArrayColumn<Complex> &column, const Slicer &slicer, const Array<Complex> &array);

  // non-virtual, no reason to template this function because Bool is the only type needed
  void putColScalar(ScalarColumn<Bool> &column, const Vector<Bool> &array);

  ArrayColumn<Complex> RWcolVis;
  ArrayColumn<Float> RWcolFloatVis;
  ArrayColumn<Complex> RWcolModelVis;
  ArrayColumn<Complex> RWcolCorrVis;
  ArrayColumn<Float> RWcolWeight;
  ArrayColumn<Float> RWcolWeightSpectrum;
  ArrayColumn<Float> RWcolSigma;
  ArrayColumn<Bool> RWcolFlag;
  ScalarColumn<Bool> RWcolFlagRow;

  // jagonzal: MS access mutex
  casa::async::Mutex * msAccessMutex_p;
  bool msAccessMutexSet_p;

};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <msvis/MSVis/VisibilityIterator.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif


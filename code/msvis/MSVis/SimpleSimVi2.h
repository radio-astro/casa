//# SimpleSimVi2.h: Rudimentary data simulator
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

#if ! defined (MSVIS_SimplSimVi2_H)
#define MSVIS_SimplSimVi2_H

#include <casa/aips.h>
#include <casa/BasicSL.h>
#include <msvis/MSVis/VisBufferComponents2.h>
#include <msvis/MSVis/ViImplementation2.h>
#include <msvis/MSVis/ViiLayerFactory.h>
#include <msvis/MSVis/VisBufferImpl2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/Stokes.h>
#include <casa/Arrays.h>

#include <map>
#include <vector>

#define SSVi2NotPossible() ThrowCc("Not Possible in SimpleSimVi2"); 
#define SSVi2NotYetImplemented() ThrowCc("Not yet implemented in SimpleSimVi2");

namespace casa { //# NAMESPACE CASA - BEGIN

class RecordInterface;
class Slice;
class String;
template <typename T, Int n> class SquareMatrix;
template <typename T> class Vector;
class VisImagingWeight;

namespace vi {


//# forward decl

class Subchunk;
class VisBuffer2;


class SimpleSimVi2Parameters {

public:

  // Default ctor (sets default values, useful for self-testing)
  SimpleSimVi2Parameters();

  SimpleSimVi2Parameters(Int nField,Int nScan,Int nSpw,Int nAnt,Int nCorr,
			 const Vector<Int>& nTimePerField, const Vector<Int>& nChan,
			 String date0, Double dt, 
			 const Vector<Double>& refFreq, const Vector<Double>& df,
			 const Matrix<Float>& stokes, 
			 Bool doNoise,
			 const Matrix<Float>& gain, const Matrix<Float>& tsys, 
			 Bool doNorm=True,
			 String polBasis="circ", Bool doAC=False);
  
  SimpleSimVi2Parameters(const SimpleSimVi2Parameters& other);
  SimpleSimVi2Parameters& operator=(const SimpleSimVi2Parameters& other);

  ~SimpleSimVi2Parameters();


  void summary() const;

  Int nField_, nScan_, nSpw_, nAnt_, nCorr_;
  Vector<Int> nTimePerField_, nChan_;
  String date0_; 
  Double dt_;
  Vector<Double> refFreq_, df_;
  Bool doNoise_;
  Matrix<Float> stokes_, gain_, tsys_;
  Bool doNorm_;
  String polBasis_;
  Bool doAC_;

private:
  void initialize(const Vector<Int>& nTimePerField, const Vector<Int>& nChan,
		  const Vector<Double>& refFreq, const Vector<Double>& df,
		  const Matrix<Float>& stokes, 
		  const Matrix<Float>& gain, const Matrix<Float>& tsys);

};


// We must override modelDataIsVirtual() to avoid dep on a real MS
//   This is otherwise an ordinary VB2
class SimpleSimVB2 : public VisBufferImpl2 {

public:
  static VisBuffer2* factory(ViImplementation2 * vi, VisBufferOptions options);

  // Never virtual
  Bool modelDataIsVirtual() const { return False; };

protected:

  SimpleSimVB2(ViImplementation2 * vii,VisBufferOptions options);

};


class SimpleSimVi2 : public ViImplementation2 {

public:

  SimpleSimVi2 (const SimpleSimVi2Parameters& pars);

  // make noncopyable...
  SimpleSimVi2( const SimpleSimVi2& ) = delete;
  SimpleSimVi2& operator=( const SimpleSimVi2& ) = delete;

  // Destructor
  virtual ~SimpleSimVi2 ();

  //   +==================================+
  //   |                                  |
  //   | Iteration Control and Monitoring |
  //   |                                  |
  //   +==================================+
  

  // Methods to control and monitor subchunk iteration

  virtual void origin ();
  virtual Bool more () const;
  virtual void next ();
  virtual Subchunk getSubchunkId () const;
  
  // Methods to control chunk iterator

  virtual void originChunks (Bool forceRewind = False);
  virtual Bool moreChunks () const;
  virtual void nextChunk ();
  
  virtual Bool isWritable () const { return False; };

  // Return the time interval (in seconds) used for iteration.
  // This is not the same as the INTERVAL column.  Setting the
  // the interval requires calling origin chunks before performing
  // further iterator.
  
  virtual Double getInterval() const {return 1.0e9; };
  virtual void setInterval (Double) { SSVi2NotPossible() };

  // Select the channels to be returned.  Requires calling originChunks before
  // performing additional iteration.
  
  virtual void setFrequencySelections (const FrequencySelections & selection);

  // Set the 'blocking' size for returning data.
  virtual void setRowBlocking (Int) { SSVi2NotPossible() };
  
  virtual Bool existsColumn (VisBufferComponent2) const { SSVi2NotPossible() };
  
  virtual const SortColumns & getSortColumns() const { SSVi2NotPossible() };

  virtual Bool isNewArrayId () const { return False; };
  virtual Bool isNewFieldId () const { return thisField_!=lastField_; };
  virtual Bool isNewMs () const  { return False; };
  virtual Bool isNewSpectralWindow () const { return thisSpw_!=lastSpw_; };

  // Return the number of rows in the current iteration

  virtual Int nRows () const { return nBsln_; };
  
  // Return the row ids as from the original root table. This is useful
  // to find correspondance between a given row in this iteration to the
  // original ms row

  virtual void getRowIds (Vector<uInt> & rowids) const;

  virtual VisBuffer2 * getVisBuffer (const VisibilityIterator2 *) { SSVi2NotPossible() };
  virtual VisBuffer2 * getVisBuffer ();


  //   +=========================+
  //   |                         |
  //   | Subchunk Data Accessors |
  //   |                         |
  //   +=========================+
  
  // Return info
  virtual void antenna1 (Vector<Int> & ant1) const;
  virtual void antenna2 (Vector<Int> & ant2) const;
  virtual void corrType (Vector<Int> & corrTypes) const;
  virtual Int  dataDescriptionId () const;
  virtual void dataDescriptionIds (Vector<Int> & ddis) const;
  virtual void exposure (Vector<Double> & expo) const;
  virtual void feed1 (Vector<Int> & fd1) const;
  virtual void feed2 (Vector<Int> & fd2) const;
  virtual void fieldIds (Vector<Int>&) const;
  virtual void arrayIds (Vector<Int>&) const;
  virtual String fieldName () const;

  virtual void flag (Cube<Bool> & flags) const;
  virtual void flag (Matrix<Bool> &) const { SSVi2NotPossible() };
  virtual Bool flagCategoryExists () const { return False; };
  virtual void flagCategory (Array<Bool> &) const { SSVi2NotPossible() };
  virtual void flagRow (Vector<Bool> & rowflags) const;
  virtual void observationId (Vector<Int> & obsids) const;
  virtual Int polarizationId () const;
  virtual void processorId (Vector<Int> & procids) const;
  virtual void scan (Vector<Int> & scans) const;
  virtual String sourceName () const;
  virtual void stateId (Vector<Int> & stateids) const;
  virtual void jonesC (Vector<SquareMatrix<Complex, 2> > &) const { SSVi2NotPossible() };
  virtual Int polFrame () const;
  virtual void sigma (Matrix<Float> & sigmat) const;
  virtual Int spectralWindow () const;
  virtual void spectralWindows (Vector<Int> & spws) const;
  virtual void time (Vector<Double> & t) const;
  virtual void timeCentroid (Vector<Double> & t) const;
  virtual void timeInterval (Vector<Double> & ti) const;
  virtual void uvw (Matrix<Double> & uvwmat) const;

  virtual void visibilityCorrected (Cube<Complex> & vis) const;
  virtual void visibilityModel (Cube<Complex> & vis) const;
  virtual void visibilityObserved (Cube<Complex> & vis) const;
  virtual void floatData (Cube<Float> & fcube) const;

  virtual IPosition visibilityShape () const;

  virtual void weight (Matrix<Float> & wtmat) const;
  virtual Bool weightSpectrumExists () const;
  virtual Bool sigmaSpectrumExists () const;
  virtual void weightSpectrum (Cube<Float> & wtsp) const;
  virtual void sigmaSpectrum (Cube<Float> & wtsp) const;

  virtual void setWeightScaling (CountedPtr<WeightScaling>) { SSVi2NotPossible() };
  virtual Bool hasWeightScaling () const { return False; };
  virtual CountedPtr<WeightScaling> getWeightScaling () const; //  { SSVi2NotPossible() };

  //   +------------------------+
  //   |                        |
  //   | Angular Data Providers |
  //   |                        |
  //   +------------------------+

  // No underlying geometry is available for these!
  virtual Bool allBeamOffsetsZero () const { SSVi2NotPossible() };
  virtual MDirection azel0 (Double) const { SSVi2NotPossible() };
  virtual const Vector<MDirection> & azel (Double) const { SSVi2NotPossible() };
  virtual const Vector<Float> & feed_pa (Double) const { SSVi2NotPossible() };
  virtual const Cube<RigidVector<Double, 2> > & getBeamOffsets () const { SSVi2NotPossible() };
  virtual Double hourang (Double) const { SSVi2NotPossible() };
  virtual const Float & parang0 (Double) const { SSVi2NotPossible() };
  virtual const Vector<Float> & parang (Double) const { SSVi2NotPossible() };
  virtual const MDirection & phaseCenter () const { SSVi2NotPossible() };  // needs POINTING subtable!
  virtual const Cube<Double> & receptorAngles () const { SSVi2NotPossible() };

  //   +=========================+
  //   |                         |
  //   | Chunk and MS Level Data |
  //   |                         |
  //   +=========================+

  virtual const Vector<String> & antennaMounts () const { SSVi2NotPossible() };
  virtual const VisImagingWeight & getImagingWeightGenerator () const { SSVi2NotPossible() };
  
  virtual MFrequency::Types getObservatoryFrequencyType () const { SSVi2NotPossible() };
  virtual MPosition getObservatoryPosition () const { SSVi2NotPossible() };
  virtual Vector<Float> getReceptor0Angle () { SSVi2NotPossible() };
 
  virtual Int getReportingFrameOfReference () const { return -2;  /*SSVi2NotPossible()*/ };
  virtual void setReportingFrameOfReference (Int) { SSVi2NotPossible() };
 
  virtual MEpoch getEpoch () const;

  virtual Vector<Int> getCorrelations () const;

  virtual Vector<Stokes::StokesTypes> getCorrelationTypesDefined () const { return corrdef_; /*SSVi2NotPossible()*/ };
  virtual Vector<Stokes::StokesTypes> getCorrelationTypesSelected () const { return corrdef_; /*SSVi2NotPossible()*/ };

  virtual Vector<Int> getChannels (Double time, Int frameOfReference, Int spectralWndow = -1,
				   Int msId = -1) const;
  virtual Vector<Double> getFrequencies (Double time, Int frameOfReference, Int spectralWndow = -1,
					 Int msId = -1) const;

  //reference to actual ms in interator  (TRIVIAL returns
  virtual Int msId () const { return -1; }; // zero-based index of current MS in set of MSs
  virtual const MeasurementSet & ms () const { SSVi2NotPossible() };
  virtual Int getNMs () const { return 0; };

  // Call to use the slurp i/o method for all scalar columns. 
  //  Not meaningful for non-I/O
  virtual void slurp () const { SSVi2NotPossible() };

  // Access the current ROMSColumns object in MSIter -- NOT POSSIBLE
  virtual const vi::SubtableColumns & subtableColumns () const { SSVi2NotPossible() };


  // get back the selected spectral windows and spectral channels for
  // current ms

  virtual const SpectralWindowChannels & getSpectralWindowChannels (Int msId, Int spectralWindowId) const;

  //assign a VisImagingWeight object to this iterator
  virtual void useImagingWeight (const VisImagingWeight & ) { SSVi2NotPossible() };
  
  // Return number of antennasm spws, polids, ddids
  
  virtual Int nAntennas () const;
  virtual Int nDataDescriptionIds () const;
  virtual Int nPolarizationIds () const;
  virtual Int nRowsInChunk () const; // number rows in current chunk
  virtual Int nRowsViWillSweep () const; // number of rows in all selected ms's
  virtual Int nSpectralWindows () const;

  // Writeback methods are Irrelevant for non-disk-reading VI2
  virtual void writeBackChanges (VisBuffer2* ) { SSVi2NotPossible() };
  virtual void writeFlag (const Cube<Bool> &) { SSVi2NotPossible() };
  virtual void writeFlagRow (const Vector<Bool> &) { SSVi2NotPossible() };
  virtual void writeFlagCategory(const Array<Bool>&) { SSVi2NotPossible() };
  virtual void writeVisCorrected (const Cube<Complex> &) { SSVi2NotPossible() };
  virtual void writeVisModel (const Cube<Complex> &) { SSVi2NotPossible() };
  virtual void writeVisObserved (const Cube<Complex> &) { SSVi2NotPossible() };
  virtual void writeWeight (const Matrix<Float> &) { SSVi2NotPossible() };
  virtual void writeWeightSpectrum (const Cube<Float> &) { SSVi2NotPossible() };
  virtual void initWeightSpectrum (const Cube<Float> &) { SSVi2NotPossible() };
  virtual void writeSigmaSpectrum (const Cube<Float> &) { SSVi2NotPossible() };
  virtual void writeSigma (const Matrix<Float> &) { SSVi2NotPossible() };
  virtual void writeModel(const RecordInterface&,Bool,Bool) { SSVi2NotPossible() };

protected:

  VisBuffer2 * createAttachedVisBuffer (VisBufferOptions options);

private:

  // Make default ctor invisible
  SimpleSimVi2 ();

  // Keep VB2 sync'd
  void configureNewSubchunk();

  // Generate noise on data
  void addNoise(Cube<Complex>& vis) const;

  // Input parameters
  const SimpleSimVi2Parameters pars_;
  /*
  Int nField_,nScan_,nSpw_,nAnt_,nCorr_;
  Vector<Int> nTimePerField_, nChan_;
  String date0_;
  Double dt_;
  Vector<Double> refFreq_, df_;
  Matrix<Float> stokes_, sefd_;
  String polBasis_;
  Bool doAC_;
  */

  // Derived parameters
  Int  nChunk_, nBsln_;
  Double t0_;
  Vector<Float> wt0_;
  Matrix<Complex> vis0_;

  // Counters
  Int iChunk_,iSubChunk_,iRow0_;
  Int iScan_;
  Double iChunkTime0_;

  // Meta-info
  Int thisScan_, thisField_, thisSpw_;
  Int lastScan_, lastField_, lastSpw_;
  Double thisTime_;

  // Correlation stuff
  Vector<Stokes::StokesTypes> corrdef_;

  // The associated VB
  VisBuffer2* vb_;

};



class SimpleSimVi2Factory : public ViFactory {

public:

  SimpleSimVi2Factory(const SimpleSimVi2Parameters& pars);
  
  ~SimpleSimVi2Factory ();

protected:

  virtual ViImplementation2 * createVi () const;

private:

  const SimpleSimVi2Parameters& pars_;

};


class SimpleSimVi2LayerFactory : public ViiLayerFactory {

public:

  SimpleSimVi2LayerFactory(const SimpleSimVi2Parameters& pars);

  virtual ~SimpleSimVi2LayerFactory () {}

protected:

  // SimpleSimVi2-specific layer-creater
  //   
  virtual ViImplementation2 * createInstance (ViImplementation2* vii0) const;

private:

  // Store a reference to the parameters
  const SimpleSimVi2Parameters& pars_;


};






} // end namespace vi

} //# NAMESPACE CASA - END

#endif // ! defined (MSVIS_SimpleSimVi2_H)



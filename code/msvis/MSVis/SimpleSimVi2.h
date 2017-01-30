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
#include <casa/Exceptions/Error.h>
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

namespace casacore{

class AipsError;
class RecordInterface;
class Slice;
class String;
template <typename T, Int n> class SquareMatrix;
template <typename T> class Vector;
}

namespace casa { //# NAMESPACE CASA - BEGIN

class VisImagingWeight;

namespace vi {


//# forward decl

class Subchunk;
class VisBuffer2;


class SimpleSimVi2Parameters {

public:

  // Default ctor (sets default values, useful for self-testing)
  SimpleSimVi2Parameters();

  // Simple, shape-oriented ctor
  SimpleSimVi2Parameters(casacore::Int nField,casacore::Int nScan, casacore::Int nSpw, casacore::Int nAnt, casacore::Int nCorr,
			 const casacore::Vector<casacore::Int>& nTimePerField, const casacore::Vector<casacore::Int>& nChan,
			 casacore::Complex c0=casacore::Complex(0.0f),
			 casacore::String polBasis="circ",
			 casacore::Bool autoPol=false,casacore::Bool doParang=false);

  // Full control
  SimpleSimVi2Parameters(casacore::Int nField,casacore::Int nScan,casacore::Int nSpw,casacore::Int nAnt,casacore::Int nCorr,
			 const casacore::Vector<casacore::Int>& nTimePerField, const casacore::Vector<casacore::Int>& nChan,
			 casacore::String date0, casacore::Double dt, 
			 const casacore::Vector<casacore::Double>& refFreq, const casacore::Vector<casacore::Double>& df,
			 const casacore::Matrix<casacore::Float>& stokes, 
			 casacore::Bool doNoise,
			 const casacore::Matrix<casacore::Float>& gain, const casacore::Matrix<casacore::Float>& tsys, 
			 casacore::Bool doNorm=true,
			 casacore::String polBasis="circ", casacore::Bool doAC=false,
			 casacore::Complex c0 = casacore::Complex(0.0f),
			 casacore::Bool doParang=false);
  
  SimpleSimVi2Parameters(const SimpleSimVi2Parameters& other);
  SimpleSimVi2Parameters& operator=(const SimpleSimVi2Parameters& other);

  ~SimpleSimVi2Parameters();


  void summary() const;

  casacore::Int nField_, nScan_, nSpw_, nAnt_, nCorr_;
  casacore::Vector<casacore::Int> nTimePerField_, nChan_;
  casacore::String date0_; 
  casacore::Double dt_;
  casacore::Vector<casacore::Double> refFreq_, df_;
  casacore::Bool doNoise_;
  casacore::Matrix<casacore::Float> stokes_, gain_, tsys_;
  casacore::Bool doNorm_;
  casacore::String polBasis_;
  casacore::Bool doAC_;
  casacore::Complex c0_;
  casacore::Bool autoPol_;   // set non-trivial linear polarization 
  casacore::Bool doParang_;  // Simple linear-in-time, for now

  // Return frequencies for specified spw
  casacore::Vector<casacore::Double> freqs(casacore::Int spw) const;

private:
  void initialize(const casacore::Vector<casacore::Int>& nTimePerField, const casacore::Vector<casacore::Int>& nChan,
		  const casacore::Vector<casacore::Double>& refFreq, const casacore::Vector<casacore::Double>& df,
		  const casacore::Matrix<casacore::Float>& stokes, 
		  const casacore::Matrix<casacore::Float>& gain, const casacore::Matrix<casacore::Float>& tsys);

};

class SimpleSimVi2 : public ViImplementation2 {

public:

  SimpleSimVi2 (const SimpleSimVi2Parameters& pars);

  // make noncopyable...
  SimpleSimVi2( const SimpleSimVi2& ) = delete;
  SimpleSimVi2& operator=( const SimpleSimVi2& ) = delete;

  // Destructor
  virtual ~SimpleSimVi2 ();

  // Report the the ViImplementation type
  virtual casacore::String ViiType() const { return casacore::String("Simulated(*)"); };

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
  
  virtual casacore::Bool isWritable () const { return false; };

  // Return the time interval (in seconds) used for iteration.
  // This is not the same as the INTERVAL column.  Setting the
  // the interval requires calling origin chunks before performing
  // further iterator.
  
  virtual casacore::Double getInterval() const {return 1.0e9; };
  virtual void setInterval (casacore::Double) { SSVi2NotPossible() };

  // Select the channels to be returned.  Requires calling originChunks before
  // performing additional iteration.
  
  virtual void setFrequencySelections (const FrequencySelections & selection);

  // Set the 'blocking' size for returning data.
  virtual void setRowBlocking (casacore::Int) { SSVi2NotPossible() };
  
  virtual casacore::Bool existsColumn (VisBufferComponent2 id) const;
  
  virtual const SortColumns & getSortColumns() const { SSVi2NotPossible() };

  virtual casacore::Bool isNewArrayId () const { return false; };
  virtual casacore::Bool isNewFieldId () const { return thisField_!=lastField_; };
  virtual casacore::Bool isNewMs () const  { return false; };
  virtual casacore::Bool isNewSpectralWindow () const { return thisSpw_!=lastSpw_; };

  // Return the number of rows in the current iteration

  virtual casacore::Int nRows () const { return nBsln_; };
  
  // Return the row ids as from the original root table. This is useful
  // to find correspondance between a given row in this iteration to the
  // original ms row

  virtual void getRowIds (casacore::Vector<casacore::uInt> & rowids) const;

  virtual VisBuffer2 * getVisBuffer (const VisibilityIterator2 *) { SSVi2NotPossible() };
  virtual VisBuffer2 * getVisBuffer ();


  //   +=========================+
  //   |                         |
  //   | Subchunk casacore::Data Accessors |
  //   |                         |
  //   +=========================+
  
  // Return info
  virtual void antenna1 (casacore::Vector<casacore::Int> & ant1) const;
  virtual void antenna2 (casacore::Vector<casacore::Int> & ant2) const;
  virtual void corrType (casacore::Vector<casacore::Int> & corrTypes) const;
  virtual casacore::Int  dataDescriptionId () const;
  virtual void dataDescriptionIds (casacore::Vector<casacore::Int> & ddis) const;
  virtual void exposure (casacore::Vector<casacore::Double> & expo) const;
  virtual void feed1 (casacore::Vector<casacore::Int> & fd1) const;
  virtual void feed2 (casacore::Vector<casacore::Int> & fd2) const;
  virtual void fieldIds (casacore::Vector<casacore::Int>&) const;
  virtual void arrayIds (casacore::Vector<casacore::Int>&) const;
  virtual casacore::String fieldName () const;

  virtual void flag (casacore::Cube<casacore::Bool> & flags) const;
  virtual void flag (casacore::Matrix<casacore::Bool> &) const { SSVi2NotPossible() };
  virtual casacore::Bool flagCategoryExists () const { return false; };
  virtual void flagCategory (casacore::Array<casacore::Bool> &) const { SSVi2NotPossible() };
  virtual void flagRow (casacore::Vector<casacore::Bool> & rowflags) const;
  virtual void observationId (casacore::Vector<casacore::Int> & obsids) const;
  virtual casacore::Int polarizationId () const;
  virtual void processorId (casacore::Vector<casacore::Int> & procids) const;
  virtual void scan (casacore::Vector<casacore::Int> & scans) const;
  virtual casacore::String sourceName () const;
  virtual void stateId (casacore::Vector<casacore::Int> & stateids) const;
  virtual void jonesC (casacore::Vector<casacore::SquareMatrix<casacore::Complex, 2> > &) const { SSVi2NotPossible() };
  virtual casacore::Int polFrame () const;
  virtual void sigma (casacore::Matrix<casacore::Float> & sigmat) const;
  virtual casacore::Int spectralWindow () const;
  virtual void spectralWindows (casacore::Vector<casacore::Int> & spws) const;
  virtual void time (casacore::Vector<casacore::Double> & t) const;
  virtual void timeCentroid (casacore::Vector<casacore::Double> & t) const;
  virtual void timeInterval (casacore::Vector<casacore::Double> & ti) const;
  virtual void uvw (casacore::Matrix<casacore::Double> & uvwmat) const;

  virtual void visibilityCorrected (casacore::Cube<casacore::Complex> & vis) const;
  virtual void visibilityModel (casacore::Cube<casacore::Complex> & vis) const;
  virtual void visibilityObserved (casacore::Cube<casacore::Complex> & vis) const;
  virtual void floatData (casacore::Cube<casacore::Float> & fcube) const;

  virtual casacore::IPosition visibilityShape () const;

  virtual void weight (casacore::Matrix<casacore::Float> & wtmat) const;
  virtual casacore::Bool weightSpectrumExists () const;
  virtual casacore::Bool sigmaSpectrumExists () const;
  virtual void weightSpectrum (casacore::Cube<casacore::Float> & wtsp) const;
  virtual void sigmaSpectrum (casacore::Cube<casacore::Float> & wtsp) const;

  virtual void setWeightScaling (casacore::CountedPtr<WeightScaling>) { SSVi2NotPossible() };
  virtual casacore::Bool hasWeightScaling () const { return false; };
  virtual casacore::CountedPtr<WeightScaling> getWeightScaling () const; //  { SSVi2NotPossible() };

  //   +------------------------+
  //   |                        |
  //   | Angular casacore::Data Providers |
  //   |                        |
  //   +------------------------+

  // No underlying geometry is available for these!
  virtual casacore::Bool allBeamOffsetsZero () const { SSVi2NotPossible() };
  virtual casacore::MDirection azel0 (casacore::Double) const { SSVi2NotPossible() };
  virtual const casacore::Vector<casacore::MDirection> & azel (casacore::Double) const { SSVi2NotPossible() };
  virtual const casacore::Vector<casacore::Float> & feed_pa (casacore::Double t) const; 
  virtual std::pair<bool, casacore::MDirection> getPointingAngle (int /*antenna*/, double /*time*/) const
  { return std::make_pair (true, phaseCenter()); }
  virtual const casacore::Cube<casacore::RigidVector<casacore::Double, 2> > & getBeamOffsets () const { SSVi2NotPossible() };
  virtual casacore::Double hourang (casacore::Double) const { SSVi2NotPossible() };
  virtual const casacore::Float & parang0 (casacore::Double) const { SSVi2NotPossible() };
  virtual const casacore::Vector<casacore::Float> & parang (casacore::Double) const { SSVi2NotPossible() };
  virtual const casacore::MDirection & phaseCenter () const { return phaseCenter_; }; // trivial value (for now)
  virtual const casacore::Cube<casacore::Double> & receptorAngles () const { SSVi2NotPossible() };

  //   +=========================+
  //   |                         |
  //   | Chunk and casacore::MS Level casacore::Data |
  //   |                         |
  //   +=========================+

  virtual const casacore::Vector<casacore::String> & antennaMounts () const { SSVi2NotPossible() };
  virtual const VisImagingWeight & getImagingWeightGenerator () const { SSVi2NotPossible() };
  
  virtual casacore::MFrequency::Types getObservatoryFrequencyType () const { SSVi2NotPossible() };
  virtual casacore::MPosition getObservatoryPosition () const { SSVi2NotPossible() };
  virtual casacore::Vector<casacore::Float> getReceptor0Angle () { SSVi2NotPossible() };
 
  virtual casacore::Int getReportingFrameOfReference () const { return -2;  /*SSVi2NotPossible()*/ };
  virtual void setReportingFrameOfReference (casacore::Int) { SSVi2NotPossible() };
 
  virtual casacore::MEpoch getEpoch () const;

  virtual casacore::Vector<casacore::Int> getCorrelations () const;

  virtual casacore::Vector<casacore::Stokes::StokesTypes> getCorrelationTypesDefined () const { return corrdef_; /*SSVi2NotPossible()*/ };
  virtual casacore::Vector<casacore::Stokes::StokesTypes> getCorrelationTypesSelected () const { return corrdef_; /*SSVi2NotPossible()*/ };

  virtual casacore::Vector<casacore::Int> getChannels (casacore::Double time, casacore::Int frameOfReference, casacore::Int spectralWndow = -1,
				   casacore::Int msId = -1) const;
  virtual casacore::Vector<casacore::Double> getFrequencies (casacore::Double time, casacore::Int frameOfReference, casacore::Int spectralWndow = -1,
					 casacore::Int msId = -1) const;

  //reference to actual ms in interator  (TRIVIAL returns
  virtual casacore::Int msId () const { return -1; }; // zero-based index of current casacore::MS in set of MSs
  virtual const casacore::MeasurementSet & ms () const { SSVi2NotPossible() };
  virtual casacore::Int getNMs () const { return 0; };

  // Name of the MS in the interator
  virtual casacore::String msName() const { return casacore::String("<noms>"); };

  // Call to use the slurp i/o method for all scalar columns. 
  //  Not meaningful for non-I/O
  virtual void slurp () const { SSVi2NotPossible() };

  // Access the current casacore::ROMSColumns object in MSIter -- NOT POSSIBLE
  virtual const vi::SubtableColumns & subtableColumns () const { SSVi2NotPossible() };


  // get back the selected spectral windows and spectral channels for
  // current ms

  virtual const SpectralWindowChannels & getSpectralWindowChannels (casacore::Int msId, casacore::Int spectralWindowId) const;

  //assign a VisImagingWeight object to this iterator
  virtual void useImagingWeight (const VisImagingWeight & ) { SSVi2NotPossible() };
  
  // Return number of antennasm spws, polids, ddids
  
  virtual casacore::Int nAntennas () const;
  virtual casacore::Int nDataDescriptionIds () const;
  virtual casacore::Int nPolarizationIds () const;
  virtual casacore::Int nRowsInChunk () const; // number rows in current chunk
  virtual casacore::Int nRowsViWillSweep () const; // number of rows in all selected ms's
  virtual casacore::Int nSpectralWindows () const;

  // Writeback methods are Irrelevant for non-disk-reading VI2
  virtual void writeBackChanges (VisBuffer2* ) { SSVi2NotPossible() };
  virtual void writeFlag (const casacore::Cube<casacore::Bool> &) { SSVi2NotPossible() };
  virtual void writeFlagRow (const casacore::Vector<casacore::Bool> &) { SSVi2NotPossible() };
  virtual void writeFlagCategory(const casacore::Array<casacore::Bool>&) { SSVi2NotPossible() };
  virtual void writeVisCorrected (const casacore::Cube<casacore::Complex> &) { SSVi2NotPossible() };
  virtual void writeVisModel (const casacore::Cube<casacore::Complex> &) { SSVi2NotPossible() };
  virtual void writeVisObserved (const casacore::Cube<casacore::Complex> &) { SSVi2NotPossible() };
  virtual void writeWeight (const casacore::Matrix<casacore::Float> &) { SSVi2NotPossible() };
  virtual void writeWeightSpectrum (const casacore::Cube<casacore::Float> &) { SSVi2NotPossible() };
  virtual void initWeightSpectrum (const casacore::Cube<casacore::Float> &) { SSVi2NotPossible() };
  virtual void writeSigmaSpectrum (const casacore::Cube<casacore::Float> &) { SSVi2NotPossible() };
  virtual void writeSigma (const casacore::Matrix<casacore::Float> &) { SSVi2NotPossible() };
  virtual void writeModel(const casacore::RecordInterface&,casacore::Bool,casacore::Bool) { SSVi2NotPossible() };

private:

  // Make default ctor invisible
  SimpleSimVi2 ();

  // Keep VB2 sync'd
  void configureNewSubchunk();

  // Generate noise on data
  void addNoise(casacore::Cube<casacore::Complex>& vis) const;

  // Corrupt by (ad hoc) parang factors
  void corruptByParang(casacore::Cube<casacore::Complex>& vis) const;

  // casacore::Input parameters
  const SimpleSimVi2Parameters pars_;
  /*
  casacore::Int nField_,nScan_,nSpw_,nAnt_,nCorr_;
  casacore::Vector<casacore::Int> nTimePerField_, nChan_;
  casacore::String date0_;
  casacore::Double dt_;
  casacore::Vector<casacore::Double> refFreq_, df_;
  casacore::Matrix<casacore::Float> stokes_, sefd_;
  casacore::String polBasis_;
  casacore::Bool doAC_;
  */

  // Derived parameters
  casacore::Int  nChunk_, nBsln_;
  casacore::Complex c0_;
  casacore::Double t0_;
  casacore::Vector<casacore::Float> wt0_;
  casacore::Matrix<casacore::Complex> vis0_;


  // Counters
  casacore::Int iChunk_,iSubChunk_,iRow0_;
  casacore::Int iScan_;
  casacore::Double iChunkTime0_;

  // Meta-info
  casacore::Int thisScan_, thisField_, thisSpw_;
  casacore::Int lastScan_, lastField_, lastSpw_;
  casacore::Double thisTime_;

  // Correlation stuff
  casacore::Vector<casacore::Stokes::StokesTypes> corrdef_;

  // The associated VB
  VisBuffer2* vb_;

  // Trivial (for now) MDirection, so phaseCenter() has something to return
  casacore::MDirection phaseCenter_;

  // Trivial (for now) parang
  mutable casacore::Vector<casacore::Float> feedpa_;

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

  // Store a copy of the parameters
  const SimpleSimVi2Parameters pars_;


};






} // end namespace vi

} //# NAMESPACE CASA - END

#endif // ! defined (MSVIS_SimpleSimVi2_H)



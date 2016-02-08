//# SimpleSimVi2.cc: Rudimentary data simulator--implementation
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

#include <msvis/MSVis/SimpleSimVi2.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Arrays.h>
#include <casa/BasicMath/Random.h>
#include <casa/Quanta/MVTime.h>

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

SimpleSimVi2Parameters::SimpleSimVi2Parameters() :
  nField_(1),
  nScan_(1),
  nSpw_(1),
  nAnt_(4),
  nCorr_(4),
  nTimePerField_(Vector<Int>(1,1)),
  nChan_(Vector<Int>(1,1)),
  date0_("2016/01/06/00:00:00."),
  dt_(1.0),
  refFreq_(Vector<Double>(1,100.0e9)),
  df_(Vector<Double>(1,1.0e9)),
  doNoise_(False),
  stokes_(Matrix<Float>(1,1,1.0)),
  gain_(Matrix<Float>(1,1,1.0)),
  tsys_(Matrix<Float>(1,1,1.0)),
  doNorm_(False),
  polBasis_("circ"),
  doAC_(False)
{
  
  Vector<Int> nTimePerField(nTimePerField_);
  Vector<Int> nChan(nChan_);
  Vector<Double> refFreq(refFreq_);
  Vector<Double> df(df_);
  Matrix<Float> stokes(stokes_);
  Matrix<Float> gain(gain_);
  Matrix<Float> tsys(tsys_);


  // Generic initialization
  this->initialize(nTimePerField,nChan,refFreq,df,stokes,gain,tsys);

}

SimpleSimVi2Parameters::SimpleSimVi2Parameters(Int nField,Int nScan,Int nSpw,Int nAnt,Int nCorr,
					       const Vector<Int>& nTimePerField,const Vector<Int>& nChan,
					       String date0, Double dt, 
					       const Vector<Double>& refFreq, const Vector<Double>& df,
					       const Matrix<Float>& stokes, 
					       Bool doNoise,
					       const Matrix<Float>& gain, const Matrix<Float>& tsys, 
					       Bool doNorm,
					       String polBasis, Bool doAC) : 
  nField_(nField),
  nScan_(nScan),
  nSpw_(nSpw),
  nAnt_(nAnt),
  nCorr_(nCorr),
  nTimePerField_(),
  nChan_(),
  date0_(date0),
  dt_(dt),
  refFreq_(),
  df_(),
  doNoise_(doNoise),
  stokes_(),
  gain_(),
  tsys_(),
  doNorm_(doNorm),
  polBasis_(polBasis),
  doAC_(doAC)
{

  // Generic initialization
  this->initialize(nTimePerField,nChan,refFreq,df,stokes,gain,tsys);

}

SimpleSimVi2Parameters::SimpleSimVi2Parameters(const SimpleSimVi2Parameters& other) {
  *this=other;
}

SimpleSimVi2Parameters& SimpleSimVi2Parameters::operator=(const SimpleSimVi2Parameters& other) {

  if (this != &other) {
    nField_=other.nField_;
    nScan_=other.nScan_;
    nSpw_=other.nSpw_;
    nAnt_=other.nAnt_;
    nCorr_=other.nCorr_;
    nTimePerField_.assign(other.nTimePerField_);      // NB: Array::assign() forces reshape
    nChan_.assign(other.nChan_);
    date0_=other.date0_;
    dt_=other.dt_;
    refFreq_.assign(other.refFreq_);
    df_.assign(other.df_);
    doNoise_=other.doNoise_;
    stokes_.assign(other.stokes_);
    gain_.assign(other.gain_);
    tsys_.assign(other.tsys_);
    doNorm_=other.doNorm_;
    polBasis_=other.polBasis_;
    doAC_=other.doAC_;
  }
  return *this;

}


SimpleSimVi2Parameters::~SimpleSimVi2Parameters() {}



void SimpleSimVi2Parameters::summary() const {

  cout << endl << "***SimpleSimVi2Parameters Summary******************" << endl;
  cout << boolalpha;
  cout << "*  nField = " << nField_ << endl;
  cout << "*  nScan  = " << nScan_ << endl;
  cout << "*  nSpw   = " << nSpw_ << endl;
  cout << "*  nAnt   = " << nAnt_ << endl;
  cout << "*  nCorr  = " << nCorr_ << endl;

  cout << "*  nTimePerField = " << nTimePerField_ << endl;
  cout << "*  nChan         = " << nChan_ << endl;

  cout << "*  date0 = " << date0_ << endl;
  cout << "*  dt    = " << dt_ << endl;

  cout << "*  refFreq = " << refFreq_ << endl;
  cout << "*  df      = " << df_ << endl;

  cout << "*  stokes = " << stokes_ << endl;

  cout << "*  doNoise = " << doNoise_ << endl;

  cout << "*  gain   = " << gain_ << endl;
  cout << "*  tsys   = " << tsys_ << endl;

  cout << "*  doNorm = " << doNorm_ << endl;

  cout << "*  polBasis = " << polBasis_ << endl;
  cout << "*  doAC     = " << doAC_ << endl;
  cout << "***************************************************" << endl << endl;
}


void SimpleSimVi2Parameters::initialize(const Vector<Int>& nTimePerField,const Vector<Int>& nChan,
					const Vector<Double>& refFreq, const Vector<Double>& df,
					const Matrix<Float>& stokes, 
					const Matrix<Float>& gain, const Matrix<Float>& tsys) {

  nTimePerField_.resize(nField_);  // field-dep scan length
  if (nTimePerField.nelements()==1)
    nTimePerField_.set(nTimePerField(0));
  else
    nTimePerField_=nTimePerField; // will throw if length mismatch

  nChan_.resize(nSpw_);
  if (nChan.nelements()==1)
    nChan_.set(nChan(0));
  else
    nChan_=nChan; // will throw if length mismatch

  refFreq_.resize(nSpw_);
  refFreq_=refFreq;  // will throw if length mismatch

  df_.resize(nSpw_);
  if (df.nelements()==1)
    df_.set(df(0));
  else
    df_=df;  // will throw if length mismatch

  stokes_.resize(4,nField_);
  if (stokes.nelements()==1)
    stokes_(Slice(0),Slice())=stokes(0,0);
  else
    stokes_=stokes; // insist shapes match

  gain_.resize(2,nAnt_);
  if (gain.nelements()==1)
    gain_.set(gain(0,0));
  else
    gain_=gain;  // will throw if shapes mismatch


  tsys_.resize(2,nAnt_);
  if (tsys.nelements()==1)
    tsys_.set(tsys(0,0));
  else
    tsys_=tsys;  // will throw if shapes mismatch

}


SimpleSimVB2::SimpleSimVB2(ViImplementation2 * vii,VisBufferOptions options)
: VisBufferImpl2(vii,options)
{}
  
VisBuffer2* SimpleSimVB2::factory(ViImplementation2 * vi, 
				  VisBufferOptions options) {

  VisBuffer2 * result = NULL;
  result = new SimpleSimVB2 (vi, options);
  return result;

}




SimpleSimVi2::SimpleSimVi2 () {}

SimpleSimVi2::SimpleSimVi2 (const SimpleSimVi2Parameters& pars)

  : ViImplementation2(),
    pars_(pars),
    /*
    nField_(pars.nField_),
    nScan_(pars.nScan_),
    pars_.nSpw_(pars.pars_.nSpw_),
    nAnt_(pars.nAnt_),
    nCorr_(pars.nCorr_),
    nTimePerField_(pars.nTimePerField_),
    nChan_(pars.nChan_),
    date0_(pars.date0_),
    dt_(pars.dt_),
    refFreq_(pars.refFreq_),
    df_(pars.df_),
    stokes_(pars.stokes_),
    sefd_(pars.sefd_),
    polBasis_(pars.polBasis_),
    doAC_(pars.doAC_),
    */

    nChunk_(0),
    nBsln_(0),
    t0_(0.0),
    wt0_(),
    vis0_(),
    iChunk_(0),
    iSubChunk_(0),
    iRow0_(0),
    iScan_(0),
    iChunkTime0_(0),
    thisScan_(1),
    thisField_(0),
    thisSpw_(0),
    lastScan_(-1),
    lastField_(-1),
    lastSpw_(-1),
    thisTime_(0.0),
    corrdef_(4,Stokes::Undefined),
    vb_(0)
{
  // Derived stuff

  nChunk_=pars_.nScan_*pars_.nSpw_;

  nBsln_=pars_.nAnt_*(pars_.nAnt_+ (pars_.doAC_ ? 1 : -1))/2;

  // Time tbd
  //  cout << "Using 2016/01/06/00:00:00.0 as reference time." << endl;
  t0_=4958755200.0;

  // Fundamental weight value is pars_.df_*dt_
  wt0_.resize(pars_.nSpw_);
  if (pars_.doNoise_) {
    convertArray(wt0_,pars_.df_);  // Float <- Double
    wt0_*=Float(pars_.dt_);
  }
  else 
    wt0_.set(1.0);

  // Fundamental vis are just stokes combos (complex)
  vis0_.resize(pars_.nCorr_,pars_.nField_);
  vis0_.set(Complex(0.0));
  for (Int ifld=0;ifld<pars_.nField_;++ifld) {
    if (pars_.polBasis_=="circ") {
      vis0_(0,ifld)=Complex(pars_.stokes_(0,ifld)+pars_.stokes_(3,ifld),0.0);
      if (pars_.nCorr_>1) {
	vis0_(3,ifld)=Complex(pars_.stokes_(0,ifld)-pars_.stokes_(3,ifld),0.0);
	if (pars_.nCorr_>2) {
	  vis0_(1,ifld)=Complex(pars_.stokes_(1,ifld),pars_.stokes_(2,ifld));
	  vis0_(2,ifld)=Complex(pars_.stokes_(1,ifld),-1.0*pars_.stokes_(2,ifld));
	}
      }
    }
  }

  corrdef_(0)=Stokes::type("RR");
  corrdef_(1)=Stokes::type("RL");
  corrdef_(2)=Stokes::type("LR");
  corrdef_(3)=Stokes::type("LL");

  VisBufferOptions vbopt=VbWritable;
  vb_ = createAttachedVisBuffer(vbopt);

}

// Destructor
SimpleSimVi2::~SimpleSimVi2 () {}


  //   +==================================+
  //   |                                  |
  //   | Iteration Control and Monitoring |
  //   |                                  |
  //   +==================================+

  // Methods to control chunk iterator

void SimpleSimVi2::originChunks (Bool)
{
  // Initialize global indices
  iChunk_=0;
  thisField_=0;
  thisSpw_=0;

  // First Scan
  thisScan_=1;

  // Initialize time
  iChunkTime0_=t0_+pars_.dt_/2.;
  thisTime_=iChunkTime0_;

  iRow0_=-nBsln_;

}


Bool SimpleSimVi2::moreChunks () const
{ 
  // if there are more chunks...
  return iChunk_<nChunk_; 
}

void SimpleSimVi2::nextChunk () 
{
  // Remember last chunk's indices
  lastScan_=thisScan_;
  lastField_=thisField_;
  lastSpw_=thisSpw_;

  // Increment chunk counter
  ++iChunk_;

  // New scan each pars_.nSpw_ chunks
  iScan_ = iChunk_/pars_.nSpw_;
  // 1-based
  thisScan_ = 1+ iScan_;

  // Each scan is new field
  thisField_ = iScan_%pars_.nField_;

  // Each chunk is new spw
  thisSpw_ = iChunk_%pars_.nSpw_;

  // Increment chunk time if new scan 
  //  (spws have been exhausted on previous scan)
  if (thisScan_!=lastScan_) iChunkTime0_=thisTime_ + pars_.dt_;

  // Ensure subchunks initialized
  //  this->origin();

}

  // Methods to control and monitor subchunk iteration

void SimpleSimVi2::origin ()
{
  // First subchunk this chunk
  iSubChunk_=0;

  // time is first time of the chunk
  thisTime_=iChunkTime0_;

  // row counter
  iRow0_+=nBsln_;

  // Keep VB sync'd
  this->configureNewSubchunk();

}

Bool SimpleSimVi2::more () const
{
  // True if still more subchunks for this scan's field
  return (iSubChunk_<pars_.nTimePerField_(thisField_));
}

void SimpleSimVi2::next () {
  // Advance counter and time
  ++iSubChunk_;
  thisTime_+=pars_.dt_;

  // Keep VB sync'd
  this->configureNewSubchunk();

}

Subchunk SimpleSimVi2::getSubchunkId () const { return Subchunk(iChunk_,iSubChunk_);}
  
  // Return the time interval (in seconds) used for iteration.
  // This is not the same as the INTERVAL column.  Setting the
  // the interval requires calling origin chunks before performing
  // further iterator.
  
  // Select the channels to be returned.  Requires calling originChunks before
  // performing additional iteration.
  
void SimpleSimVi2::setFrequencySelections (const FrequencySelections &) 
{
  SSVi2NotYetImplemented()
}

  // Return the row ids as from the original root table. This is useful
  // to find correspondance between a given row in this iteration to the
  // original ms row

void SimpleSimVi2::getRowIds (Vector<uInt> &) const {
  SSVi2NotYetImplemented()
  /*
  rowids.resize(nRows());
  indgen(rowids);
  rowids+=iRow0;  // offset to current iteration
  */
}

  /*
VisBuffer2 * SimpleSimVi2::getVisBuffer (const VisibilityIterator2 * vi)
{
  ThrowIf (vb_ == nullptr, "VI Implementation has not VisBuffer.");
  vb_->associateWithVi2 (vi);
  return vb_;
}
  */

VisBuffer2 * SimpleSimVi2::getVisBuffer () { return vb_; }

  //   +=========================+
  //   |                         |
  //   | Subchunk Data Accessors |
  //   |                         |
  //   +=========================+
  
  // Return info
void SimpleSimVi2::antenna1 (Vector<Int> & ant1) const {
  ant1.resize(nBsln_);
  Int k=0;
  for (Int i=0;i<(pars_.doAC_ ? pars_.nAnt_ : pars_.nAnt_-1);++i) {
    for (Int j=(pars_.doAC_ ? i : i+1);j<pars_.nAnt_;++j) {
      ant1[k]=i;
      ++k;
    }
  }
}
void SimpleSimVi2::antenna2 (Vector<Int> & ant2) const {
  ant2.resize(nBsln_);
  Int k=0;
  for (Int i=0;i<(pars_.doAC_ ? pars_.nAnt_ : pars_.nAnt_-1);++i) {
    for (Int j=(pars_.doAC_ ? i : i+1);j<pars_.nAnt_;++j) {
      ant2[k]=j;
      ++k;
    }
  }
}

void SimpleSimVi2::corrType (Vector<Int> & corrs) const { 
  corrs.resize(pars_.nCorr_);
  corrs[0]=corrdef_(0);
  corrs[1]=corrdef_(1);
  corrs[2]=corrdef_(2);
  corrs[3]=corrdef_(3);
  /*SSVi2NotYetImplemented() */
}

Int  SimpleSimVi2::dataDescriptionId () const { return thisSpw_; }
void SimpleSimVi2::dataDescriptionIds (Vector<Int> & ddis) const { ddis.resize(nRows()); ddis.set(thisSpw_); }
void SimpleSimVi2::exposure (Vector<Double> & expo) const { expo.resize(nRows()); expo.set(pars_.dt_); }
void SimpleSimVi2::feed1 (Vector<Int> & fd1) const { fd1.resize(nRows()); fd1.set(0); }
void SimpleSimVi2::feed2 (Vector<Int> & fd2) const { fd2.resize(nRows()); fd2.set(0); }
void SimpleSimVi2::fieldIds (Vector<Int>& fieldids) const { fieldids.resize(nRows()); fieldids.set(thisField_); }
void SimpleSimVi2::arrayIds (Vector<Int>& arrayids) const { arrayids.resize(nRows()); arrayids.set(0); }
String SimpleSimVi2::fieldName () const {return "Field"+String(thisField_); }

void SimpleSimVi2::flag (Cube<Bool> & flags) const {
  // unflagged
  flags.resize(pars_.nCorr_,pars_.nChan_(thisSpw_),nRows());
  flags.set(False);
}
void SimpleSimVi2::flagRow (Vector<Bool> & rowflags) const { rowflags.resize(nRows()); rowflags.set(False); }
void SimpleSimVi2::observationId (Vector<Int> & obsids) const { obsids.resize(nRows()); obsids.set(0); }
Int SimpleSimVi2::polarizationId () const { return 0; }
void SimpleSimVi2::processorId (Vector<Int> & procids) const { procids.resize(nRows()); procids.set(0); }
void SimpleSimVi2::scan (Vector<Int> & scans) const { scans.resize(nRows()); scans.set(thisScan_); }
String SimpleSimVi2::sourceName () const { return "Source"+String(thisField_); }
void SimpleSimVi2::stateId (Vector<Int> & stateids) const { stateids.resize(nRows()); stateids.set(0); }

  Int SimpleSimVi2::polFrame () const { SSVi2NotYetImplemented() }

Int SimpleSimVi2::spectralWindow () const { return thisSpw_; }
void SimpleSimVi2::spectralWindows (Vector<Int> & spws) const { spws.resize(nRows()); spws.set(thisSpw_); }
void SimpleSimVi2::time (Vector<Double> & t) const { t.resize(nRows()); t.set(thisTime_); }
void SimpleSimVi2::timeCentroid (Vector<Double> & t) const { t.resize(nRows()); t.set(thisTime_); }

void SimpleSimVi2::timeInterval (Vector<Double> & ti) const { ti.resize(nRows()); ti.set(pars_.dt_); }
void SimpleSimVi2::uvw (Matrix<Double> & uvwmat) const { uvwmat.resize(3,nRows()); uvwmat.set(0); }  // zero for now

void SimpleSimVi2::visibilityCorrected (Cube<Complex> & vis) const {
  // from DATA, for now
  this->visibilityObserved(vis);
}
void SimpleSimVi2::visibilityModel (Cube<Complex> & vis) const {
  vis.resize(pars_.nCorr_,pars_.nChan_(thisSpw_),nRows());
  // set according to stokes
  // TBD
  for (int icor=0;icor<pars_.nCorr_;++icor)
    vis(Slice(icor),Slice(),Slice()).set(vis0_(icor,thisField_));
}
void SimpleSimVi2::visibilityObserved (Cube<Complex> & vis) const {
  // get basic signals from model
  this->visibilityModel(vis);

  Vector<Int> a1;
  Vector<Int> a2;
  this->antenna1(a1);
  this->antenna2(a2);

  Array<Complex> specvis;
  Matrix<Float> G(pars_.gain_);
  Matrix<Float> Tsys(pars_.tsys_);
  for (Int irow=0;irow<nRows();++irow) {
    for (int icorr=0;icorr<pars_.nCorr_;++icorr) {
      specvis.reference(vis(Slice(icorr),Slice(),Slice(irow)));
      specvis*=sqrt( G(icorr/2,a1(irow)) * G(icorr%2,a2(irow)) );
      if (pars_.doNorm_) 
	specvis/=sqrt( Tsys(icorr/2,a1(irow)) * Tsys(icorr%2,a2(irow)) );
    }
  }

  // Now add noise
  if (pars_.doNoise_) 
    this->addNoise(vis);
}

void SimpleSimVi2::floatData (Cube<Float> & fcube) const {
  fcube.resize(pars_.nCorr_,pars_.nChan_(thisSpw_),nRows());
  // set according to stokes
  // TBD
  fcube.set(0.0);
  // add noise
  // TBD
}

IPosition SimpleSimVi2::visibilityShape () const { return IPosition(3,pars_.nCorr_,pars_.nChan_(thisSpw_),nRows()); }

void SimpleSimVi2::sigma (Matrix<Float> & sigmat) const {
  sigmat.resize(pars_.nCorr_,nRows());
  Matrix<Float> wtmat;
  this->weight(wtmat);
  sigmat=(1.f/sqrt(wtmat));
}
void SimpleSimVi2::weight (Matrix<Float> & wtmat) const {
  wtmat.resize(pars_.nCorr_,nRows());
  wtmat.set(wt0_(thisSpw_)); // spw-specific
  // non-ACs have twice this weight
  Int k=0;
  for (Int i=0;i<(pars_.doAC_ ? pars_.nAnt_ : pars_.nAnt_-1);++i) {
    for (Int j=(pars_.doAC_ ? i : i+1);j<pars_.nAnt_;++j) {
      if (i!=j) {
	Array<Float> thiswtmat(wtmat(Slice(),Slice(k)));
	thiswtmat*=Float(2.0);
      }
      ++k;
    }
  }
}
Bool SimpleSimVi2::weightSpectrumExists () const { return True; }
Bool SimpleSimVi2::sigmaSpectrumExists () const { return True; } 
void SimpleSimVi2::weightSpectrum (Cube<Float> & wtsp) const {
  wtsp.resize(pars_.nCorr_,pars_.nChan_(thisSpw_),nRows());
  wtsp.set(wt0_(thisSpw_));

  if (!pars_.doNoise_) return;

  Vector<Int> a1;
  Vector<Int> a2;
  this->antenna1(a1);
  this->antenna2(a2);
  Matrix<Float> Tsys(pars_.tsys_);

  Int k=0;
  for (Int i=0;i<(pars_.doAC_ ? pars_.nAnt_ : pars_.nAnt_-1);++i) {
    for (Int j=(pars_.doAC_ ? i : i+1);j<pars_.nAnt_;++j) {
      // non-ACs have twice wt0_
      if (i!=j) {
	Array<Float> thiswtsp(wtsp(Slice(),Slice(),Slice(k)));
	thiswtsp*=Float(2.0);
      }

      if (!pars_.doNorm_) {
	for (Int icorr=0;icorr<pars_.nCorr_;++icorr) {
	  Array<Float> thiswt(wtsp(Slice(icorr),Slice(),Slice(k)));
	  Float c= Tsys(icorr/2,a1(k))*Tsys(icorr%2,a2(k));
	  thiswt/=c;
	}
      }
      ++k;
    }
  }
}
void SimpleSimVi2::sigmaSpectrum (Cube<Float> & sigsp) const {
  sigsp.resize(pars_.nCorr_,pars_.nChan_(thisSpw_),nRows());
  Cube<Float> wtsp;
  this->weightSpectrum(wtsp);
  sigsp=(1.f/sqrt(wtsp));
}

  //   +=========================+
  //   |                         |
  //   | Chunk and MS Level Data |
  //   |                         |
  //   +=========================+


CountedPtr<WeightScaling> SimpleSimVi2::getWeightScaling () const {
  return WeightScaling::generateUnityWeightScaling();
}

MEpoch SimpleSimVi2::getEpoch () const { SSVi2NotYetImplemented() }
  
Vector<Int> SimpleSimVi2::getCorrelations () const { 
  // The correlation indices
  Vector<Int> corrs(pars_.nCorr_);
  indgen(corrs);
  return corrs;
}

Vector<Int> SimpleSimVi2::getChannels (Double, Int, Int spw, Int) const { 
  Vector<Int> chans(pars_.nChan_(spw));
  indgen(chans);
  return chans;
}

Vector<Double> SimpleSimVi2::getFrequencies (Double, Int, Int spw, Int) const { 
  Vector<Double> freqs(pars_.nChan_(spw));
  indgen(freqs);
  freqs*=pars_.df_(spw);
  freqs+=(pars_.df_(spw)/2. + pars_.refFreq_(spw));
  return freqs;
}

  // get back the selected spectral windows and spectral channels for
  // current ms

const SpectralWindowChannels & SimpleSimVi2::getSpectralWindowChannels (Int /*msId*/, Int /*spectralWindowId*/) const { SSVi2NotYetImplemented() }

  // Return number of antennasm spws, polids, ddids
  
Int SimpleSimVi2::nAntennas () const { return pars_.nAnt_; }
Int SimpleSimVi2::nDataDescriptionIds () const { return pars_.nSpw_; }
Int SimpleSimVi2::nPolarizationIds () const { return Int(1); }

Int SimpleSimVi2::nRowsInChunk () const { SSVi2NotYetImplemented() } // number rows in current chunk
Int SimpleSimVi2::nRowsViWillSweep () const { SSVi2NotYetImplemented() } // number of rows in all selected ms's

Int SimpleSimVi2::nSpectralWindows () const { return pars_.nSpw_; }


void SimpleSimVi2::configureNewSubchunk() {

  // Poke the vb to do this
  vb_->configureNewSubchunk(0,"faked",False,
			    isNewArrayId(),isNewFieldId(),
			    isNewSpectralWindow(),getSubchunkId(),
			    nRows(),pars_.nChan_(thisSpw_),pars_.nCorr_,
			    getCorrelations(),
			    corrdef_,corrdef_,
			    WeightScaling::generateUnityWeightScaling());
}


VisBuffer2 * SimpleSimVi2::createAttachedVisBuffer (VisBufferOptions options) {

  return SimpleSimVB2::factory(this,options);

}

  // Generate noise on data
void SimpleSimVi2::addNoise(Cube<Complex>& vis) const {

  IPosition sh3(vis.shape());

  Int64 seed(thisScan_*1000000+thisField_*100000+thisSpw_*10000 + Int(thisTime_-t0_));
  ACG r(seed);
  Normal rn(&r,0.0,1.0/wt0_(thisSpw_));

  Vector<Int> a1;
  Vector<Int> a2;
  this->antenna1(a1);
  this->antenna2(a2);

  Matrix<Float> Tsys(pars_.tsys_);
  for (Int i=0;i<sh3[2];++i) {
    Float c(1.0);
    if (a1(i)!=a2(i))
      c=1./sqrt(2.0);
    for (Int k=0;k<sh3[0];++k) {
      Float d(c);
      if (!pars_.doNorm_)
	d*=sqrt(Tsys(k/2,a1(i))*Tsys(k%2,a2(i)));
      for (Int j=0;j<sh3[1];++j) {
	vis(k,j,i)+=Complex(d)*Complex(rn(),rn());
      }
    }
  }

}


SimpleSimVi2Factory::SimpleSimVi2Factory(const SimpleSimVi2Parameters& pars)
  : pars_(pars)
{}
  
SimpleSimVi2Factory::~SimpleSimVi2Factory () {}

ViImplementation2 * SimpleSimVi2Factory::createVi () const {

  ViImplementation2* vii = new SimpleSimVi2(pars_);
  return vii;
  
}



SimpleSimVi2LayerFactory::SimpleSimVi2LayerFactory(const SimpleSimVi2Parameters& pars)
  : ViiLayerFactory(),
    pars_(pars)
{}


// SimpleSimVi2-specific layer-creater
ViImplementation2 * SimpleSimVi2LayerFactory::createInstance (ViImplementation2* /*vii0*/) const {

  // No deeper layers
  //  Assert(vii0==NULL);
  
  // Make it and return it
  ViImplementation2 *vii = new SimpleSimVi2(pars_);
  return vii;
}


} // end namespace vi

} //# NAMESPACE CASA - END



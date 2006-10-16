//# BJonesPoly.cc: Implementation of BJonesPoly.h
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
//# $Id$

#include <synthesis/MeasurementComponents/BJonesPoly.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/sstream.h>
#include <casa/math.h>
#include <casa/System/PGPlotter.h>
#include <graphics/Graphics/PGPlotterLocal.h>
#include <calibration/CalTables/BJonesMBuf.h>
#include <calibration/CalTables/BJonesMCol.h>
#include <ms/MeasurementSets/MSSpWindowIndex.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Define external CLIC solvers
#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define polyant polyant_
#define splinant splinant_
#define getbspl getbspl_
#define phaseant phaseant_
#define ampliant ampliant_
#define cheb cheb_
#endif

extern "C" { 
  void polyant(Int*, Int*, Int*, Int*, Int*, Int*, Int*, Int*,
               Double*, Double*, Double*, Double*, Double*, Double*,
               Double*, Double* );
  void splinant(Int*, Int*, Int*, Int*, Int*, Int*, Int*, Int*,
		Double*, Double*, Double*, Double*, Double*, Double*,
		Double*, Double*, Double* );
  
  void getbspl(Int*, Double*, Double*, Double*, Double*, Int*);
  
  void phaseant(Int*, Int*, Int*, Int*, Double*, Double*, Double*, Double*, 
		Int*, Double*, Double*);
  
  void ampliant(Int*, Int*, Int*, Int*, Double*, Double*, Double*, Double*, 
		Int*, Double*, Double*);
  
  void cheb(Int*, Double*, Double*, Int*);
}

//----------------------------------------------------------------------------

BJonesPoly::BJonesPoly (VisSet& vs) :
  BJones(vs),
  solveTable_p(""),
  append_p(True),
  interval_p(DBL_MAX),
  degamp_p(3),
  degphase_p(3),
  visnorm_p(False),
  bpnorm_p(True),
  maskcenter_p(1),
  maskedge_p(5.0),
  phaseonly_p(False),
  refant_p(0),
  maskcenterHalf_p(0),
  maskedgeFrac_p(0.05),
  applyTable_p(""),
  applySelect_p(""),
  applyInterval_p(0)
{
// Construct from a visibility set
// Input:
//    vs                VisSet&            Visibility set
// Output to private data:
//    solveTable_p      String             Cal. table name for output solutions
//    append_p          Bool               True if appending solutions
//                                         to an existing calibration table
//    interval_p        Double             Solution interval
//    degamp_p          Int                Polynomial degree in amplitude
//    degphase_p        Int                Polynomial degree in phase
//    visnorm           const Bool&        True if pre-normalization of the 
//                                         visibility data over frequency is
//                                         required before solving.
//    bpnorm            const Bool&        True if the output bandpass
//                                         solutions should be normalized.
//    maskcenter        const Int&         No. of central channels to mask
//                                         during the solution
//    maskedge          const Float&       Fraction of spectrum to mask at
//                                         either edge during solution
//    phaseonly_p       Bool               True if phase-only solution
//    refant_p          Int                Reference antenna number
//    maskcenterHalf_p  Int                Central mask half-width
//    maskedgeFrac_p    Float              Fractional edge mask
//    applyTable_p      String             Cal. table name containing
//                                         solutions to be applied
//    applySelect_p     String             Selection for the applied
//                                         calibration table
//    applyInterval_p   Double             Interpolation interval
// Output to protected data:
//    startChan_        Vector<Int>        Start channel in each spw.
//    numberChan_       Vector<Int>        Number of channels in each spw.
//    numberAnt_        Int                Number of antennas
//    numberSpw_        Int                Number of spectral windows
//
  // Initialize a visibility set for iteration with the
  // specified solution interval (set to DBL_MAX for now
  // to conform with the CLIC default).

  Block<Int> columns(0);
  vs_ = new VisSet (vs, columns, interval_p);
  preavg_ = interval_p;
  localVS_ = True;

  // This is currently a one-parameter type (single pol)
  nPar_=1;

  // Initialize protected BJones/VisJones data
  //  startChan_ = vs.startChan();
  //  numberChan_ = vs.numberChan();
  //  numberAnt_ = vs.numberAnt();
  //  numberSpw_ = vs.numberSpw();

  // Neither solved nor applied at this point
  setSolved(False);
  setApplied(False);
};

//----------------------------------------------------------------------------

void BJonesPoly::setSolver (const Record& solver)
{
// Set the solver parameters
// Input:
//    solver            const Record&      Solver parameters
// Output to private data:
//    solveTable_p      String             Cal. table name for output solutions
//    append_p          Bool               True if appending solutions
//                                         to an existing calibration table
//    interval_p        Double             Solution interval
//    degamp_p          Int                Polynomial degree in amplitude
//    degphase_p        Int                Polynomial degree in phase
//    visnorm           const Bool&        True if pre-normalization of the 
//                                         visibility data over frequency is
//                                         required before solving.
//    bpnorm            const Bool&        True if the output bandpass
//                                         solutions should be normalized.
//    maskcenter        const Int&         No. of central channels to mask
//                                         during the solution
//    maskedge          const Float&       Fraction of spectrum to mask at
//                                         either edge during solution
//    phaseonly_p       Bool               True if phase-only solution
//    refant_p          Int                Reference antenna number
//    maskcenterHalf_p  Int                Central mask half-width
//    maskedgeFrac_p    Float              Fractional edge mask
//
  // Extract the solver parameters
  if (solver.isDefined("table")) solveTable_p = solver.asString("table");
  if (solver.isDefined("append")) append_p = solver.asBool("append");
  if (solver.isDefined("t")) interval_p = solver.asDouble("t");
  if (solver.isDefined("degamp")) degamp_p = solver.asInt("degamp");
  if (solver.isDefined("degphase")) degphase_p = solver.asInt("degphase");
  if (solver.isDefined("visnorm")) visnorm_p = solver.asBool("visnorm");
  if (solver.isDefined("bpnorm")) bpnorm_p = solver.asBool("bpnorm");
  if (solver.isDefined("maskcenter")) maskcenter_p = 
					solver.asInt("maskcenter");
  if (solver.isDefined("maskedge")) maskedge_p = solver.asFloat("maskedge");
  if (solver.isDefined("phaseonly")) phaseonly_p = solver.asBool("phaseonly");
  if (solver.isDefined("refant")) refant_p = solver.asInt("refant");

  // Compute derived mask parameters
  maskcenterHalf_p = maskcenter_p / 2;
  maskedgeFrac_p = maskedge_p / 100.0;

  // Mark the Jones matrix as being solved for
  setSolved(True);

  return;
};

//----------------------------------------------------------------------------

void BJonesPoly::setInterpolation (const Record& interpolation)
{
// Set the interpolation parameters
// Input:
//    interpolation     const Record&      Interpolation parameters
// Output to private data:
//    applyTable_p      String             Cal. table name containing
//                                         solutions to be applied
//    applySelect_p     String             Selection for the applied
//                                         calibration table
//    applyInterval_p   Double             Interpolation interval
//
  // Extract the interpolation parameters
  if (interpolation.isDefined("table")) {
    applyTable_p = interpolation.asString("table");
  };
  if (interpolation.isDefined("select")) {
    applySelect_p = interpolation.asString("select");
  };
  if (interpolation.isDefined("t")) {
    applyInterval_p = interpolation.asDouble("t");
  };

  // Fill the bandpass correction cache from the applied cal. table
  load(applyTable_p);

  initMetaCache();
  initThisGain();
  initSolveCache();

  // Mark the Jones matrix as being applied
  setApplied(True);

  return;
};

//----------------------------------------------------------------------------

#define GRIDFREQ
#ifdef GRIDFREQ
#undef GRIDFREQ

// This is a new version which packages the baseline data properly for the
//  Gildas routines and grids each spectrum.  It also does more sanity
//  checking, logging, and produces a nicer plot.

Bool BJonesPoly::solve (VisEquation& me)
{
// Solver for the polynomial bandpass solution
// Input:
//    me           VisEquation&         Measurement Equation (ME) in
//                                      which this Jones matrix resides
// Output:
//    solve        Bool                 True is solution succeeded
//                                      else False
//

// TODO:
//   1. Make pointers private, make delete function and use it 
//   2. Use antenna names
//

  LogIO os (LogOrigin("BJonesPoly", "solve()", WHERE));

  os << LogIO::NORMAL
     << "Fitting bandpass amplitude and phase polynomials."
     << LogIO::POST;
  os << LogIO::NORMAL 
     << "Polynomial degree for amplitude is " << degamp_p 
     << LogIO::POST;
  os << LogIO::NORMAL 
     << "Polynomial degree for phase is " << degphase_p 
     << LogIO::POST;

  // Bool to short-circuit operation
  Bool ok(True);

  // Construct a local ME which can be modified
  VisEquation lme(me);
  
  // Set the visibility set on which the local ME is to
  // operate. This visibility set is already initialized
  // in the constructor for iteration in data chunks of
  // duration interval_p.
  lme.setVisSet(*vs_);

  // Use the iterator from the underlying visibility set,
  // and attach a visibility data buffer
  VisIter& vi(vs_->iter());
  //  vi.setRowBlocking(1000);
  VisBuffer vb(vi);

  // Initialize the baseline index
  Int nAnt = vs_->numberAnt();
  Int nBasl = nAnt * (nAnt - 1) / 2;
  Vector<Int> ant1(nBasl, -1);
  Vector<Int> ant2(nBasl, -1);

  Int nSpw = vs_->numberSpw();
  Vector<Int> numFreqChan(nSpw, 0);

  // make gridded freq array
  // The minimum number of frequency channels required for a solution
  //  is the number of coefficients in the fit.  For a gridded spectrum,
  //  filled from irregularly spaced input spectral windows, it is possible
  //  that only very few channels get filled.  So, we will be conservative
  //  and make a gridded spectrum with 2*ncoeff channels, where ncoeff is
  //  the maximum of the number of coefficients requested for the phase and
  //  amp solutions. We will then check to make sure that a sufficient
  //  number of gridded slots will be filled by the input frequency channels.

  Double minfreq(DBL_MAX), maxfreq(0.0), maxdf(0.0);
  PtrBlock<Vector<Double>*> freq; freq.resize(nSpw); freq=0;
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
    Int spwid=vi.spectralWindow();
    Int nChan = vs_->numberChan()(spwid);
    numFreqChan(spwid) = nChan;
    freq[spwid] = new Vector<Double>;
    (*freq[spwid])=vb.frequency();
    Double df2=abs((*freq[spwid])(1)-(*freq[spwid])(0))/2.0;
    maxdf=max(maxdf,2.0*df2);
    minfreq=min(minfreq,min((*freq[spwid])));
    maxfreq=max(maxfreq,max((*freq[spwid])));
  }
  minfreq=minfreq-maxdf/2.0;
  maxfreq=maxfreq+maxdf/2.0;

  // minfreq is the low edge of the lowest channel
  // maxfreq is the high edge of the highest channel

  Double freqstep;
  Int nFreqGrid;

  // Derive grid spacing/number from requested poly degree
  // FOR NOW, it is less error-prone to use input spacing
  //  nFreqGrid=2*(max(degamp_p,degphase_p)+1);
  //  nFreqGrid=max(nFreqGrid,16);  // no fewer than 16, in any case.
  //  freqstep=((maxfreq-minfreq)/Double(nFreqGrid));

  // Grid spacing is (multiple of?) maximum input channel spacing
  freqstep=maxdf;
  nFreqGrid = Int ((maxfreq-minfreq)/freqstep+0.5);

  // Fill the gridded frequency list
  Vector<Double> totalFreq(nFreqGrid,0.0);
  for (Int i=0;i<nFreqGrid;i++) {
    totalFreq(i)=minfreq + freqstep*(Double(i)+0.5);
  }

  // Populate the frequency grid with the input frequency channels,
  //  and demand that enough (?) will get filled.
  Vector<Bool> freqGridOk(nFreqGrid,False);
  for (Int ispw=0;ispw<nSpw;ispw++) {
    if (freq[ispw]) {
      for (Int ichan=0;ichan<numFreqChan(ispw);ichan++) {
	Int chanidx=(Int) floor(((*freq[ispw])(ichan)-minfreq)/freqstep);
	if(chanidx >= nFreqGrid){

	  cout << "spw " << ispw <<" " <<  chanidx << endl;
	}
	freqGridOk(chanidx)=True;
      }
    }
  }

  // Sanity check on polynomial order and grid count
  Int nok=ntrue(freqGridOk);
  if (nok < (degamp_p+1) ) {
    os << LogIO::SEVERE 
       << "Selected spectral window(s) nominaly fill only " << nok << " grid points." 
       << LogIO::POST;
    os << LogIO::SEVERE 
       << "Reduce degamp by at least " << degamp_p+1-nok << " and try again." 
       << LogIO::POST;
    ok=False;
  }
  if (nok < (degphase_p+1) ) {
    os << LogIO::SEVERE 
       << "Selected spectral window(s) nominally fill only " << nok << " grid points." 
       << LogIO::POST;
    os << LogIO::SEVERE 
       << "Reduce degphase by at least " << degphase_p+1-nok << " and try again." 
       << LogIO::POST;
    ok=False;
  }
  // If either degree vs nGrid test failed, quit here
  if (!ok) return False;

  // Report spectral information
  os << LogIO::NORMAL 
     << "Spectral grid for fit will have " << nFreqGrid
     << " points spaced by " << freqstep/1000.0 << " kHz."
     << LogIO::POST;

  os << LogIO::NORMAL 
     << "Polynomial solution will be valid over frequency range: "
     << totalFreq(0) << "-" << totalFreq(nFreqGrid-1) << " Hz."
     << LogIO::POST;

  os << LogIO::NORMAL 
     << "Total bandwidth: "
     << freqstep*Double(nFreqGrid)/1000.0 << " kHz."
     << LogIO::POST;


  // We must keep track of good ants and baselines
  Vector<Bool> antok(nAnt,False);
  Matrix<Bool> bslok(nAnt,nAnt,False);
  Int nGoodBasl=0;
  Matrix<Int> bslidx(nAnt,nAnt,-1);
  Matrix<Int> antOkChan(nFreqGrid, nAnt,0);
  Vector<Int> ant1num, ant2num;  // clic solver antenna numbers (1-based, contiguous)
  ant1num.resize(nBasl);
  ant2num.resize(nBasl);
  Vector<Int> ant1idx, ant2idx;  // MS.ANTENNA indices (for plot, storage)
  ant1idx.resize(nBasl);
  ant2idx.resize(nBasl);


  Matrix<Complex> accumVis;
  Matrix<Double> accumWeight;
  accumVis.resize(nFreqGrid, nBasl); accumVis=Complex(0.0,0.0);
  accumWeight.resize(nFreqGrid, nBasl); accumWeight=0.0;
  Vector<Complex> normVis;
  Vector<Double> normWeight;
  normVis.resize(nBasl); normVis=Complex(0.0,0.0);
  normWeight.resize(nBasl); normWeight=0.0;

  Vector<Int> indexSpw;

  // By constraint, this solver should see data only from one sideband.
  // Pick up the frequency group name from the current spectral window
  // accordingly.
  String freqGroup("");

  // Iterate, accumulating the averaged spectrum for each 
  // spectral window sub-band.
  Int chunk;
  for (chunk=0, vi.originChunks(); vi.moreChunks(); vi.nextChunk(), chunk++) {

    // Extract the current visibility buffer spectral window id.
    // and number for frequency channels

    Int spwid = vi.spectralWindow();
    Int nChan = vs_->numberChan()(spwid);
    freqGroup = freqGrpName(spwid);
    
    os << LogIO::NORMAL << "Accumulating spw id = " << (spwid+1)
       << ", nchan= " << nChan 
       << " (freq group= " << freqGroup << ") for fit."       
       << LogIO::POST;

    // Compute the corrected and corrupted data at the position of
    // this Jones matrix in the Measurement Equation. The corrupted
    // data are the model visibilities propagated along the ME from
    // the sky to the immediate right of the current Jones matrix.
    // The corrected data are the observed data corrected for all
    // Jones matrices up to the immediate left of the current Jones
    // matrix.
    lme.initChiSquare(*this);
    VisBuffer correctedvb = lme.corrected();
    VisBuffer corruptedvb = lme.corrupted();

    // Data and model values
    Complex data, model;

    // Compute the amplitude and phase spectrum for this spectral window
    for (Int row=0; row < correctedvb.nRow(); row++) {
      // Antenna indices
      Int a1 = correctedvb.antenna1()(row);
      Int a2 = correctedvb.antenna2()(row);
      Double rowwt=correctedvb.weight()(row);

      // Reject auto-correlation data, zero weights, fully-flagged spectra
      if ( (a1 != a2) && 
           (rowwt > 0) && 
	   nfalse(correctedvb.flag().column(row)) > 0 ) {

	// These ants, this baseline is ok (there is at least some good data here)
	antok(a1)=True;
	antok(a2)=True;
	if (!bslok(a1,a2)) {   // first visit to this baseline
	  bslok(a1,a2)=True;
	  bslidx(a1,a2)=nGoodBasl++;
	  ant1idx(bslidx(a1,a2))=a1;
	  ant2idx(bslidx(a1,a2))=a2;
	}

	// Loop over the frequency channels
	for (Int ichan=0; ichan < nChan; ichan++) {
	  // Reject flagged/masked channels
	  if (!correctedvb.flag()(ichan,row) && !maskedChannel(ichan, nChan)) {

	    data = correctedvb.visibility()(ichan,row)(0);
	    model = corruptedvb.visibility()(ichan,row)(0);

	    // accumulate accumVis, accumWeight
	    if (abs(model) > 0) {
	      Vector<Double> freq1=correctedvb.frequency();
	      Int chanidx=(Int) floor((freq1(ichan)-minfreq)/freqstep);
	      accumVis(chanidx,bslidx(a1,a2))+=(Complex(rowwt,0.0)*data/model);
	      accumWeight(chanidx,bslidx(a1,a2))+=rowwt;

	      // count this channel good for these ants
	      antOkChan(chanidx,a1)++;
	      antOkChan(chanidx,a2)++;

	      // Accumulate data normalizing factor (visnorm)
	      normVis(bslidx(a1,a2))+=(Complex(rowwt,0.0)*data/model);
	      normWeight(bslidx(a1,a2))+=rowwt;

	      // cout << row << " " << ichan << " " << chanidx << " "
	      //   << data << " " << model << " " << rowwt << " "
	      //   << accumVis(chanidx,bslidx(a1,a2)) << " "
	      //   << accumWeight(chanidx,bslidx(a1,a2)) << endl;
	    }

	  };
	};
      };
    };
  }; // for (chunk...) iteration

  // Delete freq PtrBlock
  for (Int ispw=0;ispw<nSpw;ispw++) {
    if (freq[ispw]) { delete freq[ispw]; freq[ispw]=0; }
  }

  // Form a contiguous 1-based antenna index list 
  Vector<Int> antnum(nAnt,-1);
  Int antcount=0;
  for (Int iant=0;iant<nAnt;iant++) {
    if (antok(iant)) antnum(iant)=(++antcount);
  }

  Int nGoodAnt=Int(ntrue(antok));

  os << LogIO::NORMAL 
     << "Found data for " << nGoodBasl 
     << " baselines among " << nGoodAnt
     << " antennas."
     << LogIO::POST;

  //  cout << "antOkChan = " << antOkChan << endl;

  Matrix<Double> totalWeight, totalPhase, totalAmp;
  totalAmp.resize(nFreqGrid, nGoodBasl); totalAmp=0.0;
  totalPhase.resize(nFreqGrid, nGoodBasl); totalPhase=0.0;
  totalWeight.resize(nFreqGrid, nGoodBasl); totalWeight=0.0;

  for (Int ibl=0;ibl<nGoodBasl;ibl++) {
    ant1num(ibl)=antnum(ant1idx(ibl));
    ant2num(ibl)=antnum(ant2idx(ibl));
    normVis(ibl)/=(static_cast<Complex>(normWeight(ibl)));
    for (Int ichan=0;ichan<nFreqGrid;ichan++) {
      Double &wt=accumWeight(ichan,ibl);
      // insist at least 4 baselines with good data for these antennas in this channel
      if (wt > 0 &&
	  antOkChan(ichan,ant1idx(ibl)) > 3 &&   
	  antOkChan(ichan,ant2idx(ibl)) > 3 ) {
	accumVis(ichan,ibl)/= (static_cast<Complex>(wt));

	// If requested, normalize the data
	if (visnorm_p) {
	  accumVis(ichan,ibl)/=normVis(ibl);
	}

	totalAmp(ichan,ibl)=static_cast<Double>(log(abs(accumVis(ichan,ibl))));
	totalPhase(ichan,ibl)=static_cast<Double>(arg(accumVis(ichan,ibl)));
	totalWeight(ichan,ibl)=wt;
      }
    }
  }
  ant1num.resize(nGoodBasl,True);
  ant2num.resize(nGoodBasl,True);
  ant1idx.resize(nGoodBasl,True);
  ant2idx.resize(nGoodBasl,True);

  // GILDAS solver uses one-relative antenna numbers
  Int refant = refant_p + 1;

  // First fit the bandpass amplitude
  os << LogIO::NORMAL 
     << "Fitting amplitude polynomial."
     << LogIO::POST;
  
  Int degree = degamp_p + 1;
  Int iy = 1;
  Bool dum;
  Vector<Double> rmsAmpFit2(nGoodBasl,0.0);
  Matrix<Double> ampCoeff2(nGoodAnt, degree, 1.0);
  Matrix<Double> ampCoeff(nAnt, degree,0.0);  // solutions stored here later
   
  {
    // Create work arrays
    Vector<Double> wk1(degree);
    Vector<Double> wk2(degree*degree*nGoodAnt*nGoodAnt);
    Vector<Double> wk3(degree*nGoodAnt);
    
    // Call the CLIC solver for amplitude
    polyant(&iy,
	    &nFreqGrid,
	    &nGoodBasl,
	    ant1num.getStorage(dum),
	    ant2num.getStorage(dum),
	    &refant,
	    &degree,
	    &nGoodAnt,
	    totalFreq.getStorage(dum),
	    totalAmp.getStorage(dum),
	    totalWeight.getStorage(dum),
	    wk1.getStorage(dum),
	    wk2.getStorage(dum),
	    wk3.getStorage(dum),
	    rmsAmpFit2.getStorage(dum),
	    ampCoeff2.getStorage(dum));
  }

  os << LogIO::NORMAL 
     << "Per-baseline RMS log(Amp) statistics: (min/mean/max) = "
     << min(rmsAmpFit2) << "/" << mean(rmsAmpFit2) << "/" << max(rmsAmpFit2)
     << LogIO::POST;

  // Now fit the bandpass phase
  os << LogIO::NORMAL 
     << "Fitting phase polynomial."
     << LogIO::POST;
  
  // Call the CLIC solver for phase
  degree = degphase_p + 1;
  iy = 2;
  Vector<Double> rmsPhaseFit2(nGoodBasl,0.0);
  Matrix<Double> phaseCoeff2(nGoodAnt, degree, 123.0);
  Matrix<Double> phaseCoeff(nAnt, degree, 0.0);  // solutions stored here later
  
  {
    // Create work arrays
    Vector<Double> wk6(degree);
    Vector<Double> wk7(degree*degree*nGoodAnt*nGoodAnt);
    Vector<Double> wk8(degree*nGoodAnt);
    
    polyant(&iy,
	    &nFreqGrid,
	    &nGoodBasl,
	    ant1num.getStorage(dum),
	    ant2num.getStorage(dum),
	    &refant,
	    &degree,
	    &nGoodAnt,
	    totalFreq.getStorage(dum),
	    totalPhase.getStorage(dum),
	    totalWeight.getStorage(dum),
	    wk6.getStorage(dum),
	    wk7.getStorage(dum),
	    wk8.getStorage(dum),
	    rmsPhaseFit2.getStorage(dum),
	    phaseCoeff2.getStorage(dum));
  }
  os << LogIO::NORMAL 
     << "Per baseline RMS phase (deg) statistics: (min/mean/max) = "
     << min(rmsPhaseFit2)*180.0/C::pi << "/" 
     << mean(rmsPhaseFit2)*180.0/C::pi << "/" 
     << max(rmsPhaseFit2)*180.0/C::pi
     << LogIO::POST;

  // Expand solutions into full antenna list
  for (Int iant=0;iant<nAnt;iant++) {
    if (antok(iant)) {
      ampCoeff.row(iant)=ampCoeff2.row(antnum(iant)-1);
      phaseCoeff.row(iant)=phaseCoeff2.row(antnum(iant)-1);
    }
  }

  // plot amplitude and phase baseline data/solutions
  plotsolve2(totalFreq, totalAmp, totalPhase, totalWeight, ant1idx, ant2idx,
	     rmsAmpFit2, ampCoeff, rmsPhaseFit2, phaseCoeff);

  Int nChanTotal=nFreqGrid;
  Double meanfreq=mean(totalFreq);

  // Compute the reference frequency and reference phasor
  Vector<Complex> refAmp;
  Vector<MFrequency> refFreq;
  refAmp.resize(nAnt);
  refFreq.resize(nAnt);  
  for (Int k=0; k < nAnt; k++) {
    refAmp(k) = Complex(1.0,0); 
    refFreq(k) = MFrequency(Quantity(meanfreq, "Hz"));
  }; 
  
  // Frequency range
  Double loFreq = totalFreq(0);
  Double hiFreq = totalFreq(nChanTotal-1);
    
  Matrix<Double> validDomain(nAnt, 2);
  validDomain.column(0) = loFreq;
  validDomain.column(1) = hiFreq;

  // Normalize the output calibration solutions if required
  Vector<Complex> scaleFactor(nAnt, Complex(1,0));
  if (bpnorm_p) {
    os << LogIO::NORMAL 
       << "Normalizing antenna-based solutions."
       << LogIO::POST;
    // Loop over antenna
    for (Int iant=0; iant < nAnt; iant++) {
      Complex meanCorr(0,0);
      // Normalize mean phasor across the spectrum
      for (Int chan=0; chan < nChanTotal; chan++) {
	// only use channels that participated in the fit
	if (antOkChan(chan,iant) > 3) {
	  Double ampval = getChebVal(ampCoeff.row(iant), loFreq, hiFreq,
				     totalFreq(chan));
	  Double phaseval = getChebVal(phaseCoeff.row(iant), loFreq, hiFreq,
				       totalFreq(chan));
	  meanCorr += 
	    Complex(cos(phaseval), sin(phaseval)) * exp(ampval);
	};
      }
      // Set the polynomial scale factor to normalize the solution
      if (meanCorr > 0) 
	scaleFactor(iant) = static_cast<Complex>(nChanTotal) / meanCorr;
    };
  };
  
  // Update the output calibration table
  Vector<Int> antId(nAnt);
  indgen(antId);
  Vector<String> polyType(nAnt, "CHEBYSHEV");
  Vector<String> phaseUnits(nAnt, "RADIANS");
  Vector<Int> refAnt(nAnt, refant_p);
  
  
  updateCalTable (freqGroup, antId, polyType, scaleFactor, validDomain,
		  ampCoeff, phaseCoeff, phaseUnits, refAmp, refFreq, refAnt);
  
  return True;
};

#else

// This is the original version

Bool BJonesPoly::solve (VisEquation& me)
{
// Solver for the polynomial bandpass solution
// Input:
//    me           VisEquation&         Measurement Equation (ME) in
//                                      which this Jones matrix resides
// Output:
//    solve        Bool                 True is solution succeeded
//                                      else False
//
  LogIO os (LogOrigin("BJonesPoly", "solve()", WHERE));

  // Construct a local ME which can be modified
  VisEquation lme(me);
  
  // Set the visibility Jones matrix in the local ME
  lme.setVisJones(*this);
  
  // Set the visibility set on which the local ME is to
  // operate. This visibility set is already initialized
  // in the constructor for iteration in data chunks of
  // duration interval_p.
  lme.setVisSet(*vs_);

  // Use the iterator from the underlying visibility set,
  // and attach a visibility data buffer
  VisIter& vi(vs_->iter());
  //  vi.setRowBlocking(1000);
  VisBuffer vb(vi);

  // Initialize the baseline index
  Int nAnt = vs_->numberAnt();
  Int nBasl = nAnt * (nAnt - 1) / 2;
  Vector<Int> ant1(nBasl, -1);
  Vector<Int> ant2(nBasl, -1);

  Int nSpw = vs_->numberSpw();
  Vector<Int> numFreqChan(nSpw, 0);

  // Initialize accumulation buffers for amplitude, phase,
  // weight and frequency, per spectral window and baseline
  // index, as required by the CLIC solvers.
  PtrBlock<Matrix<Double> *> amp, phase, weight;
  PtrBlock<Vector<Double> *> freq;
  amp.resize(nSpw);
  phase.resize(nSpw);
  weight.resize(nSpw);
  freq.resize(nSpw);

  for (Int k=0; k < nSpw; k++) {
    amp[k] = new Matrix<Double>;
    phase[k] = new Matrix<Double>;
    weight[k] = new Matrix<Double>;
    freq[k] = new Vector<Double>;
  };

  for (Int k=0; k < nAnt; k++) {
    for (Int j=k+1; j < nAnt; j++) {
      // The antenna numbering is one-based for the FORTRAN CLIC solver
      Int index = k * nAnt - k * (k+1) / 2 + j - 1 - k;
      ant1(index) = k + 1;
      ant2(index) = j + 1;
    };
  };

  // By constraint, this solver should see data only from one sideband.
  // Pick up the frequency group name from the current spectral window
  // accordingly.
  String freqGroup("");

  // Iterate, accumulating the averaged spectrum for each 
  // spectral window sub-band. 
  Int chunk;
  for (chunk=0, vi.originChunks(); vi.moreChunks(); vi.nextChunk(), chunk++) {

    // Extract the current visibility buffer spectral window id.
    // and number for frequency channels
    Int spwid = vi.spectralWindow();
    freqGroup = freqGrpName(spwid);
    Int nChan = vs_->numberChan()(spwid);
    numFreqChan(spwid) = nChan;
    
    os << LogIO::NORMAL << "Freq. group " << freqGroup << ", spw= " << (spwid+1) 
       << ", nchan= " << nChan << LogIO::POST;

    // Compute the corrected and corrupted data at the position of
    // this Jones matrix in the Measurement Equation. The corrupted
    // data are the model visibilities propagated along the ME from
    // the sky to the immediate right of the current Jones matrix.
    // The corrected data are the observed data corrected for all
    // Jones matrices up to the immediate left of the current Jones
    // matrix.
    lme.initChiSquare(*this);
    VisBuffer correctedvb = lme.corrected();
    VisBuffer corruptedvb = lme.corrupted();

    // Resize the current accumulation buffers
    amp[spwid]->resize(nChan, nBasl);
    amp[spwid]->set(0);
    phase[spwid]->resize(nChan, nBasl);
    phase[spwid]->set(0);
    freq[spwid]->resize();
    *(freq[spwid]) = vb.frequency();
    weight[spwid]->resize(nChan, nBasl);
    weight[spwid]->set(0);

    // Data and model values
    Complex data, model;

    // Compute the amplitude and phase spectrum for this spectral window
    for (Int row=0; row < correctedvb.nRow(); row++) {
      // Antenna numbers
      Int a1 = correctedvb.antenna1()(row);
      Int a2 = correctedvb.antenna2()(row);

      // Reject auto-correlation data, zero weights, fully-flagged spectra
      if ( (a1 != a2) && 
           (correctedvb.weight()(row) > 0) && 
	   nfalse(correctedvb.flag().column(row)) > 0 ) {
	
	// Compute baseline index
	Int baselineIndex = a1 * nAnt - a1 * (a1 + 1) / 2 + a2 - 1 - a1;

	// Loop over the frequency channels
	for (Int chan=0; chan < nChan; chan++) {
	  // Reject flagged/masked channels
	  if (!correctedvb.flag()(chan,row) && !maskedChannel(chan, nChan)) {

	    (*(weight[spwid]))(chan,baselineIndex) = correctedvb.weight()(row);
	    data = correctedvb.visibility()(chan,row)(0);
	    model = corruptedvb.visibility()(chan,row)(0);

	    // Compute phase
	    Float meanPhase = arg(data) - arg(model);
	    meanPhase = meanPhase - Int(meanPhase/2/C::pi) * 2 * C::pi;
	    while (meanPhase > C::pi) {
	      meanPhase -= 2 * C::pi;
	    };
	    while (meanPhase < -(C::pi)) {
	      meanPhase += 2 * C::pi;
	    };
	    (*(phase[spwid]))(chan, baselineIndex) = static_cast<Double>(meanPhase);

	    // Compute amplitude
	    if (abs(model) > 0) {
	      Float meanAmpl = abs(data) / abs(model);
	      (*(amp[spwid]))(chan,baselineIndex) = 
		static_cast<Double>(log(meanAmpl));
	    } else {
	      (*(amp[spwid]))(chan,baselineIndex) = 0;
	      (*(weight[spwid]))(chan,baselineIndex) = 0;
	    };

	  };
	};
      };
    };
  }; // for (chunk...) iteration


  // Combine the spectral window sub-bands into one complete spectrum
  // First define the combined spectrum (on a common frequency grid)
  Vector<Double> totalFreq;
  Matrix<Double> totalWeight, totalPhase, totalAmp;
  
  // Index all spectral windows for which data was encountered
  // in the iteration through the data to be solved over.
    Vector<Int> indexSpw;
    Int nSpwIndex=0;
    for (Int k=0; k < nSpw; k++) {
      // Check for spectral window id.'s found during 
      // iteration through the data
      if (amp[k]->shape().product() > 0) {
	indexSpw.resize(++nSpwIndex, True);
	indexSpw(nSpwIndex-1) = k;
      };
    };

        
    Vector<Int> kounter(nSpwIndex);
    Vector<Int> stepsign(nSpwIndex);
    for(Int k=0; k < nSpwIndex; k++){
      Int spw = indexSpw(k);
      if( (*(freq[spw]))[0] > (*(freq[spw]))[1]){
	stepsign[k]=-1;
	kounter[k]=numFreqChan[spw]-1;
      }
      else{
	stepsign[k]=1;
	kounter[k]=0;
      }
    }

    Vector<Bool> validsubband(nSpwIndex);
    validsubband.set(True);

    // Compute the size of the combined spectrum
    Int nChanTotal=sum(numFreqChan);
    totalFreq.resize(nChanTotal);
    totalAmp.resize(nChanTotal, nBasl);
    totalPhase.resize(nChanTotal, nBasl);
    totalWeight.resize(nChanTotal, nBasl);

    Int minsubband=0;
    for(Int chan=0; chan<nChanTotal ; chan++){
      for (Int k=0; k<nSpwIndex; k++){
	if(validsubband[k] && validsubband[minsubband]){
	  Int spw = indexSpw(k);
	  Int spwMin = indexSpw(minsubband);
	  if( (*(freq[spw]))[kounter[k]] < 
	      (*(freq[spwMin]))[kounter[minsubband]]){
	    minsubband=k;
	  } 
	}
      }  
      Int spwMin = indexSpw(minsubband);

      totalFreq[chan]=(*(freq[spwMin]))[kounter[minsubband]];
      totalAmp.row(chan)=(*(amp[spwMin])).row(kounter[minsubband]);
      totalPhase.row(chan)=(*(phase[spwMin])).row(kounter[minsubband]);
      totalWeight.row(chan)=(*(weight[spwMin])).row(kounter[minsubband]);

      kounter[minsubband]+=stepsign[minsubband];
      if((kounter[minsubband] >= numFreqChan[spwMin]) 
	 || (kounter[minsubband]<0)){
	validsubband[minsubband]=False;
	minsubband=0;
	while(!validsubband[minsubband] && (minsubband<nSpwIndex) ){
	  ++minsubband;
	}
      }
    }

    // Delete the local accumulation buffers
    for (Int k=0; k< nSpw; k++){
      delete amp[k];
      delete phase[k];
      delete freq[k];
      delete weight[k];
    };

    // First fit the bandpass amplitude
    Int degree = degamp_p + 1;

    // Create work arrays
    Vector<Double> wk1(degree);
    Vector<Double> wk2(degree*degree*nAnt*nAnt);
    Vector<Double> wk3(degree*nAnt);

    // Values to be returned
    Vector<Double> rmsAmpFit(nBasl);
    Matrix<Double> ampCoeff(nAnt, degree, 123.0);

    // Call the CLIC solver for amplitude
    Int iy = 1;
    Bool dum;
    // GILDAS solver uses one-relative antenna numbers
    Int refant = refant_p + 1;

    polyant(&iy,
	    &nChanTotal,
	    &nBasl,
	    ant1.getStorage(dum),
	    ant2.getStorage(dum),
	    &refant,
	    &degree,
	    &nAnt,
	    totalFreq.getStorage(dum),
	    totalAmp.getStorage(dum),
	    totalWeight.getStorage(dum),
	    wk1.getStorage(dum),
	    wk2.getStorage(dum),
	    wk3.getStorage(dum),
	    rmsAmpFit.getStorage(dum),
	    ampCoeff.getStorage(dum));


    cout << "Done with amplitude" << endl;
    plotsolve(totalFreq, totalAmp, totalWeight, rmsAmpFit, ampCoeff, False);
    cout << "Done plotting." <<endl;

 /*
    // Compute an antenna-based average to store as a 
    // reference for each spectral window
    Vector<Double> avgAntAmp;
    avgAntAmp.resize(nAnt);
    avgAntAmp.set(0.0);
    Vector<Double> ampWgtReturned(nAnt);
    Vector<Double> meanAmpVal(nBasl);
    Vector<Double> meanAmpWgt(nBasl);

    for (Int k=0; k< nBasl; k++){
      meanAmpVal(k)=mean(totalAmp.column(k));
      //CHECK: this has to be revisited for phase
      meanAmpWgt(k)=mean(totalWeight.column(k));
    }

    Int errorval;
    Matrix<Double> wk4(nAnt, nAnt);
    Vector<Double> wk5(nBasl);
    ampliant(ant1.getStorage(dum),
	     ant2.getStorage(dum),
	     &nAnt, 
	     &nBasl,
	     meanAmpVal.getStorage(dum),
	     meanAmpWgt.getStorage(dum),
	     avgAntAmp.getStorage(dum),
	     ampWgtReturned.getStorage(dum),
	     &errorval,
	     wk4.getStorage(dum),
	     wk5.getStorage(dum));
 */



    // Now fit the bandpass phase
    degree = degphase_p + 1;

    // Create work arrays
    Vector<Double> wk6(degree);
    Vector<Double> wk7(degree*degree*nAnt*nAnt);
    Vector<Double> wk8(degree*nAnt);

    // Values to be returned
    Vector<Double> rmsPhaseFit(nBasl);
    Matrix<Double> phaseCoeff(nAnt, degree, 1.0);

    // Call the CLIC solver for phase
    iy = 2;

    cout << "Starting phase." << endl;

    polyant(&iy,
	    &nChanTotal,
	    &nBasl,
	    ant1.getStorage(dum),
	    ant2.getStorage(dum),
	    &refant,
	    &degree,
	    &nAnt,
	    totalFreq.getStorage(dum),
	    totalPhase.getStorage(dum),
	    totalWeight.getStorage(dum),
	    wk6.getStorage(dum),
	    wk7.getStorage(dum),
	    wk8.getStorage(dum),
	    rmsPhaseFit.getStorage(dum),
	    phaseCoeff.getStorage(dum));

    cout << "Done with phase" << endl;

    plotsolve(totalFreq, totalPhase, totalWeight, 
		rmsPhaseFit, phaseCoeff, True);

    //    plotsolve2(totalFreq, totalAmp, totalPhase, totalWeight, ant1, ant2,
    //	       rmsAmpFit, ampCoeff, rmsPhaseFit, phaseCoeff);



 /*
    // Compute an antenna-based average to store as a 
    // reference for each spectral window
    Vector<Double> avgAntPhase;
    avgAntPhase.resize(nAnt);
    avgAntPhase.set(0.0);
    Vector<Double> phaseWgtReturned(nAnt);
    Vector<Double> meanPhaseVal(nBasl);
    Vector<Double> meanPhaseWgt(nBasl);

    for (Int k=0; k< nBasl; k++){
      meanPhaseVal(k)=mean(totalPhase.column(k));
      //CHECK: this has to be revisited for phase
      meanPhaseWgt(k)=mean(totalWeight.column(k));
    }

    Matrix<Double> wk9(nAnt, nAnt);
    Vector<Double> wk10(nBasl);
    phaseant(ant1.getStorage(dum),
	     ant2.getStorage(dum),
	     &nAnt, 
	     &nBasl,
	     meanPhaseVal.getStorage(dum),
	     meanPhaseWgt.getStorage(dum),
	     avgAntPhase.getStorage(dum),
	     phaseWgtReturned.getStorage(dum),
	     &errorval,
	     wk9.getStorage(dum),
	     wk10.getStorage(dum));
 */
    

    Double meanfreq=meanFrequency(indexSpw);

    // Compute the reference frequency and reference phasor
    Vector<Complex> refAmp;
    Vector<MFrequency> refFreq;
    refAmp.resize(nAnt);
    refFreq.resize(nAnt);

    for (Int k=0; k < nAnt; k++) {
      refAmp(k) = Complex(1.0,0); 
	//	Complex(cos(avgAntPhase(k)), sin(avgAntPhase(k))) * exp(avgAntAmp(k));
      refFreq(k) = MFrequency(Quantity(meanfreq, "Hz"));
    }; 
    
    // Update the output calibration table
    Vector<Int> antId(nAnt);
    indgen(antId);
    Vector<String> polyType(nAnt, "CHEBYSHEV");

    // Normalize the output calibration solutions if required
    Vector<Complex> scaleFactor(nAnt, Complex(1,0));
    // Frequency range
    Double loFreq = totalFreq(0);
    Double hiFreq = totalFreq(nChanTotal-1);

    if (bpnorm_p) {
      // Loop over antenna
      for (Int ant=0; ant < nAnt; ant++) {
	Complex meanCorr(0,0);
	// Normalize mean phasor across the spectrum
	for (Int chan=0; chan < nChanTotal; chan++) {
	  Double ampval = getChebVal(ampCoeff.row(ant), loFreq, hiFreq,
				     totalFreq(chan));
	  Double phaseval = getChebVal(phaseCoeff.row(ant), loFreq, hiFreq,
				       totalFreq(chan));
	  meanCorr += 
	    Complex(cos(phaseval), sin(phaseval)) * exp(ampval);
	};
	// Set the polynomial scale factor to normalize the solution
	if (meanCorr > 0) 
	  scaleFactor(ant) = static_cast<Complex>(nChanTotal) / meanCorr;
      };
    };
    

    Matrix<Double> validDomain(nAnt, 2);
    validDomain.column(0) = loFreq;
    validDomain.column(1) = hiFreq;

    cout << "validDomain = " << validDomain.row(0) << endl;

    Vector<String> phaseUnits(nAnt, "RADIANS");
    Vector<Int> refAnt(nAnt, refant_p);




    updateCalTable (freqGroup, antId, polyType, scaleFactor, validDomain,
		    ampCoeff, phaseCoeff, phaseUnits, refAmp, refFreq, refAnt);

  return True;
};

#endif



//----------------------------------------------------------------------------
    
void BJonesPoly::updateCalTable (const String& freqGrpName, 
				 const Vector<Int>& antennaId,
				 const Vector<String>& polyType, 
				 const Vector<Complex>& scaleFactor,
				 const Matrix<Double>& validDomain,
				 const Matrix<Double>& polyCoeffAmp,
				 const Matrix<Double>& polyCoeffPhase,
				 const Vector<String>& phaseUnits,
				 const Vector<Complex>& sideBandRef,
				 const Vector<MFrequency>& refFreq, 
				 const Vector<Int>& refAnt)
{
// Update the output calibration table to include the current soln. parameters
// Input:
//    freqGrpName     const String&             Freq. group name
//    antennaId       const Vector<Int>&        Antenna id. for each soln.
//    polyType        const Vector<String>&     Polynomial type (per soln.)
//    scaleFactor     const Vector<Complex>&    Scale factor (per soln.)
//    validDomain     const Matrix<Double>&     Valid domain [x_0, x_1] 
//                                              (per solution)
//    polyCoeffAmp    const Matrix<Double>&     Polynomial amplitude 
//                                              coefficients (per soln.)
//    polyCoeffPhase  const Matrix<Double>&     Polynomial phase coefficients
//                                              (per solution)
//    phaseUnits      const Vector<String>&     Phase units (per solution)
//    sideBandRef     const Vector<Complex>&    Sideband reference phasor
//                                              (per solution)
//    refFreq         const Vec<MFrequency>&    Sideband reference frequency
//                                              (per solution)
//    refAnt          const Vector<Int>&        Reference antenna (per soln.)
// Input from private data:
//    solveTable_p    String                    Output calibration table name
//

  LogIO os (LogOrigin("BJonesPoly", "updateCalTable()", WHERE));

  // Fill the bandpass solution parameters to a BJonesPoly calibration
  // buffer spanning the antenna id.'s
  Vector<Int> key(1, MSCalEnums::ANTENNA1);
  Block<Vector<Int> > keyvals(1, antennaId);
  BJonesPolyMBuf buffer(key, keyvals);

  // Add each solution to the calibration buffer
  for (uInt k=0; k < antennaId.nelements(); k++) {
    buffer.putAntGain(antennaId(k), freqGrpName, polyType(k), scaleFactor(k),
		      validDomain.row(k), polyCoeffAmp.row(k).nelements(),
		      polyCoeffPhase.row(k).nelements(),
		      polyCoeffAmp.row(k), polyCoeffPhase.row(k), 
		      phaseUnits(k), sideBandRef(k), refFreq(k), refAnt(k));
  };

  // Delete the output calibration table is append not specified
  if (!append_p && Table::canDeleteTable(solveTable_p)) {
    Table::deleteTable(solveTable_p);
  };

  os << LogIO::NORMAL 
     << "Storing calibration in table " << solveTable_p
     << LogIO::POST;


  // Append the calibration buffer to the output calibration table
  Table::TableOption accessMode = Table::New;
  if (Table::isWritable(solveTable_p)) accessMode = Table::Update;
  BJonesPolyTable calTable(solveTable_p, accessMode);
  buffer.append(calTable);

  return;
}

//----------------------------------------------------------------------------

Double BJonesPoly::getChebVal (const Vector<Double>& coeff, 
			       const Double& xinit, const Double& xfinal,
			       const Double& x) 
{
// Compute a Chebyshev polynomial value using the CLIC library
// Input:
//    coeff       const Vector<Double>&       Chebyshev coefficients
//    xinit       const Double&               Domain start
//    xfinal      const Double&               Domain end
//    x           const Double&               x-ordinate
// Output:
//    getChebVal  Double                      Chebyshev polynomial value
//
  // Re-scale x-ordinate
  Double xcap=0; 
  xcap=((x-xinit)-(xfinal-x))/(xfinal-xinit);

  // Compute polynomial
  Int deg=coeff.shape().asVector()(0);
  Vector<Double> val(deg);
  Bool check;
  Int checkval;
  cheb(&deg,
       &xcap,
       val.getStorage(check),
       &checkval);

  Double soly=0.0;
  for (Int mm=0; mm< deg; mm++){
    soly+= coeff[mm]*val[mm];
  }
  return soly;
}

//----------------------------------------------------------------------------
    
Bool BJonesPoly::maskedChannel (const Int& chan, const Int& nChan) 
{
// Check if a given channel is masked or not
// Input:
//    chan               const Int&            Channel number
//    nChan              const Int&            No. of channels in spectrum
// Output:
//    maskedChannel      Bool                  Returns true if channel lies
//                                             in edge or center mask
//
  // Initialization
  Bool masked = False;
  Int loChan = nChan / 2 - maskcenterHalf_p;
  Int hiChan = loChan + maskcenter_p;

  // Check mask at center of spectrum
  if ((chan >= loChan) && (chan < hiChan)) {
    masked = True;
  };

  // Check mask at edge of spectrum
  if ((chan < (nChan*maskedgeFrac_p)) || (chan > nChan*(1-maskedgeFrac_p))) {
    masked = True;
  };

  return masked;
};

//----------------------------------------------------------------------------
    
void BJonesPoly::load (const String& applyTable) 
{
// Load and cache the polynomial bandpass corrections
// Input:
//    applyTable      const String&            Cal. table to be applied
// Output to protected data:
//    Antenna and interferometer bandpass correction caches 
//    in the BJones parent class.
//
  // Open the BJonesPoly calibration table
  BJonesPolyTable calTable(applyTable, Table::Update);

  // Attach a calibration table columns accessor
  BJonesPolyMCol col(calTable);

  // Fill the bandpass correction cache
  Int nrows = calTable.nRowMain();

  // Set slot axis length for present spws
  //  String fGN = col.freqGrpName().asString(0);
  //  Vector<Int> spwIds = spwIdsInGroup(fGN);
  //  for (Int ispw=0; ispw<Int(spwIds.nelements()); ispw++) {
  //    numberSlot_(spwIds(ispw))=1;
  //  }


  // For now, only one slot per spw
  numberSlot_=1;

  // Initialize caches
  initAntGain();

  for (Int ispw=0; ispw<numberSpw_; ispw++) {
    if (jonesParOK_[ispw]) {
      *(jonesParOK_[ispw]) = False;
    }
  }

  IPosition ipos(4,0,0,0,0);
  IPosition iposOK(3,0,0,0);

  for (Int row=0; row < nrows; row++) {

    // Antenna id.
    Int antennaId = col.antenna1().asInt(row);
    ipos(2)=antennaId;
    iposOK(1)=antennaId;

    // Frequency group name
    String freqGrpName = col.freqGrpName().asString(row);

    // Extract the polynomial scale factor
    Complex factor = col.scaleFactor().asComplex(row);

    // Extract the valid domain for the polynomial
    Vector<Double> freqDomain(2);
    col.validDomain().get(row, freqDomain);

    // Extract the polynomial coefficients in amplitude and phase
    Int nAmp = col.nPolyAmp().asInt(row);
    Int nPhase = col.nPolyPhase().asInt(row);
    Vector<Double> ampCoeff(nAmp);
    Vector<Double> phaseCoeff(nPhase);
    Array<Double> ampCoeffArray, phaseCoeffArray;
    col.polyCoeffAmp().get(row, ampCoeffArray);
    col.polyCoeffPhase().get(row, phaseCoeffArray);
    IPosition ampPos = ampCoeffArray.shape();
    ampPos = 0;
    for (Int k=0; k < nAmp; k++) {
      ampPos.setLast(IPosition(1,k));
      ampCoeff(k) = ampCoeffArray(ampPos);
    };

    IPosition phasePos = phaseCoeffArray.shape();
    phasePos = 0;
    for (Int k=0; k < nPhase; k++) {
      phasePos.setLast(IPosition(1,k));
      phaseCoeff(k) = phaseCoeffArray(phasePos);
    };

    // Loop over all spectral window id.'s in this frequency group
    Vector<Int> spwIds = spwIdsInGroup(freqGrpName);

    for (uInt k=0; k < spwIds.nelements(); k++) {

      Int spwId = spwIds(k);
      // Fill the bandpass correction cache

      Int nChan = nSolnChan_(spwId);
      
      // The next line should use startChan!!!  ***HELP***
      Vector<Double> freq = freqAxis(spwId);

      Double x1 = freqDomain(0);
      Double x2 = freqDomain(1);

      // Loop over frequency channel
      for (Int chan=0; chan < nChan; chan++) {
	ipos(1)=chan;
	iposOK(0)=chan;
	Double ampval(0.0),phaseval(0.0);
	// only if in domain, calculate Cheby
	if (freq(chan) >=x1 && freq(chan)<= x2) {
	  ampval = getChebVal(ampCoeff, x1, x2, freq(chan));
	  phaseval = getChebVal(phaseCoeff, x1, x2, freq(chan));
	}
	(*jonesPar_[spwId])(ipos) = factor *
	  exp(ampval) * Complex(cos(phaseval),sin(phaseval));
	// Set flag for valid cache value 
	(*jonesParOK_[spwId])(iposOK) = True;
      };
    };
  };


  return;
}

//----------------------------------------------------------------------------
    
Double BJonesPoly::meanFrequency (const Vector<Int>& spwid) 
{
// Compute the bandwidth-weighted average frequency of a set of spw id.'s
// Input:
//    spwid           const Vector<Int>&       Spectral window id.'s
// Input from private data:
//    vs_             VisSet*                  Current visibility set
// Output:
//    meanFrequency   Double                   Mean frequency (as Double)
//
  // Open the SPECTRAL_WINDOW sub-table

  //  MeasurementSet ms(vs_->msName());
  //  MSSpectralWindow spectralTable = ms.spectralWindow();
  //  ROArrayColumn<Double> 
  //    frequencies(spectralTable,
  //		MSSpectralWindow::columnName(MSSpectralWindow::CHAN_FREQ));
  //  ROScalarColumn<Double> 
  //    totalbw(spectralTable,
  //	    MSSpectralWindow::columnName(MSSpectralWindow::TOTAL_BANDWIDTH));

  const ROMSColumns& mscol(vs_->iter().msColumns());
  const ROMSSpWindowColumns& spwcol(mscol.spectralWindow());
  const ROArrayColumn<Double>& frequencies(spwcol.chanFreq());
  const ROScalarColumn<Double>& totalbw(spwcol.totalBandwidth());

  Int numspw=spwid.shape().asVector()(0);

  Double meanFreq=0.0;
  Double sumbw=0.0;
  for (Int k=0; k<numspw; k++){
    meanFreq = meanFreq +
      sum(frequencies(spwid(k)))*totalbw(spwid(k)) / 
      (frequencies(spwid(k)).nelements());
    sumbw = sumbw + totalbw(spwid(k));
  }
  // Compute the mean frequency
  meanFreq=meanFreq/sumbw;
  return meanFreq;
}

//----------------------------------------------------------------------------
    
String BJonesPoly::freqGrpName (const Int& spwId) 
{
// Return the frequency group name for a given spectral window id.
// Input:
//    spwId           const Int&               Spectral window id.
// Input from private data:
//    vs_             VisSet*                  Current visibility set
// Output:
//    freqGrpName     String                   Frequency group name
//
  // Open a SPECTRAL_WINDOW sub-table index
  //  MeasurementSet ms(vs_->msName());
  //  ROMSSpWindowColumns spwCol(ms.spectralWindow());

  const ROMSColumns& mscol(vs_->iter().msColumns());
  const ROMSSpWindowColumns& spwCol(mscol.spectralWindow());

  return spwCol.freqGroupName().asString(spwId);
}

//----------------------------------------------------------------------------
    
Vector<Int> BJonesPoly::spwIdsInGroup (const String& freqGrpName) 
{
// Return the spw. id.'s in a freq. group of a given name
// Input:
//    freqGrpName     const String&            Frequency group name
// Input from private data:
//    vs_             VisSet*                  Current visibility set
// Output:
//    spwIdsInGroup   Vector<Int>              Spw. id.'s in freq. group
//
  // Open a SPECTRAL_WINDOW sub-table index
  MeasurementSet ms(vs_->msName().before("/SELECTED"));
  MSSpWindowIndex spwIndex(ms.spectralWindow());
  return spwIndex.matchFreqGrpName(freqGrpName);
}

//----------------------------------------------------------------------------
    
Vector<Double> BJonesPoly::freqAxis (const Int& spwId) 
{
// Return the frequency axis values for a given spectral window id.
// Input:
//    spwId           const Int&               Spectral window id.
// Input from private data:
//    vs_             VisSet*                  Current visibility set
// Output:
//    freqAxis        Vector<Double>           Frequency axis values
//
  // Open a SPECTRAL_WINDOW sub-table columns accessor
  //  MeasurementSet ms(vs_->msName());
  //  ROMSSpWindowColumns spwCol(ms.spectralWindow());

  const ROMSColumns& mscol(vs_->iter().msColumns());
  const ROMSSpWindowColumns& spwCol(mscol.spectralWindow());

  Vector<Double> freqVal;
  spwCol.chanFreq().get(spwId, freqVal);
  return freqVal;
}

//----------------------------------------------------------------------------
void BJonesPoly::plotsolve(const Vector<Double>& x, 
			   const Matrix<Double>& yall, 
			   const Matrix<Double>& weightall, 
			   const Vector<Double>& errall, 
			   Matrix<Double>& coeff, Bool phasesoln){



  // Function to plot and compare Bandpass data and solution
  // Input:
  // x - vector of frequecies
  // yall - matrix of data - shape is yall(freqindex, baselineindex)
  // weightall - matrix of weight; same shape as yall
  // errall - error of fits for each baseline
  // coeff - matrix coefficients of polynomial fits -shape coeff(nant, degree)
  // phasesoln - either phase or amplitude plots

  String device;
  if (phasesoln){
    device=(vs_->msName().before("/SELECTED"));
    device=(device.before("/SORTED_TABLE"))+".RF_PHASE.ps/cps";
  }
  else{
    device=(vs_->msName().before("/SELECTED"));
    device=(device.before("/SORTED_TABLE"))+".RF_AMP.ps/cps";
  }

  PGPlotterLocal pg(device);
  pg.subp(4,3);
  Int numpoints= max(x.shape().asVector());
  Int numplotpoints=4*numpoints;
  Int num_valid_points=0;
  Vector<Float> x1(numpoints);
  Vector<Float> y1(numpoints);
  Vector<Float> errarray(numpoints); 
  Vector<Double>  xval(numplotpoints);
  Vector<Float> xfloatval(numplotpoints);
  Vector<Float> soly1(numplotpoints);
  Vector<Double> y, weight;
  Double err;
  Double minfreq;
  minfreq=min(x);
  Int num_ant = vs_->numberAnt();
  Vector<Double> ant1coeff, ant2coeff; 
  for (Int ant1=0; ant1 < num_ant; ++ant1){
    for (Int ant2=ant1+1; ant2 < num_ant; ++ant2){
      Int basnum=ant1*num_ant-ant1*(ant1+1)/2+ant2-1-ant1;
      
      x1.resize(numpoints);
      y1.resize(numpoints);
      errarray.resize(numpoints);
      y.resize(); 
      weight.resize();
      ant1coeff=coeff.row(ant1);
      ant2coeff=coeff.row(ant2);
      
      y=yall.column(basnum);
      weight=weightall.column(basnum);  
      err=errall[basnum];
      num_valid_points=0;

      for(Int k=0; k < numpoints; k++){
	if (weight(k)>0){
	  x1(num_valid_points)=(x(k)-minfreq)/1e3;
	  y1(num_valid_points)=y(k);
	  if(phasesoln==True){
	    y1(num_valid_points)=remainder(y1(num_valid_points), 2*C::pi);
	    y1(num_valid_points)=y1(num_valid_points)/C::pi*180.0;
	    //	    errarray(num_valid_points)=Float(err);
	    errarray(num_valid_points)=Float(err)/C::pi*180.0;
	  }
	  else{
	    y1(num_valid_points)=exp(y1(num_valid_points));
	    errarray(num_valid_points)=Float(err)*y1(num_valid_points);
	    //	    errarray(num_valid_points)=exp(err);
	  }
	  num_valid_points+=1;
	}
	
      }
      
      if(num_valid_points == 0){
      	cout << "No valid point on baselines with antennas " 
	     <<  ant1 << " & " << ant2 << endl;
	pg.sci(1);
	pg.env(0.0,1.0,0.0,1.0,0,-2);
	pg.sch(2.0);
	ostringstream oos;
	oos << "No data for baseline " << ant1+1 << " & " << ant2+1;
	pg.ptxt(0.5,0.5,0.0,0.5,oos);
	pg.sch(1.0);
      } else {

      for(Int k=0; k < numplotpoints; k++){
	xval[k]= (k)*((x[numpoints-1]-x[0]))/Double(numplotpoints-1)
	    +(x[0]);	
	
	if(phasesoln==True){
	  soly1(k)=0; 
	  soly1(k)=getChebVal(ant2coeff,x(0),x(numpoints-1), xval[k])-
	      getChebVal(ant1coeff,x(0),x(numpoints-1), xval[k]) ;
	  while(soly1(k) > C::pi){
	    soly1(k) -= 2.0*(C::pi);
	  }
	  while(soly1(k) < (-(C::pi))){
	    soly1(k) += 2.0*(C::pi);
	  }
	  soly1(k)=soly1(k)/(C::pi)*180.0;
		      
	  
	}
	else{
	  soly1[k]=getChebVal(ant2coeff,x(0),x(numpoints-1), xval(k))+
	    getChebVal(ant1coeff,x(0),x(numpoints-1), xval(k));
	  soly1[k]=exp(soly1[k]);
	  
	  
	  
	}
	
	xfloatval(k)=Float((xval(k)-minfreq)/1.0e3);
      }


      //cout << " Mean of sol " << mean(soly1) << endl;
      x1.resize(num_valid_points, True);
      y1.resize(num_valid_points, True);
      errarray.resize(num_valid_points, True);
      pg.sci(1);
      Float max_data, min_data, max_err;
      max_err = max(errarray);
      max_data=max(y1)+1.5*max_err;
      min_data=min(y1)-1.5*max_err;
      max_data=max(max_data, max(soly1));
      min_data=min(min_data, min(soly1));
      Float min_x= min(xfloatval);
      Float max_x= max(xfloatval);
      pg.sch(1.5);
      pg.env(min_x, max_x, min_data, max_data, 0, 0);
      ostringstream oos, xlab;
      xlab << "Frequency in kHz (-" << minfreq/1.0e9 << " GHz)";
      if(phasesoln){
	oos << "Bandpass phase for antenna " << ant1+1 << " & " << ant2+1 ;
	pg.lab(xlab, "Phase in deg", oos);
      }
      else{
	oos << "Bandpass amplitude for antenna " << ant1+1 << " & " << ant2+1 ;
	pg.lab(xlab, "Amplitude", oos);
      }
      pg.sch(0.75);
      pg.sci(2);
      pg.errb(6, x1, y1, errarray, 2.0);
      pg.sci(3);
      pg.pt(x1, y1, 17);
      pg.sci(4);
      pg.line(xfloatval, soly1);
      }
    }
  }

}

//----------------------------------------------------------------------------
void BJonesPoly::plotsolve2(const Vector<Double>& x, 
			    const Matrix<Double>& ampdata, 
			    const Matrix<Double>& phadata, 
			    const Matrix<Double>& wtdata, 
			    const Vector<Int>& ant1idx, const Vector<Int>& ant2idx, 
			    const Vector<Double>& amperr, Matrix<Double>& ampcoeff, 
			    const Vector<Double>& phaerr, Matrix<Double>& phacoeff) 

{
  // Function to plot and compare Bandpass data and solution
  // Input:
  // x - vector of frequecies
  // ampdata - matrix of data - shape is yall(freqindex, baselineindex)
  // phadata - matrix of data - shape is yall(freqindex, baselineindex)
  // wtdata - matrix of weight; same shape as yall
  // ant1idx - indices for antenna1 
  // ant2idx - indices for antenna2
  // amperr - baseline-based amplitude fit errors 
  // ampcoeff - antenna-based amplitude poly coefficients
  // phaerr - baseline-based phase fit errors
  // phacoeff - antenna-based phase poly coefficients

  LogIO os (LogOrigin("BJonesPoly", "plotsolve2()", WHERE));

  String device;
  device=solveTable_p + ".ps/cps";

  os << LogIO::NORMAL 
     << "Generating plot file:" << device
     << LogIO::POST;

  PGPlotterLocal pg(device);
  pg.subp(4,3);

  Int ndatapts= max(x.shape().asVector());
  Int nmodelpts=4*(ndatapts-2)-1;
  Int num_valid_points=0;

  Int numbasl=ampdata.shape()(1);

  Vector<Float> x1(ndatapts);
  Vector<Float> amp1(ndatapts);
  Vector<Float> pha1(ndatapts);
  Vector<Float> amperr1(ndatapts); 
  Vector<Float> phaerr1(ndatapts); 
  Vector<Double> weight;

  Vector<Double>  xval(nmodelpts);
  Vector<Float> xfloatval(nmodelpts);

  Vector<Float> amp2a(nmodelpts);
  Vector<Float> amp2b(nmodelpts);
  Vector<Float> amp2(nmodelpts);
  Vector<Float> pha2a(nmodelpts);
  Vector<Float> pha2b(nmodelpts);
  Vector<Float> pha2(nmodelpts);

  Double minfreq,maxfreq;
  minfreq=min(x); 
  maxfreq=max(x); 

  Double dmodfreq;
  dmodfreq = (maxfreq-minfreq)/Double(ndatapts-1)/4;

  Vector<Double> ant1ampcoeff, ant2ampcoeff; 
  Vector<Double> ant1phacoeff, ant2phacoeff; 

  Vector<Float> meanamp(numbasl,0.0);
  Vector<Float> meanampmod(numbasl,0.0);
  Vector<Float> meanpha(numbasl,0.0);
  Vector<Float> meanphamod(numbasl,0.0);
  for (Int ibl=0; ibl<numbasl; ibl++) {

    // refresh all plotting variables
    x1.resize(ndatapts);
    amp1.resize(ndatapts);
    pha1.resize(ndatapts);
    amperr1.resize(ndatapts);
    phaerr1.resize(ndatapts);
    weight.resize();
    weight=wtdata.column(ibl);  

    num_valid_points=0;
    

    for(Int k=0; k < ndatapts; k++){
      if (weight(k)>0){
	x1(num_valid_points)=(x(k)-minfreq)/1e3;
	
	amp1(num_valid_points)=exp(ampdata.column(ibl)(k));
	amperr1(num_valid_points)=Float(amperr(ibl))*amp1(num_valid_points);

	pha1(num_valid_points)=remainder(phadata.column(ibl)(k), 2*C::pi)/C::pi*180.0;
	phaerr1(num_valid_points)=Float(phaerr(ibl))/C::pi*180.0;
	  
	num_valid_points+=1;
      }
    }
      
    if (num_valid_points > 0){

      // resize data arrays according to num_valid_points
      x1.resize(num_valid_points, True);
      amp1.resize(num_valid_points, True);
      amperr1.resize(num_valid_points, True);
      pha1.resize(num_valid_points, True);
      phaerr1.resize(num_valid_points, True);

      ant1ampcoeff=ampcoeff.row(ant1idx(ibl));
      ant2ampcoeff=ampcoeff.row(ant2idx(ibl));
      ant1phacoeff=phacoeff.row(ant1idx(ibl));
      ant2phacoeff=phacoeff.row(ant2idx(ibl));
      
      for(Int k=0; k < nmodelpts; k++){

	xval[k]= Double(k-1)*dmodfreq + x[1];	
	
	// Float version, offset & scaled, for plotting
	xfloatval(k)=Float((xval(k)-minfreq)/1.0e3);

	pha2(k)=0;
	pha2a(k)=getChebVal(ant1phacoeff,x(0),x(ndatapts-1), xval[k]);
	pha2b(k)=getChebVal(ant2phacoeff,x(0),x(ndatapts-1), xval[k]);
	pha2(k)=pha2b(k)-pha2a(k);
	          
	pha2a(k)=remainder(pha2a(k),2*C::pi)*180.0/C::pi;
	pha2b(k)=remainder(pha2b(k),2*C::pi)*180.0/C::pi;
	pha2(k)=remainder(pha2(k),2*C::pi)*180.0/C::pi;


	amp2a(k)=getChebVal(ant1ampcoeff,x(0),x(ndatapts-1), xval(k));
	amp2b(k)=getChebVal(ant2ampcoeff,x(0),x(ndatapts-1), xval(k));
	amp2(k)=amp2b(k) + amp2a(k);
                  
	amp2a(k)=exp(amp2a(k));
	amp2b(k)=exp(amp2b(k));
	amp2(k)=exp(amp2(k));

      }

      meanamp(ibl) = mean(amp1);
      meanampmod(ibl) = mean(amp2);
      meanpha(ibl) = mean(pha1);
      meanphamod(ibl) = mean(pha2);


 /*
      cout << ant1idx(ibl)+1 << "-" << ant2idx(ibl)+1 << "  Means: " 
	   << mean(amp1) << " "
	   << mean(amp2) << " "
	   << mean(pha1) << " "
	   << mean(pha2) << endl;
      cout << "means    = " 
	   << mean(meanamp) << " "
	   << mean(meanampmod)  << " "
	   << mean(meanpha)  << " "
	   << mean(meanphamod)  << " "
	   << endl;
 */

      //cout << " Mean of sol " << mean(soly1) << endl;

      Float min_x= 0.0;
      Float max_x= Float((maxfreq-minfreq)/1.0e3);

      Float min_amp, max_amp, min_pha, max_pha;
      min_amp=min( (min(amp1)-3.0*max(amperr1)), min(amp2) );
      max_amp=max( (max(amp1)+3.0*max(amperr1)), max(amp2) );
      min_pha=min( (min(pha1)-3.0*max(phaerr1)), min(pha2) );
      max_pha=max( (max(pha1)+3.0*max(phaerr1)), max(pha2) );

      min_amp=min(amp1)-1.0*max(amperr1);
      max_amp=max(amp1)+1.0*max(amperr1);
      min_pha=min(pha1)-1.0*max(phaerr1);
      max_pha=max(pha1)+1.0*max(phaerr1);

      Float damp=0.1*(max_amp-min_amp);
      Float dpha=0.1*(max_pha-min_pha);
      min_amp-=damp;
      max_amp+=damp;
      min_pha-=dpha;
      max_pha+=dpha;
	

      pg.page();

      // arrange labels
      ostringstream titlelab, xlab, ampdeg, phadeg, pharms, amprms;
      xlab.precision(12);
      xlab << "Frequency in kHz (-" << minfreq/1.0e9 << " GHz)";
      titlelab << "B polynomial for baseline " 
	       << ant1idx(ibl)+1 << " & " << ant2idx(ibl)+1;
      ampdeg << "degree = " << degamp_p;
      phadeg << "degree = " << degphase_p;
      pharms << "rms = " << phaerr1(0);
      amprms << "rms = " << amperr1(0);


      // plot phase in upper half
      pg.sci(1); pg.sch(1.5);
      //      pg.svp(0.13,0.87,0.5,0.85);
      pg.svp(0.10,0.90,0.525,0.90);
      pg.swin(min_x,max_x,min_pha,max_pha);
      pg.box("BCST",0.0,0,"BCNST",0.0,0);

      pg.sch(0.75); 
      pg.sci(2);
      pg.errb(6, x1, pha1, phaerr1, 2.0);
      pg.sci(3);
      pg.pt(x1, pha1, 17);
      pg.sci(4); pg.sls(1);
      pg.line(xfloatval, pha2);
/*
      pg.sci(4); pg.sls(4);  // plot each antenna soln
      pg.line(xfloatval, pha2a);
      pg.line(xfloatval, pha2b);
      pg.sls(1);
*/
      pg.sci(1); pg.sch(1.5);
      pg.lab(" ", "Phase (deg)", " ");
      pg.mtxt("T",0.75,0.5,0.5,titlelab);
      pg.sch(1.2);
      pg.mtxt("T",-1.5,0.95,1.0,phadeg);
      pg.mtxt("B",-1.5,0.95,1.0,pharms);


      // plot amp in upper half
      pg.sci(1); pg.sch(1.5);

      pg.svp(0.10,0.90,0.15,0.525);
      pg.swin(min_x,max_x,min_amp,max_amp);
      pg.box("BCNST",0.0,0,"BCNST",0.0,0);
      pg.sch(0.75);
      pg.sci(2);
      pg.errb(6, x1, amp1, amperr1, 2.0);
      pg.sci(3);
      pg.pt(x1, amp1, 17);
      pg.sci(4); pg.sls(1);
      pg.line(xfloatval, amp2);
/*
      pg.sci(4); pg.sls(4);  // plot each antenna soln
      pg.line(xfloatval, amp2a);
      pg.line(xfloatval, amp2b);
      pg.sls(1);
*/
      pg.sci(1) ; pg.sch(1.5);
      pg.lab(xlab, "Amp", " ");
      pg.sch(1.2);
      pg.mtxt("T",-1.5,0.95,1.0,ampdeg);
      pg.mtxt("B",-1.5,0.95,1.0,amprms);


    } else {
      // shouldn't get here because this wouldn't be one of the good baselines
      pg.sci(1);
      pg.env(0.0,1.0,0.0,1.0,0,-2);
      pg.sch(2.0);
      ostringstream oos;
      oos << "No data for baseline " << ant1idx(ibl)+1 << " & " << ant2idx(ibl)+1;
      pg.ptxt(0.5,0.5,0.0,0.5,oos);
      pg.sch(1.0);
    }
  }
}

//---------------------------------------------------------------------------

} //# NAMESPACE CASA - END


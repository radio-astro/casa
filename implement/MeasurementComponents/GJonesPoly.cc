//# GJonesPoly.cc: Implementation of GJonesPoly.h
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

#include <synthesis/MeasurementComponents/GJonesPoly.h>

#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <casa/Quanta/MVTime.h>
//#include <casa/Containers/SimOrdMap.h>
#include <casa/Containers/Block.h>
#include <casa/math.h>
#include <casa/fstream.h>

#include <casa/System/PGPlotter.h>
#include <graphics/Graphics/PGPlotterLocal.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <calibration/CalTables/GJonesMBuf.h>
#include <calibration/CalTables/GJonesTable.h>
#include <calibration/CalTables/CalIter.h>

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

GJonesSpline::GJonesSpline (VisSet& vs) :
  GJones(vs),
  solveTable_p(""),
  append_p(True),
  mode_p("PHAS"),
  solveAmp_p(False),
  solvePhase_p(True),
  interval_p(DBL_MAX),
  preavg_p(0),
  splinetime_p(7200.0),
  refant_p(0),
  applyTable_p(""),
  applySelect_p(""),
  applyInterval_p(0),
  cacheTimeValid_p(0),
  calBuffer_p(NULL),
  rawPhaseRemoval_p(False),
  timeValueMap_p(0)
{
// Construct from a visibility set
// Input:
//    vs                VisSet&            Visibility set
// Output to private data:
//    solveTable_p      String             Cal. table name for solutions
//    append_p          Bool               True if appending solutions
//                                         to an existing calibration table
//    mode_p            String             Solve mode (AMP, PHAS or A&P)
//    solveAmp_p        Bool               True if mode_p includes amp. soln.
//    solvePhase_p      Bool               True if mode_p includes phase soln.
//    interval_p        Double             Solution interval (sec)
//    preavg_p          Double             Pre-averaging interval (sec)
//    refant_p          Int                Reference antenna number
//    applyTable_p      String             Cal. table name containing
//                                         solutions to be applied
//    applySelect_p     String             Selection for the applied
//                                         calibration table
//    applyInterval_p   Double             Interpolation interval
//    cacheTimeValid_p  Double             Time for which the current
//                                         calibration cache is valid
//    calBuffer_p       GJonesSplineMBuf*  Ptr to the applied cal. buffer
//
  // Initialize a visibility set for iteration with the
  // specified solution interval (set to DBL_MAX for now
  // to conform with CLIC default).
  Block<Int> columns(0);
  vs_ = new VisSet (vs, columns, interval_p);
  localVS_=True;
  
  // Initialize protected TimeVarVisJones data 
  //  numberAnt_ = vs.numberAnt();
  //  numberSpw_ = vs.numberSpw();

  // Mark the Jones matrix as neither solved for nor applied,
  // pending initialization by setSolver() or setInterpolation()
  setSolved(False);
  setApplied(False);

  // Set the pre-averaging interval used in the ME when forming
  // the corrected and corrupted data
  preavg_ = preavg_p;
};

//----------------------------------------------------------------------------

GJonesSpline::~GJonesSpline () 
{
// Virtual destructor
// Output to private data:
//    calBuffer_p       GJonesSplineMBuf*  Ptr to the applied cal. buffer
//
  // Delete the calibration buffer
  if (calBuffer_p) delete(calBuffer_p);
};

//----------------------------------------------------------------------------

void GJonesSpline::setSolver (const Record& solver)
{
// Set the solver parameters
// Input:
//    solver            const Record&      Solver parameters
// Output to private data:
//    solveTable_p      String             Cal. table name for solutions
//    append_p          Bool               True if appending solutions
//                                         to an existing calibration table
//    mode_p            String             Solve mode (AMP, PHAS or A&P)
//    interval_p        Double             Solution interval (sec)
//    preavg_p          Double             Pre-averaging interval (sec)
//    splinetime_p      Double             Spline knot timescale
//    refant_p          Int                Reference antenna number
//
  // Extract the solver parameters
  if (solver.isDefined("table")) solveTable_p = solver.asString("table");
  if (solver.isDefined("append")) append_p = solver.asBool("append");

  if (solver.isDefined("mode")) mode_p = solver.asString("mode");
  solveAmp_p = (mode_p.contains("AMP") || mode_p.contains("amp"));
  solvePhase_p = (mode_p.contains("PHAS") || mode_p.contains("phas"));

  if (solver.isDefined("t")) interval_p = solver.asDouble("t");
  if (solver.isDefined("preavg")) preavg_p = solver.asDouble("preavg");
  if (solver.isDefined("splinetime")) splinetime_p = solver.asDouble("splinetime");


  if (solver.isDefined("refant")) refant_p = solver.asInt("refant");

  // Mark the Jones matrix as being solved for
  setSolved(True);

  // Set the pre-averaging interval used in the ME when forming
  // the corrected and corrupted data
  preavg_ = preavg_p;

  return;
};

//----------------------------------------------------------------------------

void GJonesSpline::setInterpolation (const Record& interpolation)
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

  // Initialize the TimeVarVisJones gain correction cache
  TimeVarVisJones::initThisGain();

  // Attach a calibration buffer and iterator to the calibration 
  // table containing corrections to be applied
  GJonesSplineTable calTable(applyTable_p, Table::Update);
  CalIter calIter(calTable);
  // Create the buffer and synchronize with the iterator
  if (calBuffer_p) delete calBuffer_p;
  calBuffer_p = new GJonesSplineMBuf(calIter);
  calBuffer_p->synchronize();
  calBuffer_p->fillCache();

  // Mark the Jones matrix as being applied
  setApplied(True);
  
  return;
};

//----------------------------------------------------------------------------

Bool GJonesSpline::solve (VisEquation& me)
{
// Solver for the electronic gain in spline form
// Input:
//    me           VisEquation&         Measurement Equation (ME) in
//                                      which this Jones matrix resides
// Output:
//    solve        Bool                 True is solution succeeded
//                                      else False
//
  LogIO os (LogOrigin("GJonesSpline", "solve()", WHERE));

  os << LogIO::NORMAL
     << "Fitting time-dependent cubic splines."
     << LogIO::POST;
  if (solvePhase_p) {
    os << LogIO::NORMAL
       << "Solving for phase splines with splinetime= " << splinetime_p
       << LogIO::POST;
  }
  if (solveAmp_p) {
    os << LogIO::NORMAL
       << "Solving for amplitude splines with splinetime= " << splinetime_p
       << LogIO::POST;
  }

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
  VisBuffer vb(vi);

  // Initialize time-series accumulation buffers for the
  // corrected and corrupted visibility data and associated
  // weights. 
  SimpleOrderedMap<String,Int> timeValueMap(0);
  Vector<Double> timeValues;
  PtrBlock<Vector<Complex>* > visTimeSeries, modelTimeSeries;
  PtrBlock<Vector<Double>* > weightTimeSeries;
  Int nTimeSeries = 0;

  // Initialize the baseline index
  Int nAnt = vs_->numberAnt();
  Int nBasl = nAnt * (nAnt - 1) / 2;
  Vector<Int> ant1(nBasl, -1);
  Vector<Int> ant2(nBasl, -1);

  for (Int k=0; k < nAnt; k++) {
    for (Int j=k+1; j < nAnt; j++) {
      // The antenna numbering is one-based for the FORTRAN CLIC solver
      Int index = k * nAnt - k * (k+1) / 2 + j - 1 - k;
      ant1(index) = k + 1;
      ant2(index) = j + 1;
    };
  };

  // Iterate, accumulating the averaged visibility time-series
  Int chunk;
  // Mean reference frequency
  Double meanFreq = 0;
  Int nMeanFreq = 0;

  for (chunk=0, vi.originChunks(); vi.moreChunks(); vi.nextChunk(), chunk++) {

    // Extract the current visibility buffer spectral window id.
    // and number for frequency channels
    Int spwid = vi.spectralWindow();
    Int nChan = vs_->numberChan()(spwid);
    String fieldName = vi.fieldName();

    os << LogIO::NORMAL 
       << "Accumulating data for:  field= " << fieldName 
       << ", spw= " << spwid+1 
       << ", nchan= " << nChan 
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

    // Accumulate mean frequency of the averaged data
    meanFreq += mean(correctedvb.frequency());
    nMeanFreq++;

    // Iterate of the current visibility buffer, accumulating
    // a common time series in the visibility data
    for (Int row=0; row < correctedvb.nRow(); row++) {
      // Antenna numbers
      Int ant1num = correctedvb.antenna1()(row);
      Int ant2num = correctedvb.antenna2()(row);

      // Reject auto-correlation data
      if (correctedvb.antenna1()(row) != correctedvb.antenna2()(row)) {
	// Compute baseline index
	Int baselineIndex = ant1num * nAnt - ant1num * (ant1num + 1) / 2 +
	  ant2num - 1 - ant1num;

	// Weight
	Double rowWeight = correctedvb.weight()(row);
	if (rowWeight > 0) {
	  // Map the current time stamp to a time series index
	  // using 0.1 second precision, i.e. time values within
	  // a tenth of a second of each other will be accumulated
	  // within the same bin.
	  MVTime mvt(correctedvb.time()(row) / C::day);
	  String timeKey = mvt.string(MVTime::TIME, 7);
	  Int timeIndex = 0;
	  // Check the time stamp index to this precision
	  if (timeValueMap.isDefined(timeKey)) {
	    timeIndex = timeValueMap(timeKey);
	  } else {
	    // Create a new time series entry
	    timeIndex = nTimeSeries++;
	    timeValueMap.define(timeKey, timeIndex);
	    timeValues.resize(nTimeSeries, True);
	    timeValues(timeIndex) = correctedvb.time()(row);
	    visTimeSeries.resize(nTimeSeries, True);
	    Complex czero(0,0);
	    visTimeSeries[timeIndex] = new Vector<Complex>(nBasl, czero);
	    modelTimeSeries.resize(nTimeSeries, True);
	    modelTimeSeries[timeIndex] = new Vector<Complex>(nBasl, czero);
	    weightTimeSeries.resize(nTimeSeries, True);
	    weightTimeSeries[timeIndex] = new Vector<Double>(nBasl, 0);
	  };

	  // Accumulate the current visbility row in the common time series
	  (*visTimeSeries[timeIndex])(baselineIndex) +=
	    correctedvb.visibility()(0,row)(0) * rowWeight;
	  (*modelTimeSeries[timeIndex])(baselineIndex) +=
	    corruptedvb.visibility()(0,row)(0) * rowWeight;
	  (*weightTimeSeries[timeIndex])(baselineIndex) += rowWeight;
	};
      };
    };
  }; // for (chunk...) iteration

  // Create amplitude, phase and weight arrays per time-slot
  // and interferometer index in the form required by the
  // GILDAS solver, splinant.
  Int nTimes = timeValueMap.ndefined();

  os << LogIO::NORMAL 
     << "Number of timestamps in data = " << nTimes
     << LogIO::POST;

  Vector<Double> time(nTimes, 0);
  Matrix<Double> amp(nTimes, nBasl, 0);
  Matrix<Double> phase(nTimes, nBasl, 0);
  Matrix<Double> weight(nTimes, nBasl, 0);
  //
  // First create a simple index in order of ascending time ordinate
  Vector<Int> index(nTimes);
  indgen(index);
  for (Int j=0; j < nTimes; j++) {
    for (Int k=j+1; k < nTimes; k++) {
      if (timeValues(index(j)) > timeValues(index(k))) {
	Int swap = index(j);
	index(j) = index(k);
	index(k) = swap;
      };
    };
  };

  // Fill the amplitude, phase and weight arrays with normalized 
  // and scaled vaues from the accumulated time series.
  for (Int t=0; t < nTimes; t++) {
    for (Int baselineIndex=0; baselineIndex < nBasl; ++baselineIndex) {
      // Iterate in ascending time order
      Int indx = index(t);
      // Time
      time(t) = timeValues(indx);

      Double wgt = (*weightTimeSeries[indx])(baselineIndex);
      if (wgt > 0) {
	Complex vis = (*visTimeSeries[indx])(baselineIndex) / 
	  (*modelTimeSeries[indx])(baselineIndex);
	// Amplitude
	if (abs(vis) > 0) {
	  amp(t,baselineIndex) = log(abs(vis));
	};
	// Phase
	Double visPhase = arg(vis);
	
	if(solvePhase_p){
	  visPhase = remainder(visPhase, 2*C::pi);
	  if (visPhase > C::pi) {
	    visPhase -= 2 * C::pi;
	  } else if (visPhase < -C::pi) {
	    visPhase += 2 * C::pi;
	  };
	}
	
	phase(t,baselineIndex) = visPhase;

	// Weight
	weight(t,baselineIndex) = wgt;
      };
    };
  };

  if(solvePhase_p){
    os << LogIO::NORMAL 
       << "Searching for and correcting phase-wraps on each baseline."
       << LogIO::POST;

    for(Int bsInd=0; bsInd < nBasl ; ++bsInd){
      
      Int npoi=numpoint_p; 
      if (npoi < 2) npoi=2;
      
      for (Int k1=npoi; k1 < nTimes-npoi; k1=k1+(npoi/2)){
	Vector<Double> vecAheadPh(npoi), vecBheadPh(npoi);
	for (Int k=0; k < npoi; ++k){
	  vecBheadPh[k]=phase(k1-k-1,bsInd);
	  vecAheadPh[k]=phase(k1+k, bsInd);
	}
	Double medA=median(vecAheadPh);
	Double medB=median(vecBheadPh);
	if((medA- medB) >phaseWrap_p)
	  for(Int k=k1; k< nTimes; ++k)
	    phase(k, bsInd) -= 2*C::pi;
	if((medA-medB) < -phaseWrap_p)
	  for(Int k=k1; k< nTimes; ++k)
	    phase(k, bsInd) += 2*C::pi;
	
      }
      
      if(nTimes >npoi*2){
      
	//the last npoi pts
	Vector<Double> vecAheadPh(npoi), vecBheadPh(npoi);
	for (Int k=0; k <npoi; ++k){
	  vecBheadPh[k]=phase(nTimes-2*npoi+k,bsInd);
	  vecAheadPh[k]=phase(nTimes-npoi+k, bsInd);
	}
	Double medA=median(vecAheadPh);
	Double medB=median(vecBheadPh);
	if((medA- medB) > phaseWrap_p)
	  for(Int k=(nTimes-npoi); k< nTimes; ++k)
	    phase(k, bsInd) -= 2*C::pi;
	if((medA-medB) < -phaseWrap_p)
	  for(Int k=(nTimes-npoi); k< nTimes; ++k)
	    phase(k, bsInd) += 2*C::pi;
	
	// The first npoi points
	
	for (Int k=0; k <npoi; ++k){
	  vecBheadPh[k]=phase(k,bsInd);
	  vecAheadPh[k]=phase(npoi+k, bsInd);
	}
	medA=median(vecAheadPh);
	medB=median(vecBheadPh);
	if((medA- medB) > phaseWrap_p)
	  for(Int k=0; k< npoi; ++k)
	    phase(k, bsInd) += 2*C::pi;
	if((medA-medB) < -phaseWrap_p)
	  for(Int k=0; k< npoi; ++k)
	    phase(k, bsInd) -= 2*C::pi;
	
      }
    }
  }
  // Delete the accumulation buffers
  for (Int k=0; k < nTimes; k++) {
    delete(visTimeSeries[k]);
    delete(modelTimeSeries[k]);
    delete(weightTimeSeries[k]);
  };

  // Compute the number of spline knots
  Vector<Double> knots, ampKnots(1,0.0), phaKnots(1,0.0);
  Int nKnots = getKnots(time, knots);
  
  os << LogIO::NORMAL 
     << "Number of cubic spline knots = " << nKnots
     << LogIO::POST;

  os << LogIO::NORMAL 
     << "Number of cubic spline segments = " << nKnots-7
     << LogIO::POST;

  if (nTimes < (nKnots-2) ) {
    os << LogIO::SEVERE
       << "Insufficient number of timestamps for cubic splines:" << endl 
       << "  nKnots - nTimes > 2  (" << nKnots-nTimes << ")"
       << LogIO::POST;
    os << LogIO::SEVERE
       << "Increase splinetime or reduce preavg (if possible), or " << endl
       << "  solve for ordinary G instead."
       << LogIO::POST;
    return False;
  };

  // Declare work arrays and returned value
  // arrays to be used by the solver
  Matrix<Double> wk1(4, nTimes);
  Matrix<Double> wk2(4*nAnt, nKnots*nAnt);
  Vector<Double> wk3(nKnots*nAnt);
  Vector<Double> rmsfit(nBasl, 0);

  Double dzero(0);
  Matrix<Double> polyCoeffAmp(nAnt,1, dzero);
  Matrix<Double> polyCoeffPhase(nAnt,1, dzero);

  // Fit using spline polynomials (GILDAS splinant)
  Bool dum;
  Int iy;
  // GILDAS solver uses one-relative antenna numbers
  Int refant = refant_p + 1;

  if (solveAmp_p) {
    // Amplitude solution

    os << LogIO::NORMAL 
       << "Fitting amplitude spline."
       << LogIO::POST;

    polyCoeffAmp.resize(nAnt,nKnots);
    polyCoeffAmp=dzero;
    ampKnots.resize(knots.shape());
    ampKnots=knots;
    iy = 1;
    splinant(&iy,
	     &nTimes,
	     &nBasl,
	     ant1.getStorage(dum),
	     ant2.getStorage(dum),
	     &refant,
	     &nKnots,
	     &nAnt,
	     time.getStorage(dum),
	     amp.getStorage(dum),
	     weight.getStorage(dum),
	     knots.getStorage(dum),
	     wk1.getStorage(dum),
	     wk2.getStorage(dum),
	     wk3.getStorage(dum),
	     rmsfit.getStorage(dum),
	     polyCoeffAmp.getStorage(dum));
    
    String logfile;
    //    logfile=(vs_->msName().before("/SELECTED"));
    //    logfile=(logfile.before("/SORTED_TABLE"))+".amp.log";
    logfile=solveTable_p + ".AMP.log";
    writeAsciiLog(logfile, polyCoeffAmp, rmsfit, False);
    plotsolve(time, amp, weight, rmsfit, polyCoeffAmp, False);

  }

  if (solvePhase_p){
    // Phase solution

    os << LogIO::NORMAL 
       << "Fitting phase spline."
       << LogIO::POST;

    polyCoeffPhase.resize(nAnt,nKnots);
    polyCoeffPhase=dzero;
    phaKnots.resize(knots.shape());
    phaKnots=knots;

    iy = 2;
    Matrix<Double> wght(nTimes, nBasl, 1.0);
    splinant(&iy,
	     &nTimes,
	     &nBasl,
	     ant1.getStorage(dum),
	     ant2.getStorage(dum),
	     &refant,
	     &nKnots,
	     &nAnt,
	     time.getStorage(dum),
	     phase.getStorage(dum),
	     wght.getStorage(dum),
	     knots.getStorage(dum),
	     wk1.getStorage(dum),
	     wk2.getStorage(dum),
	     wk3.getStorage(dum),
	     rmsfit.getStorage(dum),
	     polyCoeffPhase.getStorage(dum));

    String logfile;
    //    logfile=(vs_->msName().before("/SELECTED"));
    //    logfile=(logfile.before("/SORTED_TABLE"))+".phase.log";
    logfile=solveTable_p + ".PHASE.log";
    writeAsciiLog(logfile, polyCoeffPhase, rmsfit, True);

    plotsolve(time, phase, weight, rmsfit, polyCoeffPhase, True);

  };

  // Update the output calibration table
  Vector<Int> antId(nAnt);
  indgen(antId);
  Vector<Int> fieldIdKeys = fieldIdRange();
  Vector<String> freqGrpName(nAnt, "");
  Vector<String> polyType(nAnt, "SPLINE");
  Vector<String> polyMode(nAnt, mode_p);
  Vector<Complex> scaleFactor(nAnt, Complex(1,0));
  Vector<String> phaseUnits(nAnt, "RADIANS");
  Vector<Int> refAnt(nAnt, refant_p);
  
  // Use mean frequency as the reference frequency
  if (nMeanFreq > 0) meanFreq = meanFreq / nMeanFreq;
  Vector<MFrequency> refFreq(nAnt, MFrequency(Quantity(meanFreq, "Hz")));

  // If not incremental calibration then create an empty
  // calibration buffer to hold the output calibration solutions
  if (!calBuffer_p) {
    newCalBuffer(fieldIdKeys, antId);
  } else {
    // Force an explicit read to cache for all calibration columns
    // (in case the input calibration table is being overwritten
    // in place)
    calBuffer_p->fillCache();

  };

  // Update the calibration table
  updateCalTable(fieldIdKeys, antId, freqGrpName, polyType, polyMode, 
		 scaleFactor, polyCoeffAmp, polyCoeffPhase, phaseUnits, 
		 ampKnots, phaKnots, refFreq, refAnt);
	
  return True;
};

//----------------------------------------------------------------------------

void GJonesSpline::getThisGain(const VisBuffer& vb,
			       const Bool& forceAntMat,
			       const Bool& doInverse,
			       const Bool& forceIntMat)

{
// Check and refresh the antenna- and baseline-based gain cache as necessary
// Input:
//    vb           const VisBuffer&         Input visibility buffer
//    spw          Int                      Spectral window id. in buffer
//    time         Double                   Time in buffer
// Output to protected data:
//    Various antenna and baseline gain cache arrays.
//
  LogIO os (LogOrigin("GJonesSpline", "getThisGain", WHERE));

  currentSpw_=vb.spectralWindow();
  Double currTimeStamp(vb.time()(0));

  // realize spw mapping:
  currentCalSpw_= (spwMap_(currentSpw_)>0) ? spwMap_(currentSpw_) : currentSpw_;

  // Only re-compute the calibration corrections if the visibility
  // buffer time stamp has changed; else use the cached corrections

  if (currTimeStamp == cacheTimeValid_p) return;
  cacheTimeValid_p = currTimeStamp;

  // Extract the antenna id. range in the visibility buffer
  Vector<Int> antennaIds = vb.antIdRange();
  Double freqRatio=1.0;

  String timeKey;
  Bool isTimeDefined=False;
  if(rawPhaseRemoval_p){
    MVTime mvt(currTimeStamp / C::day);
    timeKey = mvt.string(MVTime::TIME, 7);
    isTimeDefined=timeValueMap_p.isDefined(timeKey);
    //   if(!isTimeDefined) cout << "Time " << timeKey << "is not defined" << endl;
  }


  // References to ease syntax
  Matrix<mjJones2> tAGM1; tAGM1.reference( *thisJonesMat_[currentCalSpw_] );
  Vector<Bool>     tAGOK; tAGOK.reference( *thisJonesOK_[currentCalSpw_] );
  Cube<mjJones4>   tIGM;  tIGM.reference(  *thisMuellerMat_[currentCalSpw_] );

  // Space to hold conjugates for int matrix calculation
  Matrix<mjJones2> tAGM2(1,numberAnt_);
  tAGM2 = mjJones2(Complex(1.0,0.0));


  // Compute antenna-based spline polynomial correction factors

  for (uInt j=0; j < antennaIds.nelements(); j++) {
    Int antId = antennaIds(j);

    // Match this antenna id. and the visibility buffer field id. 
    // in the calibration buffer
    Vector<Int> matchingRows = 
      calBuffer_p->matchAntenna1AndFieldId(antId, vb.fieldId());

    if (matchingRows.nelements() > 0) {
      // Matching calibration solution found
      Int row = matchingRows(0);
      tAGOK(antId) = True;
      Double ampVal = 1.0;
      Double phaseVal = 0.0;
      Complex gain(calBuffer_p->scaleFactor()(row));
      String mode = calBuffer_p->polyMode()(row);

      // Compute the ratio between the calibration solution 
      // reference frequency and the mean observed frequency
      // of the visibility data to be corrected
      IPosition refFreqPos = calBuffer_p->refFreqMeas().shape();
      refFreqPos = 0;
      refFreqPos.setLast(IPosition(1,row));
      MFrequency refFreq = calBuffer_p->refFreqMeas()(refFreqPos);
      Double refFreqHz = refFreq.get("Hz").getValue();
      Double vbFreqHz = mean(vb.frequency());
      freqRatio = abs(refFreqHz) > 0 ? vbFreqHz / refFreqHz : 1.0;
      //      cout << "FreqRation " << freqRatio << endl;
      // Compute amplitude polynomial
      if (mode.contains("AMP") || mode.contains("A&P")) {
	// Extract amplitude spline polynomial knots
	IPosition ampKnotsPos = calBuffer_p->splineKnotsAmp().shape();
	ampKnotsPos = 0;
	Int nAmpKnotsPos = ampKnotsPos.nelements();
	ampKnotsPos[nAmpKnotsPos-1] = row;
	Int nKnots = calBuffer_p->nKnotsAmp()(row);
	Vector<Double> ampKnots(nKnots);
	for (Int k=0; k < nKnots; k++) {
	  ampKnotsPos[nAmpKnotsPos-2] = k;
	  ampKnots(k) = calBuffer_p->splineKnotsAmp()(ampKnotsPos);
	};
	
	// Extract amplitude spline polynomial coefficients
	IPosition ampCoeffPos = calBuffer_p->polyCoeffAmp().shape();
	ampCoeffPos = 0;
	Int nAmpCoeffPos = ampCoeffPos.nelements();
	ampCoeffPos[nAmpCoeffPos-1] = row;
	Int nPoly = calBuffer_p->nPolyAmp()(row);
	Vector<Double> ampCoeff(nPoly);
	for (Int k=0; k < nPoly; k++) {
	  ampCoeffPos[nAmpCoeffPos-2] = k;
	  ampCoeff(k) = calBuffer_p->polyCoeffAmp()(ampCoeffPos);
	};

	// Compute amplitude spline polynomial value
	ampVal *= exp(getSplineVal(currTimeStamp, ampKnots, ampCoeff));
      };

      // Compute phase polynomial
      if (mode.contains("PHAS") || mode.contains("A&P")) {
	if(!rawPhaseRemoval_p || !isTimeDefined){
	// Extract phase spline polynomial knots
	IPosition phaseKnotsPos = calBuffer_p->splineKnotsPhase().shape();
	phaseKnotsPos = 0;
	Int nPhaseKnotsPos = phaseKnotsPos.nelements();
	phaseKnotsPos[nPhaseKnotsPos-1] = row;
	Int nKnots = calBuffer_p->nKnotsPhase()(row);
	Vector<Double> phaseKnots(nKnots);
	for (Int k=0; k < nKnots; k++) {
	  phaseKnotsPos[nPhaseKnotsPos-2] = k;
	  phaseKnots(k) = calBuffer_p->splineKnotsPhase()(phaseKnotsPos);
	};

	// Extract phase spline polynomial coefficients
	IPosition phaseCoeffPos = calBuffer_p->polyCoeffPhase().shape();
	phaseCoeffPos = 0;
	Int nPhaseCoeffPos = phaseCoeffPos.nelements();
	phaseCoeffPos[nPhaseCoeffPos-1] = row;
	Int nPoly = calBuffer_p->nPolyPhase()(row);
	Vector<Double> phaseCoeff(nPoly);
	for (Int k=0; k < nPoly; k++) {
	  phaseCoeffPos[nPhaseCoeffPos-2] = k;
	  phaseCoeff(k) = calBuffer_p->polyCoeffPhase()(phaseCoeffPos);
	};

	// Compute phase spline polynomial value
	phaseVal = getSplineVal(currTimeStamp, phaseKnots, phaseCoeff);
	
	// Handle gildas sign convention on spline phases
	phaseVal = -phaseVal;

	// Scale by the ratio of the observing frequency of the 
	// data to be corrected and the reference frequency of the
	// calibration solution
	phaseVal *= freqRatio;
	}
	else{
	  phaseVal=0;
	  //	  cout << "CHKCACHE In rawphase removal " << endl;
	}
      };

      // Fill antenna-based gain correction cache
      tAGM1(0,antId) = gain * ampVal * Complex(cos(phaseVal), sin(phaseVal));
      tAGOK(antId) = True;
      
      // If requested (correct context), invert:
      if (doInverse) {
	mjJones2 agInv;
	tAGM1(0,antId).inverse(agInv);
	tAGM1(0,antId) = agInv;
      }

      // Form conjugate, too
      tAGM2(0,antId) = tAGM1(0,antId); 
      tAGM2(0,antId).conj();

    } else {
      // No valid calibration solution found
      tAGOK(antId) = False;
    };
  };
  validateJM(currentCalSpw_);

  // Fill interferometer-based gain correction cache
  for (Int iant1=0;iant1<numberAnt_;iant1++) {
    for (Int iant2=iant1;iant2<numberAnt_;iant2++) {
      if (tAGOK(iant1) && tAGOK(iant2)) {
	directProduct(tIGM(0,iant1,iant2),tAGM1(0,iant1),tAGM2(0,iant2));

	// Handle raw phase removal
	//  (this may have cycle slip issues w.r.t. freqRatio application)
	if(rawPhaseRemoval_p && isTimeDefined) {
	  Double phaseval= getRawPhase(iant1,iant2,currTimeStamp)*freqRatio;
	  if (doInverse) phaseval = -phaseval;
	  SquareMatrix<Complex,4> rawphase;
	  rawphase = (Complex(cos(phaseval),sin(phaseval)));
	  tIGM(0,iant1,iant2)*=rawphase;
	}

      } else {
	tIGM(0,iant1,iant2)=mjJones4(Complex(1.0,0.0));
      }
    }
  }
  validateMM(currentCalSpw_);

  return;
};

//----------------------------------------------------------------------------

void GJonesSpline::newCalBuffer (const Vector<Int>& fieldIdKeys,
				 const Vector<Int>& antennaId)
{
// Create and fill and empty output calibration buffer
// Input:
//    fieldIdKeys      const Vec<Int>&        Field id.'s to span
//    antennaId        const Vec<Int>&        Antenna id.'s to span
// Output to private data:
//    calBuffer_p      GJonesSplineMBuf*      Calibration buffer
//
  LogIO os (LogOrigin("GJonesSpline", "newCalBuffer", WHERE));

  // Delete calibration buffer if it already exists
  if (calBuffer_p) delete calBuffer_p;

  // Initialize the GJonesSpline calibration buffer, spanning the 
  // antenna id.'s and field id.'s specified.
  Vector<Int> key(2);
  key(0) = MSC::ANTENNA1;
  key(1) = MSC::FIELD_ID;
  Block<Vector<Int> > keyvals(2);
  keyvals[0] = antennaId;
  keyvals[1] = fieldIdKeys;
  calBuffer_p = new GJonesSplineMBuf(key, keyvals);

  return;
};

//----------------------------------------------------------------------------

Int GJonesSpline::getKnots (const Vector<Double>& times, Vector<Double>& knots)
{
// Compute the number and location of the spline knots
// Input:
//    times        const Vector<Double>&    Vector of time values
// Output:
//    knots        Vector<Double>&          Knot positions
//    getKnots     Int                      Number of knots
//
  LogIO os (LogOrigin("GJonesSpline", "getKnots()", WHERE));

  // Use algorithm in GILDAS, with user-defined timescale
  Int n=times.nelements();
  Int ncenterKnots = Int((times(n-1)-times(0))/splinetime_p);
  Double step = (times(n-1) - times(0)) / (ncenterKnots+1);

  os << LogIO::NORMAL
     << "Gridded splinetime = " << step << " sec."
     << LogIO::POST;

  Int numOfknots = ncenterKnots + 8; 
  //cout << "numOfknots" << numOfknots << endl;
  knots.resize(numOfknots);
  knots(0)=knots(1) = knots(2) = knots(3) = times(0);
  knots(numOfknots-1) = knots(numOfknots-2) = knots(numOfknots-3) =
    knots(numOfknots-4)=times(n-1);
  for(Int k=0; k < ncenterKnots; k++){
    knots(k+4) = times(0)+ (k + 1) * step;
  }

  return numOfknots;
};

//----------------------------------------------------------------------------

void GJonesSpline::updateCalTable (const Vector<Int>& fieldIdKeys,
				   const Vector<Int>& antennaId,
				   const Vector<String>& freqGrpName,
				   const Vector<String>& polyType,
				   const Vector<String>& polyMode,
				   const Vector<Complex>& scaleFactor,
				   const Matrix<Double>& polyCoeffAmp,
				   const Matrix<Double>& polyCoeffPhase,
				   const Vector<String>& phaseUnits,
				   const Vector<Double>& splineKnotsAmp,
				   const Vector<Double>& splineKnotsPhase,
				   const Vector<MFrequency>& refFreq,
				   const Vector<Int>& refAnt)
				   
{
// Update the output calibration table
// Input:
//    fieldIdKeys      const Vec<Int>&        Field id.'s to span
//    antennaId        const Vec<Int>&        Antenna id. for each solution
//    freqGrpName      const Vec<String>&     Freq. group name (per soln.)
//    polyType         const Vec<String>&     Polynomial type (per soln.)
//    polyMode         const Vec<String>&     Polynomial mode (per soln.)
//    scaleFactor      const Vec<Complex>&    Polynomial scale factor 
//                                            (per solution)
//    polyCoeffAmp     const Matrix<Double>&  Polynomial amplitude 
//                                            coefficients (per solution)
//    polyCoeffPhase   const Matrix<Double>&  Polynomial phase coefficients
//                                            (per solution)
//    phaseUnits       const Vec<String>&     Phase units
//    splineKnotsAmp   const Vector<Double>&  Amp. spline knot positions
//                                            (same for all solutions)
//    splineKnotsPhase const Vector<Double>&  Phase spline knot positions
//                                            (same for all solutions)
//    refFreq          const Vec<MFreq>&      Reference freq. (per soln.)
//    refAnt           const Vec<Int>&        Reference antenna (per soln.)
//
  LogIO os (LogOrigin("GJonesSpline", "updateCalTable", WHERE));

  // Add each solution to the calibration buffer
  for (uInt k=0; k < antennaId.nelements(); k++) {
    for (uInt j=0; j < fieldIdKeys.nelements(); j++) {
      // Find matching rows for this antenna and field id.
      Vector<Int> matchingRows = 
	calBuffer_p->matchAntenna1AndFieldId(antennaId(k), fieldIdKeys(j));

      // Update the matching rows in the calibration buffer
      calBuffer_p->fillMatchingRows(matchingRows, freqGrpName(k), polyType(k), 
				    polyMode(k), scaleFactor(k), 
				    polyCoeffAmp.row(k).nelements(),
				    polyCoeffPhase.row(k).nelements(),
				    polyCoeffAmp.row(k), polyCoeffPhase.row(k),
				    phaseUnits(k), splineKnotsAmp.nelements(),
				    splineKnotsPhase.nelements(),
				    splineKnotsAmp, splineKnotsPhase,
				    refFreq(k), refAnt(k));
    };
  };

  // Delete the output calibration table if it already exists
  if (Table::canDeleteTable(solveTable_p)) {
    Table::deleteTable(solveTable_p);
  };

  os << LogIO::NORMAL
     << "Storing solutions in table " << solveTable_p
     << LogIO::POST;

  // Write the calibration buffer to the output calibration table
  Table::TableOption accessMode = Table::New;
  if (Table::isWritable(solveTable_p)) accessMode = Table::Update;
  GJonesSplineTable calTable(solveTable_p, accessMode);
  calBuffer_p->append(calTable);

  return;
};

//----------------------------------------------------------------------------

Double GJonesSpline::getSplineVal (Double x, 
				   Vector<Double>& knots,
				   Vector<Double>& coeff)
{
// Compute a polynomial spline value using the GILDAS routine getbspl
// Input:
//    x             Double                   Value at which to compute spline
//    knots         Vector<Double>&          Knot locations
//    coeff         Vector<Double>&          Spline coefficients
// Output:
//    getSplineVal  Double                   Computed spline value
//
  LogIO os (LogOrigin("GJonesSpline", "getSplineVal()", WHERE));

  // Use GILDAS library routine, getbspl
  Int numOfknots=knots.nelements();
  Int failflag;
  Bool dum;
  Double yval;
  getbspl(&numOfknots, 
	  knots.getStorage(dum),
	  coeff.getStorage(dum),
	  &x,
	  &yval,
	  &failflag);
  return yval;
}

//----------------------------------------------------------------------------
void GJonesSpline::plotsolve(const Vector<Double>& x, 
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
  // phasesoln - either phase or amplitude plot


  LogIO os (LogOrigin("GJonesSpline", "plotsolve()", WHERE));

  String device;
  device = solveTable_p;
  if (phasesoln){
    //    device=(vs_->msName().before("/SELECTED"));
    //    device=(device.before("/SORTED_TABLE"))+".PHASE.ps/cps";
    device = device + ".PHASE.ps/cps";
  }
  else{
    //    device=(vs_->msName().before("/SELECTED"));
    //    device=(device.before("/SORTED_TABLE"))+".AMP.ps/cps";
    device = device + ".AMP.ps/cps";
  }

  os << LogIO::NORMAL 
     << "Generating plot file: " << device
     << LogIO::POST;

  PGPlotterLocal pg(device);
  pg.subp(4,3);
  Vector<Double> knots;
  Int numOfKnots=getKnots(x,knots);
  Int numpoints= max(x.shape().asVector());
  Int numplotpoints=20*numpoints;
  Int num_valid_points=0;
  Vector<Float> x1(numpoints);
  Vector<Float> y1(numpoints);
  Vector<Float> errarray(numpoints); 
  Vector<Double>  xval(numplotpoints);
  Vector<Float> xfloatval(numplotpoints);
  Vector<Float> soly1(numplotpoints);
  Vector<Double> y, weight;
  Double err;
  Int num_ant = vs_->numberAnt();
  Vector<Double> ant1coeff, ant2coeff; 

  //  Float max_data, min_data, max_err;
  //  max_err=max(errall);
  //  max_data=max(yall)+1.5*max_err;
  //  min_data=min(yall)-1.5*max_err;

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
	  x1(num_valid_points)=x(k)-x(0);
	  y1(num_valid_points)=y(k);
	  if(phasesoln==True){
	    y1(num_valid_points)=remainder(y1(num_valid_points), 2*C::pi);
	    y1(num_valid_points)=y1(num_valid_points)/C::pi*180.0;
	    errarray(num_valid_points)=Float(err)*180.0/C::pi;
	  } 
	  else{
	    y1(num_valid_points)=exp(y1(num_valid_points));
	    errarray(num_valid_points)=Float(err)*y1(num_valid_points);
	  }
	  num_valid_points+=1;
	}; // weight(k) > 0
      }; // for k=0
      
      if(num_valid_points == 0){
      	cout << "No valid point on baselines with antennas " 
	     <<  ant1 << " & " << ant2 << endl;
	return;  
      }

      for(Int k=0; k < numplotpoints; k++){
	xval[k]= (k)*((x[numpoints-1]-x[0]))/Double(numplotpoints-1)
	  +(x[0]);	
	
	if(phasesoln==True){
	  soly1(k)=0; 
	  Double xx, yy1, yy2;
	  xx=xval[k];
	  yy1=getSplineVal(xx, knots, ant1coeff);
	  yy2=getSplineVal(xx, knots, ant2coeff);
	  soly1(k)=yy2-yy1;
	  while(soly1(k) > C::pi){
	    soly1(k) -= 2.0*(C::pi);
	  }
	  while(soly1(k) < (-(C::pi))){
	    soly1(k) += 2.0*(C::pi);
	  }
	  soly1(k)=soly1(k)/(C::pi)*180.0;
		      
	}
	else{

	  Double xx, yy1, yy2;
	  xx=xval[k];
	  yy1=getSplineVal(xx, knots, ant1coeff);
	  yy2=getSplineVal(xx, knots, ant2coeff);  
	  soly1[k]=(yy1+yy2);
	  soly1[k]=exp(soly1[k]);
	  
	  
	  
	}
	
	xfloatval(k)=Float(xval(k)-xval(0));
      }


      //cout << " Mean of sol " << mean(soly1) << endl;
      x1.resize(num_valid_points, True);
      y1.resize(num_valid_points, True);
      errarray.resize(num_valid_points, True);
      pg.sci(1);

      Float max_data, min_data, max_err;
      max_err=max(errarray);
      max_data=max(y1);
      min_data=min(y1);
      max_data=max(max_data, max(soly1))+1.5*max_err;
      min_data=min(min_data, min(soly1))-1.5*max_err;

      Float min_x= min(xfloatval);
      Float max_x= max(xfloatval);

      Float edge=(max_x -min_x)*0.05;
      min_x-=edge;
      max_x+=edge;

      pg.sch(1.5);
      //      pg.env(min_x, max_x, min_data, max_data, 0, 0);
      pg.page();
      pg.svp(0.1,0.9,0.15,0.9);
      pg.swin(min_x,max_x,min_data,max_data);
      pg.box("BCNST",0.0,0,"BCNST",0.0,0);

      ostringstream oos;
      if(phasesoln){
	oos << "G SPLINE phase for baseline " << ant1+1 << " & " << ant2+1 ;
	pg.lab("Time in s", "Phase in deg", " ");
	pg.mtxt("T",0.75,0.5,0.5,oos);
      }
      else{
	oos << "G SPLINE amplitude for baseline " << ant1+1 << " & " << ant2+1 ;
	pg.lab("Time in s", "Amplitude", " ");
	pg.mtxt("T",0.75,0.5,0.5,oos);
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

//---------------------------------------------------------------------------

Vector<Int> GJonesSpline::fieldIdRange()
{
// Return all field id.'s in the underlying MS
// Output:
//    fieldIdRange      Vector<Int>        All FIELD_ID's in the MS
//
  // Open the FIELD sub-table
  //  MeasurementSet ms(vs_->msName());

  const ROMSColumns& mscol(vs_->iter().msColumns());
  const ROMSFieldColumns& fldCol(mscol.field());

  // Fill vector containing all field id.'s
  Vector<Int> retval(fldCol.nrow());
  indgen(retval);

  return retval;
};

//----------------------------------------------------------------------------

void GJonesSpline::setRawPhaseVisSet(VisSet& vs){

  Block<Int> columns(0);
  
  rawvs_p = new VisSet (vs, columns, DBL_MAX);
  rawPhaseRemoval_p=True;
  fillRawPhaseBuff();
}

void GJonesSpline::fillRawPhaseBuff(){

  //Make a phase array of BaselineIndex and TimeIndex of raw phases
  LogIO os (LogOrigin("GJonesSpline", "fillRawPhaseBuff()", WHERE));


 // Use the iterator from the underlying visibility set,
  // and attach a visibility data buffer
  VisIter& vi(rawvs_p->iter());
  VisBuffer vb(vi);

  // Initialize stuff
  // SimpleOrderedMap<String,Int> timeValueMap(0);
  Vector<Double> timeValues;
  PtrBlock<Vector<Complex>* > rawVisTimeSeries;
  PtrBlock<Vector<Double>* > weightTimeSeries;
  Int nTimeSeries = 0;

  // Initialize the baseline index
  Int nAnt = vs_->numberAnt();
  Int nBasl = nAnt * (nAnt - 1) / 2;


  // Iterate, accumulating the averaged visibility time-series
  Int chunk;

  for (chunk=0, vi.originChunks(); vi.moreChunks(); vi.nextChunk(), chunk++) {
    // Extract the current visibility buffer spectral window id.
    // and number for frequency channels
    Int spwid = vi.spectralWindow();
    Int nChan = rawvs_p->numberChan()(spwid);
    String fieldName = vi.fieldName();

    os << LogIO::NORMAL << "Slot= " << (chunk+1) << ", field= "
       << fieldName << ", spw= " << spwid+1 
       << ", nchan= " << nChan << LogIO::POST;

    //average the buffer across frequency
   for(vi.origin(); vi.more(); vi++){
 
     vb.visibility()=vb.correctedVisibility();
     vb.freqAverage();

     //     cout << "AFTER freq aver length" << vb.correctedVisibility().shape().asVector() << endl;

    // Iterate of the current visibility buffer, accumulating
    // a common time series in the visibility data
    for (Int row=0; row < vb.nRow(); row++) {
      // Antenna numbers
      Int ant1num = vb.antenna1()(row);
      Int ant2num = vb.antenna2()(row);

      // Reject auto-correlation data
      if (vb.antenna1()(row) != vb.antenna2()(row)) {
	// Compute baseline index
	Int baselineIndex = ant1num * nAnt - ant1num * (ant1num + 1) / 2 +
	  ant2num - 1 - ant1num;

	// Weight
	Double rowWeight = vb.weight()(row);
	if (rowWeight > 0) {
	  // Map the current time stamp to a time series index
	  // using 0.1 second precision, i.e. time values within
	  // a tenth of a second of each other will be accumulated
	  // within the same bin.
	  MVTime mvt(vb.time()(row) / C::day);
	  String timeKey = mvt.string(MVTime::TIME, 7);
	  //	  cout << "FILL Bef timekey "<< timeKey << " ntimeseries " << nTimeSeries << endl;
	  Int timeIndex = 0;
	  // Check the time stamp index to this precision
	  if (timeValueMap_p.isDefined(timeKey)) {
	    timeIndex = timeValueMap_p(timeKey);
	  } else {
	    // Create a new time series entry
	    timeIndex = nTimeSeries++;
	    //	    cout << "FILL timekey "<< timeKey << " index " << timeIndex << endl;
	    timeValueMap_p.define(timeKey, timeIndex);
	    timeValues.resize(nTimeSeries, True);
	    timeValues(timeIndex) = vb.time()(row);
	    rawVisTimeSeries.resize(nTimeSeries, True);
	    Complex czero(0,0);
	    rawVisTimeSeries[timeIndex] = new Vector<Complex>(nBasl, czero);
	    weightTimeSeries.resize(nTimeSeries, True);
	    weightTimeSeries[timeIndex] = new Vector<Double>(nBasl, 0);
	  };

	  //CAUTION going to use corrected
	  // Accumulate the current visbility row in the common time series
	  (*rawVisTimeSeries[timeIndex])(baselineIndex) +=
	    vb.visibility()(0,row)(0) * rowWeight;
	  (*weightTimeSeries[timeIndex])(baselineIndex) += rowWeight;
	};
      };
    };
    };
  }; // for (chunk...) iteration

  Int nTimes = timeValueMap_p.ndefined();
  rawPhase_p.resize(nTimes, nBasl);
  for (Int k=0; k< nTimes; ++k){
    for(Int j=0; j< nBasl; ++j){
      //rawPhase_p(k, j)=arg((*(rawVisTimeSeries[k]))(j));
      rawPhase_p(k,j)=arg(((*rawVisTimeSeries[k])(j))
      		  / ((*weightTimeSeries[k])(j)));

    }

  }



}


Double GJonesSpline::getRawPhase(Int ant1, Int ant2, Double time){

  if (ant1 == ant2) return 0.0;
  Int nAnt = vs_->numberAnt();
  Bool flip=False;
  if(ant1 > ant2){
    Int tmp=ant1;
    ant1=ant2;
    ant2=tmp;
    flip=True;
  }
  Int baselineIndex = ant1 * nAnt - ant1 * (ant1 + 1) / 2 +
                      ant2 - 1 - ant1;

  MVTime mvt(time / C::day);
  String timeKey = mvt.string(MVTime::TIME, 7);
  Int timeIndex=timeValueMap_p(timeKey);
  // if (ant1==0 && ant2==1) 
  //   cout << "phase 1 2 " << rawPhase_p(timeIndex, baselineIndex)*180.0/C::pi << " Time " << timeKey << endl;
  //   cout << " Timekey " << timeKey << "Timeindex " << timeIndex << endl;
  if(flip){ 
    return -rawPhase_p(timeIndex, baselineIndex);
  }
  else {
    return rawPhase_p(timeIndex, baselineIndex);
  }

}


void GJonesSpline::writeAsciiLog(const String& filename, const Matrix<Double>& coeff, const Vector<Double>& rmsFit, Bool phasesoln){

  Int numAnt=coeff.shape().asVector()(0);
  const char* fil=filename.chars();
  ofstream outLog(fil);

  if(phasesoln){
    outLog << "*****PHASE summary********"<< endl;
  }
  else{
    outLog << "*****AMPLITUDE summary********"<< endl;
  }
    outLog << "Antenna based spline coefficients " << endl;
  for (Int k=0; k < numAnt; k++){
    outLog << "  " << k+1 << "   " << coeff.row(k) << endl;
    
  }


  outLog << " RMS Fit per baseline " << endl;
  outLog << "Antenna1      Antenna2      rmsfit" << endl;
  for (Int ant1=0; ant1 < numAnt; ++ant1){
    for (Int ant2=ant1+1; ant2 < numAnt; ++ant2){
      Int basnum=ant1*numAnt-ant1*(ant1+1)/2+ant2-1-ant1;
      outLog << "   " << ant1+1 << "           " << ant2+1 << "            " 
	     << rmsFit[basnum] << endl;  
    }
  }
}

void GJonesSpline::setPhaseWrapHelp(const Int& numpoi, 
				    const Double& phaseWrap){

  numpoint_p=numpoi;
  phaseWrap_p=phaseWrap;


}

} //# NAMESPACE CASA - END


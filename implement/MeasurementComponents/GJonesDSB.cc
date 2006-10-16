//# GJonesDSB.cc: Implementation of GJonesDSB.h
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

#undef USETHIS
#ifdef USETHIS

#include <synthesis/MeasurementComponents/GJonesDSB.h>

#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayUtil.h>
#include <tables/Tables/RefRows.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/sstream.h>
#include <casa/math.h>

#include <casa/System/PGPlotter.h>
#include <calibration/CalTables/GJonesMBuf.h>
#include <calibration/CalTables/GJonesMCol.h>
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

GJonesDSB::GJonesDSB (VisSet& vs) :
  solveTable_p(""),
  append_p(True),
  interval_p(DBL_MAX),
  maskcenter_p(1),
  maskedge_p(5.0),
  mode_p("A&P"),
  refsideband_p("LSB"),
  refant_p(1),
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
//    maskcenter_p      Int                No. of central channels to mask
//                                         during the solution
//    maskedge_p        Float              Fraction of spectrum to mask at
//                                         either edge during solution
//    mode_p            String             Solution mode (e.g. A&P)
//    refsideband_p     String             Reference sideband for gain ratio
//    refant_p          Int                Reference antenna number
//    maskcenterHalf_p  Int                Central mask half-width
//    maskedgeFrac_p    Float              Fractional edge mask
//    applyTable_p      String             Cal. table name containing
//                                         solutions to be applied
//    applySelect_p     String             Selection for the applied
//                                         calibration table
//    applyInterval_p   Double             Interpolation interval
// Output to protected data:
//    numberAnt_        Int                Number of antennas
//    numberSpw_        Int                Number of spectral windows
//
  // Initialize a visibility set for iteration with the
  // specified solution interval (set to DBL_MAX for now
  // to conform with the CLIC default).
  Block<Int> columns(0);
  vs_ = new VisSet (vs, columns, interval_p);

  // Initialize protected TimeVarVisJones data
  numberAnt_ = vs.numberAnt();
  numberSpw_ = vs.numberSpw();
};

//----------------------------------------------------------------------------

void GJonesDSB::setSolver (const Record& solver)
{
// Set the solver parameters
// Input:
//    solver            const Record&      Solver parameters
// Output to private data:
//    solveTable_p      String             Cal. table name for output solutions
//    append_p          Bool               True if appending solutions
//                                         to an existing calibration table
//    interval_p        Double             Solution interval
//    maskcenter_p      Int                No. of central channels to mask
//                                         during the solution
//    maskedge_p        Float              Fraction of spectrum to mask at
//                                         either edge during solution
//    mode_p            String             Solution mode (e.g. A&P)
//    refsideband_p     String             Reference sideband for gain ratio
//    refant_p          Int                Reference antenna number
//    maskcenterHalf_p  Int                Central mask half-width
//    maskedgeFrac_p    Float              Fractional edge mask
//
  // Extract the solver parameters
  if (solver.isDefined("table")) solveTable_p = solver.asString("table");
  if (solver.isDefined("append")) append_p = solver.asBool("append");
  if (solver.isDefined("t")) interval_p = solver.asDouble("t");
  if (solver.isDefined("maskcenter")) maskcenter_p = 
					solver.asInt("maskcenter");
  if (solver.isDefined("maskedge")) maskedge_p = solver.asInt("maskedge");
  if (solver.isDefined("mode")) mode_p = solver.asString("mode");
  if (solver.isDefined("refsideband")) refsideband_p = 
					 solver.asString("refsideband");
  if (solver.isDefined("refant")) refant_p = solver.asInt("refant");

  // Compute derived mask parameters
  maskcenterHalf_p = maskcenter_p / 2;
  maskedgeFrac_p = maskedge_p / 100.0;

  return;
};

//----------------------------------------------------------------------------

void GJonesDSB::setInterpolation (const Record& interpolation)
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
  TimeVarVisJones::initializeCache();
  antGainOKCache_.resize(numberAnt_, numberSpw_);

  // Fill the gain correction cache from the applied cal. table
  load(applyTable_p);

  return;
};

//----------------------------------------------------------------------------


Bool GJonesDSB::solve (VisEquation& me)
{
// Solver for the electronic gain sideband ratios
// Input:
//    me           VisEquation&         Measurement Equation (ME) in
//                                      which this Jones matrix resides
// Output:
//    solve        Bool                 True is solution succeeded
//                                      else False
//
  LogIO os (LogOrigin("GJonesDSB", "solve()", WHERE));

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
  VisBuffer vb(vi);

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

  // Initialize accumulation buffers for amplitude, phase and
  // weight, per sideband and baseline index, as required by 
  // the GILDAS solvers.
  Matrix<Double> amp(nBasl,2), phase(nBasl,2), weight(nBasl,2);
  Matrix<Complex> gain(nAnt,2);
  gain = Complex(1,0);
  Vector<Double> refFreq(2,0);
  Vector<Int> nRefFreq(2,0);

  // By constraint, this solver should see data only from one frequency band
  // (but one or more sidebands). Record the sidebands encountered and
  // their associated frequency groups.
  Vector<String> freqGroup(2,"");
  Vector<Bool> sbFound(2,False);

  // Iterate, accumulating the averaged spectrum for each sideband
  // and baseline index
  Int chunk;
  for (chunk=0, vi.originChunks(); vi.moreChunks(); vi.nextChunk(), chunk++) {

    // Extract the current visibility buffer spectral window id.,
    // sideband and number for frequency channels
    Int spwid = vi.spectralWindow();
    Int sbId = sideBand(spwid);
    freqGroup(sbId) = freqGrpName(spwid);
    Int nChan = vs_->numberChan()(spwid);

    os << LogIO::NORMAL << "Freq. group " << freqGroup(sbId) << ", spw= " 
       << spwid << ", nchan= " << nChan << LogIO::POST;

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

    // Mark sideband found, and accumulate mean frequency
    sbFound(sbId) = True;
    refFreq(sbId) += mean(correctedvb.frequency());
    nRefFreq(sbId)++;

    // Data and model values
    Complex data, model;

    // Compute the amplitude and phase spectrum for this spectral window
    for (Int row=0; row < correctedvb.nRow(); row++) {
      // Antenna numbers
      Int ant1num = correctedvb.antenna1()(row);
      Int ant2num = correctedvb.antenna2()(row);

      // Reject auto-correlation data
      if (correctedvb.antenna1()(row) != correctedvb.antenna2()(row)) {
	// Compute baseline index
	Int baselineIndex = ant1num * nAnt - ant1num * (ant1num + 1) / 2 +
	  ant2num - 1 - ant1num;
	// Loop over the frequency channels
	for (Int chan=0; chan < nChan; chan++) {
	  // Reject masked channels and data with zero weight 
	  if (!maskedChannel(chan, nChan) && (correctedvb.weight()(row) > 0)) {
	    weight(baselineIndex,sbId) = correctedvb.weight()(row);
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
	    phase(baselineIndex,sbId) = static_cast<Double>(meanPhase);

	    // Compute amplitude
	    if (abs(model) > 0) {
	      Float meanAmpl = abs(data) / abs(model);
	      amp(baselineIndex,sbId) = static_cast<Double>(log(meanAmpl));
	    } else {
	      amp(baselineIndex,sbId) = 0;
	      weight(baselineIndex,sbId) = 0;
	    };
	  };
	};
      };
    };

  }; // for (chunk...) iteration

  // Compute an antenna-based average phase for each sideband
  for (Int sbId=0; sbId < 2; sbId++) {
    
    if (sbFound(sbId)) {
      // First solve for antenna-based amplitude factors for this sideband
      //
      // Define arrays to hold the returned antenna-based amplitudes 
      // and their weight
      Vector<Double> avgAntAmp;
      avgAntAmp.resize(nAnt);
      avgAntAmp.set(0.0);
      Vector<Double> ampWgtReturned(nAnt);

      Int errorval;
      Matrix<Double> wk1(nAnt, nAnt);
      Vector<Double> wk2(nBasl);
      // Call the GILDAS solver
      Bool dum;
      ampliant(ant1.getStorage(dum),
	       ant2.getStorage(dum),
	       &nAnt, 
	       &nBasl,
	       amp.column(sbId).getStorage(dum),
	       weight.column(sbId).getStorage(dum),
	       avgAntAmp.getStorage(dum),
	       ampWgtReturned.getStorage(dum),
	       &errorval,
	       wk1.getStorage(dum),
	       wk2.getStorage(dum));
    
      // Now solve for antenna-based phase factors for this sideband
      // reference for each spectral window
      Vector<Double> avgAntPhase;
      avgAntPhase.resize(nAnt);
      avgAntPhase.set(0.0);
      Vector<Double> phaseWgtReturned(nAnt);

      Matrix<Double> wk3(nAnt, nAnt);
      Vector<Double> wk4(nBasl);
      phaseant(ant1.getStorage(dum),
	       ant2.getStorage(dum),
	       &nAnt, 
	       &nBasl,
	       phase.column(sbId).getStorage(dum),
	       weight.column(sbId).getStorage(dum),
	       avgAntPhase.getStorage(dum),
	       phaseWgtReturned.getStorage(dum),
	       &errorval,
	       wk3.getStorage(dum),
	       wk4.getStorage(dum));

      // Compute the reference frequency and reference phasor
      if (nRefFreq(sbId) > 0) refFreq(sbId) = refFreq(sbId) / nRefFreq(sbId);
      for (Int k=0; k < nAnt; k++) {
	gain(k,sbId) = Complex(exp(avgAntAmp(k)),0) * 
	  Complex(cos(avgAntPhase(k)), sin(avgAntPhase(k)));
      };
    };
  };

  // Compute the sideband ratio
  Int sbRefId = refsideband_p.contains("LSB") ? 0 : 1;
  Int sbOtherId = 1 - sbRefId;

  for (Int ant=0; ant < nAnt; ant++) {
    // Deal with all cases of sideband combinations
    // Both LSB and USB
    if (sbFound(0) && sbFound(1)) {
      Complex sbRatio = gain(ant,sbOtherId) / gain(ant,sbRefId);
      gain(ant,sbOtherId) = sbRatio;
      gain(ant,sbRefId) = Complex(1,0);
      // LSB only
    } else if (sbFound(0) && !sbFound(1)) {
      gain(ant,0) = gain(ant,0) / abs(gain(ant,0));
      // USB only
    } else if (sbFound(1) && !sbFound(0)) {
      gain(ant,1) = gain(ant,1) / abs(gain(ant,1));
    };
  };

  // Update the output calibration table
  Vector<Int> antId(nAnt);
  indgen(antId);
  Vector<Int> refAnt(nAnt, refant_p);
  Vector<MFrequency> refFreqMeas(2);
  for (Int j=0; j < 2; j++) {
    refFreqMeas(j) = MFrequency(Quantity(refFreq(j), "Hz"));
  };
  updateCalTable (freqGroup, antId, gain, refFreqMeas, refAnt);

  return True;
};
//----------------------------------------------------------------------------
    
void GJonesDSB::updateCalTable (const Vector<String>& freqGrpName, 
				const Vector<Int>& antennaId,
				const Matrix<Complex>& sideBandRef,
				const Vector<MFrequency>& refFreq, 
				const Vector<Int>& refAnt)
{
// Update the output calibration table to include the current soln. parameters
// Input:
//    freqGrpName     const Vector<String>&     Freq. group name (per sideband)
//    antennaId       const Vector<Int>&        Antenna id. for each soln.
//    sideBandRef     const Matrix<Complex>&    Sideband reference phasor
//                                              (per solution and sideband)
//    refFreq         const Vector<MFrequency>& Sideband reference frequency
//                                              (per solution and sideband)
//    refAnt          const Vector<Int>&        Reference antenna (per soln.)
// Input from private data:
//    solveTable_p    String                    Output calibration table name
//
  // Fill the bandpass solution parameters to a GJones calibration
  // buffer spanning the antenna id.'s and sidebands
  Vector<Int> key(2,0);
  key(0) = MSC::ANTENNA1;
  key(1) = MSC::FREQ_GROUP;
  Vector<Int> freqId(2);
  freqId(0) = freqGrpId(freqGrpName(0));
  freqId(1) = freqGrpId(freqGrpName(1));

  Block<Vector<Int> > keyvals(2);
  keyvals[0] = antennaId;
  keyvals[1] = freqId;
  GJonesMBuf buffer(key, keyvals);

  // Add each solution to the calibration buffer
  // Loop over antenna
  for (uInt k=0; k < antennaId.nelements(); k++) {
    // Loop over sideband
    for (uInt j=0; j < 2; j++) {
      Vector<Int> matchingRows = 
	buffer.matchAntenna1AndFreqGrp(antennaId(k), freqGrpName(j));
      buffer.fillMatchingRows(matchingRows, freqGrpName(j), sideBandRef(k,j), 
			     refFreq(j), refAnt(k));
    };
  };

  // Delete the output calibration table is append not specified
  if (!append_p && Table::canDeleteTable(solveTable_p)) {
    Table::deleteTable(solveTable_p);
  };

  // Append the calibration buffer to the output calibration table
  Table::TableOption accessMode = Table::New;
  if (Table::isWritable(solveTable_p)) accessMode = Table::Update;
  GJonesTable calTable(solveTable_p, accessMode);
  buffer.append(calTable);

  return;
}

//----------------------------------------------------------------------------
    
Bool GJonesDSB::maskedChannel (const Int& chan, const Int& nChan) 
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
  Int midChan = nChan / 2;

  // Check mask at center of spectrum
  if ((chan >= (midChan-maskcenterHalf_p)) &&
      (chan <= (midChan+maskcenterHalf_p))) {
    masked = True;
  };

  // Check mask at edge of spectrum
  if ((chan < (nChan*maskedgeFrac_p)) || (chan > nChan*(1-maskedgeFrac_p))) {
    masked = True;
  };
  return masked;
};

//----------------------------------------------------------------------------
    
void GJonesDSB::load (const String& applyTable) 
{
// Load and cache the polynomial bandpass corrections
// Input:
//    applyTable      const String&            Cal. table to be applied
// Output to protected data:
//    Antenna and interferometer bandpass correction caches 
//    in the GJones parent class.
//
  // Open the GJones calibration table
  GJonesTable calTable(applyTable, Table::Update);

  // Attach a calibration table columns accessor
  GJonesMCol col(calTable);

  // Fill the bandpass correction cache
  Int nrows = calTable.nRowMain();
  antGainOKCache_ = False;

  for (Int row=0; row < nrows; row++) {
    // Antenna id.
    Int antennaId = col.antenna1().asInt(row);

    // Frequency group name
    String freqGrpName = col.freqGrpName().asString(row);

    // Extract the gain correction
    RefRows refRow(row,row);
    Matrix<Complex> calGain = col.gain().getColumnCells(refRow);

    // Loop over all spectral window id.'s in this frequency group
    Vector<Int> spwIds = spwIdsInGroup(freqGrpName);
    for (uInt k=0; k < spwIds.nelements(); k++) {
      Int spwId = spwIds(k);
      // Fill the gain correction cache
      antGainCache_(antennaId,spwId)(0,0) = calGain(0,0);

      // Set flag for valid cache value
      antGainOKCache_(antennaId,spwId) = True;
    };
  };
    
  // Fill the interferometer-based banpass-correction cache (and its
  // matrix inverse for applyInv).
  for (Int ant1=0; ant1 < numberAnt_; ant1++) {
    for (Int ant2=ant1+1; ant2 < numberAnt_; ant2++) {
      for (Int spwId=0; spwId < numberSpw_; spwId++) {
	// Check gain cache flag
	if (antGainOKCache_(ant1,spwId) && antGainOKCache_(ant2,spwId)) {
	  mjJones2 aconj = antGainCache_(ant2,spwId);
	  aconj.conj();
	  // Compute interferometer-based correction
	  directProduct(intGainCache_(ant1,ant2,spwId),
			antGainCache_(ant1,spwId), aconj);
	  intGainCache_(ant1,ant2,spwId).inverse
	    (intGainInvCache_(ant1,ant2,spwId));
	};
      };
    };
  };

  return;
}

//----------------------------------------------------------------------------
    
Double GJonesDSB::meanFrequency (const Vector<Int>& spwid) 
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
  MeasurementSet ms(vs_->msName());
  MSSpectralWindow spectralTable = ms.spectralWindow();
  ROArrayColumn<Double> 
    frequencies(spectralTable,
		MSSpectralWindow::columnName(MSSpectralWindow::CHAN_FREQ));
  ROScalarColumn<Double> 
    totalbw(spectralTable,
	    MSSpectralWindow::columnName(MSSpectralWindow::TOTAL_BANDWIDTH));
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
    
String GJonesDSB::freqGrpName (const Int& spwId) 
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
  MeasurementSet ms(vs_->msName());
  ROMSSpWindowColumns spwCol(ms.spectralWindow());
  return spwCol.freqGroupName().asString(spwId);
}

//----------------------------------------------------------------------------
    
Int GJonesDSB::freqGrpId (const String& freqGrpName) 
{
// Return the frequency group id. for a given frequency group name
// Input:
//    freqGrpName     const String&            Frequency group name
// Input from private data:
//    vs_             VisSet*                  Current visibility set
// Output:
//    freqGrpId       Int                      Frequency group id.
//
  // Initialization
  Bool retval = 0;

  // Open the SPECTRAL_WINDOW sub-table
  MeasurementSet ms(vs_->msName());
  ROMSSpWindowColumns spwCol(ms.spectralWindow());

  // Retrieve the first matching freq. group id. for the freq. group name
  LogicalArray maskArray = (spwCol.freqGroupName().getColumn() == freqGrpName
			    && !spwCol.flagRow().getColumn());
  Vector<Int> rowNo(maskArray.nelements());
  indgen(rowNo);
  MaskedArray<Int> maskedRowNo(rowNo, maskArray);
  Vector<Int> matchingRows = maskedRowNo.getCompressedArray();
  if (matchingRows.nelements() > 0) {
    retval = spwCol.freqGroup().asInt(0);
  };
  return retval;
}

//----------------------------------------------------------------------------
    
Int GJonesDSB::sideBand (const Int& spwId) 
{
// Return the sideband id. for a given spectral window id.
// Input:
//    spwId           const Int&               Spectral window id.
// Input from private data:
//    vs_             VisSet*                  Current visibility set
// Output:
//    sideBand        Int                      Sideband id. [0,1]
//
  // Open a SPECTRAL_WINDOW sub-table index
  MeasurementSet ms(vs_->msName());
  ROMSSpWindowColumns spwCol(ms.spectralWindow());
  return spwCol.netSideband().asInt(spwId);
}

//----------------------------------------------------------------------------
    
Vector<Int> GJonesDSB::spwIdsInGroup (const String& freqGrpName) 
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
  MeasurementSet ms(vs_->msName());
  MSSpWindowIndex spwIndex(ms.spectralWindow());
  return spwIndex.matchFreqGrpName(freqGrpName);
}

//----------------------------------------------------------------------------
    
Vector<Double> GJonesDSB::freqAxis (const Int& spwId) 
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
  MeasurementSet ms(vs_->msName());
  ROMSSpWindowColumns spwCol(ms.spectralWindow());
  Vector<Double> freqVal;
  spwCol.chanFreq().get(spwId, freqVal);
  return freqVal;
}

//----------------------------------------------------------------------------


} //# NAMESPACE CASA - END

#endif


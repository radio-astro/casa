//#---------------------------------------------------------------------------
//# PKSMS2reader.cc: Class to read Parkes Multibeam data from a v2 MS.
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2000-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: PKSMS2reader.cc,v 19.23 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2000/08/03, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/pks/pks_maths.h>
#include <atnf/PKSIO/PKSmsg.h>
#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/PKSMS2reader.h>

#include <casa/stdio.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <tables/Tables.h>

//------------------------------------------------- PKSMS2reader::PKSMS2reader

// Default constructor.

PKSMS2reader::PKSMS2reader()
{
  cMSopen = False;

  // By default, messages are written to stderr.
  initMsg();
}

//------------------------------------------------ PKSMS2reader::~PKSMS2reader

PKSMS2reader::~PKSMS2reader()
{
  close();
}

//--------------------------------------------------------- PKSMS2reader::open

// Open the MS for reading.

Int PKSMS2reader::open(
        const String msName,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra)
{
  // Check that MS is readable.
  if (!MS::isReadable(msName)) {
    return 1;
  }

  if (cMSopen) {
    close();
  }

  cPKSMS  = MeasurementSet(msName);
  cIdx    = 0;
  cNRow   = cPKSMS.nrow();
  cMSopen = True;

  // Lock the table for read access.
  cPKSMS.lock(False);

  // Main MS table and subtable column access.
  ROMSMainColumns         msCols(cPKSMS);
  ROMSDataDescColumns     dataDescCols(cPKSMS.dataDescription());
  ROMSFeedColumns         feedCols(cPKSMS.feed());
  ROMSFieldColumns        fieldCols(cPKSMS.field());
  ROMSPointingColumns     pointingCols(cPKSMS.pointing());
  ROMSPolarizationColumns polarizationCols(cPKSMS.polarization());
  ROMSSourceColumns       sourceCols(cPKSMS.source());
  ROMSSpWindowColumns     spWinCols(cPKSMS.spectralWindow());
  ROMSStateColumns        stateCols(cPKSMS.state());
  ROMSSysCalColumns       sysCalCols(cPKSMS.sysCal());
  ROMSWeatherColumns      weatherCols(cPKSMS.weather());

  // Column accessors for required columns.
  cScanNoCol.reference(msCols.scanNumber());
  cTimeCol.reference(msCols.time());
  cIntervalCol.reference(msCols.interval());

  cFieldIdCol.reference(msCols.fieldId());
  cFieldNameCol.reference(fieldCols.name());

  cSrcIdCol.reference(fieldCols.sourceId());
  cSrcNameCol.reference(sourceCols.name());
  cSrcDirCol.reference(sourceCols.direction());
  cSrcPMCol.reference(sourceCols.properMotion());
  cSrcRestFrqCol.reference(sourceCols.restFrequency());

  cStateIdCol.reference(msCols.stateId());
  cObsModeCol.reference(stateCols.obsMode());

  cDataDescIdCol.reference(msCols.dataDescId());
  cChanFreqCol.reference(spWinCols.chanFreq());

  cWeatherTimeCol.reference(weatherCols.time());
  cTemperatureCol.reference(weatherCols.temperature());
  cPressureCol.reference(weatherCols.pressure());
  cHumidityCol.reference(weatherCols.relHumidity());

  cBeamNoCol.reference(msCols.feed1());
  cPointingCol.reference(pointingCols.direction());
  cSigmaCol.reference(msCols.sigma());
  cNumReceptorCol.reference(feedCols.numReceptors());

  // Optional columns.
  if ((cHaveSrcVel = cPKSMS.source().tableDesc().isColumn("SYSVEL"))) {
    cSrcVelCol.attach(cPKSMS.source(), "SYSVEL");
  }

  if ((cHaveTsys = cPKSMS.sysCal().tableDesc().isColumn("TSYS"))) {
    cTsysCol.attach(cPKSMS.sysCal(), "TSYS");
  }

  if ((cHaveCalFctr = cPKSMS.tableDesc().isColumn("CALFCTR"))) {
    cCalFctrCol.attach(cPKSMS, "CALFCTR");
  }

  if ((cHaveBaseLin = cPKSMS.tableDesc().isColumn("BASELIN"))) {
    cBaseLinCol.attach(cPKSMS, "BASELIN");
    cBaseSubCol.attach(cPKSMS, "BASESUB");
  }

  // Spectral data should always be present.
  haveSpectra = True;
  cFloatDataCol.reference(msCols.floatData());
  cFlagCol.reference(msCols.flag());

  if ((cGetXPol = cPKSMS.isColumn(MSMainEnums::DATA))) {
    if ((cHaveXCalFctr = cPKSMS.tableDesc().isColumn("XCALFCTR"))) {
      cXCalFctrCol.attach(cPKSMS, "XCALFCTR");
    }

    cDataCol.reference(msCols.data());
  }

  // Find which beams are present in the data.
  Vector<Int> beamNos = cBeamNoCol.getColumn();
  Int maxBeamNo = max(beamNos) + 1;
  beams.resize(maxBeamNo);

  beams = False;
  for (uInt irow = 0; irow < beamNos.nelements(); irow++) {
    beams(beamNos(irow)) = True;
  }


  // Number of IFs.
  uInt nIF = dataDescCols.nrow();
  IFs.resize(nIF);
  IFs = True;

  // Number of polarizations and channels in each IF.
  ROScalarColumn<Int> spWinIdCol(dataDescCols.spectralWindowId());
  ROScalarColumn<Int> numChanCol(spWinCols.numChan());

  ROScalarColumn<Int> polIdCol(dataDescCols.polarizationId());
  ROScalarColumn<Int> numPolCol(polarizationCols.numCorr());

  nChan.resize(nIF);
  nPol.resize(nIF);
  for (uInt iIF = 0; iIF < nIF; iIF++) {
    nChan(iIF) = numChanCol(spWinIdCol(iIF));
    nPol(iIF)  = numPolCol(polIdCol(iIF));
  }

  // Cross-polarization data present?
  haveXPol.resize(nIF);
  haveXPol = False;

  if (cGetXPol) {
    for (Int irow = 0; irow < cNRow; irow++) {
      if (cDataCol.isDefined(irow)) {
        Int iIF = cDataDescIdCol(irow);
        haveXPol(iIF) = True;
      }
    }
  }


  // Initialize member data.
  cBeams.assign(beams);
  cIFs.assign(IFs);
  cNChan.assign(nChan);
  cNPol.assign(nPol);
  cHaveXPol.assign(haveXPol);


  // Default channel range selection.
  cStartChan.resize(nIF);
  cEndChan.resize(nIF);
  cRefChan.resize(nIF);

  for (uInt iIF = 0; iIF < nIF; iIF++) {
    cStartChan(iIF) = 1;
    cEndChan(iIF)   = cNChan(iIF);
    cRefChan(iIF)   = cNChan(iIF)/2 + 1;
  }

  Slice all;
  cDataSel.resize(nIF);
  cDataSel = Slicer(all, all);

  cScanNo  = 0;
  cCycleNo = 1;
  cTime    = cTimeCol(0);

  return 0;
}

//---------------------------------------------------- PKSMS2reader::getHeader

// Get parameters describing the data.

Int PKSMS2reader::getHeader(
        String &observer,
        String &project,
        String &antName,
        Vector<Double> &antPosition,
        String &obsType,
        String &bunit,
        Float  &equinox,
        String &dopplerFrame,
        Double &mjd,
        Double &refFreq,
        Double &bandwidth)
{
  if (!cMSopen) {
    return 1;
  }

  // Observer and project.
  ROMSObservationColumns observationCols(cPKSMS.observation());
  observer = observationCols.observer()(0);
  project  = observationCols.project()(0);

  // Antenna name and ITRF coordinates.
  ROMSAntennaColumns antennaCols(cPKSMS.antenna());
  antName = antennaCols.name()(0);
  antPosition = antennaCols.position()(0);

  // Observation type.
  if (cObsModeCol.nrow()) {
    obsType = cObsModeCol(0);
    if (obsType == "\0") obsType = "RF";
  } else {
    obsType = "RF";
  }

  // Brightness units.
  bunit = cPKSMS.unit(MSMainEnums::FLOAT_DATA);

  // Coordinate equinox.
  ROMSPointingColumns pointingCols(cPKSMS.pointing());
  String dirref = pointingCols.direction().keywordSet().asRecord("MEASINFO").
                    asString("Ref");
  sscanf(dirref.chars()+1, "%f", &equinox);

  // Frequency/velocity reference frame.
  ROMSSpWindowColumns spWinCols(cPKSMS.spectralWindow());
  dopplerFrame = MFrequency::showType(spWinCols.measFreqRef()(0));

  // Translate to FITS standard names.
  if (dopplerFrame == "TOPO") {
    dopplerFrame = "TOPOCENT";
  } else if (dopplerFrame == "GEO") {
    dopplerFrame = "GEOCENTR";
  } else if (dopplerFrame == "BARY") {
    dopplerFrame = "BARYCENT";
  } else if (dopplerFrame == "GALACTO") {
    dopplerFrame = "GALACTOC";
  } else if (dopplerFrame == "LGROUP") {
    dopplerFrame = "LOCALGRP";
  } else if (dopplerFrame == "CMB") {
    dopplerFrame = "CMBDIPOL";
  } else if (dopplerFrame == "REST") {
    dopplerFrame = "SOURCE";
  }

  // MJD at start of observation.
  mjd = cTimeCol(0)/86400.0;

  // Reference frequency and bandwidth.
  refFreq   = spWinCols.refFrequency()(0);
  bandwidth = spWinCols.totalBandwidth()(0);

  return 0;
}

//-------------------------------------------------- PKSMS2reader::getFreqInfo

// Get frequency parameters for each IF.

Int PKSMS2reader::getFreqInfo(
        Vector<Double> &startFreq,
        Vector<Double> &endFreq)
{
  uInt nIF = cIFs.nelements();
  startFreq.resize(nIF);
  endFreq.resize(nIF);

  for (uInt iIF = 0; iIF < nIF; iIF++) {
    Vector<Double> chanFreq = cChanFreqCol(iIF);

    Int nChan = chanFreq.nelements();
    startFreq(iIF) = chanFreq(0);
    endFreq(iIF)   = chanFreq(nChan-1);
  }

  return 0;
}

//------------------------------------------------------- PKSMS2reader::select

// Set data selection by beam number and channel.

uInt PKSMS2reader::select(
        const Vector<Bool> beamSel,
        const Vector<Bool> IFsel,
        const Vector<Int>  startChan,
        const Vector<Int>  endChan,
        const Vector<Int>  refChan,
        const Bool getSpectra,
        const Bool getXPol,
        const Int  coordSys)
{
  if (!cMSopen) {
    return 1;
  }

  // Beam selection.
  uInt nBeam = cBeams.nelements();
  uInt nBeamSel = beamSel.nelements();
  for (uInt ibeam = 0; ibeam < nBeam; ibeam++) {
    if (ibeam < nBeamSel) {
      cBeams(ibeam) = beamSel(ibeam);
    } else {
      cBeams(ibeam) = False;
    }
  }

  uInt nIF = cIFs.nelements();
  uInt maxNChan = 0;
  for (uInt iIF = 0; iIF < nIF; iIF++) {
    // IF selection.
    if (iIF < IFsel.nelements()) {
      cIFs(iIF) = IFsel(iIF);
    } else {
      cIFs(iIF) = False;
    }

    if (!cIFs(iIF)) continue;


    // Channel selection.
    if (iIF < startChan.nelements()) {
      cStartChan(iIF) = startChan(iIF);

      if (cStartChan(iIF) <= 0) {
        cStartChan(iIF) += cNChan(iIF);
      } else if (cStartChan(iIF) > Int(cNChan(iIF))) {
        cStartChan(iIF)  = cNChan(iIF);
      }
    }

    if (iIF < endChan.nelements()) {
      cEndChan(iIF) = endChan(iIF);

      if (cEndChan(iIF) <= 0) {
        cEndChan(iIF) += cNChan(iIF);
      } else if (cEndChan(iIF) > Int(cNChan(iIF))) {
        cEndChan(iIF)  = cNChan(iIF);
      }
    }

    if (iIF < refChan.nelements()) {
      cRefChan(iIF) = refChan(iIF);
    } else {
      cRefChan(iIF) = cStartChan(iIF);
      if (cStartChan(iIF) <= cEndChan(iIF)) {
        cRefChan(iIF) += (cEndChan(iIF) - cStartChan(iIF) + 1)/2;
      } else {
        cRefChan(iIF) -= (cStartChan(iIF) - cEndChan(iIF) + 1)/2;
      }
    }

    uInt nChan = abs(cEndChan(iIF) - cStartChan(iIF)) + 1;
    if (maxNChan < nChan) {
      maxNChan = nChan;
    }

    // Inverted Slices are not allowed.
    Slice outPols;
    Slice outChans(min(cStartChan(iIF),cEndChan(iIF))-1, nChan);
    cDataSel(iIF) = Slicer(outPols, outChans);
  }

  // Get spectral data?
  cGetSpectra = getSpectra;

  // Get cross-polarization data?
  cGetXPol = cGetXPol && getXPol;

  // Coordinate system?  (Only equatorial available.)
  cCoordSys = 0;

  return maxNChan;
}

//---------------------------------------------------- PKSMS2reader::findRange

// Find the range of the data in time and position.

Int PKSMS2reader::findRange(
        Int    &nRow,
        Int    &nSel,
        Vector<Double> &timeSpan,
        Matrix<Double> &positions)
{
  if (!cMSopen) {
    return 1;
  }

  nRow = cNRow;

  // Find the number of rows selected.
  nSel = 0;
  Vector<Bool> sel(nRow);
  for (Int irow = 0; irow < nRow; irow++) {
    if ((sel(irow) = cBeams(cBeamNoCol(irow)) &&
                     cIFs(cDataDescIdCol(irow)))) {
      nSel++;
    }
  }

  // Find the time range (s).
  timeSpan.resize(2);
  timeSpan(0) = cTimeCol(0);
  timeSpan(1) = cTimeCol(nRow-1);

  // Retrieve positions for selected data.
  Int isel = 0;
  positions.resize(2,nSel);
  for (Int irow = 0; irow < nRow; irow++) {
    if (sel(irow)) {
      Matrix<Double> pointingDir = cPointingCol(cFieldIdCol(irow));
      positions.column(isel++) = pointingDir.column(0);
    }
  }

  return 0;
}

//--------------------------------------------------------- PKSMS2reader::read

// Read the next data record.

Int PKSMS2reader::read(PKSrecord &pksrec)
{
  if (!cMSopen) {
    return 1;
  }

  // Check for EOF.
  if (cIdx >= cNRow) {
    return -1;
  }

  // Find the next selected beam and IF.
  Int ibeam;
  Int iIF;
  while (True) {
    ibeam = cBeamNoCol(cIdx);
    iIF   = cDataDescIdCol(cIdx);
    if (cBeams(ibeam) && cIFs(iIF)) {
      break;
    }

    // Check for EOF.
    if (++cIdx >= cNRow) {
      return -1;
    }
  }

  // Renumerate scan no. Here still is 1-based
  pksrec.scanNo = cScanNoCol(cIdx) - cScanNoCol(0) + 1;

  if (pksrec.scanNo != cScanNo) {
    // Start of new scan.
    cScanNo  = pksrec.scanNo;
    cCycleNo = 1;
    cTime    = cTimeCol(cIdx);
  }

  Double time = cTimeCol(cIdx);
  pksrec.mjd      = time/86400.0;
  pksrec.interval = cIntervalCol(cIdx);

  // Reconstruct the integration cycle number; due to small latencies the
  // integration time is usually slightly less than the time between cycles,
  // resetting cTime will prevent the difference from accumulating.
  cCycleNo += nint((time - cTime)/pksrec.interval);
  pksrec.cycleNo = cCycleNo;
  cTime = time;

  Int fieldId = cFieldIdCol(cIdx);
  pksrec.fieldName = cFieldNameCol(fieldId);

  Int srcId = cSrcIdCol(fieldId);
  pksrec.srcName = cSrcNameCol(srcId);
  pksrec.srcDir  = cSrcDirCol(srcId);
  pksrec.srcPM   = cSrcPMCol(srcId);

  // Systemic velocity.
  if (!cHaveSrcVel) {
    pksrec.srcVel = 0.0f;
  } else {
    pksrec.srcVel = cSrcVelCol(srcId)(IPosition(1,0));
  }

  // Observation type.
  Int stateId = cStateIdCol(cIdx);
  pksrec.obsType = cObsModeCol(stateId);

  pksrec.IFno = iIF + 1;
  Int nChan = abs(cEndChan(iIF) - cStartChan(iIF)) + 1;

  // Minimal handling on continuum data.
  Vector<Double> chanFreq = cChanFreqCol(iIF);
  if (nChan == 1) {
    cout << "The input is continuum data. "<< endl;
    pksrec.freqInc  = chanFreq(0);
    pksrec.refFreq  = chanFreq(0);
    pksrec.restFreq = 0.0f;
  } else {
    if (cStartChan(iIF) <= cEndChan(iIF)) {
      pksrec.freqInc = chanFreq(1) - chanFreq(0);
    } else {
      pksrec.freqInc = chanFreq(0) - chanFreq(1);
    }

    pksrec.refFreq  = chanFreq(cRefChan(iIF)-1);
    pksrec.restFreq = cSrcRestFrqCol(srcId)(IPosition(1,0));
  }
  pksrec.bandwidth = abs(pksrec.freqInc * nChan);

  pksrec.tcal.resize(cNPol(iIF));
  pksrec.tcal      = 0.0f;
  pksrec.tcalTime  = "";
  pksrec.azimuth   = 0.0f;
  pksrec.elevation = 0.0f;
  pksrec.parAngle  = 0.0f;

  pksrec.focusAxi  = 0.0f;
  pksrec.focusTan  = 0.0f;
  pksrec.focusRot  = 0.0f;

  // Find the appropriate entry in the WEATHER subtable.
  Vector<Double> wTimes = cWeatherTimeCol.getColumn();
  Int weatherIdx;
  for (weatherIdx = wTimes.nelements()-1; weatherIdx >= 0; weatherIdx--) {
    if (cWeatherTimeCol(weatherIdx) <= time) {
      break;
    }
  }

  if (weatherIdx < 0) {
    // No appropriate WEATHER entry.
    pksrec.temperature = 0.0f;
    pksrec.pressure    = 0.0f;
    pksrec.humidity    = 0.0f;
  } else {
    pksrec.temperature = cTemperatureCol(weatherIdx);
    pksrec.pressure    = cPressureCol(weatherIdx);
    pksrec.humidity    = cHumidityCol(weatherIdx);
  }

  pksrec.windSpeed = 0.0f;
  pksrec.windAz    = 0.0f;

  pksrec.refBeam = 0;
  pksrec.beamNo  = ibeam + 1;

  Matrix<Double> pointingDir = cPointingCol(fieldId);
  pksrec.direction = pointingDir.column(0);
  pksrec.pCode = 0;
  pksrec.rateAge = 0.0f;
  uInt ncols = pointingDir.ncolumn();
  if (ncols == 1) {
    pksrec.scanRate = 0.0f;
  } else {
    pksrec.scanRate(0) = pointingDir.column(1)(0);
    pksrec.scanRate(1) = pointingDir.column(1)(1);
  }
  pksrec.paRate = 0.0f;

  // Get Tsys assuming that entries in the SYSCAL table match the main table.
  if (cHaveTsys) {
    Int nTsysColRow = cTsysCol.nrow();
    if (nTsysColRow != cNRow) {
      cHaveTsys=0;
    }
  }
  if (cHaveTsys) {
    cTsysCol.get(cIdx, pksrec.tsys, True);
  } else {
    Int numReceptor;
    cNumReceptorCol.get(0, numReceptor);
    pksrec.tsys.resize(numReceptor);
    pksrec.tsys = 1.0f;
  }
  cSigmaCol.get(cIdx, pksrec.sigma, True);

  // Calibration factors (if available).
  pksrec.calFctr.resize(cNPol(iIF));
  if (cHaveCalFctr) {
    cCalFctrCol.get(cIdx, pksrec.calFctr);
  } else {
    pksrec.calFctr = 0.0f;
  }

  // Baseline parameters (if available).
  if (cHaveBaseLin) {
    pksrec.baseLin.resize(2,cNPol(iIF));
    cBaseLinCol.get(cIdx, pksrec.baseLin);

    pksrec.baseSub.resize(24,cNPol(iIF));
    cBaseSubCol.get(cIdx, pksrec.baseSub);

  } else {
    pksrec.baseLin.resize(0,0);
    pksrec.baseSub.resize(0,0);
  }


  // Get spectral data.
  if (cGetSpectra) {
    Matrix<Float> tmpData;
    Matrix<Bool>  tmpFlag;
    cFloatDataCol.getSlice(cIdx, cDataSel(iIF), tmpData, True);
    cFlagCol.getSlice(cIdx, cDataSel(iIF), tmpFlag, True);

    // Transpose spectra.
    Int nPol = tmpData.nrow();
    pksrec.spectra.resize(nChan, nPol);
    pksrec.flagged.resize(nChan, nPol);
    if (cEndChan(iIF) >= cStartChan(iIF)) {
      // Simple transposition.
      for (Int ipol = 0; ipol < nPol; ipol++) {
        for (Int ichan = 0; ichan < nChan; ichan++) {
          pksrec.spectra(ichan,ipol) = tmpData(ipol,ichan);
          pksrec.flagged(ichan,ipol) = tmpFlag(ipol,ichan);
        }
      }

    } else {
      // Transpose with inversion.
      Int jchan = nChan - 1;
      for (Int ipol = 0; ipol < nPol; ipol++) {
        for (Int ichan = 0; ichan < nChan; ichan++, jchan--) {
          pksrec.spectra(ichan,ipol) = tmpData(ipol,jchan);
          pksrec.flagged(ichan,ipol) = tmpFlag(ipol,jchan);
        }
      }
    }
  }

  // Get cross-polarization data.
  if (cGetXPol) {
    if (cHaveXCalFctr) {
      cXCalFctrCol.get(cIdx, pksrec.xCalFctr);
    } else {
      pksrec.xCalFctr = Complex(0.0f, 0.0f);
    }

    cDataCol.get(cIdx, pksrec.xPol, True);

    if (cEndChan(iIF) < cStartChan(iIF)) {
      Complex ctmp;
      Int jchan = nChan - 1;
      for (Int ichan = 0; ichan < nChan/2; ichan++, jchan--) {
        ctmp = pksrec.xPol(ichan);
        pksrec.xPol(ichan) = pksrec.xPol(jchan);
        pksrec.xPol(jchan) = ctmp;
      }
    }
  }

  cIdx++;

  return 0;
}

//-------------------------------------------------------- PKSMS2reader::close

// Close the MS.

void PKSMS2reader::close()
{
  cPKSMS = MeasurementSet();
  cMSopen = False;
}

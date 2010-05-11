//#---------------------------------------------------------------------------
//# PKSMS2reader.cc: Class to read Parkes Multibeam data from a v2 MS.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2008
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
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
//#---------------------------------------------------------------------------
//# Original: 2000/08/03, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------


// AIPS++ includes.
#include <casa/stdio.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Slice.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <tables/Tables.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/BasicMath/Math.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Sort.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasRef.h>


// Parkes includes.
#include <atnf/pks/pks_maths.h>
#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/PKSMS2reader.h>


//------------------------------------------------- PKSMS2reader::PKSMS2reader

// Default constructor.

PKSMS2reader::PKSMS2reader()
{
  cMSopen = False;
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
        const String antenna, 
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

  // data selection by antenna
  if ( antenna.length() == 0 ) {
    cAntId.resize( 1 ) ;
    cAntId[0] = 0 ;
  }
  else {
    setupAntennaList( antenna ) ;
    if ( cAntId.size() > 1 ) {
      LogIO os( LogOrigin( "PKSMS2reader", "open()", WHERE ) ) ;
      os << LogIO::WARN << "PKSMS2reader is not ready for multiple antenna selection. Use first antenna id " << cAntId[0] << "."<< LogIO::POST ;
      Int tmp = cAntId[0] ;
      cAntId.resize( 1 ) ;
      cAntId[0] = tmp ;
    }
    stringstream ss ;
    ss << "SELECT FROM $1 WHERE ANTENNA1 == ANTENNA2 && ANTENNA1 IN [" ;
    for ( uInt i = 0 ; i < cAntId.size() ; i++ ) {
      ss << cAntId[i] ;
      if ( i == cAntId.size()-1 ) {
        ss << "]" ;
      }
      else {
        ss << "," ;
      }
    }
    string taql = ss.str() ;
    //cerr << "taql = " << taql << endl ;
    cPKSMS = MeasurementSet( tableCommand( taql, cPKSMS ) ) ;
  }

  // taql access to the syscal table
  cHaveSysCal = False;
  if (cHaveSysCal=Table::isReadable(cPKSMS.sysCalTableName())) {
    cSysCalTab = Table(cPKSMS.sysCalTableName());
  } 

  // Lock the table for read access.
  cPKSMS.lock(False);

  cIdx    = 0;
  lastmjd = 0.0;
  cNRow   = cPKSMS.nrow();
  cMSopen = True;

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
  ROMSAntennaColumns      antennaCols(cPKSMS.antenna());

  // Column accessors for required columns.
  cScanNoCol.reference(msCols.scanNumber());
  cTimeCol.reference(msCols.time());
  cIntervalCol.reference(msCols.interval());

  cFieldIdCol.reference(msCols.fieldId());
  cFieldNameCol.reference(fieldCols.name());
  cFieldDelayDirCol.reference(fieldCols.delayDir());

  cSrcIdCol.reference(fieldCols.sourceId());
  cSrcId2Col.reference(sourceCols.sourceId());
  cSrcNameCol.reference(sourceCols.name());
  cSrcDirCol.reference(sourceCols.direction());
  cSrcPMCol.reference(sourceCols.properMotion());
  cSrcRestFrqCol.reference(sourceCols.restFrequency());

  cStateIdCol.reference(msCols.stateId());
  cObsModeCol.reference(stateCols.obsMode());
  cCalCol.reference(stateCols.cal());
  cSigStateCol.reference(stateCols.sig());
  cRefStateCol.reference(stateCols.ref());

  cDataDescIdCol.reference(msCols.dataDescId());
  cSpWinIdCol.reference(dataDescCols.spectralWindowId());
  cChanFreqCol.reference(spWinCols.chanFreq());
  cTotBWCol.reference(spWinCols.totalBandwidth());

  cWeatherTimeCol.reference(weatherCols.time());
  cTemperatureCol.reference(weatherCols.temperature());
  cPressureCol.reference(weatherCols.pressure());
  cHumidityCol.reference(weatherCols.relHumidity());

  cBeamNoCol.reference(msCols.feed1());
  cPointingCol.reference(pointingCols.direction());
  cPointingTimeCol.reference(pointingCols.time());
  cSigmaCol.reference(msCols.sigma());
  cNumReceptorCol.reference(feedCols.numReceptors());

  // Optional columns.
  cHaveTsys = False;
  cHaveTcal = False;
  if ((cHaveSrcVel = cPKSMS.source().tableDesc().isColumn("SYSVEL"))) {
    cSrcVelCol.attach(cPKSMS.source(), "SYSVEL");
  }

  if (cHaveSysCal && (cHaveTsys = cPKSMS.sysCal().tableDesc().isColumn("TSYS"))) {
    cTsysCol.attach(cPKSMS.sysCal(), "TSYS");
  }
  
  if (cHaveSysCal && (cHaveTcal = cPKSMS.sysCal().tableDesc().isColumn("TCAL"))) {
    cTcalCol.attach(cPKSMS.sysCal(), "TCAL");
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
  cHaveDataCol = False;
  cHaveCorrectedDataCol = False;
  ROMSObservationColumns observationCols(cPKSMS.observation());
  //String telName = observationCols.telescopeName()(0);
  cTelName = observationCols.telescopeName()(0);
  //cATF = cTelName.contains("ATF");
  //cOSF = cTelName.contains("OSF");
  //cALMA = cTelName.contains("ALMA");
  cALMA = cTelName.contains("ATF")||cTelName.contains("OSF")||
           cTelName.contains("ALMA");

  if (cHaveDataCol = cPKSMS.isColumn(MSMainEnums::DATA)) { 
    if (cALMA) {
      //try to read a single baseline interferometeric data 
      //and treat it as single dish data
      //maybe extended for ALMA commissioning later
      cDataCol.reference(msCols.data());
      if (cHaveCorrectedDataCol = cPKSMS.isColumn(MSMainEnums::CORRECTED_DATA)) { 
        //cerr<<"Do have CORRECTED_DATA column"<<endl;
        cCorrectedDataCol.reference(msCols.correctedData());
      }
    }
  }
  else {
    cFloatDataCol.reference(msCols.floatData());
  }
  cFlagCol.reference(msCols.flag());
  cFlagRowCol.reference(msCols.flagRow());

  if (cGetXPol = (cPKSMS.isColumn(MSMainEnums::DATA) && (!cALMA))) {
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
  //uInt nIF = dataDescCols.nrow();
  uInt nIF =spWinCols.nrow();
  Vector<Int> spWinIds = cSpWinIdCol.getColumn() ;
  IFs.resize(nIF);
  IFs = True;
  for ( Int ispw = 0 ; ispw < nIF ; ispw++ ) {
    if ( allNE( ispw, spWinIds ) ) {
      IFs(ispw) = False ;
    }
  }

  // Number of polarizations and channels in each IF.
  ROScalarColumn<Int> numChanCol(spWinCols.numChan());

  ROScalarColumn<Int> polIdCol(dataDescCols.polarizationId());
  ROScalarColumn<Int> numPolCol(polarizationCols.numCorr());

  nChan.resize(nIF);
  nPol.resize(nIF);
  for (uInt iIF = 0; iIF < nIF; iIF++) {
    if ( IFs(iIF) ) {
      nChan(iIF) = numChanCol(cSpWinIdCol(iIF)) ;
      nPol(iIF) = numPolCol(polIdCol(iIF)) ;
    }
    else {
      nChan(iIF) = 0 ;
      nPol(iIF) = 0 ;
    }
  }

  // Cross-polarization data present?
  haveXPol.resize(nIF);
  haveXPol = False;

  if (cGetXPol && !(cALMA)) {
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
        // before merge...
        //String &obsMode, 
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
  //antName = antennaCols.name()(0);
  antName = antennaCols.name()(cAntId[0]);
  if (cALMA) {
     antName = cTelName + "-" + antName;
  }
  //antPosition = antennaCols.position()(0);
  antPosition = antennaCols.position()(cAntId[0]);

  // Observation type.
  if (cObsModeCol.nrow()) {
    obsType = cObsModeCol(0);
    if (obsType == "\0") obsType = "RF";
  } else {
    obsType = "RF";
  }

  bunit = "";
  if (cHaveDataCol) {
    const TableRecord& keywordSet2
       = cDataCol.columnDesc().keywordSet();
    if(keywordSet2.isDefined("UNIT")) {
      bunit = keywordSet2.asString("UNIT");
    }
  } else {
    const TableRecord& keywordSet
       = cFloatDataCol.columnDesc().keywordSet();
    if(keywordSet.isDefined("UNIT")) {
      bunit = keywordSet.asString("UNIT");
    }
  }

/***
  const TableRecord& keywordSet
       = cFloatDataCol.columnDesc().keywordSet();
  if(keywordSet.isDefined("UNIT")) {
    fluxunit = keywordSet.asString("UNIT");
  }
***/
  // Coordinate equinox.
  ROMSPointingColumns pointingCols(cPKSMS.pointing());
  String dirref = pointingCols.direction().keywordSet().asRecord("MEASINFO").
                    asString("Ref");
  cDirRef = dirref;
  if (dirref =="AZELGEO" || dirref == "AZEL") {
     dirref = "J2000";
  }
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
        const Bool getFeedPos,
        const Bool getPointing,
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

  // Get feed positions?  (Not available.)
  cGetFeedPos = False;

  // Get Pointing data (for MS)
  cGetPointing = getPointing;

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

/**
Int PKSMS2reader::read(
        Int             &scanNo,
        Int             &cycleNo,
        Double          &mjd,
        Double          &interval,
        String          &fieldName,
        String          &srcName,
        Vector<Double>  &srcDir,
        Vector<Double>  &srcPM,
        Double          &srcVel,
        String          &obsMode,
        Int             &IFno,
        Double          &refFreq,
        Double          &bandwidth,
        Double          &freqInc,
        Vector<Double>  &restFreq,
        Vector<Float>   &tcal,
        String          &tcalTime,
        Float           &azimuth,
        Float           &elevation,
        Float           &parAngle,
        Float           &focusAxi,
        Float           &focusTan,
        Float           &focusRot,
        Float           &temperature,
        Float           &pressure,
        Float           &humidity,
        Float           &windSpeed,
        Float           &windAz,
        Int             &refBeam,
        Int             &beamNo,
        Vector<Double>  &direction,
        Vector<Double>  &scanRate,
        Vector<Float>   &tsys,
        Vector<Float>   &sigma,
        Vector<Float>   &calFctr,
        Matrix<Float>   &baseLin,
        Matrix<Float>   &baseSub,
        Matrix<Float>   &spectra,
        Matrix<uChar>   &flagged,
	uInt            &flagrow,
        Complex         &xCalFctr,
        Vector<Complex> &xPol)
**/
Int PKSMS2reader::read(PKSrecord &pksrec)
{
  LogIO os( LogOrigin( "PKSMS2reader", "read()", WHERE ) ) ;

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
  Int iDataDesc;

  while (True) {
    ibeam = cBeamNoCol(cIdx);
    iDataDesc   = cDataDescIdCol(cIdx);
    iIF   =cSpWinIdCol(iDataDesc);
    if (cBeams(ibeam) && cIFs(iIF)) {
      break;
    }

    // Check for EOF.
    if (++cIdx >= cNRow) {
      return -1;
    }
  }
  // Renumerate scan no. Here still is 1-based
  //scanNo = cScanNoCol(cIdx) - cScanNoCol(0) + 1;
  //scanNo = cScanNoCol(cIdx);
  pksrec.scanNo = cScanNoCol(cIdx);

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
  //For source with multiple spectral window setting, this is not
  // correct. Source name of srcId may not be at 'srcId'th row of SrcNameCol
  //srcName = cSrcNameCol(srcId);
  for (uInt irow = 0; irow < cSrcId2Col.nrow(); irow++) {
    if (cSrcId2Col(irow) == srcId) {
      //srcName = cSrcNameCol(irow);
      pksrec.srcName = cSrcNameCol(irow);
    }
  }

  pksrec.srcDir  = cSrcDirCol(srcId);
  pksrec.srcPM   = cSrcPMCol(srcId);

  // Systemic velocity.
  if (!cHaveSrcVel || cALMA) {
    pksrec.srcVel = 0.0f;
  } else {
    pksrec.srcVel = cSrcVelCol(srcId)(IPosition(1,0));
  }

  ROMSAntennaColumns antennaCols(cPKSMS.antenna());
  //String telescope = antennaCols.name()(0);
  String telescope = antennaCols.name()(cAntId[0]);
  Bool cGBT = telescope.contains("GBT");
  //Bool cPM = telescope.contains("PM"); // ACA TP antenna
  //Bool cDV = telescope.contains("DV"); // VERTEX
  //Bool cCM = telescope.contains("CM"); // ACA 7m antenna
  //Bool cALMA = cPM || cDV || cCM ;
  // Observation type.
  // check if State Table exist
  //Bool cHaveStateTab=Table::isReadable(cPKSMS.stateTableName());
  Int stateId = 0;
  Int StateNRow = 0;
  StateNRow=cObsModeCol.nrow();
  if (Table::isReadable(cPKSMS.stateTableName())) {
        pksrec.obsType = " ";
    if (StateNRow > 0) {
      stateId = cStateIdCol(cIdx);
      if (stateId == -1) {
        //pksrec.obsType = " ";
      } else {
        pksrec.obsType = cObsModeCol(stateId);
        Bool sigState =cSigStateCol(stateId);
        Bool refState =cRefStateCol(stateId);
        //DEBUG
        //cerr <<"stateid="<<stateId<<" obsmode="<<pksrec.obsType<<endl;
        if (cGBT) {
          // split the obsType string and append a proper label 
          // (these are GBT specific)
          int epos = pksrec.obsType.find_first_of(':');
          int nextpos = pksrec.obsType.find_first_of(':',epos+1);
          string obsMode1 = pksrec.obsType.substr(0,epos);
          string obsMode2 = pksrec.obsType.substr(epos+1,nextpos-epos-1);
     
          //cerr <<"obsMode2= "<<obsMode2<<endl;
          if (!pksrec.srcName.contains("_ps") 
              &&!pksrec.srcName.contains("_psr")
              &&!pksrec.srcName.contains("_nod")
              &&!pksrec.srcName.contains("_fs")
              &&!pksrec.srcName.contains("_fsr")) {
            // if Nod mode observation , append '_nod'
            if (obsMode1 == "Nod") {
              //pksrec.srcName.append("_nod");
              pksrec.srcType = SrcType::NOD ;
            } else if (obsMode1 == "OffOn") {
            // for GBT position switch observations (OffOn or OnOff) 
              //if (obsMode2 == "PSWITCHON") pksrec.srcName.append("_ps");
              //if (obsMode2 == "PSWITCHOFF") pksrec.srcName.append("_psr");
              if (obsMode2 == "PSWITCHON") pksrec.srcType = SrcType::PSON ;
              if (obsMode2 == "PSWITCHOFF") pksrec.srcType = SrcType::PSOFF ;
            } else {
              if (obsMode2 == "FSWITCH") {
              // for GBT frequency switch mode
                //if (sigState) pksrec.srcName.append("_fs");
                //if (refState) pksrec.srcName.append("_fsr");
                if (sigState) pksrec.srcType = SrcType::FSON ;
                if (refState) pksrec.srcType = SrcType::FSOFF ;
              }
            } 
          }
        }
	else if (cALMA) {
	  // ALMA tag
          // split the obsType string and append a proper label
          string substr[1] ;
          int numSubstr = split( pksrec.obsType, substr, 1, "," );
          String obsType = String( substr[0] );
          int epos = obsType.find_first_of('.');
          int nextpos = obsType.find_first_of('.',epos+1);
          string obsMode1 = obsType.substr(0,epos);
          string obsMode2 = obsType.substr(epos+1,nextpos-epos-1);
     
          //cerr <<"obsMode2= "<<obsMode2<<endl;
          // Current OBS_MODE format:
          //
          //     ON: OBSERVE_TARGET.ON_SOURCE
          //    OFF: OBSERVE_TARGET.OFF_SOURCE
          //
          if (obsMode1 == "OBSERVE_TARGET") {
            //if (obsMode2 == "ON_SOURCE") pksrec.srcName.append("_pson");
            //if (obsMode2 == "OFF_SOURCE") pksrec.srcName.append("_psoff");
            if (obsMode2 == "ON_SOURCE") pksrec.srcType = SrcType::PSON ;
            if (obsMode2 == "OFF_SOURCE") pksrec.srcType = SrcType::PSOFF ;
          } 
	}
      } 
    }
  }
  // CAL state 
  // this should be apply just for GBT data?
  Double Cal;
  if (stateId==-1 || StateNRow==0) {
    Cal = 0;
  } else { 
    Cal = cCalCol(stateId);
  }
  if (cGBT) {
    if (Cal > 0 && !pksrec.srcName.contains("_calon")) {
      //pksrec.srcName.append("_calon");
      if ( pksrec.srcType == SrcType::NOD )
        pksrec.srcType = SrcType::NODCAL ;
      else if ( pksrec.srcType == SrcType::PSON ) 
        pksrec.srcType = SrcType::PONCAL ;
      else if ( pksrec.srcType == SrcType::PSOFF )
        pksrec.srcType = SrcType::POFFCAL ;
      else if ( pksrec.srcType == SrcType::FSON )
        pksrec.srcType = SrcType::FONCAL ;
      else if ( pksrec.srcType == SrcType::FSOFF )
        pksrec.srcType = SrcType::FOFFCAL ;
      else
        pksrec.srcName.append("_calon");
    }
  }

  pksrec.IFno = iIF + 1;
  Int nChan = abs(cEndChan(iIF) - cStartChan(iIF)) + 1;
  
  // Minimal handling on continuum data.
  Vector<Double> chanFreq = cChanFreqCol(iIF);
  pksrec.nchan = nChan;
  if (nChan == 1) {
    //pksrec.freqInc  = chanFreq(0);
    pksrec.freqInc  = cTotBWCol(iIF); 
    pksrec.refFreq  = chanFreq(0);
    pksrec.restFreq.resize(1);
    pksrec.restFreq[0] = 0.0f;
  } else {
  
    if (cStartChan(iIF) <= cEndChan(iIF)) {
      pksrec.freqInc = chanFreq(1) - chanFreq(0);
    } else {
      pksrec.freqInc = chanFreq(0) - chanFreq(1);
    }

    pksrec.refFreq  = chanFreq(cRefChan(iIF)-1);

    Bool HaveSrcRestFreq= cSrcRestFrqCol.isDefined(srcId);
    if (HaveSrcRestFreq) {
      //restFreq = cSrcRestFrqCol(srcId)(IPosition(1,0));
      //restFreq = cSrcRestFrqCol(srcId);
      pksrec.restFreq = cSrcRestFrqCol(srcId);
    } else {
      pksrec.restFreq.resize(1);
      pksrec.restFreq[0] = 0.0f;
    }
  }
  //pksrec.bandwidth = abs(pksrec.freqInc * nChan);
  pksrec.bandwidth = abs(cTotBWCol(0));

  pksrec.tcal.resize(cNPol(iIF));
  pksrec.tcal      = 0.0f;
  pksrec.tcalTime  = "";
//  pksrec.azimuth   = 0.0f;
//  pksrec.elevation = 0.0f;
  pksrec.parAngle  = 0.0f;

  pksrec.focusAxi  = 0.0f;
  pksrec.focusTan  = 0.0f;
  pksrec.focusRot  = 0.0f;

  // Find the appropriate entry in the WEATHER subtable.
  //Bool cHaveStateTab=Table::isReadable(cPKSMS.stateTableName());
  Bool cHaveWeatherTab = Table::isReadable(cPKSMS.weatherTableName());
  Int weatherIdx=-1;
  if (cHaveWeatherTab) {
    Vector<Double> wTimes = cWeatherTimeCol.getColumn();
    for (weatherIdx = wTimes.nelements()-1; weatherIdx >= 0; weatherIdx--) {
      if (cWeatherTimeCol(weatherIdx) <= time) {
        break;
      }
    }
  }

  if (weatherIdx < 0 || !cHaveWeatherTab) {
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

  //pointing/azel
  //MVPosition mvpos(antennaCols.position()(0));
  MVPosition mvpos(antennaCols.position()(cAntId[0]));
  MPosition mp(mvpos); 
  Quantum<Double> qt(time,"s");
  MVEpoch mvt(qt);
  MEpoch me(mvt);
  MeasFrame frame(mp, me);
  MDirection md;
  pksrec.pCode = 0;
  pksrec.rateAge = 0.0f;
  pksrec.paRate = 0.0f;
  if (cGetPointing) {
    //cerr << "get pointing data ...." << endl;
    Vector<Double> pTimes = cPointingTimeCol.getColumn();
    ROScalarColumn<Int> pAntIdCol ;
    pAntIdCol.attach( cPKSMS.pointing(), "ANTENNA_ID" ) ;
    Vector<Int> antIds = pAntIdCol.getColumn() ;
    Int PtIdx=-1;
    for (PtIdx = pTimes.nelements()-1; PtIdx >= 0; PtIdx--) {
      if ( (cPointingTimeCol(PtIdx) <= time) && antIds(PtIdx) == cAntId[0] ) {
        break;
      }
    }
    if ( PtIdx == -1 ) {
      PtIdx = 0 ;
    }
    //cerr << "got index=" << PtIdx << endl;
    Matrix<Double> pointingDir = cPointingCol(PtIdx);

    ROMSPointingColumns PtCols(cPKSMS.pointing());
    Vector<MDirection> vmd(1);
    PtCols.directionMeasCol().get(PtIdx,vmd);
    md = vmd[0];
    // put J2000 coordinates in "direction" 
    if (cDirRef =="J2000") {
      pksrec.direction = pointingDir.column(0);
    }
    else {
      pksrec.direction =
        MDirection::Convert(md, MDirection::Ref(MDirection::J2000,
                                                frame)
                            )().getAngle("rad").getValue();
      
    }
    uInt ncols = pointingDir.ncolumn();
    if (ncols == 1) {
      pksrec.scanRate = 0.0f;
    } else {
      pksrec.scanRate(0) = pointingDir.column(1)(0);
      pksrec.scanRate(1) = pointingDir.column(1)(1);
    }
  }
  else {
  // Get direction from FIELD table 
  // here, assume direction to be the field direction not pointing
    Matrix<Double> delayDir = cFieldDelayDirCol(fieldId);
    pksrec.direction = delayDir.column(0);
    uInt ncols = delayDir.ncolumn();
    if (ncols == 1) {
      pksrec.scanRate = 0.0f;
    } else {
      pksrec.scanRate(0)  = delayDir.column(1)(0);
      pksrec.scanRate(1)  = delayDir.column(1)(1);
    }
  }
  // caluculate azimuth and elevation
  // first, get the reference frame 
 /**
  MVPosition mvpos(antennaCols.position()(0));
  MPosition mp(mvpos); 
  Quantum<Double> qt(time,"s");
  MVEpoch mvt(qt);
  MEpoch me(mvt);
  MeasFrame frame(mp, me);
  **/
  //
  ROMSFieldColumns fldCols(cPKSMS.field());
  Vector<MDirection> vmd(1);
  //MDirection md;
  fldCols.delayDirMeasCol().get(fieldId,vmd);
  md = vmd[0];
  //Vector<Double> dircheck = md.getAngle("rad").getValue();
  //cerr<<"dircheck="<<dircheck<<endl;

  Vector<Double> azel =
        MDirection::Convert(md, MDirection::Ref(MDirection::AZEL,
                                                frame)
                            )().getAngle("rad").getValue();
  //cerr<<"azel="<<azel<<endl;
  pksrec.azimuth = azel[0];
  pksrec.elevation = azel[1];

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

  //get Tcal if available
  if (cHaveTcal) {
    Int nTcalColRow = cTcalCol.nrow();
    uInt nBeam = cBeams.nelements();
    uInt nIF = cIFs.nelements();
    uInt nrws = nBeam * nIF;
    if (nTcalColRow > 0) {  
    // find tcal match with the data with the data time stamp
      Double mjds = pksrec.mjd*(24*3600);
      Double dtcalTime;
      if ( pksrec.mjd > lastmjd || cIdx==0 ) {
        //Table tmptab = cSysCalTab(near(cSysCalTab.col("TIME"),mjds));
        tmptab = cSysCalTab(near(cSysCalTab.col("TIME"),mjds), nrws);
        //DEBUG
        //if (cIdx == 0) {
        //  cerr<<"inital table retrieved"<<endl;
        //}
       
      }

      if (nBeam == 1) {
        tmptab2 = tmptab( tmptab.col("SPECTRAL_WINDOW_ID") == iIF, 1);
      } else {
        tmptab2 = tmptab( tmptab.col("SPECTRAL_WINDOW_ID") == iIF && 
                              tmptab.col("FEED_ID") == ibeam , 1);
      }
      //cerr<<"first subtab rows="<<tmptab.nrow()<<endl;
      int syscalrow = tmptab2.nrow();
      ROArrayColumn<Float> tcalCol(tmptab2, "TCAL");
      ROScalarColumn<Double> tcalTimeCol(tmptab2, "TIME");
      if (syscalrow==0) {
        os << LogIO::NORMAL 
           <<"Cannot find any matching Tcal at/near the data timestamp."
           << " Set Tcal=0.0" << LogIO::POST ;
      } else {
        tcalCol.get(0, pksrec.tcal);
        tcalTimeCol.get(0,dtcalTime);
        pksrec.tcalTime = MVTime(dtcalTime/(24*3600)).string(MVTime::YMD);
        //DEBUG
        //cerr<<"cIdx:"<<cIdx<<" tcal="<<tcal<<" tcalTime="<<tcalTime<<endl;
        tmptab.markForDelete(); 
        tmptab2.markForDelete(); 
      }
    }
    lastmjd = pksrec.mjd;
  }

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

    pksrec.baseSub.resize(9,cNPol(iIF));
    cBaseSubCol.get(cIdx, pksrec.baseSub);

  } else {
    pksrec.baseLin.resize(0,0);
    pksrec.baseSub.resize(0,0);
  }


  // Get spectral data.
  if (cGetSpectra) {
    Matrix<Float> tmpData;
    Matrix<Bool>  tmpFlag;
    if (cHaveDataCol) {
      Matrix<Complex> tmpCmplxData;
      Matrix<Float> tmpReData;
      Matrix<Float> tmpImData;
      //cerr<<"reading spectra..."<<endl;
      //# TODO - should have a flag to user to select DATA or CORRECTED_DATA
      //# currently just automatically determined, --- read CORRECTED one 
      //# if the column exist. 
      if (cHaveCorrectedDataCol) {
        cCorrectedDataCol.getSlice(cIdx, cDataSel(iIF), tmpCmplxData, True);
      } else {
        cDataCol.getSlice(cIdx, cDataSel(iIF), tmpCmplxData, True);
      }
      tmpReData = real(tmpCmplxData);
      tmpImData = imag(tmpCmplxData);
      tmpData = sqrt(tmpReData*tmpReData + tmpImData*tmpImData);
    } else {
      cFloatDataCol.getSlice(cIdx, cDataSel(iIF), tmpData, True);
    }
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

    // Row-based flagging info. (True:1, False:0)
    pksrec.flagrow = (cFlagRowCol(cIdx) ? 1 : 0);
  }

  // Get cross-polarization data.
  if (cGetXPol) {
    //cerr<<"cGetXPol="<<cGetXPol<<endl;
    //cerr<<"cHaveXCalFctr="<<cHaveXCalFctr<<endl;

    if (cHaveXCalFctr) {
      cXCalFctrCol.get(cIdx, pksrec.xCalFctr);
    } else {
      pksrec.xCalFctr = Complex(0.0f, 0.0f);
    }

    if(!cALMA) {
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
  }
  /**
  cerr<<"scanNo="<<scanNo<<endl;
  cerr<<"cycleNo="<<cycleNo<<endl;
  cerr<<"mjd="<<mjd<<endl;
  cerr<<"interval="<<interval<<endl;
  cerr<<"fieldName="<<fieldName<<endl;
  cerr<<"srcNmae="<<srcName<<endl;
  cerr<<"srcDir="<<srcDir<<endl;
  cerr<<"srcPM="<<srcPM<<endl;
  cerr<<"srcVel="<<srcVel<<endl;
  cerr<<"obsMode="<<obsMode<<endl;
  cerr<<"IFno="<<IFno<<endl;
  cerr<<"refFreq="<<refFreq<<endl;
  cerr<<"tcal="<<tcal<<endl;
  cerr<<"direction="<<direction<<endl;
  cerr<<"scanRate="<<scanRate<<endl;
  cerr<<"tsys="<<tsys<<endl;
  cerr<<"sigma="<<sigma<<endl;
  cerr<<"calFctr="<<calFctr<<endl;
  cerr<<"baseLin="<<baseLin<<endl;
  cerr<<"baseSub="<<baseSub<<endl;
  cerr<<"spectra="<<spectra.shape()<<endl; 
  cerr<<"flagged="<<flagged.shape()<<endl;
  cerr<<"xCalFctr="<<xCalFctr<<endl;
  cerr<<"xPol="<<xPol<<endl;
  **/
  cIdx++;

  return 0;
}

//--------------------------------------------------------- PKSMS2reader::read

// Read the next data record, just the basics.

Int PKSMS2reader::read(
        Int           &IFno,
        Vector<Float> &tsys,
        Vector<Float> &calFctr,
        Matrix<Float> &baseLin,
        Matrix<Float> &baseSub,
        Matrix<Float> &spectra,
        Matrix<uChar> &flagged)
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
  Int iDataDesc;
  while (True) {
    ibeam = cBeamNoCol(cIdx);
    //iIF   = cDataDescIdCol(cIdx);
    iDataDesc   = cDataDescIdCol(cIdx);
    iIF   = cSpWinIdCol(iDataDesc);
    if (cBeams(ibeam) && cIFs(iIF)) {
      break;
    }

    // Check for EOF.
    if (++cIdx >= cNRow) {
      return -1;
    }
  }

  IFno = iIF + 1;
  // Get Tsys assuming that entries in the SYSCAL table match the main table.
  cTsysCol.get(cIdx, tsys, True);

  // Calibration factors (if available).
  if (cHaveCalFctr) {
    cCalFctrCol.get(cIdx, calFctr, True);
  } else {
    calFctr.resize(cNPol(iIF));
    calFctr = 0.0f;
  }

  // Baseline parameters (if available).
  if (cHaveBaseLin) {
    baseLin.resize(2,cNPol(iIF));
    cBaseLinCol.get(cIdx, baseLin);

    baseSub.resize(9,cNPol(iIF));
    cBaseSubCol.get(cIdx, baseSub);

  } else {
    baseLin.resize(0,0);
    baseSub.resize(0,0);
  }

  if (cGetSpectra) {
    // Get spectral data.
    Matrix<Float> tmpData;
    Matrix<Bool>  tmpFlag;
    if (cHaveDataCol) {
      Matrix<Complex> tmpCmplxData;
      cDataCol.getSlice(cIdx, cDataSel(iIF), tmpCmplxData, True);
      tmpData = real(tmpCmplxData);
    } else {
      cFloatDataCol.getSlice(cIdx, cDataSel(iIF), tmpData, True);
    }
    cFlagCol.getSlice(cIdx, cDataSel(iIF), tmpFlag, True);

    // Transpose spectra.
    Int nChan = tmpData.ncolumn();
    Int nPol  = tmpData.nrow();
    spectra.resize(nChan, nPol);
    flagged.resize(nChan, nPol);
    if (cEndChan(iIF) >= cStartChan(iIF)) {
      // Simple transposition.
      for (Int ipol = 0; ipol < nPol; ipol++) {
        for (Int ichan = 0; ichan < nChan; ichan++) {
          spectra(ichan,ipol) = tmpData(ipol,ichan);
          flagged(ichan,ipol) = tmpFlag(ipol,ichan);
        }
      }

    } else {
      // Transpose with inversion.
      Int jchan = nChan - 1;
      for (Int ipol = 0; ipol < nPol; ipol++) {
        for (Int ichan = 0; ichan < nChan; ichan++, jchan--) {
          spectra(ichan,ipol) = tmpData(ipol,jchan);
          flagged(ichan,ipol) = tmpFlag(ipol,jchan);
        }
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

//-------------------------------------------------------- PKSMS2reader::splitAntenanSelectionString

// split antenna selection string
// delimiter is ','

Vector<String> PKSMS2reader::splitAntennaSelectionString( const String s ) 
{
  Char delim = ',' ;
  Int n = s.freq( delim ) + 1 ;
  Vector<String> antlist ;
  string sl[n] ;
  Int numSubstr = split( s, sl, n, "," );
  antlist.resize( numSubstr ) ;
  for ( Int i = 0 ; i < numSubstr ; i++ ) {
    antlist[i] = String( sl[i] ) ;
    antlist[i].trim() ;
  }
  //cerr << "antlist = " << antlist << endl ;
  return antlist ;
}

//-------------------------------------------------------- PKSMS2reader::setupAntennaList

// Fill cAntenna and cAntId

void PKSMS2reader::setupAntennaList( const String s ) 
{
  LogIO os( LogOrigin( "PKSMS2reader", "setupAntennaList()", WHERE ) ) ;
  //cerr << "antenna specification: " << s << endl ;
  ROMSAntennaColumns antennaCols(cPKSMS.antenna());
  ROScalarColumn<String> antNames = antennaCols.name();
  Int nrow = antNames.nrow() ;
  Vector<String> antlist = splitAntennaSelectionString( s ) ;
  Int len = antlist.size() ;
  Vector<Int> AntId( len ) ;
  Regex re( "[0-9]+" ) ;
  for ( Int i = 0 ; i < len ; i++ ) {
    if ( antlist[i].matches( re ) ) {
      AntId[i] = atoi( antlist[i].c_str() ) ;
      if ( AntId[i] >= nrow ) {
        os << LogIO::SEVERE << "Antenna index out of range: " << AntId[i] << LogIO::EXCEPTION ;
      }
    }
    else {
      AntId[i] = -1 ;
      for ( uInt j = 0 ; j < antNames.nrow() ; j++ ) {
        if ( antlist[i] == antNames(j) ) {
          AntId[i] = j ;
          break ;
        }
      }
      if ( AntId[i] == -1 ) {
        os << LogIO::SEVERE << "Specified antenna name not found: " << antlist[i] << LogIO::EXCEPTION ;
      }
    }
  }
  //cerr << "AntId = " << AntId << endl ;
  vector<Int> uniqId ;
  uniqId.push_back( AntId(0) ) ;
  for ( uInt i = 1 ; i < AntId.size() ; i++ ) {
    if ( count(uniqId.begin(),uniqId.end(),AntId[i]) == 0 ) {
      uniqId.push_back( AntId[i] ) ;
    }
  }
  Vector<Int> newAntId( uniqId ) ;
  cAntId.assign( newAntId ) ;
  //cerr << "cAntId = " << cAntId << endl ;
}

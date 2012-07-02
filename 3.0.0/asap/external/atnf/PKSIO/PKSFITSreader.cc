//#---------------------------------------------------------------------------
//# PKSFITSreader.cc: Class to read Parkes multibeam data from a FITS file.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//# Original: 2000/08/02, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/MBFITSreader.h>
#include <atnf/PKSIO/SDFITSreader.h>
#include <atnf/PKSIO/PKSFITSreader.h>
#include <atnf/PKSIO/PKSMBrecord.h>

#include <casa/Arrays/Array.h>
#include <casa/BasicMath/Math.h>
#include <casa/Quanta/MVTime.h>

#include <casa/stdio.h>

//----------------------------------------------- PKSFITSreader::PKSFITSreader

// Constructor sets the method of position interpolation.

PKSFITSreader::PKSFITSreader(
        const String fitsType,
        const Int    retry,
        const Bool   interpolate)
{
  cMBrec.setNIFs(1);

  if (fitsType == "SDFITS") {
    cReader = new SDFITSreader();
  } else {
    cReader = new MBFITSreader(retry, interpolate ? 1 : 0);
  }
}

//---------------------------------------------- PKSFITSreader::~PKSFITSreader

// Destructor.

PKSFITSreader::~PKSFITSreader()
{
  close();
  delete cReader;
}

//-------------------------------------------------------- PKSFITSreader::open

// Open the FITS file for reading.

Int PKSFITSreader::open(
        const String fitsName,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra)
{
  int    extraSysCal, haveBase_, *haveXPol_, haveSpectra_, nBeam, *nChan_,
         nIF, *nPol_, status;
  if ((status = cReader->open((char *)fitsName.chars(), nBeam, cBeams, nIF,
                              cIFs, nChan_, nPol_, haveXPol_, haveBase_,
                              haveSpectra_, extraSysCal))) {
    return status;
  }

  // Beams present in data.
  beams.resize(nBeam);
  for (Int ibeam = 0; ibeam < nBeam; ibeam++) {
    beams(ibeam) = cBeams[ibeam];
  }

  // IFs, channels, and polarizations present in data.
  IFs.resize(nIF);
  nChan.resize(nIF);
  nPol.resize(nIF);
  haveXPol.resize(nIF);

  for (Int iIF = 0; iIF < nIF; iIF++) {
    IFs(iIF)   = cIFs[iIF];
    nChan(iIF) = nChan_[iIF];
    nPol(iIF)  = nPol_[iIF];

    // Cross-polarization data present?
    haveXPol(iIF) = haveXPol_[iIF];
  }

  cNBeam = nBeam;
  cNIF   = nIF;
  cNChan.assign(nChan);
  cNPol.assign(nPol);
  cHaveXPol.assign(haveXPol);

  // Baseline parameters present?
  haveBase = haveBase_;

  // Spectral data present?
  haveSpectra = haveSpectra_;

  return 0;
}

//--------------------------------------------------- PKSFITSreader::getHeader

// Get parameters describing the data.

Int PKSFITSreader::getHeader(
        String &observer,
        String &project,
        String &antName,
        Vector<Double> &antPosition,
        String &obsType,
        Float  &equinox,
        String &dopplerFrame,
        Double &mjd,
        Double &refFreq,
        Double &bandwidth,
        String &fluxunit)
{
  char   datobs[32], dopplerFrame_[32], observer_[32], obsType_[32],
         project_[32], radecsys[32], telescope[32];
  float  equinox_;
  double antPos[3], utc;

  if (cReader->getHeader(observer_, project_, telescope, antPos, obsType_,
                         equinox_, radecsys, dopplerFrame_, datobs, utc,
                         refFreq, bandwidth)) {
    return 1;
  }

  fluxunit = "";
  observer = trim(observer_);
  project  = trim(project_);
  antName  = trim(telescope);
  antPosition.resize(3);
  antPosition(0) = antPos[0];
  antPosition(1) = antPos[1];
  antPosition(2) = antPos[2];
  obsType = trim(obsType_);
  equinox = equinox_;
  dopplerFrame = trim(dopplerFrame_);

  // Get UTC in MJD form.
  Int day, month, year;
  sscanf(datobs, "%4d-%2d-%2d", &year, &month, &day);
  MVTime date(year, month, Double(day));
  mjd = date.day() + utc/86400.0;

  return 0;
}

//------------------------------------------------- PKSFITSreader::getFreqInfo

// Get frequency parameters for each IF.

Int PKSFITSreader::getFreqInfo(
        Vector<Double> &startFreq,
        Vector<Double> &endFreq)
{
  int     nIF;
  double *startfreq, *endfreq;

  Int status;
  if (!(status = cReader->getFreqInfo(nIF, startfreq, endfreq))) {
    startFreq.takeStorage(IPosition(1,nIF), startfreq, TAKE_OVER);
    endFreq.takeStorage(IPosition(1,nIF), endfreq, TAKE_OVER);
  }

  return status;
}

//------------------------------------------------------ PKSFITSreader::select

// Set data selection by beam number and channel.

uInt PKSFITSreader::select(
        const Vector<Bool> beamSel,
        const Vector<Bool> IFsel,
        const Vector<Int>  startChan,
        const Vector<Int>  endChan,
        const Vector<Int>  refChan,
        const Bool getSpectra,
        const Bool getXPol,
        const Bool getFeedPos)
{
  // Apply beam selection.
  uInt nBeamSel = beamSel.nelements();
  for (uInt ibeam = 0; ibeam < cNBeam; ibeam++) {
    // This modifies FITSreader::cBeams[].
    if (ibeam < nBeamSel) {
      cBeams[ibeam] = cBeams[ibeam] && beamSel(ibeam);
    } else {
      cBeams[ibeam] = 0;
    }
  }

  // Apply IF selection.
  int *end   = new int[cNIF];
  int *ref   = new int[cNIF];
  int *start = new int[cNIF];

  for (uInt iIF = 0; iIF < cNIF; iIF++) {
    // This modifies FITSreader::cIFs[].
    if (iIF < IFsel.nelements()) {
      cIFs[iIF] = cIFs[iIF] && IFsel(iIF);
    } else {
      cIFs[iIF] = 0;
    }

    if (cIFs[iIF]) {
      if (iIF < startChan.nelements()) {
        start[iIF] = startChan(iIF);

        if (start[iIF] <= 0) {
          start[iIF] += cNChan(iIF);
        } else if (start[iIF] > Int(cNChan(iIF))) {
          start[iIF]  = cNChan(iIF);
        }

      } else {
        start[iIF] = 1;
      }

      if (iIF < endChan.nelements()) {
        end[iIF] = endChan(iIF);

        if (end[iIF] <= 0) {
          end[iIF] += cNChan(iIF);
        } else if (end[iIF] > Int(cNChan(iIF))) {
          end[iIF]  = cNChan(iIF);
        }

      } else {
        end[iIF] = cNChan(iIF);
      }

      if (iIF < refChan.nelements()) {
        ref[iIF] = refChan(iIF);
      } else {
        if (start[iIF] <= end[iIF]) {
          ref[iIF] = start[iIF] + (end[iIF] - start[iIF] + 1)/2;
        } else {
          ref[iIF] = start[iIF] - (start[iIF] - end[iIF] + 1)/2;
        }
      }
    }
  }

  cGetSpectra = getSpectra;
  cGetXPol    = getXPol;
  cGetFeedPos = getFeedPos;

  uInt maxNChan = cReader->select(start, end, ref, cGetSpectra, cGetXPol,
                                  cGetFeedPos);

  delete [] end;
  delete [] ref;
  delete [] start;

  return maxNChan;
}

//--------------------------------------------------- PKSFITSreader::findRange

// Read the TIME column.

Int PKSFITSreader::findRange(
        Int    &nRow,
        Int    &nSel,
        Vector<Double> &timeSpan,
        Matrix<Double> &positions)
{
  char    dateSpan[2][32];
  double  utcSpan[2];
  double* posns;

  Int status;
  if (!(status = cReader->findRange(nRow, nSel, dateSpan, utcSpan, posns))) {
    timeSpan.resize(2);

    Int day, month, year;
    sscanf(dateSpan[0], "%4d-%2d-%2d", &year, &month, &day);
    timeSpan(0) = MVTime(year, month, Double(day)).second() + utcSpan[0];
    sscanf(dateSpan[1], "%4d-%2d-%2d", &year, &month, &day);
    timeSpan(1) = MVTime(year, month, Double(day)).second() + utcSpan[1];

    positions.takeStorage(IPosition(2,2,nSel), posns, TAKE_OVER);
  }

  return status;
}

//-------------------------------------------------------- PKSFITSreader::read

// Read the next data record.

Int PKSFITSreader::read(
        Int             &scanNo,
        Int             &cycleNo,
        Double          &mjd,
        Double          &interval,
        String          &fieldName,
        String          &srcName,
        Vector<Double>  &srcDir,
        Vector<Double>  &srcPM,
        Double          &srcVel,
        String          &obsType,
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
        Complex         &xCalFctr,
        Vector<Complex> &xPol)
{
  Int status;

  if ((status = cReader->read(cMBrec))) {
    if (status != -1) {
      status = 1;
    }

    return status;
  }


  uInt nChan = cMBrec.nChan[0];
  uInt nPol  = cMBrec.nPol[0];

  scanNo  = cMBrec.scanNo;
  cycleNo = cMBrec.cycleNo;

  // Extract MJD.
  Int day, month, year;
  sscanf(cMBrec.datobs, "%4d-%2d-%2d", &year, &month, &day);
  mjd = MVTime(year, month, Double(day)).day() + cMBrec.utc/86400.0;

  interval  = cMBrec.exposure;

  fieldName = trim(cMBrec.srcName);
  srcName   = fieldName;
  srcDir(0) = cMBrec.srcRA;
  srcDir(1) = cMBrec.srcDec;
  srcPM(0)  = 0.0;
  srcPM(1)  = 0.0;
  srcVel    = 0.0;
  obsType   = trim(cMBrec.obsType);

  IFno = cMBrec.IFno[0];
  Double chanWidth = fabs(cMBrec.fqDelt[0]);
  refFreq   = cMBrec.fqRefVal[0];
  bandwidth = chanWidth * nChan;
  freqInc   = cMBrec.fqDelt[0];
  //restFreq  = cMBrec.restFreq;
  restFreq.resize(1);
  restFreq(0)  = cMBrec.restFreq;

  tcal.resize(nPol);
  for (uInt ipol = 0; ipol < nPol; ipol++) {
    tcal(ipol) = cMBrec.tcal[0][ipol];
  }
  tcalTime  = trim(cMBrec.tcalTime);
  azimuth   = cMBrec.azimuth;
  elevation = cMBrec.elevation;
  parAngle  = cMBrec.parAngle;
  focusAxi  = cMBrec.focusAxi;
  focusTan  = cMBrec.focusTan;
  focusRot  = cMBrec.focusRot;

  temperature = cMBrec.temp;
  pressure    = cMBrec.pressure;
  humidity    = cMBrec.humidity;
  windSpeed   = cMBrec.windSpeed;
  windAz      = cMBrec.windAz;

  refBeam = cMBrec.refBeam;
  beamNo  = cMBrec.beamNo;

  direction(0) = cMBrec.ra;
  direction(1) = cMBrec.dec;
  scanRate(0)  = cMBrec.raRate;
  scanRate(1)  = cMBrec.decRate;

  tsys.resize(nPol);
  sigma.resize(nPol);
  calFctr.resize(nPol);
  for (uInt ipol = 0; ipol < nPol; ipol++) {
    tsys(ipol)  = cMBrec.tsys[0][ipol];
    sigma(ipol) = tsys(ipol) / 0.81 / sqrt(interval * chanWidth);
    calFctr(ipol) = cMBrec.calfctr[0][ipol];
  }

  if (cMBrec.haveBase) {
    baseLin.resize(2,nPol);
    baseSub.resize(9,nPol);

    for (uInt ipol = 0; ipol < nPol; ipol++) {
      baseLin(0,ipol) = cMBrec.baseLin[0][ipol][0];
      baseLin(1,ipol) = cMBrec.baseLin[0][ipol][1];

      for (uInt j = 0; j < 9; j++) {
        baseSub(j,ipol) = cMBrec.baseSub[0][ipol][j];
      }
    }

  } else {
    baseLin.resize(0,0);
    baseSub.resize(0,0);
  }

  if (cGetSpectra && cMBrec.haveSpectra) {
    spectra.resize(nChan,nPol);
    spectra.takeStorage(IPosition(2,nChan,nPol), cMBrec.spectra[0], SHARE);

    flagged.resize(nChan,nPol);
    flagged.takeStorage(IPosition(2,nChan,nPol), cMBrec.flagged[0], SHARE);

  } else {
    spectra.resize(0,0);
    flagged.resize(0,0);
  }

  if (cGetXPol) {
    xCalFctr = Complex(cMBrec.xcalfctr[0][0], cMBrec.xcalfctr[0][1]);
    xPol.resize(nChan);
    xPol.takeStorage(IPosition(1,nChan), (Complex *)cMBrec.xpol[0], SHARE);
  }

  return 0;
}

//-------------------------------------------------------- PKSFITSreader::read

// Read the next data record, just the basics.

Int PKSFITSreader::read(
        Int           &IFno,
        Vector<Float> &tsys,
        Vector<Float> &calFctr,
        Matrix<Float> &baseLin,
        Matrix<Float> &baseSub,
        Matrix<Float> &spectra,
        Matrix<uChar> &flagged)
{
  Int status;

  if ((status = cReader->read(cMBrec))) {
    if (status != -1) {
      status = 1;
    }

    return status;
  }

  IFno = cMBrec.IFno[0];

  uInt nChan = cMBrec.nChan[0];
  uInt nPol  = cMBrec.nPol[0];

  tsys.resize(nPol);
  calFctr.resize(nPol);
  for (uInt ipol = 0; ipol < nPol; ipol++) {
    tsys(ipol) = cMBrec.tsys[0][ipol];
    calFctr(ipol) = cMBrec.calfctr[0][ipol];
  }

  if (cMBrec.haveBase) {
    baseLin.resize(2,nPol);
    baseSub.resize(9,nPol);

    for (uInt ipol = 0; ipol < nPol; ipol++) {
      baseLin(0,ipol) = cMBrec.baseLin[0][ipol][0];
      baseLin(1,ipol) = cMBrec.baseLin[0][ipol][1];

      for (uInt j = 0; j < 9; j++) {
        baseSub(j,ipol) = cMBrec.baseSub[0][ipol][j];
      }
    }

  } else {
    baseLin.resize(0,0);
    baseSub.resize(0,0);
  }

  if (cGetSpectra && cMBrec.haveSpectra) {
    spectra.resize(nChan,nPol);
    spectra.takeStorage(IPosition(2,nChan,nPol), cMBrec.spectra[0], SHARE);

    flagged.resize(nChan,nPol);
    flagged.takeStorage(IPosition(2,nChan,nPol), cMBrec.flagged[0], SHARE);

  } else {
    spectra.resize(0,0);
    flagged.resize(0,0);
  }

  return 0;
}

//------------------------------------------------------- PKSFITSreader::close

// Close the FITS file.

void PKSFITSreader::close()
{
  cReader->close();
}

//-------------------------------------------------------- PKSFITSreader::trim

// Trim trailing blanks from a null-terminated character string.

char* PKSFITSreader::trim(char *string)
{
  int j = 0, k = 0;
  while (string[j] != '\0') {
    if (string[j++] != ' ') {
      k = j;
    }
  }

  string[k] = '\0';

  return string;
}

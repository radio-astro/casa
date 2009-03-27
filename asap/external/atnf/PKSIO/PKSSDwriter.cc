//#---------------------------------------------------------------------------
//# PKSSDwriter.cc: Class to write Parkes multibeam data to an SDFITS file.
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

#include <atnf/PKSIO/PKSMBrecord.h>
#include <atnf/PKSIO/PKSSDwriter.h>

#include <casa/Quanta/MVTime.h>


//--------------------------------------------------- PKSSDwriter::PKSSDwriter

// Default constructor.

PKSSDwriter::PKSSDwriter()
{
}

//-------------------------------------------------- PKSSDwriter::~PKSSDwriter

// Destructor.

PKSSDwriter::~PKSSDwriter()
{
  close();
}

//-------------------------------------------------------- PKSSDwriter::create

// Create the SDFITS file and and write static data.

Int PKSSDwriter::create(
        const String sdName,
        const String observer,
        const String project,
        const String antName,
        const Vector<Double> antPosition,
        const String obsMode,
        const Float  equinox,
        const String dopplerFrame,
        const Vector<uInt> nChan,
        const Vector<uInt> nPol,
        const Vector<Bool> haveXPol,
        const Bool   haveBase,
        const String fluxUnit)
{
  double antPos[3];
  antPos[0] = antPosition(0);
  antPos[1] = antPosition(1);
  antPos[2] = antPosition(2);

  cNIF = nChan.nelements();
  if (nPol.nelements() != cNIF || haveXPol.nelements() != cNIF) {
    cerr << "PKSSDwriter::create: "
         << "Inconsistent number of IFs for nChan, nPol, and/or haveXPol."
         << endl;
    return 1;
  }

  cNChan.assign(nChan);
  cNPol.assign(nPol);

  cHaveXPol.resize(cNIF);
  for (uInt iIF = 0; iIF < cNIF; iIF++) {
    // Convert Bool -> uInt.
    cHaveXPol(iIF) = haveXPol(iIF) ? 1 : 0;
  }

  cHaveBase = haveBase;

  // Storage in the trivial cNChan, cNPol, and cHaveXPol arrays should always
  // be contiguous so the pointer returned by getStorage() shouldn't need to
  // be deleted via freeStorage() (i.e. deleteIt always returned False).  This
  // storage will, of course, be deleted when the PKSwriter object is deleted.
  Bool deleteIt;
  Int status = cSDwriter.create((char *)sdName.chars(),
        (char *)observer.chars(), (char *)project.chars(),
        (char *)antName.chars(), antPos, (char *)obsMode.chars(), equinox,
        (char *)dopplerFrame.chars(), cNIF,
        (int *)cNChan.getStorage(deleteIt),
        (int *)cNPol.getStorage(deleteIt),
        (int *)cHaveXPol.getStorage(deleteIt), (int)cHaveBase, 1);
  if (status) {
    cSDwriter.reportError();
    cSDwriter.deleteFile();
    close();
  }

  return status;
}

//--------------------------------------------------------- PKSSDwriter::write

// Write the next data record.

Int PKSSDwriter::write(
        const Int             scanNo,
        const Int             cycleNo,
        const Double          mjd,
        const Double          interval,
        const String          fieldName,
        const String          srcName,
        const Vector<Double>  srcDir,
        const Vector<Double>  srcPM,
        const Double          srcVel,
        const String          obsMode,
        const Int             IFno,
        const Double          refFreq,
        const Double          bandwidth,
        const Double          freqInc,
        //const Double          restFreq,
        const Vector<Double>  restFreq,
        const Vector<Float>   tcal,
        const String          tcalTime,
        const Float           azimuth,
        const Float           elevation,
        const Float           parAngle,
        const Float           focusAxi,
        const Float           focusTan,
        const Float           focusRot,
        const Float           temperature,
        const Float           pressure,
        const Float           humidity,
        const Float           windSpeed,
        const Float           windAz,
        const Int             refBeam,
        const Int             beamNo,
        const Vector<Double>  direction,
        const Vector<Double>  scanRate,
        const Vector<Float>   tsys,
        const Vector<Float>   sigma,
        const Vector<Float>   calFctr,
        const Matrix<Float>   baseLin,
        const Matrix<Float>   baseSub,
        const Matrix<Float>   &spectra,
        const Matrix<uChar>   &flagged,
        const Complex         xCalFctr,
        const Vector<Complex> &xPol)
{
  // Do basic checks.
  uInt iIF = IFno - 1;
  if (IFno < 1 || Int(cNIF) < IFno) {
    cerr << "PKSDwriter::write: "
         << "Invalid IF number " << IFno
         << " (maximum " << cNIF << ")." << endl;
    return 1;
  }

  uInt nChan = spectra.nrow();
  if (nChan != cNChan(iIF)) {
    cerr << "PKSDwriter::write: "
         << "Wrong number of channels for IF " << IFno << "," << endl
         << "                   "
         << "got " << nChan << " should be " << cNChan(iIF) << "." << endl;
    return 1;
  }

  uInt nPol = spectra.ncolumn();
  if (nPol != cNPol(iIF)) {
    cerr << "PKSDwriter::write: "
         << "Wrong number of polarizations for IF " << IFno << "," << endl
         << "                   "
         << "got " << nPol << " should be " << cNPol(iIF) << "." << endl;
    return 1;
  }

  // Extract calendar information from mjd.
  MVTime time(mjd);
  Int year  = time.year();
  Int month = time.month();
  Int day   = time.monthday();

  // Transfer data to a single-IF PKSMBrecord.
  PKSMBrecord mbrec(1);

  // Start with basic beam- and IF-independent bookkeeping information.
  mbrec.scanNo  = scanNo;
  mbrec.cycleNo = cycleNo;

  sprintf(mbrec.datobs, "%4.4d-%2.2d-%2.2d", year, month, day);
  mbrec.utc      = fmod(mjd, 1.0) * 86400.0;
  mbrec.exposure = float(interval);

  strncpy(mbrec.srcName, (char *)srcName.chars(), 17);
  mbrec.srcRA    = srcDir(0);
  mbrec.srcDec   = srcDir(1);

  //mbrec.restFreq = restFreq;
  mbrec.restFreq = restFreq(0);

  strncpy(mbrec.obsType, (char *)obsMode.chars(), 16);

  // Now beam-dependent parameters.
  mbrec.beamNo   = beamNo;
  mbrec.ra       = direction(0);
  mbrec.dec      = direction(1);
  mbrec.raRate   = scanRate(0);
  mbrec.decRate  = scanRate(1);

  // Now IF-dependent parameters.
  mbrec.nIF      = 1;
  mbrec.IFno[0]  = IFno;
  mbrec.nChan[0] = nChan;
  mbrec.nPol[0]  = nPol;

  mbrec.fqRefPix[0] = (nChan/2) + 1;
  mbrec.fqRefVal[0] = refFreq;
  mbrec.fqDelt[0]   = freqInc;

  // Now the data itself.
  for (uInt i = 0; i < tsys.nelements(); i++) {
    mbrec.tsys[0][i] = tsys(i);
  }

  for (uInt ipol = 0; ipol < nPol; ipol++) {
    mbrec.calfctr[0][ipol] = calFctr(ipol);
  }

  if (cHaveXPol(iIF)) {
    mbrec.xcalfctr[0][0] = xCalFctr.real();
    mbrec.xcalfctr[0][1] = xCalFctr.imag();
  } else {
    mbrec.xcalfctr[0][0] = 0.0f;
    mbrec.xcalfctr[0][1] = 0.0f;
  }

  if (cHaveBase) {
    mbrec.haveBase = 1;

    for (uInt ipol = 0; ipol < nPol; ipol++) {
      mbrec.baseLin[0][ipol][0] = baseLin(0,ipol);
      mbrec.baseLin[0][ipol][1] = baseLin(1,ipol);

      for (uInt j = 0; j < baseSub.nrow(); j++) {
        mbrec.baseSub[0][ipol][j] = baseSub(j,ipol);
      }
      for (uInt j = baseSub.nrow(); j < 9; j++) {
        mbrec.baseSub[0][ipol][j] = 0.0f;
      }
    }

  } else {
    mbrec.haveBase = 0;
  }

  Bool delSpectra = False;
  const Float *specstor = spectra.getStorage(delSpectra);
  mbrec.spectra[0] = (float *)specstor;

  Bool delFlagged = False;
  const uChar *flagstor = flagged.getStorage(delFlagged);
  mbrec.flagged[0] = (unsigned char *)flagstor;

  Bool delXPol = False;
  const Complex *xpolstor;
  if (cHaveXPol(iIF)) {
    xpolstor = xPol.getStorage(delXPol);
  } else {
    xpolstor = 0;
  }
  mbrec.xpol[0] = (float *)xpolstor;

  // Finish off with system calibration parameters.
  mbrec.extraSysCal = 1;
  mbrec.refBeam     = refBeam;
  for (uInt i = 0; i < tcal.nelements(); i++) {
    mbrec.tcal[0][i] = tcal(i);
  }
  strncpy(mbrec.tcalTime, (char *)tcalTime.chars(), 16);
  mbrec.azimuth   = azimuth;
  mbrec.elevation = elevation;
  mbrec.parAngle  = parAngle;
  mbrec.focusAxi  = focusAxi;
  mbrec.focusTan  = focusTan;
  mbrec.focusRot  = focusRot;
  mbrec.temp      = temperature;
  mbrec.pressure  = pressure;
  mbrec.humidity  = humidity;
  mbrec.windSpeed = windSpeed;
  mbrec.windAz    = windAz;

  Int status = cSDwriter.write(mbrec);
  if (status) {
    cSDwriter.reportError();
    status = 1;
  }

  spectra.freeStorage(specstor, delSpectra);
  flagged.freeStorage(flagstor, delFlagged);
  xPol.freeStorage(xpolstor, delXPol);

  return status;
}

//--------------------------------------------------------- PKSSDwriter::close

// Close the SDFITS file.

void PKSSDwriter::close()
{
  cSDwriter.close();
}

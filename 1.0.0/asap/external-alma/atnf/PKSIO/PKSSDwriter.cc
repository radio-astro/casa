//#---------------------------------------------------------------------------
//# PKSSDwriter.cc: Class to write Parkes multibeam data to an SDFITS file.
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
//# $Id: PKSSDwriter.cc,v 19.17 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/MBrecord.h>
#include <atnf/PKSIO/PKSSDwriter.h>

#include <casa/Logging/LogIO.h>

#include <casa/stdio.h>
#include <casa/Quanta/MVTime.h>

#include <string>
#include <cstring>

// Class name
const string className = "PKSSDwriter" ;

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
        const String bunit,
        const Float  equinox,
        const String dopplerFrame,
        const Vector<uInt> nChan,
        const Vector<uInt> nPol,
        const Vector<Bool> haveXPol,
        const Bool   haveBase)
{
  const string methodName = "create()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  double antPos[3];
  antPos[0] = antPosition(0);
  antPos[1] = antPosition(1);
  antPos[2] = antPosition(2);

  cNIF = nChan.nelements();
  if (nPol.nelements() != cNIF || haveXPol.nelements() != cNIF) {
    os << LogIO::SEVERE << "Inconsistent number of IFs for nChan, nPol, and/or haveXPol." << LogIO::POST ;
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
        (char *)antName.chars(), antPos, (char *)obsMode.chars(),
        (char *)bunit.chars(), equinox, (char *)dopplerFrame.chars(), cNIF,
        (int *)cNChan.getStorage(deleteIt),
        (int *)cNPol.getStorage(deleteIt),
        (int *)cHaveXPol.getStorage(deleteIt), (int)cHaveBase, 1);
  //logMsg(cSDwriter.getMsg());
  //cSDwriter.clearMsg();
  if (status) {
    cSDwriter.deleteFile();
    close();
  }

  return status;
}

//--------------------------------------------------------- PKSSDwriter::write

// Write the next data record.

Int PKSSDwriter::write(
        const PKSrecord &pksrec)
{
  const string methodName = "write()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  // Do basic checks.
  Int IFno = pksrec.IFno;
  uInt iIF = IFno - 1;
  if (IFno < 1 || Int(cNIF) < IFno) {
    os << LogIO::SEVERE 
       << "Invalid IF number " << IFno
       << " (maximum " << cNIF << ")." << LogIO::POST ;
    return 1;
  }

  uInt nChan = pksrec.spectra.nrow();
  if (nChan != cNChan(iIF)) {
    os << LogIO::SEVERE << "Wrong number of channels for IF " << IFno << "," << endl
         << "got " << nChan << " should be " << cNChan(iIF) << "." << endl;
    os << LogIO::POST ;
    return 1;
  }

  uInt nPol = pksrec.spectra.ncolumn();
  if (nPol != cNPol(iIF)) {
    os << LogIO::SEVERE << "Wrong number of polarizations for IF " << IFno << "," << endl
       << "got " << nPol << " should be " << cNPol(iIF) << "." << endl;
    os << LogIO::POST ;
    return 1;
  }

  // Extract calendar information frrom mjd.
  MVTime time(pksrec.mjd);
  Int year  = time.year();
  Int month = time.month();
  Int day   = time.monthday();

  // Transfer data to a single-IF MBrecord.
  MBrecord mbrec(1);

  // Start with basic beam- and IF-independent bookkeeping information.
  mbrec.scanNo  = pksrec.scanNo;
  mbrec.cycleNo = pksrec.cycleNo;

  sprintf(mbrec.datobs, "%4.4d-%2.2d-%2.2d", year, month, day);
  mbrec.utc      = fmod(pksrec.mjd, 1.0) * 86400.0;
  mbrec.exposure = float(pksrec.interval);

  strncpy(mbrec.srcName, (char *)pksrec.srcName.chars(), 17);
  mbrec.srcRA    = pksrec.srcDir(0);
  mbrec.srcDec   = pksrec.srcDir(1);
  if (pksrec.restFreq.shape()==0) { 
     mbrec.restFreq = 0;
  }
  else {
     mbrec.restFreq = pksrec.restFreq(0);
  }
  strncpy(mbrec.obsType, (char *)pksrec.obsType.chars(), 16);

  // Now beam-dependent parameters.
  mbrec.beamNo   = pksrec.beamNo;
  mbrec.ra       = pksrec.direction(0);
  mbrec.dec      = pksrec.direction(1);
  mbrec.raRate   = pksrec.scanRate(0);
  mbrec.decRate  = pksrec.scanRate(1);

  // Now IF-dependent parameters.
  mbrec.nIF      = 1;
  mbrec.IFno[0]  = IFno;
  mbrec.nChan[0] = nChan;
  mbrec.nPol[0]  = nPol;

  mbrec.fqRefPix[0] = (nChan/2) + 1;
  mbrec.fqRefVal[0] = pksrec.refFreq;
  mbrec.fqDelt[0]   = pksrec.freqInc;

  // Now the data itself.
  for (uInt i = 0; i < pksrec.tsys.nelements(); i++) {
    mbrec.tsys[0][i] = pksrec.tsys(i);
  }

  for (uInt ipol = 0; ipol < nPol; ipol++) {
    mbrec.calfctr[0][ipol] = pksrec.calFctr(ipol);
  }

  if (cHaveXPol(iIF)) {
    mbrec.xcalfctr[0][0] = pksrec.xCalFctr.real();
    mbrec.xcalfctr[0][1] = pksrec.xCalFctr.imag();
  } else {
    mbrec.xcalfctr[0][0] = 0.0f;
    mbrec.xcalfctr[0][1] = 0.0f;
  }

  if (cHaveBase) {
    mbrec.haveBase = 1;

    for (uInt ipol = 0; ipol < nPol; ipol++) {
      mbrec.baseLin[0][ipol][0] = pksrec.baseLin(0,ipol);
      mbrec.baseLin[0][ipol][1] = pksrec.baseLin(1,ipol);

      for (uInt j = 0; j < pksrec.baseSub.nrow(); j++) {
        mbrec.baseSub[0][ipol][j] = pksrec.baseSub(j,ipol);
      }
      for (uInt j = pksrec.baseSub.nrow(); j < 24; j++) {
        mbrec.baseSub[0][ipol][j] = 0.0f;
      }
    }

  } else {
    mbrec.haveBase = 0;
  }

  Bool delSpectra = False;
  const Float *specstor = pksrec.spectra.getStorage(delSpectra);
  mbrec.spectra[0] = (float *)specstor;

  Bool delFlagged = False;
  const uChar *flagstor = pksrec.flagged.getStorage(delFlagged);
  mbrec.flagged[0] = (unsigned char *)flagstor;

  Bool delXPol = False;
  const Complex *xpolstor;
  if (cHaveXPol(iIF)) {
    xpolstor = pksrec.xPol.getStorage(delXPol);
  } else {
    xpolstor = 0;
  }
  mbrec.xpol[0] = (float *)xpolstor;

  // Finish off with system calibration parameters.
  mbrec.extraSysCal = 1;
  mbrec.refBeam     = pksrec.refBeam;
  for (uInt i = 0; i < pksrec.tcal.nelements(); i++) {
    mbrec.tcal[0][i] = pksrec.tcal(i);
  }
  strncpy(mbrec.tcalTime, (char *)pksrec.tcalTime.chars(), 16);
  mbrec.azimuth   = pksrec.azimuth;
  mbrec.elevation = pksrec.elevation;
  mbrec.parAngle  = pksrec.parAngle;
  mbrec.focusAxi  = pksrec.focusAxi;
  mbrec.focusTan  = pksrec.focusTan;
  mbrec.focusRot  = pksrec.focusRot;
  mbrec.temp      = pksrec.temperature;
  mbrec.pressure  = pksrec.pressure;
  mbrec.humidity  = pksrec.humidity;
  mbrec.windSpeed = pksrec.windSpeed;
  mbrec.windAz    = pksrec.windAz;

  Int status = cSDwriter.write(mbrec);
  //logMsg(cSDwriter.getMsg());
  //cSDwriter.clearMsg();
  if (status) {
    status = 1;
  }

  pksrec.spectra.freeStorage(specstor, delSpectra);
  pksrec.flagged.freeStorage(flagstor, delFlagged);
  pksrec.xPol.freeStorage(xpolstor, delXPol);

  return status;
}

//------------------------------------------------------- PKSSDwriter::history

// Write a history record.

Int PKSSDwriter::history(const String text)
{
  return cSDwriter.history((char *)text.chars());
}

Int PKSSDwriter::history(const char *text)
{
  return cSDwriter.history((char *)text);
}

//--------------------------------------------------------- PKSSDwriter::close

// Close the SDFITS file.

void PKSSDwriter::close()
{
  cSDwriter.close();
  //logMsg(cSDwriter.getMsg());
  //cSDwriter.clearMsg();
}

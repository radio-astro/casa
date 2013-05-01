//#---------------------------------------------------------------------------
//# MBrecord.cc: Class to store an MBFITS single-dish data record.
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
//# $Id: MBrecord.cc,v 19.14 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# The MBrecord class stores an MBFITS single-dish data record.
//#
//# Original: 2000/08/01 Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/MBrecord.h>

#include <string.h>

//--------------------------------------------------------- MBrecord::MBrecord

// Default constructor.

MBrecord::MBrecord(int nif)
{
  // Construct arrays for the required number of IFs.
  cNIF = 0;
  setNIFs(nif);

  scanNo  = 0;
  cycleNo = 0;
  beamNo  = 0;
  pCode   = 0;
  rateAge = 0.0f;
  raRate  = 0.0f;
  decRate = 0.0f;
  nIF     = 0;
}

//-------------------------------------------------------- MBrecord::~MBrecord

// Destructor.

MBrecord::~MBrecord()
{
  free();
}

//---------------------------------------------------------- MBrecord::setNIFs

// Expand arrays if necessary to accomodate the required number of IFs; never
// contracts them.

void MBrecord::setNIFs(int nif)
{
  if (nif < 1) return;

  if (cNIF < nif) {
    // Too few IFs, free everything.
    if (cNIF) free();
  }

  if (cNIF == 0) {
    IFno     = new short[nif];
    nChan    = new int[nif];
    nPol     = new int[nif];
    fqRefPix = new float[nif];
    fqRefVal = new double[nif];
    fqDelt   = new double[nif];

    tsys     = new float[nif][2];
    calfctr  = new float[nif][2];
    xcalfctr = new float[nif][2];
    baseLin  = new float[nif][2][2];
    baseSub  = new float[nif][2][24];
    spectra  = new float*[nif];
    flagged  = new unsigned char*[nif];
    xpol     = new float*[nif];
    tcal     = new float[nif][2];

    cNProd   = new int[nif];
    cNXPol   = new int[nif];

    for (int iIF = 0; iIF < nif; iIF++) {
      spectra[iIF] = 0x0;
      flagged[iIF] = 0x0;
      xpol[iIF]    = 0x0;

      cNProd[iIF] = 0;
      cNXPol[iIF] = 0;
    }

    // The number we can accomodate, may exceed the number we have.
    cNIF = nif;
  }
}

//--------------------------------------------------------- MBrecord::allocate

// Ensure there is enough storage for the specified number of spectral
// products (channels x polarizations) for IF with array index iIF (i.e.
// the actual IF number is IFno[iIF]).  Expands arrays if necessary but
// never contracts.

void MBrecord::allocate(
        int iIF,
        int nprod,
        int nxpol)
{
  // Don't mess with storage we didn't allocate.
  if (cNProd[iIF] || spectra[iIF] == 0x0) {
    if (cNProd[iIF] < nprod) {
      if (cNProd[iIF]) {
        // Free storage previously allocated.
        delete [] spectra[iIF];
        delete [] flagged[iIF];
      }

      // Reallocate data storage.
      cNProd[iIF]  = nprod;
      spectra[iIF] = new float[nprod];
      flagged[iIF] = new unsigned char[nprod];
    }
  }

  if (cNXPol[iIF] || xpol[iIF] == 0x0) {
    if (cNXPol[iIF] < nxpol) {
      if (cNXPol[iIF]) {
        // Free storage previously allocated.
        delete [] xpol[iIF];
      }

      // Reallocate xpol storage.
      cNXPol[iIF] = nxpol;
      xpol[iIF] = new float[nxpol];
    }
  }
}

//------------------------------------------------------------- MBrecord::free

// Free all allocated storage.

void MBrecord::free()
{
  if (cNIF) {
    for (int iIF = 0; iIF < cNIF; iIF++) {
      // Don't free storage we didn't allocate.
      if (cNProd[iIF]) {
        delete [] spectra[iIF];
        delete [] flagged[iIF];
      }

      if (cNXPol[iIF]) {
        delete [] xpol[iIF];
      }
    }

    delete [] IFno;
    delete [] nChan;
    delete [] nPol;
    delete [] fqRefPix;
    delete [] fqRefVal;
    delete [] fqDelt;

    delete [] tsys;
    delete [] calfctr;
    delete [] xcalfctr;
    delete [] baseLin;
    delete [] baseSub;

    delete [] spectra;
    delete [] flagged;
    delete [] xpol;

    delete [] tcal;

    delete [] cNProd;
    delete [] cNXPol;

    cNIF = 0;
  }
}

//-------------------------------------------------------- MBrecord::operator=

// Do a deep copy of one MBrecord to another.

MBrecord &MBrecord::operator=(const MBrecord &other)
{
  if (this == &other) {
    return *this;
  }

  setNIFs(other.nIF);

  scanNo  = other.scanNo;
  cycleNo = other.cycleNo;
  strcpy(datobs, other.datobs);
  utc = other.utc;

  exposure = other.exposure;
  strcpy(srcName, other.srcName);
  srcRA  = other.srcRA;
  srcDec = other.srcDec;
  restFreq = other.restFreq;
  strcpy(obsType, other.obsType);

  // Beam-dependent parameters.
  beamNo  = other.beamNo;
  ra      = other.ra;
  dec     = other.dec;
  pCode   = other.pCode;
  rateAge = other.rateAge;
  raRate  = other.raRate;
  decRate = other.decRate;

  // IF-dependent parameters.
  nIF = other.nIF;
  for (int iIF = 0; iIF < nIF; iIF++) {
    IFno[iIF]     = other.IFno[iIF];
    nChan[iIF]    = other.nChan[iIF];
    nPol[iIF]     = other.nPol[iIF];
    fqRefPix[iIF] = other.fqRefPix[iIF];
    fqRefVal[iIF] = other.fqRefVal[iIF];
    fqDelt[iIF]   = other.fqDelt[iIF];

    for (int j = 0; j < 2; j++) {
      tsys[iIF][j] = other.tsys[iIF][j];
    }

    for (int j = 0; j < 2; j++) {
      calfctr[iIF][j]  = other.calfctr[iIF][j];
      xcalfctr[iIF][j] = other.xcalfctr[iIF][j];
    }

    haveBase = other.haveBase;
    for (int ipol = 0; ipol < nPol[iIF]; ipol++) {
      baseLin[iIF][ipol][0] = other.baseLin[iIF][ipol][0];
      baseLin[iIF][ipol][1] = other.baseLin[iIF][ipol][1];

      for (int j = 0; j < 24; j++) {
        baseSub[iIF][ipol][j] = other.baseSub[iIF][ipol][j];
      }
    }

    for (int j = 0; j < 2; j++) {
      tcal[iIF][j] = other.tcal[iIF][j];
    }
  }

  haveSpectra = other.haveSpectra;
  if (haveSpectra) {
    for (int iIF = 0; iIF < nIF; iIF++) {
      int nprod = nChan[iIF] * nPol[iIF];
      int nxpol = other.xpol[iIF] ? nChan[iIF] * 2 : 0;
      allocate(iIF, nprod, nxpol);
    }

    // Copy data.
    for (int iIF = 0; iIF < nIF; iIF++) {
      float *specp  = spectra[iIF];
      float *ospecp = other.spectra[iIF];
      unsigned char *flagp  = flagged[iIF];
      unsigned char *oflagp = other.flagged[iIF];
      for (int j = 0; j < nChan[iIF]*nPol[iIF]; j++) {
        *(specp++) = *(ospecp++);
        *(flagp++) = *(oflagp++);
      }

      if (xpol[iIF]) {
        float *xpolp  = xpol[iIF];
        float *oxpolp = other.xpol[iIF];
        for (int j = 0; j < 2*nChan[iIF]; j++) {
          *(xpolp++) = *(oxpolp++);
        }
      }
    }
  }

  extraSysCal = other.extraSysCal;

  azimuth   = other.azimuth;
  elevation = other.elevation;
  parAngle  = other.parAngle;
  paRate    = other.paRate;

  focusAxi  = other.focusAxi;
  focusTan  = other.focusTan;
  focusRot  = other.focusRot;

  temp      = other.temp;
  pressure  = other.pressure;
  humidity  = other.humidity;
  windSpeed = other.windSpeed;
  windAz    = other.windAz;

  strcpy(tcalTime, other.tcalTime);

  refBeam = other.refBeam;

  return *this;
}

//---------------------------------------------------------- MBrecord::extract

// Extract a selected IF from one MBrecord into another.

int MBrecord::extract(const MBrecord &other, int iIF)
{
  if (this == &other) {
    return 1;
  }

  setNIFs(1);

  scanNo  = other.scanNo;
  cycleNo = other.cycleNo;
  strcpy(datobs, other.datobs);
  utc = other.utc;

  exposure = other.exposure;
  strcpy(srcName, other.srcName);
  srcRA  = other.srcRA;
  srcDec = other.srcDec;
  restFreq = other.restFreq;
  strcpy(obsType, other.obsType);

  // Beam-dependent parameters.
  beamNo  = other.beamNo;
  ra      = other.ra;
  dec     = other.dec;
  pCode   = other.pCode;
  rateAge = other.rateAge;
  raRate  = other.raRate;
  decRate = other.decRate;
  paRate  = other.paRate;

  // IF-dependent parameters.
  nIF = 1;
  IFno[0]     = other.IFno[iIF];
  nChan[0]    = other.nChan[iIF];
  nPol[0]     = other.nPol[iIF];
  fqRefPix[0] = other.fqRefPix[iIF];
  fqRefVal[0] = other.fqRefVal[iIF];
  fqDelt[0]   = other.fqDelt[iIF];

  for (int j = 0; j < 2; j++) {
    tsys[0][j] = other.tsys[iIF][j];
  }

  for (int j = 0; j < 2; j++) {
    calfctr[0][j]  = other.calfctr[iIF][j];
    xcalfctr[0][j] = other.xcalfctr[iIF][j];
  }

  haveBase = other.haveBase;
  for (int ipol = 0; ipol < nPol[0]; ipol++) {
    baseLin[0][ipol][0] = other.baseLin[iIF][ipol][0];
    baseLin[0][ipol][1] = other.baseLin[iIF][ipol][1];

    for (int j = 0; j < 24; j++) {
      baseSub[0][ipol][j] = other.baseSub[iIF][ipol][j];
    }
  }

  for (int j = 0; j < 2; j++) {
    tcal[0][j] = other.tcal[iIF][j];
  }

  haveSpectra = other.haveSpectra;
  if (haveSpectra) {
    int nprod = nChan[0] * nPol[0];
    int nxpol = other.xpol[iIF] ? nChan[0] * 2 : 0;
    allocate(0, nprod, nxpol);

    // Copy data.
    float *specp  = spectra[0];
    float *ospecp = other.spectra[iIF];
    unsigned char *flagp  = flagged[0];
    unsigned char *oflagp = other.flagged[iIF];
    for (int j = 0; j < nChan[0]*nPol[0]; j++) {
      *(specp++) = *(ospecp++);
      *(flagp++) = *(oflagp++);
    }

    if (xpol[0]) {
      float *xpolp  = xpol[0];
      float *oxpolp = other.xpol[iIF];
      for (int j = 0; j < 2*nChan[0]; j++) {
        *(xpolp++) = *(oxpolp++);
      }
    }
  }

  extraSysCal = other.extraSysCal;

  azimuth   = other.azimuth;
  elevation = other.elevation;
  parAngle  = other.parAngle;

  focusAxi  = other.focusAxi;
  focusTan  = other.focusTan;
  focusRot  = other.focusRot;

  temp      = other.temp;
  pressure  = other.pressure;
  humidity  = other.humidity;
  windSpeed = other.windSpeed;
  windAz    = other.windAz;

  strcpy(tcalTime, other.tcalTime);

  refBeam = other.refBeam;

  return 0;
}

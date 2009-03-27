//#---------------------------------------------------------------------------
//# PKSMBrecord.h: Class to store an MBFITS single-dish data record.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
//# Mark Calabretta, ATNF
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
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: mcalabre@atnf.csiro.au.
//#        Postal address: Dr. Mark Calabretta,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id$
//#---------------------------------------------------------------------------
//# The PKSMBrecord class stores an MBFITS single-dish data record.
//#
//# Storage for spectral data may be managed in either of two ways:
//#
//#   1) The allocate() member function may be used to allocate storage that
//#      is subsequently managed by the PKSMBrecord class; the assignment
//#      operator automatically deletes and reallocates more if insufficient
//#      was provided, and the PKSMBrecord destructor deletes it.
//#
//#      Allocation of storage for cross-polarization data is optional.
//#
//#   2) In some cases it may be desirable for the user to provide storage via
//#      the 'spectra', and 'flagged' (and 'xpol') variables in order to avoid
//#      in-core copying.  It is assumed that space has been provided for at
//#      least nChan*nPol floats for 'spectra', nChan*nPol unsigned chars for
//#      'flagged', and 2*nChan floats for 'xpol'.  This storage will not be
//#      reassigned by the assignment operator nor deleted by the destructor.
//#
//# The two methods may not be mixed; allocate() checks that either
//#
//#   a) the 'spectra', 'flagged', and 'xpol' variables are null pointers,
//#
//#   b) storage was previously allocated via allocate().
//#
//# Original: 2000/08/01 Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_PKSMBRECORD_H
#define ATNF_PKSMBRECORD_H

// <summary>
// Class to store an MBFITS single-dish data record.
// </summary>

class PKSMBrecord
{
  public:
    // Default constructor allocates arrays for the required number of IFs.
    PKSMBrecord(int nIF = 0);

    // Destructor; deletes any storage that may have been auto-allocated by
    // the assignment operator.
    ~PKSMBrecord();

    // Expand arrays if necessary to accomodate the required number of IFs.
    void setNIFs(int nIF);

    // Ensure there is enough storage for the specified number of spectral
    // products (channels x polarizations) for IF with array index iif (i.e.
    // the actual IF number is IFno[iif]).  Expands arrays if necessary but
    // never contracts.
    void allocate(int iIF, int nprod, int nxpol);

    // Free all allocate()'d storage.
    void free();

    // The assignment operator does a deep copy and will auto-allocate or
    // re-allocate data storage if necessary.
    PKSMBrecord &operator=(const PKSMBrecord &other);

    // Extract a selected IF from a PKSMBrecord into another.
    int extract(const PKSMBrecord &other, int iIF);

    int    scanNo;		// Scan number.
    int    cycleNo;		// Integration cycle number.
    char   datobs[11];		// Date of observation YYYY-MM-DD.
    double utc;			// UTC of the integration, s.
    float  exposure;		// Integration time, s.
    char   srcName[17];		// Source name.
    double srcRA;		// Source J2000 right ascension, radian.
    double srcDec;		// Source J2000 declination, radian.
    double restFreq;		// Line rest frequency, Hz.
    char   obsType[16];		// Two-letter observation type codes.

    // Beam-dependent parameters.
    short  beamNo;		// Multibeam beam number.
    double ra;			// J2000 right ascension, radian.
    double dec;			// J2000 declination, radian,
    float  raRate;		// Scan rate in right ascension, radian/s.
    float  decRate;		// Scan rate in declination, radian/s.

    // IF-dependent parameters.
    short  nIF;			// Number of IFs.
    short  *IFno;		// IF number.
    int    *nChan;		// Number of channels.
    int    *nPol;		// Number of polarizations.
    float  *fqRefPix;		// Frequency reference pixel.
    double *fqRefVal;		// Frequency reference value, Hz.
    double *fqDelt;		// Frequency separation between channels, Hz.
    float  (*tsys)[2];		// Tsys for each polarization, Jy.
    float  (*calfctr)[2];	// Calibration factor for each polarization.
    float  (*xcalfctr)[2];	// Calibration factor for cross-polarizations.
    int    haveBase;		// Are baseline parameters present?
    float  (*baseLin)[2][2];	// Linear baseline fit for each polarization.
    float  (*baseSub)[2][9];	// Polynomial baseline subtracted.
    int    haveSpectra;		// Is spectral data present?
    float* *spectra;		// Spectra for each polarization, Jy.
    unsigned char* *flagged;	// Channel flagging, 0 = good, else bad.
    float* *xpol;		// Cross polarization spectra (if any).

    // Only present for Parkes Multibeam or LBA data after 1997/02/02.
    float   (*tcal)[2];		// Tcal for each polarization.

    // Extra syscal data available for Parkes Multibeam observations only.
    int    extraSysCal;		// Is this extra SysCal data available?
    float  azimuth;		// Azimuth, radian.
    float  elevation;		// Elevation, radian.
    float  parAngle;		// Parallactic angle, radian.
    float  focusAxi;		// Axial focus position, m.
    float  focusTan;		// Focus platform translation, m.
    float  focusRot;		// Focus rotation, radian.
    float  temp;		// Temperature, C.
    float  pressure;		// Pressure, Pa.
    float  humidity;		// Relative humidity, %.
    float  windSpeed;		// Wind speed, m/s.
    float  windAz;		// Wind azimuth, radian.
    char   tcalTime[17];	// Time of measurement of cal signals.
    short  refBeam;		// Reference beam, in beam-switching (MX)
				// mode, added 1999/03/17.

  private:
    int    cNIF;		// Number of IFs allocated.
    int*   cNProd;		// Storage allocated for data.
    int*   cNXPol;		// Storage allocated for xpol.
};

#endif

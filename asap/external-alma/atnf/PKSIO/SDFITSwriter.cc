//#---------------------------------------------------------------------------
//# SDFITSwriter.cc: ATNF CFITSIO interface class for SDFITS output.
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
//# $Id: SDFITSwriter.cc,v 19.18 2009-09-29 07:33:39 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2000/07/24, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/MBrecord.h>
#include <atnf/PKSIO/SDFITSwriter.h>

#include <casa/Logging/LogIO.h>

#include <casa/iostream.h>

#include <algorithm>
#include <math.h>
#include <cstring>

using namespace std;

// Numerical constants.
const double PI  = 3.141592653589793238462643;

// Factor to convert radians to degrees.
const double R2D = 180.0 / PI;

// Class name
const string className = "SDFITSwriter" ;

//------------------------------------------------- SDFITSwriter::SDFITSwriter

SDFITSwriter::SDFITSwriter()
{
  // Default constructor.
  cSDptr = 0x0;
}

//------------------------------------------------ SDFITSwriter::~SDFITSwriter

SDFITSwriter::~SDFITSwriter()
{
  close();
}

//------------------------------------------------------- SDFITSwriter::create

// Create the output SDFITS file.

int SDFITSwriter::create(
        char*  sdName,
        char*  observer,
        char*  project,
        char*  telescope,
        double antPos[3],
        char*  obsMode,
        char*  bunit,
        float  equinox,
        char*  dopplerFrame,
        int    nIF,
        int*   nChan,
        int*   nPol,
        int*   haveXPol,
        int    haveBase,
        int    extraSysCal)
{
  const string methodName = "create()" ;

  if (cSDptr) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Output file already open, close it first.");
    return 1;
  }

  // Prepend an '!' to the output name to force it to be overwritten.
  char sdname[80];
  sdname[0] = '!';
  strcpy(sdname+1, sdName);

  // Create a new SDFITS file.
  cStatus = 0;
  if (fits_create_file(&cSDptr, sdname, &cStatus)) {
    sprintf(cMsg, "Failed to create SDFITS file\n       %s", sdName);
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, cMsg);
    return cStatus;
  }

  cIsMX  = strstr(obsMode, "MX") != 0;
  cNIF   = nIF;
  cNChan = nChan;
  cNPol  = nPol;
  cHaveXPol = haveXPol;
  cHaveBase = haveBase;
  cExtraSysCal = extraSysCal;

  // Do all IFs have the same number of channels and polarizations?
  cDoTDIM = 0;
  int nprod = cNChan[0] * cNPol[0];
  for (int iIF = 0; iIF < nIF; iIF++) {
    if (cNChan[iIF]*cNPol[iIF] != nprod) {
      // Need variable-length arrays as well as a TDIM column.
      cDoTDIM = 2;
      break;
    }

    if (cNChan[iIF] != cNChan[0] || cNPol[iIF] != cNPol[0]) {
      // Varying channels and/or polarizations, need a TDIM column at least.
      cDoTDIM = 1;
    }
  }

  // Find the maximum number of polarizations in any IF.
  int maxNPol = 0;
  for (int iIF = 0; iIF < nIF; iIF++) {
    if (cNPol[iIF] > maxNPol) maxNPol = cNPol[iIF];
  }

  // Do any IFs have cross-polarizations?
  cDoXPol = 0;
  for (int iIF = 0; iIF < nIF; iIF++) {
    if (cHaveXPol[iIF]) {
      cDoXPol = 1;
      break;
    }
  }


  cRow = 0;

  // Write required primary header keywords.
  if (fits_write_imghdr(cSDptr, 8, 0, 0, &cStatus)) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to write required primary header keywords.");
    return cStatus;
  }

  // Identify the origin of the data.
  fits_write_comment(cSDptr, " ", &cStatus);
  fits_write_comment(cSDptr,
    "This single dish FITS (SDFITS) file has a binary table extension which",
     &cStatus);
  fits_write_comment(cSDptr,
    "contains data obtained from a telescope run by the Australia Telescope",
     &cStatus);
  fits_write_comment(cSDptr, "National Facility (ATNF).", &cStatus);
  fits_write_comment(cSDptr, " ", &cStatus);

  fits_write_date(cSDptr, &cStatus);

  char text[72];
  char version[7];
  char date[11];
  sscanf("$Revision: 19.18 $", "%*s%s", version);
  sscanf("$Date: 2009-09-29 07:33:39 $", "%*s%s", date);
  sprintf(text, "SDFITSwriter (v%s, %s)", version, date);
  fits_write_key_str(cSDptr, "ORIGIN", text, "output class", &cStatus);

  float cfvers;
  fits_write_comment(cSDptr, "Written by Mark Calabretta "
                     "(mcalabre@atnf.csiro.au)", &cStatus);
  sprintf(text, "using cfitsio v%.3f.", fits_get_version(&cfvers));
  fits_write_comment(cSDptr, text, &cStatus);

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed in writing primary header.");
    return cStatus;
  }


  // Create an SDFITS extension.
  long nrow = 0;
  int  ncol = 0;
  if (fits_create_tbl(cSDptr, BINARY_TBL, nrow, ncol, NULL, NULL, NULL,
      "SINGLE DISH", &cStatus)) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to create a binary table extension.");
    return 1;
  }

  char ttype[16];
  char tform[9];
  char tunit[9];

  // NMATRIX (core, virtual).
  fits_write_key_lng(cSDptr, "NMATRIX", 1l, "Number of DATA arrays",
                     &cStatus);

  // OBSERVER (shared, virtual).
  fits_write_key_str(cSDptr, "OBSERVER", observer, "Observer name(s)",
                     &cStatus);

  // PROJID (shared, virtual).
  fits_write_key_str(cSDptr, "PROJID", project, "Project name", &cStatus);

  // TELESCOP (core, virtual).
  fits_write_key_str(cSDptr, "TELESCOP", telescope, "Telescope name",
                     &cStatus);

  // OBSGEO-X/Y/Z (additional, virtual).
  fits_write_key_dbl(cSDptr, "OBSGEO-X", antPos[0], 9,
                     "[m] Antenna ITRF X-coordinate", &cStatus);
  fits_write_key_dbl(cSDptr, "OBSGEO-Y", antPos[1], 9,
                     "[m] Antenna ITRF Y-coordinate", &cStatus);
  fits_write_key_dbl(cSDptr, "OBSGEO-Z", antPos[2], 9,
                     "[m] Antenna ITRF Z-coordinate", &cStatus);

  // SCAN (shared, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"SCAN", (char *)"1I", &cStatus);

  // CYCLE (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"CYCLE", (char *)"1J", &cStatus);

  // DATE-OBS (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"DATE-OBS", (char *)"10A", &cStatus);

  // TIME (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"TIME", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "s", "units of field", &cStatus);

  // EXPOSURE (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"EXPOSURE", (char *)"1E", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "s", "units of field", &cStatus);

  // OBJECT (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"OBJECT", (char *)"16A", &cStatus);

  // OBJ-RA (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"OBJ-RA", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

  // OBJ-DEC (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"OBJ-DEC", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

  // RESTFRQ (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"RESTFRQ",(char *) "1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "Hz", "units of field", &cStatus);

  // OBSMODE (shared, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"OBSMODE", (char *)"16A", &cStatus);

  // BEAM (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"BEAM", (char *)"1I", &cStatus);

  // IF (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"IF", (char *)"1I", &cStatus);

  // FREQRES (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"FREQRES", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "Hz", "units of field", &cStatus);

  // BANDWID (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"BANDWID", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "Hz", "units of field", &cStatus);

  // CTYPE1 (core, virtual).
  fits_write_key_str(cSDptr, "CTYPE1", "FREQ",
                     "DATA array axis 1: frequency in Hz.", &cStatus);

  // CRPIX1 (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"CRPIX1", (char *)"1E", &cStatus);

  // CRVAL1 (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"CRVAL1", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "Hz", "units of field", &cStatus);

  // CDELT1 (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"CDELT1", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "Hz", "units of field", &cStatus);


  // CTYPE2 (core, virtual).
  fits_write_key_str(cSDptr, "CTYPE2", "STOKES",
                     "DATA array axis 2: polarization code", &cStatus);

  // CRPIX2 (core, virtual).
  fits_write_key_flt(cSDptr, "CRPIX2", 1.0f, 1,
                     "Polarization code reference pixel", &cStatus);

  // CRVAL2 (core, virtual).
  fits_write_key_dbl(cSDptr, "CRVAL2", -5.0, 1,
                     "Polarization code at reference pixel (XX)", &cStatus);

  // CDELT2 (core, virtual).
  fits_write_key_dbl(cSDptr, "CDELT2", -1.0, 1,
                     "Polarization code axis increment", &cStatus);


  // CTYPE3 (core, virtual).
  fits_write_key_str(cSDptr, "CTYPE3", "RA",
                     "DATA array axis 3 (degenerate): RA (mid-int)",
                     &cStatus);

  // CRPIX3 (core, virtual).
  fits_write_key_flt(cSDptr, "CRPIX3", 1.0f, 1, "RA reference pixel",
                     &cStatus);

  // CRVAL3 (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"CRVAL3", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

  // CDELT3 (core, virtual).
  fits_write_key_dbl(cSDptr, "CDELT3", -1.0, 1, "RA axis increment", &cStatus);

  // CTYPE4 (core, virtual).
  fits_write_key_str(cSDptr, "CTYPE4", "DEC",
                     "DATA array axis 4 (degenerate): Dec (mid-int)",
                     &cStatus);

  // CRPIX4 (core, virtual).
  fits_write_key_flt(cSDptr, "CRPIX4", 1.0f, 1, "Dec reference pixel",
                     &cStatus);

  // CRVAL4 (core, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"CRVAL4", (char *)"1D", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

  // CDELT4 (core, virtual).
  fits_write_key_dbl(cSDptr, "CDELT4", 1.0, 1, "Dec axis increment", &cStatus);

  // SCANRATE (additional, real).
  fits_insert_col(cSDptr, ++ncol, (char *)"SCANRATE", (char *)"2E", &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, "deg/s", "units of field", &cStatus);

  // SPECSYS (additional, virtual).
  fits_write_key_str(cSDptr, "SPECSYS", dopplerFrame,
                     "Doppler reference frame (transformed)", &cStatus);

  // SSYSOBS (additional, virtual).
  fits_write_key_str(cSDptr, "SSYSOBS", "TOPOCENT",
                     "Doppler reference frame of observation", &cStatus);

  // EQUINOX (shared, virtual).
  fits_write_key_flt(cSDptr, "EQUINOX", equinox, 1,
                     "Equinox of equatorial coordinates", &cStatus);

  // RADESYS (additional, virtual).
  fits_write_key_str(cSDptr, "RADESYS", "FK5", "Equatorial coordinate frame",
                     &cStatus);

  // TSYS (core, real).
  sprintf(tform, "%dE", maxNPol);
  fits_insert_col(cSDptr, ++ncol, (char *)"TSYS", tform, &cStatus);
  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, bunit, "units of field", &cStatus);

  // CALFCTR (additional, real).
  sprintf(tform, "%dE", maxNPol);
  fits_insert_col(cSDptr, ++ncol, (char *)"CALFCTR", tform, &cStatus);

  if (cHaveBase) {
    // BASELIN (additional, real).
    sprintf(tform, "%dE", 2*maxNPol);
    fits_insert_col(cSDptr, ++ncol, (char *)"BASELIN", tform, &cStatus);
    long tdim[] = {2, maxNPol};
    fits_write_tdim(cSDptr, ncol, 2, tdim, &cStatus);

    // BASESUB (additional, real).
    sprintf(tform, "%dE", 24*maxNPol);
    fits_insert_col(cSDptr, ++ncol, (char *)"BASESUB", tform, &cStatus);
    tdim[0] = 24;
    fits_write_tdim(cSDptr, ncol, 2, tdim, &cStatus);
  }

  // DATA (core, real).
  if (cDoTDIM < 2) {
    // IFs all have the same number of products, use fixed-length arrays.
    sprintf(tform, "%dE", cNChan[0]*cNPol[0]);
  } else {
    // IFs have a differing number of products, use variable-length arrays.
    strcpy(tform, "1PE");
  }
  fits_insert_col(cSDptr, ++ncol, (char *)"DATA", tform, &cStatus);

  if (cDoTDIM) {
    // TDIMn varies with IF, write a TDIM column.
    sprintf(ttype, "TDIM%d", ncol);
    fits_insert_col(cSDptr, ++ncol, ttype, (char *)"16A", &cStatus);
  } else {
    // TDIMn fixed for each IF, write a TDIM keyword.
    long tdim[] = {cNChan[0], cNPol[0], 1, 1};
    fits_write_tdim(cSDptr, ncol, 4, tdim, &cStatus);
  }

  sprintf(tunit, "TUNIT%d", ncol);
  fits_write_key_str(cSDptr, tunit, bunit, "units of field", &cStatus);

  // FLAGGED (additional, logical).
  if (cDoTDIM < 2) {
    // IFs all have the same number of products, use fixed-length arrays.
    sprintf(tform, "%dB", cNChan[0]*cNPol[0]);
  } else {
    // IFs have a differing number of products, use variable-length arrays.
    strcpy(tform, "1PB");
  }
  fits_insert_col(cSDptr, ++ncol, (char *)"FLAGGED", tform, &cStatus);

  if (cDoTDIM) {
    // TDIMn varies with IF, write a TDIM column.
    sprintf(ttype, "TDIM%d", ncol);
    fits_insert_col(cSDptr, ++ncol, ttype, (char *)"16A", &cStatus);
  } else {
    // TDIMn fixed for each IF, write a TDIM keyword.
    long tdim[] = {cNChan[0], cNPol[0], 1, 1};
    fits_write_tdim(cSDptr, ncol, 4, tdim, &cStatus);
  }

  if (cDoXPol) {
    // XCALFCTR (additional, real).
    sprintf(tform, "%dE", 2);
    fits_insert_col(cSDptr, ++ncol, (char *)"XCALFCTR", tform, &cStatus);

    // XPOLDATA (additional, real).
    if (cDoTDIM < 2) {
      // IFs all have the same number of products, use fixed-length arrays.
      sprintf(tform, "%dE", 2*cNChan[0]);
    } else {
      // IFs have a differing number of products, use variable-length arrays.
      strcpy(tform, "1PE");
    }
    fits_insert_col(cSDptr, ++ncol, (char *)"XPOLDATA", tform, &cStatus);

    if (cDoTDIM) {
      // TDIMn varies with IF, write a TDIM column.
      sprintf(ttype, "TDIM%d", ncol);
      fits_insert_col(cSDptr, ++ncol, ttype, (char *)"16A", &cStatus);
    } else {
      // TDIMn fixed for each IF, write a TDIM keyword.
      long tdim[] = {2, cNChan[0]};
      fits_write_tdim(cSDptr, ncol, 2, tdim, &cStatus);
    }

    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, bunit, "units of field", &cStatus);
  }

  if (cExtraSysCal) {
    if (cIsMX) {
      // REFBEAM (additional, real).
      fits_insert_col(cSDptr, ++ncol, (char *)"REFBEAM",  (char *)"1I", &cStatus);
    }

    // TCAL (shared, real).
    sprintf(tform, "%dE", min(maxNPol,2));
    fits_insert_col(cSDptr, ++ncol, (char *)"TCAL", tform, &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "Jy", "units of field", &cStatus);

    // TCALTIME (additional, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"TCALTIME",  (char *)"16A", &cStatus);

    // AZIMUTH (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"AZIMUTH",  (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

    // ELEVATIO (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"ELEVATIO", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

    // PARANGLE (additional, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"PARANGLE", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);

    // FOCUSAXI (additional, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"FOCUSAXI", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "m", "units of field", &cStatus);

    // FOCUSTAN (additional, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"FOCUSTAN", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "m", "units of field", &cStatus);

    // FOCUSROT (additional, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"FOCUSROT", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, (char *)"deg", (char *)"units of field", &cStatus);

    // TAMBIENT (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"TAMBIENT", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "C", "units of field", &cStatus);

    // PRESSURE (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"PRESSURE", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "Pa", "units of field", &cStatus);

    // HUMIDITY (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"HUMIDITY", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "%", "units of field", &cStatus);

    // WINDSPEE (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"WINDSPEE", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "m/s", "units of field", &cStatus);

    // WINDDIRE (shared, real).
    fits_insert_col(cSDptr, ++ncol, (char *)"WINDDIRE", (char *)"1E", &cStatus);
    sprintf(tunit, "TUNIT%d", ncol);
    fits_write_key_str(cSDptr, tunit, "deg", "units of field", &cStatus);
  }

  // Set scaling parameters.
  for (int j = 1; j <= ncol; j++) {
    fits_set_tscale(cSDptr, j, 1.0, 0.0, &cStatus);
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed in writing binary table header.");
  }

  return cStatus;
}

//-------------------------------------------------------- SDFITSwriter::write

// Write a record to the SDFITS file.

int SDFITSwriter::write(MBrecord &mbrec)
{
  const string methodName = "write()" ;
  LogIO os( LogOrigin( className, methodName, WHERE ) ) ;

  char *cptr;

  // Check parameters.
  int IFno = mbrec.IFno[0];
  if (IFno < 1 || cNIF < IFno) {
    os << LogIO::WARN
       << "SDFITSwriter::write: "
       << "Invalid IF number " << IFno
       << " (maximum " << cNIF << ")." << LogIO::POST ;
    return 1;
  }

  int iIF = IFno - 1;
  int nChan = cNChan[iIF];
  if (mbrec.nChan[0] != nChan) {
    os << LogIO::WARN
       << "SDFITSriter::write: "
       << "Wrong number of channels for IF " << IFno << "," << endl
       << "                    "
       << "got " << nChan << " should be " << mbrec.nChan[0] << "." << endl;
    os << LogIO::POST ;
    return 1;
  }

  int nPol = cNPol[iIF];
  if (mbrec.nPol[0] != nPol) {
    os << LogIO::WARN
       << "SDFITSriter::write: "
       << "Wrong number of polarizations for IF " << IFno << "," << endl
       << "                    "
       << "got " << nPol << " should be " << mbrec.nPol[0] << "." << endl;
    os << LogIO::POST ;
    return 1;
  }


  // Next row.
  cRow++;

  int icol = 0;

  // SCAN.
  fits_write_col_int(cSDptr, ++icol, cRow, 1, 1, &mbrec.scanNo, &cStatus);

  // CYCLE.
  fits_write_col_int(cSDptr, ++icol, cRow, 1, 1, &mbrec.cycleNo, &cStatus);

  // DATE-OBS.
  cptr = mbrec.datobs;
  fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);

  // TIME.
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &mbrec.utc, &cStatus);

  // EXPOSURE.
  fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.exposure, &cStatus);

  // OBJECT.
  cptr = mbrec.srcName;
  fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);

  // OBJ-RA.
  double srcRA = mbrec.srcRA * R2D;
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &srcRA, &cStatus);

  // OBJ-DEC.
  double srcDec = mbrec.srcDec * R2D;
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &srcDec, &cStatus);

  // RESTFRQ.
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &mbrec.restFreq, &cStatus);

  // OBJECT.
  cptr = mbrec.obsType;
  fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);

  // BEAM.
  fits_write_col_sht(cSDptr, ++icol, cRow, 1, 1, &mbrec.beamNo, &cStatus);

  // IF.
  fits_write_col_sht(cSDptr, ++icol, cRow, 1, 1, &mbrec.IFno[0], &cStatus);

  // FREQRES.
  double freqRes = fabs(mbrec.fqDelt[0]);
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &freqRes, &cStatus);

  // BANDWID.
  double bandwidth = freqRes * nChan;
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &bandwidth, &cStatus);

  // CRPIX1.
  fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.fqRefPix[0],
                     &cStatus);

  // CRVAL1.
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &mbrec.fqRefVal[0],
                     &cStatus);

  // CDELT1.
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &mbrec.fqDelt[0], &cStatus);

  // CRVAL3.
  double ra = mbrec.ra * R2D;
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &ra, &cStatus);

  // CRVAL4.
  double dec = mbrec.dec * R2D;
  fits_write_col_dbl(cSDptr, ++icol, cRow, 1, 1, &dec, &cStatus);

  // SCANRATE.
  float scanrate[2];
  scanrate[0] = mbrec.raRate  * R2D;
  scanrate[1] = mbrec.decRate * R2D;
  fits_write_col_flt(cSDptr, ++icol, cRow, 1, 2, scanrate, &cStatus);

  // TSYS.
  fits_write_col_flt(cSDptr, ++icol, cRow, 1, nPol, mbrec.tsys[0], &cStatus);

  // CALFCTR.
  fits_write_col_flt(cSDptr, ++icol, cRow, 1, nPol, mbrec.calfctr[0],
                     &cStatus);

  if (cHaveBase) {
    // BASELIN.
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 2*nPol, mbrec.baseLin[0][0],
                       &cStatus);

    // BASESUB.
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 24*nPol, mbrec.baseSub[0][0],
                       &cStatus);
  }

  // DATA.
  fits_write_col_flt(cSDptr, ++icol, cRow, 1, nChan*nPol, mbrec.spectra[0],
                     &cStatus);

  if (cDoTDIM) {
    // TDIM(DATA).
    char tdim[16];
    sprintf(tdim, "(%d,%d,1,1)", nChan, nPol);
    cptr = tdim;
    fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);
  }

  // FLAGGED.
  fits_write_col_byt(cSDptr, ++icol, cRow, 1, nChan*nPol, mbrec.flagged[0],
                     &cStatus);

  if (cDoTDIM) {
    // TDIM(FLAGGED).
    char tdim[16];
    sprintf(tdim, "(%d,%d,1,1)", nChan, nPol);
    cptr = tdim;
    fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);
  }

  if (cDoXPol) {
    if (cHaveXPol[iIF] && mbrec.xpol[0]) {
      // XCALFCTR.
      fits_write_col_flt(cSDptr, ++icol, cRow, 1, 2, mbrec.xcalfctr[0],
                         &cStatus);

      // XPOLDATA.
      fits_write_col_flt(cSDptr, ++icol, cRow, 1, 2*nChan, mbrec.xpol[0],
                        &cStatus);

      if (cDoTDIM) {
        // TDIM(XPOLDATA).
        char tdim[16];
        sprintf(tdim, "(2,%d)", nChan);
        cptr = tdim;
        fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);
      }

    } else {
      // Skip columns.
      icol += cDoTDIM ? 3 : 2;
    }
  }


  // Extra system calibration quantities from Parkes.
  if (cExtraSysCal) {
    if (cIsMX) {
      fits_write_col_sht(cSDptr, ++icol, cRow, 1, 1, &mbrec.refBeam, &cStatus);
    }

    fits_write_col_flt(cSDptr, ++icol, cRow, 1, min(nPol,2), mbrec.tcal[0],
                       &cStatus);
    cptr = mbrec.tcalTime;
    fits_write_col_str(cSDptr, ++icol, cRow, 1, 1, &cptr, &cStatus);

    float azimuth   = mbrec.azimuth   * R2D;
    float elevation = mbrec.elevation * R2D;
    float parAngle  = mbrec.parAngle  * R2D;
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &azimuth, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &elevation, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &parAngle, &cStatus);

    float focusRot = mbrec.focusRot * R2D;
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.focusAxi, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.focusTan, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &focusRot, &cStatus);

    float windAz = mbrec.windAz * R2D;
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.temp, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.pressure, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.humidity, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &mbrec.windSpeed, &cStatus);
    fits_write_col_flt(cSDptr, ++icol, cRow, 1, 1, &windAz, &cStatus);
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed in writing binary table entry.");
  }

  return cStatus;
}


//------------------------------------------------------ SDFITSwriter::history

// Write a history record.

int SDFITSwriter::history(char *text)

{
  const string methodName = "history()" ;

  if (!cSDptr) {
    return 1;
  }

  if (fits_write_history(cSDptr, text, &cStatus)) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed in writing HISTORY records.");
  }

  return cStatus;
}

//-------------------------------------------------------- SDFITSwriter::close

// Close the SDFITS file.

void SDFITSwriter::close()
{
  const string methodName = "close()" ;

  if (cSDptr) {
    cStatus = 0;
    if (fits_close_file(cSDptr, &cStatus)) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to close file.");
    }

    cSDptr = 0;
  }
}

//--------------------------------------------------- SDFITSwriter::deleteFile

// Delete the SDFITS file.

void SDFITSwriter::deleteFile()
{
  const string methodName = "deleteFile()" ;

  if (cSDptr) {
    cStatus = 0;
    if (fits_delete_file(cSDptr, &cStatus)) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to close and delete file.");
    }

    cSDptr = 0;
  }
}

//------------------------------------------------------- SDFITSwriter::log

// Log a message.  If the current CFITSIO status value is non-zero, also log
// the corresponding error message and dump the CFITSIO message stack.

void SDFITSwriter::log(LogOrigin origin, LogIO::Command cmd, const char *msg)
{
  LogIO os( origin ) ;

  os << cmd << msg << endl ;

  if (cStatus) {
    fits_get_errstatus(cStatus, cMsg);
    os << cMsg << endl ; 

    while (fits_read_errmsg(cMsg)) {
      os << cMsg << endl ;
    }
  }

  os << LogIO::POST ;
}

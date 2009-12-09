//#---------------------------------------------------------------------------
//# SDFITSreader.cc: ATNF CFITSIO interface class for SDFITS input.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2008
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
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: SDFITSreader.cc,v 19.33 2008-11-17 06:58:34 cal103 Exp $
//#---------------------------------------------------------------------------
//# The SDFITSreader class reads single dish FITS files such as those written
//# by SDFITSwriter containing Parkes Multibeam data.
//#
//# Original: 2000/08/09, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/pks/pks_maths.h>
#include <atnf/PKSIO/MBrecord.h>
#include <atnf/PKSIO/SDFITSreader.h>

#include <casa/Logging/LogIO.h>
#include <casa/Quanta/MVTime.h>
#include <casa/math.h>
#include <casa/stdio.h>
#include <cstring>

#include <algorithm>
#include <strings.h>

class FITSparm
{
  public:
    char *name;		// Keyword or column name.
    int  type;		// Expected keyvalue or column data type.
    int  colnum;	// Column number; 0 for keyword; -1 absent.
    int  coltype;	// Column data type, as found.
    long nelem;		// Column data repeat count; < 0 for vardim.
    int  tdimcol;	// TDIM column number; 0 for keyword; -1 absent.
    char units[32];	// Units from TUNITn keyword.
};

// Numerical constants.
const double PI  = 3.141592653589793238462643;

// Factor to convert radians to degrees.
const double D2R = PI / 180.0;

// Class name
const string className = "SDFITSreader" ;

//------------------------------------------------- SDFITSreader::SDFITSreader

SDFITSreader::SDFITSreader()
{
  // Default constructor.
  cSDptr = 0;

  // Allocate space for data descriptors.
  cData = new FITSparm[NDATA];

  for (int iData = 0; iData < NDATA; iData++) {
    cData[iData].colnum = -1;
  }

  // Initialize pointers.
  cBeams     = 0x0;
  cIFs       = 0x0;
  cStartChan = 0x0;
  cEndChan   = 0x0;
  cRefChan   = 0x0;
  cPols      = 0x0;
}

//------------------------------------------------ SDFITSreader::~SDFITSreader

SDFITSreader::~SDFITSreader()
{
  close();

  delete [] cData;
}

//--------------------------------------------------------- SDFITSreader::open

// Open an SDFITS file for reading.

int SDFITSreader::open(
        char*  sdName,
        int    &nBeam,
        int*   &beams,
        int    &nIF,
        int*   &IFs,
        int*   &nChan,
        int*   &nPol,
        int*   &haveXPol,
        int    &haveBase,
        int    &haveSpectra,
        int    &extraSysCal)
{
  const string methodName = "open()" ;

  if (cSDptr) {
    close();
  }

  // Open the SDFITS file.
  cStatus = 0;
  if (fits_open_file(&cSDptr, sdName, READONLY, &cStatus)) {
    sprintf(cMsg, "ERROR: Failed to open SDFITS file\n       %s", sdName);
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, cMsg);
    return 1;
  }

  // Move to the SDFITS extension.
  cALFA = cALFA_BD = cALFA_CIMA = 0;
  if (fits_movnam_hdu(cSDptr, BINARY_TBL, "SINGLE DISH", 0, &cStatus)) {
    // No SDFITS table, look for BDFITS or CIMAFITS.
    cStatus = 0;
    if (fits_movnam_hdu(cSDptr, BINARY_TBL, "BDFITS", 0, &cStatus) == 0) {
      cALFA_BD = 1;

    } else {
      cStatus = 0;
      if (fits_movnam_hdu(cSDptr, BINARY_TBL, "CIMAFITS", 0, &cStatus) == 0) {
        cALFA_CIMA = 1;

        // Check for later versions of CIMAFITS.
        float version;
        readParm("VERSION", TFLOAT, &version);
        if (version >= 2.0f) cALFA_CIMA = int(version);

      } else {
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to locate SDFITS binary table.");
        close();
        return 1;
      }
    }

    // Arecibo ALFA data of some kind.
    cALFA = 1;
    for (int iBeam = 0; iBeam < 8; iBeam++) {
      for (int iPol = 0; iPol < 2; iPol++) {
        cALFAcalOn[iBeam][iPol]  = 0.0f;
        cALFAcalOff[iBeam][iPol] = 0.0f;

        // Nominal factor to calibrate spectra in Jy.
        cALFAcal[iBeam][iPol] = 3.0f;
      }
    }
  }

  // GBT data.
  char telescope[32];
  readParm("TELESCOP", TSTRING, telescope);      // Core.
  cGBT = strncmp(telescope, "GBT", 3) == 0 ||
         strncmp(telescope, "NRAO_GBT", 8) == 0;

  cRow = 0;


  // Check that the DATA array column is present.
  findData(DATA, "DATA", TFLOAT);
  haveSpectra = cHaveSpectra = cData[DATA].colnum > 0;

  if (cHaveSpectra) {
    // Find the number of data axes (must be the same for each IF).
    cNAxis = 5;
    if (readDim(DATA, 1, &cNAxis, cNAxes)) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      close();
      return 1;
    }

    if (cALFA_BD) {
      // ALFA BDFITS: variable length arrays don't actually vary and there is
      // no TDIM (or MAXISn) card; use the LAGS_IN value.
      cNAxis = 5;
      readParm("LAGS_IN", TLONG, cNAxes);
      cNAxes[1] = 1;
      cNAxes[2] = 1;
      cNAxes[3] = 1;
      cNAxes[4] = 1;
      cData[DATA].nelem = cNAxes[0];
    }

    if (cNAxis < 4) {
      // Need at least four axes (for now).
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "DATA array contains fewer than four axes.");
      close();
      return 1;
    } else if (cNAxis > 5) {
      // We support up to five axes.
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "DATA array contains more than five axes.");
      close();
      return 1;
    }

    findData(FLAGGED, "FLAGGED", TBYTE);

  } else {
    // DATA column not present, check for a DATAXED keyword.
    findData(DATAXED, "DATAXED", TSTRING);
    if (cData[DATAXED].colnum < 0) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "DATA array column absent from binary table.");
      close();
      return 1;
    }

    // Determine the number of axes and their length.
    char dataxed[32];
    readParm("DATAXED", TSTRING, dataxed);

    for (int iaxis = 0; iaxis < 5; iaxis++) cNAxes[iaxis] = 0;
    sscanf(dataxed, "(%ld,%ld,%ld,%ld,%ld)", cNAxes, cNAxes+1, cNAxes+2,
      cNAxes+3, cNAxes+4);
    for (int iaxis = 4; iaxis > -1; iaxis--) {
      if (cNAxes[iaxis] == 0) cNAxis = iaxis;
    }
  }

  char  *CTYPE[5] = {"CTYPE1", "CTYPE2", "CTYPE3", "CTYPE4", "CTYPE5"};
  char  *CRPIX[5] = {"CRPIX1", "CRPIX2", "CRPIX3", "CRPIX4", "CRPIX5"};
  char  *CRVAL[5] = {"CRVAL1", "CRVAL2", "CRVAL3", "CRVAL4", "CRVAL5"};
  char  *CDELT[5] = {"CDELT1", "CDELT2", "CDELT3", "CDELT4", "CDELT5"};

  // Find required DATA array axes.
  char ctype[5][72];
  for (int iaxis = 0; iaxis < cNAxis; iaxis++) {
    strcpy(ctype[iaxis], "");
    readParm(CTYPE[iaxis], TSTRING, ctype[iaxis]);      // Core.
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    close();
    return 1;
  }

  char *fqCRPIX  = 0;
  char *fqCRVAL  = 0;
  char *fqCDELT  = 0;
  char *raCRVAL  = 0;
  char *decCRVAL = 0;
  char *timeCRVAL = 0;
  char *beamCRVAL = 0;
  char *polCRVAL = 0;

  for (int iaxis = 0; iaxis < cNAxis; iaxis++) {
    if (strncmp(ctype[iaxis], "FREQ", 4) == 0) {
      cReqax[0] = iaxis;
      fqCRPIX  = CRPIX[iaxis];
      fqCRVAL  = CRVAL[iaxis];
      fqCDELT  = CDELT[iaxis];

    } else if (strncmp(ctype[iaxis], "STOKES", 6) == 0) {
      cReqax[1] = iaxis;
      polCRVAL = CRVAL[iaxis];

    } else if (strncmp(ctype[iaxis], "RA", 2) == 0) {
      cReqax[2] = iaxis;
      raCRVAL  = CRVAL[iaxis];

    } else if (strncmp(ctype[iaxis], "DEC", 3) == 0) {
      cReqax[3] = iaxis;
      decCRVAL = CRVAL[iaxis];

    } else if (strcmp(ctype[iaxis], "TIME") == 0) {
      // TIME (UTC seconds since midnight) can be a keyword or axis type.
      timeCRVAL = CRVAL[iaxis];

    } else if (strcmp(ctype[iaxis], "BEAM") == 0) {
      // BEAM can be a keyword or axis type.
      beamCRVAL = CRVAL[iaxis];
    }
  }

  if (cALFA_BD) {
    // Fixed in ALFA CIMAFITS.
    cReqax[2] = 2;
    raCRVAL = "CRVAL2A";

    cReqax[3] = 3;
    decCRVAL = "CRVAL3A";
  }

  // Check that all are present.
  for (int iaxis = 0; iaxis < 4; iaxis++) {
    if (cReqax[iaxis] < 0) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Could not find required DATA array axes.");
      close();
      return 1;
    }
  }

  // Set up machinery for data retrieval.
  findData(SCAN,     "SCAN",     TINT);         // Shared.
  findData(CYCLE,    "CYCLE",    TINT);         // Additional.
  findData(DATE_OBS, "DATE-OBS", TSTRING);      // Core.
  findData(TIME,     "TIME",     TDOUBLE);      // Core.
  findData(EXPOSURE, "EXPOSURE", TFLOAT);       // Core.
  findData(OBJECT,   "OBJECT",   TSTRING);      // Core.
  findData(OBJ_RA,   "OBJ-RA",   TDOUBLE);      // Additional.
  findData(OBJ_DEC,  "OBJ-DEC",  TDOUBLE);      // Additional.
  findData(RESTFRQ,  "RESTFRQ",  TDOUBLE);      // Additional.
  findData(OBSMODE,  "OBSMODE",  TSTRING);      // Shared.

  findData(BEAM,     "BEAM",     TSHORT);       // Additional.
  findData(IF,       "IF",       TSHORT);       // Additional.
  findData(FqRefPix,  fqCRPIX,   TFLOAT);       // Frequency reference pixel.
  findData(FqRefVal,  fqCRVAL,   TDOUBLE);      // Frequency reference value.
  findData(FqDelt,    fqCDELT,   TDOUBLE);      // Frequency increment.
  findData(RA,        raCRVAL,   TDOUBLE);      // Right ascension.
  findData(DEC,      decCRVAL,   TDOUBLE);      // Declination.
  findData(SCANRATE, "SCANRATE", TFLOAT);       // Additional.

  findData(TSYS,     "TSYS",     TFLOAT);       // Core.
  findData(CALFCTR,  "CALFCTR",  TFLOAT);       // Additional.
  findData(XCALFCTR, "XCALFCTR", TFLOAT);       // Additional.
  findData(BASELIN,  "BASELIN",  TFLOAT);       // Additional.
  findData(BASESUB,  "BASESUB",  TFLOAT);       // Additional.
  findData(XPOLDATA, "XPOLDATA", TFLOAT);       // Additional.

  findData(REFBEAM,  "REFBEAM",  TSHORT);       // Additional.
  findData(TCAL,     "TCAL",     TFLOAT);       // Shared.
  findData(TCALTIME, "TCALTIME", TSTRING);      // Additional.
  findData(AZIMUTH,  "AZIMUTH",  TFLOAT);       // Shared.
  findData(ELEVATIO, "ELEVATIO", TFLOAT);       // Shared.
  findData(PARANGLE, "PARANGLE", TFLOAT);       // Additional.
  findData(FOCUSAXI, "FOCUSAXI", TFLOAT);       // Additional.
  findData(FOCUSTAN, "FOCUSTAN", TFLOAT);       // Additional.
  findData(FOCUSROT, "FOCUSROT", TFLOAT);       // Additional.
  findData(TAMBIENT, "TAMBIENT", TFLOAT);       // Shared.
  findData(PRESSURE, "PRESSURE", TFLOAT);       // Shared.
  findData(HUMIDITY, "HUMIDITY", TFLOAT);       // Shared.
  findData(WINDSPEE, "WINDSPEE", TFLOAT);       // Shared.
  findData(WINDDIRE, "WINDDIRE", TFLOAT);       // Shared.

  findData(STOKES,    polCRVAL,  TINT);
  findData(SIG,       "SIG",     TSTRING);
  findData(CAL,       "CAL",     TSTRING);

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    close();
    return 1;
  }


  // Check for alternative column names.
  if (cALFA) {
    // ALFA data.
    cALFAscan = 0;
    cScanNo = 0;
    if (cALFA_CIMA) {
      findData(SCAN,  "SCAN_ID", TINT);
      if (cALFA_CIMA > 1) {
        findData(CYCLE, "RECNUM", TINT);
      } else {
        findData(CYCLE, "SUBSCAN", TINT);
      }
    } else if (cALFA_BD) {
      findData(SCAN,  "SCAN_NUMBER", TINT);
      findData(CYCLE, "PATTERN_NUMBER", TINT);
    }
  } else {
    readData(SCAN, 1, &cFirstScanNo);
  }

  cCycleNo = 0;
  cLastUTC = 0.0;
  for ( int i = 0 ; i < 4 ; i++ ) {
    cGLastUTC[i] = 0.0 ;
    cGLastScan[i] = -1 ;
    cGCycleNo[i] = 0 ;
  }

  // Beam number, 1-relative by default.
  cBeam_1rel = 1;
  if (cALFA) {
    // ALFA INPUT_ID, 0-relative (overrides BEAM column if present).
    findData(BEAM, "INPUT_ID", TSHORT);
    cBeam_1rel = 0;

  } else if (cData[BEAM].colnum < 0) {
    if (beamCRVAL) {
      // There is a BEAM axis.
      findData(BEAM, beamCRVAL, TDOUBLE);
    } else {
      // ms2sdfits output, 0-relative "feed" number.
      findData(BEAM, "MAIN_FEED1", TSHORT);
      cBeam_1rel = 0;
    }
  }

  // IF number, 1-relative by default.
  cIF_1rel = 1;
  if (cALFA && cData[IF].colnum < 0) {
    // ALFA data, 0-relative.
    if (cALFA_CIMA > 1) {
      findData(IF, "IFN", TSHORT);
    } else {
      findData(IF, "IFVAL", TSHORT);
    }
    cIF_1rel = 0;
  }

  if (cData[TIME].colnum < 0) {
    if (timeCRVAL) {
      // There is a TIME axis.
      findData(TIME, timeCRVAL, TDOUBLE);
    }
  }

  // ms2sdfits writes a scalar "TSYS" column that averages the polarizations.
  int colnum;
  findCol("SYSCAL_TSYS", &colnum);
  if (colnum > 0) {
    // This contains the vector Tsys.
    findData(TSYS, "SYSCAL_TSYS", TFLOAT);
  }

  // XPOLDATA?

  if (cData[SCANRATE].colnum < 0) {
    findData(SCANRATE, "FIELD_POINTING_DIR_RATE", TFLOAT);
  }

  if (cData[RESTFRQ].colnum < 0) {
    findData(RESTFRQ, "RESTFREQ", TDOUBLE);
    if (cData[RESTFRQ].colnum < 0) {
      findData(RESTFRQ, "SPECTRAL_WINDOW_REST_FREQUENCY", TDOUBLE);
    }
  }

  if (cData[OBJ_RA].colnum < 0) {
    findData(OBJ_RA, "SOURCE_DIRECTION", TDOUBLE);
  }
  if (cData[OBJ_DEC].colnum < 0) {
    findData(OBJ_DEC, "SOURCE_DIRECTION", TDOUBLE);
  }

  // REFBEAM?

  if (cData[TCAL].colnum < 0) {
    findData(TCAL, "SYSCAL_TCAL", TFLOAT);
  } else if (cALFA_BD) {
    // ALFA BDFITS has a different TCAL with 64 elements - kill it!
    findData(TCAL, "NO NO NO", TFLOAT);
  }

  if (cALFA_BD) {
    // ALFA BDFITS.
    findData(AZIMUTH, "CRVAL2B", TFLOAT);
    findData(ELEVATIO, "CRVAL3B", TFLOAT);
  }

  if (cALFA) {
    // ALFA data.
    findData(PARANGLE, "PARA_ANG", TFLOAT);
  }

  if (cData[TAMBIENT].colnum < 0) {
    findData(TAMBIENT, "WEATHER_TEMPERATURE", TFLOAT);
  }

  if (cData[PRESSURE].colnum < 0) {
    findData(PRESSURE, "WEATHER_PRESSURE", TFLOAT);
  }

  if (cData[HUMIDITY].colnum < 0) {
    findData(HUMIDITY, "WEATHER_REL_HUMIDITY", TFLOAT);
  }

  if (cData[WINDSPEE].colnum < 0) {
    findData(WINDSPEE, "WEATHER_WIND_SPEED", TFLOAT);
  }

  if (cData[WINDDIRE].colnum < 0) {
    findData(WINDDIRE, "WEATHER_WIND_DIRECTION", TFLOAT);
  }


  // Find the number of rows.
  fits_get_num_rows(cSDptr, &cNRow, &cStatus);
  if (!cNRow) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Table contains no entries.");
    close();
    return 1;
  }


  // Determine which beams are present in the data.
  if (cData[BEAM].colnum > 0) {
    short *beamCol = new short[cNRow];
    short beamNul = 1;
    int   anynul;
    if (fits_read_col(cSDptr, TSHORT, cData[BEAM].colnum, 1, 1, cNRow,
                      &beamNul, beamCol, &anynul, &cStatus)) {
      delete [] beamCol;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      close();
      return 1;
    }

    // Find the maximum beam number.
    cNBeam = cBeam_1rel - 1;
    for (int irow = 0; irow < cNRow; irow++) {
      if (beamCol[irow] > cNBeam) {
        cNBeam = beamCol[irow];
      }

      // Check validity.
      if (beamCol[irow] < cBeam_1rel) {
        delete [] beamCol;
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "SDFITS file contains invalid beam number.");
        close();
        return 1;
      }
    }

    if (!cBeam_1rel) cNBeam++;

    // Find all beams present in the data.
    cBeams = new int[cNBeam];
    for (int ibeam = 0; ibeam < cNBeam; ibeam++) {
      cBeams[ibeam] = 0;
    }

    for (int irow = 0; irow < cNRow; irow++) {
      cBeams[beamCol[irow] - cBeam_1rel] = 1;
    }

    delete [] beamCol;

  } else {
    // No BEAM column.
    cNBeam = 1;
    cBeams = new int[1];
    cBeams[0] = 1;
  }

  // Passing back the address of the array allows PKSFITSreader::select() to
  // modify its elements directly.
  nBeam = cNBeam;
  beams = cBeams;


  // Determine which IFs are present in the data.
  if (cData[IF].colnum > 0) {
    short *IFCol = new short[cNRow];
    short IFNul = 1;
    int   anynul;
    if (fits_read_col(cSDptr, TSHORT, cData[IF].colnum, 1, 1, cNRow,
                      &IFNul, IFCol, &anynul, &cStatus)) {
      delete [] IFCol;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      close();
      return 1;
    }

    // Find the maximum IF number.
    cNIF = cIF_1rel - 1;
    for (int irow = 0; irow < cNRow; irow++) {
      if (IFCol[irow] > cNIF) {
        cNIF = IFCol[irow];
      }

      // Check validity.
      if (IFCol[irow] < cIF_1rel) {
        delete [] IFCol;
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "SDFITS file contains invalid IF number.");
        close();
        return 1;
      }
    }

    if (!cIF_1rel) cNIF++;

    // Find all IFs present in the data.
    cIFs      = new int[cNIF];
    cNChan    = new int[cNIF];
    cNPol     = new int[cNIF];
    cHaveXPol = new int[cNIF];
    cGetXPol  = 0;

    for (int iIF = 0; iIF < cNIF; iIF++) {
      cIFs[iIF]   = 0;
      cNChan[iIF] = 0;
      cNPol[iIF]  = 0;
      cHaveXPol[iIF] = 0;
    }

    for (int irow = 0; irow < cNRow; irow++) {
      int iIF = IFCol[irow] - cIF_1rel;
      if (cIFs[iIF] == 0) {
        cIFs[iIF] = 1;

        // Find the axis lengths.
        if (cHaveSpectra) {
          if (cData[DATA].nelem < 0) {
            // Variable dimension array.
            if (readDim(DATA, irow+1, &cNAxis, cNAxes)) {
              log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
              close();
              return 1;
            }
          }

        } else {
          if (cData[DATAXED].colnum > 0) {
            char dataxed[32];
            readParm("DATAXED", TSTRING, dataxed);

            sscanf(dataxed, "(%ld,%ld,%ld,%ld,%ld)", cNAxes, cNAxes+1,
              cNAxes+2, cNAxes+3, cNAxes+4);
          }
        }

        // Number of channels and polarizations.
        cNChan[iIF]    = cNAxes[cReqax[0]];
        cNPol[iIF]     = cNAxes[cReqax[1]];
        cHaveXPol[iIF] = 0;

        // Is cross-polarization data present?
        if (cData[XPOLDATA].colnum > 0) {
          // Check that it conforms.
          int  nAxis;
          long nAxes[2];

          if (readDim(XPOLDATA, irow+1, &nAxis, nAxes)) {
            log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE );
            close();
            return 1;
          }

          // Default is to get it if we have it.
          if (nAxis    == 2 &&
              nAxes[0] == 2 &&
              nAxes[1] == cNChan[iIF]) {
            cGetXPol = cHaveXPol[iIF] = 1;
          }
        }
      }
    }

    delete [] IFCol;

  } else {
    // No IF column.
    cNIF = 1;
    cIFs = new int[1];
    cIFs[0] = 1;

    cNChan    = new int[1];
    cNPol     = new int[1];
    cHaveXPol = new int[1];
    cGetXPol  = 0;

    // Number of channels and polarizations.
    cNChan[0] = cNAxes[cReqax[0]];
    cNPol[0]  = cNAxes[cReqax[1]];
    cHaveXPol[0] = 0;
  }

  if (cALFA && cALFA_CIMA < 2) {
    // Older ALFA data labels each polarization as a separate IF.
    cNPol[0] = cNIF;
    cNIF = 1;
  }

  // For GBT data that stores spectra for each polarization in separate rows
  if ( cData[STOKES].colnum > 0 ) {
    int *stokesCol = new int[cNRow];
    int stokesNul = 1;
    int   anynul;
    if (fits_read_col(cSDptr, TINT, cData[STOKES].colnum, 1, 1, cNRow,
                      &stokesNul, stokesCol, &anynul, &cStatus)) {
      delete [] stokesCol;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      close();
      return 1;
    }

    vector<int> pols ;
    pols.push_back( stokesCol[0] ) ;
    for ( int i = 0 ; i < cNRow ; i++ ) {
      bool pmatch = false ;
      for ( uint j = 0 ; j < pols.size() ; j++ ) {
        if ( stokesCol[i] == pols[j] ) {
          pmatch = true ;
          break ;
        }
      }
      if ( !pmatch ) {
        pols.push_back( stokesCol[i] ) ;
      }
    }

    cPols = new int[pols.size()] ;
    for ( uint i = 0 ; i < pols.size() ; i++ ) {
      cPols[i] = pols[i] ;
    }

    for ( int i = 0 ; i < cNIF ; i++ ) {
      cNPol[i] = pols.size() ;
    }

    delete [] stokesCol ;
  }

  // Passing back the address of the array allows PKSFITSreader::select() to
  // modify its elements directly.
  nIF = cNIF;
  IFs = cIFs;

  nChan    = cNChan;
  nPol     = cNPol;
  haveXPol = cHaveXPol;


  // Default channel range selection.
  cStartChan = new int[cNIF];
  cEndChan   = new int[cNIF];
  cRefChan   = new int[cNIF];

  for (int iIF = 0; iIF < cNIF; iIF++) {
    cStartChan[iIF] = 1;
    cEndChan[iIF] = cNChan[iIF];
    cRefChan[iIF] = cNChan[iIF]/2 + 1;
  }

  // Default is to get it if we have it.
  cGetSpectra = cHaveSpectra;


  // Are baseline parameters present?
  cHaveBase = 0;
  if (cData[BASELIN].colnum) {
    // Check that it conforms.
    int  nAxis, status = 0;
    long nAxes[2];

    if (fits_read_tdim(cSDptr, cData[BASELIN].colnum, 2, &nAxis, nAxes,
                       &status) == 0) {
      cHaveBase = (nAxis == 2);
    }
  }
  haveBase = cHaveBase;


  // Is extra system calibration data available?
  cExtraSysCal = 0;
  for (int iparm = REFBEAM; iparm < NDATA; iparm++) {
    if (cData[iparm].colnum >= 0) {
      cExtraSysCal = 1;
      break;
    }
  }

  extraSysCal = cExtraSysCal;

  return 0;
}

//---------------------------------------------------- SDFITSreader::getHeader

// Get parameters describing the data.

int SDFITSreader::getHeader(
        char   observer[32],
        char   project[32],
        char   telescope[32],
        double antPos[3],
        char   obsMode[32],
        char   bunit[32],
        float  &equinox,
        char   radecsys[32],
        char   dopplerFrame[32],
        char   datobs[32],
        double &utc,
        double &refFreq,
        double &bandwidth)
{
  const string methodName = "getHeader()" ;
  
  // Has the file been opened?
  if (!cSDptr) {
    return 1;
  }

  // Read parameter values.
  readParm("OBSERVER", TSTRING, observer);		// Shared.
  readParm("PROJID",   TSTRING, project);		// Shared.
  readParm("TELESCOP", TSTRING, telescope);		// Core.

  antPos[0] = 0.0;
  antPos[1] = 0.0;
  antPos[2] = 0.0;
  if (readParm("ANTENNA_POSITION", TDOUBLE, antPos)) {
    readParm("OBSGEO-X",  TDOUBLE, antPos);		// Additional.
    readParm("OBSGEO-Y",  TDOUBLE, antPos + 1);		// Additional.
    readParm("OBSGEO-Z",  TDOUBLE, antPos + 2);		// Additional.
  }

  if (antPos[0] == 0.0) {
    if (strncmp(telescope, "ATPKS", 5) == 0) {
      // Parkes coordinates.
      antPos[0] = -4554232.087;
      antPos[1] =  2816759.046;
      antPos[2] = -3454035.950;
    } else if (strncmp(telescope, "ATMOPRA", 7) == 0) {
      // Mopra coordinates.
      antPos[0] = -4682768.630;
      antPos[1] =  2802619.060;
      antPos[2] = -3291759.900;
    } else if (strncmp(telescope, "ARECIBO", 7) == 0) {
      // Arecibo coordinates.
      antPos[0] =  2390486.900;
      antPos[1] = -5564731.440;
      antPos[2] =  1994720.450;
    }
  }

  readData(OBSMODE, 1, obsMode);			// Shared.

  // Brightness unit.
  if (cData[DATAXED].colnum >= 0) {
    strcpy(bunit, "Jy");
  } else {
    strcpy(bunit, cData[DATA].units);
  }

  if (strcmp(bunit, "JY") == 0) {
    bunit[1] = 'y';
  } else if (strcmp(bunit, "JY/BEAM") == 0) {
    strcpy(bunit, "Jy/beam");
  }

  readParm("EQUINOX",  TFLOAT,  &equinox);		// Shared.
  if (cStatus == 405) {
    // EQUINOX was written as string value in early versions.
    cStatus = 0;
    char strtmp[32];
    readParm("EQUINOX", TSTRING, strtmp);
    sscanf(strtmp, "%f", &equinox);
  }

  if (readParm("RADESYS", TSTRING, radecsys)) {		// Additional.
    if (readParm("RADECSYS", TSTRING, radecsys)) {	// Additional.
      strcpy(radecsys, "");
    }
  }

  if (readParm("SPECSYS", TSTRING, dopplerFrame)) {	// Additional.
    // Fallback value.
    strcpy(dopplerFrame, "TOPOCENT");

    // Look for VELFRAME, written by earlier versions of Livedata.
    //
    // Added few more codes currently (as of 2009 Oct) used in the GBT
    // SDFITS (based io_sdfits_define.pro of GBTIDL). - TT
    if (readParm("VELFRAME", TSTRING, dopplerFrame)) {	// Additional.
      // No, try digging it out of the CTYPE card (AIPS convention).
      char keyw[9], ctype[9];
      sprintf(keyw, "CTYPE%ld", cReqax[0]+1);
      readParm(keyw, TSTRING, ctype);

      if (strncmp(ctype, "FREQ-", 5) == 0) {
        strcpy(dopplerFrame, ctype+5);
        if (strcmp(dopplerFrame, "LSR") == 0) {
          // LSR unqualified usually means LSR (kinematic).
          strcpy(dopplerFrame, "LSRK");
        } else if (strcmp(dopplerFrame, "LSD") == 0) {
          // LSR as a dynamical defintion 
          strcpy(dopplerFrame, "LSRD");
        } else if (strcmp(dopplerFrame, "HEL") == 0) {
          // Almost certainly barycentric.
          strcpy(dopplerFrame, "BARYCENT");
        } else if (strcmp(dopplerFrame, "BAR") == 0) {
          // barycentric.
          strcpy(dopplerFrame, "BARYCENT");
        } else if (strcmp(dopplerFrame, "OBS") == 0) {
          // observed or topocentric.
          strcpy(dopplerFrame, "TOPO");
        } else if (strcmp(dopplerFrame, "GEO") == 0) {
          // geocentric 
          strcpy(dopplerFrame, "GEO");
        } else if (strcmp(dopplerFrame, "GAL") == 0) {
          // galactic 
          strcpy(dopplerFrame, "GAL");
        } else if (strcmp(dopplerFrame, "LGR") == 0) {
          // Local group 
          strcpy(dopplerFrame, "LGROUP");
        } else if (strcmp(dopplerFrame, "CMB") == 0) {
          // Cosimic Microwave Backgroup
          strcpy(dopplerFrame, "CMB");
        }
      } else {
        strcpy(dopplerFrame, "");
      }
    }
    // Translate to FITS standard names.
    if (strncmp(dopplerFrame, "TOP", 3) == 0) {
      strcpy(dopplerFrame, "TOPOCENT");
    } else if (strncmp(dopplerFrame, "GEO", 3) == 0) {
      strcpy(dopplerFrame, "GEOCENTR");
    } else if (strncmp(dopplerFrame, "HEL", 3) == 0) {
      strcpy(dopplerFrame, "HELIOCEN");
    } else if (strncmp(dopplerFrame, "BARY", 4) == 0) {
      strcpy(dopplerFrame, "BARYCENT");
    } else if (strncmp(dopplerFrame, "GAL", 3) == 0) {
      strcpy(dopplerFrame, "GALACTOC");
    } else if (strncmp(dopplerFrame, "LGROUP", 6) == 0) {
      strcpy(dopplerFrame, "LOCALGRP");
    } else if (strncmp(dopplerFrame, "CMB", 3) == 0) {
      strcpy(dopplerFrame, "CMBDIPOL");
    }
  }
  
  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  // Get parameters from first row of table.
  readData(DATE_OBS, 1, datobs);
  readData(TIME,     1, &utc);
  readData(FqRefVal, 1, &refFreq);
  readParm("BANDWID", TDOUBLE, &bandwidth);		// Core.

  if (cALFA_BD) utc *= 3600.0;

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  // Check DATE-OBS format.
  if (datobs[2] == '/') {
    // Translate an old-format DATE-OBS.
    datobs[9] = datobs[1];
    datobs[8] = datobs[0];
    datobs[2] = datobs[6];
    datobs[5] = datobs[3];
    datobs[3] = datobs[7];
    datobs[6] = datobs[4];
    datobs[7] = '-';
    datobs[4] = '-';
    datobs[1] = '9';
    datobs[0] = '1';
    datobs[10] = '\0';

  } else if (datobs[10] == 'T' && cData[TIME].colnum < 0) {
    // Dig UTC out of a new-format DATE-OBS.
    int   hh, mm;
    float ss;
    sscanf(datobs+11, "%d:%d:%f", &hh, &mm, &ss);
    utc = (hh*60 + mm)*60 + ss;
    datobs[10] = '\0';
  }

  return 0;
}

//-------------------------------------------------- SDFITSreader::getFreqInfo

// Get frequency parameters for each IF.

int SDFITSreader::getFreqInfo(
        int     &nIF,
        double* &startFreq,
        double* &endFreq)
{
  const string methodName = "getFreqInfo()" ;

  float  fqRefPix;
  double fqDelt, fqRefVal;

  nIF = cNIF;
  startFreq = new double[nIF];
  endFreq   = new double[nIF];

  if (cData[IF].colnum > 0) {
    short *IFCol = new short[cNRow];
    short IFNul = 1;
    int   anynul;
    if (fits_read_col(cSDptr, TSHORT, cData[IF].colnum, 1, 1, cNRow,
                      &IFNul, IFCol, &anynul, &cStatus)) {
      delete [] IFCol;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      close();
      return 1;
    }

    for (int iIF = 0; iIF < nIF; iIF++) {
      if (cIFs[iIF]) {
        // Find the first occurrence of this IF in the table.
        int IFno = iIF + cIF_1rel;
        for (int irow = 0; irow < cNRow;) {
          if (IFCol[irow++] == IFno) {
            readData(FqRefPix, irow, &fqRefPix);
            readData(FqRefVal, irow, &fqRefVal);
            readData(FqDelt,   irow, &fqDelt);

            if (cALFA_BD) {
              unsigned char invert;
              readData("UPPERSB", TBYTE, irow, &invert);

              if (invert) {
                fqDelt = -fqDelt;
              }
            }

            startFreq[iIF] = fqRefVal + (          1 - fqRefPix) * fqDelt;
            endFreq[iIF]   = fqRefVal + (cNChan[iIF] - fqRefPix) * fqDelt;

            break;
          }
        }

      } else {
        startFreq[iIF] = 0.0;
        endFreq[iIF]   = 0.0;
      }
    }

    delete [] IFCol;

  } else {
    // No IF column, read the first table entry.
    readData(FqRefPix, 1, &fqRefPix);
    readData(FqRefVal, 1, &fqRefVal);
    readData(FqDelt,   1, &fqDelt);

    startFreq[0] = fqRefVal + (        1 - fqRefPix) * fqDelt;
    endFreq[0]   = fqRefVal + (cNChan[0] - fqRefPix) * fqDelt;
  }

  return cStatus;
}

//---------------------------------------------------- SDFITSreader::findRange

// Find the range of the data in time and position.

int SDFITSreader::findRange(
        int    &nRow,
        int    &nSel,
        char   dateSpan[2][32],
        double utcSpan[2],
        double* &positions)
{
  const string methodName = "findRange()" ;

  // Has the file been opened?
  if (!cSDptr) {
    return 1;
  }

  nRow = cNRow;

  // Find the number of rows selected.
  short *sel = new short[nRow];
  for (int irow = 0; irow < nRow; irow++) {
    sel[irow] = 1;
  }

  int anynul;
  if (cData[BEAM].colnum > 0) {
    short *beamCol = new short[cNRow];
    short beamNul = 1;
    if (fits_read_col(cSDptr, TSHORT, cData[BEAM].colnum, 1, 1, cNRow,
                      &beamNul, beamCol, &anynul, &cStatus)) {
      delete [] beamCol;
      delete [] sel;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      return 1;
    }

    for (int irow = 0; irow < nRow; irow++) {
      if (!cBeams[beamCol[irow]-cBeam_1rel]) {
        sel[irow] = 0;
      }
    }

    delete [] beamCol;
  }

  if (cData[IF].colnum > 0) {
    short *IFCol = new short[cNRow];
    short IFNul = 1;
    if (fits_read_col(cSDptr, TSHORT, cData[IF].colnum, 1, 1, cNRow,
                      &IFNul, IFCol, &anynul, &cStatus)) {
      delete [] IFCol;
      delete [] sel;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      return 1;
    }

    for (int irow = 0; irow < nRow; irow++) {
      if (!cIFs[IFCol[irow]-cIF_1rel]) {
        sel[irow] = 0;
      }
    }

    delete [] IFCol;
  }

  nSel = 0;
  for (int irow = 0; irow < nRow; irow++) {
    nSel += sel[irow];
  }


  // Find the time range assuming the data is in chronological order.
  readData(DATE_OBS, 1,    dateSpan[0]);
  readData(DATE_OBS, nRow, dateSpan[1]);
  readData(TIME, 1,    utcSpan);
  readData(TIME, nRow, utcSpan+1);

  if (cALFA_BD) {
    utcSpan[0] *= 3600.0;
    utcSpan[1] *= 3600.0;
  }

  // Check DATE-OBS format.
  for (int i = 0; i < 2; i++) {
    if (dateSpan[0][2] == '/') {
      // Translate an old-format DATE-OBS.
      dateSpan[i][9] = dateSpan[i][1];
      dateSpan[i][8] = dateSpan[i][0];
      dateSpan[i][2] = dateSpan[i][6];
      dateSpan[i][5] = dateSpan[i][3];
      dateSpan[i][3] = dateSpan[i][7];
      dateSpan[i][6] = dateSpan[i][4];
      dateSpan[i][7] = '-';
      dateSpan[i][4] = '-';
      dateSpan[i][1] = '9';
      dateSpan[i][0] = '1';
      dateSpan[i][10] = '\0';
    }

    if (dateSpan[i][10] == 'T' && cData[TIME].colnum < 0) {
      // Dig UTC out of a new-format DATE-OBS.
      int   hh, mm;
      float ss;
      sscanf(dateSpan[i]+11, "%d:%d:%f", &hh, &mm, &ss);
      utcSpan[i] = (hh*60 + mm)*60 + ss;
    }
  }


  // Retrieve positions for selected data.
  int isel = 0;
  positions = new double[2*nSel];

  if (cCoordSys == 1) {
    // Vertical (Az,El).
    if (cData[AZIMUTH].colnum  < 1 ||
        cData[ELEVATIO].colnum < 1) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::WARN, "Azimuth/elevation information absent.");
      cStatus = -1;

    } else {
      float *az = new float[cNRow];
      float *el = new float[cNRow];
      fits_read_col(cSDptr, TFLOAT, cData[AZIMUTH].colnum,  1, 1, nRow, 0, az,
                    &anynul, &cStatus);
      fits_read_col(cSDptr, TFLOAT, cData[ELEVATIO].colnum, 1, 1, nRow, 0, el,
                    &anynul, &cStatus);

      if (!cStatus) {
        for (int irow = 0; irow < nRow; irow++) {
          if (sel[irow]) {
            positions[isel++] = az[irow] * D2R;
            positions[isel++] = el[irow] * D2R;
          }
        }
      }

      delete [] az;
      delete [] el;
    }

  } else {
    double *ra  = new double[cNRow];
    double *dec = new double[cNRow];
    fits_read_col(cSDptr, TDOUBLE, cData[RA].colnum,  1, 1, nRow, 0, ra,
                  &anynul, &cStatus);
    fits_read_col(cSDptr, TDOUBLE, cData[DEC].colnum, 1, 1, nRow, 0, dec,
                  &anynul, &cStatus);
    if (cStatus) {
      delete [] ra;
      delete [] dec;
      goto cleanup;
    }

    if (cALFA_BD) {
      for (int irow = 0; irow < nRow; irow++) {
        // Convert hours to degrees.
        ra[irow] *= 15.0;
      }
    }

    if (cCoordSys == 0) {
      // Equatorial (RA,Dec).
      for (int irow = 0; irow < nRow; irow++) {
        if (sel[irow]) {
          positions[isel++] =  ra[irow] * D2R;
          positions[isel++] = dec[irow] * D2R;
        }
      }

    } else if (cCoordSys == 2) {
      // Feed-plane.
      if (cData[OBJ_RA].colnum   < 0 ||
          cData[OBJ_DEC].colnum  < 0 ||
          cData[PARANGLE].colnum < 1 ||
          cData[FOCUSROT].colnum < 1) {
        log( LogOrigin( className, methodName, WHERE ), LogIO::WARN, 
             "Insufficient information to compute feed-plane\n"
             "         coordinates.");
        cStatus = -1;

      } else {
        double *srcRA  = new double[cNRow];
        double *srcDec = new double[cNRow];
        float  *par = new float[cNRow];
        float  *rot = new float[cNRow];

        if (cData[OBJ_RA].colnum == 0) {
          // Header keyword.
          readData(OBJ_RA, 0, srcRA);
          for (int irow = 1; irow < nRow; irow++) {
            srcRA[irow] = *srcRA;
          }
        } else {
          // Table column.
          fits_read_col(cSDptr, TDOUBLE, cData[OBJ_RA].colnum,   1, 1, nRow,
                        0, srcRA,  &anynul, &cStatus);
        }

        if (cData[OBJ_DEC].colnum == 0) {
          // Header keyword.
          readData(OBJ_DEC, 0, srcDec);
          for (int irow = 1; irow < nRow; irow++) {
            srcDec[irow] = *srcDec;
          }
        } else {
          // Table column.
          fits_read_col(cSDptr, TDOUBLE, cData[OBJ_DEC].colnum,  1, 1, nRow,
                        0, srcDec, &anynul, &cStatus);
        }

        fits_read_col(cSDptr, TFLOAT,  cData[PARANGLE].colnum, 1, 1, nRow, 0,
                      par,    &anynul, &cStatus);
        fits_read_col(cSDptr, TFLOAT,  cData[FOCUSROT].colnum, 1, 1, nRow, 0,
                      rot,    &anynul, &cStatus);

        if (!cStatus) {
          for (int irow = 0; irow < nRow; irow++) {
            if (sel[irow]) {
              // Convert to feed-plane coordinates.
              Double dist, pa;
              distPA(ra[irow]*D2R, dec[irow]*D2R, srcRA[irow]*D2R,
                     srcDec[irow]*D2R, dist, pa);

              Double spin = (par[irow] + rot[irow])*D2R - pa + PI;
              if (spin > 2.0*PI) spin -= 2.0*PI;
              Double squint = PI/2.0 - dist;

              positions[isel++] = spin;
              positions[isel++] = squint;
            }
          }
        }

        delete [] srcRA;
        delete [] srcDec;
        delete [] par;
        delete [] rot;
      }
    }

    delete [] ra;
    delete [] dec;
  }

cleanup:
  delete [] sel;

  if (cStatus) {
    nSel = 0;
    delete [] positions;
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    cStatus = 0;
    return 1;
  }

  return 0;
}


//--------------------------------------------------------- SDFITSreader::read

// Read the next data record.

int SDFITSreader::read(
        MBrecord &mbrec)
{
  const string methodName = "read()" ;

  // Has the file been opened?
  if (!cSDptr) {
    return 1;
  }
  // Find the next selected beam and IF.
  short iBeam = 0, iIF = 0;
  int iPol = -1 ;
  while (++cRow <= cNRow) {
    if (cData[BEAM].colnum > 0) {
      readData(BEAM, cRow, &iBeam);

      // Convert to 0-relative.
      if (cBeam_1rel) iBeam--;
    }


    if (cBeams[iBeam]) {
      if (cData[IF].colnum > 0) {
        readData(IF, cRow, &iIF);

        // Convert to 0-relative.
        if (cIF_1rel) iIF--;
      }

      if (cIFs[iIF]) {
        if (cALFA) {
          // ALFA data, check for calibration data.
          char chars[32];
          readData(OBSMODE, cRow, chars);
          if (strcmp(chars, "CAL") == 0) {
            if (cALFA_CIMA > 1) {
              for (short iPol = 0; iPol < cNPol[iIF]; iPol++) {
                alfaCal(iBeam, iIF, iPol);
              }
              continue;
            } else {
              // iIF is really the polarization in older ALFA data.
              alfaCal(iBeam, 0, iIF);
              continue;
            }
          }
        }

        // for GBT SDFITS
        if (cData[STOKES].colnum > 0 ) {
          readData(STOKES, cRow, &iPol ) ;
          for ( int i = 0 ; i < cNPol[iIF] ; i++ ) {
            if ( cPols[i] == iPol ) {
              iPol = i ;
              break ;
            }
          }
        }
        break;
      }
    }
  }

  // EOF?
  if (cRow > cNRow) {
    return -1;
  }


  if (cALFA) {
    int scanNo;
    readData(SCAN, cRow, &scanNo);
    if (scanNo != cALFAscan) {
      cScanNo++;
      cALFAscan = scanNo;
    }
    mbrec.scanNo = cScanNo;

  } else {
    readData(SCAN, cRow, &mbrec.scanNo);

    // Ensure that scan number is 1-relative.
    mbrec.scanNo -= (cFirstScanNo - 1);
  }

  // Times.
  char datobs[32];
  readData(DATE_OBS, cRow,  datobs);
  if ( cData[TIME].colnum > 0 )
    readData(TIME,     cRow, &mbrec.utc);
  else {
    Int yy, mm ;
    Double dd, hour, min, sec ;
    sscanf( datobs, "%d-%d-%lfT%lf:%lf:%lf", &yy, &mm, &dd, &hour, &min, &sec ) ;
    dd = dd + ( hour * 3600.0 + min * 60.0 + sec ) / 86400.0 ;
    MVTime mvt( yy, mm, dd ) ;
    dd = mvt.day() ;
    mbrec.utc = fmod( dd, 1.0 ) * 86400.0 ;
  }
  if (cALFA_BD) mbrec.utc *= 3600.0;

  if (datobs[2] == '/') {
    // Translate an old-format DATE-OBS.
    datobs[9] = datobs[1];
    datobs[8] = datobs[0];
    datobs[2] = datobs[6];
    datobs[5] = datobs[3];
    datobs[3] = datobs[7];
    datobs[6] = datobs[4];
    datobs[7] = '-';
    datobs[4] = '-';
    datobs[1] = '9';
    datobs[0] = '1';

  } else if (datobs[10] == 'T' && cData[TIME].colnum < 0) {
    // Dig UTC out of a new-format DATE-OBS.
    int   hh, mm;
    float ss;
    sscanf(datobs+11, "%d:%d:%f", &hh, &mm, &ss);
    mbrec.utc = (hh*60 + mm)*60 + ss;
  }

  datobs[10] = '\0';
  strcpy(mbrec.datobs, datobs);

  if (cData[CYCLE].colnum > 0) {
    readData(CYCLE, cRow, &mbrec.cycleNo);
    if (cALFA_BD) mbrec.cycleNo++;
  } else {
    // Cycle number not recorded, must do our own bookkeeping.
    if (mbrec.utc != cLastUTC) {
      mbrec.cycleNo = ++cCycleNo;
      cLastUTC = mbrec.utc;
    }
  }

  if ( iPol != -1 ) {
    if ( mbrec.scanNo != cGLastScan[iPol] ) {
      cGLastScan[iPol] = mbrec.scanNo ;
      cGCycleNo[iPol] = 0 ;
      mbrec.cycleNo = ++cGCycleNo[iPol] ;
    }
    else {
      mbrec.cycleNo = ++cGCycleNo[iPol] ;
    }
  }

  readData(EXPOSURE, cRow, &mbrec.exposure);

  // Source identification.
  readData(OBJECT, cRow, mbrec.srcName);

  if ( iPol != -1 ) {
    char obsmode[32] ;
    readData( OBSMODE, cRow, obsmode ) ;
    char sig[1] ;
    char cal[1] ;
    readData( SIG, cRow, sig ) ;
    readData( CAL, cRow, cal ) ;
    if ( strstr( obsmode, "PSWITCH" ) != NULL ) {
      // position switch
      strcat( mbrec.srcName, "_p" ) ;
      if ( strstr( obsmode, "PSWITCHON" ) != NULL ) {
        strcat( mbrec.srcName, "s" ) ;
      }
      else if ( strstr( obsmode, "PSWITCHOFF" ) != NULL ) {
        strcat( mbrec.srcName, "r" ) ;
      }
    }
    else if ( strstr( obsmode, "Nod" ) != NULL ) {
      // nod
      strcat( mbrec.srcName, "_n" ) ;
      if ( sig[0] == 'T' ) {
        strcat( mbrec.srcName, "s" ) ;
      }
      else {
        strcat( mbrec.srcName, "r" ) ;
      }
    }
    else if ( strstr( obsmode, "FSWITCH" ) != NULL ) {
      // frequency switch
      strcat( mbrec.srcName, "_f" ) ;
      if ( sig[0] == 'T' ) {
        strcat( mbrec.srcName, "s" ) ;
      }
      else {
        strcat( mbrec.srcName, "r" ) ;
      }
    }
    if ( cal[0] == 'T' ) {
      strcat( mbrec.srcName, "c" ) ;
    }
    else {
      strcat( mbrec.srcName, "o" ) ;
    }
  }

  readData(OBJ_RA,  cRow, &mbrec.srcRA);
  if (strcmp(cData[OBJ_RA].name, "OBJ-RA") == 0) {
    mbrec.srcRA  *= D2R;
  }

  if (strcmp(cData[OBJ_DEC].name, "OBJ-DEC") == 0) {
    readData(OBJ_DEC, cRow, &mbrec.srcDec);
    mbrec.srcDec *= D2R;
  }

  // Line rest frequency (Hz).
  readData(RESTFRQ, cRow, &mbrec.restFreq);
  if (mbrec.restFreq == 0.0 && cALFA_BD) {
    mbrec.restFreq = 1420.40575e6;
  }

  // Observation mode.
  readData(OBSMODE, cRow, mbrec.obsType);

  // Beam-dependent parameters.
  mbrec.beamNo = iBeam + 1;

  readData(RA,  cRow, &mbrec.ra);
  readData(DEC, cRow, &mbrec.dec);
  mbrec.ra  *= D2R;
  mbrec.dec *= D2R;

  if (cALFA_BD) mbrec.ra *= 15.0;

  float scanrate[2];
  readData(SCANRATE, cRow, &scanrate);
  if (strcmp(cData[SCANRATE].name, "SCANRATE") == 0) {
    mbrec.raRate  = scanrate[0] * D2R;
    mbrec.decRate = scanrate[1] * D2R;
  }
  mbrec.paRate = 0.0f;

  // IF-dependent parameters.
  int startChan = cStartChan[iIF];
  int endChan   = cEndChan[iIF];
  int refChan   = cRefChan[iIF];

  // Allocate data storage.
  int nChan = abs(endChan - startChan) + 1;
  int nPol = cNPol[iIF];

  if ( cData[STOKES].colnum > 0 )
    nPol = 1 ;

  if (cGetSpectra || cGetXPol) {
    int nxpol = cGetXPol ? 2*nChan : 0;
    mbrec.allocate(0, nChan*nPol, nxpol);
  }

  mbrec.nIF = 1;
  mbrec.IFno[0]  = iIF + 1;
  mbrec.nChan[0] = nChan;
  mbrec.nPol[0]  = nPol;
  mbrec.polNo = iPol ;

  readData(FqRefPix, cRow, mbrec.fqRefPix);
  readData(FqRefVal, cRow, mbrec.fqRefVal);
  readData(FqDelt,   cRow, mbrec.fqDelt);

  if (cALFA_BD) {
    unsigned char invert;
    int anynul, colnum;
    findCol("UPPERSB", &colnum);
    fits_read_col(cSDptr, TBYTE, colnum, cRow, 1, 1, 0, &invert, &anynul,
                  &cStatus);

    if (invert) {
      mbrec.fqDelt[0] = -mbrec.fqDelt[0];
    }
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  // Adjust for channel selection.
  if (mbrec.fqRefPix[0] != refChan) {
    mbrec.fqRefVal[0] += (refChan - mbrec.fqRefPix[0]) * mbrec.fqDelt[0];
    mbrec.fqRefPix[0]  =  refChan;
  }

  if (endChan < startChan) {
    mbrec.fqDelt[0] = -mbrec.fqDelt[0];
  }

  // The data may only have a scalar Tsys value.
  mbrec.tsys[0][0] = 0.0f;
  mbrec.tsys[0][1] = 0.0f;
  if (cData[TSYS].nelem >= nPol) {
    readData(TSYS, cRow, mbrec.tsys[0]);
  }

  for (int j = 0; j < 2; j++) {
    mbrec.calfctr[0][j] = 0.0f;
  }
  if (cData[CALFCTR].colnum > 0) {
    readData(CALFCTR, cRow, mbrec.calfctr);
  }

  if (cHaveBase) {
    mbrec.haveBase = 1;
    readData(BASELIN, cRow, mbrec.baseLin);
    readData(BASESUB, cRow, mbrec.baseSub);
  } else {
    mbrec.haveBase = 0;
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  // Read data, sectioning and transposing it in the process.
  long *blc = new long[cNAxis+1];
  long *trc = new long[cNAxis+1];
  long *inc = new long[cNAxis+1];
  for (int iaxis = 0; iaxis <= cNAxis; iaxis++) {
    blc[iaxis] = 1;
    trc[iaxis] = 1;
    inc[iaxis] = 1;
  }

  blc[cReqax[0]] = std::min(startChan, endChan);
  trc[cReqax[0]] = std::max(startChan, endChan);
  blc[cNAxis] = cRow;
  trc[cNAxis] = cRow;

  mbrec.haveSpectra = cGetSpectra;
  if (cGetSpectra) {
    int  anynul;

    for (int ipol = 0; ipol < nPol; ipol++) {
      blc[cReqax[1]] = ipol+1;
      trc[cReqax[1]] = ipol+1;

      if (cALFA && cALFA_CIMA < 2) {
        // ALFA data: polarizations are stored in successive rows.
        blc[cReqax[1]] = 1;
        trc[cReqax[1]] = 1;

        if (ipol) {
          if (++cRow > cNRow) {
            return -1;
          }

          blc[cNAxis] = cRow;
          trc[cNAxis] = cRow;
        }

      } else if (cData[DATA].nelem < 0) {
        // Variable dimension array; get axis lengths.
        int  naxis = 5, status;

        if ((status = readDim(DATA, cRow, &naxis, cNAxes))) {
          log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);

        } else if ((status = (naxis != cNAxis))) {
          log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "DATA array dimensions changed.");
        }

        if (status) {
          delete [] blc;
          delete [] trc;
          delete [] inc;
          return 1;
        }
      }

      if (fits_read_subset_flt(cSDptr, cData[DATA].colnum, cNAxis, cNAxes,
          blc, trc, inc, 0, mbrec.spectra[0] + ipol*nChan, &anynul,
          &cStatus)) {
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
        delete [] blc;
        delete [] trc;
        delete [] inc;
        return 1;
      }

      if (endChan < startChan) {
        // Reverse the spectrum.
        float *iptr = mbrec.spectra[0] + ipol*nChan;
        float *jptr = iptr + nChan - 1;
        float *mid  = iptr + nChan/2;
        while (iptr < mid) {
          float tmp = *iptr;
          *(iptr++) = *jptr;
          *(jptr--) = tmp;
        }
      }

      if (cALFA) {
        // ALFA data, rescale the spectrum.
        float *chan  = mbrec.spectra[0] + ipol*nChan;
        float *chanN = chan + nChan;
        while (chan < chanN) {
          // Approximate conversion to Jy.
          *(chan++) *= cALFAcal[iBeam][iIF];
        }
      }

      if (mbrec.tsys[0][ipol] == 0.0) {
        // Compute Tsys as the average across the spectrum.
        float *chan  = mbrec.spectra[0] + ipol*nChan;
        float *chanN = chan + nChan;
        float *tsys = mbrec.tsys[0] + ipol;
        while (chan < chanN) {
          *tsys += *(chan++);
        }

        *tsys /= nChan;
      }

      // Read data flags.
      if (cData[FLAGGED].colnum > 0) {
        if (fits_read_subset_byt(cSDptr, cData[FLAGGED].colnum, cNAxis,
            cNAxes, blc, trc, inc, 0, mbrec.flagged[0] + ipol*nChan, &anynul,
            &cStatus)) {
          log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
          delete [] blc;
          delete [] trc;
          delete [] inc;
          return 1;
        }

        if (endChan < startChan) {
          // Reverse the flag vector.
          unsigned char *iptr = mbrec.flagged[0] + ipol*nChan;
          unsigned char *jptr = iptr + nChan - 1;
          for (int ichan = 0; ichan < nChan/2; ichan++) {
            unsigned char tmp = *iptr;
            *(iptr++) = *jptr;
            *(jptr--) = tmp;
          }
        }

      } else {
        // All channels are unflagged by default.
        unsigned char *iptr = mbrec.flagged[0] + ipol*nChan;
        for (int ichan = 0; ichan < nChan; ichan++) {
          *(iptr++) = 0;
        }
      }
    }
  }


  // Read cross-polarization data.
  if (cGetXPol) {
    int anynul;
    for (int j = 0; j < 2; j++) {
      mbrec.xcalfctr[0][j] = 0.0f;
    }
    if (cData[XCALFCTR].colnum > 0) {
      readData(XCALFCTR, cRow, mbrec.xcalfctr);
    }

    blc[0] = 1;
    trc[0] = 2;
    blc[1] = std::min(startChan, endChan);
    trc[1] = std::max(startChan, endChan);
    blc[2] = cRow;
    trc[2] = cRow;

    int  nAxis = 2;
    long nAxes[] = {2, nChan};

    if (fits_read_subset_flt(cSDptr, cData[XPOLDATA].colnum, nAxis, nAxes,
        blc, trc, inc, 0, mbrec.xpol[0], &anynul, &cStatus)) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      delete [] blc;
      delete [] trc;
      delete [] inc;
      return 1;
    }

    if (endChan < startChan) {
      // Invert the cross-polarization spectrum.
      float *iptr = mbrec.xpol[0];
      float *jptr = iptr + nChan - 2;
      for (int ichan = 0; ichan < nChan/2; ichan++) {
        float tmp = *iptr;
        *iptr = *jptr;
        *jptr = tmp;

        tmp = *(iptr+1);
        *(iptr+1) = *(jptr+1);
        *(jptr+1) = tmp;

        iptr += 2;
        jptr -= 2;
      }
    }
  }

  delete [] blc;
  delete [] trc;
  delete [] inc;

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  mbrec.extraSysCal = cExtraSysCal;
  readData(REFBEAM,  cRow, &mbrec.refBeam);
  readData(TCAL,     cRow, &mbrec.tcal[0]);
  readData(TCALTIME, cRow,  mbrec.tcalTime);

  readData(AZIMUTH,  cRow, &mbrec.azimuth);
  readData(ELEVATIO, cRow, &mbrec.elevation);
  readData(PARANGLE, cRow, &mbrec.parAngle);

  readData(FOCUSAXI, cRow, &mbrec.focusAxi);
  readData(FOCUSTAN, cRow, &mbrec.focusTan);
  readData(FOCUSROT, cRow, &mbrec.focusRot);

  readData(TAMBIENT, cRow, &mbrec.temp);
  readData(PRESSURE, cRow, &mbrec.pressure);
  readData(HUMIDITY, cRow, &mbrec.humidity);
  readData(WINDSPEE, cRow, &mbrec.windSpeed);
  readData(WINDDIRE, cRow, &mbrec.windAz);

  if (cALFA_BD) {
    // ALFA BDFITS stores zenith angle rather than elevation.
    mbrec.elevation = 90.0 - mbrec.elevation;
  }

  mbrec.azimuth   *= D2R;
  mbrec.elevation *= D2R;
  mbrec.parAngle  *= D2R;
  mbrec.focusRot  *= D2R;
  mbrec.windAz    *= D2R;

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  return 0;
}

//-------------------------------------------------------- SDFITSreader::close

// Close the SDFITS file.

void SDFITSreader::close()
{
  if (cSDptr) {
    int status = 0;
    fits_close_file(cSDptr, &status);
    cSDptr = 0;

    if (cBeams)     delete [] cBeams;
    if (cIFs)       delete [] cIFs;
    if (cStartChan) delete [] cStartChan;
    if (cEndChan)   delete [] cEndChan;
    if (cRefChan)   delete [] cRefChan;
  }
}

//------------------------------------------------------- SDFITSreader::log

// Log a message.  If the current CFITSIO status value is non-zero, also log
// the corresponding error message and the CFITSIO message stack.

void SDFITSreader::log(LogOrigin origin, LogIO::Command cmd, const char *msg)
{
  LogIO os( origin ) ;

  os << msg << endl ;

  if (cStatus > 0) {
    fits_get_errstatus(cStatus, cMsg);
    os << cMsg << endl ;

    while (fits_read_errmsg(cMsg)) {
      os << cMsg << endl ;
    }
  }
  os << LogIO::POST ;
}

//----------------------------------------------------- SDFITSreader::findData

// Locate a data item in the SDFITS file.

void SDFITSreader::findData(
        int  iData,
        char *name,
        int  type)
{
  cData[iData].name = name;
  cData[iData].type = type;

  int colnum;
  findCol(name, &colnum);
  cData[iData].colnum = colnum;

  // Determine the number of data elements.
  if (colnum > 0) {
    int  coltype;
    long nelem, width;
    fits_get_coltype(cSDptr, colnum, &coltype, &nelem, &width, &cStatus);
    fits_get_bcolparms(cSDptr, colnum, 0x0, cData[iData].units, 0x0, 0x0, 0x0,
      0x0, 0x0, 0x0, &cStatus);

    // Look for a TDIMnnn keyword or column.
    char tdim[8];
    sprintf(tdim, "TDIM%d", colnum);
    findCol(tdim, &cData[iData].tdimcol);

    if (coltype < 0) {
      // CFITSIO returns coltype < 0 for variable length arrays.
      cData[iData].coltype = -coltype;
      cData[iData].nelem   = -nelem;

    } else {
      cData[iData].coltype = coltype;

      // Is there a TDIMnnn column?
      if (cData[iData].tdimcol > 0) {
        // Yes, dimensions of the fixed-length array could still vary.
        cData[iData].nelem = -nelem;
      } else {
        cData[iData].nelem =  nelem;
      }
    }

  } else if (colnum == 0) {
    // Keyword.
    cData[iData].coltype =  0;
    cData[iData].nelem   =  1;
    cData[iData].tdimcol = -1;
  }
}

//------------------------------------------------------ SDFITSreader::readDim

// Determine the dimensions of an array in the SDFITS file.

int SDFITSreader::readDim(
        int  iData,
        long iRow,
        int *naxis,
        long naxes[])
{
  int colnum = cData[iData].colnum;
  if (colnum <= 0) {
    return 1;
  }

  int maxdim = *naxis;
  if (cData[iData].tdimcol < 0) {
    // No TDIMnnn column for this array.
    if (cData[iData].nelem < 0) {
      // Variable length array; read the array descriptor.
      *naxis = 1;
      long dummy;
      if (fits_read_descript(cSDptr, colnum, iRow, naxes, &dummy, &cStatus)) {
        return 1;
      }

    } else {
      // Read the repeat count from TFORMnnn.
      if (fits_read_tdim(cSDptr, colnum, maxdim, naxis, naxes, &cStatus)) {
        return 1;
      }
    }

  } else {
    // Read the TDIMnnn value from the header or table.
    char tdim[8], tdimval[64];
    sprintf(tdim, "TDIM%d", colnum);
    readData(tdim, TSTRING, iRow, tdimval);

    // fits_decode_tdim() checks that the TDIMnnn value is within the length
    // of the array in the specified column number but unfortunately doesn't
    // recognize variable-length arrays.  Hence we must decode it here.
    char *tp = tdimval;
    if (*tp != '(') return 1;

    tp++;
    *naxis = 0;
    for (size_t j = 1; j < strlen(tdimval); j++) {
      if (tdimval[j] == ',' || tdimval[j] == ')') {
        sscanf(tp, "%ld", naxes + (*naxis)++);
        if (tdimval[j] == ')') break;
        tp = tdimval + j + 1;
      }
    }
  }

  return 0;
}

//----------------------------------------------------- SDFITSreader::readParm

// Read a parameter value from the SDFITS file.

int SDFITSreader::readParm(
        char *name,
        int  type,
        void *value)
{
  return readData(name, type, 1, value);
}

//----------------------------------------------------- SDFITSreader::readData

// Read a data value from the SDFITS file.

int SDFITSreader::readData(
        char *name,
        int  type,
        long iRow,
        void *value)
{
  int colnum;
  findCol(name, &colnum);

  if (colnum > 0) {
    // Read the first value from the specified row of the table.
    int  coltype;
    long nelem, width;
    fits_get_coltype(cSDptr, colnum, &coltype, &nelem, &width, &cStatus);

    int anynul;
    if (type == TSTRING) {
      if (nelem) {
        fits_read_col(cSDptr, type, colnum, iRow, 1, 1, 0, &value, &anynul,
                      &cStatus);
      } else {
        strcpy((char *)value, "");
      }

    } else {
      if (nelem) {
        fits_read_col(cSDptr, type, colnum, iRow, 1, 1, 0, value, &anynul,
                      &cStatus);
      } else {
        if (type == TSHORT) {
          *((short *)value) = 0;
        } else if (type == TINT) {
          *((int *)value) = 0;
        } else if (type == TFLOAT) {
          *((float *)value) = 0.0f;
        } else if (type == TDOUBLE) {
          *((double *)value) = 0.0;
        }
      }
    }

  } else if (colnum == 0) {
    // Read keyword value.
    fits_read_key(cSDptr, type, name, value, 0, &cStatus);

  } else {
    // Not present.
    if (type == TSTRING) {
      strcpy((char *)value, "");
    } else if (type == TSHORT) {
      *((short *)value) = 0;
    } else if (type == TINT) {
      *((int *)value) = 0;
    } else if (type == TFLOAT) {
      *((float *)value) = 0.0f;
    } else if (type == TDOUBLE) {
      *((double *)value) = 0.0;
    }
  }

  return colnum < 0;
}

//----------------------------------------------------- SDFITSreader::readData

// Read data from the SDFITS file.

int SDFITSreader::readData(
        int  iData,
        long iRow,
        void *value)
{
  char *name  = cData[iData].name;
  int  type   = cData[iData].type;
  int  colnum = cData[iData].colnum;
  long nelem  = cData[iData].nelem;

  if (colnum > 0) {
    // Read the required number of values from the specified row of the table.
    int anynul;
    if (type == TSTRING) {
      if (nelem) {
        fits_read_col(cSDptr, type, colnum, iRow, 1, 1, 0, &value, &anynul,
                      &cStatus);
      } else {
        strcpy((char *)value, "");
      }

    } else {
      if (nelem) {
        fits_read_col(cSDptr, type, colnum, iRow, 1, abs(nelem), 0, value,
                      &anynul, &cStatus);
      } else {
        if (type == TSHORT) {
          *((short *)value) = 0;
        } else if (type == TINT) {
          *((int *)value) = 0;
        } else if (type == TFLOAT) {
          *((float *)value) = 0.0f;
        } else if (type == TDOUBLE) {
          *((double *)value) = 0.0;
        }
      }
    }

  } else if (colnum == 0) {
    // Read keyword value.
    fits_read_key(cSDptr, type, name, value, 0, &cStatus);

  } else {
    // Not present.
    if (type == TSTRING) {
      strcpy((char *)value, "");
    } else if (type == TSHORT) {
      *((short *)value) = 0;
    } else if (type == TINT) {
      *((int *)value) = 0;
    } else if (type == TFLOAT) {
      *((float *)value) = 0.0f;
    } else if (type == TDOUBLE) {
      *((double *)value) = 0.0;
    }
  }

  return colnum < 0;
}

//------------------------------------------------------ SDFITSreader::findCol

// Locate a parameter in the SDFITS file.

void SDFITSreader::findCol(
        char *name,
        int *colnum)
{
  *colnum = 0;
  int status = 0;
  fits_get_colnum(cSDptr, CASESEN, name, colnum, &status);

  if (status) {
    // Not a real column - maybe it's virtual.
    char card[81];

    status = 0;
    fits_read_card(cSDptr, name, card, &status);
    if (status) {
      // Not virtual either.
      *colnum = -1;
    }

    // Clear error messages.
    fits_clear_errmsg();
  }
}

//------------------------------------------------------ SDFITSreader::alfaCal

// Process ALFA calibration data.

int SDFITSreader::alfaCal(
        short iBeam,
        short iIF,
        short iPol)
{
  const string methodName = "alfaCal()" ;

  int  calOn;
  char chars[32];
  if (cALFA_BD) {
    readData("OBS_NAME", TSTRING, cRow, chars);
  } else {
    readData("SCANTYPE", TSTRING, cRow, chars);
  }

  if (strcmp(chars, "ON") == 0) {
    calOn = 1;
  } else if (strcmp(chars, "OFF") == 0) {
    calOn = 0;
  } else {
    return 1;
  }

  // Read cal data.
  long *blc = new long[cNAxis+1];
  long *trc = new long[cNAxis+1];
  long *inc = new long[cNAxis+1];
  for (int iaxis = 0; iaxis <= cNAxis; iaxis++) {
    blc[iaxis] = 1;
    trc[iaxis] = 1;
    inc[iaxis] = 1;
  }

  // User channel selection.
  int startChan = cStartChan[iIF];
  int endChan   = cEndChan[iIF];

  blc[cNAxis] = cRow;
  trc[cNAxis] = cRow;
  blc[cReqax[0]] = std::min(startChan, endChan);
  trc[cReqax[0]] = std::max(startChan, endChan);
  if (cALFA_CIMA > 1) {
    // CIMAFITS 2.x has a legitimate STOKES axis...
    blc[cReqax[1]] = iPol+1;
    trc[cReqax[1]] = iPol+1;
  } else {
    // ...older ALFA data does not.
    blc[cReqax[1]] = 1;
    trc[cReqax[1]] = 1;
  }

  float spectrum[endChan];
  int anynul;
  if (fits_read_subset_flt(cSDptr, cData[DATA].colnum, cNAxis, cNAxes,
      blc, trc, inc, 0, spectrum, &anynul, &cStatus)) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    delete [] blc;
    delete [] trc;
    delete [] inc;
    return 1;
  }

  // Average the spectrum.
  float mean = 1e9f;
  for (int k = 0; k < 2; k++) {
    float discrim = 2.0f * mean;

    int nChan = 0;
    float sum = 0.0f;

    float *chanN = spectrum + abs(endChan - startChan) + 1;
    for (float *chan = spectrum; chan < chanN; chan++) {
      // Simple discriminant that eliminates strong radar interference.
      if (*chan < discrim) {
        nChan++;
        sum += *chan;
      }
    }

    mean = sum / nChan;
  }

  if (calOn) {
    cALFAcalOn[iBeam][iPol]  += mean;
  } else {
    cALFAcalOff[iBeam][iPol] += mean;
  }

  if (cALFAcalOn[iBeam][iPol] != 0.0f &&
      cALFAcalOff[iBeam][iPol] != 0.0f) {
    // Tcal should come from the TCAL table, it varies weakly with beam,
    // polarization, and frequency.  However, TCAL is not written properly.
    float Tcal = 12.0f;
    cALFAcal[iBeam][iPol] = Tcal / (cALFAcalOn[iBeam][iPol] -
                                    cALFAcalOff[iBeam][iPol]);

    // Scale from K to Jy; the gain also varies weakly with beam,
    // polarization, frequency, and zenith angle.
    float fluxCal = 10.0f;
    cALFAcal[iBeam][iPol] /= fluxCal;

    cALFAcalOn[iBeam][iPol]  = 0.0f;
    cALFAcalOff[iBeam][iPol] = 0.0f;
  }

  return 0;
}

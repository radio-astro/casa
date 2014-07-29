//#---------------------------------------------------------------------------
//# GBTFITSreader.cc: interface class for GBT SDFITS input using CFITSIO.
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
//# $Id: SDFITSreader.cc,v 19.45 2009-09-30 07:23:48 cal103 Exp $
//#---------------------------------------------------------------------------
//# The GBTFITSreader class reads single dish FITS files from Green Bank 
//# telescopes. This class is actually a specific version of SDFITSreader.
//#
//# Original: 2000/08/09, Mark Calabretta, ATNF
//#   Update: 2010/07/29, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#include <atnf/pks/pks_maths.h>
#include <atnf/PKSIO/MBrecord.h>
#include <atnf/PKSIO/GBTFITSreader.h>
#include <atnf/PKSIO/SrcType.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCFrequency.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MDirection.h>

#include <casa/Logging/LogIO.h>
#include <casa/Quanta/MVTime.h>
#include <casa/math.h>
#include <casa/stdio.h>

#include <algorithm>
#include <strings.h>
#include <cstring>

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
const string className = "GBTFITSreader" ;

//---------------------------------------------------- GBTFITSreader::(statics)

int GBTFITSreader::sInit  = 1;
int GBTFITSreader::sReset = 0;
int (*GBTFITSreader::sALFAcalNon)[2]   = (int (*)[2])(new float[16]);
int (*GBTFITSreader::sALFAcalNoff)[2]  = (int (*)[2])(new float[16]);
float (*GBTFITSreader::sALFAcalOn)[2]  = (float (*)[2])(new float[16]);
float (*GBTFITSreader::sALFAcalOff)[2] = (float (*)[2])(new float[16]);
float (*GBTFITSreader::sALFAcal)[2]    = (float (*)[2])(new float[16]);

//------------------------------------------------- GBTFITSreader::GBTFITSreader

GBTFITSreader::GBTFITSreader()
{
  // Default constructor.
  cSDptr = 0x0;

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

  // other initialization
  numHDU = 0 ;

}

//------------------------------------------------ GBTFITSreader::~GBTFITSreader

GBTFITSreader::~GBTFITSreader()
{
  close();

  delete [] cData;
}

//--------------------------------------------------------- GBTFITSreader::open

// Open an SDFITS file for reading.

int GBTFITSreader::open(
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

  // Get number of HDUs in the file
  if ( fits_get_num_hdus( cSDptr, &numHDU, &cStatus ) ) {
    sprintf(cMsg, "ERROR: Failed to get number of HDUs in the file\n       %s", sdName );
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, cMsg) ;
    return 1 ;
  }
  // ignore Primary HDU
  numHDU = numHDU - 1 ; 
  sprintf(cMsg, "number of HDUs = %d", numHDU) ;
  log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;

  // Move to the SDFITS extension.
  cALFA = cALFA_BD = cALFA_CIMA = 0;
  char *extName = "SINGLE DISH" ;
  if (fits_movnam_hdu(cSDptr, BINARY_TBL, "SINGLE DISH", 0, &cStatus)) {
    // No SDFITS table, look for BDFITS or CIMAFITS.
    cStatus = 0;
    if (fits_movnam_hdu(cSDptr, BINARY_TBL, "BDFITS", 0, &cStatus) == 0) {
      cALFA_BD = 1;
      extName = "BDFITS" ;

    } else {
      cStatus = 0;
      if (fits_movnam_hdu(cSDptr, BINARY_TBL, "CIMAFITS", 0, &cStatus) == 0) {
        cALFA_CIMA = 1;
        extName = "CIMAFITS" ;

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
    if (sInit) {
      for (int iBeam = 0; iBeam < 8; iBeam++) {
        for (int iPol = 0; iPol < 2; iPol++) {
          sALFAcalOn[iBeam][iPol]  = 0.0f;
          sALFAcalOff[iBeam][iPol] = 0.0f;

          // Nominal factor to calibrate spectra in Jy.
          sALFAcal[iBeam][iPol] = 3.0f;
        }
      }

      sInit = 0;
    }
  }

  // Find the number of rows.
  cNRowT = new long[numHDU] ;
  cNRowCum = new long[numHDU] ;
  hduId = new int[numHDU] ;
  cNRow = 0 ;
  fits_movabs_hdu( cSDptr, 1, NULL, &cStatus ) ;
  for ( int i = 0 ; i < numHDU ; i++ ) {
    if ( i == 0 ) {
      fits_movnam_hdu( cSDptr, BINARY_TBL, extName, 0, &cStatus ) ;
      fits_get_hdu_num( cSDptr, &(hduId[i]) ) ;
    }
    else {
      if ( fits_movrel_hdu( cSDptr, 1, NULL, &cStatus ) ) {
        log( LogOrigin( className, methodName, WHERE), LogIO::SEVERE, "Failed to move" ) ;
      }
      fits_get_hdu_num( cSDptr, &(hduId[i]) ) ;
    }
    sprintf(cMsg, "HDU ID for %d: %d", i, hduId[i]) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
    fits_get_num_rows( cSDptr, &cNRowT[i], &cStatus ) ;
    if (!cNRowT[i]) {
      ostringstream oss ;
      sprintf(cMsg, "Table %d contains no entries.", i ) ;
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, cMsg);
      close();
      return 1;
    }
    cNRow += cNRowT[i] ;
    cNRowCum[i] = cNRow ;
    sprintf(cMsg, "number of rows for HDU %d = %ld", i, cNRowT[i]) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
    sprintf(cMsg, "cumulative number of rows for HDU %d = %ld", i, cNRowCum[i]) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
  }
  sprintf(cMsg, "number of rows = %ld", cNRow) ;
  log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;

  // Move back to first binary table
  fits_movabs_hdu( cSDptr, hduId[0], NULL, &cStatus ) ;

  //////
  // For devlopment
//   cNRow = 0 ;
//   for ( int ihdu = 0 ; ihdu < numHDU ; ihdu++ ) {
//     //cNRowT[ihdu] = 10 ;
//     //cNRowT[ihdu] = 100 ;
//     cNRowT[ihdu] = 500 ;
//     cNRow += cNRowT[ihdu] ;
//     cNRowCum[ihdu] = cNRow ;
//     sprintf(cMsg, "number of rows for HDU %d = %ld", ihdu, cNRowT[ihdu]) ;
//     log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
//     sprintf(cMsg, "cumulative number of rows for HDU %d = %ld", ihdu, cNRowCum[ihdu]) ;
//     log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
//   }
//   sprintf(cMsg, "number of rows = %ld", cNRow) ;
//   log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
  // Original cNRow is cNRowT[0]
  // cNRow = cNRowT[0] ;
  // fits_get_num_rows(cSDptr, &cNRow, &cStatus);
  /////

  if (!cNRow) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Table contains no entries.");
    close();
    return 1;
  }
  
  // GBT data.
  char telescope[32];
  readParm("TELESCOP", TSTRING, telescope);      // Core.
  cGBT = strncmp(telescope, "GBT", 3) == 0 ||
         strncmp(telescope, "NRAO_GBT", 8) == 0;


  // Check that the DATA array column is present.
  findData(DATA, "DATA", TFLOAT);
  haveSpectra = cHaveSpectra = cData[DATA].colnum > 0;

  cNAxisTime = 0;
  if (cHaveSpectra) {
    // Find the number of data axes (must be the same for each IF).
    cNAxes = 5;
    if (readDim(DATA, 1, &cNAxes, cNAxis)) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
      close();
      return 1;
    }

    if (cALFA_BD) {
      // ALFA BDFITS: variable length arrays don't actually vary and there is
      // no TDIM (or MAXISn) card; use the LAGS_IN value.
      cNAxes = 5;
      readParm("LAGS_IN", TLONG, cNAxis);
      cNAxis[1] = 1;
      cNAxis[2] = 1;
      cNAxis[3] = 1;
      cNAxis[4] = 1;
      cData[DATA].nelem = cNAxis[0];
    }

    if (cNAxes < 4) {
      // Need at least four axes (for now).
      log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "DATA array contains fewer than four axes.");
      close();
      return 1;
    } else if (cNAxes > 5) {
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

    for (int iaxis = 0; iaxis < 5; iaxis++) cNAxis[iaxis] = 0;
    sscanf(dataxed, "(%ld,%ld,%ld,%ld,%ld)", cNAxis, cNAxis+1, cNAxis+2,
      cNAxis+3, cNAxis+4);
    for (int iaxis = 4; iaxis > -1; iaxis--) {
      if (cNAxis[iaxis] == 0) cNAxes = iaxis;
    }
  }

  char  *CTYPE[5] = {"CTYPE1", "CTYPE2", "CTYPE3", "CTYPE4", "CTYPE5"};
  char  *CRPIX[5] = {"CRPIX1", "CRPIX2", "CRPIX3", "CRPIX4", "CRPIX5"};
  char  *CRVAL[5] = {"CRVAL1", "CRVAL2", "CRVAL3", "CRVAL4", "CRVAL5"};
  char  *CDELT[5] = {"CDELT1", "CDELT2", "CDELT3", "CDELT4", "CDELT5"};

  // Find required DATA array axes.
  char ctype[5][72];
  for (int iaxis = 0; iaxis < cNAxes; iaxis++) {
    strcpy(ctype[iaxis], "");
    readParm(CTYPE[iaxis], TSTRING, ctype[iaxis]);      // Core.
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    close();
    return 1;
  }

  char *fqCRVAL  = 0;
  char *fqCDELT  = 0;
  char *fqCRPIX  = 0;
  char *raCRVAL  = 0;
  char *decCRVAL = 0;
  char *timeCRVAL = 0;
  char *timeCDELT = 0;
  char *timeCRPIX = 0;
  char *beamCRVAL = 0;
  char *polCRVAL = 0;

  cFreqAxis   = -1;
  cStokesAxis = -1;
  cRaAxis     = -1;
  cDecAxis    = -1;
  cTimeAxis   = -1;
  cBeamAxis   = -1;

  for (int iaxis = 0; iaxis < cNAxes; iaxis++) {
    if (strncmp(ctype[iaxis], "FREQ", 4) == 0) {
      cFreqAxis = iaxis;
      fqCRVAL   = CRVAL[iaxis];
      fqCDELT   = CDELT[iaxis];
      fqCRPIX   = CRPIX[iaxis];

    } else if (strncmp(ctype[iaxis], "STOKES", 6) == 0) {
      cStokesAxis = iaxis;
      polCRVAL = CRVAL[iaxis];

    } else if (strncmp(ctype[iaxis], "RA", 2) == 0) {
      cRaAxis   = iaxis;
      raCRVAL   = CRVAL[iaxis];

    } else if (strncmp(ctype[iaxis], "DEC", 3) == 0) {
      cDecAxis  = iaxis;
      decCRVAL  = CRVAL[iaxis];

    } else if (strcmp(ctype[iaxis], "TIME") == 0) {
      // TIME (UTC seconds since midnight); axis type, if present, takes
      // precedence over keyword.
      cTimeAxis = iaxis;
      timeCRVAL = CRVAL[iaxis];

      // Check for non-degeneracy.
      if ((cNAxisTime = cNAxis[iaxis]) > 1) {
        timeCDELT = CDELT[iaxis];
        timeCRPIX = CRPIX[iaxis];
        sprintf(cMsg, "DATA array contains a TIME axis of length %ld.",
          cNAxisTime);
        //logMsg(cMsg);
        log(LogOrigin( className, methodName, WHERE ), LogIO::NORMAL, cMsg);
      }

    } else if (strcmp(ctype[iaxis], "BEAM") == 0) {
      // BEAM can be a keyword or axis type.
      cBeamAxis = iaxis;
      beamCRVAL = CRVAL[iaxis];
    }
  }

  if (cALFA_BD) {
    // Fixed in ALFA CIMAFITS.
    cRaAxis = 2;
    raCRVAL = "CRVAL2A";

    cDecAxis = 3;
    decCRVAL = "CRVAL3A";
  }


  // Check that required axes are present.
  if (cFreqAxis < 0 || cStokesAxis < 0 || cRaAxis < 0 || cDecAxis < 0) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Could not find required DATA array axes.");
    close();
    return 1;
  }

  // Set up machinery for data retrieval.
  findData(SCAN,     "SCAN",     TINT);         // Shared.
  findData(CYCLE,    "CYCLE",    TINT);         // Additional.
  findData(DATE_OBS, "DATE-OBS", TSTRING);      // Core.

  if (cTimeAxis >= 0) {
    // The DATA array has a TIME axis.
    if (cNAxisTime > 1) {
      // Non-degenerate.
      findData(TimeRefVal, timeCRVAL, TDOUBLE); // Time reference value.
      findData(TimeDelt,   timeCDELT, TDOUBLE); // Time increment.
      findData(TimeRefPix, timeCRPIX, TFLOAT);  // Time reference pixel.
    } else {
      // Degenerate, treat its like a simple TIME keyword.
      findData(TIME, timeCRVAL,  TDOUBLE);
    }

  } else {
    findData(TIME,   "TIME",     TDOUBLE);      // Core.
  }

  findData(EXPOSURE, "EXPOSURE", TFLOAT);       // Core.
  findData(OBJECT,   "OBJECT",   TSTRING);      // Core.
  findData(OBJ_RA,   "OBJ-RA",   TDOUBLE);      // Additional.
  findData(OBJ_DEC,  "OBJ-DEC",  TDOUBLE);      // Additional.
  findData(RESTFRQ,  "RESTFRQ",  TDOUBLE);      // Additional.
  findData(OBSMODE,  "OBSMODE",  TSTRING);      // Shared.

  findData(BEAM,     "BEAM",     TSHORT);       // Additional.
  findData(IF,       "IF",       TSHORT);       // Additional.
  findData(FqRefVal,  fqCRVAL,   TDOUBLE);      // Frequency reference value.
  findData(FqDelt,    fqCDELT,   TDOUBLE);      // Frequency increment.
  findData(FqRefPix,  fqCRPIX,   TFLOAT);       // Frequency reference pixel.
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

  findData(RVSYS,     "RVSYS",   TDOUBLE);
  findData(VFRAME,    "VFRAME",  TDOUBLE);
  findData(VELDEF,    "VELDEF",  TSTRING);

  findData(TIMESTAMP, "TIMESTAMP", TSTRING);
  findData(DURATION,  "DURATION", TDOUBLE);
  findData(SAMPLER,   "SAMPLER", TSTRING);

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
        // Note that RECNUM increases by cNAxisTime per row.
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
            if (readDim(DATA, irow+1, &cNAxes, cNAxis)) {
              log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
              close();
              return 1;
            }
          }

        } else {
          if (cData[DATAXED].colnum > 0) {
            char dataxed[32];
            readParm("DATAXED", TSTRING, dataxed);

            sscanf(dataxed, "(%ld,%ld,%ld,%ld,%ld)", cNAxis, cNAxis+1,
              cNAxis+2, cNAxis+3, cNAxis+4);
          }
        }

        // Number of channels and polarizations.
        cNChan[iIF]    = cNAxis[cFreqAxis];
        cNPol[iIF]     = cNAxis[cStokesAxis];
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

  } else if ( numHDU > 1 ) {
    Vector<Int> nIFs = nIFPerHDU() ;
    cNIF = sum( nIFs ) ;
    sprintf( cMsg, "cNIF = %d", cNIF ) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::NORMAL, cMsg ) ;
    cIFs = new int[cNIF] ;
    cNChan    = new int[cNIF];
    cNPol     = new int[cNIF];
    cHaveXPol = new int[cNIF];
    cGetXPol  = 0;
    for ( int iif = 0 ; iif < numHDU ; iif++ ) {
      cIFs[iif] = 1 ;
      cNChan[iif] = 0 ;
      cNPol[iif] = 1 ;
      cHaveXPol[iif] = 0 ;
    }
    Int cidx = 0 ;
    for ( int ihdu = 0 ; ihdu < numHDU ; ihdu++ ) {
      // Move 
      fits_movabs_hdu( cSDptr, hduId[ihdu], NULL, &cStatus ) ;

      if (readDim(DATA, 1, &cNAxes, cNAxis)) {
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
        close();
        return 1;
      }

      // Number of channels and polarizations.
      for ( int irow = 0 ; irow < nIFs[ihdu] ; irow++ ) {
        cNChan[cidx+irow]    = cNAxis[cFreqAxis];
        cNPol[cidx+irow]     = cNAxis[cStokesAxis];
        cHaveXPol[cidx+irow] = 0;

        // Is cross-polarization data present?
        if (cData[XPOLDATA].colnum > 0) {
          // Check that it conforms.
          int  nAxis;
          long nAxes[2];
          
          if (readDim(XPOLDATA, 1, &nAxis, nAxes)) {
            log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE );
            close();
            return 1;
          }
          
          // Default is to get it if we have it.
          for ( int irow = 0 ; irow < nIFs[ihdu] ; irow++ ) {
            if (nAxis    == 2 &&
                nAxes[0] == 2 &&
                nAxes[1] == cNChan[cidx+irow]) {
              cGetXPol = cHaveXPol[cidx+irow] = 1;
            }
          }
        }
      }
      cidx += nIFs[ihdu] ;
    }
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
    cNChan[0] = cNAxis[cFreqAxis];
    cNPol[0]  = cNAxis[cStokesAxis];
    cHaveXPol[0] = 0;
  }

  if (cALFA && cALFA_CIMA < 2) {
    // Older ALFA data labels each polarization as a separate IF.
    cNPol[0] = cNIF;
    cNIF = 1;
  }

  // For GBT data that stores spectra for each polarization in separate rows
  if ( cData[STOKES].colnum > 0 ) {
//     int *stokesCol = new int[cNRow];
    int stokesNul = 1;
    int   anynul;
//     if (fits_read_col(cSDptr, TINT, cData[STOKES].colnum, 1, 1, cNRow,
//                       &stokesNul, stokesCol, &anynul, &cStatus)) {
//       delete [] stokesCol;
//       log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
//       close();
//       return 1;
//     }
    vector<int *> stokesCols( numHDU ) ;
    for ( int ihdu = 0 ; ihdu < numHDU ; ihdu++ ) {
      fits_movabs_hdu( cSDptr, hduId[ihdu], NULL, &cStatus ) ;
      stokesCols[ihdu] = new int[cNRowT[ihdu]] ;
      fits_movabs_hdu( cSDptr, hduId[ihdu], NULL, &cStatus ) ;
      if (fits_read_col(cSDptr, TINT, cData[STOKES].colnum, 1, 1, cNRowT[ihdu],
                        &stokesNul, stokesCols[ihdu] , &anynul, &cStatus)) {
        for ( int jhdu = 0 ; jhdu <= ihdu ; jhdu++ ) {
          delete [] stokesCols[jhdu] ;
        }
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
        close();
        return 1;
      }
    }
    fits_movabs_hdu( cSDptr, hduId[0], NULL, &cStatus ) ;

    vector<int> pols ;
//     pols.push_back( stokesCol[0] ) ;
//     for ( int i = 0 ; i < cNRow ; i++ ) {
//       bool pmatch = false ;
//       for ( uint j = 0 ; j < pols.size() ; j++ ) {
//         if ( stokesCol[i] == pols[j] ) {
//           pmatch = true ;
//           break ;
//         }
//       }
//       if ( !pmatch ) {
//         pols.push_back( stokesCol[i] ) ;
//       }
//     }
    pols.push_back( stokesCols[0][0] ) ;
    for ( int k = 0 ; k < numHDU ; k++ ) {
      for ( int i = 0 ; i < cNRowT[k] ; i++ ) {
        bool pmatch = false ;
        for ( uint j = 0 ; j < pols.size() ; j++ ) {
          if ( stokesCols[k][i] == pols[j] ) {
            pmatch = true ;
            break ;
          }
        }
        if ( !pmatch ) {
          pols.push_back( stokesCols[k][i] ) ;
        }
      }
    }

    cPols = new int[pols.size()] ;
    for ( uint i = 0 ; i < pols.size() ; i++ ) {
      cPols[i] = pols[i] ;
    }

    for ( int i = 0 ; i < cNIF ; i++ ) {
      cNPol[i] = pols.size() ;
    }

//     delete [] stokesCol ;
    for ( int k = 0 ; k < numHDU ; k++ ) {
      delete [] stokesCols[k] ;
    }
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


  // Extras for ALFA data.
  cALFAacc = 0.0f;
  if (cALFA_CIMA > 1) {
    // FFTs per second when the Mock correlator operates in RFI blanking mode.
    readData("PHFFTACC", TFLOAT, 0, &cALFAacc);
  }


  cRow = 0;
  cTimeIdx = cNAxisTime;

  return 0;
}

//---------------------------------------------------- GBTFITSreader::getHeader

// Get parameters describing the data.

int GBTFITSreader::getHeader(
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
    if (cGBT) {
      readParm( "SITELONG", TDOUBLE, antPos ) ;
      readParm( "SITELAT", TDOUBLE, antPos+1 ) ;
      readParm( "SITEELEV", TDOUBLE, antPos+2 ) ;
      Vector<Double> pos( 2 ) ;
      pos[0] = Double( antPos[0] ) ;
      pos[1] = Double( antPos[1] ) ;
      MPosition mp( MVPosition( Quantity( Double(antPos[2]), "m" ), 
                                Quantum< Vector<Double> >( pos, "deg" ) ), 
                    MPosition::WGS84 ) ;
      MeasFrame mFrame( mp ) ;
      MVPosition mvp ;
      mFrame.getITRF( mvp ) ;
      Vector<Double> antPosVec = mvp.getValue() ;
      antPos[0] = (double)antPosVec[0] ;
      antPos[1] = (double)antPosVec[1] ;
      antPos[2] = (double)antPosVec[2] ;
    }
    else if (strncmp(telescope, "ATPKS", 5) == 0) {
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
      sprintf(keyw, "CTYPE%ld", cFreqAxis+1);
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

    // always LSRK if GBT data
    if (cGBT) {
      strcpy(dopplerFrame, "LSRK");
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
  readTime(1, 1, datobs, utc);
  readData(FqRefVal, 1, &refFreq);
  readParm("BANDWID", TDOUBLE, &bandwidth);		// Core.

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  return 0;
}

//-------------------------------------------------- GBTFITSreader::getFreqInfo

// Get frequency parameters for each IF.

int GBTFITSreader::getFreqInfo(
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

//---------------------------------------------------- GBTFITSreader::findRange

// Find the range of the data in time and position.

int GBTFITSreader::findRange(
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
  short *sel = new short[cNRow];
  for (int irow = 0; irow < cNRow; irow++) {
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

    for (int irow = 0; irow < cNRow; irow++) {
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

    for (int irow = 0; irow < cNRow; irow++) {
      if (!cIFs[IFCol[irow]-cIF_1rel]) {
        sel[irow] = 0;
      }
    }

    delete [] IFCol;
  }

  nSel = 0;
  for (int irow = 0; irow < cNRow; irow++) {
    nSel += sel[irow];
  }


  // Find the time range assuming the data is in chronological order.
  readTime(1, 1, dateSpan[0], utcSpan[0]);
  readTime(cNRow, cNAxisTime, dateSpan[1], utcSpan[1]);


  // Retrieve positions for selected data.
  int isel = 0;
  positions = new double[2*nSel];

  if (cCoordSys == 1) {
    // Horizontal (Az,El).
    if (cData[AZIMUTH].colnum  < 0 ||
        cData[ELEVATIO].colnum < 0) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::WARN, "Azimuth/elevation information absent.");
      cStatus = -1;

    } else {
      float *az = new float[cNRow];
      float *el = new float[cNRow];
      readCol(AZIMUTH,  az);
      readCol(ELEVATIO, el);

      if (!cStatus) {
        for (int irow = 0; irow < cNRow; irow++) {
          if (sel[irow]) {
            positions[isel++] = az[irow] * D2R;
            positions[isel++] = el[irow] * D2R;
          }
        }
      }

      delete [] az;
      delete [] el;
    }

  } else if (cCoordSys == 3) {
    // ZPA-EL.
    if (cData[BEAM].colnum < 0 ||
        cData[FOCUSROT].colnum < 0 ||
        cData[ELEVATIO].colnum < 0) {
      log(LogOrigin( className, methodName, WHERE ), LogIO::WARN, "ZPA/elevation information absent.");
      cStatus = -1;

    } else {
      short *beam = new short[cNRow];
      float *rot  = new float[cNRow];
      float *el   = new float[cNRow];
      readCol(BEAM,     beam);
      readCol(FOCUSROT, rot);
      readCol(ELEVATIO, el);

      if (!cStatus) {
        for (int irow = 0; irow < cNRow; irow++) {
          if (sel[irow]) {
            Int beamNo = beam[irow];
            Double zpa = rot[irow];
            if (beamNo > 1) {
              // Beam geometry for the Parkes multibeam.
              if (beamNo < 8) {
                zpa += -60.0 + 60.0*(beamNo-2);
              } else {
                zpa += -90.0 + 60.0*(beamNo-8);
              }

              if (zpa < -180.0) {
                zpa += 360.0;
              } else if (zpa > 180.0) {
                zpa -= 360.0;
              }
            }

            positions[isel++] = zpa * D2R;
            positions[isel++] = el[irow] * D2R;
          }
        }
      }

      delete [] beam;
      delete [] rot;
      delete [] el;
    }

  } else {
    double *ra  = new double[cNRow];
    double *dec = new double[cNRow];
    readCol(RA,  ra);
    readCol(DEC, dec);

    if (cStatus) {
      delete [] ra;
      delete [] dec;
      goto cleanup;
    }

    if (cALFA_BD) {
      for (int irow = 0; irow < cNRow; irow++) {
        // Convert hours to degrees.
        ra[irow] *= 15.0;
      }
    }

    if (cCoordSys == 0) {
      // Equatorial (RA,Dec).
      for (int irow = 0; irow < cNRow; irow++) {
        if (sel[irow]) {
          positions[isel++] =  ra[irow] * D2R;
          positions[isel++] = dec[irow] * D2R;
        }
      }

    } else if (cCoordSys == 2) {
      // Feed-plane.
      if (cData[OBJ_RA].colnum   < 0 ||
          cData[OBJ_DEC].colnum  < 0 ||
          cData[PARANGLE].colnum < 0 ||
          cData[FOCUSROT].colnum < 0) {
        log( LogOrigin( className, methodName, WHERE ), LogIO::WARN, 
             "Insufficient information to compute feed-plane\n"
             "         coordinates.");
        cStatus = -1;

      } else {
        double *srcRA  = new double[cNRow];
        double *srcDec = new double[cNRow];
        float  *par = new float[cNRow];
        float  *rot = new float[cNRow];

        readCol(OBJ_RA,   srcRA);
        readCol(OBJ_DEC,  srcDec);
        readCol(PARANGLE, par);
        readCol(FOCUSROT, rot);

        if (!cStatus) {
          for (int irow = 0; irow < cNRow; irow++) {
            if (sel[irow]) {
              // Convert to feed-plane coordinates.
              Double dist, pa;
              distPA(ra[irow]*D2R, dec[irow]*D2R, srcRA[irow]*D2R,
                     srcDec[irow]*D2R, dist, pa);

              Double spin = (par[irow] + rot[irow])*D2R - pa;
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


//--------------------------------------------------------- GBTFITSreader::read

// Read the next data record.

int GBTFITSreader::read(
        MBrecord &mbrec)
{
  const string methodName = "read()" ;

  // for multiple binary table 
  long cRowR = -1 ;

  // Has the file been opened?
  if (!cSDptr) {
    return 1;
  }
  // Find the next selected beam and IF.
  short iBeam = 0, iIF = 0;
  int iPol = -1 ;
  while (1) {
    if (++cTimeIdx > cNAxisTime) {
      if (++cRow > cNRow) break;
      cTimeIdx = 1;
    }
    
    //cRowR = cRow ;
    cRowR = cRowRelative( cRow ) ;

    if (cData[BEAM].colnum > 0) {
      readData(BEAM, cRowR, &iBeam);

      // Convert to 0-relative.
      if (cBeam_1rel) iBeam--;
    }


    if (cBeams[iBeam]) {
      if (cData[IF].colnum > 0) {
        readData(IF, cRowR, &iIF);

        // Convert to 0-relative.
        if (cIF_1rel) iIF--;
      }

      if (cIFs[iIF]) {
        if (cALFA) {
          // ALFA data, check for calibration data.
          char chars[32];
          readData(OBSMODE, cRowR, chars);
          if (strcmp(chars, "DROP") == 0) {
            // Completely flagged integration.
            continue;

          } else if (strcmp(chars, "CAL") == 0) {
            sReset = 1;
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

          } else {
            // Reset for the next CAL record.
            if (sReset) {
              for (short iPol = 0; iPol < cNPol[iIF]; iPol++) {
                sALFAcalNon[iBeam][iPol]  = 0;
                sALFAcalNoff[iBeam][iPol] = 0;
                sALFAcalOn[iBeam][iPol]   = 0.0f;
                sALFAcalOff[iBeam][iPol]  = 0.0f;
              }
              sReset = 0;

              sprintf(cMsg, "ALFA cal factors for beam %d: %.3e, %.3e",
                iBeam+1, sALFAcal[iBeam][0], sALFAcal[iBeam][1]);
              log(LogOrigin( className, methodName, WHERE ), LogIO::NORMAL, cMsg);
              //logMsg(cMsg);
            }
          }
        }

        // for GBT SDFITS
        if (cData[STOKES].colnum > 0 ) {
          readData(STOKES, cRowR, &iPol ) ;
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

  int idx_GBT = 0 ;
  if ( numHDU > 1 ) {
    char sampler[10];
    readData(SAMPLER,cRowR,sampler);
    readData(RESTFRQ, cRowR, &mbrec.restFreq);
    int iiIF = 0 ;
    int hduid ;
    fits_get_hdu_num( cSDptr, &hduid ) ;
    while ( iiIF < numHDU && hduId[iiIF] != hduid ) {
      //iiIF++ ;
      idx_GBT += cNumKeys[iiIF++] ;
    }
    char sarf[48] ;
    sprintf( sarf, "%d/%s/%lf", iiIF, sampler, mbrec.restFreq ) ;
    String sr = String( sarf ) ;
    while ( idx_GBT < (int)cRefValKey.size() && sr != cRefValKey[idx_GBT] )
      idx_GBT++ ;
    iIF = (short)cIFNO[idx_GBT] ;
    sprintf( cMsg, "iIF = %d (key=%s), cRow = %ld" , iIF, sarf, cRow ) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
  }

  if (cALFA) {
    int scanNo;
    readData(SCAN, cRowR, &scanNo);
    if (scanNo != cALFAscan) {
      cScanNo++;
      cALFAscan = scanNo;
    }
    mbrec.scanNo = cScanNo;

  } else {
    readData(SCAN, cRowR, &mbrec.scanNo);

    // Ensure that scan number is 1-relative.
    mbrec.scanNo -= (cFirstScanNo - 1);
  }

  // Times.
  char datobs[32];
  readTime(cRowR, cTimeIdx, datobs, mbrec.utc);
  strcpy(mbrec.datobs, datobs);

  if (cData[CYCLE].colnum > 0) {
    readData(CYCLE, cRowR, &mbrec.cycleNo);
    mbrec.cycleNo += cTimeIdx - 1;
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

  readData(EXPOSURE, cRowR, &mbrec.exposure);

  // Source identification.
  readData(OBJECT, cRowR, mbrec.srcName);

  if ( iPol != -1 ) {
    char obsmode[32] ;
    readData( OBSMODE, cRowR, obsmode ) ;
    char sig[1] ;
    char cal[1] ;
    readData( SIG, cRowR, sig ) ;
    readData( CAL, cRowR, cal ) ;
    if ( strstr( obsmode, "PSWITCH" ) != NULL ) {
      // position switch
      //strcat( mbrec.srcName, "_p" ) ;
      if ( strstr( obsmode, "PSWITCHON" ) != NULL ) {
        //strcat( mbrec.srcName, "s" ) ;
        mbrec.srcType = SrcType::PSON ; 
      }
      else if ( strstr( obsmode, "PSWITCHOFF" ) != NULL ) {
        //strcat( mbrec.srcName, "r" ) ;
        mbrec.srcType = SrcType::PSOFF ;
      }
    }
    else if ( strstr( obsmode, "Nod" ) != NULL ) {
      // nod
      //strcat( mbrec.srcName, "_n" ) ;
      mbrec.srcType = SrcType::NOD ;
      if ( sig[0] == 'T' ) {
        //strcat( mbrec.srcName, "s" ) ;
      }
      else {
        //strcat( mbrec.srcName, "r" ) ;
      }
    }
    else if ( strstr( obsmode, "FSWITCH" ) != NULL ) {
      // frequency switch
      //strcat( mbrec.srcName, "_f" ) ;
      if ( sig[0] == 'T' ) {
        //strcat( mbrec.srcName, "s" ) ;
        mbrec.srcType = SrcType::FSON ; 
      }
      else {
        //strcat( mbrec.srcName, "r" ) ;
        mbrec.srcType = SrcType::FSOFF ; 
      }
    }
    if ( cal[0] == 'T' ) {
      //strcat( mbrec.srcName, "c" ) ;
      if ( mbrec.srcType == SrcType::PSON )
        mbrec.srcType = SrcType::PONCAL ;
      else if ( mbrec.srcType == SrcType::PSOFF )
        mbrec.srcType = SrcType::POFFCAL ;
      else if ( mbrec.srcType == SrcType::NOD )
        mbrec.srcType = SrcType::NODCAL ;
      else if ( mbrec.srcType == SrcType::FSON )
        mbrec.srcType = SrcType::FONCAL ;
      else if ( mbrec.srcType == SrcType::FSOFF )
        mbrec.srcType = SrcType::FOFFCAL ;
    }
    else {
      //strcat( mbrec.srcName, "o" ) ;
    }
  }

  readData(OBJ_RA,  cRowR, &mbrec.srcRA);
  if (strcmp(cData[OBJ_RA].name, "OBJ-RA") == 0) {
    mbrec.srcRA  *= D2R;
  }

  if (strcmp(cData[OBJ_DEC].name, "OBJ-DEC") == 0) {
    readData(OBJ_DEC, cRowR, &mbrec.srcDec);
    mbrec.srcDec *= D2R;
  }

  // Line rest frequency (Hz).
  readData(RESTFRQ, cRowR, &mbrec.restFreq);
  if (mbrec.restFreq == 0.0 && cALFA_BD) {
    mbrec.restFreq = 1420.40575e6;
  }

  // Observation mode.
  readData(OBSMODE, cRowR, mbrec.obsType);

  // Beam-dependent parameters.
  mbrec.beamNo = iBeam + 1;

  readData(RA,  cRowR, &mbrec.ra);
  readData(DEC, cRowR, &mbrec.dec);
  mbrec.ra  *= D2R;
  mbrec.dec *= D2R;

  if (cALFA_BD) mbrec.ra *= 15.0;

  float scanrate[2];
  readData(SCANRATE, cRowR, &scanrate);
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

  readData(FqRefPix, cRowR, mbrec.fqRefPix);
  readData(FqRefVal, cRowR, mbrec.fqRefVal);
  readData(FqDelt,   cRowR, mbrec.fqDelt);

  // Convert frequency reference to LSRK if GBT TOPO
  if ( cGBT && numHDU > 1 ) {
    char sampler[10];
    readData(SAMPLER,cRowR,sampler);
    //Double fqrefval = getRefValLSR( cRow, idx_GBT, sampler, mbrec.restFreq ) ;
    sprintf( cMsg, "Row %ld (%s): reference frequency %lf", cRow, sampler, cRefVal[idx_GBT] ) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;

    if ( idx_GBT == (int)cRefValKey.size() ) {
      log( LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to get reference frequency value" ) ;
      mbrec.fqRefVal[0] = 0.0 ;
    }
    else {
      mbrec.fqRefVal[0] = (double)cRefVal[idx_GBT] ; 
    }
  }

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
    readData(TSYS, cRowR, mbrec.tsys[0]);
  }

  for (int j = 0; j < 2; j++) {
    mbrec.calfctr[0][j] = 0.0f;
  }
  if (cData[CALFCTR].colnum > 0) {
    readData(CALFCTR, cRowR, mbrec.calfctr);
  }

  if (cHaveBase) {
    mbrec.haveBase = 1;
    readData(BASELIN, cRowR, mbrec.baseLin);
    readData(BASESUB, cRowR, mbrec.baseSub);
  } else {
    mbrec.haveBase = 0;
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  // Read data, sectioning and transposing it in the process.
  long *blc = new long[cNAxes+1];
  long *trc = new long[cNAxes+1];
  long *inc = new long[cNAxes+1];
  for (int iaxis = 0; iaxis <= cNAxes; iaxis++) {
    blc[iaxis] = 1;
    trc[iaxis] = 1;
    inc[iaxis] = 1;
  }

  blc[cFreqAxis] = std::min(startChan, endChan);
  trc[cFreqAxis] = std::max(startChan, endChan);
  if (cTimeAxis >= 0) {
    blc[cTimeAxis] = cTimeIdx;
    trc[cTimeAxis] = cTimeIdx;
  }
  blc[cNAxes] = cRowR;
  trc[cNAxes] = cRowR;

  mbrec.haveSpectra = cGetSpectra;
  if (cGetSpectra) {
    int  anynul;

    for (int iPol = 0; iPol < nPol; iPol++) {
      blc[cStokesAxis] = iPol+1;
      trc[cStokesAxis] = iPol+1;

      if (cALFA && cALFA_CIMA < 2) {
        // ALFA data: polarizations are stored in successive rows.
        blc[cStokesAxis] = 1;
        trc[cStokesAxis] = 1;

        if (iPol) {
          if (++cRow > cNRow) {
            return -1;
          }

          blc[cNAxes] = cRowR;
          trc[cNAxes] = cRowR;
        }

      } else if (cData[DATA].nelem < 0) {
        // Variable dimension array; get axis lengths.
        int naxes = 5, status;

        if ((status = readDim(DATA, cRowR, &naxes, cNAxis))) {
          log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);

        } else if ((status = (naxes != cNAxes))) {
          log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "DATA array dimensions changed.");
        }

        if (status) {
          delete [] blc;
          delete [] trc;
          delete [] inc;
          return 1;
        }
      }

      if (fits_read_subset_flt(cSDptr, cData[DATA].colnum, cNAxes, cNAxis,
          blc, trc, inc, 0, mbrec.spectra[0] + iPol*nChan, &anynul,
          &cStatus)) {
        log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
        delete [] blc;
        delete [] trc;
        delete [] inc;
        return 1;
      }

      if (endChan < startChan) {
        // Reverse the spectrum.
        float *iptr = mbrec.spectra[0] + iPol*nChan;
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
        float el, zd;
        readData(ELEVATIO, cRowR, &el);
        zd = 90.0f - el;

        float factor = sALFAcal[iBeam][iPol] / alfaGain(zd);

        if (cALFA_CIMA > 1) {
          // Rescale according to the number of unblanked accumulations.
          int colnum, naccum;
          findCol("STAT", &colnum);
          fits_read_col(cSDptr, TINT, colnum, cRowR, 10*(cTimeIdx-1)+2, 1, 0,
                        &naccum, &anynul, &cStatus);
          factor *= cALFAacc / naccum;
        }

        float *chan  = mbrec.spectra[0] + iPol*nChan;
        float *chanN = chan + nChan;
        while (chan < chanN) {
          // Approximate conversion to Jy.
          *(chan++) *= factor;
        }
      }

      if (mbrec.tsys[0][iPol] == 0.0) {
        // Compute Tsys as the average across the spectrum.
        float *chan  = mbrec.spectra[0] + iPol*nChan;
        float *chanN = chan + nChan;
        float *tsys = mbrec.tsys[0] + iPol;
        while (chan < chanN) {
          *tsys += *(chan++);
        }

        *tsys /= nChan;
      }

      // Read data flags.
      if (cData[FLAGGED].colnum > 0) {
        if (fits_read_subset_byt(cSDptr, cData[FLAGGED].colnum, cNAxes,
            cNAxis, blc, trc, inc, 0, mbrec.flagged[0] + iPol*nChan, &anynul,
            &cStatus)) {
          log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
          delete [] blc;
          delete [] trc;
          delete [] inc;
          return 1;
        }

        if (endChan < startChan) {
          // Reverse the flag vector.
          unsigned char *iptr = mbrec.flagged[0] + iPol*nChan;
          unsigned char *jptr = iptr + nChan - 1;
          for (int ichan = 0; ichan < nChan/2; ichan++) {
            unsigned char tmp = *iptr;
            *(iptr++) = *jptr;
            *(jptr--) = tmp;
          }
        }

      } else {
        // All channels are unflagged by default.
        unsigned char *iptr = mbrec.flagged[0] + iPol*nChan;
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
      readData(XCALFCTR, cRowR, mbrec.xcalfctr);
    }

    blc[0] = 1;
    trc[0] = 2;
    blc[1] = std::min(startChan, endChan);
    trc[1] = std::max(startChan, endChan);
    blc[2] = cRowR;
    trc[2] = cRowR;

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
  readData(REFBEAM,  cRowR, &mbrec.refBeam);
  readData(TCAL,     cRowR, &mbrec.tcal[0]);
  readData(TCALTIME, cRowR,  mbrec.tcalTime);

  readData(AZIMUTH,  cRowR, &mbrec.azimuth);
  readData(ELEVATIO, cRowR, &mbrec.elevation);
  readData(PARANGLE, cRowR, &mbrec.parAngle);

  readData(FOCUSAXI, cRowR, &mbrec.focusAxi);
  readData(FOCUSTAN, cRowR, &mbrec.focusTan);
  readData(FOCUSROT, cRowR, &mbrec.focusRot);

  readData(TAMBIENT, cRowR, &mbrec.temp);
  readData(PRESSURE, cRowR, &mbrec.pressure);
  readData(HUMIDITY, cRowR, &mbrec.humidity);
  readData(WINDSPEE, cRowR, &mbrec.windSpeed);
  readData(WINDDIRE, cRowR, &mbrec.windAz);

  if (cALFA_BD) {
    // ALFA BDFITS stores zenith angle rather than elevation.
    mbrec.elevation = 90.0 - mbrec.elevation;
  }

  mbrec.azimuth   *= D2R;
  mbrec.elevation *= D2R;
  mbrec.parAngle  *= D2R;
  mbrec.focusRot  *= D2R;
  mbrec.windAz    *= D2R;

  // For GBT data, source velocity can be evaluated
  if ( cData[RVSYS].colnum > 0 && cData[VFRAME].colnum > 0 ) {
    double vframe;
    readData(VFRAME, cRowR, &vframe);
    double rvsys;
    readData(RVSYS,  cRowR, &rvsys);
    //mbrec.srcVelocity = rvsys - vframe ;
    mbrec.srcVelocity = rvsys ;
  }

  if (cStatus) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    return 1;
  }

  return 0;
}

//-------------------------------------------------------- GBTFITSreader::close

// Close the SDFITS file.

void GBTFITSreader::close()
{
  if (cSDptr) {
    int status = 0;
    fits_close_file(cSDptr, &status);
    cSDptr = 0x0;

    if (cBeams)     delete [] cBeams;
    if (cIFs)       delete [] cIFs;
    if (cStartChan) delete [] cStartChan;
    if (cEndChan)   delete [] cEndChan;
    if (cRefChan)   delete [] cRefChan;
    if (cNRowT)     delete [] cNRowT;
    if (cNRowCum)   delete [] cNRowCum;
    if (hduId)      delete [] hduId;
  }
}

//------------------------------------------------------- GBTFITSreader::log

// Log a message.  If the current CFITSIO status value is non-zero, also log
// the corresponding error message and the CFITSIO message stack.

void GBTFITSreader::log(LogOrigin origin, LogIO::Command cmd, const char *msg)
{
  LogIO os( origin ) ;

  os << cmd ;

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

//----------------------------------------------------- GBTFITSreader::findData

// Locate a data item in the SDFITS file.

void GBTFITSreader::findData(
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

//------------------------------------------------------ GBTFITSreader::findCol

// Locate a parameter in the SDFITS file.

void GBTFITSreader::findCol(
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

//------------------------------------------------------ GBTFITSreader::readDim

// Determine the dimensions of an array in the SDFITS file.

int GBTFITSreader::readDim(
        int  iData,
        long iRow,
        int *naxes,
        long naxis[])
{
  int colnum = cData[iData].colnum;
  if (colnum <= 0) {
    return 1;
  }

  int maxdim = *naxes;
  if (cData[iData].tdimcol < 0) {
    // No TDIMnnn column for this array.
    if (cData[iData].nelem < 0) {
      // Variable length array; read the array descriptor.
      *naxes = 1;
      long dummy;
      if (fits_read_descript(cSDptr, colnum, iRow, naxis, &dummy, &cStatus)) {
        return 1;
      }

    } else {
      // Read the repeat count from TFORMnnn.
      if (fits_read_tdim(cSDptr, colnum, maxdim, naxes, naxis, &cStatus)) {
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
    *naxes = 0;
    for (size_t j = 1; j < strlen(tdimval); j++) {
      if (tdimval[j] == ',' || tdimval[j] == ')') {
        sscanf(tp, "%ld", naxis + (*naxes)++);
        if (tdimval[j] == ')') break;
        tp = tdimval + j + 1;
      }
    }
  }

  return 0;
}

//----------------------------------------------------- GBTFITSreader::readParm

// Read a parameter value from the SDFITS file.

int GBTFITSreader::readParm(
        char *name,
        int  type,
        void *value)
{
  return readData(name, type, 1, value);
}

//----------------------------------------------------- GBTFITSreader::readData

// Read a data value from the SDFITS file.

int GBTFITSreader::readData(
        char *name,
        int  type,
        long iRow,
        void *value)
{
  int colnum;
  findCol(name, &colnum);

  if (colnum > 0 && iRow > 0) {
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

//----------------------------------------------------- GBTFITSreader::readData

// Read data from the SDFITS file.

int GBTFITSreader::readData(
        int  iData,
        long iRow,
        void *value)
{
  int  type   = cData[iData].type;
  int  colnum = cData[iData].colnum;

  if (colnum > 0 && iRow > 0) {
    // Read the required number of values from the specified row of the table.
    long nelem = cData[iData].nelem;
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
    char *name  = cData[iData].name;
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

//------------------------------------------------------ GBTFITSreader::readCol

// Read a scalar column from the SDFITS file.

int GBTFITSreader::readCol(
        int  iData,
        void *value)
{
  int type = cData[iData].type;

  if (cData[iData].colnum > 0) {
    // Table column.
    int anynul;
    int offset = 0 ;
    for ( int ihdu = 0 ; ihdu < numHDU ; ihdu++ ) {
      fits_movabs_hdu(cSDptr, hduId[ihdu], NULL, &cStatus) ;
      if ( type == TSHORT ) {
        fits_read_col(cSDptr, type, cData[iData].colnum, 1, 1, cNRowT[ihdu], 0,
                      &((short *)value)[offset], &anynul, &cStatus);
      }
      else if ( type == TINT ) {
        fits_read_col(cSDptr, type, cData[iData].colnum, 1, 1, cNRowT[ihdu], 0,
                      &((int *)value)[offset], &anynul, &cStatus);
      }
      else if ( type == TFLOAT ) {
        fits_read_col(cSDptr, type, cData[iData].colnum, 1, 1, cNRowT[ihdu], 0,
                      &((float *)value)[offset], &anynul, &cStatus);
      }
      else if ( type == TDOUBLE ) {
        fits_read_col(cSDptr, type, cData[iData].colnum, 1, 1, cNRowT[ihdu], 0,
                      &((double *)value)[offset], &anynul, &cStatus);
      }
      offset = cNRowCum[ihdu] ;
    }
  } else {
    // Header keyword.
    readData(iData, 0, value);
    for (int irow = 1; irow < cNRow; irow++) {
      if (type == TSHORT) {
        ((short *)value)[irow] = *((short *)value);
      } else if (type == TINT) {
        ((int *)value)[irow] = *((int *)value);
      } else if (type == TFLOAT) {
        ((float *)value)[irow] = *((float *)value);
      } else if (type == TDOUBLE) {
        ((double *)value)[irow] = *((double *)value);
      }
    }
  }

  return cData[iData].colnum < 0;
}

//----------------------------------------------------- GBTFITSreader::readTime

// Read the time from the SDFITS file.

int GBTFITSreader::readTime(
        long iRow,
        int  iPix,
        char   *datobs,
        double &utc)
{
  readData(DATE_OBS, iRow, datobs);
  if (cData[TIME].colnum >= 0) {
    readData(TIME, iRow, &utc);
  } else if (cGBT) {
    Int yy, mm ;
    Double dd, hour, min, sec ;
    sscanf( datobs, "%d-%d-%lfT%lf:%lf:%lf", &yy, &mm, &dd, &hour, &min, &sec ) ;
    dd = dd + ( hour * 3600.0 + min * 60.0 + sec ) / 86400.0 ;
    MVTime mvt( yy, mm, dd ) ;
    dd = mvt.day() ;
    utc = fmod( dd, 1.0 ) * 86400.0 ;
  } else if (cNAxisTime > 1) {
    double timeDelt, timeRefPix, timeRefVal;
    readData(TimeRefVal, iRow, &timeRefVal);
    readData(TimeDelt,   iRow, &timeDelt);
    readData(TimeRefPix, iRow, &timeRefPix);
    utc = timeRefVal + (iPix - timeRefPix) * timeDelt;
  }

  if (cALFA_BD) utc *= 3600.0;

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

  } else if (datobs[10] == 'T' && cData[TIME].colnum < 0) {
    // Dig UTC out of a new-format DATE-OBS.
    int   hh, mm;
    float ss;
    sscanf(datobs+11, "%d:%d:%f", &hh, &mm, &ss);
    utc = (hh*60 + mm)*60 + ss;
  }

  datobs[10] = '\0';

  return 0;
}

//------------------------------------------------------ GBTFITSreader::alfaCal

// Process ALFA calibration data.

int GBTFITSreader::alfaCal(
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
  long *blc = new long[cNAxes+1];
  long *trc = new long[cNAxes+1];
  long *inc = new long[cNAxes+1];
  for (int iaxis = 0; iaxis <= cNAxes; iaxis++) {
    blc[iaxis] = 1;
    trc[iaxis] = 1;
    inc[iaxis] = 1;
  }

  // User channel selection.
  int startChan = cStartChan[iIF];
  int endChan   = cEndChan[iIF];

  blc[cFreqAxis] = std::min(startChan, endChan);
  trc[cFreqAxis] = std::max(startChan, endChan);
  if (cALFA_CIMA > 1) {
    // CIMAFITS 2.x has a legitimate STOKES axis...
    blc[cStokesAxis] = iPol+1;
    trc[cStokesAxis] = iPol+1;
  } else {
    // ...older ALFA data does not.
    blc[cStokesAxis] = 1;
    trc[cStokesAxis] = 1;
  }
  if (cTimeAxis >= 0) {
    blc[cTimeAxis] = cTimeIdx;
    trc[cTimeAxis] = cTimeIdx;
  }
  blc[cNAxes] = cRow;
  trc[cNAxes] = cRow;

  float spectrum[endChan];
  int anynul;
  if (fits_read_subset_flt(cSDptr, cData[DATA].colnum, cNAxes, cNAxis,
      blc, trc, inc, 0, spectrum, &anynul, &cStatus)) {
    log(LogOrigin( className, methodName, WHERE ), LogIO::SEVERE);
    delete [] blc;
    delete [] trc;
    delete [] inc;
    return 1;
  }

  // Factor to rescale according to the number of unblanked accumulations.
  float factor = 1.0f;
  if (cALFA_CIMA > 1) {
    int   colnum, naccum;
    findCol("STAT", &colnum);
    fits_read_col(cSDptr, TINT, colnum, cRow, 2, 1, 0, &naccum, &anynul,
                  &cStatus);
    factor = cALFAacc / naccum;
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
        sum += *chan * factor;
      }
    }

    mean = sum / nChan;
  }

  if (calOn) {
    sALFAcalOn[iBeam][iPol]  *= sALFAcalNon[iBeam][iPol];
    sALFAcalOn[iBeam][iPol]  += mean;
    sALFAcalOn[iBeam][iPol]  /= ++sALFAcalNon[iBeam][iPol];
  } else {
    sALFAcalOff[iBeam][iPol] *= sALFAcalNoff[iBeam][iPol];
    sALFAcalOff[iBeam][iPol] += mean;
    sALFAcalOff[iBeam][iPol] /= ++sALFAcalNoff[iBeam][iPol];
  }

  if (sALFAcalNon[iBeam][iPol] && sALFAcalNoff[iBeam][iPol]) {
    // Tcal should come from the TCAL table, it varies weakly with beam,
    // polarization, and frequency.  However, TCAL is not written properly.
    float Tcal = 12.0f;
    sALFAcal[iBeam][iPol] = Tcal / (sALFAcalOn[iBeam][iPol] -
                                    sALFAcalOff[iBeam][iPol]);

    // Scale from K to Jy; the gain also varies weakly with beam,
    // polarization, frequency, and zenith angle.
    float fluxCal = 10.0f;
    sALFAcal[iBeam][iPol] /= fluxCal;
  }

  return 0;
}

//----------------------------------------------------- GBTFITSreader::alfaGain

// ALFA gain factor.

float GBTFITSreader::alfaGain(
        float zd)
{
  // Gain vs zenith distance table from Robert Minchin, 2008/12/08.
  const int nZD = 37;
  const float zdLim[] = {1.5f, 19.5f};
  const float zdInc = (nZD - 1) / (zdLim[1] - zdLim[0]);
  float zdGain[] = {                                       1.00723708,
                    1.16644573,  1.15003645,  1.07117307,  1.02532673,
                    1.01788402,  1.01369524,  1.00000000,  0.989855111,
                    0.990888834, 0.993996620, 0.989964068, 0.982213855,
                    0.978662670, 0.979349494, 0.978478372, 0.974631131,
                    0.972126007, 0.972835243, 0.972742677, 0.968671739,
                    0.963891327, 0.963452935, 0.966831207, 0.969585896,
                    0.970700860, 0.972644389, 0.973754644, 0.967344403,
                    0.952168941, 0.937160134, 0.927843094, 0.914048433,
                    0.886700928, 0.864701211, 0.869126320, 0.854309499};

  float gain;
  // Do table lookup by linear interpolation.
  float lambda = zdInc * (zd - zdLim[0]);
  int j = int(lambda);
  if (j < 0) {
    gain = zdGain[0];
  } else if (j >= nZD-1) {
    gain = zdGain[nZD-1];
  } else {
    gain = zdGain[j] + (lambda - j) * (zdGain[j+1] - zdGain[j]);
  }

  return gain;
}

//----------------------------------------------------- GBTFITSreader::cRowRelative

// get relative row index in appropriate binary table 

long GBTFITSreader::cRowRelative( long iRow ) 
{
  // iRow is 1-relative
  const string methodName = "cRowRelative" ;
  if ( numHDU == 0 ) {
    log( LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, 
         "You should call open() first" ) ;
    return 0L ;
  }
  else if ( numHDU == 1 ) {
    if ( iRow > cNRow ) 
      return 0L ;
    else 
      return iRow ;
  }

  long jRow = 0L ;
  sprintf( cMsg, "iRow = %ld", iRow );
  log(LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg) ;
  if ( iRow <= cNRowCum[0] ) {
    jRow = iRow ;
    sprintf( cMsg, "Moving %d(%d)", 0, hduId[0] ) ;
    log(LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg) ;
    if ( fits_movabs_hdu( cSDptr, hduId[0], NULL, &cStatus ) ) {
      log( LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, 
           "Failed to move HDU" ) ;
      return 0L ;
    }
  }
  else {
    for ( int ihdu = 1 ; ihdu < numHDU ; ihdu++ ) {
      if ( iRow <= cNRowCum[ihdu] ) {
        jRow = iRow - cNRowCum[ihdu-1] ;
        sprintf( cMsg, "Moving %d(%d)", ihdu, hduId[ihdu] ) ;
        log(LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg) ;
        if ( fits_movabs_hdu( cSDptr, hduId[ihdu], NULL, &cStatus ) ) {
          log( LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, 
               "Failed to move HDU" ) ;
          return 0L ;
        }
        break ;
      }
    }
  }
  sprintf( cMsg, "jRow = %ld", jRow );
  log(LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg) ;
      
  return jRow ;
}

//----------------------------------------------------- GBTFITSreader::nIFPerHDU

// get number of IFs per HDU

Vector<Int> GBTFITSreader::nIFPerHDU() 
{
  const string methodName = "nIFPerHDU()" ;
  //cNumKeys.resize( numHDU ) ;
  cNumKeys.resize( 0 ) ;
  cRefValKey.resize( 0 ) ;
  cRefVal.resize( 0 ) ;
  cIncVal.resize( 0 ) ;
  cIFNO.resize( 0 ) ;
  Vector<Int> nIF_per_HDU( numHDU, 0 ) ;
  for ( int ihdu = 0 ; ihdu < numHDU ; ihdu++ ) {
    Vector<String> sampler( 0 ) ;
    Vector<Double> rfreq( 0 ) ;
    Vector<String> samprf( 0 ) ;
    if ( fits_movabs_hdu( cSDptr, hduId[ihdu], NULL, &cStatus ) ) {
      log( LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, 
           "Failed to move HDU" ) ;
      nIF_per_HDU = -1 ;
      return nIF_per_HDU ;
    }
    int startpos = (int) cRefValKey.size() ;
    int numkeys = 0 ;
    for ( int irow = 0 ; irow < cNRowT[ihdu] ; irow++ ) {
      char samp[24] ;
      double rf ;
      readData( SAMPLER, irow+1, samp ) ;
      readData( RESTFRQ, irow+1, &rf ) ;
      char sarf[48] ;
      sprintf( sarf, "%d/%s/%lf", ihdu, samp, rf ) ;
      String sr = String( sarf ) ;
      if ( allNE( sr, samprf ) ) {
        int oldsize = samprf.size() ;
        samprf.resize( oldsize+1, True ) ;
        samprf[oldsize] = sr ;
        sprintf( cMsg, "added samprf = %s", sarf ) ;
        log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
        Double refval = getRefValLSR( irow+1 ) ;
        double increm ;
        readData(FqDelt, irow+1, &increm);
        cRefValKey.push_back( sr ) ;
        cRefVal.push_back( (double)refval ) ;
        cIncVal.push_back( increm ) ;
        numkeys++ ;
      }
    }
    cNumKeys.push_back( numkeys ) ;
    nIF_per_HDU[ihdu] = mergeIFPerHDU( startpos, numkeys ) ;
    sprintf( cMsg, "number of IF for HDU %d = %d", 
             hduId[ihdu], nIF_per_HDU[ihdu] ) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::NORMAL, cMsg ) ;
  }

  return nIF_per_HDU ;
}

Double GBTFITSreader::getRefValLSR( int irow )
{
  const string methodName = "getRefValLSR()" ;

  char keyw[9], ctype[9];
  sprintf(keyw, "CTYPE%ld", cFreqAxis+1);
  readParm(keyw, TSTRING, ctype);
  double fqRefVal ;
  readData(FqRefVal, irow, &fqRefVal);
  if ( strstr( ctype, "-LSR" ) == NULL && strstr( ctype, "LSRK" ) == NULL ) {
    // need conversion
    Int year, month ;
    Double dd, hour, min, sec ;
    char dateObs[32] ;
    readData(DATE_OBS, irow, dateObs);
    sscanf( dateObs, "%4d-%2d-%2lfT%lf:%lf:%lf", &year, &month, &dd, &hour, &min, &sec ) ;
    dd = dd + ( hour * 3600.0 + min * 60.0 + sec ) / 86400.0 ;
    dd = MVTime(year, month, dd).day() ;
    double duration ;
    readData(DURATION, irow, &duration) ;
    dd += 0.5 * duration / 86400.0 ;
    MEpoch me( Quantity( dd, "d" ), MEpoch::UTC ) ;
    double antPos[3] ;
    readParm( "SITELONG", TDOUBLE, antPos ) ;
    readParm( "SITELAT", TDOUBLE, antPos+1 ) ;
    readParm( "SITEELEV", TDOUBLE, antPos+2 ) ;
    Vector<Double> pos( 2 ) ;
    pos[0] = Double( antPos[0] ) ;
    pos[1] = Double( antPos[1] ) ;
    MPosition mp( MVPosition( Quantity( Double(antPos[2]), "m" ), 
                              Quantum< Vector<Double> >( pos, "deg" ) ), 
                  MPosition::WGS84 ) ;
    double ra, dec ;
    readData(RA,  irow, &ra);
    readData(DEC, irow, &dec);
    MDirection md( Quantity( Double(ra), "deg" ), Quantity( Double(dec), "deg" ), MDirection::J2000 ) ;
    MeasFrame mf( me, mp, md ) ;
    MFrequency::Types fromType = MFrequency::DEFAULT ;
    if ( strstr( ctype, "-OBS" ) != NULL ) {
      fromType = MFrequency::TOPO ;
    }
    else if ( strstr( ctype, "-HEL" ) != NULL ) {
      fromType = MFrequency::BARY ;
    }
    else if ( strstr( ctype, "-GEO" ) != NULL ) {
      fromType = MFrequency::GEO ;
    }
    else if ( strstr( ctype, "-GAL" ) != NULL ) {
      fromType = MFrequency::GALACTO ;
    }
    else if ( strstr( ctype, "REST" ) != NULL ) {
      fromType = MFrequency::REST ;
    }
    MFrequency::Convert tolsr( fromType, MFrequency::Ref( MFrequency::LSRK, mf ) ) ;
    Double world = tolsr( Double( fqRefVal ) ).get( "Hz" ).getValue() ;
    char sampler[10];
    readData(SAMPLER,irow,sampler);
    int hduId = 0 ;
    fits_get_hdu_num( cSDptr, &hduId ) ;
    sprintf( cMsg, "HDU %d Row %d (%s): before conversion %.7f, after conversion %.7f", hduId, irow, sampler, fqRefVal, world ) ;
    log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;
    fqRefVal = (double)world ;
  }

  return Double(fqRefVal) ;
}

Double GBTFITSreader::getRefValLSR( long iRow, int idx, char *sampler, double restfreq ) 
{
  const string methodName = "getRefValLSR()" ;

  // get ihdu
//   int hduid = 0 ;
//   while ( iRow > cNRowCum[hduid] && hduid < numHDU) {
//     hduid++ ;
//   }

  // create key
//   char sarf[48] ;
//   sprintf( sarf, "%d/%s/%lf", hduid, sampler, restfreq ) ;
//   String sr = String( sarf ) ;
  
  sprintf( cMsg, "Row %ld (%s): reference frequency %lf", iRow, sampler, cRefVal[idx] ) ;
  log( LogOrigin( className, methodName, WHERE ), LogIO::DEBUGGING, cMsg ) ;

  if ( idx == (int)cRefValKey.size() ) {
    log( LogOrigin( className, methodName, WHERE ), LogIO::SEVERE, "Failed to get reference frequency value" ) ;
    return 0.0 ;
  }

  return cRefVal[idx] ;
}

Int GBTFITSreader::mergeIFPerHDU( int start, int n ) 
{
  Int if0 = 0 ;
  if ( cIFNO.size() > 0 ) {
    if0 = max( cIFNO ) + 1 ;
  }
  cIFNO.resize( start+n, True ) ;
  Vector< Vector<Double> > lIncVal( 0 ) ;
  for ( int i = start ; i < start+n ; i++ ) {
    Vector<Double> v( 2 ) ;
    v[0] = cRefVal[i] ;
    v[1] = cIncVal[i] ;
    int idx = -1 ;
    for ( int j = 0 ; j < (int)lIncVal.size() ; j++ ) {
      if ( allEQ( v, lIncVal[j] ) ) {
        idx = j ;
        break ;
      }
    }
    if ( idx == -1 ) {
      int oldsize = lIncVal.size() ;
      lIncVal.resize( oldsize+1, True ) ;
      lIncVal[oldsize] = v ;
      cIFNO[i] = if0 + oldsize ;
    }
    else {
      cIFNO[i] = if0 + idx ;
    }
  }
  return max(cIFNO) - if0 + 1 ;
}

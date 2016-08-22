//#---------------------------------------------------------------------------
//# SDFITSreader.h: ATNF CFITSIO interface class for SDFITS input.
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
//# $Id: SDFITSreader.h,v 19.22 2009-09-29 07:33:39 cal103 Exp $
//#---------------------------------------------------------------------------
//# The SDFITSreader class reads single dish FITS files such as those written
//# by SDFITSwriter containing Parkes Multibeam data.
//#
//# Original: 2000/08/09, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_SDFITSREADER_H
#define ATNF_SDFITSREADER_H

#include <atnf/PKSIO/FITSreader.h>
#include <atnf/PKSIO/MBrecord.h>

#include <casa/Logging/LogIO.h>

#include <fitsio.h>

using namespace std;
using namespace casa;

// <summary>
// ATNF class for SDFITS input using CFITSIO.
// </summary>

class SDFITSreader : public FITSreader
{
  public:
    // Default constructor.
    SDFITSreader();

    // Destructor.
    virtual ~SDFITSreader();

    // Open an SDFITS file for reading.
    virtual int open(
        char*  sdname,
        int    &nBeam,
        int*   &beams,
        int    &nIF,
        int*   &IFs,
        int*   &nChan,
        int*   &nPol,
        int*   &haveXPol,
        int    &haveBase,
        int    &haveSpectra,
        int    &extraSysCal);

    // Get parameters describing the data.
    virtual int getHeader(
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
        double &bandwidth);

    // Get frequency parameters for each IF.
    virtual int getFreqInfo(
        int     &nIF,
        double* &startFreq,
        double* &endFreq);

    // Find the range of the data selected in time and position.
    virtual int findRange(
        int    &nRow,
        int    &nSel,
        char   dateSpan[2][32],
        double utcSpan[2],
        double* &positions);

    // Read the next data record.
    virtual int read(MBrecord &record);

    // Close the SDFITS file.
    virtual void close(void);

  private:
    int      cCycleNo, cExtraSysCal, cNAxes, cStatus;
    long     cBeamAxis, cDecAxis, cFreqAxis, cNAxis[5], cNAxisTime, cNRow,
             cRaAxis, cRow, cStokesAxis, cTimeAxis, cTimeIdx;
    double   cLastUTC;
    fitsfile *cSDptr;
    class FITSparm *cData;

    // These are to differentiate 0-, and 1-relative beam and IF numbering.
    int  cBeam_1rel, cIF_1rel;

    // for GBT
    int *cPols ;

    enum {SCAN, CYCLE, DATE_OBS, TIME, EXPOSURE, OBJECT, OBJ_RA, OBJ_DEC,
          RESTFRQ, OBSMODE, BEAM, IF, FqRefVal, FqDelt, FqRefPix, RA, DEC,
          TimeRefVal, TimeDelt, TimeRefPix, SCANRATE, TSYS, CALFCTR, XCALFCTR,
          BASELIN, BASESUB, DATA, FLAGGED, DATAXED, XPOLDATA, REFBEAM, TCAL,
          TCALTIME, AZIMUTH, ELEVATIO, PARANGLE, FOCUSAXI, FOCUSTAN, FOCUSROT,
          TAMBIENT, PRESSURE, HUMIDITY, WINDSPEE, WINDDIRE, STOKES, SIG, CAL, 
          VFRAME, RVSYS, VELDEF, NDATA};

    // Message handling.
    void log(LogOrigin origin, LogIO::Command cmd, const char *msg = 0x0);

    void findData(int iData, char *name, int type);
    void  findCol(char *name, int *colnum);
    int   readDim(int iData, long iRow, int *naxis, long naxes[]);
    int  readParm(char *name, int type, void *value);
    int  readData(char *name, int type, long iRow, void *value);
    int  readData(int iData, long iRow, void *value);
    int  readCol(int iData, void *value);
    int  readTime(long iRow, int iPix, char *datobs, double &utc);

    // These are for ALFA data: "BDFITS" or "CIMAFITS".  Statics are required
    // for CIMAFITS v2.0 because CAL ON/OFF data is split into separate files.
    static int  sInit, sReset;
    static int  (*sALFAcalNon)[2], (*sALFAcalNoff)[2];
    static float (*sALFAcal)[2], (*sALFAcalOn)[2], (*sALFAcalOff)[2];

    int   cALFA, cALFA_BD, cALFA_CIMA, cALFAscan, cScanNo;
    float cALFAacc;
    int   alfaCal(short iBeam, short iIF, short iPol);
    float alfaGain(float zd);

    // These are for GBT data.
    int   cGBT, cFirstScanNo;
    double cGLastUTC[4] ;
    int cGLastScan[4] ;
    int cGCycleNo[4] ;
};

#endif

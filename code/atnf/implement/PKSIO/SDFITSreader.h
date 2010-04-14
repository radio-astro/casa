//#---------------------------------------------------------------------------
//# SDFITSreader.h: ATNF CFITSIO interface class for SDFITS input.
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
//# $Id: SDFITSreader.h,v 19.16 2008-11-17 06:47:05 cal103 Exp $
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
    int      cCycleNo, cExtraSysCal, cNAxis, cStatus;
    long     cNAxes[5], cNRow, cReqax[4], cRow;
    double   cLastUTC;
    fitsfile *cSDptr;
    class FITSparm *cData;

    // These are to differentiate 0-, and 1-relative beam and IF numbering.
    int  cBeam_1rel, cIF_1rel;

    // for GBT
    int *cPols ;

    enum {SCAN, CYCLE, DATE_OBS, TIME, EXPOSURE, OBJECT, OBJ_RA, OBJ_DEC,
          RESTFRQ, OBSMODE, BEAM, IF, FqRefPix, FqRefVal, FqDelt, RA, DEC,
          SCANRATE, TSYS, CALFCTR, XCALFCTR, BASELIN, BASESUB, DATA, FLAGGED,
          DATAXED, XPOLDATA, REFBEAM, TCAL, TCALTIME, AZIMUTH, ELEVATIO,
          PARANGLE, FOCUSAXI, FOCUSTAN, FOCUSROT, TAMBIENT, PRESSURE,
          HUMIDITY, WINDSPEE, WINDDIRE, STOKES, SIG, CAL, NDATA, VFRAME, RVSYS, VELDEF};

    // Message handling.
    void log(LogOrigin origin, LogIO::Command cmd, const char *msg = 0x0);

    void findData(int iData, char *name, int type);
    int   readDim(int iData, long iRow, int *naxis, long naxes[]);
    int  readParm(char *name, int type, void *value);
    int  readData(char *name, int type, long iRow, void *value);
    int  readData(int iData, long iRow, void *value);
    void  findCol(char *name, int *colnum);

    // These are for ALFA data: "BDFITS" or "CIMAFITS".
    int   cALFA, cALFA_BD, cALFA_CIMA, cALFAscan, cScanNo;
    float cALFAcal[8][2], cALFAcalOn[8][2], cALFAcalOff[8][2];
    int   alfaCal(short iBeam, short iIF, short iPol);

    // These are for GBT data.
    int   cGBT, cFirstScanNo;
    double cGLastUTC[4] ;
    int cGLastScan[4] ;
    int cGCycleNo[4] ;
};

#endif

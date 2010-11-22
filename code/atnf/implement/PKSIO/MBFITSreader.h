//#---------------------------------------------------------------------------
//# MBFITSreader.h: ATNF single-dish RPFITS reader.
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
//# $Id: MBFITSreader.h,v 19.23 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# The MBFITSreader class reads single dish RPFITS files (such as Parkes
//# Multibeam MBFITS files).
//#
//# Original: 2000/07/28 Mark Calabretta
//#---------------------------------------------------------------------------

#ifndef ATNF_MBFITSREADER_H
#define ATNF_MBFITSREADER_H

#include <atnf/PKSIO/FITSreader.h>
#include <atnf/PKSIO/MBrecord.h>

using namespace std;

// <summary>
// ATNF single-dish RPFITS reader.
// </summary>

class MBFITSreader : public FITSreader
{
  public:
    // Default constructor; position interpolation codes are:
    //   0: no interpolation,
    //   1: correct interpolation,
    //   2: pksmbfits interpolation.
    MBFITSreader(const int retry = 0, const int interpolate = 1);

    // Destructor.
    virtual ~MBFITSreader();

    // Open the RPFITS file for reading.
    virtual int open(
        char   *rpname,
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
        char   obsType[32],
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

    // Close the RPFITS file.
    virtual void close(void);

  private:
    // RPFITSIN subroutine arguments.
    int   cBaseline, cFlag, cBin, cIFno, cSrcNo;
    float cUTC, cU, cV, cW, *cVis, *cWgt;

    char   cDateObs[12];
    int    *cBeamSel, *cChanOff, cFirst, *cIFSel, cInterp, cIntTime, cMBopen,
           cMopra, cNBeamSel, cNBin, cRetry, cSimulIF, cSUpos, *cXpolOff;

    // The data has to be bufferred to allow positions to be interpolated.
    int    cEOF, cEOS, cFlushBin, cFlushIF, cFlushing;
    double *cPosUTC;
    MBrecord *cBuffer;

    // Scan and cycle number bookkeeping.
    int    cCycleNo, cScanNo;
    double cPrevUTC;

    // Read the next data record from the RPFITS file.
    int rpget(int syscalonly, int &EOS);
    int rpfitsin(int &jstat);

    // Check and, if necessary, repair a position timestamp.
    int    cCode5, cNRate;
    double cAvRate[2];
    int fixw(const char *datobs, int cycleNo, int beamNo, double avRate[2],
             double thisRA, double thisDec, double thisUTC,
             double nextRA, double nextDec, float &nextUTC);

    // Subtract two UTCs (s).
    double utcDiff(double utc1, double utc2);

    // Compute and apply the scan rate corrected for grid convergence.
    double cRA0, cDec0;
    void  scanRate(double ra0, double dec0,
                   double ra1, double dec1,
                   double ra2, double dec2, double dt,
                   double &raRate, double &decRate);
    void applyRate(double ra0, double dec0,
                   double ra1, double dec1,
                   double raRate, double decRate, double dt,
                   double &ra2, double &dec2);
    void eulerx(double lng0, double lat0, double phi0, double theta,
                double phi, double &lng1, double &lat1);
};

#endif

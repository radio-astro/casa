//#---------------------------------------------------------------------------
//# MBFITSreader.h: ATNF single-dish RPFITS reader.
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
//# The MBFITSreader class reads single dish RPFITS files (such as Parkes
//# Multibeam MBFITS files).
//#
//# Original: 2000/07/28 Mark Calabretta
//#---------------------------------------------------------------------------

#ifndef ATNF_MBFITSREADER_H
#define ATNF_MBFITSREADER_H

#include <atnf/PKSIO/FITSreader.h>
#include <atnf/PKSIO/PKSMBrecord.h>

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
    virtual int read(PKSMBrecord &record);

    // Close the RPFITS file.
    virtual void close(void);

  private:
    char   cDateObs[10];
    int    *cBeamSel, *cChanOff, cFirst, *cIFSel, cInterp, cIntTime, cMBopen,
           cMopra, cNBeamSel, cNBin, cRetry, *cStaleness, cTid, *cXpolOff;
    float  *cVis;

    // The data has to be bufferred to allow positions to be interpolated.
    int    cEOF, cEOS, cFlushBin, cFlushIF, cFlushing;
    double *cPosUTC;
    PKSMBrecord *cBuffer;

    int    cCycleNo, cScanNo;
    double cUTC;

    // Read the next data record from the RPFITS file.
    int rpget(int syscalonly, int &EOS);
    // These are no longer define in the RPFITS.h file so moved here
    int jstat;
    float *weight;
    int baseline;
    int flag;
    int bin;
    int if_no;
    int sourceno;
    float *vis;
    float u;
    float v;
    float w;
    float ut;
	    
};

#endif

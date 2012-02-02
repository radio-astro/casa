//#---------------------------------------------------------------------------
//# FITSreader.h: ATNF single-dish FITS reader.
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
//# $Id: FITSreader.h,v 19.11 2009-09-29 07:33:38 cal103 Exp $
//#---------------------------------------------------------------------------
//# The FITSreader class is an abstract base class for the Parkes Multibeam
//# RPFITS and SDFITS readers.
//#
//# Original: 2000/08/14 Mark Calabretta
//#---------------------------------------------------------------------------

#ifndef ATNF_FITSREADER_H
#define ATNF_FITSREADER_H

#include <atnf/PKSIO/MBrecord.h>

using namespace std;


// <summary>
// ATNF single-dish FITS reader.
// </summary>

//class FITSreader
class FITSreader
{
  public:
    // Destructor.
    virtual ~FITSreader() {};

    // Open the FITS file for reading.  Returns a pointer, beams, to an array
    // of length nBeam that indicates which beams are present in the data.
    // Beam selection is done by zeroing the unwanted elements of this array.
    // Likewise for IF selection (e.g. for frequency-switched data).
    virtual int open(
        char   *FITSname,
        int    &nBeam,
        int*   &beams,
        int    &nIF,
        int*   &IFs,
        int*   &nChan,
        int*   &nPol,
        int*   &haveXPol,
        int    &haveBase,
        int    &haveSpectra,
        int    &extraSysCal) = 0;

    // Get metadata.
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
        double &bandwidth) = 0;

    // Get frequency parameters for each IF.
    virtual int getFreqInfo(
        int     &nIF,
        double* &startFreq,
        double* &endFreq) = 0;

    // Set data selection criteria.  Channel numbering is 1-relative, zero or
    // negative channel numbers are taken to be offsets from the last channel.
    // Coordinate systems are
    //   0: equatorial (RA,Dec),
    //   1: horizontal (Az,El),
    //   2: feed-plane,
    //   3: zenithal position angle of feed and elevation, (ZPA,El).
    int select(
        const int startChan[],
        const int endChan[],
        const int refChan[],
        const int getSpectra = 1,
        const int getXPol = 0,
        const int getFeedPos = 0,
        const int getPointing = 0,
        const int coordSys = 0);


    // Find the range in time and position of the data selected.
    virtual int findRange(
        int    &nRow,
        int    &nSel,
        char   dateSpan[2][32],
        double utcSpan[2],
        double* &positions) = 0;

    // Read the next data record.
    virtual int read(
//        PKSMBrecord &record) = 0;
        MBrecord &record) = 0;

    // Close the RPFITS file.
    virtual void close(void) = 0;

  protected:
    int  *cBeams, *cEndChan, cGetFeedPos, cCoordSys, cGetSpectra, cGetXPol, 
           cHaveBase, cHaveSpectra, *cHaveXPol, *cIFs, cNBeam, *cNChan, cNIF, 
           *cNPol, *cRefChan, *cStartChan;

    // For use in constructing messages.
    char cMsg[256];

};

#endif

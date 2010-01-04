//#---------------------------------------------------------------------------
//# FITSreader.h: ATNF single-dish FITS reader.
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
//# The FITSreader class is an abstract base class for the Parkes Multibeam
//# RPFITS and SDFITS readers.
//#
//# Original: 2000/08/14 Mark Calabretta
//#---------------------------------------------------------------------------

#ifndef ATNF_FITSREADER_H
#define ATNF_FITSREADER_H

#include <atnf/PKSIO/PKSMBrecord.h>

// <summary>
// ATNF single-dish FITS reader.
// </summary>

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
    int select(
        const int startChan[],
        const int endChan[],
        const int refChan[],
        const int getSpectra = 1,
        const int getXPol = 0,
        const int getFeedPos = 0);

    // Find the range in time and position of the data selected.
    virtual int findRange(
        int    &nRow,
        int    &nSel,
        char   dateSpan[2][32],
        double utcSpan[2],
        double* &positions) = 0;

    // Read the next data record.
    virtual int read(
        PKSMBrecord &record) = 0;

    // Close the RPFITS file.
    virtual void close(void) = 0;

  protected:
    int    *cBeams, *cEndChan, cGetFeedPos, cGetSpectra, cGetXPol, cHaveBase,
           cHaveSpectra, *cHaveXPol, *cIFs, cNBeam, *cNChan, cNIF, *cNPol,
           *cRefChan, *cStartChan;
};

#endif

//#---------------------------------------------------------------------------
//# SDFITSwriter.h: ATNF CFITSIO interface class for SDFITS output.
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
//# $Id: SDFITSwriter.h,v 19.10 2009-09-29 07:33:39 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2000/07/24, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_SDFITSWRITER_H
#define ATNF_SDFITSWRITER_H

#include <atnf/PKSIO/MBrecord.h>
#include <atnf/PKSIO/PKSmsg.h>

#include <fitsio.h>

using namespace std;

// <summary>
// ATNF CFITSIO interface class for SDFITS output.
// </summary>

class SDFITSwriter : public PKSmsg
{
  public:
    // Default constructor.
    SDFITSwriter();

    // Destructor.
    virtual ~SDFITSwriter();

    // Create a new SDFITSwriter and store static data.
    int create(
        char*  sdname,
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
        int    extraSysCal);

    // Store time-variable data.
    int write(MBrecord &record);

    // Write a history record.
    int history(char* text);

    // Close the SDFITS file.
    void close();

    // Close and delete the SDFITS file.
    void deleteFile();

  private:
    fitsfile *cSDptr;
    int  cDoTDIM, cDoXPol, cExtraSysCal, cHaveBase, *cHaveXPol, cIsMX,
         *cNChan, cNIF, *cNPol, cStatus;
    long cRow;

    // Message handling.
    char cMsg[256];
    virtual void logMsg(const char *msg = 0x0);
};

#endif

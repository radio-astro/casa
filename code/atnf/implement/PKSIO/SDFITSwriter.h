//#---------------------------------------------------------------------------
//# SDFITSwriter.h: ATNF CFITSIO interface class for SDFITS output.
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
//# Original: 2000/07/24, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#ifndef ATNF_SDFITSWRITER_H
#define ATNF_SDFITSWRITER_H

#include <atnf/PKSIO/PKSMBrecord.h>

#include <fitsio.h>

// <summary>
// ATNF CFITSIO interface class for SDFITS output.
// </summary>

class SDFITSwriter
{
  public:
    // Default constructor.
    SDFITSwriter();

    // Destructor.
    ~SDFITSwriter();

    // Create a new SDFITSwriter and store static data.
    int create(
        char*  sdname,
        char*  observer,
        char*  project,
        char*  telescope,
        double antPos[3],
        char*  obsMode,
        float  equinox,
        char*  dopplerFrame,
        int    nIF,
        int*   nChan,
        int*   nPol,
        int*   haveXPol,
        int    haveBase,
        int    extraSysCal);

    // Store time-variable data.
    int write(PKSMBrecord &record);

    // Print out CFITSIO error messages.
    void reportError();

    // Close the SDFITS file.
    void close();

    // Close and delete the SDFITS file.
    void deleteFile();

  private:
    fitsfile *cSDptr;
    int  cDoTDIM, cDoXPol, cExtraSysCal, cHaveBase, *cHaveXPol, cIsMX,
         *cNChan, cNIF, *cNPol, cStatus;
    long cRow;
};

#endif

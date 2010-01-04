//#---------------------------------------------------------------------------
//# FITSreader.cc: ATNF single-dish FITS reader.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2007
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
//# Original: 2000/07/28 Mark Calabretta
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/FITSreader.h>

#include <algorithm>

using namespace std;

//--------------------------------------------------------- FITSreader::select

// Set data selection criteria.  Note that cBeams and cIFs, the addresses of
// int arrays, are returned by open() thereby allowing their elements to be
// modified directly elsewhere (specifically by PKSFITSreader::select()).

int FITSreader::select(
        const int startChan[],
        const int endChan[],
        const int refChan[],
        const int getSpectra,
        const int getXPol,
        const int getFeedPos,
        const int getPointing,
        const int coordSys)
{
  int maxNChan = 0;

  for (int iIF = 0; iIF < cNIF; iIF++) {
    if (!cIFs[iIF]) {
      continue;
    }

    cStartChan[iIF] = startChan[iIF];
    cEndChan[iIF]   = endChan[iIF];
    cRefChan[iIF]   = refChan[iIF];

    if (cStartChan[iIF] <= 0) {
      cStartChan[iIF] += cNChan[iIF];
    } else if (cStartChan[iIF] > cNChan[iIF]) {
      cStartChan[iIF]  = cNChan[iIF];
    }

    if (cEndChan[iIF] <= 0) {
      cEndChan[iIF] += cNChan[iIF];
    } else if (cEndChan[iIF] > cNChan[iIF]) {
      cEndChan[iIF]  = cNChan[iIF];
    }

    maxNChan = max(maxNChan, abs(cEndChan[iIF] - cStartChan[iIF]) + 1);
  }

  cGetSpectra = getSpectra && cHaveSpectra;
  cGetXPol    = getXPol    && cGetXPol;
  cGetFeedPos = getFeedPos;
  cCoordSys   = coordSys;


  return maxNChan;
}

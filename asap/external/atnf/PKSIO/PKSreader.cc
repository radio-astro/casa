//#---------------------------------------------------------------------------
//# PKSreader.cc: Class to read Parkes multibeam data.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//#---------------------------------------------------------------------------
//# Original: 2000/08/23, Mark Calabretta, ATNF
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/PKSreader.h>
#include <atnf/PKSIO/PKSFITSreader.h>

#ifndef NOPKSMS
#include <atnf/PKSIO/PKSMS2reader.h>
#endif

#include <casa/IO/RegularFileIO.h>
#include <casa/OS/File.h>


//--------------------------------------------------------------- getPKSreader

// Return an appropriate PKSreader for a Parkes Multibeam dataset.

PKSreader* getPKSreader(
        const String name,
        const Int retry,
        const Int interpolate,
        String &format,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra)
{
  // Check accessibility of the input.
  File inFile(name);
  if (!inFile.exists()) {
    format = "DATASET NOT FOUND";
    return 0;
  }

  if (!inFile.isReadable()) {
    format = "DATASET UNREADABLE";
    return 0;
  }

  // Determine the type of input.
  PKSreader *reader = 0;
  if (inFile.isRegular()) {
    // Is it MBFITS or SDFITS?
    RegularFileIO file(name);
    char buf[32];
    file.read(30, buf, False);
    buf[30] = '\0';
    if (String(buf) == "SIMPLE  =                    T") {
      // Looks like SDFITS.
      format = "SDFITS";
      reader = new PKSFITSreader("SDFITS");

    } else {
      // Assume it's MBFITS.
      format = "MBFITS";
      reader = new PKSFITSreader("MBFITS", retry, interpolate);
    }

  } else if (inFile.isDirectory()) {
    if (File(name + "/DATA_DESCRIPTION").exists()) {
      // MS version 2.
      #ifdef NOPKSMS
      format = "MS2 INPUT FORMAT IS NO LONGER SUPPORTED";
      #else
      format = "MS2";
      reader = new PKSMS2reader();
      #endif
    }

  } else {
    format = "UNRECOGNIZED INPUT FORMAT";
  }


  // Try to open it.
  if (reader) {
    if (reader->open(name, beams, IFs, nChan, nPol, haveXPol, haveBase,
                     haveSpectra)) {
      format += " OPEN ERROR";
      delete reader;
    } else {
      return reader;
    }
  }

  return 0;
}


//--------------------------------------------------------------- getPKSreader

// Search a list of directories for a Parkes Multibeam dataset and return an
// appropriate PKSreader for it.

PKSreader* getPKSreader(
        const String name,
        const Vector<String> directories,
        const Int retry,
        const Int interpolate,
        Int    &iDir,
        String &format,
        Vector<Bool> &beams,
        Vector<Bool> &IFs,
        Vector<uInt> &nChan,
        Vector<uInt> &nPol,
        Vector<Bool> &haveXPol,
        Bool   &haveBase,
        Bool   &haveSpectra)
{
  Int nDir = directories.nelements();
  for (iDir = 0; iDir < nDir; iDir++) {
    String inName = directories(iDir) + "/" + name;
    PKSreader *reader = getPKSreader(inName, retry, interpolate, format,
                                     beams, IFs, nChan, nPol, haveXPol,
                                     haveBase, haveSpectra);
    if (reader != 0) {
      return reader;
    }
  }

  iDir = -1;
  return 0;
}

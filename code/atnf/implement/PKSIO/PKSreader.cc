//#---------------------------------------------------------------------------
//# PKSreader.cc: Class to read Parkes multibeam data.
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
        String &format)
{
  // Check accessibility of the input.
  File inFile(name);
  if (!inFile.exists()) {
    format = "DATASET NOT FOUND";
    return 0x0;
  }

  if (!inFile.isReadable()) {
    format = "DATASET UNREADABLE";
    return 0x0;
  }

  // Determine the type of input.
  PKSreader *reader = 0x0;
  if (inFile.isRegular()) {
    // Is it MBFITS or SDFITS?
    if (strstr(name.chars(), ".sdfits")) {
      // Looks like SDFITS, possibly gzip'd.
      format = "SDFITS";
      reader = new PKSFITSreader("SDFITS");

    } else {
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
  return reader;
}

//--------------------------------------------------------------- getPKSreader

// Search a list of directories for a Parkes Multibeam dataset and return an

PKSreader* getPKSreader(
        const String name,
        const Vector<String> directories,
        const Int retry,
        const Int interpolate,
        Int    &iDir,
        String &format)
{
  PKSreader *reader = 0x0;

  iDir = -1;
  Int nDir = directories.nelements();
  for (Int i = 0; i < nDir; i++) {
    String inName = directories(i) + "/" + name;
    reader = getPKSreader(inName, retry, interpolate, format);
    if (reader) {
      iDir = i;
      break;
    }
  }

  return reader;
}

//--------------------------------------------------------------- getPKSreader

// Open an appropriate PKSreader for a Parkes Multibeam dataset.

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
  PKSreader *reader = getPKSreader(name, retry, interpolate, format);

  // Try to open it.
  if (reader) {
    if (reader->open(name, beams, IFs, nChan, nPol, haveXPol, haveBase,
                     haveSpectra)) {
      format += " OPEN ERROR";
      delete reader;
      reader = 0x0;
    }
  }

  return reader;
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
  PKSreader *reader = getPKSreader(name, directories, retry, interpolate,
                                   iDir, format);

  // Try to open it.
  if (reader) {
    if (reader->open(name, beams, IFs, nChan, nPol, haveXPol, haveBase,
                     haveSpectra)) {
      format += " OPEN ERROR";
      delete reader;
      reader = 0x0;
    }
  }

  return reader;
}

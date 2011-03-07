//#---------------------------------------------------------------------------
//# PKSreader.cc: Class to read Parkes multibeam data.
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
//# $Id: PKSreader.cc,v 19.13 2009-09-29 07:33:39 cal103 Exp $
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
        file.seek(560);
        file.read(26, buf, False);
        buf[26] = '\0' ;
        if ( String(buf) == "ORIGIN  = 'NRAO Green Bank" ) {
          // Looks like GBT SDFITS
          format = "GBTFITS" ;
          reader = new PKSFITSreader("GBTFITS") ;
        }
        else {
          // Looks like SDFITS.
          format = "SDFITS";
          reader = new PKSFITSreader("SDFITS");
        }
       } else {
         // Assume it's MBFITS.
         format = "MBFITS";
         reader = new PKSFITSreader("MBFITS", retry, interpolate);
       }
    }

  } else if (inFile.isDirectory()) {
    Bool isMS = ( (File(name+"/table.info").exists()) 
                  && File(name+"/table.dat").exists() );
    if (isMS) {
      RegularFileIO ifs(name+"/table.info") ;
      char buf[128] ;
      ifs.read(sizeof(buf),buf,False) ;
      if ( strstr( buf, "Measurement Set" ) == NULL ) 
        isMS = False ;
    }
    //if (File(name + "/DATA_DESCRIPTION").exists()) {
    if (isMS) {
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
        const String antenna,
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
    if (reader->open(name, antenna, beams, IFs, nChan, nPol, haveXPol, 
                     haveBase, haveSpectra)) {
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
        const String antenna,
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
    if (reader->open(name, antenna, beams, IFs, nChan, nPol, haveXPol, 
                     haveBase, haveSpectra)) {
      format += " OPEN ERROR";
      delete reader;
      reader = 0x0;
    }
  }

  return reader;
}

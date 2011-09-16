//#---------------------------------------------------------------------------
//# STFITSImageWriter.h: ASAP class to write out single dish spectra as image FITS
//#---------------------------------------------------------------------------
//# Copyright (C) 2008
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: STFITSImageWriter.h 1106 2006-08-03 00:28:39Z mar637 $
//#---------------------------------------------------------------------------
#ifndef STFITSIMAGEWRITER_H
#define STFITSIMAGEWRITER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/ostream.h>

#include <coordinates/Coordinates/DirectionCoordinate.h>
#include "Scantable.h"
#include "Logger.h"

namespace asap {
/**
 * A class to export a Scantable to FITS file(s)
 */
class STFITSImageWriter : public Logger {
public:
// Constructor
  STFITSImageWriter();

// Destructor
  virtual ~STFITSImageWriter();

// Write out ascii table
  casa::Bool write(const Scantable& table, const casa::String& name);

  void setClass(casa::Bool flag)  { isClass_ = flag; }

private:
  casa::DirectionCoordinate getDirectionCoordinate(const casa::String& reff,
                                                   casa::Double lon, 
                                                   casa::Double lat);

  casa::Bool isClass_;
};

}// namespace
#endif

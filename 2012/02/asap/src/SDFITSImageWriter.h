//#---------------------------------------------------------------------------
//# SDFITSImageWriter.h: ASAP class to write out single dish spectra as image FITS
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
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
//# $Id: SDFITSImageWriter.h 894 2006-03-08 03:03:25Z mar637 $
//#---------------------------------------------------------------------------
#ifndef SDFITSIMAGEWRITER_H
#define SDFITSIMAGEWRITER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <SDMemTable.h>

#include "Logger.h"

namespace asap {

class SDFITSImageWriter : public Logger {
public:
  // Constructor
  SDFITSImageWriter();
  
  // Destructor
  ~SDFITSImageWriter();
  
  // Write out lots of FITS images, one per row, beam, IF and polarization
  casa::Bool write(const SDMemTable& table, const casa::String& rootName,
                   casa::Bool toStokes, casa::Bool verbose=casa::True);
  
private:
  casa::Int convertStokes(casa::Int val, casa::Bool toStokes,
			  casa::Bool linear) const;
};

}// namespace
#endif

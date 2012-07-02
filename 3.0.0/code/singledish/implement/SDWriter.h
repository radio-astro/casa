//#---------------------------------------------------------------------------
//# SDWriter.h: ASAP class to write out single dish spectra.
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
//# $Id$
//#---------------------------------------------------------------------------
#ifndef SDWRITER_H
#define SDWRITER_H

#include <string>

#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>

#include <singledish/SDMemTable.h>
#include <singledish/SDMemTableWrapper.h>

#include <singledish/SDLog.h>

class PKSwriter;

namespace asap {

class SDWriter : public SDLog {
public:
  SDWriter(const string &format = "SDFITS");
  ~SDWriter();

// Format can be "SDFITS", "FITS", "MS2" or "ASCII"
// Stokes conversion available for FITS and ASCII at present
  casa::Int setFormat(const string &format = "SDFITS");
  casa::Int write(const casa::CountedPtr<SDMemTable> table,
            const string &filename, casa::Bool toStokes);

private:
  std::string     cFormat;
  PKSwriter *cWriter;
};

}// namespace
#endif

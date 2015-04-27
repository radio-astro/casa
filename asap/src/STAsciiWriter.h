//#---------------------------------------------------------------------------
//# STAsciiWriter.h: ASAP class to write out single dish spectra as image FITS
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
//# $Id: STAsciiWriter.h 2658 2012-10-10 03:29:17Z MalteMarquarding $
//#---------------------------------------------------------------------------
#ifndef STASCIIWRITER_H
#define STASCIIWRITER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/ostream.h>

#include "Scantable.h"


namespace casa {
  template<class T> class Vector;
  class MDirection;
}

namespace asap {
/**
 * A class to export a Scnatble to ASCII file(s)
 */
class STAsciiWriter {
public:
// Constructor
  STAsciiWriter();

// Destructor
  virtual ~STAsciiWriter();

// Write out ascii table
  casa::Bool write(const Scantable& table, const casa::String& name);

private:
  casa::String formatDirection(const casa::MDirection& md) const;
  template <class T>
  void addLine(std::ostream& of, const casa::String& lbl,
               const T& value);
};

}// namespace
#endif

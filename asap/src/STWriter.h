//#---------------------------------------------------------------------------
//# STWriter.h: ASAP class to write out single dish spectra.
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
//# $Id: STWriter.h 1757 2010-06-09 09:03:06Z KanaSugimoto $
//#---------------------------------------------------------------------------
#ifndef STWRITER_H
#define STWRITER_H

#include <string>

#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/BasicSL/String.h>

#include "Logger.h"
#include "Scantable.h"

class PKSwriter;
class casa::Table;

namespace asap {
/**
  * This exports the ASAP internal data format to othe formats,
  * such as "SDFITS", "FITS", "MS2" or "ASCII"
  *
  * @brief Export of ASAP data container into foreign formats
  * @author Malte Marquarding
  * @date 2006-03-08
  * @version 2.0a
*/
class STWriter : public Logger {
public:
  explicit STWriter(const string &format = "SDFITS");
  virtual ~STWriter();

  /**
   * Set the format the data should be exported in
   * @param format an be "SDFITS", "FITS", "MS2" or "ASCII"
   * @return ststus code from PKSwriter
   */
  casa::Int setFormat(const string& format = "SDFITS");

  /**
   * Write the data to a file (directory)
   * @param table the Scantable object
   * @param filename the output file name
   * @return
   */
  casa::Int write(const casa::CountedPtr<Scantable> table,
            const string& filename);

private:
  casa::Vector<casa::Float> tsysFromTable(const casa::Table& tab);

  void polConversion( casa::Matrix<casa::Float>& spec,
                      casa::Matrix<casa::uChar>& flag,
                      casa::Vector<casa::Complex>& xpol,
                      const casa::Table& tab);

  void replacePtTab(const casa::Table& tab, const std::string& fname);

  casa::String getObsTypes( casa::Int srctype ) ;

  std::string     format_;
  PKSwriter* writer_;
};

}// namespace
#endif

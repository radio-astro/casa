//#---------------------------------------------------------------------------
//# SDReader.h: A class to read single dish spectra from SDFITS, RPFITS
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
//# $Id:
//#---------------------------------------------------------------------------
#ifndef SDREADER_H
#define SDREADER_H

#include <vector>
#include <string>

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

//#include "SDMemTable.h"
//#include "SDContainer.h"
//#include "SDLog.h"
#include <singledish/SDMemTable.h>
#include <singledish/SDContainer.h>
#include <singledish/SDLog.h>

class PKSreader;

namespace asap {

class SDReader : public SDLog {
public:
  SDReader();
  SDReader(const std::string& filename, 
           int whichIF=-1, int whichBeam=-1);
  SDReader(casa::CountedPtr<SDMemTable> tbl);
  virtual ~SDReader();

  void open(const std::string& filename,
            int whichIF=-1,
            int whichBeam=-1);
  void close();
  int read(const std::vector<int>& seq);

  casa::CountedPtr<SDMemTable> getTable() const { return table_;}

  void reset();

  std::vector<int> pseudoHeader() const {
    std::vector<int> v;
    v.push_back(nBeam_);v.push_back(nIF_);
    v.push_back(nPol_);v.push_back(nChan_);
    return v;
  }


protected:
  
private:
  casa::Int nBeam_,nIF_,nPol_,nChan_;
  PKSreader* reader_;  
  SDHeader* header_;
  SDFrequencyTable* frequencies_;
  casa::CountedPtr<SDMemTable> table_;
  casa::String filename_;
  casa::uInt cursor_;
  casa::Double timestamp_;
  casa::uInt beamOffset_, ifOffset_;
  casa::Bool haveXPol_;
};

}// namespace
#endif

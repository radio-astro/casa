//#---------------------------------------------------------------------------
//# STHeader.h: A container class for single dish integrations
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
//# $Id: STHeader.h 1603 2009-07-17 20:35:47Z TakTsutsumi $
//#---------------------------------------------------------------------------
#ifndef STHEADER_H
#define STHEADER_H

#include <vector>

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <measures/Measures/MDirection.h>

namespace casa {
  template<class T> class Matrix;
}

namespace asap {


struct STHeader {

  bool conformant(const STHeader& other);
  casa::String diff( const STHeader& other );


  casa::Int nchan;
  casa::Int npol;
  casa::Int nif;
  casa::Int nbeam;
  casa::String observer;
  casa::String project;
  casa::String obstype;
  casa::String antennaname;
  casa::Vector<casa::Double> antennaposition;
  casa::Float equinox;
  casa::String freqref;
  casa::Double reffreq;
  casa::Double bandwidth;
  casa::Double utc;
  casa::String fluxunit;
  casa::String epoch;
  casa::String poltype;
  void print() const ;
};

class SDDataDesc {

public:

  // Constructor
  SDDataDesc() : n_(0) {;}
  ~SDDataDesc() {;}

  // Add an entry if source name and Integer ID (can be anything you
  // like, such as FreqID) are unique.  You can add secondary entries
  // direction and another integer index which are just stored along
  // with the the primary entries
  casa::uInt addEntry(const casa::String& source, casa::uInt ID,
                      const casa::MDirection& secDir, casa::uInt secID);

  // Number of entries
  casa::Int length() const { return n_;}

  // Get attributes
  casa::String source(casa::uInt which) const {return source_[which];}
  casa::uInt ID(casa::uInt which) const {return ID_[which];}
  casa::uInt secID(casa::uInt which) const {return secID_[which];}
  casa::MDirection secDir(casa::uInt which) const {return secDir_[which];}

  // Summary
  void summary() const;

private:
  casa::uInt n_;
  casa::Vector<casa::String> source_;
  casa::Vector<casa::uInt> ID_, secID_;
  casa::Block<casa::MDirection> secDir_;

  SDDataDesc(const SDDataDesc& other);

};


} // namespace
#endif

//#---------------------------------------------------------------------------
//# python_SrcType.cc: python exposure of c++ atnf/PKSIO/SrcType class
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
//# $Id: python_SrcType.cpp 1677 2010-01-27 07:29:48Z TakeshiNakazato $
//#---------------------------------------------------------------------------
#include <vector>
#include <boost/python.hpp>
#include <boost/python/enum.hpp>

#include <atnf/PKSIO/SrcType.h>

using namespace boost::python;

//SrcType::type identity_( SrcType::type x ) { return x ; }

namespace asap {
  namespace python {
    void python_SrcType() {
      enum_<SrcType::type>("srctype")
        .value( "pson", SrcType::PSON )
        .value( "psoff", SrcType::PSOFF )
        .value( "nod", SrcType::NOD )
        .value( "fson", SrcType::FSON )
        .value( "fsoff", SrcType::FSOFF )
        .value( "sky", SrcType::SKY )
        .value( "hot", SrcType::HOT )
        .value( "warm", SrcType::WARM )
        .value( "cold", SrcType::COLD )
        .value( "poncal", SrcType::PONCAL )
        .value( "poffcal", SrcType::POFFCAL )
        .value( "nodcal", SrcType::NODCAL )
        .value( "foncal", SrcType::FONCAL )
        .value( "foffcal", SrcType::FOFFCAL )
        .value( "fslo", SrcType::FSLO )
        .value( "flooff", SrcType::FLOOFF )
        .value( "flosky", SrcType::FLOSKY )
        .value( "flohot", SrcType::FLOHOT )
        .value( "flowarm", SrcType::FLOWARM )
        .value( "flocold", SrcType::FLOCOLD )
        .value( "fshi", SrcType::FSHI )
        .value( "fhioff", SrcType::FHIOFF )
        .value( "fhisky", SrcType::FHISKY )
        .value( "fhihot", SrcType::FHIHOT )
        .value( "fhiwarm", SrcType::FHIWARM )
        .value( "fhicold", SrcType::FHICOLD )
        .value( "notype", SrcType::NOTYPE )
        ;

      //def( "identity", identity_ ) ;
    };

  } //namespace python
} // namespace asap


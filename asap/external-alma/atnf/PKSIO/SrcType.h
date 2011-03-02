//#---------------------------------------------------------------------------
//# SrcType.h: Class to define source type.
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
//# Original: 2010/02/08, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef ATNF_SRCTYPE_H
#define ATNF_SRCTYPE_H

//# Includes
#include <casa/aips.h>
#include <casa/BasicSL/String.h>

using namespace std ;
#include <casa/namespace.h>

// <sumamry>
// enum to define source type (scan intent)
//</summary>
class SrcType {
 public:
  enum type { PSON     = 0,
              PSOFF    = 1,
              NOD      = 2,
              FSON     = 3,
              FSOFF    = 4,
              SKY      = 6,
              HOT      = 7,
              WARM     = 8,
              COLD     = 9,
              PONCAL   = 10,
              POFFCAL  = 11,
              NODCAL   = 12,
              FONCAL   = 13,
              FOFFCAL  = 14,
              FSLO     = 20,
              FLOOFF   = 21,
              FLOSKY   = 26,
              FLOHOT   = 27,
              FLOWARM  = 28,
              FLOCOLD  = 29,
              FSHI     = 30,
              FHIOFF   = 31,
              FHISKY   = 36,
              FHIHOT   = 37,
              FHIWARM  = 38,
              FHICOLD  = 39,
              SIG      = 90,
              REF      = 91,
              CAL      = 92,
              NOTYPE   = 99 } ;

  static String getName(const Int srctype)
{
  String obsType ;
  switch( srctype ) {
  case Int(SrcType::PSON):
    obsType = "PSON" ;
    break ;
  case Int(SrcType::PSOFF):
    obsType = "PSOFF" ;
    break ;
  case Int(SrcType::NOD):
    obsType = "NOD" ;
    break ;
  case Int(SrcType::FSON):
    obsType = "FSON" ;
    break ;
  case Int(SrcType::FSOFF):
    obsType = "FSOFF" ;
    break ;
  case Int(SrcType::SKY):
    obsType = "SKY" ;
    break ;
  case Int(SrcType::HOT):
    obsType = "HOT" ;
    break ;
  case Int(SrcType::WARM):
    obsType = "WARM" ;
    break ;
  case Int(SrcType::COLD):
    obsType = "COLD" ;
    break ;
  case Int(SrcType::PONCAL):
    obsType = "PSON:CALON" ;
    break ;
  case Int(SrcType::POFFCAL):
    obsType = "PSOFF:CALON" ;
    break ;
  case Int(SrcType::NODCAL):
    obsType = "NOD:CALON" ;
    break ;
  case Int(SrcType::FONCAL):
    obsType = "FSON:CALON" ;
    break ;
  case Int(SrcType::FOFFCAL):
    obsType = "FSOFF:CALOFF" ;
    break ;
  case Int(SrcType::FSLO):
    obsType = "FSLO" ;
    break ;
  case Int(SrcType::FLOOFF):
    obsType = "FS:LOWER:OFF" ;
    break ;
  case Int(SrcType::FLOSKY):
    obsType = "FS:LOWER:SKY" ;
    break ;
  case Int(SrcType::FLOHOT):
    obsType = "FS:LOWER:HOT" ;
    break ;
  case Int(SrcType::FLOWARM):
    obsType = "FS:LOWER:WARM" ;
    break ;
  case Int(SrcType::FLOCOLD):
    obsType = "FS:LOWER:COLD" ;
    break ;
  case Int(SrcType::FSHI):
    obsType = "FSHI" ;
    break ;
  case Int(SrcType::FHIOFF):
    obsType = "FS:HIGHER:OFF" ;
    break ;
  case Int(SrcType::FHISKY):
    obsType = "FS:HIGHER:SKY" ;
    break ;
  case Int(SrcType::FHIHOT):
    obsType = "FS:HIGHER:HOT" ;
    break ;
  case Int(SrcType::FHIWARM):
    obsType = "FS:HIGHER:WARM" ;
    break ;
  case Int(SrcType::FHICOLD):
    obsType = "FS:HIGHER:COLD" ;
    break ;
  default:
    obsType = "NOTYPE" ;
  }

  return obsType ;
} ;

} ; 

#endif

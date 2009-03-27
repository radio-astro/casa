//#---------------------------------------------------------------------------
//# NRO45Header.h: Class for NRO 45m data header.
//#---------------------------------------------------------------------------
//# Copyright (C) 2000-2006
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
//# Original: 2008/10/30, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#ifndef NRO45_HEADER_H
#define NRO45_HEADER_H

#define NRO_ARYMAX 35

#include <atnf/PKSIO/NROHeader.h>

#include <string>

using namespace std ;

// <summary>
// Class specific for NRO 45m data header.
// </summary>

// 
// NRO45Header
//
// Class for NRO 45m data header.
//
class NRO45Header : public NROHeader
{
 public:
  // constructor
  NRO45Header() ;

  // destructor
  ~NRO45Header() ;

  // data initialization 
  void initialize() ;

  // data finalization
  void finalize() ;
  
  // fill header from file 
  int fill( FILE *fp, bool sameEndian ) ;
  int fill( string name ) ;
} ;


#endif /* NRO45_HEADER_H */

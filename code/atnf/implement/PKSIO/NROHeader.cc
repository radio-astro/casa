//#---------------------------------------------------------------------------
//# NROHeader.cc: Base class for NRO header data.
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

#include <atnf/PKSIO/NROHeader.h>

// 
// NROHeader
//
// Base class for NRO data header.
//

// constructor 
NROHeader::NROHeader() 
{
  // memory allocation
  initialize() ;
}

// destructor 
NROHeader::~NROHeader() 
{
  // release memory
  finalize() ;
}

// data initialization
void NROHeader::initialize()
{
  LOFIL = new char[8] ;
  VER = new char[8] ;
  GROUP = new char[16] ;
  PROJ = new char[16] ;
  SCHED = new char[24] ;
  OBSVR = new char[40] ;
  LOSTM = new char[16] ;
  LOETM = new char[16] ;
  TITLE = new char[120] ;
  OBJ = new char[16] ;
  EPOCH = new char[8] ;
  SCMOD = new char[120] ;
  VREF = new char[4] ;
  VDEF = new char[4] ;
  SWMOD = new char[8] ;
  CMTTM = new char[24] ;
  SITE = new char[8] ;

  datasize_ = sizeof( char ) * 8   // LOFIL
    + sizeof( char ) * 8           // VER
    + sizeof( char ) * 16          // GROUP
    + sizeof( char ) * 16          // PROJ
    + sizeof( char ) * 24          // SCHED
    + sizeof( char ) * 40          // OBSVR
    + sizeof( char ) * 16          // LOSTM
    + sizeof( char ) * 16          // LOETM
    + sizeof( int ) * 2            // ARYNM, NSCAN
    + sizeof( char ) * 120         // TITLE
    + sizeof( char ) * 16          // OBJ
    + sizeof( char ) * 8           // EPOCH
    + sizeof( double ) * 4         // RA0, DEC0, GLNG0, GLAT0
    + sizeof( int ) * 2            // NCALB, SCNCD
    + sizeof( char ) * 120         // SCMOD
    + sizeof( double )             // URVEL
    + sizeof( char ) * 4           // VREF
    + sizeof( char ) * 4           // VDEF
    + sizeof( char ) * 8           // SWMOD
    + sizeof( double ) * 8         // FRQSW, DBEAM, MLTOF, CMTQ, CMTE, CMTSOM, CMTNODE, CMTI
    + sizeof( char ) * 24          // CMTTM
    + sizeof( double ) * 6         // SBDX, SBDY, SBDZ1, SBDZ2, DAZP, DELP
    + sizeof( int ) * 4            // CHBIND, NUMCH, CHMIN, CHMAX
    + sizeof( double ) * 3         // ALCTM, IPTIM, PA
    + sizeof( int ) * 3            // SCNLEN, SBIND, IBIT
    + sizeof( char ) * 8 ;         // SITE
}

// finalization
void NROHeader::finalize() 
{
  delete LOFIL ;
  delete VER ;
  delete GROUP ;
  delete PROJ ;
  delete SCHED ;
  delete OBSVR ;
  delete LOSTM ;
  delete LOETM ;
  delete TITLE ;
  delete OBJ ;
  delete EPOCH ;
  delete SCMOD ;
  delete VREF ;
  delete VDEF ;
  delete SWMOD ;
  delete CMTTM ;
  delete SITE ;
}

void NROHeader::convertEndian( int &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( int ) ;
  std::reverse( first, last ) ;
}

void NROHeader::convertEndian( float &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( float ) ;
  std::reverse( first, last ) ;
}

void NROHeader::convertEndian( double &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( double ) ;
  std::reverse( first, last ) ;
}

int NROHeader::readChar( char *v, FILE *f, int size ) 
{
  if ( (int)( fread( v, 1, size, f ) ) != size ) {
    return -1 ;
  }
  return 0 ;
}

int NROHeader::readInt( int &v, FILE *f, bool b ) 
{
  if ( fread( &v, 1, sizeof(int), f ) != sizeof(int) ) {
    return -1 ;
  }

  if ( !b )
    convertEndian( v ) ;

  return 0 ;
}

int NROHeader::readFloat( float &v, FILE *f, bool b ) 
{
  if ( fread( &v, 1, sizeof(float), f ) != sizeof(float) ) {
    return -1 ;
  }

  if ( !b )
    convertEndian( v ) ;

  return 0 ;
}

int NROHeader::readDouble( double &v, FILE *f, bool b ) 
{
  if ( fread( &v, 1, sizeof(double), f ) != sizeof(double) ) {
    return -1 ;
  }

  if ( !b )
    convertEndian( v ) ;

  return 0 ;
}

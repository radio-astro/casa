//#---------------------------------------------------------------------------
//# ASTEDataset.cc: Class for ASTE dataset.
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

#include <atnf/PKSIO/ASTEDataset.h>

#include <iostream>
#include <cstring>

#define STRING2CHAR(s) const_cast<char *>((s).c_str())

using namespace std ;

// constructor 
ASTEDataset::ASTEDataset( string name )
  : NRODataset( name ) 
{}

// destructor 
ASTEDataset::~ASTEDataset() 
{}

// data initialization
void ASTEDataset::initialize()
{
  int arymax = arrayMax() ;

  // it must be called
  initializeCommon() ;

  // additional initialization
  datasize_ += sizeof( char ) * arymax * 16 // RX
    + sizeof( double ) * arymax * 6         // HPBW, EFFA, EFFB, EFFL, EFSS GAIN
    + sizeof( char ) * arymax * 4           // HORN
    + sizeof( char ) * arymax * 4           // POLTP
    + sizeof( double ) * arymax * 3         // POLDR, POLAN, DFRQ
    + sizeof( char ) * arymax * 4           // SIDBID
    + sizeof( int ) * arymax * 3            // REFN, IPINT, MULTN
    + sizeof( double ) * arymax             // MLTSCF
    + sizeof( char ) * arymax * 8           // LAGWIND
    + sizeof( double ) * arymax * 3         // BEBW, BERES, CHWID
    + sizeof( int ) * arymax * 2            // ARRY, NFCAL
    + sizeof( double ) * arymax             // F0CAL
    + sizeof( double ) * arymax * 10 * 3    // FQCAL, CHCAL, CWCAL
    + sizeof( char ) * 116                       // CDMY1
    + sizeof( double ) * arymax ;           // DSBFC
}

int ASTEDataset::fillHeader( int sameEndian )
{
  LogIO os( LogOrigin( "ASTEDataset", "fillHeader()", WHERE ) ) ;

  int arymax = arrayMax();

  // make sure file pointer points a beginning of the file
  fseek( fp_, 0, SEEK_SET ) ;

  fillHeaderCommon( sameEndian ) ;

  // specific part
  CDMY1.resize(116);
  if ( readHeader( STRING2CHAR(CDMY1), 116 ) == -1 ) {
    os << LogIO::WARN << "Error while reading data CDMY1." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CDMY1 = " << CDMY1 << endl ;
  //
  for ( int i = 0 ; i < arymax ; i++ ) {
    if ( readHeader( DSBFC[i], sameEndian ) == -1 ) {
      os << LogIO::WARN << "Error while reading data DSBFC[" << i << "]." << LogIO::POST ;
      return -1 ;
    }
  }
  // DEBUG
//   nro_debug_output( "DSBFC", arymax, DSBFC ) ;
  // 

  return 0 ;
}

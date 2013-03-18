//#---------------------------------------------------------------------------
//# NROOTFDataset.cc: Class for NRO 45m OTF dataset.
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
//# Original: 2009/02/27, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/NROOTFDataset.h>

#include <iostream>
#include <cstring>

#define STRING2CHAR(s) const_cast<char *>((s).c_str())

using namespace std ;

// constructor 
NROOTFDataset::NROOTFDataset( string name )
  : NRODataset( name ) 
{}

// destructor 
NROOTFDataset::~NROOTFDataset() 
{}

// data initialization
void NROOTFDataset::initialize()
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
    + sizeof( char ) * 180 ;                    // CDMY1

  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    DSBFC[i] = 1.0 ;
  }
}

int NROOTFDataset::fillHeader( int sameEndian )
{
  LogIO os( LogOrigin( "NROOTFDataset", "fillHeader()", WHERE ) ) ;

  // make sure file pointer points a beginning of the file
  fseek( fp_, 0, SEEK_SET ) ;

  fillHeaderCommon( sameEndian ) ;

  // specific part
  CDMY1.resize(180);
  if ( readHeader( STRING2CHAR(CDMY1), 180 ) == -1 ) {
    os << LogIO::WARN << "Error while reading data CDMY1." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CDMY1 = " << CDMY1 << endl ;
  //

  return 0 ;
}


//#---------------------------------------------------------------------------
//# NRO45Reader.cc: Class to read NRO 45m OTF data.
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
//# Original: 2008/11/04, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/NRO45Reader.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
NRO45Reader::NRO45Reader( string name ) 
  : NROReader( name )
{
  // DEBUG
  cout << "NRO45Reader::NRO45Reader()" << endl ;
  //
}

// Destructor.
NRO45Reader::~NRO45Reader() 
{
}
  
// Read data header
Int NRO45Reader::readHeader() 
{
  // DEBUG
  //cout << "NRO45Reader::readHeader()" << endl ;
  //
  int status = 0 ;

  // check endian
  fseek( fp_, 144, SEEK_SET ) ;
  int tmp ;
  if( fread( &tmp, 1, sizeof(int), fp_ ) != sizeof(int) ) {
    cerr << "Error while checking endian of the file. " << endl ;
    return -1 ;
  }
  if ( ( 0 < tmp ) && ( tmp <= NRO_ARYMAX ) ) {
    same_ = true ;
    cout << "NRO45Reader::read()  same endian " << endl ;
  }
  else {
    same_ = false ;
    cout << "NRO45Reader::read()  different endian " << endl ;
  }
  fseek( fp_, 0, SEEK_SET ) ;

  // create NRO45Header
  header_ = new NRO45Header() ;

  // fill NRO45Header
  status = header_->fill( fp_, same_ ) ;

  if ( status == -1 ) {
    cerr << "Failed to fill data header." << endl ;
    scanNum_ = 0 ;
    scanLen_ = 0 ;
  }
  else {
    scanNum_ = header_->getNSCAN() + 1 ; // includes ZERO scan
    scanLen_ = header_->getSCNLEN() ;
    rowNum_ = scanNum_ * header_->getARYNM() ;
    chmax_ = (int) ( scanLen_ - SCAN_HEADER_SIZE ) * 8 / header_->getIBIT() ;
    data_->LDATA = new char[scanLen_-SCAN_HEADER_SIZE] ;
    cout << "NRO45Reader::readHeader()  Number of scan        = " << scanNum_ << endl ;
    cout << "NRO45Reader::readHeader()  Number of data record = " << rowNum_ << endl ;
    cout << "NRO45Reader::readHeader()  Length of data record = " << scanLen_ << " byte" << endl ;
    cout << "NRO45Reader::readHeader()  Max number of channel = " << chmax_ << endl ;
    cout << "NRO45Reader::readHeader()  allocated memory for spectral data: " << scanLen_-SCAN_HEADER_SIZE << " bytes" << endl ;
  }

  return status ;
}

vector<double> NRO45Reader::getAntennaPosition() 
{
  // NOBEYAMA in ITRF2005
  // Obtained from ITRF website http://itrf.ensg.ign.fr/
  //vector<double> pos( 3 ) ;
  //pos[0] = -3871169.229 ;
  //pos[1] = 3428274.975 ;
  //pos[2] = 3723698.519 ;

  // NOBEYAMA in World Geodetic System
  // 
  // E138d28m21.2s N35d56m40.9s 1350m in old Japanese Geodetic System 
  // (from http://www.nro.nao.ac.jp/Misc/hist_NRO.html)
  // 
  // E138d28m09.96444s N35d56m52.3314s 1350m in World Geodetic System
  // (conversion is done by http://vldb.gsi.go.jp/sokuchi/tky2jgd/)
  
  double elon = 138. + 28. / 60. + 9.96444 / 3600. ;
  double nlat = 35. + 56. / 60. + 52.3314 / 3600. ;
  double alti = 1350. ;

  MPosition p( MVPosition( Quantity( alti, "m" ),
                           Quantity( elon, "deg" ),
                           Quantity( nlat, "deg" ) ),
               MPosition::Ref( MPosition::WGS84 ) ) ;
  MeasFrame frame( p ) ;
  MVPosition mvp ;
  frame.getITRF( mvp ) ;
  Vector<Double> pp = mvp.getValue() ;
  vector<double> pos ;
  pp.tovector( pos ) ;
  //cout << "NRO45Reader::getAntennaPosition()  pp[0] = " << pp[0]
  //     << " pp[1] = " << pp[1] << " pp[2] = " << pp[2] << endl ;

  return pos ;
}


//#---------------------------------------------------------------------------
//# ASTEFXReader.cc: Class to read ASTE-FX data.
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
//# Original: 2008/11/07, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/ASTEFXReader.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
ASTEFXReader::ASTEFXReader( string name ) 
  : NROReader( name )
{
  // read Header
  readHeader() ;
  // allocate memory for spectral data
  data_->LDATA = new char[scanLen_-SCAN_HEADER_SIZE] ;
  // DEBUG
  cout << "ASTEFXReader::ASTEFXReader()  allocated memory for spectral data: " << scanLen_-SCAN_HEADER_SIZE << " bytes" << endl ;
  //
}

// Destructor.
ASTEFXReader::~ASTEFXReader() 
{
}
  
// Read data header
Int ASTEFXReader::readHeader() 
{
  int status = 0 ;

  // check endian
  fseek( fp_, 144, SEEK_SET ) ;
  int tmp ;
  if( fread( &tmp, 1, sizeof(int), fp_ ) != sizeof(int) ) {
    cerr << "Error while checking endian of the file. " << endl ;
    return -1 ;
  }
  if ( ( 0 < tmp ) && ( tmp <= ASTE_ARYMAX_FX ) ) {
    same_ = true ;
  }
  else {
    same_ = false ;
  }
  fseek( fp_, 0, SEEK_SET ) ;

  // create ASTEFXHeader
  header_ = new ASTEFXHeader() ;

  // fill ASTEFXHeader
  status = header_->fill( fp_, same_ ) ;

  if ( status == -1 ) {
    cerr << "Failed to fill data header." << endl ;
    scanNum_ = 0 ;
    scanLen_ = 0 ;
  }
  else {
    scanNum_ = header_->getNSCAN() + 1 ;   // includes ZERO scan
    scanLen_ = header_->getSCNLEN() ;
    rowNum_ = scanNum_ * header_->getARYNM() ;
    chmax_ = (int) ( scanLen_ - SCAN_HEADER_SIZE ) * 8 / header_->getIBIT() ;
    cout << "ASTEFXReader::readHeader()  Number of scan        = " << scanNum_ << endl ;
    cout << "ASTEFXReader::readHeader()  Number of data record = " << rowNum_ << endl ;
    cout << "ASTEFXReader::readHeader()  Length of data record = " << scanLen_ << " byte" << endl ;
    cout << "ASTEFXReader::readHeader()  Max number of channel = " << chmax_ << endl ;
  }

  return status ;
}

vector<double> ASTEFXReader::getAntennaPosition() 
{
  // ASTE in ITRF2005
  // tentative 
  //vector<double> pos( 3 ) ;
  //pos[0] = 2412830.391  ;
  //pos[1] = -5271936.712 ;
  //pos[2] = -2652209.088 ;

  // ASTE in World Geodetic System
  // 
  // W67d42m11s S22d58m18s 2400m 
  // Ref.: Ezawa, H. et al. 2004, Proc. SPIE, 5489, 763
  //
  double elon = -67. - 42. / 60. - 11. / 3600. ;
  double nlat = -22. - 58. / 60. - 18. / 3600. ;
  double alti = 4800. ;

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
  //cout << "ASTEFXReader::getAntennaPosition()  pp[0] = " << pp[0]
  //     << " pp[1] = " << pp[1] << " pp[2] = " << pp[2] << endl ;

  return pos ;
}


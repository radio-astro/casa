//#---------------------------------------------------------------------------
//# NRO45Reader.cc: Class to read NRO 45m data.
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
  // open file if not opened yet
  if ( fp_ == NULL ) {
    status = open() ;
    if ( status == -1 ) {
      cerr << "Error opening file " << filename_ << "." << endl ;
      cerr << "Failed to read data header." << endl ;
      return status ;
    }
  }

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
    //rowNum_ = scanNum_  ;
    cout << "NRO45Reader::readHeader()  Number of scan        = " << scanNum_ << endl ;
    cout << "NRO45Reader::readHeader()  Number of data record = " << rowNum_ << endl ;
    cout << "NRO45Reader::readHeader()  Length of data record = " << scanLen_ << " byte" << endl ;
  }

  return status ;
}

vector<double> NRO45Reader::getAntennaPosition() 
{
  // NOBEYAMA in ITRF2005
  // Obtained from ITRF website http://itrf.ensg.ign.fr/
  vector<double> pos( 3 ) ;
  pos[0] = -3871169.229 ;
  pos[1] = 3428274.975 ;
  pos[2] = 3723698.519 ;

  return pos ;
}


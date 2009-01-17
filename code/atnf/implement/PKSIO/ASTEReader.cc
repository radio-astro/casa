//#---------------------------------------------------------------------------
//# ASTEReader.cc: Class to read ASTE data.
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

#include <atnf/PKSIO/ASTEReader.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
ASTEReader::ASTEReader( string name ) 
  : NROReader( name )
{
}

// Destructor.
ASTEReader::~ASTEReader() 
{
}
  
// Read data header
Int ASTEReader::readHeader() 
{
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
  if ( ( 0 < tmp ) && ( tmp <= ASTE_ARYMAX ) ) {
    same_ = true ;
  }
  else {
    same_ = false ;
  }
  fseek( fp_, 0, SEEK_SET ) ;

  // create ASTEHeader
  header_ = new ASTEHeader() ;

  // fill ASTEHeader
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
    cout << "ASTEReader::readHeader()  Number of scan        = " << scanNum_ << endl ;
    cout << "ASTEReader::readHeader()  Number of data record = " << rowNum_ << endl ;
    cout << "ASTEReader::readHeader()  Length of data record = " << scanLen_ << " byte" << endl ;
  }

  return status ;
}

vector<double> ASTEReader::getAntennaPosition() 
{
  // ASTE in ITRF2005
  // no information 
  vector<double> pos( 3 ) ;
  pos[0] = 0.0 ;
  pos[1] = 0.0 ;
  pos[2] = 0.0 ;

  return pos ;
}


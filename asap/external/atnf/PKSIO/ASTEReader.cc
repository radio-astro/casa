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
#include <atnf/PKSIO/ASTEDataset.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
ASTEReader::ASTEReader( string name ) 
  : NROReader( name )
{
  // DEBUG
  //cout << "ASTEReader::ASTEReader()" << endl ;
  //
}

// Destructor.
ASTEReader::~ASTEReader() 
{
}
  
// Read data header
Int ASTEReader::read() 
{
  LogIO os( LogOrigin( "ASTEReader", "read()", WHERE ) ) ;

  int status = 0 ;

  // create ASTEDataset
  dataset_ = new ASTEDataset( filename_ ) ;

  // fill ASTEDataset
  status = dataset_->fillHeader() ;

  if ( status != 0 ) {
    os << LogIO::SEVERE << "Failed to fill data header." << LogIO::EXCEPTION ;
  }

  return status ;
}

vector<double> ASTEReader::getAntennaPosition() 
{
  // ASTE in ITRF2005
  // tentative
  //vector<double> pos( 3 ) ;
  //pos[0] = 2412830.391  ;
  //pos[1] = -5271936.712 ;
  //pos[2] = -2652209.088 ;


  // ASTE in World Geodetic System
  // 
  // W67d42m11s S22d58m18s 4800m 
  // Ref.: Ezawa, H. et al. 2004, Proc. SPIE, 5489, 763
  //
  double elon = -67. - 42. / 60. - 11. / 3600. ;
  double nlat = -22. - 58. / 60. - 18. / 3600. ;
  double alti = 2400. ;

  // APEX value
  //double elon = -67.7592 ;
  //double nlat = -23.0057 ;
  //double alti = 5105. ;

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
  //cout << "ASTEReader::getAntennaPosition()  pp[0] = " << pp[0]
  //     << " pp[1] = " << pp[1] << " pp[2] = " << pp[2] << endl ;


  return pos ;
}


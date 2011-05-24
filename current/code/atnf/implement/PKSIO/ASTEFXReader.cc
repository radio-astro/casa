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
#include <atnf/PKSIO/ASTEFXDataset.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
ASTEFXReader::ASTEFXReader( string name ) 
  : ASTEReader( name )
{
  // DEBUG
  //cout << "ASTEFXReader::ASTEFXReader()" << endl ;
  //
}

// Destructor.
ASTEFXReader::~ASTEFXReader() 
{
}
  
// Read data header
Int ASTEFXReader::read() 
{
  LogIO os( LogOrigin( "ASTEFXReader", "read()", WHERE ) ) ;

  int status = 0 ;

  // create ASTEFXDataset
  dataset_ = new ASTEFXDataset( filename_ ) ;

  // fill ASTEFXDataset
  status = dataset_->fillHeader() ;

  if ( status != 0 ) {
    //cerr << "Failed to fill data header." << endl ;
    os << LogIO::SEVERE << "Failed to fill data header." << LogIO::EXCEPTION ;
  }

  return status ;
}


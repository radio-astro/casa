//#---------------------------------------------------------------------------
//# NRO45FITSReader.cc: Class to read NRO 45m FITS data.
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
//# Original: 2009/02/26, Takeshi Nakazato, NAOJ
//#---------------------------------------------------------------------------

#include <atnf/PKSIO/NRO45FITSReader.h>
#include <atnf/PKSIO/NROFITSDataset.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
NRO45FITSReader::NRO45FITSReader( string name ) 
  : NRO45Reader( name )
{
  // DEBUG
  cout << "NRO45FITSReader::NRO45FITSReader()" << endl ;
  //
}

// Destructor.
NRO45FITSReader::~NRO45FITSReader() 
{
}
  
// Read data header
Int NRO45FITSReader::read() 
{
  // DEBUG
  //cout << "NRO45FITSReader::read()" << endl ;
  //
  int status = 0 ;

  // create NRO45Dataset
  dataset_ = new NROFITSDataset( filename_ ) ;

  // fill NROFITSDataset
  status = dataset_->fillHeader() ;

  if ( status != 0 ) {
    cerr << "Failed to fill data header." << endl ;
  }

  return status ;
}

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
#include <atnf/PKSIO/NROOTFDataset.h>

#include <string>
#include <stdio.h>

using namespace std ;

// Constructor 
NRO45Reader::NRO45Reader( string name ) 
  : NROReader( name )
{
  // DEBUG
  //cout << "NRO45Reader::NRO45Reader()" << endl ;
  //
}

// Destructor.
NRO45Reader::~NRO45Reader() 
{
}
  
void NRO45Reader::initDataset()
{
  dataset_ = new NROOTFDataset( filename_ ) ;
  dataset_->initialize() ;
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
  
//   double elon = 138. + 28. / 60. + 9.96444 / 3600. ;
//   double nlat = 35. + 56. / 60. + 52.3314 / 3600. ;
//   double alti = 1350. ;

//   Double elon = 138.4725 ;
//   Double nlat = 35.9445 ;
//   Double alti = 1412.599 ;

//   MPosition p( MVPosition( Quantity( alti, "m" ),
//                            Quantity( elon, "deg" ),
//                            Quantity( nlat, "deg" ) ),
//                MPosition::Ref( MPosition::WGS84 ) ) ;

  Double posx = -3.8710235e6 ;
  Double posy = 3.4281068e6 ;
  Double posz = 3.7240395e6 ;
  MPosition p( MVPosition( posx, posy, posz ),
               MPosition::ITRF ) ;

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

Int NRO45Reader::getNumIF() 
{
  Int nif = 1 ;
  if ( (dataset_->getRX()[0]).find("MULT2-") == string::npos ) {
    vector<Bool> v ;
    vector<int> arry = dataset_->getARRY() ;
    for ( uInt i = 0 ; i < arry.size() ; i++ ) {
      if ( arry[i] != 0 ) {
        v.push_back( True ) ;
      }
    }
    nif = v.size() ;
  }
  return nif ;
}

Int NRO45Reader::getNumBeam()
{
  Int nbeam = 25 ;
  if ( (dataset_->getRX()[0]).find("MULT2-") == string::npos ) {
    nbeam = 1 ;
  }
  return nbeam ;
}

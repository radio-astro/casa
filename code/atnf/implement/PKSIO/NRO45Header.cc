//#---------------------------------------------------------------------------
//# NRO45Header.cc: Class for NRO 45m data header.
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

#include <atnf/PKSIO/NRO45Header.h>

#include <iostream>

using namespace std ;

// 
// NRO45Header
//
// Class for NRO 45m data header.
//

// constructor 
NRO45Header::NRO45Header() 
{
  // memory allocation
  initialize() ;

  // data initialization
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    DSBFC[i] = 1.0 ;
  }
}

// destructor 
NRO45Header::~NRO45Header() 
{
  // release memory
  finalize() ;
}

// data initialization
void NRO45Header::initialize()
{
  RX.resize( NRO_ARYMAX ) ;
  HPBW.resize( NRO_ARYMAX ) ;
  EFFA.resize( NRO_ARYMAX ) ;
  EFFB.resize( NRO_ARYMAX ) ;
  EFFL.resize( NRO_ARYMAX ) ;
  EFSS.resize( NRO_ARYMAX ) ;
  GAIN.resize( NRO_ARYMAX ) ;
  HORN.resize( NRO_ARYMAX ) ;
  POLTP.resize( NRO_ARYMAX ) ;
  POLDR.resize( NRO_ARYMAX ) ;
  POLAN.resize( NRO_ARYMAX ) ;
  DFRQ.resize( NRO_ARYMAX ) ;
  SIDBD.resize( NRO_ARYMAX ) ;
  REFN.resize( NRO_ARYMAX ) ;
  IPINT.resize( NRO_ARYMAX ) ;
  MULTN.resize( NRO_ARYMAX ) ;
  MLTSCF.resize( NRO_ARYMAX ) ;
  LAGWIND.resize( NRO_ARYMAX ) ;
  BEBW.resize( NRO_ARYMAX ) ;
  BERES.resize( NRO_ARYMAX ) ;
  CHWID.resize( NRO_ARYMAX ) ;
  ARRY.resize( NRO_ARYMAX ) ;
  NFCAL.resize( NRO_ARYMAX ) ;
  F0CAL.resize( NRO_ARYMAX ) ;
  FQCAL.resize( NRO_ARYMAX ) ;
  CHCAL.resize( NRO_ARYMAX ) ;
  CWCAL.resize( NRO_ARYMAX ) ;
  CDMY1 = new char[180] ;
  DSBFC.resize( NRO_ARYMAX ) ;

  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    RX[i] = new char[16] ;
    HORN[i] = new char[4] ;
    POLTP[i] = new char[4] ;
    SIDBD[i] = new char[4] ;
    LAGWIND[i] = new char[8] ;
    FQCAL[i].resize( 10 ) ;
    CHCAL[i].resize( 10 ) ;
    CWCAL[i].resize( 10 ) ;
  }

  datasize_ += sizeof( char ) * NRO_ARYMAX * 16 // RX
    + sizeof( double ) * NRO_ARYMAX * 6         // HPBW, EFFA, EFFB, EFFL, EFSS GAIN
    + sizeof( char ) * NRO_ARYMAX * 4           // HORN
    + sizeof( char ) * NRO_ARYMAX * 4           // POLTP
    + sizeof( double ) * NRO_ARYMAX * 3         // POLDR, POLAN, DFRQ
    + sizeof( char ) * NRO_ARYMAX * 4           // SIDBID
    + sizeof( int ) * NRO_ARYMAX * 3            // REFN, IPINT, MULTN
    + sizeof( double ) * NRO_ARYMAX             // MLTSCF
    + sizeof( char ) * NRO_ARYMAX * 8           // LAGWIND
    + sizeof( double ) * NRO_ARYMAX * 3         // BEBW, BERES, CHWID
    + sizeof( int ) * NRO_ARYMAX * 2            // ARRY, NFCAL
    + sizeof( double ) * NRO_ARYMAX             // F0CAL
    + sizeof( double ) * NRO_ARYMAX * 10 * 3    // FQCAL, CHCAL, CWCAL
    + sizeof( char ) * 180 ;                    // CDMY1
}

// finalization
void NRO45Header::finalize() 
{
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    delete RX[i] ;
    delete HORN[i] ;
    delete POLTP[i] ;
    delete SIDBD[i];
    delete LAGWIND[i] ;
  }
  delete CDMY1 ;
}

// fill data header
int NRO45Header::fill( string name ) 
{
  // open file
  FILE *fp ;
  if ( ( fp = fopen( name.c_str(), "rb" ) ) == NULL ) {
    cerr << "Error opening file " << name << "." << endl ;
    return -1 ;
  }

  // check endian
  bool same ;
  fseek( fp, 144, SEEK_SET ) ;
  int tmp ;
  if( fread( &tmp, 1, sizeof(int), fp ) != sizeof(int) ) {
    cerr << "Error while checking endian of the file. " << endl ;
    return -1 ;
  }
  if ( ( 0 < tmp ) && ( tmp <= NRO_ARYMAX ) ) {
    same = true ;
    cout << "NRO45Header::fill()  same endian " << endl ;
  }
  else {
    same = false ;
    cout << "NRO45Header::fill()  different endian " << endl ;
  }
  fseek( fp, 0, SEEK_SET ) ;
 
  // fill
  int status = fill( fp, same ) ;

  return status ;
}

int NRO45Header::fill( FILE *fp, bool sameEndian )
{
  // make sure file pointer points a beginning of the file
  fseek( fp, 0, SEEK_SET ) ;

  // read data header
  if ( readChar( LOFIL, fp,  8 ) == -1 ) {
    cerr << "Error while reading data LOFIL." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "LOFIL = " << LOFIL << endl ;
  //
  if ( readChar( VER, fp, 8 ) == -1 ) {
    cerr << "Error while reading data VER." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VER = " << VER << endl ;
  //
  if ( readChar( GROUP, fp, 16 ) == -1 ) {
    cerr << "Error while reading data GROUP." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GROUP = " << GROUP << endl ;
  //
  if ( readChar( PROJ, fp, 16 ) == -1 ) {
    cerr << "Error while reading data PROJ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "PROJ = " << PROJ << endl ;
  //
  if ( readChar( SCHED, fp, 24 ) == -1 ) {
    cerr << "Error while reading data SCHED." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCHED = " << SCHED << endl ;
  //
  if ( readChar( OBSVR, fp, 40 ) == -1 ) {
    cerr << "Error while reading data OBSVR." << endl ;
    return -1 ;
  }  
  // DEBUG
  //cout << "OBSVR = " << OBSVR << endl ;
  //
  if ( readChar( LOSTM, fp, 16 ) == -1 ) {
    cerr << "Error while reading data LOSTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "LOSTM = " << LOSTM << endl ;
  //
  if ( readChar( LOETM, fp, 16 ) == -1 ) {
    cerr << "Error while reading data LOETM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "LOETM = " << LOETM << endl ;
  //
  if ( readInt( ARYNM, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data ARYNM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "ARYNM = " << ARYNM << endl ;
  //
  if ( readInt( NSCAN, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data NSCAN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NSCAN = " << NSCAN << endl ;
  //
  if ( readChar( TITLE, fp, 120 ) == -1 ) {
    cerr << "Error while reading data TITLE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "TITLE = " << TITLE << endl ;
  //
  if ( readChar( OBJ, fp, 16 ) == -1 ) {
    cerr << "Error while reading data OBJ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "OBJ = " << OBJ << endl ;
  //
  if ( readChar( EPOCH, fp, 8 ) == -1 ) {
    cerr << "Error while reading data EPOCH." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "EPOCH = " << EPOCH << endl ;
  //
  if ( readDouble( RA0, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data RA0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "RA0 = " << RA0 << endl ;
  //
  if ( readDouble( DEC0, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data DEC0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DEC0 = " << DEC0 << endl ;
  //
  if ( readDouble( GLNG0, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data GLNG0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLNG0 = " << GLNG0 << endl ;
  //
  if ( readDouble( GLAT0, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data GLAT0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLAT0 = " << GLAT0 << endl ;
  //
  if ( readInt( NCALB, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data NCALB." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NCALB = " << NCALB << endl ;
  //
  if ( readInt( SCNCD, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SCNCD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNCD = " << SCNCD << endl ;
  //
  if ( readChar( SCMOD, fp, 120 ) == -1 ) {
    cerr << "Error while reading data SCMOD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCMOD = " << SCMOD << endl ;
  //
  if ( readDouble( URVEL, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data URVEL." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "URVEL = " << URVEL << endl ;
  //
  if ( readChar( VREF, fp, 4 ) == -1 ) {
    cerr << "Error while reading data VREF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VREF = " << VREF << endl ;
  //
  if ( readChar( VDEF, fp, 4 ) == -1 ) {
    cerr << "Error while reading data VDEF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VDEF = " << VDEF << endl ;
  //
  if ( readChar( SWMOD, fp, 8 ) == -1 ) {
    cerr << "Error while reading data SWMOD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SWMOD = " << SWMOD << endl ;
  //
  if ( readDouble( FRQSW, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data FRQSW." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "FRQSW = " << FRQSW << endl ;
  //
  if ( readDouble( DBEAM, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data DBEAM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DBEAM = " << DBEAM << endl ;
  //
  if ( readDouble( MLTOF, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data MLTOF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "MLTOF = " << MLTOF << endl ;
  //
  if ( readDouble( CMTQ, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTQ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTQ = " << CMTQ << endl ;
  //
  if ( readDouble( CMTE, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTE = " << CMTE << endl ;
  //
  if ( readDouble( CMTSOM, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTSOM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTSOM = " << CMTSOM << endl ;
  //
  if ( readDouble( CMTNODE, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTNODE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTNODE = " << CMTNODE << endl ;
  //
  if ( readDouble( CMTI, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTI." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTI = " << CMTI << endl ;
  //
  if ( readChar( CMTTM, fp, 24 ) == -1 ) {
    cerr << "Error while reading data CMTTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTTM = " << CMTTM << endl ;
  //
  if ( readDouble( SBDX, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDX." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDX = " << SBDX << endl ;
  //
  if ( readDouble( SBDY, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDY." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDY = " << SBDY << endl ;
  //
  if ( readDouble( SBDZ1, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDZ1." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ1 = " << SBDZ1 << endl ;
  //
  if ( readDouble( SBDZ2, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDZ2." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ2 = " << SBDZ2 << endl ;
  //
  if ( readDouble( DAZP, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data DAZP." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DAZP = " << DAZP << endl ;
  //
  if ( readDouble( DELP, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data DELP." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DELP = " << DELP << endl ;
  //
  if ( readInt( CHBIND, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CHBIND." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CHBIND = " << CHBIND << endl ;
  //
  if ( readInt( NUMCH, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data NUMCH." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NUMCH = " << NUMCH << endl ;
  //
  if ( readInt( CHMIN, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CHMIN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CHMIN = " << CHMIN << endl ;
  //
  if ( readInt( CHMAX, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data CHMAX." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CHMAX = " << CHMAX << endl ;
  //
  if ( readDouble( ALCTM, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data ALCTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "ALCTM = " << ALCTM << endl ;
  //
  if ( readDouble( IPTIM, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data IPTIM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "IPTIM = " << IPTIM << endl ;
  //
  if ( readDouble( PA, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data PA." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "PA = " << PA << endl ;
  //
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    if ( readChar( RX[i], fp, 16 ) == -1 ) {
      cerr << "Error while reading data RX[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "RX      " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << RX[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    if ( readDouble( HPBW[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data HPBW[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "HPBW    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << HPBW[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    if ( readDouble( EFFA[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data EFFA[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "EFFA    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << EFFA[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    if ( readDouble( EFFB[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data EFFB[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "EFFB    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << EFFB[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    if ( readDouble( EFFL[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data EFFL[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "EFFL    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     } 
//     cout << EFFL[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++ ) {
    if ( readDouble( EFSS[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data EFSS[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "EFSS    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << EFSS[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( GAIN[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data GAIN[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "GAIN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << GAIN[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readChar( HORN[i], fp, 4 ) == -1 ) {
      cerr << "Error while reading data HORN[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "HORN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << HORN[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readChar( POLTP[i], fp, 4 ) == -1 ) {
      cerr << "Error while reading data POLTP[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "POLTP   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << POLTP[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( POLDR[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data POLDR[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "POLDR   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << POLDR[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( POLAN[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data POLAN[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "POLAN   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << POLAN[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( DFRQ[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data DFRQ[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "DFRQ    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << DFRQ[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readChar( SIDBD[i], fp, 4 ) == -1 ) {
      cerr << "Error while reading data SIDBD[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "SIDBD   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << SIDBD[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readInt( REFN[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data REFN[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "REFN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << REFN[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readInt( IPINT[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data IPINT[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "IPINT   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << IPINT[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readInt( MULTN[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data MULTN[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "MULTN   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << MULTN[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( MLTSCF[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data MLTSCF[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "MLTSCF  " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << MLTSCF[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readChar( LAGWIND[i], fp, 8 ) == -1 ) {
      cerr << "Error while reading data LAGWIND[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "LAGWIND " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << LAGWIND[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( BEBW[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data BEBW[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "BEBW    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << BEBW[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( BERES[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data BERES[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "BERES   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << BERES[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( CHWID[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data CHWID[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "CHWID   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << CHWID[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readInt( ARRY[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data ARRY[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "ARRY    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << ARRY[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readInt( NFCAL[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data NFCAL[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "NFCAL   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << NFCAL[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    if ( readDouble( F0CAL[i], fp, sameEndian ) == -1 ) {
      cerr << "Error while reading data F0CAL[" << i << "]." << endl ;
      return -1 ;
    }
    // DEBUG
//     if ( i == 0 ) {
//       cout << "F0CAL   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << F0CAL[i] << " " ;
    //
  }
  //cout << endl ;
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    for ( int j = 0 ; j < 10 ; j++ ) {
      if ( readDouble( FQCAL[i][j], fp, sameEndian ) == -1 ) {
        cerr << "Error while reading data FQCAL[" << i << "][" << j << "]." << endl ;
        return -1 ;
      }
      // DEBUG
//       if ( j == 0 ) {
//         if ( i < 10 ) 
//           cout << "FQCAL0" << i << " " ;
//         else 
//           cout << "FQCAL" << i << " " ;
//       }
//       else if ( ( j % 5 ) == 0 ) {
//         cout << endl << "        " ;
//       }
//       cout << FQCAL[i][j] << " " ;
      //
    }
    //cout << endl ;
  }
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    for ( int j = 0 ; j < 10 ; j++ ) {
      if ( readDouble( CHCAL[i][j], fp, sameEndian ) == -1 ) {
        cerr << "Error while reading data CHCAL[" << i << "][" << j << "]." << endl ;
        return -1 ;
      }
     // DEBUG
//       if ( j == 0 ) {
//         if ( i < 10 ) 
//           cout << "CHCAL0" << i << " " ;
//         else 
//           cout << "CHCAL" << i << " " ;
//       }
//       else if ( ( j % 5 ) == 0 ) {
//         cout << endl << "        " ;
//       }
//       cout << CHCAL[i][j] << " " ;
     //
    }
    //cout << endl ;
  }
  for ( int i = 0 ; i < NRO_ARYMAX ; i++) {
    for ( int j = 0 ; j < 10 ; j++ ) {
      if ( readDouble( CWCAL[i][j], fp, sameEndian ) == -1 ) {
        cerr << "Error while reading data CWCAL[" << i << "][" << j << "]." << endl ;
        return -1 ;
      }
      // DEBUG
//       if ( j == 0 ) {
//         if ( i < 10 ) 
//           cout << "CWCAL0" << i << " " ;
//         else 
//           cout << "CWCAL" << i << " " ;
//       }
//       else if ( ( j % 5 ) == 0 ) {
//         cout << endl << "        " ;
//       }
//       cout << CWCAL[i][j] << " " ;
      //
    }
    //cout << endl ;
  }
  if ( readInt( SCNLEN, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SCNLEN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNLEN = " << SCNLEN << endl ;
  //
  if ( readInt( SBIND, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBIND." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBIND = " << SBIND << endl ;
  //
  if ( readInt( IBIT, fp, sameEndian ) == -1 ) {
    cerr << "Error while reading data IBIT." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "IBIT = " << IBIT << endl ;
  //
  if ( readChar( SITE, fp, 8 ) == -1 ) {
    cerr << "Error while reading data SITE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SITE = " << SITE << endl ;
  //
  if ( readChar( CDMY1, fp, 180 ) == -1 ) {
    cerr << "Error while reading data CDMY1." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CDMY1 = " << CDMY1 << endl ;
  //

  return 0 ;
}


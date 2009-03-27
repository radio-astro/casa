//#---------------------------------------------------------------------------
//# ASTEFXDataset.cc: Class for ASTE-FX dataset.
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

#include <atnf/PKSIO/ASTEFXDataset.h>

#include <iostream>

using namespace std ;

// constructor 
ASTEFXDataset::ASTEFXDataset( string name )
  : NRODataset( name ) 
{
  // check endian
  open() ;
  fseek( fp_, 144, SEEK_SET ) ;
  int tmp ;
  if( fread( &tmp, 1, sizeof(int), fp_ ) != sizeof(int) ) {
    cerr << "Error while checking endian of the file. " << endl ;
    return ;
  }
  if ( ( 0 < tmp ) && ( tmp <= ASTE_ARYMAX_FX ) ) {
    same_ = true ;
    cout << "ASTEFXDataset::ASTEFXDataset()  same endian " << endl ;
  }
  else {
    same_ = false ;
    cout << "ASTEFXDataset::ASTEFXDataset()  different endian " << endl ;
  }
  fseek( fp_, 0, SEEK_SET ) ;

  // memory allocation
  initialize() ;
}

// destructor 
ASTEFXDataset::~ASTEFXDataset() 
{
  // release memory
  finalize() ;
}

// data initialization
void ASTEFXDataset::initialize()
{
  RX.resize( ASTE_ARYMAX_FX ) ;
  HPBW.resize( ASTE_ARYMAX_FX ) ;
  EFFA.resize( ASTE_ARYMAX_FX ) ;
  EFFB.resize( ASTE_ARYMAX_FX ) ;
  EFFL.resize( ASTE_ARYMAX_FX ) ;
  EFSS.resize( ASTE_ARYMAX_FX ) ;
  GAIN.resize( ASTE_ARYMAX_FX ) ;
  HORN.resize( ASTE_ARYMAX_FX ) ;
  POLTP.resize( ASTE_ARYMAX_FX ) ;
  POLDR.resize( ASTE_ARYMAX_FX ) ;
  POLAN.resize( ASTE_ARYMAX_FX ) ;
  DFRQ.resize( ASTE_ARYMAX_FX ) ;
  SIDBD.resize( ASTE_ARYMAX_FX ) ;
  REFN.resize( ASTE_ARYMAX_FX ) ;
  IPINT.resize( ASTE_ARYMAX_FX ) ;
  MULTN.resize( ASTE_ARYMAX_FX ) ;
  MLTSCF.resize( ASTE_ARYMAX_FX ) ;
  LAGWIND.resize( ASTE_ARYMAX_FX ) ;
  BEBW.resize( ASTE_ARYMAX_FX ) ;
  BERES.resize( ASTE_ARYMAX_FX ) ;
  CHWID.resize( ASTE_ARYMAX_FX ) ;
  ARRY.resize( ASTE_ARYMAX_FX ) ;
  NFCAL.resize( ASTE_ARYMAX_FX ) ;
  F0CAL.resize( ASTE_ARYMAX_FX ) ;
  FQCAL.resize( ASTE_ARYMAX_FX ) ;
  CHCAL.resize( ASTE_ARYMAX_FX ) ;
  CWCAL.resize( ASTE_ARYMAX_FX ) ;
  CDMY1 = new char[116] ;   // 84?
  DSBFC.resize( ASTE_ARYMAX_FX ) ;

  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    RX[i] = new char[16] ;
    HORN[i] = new char[4] ;
    POLTP[i] = new char[4] ;
    SIDBD[i] = new char[4] ;
    LAGWIND[i] = new char[8] ;
    FQCAL[i].resize( 10 ) ;
    CHCAL[i].resize( 10 ) ;
    CWCAL[i].resize( 10 ) ;
  }

  datasize_ += sizeof( char ) * ASTE_ARYMAX_FX * 16 // RX
    + sizeof( double ) * ASTE_ARYMAX_FX * 6         // HPBW, EFFA, EFFB, EFFL, EFSS GAIN
    + sizeof( char ) * ASTE_ARYMAX_FX * 4           // HORN
    + sizeof( char ) * ASTE_ARYMAX_FX * 4           // POLTP
    + sizeof( double ) * ASTE_ARYMAX_FX * 3         // POLDR, POLAN, DFRQ
    + sizeof( char ) * ASTE_ARYMAX_FX * 4           // SIDBID
    + sizeof( int ) * ASTE_ARYMAX_FX * 3            // REFN, IPINT, MULTN
    + sizeof( double ) * ASTE_ARYMAX_FX             // MLTSCF
    + sizeof( char ) * ASTE_ARYMAX_FX * 8           // LAGWIND
    + sizeof( double ) * ASTE_ARYMAX_FX * 3         // BEBW, BERES, CHWID
    + sizeof( int ) * ASTE_ARYMAX_FX * 2            // ARRY, NFCAL
    + sizeof( double ) * ASTE_ARYMAX_FX             // F0CAL
    + sizeof( double ) * ASTE_ARYMAX_FX * 10 * 3    // FQCAL, CHCAL, CWCAL
    + sizeof( char ) * 116                          // CDMY1
    + sizeof( double ) * ASTE_ARYMAX_FX ;           // DSBFC
}

// finalization
void ASTEFXDataset::finalize() 
{
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    delete RX[i] ;
    delete HORN[i] ;
    delete POLTP[i] ;
    delete SIDBD[i];
    delete LAGWIND[i] ;
  }

  delete CDMY1 ;
}

// fill data header
int ASTEFXDataset::fillHeader() 
{
  // open file
  if ( open() ) {
    cerr << "Error opening file " << filename_ << "." << endl ;
    return -1 ;
  }

  // fill
  int status = fillHeader( same_ ) ;

  return status ;
}

int ASTEFXDataset::fillHeader( int sameEndian )
{
  // make sure file pointer points a beginning of the file
  fseek( fp_, 0, SEEK_SET ) ;

  // read data header
  if ( readHeader( LOFIL, 8 ) == -1 ) {
    cerr << "Error while reading data LOFIL." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "LOFIL = " << LOFIL << endl ;
  //
  if ( readHeader( VER, 8 ) == -1 ) {
    cerr << "Error while reading data VER." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VER = " << VER << endl ;
  //
  if ( readHeader( GROUP, 16 ) == -1 ) {
    cerr << "Error while reading data GROUP." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GROUP = " << GROUP << endl ;
  //
  if ( readHeader( PROJ, 16 ) == -1 ) {
    cerr << "Error while reading data PROJ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "PROJ = " << PROJ << endl ;
  //
  if ( readHeader( SCHED, 24 ) == -1 ) {
    cerr << "Error while reading data SCHED." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCHED = " << SCHED << endl ;
  //
  if ( readHeader( OBSVR, 40 ) == -1 ) {
    cerr << "Error while reading data OBSVR." << endl ;
    return -1 ;
  }  
  // DEBUG
  //cout << "OBSVR = " << OBSVR << endl ;
  //
  if ( readHeader( LOSTM, 16 ) == -1 ) {
    cerr << "Error while reading data LOSTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "LOSTM = " << LOSTM << endl ;
  //
  if ( readHeader( LOETM, 16 ) == -1 ) {
    cerr << "Error while reading data LOETM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "LOETM = " << LOETM << endl ;
  //
  if ( readHeader( ARYNM, sameEndian ) == -1 ) {
    cerr << "Error while reading data ARYNM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "ARYNM = " << ARYNM << endl ;
  //
  if ( readHeader( NSCAN, sameEndian ) == -1 ) {
    cerr << "Error while reading data NSCAN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NSCAN = " << NSCAN << endl ;
  //
  if ( readHeader( TITLE, 120 ) == -1 ) {
    cerr << "Error while reading data TITLE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "TITLE = " << TITLE << endl ;
  //
  if ( readHeader( OBJ, 16 ) == -1 ) {
    cerr << "Error while reading data OBJ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "OBJ = " << OBJ << endl ;
  //
  if ( readHeader( EPOCH, 8 ) == -1 ) {
    cerr << "Error while reading data EPOCH." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "EPOCH = " << EPOCH << endl ;
  //
  if ( readHeader( RA0, sameEndian ) == -1 ) {
    cerr << "Error while reading data RA0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "RA0 = " << RA0 << endl ;
  //
  if ( readHeader( DEC0, sameEndian ) == -1 ) {
    cerr << "Error while reading data DEC0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DEC0 = " << DEC0 << endl ;
  //
  if ( readHeader( GLNG0, sameEndian ) == -1 ) {
    cerr << "Error while reading data GLNG0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLNG0 = " << GLNG0 << endl ;
  //
  if ( readHeader( GLAT0, sameEndian ) == -1 ) {
    cerr << "Error while reading data GLAT0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLAT0 = " << GLAT0 << endl ;
  //
  if ( readHeader( NCALB, sameEndian ) == -1 ) {
    cerr << "Error while reading data NCALB." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NCALB = " << NCALB << endl ;
  //
  if ( readHeader( SCNCD, sameEndian ) == -1 ) {
    cerr << "Error while reading data SCNCD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNCD = " << SCNCD << endl ;
  //
  if ( readHeader( SCMOD, 120 ) == -1 ) {
    cerr << "Error while reading data SCMOD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCMOD = " << SCMOD << endl ;
  //
  if ( readHeader( URVEL, sameEndian ) == -1 ) {
    cerr << "Error while reading data URVEL." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "URVEL = " << URVEL << endl ;
  //
  if ( readHeader( VREF, 4 ) == -1 ) {
    cerr << "Error while reading data VREF." << endl ;
    return -1 ;
  }
   // DEBUG
  //cout << "VREF = " << VREF << endl ;
  //
 if ( readHeader( VDEF, 4 ) == -1 ) {
    cerr << "Error while reading data VDEF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VDEF = " << VDEF << endl ;
  //
  if ( readHeader( SWMOD, 8 ) == -1 ) {
    cerr << "Error while reading data SWMOD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SWMOD = " << SWMOD << endl ;
  //
  if ( readHeader( FRQSW, sameEndian ) == -1 ) {
    cerr << "Error while reading data FRQSW." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "FRQSW = " << FRQSW << endl ;
  //
  if ( readHeader( DBEAM, sameEndian ) == -1 ) {
    cerr << "Error while reading data DBEAM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DBEAM = " << DBEAM << endl ;
  //
  if ( readHeader( MLTOF, sameEndian ) == -1 ) {
    cerr << "Error while reading data MLTOF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "MLTOF = " << MLTOF << endl ;
  //
  if ( readHeader( CMTQ, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTQ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTQ = " << CMTQ << endl ;
  //
  if ( readHeader( CMTE, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTE = " << CMTE << endl ;
  //
  if ( readHeader( CMTSOM, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTSOM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTSOM = " << CMTSOM << endl ;
  //
  if ( readHeader( CMTNODE, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTNODE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTNODE = " << CMTNODE << endl ;
  //
  if ( readHeader( CMTI, sameEndian ) == -1 ) {
    cerr << "Error while reading data CMTI." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTI = " << CMTI << endl ;
  //
  if ( readHeader( CMTTM, 24 ) == -1 ) {
    cerr << "Error while reading data CMTTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTTM = " << CMTTM << endl ;
  //
  if ( readHeader( SBDX, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDX." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDX = " << SBDX << endl ;
  //
  if ( readHeader( SBDY, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDY." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDY = " << SBDY << endl ;
  //
  if ( readHeader( SBDZ1, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDZ1." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ1 = " << SBDZ1 << endl ;
  //
  if ( readHeader( SBDZ2, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBDZ2." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ2 = " << SBDZ2 << endl ;
  //
  if ( readHeader( DAZP, sameEndian ) == -1 ) {
    cerr << "Error while reading data DAZP." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DAZP = " << DAZP << endl ;
  //
  if ( readHeader( DELP, sameEndian ) == -1 ) {
    cerr << "Error while reading data DELP." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DELP = " << DELP << endl ;
  //
  if ( readHeader( CHBIND, sameEndian ) == -1 ) {
    cerr << "Error while reading data CHBIND." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CHBIND = " << CHBIND << endl ;
  //
  if ( readHeader( NUMCH, sameEndian ) == -1 ) {
    cerr << "Error while reading data NUMCH." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NUMCH = " << NUMCH << endl ;
  //
  if ( readHeader( CHMIN, sameEndian ) == -1 ) {
    cerr << "Error while reading data CHMIN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CHMIN = " << CHMIN << endl ;
  //
  if ( readHeader( CHMAX, sameEndian ) == -1 ) {
    cerr << "Error while reading data CHMAX." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CHMAX = " << CHMAX << endl ;
  //
  if ( readHeader( ALCTM, sameEndian ) == -1 ) {
    cerr << "Error while reading data ALCTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "ALCTM = " << ALCTM << endl ;
  //
  if ( readHeader( IPTIM, sameEndian ) == -1 ) {
    cerr << "Error while reading data IPTIM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "IPTIM = " << IPTIM << endl ;
  //
  if ( readHeader( PA, sameEndian ) == -1 ) {
    cerr << "Error while reading data PA." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "PA = " << PA << endl ;
  //
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( RX[i], 16 ) == -1 ) {
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
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( HPBW[i], sameEndian ) == -1 ) {
      cerr << "Error while reading data HPBW[" << i << "]." << endl ;
      return -1 ;
    }
//     // DEBUG
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
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( EFFA[i], sameEndian ) == -1 ) {
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
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( EFFB[i], sameEndian ) == -1 ) {
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
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( EFFL[i], sameEndian ) == -1 ) {
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
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( EFSS[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( GAIN[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( HORN[i], 4 ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( POLTP[i], 4 ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( POLDR[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( POLAN[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( DFRQ[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( SIDBD[i], 4 ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( REFN[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( IPINT[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( MULTN[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( MLTSCF[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( LAGWIND[i], 8 ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( BEBW[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( BERES[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( CHWID[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( ARRY[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( NFCAL[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    if ( readHeader( F0CAL[i], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    for ( int j = 0 ; j < 10 ; j++ ) {
      if ( readHeader( FQCAL[i][j], sameEndian ) == -1 ) {
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
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    for ( int j = 0 ; j < 10 ; j++ ) {
      if ( readHeader( CHCAL[i][j], sameEndian ) == -1 ) {
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
//      cout << CHCAL[i][j] << " " ;
     //
    }
    //cout << endl ;
  }
  for ( int i= 0 ; i < ASTE_ARYMAX_FX ; i++) {
    for ( int j = 0 ; j < 10 ; j++ ) {
      if ( readHeader( CWCAL[i][j], sameEndian ) == -1 ) {
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
  if ( readHeader( SCNLEN, sameEndian ) == -1 ) {
    cerr << "Error while reading data SCNLEN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNLEN = " << SCNLEN << endl ;
  //
  if ( readHeader( SBIND, sameEndian ) == -1 ) {
    cerr << "Error while reading data SBIND." << endl ;
    return -1 ;
  }
   // DEBUG
  //cout << "SBIND = " << SBIND << endl ;
  //
 if ( readHeader( IBIT, sameEndian ) == -1 ) {
    cerr << "Error while reading data IBIT." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "IBIT = " << IBIT << endl ;
  //
  if ( readHeader( SITE, 8 ) == -1 ) {
    cerr << "Error while reading data SITE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SITE = " << SITE << endl ;
  //
  if ( readHeader( CDMY1, 116 ) == -1 ) {
    cerr << "Error while reading data CDMY1." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CDMY1 = " << CDMY1 << endl ;
  //
  for ( int i = 0 ; i < ASTE_ARYMAX_FX ; i++ ) {
    if ( readHeader( DSBFC[i], sameEndian ) == -1 ) {
      cerr << "Error while reading data DSBFC[" << i << "]." << endl ;
      return -1 ;
    }
     // DEBUG
//     if ( i == 0 ) {
//       cout << "DSBFC   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << DSBFC[i] << " " ;
    //
  }
  //cout << endl ;

  scanNum_ = NSCAN + 1 ; // includes ZERO scan
  rowNum_ = scanNum_ * ARYNM ;
  scanLen_ = SCNLEN ;
  dataLen_ = scanLen_ - SCAN_HEADER_SIZE ;
  chmax_ = (int) ( dataLen_ * 8 / IBIT ) ;
  record_->LDATA = new char[dataLen_] ;

  // DEBUG
  cout << "ASTEFXDataset::fillHeader()  Number of scan        = " << scanNum_ << endl ;
  cout << "ASTEFXDataset::fillHeader()  Number of data record = " << rowNum_ << endl ;
  cout << "ASTEFXDataset::fillHeader()  Length of data record = " << scanLen_ << " byte" << endl ;
  cout << "ASTEFXDataset::fillHeader()  allocated memory for spectral data: " << dataLen_ << " bytes" << endl ;
  cout << "ASTEFXDataset::fillHeader()  Max number of channel: " << chmax_ << endl ;
  //

  return 0 ;
}

//#---------------------------------------------------------------------------
//# NROFITSDataset.cc: Class for NRO 45m FITS dataset.
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

#include <atnf/PKSIO/NROFITSDataset.h>

#include <iostream>
#include <math.h>

using namespace std ;

//header size (8*2880bytes)
#define FITS_HEADER_SIZE 23040

// constructor 
NROFITSDataset::NROFITSDataset( string name )
  : NRODataset( name ) 
{
  fp_ = NULL ;
  dataid_ = -1 ;
  record_ = new NRODataRecord() ;

  // check endian
  // FITS file is always BIG_ENDIAN
  if ( endian_ == BIG_ENDIAN ) {
    same_ = 1 ;
    cout << "NROFITSDataset::NROFITSDataset()  same endian " << endl ;
  }
  else {
    same_ = 0 ;
    cout << "NROFITSDataset::NROFITSDataset()  different endian " << endl ;
  }

  // open file
  if ( open() ) 
    cerr << "NROFITSDataset::initialize()  error while opening file " << filename_ << endl ;
 
  // memory allocation
  initialize() ;

  readHeader( numField_, "TFIELDS", same_ ) ;
  forms_.resize( numField_ ) ;
  names_.resize( numField_ ) ;
  units_.resize( numField_ ) ;

  // data initialization
  getField() ;

  // DEBUG
  cout << "NROFITSDataset::NROFITSDataset()  Number of scan        = " << scanNum_ << endl ;
  cout << "NROFITSDataset::NROFITSDataset()  Number of data record = " << rowNum_ << endl ;
  cout << "NROFITSDataset::NROFITSDataset()  Length of data record = " << scanLen_ << " byte" << endl ;
  cout << "NROFITSDataset::NROFITSDataset()  allocated memory for spectral data: " << dataLen_ << " bytes" << endl ;
  cout << "NROFITSDataset::NROFITSDataset()  Max number of channel: " << chmax_ << endl ;
  //
}

// destructor 
NROFITSDataset::~NROFITSDataset() 
{
  // close file
  close() ;
  // release memory
  finalize() ;
}

// data initialization
void NROFITSDataset::initialize()
{
  int status = 0 ;
  status = readHeader( ARYNM, "ARYNM", same_ ) ;
  if ( status != 0 ) 
    ARYNM = 1 ;
  readHeader( rowNum_, "NAXIS2", same_ ) ;
  readHeader( scanLen_, "NAXIS1", same_ ) ;
  status = 0 ;
  scanNum_ = rowNum_ / ARYNM ;
  chmax_ = 2048 ;
  datasize_ = sizeof( int ) * chmax_ ;
  record_->JDATA.resize( chmax_ ) ;

  RX.resize( ARYNM ) ;
  HPBW.resize( ARYNM ) ;
  EFFA.resize( ARYNM ) ;
  EFFB.resize( ARYNM ) ;
  EFFL.resize( ARYNM ) ;
  EFSS.resize( ARYNM ) ;
  GAIN.resize( ARYNM ) ;
  HORN.resize( ARYNM ) ;
  POLTP.resize( ARYNM ) ;
  POLDR.resize( ARYNM ) ;
  POLAN.resize( ARYNM ) ;
  DFRQ.resize( ARYNM ) ;
  SIDBD.resize( ARYNM ) ;
  REFN.resize( ARYNM ) ;
  IPINT.resize( ARYNM ) ;
  MULTN.resize( ARYNM ) ;
  MLTSCF.resize( ARYNM ) ;
  LAGWIND.resize( ARYNM ) ;
  BEBW.resize( ARYNM ) ;
  BERES.resize( ARYNM ) ;
  CHWID.resize( ARYNM ) ;
  ARRY.resize( NRO_FITS_ARYMAX ) ;  
  //ARRY.resize( ARYNM ) ;
  NFCAL.resize( ARYNM ) ;
  F0CAL.resize( ARYNM ) ;
  FQCAL.resize( ARYNM ) ;
  CHCAL.resize( ARYNM ) ;
  CWCAL.resize( ARYNM ) ;
  //CDMY1 = new char[180] ;
  DSBFC.resize( ARYNM ) ;

  ARYTP.resize( ARYNM ) ;
  arrayid_.resize( ARYNM ) ;
  for ( int i = 0 ; i < ARYNM ; i++ ) 
    arrayid_[i] = -1 ;

  for ( int i = 0 ; i < ARYNM ; i++ ) {
    RX[i] = new char[16] ;
    HORN[i] = new char[4] ;
    POLTP[i] = new char[4] ;
    SIDBD[i] = new char[4] ;
    LAGWIND[i] = new char[8] ;
    FQCAL[i].resize( 10 ) ;
    CHCAL[i].resize( 10 ) ;
    CWCAL[i].resize( 10 ) ;
  }

  datasize_ += sizeof( char ) * ARYNM * 16 // RX
    + sizeof( double ) * ARYNM * 6         // HPBW, EFFA, EFFB, EFFL, EFSS GAIN
    + sizeof( char ) * ARYNM * 4           // HORN
    + sizeof( char ) * ARYNM * 4           // POLTP
    + sizeof( double ) * ARYNM * 3         // POLDR, POLAN, DFRQ
    + sizeof( char ) * ARYNM * 4           // SIDBID
    + sizeof( int ) * ARYNM * 3            // REFN, IPINT, MULTN
    + sizeof( double ) * ARYNM             // MLTSCF
    + sizeof( char ) * ARYNM * 8           // LAGWIND
    + sizeof( double ) * ARYNM * 3         // BEBW, BERES, CHWID
    + sizeof( int ) * NRO_FITS_ARYMAX      // ARRY
    + sizeof( int ) * ARYNM                // NFCAL
    + sizeof( double ) * ARYNM             // F0CAL
    + sizeof( double ) * ARYNM * 10 * 3    // FQCAL, CHCAL, CWCAL
    + sizeof( char ) * 180 ;               // CDMY1
}

// finalization
void NROFITSDataset::finalize() 
{
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    delete RX[i] ;
    delete HORN[i] ;
    delete POLTP[i] ;
    delete SIDBD[i];
    delete LAGWIND[i] ;
  }
  //delete CDMY1 ;
}

// fill data header
int NROFITSDataset::fillHeader() 
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

int NROFITSDataset::fillHeader( int sameEndian )
{
  // fill array type
  fillARYTP() ;

  // read data header
  float ftmp = 0.0 ;
  //int itmp = 0 ;
  //double dtmp = 0.0 ;
  if ( readHeader( LOFIL, "LOFIL" ) != 0 ) {
    //cerr << "Info: LOFIL set to FITS." << endl ;
    strcpy( LOFIL, "FITS" ) ; 
  }
  // DEBUG
  //cout << "LOFIL = " << LOFIL << endl ;
  //
  if ( readHeader( VER, "VER" ) != 0 ) {
    if ( readHeader( VER, "HISTORY NEWSTAR VER" ) != 0 ) {
      //cerr << "Info: VER set to V000." << endl ;
      strcpy( VER, "V000" ) ;
    }
  }
  // DEBUG
  //cout << "VER = " << VER << endl ;
  //
  if ( readHeader( GROUP, "GROUP" ) != 0 ) {
    if ( readHeader( GROUP, "HISTORY NEWSTAR GROUP" ) != 0 ) {
      //cerr << "Info: GROUP set to GRP0." << endl ;
      strcpy( GROUP, "GROUP0" ) ;
    }
  }
  // DEBUG
  //cout << "GROUP = " << GROUP << endl ;
  //
  if ( readHeader( PROJ, "PROJECT" ) != 0 ) {
    if ( readHeader( PROJ, "HISTORY NEWSTAR PROJECT" ) != 0 ) {
      //cerr << "Info: PROJ set to PROJ0." << endl ;
      strcpy( PROJ, "PROJECT0" ) ;
    }
  }
  // DEBUG
  //cout << "PROJ = " << PROJ << endl ;
  //
  if ( readHeader( SCHED, "SCHED" ) != 0 ) {
    if ( readHeader( SCHED, "HISTORY NEWSTAR SCHED" ) != 0 ) {
      //cerr << "Info: SCHED set to SCHED0." << endl ;
      strcpy( SCHED, "SCHED0" ) ;
    }
  }
  // DEBUG
  //cout << "SCHED = " << SCHED << endl ;
  //
  if ( readHeader( OBSVR, "OBSERVER" ) != 0 ) {
    //cerr << "Error while reading data OBSVR" << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "OBSVR = " << OBSVR << endl ;
  //
  char ctmp1[18] ;
  if ( readHeader( ctmp1, "STRSC" ) != 0 ) {
    //cerr << "Error while reading data LOSTM." << endl ;
    return -1 ;
  }
  if ( ctmp1[0] == '9' ) {
    strncpy( LOSTM, "19", 2 ) ;
    strncpy( LOSTM+2, ctmp1, 12 ) ;
    LOSTM[14] = '\0' ;
  }
  else if ( ctmp1[0] == '0') {
    strncpy( LOSTM, "20", 2 ) ;
    strncpy( LOSTM+2, ctmp1, 12 ) ;
    LOSTM[14] = '\0' ;
  } 
  // DEBUG
  //cout << "LOSTM = " << LOSTM << endl ;
  //
  if ( readHeader( ctmp1, "STPSC" ) != 0 ) {
    //cerr << "Error while reading data LOETM." << endl ;
    return -1 ;
  }
  if ( ctmp1[0] == '9' ) {
    strncpy( LOETM, "19", 2 ) ;
    strncpy( LOETM+2, ctmp1, 12 ) ;
    //LOETM[14] = '\0' ;
  }
  else if ( ctmp1[0] == '0') {
    strncpy( LOETM, "20", 2 ) ;
    strncpy( LOETM+2, ctmp1, 12 ) ;
    //LOETM[14] = '\0' ;
  }   
  // DEBUG
  //cout << "LOETM = " << LOETM << endl ;
  //
  //if ( readHeader( ARYNM, "ARYNM", sameEndian ) != 0 ) {
  //cerr << "Error while reading data ARYNM." << endl ;
  //return -1 ;
  //}
  // DEBUG
  //cout << "ARYNM = " << ARYNM << endl ;
  //
  if ( readHeader( NSCAN, "NAXIS2", sameEndian ) != 0 ) {
    //cerr << "Error while reading data NSCAN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NSCAN = " << NSCAN << endl ;
  //
  char ctmp[256] ;
  if ( readHeader( TITLE, "TITLE" ) != 0 ) {
    int stat1 = readHeader( TITLE, "HISTORY NEWSTAR TITLE1" ) ;
    int stat2 = readHeader( ctmp, "HISTORY NEWSTAR TITLE2" ) ;
    if ( stat1 != 0 && stat2 != 0 ) {
      //cerr << "Info: TITLE set to NOTITLE." << endl ;
      strcpy( TITLE, "NOTITLE" ) ;
    }
    else {
      //cout << "TITLE = " << TITLE << endl ;
      //cout << "ctmp  = " << ctmp << endl ;
      strcat( TITLE, ctmp ) ;
    }
  }
  // DEBUG
  //cout << "TITLE = " << TITLE << endl ;
  //
  if ( readHeader( OBJ, "OBJECT" ) != 0 ) {
    //cerr << "Error while reading data OBJ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "OBJ = " << OBJ << endl ;
  //
  if ( readHeader( ftmp, "EPOCH", sameEndian ) != 0 ) {
    //cerr << "Error while reading data EPOCH." << endl ;
    return -1 ;
  }
  if ( ftmp == 1950.0 ) 
    strcpy( EPOCH, "B1950" ) ;
  else if ( ftmp == 2000.0 ) 
    strcpy( EPOCH, "J2000" ) ;
  else 
    strcpy( EPOCH, "XXXXX" ) ;
  // DEBUG
  //cout << "EPOCH = " << EPOCH << endl ;
  //
  if ( readHeader( ctmp, "RA" ) != 0 ) {
    //cerr << "Error while reading data RA0." << endl ;
    return -1 ;
  }
  RA0 = radRA( ctmp ) ;
  // DEBUG
  //cout << "RA0 = " << RA0 << endl ;
  //
  if ( readHeader( ctmp, "DEC" ) != 0 ) {
    //cerr << "Error while reading data DEC0." << endl ;
    return -1 ;
  }
  DEC0 = radDEC( ctmp ) ;
  // DEBUG
  //cout << "DEC0 = " << DEC0 << endl ;
  //
  if ( readHeader( GLNG0, "GL0", sameEndian ) != 0 ) {
    //cerr << "Error while reading data GLNG0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLNG0 = " << GLNG0 << endl ;
  //
  if ( readHeader( GLAT0, "GB0", sameEndian ) != 0 ) {
    //cerr << "Error while reading data GLAT0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLAT0 = " << GLAT0 << endl ;
  //
  if ( readHeader( NCALB, "NCALB", sameEndian ) != 0 ) {
    //cerr << "Error while reading data NCALB." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NCALB = " << NCALB << endl ;
  //
  if ( readHeader( SCNCD, "SCNCD", sameEndian ) != 0 ) {
    //cerr << "Info: SCNCD set to 0 (RADEC)." << endl ;
    SCNCD = 0 ;
  }
  // DEBUG
  //cout << "SCNCD = " << SCNCD << endl ;
  //
  if ( readHeader( SCMOD, "SCMOD1" ) != 0 ) {
    //cerr << "Error while reading data SCMOD." << endl ;
    return -1 ;
  }
  char *pos ;
  if ( ( pos = strstr( SCMOD, " " ) ) != NULL ) 
    strncpy( pos+1, "\0", 1 ) ;
  if ( readHeader( ctmp, "SCMOD2" ) == 0 && strncmp( ctmp, " ", 1 ) != 0 ) {
    strcat( SCMOD, ctmp ) ;
    if ( ( pos = strstr( pos+1, " " ) ) != NULL )
      strncpy( pos+1, "\0", 1 ) ;
  }
  if ( readHeader( ctmp, "SCMOD3" ) == 0 && strncmp( ctmp, " ", 1 ) != 0 ) {
    strcat( SCMOD, ctmp ) ;
    if ( ( pos = strstr( pos+1, " " ) ) != NULL )
      strncpy( pos+1, "\0", 1 ) ;
  }
  if ( readHeader( ctmp, "SCMOD4" ) == 0 && strncmp( ctmp, " ", 1 ) != 0 ) {
    strcat( SCMOD, ctmp ) ;
    if ( ( pos = strstr( pos+1, " " ) ) != NULL )
      strncpy( pos+1, "\0", 1 ) ;
  }
  if ( readHeader( ctmp, "SCMOD5" ) == 0 && strncmp( ctmp, " ", 1 ) != 0 ) {
    strcat( SCMOD, ctmp ) ;
    if ( ( pos = strstr( pos+1, " " ) ) != NULL )
      strncpy( pos+1, "\0", 1 ) ;
  }
  if ( readHeader( ctmp, "SCMOD6" ) == 0 && strncmp( ctmp, " ", 1 ) != 0 ) {
    strcat( SCMOD, ctmp ) ;
    if ( ( pos = strstr( pos+1, " " ) ) != NULL )
      strncpy( pos+1, "\0", 1 ) ;
  }
  // DEBUG
  //cout << "SCMOD = \'" << SCMOD << "\'" << endl ;
  //
  if ( readHeader( URVEL, "VEL", sameEndian ) != 0 ) {
    //cerr << "Error while reading data URVEL." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "URVEL = " << URVEL << endl ;
  //
  if ( readHeader( VREF, "VREF" ) != 0 ) {
    //cerr << "Error while reading data VREF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VREF = " << VREF << endl ;
  //
  if ( readHeader( VDEF, "VDEF" ) != 0 ) {
    //cerr << "Error while reading data VDEF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VDEF = " << VDEF << endl ;
  //
  if ( readHeader( SWMOD, "SWMOD" ) != 0 ) {
    //cerr << "Error while reading data SWMOD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SWMOD = " << SWMOD << endl ;
  //
  if ( readHeader( FRQSW, "FRQSW", sameEndian ) != 0 ) {
    //cerr << "Error while reading data FRQSW." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "FRQSW = " << FRQSW << endl ;
  //
  if ( readHeader( DBEAM, "DBEAM", sameEndian ) != 0 ) {
    //cerr << "Error while reading data DBEAM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DBEAM = " << DBEAM << endl ;
  //
  if ( readHeader( MLTOF, "MLTOF", sameEndian ) != 0 ) {
    //cerr << "Info: MLTOF set to 0." << endl ;
    MLTOF = 0.0 ;
  }
  // DEBUG
  //cout << "MLTOF = " << MLTOF << endl ;
  //
  if ( readHeader( CMTQ, "CMTQ", sameEndian ) != 0 ) {
    cerr << "Error while reading data CMTQ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTQ = " << CMTQ << endl ;
  //
  if ( readHeader( CMTE, "CMTE", sameEndian ) != 0 ) {
    //cerr << "Error while reading data CMTE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTE = " << CMTE << endl ;
  //
  if ( readHeader( CMTSOM, "CMTSOM", sameEndian ) != 0 ) {
    //cerr << "Error while reading data CMTSOM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTSOM = " << CMTSOM << endl ;
  //
  if ( readHeader( CMTNODE, "CMTNODE", sameEndian ) != 0 ) {
    //cerr << "Error while reading data CMTNODE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTNODE = " << CMTNODE << endl ;
  //
  if ( readHeader( CMTI, "CMTI", sameEndian ) != 0 ) {
    //cerr << "Error while reading data CMTI." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTI = " << CMTI << endl ;
  //
  if ( readHeader( CMTTM, "CMTTM" ) != 0 ) {
    //cerr << "Error while reading data CMTTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTTM = " << CMTTM << endl ;
  //
  if ( readHeader( SBDX, "SDBX", sameEndian ) != 0 ) {
    //cerr << "Error while reading data SBDX." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDX = " << SBDX << endl ;
  //
  if ( readHeader( SBDY, "SDBY", sameEndian ) != 0 ) {
    //cerr << "Error while reading data SBDY." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDY = " << SBDY << endl ;
  //
  if ( readHeader( SBDZ1, "SDBZ1", sameEndian ) != 0 ) {
    //cerr << "Error while reading data SBDZ1." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ1 = " << SBDZ1 << endl ;
  //
  if ( readHeader( SBDZ2, "SDBZ2", sameEndian ) != 0 ) {
    //cerr << "Error while reading data SBDZ2." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ2 = " << SBDZ2 << endl ;
  //
  if ( readHeader( DAZP, "DAZP", sameEndian ) != 0 ) {
    //cerr << "Info: DAZP set to 0." << endl ;
    DAZP = 0.0 ;
  }
  // DEBUG
  //cout << "DAZP = " << DAZP << endl ;
  //
  if ( readHeader( DELP, "DELP", sameEndian ) != 0 ) {
    //cerr << "Info: DELP set to 0." << endl ;
    DELP = 0.0 ;
  }
  // DEBUG
  //cout << "DELP = " << DELP << endl ;
  //
  if ( readHeader( CHBIND, "CHBIND", sameEndian ) != 0 ) {
    //cerr << "Info: CHBIND set to 1." << endl ;
    CHBIND = 1 ;
  }
  // DEBUG
  //cout << "CHBIND = " << CHBIND << endl ;
  //
  if ( readHeader( NUMCH, "NCH", sameEndian ) != 0 ) {
    if ( readTable( NUMCH, "NCH", sameEndian ) != 0 ) {
      cerr << "Info: NUMCH set to 2048." << endl ;
      NUMCH = 2048 ;
    }
  }
  // DEBUG
  //cout << "NUMCH = " << NUMCH << endl ;
  //
  if ( readHeader( CHMIN, "CHMIN", sameEndian ) != 0 ) {
    //cerr << "Info: CHMIN set to 1." << endl ;
    CHMIN = 1 ;
  }
  // DEBUG
  //cout << "CHMIN = " << CHMIN << endl ;
  //
  if ( readHeader( CHMAX, "CHMAX", sameEndian ) != 0 ) {
    //cerr << "Info: CHMAX set to 2048." << endl ;
    CHMAX = 2048 ;
  }
  // DEBUG
  //cout << "CHMAX = " << CHMAX << endl ;
  //
  if ( readHeader( ALCTM, "ALCTM", sameEndian ) != 0 ) {
    if ( readTable( ALCTM, "ALCTM", sameEndian ) != 0 ) {
      cerr << "Error while reading data ALCTM." << endl ;
      return -1 ;
    }
  }
  // DEBUG
  //cout << "ALCTM = " << ALCTM << endl ;
  //
  int itmp ;
  if ( readHeader( itmp, "INTEG", sameEndian ) != 0 ) {
    if ( readTable( itmp, "INTEG", sameEndian ) != 0 ) {
      //cerr << "Error while reading data IPTIM." << endl ;
      return -1 ;
    }
  }
  IPTIM = (double)itmp ;
  // DEBUG
  //cout << "IPTIM = " << IPTIM << endl ;
  //
  if ( readHeader( PA, "PA", sameEndian ) != 0 ) {
    if ( readTable( PA, "PA", sameEndian ) != 0 ) {
      //cerr << "Error while reading data PA." << endl ;
      return -1 ;
    }
  }
  // DEBUG
  //cout << "PA = " << PA << endl ;
  //

  // find data index for each ARYTP
  findData() ;

  if ( readColumn( RX, "RX" ) != 0 ) {
    //cerr << "Error while reading data RX." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "RX      " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << RX[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( HPBW, "HPBW", sameEndian ) != 0 ) {
    //cerr << "Error while reading data HPBW." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "HPBW    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << HPBW[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( EFFA, "EFFA", sameEndian ) != 0 ) {
    //cerr << "Error while reading data EFFA." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "EFFA    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << EFFA[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( EFFB, "EFFB", sameEndian ) != 0 ) {
    //cerr << "Error while reading data EFFB." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "EFFB    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << EFFB[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( EFFL, "EFFL", sameEndian ) != 0 ) {
    //cerr << "Error while reading data EFFL." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "EFFL    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     } 
//     cout << EFFL[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( EFSS, "EFSS", sameEndian ) != 0 ) {
    //cerr << "Error while reading data EFSS." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "EFSS    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << EFSS[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( GAIN, "GAIN", sameEndian ) != 0 ) {
    //cerr << "Error while reading data GAIN." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "GAIN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << GAIN[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( HORN, "HORN" ) != 0 ) {
    //cerr << "Error while reading data HORN." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "HORN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << HORN[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( POLTP, "POLTP" ) != 0 ) {
    //cerr << "Error while reading data POLTP." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "POLTP   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << POLTP[i] << " " ;
//   }
//   cout << endl ;
  //
  vector<int> ipoldr( ARYNM, 0 ) ;
  if ( readColumn( ipoldr, "POLDR", sameEndian ) != 0 ) {
    //cerr << "Error while reading data POLDR." << endl ;
    return -1 ;
  }
  for ( int i = 0 ; i < ARYNM ; i++ ) 
    POLDR[i] = (double)ipoldr[i] ;
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "POLDR   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << POLDR[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( POLAN, "POLAN", sameEndian ) != 0 ) {
    //cerr << "Error while reading data POLAN." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "POLAN   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << POLAN[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( DFRQ, "DFRQ", sameEndian ) != 0 ) {
    //cerr << "Error while reading data DFRQ." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "DFRQ    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << DFRQ[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( SIDBD, "SIDBD" ) != 0 ) {
    //cerr << "Error while reading data SIDBD." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "SIDBD   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << SIDBD[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( REFN, "REFN", sameEndian ) != 0 ) {
    //cerr << "Error while reading data REFN." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "REFN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << REFN[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( IPINT, "IPINT", sameEndian ) != 0 ) {
    //cerr << "Error while reading data IPINT." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "IPINT   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << IPINT[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( MULTN, "MULTN", sameEndian ) != 0 ) {
    //cerr << "Error while reading data MULTN." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "MULTN   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << MULTN[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( MLTSCF, "MLTSCF", sameEndian ) != 0 ) {
    //cerr << "Error while reading data MLTSCF." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "MLTSCF  " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << MLTSCF[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( LAGWIND, "LAGWIN" ) != 0 ) {
    //cerr << "Error while reading data LAGWIND." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "LAGWIND " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << LAGWIND[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( BEBW, "BEBW", sameEndian ) != 0 ) {
    //cerr << "Error while reading data BEBW." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "BEBW    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << BEBW[i] << " " ;
//   }  
//   cout << endl ;
  //
  if ( readColumn( BERES, "BERES", sameEndian ) != 0 ) {
    //cerr << "Error while reading data BERES." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "BERES   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << BERES[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( CHWID, "CHWID", sameEndian ) != 0 ) {
    //cerr << "Error while reading data CHWID." << endl ;
    return -1 ;
  }
    // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "CHWID   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << CHWID[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readARRY() != 0 ) {
    //cerr << "Error while reading data ARRY." << endl ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < NRO_FITS_ARYMAX ; i++ ) {
//     if ( i == 0 ) {
//       cout << "ARRY    " ;
//     }
//     else if ( ( i % 20 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << ARRY[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( NFCAL, "NFCAL", sameEndian ) != 0 ) {
    //cerr << "Error while reading data NFCAL." << endl ;
    return -1 ;
  }
    // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "NFCAL   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << NFCAL[i] << " " ;
//   }
//   cout << endl ;
  //
  if ( readColumn( F0CAL, "F0CAL", sameEndian ) != 0 ) {
    //cerr << "Error while reading data F0CAL." << endl ;
    return -1 ;
  }
    // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "F0CAL   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << F0CAL[i] << " " ;
//   }
//   cout << endl ;
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readColumn( vv, "FQCAL", sameEndian, i ) != 0 ) {
      //cerr << "Error while reading data FQCAL." << endl ;
      return -1 ;
    }
    for ( int j = 0 ; j < ARYNM ; j++ ) {
      FQCAL[j][i] = vv[i] ;
    }
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     for ( int j = 0 ; j < 10 ; j++ ) {
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
//     }
//     cout << endl ;
//   }
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readColumn( vv, "CHCAL", sameEndian, i ) != 0 ) {
      //cerr << "Error while reading data CHCAL." << endl ;
      return -1 ;
    }
    for ( int j = 0 ; j < ARYNM ; j++ ) {
      CHCAL[j][i] = vv[i] ;
    }
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     for ( int j = 0 ; j < 10 ; j++ ) {
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
//    }
//    cout << endl ;
//   }
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readColumn( vv, "CWCAL", sameEndian, i ) != 0 ) {
      //cerr << "Error while reading data CWCAL." << endl ;
      return -1 ;
    }
    for ( int j = 0 ; j < ARYNM ; j++ ) {
      CWCAL[j][i] = vv[i] ;
    }
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     for ( int j = 0 ; j < 10 ; j++ ) {
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
//     }
//     cout << endl ;
//   }
  //
  if ( readHeader( SCNLEN, "NAXIS1", sameEndian ) != 0 ) {
    //cerr << "Error while reading data SCNLEN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNLEN = " << SCNLEN << endl ;
  //
  if ( readHeader( SBIND, "SBIND", sameEndian ) != 0 ) {
    //cerr << "Info: SBIND set to 0." << endl ;
    SBIND = 0 ;
  }
  // DEBUG
  //cout << "SBIND = " << SBIND << endl ;
  //
  if ( readHeader( IBIT, "IBIT", sameEndian ) != 0 ) {
    //cerr << "Info: IBIT set to 8." << endl ;
    IBIT = 8 ;  // 8 bit? 12 bit?
  }
  // DEBUG
  //cout << "IBIT = " << IBIT << endl ;
  //
  if ( readHeader( SITE, "TELESCOP" ) != 0 ) {
    //cerr << "Error while reading data SITE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SITE = " << SITE << endl ;
  //
//   if ( readHeader( CDMY1, "CDMY1" ) != 0 ) {
//     cerr << "Info: CDMY1 set to \"\"." << endl ;
//     CDMY1 = "" ;
//   }
  // DEBUG
  //cout << "CDMY1 = " << CDMY1 << endl ;
  //
  if ( readColumn( DSBFC, "DSBFC", sameEndian ) != 0 ) {
    //cerr << "Info: DSBFC set to 1." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      DSBFC[i] = 1.0 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       cout << "DSBFC   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       cout << endl << "        " ;
//     }
//     cout << DSBFC[i] << " " ;
//   }
//   cout << endl ;
  //

  return 0 ;
}

int NROFITSDataset::fillRecord( int i )
{
  int status = 0 ;
  status = readTable( record_->LSFIL, "LSFIL", i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading LSFIL." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "LSFIL(" << i << ") = " << record_->LSFIL << endl ;
  //
  status = readTable( record_->ISCAN, "ISCN", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading ISCAN." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "ISCAN(" << i << ") = " << record_->ISCAN << endl ;
  //
  vector<int> itmp( 6, 0 ) ;
  status = readTable( itmp, "LAVST", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading LAVST." << endl ;
    return status ;
  }
  else {
    sprintf( record_->LAVST, "%d%d%d%d%d%d.000", itmp[0], itmp[1], itmp[2], itmp[3], itmp[4], itmp[5] ) ;
  }
  // DEBUG
  //cout << "LAVST(" << i << ") = " << record_->LAVST << endl ;
  //
  status = readTable( record_->SCANTP, "SCNTP", i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading SCANTP." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "SCANTP(" << i << ") = " << record_->SCANTP << endl ;
  //
  char *name1 = "" ;
  char *name2 = "" ;
  if ( SCNCD == 0 ) {
    name1 = "DRA" ;
    name2 = "DDEC" ;
  }
  else if ( SCNCD == 1 ) {
    name1 = "DGL" ;
    name2 = "DGB" ;
  }
  else {
    name1 = "DAZ" ;
    name2 = "DEL" ;
  }
  status = readTable( record_->DSCX, name1, same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading DSCX." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "DSCX(" << i << ") = " << record_->DSCX << endl ;
  //
  status = readTable( record_->DSCY, name2, same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading DSCY." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "DSCY(" << i << ") = " << record_->DSCY << endl ;
  //
  if ( SCNCD == 0 ) {
    name1 = "RA" ;
    name2 = "DEC" ;
  }
  else if ( SCNCD == 1 ) {
    name1 = "GL" ;
    name2 = "GB" ;
  }
  else {
    name1 = "AZ" ;
    name2 = "EL" ;
  }
  status = readTable( record_->SCX, name1, same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading SCX." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "SCX(" << i << ") = " << record_->SCX << endl ;
  //
  status = readTable( record_->SCY, name2, same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading SCY." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "SCY(" << i << ") = " << record_->SCY << endl ;
  //
  status = readTable( record_->PAZ, "PAZ", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading PAZ." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "PAZ(" << i << ") = " << record_->PAZ << endl ;
  //
  status = readTable( record_->PEL, "PEL", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading PEL." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "PEL(" << i << ") = " << record_->PEL << endl ;
  //
  status = readTable( record_->RAZ, "RAZ", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading RAZ." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "RAZ(" << i << ") = " << record_->RAZ << endl ;
  //
  status = readTable( record_->REL, "REL", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading REL." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "REL(" << i << ") = " << record_->REL << endl ;
  //
  status = readTable( record_->XX, "XX", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading XX." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "XX(" << i << ") = " << record_->XX << endl ;
  //
  status = readTable( record_->YY, "YY", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading YY." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "YY(" << i << ") = " << record_->YY << endl ;
  //
  status = readTable( record_->ARRYT, "ARRYT", i ) ;
  record_->ARRYT[3] = '\0' ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading ARRYT." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "ARRYT(" << i << ") = " << record_->ARRYT << endl ;
  //
  double dtmp ;
  status = readTable( dtmp, "TEMP", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading TEMP." << endl ;
    return status ;
  }
  else {
    record_->TEMP = dtmp ;
  }
  // DEBUG
  //cout << "TEMP(" << i << ") = " << record_->TEMP << endl ;
  //
  status = readTable( dtmp, "PATM", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading PATM." << endl ;
    return status ;
  }
  else {
    record_->PATM = dtmp ;
  }
  // DEBUG
  //cout << "PATM(" << i << ") = " << record_->PATM << endl ;
  //
  status = readTable( dtmp, "PH2O", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading PH2O." << endl ;
    return status ;
  }
  else {
    record_->PH2O = dtmp ;
  }
  // DEBUG
  //cout << "PH2O(" << i << ") = " << record_->PH2O << endl ;
  //
  status = readTable( dtmp, "VWIND", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading VWIND." << endl ;
    return status ;
  }
  else {
    record_->VWIND = dtmp ;
  }
  // DEBUG
  //cout << "VWIND(" << i << ") = " << record_->VWIND << endl ;
  //
  status = readTable( dtmp, "DWIND", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading DWIND." << endl ;
    return status ;
  }
  else {
    record_->DWIND = dtmp ;
  }
  // DEBUG
  //cout << "DWIND(" << i << ") = " << record_->DWIND << endl ;
  //
  status = readTable( dtmp, "TAU", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading TAU." << endl ;
    return status ;
  }
  else {
    record_->TAU = dtmp ;
  }
  // DEBUG
  //cout << "TAU(" << i << ") = " << record_->TAU << endl ;
  //
  status = readTable( dtmp, "TSYS", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading TSYS." << endl ;
    return status ;
  }
  else {
    record_->TSYS = dtmp ;
  }
  // DEBUG
  //cout << "TSYS(" << i << ") = " << record_->TSYS << endl ;
  //
  status = readTable( dtmp, "BATM", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading BATM." << endl ;
    return status ;
  }
  else {
    record_->BATM = dtmp ;
  }
  // DEBUG
  //cout << "BATM(" << i << ") = " << record_->BATM << endl ;
  //
  status = readTable( record_->VRAD, "VRAD", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading TEMP." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "VRAD(" << i << ") = " << record_->VRAD << endl ;
  //
  status = readTable( record_->FREQ0, "FRQ0", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading FREQ0." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "FREQ0(" << i << ") = " << record_->FREQ0 << endl ;
  //
  status = readTable( record_->FQTRK, "FQTRK", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading FQTRK." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "FQTRK(" << i << ") = " << record_->FQTRK << endl ;
  //
  status = readTable( record_->FQIF1, "FQIF1", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading FQIF1." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "FQIF1(" << i << ") = " << record_->FQIF1 << endl ;
  //
  status = readTable( record_->ALCV, "ALCV", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading ALCV." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "ALCV(" << i << ") = " << record_->ALCV << endl ;
  //
  record_->IDMY0 = 0 ;
  status = readTable( record_->DPFRQ, "DPFRQ", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord()  DPFRQ set to 0." << endl ;
    record_->DPFRQ = 0.0 ;
  }
  // DEBUG
  //cout << "DPFRQ(" << i << ") = " << record_->DPFRQ << endl ;
  //
  status = readTable( record_->SFCTR, "SFCTR", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading SFCTR." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "SFCTR(" << i << ") = " << record_->SFCTR << endl ;
  //
  status = readTable( record_->ADOFF, "ADOFF", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading ADOFF." << endl ;
    return status ;
  }
  // DEBUG
  //cout << "ADOFF(" << i << ") = " << record_->ADOFF << endl ;
  //
  status = readTable( record_->JDATA, "LDATA", same_, i ) ;
  if ( status ) {
    //cerr << "NROFITSDataset::fillRecord() while reading JDATA." << endl ;
    return status ;
  }
  // DEBUG
//   for ( int i = 0 ; i < chmax_ ; i++ ) 
//     cout << "JDATA[" << i << "] = " << record_->JDATA[i] << " " ;
//   cout << endl ;
  //
  return status ;
}

vector< vector<double> > NROFITSDataset::getSpectrum()
{
  vector< vector<double> > spec;

  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    spec.push_back( getSpectrum( i ) ) ;
  }

  return spec ;
}

vector<double> NROFITSDataset::getSpectrum( int i ) 
{
  vector<double> spec( chmax_, 0.0 ) ;
  NRODataRecord *record = getRecord( i ) ;
  double scale = record->SFCTR ;
  double offset = record->ADOFF ;
  double dscale = MLTSCF[getIndex( i )] ;
  vector<int> ispec = record->JDATA ;
  for ( int i = 0 ; i < chmax_ ; i++ ) {
    spec[i] = (double)( ispec[i] * scale + offset ) * dscale ;
  }
  return spec ;
}

int NROFITSDataset::getIndex( int irow ) 
{
  NRODataRecord *record = getRecord( irow ) ;
  string str = record->ARRYT ;
  int pos = str.find( " " ) ;
  if ( pos != string::npos ) 
    str = str.substr( 0, pos ) ;
  int index = -1 ;
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    if ( str.compare( 0, 3, ARYTP[i] ) == 0 ) {
      index = i ;
      break ;
    }
  }
  return index ;
}

double NROFITSDataset::radRA( char *ra ) 
{
  char *ch = strtok( ra, ":" ) ;
  char *cm = strtok( NULL, ":" ) ;
  char *cs = strtok( NULL, ":" ) ;
  double h = atof( ch ) + atof( cm ) / 60.0 + atof( cs ) / 3600.0 ;
  double rra = h * M_PI / 12.0 ;
  return rra ;
}

double NROFITSDataset::radDEC( char *dec ) 
{
  char *cd = strtok( dec, ":" ) ;
  char *cm = strtok( NULL, ":" ) ;
  char *cs = strtok( NULL, ":" ) ;
  double d = atof( cd ) + atof( cm ) / 60.0 + atof( cs ) / 3600.0 ;
  double rdec = d * M_PI / 180.0 ;
  return rdec ;
}

void NROFITSDataset::getField() 
{
  for ( int i = 0 ; i < numField_ ; i++ ) {
    char *key1 ;
    char *key2 ;
    char *key3 ;
    if ( i < 9 ) {
      key1 = new char[8] ;
      sprintf( key1, "TFORM%d  ", i+1 ) ;
      key2 = new char[8] ;
      sprintf( key2, "TTYPE%d  ", i+1 ) ;
      key3 = new char[8] ;
      sprintf( key3, "TUNIT%d  ", i+1 ) ;
      //cout << "key1 = " << key1 << ", key2 = " << key2 << ", key3 = " << key3 << endl ;
    }
    else if ( i < 99 ) {
      key1 = new char[8] ;
      sprintf( key1, "TFORM%2d ", i+1 ) ;
      key2 = new char[8] ;
      sprintf( key2, "TTYPE%2d ", i+1 ) ;
      key3 = new char[8] ;
      sprintf( key3, "TUNIT%2d ", i+1 ) ;
      //cout << "key1 = " << key1 << ", key2 = " << key2 << ", key3 = " << key3 << endl ;
    }
    else {
      key1 = new char[8] ;
      sprintf( key1, "TFORM%3d", i+1 ) ;
      key2 = new char[8] ;
      sprintf( key2, "TTYPE%3d", i+1 ) ; 
      key3 = new char[8] ;
      sprintf( key3, "TUNIT%3d", i+1 ) ;
      //cout << "key1 = " << key1 << ", key2 = " << key2 << ", key3 = " << key3 << endl ;
    }
    char tmp[8] ;
    if ( readHeader( tmp, key1 ) != 0 ) {
      cerr << "Error while reading field keyword for scan header." << endl ;
      delete key1 ;
      delete key2 ;
      delete key3 ;
      return ;
    }
    forms_[i] = string( tmp ) ;
    if ( readHeader( tmp, key2 ) != 0 ) {
      cerr << "Error while reading field type for scan header." << endl ;
      delete key1 ;
      delete key2 ;
      delete key3 ;
      return ;
    }
    names_[i] = string( tmp ) ;
    if ( forms_[i].find( "A" ) != string::npos ) {
      //cout << "skip to get unit: name = " << forms_[i] << endl ;
      strcpy( tmp, "none    " ) ;
    }
    else {
      //cout << "get unit: name = " << forms_[i] << endl ;
      if ( readHeader( tmp, key3 ) != 0 ) {
        strcpy( tmp, "none    " ) ;
      }
    }
    units_[i] = string( tmp ) ;
    //cout << "i = " << i << ": name=" << forms_[i] << " type=" << names_[i] << " unit=" << units_[i] << endl ;
    delete key1 ;
    delete key2 ;
    delete key3 ;
  }  
}

void NROFITSDataset::fillARYTP()
{
  char arry[20] ;
  int count = 0 ;
  string arry1 ;
  string arry2 ;
  string arry3 ;
  string arry4 ;
  if ( readHeader( arry, "ARRY1" ) == 0 ) 
    arry1 = string( arry ) ;
  else 
    arry1 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry1[i] == '1' ) {
      char arytp[3] ;
      //sprintf( arytp, "H%02d", i+1 ) ;
      sprintf( arytp, "H%d", i+1 ) ;
      ARYTP[count++] = string( arytp ) ;
      //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ; 
    }
  }
  if ( readHeader( arry, "ARRY2" ) == 0 )
    arry2 = string( arry ) ;
  else 
    arry2 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry2[i] == '1' ) {
      if ( i < 10 ) {
        char arytp[3] ;
        //sprintf( arytp, "W%02d", i+1 ) ;
        sprintf( arytp, "W%d", i+1 ) ;
        ARYTP[count++] = string( arytp ) ;
        //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
      }
      else if ( i < 15 ) {
        char arytp[3] ;
        //sprintf( arytp, "U%02d", i-9 ) ;
        sprintf( arytp, "U%d", i-9 ) ;
        ARYTP[count++] = string( arytp ) ;
        //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
      }
      else {
        //char arytp[4] ;
        //sprintf( arytp, "FX%02d", i-14 ) ;
        char arytp[3] ;
        sprintf( arytp, "F%d", i-14 ) ;
        ARYTP[count++] = string( arytp ) ;
        //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
      }
    }
  }
  if ( readHeader( arry, "ARRY3" ) == 0 ) 
    arry3 = string( arry ) ;
  else 
    arry3 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry3[i] == '1' ) {
      char arytp[3] ;
      //sprintf( arytp, "A%02d", i ) ;
      sprintf( arytp, "A%d", i ) ;
      ARYTP[count++] = string( arytp ) ;
      //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
    }
  }
  if ( readHeader( arry, "ARRY4" ) == 0 )
    arry4 = string( arry ) ;
  else 
    arry4 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry4[i] == '1' ) {
      char arytp[3] ;
      //sprintf( arytp, "A%02d", i+21 ) ;
      sprintf( arytp, "A%d", i+21 ) ;
      ARYTP[count++] = string( arytp ) ;
      //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
    }
  }      
}

int NROFITSDataset::readARRY()
{
  char *arry1 = new char[21] ;
  char *arry2 = new char[21] ;
  char *arry3 = new char[21] ;
  char *arry4 = new char[21] ;
  int status = readHeader( arry1, "ARRY1" ) ;
  if ( status ) {
    cerr << "Error while reading ARRY1" << endl ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  status = readHeader( arry2, "ARRY2" ) ;
  if ( status ) {
    cerr << "Error while reading ARRY2" << endl ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  status = readHeader( arry3, "ARRY3" ) ;
  if ( status ) {
    cerr << "Error while reading ARRY3" << endl ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  status = readHeader( arry4, "ARRY4" ) ;
  if ( status ) {
    cerr << "Error while reading ARRY4" << endl ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  int index = 0 ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    // AOSH
    if ( arry1[i] == '1' )
      ARRY[index] = 1 ;
    else
      ARRY[index] = 0 ;
    // AOSW, AOSU, FX
    if ( arry2[i] == '1' )
      ARRY[index+20] = 1 ;
    else
      ARRY[index+20] = 0 ;
    // AC45 (1-20)
    if ( arry3[i] == '1' ) 
      ARRY[index+40] = 1 ;
    else 
      ARRY[index+40] = 0 ;
    // AC45 (21-35)
    if ( i < 15 ) {
      if ( arry4[i] == '1' )
        ARRY[index+60] = 1 ;
      else
        ARRY[index+60] = 0 ;
    }
    index++ ;
  }
  delete arry1 ;
  delete arry2 ;
  delete arry3 ;
  delete arry4 ;
  return status ;
}

void NROFITSDataset::findData() 
{
  //cout << "NROFITSDataset::findData()  start." << endl ;

  // skip header
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // get offset
  int offset = getOffset( "ARRYT" ) ;
  if ( offset == -1 ) {
    //cerr << "Error, ARRYT is not found in the name list." << endl ; 
    return ;
  }
  //cout << "offset for ARRYT is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;
  int count = 0 ;
  int index = 0 ;
  while ( count < ARYNM && index < rowNum_ ) {
    char ctmp[5] ;
    fread( ctmp, 1, 4, fp_ ) ;
    ctmp[4] = '\0' ;
    //cout << "ctmp = " << ctmp << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) {
      if ( arrayid_[i] != -1 ) 
        continue ;
      else if ( strncmp( ctmp, ARYTP[i].c_str(), ARYTP[i].size() ) == 0 ) {
        //cout << "matched: i = " << i << ", ARYTP = " << ARYTP[i] << ", ctmp = " << ctmp << endl ;
        arrayid_[i] = index ;
        count++ ;
      }
    }
    fseek( fp_, scanLen_-4, SEEK_CUR ) ;
    index++ ;
  }

  if ( count != ARYNM ) {
    cerr << "NROFITSDataset::findData()  failed to find rows for " ;
    for ( int i = 0 ; i < ARYNM ; i++ ) {
      if ( arrayid_[i] == -1 ) {
        cout << ARYTP[i] << " " ;
      }
    }
    cout << endl ;
  }

//   for ( int i = 0 ; i < ARYNM ; i++ ) 
//     cout << "arrayid_[" << i << "] = " << arrayid_[i] << endl ;
  
  //cout << "NROFITSDataset::findData()  end." << endl ;
}

int NROFITSDataset::getOffset( char *name ) 
{
  int offset = 0 ;
  string sname( name ) ;
  bool found = false ;
  for ( int i = 0 ; i < numField_ ; i++ ) {
    // escape if name is found
    //cout << "names_[" << i << "] = " << names_[i] << "  sname = " << sname << endl ;
    if ( names_[i] == sname ) {
      found = true ;
      break ;
    }

    // form analysis
    string substr1 = forms_[i].substr( 0, forms_[i].size()-1 ) ;
    string substr2 = forms_[i].substr( forms_[i].size()-1, 1 ) ;
    //cout << "substr1 = " << substr1 << ", substr2 = " << substr2 << endl ;
    int o1 = atoi( substr1.c_str() ) ;
    int o2 = 0 ;
    if ( substr2 == "A" )
      o2 = sizeof(char) ;
    else if ( substr2 == "J" ) 
      o2 = sizeof(int) ;
    else if ( substr2 == "F" )
      o2 = sizeof(float) ;
    else if ( substr2 == "D" )
      o2 = sizeof(double) ;
    //cout << "o1 = " << o1 << ", o2 = " << o2 << endl ;
    offset += o1 * o2 ;
  }

  if ( !found )
    offset = -1 ;

  return offset ;
}

int NROFITSDataset::getPolarizationNum()
{
  int npol = 0 ;

  vector<char> type( 2 ) ;
  type[0] = 'C' ;
  type[1] = 'L' ;
  vector<double> crot ;
  vector<double> lagl ;

  for ( int i = 0 ; i < ARYNM ; i++ ) { 
    if ( POLTP[i][0] == type[0] ) {
      // circular polarization
      if( count( crot.begin(), crot.end(), POLDR[i] ) != 0 ) {
        crot.push_back( POLDR[i] ) ;
        npol++ ;
      }
    }
    else if ( POLTP[i][0] == type[1] ) {
      // linear polarization
      if ( count( lagl.begin(), lagl.end(), POLAN[i] ) != 0 ) {
        lagl.push_back( POLAN[i] ) ;
        npol++ ;
      }
    }
  }

  if ( npol == 0 )
    npol = 1 ;


  return npol ;
}

int NROFITSDataset::readHeader( char *v, char *name ) 
{
  //
  // Read 'name' attribute defined as char from the FITS Header
  //
  int status = 0 ;
  
  char buf[81] ;
  strcpy( buf, "     " ) ;
  fseek( fp_, 0, SEEK_SET ) ;
  //int count = 0 ;
  while ( strncmp( buf, name, strlen(name) ) != 0 && strncmp( buf, "END", 3 ) != 0 ) {
    fread( buf, 1, 80, fp_ ) ;
    buf[80] = '\0' ;
    //count++ ;
  }
  if ( strncmp( buf, "END", 3 ) == 0 ) {
    //cerr << "NROFITSDataset::readHeader()  keyword " << name << " not found." << endl ;
    //cerr << "count = " << count << endl ;
    status = -1 ;
    return status ;
  }
  string str( buf ) ;
  int pos1 = str.find( '\'' ) + 1 ;
  int pos2 = str.find( '\'', pos1 ) ;
  //cout << "string: " << str << endl ;
  //cout << "value:  " << str.substr( pos1, pos2 - pos1 ).c_str() << endl ;
  strcpy( v, (str.substr( pos1, pos2 - pos1 )).c_str() ) ;
  if ( strlen( v ) > ( pos2 - pos1 ) )
    v[pos2-pos1] = '\0' ;
  //cout << "v = " << v << endl ;

  return status ;
}

int NROFITSDataset::readHeader( int &v, char *name, int b ) 
{
  //
  // Read 'name' attribute defined as int from the FITS Header
  //
  int status = 0 ;

  char buf[81] ;
  strcpy( buf, "     " ) ;
  fseek( fp_, 0, SEEK_SET ) ;
  while ( strncmp( buf, name, strlen(name) ) != 0 && strncmp( buf, "END", 3 ) != 0 ) {
    fread( buf, 1, 80, fp_ ) ;
    buf[80] = '\0' ;
    //char bufo[9] ;
    //strncpy( bufo, buf, 8 ) ;
    //bufo[8] = '\0' ;
    //cout << "header: " << bufo << endl ;
  }
  if ( strncmp( buf, "END", 3 ) == 0 ) {
    //cerr << "NROFITSDataset::readHeader()  keyword " << name << " not found." << endl ;
    status = -1 ;
    return status ;
  }
  string str( buf ) ;
  int pos1 = str.find( '=' ) + 1 ;
  int pos2 = str.find( '/' ) ;
  //cout << "string: " << str << endl ;
  //cout << "value:  " << str.substr( pos1, pos2 - pos1 ).c_str() << endl ;
  v = atoi( str.substr( pos1, pos2 - pos1 ).c_str() ) ;
  //cout << "v = " << v << endl ;

  //cout << "NROFITSDataset::readHeader() end to read" << endl ;
  return status ;
}


int NROFITSDataset::readHeader( float &v, char *name, int b ) 
{
  //
  // Read 'name' attribute defined as float from the FITS Header
  //
  int status = 0 ;

  char buf[81] ;
  strcpy( buf, "     " ) ;
  fseek( fp_, 0, SEEK_SET ) ;
  while ( strncmp( buf, name, strlen(name) ) != 0 && strncmp( buf, "END", 3 ) != 0 ) {
    fread( buf, 1, 80, fp_ ) ;
    buf[80] = '\0' ;
    //char bufo[9] ;
    //strncpy( bufo, buf, 8 ) ;
    //bufo[8] = '\0' ;
    //cout << "header: " << bufo << endl ;
  }
  if ( strncmp( buf, "END", 3 ) == 0 ) {
    //cerr << "NROFITSDataset::readHeader()  keyword " << name << " not found." << endl ;
    status = -1 ;
    return status ;
  }
  string str( buf ) ;
  int pos1 = str.find( '=' ) + 1 ;
  int pos2 = str.find( '/' ) ;
  //cout << "string: " << str << endl ;
  //cout << "value:  " << str.substr( pos1, pos2 - pos1 ).c_str() << endl ;
  v = atof( str.substr( pos1, pos2 - pos1 ).c_str() ) ;
  //cout << "v = " << v << endl ;

  return status ;
}

int NROFITSDataset::readHeader( double &v, char *name, int b ) 
{
  //
  // Read 'name' attribute defined as double from the FITS Header
  //
  int status = 0 ;

  char buf[81] ;
  strcpy( buf, "     " ) ;
  fseek( fp_, 0, SEEK_SET ) ;
  while ( strncmp( buf, name, strlen(name) ) != 0 && strncmp( buf, "END", 3 ) != 0 ) {
    fread( buf, 1, 80, fp_ ) ;
    buf[80] = '\0' ;
    char bufo[9] ;
    strncpy( bufo, buf, 8 ) ;
    bufo[8] = '\0' ;
    //cout << "header: \'" << bufo << "\' bufo = \'" << bufo << "\' ";
    //cout << strncmp( buf, name, strlen(name) ) << endl ;
  }
  if ( strncmp( buf, "END", 3 ) == 0 ) {
    //cerr << "NROFITSDataset::readHeader()  keyword " << name << " not found." << endl ;
    status = -1 ;
    return status ;
  }
  string str( buf ) ;
  int pos1 = str.find( '=' ) + 1 ;
  int pos2 = str.find( '/' ) ;
  //cout << "string: " << str << endl ;
  //cout << "value:  " << str.substr( pos1, pos2 - pos1 ).c_str() << endl ;
  v = atof( str.substr( pos1, pos2 - pos1 ).c_str() ) ;
  //cout << "v = " << v << endl ;

  return status ;
}

int NROFITSDataset::readTable( char *v, char *name ) 
{
  //
  // Read 'name' attribute defined as char from the 0-th row 
  // of the FITS Scan Record
  //
  int status = readTable( v, name, 0 ) ;

  return status ;
}

int NROFITSDataset::readTable( char *v, char *name, int idx ) 
{
  //
  // Read 'name' attribute defined as char from the idx-th row 
  // of the FITS Scan Record
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  // get length of char
  int index = -1 ;
  for ( int i = 0 ; i < numField_ ; i++ ) {
    if ( names_[i] == name ) {
      index = i ;
      break ;
    }
  }
  string substr = forms_[index].substr( 0, forms_[index].size()-1 ) ;
  int xsize = atoi( substr.c_str() ) ;
  //cout << "xsize = " << xsize << endl ;

  // read data
  fread( v, 1, xsize, fp_ ) ;
  v[xsize] = '\0' ;

  return status ;
}

int NROFITSDataset::readTable( int &v, char *name, int b ) 
{
  //
  // Read 'name' attribute defined as int from the 0-th row 
  // of the FITS Scan Record
  //
  int status = readTable( v, name, b, 0 ) ;

  return status ;
}

int NROFITSDataset::readTable( int &v, char *name, int b, int idx ) 
{
  //
  // Read 'name' attribute defined as int from the idx-th row 
  // of the FITS Scan Record
  //
  int status = 0 ;
  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  // read data
  fread( &v, sizeof(int), 1, fp_ ) ;
  if ( b == 0 ) 
    convertEndian( v ) ;
 
  return status ;
}

int NROFITSDataset::readTable( float &v, char *name, int b ) 
{
  //
  // Read 'name' attribute defined as float from the 0-th row 
  // of the FITS Scan Record
  //
  int status = readTable( v, name, b, 0 ) ;

  return status ;
}

int NROFITSDataset::readTable( float &v, char *name, int b, int idx ) 
{
  //
  // Read 'name' attribute defined as float from the idx-th row 
  // of the FITS Scan Record
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  // read data
  fread( &v, sizeof(float), 1, fp_ ) ;
  if ( b == 0 ) 
    convertEndian( v ) ;

  return status ;
}

int NROFITSDataset::readTable( double &v, char *name, int b ) 
{
  //
  // Read 'name' attribute defined as double from the 0-th row 
  // of the FITS Scan Record
  //
  int status = readTable( v, name, b, 0 ) ;

  return status ;
}

int NROFITSDataset::readTable( double &v, char *name, int b, int idx ) 
{
  //
  // Read 'name' attribute defined as double from the idx-th row 
  // of the FITS Scan Record
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  // read data
  fread( &v, sizeof(double), 1, fp_ ) ;
  if ( b == 0 ) 
    convertEndian( v ) ;  

  return status ;
}

int NROFITSDataset::readTable( vector<char *> &v, char *name, int idx ) 
{
  //
  // Read 'name' attribute defined as char array from the FITS Scan Record
  //
  int status = 0 ;
  
  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  // get length of char
  int index = -1 ;
  for ( int i = 0 ; i < numField_ ; i++ ) {
    if ( names_[i] == name ) {
      index = i ;
      break ;
    }
  }
  string substr = forms_[index].substr( 0, forms_[index].size()-1 ) ;
  int xsize = atoi( substr.c_str() ) ;
  //cout << "xsize = " << xsize << endl ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    fread( v[i], 1, xsize, fp_ ) ;
    v[xsize] = '\0' ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
  }

  return status ;
}

int NROFITSDataset::readTable( vector<int> &v, char *name, int b, int idx ) 
{
  //
  // Read 'name' attribute defined as int array from the FITS Scan Record
  //
  int status = 0 ;
  
  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    fread( &v[i], 1, sizeof(int), fp_ ) ;
    if ( b == 0 ) 
      convertEndian( v[i] ) ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
  }

  return status ;
}

int NROFITSDataset::readTable( vector<float> &v, char *name, int b, int idx ) 
{
  //
  // Read 'name' attribute defined as float array from the FITS Scan Record
  //
  int status = 0 ;
  
  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    fread( &v[i], 1, sizeof(float), fp_ ) ;
    if ( b == 0 ) 
      convertEndian( v[i] ) ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
  }

  return status ;
}

int NROFITSDataset::readTable( vector<double> &v, char *name, int b, int idx ) 
{
  //
  // Read 'name' attribute defined as double array from the FITS Scan Record
  //
  int status = 0 ;
  
  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  offset += idx * scanLen_ ;
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    fread( &v[i], 1, sizeof(double), fp_ ) ;
    if ( b == 0 ) 
      convertEndian( v[i] ) ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
  }

  return status ;
}

int NROFITSDataset::readColumn( vector<char *> &v, char *name )
{
  // 
  // Read 0-th column of ARRYTP-dependent 'name' attributes 
  // defined as char array from the FITS Scan Record 
  //
  int status = readColumn( v, name, 0 ) ;

  return status ;
}

int NROFITSDataset::readColumn( vector<char *> &v, char *name, int idx )
{
  // 
  // Read idx-th column of ARRYTP-dependent 'name' attributes 
  // defined as char array from the FITS Scan Record 
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  // get length of char
  int index = -1 ;
  for ( int i = 0 ; i < numField_ ; i++ ) {
    if ( names_[i] == name ) {
      index = i ;
      break ;
    }
  }
  string substr = forms_[index].substr( 0, forms_[index].size()-1 ) ;
  int xsize = atoi( substr.c_str() ) ;
  //cout << "xsize = " << xsize << endl ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    offset = scanLen_ * arrayid_[i] + xsize * idx ;
    fseek( fp_, offset, SEEK_CUR ) ;
    fread( v[i], 1, xsize, fp_ ) ;
    v[i][xsize] = '\0' ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
    fseek( fp_, -xsize-offset, SEEK_CUR ) ;
  }

  return status ;
}

int NROFITSDataset::readColumn( vector<int> &v, char *name, int b ) 
{
  // 
  // Read 0-th column of ARRYTP-dependent 'name' attributes 
  // defined as int array from the FITS Scan Record 
  //
  int status = readColumn( v, name, b, 0 ) ;

  return status ;
}

int NROFITSDataset::readColumn( vector<int> &v, char *name, int b, int idx )
{
  // 
  // Read idx-th column of ARRYTP-dependent 'name' attributes 
  // defined as int array from the FITS Scan Record 
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    offset = scanLen_ * arrayid_[i] + sizeof(int) * idx ;
    fseek( fp_, offset, SEEK_CUR ) ;
    fread( &v[i], 1, sizeof(int), fp_ ) ;
    if ( b == 0 ) 
      convertEndian( v[i] ) ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
    fseek( fp_, -sizeof(int)-offset, SEEK_CUR ) ;
  }

  return status ;
}

int NROFITSDataset::readColumn( vector<float> &v, char *name, int b ) 
{
  // 
  // Read 0-th column of ARRYTP-dependent 'name' attributes 
  // defined as float array from the FITS Scan Record 
  //
  int status = readColumn( v, name, b, 0 ) ;

  return status ;
}

int NROFITSDataset::readColumn( vector<float> &v, char *name, int b, int idx ) 
{
  // 
  // Read idx-th column of ARRYTP-dependent 'name' attributes 
  // defined as float array from the FITS Scan Record 
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    offset = scanLen_ * arrayid_[i] + sizeof(float) * idx ;
    fseek( fp_, offset, SEEK_CUR ) ;
    fread( &v[i], 1, sizeof(float), fp_ ) ;
    if ( b == 0 ) 
      convertEndian( v[i] ) ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
    fseek( fp_, -sizeof(float)-offset, SEEK_CUR ) ;
  }

  return status ;
}

int NROFITSDataset::readColumn( vector<double> &v, char *name, int b ) 
{
  // 
  // Read 0-th column of ARRYTP-dependent 'name' attributes 
  // defined as double array from the FITS Scan Record 
  //
  int status = readColumn( v, name, b, 0 ) ;

  return status ;
}

int NROFITSDataset::readColumn( vector<double> &v, char *name, int b, int idx ) 
{
  // 
  // Read idx-th column of ARRYTP-dependent 'name' attributes 
  // defined as double array from the FITS Scan Record 
  //
  int status = 0 ;

  // skip header 
  fseek( fp_, FITS_HEADER_SIZE, SEEK_SET ) ;

  // find offset
  int offset = getOffset( name ) ;
  if ( offset == -1 ) {
    //cerr << "Error, " << name << " is not found in the name list." << endl ; 
    return -1 ;
  }
  //cout << "offset for " << name << " is " << offset << " bytes." << endl ;
  fseek( fp_, offset, SEEK_CUR ) ;

  for ( unsigned int i = 0 ; i < v.size() ; i++ ) {
    offset = scanLen_ * arrayid_[i] + sizeof(double) * idx ;
    fseek( fp_, offset, SEEK_CUR ) ;
    fread( &v[i], 1, sizeof(double), fp_ ) ;
    if ( b == 0 ) 
      convertEndian( v[i] ) ;
    //cout << "v[" << i << "] = " << v[i] << endl ;
    fseek( fp_, -sizeof(double)-offset, SEEK_CUR ) ;
  }

  return status ;
}


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

#define NULLVAL_CHAR ""
#define NULLVAL_INT -999
#define NULLVAL_FLOAT -999999999.0
#define NULLVAL_DOUBLE -999999999.0

// constructor 
NROFITSDataset::NROFITSDataset( string name )
  : NRODataset( name ) 
{
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
 
  // memory allocation
  initialize() ;

  readHeader( numField_, "TFIELDS", same_ ) ;
  names_.resize( numField_ ) ;
  types_.resize( numField_ ) ;
  units_.resize( numField_ ) ;

  // data initialization
  getField() ;

  fits_get_hdu_num( fitsp_, &numHdu_ ) ;

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
  // release memory
  finalize() ;
}

// data initialization
void NROFITSDataset::initialize()
{
  // open file
  if ( open() ) 
    cerr << "NROFITSDataset::initialize()  error while opening file " << filename_ << endl ;

  readHeader( ARYNM, "ARYNM", same_ ) ;
  readHeader( scanNum_, "NAXIS2", same_ ) ;
  scanNum_ = scanNum_ + 1 ;
  readHeader( scanLen_, "NAXIS1", same_ ) ;
  int status = 0 ;
  long ltmp = 0 ;
  fits_get_num_rows( fitsp_, &ltmp, &status ) ;
  rowNum_ = ltmp ;
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
    //RX[i] = new char[16] ;
    //HORN[i] = new char[4] ;
    //POLTP[i] = new char[4] ;
    //SIDBD[i] = new char[4] ;
    //LAGWIND[i] = new char[8] ;
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
    + sizeof( int ) * ARYNM * 2            // ARRY, NFCAL
    + sizeof( double ) * ARYNM             // F0CAL
    + sizeof( double ) * ARYNM * 10 * 3    // FQCAL, CHCAL, CWCAL
    + sizeof( char ) * 180 ;                    // CDMY1
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
  delete CDMY1 ;
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

int NROFITSDataset::open()
{
  int status = 0 ;
  if ( !fitsp_ ) {
    fits_open_data( &fitsp_, filename_.c_str(), READONLY, &status ) ;
    if ( status )
      fits_report_error( stderr, status ) ;
  }
  return status ;
}

void NROFITSDataset::close()
{
  int status = 0 ;
  fits_close_file( fitsp_, &status ) ;
  if ( status ) 
    fits_report_error( stderr, status ) ;
}


int NROFITSDataset::fillHeader( int sameEndian )
{
  // fill array type
  fillARYTP() ;

  // read data header
  float ftmp = 0.0 ;
  char *ctmp = NULL ;
  //int itmp = 0 ;
  //double dtmp = 0.0 ;
  if ( readHeader( LOFIL, "LOFIL" ) != 0 ) {
    cerr << "Info: LOFIL set to FITS." << endl ;
    LOFIL = "FITS" ; 
  }
  // DEBUG
  //cout << "LOFIL = " << LOFIL << endl ;
  //
  if ( readHeader( VER, "VER" ) != 0 ) {
    if ( readHeader( VER, "HISTORY NEWSTAR VER" ) != 0 ) {
      cerr << "Info: VER set to V000." << endl ;
      VER = "V000" ;
    }
  }
  // DEBUG
  //cout << "VER = " << VER << endl ;
  //
  if ( readHeader( GROUP, "GROUP" ) != 0 ) {
    if ( readHeader( GROUP, "HISTORY NEWSTAR GROUP" ) != 0 ) {
      cerr << "Info: GROUP set to GRP0." << endl ;
      GROUP = "GRP0" ;
    }
  }
  // DEBUG
  //cout << "GROUP = " << GROUP << endl ;
  //
  if ( readHeader( PROJ, "PROJECT" ) != 0 ) {
    if ( readHeader( PROJ, "HISTORY NEWSTAR PROJECT" ) != 0 ) {
      cerr << "Info: PROJ set to PROJ0." << endl ;
      PROJ = "PROJ0" ;
    }
  }
  // DEBUG
  //cout << "PROJ = " << PROJ << endl ;
  //
  if ( readHeader( SCHED, "SCHED" ) != 0 ) {
    if ( readHeader( SCHED, "HISTORY NEWSTAR SCHED" ) != 0 ) {
      cerr << "Info: SCHED set to SCHED0." << endl ;
      SCHED = "SCHED0" ;
    }
  }
  // DEBUG
  //cout << "SCHED = " << SCHED << endl ;
  //
  if ( readHeader( OBSVR, "OBSERVER" ) != 0 ) {
    cerr << "Error while reading data OBSVR" << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "OBSVR = " << OBSVR << endl ;
  //
  if ( readHeader( LOSTM, "STRSC" ) != 0 ) {
    cerr << "Error while reading data LOSTM." << endl ;
    return -1 ;
  }
  if ( LOSTM[0] == '9' ) {
    ctmp = LOSTM ;
    LOSTM = "19" ;
    strcat( LOSTM, ctmp ) ;
  }
  else if ( LOSTM[0] == '0') {
    ctmp = LOSTM ;
    LOSTM = "20" ;
    strcat( LOSTM, ctmp ) ;
  } 
  // DEBUG
  //cout << "LOSTM = " << LOSTM << endl ;
  //
  if ( readHeader( LOETM, "STPSC" ) != 0 ) {
    cerr << "Error while reading data LOETM." << endl ;
    return -1 ;
  }
  if ( LOETM[0] == '9' ) {
    ctmp = LOETM ;
    LOETM = "19" ;
    strcat( LOETM, ctmp ) ;
  }
  else if ( LOETM[0] == '0') {
    ctmp = LOETM ;
    LOETM = "20" ;
    strcat( LOETM, ctmp ) ;
  }   // DEBUG
  //cout << "LOETM = " << LOETM << endl ;
  //
  if ( readHeader( ARYNM, "ARYNM", sameEndian ) != 0 ) {
    cerr << "Error while reading data ARYNM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "ARYNM = " << ARYNM << endl ;
  //
  if ( readHeader( NSCAN, "NAXIS2", sameEndian ) != 0 ) {
    cerr << "Error while reading data NSCAN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NSCAN = " << NSCAN << endl ;
  //
  if ( readHeader( TITLE, "TITLE" ) != 0 ) {
    if ( readHeader( TITLE, "HISTORY NEWSTAR TITLE1" ) != 0 
         && readHeader( ctmp, "HISTORY NEWSTAR TITLE2" ) != 0 ) {
        cerr << "Info: TITLE set to NOTITLE." << endl ;
        TITLE = "NOTITLE" ;
    }
    else {
      strcat( TITLE, ctmp ) ;
    }
  }
  // DEBUG
  //cout << "TITLE = " << TITLE << endl ;
  //
  if ( readHeader( OBJ, "OBJECT" ) != 0 ) {
    cerr << "Error while reading data OBJ." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "OBJ = " << OBJ << endl ;
  //
  if ( readHeader( ftmp, "EPOCH", sameEndian ) != 0 ) {
    cerr << "Error while reading data EPOCH." << endl ;
    return -1 ;
  }
  if ( ftmp == 1950.0 ) 
    EPOCH = "B1950" ;
  else if ( ftmp == 2000.0 ) 
    EPOCH = "J2000" ;
  else 
    EPOCH = "XXXXX" ;
  // DEBUG
  //cout << "EPOCH = " << EPOCH << endl ;
  //
  if ( readHeader( ctmp, "RA" ) != 0 ) {
    cerr << "Error while reading data RA0." << endl ;
    return -1 ;
  }
  RA0 = radRA( ctmp ) ;
  // DEBUG
  //cout << "RA0 = " << RA0 << endl ;
  //
  if ( readHeader( ctmp, "DEC" ) != 0 ) {
    cerr << "Error while reading data DEC0." << endl ;
    return -1 ;
  }
  DEC0 = radDEC( ctmp ) ;
  // DEBUG
  //cout << "DEC0 = " << DEC0 << endl ;
  //
  if ( readHeader( GLNG0, "GL0", sameEndian ) != 0 ) {
    cerr << "Error while reading data GLNG0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLNG0 = " << GLNG0 << endl ;
  //
  if ( readHeader( GLAT0, "GB0", sameEndian ) != 0 ) {
    cerr << "Error while reading data GLAT0." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLAT0 = " << GLAT0 << endl ;
  //
  if ( readHeader( NCALB, "NCALB", sameEndian ) != 0 ) {
    cerr << "Error while reading data NCALB." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "NCALB = " << NCALB << endl ;
  //
  if ( readHeader( SCNCD, "SCNCD", sameEndian ) != 0 ) {
    cerr << "Info: SCNCD set to 0 (RADEC)." << endl ;
    SCNCD = 0 ;
  }
  // DEBUG
  //cout << "SCNCD = " << SCNCD << endl ;
  //
  if ( readHeader( SCMOD, "SCMOD1" ) != 0 ) {
    cerr << "Error while reading data SCMOD." << endl ;
    return -1 ;
  }
  if ( readHeader( ctmp, "SCMOD2" ) == 0 ) {
    strcat( SCMOD, ctmp ) ;
  }
  if ( readHeader( ctmp, "SCMOD3" ) == 0 ) {
    strcat( SCMOD, ctmp ) ;
  }
  if ( readHeader( ctmp, "SCMOD4" ) == 0 ) {
    strcat( SCMOD, ctmp ) ;
  }
  if ( readHeader( ctmp, "SCMOD5" ) == 0 ) {
    strcat( SCMOD, ctmp ) ;
  }
  if ( readHeader( ctmp, "SCMOD6" ) == 0 ) {
    strcat( SCMOD, ctmp ) ;
  }
  // DEBUG
  //cout << "SCMOD = " << SCMOD << endl ;
  //
  if ( readHeader( URVEL, "VEL", sameEndian ) != 0 ) {
    cerr << "Error while reading data URVEL." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "URVEL = " << URVEL << endl ;
  //
  if ( readHeader( VREF, "VREF" ) != 0 ) {
    cerr << "Error while reading data VREF." << endl ;
    return -1 ;
  }
   // DEBUG
  //cout << "VREF = " << VREF << endl ;
  //
 if ( readHeader( VDEF, "VDEF" ) != 0 ) {
    cerr << "Error while reading data VDEF." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "VDEF = " << VDEF << endl ;
  //
  if ( readHeader( SWMOD, "SWMOD" ) != 0 ) {
    cerr << "Error while reading data SWMOD." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SWMOD = " << SWMOD << endl ;
  //
  if ( readHeader( FRQSW, "FRQSW", sameEndian ) != 0 ) {
    cerr << "Error while reading data FRQSW." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "FRQSW = " << FRQSW << endl ;
  //
  if ( readHeader( DBEAM, "DBEAM", sameEndian ) != 0 ) {
    cerr << "Error while reading data DBEAM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "DBEAM = " << DBEAM << endl ;
  //
  if ( readHeader( MLTOF, "MLTOF", sameEndian ) != 0 ) {
    cerr << "Info: MLTOF set to 0." << endl ;
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
    cerr << "Error while reading data CMTE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTE = " << CMTE << endl ;
  //
  if ( readHeader( CMTSOM, "CMTSOM", sameEndian ) != 0 ) {
    cerr << "Error while reading data CMTSOM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTSOM = " << CMTSOM << endl ;
  //
  if ( readHeader( CMTNODE, "CMTNODE", sameEndian ) != 0 ) {
    cerr << "Error while reading data CMTNODE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTNODE = " << CMTNODE << endl ;
  //
  if ( readHeader( CMTI, "CMTI", sameEndian ) != 0 ) {
    cerr << "Error while reading data CMTI." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTI = " << CMTI << endl ;
  //
  if ( readHeader( CMTTM, "CMTTM" ) != 0 ) {
    cerr << "Error while reading data CMTTM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTTM = " << CMTTM << endl ;
  //
  if ( readHeader( SBDX, "SDBX", sameEndian ) != 0 ) {
    cerr << "Error while reading data SBDX." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDX = " << SBDX << endl ;
  //
  if ( readHeader( SBDY, "SDBY", sameEndian ) != 0 ) {
    cerr << "Error while reading data SBDY." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDY = " << SBDY << endl ;
  //
  if ( readHeader( SBDZ1, "SDBZ1", sameEndian ) != 0 ) {
    cerr << "Error while reading data SBDZ1." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ1 = " << SBDZ1 << endl ;
  //
  if ( readHeader( SBDZ2, "SDBZ2", sameEndian ) != 0 ) {
    cerr << "Error while reading data SBDZ2." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ2 = " << SBDZ2 << endl ;
  //
  if ( readHeader( DAZP, "DAZP", sameEndian ) != 0 ) {
    cerr << "Info: DAZP set to 0." << endl ;
    DAZP = 0.0 ;
  }
  // DEBUG
  //cout << "DAZP = " << DAZP << endl ;
  //
  if ( readHeader( DELP, "DELP", sameEndian ) != 0 ) {
    cerr << "Info: DELP set to 0." << endl ;
    DELP = 0.0 ;
  }
  // DEBUG
  //cout << "DELP = " << DELP << endl ;
  //
  if ( readHeader( CHBIND, "CHBIND", sameEndian ) != 0 ) {
    cerr << "Info: CHBIND set to 1." << endl ;
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
    cerr << "Info: CHMIN set to 1." << endl ;
    CHMIN = 1 ;
  }
  // DEBUG
  //cout << "CHMIN = " << CHMIN << endl ;
  //
  if ( readHeader( CHMAX, "CHMAX", sameEndian ) != 0 ) {
    cerr << "Info: CHMAX set to 2048." << endl ;
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
  if ( readHeader( IPTIM, "INTEG", sameEndian ) != 0 ) {
    cerr << "Error while reading data IPTIM." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "IPTIM = " << IPTIM << endl ;
  //
  if ( readHeader( PA, "PA", sameEndian ) != 0 ) {
    if ( readTable( PA, "PA", sameEndian ) != 0 ) {
      cerr << "Error while reading data PA." << endl ;
      return -1 ;
    }
  }
  // DEBUG
  //cout << "PA = " << PA << endl ;
  //
  if ( readTable( RX, "RX" ) != 0 ) {
    cerr << "Error while reading data RX." << endl ;
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
  //cout << endl ;
  if ( readTable( HPBW, "HPBW", sameEndian ) != 0 ) {
    cerr << "Error while reading data HPBW." << endl ;
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
  //cout << endl ;
  if ( readTable( EFFA, "EFFA", sameEndian ) != 0 ) {
    cerr << "Error while reading data EFFA." << endl ;
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
  //cout << endl ;
  if ( readTable( EFFB, "EFFB", sameEndian ) != 0 ) {
    cerr << "Error while reading data EFFB." << endl ;
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
  //cout << endl ;
  if ( readTable( EFFL, "EFFL", sameEndian ) != 0 ) {
    cerr << "Error while reading data EFFL." << endl ;
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
  //cout << endl ;
  if ( readTable( EFSS, "EFSS", sameEndian ) != 0 ) {
    cerr << "Error while reading data EFSS." << endl ;
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
  //cout << endl ;
  if ( readTable( GAIN, "GAIN", sameEndian ) != 0 ) {
    cerr << "Error while reading data GAIN." << endl ;
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
  //cout << endl ;
  if ( readTable( HORN, "HORN" ) != 0 ) {
    cerr << "Error while reading data HORN." << endl ;
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
  //cout << endl ;
  if ( readTable( POLTP, "POLTP" ) != 0 ) {
    cerr << "Error while reading data POLTP." << endl ;
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
  //cout << endl ;
  if ( readTable( POLDR, "POLDR", sameEndian ) != 0 ) {
    cerr << "Error while reading data POLDR." << endl ;
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
  //cout << endl ;
  if ( readTable( POLAN, "POLAN", sameEndian ) != 0 ) {
    cerr << "Error while reading data POLAN." << endl ;
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
  //cout << endl ;
  if ( readTable( DFRQ, "DFRQ", sameEndian ) != 0 ) {
    cerr << "Error while reading data DFRQ." << endl ;
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
  //cout << endl ;
  if ( readTable( SIDBD, "SIDBD" ) != 0 ) {
    cerr << "Error while reading data SIDBD." << endl ;
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
  //cout << endl ;
  if ( readTable( REFN, "REFN", sameEndian ) != 0 ) {
    cerr << "Error while reading data REFN." << endl ;
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
  //cout << endl ;
  if ( readTable( IPINT, "IPINT", sameEndian ) != 0 ) {
    cerr << "Error while reading data IPINT." << endl ;
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
  //cout << endl ;
  if ( readTable( MULTN, "MULTN", sameEndian ) != 0 ) {
    cerr << "Error while reading data MULTN." << endl ;
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
  //cout << endl ;
  if ( readTable( MLTSCF, "MLTSCF", sameEndian ) != 0 ) {
    cerr << "Error while reading data MLTSCF." << endl ;
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
  //cout << endl ;
  if ( readTable( LAGWIND, "LAGWIN" ) != 0 ) {
    cerr << "Error while reading data LAGWIND." << endl ;
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
  //cout << endl ;
  if ( readTable( BEBW, "BEBW", sameEndian ) != 0 ) {
    cerr << "Error while reading data BEBW." << endl ;
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
  //cout << endl ;
  if ( readTable( BERES, "BERES", sameEndian ) != 0 ) {
    cerr << "Error while reading data BERES." << endl ;
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
  //cout << endl ;
  if ( readTable( CHWID, "CHWID", sameEndian ) != 0 ) {
    cerr << "Error while reading data CHWID." << endl ;
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
  //cout << endl ;
  if ( readARRY() != 0 ) {
    cerr << "Error while reading data ARRY." << endl ;
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
  //cout << endl ;
  if ( readTable( NFCAL, "NFCAL", sameEndian ) != 0 ) {
    cerr << "Error while reading data NFCAL." << endl ;
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
  //cout << endl ;
  if ( readTable( F0CAL, "F0CAL", sameEndian ) != 0 ) {
    cerr << "Error while reading data F0CAL." << endl ;
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
  //cout << endl ;
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readTable( vv, "FQCAL", sameEndian, i ) != 0 ) {
      cerr << "Error while reading data FQCAL." << endl ;
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
    if ( readTable( vv, "CHCAL", sameEndian, i ) != 0 ) {
      cerr << "Error while reading data CHCAL." << endl ;
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
//  }
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readTable( vv, "CWCAL", sameEndian, i ) != 0 ) {
      cerr << "Error while reading data CWCAL." << endl ;
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
  if ( readHeader( SCNLEN, "NAXIS1", sameEndian ) != 0 ) {
    cerr << "Error while reading data SCNLEN." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNLEN = " << SCNLEN << endl ;
  //
  if ( readHeader( SBIND, "SBIND", sameEndian ) != 0 ) {
    cerr << "Info: SBIND set to 0." << endl ;
    SBIND = 0 ;
  }
   // DEBUG
  //cout << "SBIND = " << SBIND << endl ;
  //
  if ( readHeader( IBIT, "IBIT", sameEndian ) != 0 ) {
    cerr << "Info: IBIT set to 0." << endl ;
    IBIT = 0 ;
  }
  // DEBUG
  //cout << "IBIT = " << IBIT << endl ;
  //
  if ( readHeader( SITE, "TELESCOP" ) != 0 ) {
    cerr << "Error while reading data SITE." << endl ;
    return -1 ;
  }
  // DEBUG
  //cout << "SITE = " << SITE << endl ;
  //
  if ( readHeader( CDMY1, "CDMY1" ) != 0 ) {
    cerr << "Info: CDMY1 set to \"\"." << endl ;
    CDMY1 = "" ;
  }
  // DEBUG
  //cout << "CDMY1 = " << CDMY1 << endl ;
  //
  if ( readTable( DSBFC, "DSBFC", sameEndian ) != 0 ) {
    cerr << "Info: DSBFC set to 1." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      DSBFC[i] = 1.0 ;
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
  //cout << endl ;

  return 0 ;
}

int NROFITSDataset::fillRecord( int i )
{
  int status = 0 ;
  status = readColumn( record_->LSFIL, "LSFIL", i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading LSFIL." << endl ;
    return status ;
  }
  status = readColumn( record_->ISCAN, "ISCN", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading ISCAN." << endl ;
    return status ;
  }
  vector<int> itmp( 6, 0 ) ;
  status = readColumn( itmp, "LAVST", same_, i, 6 ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading LAVST." << endl ;
    return status ;
  }
  else {
    sprintf( record_->LAVST, "%d%d%d%d%d%d.000", itmp[0], itmp[1], itmp[2], itmp[3], itmp[4], itmp[5] ) ;
  }
  status = readColumn( record_->SCANTP, "SCNTP", i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading SCANTP." << endl ;
    return status ;
  }
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
  status = readColumn( record_->DSCX, name1, same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading DSCX." << endl ;
    return status ;
  }
  status = readColumn( record_->DSCY, name2, same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading DSCY." << endl ;
    return status ;
  }
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
  status = readColumn( record_->SCX, name1, same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading SCX." << endl ;
    return status ;
  }
  status = readColumn( record_->SCY, name2, same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading SCY." << endl ;
    return status ;
  }
  status = readColumn( record_->PAZ, "PAZ", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading PAZ." << endl ;
    return status ;
  }
  status = readColumn( record_->PEL, "PEL", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading PEL." << endl ;
    return status ;
  }
  status = readColumn( record_->RAZ, "RAZ", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading RAZ." << endl ;
    return status ;
  }
  status = readColumn( record_->REL, "REL", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading REL." << endl ;
    return status ;
  }
  status = readColumn( record_->XX, "XX", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading XX." << endl ;
    return status ;
  }
  status = readColumn( record_->YY, "YY", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading YY." << endl ;
    return status ;
  }
  status = readColumn( record_->ARRYT, "ARRYT", i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading ARRYT." << endl ;
    return status ;
  }
  double dtmp ;
  status = readColumn( dtmp, "TEMP", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading TEMP." << endl ;
    return status ;
  }
  else {
    record_->TEMP = dtmp ;
  }
  status = readColumn( dtmp, "PATM", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading PATM." << endl ;
    return status ;
  }
  else {
    record_->PATM = dtmp ;
  }
  status = readColumn( dtmp, "PH2O", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading PH2O." << endl ;
    return status ;
  }
  else {
    record_->PH2O = dtmp ;
  }
  status = readColumn( dtmp, "VWIND", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading VWIND." << endl ;
    return status ;
  }
  else {
    record_->VWIND = dtmp ;
  }
  status = readColumn( dtmp, "DWIND", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading DWIND." << endl ;
    return status ;
  }
  else {
    record_->DWIND = dtmp ;
  }
  status = readColumn( dtmp, "TAU", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading TAU." << endl ;
    return status ;
  }
  else {
    record_->TAU = dtmp ;
  }
  status = readColumn( dtmp, "TSYS", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading TSYS." << endl ;
    return status ;
  }
  else {
    record_->TSYS = dtmp ;
  }
  status = readColumn( dtmp, "BATM", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading BATM." << endl ;
    return status ;
  }
  else {
    record_->BATM = dtmp ;
  }
  status = readColumn( record_->VRAD, "VRAD", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading TEMP." << endl ;
    return status ;
  }
  status = readColumn( record_->FREQ0, "FRQ0", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading FREQ0." << endl ;
    return status ;
  }
  status = readColumn( record_->FQTRK, "FQTRK", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading FQTRK." << endl ;
    return status ;
  }
  status = readColumn( record_->FQIF1, "FQIF1", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading FQIF1." << endl ;
    return status ;
  }
  status = readColumn( record_->ALCV, "ALCV", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading ALCV." << endl ;
    return status ;
  }
  record_->IDMY0 = 0 ;
  status = readColumn( record_->DPFRQ, "DPFRQ", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord()  DPFRQ set to 0." << endl ;
    record_->DPFRQ = 0.0 ;
  }
  status = readColumn( record_->SFCTR, "SFCTR", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading SFCTR." << endl ;
    return status ;
  }
  status = readColumn( record_->ADOFF, "ADOFF", same_, i ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading ADOFF." << endl ;
    return status ;
  }
  status = readColumn( record_->JDATA, "LDATA", same_, i, chmax_ ) ;
  if ( status ) {
    cerr << "NROFITSDataset::fillRecord() while reading JDATA." << endl ;
    return status ;
  }
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
  int index = -1 ;
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    if ( str == ARYTP[i] ) {
      index = i ;
      break ;
    }
  }
  return index ;
}

int NROFITSDataset::readHeader( char *v, char *name ) 
{
  int status = 0 ;
  char *comment = NULL ;
  void *value = NULL ;
  fits_read_key( fitsp_, TSTRING, name, value, comment, &status ) ;
  v = (char *)value ;

  if ( status )
    fits_report_error( stderr, status ) ;

  return status ;
}

int NROFITSDataset::readHeader( int &v, char *name, int b ) 
{
  int status = 0 ;
  char *comment = NULL ;
  void *value = NULL ;
  fits_read_key( fitsp_, TINT, name, value, comment, &status ) ;
  v = *((int *)value) ;

  if ( status ) 
    fits_report_error( stderr, status ) ;

  if ( b == 0 )
    convertEndian( v ) ;

  return status ;
}


int NROFITSDataset::readHeader( float &v, char *name, int b ) 
{
  int status = 0 ;
  char *comment = NULL ;
  void *value = NULL ;
  fits_read_key( fitsp_, TFLOAT, name, value, comment, &status ) ;
  v = *((float *)value) ;

  if ( status ) 
    fits_report_error( stderr, status ) ;

  if ( b == 0 )
    convertEndian( v ) ;

  return status ;
}

int NROFITSDataset::readHeader( double &v, char *name, int b ) 
{
  int status = 0 ;
  char *comment = NULL ;
  void *value = NULL ;
  fits_read_key( fitsp_, TDOUBLE, name, value, comment, &status ) ;
  v = *((double *)value) ;

  if ( status ) 
    fits_report_error( stderr, status ) ;

  if ( b == 0 )
    convertEndian( v ) ;

  return status ;
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
    if ( i < 10 ) {
      key1 = new char[6] ;
      sprintf( key1, "TFORM%d", i ) ;
      key2 = new char[6] ;
      sprintf( key2, "TTYPE%d", i ) ;
      key3 = new char[6] ;
      sprintf( key3, "TUNIT%d", i ) ;
    }
    else if ( i < 100 ) {
      key1 = new char[7] ;
      sprintf( key1, "TFORM%2d", i ) ;
      key2 = new char[7] ;
      sprintf( key2, "TTYPE%d", i ) ;
      key3 = new char[7] ;
      sprintf( key3, "TUNIT%d", i ) ;
    }
    else {
      key1 = new char[8] ;
      sprintf( key1, "TFORM%3d", i ) ;
      key2 = new char[8] ;
      sprintf( key2, "TTYPE%d", i ) ; 
      key3 = new char[7] ;
      sprintf( key3, "TUNIT%d", i ) ;
    }
    char *tmp ;
    if ( readHeader( tmp, key1 ) != 0 ) {
      cerr << "Error while reading field keyword for scan header." << endl ;
      delete key1 ;
      delete key2 ;
      delete key3 ;
      return ;
    }
    names_[i] = string( tmp ) ;
    if ( readHeader( tmp, key2 ) != 0 ) {
      cerr << "Error while reading field type for scan header." << endl ;
      delete key1 ;
      delete key2 ;
      delete key3 ;
      return ;
    }
    types_[i] = string( tmp ) ;
    if ( readHeader( tmp, key3 ) != 0 ) {
      tmp = "none" ;
    }
    units_[i] = string( tmp ) ;
    delete key1 ;
    delete key2 ;
    delete key3 ;
  }  
}

void NROFITSDataset::fillARYTP()
{
  char *nulval = NULLVAL_CHAR;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, "ARRYT", &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() failed to fill ARRYTP." << endl ;
    return ;
  }
  char **array = new char*[rowNum_] ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) 
    array[i] = new char[4] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TSTRING, colnum, 1, 1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status )
    fits_report_error( stderr, status ) ;

  int n = 0 ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    if ( n == 0 && strcmp( array[i], NULLVAL_CHAR ) == 0 ) {
      ARYTP[n] = array[i] ;
      arrayid_[n] = i ;
      n++ ;
    }
    else {
      int count = 0 ;
      for ( int j = 0 ; j < n ; j++ ) {
        if ( strcmp( array[i], ARYTP[j].c_str() ) == 0 ) 
          count++ ;
      }
      if ( count == 0 && strcmp( array[i], NULLVAL_CHAR ) == 0 ) {
        ARYTP[n] = array[i] ;
        arrayid_[n] = i ;
        n++ ;
      }
    }
    if ( n == ARYNM ) 
      break ;
  }
}

int NROFITSDataset::readTable( char *v, char *name ) 
{
  char *nulval = NULLVAL_CHAR;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    v = nulval ;
    return status ;
  }
  char **array = new char*[rowNum_] ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) 
    array[i] = new char[20] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TSTRING, colnum, 1, 1, 1, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  else {
    v = array[0] ;
  }
  for ( int i = 0 ; i < rowNum_ ; i++ ) 
    delete array[i] ;
  delete array ;
  return status ;
}

int NROFITSDataset::readTable( int &v, char *name, int b ) 
{
  int nulval = NULLVAL_INT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    v = nulval ;
    return status ;
  }
  int *array = new int[rowNum_] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TINT, colnum, 1, 1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  vector<int> arr( rowNum_, 0 ) ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    arr[i] = array[i] ;
    if ( b == 0 ) 
      convertEndian( arr[i] ) ;
  }
  v = *max_element( arr.begin(), arr.end() ) ;
  delete array ;
  return status ;
}

int NROFITSDataset::readTable( float &v, char *name, int b ) 
{
  float nulval = NULLVAL_FLOAT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    v = nulval ;
    return status ;
  }
  float *array = new float[rowNum_] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TFLOAT, colnum, 1, 1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  vector<float> arr( rowNum_, 0 ) ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    arr[i] = array[i] ;
    if ( b == 0 )
      convertEndian( arr[i] ) ;
  }
  v = *max_element( arr.begin(), arr.end() ) ;
  delete array ;
  return status ;
}

int NROFITSDataset::readTable( double &v, char *name, int b ) 
{
  double nulval = NULLVAL_DOUBLE ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    v = nulval ;
    return status ;
  }
  double *array = new double[rowNum_] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TFLOAT, colnum, 1, 1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  vector<double> arr( rowNum_, 0 ) ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    arr[i] = array[i] ;
    if ( b == 0 ) 
      convertEndian( arr[i] ) ;
  }
  v = *max_element( arr.begin(), arr.end() ) ;
  delete array ;
  return status ;
}

int NROFITSDataset::readTable( vector<char *> &v, char *name )
{
  int status = readTable( v, name, 0 ) ;
  return status ;
}

int NROFITSDataset::readTable( vector<char *> &v, char *name, long idx )
{
  vector<char *> vv( rowNum_, "" ) ;
  int status = readColumn( vv, name, idx ) ;
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    v[i] = vv[arrayid_[i]] ;
  }
  return status ;
}

int NROFITSDataset::readTable( vector<int> &v, char *name, int b ) 
{
  int status = readTable( v, name, b, 0 ) ;
  return status ;
}

int NROFITSDataset::readTable( vector<int> &v, char *name, int b, long idx )
{
  vector<int> vv( rowNum_, 0 ) ;
  int status = readColumn( vv, name, b, idx ) ;
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    v[i] = vv[arrayid_[i]] ;
  }
  return status ;
}

int NROFITSDataset::readTable( vector<float> &v, char *name, int b ) 
{
  int status = readTable( v, name, b, 0 ) ;
  return status ;
}

int NROFITSDataset::readTable( vector<float> &v, char *name, int b, long idx ) 
{
  vector<float> vv( rowNum_, 0 ) ;
  int status = readColumn( vv, name, b, idx ) ;
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    v[i] = vv[arrayid_[i]] ;
  }
  return status ;
}

int NROFITSDataset::readTable( vector<double> &v, char *name, int b ) 
{
  int status = readTable( v, name, b, 0 ) ;
  return status ;
}

int NROFITSDataset::readTable( vector<double> &v, char *name, int b, long idx ) 
{
  vector<double> vv( rowNum_, 0 ) ;
  int status = readColumn( vv, name, b, idx ) ;
  for ( int i = 0 ; i < ARYNM ; i++ ) {
    v[i] = vv[arrayid_[i]] ;
  }
  return status ;
}

int NROFITSDataset::readARRY()
{
  char *arry1 = new char[20] ;
  char *arry2 = new char[20] ;
  char *arry3 = new char[20] ;
  char *arry4 = new char[20] ;
  int status = readHeader( arry1, "ARRY1" ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  status = readHeader( arry2, "ARRY2" ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  status = readHeader( arry3, "ARRY3" ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    delete arry1 ;
    delete arry2 ;
    delete arry3 ;
    delete arry4 ;
    return status ;
  }
  status = readHeader( arry4, "ARRY4" ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
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

int NROFITSDataset::readColumn( vector<char *> &v, char *name, long idx ) 
{
  char *nulval = NULLVAL_CHAR;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i ++ ) 
      v[i] = nulval ;
    return status ;
  }
  char **array = new char*[rowNum_] ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) 
    array[i] = new char[20] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TSTRING, colnum, 1, idx+1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < rowNum_ ; i++ ) 
      v[i] = nulval ;
  }
  else {
    for ( int i = 0 ; i < rowNum_ ; i++ ) 
      v[i] = array[i] ;
  }
  for ( int i = 0 ; i < rowNum_ ; i++ ) 
    delete array[i] ;
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( vector<int> &v, char *name, int b, long idx ) 
{
  int nulval = NULLVAL_INT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  int *array = new int[rowNum_] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TINT, colnum, 1, idx+1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < rowNum_ ; i++ ) 
      v[i] = nulval ;
  }
  else {
    for ( int i = 0 ; i < rowNum_ ; i++ ) {
      v[i] = array[i] ;
      if ( b == 0 ) 
        convertEndian( v[i] ) ;
    }
  }
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( vector<float> &v, char *name, int b, long idx ) 
{
  float nulval = NULLVAL_FLOAT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  float *array = new float[rowNum_] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TFLOAT, colnum, 1, idx+1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < rowNum_ ; i++ ) 
      v[i] = nulval ;
  }
  else {
    for ( int i = 0 ; i < rowNum_ ; i++ ) {
      v[i] = array[i] ;
      if ( b == 0 ) 
        convertEndian( v[i] ) ;
    }
  }
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( vector<double> &v, char *name, int b, long idx ) 
{
  double nulval = NULLVAL_DOUBLE ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  double *array = new double[rowNum_] ;
  int anynul = 0 ;
  fits_read_col( fitsp_, TDOUBLE, colnum, 1, idx+1, rowNum_, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < rowNum_ ; i++ ) 
      v[i] = nulval ;
  }
  else {
    for ( int i = 0 ; i < rowNum_ ; i++ ) {
      v[i] = array[i] ;
      if ( b == 0 ) 
        convertEndian( v[i] ) ;
    }
  }
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( char *v, char *name, long irow )
{
  char *nulval = NULLVAL_CHAR ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v = nulval ;
    return status ;
  }
  int anynul = 0 ;
  fits_read_col( fitsp_, TSTRING, colnum, irow, 1, 1, &nulval, v, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  return status ;
}

int NROFITSDataset::readColumn( int &v, char *name, int b, long irow )
{
  int nulval = NULLVAL_INT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v = nulval ;
    return status ;
  }
  int anynul = 0 ;
  fits_read_col( fitsp_, TINT, colnum, irow, 1, 1, &nulval, &v, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  return status ;
}

int NROFITSDataset::readColumn( float &v, char *name, int b, long irow )
{
  float nulval = NULLVAL_FLOAT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v = nulval ;
    return status ;
  }
  int anynul = 0 ;
  fits_read_col( fitsp_, TFLOAT, colnum, irow, 1, 1, &nulval, &v, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  return status ;
}

int NROFITSDataset::readColumn( double &v, char *name, int b, long irow )
{
  double nulval = NULLVAL_DOUBLE ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      v = nulval ;
    return status ;
  }
  int anynul = 0 ;
  fits_read_col( fitsp_, TDOUBLE, colnum, irow, 1, 1, &nulval, &v, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    v = nulval ;
  }
  return status ;
}

int NROFITSDataset::readColumn( vector<char *> &v, char *name, long irow, long nelem )
{
  char *nulval = NULLVAL_CHAR ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < nelem ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  int anynul = 0 ;
  char **array = new char*[nelem] ;
  for ( int i = 0 ; i < nelem ; i++ ) 
    array[i] = new char[20] ;  
  fits_read_col( fitsp_, TSTRING, colnum, irow, 1, nelem, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = nulval ;
      delete array[i] ;
    }
  }
  else {
    for ( int i = 0 ; i < nelem ; i++ ) {
      strcpy( v[i], array[i] ) ;
      delete array[i] ;
    }
  }
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( vector<int> &v, char *name, int b, long irow, long nelem )
{
  int nulval = NULLVAL_INT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < nelem ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  int anynul = 0 ;
  int *array = new int[nelem] ;
  fits_read_col( fitsp_, TINT, colnum, irow, 1, nelem, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = nulval ;
    }
  }
  else {
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = array[i] ;
    }
  }
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( vector<float> &v, char *name, int b, long irow, long nelem )
{
  float nulval = NULLVAL_FLOAT ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < nelem ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  int anynul = 0 ;
  float *array = new float[nelem] ;
  fits_read_col( fitsp_, TFLOAT, colnum, irow, 1, nelem, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = nulval ;
    }
  }
  else {
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = array[i] ;
    }
  }
  delete array ;
  return status ;
}

int NROFITSDataset::readColumn( vector<double> &v, char *name, int b, long irow, long nelem )
{
  double nulval = NULLVAL_DOUBLE ;
  int status = 0 ;
  int colnum = -1 ;
  fits_get_colnum( fitsp_, CASEINSEN, name, &colnum, &status ) ; 
  if ( status == COL_NOT_FOUND ) {
    cerr << "NROFITSDataset::readTable() " << name << " not found." << endl ;
    for ( int i = 0 ; i < nelem ; i++ ) 
      v[i] = nulval ;
    return status ;
  }
  int anynul = 0 ;
  double *array = new double[nelem] ;
  fits_read_col( fitsp_, TDOUBLE, colnum, irow, 1, nelem, &nulval, array, &anynul, &status ) ;
  if ( status ) {
    fits_report_error( stderr, status ) ;
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = nulval ;
    }
  }
  else {
    for ( int i = 0 ; i < nelem ; i++ ) {
      v[i] = array[i] ;
    }
  }
  delete array ;
  return status ;
}

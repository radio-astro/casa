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
#include <scimath/Mathematics/InterpolateArray1D.h>

#include <iostream>
#include <fstream>
#include <casa/math.h>
#include <casa/iomanip.h>

using namespace std ;

//header size (8*2880bytes)
#define FITS_HEADER_SIZE 23040

// constructor 
NROFITSDataset::NROFITSDataset( string name )
  : NRODataset( name ) 
{
  LogIO os( LogOrigin( "NROFITSDataset", "NROFITSDataset()", WHERE ) ) ;

  fp_ = NULL ;
  dataid_ = -1 ;
  record_ = new NRODataRecord() ;
  record_->LDATA = NULL ; // never use LDATA 

  // open file
  if ( open() ) 
    os << LogIO::SEVERE << "error while opening file " << filename_ << LogIO::EXCEPTION ;
 
  // data initialization
  readHeader( numField_, "TFIELDS", same_ ) ;
  forms_.resize( numField_ ) ;
  names_.resize( numField_ ) ;
  units_.resize( numField_ ) ;
  getField() ;

  // check endian
  // FITS file is always BIG_ENDIAN, but it is not true for NRO data
  vector<int> itmp( 6 ) ;
  if ( readTable( itmp, "LAVST", true, 0 ) != 0 ) {
    os << LogIO::WARN << "Error while checking endian." << LogIO::POST ;
    return ;
  }
//   os << "itmp = " << itmp[0] << " " 
//      << itmp[1] << " " << itmp[2] << " " 
//      << itmp[3] << " " << itmp[4] << " " 
//      << itmp[5] << LogIO::POST ;
  // check endian by looking month value in LAVST (start time)
  if ( itmp[1] > 0 && itmp[1] < 13 ) {
    same_ = 1 ;
    os << LogIO::NORMAL << "same endian " << LogIO::POST ;
  }
  else {
    same_ = 0 ;
    os << LogIO::NORMAL << "different endian " << LogIO::POST ;
  }

  // memory allocation
  initialize() ;
}

// destructor 
NROFITSDataset::~NROFITSDataset() 
{
  // close file
  close() ;
}

// data initialization
void NROFITSDataset::initialize()
{
  LogIO os( LogOrigin( "NROFITSDataset", "initialize()", WHERE ) ) ;

  int status = 0 ;
  status = readHeader( ARYNM, "ARYNM", same_ ) ;
  if ( status != 0 ) 
    ARYNM = 1 ;
  readHeader( rowNum_, "NAXIS2", same_ ) ;
  readHeader( scanLen_, "NAXIS1", same_ ) ;
  status = 0 ;
  scanNum_ = rowNum_ / ARYNM ;
  int nchan = 0 ;
  if ( readTable( nchan, "NCH", same_ ) != 0 ) {
    os << LogIO::WARN << "Error while checking maximum channel number." << LogIO::POST ;
    return ;
  }
  chmax_ = nchan ;
  datasize_ = sizeof( int ) * chmax_ ;
  //record_->JDATA.resize( chmax_ ) ;
  JDATA.resize( chmax_ ) ;
  // zero clear
  for ( uInt i = 0 ; i < JDATA.size() ; i++ ) 
    JDATA[i] = 0 ;

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
  NFCAL.resize( ARYNM ) ;
  F0CAL.resize( ARYNM ) ;
  FQCAL.resize( ARYNM ) ;
  CHCAL.resize( ARYNM ) ;
  CWCAL.resize( ARYNM ) ;
  DSBFC.resize( ARYNM ) ;

  ARYTP.resize( ARYNM ) ;
  arrayid_.resize( ARYNM ) ;
  for ( int i = 0 ; i < ARYNM ; i++ ) 
    arrayid_[i] = -1 ;

  for ( int i = 0 ; i < ARYNM ; i++ ) {
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

  refFreq_.resize( ARYNM, 0.0 ) ;
}

// fill data header
int NROFITSDataset::fillHeader() 
{
  LogIO os( LogOrigin( "NROFITSDataset", "fillHeader()", WHERE ) ) ;

  // open file
  if ( open() ) {
    os << LogIO::SEVERE << "Error opening file " << filename_ << "." << LogIO::EXCEPTION ;
    return -1 ;
  }

  // fill
  int status = fillHeader( same_ ) ;

  return status ;
}

int NROFITSDataset::fillHeader( int sameEndian )
{
  LogIO os( LogOrigin( "NROFITSDataset", "fillHeader()", WHERE ) ) ;

  // fill array type
  fillARYTP() ;

  // read data header
  float ftmp = 0.0 ;
  if ( readHeader( LOFIL, "LOFIL" ) != 0 ) {
    os << LogIO::NORMAL << "LOFIL set to 'FITS'." << LogIO::POST ;
    LOFIL = "FITS" ;
  }
  // DEBUG
  //cout << "LOFIL = \'" << LOFIL << "\'" << endl ;
  //
  if ( readHeader( VER, "VER" ) != 0 ) {
    if ( readHeader( VER, "HISTORY NEWSTAR VER" ) != 0 ) {
      os << LogIO::NORMAL << "VER set to 'V000'." << LogIO::POST ;
      VER = "V000" ;
    }
  }
  // DEBUG
  //cout << "VER = \'" << VER << "\'" << endl ;
  //
  if ( readHeader( GROUP, "GROUP" ) != 0 ) {
    if ( readHeader( GROUP, "HISTORY NEWSTAR GROUP" ) != 0 ) {
      os << LogIO::NORMAL << "GROUP set to 'GRP0'." << LogIO::POST ;
      GROUP = "GROUP0" ;
    }
  }
  // DEBUG
  //cout << "GROUP = \'" << GROUP << "\'" << endl ;
  //
  if ( readHeader( PROJ, "PROJECT" ) != 0 ) {
    if ( readHeader( PROJ, "HISTORY NEWSTAR PROJECT" ) != 0 ) {
      os << LogIO::NORMAL << "PROJ set to 'PROJ0'." << LogIO::POST ;
      PROJ = "PROJECT0" ;
    }
  }
  // DEBUG
  //cout << "PROJ = \'" << PROJ << "\'" << endl ;
  //
  if ( readHeader( SCHED, "SCHED" ) != 0 ) {
    if ( readHeader( SCHED, "HISTORY NEWSTAR SCHED" ) != 0 ) {
      os << LogIO::NORMAL << "SCHED set to 'SCHED0'." << LogIO::POST ;
      SCHED = "SCHED0" ;
    }
  }
  // DEBUG
  //cout << "SCHED = \'" << SCHED << "\'" << endl ;
  //
  if ( readHeader( OBSVR, "OBSERVER" ) != 0 ) {
    os << LogIO::NORMAL << "OBSVR set to 'SOMEONE'" << LogIO::POST ;
    OBSVR = "SOMEONE" ;
  }
  // DEBUG
  //cout << "OBSVR = \'" << OBSVR << "\'" << endl ;
  //
  if ( readHeader( LOSTM, "STRSC" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data LOSTM." << LogIO::POST ;
    return -1 ;
  }
  if ( LOSTM[0] == '9' ) {
    LOSTM = "19" + LOSTM ;
  }
  else if ( LOSTM[0] == '0') {
    LOSTM = "20" + LOSTM ;
  } 
  // DEBUG
  //cout << "LOSTM = \'" << LOSTM << "\'" << endl ;
  //
  if ( readHeader( LOETM, "STPSC" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data LOETM." << LogIO::POST ;
    return -1 ;
  }
  if ( LOETM[0] == '9' ) {
    LOETM = "19" + LOETM ;
  }
  else if ( LOETM[0] == '0') {
    LOETM = "20" + LOETM ;
  }   
  // DEBUG
  //cout << "LOETM = \'" << LOETM << "\'" << endl ;
  //
  if ( readHeader( NSCAN, "NAXIS2", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data NSCAN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "NSCAN = " << NSCAN << endl ;
  //
  string subt ;
  if ( readHeader( TITLE, "TITLE" ) != 0 ) {
    int stat1 = readHeader( TITLE, "HISTORY NEWSTAR TITLE1" ) ;
    int stat2 = readHeader( subt, "HISTORY NEWSTAR TITLE2" ) ;
    if ( stat1 != 0 && stat2 != 0 ) {
      os << LogIO::NORMAL << "TITLE set to 'NOTITLE'." << LogIO::POST ;
      TITLE = "NOTITLE" ;
    }
    else {
      //cout << "TITLE = \'" << TITLE << "\'" << endl ;
      //cout << "subt  = \'" << subt << "\'" << endl ;
      TITLE = TITLE + subt ;
    }
  }
  // DEBUG
  //cout << "TITLE = \'" << TITLE << "\'" << endl ;
  //
  if ( readHeader( OBJ, "OBJECT" ) != 0 ) {
    os << LogIO::NORMAL << "OBJ set to 'SOMEWHERE'." << LogIO::POST ;
    OBJ = "NOOBJ" ;
  }
  // DEBUG
  //cout << "OBJ = \'" << OBJ << "\'" << endl ;
  //
  if ( readHeader( ftmp, "EPOCH", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data EPOCH." << LogIO::POST ;
    return -1 ;
  }
  if ( ftmp == 1950.0 ) 
    EPOCH = "B1950" ;
  else if ( ftmp == 2000.0 ) 
    EPOCH = "J2000" ;
  else {
    os << LogIO::WARN << "Unidentified epoch. set to 'XXXXX'" << LogIO::POST ;
    EPOCH = "XXXXX" ;
  }
  // DEBUG
  //cout << "EPOCH = \'" << EPOCH << "\'" << endl ;
  //
  string stmp ;
  if ( readHeader( stmp, "RA" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data RA0." << LogIO::POST ;
    return -1 ;
  }
  RA0 = radRA( stmp.c_str() ) ;
  // DEBUG
  //cout << "RA0 = " << RA0 << endl ;
  //
  if ( readHeader( stmp, "DEC" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data DEC0." << LogIO::POST ;
    return -1 ;
  }
  DEC0 = radDEC( stmp.c_str() ) ;
  // DEBUG
  //cout << "DEC0 = " << DEC0 << endl ;
  //
  if ( readHeader( GLNG0, "GL0", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data GLNG0." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLNG0 = " << GLNG0 << endl ;
  //
  if ( readHeader( GLAT0, "GB0", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data GLAT0." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "GLAT0 = " << GLAT0 << endl ;
  //
  if ( readHeader( NCALB, "NCALB", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data NCALB." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "NCALB = " << NCALB << endl ;
  //
  if ( readHeader( SCNCD, "SCNCD", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "SCNCD set to 0 (RADEC)." << LogIO::POST ;
    SCNCD = 0 ;
  }
  // DEBUG
  //cout << "SCNCD = " << SCNCD << endl ;
  //
  if ( readHeader( SCMOD, "SCMOD1" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SCMOD." << LogIO::POST ;
    return -1 ;
  }
  string::size_type pos = SCMOD.find( ' ' ) ;
  if ( pos != string::npos ) {
    SCMOD = SCMOD.substr( 0, pos ) ;
    SCMOD = SCMOD + " " ;
  }
  //cout << "SDMOD1 = \'" << SCMOD << "\'" << endl ;
  if ( readHeader( stmp, "SCMOD2" ) == 0 && stmp.compare( 0, 1, " " ) != 0 ) {
    if ( ( pos = stmp.find( ' ' ) ) != string::npos )
      stmp = stmp.substr( 0, pos ) ;
    SCMOD = SCMOD + stmp + " " ;
    //cout << "SCMOD2 = \'" << SCMOD << "\'" << endl ;
  }
  if ( readHeader( stmp, "SCMOD3" ) == 0 && stmp.compare( 0, 1, " " ) != 0 ) {
    if ( ( pos = stmp.find( ' ' ) ) != string::npos )
      stmp = stmp.substr( 0, pos ) ;
    SCMOD = SCMOD + stmp + " " ;
    //cout << "SCMOD3 = \'" << SCMOD << "\'" << endl ;
  }
  if ( readHeader( stmp, "SCMOD4" ) == 0 && stmp.compare( 0, 1, " " ) != 0 ) {
    if ( ( pos = stmp.find( ' ' ) ) != string::npos )
      stmp = stmp.substr( 0, pos ) ;
    SCMOD = SCMOD + stmp + " " ;
    //cout << "SCMOD4 = \'" << SCMOD << "\'" << endl ;
  }
  if ( readHeader( stmp, "SCMOD5" ) == 0 && stmp.compare( 0, 1, " " ) != 0 ) {
    if ( ( pos = stmp.find( ' ' ) ) != string::npos )
      stmp = stmp.substr( 0, pos ) ;
    SCMOD = SCMOD + stmp + " " ;
    //cout << "SCMOD5 = \'" << SCMOD << "\'" << endl ;
  }
  if ( readHeader( stmp, "SCMOD6" ) == 0 && stmp.compare( 0, 1, " " ) != 0 ) {
    if ( ( pos = stmp.find( ' ' ) ) != string::npos )
      stmp = stmp.substr( 0, pos ) ;
    SCMOD = SCMOD + stmp + " " ;
    //cout << "SCMOD6 = \'" << SCMOD << "\'" << endl ;
  }
  // DEBUG
  //cout << "SCMOD = \'" << SCMOD << "\'" << endl ;
  //
  if ( readHeader( URVEL, "VEL", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data URVEL." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "URVEL = " << URVEL << endl ;
  //
  if ( readHeader( VREF, "VREF" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data VREF." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "VREF = \'" << VREF << "\'" << endl ;
  //
  if ( readHeader( VDEF, "VDEF" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data VDEF." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "VDEF = \'" << VDEF << "\'" << endl ;
  //
  if ( readHeader( SWMOD, "SWMOD" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SWMOD." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SWMOD = \'" << SWMOD << "\'" << endl ;
  //
  if ( readHeader( FRQSW, "FRQSW", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "FRQSW set to 0." << LogIO::POST ;
    FRQSW = 0.0 ;
  }
  // DEBUG
  //cout << "FRQSW = " << FRQSW << endl ;
  //
  if ( readHeader( DBEAM, "DBEAM", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data DBEAM." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "DBEAM = " << DBEAM << endl ;
  //
  if ( readHeader( MLTOF, "MLTOF", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "MLTOF set to 0." << LogIO::POST ;
    MLTOF = 0.0 ;
  }
  // DEBUG
  //cout << "MLTOF = " << MLTOF << endl ;
  //
  if ( readHeader( CMTQ, "CMTQ", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CMTQ." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTQ = " << CMTQ << endl ;
  //
  if ( readHeader( CMTE, "CMTE", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CMTE." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTE = " << CMTE << endl ;
  //
  if ( readHeader( CMTSOM, "CMTSOM", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CMTSOM." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTSOM = " << CMTSOM << endl ;
  //
  if ( readHeader( CMTNODE, "CMTNODE", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CMTNODE." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTNODE = " << CMTNODE << endl ;
  //
  if ( readHeader( CMTI, "CMTI", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CMTI." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTI = " << CMTI << endl ;
  //
  if ( readHeader( CMTTM, "CMTTM" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CMTTM." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "CMTTM = \'" << CMTTM << "\'" << endl ;
  //
  if ( readHeader( SBDX, "SDBX", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SBDX." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDX = " << SBDX << endl ;
  //
  if ( readHeader( SBDY, "SDBY", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SBDY." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDY = " << SBDY << endl ;
  //
  if ( readHeader( SBDZ1, "SDBZ1", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SBDZ1." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ1 = " << SBDZ1 << endl ;
  //
  if ( readHeader( SBDZ2, "SDBZ2", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SBDZ2." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SBDZ2 = " << SBDZ2 << endl ;
  //
  if ( readHeader( DAZP, "DAZP", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "DAZP set to 0." << LogIO::POST ;
    DAZP = 0.0 ;
  }
  // DEBUG
  //cout << "DAZP = " << DAZP << endl ;
  //
  if ( readHeader( DELP, "DELP", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "DELP set to 0." << LogIO::POST ;
    DELP = 0.0 ;
  }
  // DEBUG
  //cout << "DELP = " << DELP << endl ;
  //
  if ( readHeader( CHBIND, "CHBIND", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "CHBIND set to 1." << LogIO::POST ;
    CHBIND = 1 ;
  }
  // DEBUG
  //cout << "CHBIND = " << CHBIND << endl ;
  //
  if ( readHeader( NUMCH, "NCH", sameEndian ) != 0 ) {
    if ( readTable( NUMCH, "NCH", sameEndian ) != 0 ) {
      os << LogIO::NORMAL << "NUMCH set to " << chmax_ << "." << LogIO::POST ;
      NUMCH = chmax_ ;
    }
  }
  // DEBUG
  //cout << "NUMCH = " << NUMCH << endl ;
  //
  if ( readHeader( CHMIN, "CHMIN", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "CHMIN set to 1." << LogIO::POST ;
    CHMIN = 1 ;
  }
  // DEBUG
  //cout << "CHMIN = " << CHMIN << endl ;
  //
  if ( readHeader( CHMAX, "CHMAX", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "CHMAX set to " << chmax_ << "." << LogIO::POST ;
    CHMAX = chmax_ ;
  }
  // DEBUG
  //cout << "CHMAX = " << CHMAX << endl ;
  //
  if ( readHeader( ALCTM, "ALCTM", sameEndian ) != 0 ) {
    if ( readTable( ALCTM, "ALCTM", sameEndian ) != 0 ) {
      os << LogIO::WARN << "Error while reading data ALCTM." << LogIO::POST ;
      return -1 ;
    }
  }
  // DEBUG
  //cout << "ALCTM = " << ALCTM << endl ;
  //
  int itmp ;
  if ( readHeader( itmp, "INTEG", sameEndian ) != 0 ) {
    if ( readTable( itmp, "INTEG", sameEndian ) != 0 ) {
      os << LogIO::WARN << "Error while reading data IPTIM." << LogIO::POST ;
      return -1 ;
    }
  }
  IPTIM = (double)itmp ;
  // DEBUG
  //cout << "IPTIM = " << IPTIM << endl ;
  //
  if ( readHeader( PA, "PA", sameEndian ) != 0 ) {
    if ( readTable( PA, "PA", sameEndian ) != 0 ) {
      os << LogIO::WARN << "Error while reading data PA." << LogIO::POST ;
      return -1 ;
    }
  }
  // DEBUG
  //cout << "PA = " << PA << endl ;
  //

  // find data index for each ARYTP
  findData() ;
  vector<char *> v( ARYNM ) ;
  if ( readColumn( RX, "RX" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data RX." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "RX      " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << "\'" << RX[i] << "\' " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( HPBW, "HPBW", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data HPBW." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "HPBW    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << HPBW[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( EFFA, "EFFA", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data EFFA." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "EFFA    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << EFFA[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( EFFB, "EFFB", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data EFFB." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "EFFB    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << EFFB[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( EFFL, "EFFL", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data EFFL." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "EFFL    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     } 
//     //cout << EFFL[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( EFSS, "EFSS", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data EFSS." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "EFSS    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << EFSS[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( GAIN, "GAIN", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data GAIN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "GAIN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << GAIN[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( HORN, "HORN" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data HORN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "HORN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << "\'" << HORN[i] << "\' " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( POLTP, "POLTP" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data POLTP." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "POLTP   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << "\'" << POLTP[i] << "\' " ;
//   }
//   //cout << endl ;
  //
  vector<int> ipoldr( ARYNM, 0 ) ;
  if ( readColumn( ipoldr, "POLDR", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data POLDR." << LogIO::POST ;
    return -1 ;
  }
  for ( int i = 0 ; i < ARYNM ; i++ ) 
    POLDR[i] = (double)ipoldr[i] ;
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "POLDR   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << POLDR[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( POLAN, "POLAN", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data POLAN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "POLAN   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << POLAN[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( DFRQ, "DFRQ", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data DFRQ." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "DFRQ    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << DFRQ[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( SIDBD, "SIDBD" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SIDBD." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "SIDBD   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << "\'" << SIDBD[i] << "\' " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( REFN, "REFN", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data REFN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "REFN    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << REFN[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( IPINT, "IPINT", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data IPINT." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "IPINT   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << IPINT[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( MULTN, "MULTN", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data MULTN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "MULTN   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << MULTN[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( MLTSCF, "MLTSCF", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data MLTSCF." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "MLTSCF  " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << MLTSCF[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( LAGWIND, "LAGWIN" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data LAGWIND." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "LAGWIND " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << "\'" << LAGWIND[i] << "\' " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( BEBW, "BEBW", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data BEBW." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "BEBW    " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << BEBW[i] << " " ;
//   }  
//   //cout << endl ;
  //
  if ( readColumn( BERES, "BERES", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data BERES." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "BERES   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << BERES[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( CHWID, "CHWID", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data CHWID." << LogIO::POST ;
    return -1 ;
  }
    // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "CHWID   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << CHWID[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readARRY() != 0 ) {
    os << LogIO::WARN << "Error while reading data ARRY." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < NRO_FITS_ARYMAX ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "ARRY    " ;
//     }
//     else if ( ( i % 20 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << ARRY[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( NFCAL, "NFCAL", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data NFCAL." << LogIO::POST ;
    return -1 ;
  }
    // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "NFCAL   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << NFCAL[i] << " " ;
//   }
//   //cout << endl ;
  //
  if ( readColumn( F0CAL, "F0CAL", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data F0CAL." << LogIO::POST ;
    return -1 ;
  }
    // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "F0CAL   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << F0CAL[i] << " " ;
//   }
//   //cout << endl ;
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readColumn( vv, "FQCAL", sameEndian, i ) != 0 ) {
      os << LogIO::WARN << "Error while reading data FQCAL." << LogIO::POST ;
      return -1 ;
    }
    for ( int j = 0 ; j < ARYNM ; j++ ) {
      FQCAL[j][i] = vv[j] ;
    }
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     for ( int j = 0 ; j < 10 ; j++ ) {
//       if ( j == 0 ) {
//         if ( i < 10 ) 
//           //cout << "FQCAL0" << i << " " ;
//         else 
//           //cout << "FQCAL" << i << " " ;
//       }
//       else if ( ( j % 5 ) == 0 ) {
//         //cout << endl << "        " ;
//       }
//       //cout << FQCAL[i][j] << " " ;
//     }
//     //cout << endl ;
//   }
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readColumn( vv, "CHCAL", sameEndian, i ) != 0 ) {
      os << LogIO::WARN << "Error while reading data CHCAL." << LogIO::POST ;
      return -1 ;
    }
    for ( int j = 0 ; j < ARYNM ; j++ ) {
      CHCAL[j][i] = vv[j] ;
    }
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     for ( int j = 0 ; j < 10 ; j++ ) {
//       if ( j == 0 ) {
//         if ( i < 10 ) 
//           //cout << "CHCAL0" << i << " " ;
//         else 
//           //cout << "CHCAL" << i << " " ;
//       }
//       else if ( ( j % 5 ) == 0 ) {
//         //cout << endl << "        " ;
//       }
//      //cout << CHCAL[i][j] << " " ;
//    }
//    //cout << endl ;
//   }
  //
  for ( int i= 0 ; i < 10 ; i++) {
    vector<double> vv( ARYNM, 0 ) ;
    if ( readColumn( vv, "CWCAL", sameEndian, i ) != 0 ) {
      os << LogIO::WARN << "Error while reading data CWCAL." << LogIO::POST ;
      return -1 ;
    }
    for ( int j = 0 ; j < ARYNM ; j++ ) {
      CWCAL[j][i] = vv[j] ;
    }
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     for ( int j = 0 ; j < 10 ; j++ ) {
//       if ( j == 0 ) {
//         if ( i < 10 ) 
//           //cout << "CWCAL0" << i << " " ;
//         else 
//           //cout << "CWCAL" << i << " " ;
//       }
//       else if ( ( j % 5 ) == 0 ) {
//         //cout << endl << "        " ;
//       }
//       //cout << CWCAL[i][j] << " " ;
//     }
//     //cout << endl ;
//   }
  //
  if ( readHeader( SCNLEN, "NAXIS1", sameEndian ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SCNLEN." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SCNLEN = " << SCNLEN << endl ;
  //
  if ( readHeader( SBIND, "SBIND", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "SBIND set to 0." << LogIO::POST ;
    SBIND = 0 ;
  }
  // DEBUG
  //cout << "SBIND = " << SBIND << endl ;
  //
  if ( readHeader( IBIT, "IBIT", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "IBIT set to 8." << LogIO::POST ;
    IBIT = 8 ;  // 8 bit? 12 bit?
  }
  // DEBUG
  //cout << "IBIT = " << IBIT << endl ;
  //
  if ( readHeader( SITE, "TELESCOP" ) != 0 ) {
    os << LogIO::WARN << "Error while reading data SITE." << LogIO::POST ;
    return -1 ;
  }
  // DEBUG
  //cout << "SITE = \'" << SITE << "\'" << endl ;
  //
  if ( readColumn( DSBFC, "DSBFC", sameEndian ) != 0 ) {
    os << LogIO::NORMAL << "All DSBFC elements set to 1." << LogIO::POST ;
    for ( int i = 0 ; i < ARYNM ; i++ ) 
      DSBFC[i] = 1.0 ;
  }
  // DEBUG
//   for ( int i = 0 ; i < ARYNM ; i++ ) {
//     if ( i == 0 ) {
//       //cout << "DSBFC   " ;
//     }
//     else if ( ( i % 5 ) == 0 ) {
//       //cout << endl << "        " ;
//     }
//     //cout << DSBFC[i] << " " ;
//   }
//   //cout << endl ;
  //

  show() ;

  return 0 ;
}

int NROFITSDataset::fillRecord( int i )
{
  LogIO os( LogOrigin( "NROFITSDataset", "fillRecord()", WHERE ) ) ;

  int status = 0 ;
  string str4( 4, ' ' ) ;
  string str8( 8, ' ' ) ;
  string str24( 24, ' ' ) ;

  strcpy( record_->LSFIL, str4.c_str() ) ;
  status = readTable( record_->LSFIL, "LSFIL", 4, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading LSFIL." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "LSFIL(" << i << ") = " << record_->LSFIL << endl ;
  //
  status = readTable( record_->ISCAN, "ISCN", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading ISCAN." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "ISCAN(" << i << ") = " << record_->ISCAN << endl ;
  //
  vector<int> itmp( 6, 0 ) ;
  status = readTable( itmp, "LAVST", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading LAVST." << LogIO::POST ;
    return status ;
  }
  else {
    sprintf( record_->LAVST, "%4d%02d%02d%02d%02d%02d.000", itmp[0], itmp[1], itmp[2], itmp[3], itmp[4], itmp[5] ) ;
  }
  // DEBUG
  //cout << "LAVST(" << i << ") = " << record_->LAVST << endl ;
  //
  strcpy( record_->SCANTP, str8.c_str() ) ; 
    status = readTable( record_->SCANTP, "SCNTP", strlen(record_->SCANTP), i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading SCANTP." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading DSCX." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "DSCX(" << i << ") = " << record_->DSCX << endl ;
  //
  status = readTable( record_->DSCY, name2, same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading DSCY." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading SCX." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "SCX(" << i << ") = " << record_->SCX << endl ;
  //
  status = readTable( record_->SCY, name2, same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading SCY." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "SCY(" << i << ") = " << record_->SCY << endl ;
  //
  status = readTable( record_->PAZ, "PAZ", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading PAZ." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "PAZ(" << i << ") = " << record_->PAZ << endl ;
  //
  status = readTable( record_->PEL, "PEL", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading PEL." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "PEL(" << i << ") = " << record_->PEL << endl ;
  //
  status = readTable( record_->RAZ, "RAZ", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading RAZ." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "RAZ(" << i << ") = " << record_->RAZ << endl ;
  //
  status = readTable( record_->REL, "REL", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading REL." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "REL(" << i << ") = " << record_->REL << endl ;
  //
  status = readTable( record_->XX, "XX", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading XX." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "XX(" << i << ") = " << record_->XX << endl ;
  //
  status = readTable( record_->YY, "YY", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading YY." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "YY(" << i << ") = " << record_->YY << endl ;
  //
  strcpy( record_->ARRYT, str4.c_str() ) ;
  status = readTable( record_->ARRYT, "ARRYT", strlen(record_->ARRYT), i ) ;
  record_->ARRYT[3] = '\0' ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading ARRYT." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "ARRYT(" << i << ") = " << record_->ARRYT << endl ;
  //
  double dtmp ;
  status = readTable( dtmp, "TEMP", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading TEMP." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading PATM." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading PH2O." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading VWIND." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading DWIND." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading TAU." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading TSYS." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading BATM." << LogIO::POST ;
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
    os << LogIO::WARN << "Error while reading TEMP." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "VRAD(" << i << ") = " << record_->VRAD << endl ;
  //
  status = readTable( record_->FREQ0, "FRQ0", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading FREQ0." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "FREQ0(" << i << ") = " << record_->FREQ0 << endl ;
  //
  status = readTable( record_->FQTRK, "FQTRK", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading FQTRK." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "FQTRK(" << i << ") = " << record_->FQTRK << endl ;
  //
  status = readTable( record_->FQIF1, "FQIF1", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading FQIF1." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "FQIF1(" << i << ") = " << record_->FQIF1 << endl ;
  //
  status = readTable( record_->ALCV, "ALCV", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading ALCV." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "ALCV(" << i << ") = " << record_->ALCV << endl ;
  //
  record_->IDMY0 = 0 ;
  status = readTable( record_->DPFRQ, "DPFRQ", same_, i ) ;
  if ( status ) {
    //os << LogIO::WARN << "Error  DPFRQ set to 0." << LogIO::POST ;
    record_->DPFRQ = 0.0 ;
  }
  // DEBUG
  //cout << "DPFRQ(" << i << ") = " << record_->DPFRQ << endl ;
  //
  status = readTable( record_->SFCTR, "SFCTR", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading SFCTR." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "SFCTR(" << i << ") = " << record_->SFCTR << endl ;
  //
  status = readTable( record_->ADOFF, "ADOFF", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading ADOFF." << LogIO::POST ;
    return status ;
  }
  // DEBUG
  //cout << "ADOFF(" << i << ") = " << record_->ADOFF << endl ;
  //
  //status = readTable( record_->JDATA, "LDATA", same_, i ) ;
  status = readTable( JDATA, "LDATA", same_, i ) ;
  if ( status ) {
    os << LogIO::WARN << "Error while reading JDATA." << LogIO::POST ;
    return status ;
  }
  // DEBUG
//   for ( int i = 0 ; i < chmax_ ; i++ ) 
//     //cout << "JDATA[" << i << "] = " << JDATA[i] << " " ;
//   //cout << endl ;
  //


  // Update IPTIM since it depends on the row for NROFITS
  int integ ;
  status = readTable( integ, "INTEG", same_, i ) ;
  if ( !status ) {
    IPTIM = (double)integ ;
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
  vector<double> specout( chmax_, 0.0 ) ;
  NRODataRecord *record = getRecord( i ) ;
  double scale = record->SFCTR ;
  double offset = record->ADOFF ;
  double dscale = MLTSCF[getIndex( i )] ;
  //vector<int> ispec = record->JDATA ;
  vector<int> ispec = JDATA ;
  for ( int ii = 0 ; ii < chmax_ ; ii++ ) {
    spec[ii] = (double)( ispec[ii] * scale + offset ) * dscale ;
  }

  // for AOS, re-gridding is needed
  if ( strncmp( record->ARRYT, "H", 1 ) == 0 
       || strncmp( record->ARRYT, "W", 1 ) == 0 
       || strncmp( record->ARRYT, "U", 1 ) == 0 ) {

    string arryt = string( record->ARRYT ) ;
    uInt ib = getArrayId( arryt ) ;
    vector<double> fqcal = getFQCAL()[ib] ;
    vector<double> chcal = getCHCAL()[ib] ;
    int ncal = getNFCAL()[ib] ;

//     //cout << "NRODataset::getFrequencies()  ncal = " << ncal << endl ;
    while ( ncal < (int)fqcal.size() ) {
      fqcal.pop_back() ;
      chcal.pop_back() ;
    }
    Vector<Double> xin( chcal ) ;
    Vector<Double> yin( fqcal ) ;
    int nchan = getNUMCH() ;
    Vector<Double> xout( nchan ) ;
    indgen( xout ) ;
    Vector<Double> yout ;
    InterpolateArray1D<Double, Double>::interpolate( yout, xout, xin, yin, InterpolateArray1D<Double,Double>::cubic ) ;
    // debug
    //cout << "i=" << i << endl ;
    if ( i == 16 ) {
      //ofstream ofs0( "spgrid0.dat" ) ;
      for ( int ii = 0 ; ii < getNUMCH() ; ii++ ) 
        //ofs0 << xout[ii] << "," ;
      //ofs0 << endl ;
      for ( int ii = 0 ; ii < getNUMCH() ; ii++ ) 
        //ofs0 << setprecision(16) << record->FREQ0+yout[ii] << "," ;
      //ofs0 << endl ;
      //ofs0.close() ;
    }
    //
    Vector<Double> z( nchan ) ;
    Double bw = abs( yout[nchan-1] - yout[0] ) ;
    bw += 0.5 * abs( yout[nchan-1] - yout[nchan-2] + yout[1] - yout[0] ) ;
    Double dz = bw / (Double)nchan ;
    if ( yout[0] > yout[nchan-1] ) 
      dz = - dz ; 
    z[0] = yout[0] - 0.5 * ( yout[1] - yout[0] - dz ) ;
    for ( int ii = 1 ; ii < nchan ; ii++ ) 
      z[ii] = z[ii-1] + dz ;
    Vector<Double> zi( nchan+1 ) ;
    Vector<Double> yi( nchan+1 ) ;
    zi[0] = z[0] - 0.5 * dz ;
    zi[1] = z[0] + 0.5 * dz ;
    yi[0] = yout[0] - 0.5 * ( yout[1] - yout[0] ) ;
    yi[1] = yout[0] + 0.5 * ( yout[1] - yout[0] ) ;
    for ( int ii = 2 ; ii < nchan ; ii++ ) {
      zi[ii] = zi[ii-1] + dz ;
      yi[ii] = yi[ii-1] + 0.5 * ( yout[ii] - yout[ii-2] ) ;
    }
    zi[nchan] = z[nchan-1] + 0.5 * dz ;
    yi[nchan] = yout[nchan-1] + 0.5 * ( yout[nchan-1] - yout[nchan-2] ) ;
//     // debug
//     //cout << "nchan=" << nchan << ", bw=" << bw << ", dz=" << dz 
//          << ", y[1]-y[0]=" << yout[1]-yout[0] << endl ; 
//     //cout << "z: " << z[0] << " - " << z[nchan-1] 
//          << ", zi: " << zi[0] << " - " << zi[nchan] << endl ;
//     //cout << "y: " << yout[0] << " - " << yout[nchan-1] 
//          << ", yi: " << yi[0] << " - " << yi[nchan] << endl ;
//     ofstream ofs1( "spgrid1.dat", ios::out | ios::app ) ;
//     ofs1 << "spid=" << i << ", ARRYT=" << record->ARRYT << endl ;
//     ofs1 << "z[0]=" << z[0] << ", yout[0]=" << yout[0] << endl ;
//     for ( int ii = 1; ii < nchan ; ii++ ) {
//       ofs1 << "               dz=" << z[ii]-z[ii-1] << ", dy=" << yout[ii]-yout[ii-1] << endl ;
//       ofs1 << "z[" << ii << "]=" << z[ii] << ", yout[" << ii << "]=" << yout[ii] << endl ;
//     }
//     ofs1.close() ;
//     ofstream ofs2( "spgrid2.dat", ios::out | ios::app ) ;
//     ofs2 << "spid=" << i << ", ARRYT=" << record->ARRYT << endl ;
//     for ( int ii = 0 ; ii < nchan+1 ; ii++ ) 
//       ofs2 << "zi[" << ii << "]=" << zi[ii] << ", yi[" << ii << "]=" << yi[ii] << endl ;
//     ofs2.close() ;
//     //
    int ichan = 0 ;
    double wsum = 0.0 ; 
    // debug
    //ofstream ofs3( "spgrid3.dat", ios::out | ios::app ) ;
    if ( dz > 0.0 ) {
      for ( int ii = 0 ; ii < nchan ; ii++ ) {
        double zl = zi[ii] ;
        double zr = zi[ii+1] ;
        for ( int j = ichan ; j < nchan ; j++ ) {
          double yl = yi[j] ;
          double yr = yi[j+1] ;
          if ( yl <= zl ) {
            if ( yr <= zl ) {
              continue ;
            }
            else if ( yr <= zr ) {
              specout[ii] += spec[j] * ( yr - zl ) ;
              wsum += ( yr - zl ) ;
            }
            else {
              specout[ii] += spec[j] * dz ;
              wsum += dz ;
              ichan = j ;
              break ;
            }
          }
          else if ( yl < zr ) {
            if ( yr <= zr ) {
              specout[ii] += spec[j] * ( yr - yl ) ;
              wsum += ( yr - yl ) ;
            }
            else {
              specout[ii] += spec[j] * ( zr - yl ) ;
              wsum += ( zr - yl ) ;
              ichan = j ;
              break ;
            }
          }
          else {
            ichan = j - 1 ;
            break ;
          }
        }
        specout[ii] /= wsum ;
        wsum = 0.0 ;
      }
    }
    else if ( dz < 0.0 ) {
      for ( int ii = 0 ; ii < nchan ; ii++ ) {
        double zl = zi[ii] ;
        double zr = zi[ii+1] ;
        for ( int j = ichan ; j < nchan ; j++ ) {
          double yl = yi[j] ;
          double yr = yi[j+1] ;
          if ( yl >= zl ) {
            if ( yr >= zl ) {
              continue ;
            }
            else if ( yr >= zr ) {
              specout[ii] += spec[j] * abs( yr - zl ) ;
              wsum += abs( yr - zl ) ;
            }
            else {
              specout[ii] += spec[j] * abs( dz ) ;
              wsum += abs( dz ) ;
              ichan = j ;
              break ;
            }
          }
          else if ( yl > zr ) {
            if ( yr >= zr ) {
              specout[ii] += spec[j] * abs( yr - yl ) ;
              wsum += abs( yr - yl ) ;
            }
            else {
              specout[ii] += spec[j] * abs( zr - yl ) ;
              wsum += abs( zr - yl ) ;
              ichan = j ;
              break ;
            }
          }
          else {
            ichan = j - 1 ;
            break ;
          }
        }
        specout[ii] /= wsum ;
        wsum = 0.0 ;
      }
    }
    //specout = spec ;
    //ofs3.close() ;
  }
  else {
    specout = spec ;
  }

  return specout ;
}

int NROFITSDataset::getIndex( int irow ) 
{
  NRODataRecord *record = getRecord( irow ) ;
  string str = record->ARRYT ;
  string::size_type pos = str.find( " " ) ;
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

double NROFITSDataset::radRA( string ra ) 
{
  int pos1 = ra.find( ':' ) ;
  int pos2 ;
  string ch = ra.substr( 0, pos1 ) ;
  //cout << "ch = \'" << ch << "\'" << endl ;
  pos2 = pos1 + 1 ;
  pos1 = ra.find( ':', pos2 ) ;
  string cm = ra.substr( pos2, pos1 - pos2 ) ;
  //cout << "cm = \'" << cm << "\'" << endl ;
  pos2 = pos1 + 1 ;
  pos1 = ra.size() ;
  string cs = ra.substr( pos2, pos1 - pos2 ) ;
  //cout << "cs = \'" << cs << "\'" << endl ;
  double h ;
  if ( ra[0] != '-' ) 
    h = atof( ch.c_str() ) + atof( cm.c_str() ) / 60.0 + atof( cs.c_str() ) / 3600.0 ;
  else 
    h = atof( ch.c_str() ) - atof( cm.c_str() ) / 60.0 - atof( cs.c_str() ) / 3600.0 ;
  double rra = h * M_PI / 12.0 ;
  return rra ;
}

double NROFITSDataset::radDEC( string dec ) 
{
  int pos1 = dec.find( ':' ) ;
  int pos2 ;
  string cd = dec.substr( 0, pos1 ) ;
  //cout << "cd = \'" << cd << "\'" << endl ;
  pos2 = pos1 + 1 ;
  pos1 = dec.find( ':', pos2 ) ;
  string cm = dec.substr( pos2, pos1 - pos2 ) ;
  //cout << "cm = \'" << cm << "\'" << endl ;
  pos2 = pos1 + 1 ;
  pos1 = dec.size() ;
  string cs = dec.substr( pos2, pos1 - pos2 ) ;
  //cout << "cs = \'" << cs << "\'" << endl ;
  double h ;
  if ( dec[0] != '-' ) 
    h = atof( cd.c_str() ) + atof( cm.c_str() ) / 60.0 + atof( cs.c_str() ) / 3600.0 ;
  else
    h = atof( cd.c_str() ) - atof( cm.c_str() ) / 60.0 - atof( cs.c_str() ) / 3600.0 ;
  double rdec = h * M_PI / 180.0 ;
  return rdec ;
}

void NROFITSDataset::getField() 
{
  for ( int i = 0 ; i < numField_ ; i++ ) {
    char key1[9] ;
    char key2[9] ;
    char key3[9] ;
    if ( i < 9 ) {
      sprintf( key1, "TFORM%d  ", i+1 ) ;
      sprintf( key2, "TTYPE%d  ", i+1 ) ;
      sprintf( key3, "TUNIT%d  ", i+1 ) ;
      //cout << "key1 = " << key1 << ", key2 = " << key2 << ", key3 = " << key3 << endl ;
    }
    else if ( i < 99 ) {
      sprintf( key1, "TFORM%2d ", i+1 ) ;
      sprintf( key2, "TTYPE%2d ", i+1 ) ;
      sprintf( key3, "TUNIT%2d ", i+1 ) ;
      //cout << "key1 = " << key1 << ", key2 = " << key2 << ", key3 = " << key3 << endl ;
    }
    else {
      sprintf( key1, "TFORM%3d", i+1 ) ;
      sprintf( key2, "TTYPE%3d", i+1 ) ; 
      sprintf( key3, "TUNIT%3d", i+1 ) ;
      //cout << "key1 = " << key1 << ", key2 = " << key2 << ", key3 = " << key3 << endl ;
    }
    //char tmp[9] ;
    string tmp ;
    //strcpy( tmp, "         " ) ;
    if ( readHeader( tmp, key1 ) != 0 ) {
      cerr << "Error while reading field keyword for scan header." << endl ;
      return ;
    }
    //forms_[i] = string( tmp ) ;
    forms_[i] = tmp ;
    string::size_type spos = forms_[i].find( " " ) ;
    if ( spos != string::npos )
      forms_[i] = forms_[i].substr( 0, spos ) ;
    //strcpy( tmp, "         " ) ;
    if ( readHeader( tmp, key2 ) != 0 ) {
      cerr << "Error while reading field type for scan header." << endl ;
      return ;
    }
    //names_[i] = string( tmp ) ;
    names_[i] = tmp ;
    spos = names_[i].find( " " ) ;
    if ( spos != string::npos )
      names_[i] = names_[i].substr( 0, spos ) ;
    //strcpy( tmp, "         " ) ;
    if ( forms_[i].find( "A" ) != string::npos ) {
      //cout << "skip to get unit: name = " << forms_[i] << endl ;
      //strcpy( tmp, "none    " ) ;
      tmp = "none" ;
    }
    else {
      //cout << "get unit: name = " << forms_[i] << endl ;
      if ( readHeader( tmp, key3 ) != 0 ) {
        //strcpy( tmp, "none    " ) ;
        tmp = "none" ;
      }
    }
    //units_[i] = string( tmp ) ;
    units_[i] = tmp ;
    spos = units_[i].find( " " ) ;
    if ( spos != string::npos )
      units_[i] = units_[i].substr( 0, spos ) ;
    //cout << "i = " << i << ": name=" << forms_[i] << " type=" << names_[i] << " unit=" << units_[i] << endl ;
  }  
}

void NROFITSDataset::fillARYTP()
{
  string arry ;
  int count = 0 ;
  string arry1 ;
  string arry2 ;
  string arry3 ;
  string arry4 ;
  if ( readHeader( arry, "ARRY1" ) == 0 ) 
    arry1 = arry ;
  else 
    arry1 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry1[i] == '1' ) {
      char arytp[4] ;
      sprintf( arytp, "H%d", i+1 ) ;
      ARYTP[count++] = string( arytp ) ;
      //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ; 
    }
  }
  if ( readHeader( arry, "ARRY2" ) == 0 )
    arry2 = arry ;
  else 
    arry2 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry2[i] == '1' ) {
      if ( i < 10 ) {
        char arytp[4] ;
        sprintf( arytp, "W%d", i+1 ) ;
        ARYTP[count++] = string( arytp ) ;
        //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
      }
      else if ( i < 15 ) {
        char arytp[4] ;
        sprintf( arytp, "U%d", i-9 ) ;
        ARYTP[count++] = string( arytp ) ;
        //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
      }
      else {
        char arytp[4] ;
        sprintf( arytp, "X%d", i-14 ) ;
        ARYTP[count++] = string( arytp ) ;
        //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
      }
    }
  }
  if ( readHeader( arry, "ARRY3" ) == 0 ) 
    arry3 = arry ;
  else 
    arry3 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry3[i] == '1' ) {
      char arytp[4] ;
      sprintf( arytp, "A%d", i+1 ) ;
      ARYTP[count++] = string( arytp ) ;
      //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
    }
  }
  if ( readHeader( arry, "ARRY4" ) == 0 )
    arry4 = arry ;
  else 
    arry4 = "00000000000000000000" ;
  for ( int i = 0 ; i < 20 ; i++ ) {
    if ( arry4[i] == '1' ) {
      char arytp[4] ;
      sprintf( arytp, "A%d", i+21 ) ;
      ARYTP[count++] = string( arytp ) ;
      //cout << "ARYTP[" << count-1 << "] = " << ARYTP[count-1] << endl ;
    }
  }      
}

int NROFITSDataset::readARRY()
{
  LogIO os( LogOrigin( "NROFITSDataset", "readARRY()", WHERE ) ) ;

  string arry1 ;
  string arry2 ;
  string arry3 ;
  string arry4 ;
  int status = readHeader( arry1, "ARRY1" ) ;
  if ( status ) {
    os << LogIO::SEVERE << "Error while reading ARRY1" << LogIO::POST ;
    return status ;
  }
  status = readHeader( arry2, "ARRY2" ) ;
  if ( status ) {
    os << LogIO::SEVERE << "Error while reading ARRY2" << LogIO::POST ;
    return status ;
  }
  status = readHeader( arry3, "ARRY3" ) ;
  if ( status ) {
    os << LogIO::SEVERE << "Error while reading ARRY3" << LogIO::POST ;
    return status ;
  }
  status = readHeader( arry4, "ARRY4" ) ;
  if ( status ) {
    os << LogIO::SEVERE << "Error while reading ARRY4" << LogIO::POST ;
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
  return status ;
}

void NROFITSDataset::findData() 
{
  LogIO os( LogOrigin( "NROFITSDataset", "findData()", WHERE ) ) ;

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
    os << LogIO::WARN << "NROFITSDataset::findData()  failed to find rows for " ;
    for ( int i = 0 ; i < ARYNM ; i++ ) {
      if ( arrayid_[i] == -1 ) {
        os << LogIO::WARN << ARYTP[i] << " " ;
      }
    }
    os.post() ;
  }

//   for ( int i = 0 ; i < ARYNM ; i++ ) 
//     //cout << "arrayid_[" << i << "] = " << arrayid_[i] << endl ;
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

int NROFITSDataset::readHeader( string &v, char *name ) 
{
  //
  // Read 'name' attribute defined as char from the FITS Header
  //  
  int status = 0 ;
  
  char buf[81] ;
  strcpy( buf, "     " ) ;
  fseek( fp_, 0, SEEK_SET ) ;
  int count = 0 ;
  while ( strncmp( buf, name, strlen(name) ) != 0 && strncmp( buf, "END", 3 ) != 0 ) {
    fread( buf, 1, 80, fp_ ) ;
    buf[80] = '\0' ;
    count++ ;
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
  unsigned int clen = pos2 - pos1 ;
  //cout << "string: " << str << endl ;
  //cout << "value:  " << str.substr( pos1, clen ).c_str() << endl ;
  //cout << "clen = " << clen << endl ;
  v = str.substr( pos1, clen ) ;
  //cout << "v = \'" << v << "\'" << endl ; 

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
  int status = readTable( v, name, (int)strlen( v ), 0 ) ;

  return status ;
}

int NROFITSDataset::readTable( char *v, char *name, int clen, int idx ) 
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
  if ( xsize < clen ) {
    fread( v, 1, xsize, fp_ ) ;
    //v[xsize] = '\0' ;
  }
  else {
    fread( v, 1, clen-1, fp_ ) ;
    //v[clen-1] = '\0' ;
  }

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
    int clen = strlen( v[i] ) ;
    if ( clen > xsize ) {
      fread( v[i], 1, xsize, fp_ ) ;
      //v[i][xsize] = '\0' ;
    }
    else {
      fread( v[i], 1, clen, fp_ ) ;
      //v[i][clen-1] = '\0' ;
    }
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

int NROFITSDataset::readColumn( vector<string> &v, char *name )
{
  // 
  // Read 0-th column of ARRYTP-dependent 'name' attributes 
  // defined as char array from the FITS Scan Record 
  //
  int status = readColumn( v, name, 0 ) ;

  return status ;
}

int NROFITSDataset::readColumn( vector<string> &v, char *name, int idx )
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
//     int clen = (int)strlen( v[i] ) ;
//     if ( clen > xsize ) {
//       fread( v[i], 1, xsize, fp_ ) ;
//       //v[i][xsize] = '\0' ;
//     }
//     else {
//       fread( v[i], 1, clen-1, fp_ ) ;
//       //v[i][clen-1] = '\0' ;
//     }
    char c[xsize+1] ;
    fread( c, 1, xsize, fp_ ) ;
    c[xsize] = '\0' ;
    v[i] = string( c ) ;
    //cout << "v[" << i << "] = \'" << v[i] << "\'" << endl ;
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
    //cout << "offset = " << offset << ", v[" << i << "] = " << v[i] << endl ;
    fseek( fp_, -sizeof(double)-offset, SEEK_CUR ) ;
  }

//   //cout << "v: " << endl ;
//   for ( vector<double>::iterator i = v.begin() ; i != v.end() ; i++ ) 
//     //cout << *i << " " ;
//   //cout << endl ;
  
  return status ;
}

uInt NROFITSDataset::getArrayId( string type )
{
  uInt ib = 99 ;
  for ( uInt i = 0 ; i < arrayid_.size() ; i++ ) {
    uInt len = ARYTP[i].size() ;
    if ( type.compare( 0, len, ARYTP[i], 0, len ) == 0 ) {
      ib = i ;
      break ;
    }
  }
  return ib ;
}

double NROFITSDataset::getStartIntTime( int i ) 
{
  double v ;
  readTable( v, "MJDST", same_, i ) ;
  return v/86400.0 ;
}

// double NROFITSDataset::toLSR( double v, double t, double x, double y ) 
// {
//   return v ;
// }

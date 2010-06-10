//#---------------------------------------------------------------------------
//# NRODataset.cc: Base class for NRO dataset.
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

#include <atnf/PKSIO/NRODataset.h>
#include <casa/OS/Time.h>
#include <scimath/Mathematics/InterpolateArray1D.h>

#include <math.h>
#include <fstream>

//#include <casa/namespace.h>

using namespace std ;

// 
// NRODataset
//
// Base class for NRO dataset.
//

// constructor 
NRODataset::NRODataset( string name ) 
{
  LogIO os( LogOrigin( "NRODataset", "NRODataset()", WHERE ) ) ;

  // memory allocation
  initialize() ;

  filename_ = name ;
  fp_ = NULL ;
  scanNum_ = 0 ;
  rowNum_ = 0 ;
  scanLen_ = 0 ;
  dataLen_ = 0 ;
  dataid_ = -1 ;

  // OS endian 
  int i = 1 ;
  endian_ = -1 ;
  if ( *reinterpret_cast<char *>(&i) == 1 ) {
    endian_ = LITTLE_ENDIAN ;
    os << LogIO::NORMAL << "LITTLE_ENDIAN " << LogIO::POST ;
  }
  else {
    endian_ = BIG_ENDIAN ;
    os << LogIO::NORMAL << "BIG_ENDIAN " << LogIO::POST ;
  }
  same_ = -1 ;
}

// destructor 
NRODataset::~NRODataset() 
{
  // release memory
  releaseRecord() ;

  // close file
  close() ;
}

// data initialization
void NRODataset::initialize()
{
  datasize_ = sizeof( char ) * 8   // LOFIL
    + sizeof( char ) * 8           // VER
    + sizeof( char ) * 16          // GROUP
    + sizeof( char ) * 16          // PROJ
    + sizeof( char ) * 24          // SCHED
    + sizeof( char ) * 40          // OBSVR
    + sizeof( char ) * 16          // LOSTM
    + sizeof( char ) * 16          // LOETM
    + sizeof( int ) * 2            // ARYNM, NSCAN
    + sizeof( char ) * 120         // TITLE
    + sizeof( char ) * 16          // OBJ
    + sizeof( char ) * 8           // EPOCH
    + sizeof( double ) * 4         // RA0, DEC0, GLNG0, GLAT0
    + sizeof( int ) * 2            // NCALB, SCNCD
    + sizeof( char ) * 120         // SCMOD
    + sizeof( double )             // URVEL
    + sizeof( char ) * 4           // VREF
    + sizeof( char ) * 4           // VDEF
    + sizeof( char ) * 8           // SWMOD
    + sizeof( double ) * 8         // FRQSW, DBEAM, MLTOF, CMTQ, CMTE, CMTSOM, CMTNODE, CMTI
    + sizeof( char ) * 24          // CMTTM
    + sizeof( double ) * 6         // SBDX, SBDY, SBDZ1, SBDZ2, DAZP, DELP
    + sizeof( int ) * 4            // CHBIND, NUMCH, CHMIN, CHMAX
    + sizeof( double ) * 3         // ALCTM, IPTIM, PA
    + sizeof( int ) * 3            // SCNLEN, SBIND, IBIT
    + sizeof( char ) * 8 ;         // SITE

  // NRODataRecord
  record_ = new NRODataRecord() ;
  record_->LDATA = NULL ;
}

void NRODataset::convertEndian( int &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( int ) ;
  std::reverse( first, last ) ;
}

void NRODataset::convertEndian( float &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( float ) ;
  std::reverse( first, last ) ;
}

void NRODataset::convertEndian( double &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( double ) ;
  std::reverse( first, last ) ;
}

int NRODataset::readHeader( char *v, int size ) 
{
  if ( (int)( fread( v, 1, size, fp_ ) ) != size ) {
    return -1 ;
  }
  return 0 ;
}

int NRODataset::readHeader( int &v, int b ) 
{
  if ( fread( &v, 1, sizeof(int), fp_ ) != sizeof(int) ) {
    return -1 ;
  }

  if ( b == 0 )
    convertEndian( v ) ;

  return 0 ;
}

int NRODataset::readHeader( float &v, int b ) 
{
  if ( fread( &v, 1, sizeof(float), fp_ ) != sizeof(float) ) {
    return -1 ;
  }

  if ( b == 0 )
    convertEndian( v ) ;

  return 0 ;
}

int NRODataset::readHeader( double &v, int b ) 
{
  if ( fread( &v, 1, sizeof(double), fp_ ) != sizeof(double) ) {
    return -1 ;
  }

  if ( b == 0 )
    convertEndian( v ) ;

  return 0 ;
}

void NRODataset::convertEndian( NRODataRecord *r ) 
{
  convertEndian( r->ISCAN ) ;
  convertEndian( r->DSCX ) ;
  convertEndian( r->DSCY ) ;
  convertEndian( r->SCX ) ;
  convertEndian( r->SCY ) ;
  convertEndian( r->PAZ ) ;
  convertEndian( r->PEL ) ;
  convertEndian( r->RAZ ) ;
  convertEndian( r->REL ) ;
  convertEndian( r->XX ) ;
  convertEndian( r->YY ) ;
  convertEndian( r->TEMP ) ; 
  convertEndian( r->PATM ) ;
  convertEndian( r->PH2O ) ;
  convertEndian( r->VWIND ) ;
  convertEndian( r->DWIND ) ;
  convertEndian( r->TAU ) ;  
  convertEndian( r->TSYS ) ; 
  convertEndian( r->BATM ) ; 
  convertEndian( r->LINE ) ;
  for ( int i = 0 ; i < 4 ; i++ ) 
    convertEndian( r->IDMY1[i] ) ;
  convertEndian( r->VRAD ) ;
  convertEndian( r->FREQ0 ) ;
  convertEndian( r->FQTRK ) ;
  convertEndian( r->FQIF1 ) ;
  convertEndian( r->ALCV ) ; 
  for ( int i = 0 ; i < 2 ; i++ )
    for ( int j = 0 ; j < 2 ; j++ ) 
      convertEndian( r->OFFCD[i][j] ) ;
  convertEndian( r->IDMY0 ) ;
  convertEndian( r->IDMY2 ) ;
  convertEndian( r->DPFRQ ) ;
  convertEndian( r->SFCTR ) ;
  convertEndian( r->ADOFF ) ;
}

void NRODataset::releaseRecord()
{
  if ( record_ ) {
    if ( record_->LDATA != NULL ) {
      delete record_->LDATA ;
      record_->LDATA = NULL ;
    }
    delete record_ ;
    record_ = NULL ;
  }
  dataid_ = -1 ;
}

// Get specified scan
NRODataRecord *NRODataset::getRecord( int i )
{
  LogIO os( LogOrigin( "NRODataset", "getRecord()", WHERE ) ) ;

  // DEBUG
  //cout << "NRODataset::getRecord()  Start " << i << endl ;
  //
  if ( i < 0 || i >= rowNum_ ) {
    //cerr << "NRODataset::getRecord()  data index out of range." << endl ;
    os << LogIO::SEVERE << "data index " << i << " out of range. return NULL." << LogIO::POST ;
    return NULL ;
  }

  if ( i == dataid_ ) {
    return record_ ;
  }

  // DEBUG
  //cout << "NRODataset::getData()  Get new dataset" << endl ;
  //
  // read data 
  int status = fillRecord( i ) ;
  if ( status == 0 ) {
    dataid_ = i ;
  }
  else {
    //cerr << "NRODataset::getRecord()  error while reading data " << i << endl ;
    os << LogIO::SEVERE << "error while reading data " << i << ". return NULL." << LogIO::POST ;
    dataid_ = -1 ;
    return NULL ;
  }

  return record_ ;
}

int NRODataset::fillRecord( int i ) 
{
  LogIO os( LogOrigin( "NRODataset", "fillRecord()", WHERE ) ) ;

  int status = 0 ;

  status = open() ;
  if ( status != 0 ) 
    return status ;
    

  // fill NRODataset
  int offset = getDataSize() + scanLen_ * i ;
  // DEBUG
  //cout << "NRODataset::fillRecord()  offset (header) = " << offset << endl ;
  //cout << "NRODataset::fillRecord()  sizeof(NRODataRecord) = " << sizeof( NRODataRecord ) << " byte" << endl ;
  fseek( fp_, offset, SEEK_SET ) ;
  if ( (int)fread( record_, 1, SCAN_HEADER_SIZE, fp_ ) != SCAN_HEADER_SIZE ) {
    //cerr << "Failed to read scan header: " << i << endl ;
    os << LogIO::SEVERE << "Failed to read scan header for " << i << "th row." << LogIO::POST ;
    return -1 ;
  }
  if ( (int)fread( record_->LDATA, 1, dataLen_, fp_ ) != dataLen_ ) {
    //cerr << "Failed to read spectral data: " << i << endl ;
    os << LogIO::SEVERE << "Failed to read spectral data for " << i << "th row." << LogIO::POST ;
    return -1 ;
  }

  if ( same_ == 0 ) {
    convertEndian( record_ ) ;
  } 

  // DWIND unit conversion (deg -> rad)
  record_->DWIND = record_->DWIND * M_PI / 180.0 ;

  return status ;
}

// open
int NRODataset::open() 
{
  int status = 0 ;

  if ( fp_ == NULL ) {
    if ( (fp_ = fopen( filename_.c_str(), "rb" )) == NULL ) 
      status = -1 ;
    else 
      status = 0 ;
  }

  return status ;
}

// close
void NRODataset::close() 
{
  // DEBUG 
  //cout << "NRODataset::close()  close file" << endl ;
  //
  if ( fp_ != NULL )
    fclose( fp_ ) ;
  fp_ = NULL ;
}

// get spectrum
vector< vector<double> > NRODataset::getSpectrum()
{
  vector< vector<double> > spec;

  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    spec.push_back( getSpectrum( i ) ) ;
  }

  return spec ;
}

vector<double> NRODataset::getSpectrum( int i )
{
  LogIO os( LogOrigin( "NRODataset", "getSpectrum", WHERE ) ) ;
  
  // DEBUG
  //cout << "NRODataset::getSpectrum() start process (" << i << ")" << endl ;
  //
  // size of spectrum is not chmax_ but dataset_->getNCH() after binding
  int nchan = NUMCH ;
  vector<double> spec( chmax_, 0.0 ) ;  // spectrum "before" binding
  vector<double> bspec( nchan, 0.0 ) ;  // spectrum "after" binding
  // DEBUG
  //cout << "NRODataset::getSpectrum()  nchan = " << nchan << " chmax_ = " << chmax_ << endl ;
  //

  NRODataRecord *record = getRecord( i ) ;

  int bit = IBIT ;   // fixed to 12 bit
  double scale = record->SFCTR ;
  // DEBUG
  //cout << "NRODataset::getSpectrum()  scale = " << scale << endl ;
  //
  double offset = record->ADOFF ;
  // DEBUG
  //cout << "NRODataset::getSpectrum()  offset = " << offset << endl ;
  //
  if ( ( scale == 0.0 ) && ( offset == 0.0 ) ) {
    //cerr << "NRODataset::getSpectrum()  zero spectrum (" << i << ")" << endl ;
    return bspec ;
  }
  char *cdata = record->LDATA ;
  vector<double> mscale = MLTSCF ;
  double dscale = mscale[getIndex( i )] ;
  int cbind = CHBIND ;
  int chmin = CHMIN ;

  // char -> int
  vector<int> ispec( chmax_, 0 ) ;
  union SharedMemory {
    int ivalue ;
    unsigned char cbuf[4] ;
  } ;
  SharedMemory u ;
  int j = 0 ;
  char ctmp = 0x00 ;
  int sw = 0 ;
  for ( int i = 0 ; i < chmax_ ; i++ ) {
    if ( bit == 12 ) {  // 12 bit qunatization
      u.ivalue = 0 ;

      if ( endian_ == BIG_ENDIAN ) {
        // big endian
        if ( sw == 0 ) {
          char c0 = (cdata[j] >> 4) & 0x0f ;
          char c1 = ((cdata[j] << 4) & 0xf0) | ((cdata[j+1] >> 4) & 0x0f) ;
          ctmp = cdata[j+1] & 0x0f ;
          u.cbuf[2] = c0 ;
          u.cbuf[3] = c1 ;
          j += 2 ;
          sw = 1 ;
        }
        else if ( sw == 1 ) {
          u.cbuf[2] = ctmp ;
          u.cbuf[3] = cdata[j] ;
          j++ ;
          sw = 0 ;
        }
      }
      else if ( endian_ == LITTLE_ENDIAN ) {
        // little endian
        if ( sw == 0 ) {
          char c0 = (cdata[j] >> 4) & 0x0f ;
          char c1 = ((cdata[j] << 4) & 0xf0) | ((cdata[j+1] >> 4) & 0x0f) ;
          ctmp = cdata[j+1] & 0x0f ;
          u.cbuf[1] = c0 ;
          u.cbuf[0] = c1 ;
          j += 2 ;
          sw = 1 ;
        }
        else if ( sw == 1 ) {
          u.cbuf[1] = ctmp ;
          u.cbuf[0] = cdata[j] ;
          j++ ;
          sw = 0 ;
        }
      }
    }
    
    ispec[i] = u.ivalue ;
    if ( ( ispec[i] < 0 ) || ( ispec[i] > 4096 ) ) {
      //cerr << "NRODataset::getSpectrum()  ispec[" << i << "] is out of range" << endl ;
      os << LogIO::SEVERE << "ispec[" << i << "] is out of range" << LogIO::EXCEPTION ;
      return bspec ;
    }
    // DEBUG
    //cout << "NRODataset::getSpectrum()  ispec[" << i << "] = " << ispec[i] << endl ;
    //
  }

  // int -> double 
  for ( int i = 0 ; i < chmax_ ; i++ ) {
    spec[i] = (double)( ispec[i] * scale + offset ) * dscale ; 
    // DEBUG
    //cout << "NRODataset::getSpectrum()  spec[" << i << "] = " << spec[i] << endl ;
    //
  }

  // channel binding
  if ( cbind != 1 ) {
    int k = chmin ;
    double sum0 = 0 ;
    double sum1 = 0 ;
    for ( int i = 0 ; i < nchan ; i++ ) {
      for ( int j = k ; j < k + cbind ; j++ ) {
        sum0 += spec[k] ;
        sum1++ ;
      }
      bspec[i] = sum0 / sum1 ;
      k += cbind ;
      // DEBUG
      //cout << "NRODataset::getSpectrum()  bspec[" << i << "] = " << bspec[i] << endl ;
      //
    }
  }
  else {
    for ( int i = 0 ; i < nchan ; i++ ) 
      bspec[i] = spec[i] ;
  }

  // DEBUG
  //cout << "NRODataset::getSpectrum() end process" << endl ;
  //

  return bspec ;
}

int NRODataset::getIndex( int irow )
{
  // DEBUG 
  //cout << "NRODataset::getIndex()  start" << endl ;
  //
  NRODataRecord *record = getRecord( irow ) ;
  string str = record->ARRYT ;
  // DEBUG
  //cout << "NRODataset::getIndex()  str = " << str << endl ;
  //
  string substr = str.substr( 1, 2 ) ;
  unsigned int index = (unsigned int)(atoi( substr.c_str() ) - 1) ;
  // DEBUG 
  //cout << "NRODataset::getIndex()  irow = " << irow << " index = " << index << endl ;
  //

  // DEBUG 
  //cout << "NRODataset::getIndex()  end" << endl ;
  //
  return index ;
}

int NRODataset::getPolarizationNum() 
{
  // DEBUG
  //cout << "NRODataset::getPolarizationNum()  start process" << endl ;
  //
  int npol = 0 ;

  vector<string> type( 2 ) ;
  type[0] = "CIRC" ;
  type[1] = "LINR" ;
  vector<double> crot ;
  vector<double> lagl ;
  //vector<int> ntype( 2, 0 ) ;

  unsigned int imax = rowNum_ ;
  for ( unsigned int i = 0 ; i < imax ; i++ ) { 
    int index = getIndex( i ) ;
    // DEBUG 
    //cout <<"NRODataset::getPolarizationNum()  index = " << index << endl ;
    //
    if ( POLTP[index] == type[0] ) {
      if( count( crot.begin(), crot.end(), POLDR[index] ) != 0 ) {
        crot.push_back( POLDR[index] ) ;
        npol++ ;
      }
      //ntype[0] = 1 ;
    }
    else if ( POLTP[index] == type[1] ) {
      if ( count( lagl.begin(), lagl.end(), POLAN[index] ) != 0 ) {
        lagl.push_back( POLAN[index] ) ;
        npol++ ;
      }
      //ntype[1] = 1 ;
    }
  }

  if ( npol == 0 )
    npol = 1 ;

  // DEBUG
  //cout << "NRODataset::getPolarizationNum()  end process" << endl ;
  //

  return npol ;
}

vector<double> NRODataset::getStartIntTime()
{
  vector<double> times ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    times.push_back( getStartIntTime( i ) ) ;
  }
  return times ;
}

double NRODataset::getStartIntTime( int i ) 
{
  NRODataRecord *record = getRecord( i ) ;

  char *t = record->LAVST ;
  return getMJD( t ) ;
}

double NRODataset::getMJD( char *time ) 
{
  // TODO: should be checked which time zone the time depends on
  // 2008/11/14 Takeshi Nakazato
  string strStartTime( time ) ;
  string strYear = strStartTime.substr( 0, 4 ) ;
  string strMonth = strStartTime.substr( 4, 2 ) ;
  string strDay = strStartTime.substr( 6, 2 ) ;
  string strHour = strStartTime.substr( 8, 2 ) ;
  string strMinute = strStartTime.substr( 10, 2 ) ;
  string strSecond = strStartTime.substr( 12, strStartTime.size() - 12 ) ;
  unsigned int year = atoi( strYear.c_str() ) ;
  unsigned int month = atoi( strMonth.c_str() ) ;
  unsigned int day = atoi( strDay.c_str() ) ;
  unsigned int hour = atoi( strHour.c_str() ) ;
  unsigned int minute = atoi( strMinute.c_str() ) ;
  double second = atof( strSecond.c_str() ) ;
  Time t( year, month, day, hour, minute, second ) ;

  return t.modifiedJulianDay() ;
}

vector<bool> NRODataset::getIFs()
{
  vector<bool> v ;
  vector< vector<double> > fref ;
  vector< vector<double> > chcal = CHCAL ;
  vector<double> f0cal = F0CAL ;
  vector<double> beres = BERES ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    vector<double> f( 4, 0 ) ;
    uInt index = getIndex( i ) ;
    f[0] = chcal[index][0] ;
    f[1] = f0cal[index] ;
    f[2] = beres[index] ;
    if ( f[0] != 0. ) {
      f[1] = f[1] - f[0] * f[2] ;
    }
    NRODataRecord *record = getRecord( i ) ;
    f[3] = record->FREQ0 ;
    if ( v.size() == 0 ) {
      v.push_back( True ) ;
      fref.push_back( f ) ;
    }
    else {
      bool b = true ;
      int fsize = fref.size() ;
      for ( int j = 0 ; j < fsize ; j++ ) {
        if ( fref[j][1] == f[1] && fref[j][2] == f[2] && fref[j][3] == f[3] ) {
          b = false ;
        }
      }
      if ( b ) {
        v.push_back( True ) ;
        fref.push_back( f ) ;
      }
    }
  }


  // DEBUG
  //cout << "NRODataset::getIFs()   number of IF is " << v.size() << endl ;
  //

  return v ;
}

vector<double> NRODataset::getFrequencies( int i )
{
  // return value
  // v[0]  reference channel
  // v[1]  reference frequency
  // v[2]  frequency increment
  vector<double> v( 3, 0.0 ) ;

  NRODataRecord *record = getRecord( i ) ;
  string arryt = string( record->ARRYT ) ;
  //string sbeamno = arryt.substr( 1, arryt.size()-1 ) ;
  //uInt ib = atoi( sbeamno.c_str() ) - 1 ; 
  uInt ib = getArrayId( arryt ) ;

  int ia = -1 ;
  bool isAOS = false ;
  //cout << "NRODataset::getFrequencies()  record->ARRYT=" << record->ARRYT << endl ;
  //cout << "NRODataset::getFrequencies()  ib = " << ib << endl ;

  if ( strncmp( record->ARRYT, "X", 1 ) == 0 ) {
    // FX
    if ( strncmp( (record->ARRYT)+1, "1", 1 ) == 0 
         || strncmp( (record->ARRYT)+1, "3", 1 ) ) {
      // FX1, 3
      ia = 2 ;
    }
    else {
      // FX2, 4
      ia = 1 ;
    }
  }
  else if ( strncmp( record->ARRYT, "A", 1 ) == 0 )
    ia = 2 ;  // AC
  else if ( strncmp( record->ARRYT, "W", 1 ) == 0 ) {
    // AOS-W    
    ia = 2 ;  
    isAOS = true ;
  }
  else if ( strncmp( record->ARRYT, "U", 1 ) == 0 ) {
    // AOS-U
    ia = 2 ;  
    isAOS = true ;
  }
  else if ( strncmp( record->ARRYT, "H", 1 ) == 0 ) {
    // AOS-H
    isAOS = true ;
    //cout << record->ARRYT << " " <<  strlen(record->ARRYT) << endl ;
    //cout << (record->ARRYT)+1 << endl ;
    if ( strncmp( (record->ARRYT)+2, " ", 1 ) == 0 ) {
      // H1-9
      if ( strncmp( (record->ARRYT)+1, "9", 1 ) == 0 ) {
        // H9
        ia = 2 ;
      }
      else {
        // H1-8
        ia = 1 ;
      }
    }
    else {
      // H10-16
      ia = 2 ;
    }
  }

  int iu ;
  if ( record->FQIF1 > 0 )
    iu = 1 ;  // USB
  else 
    iu = 2 ;  // LSB

  int ivdef = -1 ;
  //if ( strncmp( (dataset_->getVDEF()).c_str(), "RAD", 3 ) == 0 )
  if ( (getVDEF()).compare( 0, 3, "RAD" ) == 0 )
    ivdef = 0 ;
  //else if ( strncmp( dataset_->getVDEF(), "OPT", 3 ) == 0 )
  else if ( (getVDEF()).compare( 0, 3, "OPT" ) == 0 )
    ivdef = 1 ;
  // DEBUG
  //cout << "NRODataset::getFrequencies() ivdef = " << ivdef << endl ;
  //
  double vel = getURVEL() + record->VRAD ;
  double cvel = 2.99792458e8 ; // speed of light [m/s]
  double fq0 = record->FREQ0 ;
  //double fq0 = record->FQTRK ;

  int ncal = getNFCAL()[ib] ;
  vector<double> freqs( ncal ) ;
  double cw = 0.0 ;
  vector<double> fqcal = getFQCAL()[ib] ;
  vector<double> chcal = getCHCAL()[ib] ;

  for ( int ii = 0 ; ii < ncal ; ii++ ) {
    freqs[ii] = fqcal[ii] ;
    freqs[ii] -= getF0CAL()[ib] ;
    if ( ia == 1 ) {
      if ( iu == 1 ) {
        freqs[ii] = fq0 + freqs[ii] ;
      }
      else if ( iu == 2 ) {
        freqs[ii] = fq0 - freqs[ii] ;
      }
    }
    else if ( ia == 2 ) {
      if ( iu == 1 ) {
        freqs[ii] = fq0 - freqs[ii] ;
      }
      else if ( iu == 2 ) {
        freqs[ii] = fq0 + freqs[ii] ;
      }
    }     
//       if ( ivdef == 0 ) {
//         double factor = 1.0 / ( 1.0 - vel / cvel ) ;
//         freqs[ii] = freqs[ii] * factor - record->FQTRK * ( factor - 1.0 ) ;
//       }
//       else if ( ivdef == 1 ) {
//         double factor = vel / cvel ;
//         freqs[ii] = freqs[ii] * ( 1.0 + factor ) - record->FQTRK * factor ;
//       }
  }

  if ( isAOS ) {
    // regridding
    while ( ncal < (int)chcal.size() ) {
      chcal.pop_back() ;
    }
    Vector<Double> xin( chcal ) ;
    Vector<Double> yin( freqs ) ;
    int nchan = getNUMCH() ;
    Vector<Double> xout( nchan ) ;
    indgen( xout ) ;
    Vector<Double> yout ;
    InterpolateArray1D<Double, Double>::interpolate( yout, xout, xin, yin, InterpolateArray1D<Double,Double>::cubic ) ;
    Double bw = abs( yout[nchan-1] - yout[0] ) ;
    bw += 0.5 * abs( yout[nchan-1] - yout[nchan-2] + yout[1] - yout[0] ) ;
    Double dz = bw / (Double) nchan ;
    if ( yout[0] > yout[1] ) 
      dz = - dz ;
    v[0] = 0 ;
    v[1] = yout[0] ;
    v[2] = dz ;
  }
  else {
    cw = getBERES()[ib] ;
    
    if ( cw == 0.0 ) {
      cw = ( freqs[1] - freqs[0] ) 
        / ( chcal[1] - chcal[0] ) ;
//           if ( cw < 0.0 ) 
//             cw = - cw ;
    }
    v[0] = chcal[0] - 1 ; // 0-base
    v[1] = freqs[0] ;
    v[2] = cw ;
  }

  return v ;
}

uInt NRODataset::getArrayId( string type )
{
  string sbeamno = type.substr( 1, type.size()-1 ) ;
  uInt ib = atoi( sbeamno.c_str() ) - 1 ; 
  return ib ;
}

void NRODataset::show()
{
  LogIO os( LogOrigin( "NRODataset", "show()", WHERE ) ) ;

  os << LogIO::NORMAL << "------------------------------------------------------------" << endl ;
  os << LogIO::NORMAL << "Number of scan = " << scanNum_ << endl ;
  os << LogIO::NORMAL << "Number of data record = " << rowNum_ << endl ;
  os << LogIO::NORMAL << "Length of data record = " << scanLen_ << " bytes" << endl ;
  os << LogIO::NORMAL << "Allocated memory for spectral data = " << dataLen_ << " bytes" << endl ;
  os << LogIO::NORMAL << "Max number of channel = " << chmax_ << endl ;
  os << LogIO::NORMAL << "------------------------------------------------------------" << endl ;
  os.post() ;

}

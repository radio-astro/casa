//#---------------------------------------------------------------------------
//# NROReader.cc: Base class for NRO headerdata.
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

#include <atnf/PKSIO/NROReader.h>
#include <atnf/PKSIO/NRO45Reader.h>
#include <atnf/PKSIO/ASTEReader.h>
#include <atnf/PKSIO/ASTEFXReader.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasConvert.h>

#include <casa/IO/RegularFileIO.h>
#include <casa/OS/File.h>
#include <casa/OS/Time.h>

#include <stdio.h>
#include <string>
#include <iomanip>

using namespace std ;

//
// getNROReader 
//
// Return an appropriate NROReader for a NRO 45m and ASTE dataset.
//
NROReader *getNROReader( const String filename, 
                         String &datatype )
{
  // Check accessibility of the input.
  File inFile( filename ) ;
  if ( !inFile.exists() ) {
    datatype = filename + " not found." ;
    return 0 ;
  }

  if ( !inFile.isReadable() ) {
    datatype = filename + " is not readable." ;
    return 0 ;
  }

  // Determine the type of input.
  NROReader *reader = 0;
  if ( inFile.isRegular() ) {
    FILE *file ;
    file = fopen( filename.c_str(), "r" ) ;
    // read LOFIL0
    char buf[9];
    fread( buf, 4, 1, file ) ;
    buf[4] = '\0' ;
    // DEBUG
    //cout << "getNROReader:: buf = " << buf << endl ;
    //
    if ( string( buf ) == "RW-F") {
      // ASTE-FX data
      datatype = "ASTE-FX";
      reader = new ASTEFXReader( filename );
    } else {
      // otherwise, read SITE0
      NROHeader *h = new NRO45Header() ;
      int size = h->getDataSize() - 188 ;
      delete h ;
      fseek( file, size, SEEK_SET ) ;
      fread( buf, 8, 1, file ) ;
      buf[8] = '\0' ;
      // DEBUG
      //cout << "getNROReader:: buf = " << buf << endl ;
      //
      if ( string( buf ) == "NRO" ) {
        // NRO 45m data
        datatype = "NRO 45m" ;
        reader = new NRO45Reader( filename );
      }
      else {
        h = new ASTEHeader() ;
        size = h->getDataSize() - 188 ;
        delete h ;
        fseek( file, size, SEEK_SET ) ;
        fread( buf, 8, 1, file ) ;
        buf[8] = '\0' ;
        // DEBUG
        //cout << "getNROReader:: buf = " << buf << endl ;
        //
        if ( string( buf ) == "ASTE" ) {
          // ASTE data
          datatype = "ASTE" ;
          reader = new ASTEReader( filename ) ;
        }
        else {
          datatype = "UNRECOGNIZED INPUT FORMAT";
        }
      }
    }
    fclose( file ) ;
  } else {
    datatype = "UNRECOGNIZED INPUT FORMAT";
  }

  // Try to open
  if ( reader ) {
    if ( reader->open() ) {
      datatype += " OPEN ERROR" ;
      // DEBUG
      //cout << "getNROReader:: " << filename << ": " << datatype << endl ;
      //
      delete reader ;
    }
    else  {
      // DEBUG
      //cout << "getNROReader:: " << filename << ": " << datatype << endl ;
      //
      reader->readHeader() ;
      return reader ;
    }
  }

  // DEBUG
  //cout << filename << ": " << datatype << endl ;
  //

  return 0 ;
}


//
// getNROReader
//
// Search a list of directories for a NRO 45m and ASTE dataset and return an
// appropriate NROReader for it.
//
NROReader* getNROReader( const String name,
                         const Vector<String> directories,
                         int &iDir,
                         String &datatype )
{
  int nDir = directories.size();
  for ( iDir = 0; iDir < nDir; iDir++ ) {
    string inName = directories[iDir] + "/" + name;
    NROReader *reader = getNROReader( inName, datatype ) ;

    if (reader != 0) {
      return reader;
    }
  }

  iDir = -1;
  return 0;
}


//----------------------------------------------------------------------
// constructor
NROReader::NROReader( string name ) {
  // initialization
  filename_ = name ;
  header_ = NULL ;
  fp_ = NULL ;
  scanNum_ = 0 ;
  rowNum_ = 0 ;
  scanLen_ = 0 ;
  //data_ = NULL ;
  data_ = new NRODataset() ;
  dataid_ = -1 ;

  // open file
  if ( open() != 0 ) 
    cout << "NROReader::NROReader()  error while opening " << filename_ << endl ;

  // OS endian 
  int i = 1 ;
  endian_ = UNKNOWN ;
  if ( *reinterpret_cast<char *>(&i) == 1 ) {
    endian_ = LITTLE ;
    cout << "NROReader::NROReader()  LITTLE_ENDIAN " << endl ;
  }
  else {
    endian_ = BIG ;
    cout << "NROReader::NROReader()  BIG_ENDIAN " << endl ;
  }
}

// destructor
NROReader::~NROReader()
{
  delete header_ ;
  releaseData() ;

  close() ;
}

// open
Int NROReader::open() 
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
void NROReader::close() 
{
  // DEBUG 
  //cout << "NROReader::close()  close file" << endl ;
  //
  if ( fp_ != NULL )
    fclose( fp_ ) ;
  fp_ = NULL ;
}

// get spectrum
vector< vector<double> > NROReader::getSpectrum()
{
  vector< vector<double> > spec;

  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    spec.push_back( getSpectrum( i ) ) ;
  }

  return spec ;
}

vector<double> NROReader::getSpectrum( int i )
{
  // DEBUG
  //cout << "NROReader::getSpectrum() start process (" << i << ")" << endl ;
  //
  // size of spectrum is not chmax_ but header_->getNCH() after binding
  int nchan = header_->getNUMCH() ;
  vector<double> spec( chmax_, 0.0 ) ;  // spectrum "before" binding
  vector<double> bspec( nchan, 0.0 ) ;  // spectrum "after" binding
  // DEBUG
  //cout << "NROReader::getSpectrum()  nchan = " << nchan << " chmax_ = " << chmax_ << endl ;
  //

  if ( getData( i ) != 0 ) {
    cout << "NROReader::getSpectrum()  error" << endl ;
    return bspec ;
  }
  
  int bit = header_->getIBIT() ;   // fixed to 12 bit
  double scale = data_->SFCTR ;
  // DEBUG
  //cout << "NROReader::getSpectrum()  scale = " << scale << endl ;
  //
  double offset = data_->ADOFF ;
  // DEBUG
  //cout << "NROReader::getSpectrum()  offset = " << offset << endl ;
  //
  if ( ( scale == 0.0 ) && ( offset == 0.0 ) ) {
    //cerr << "NROReader::getSpectrum()  zero spectrum (" << i << ")" << endl ;
    return bspec ;
  }
  char *cdata = data_->LDATA ;
  vector<double> mscale = header_->getMLTSCF() ;
  double dscale = mscale[getIndex( i )] ;
  int cbind = header_->getCHBIND() ;
  int chmin = header_->getCHMIN() ;

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

      if ( endian_ == BIG ) {
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
      else if ( endian_ == LITTLE ) {
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
      cerr << "NROReader::getSpectrum()  ispec[" << i << "] is out of range" << endl ;
      return bspec ;
    }
    // DEBUG
    //cout << "NROReader::getSpectrum()  ispec[" << i << "] = " << ispec[i] << endl ;
    //
  }

  // int -> double 
  for ( int i = 0 ; i < chmax_ ; i++ ) {
    spec[i] = (double)( ispec[i] * scale + offset ) * dscale ; 
    // DEBUG
    //cout << "NROReader::getSpectrum()  spec[" << i << "] = " << spec[i] << endl ;
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
      //cout << "NROReader::getSpectrum()  bspec[" << i << "] = " << bspec[i] << endl ;
      //
    }
  }
  else {
    for ( int i = 0 ; i < nchan ; i++ ) 
      bspec[i] = spec[i] ;
  }

  // DEBUG
  //cout << "NROReader::getSpectrum() end process" << endl ;
  //

  return bspec ;
}

Int NROReader::getPolarizationNum() 
{
  // DEBUG
  //cout << "NROReader::getPolarizationNum()  start process" << endl ;
  //
  int npol = 0 ;

  NROHeader *h = header_ ;

  vector<string> type( 2 ) ;
  type[0] = "CIRL" ;
  type[1] = "LINR" ;
  vector<double> crot ;
  vector<double> lagl ;
  vector<int> ntype( 2, 0 ) ;

  unsigned int imax = rowNum_ ;
  for ( unsigned int i = 0 ; i < imax ; i++ ) { 
    int index = getIndex( i ) ;
    // DEBUG 
    //cout <<"NROReader::getPolarizationNum()  index = " << index << endl ;
    //
    if ( h->getPOLTP()[index] == type[0] ) {
      if( count( crot.begin(), crot.end(), h->getPOLDR()[index] ) != 0 ) {
        crot.push_back( h->getPOLDR()[i] ) ;
        npol++ ;
      }
      ntype[0] = 1 ;
    }
    else if ( h->getPOLTP()[index] == type[1] ) {
      if ( count( lagl.begin(), lagl.end(), h->getPOLAN()[index] ) != 0 ) {
        lagl.push_back( h->getPOLAN()[i] ) ;
        npol++ ;
      }
      ntype[1] = 1 ;
    }
  }

  if ( npol == 0 )
    npol = 1 ;

  // DEBUG
  //cout << "NROReader::getPolarizationNum()  end process" << endl ;
  //

  return npol ;
}

double NROReader::getStartTime() 
{
  char *startTime = header_->getLOSTM() ;
  return getMJD( startTime ) ;
}

double NROReader::getEndTime() 
{
  char *endTime = header_->getLOETM() ;
  return getMJD( endTime ) ;
}

vector<double> NROReader::getStartIntTime()
{
  vector<double> times ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    times.push_back( getStartIntTime( i ) ) ;
  }
  return times ;
}

double NROReader::getStartIntTime( int i ) 
{
  if ( getData( i ) != 0 ) {
    cout << "NROReader::getStartIntTime()  error" << endl ;
    return -999999.0 ;
  }
  char *t = data_->LAVST ;
  return getMJD( t ) ;
}

double NROReader::getMJD( char *time ) 
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
  uInt year = atoi( strYear.c_str() ) ;
  uInt month = atoi( strMonth.c_str() ) ;
  uInt day = atoi( strDay.c_str() ) ;
  uInt hour = atoi( strHour.c_str() ) ;
  uInt minute = atoi( strMinute.c_str() ) ;
  double second = atof( strSecond.c_str() ) ;
  Time t( year, month, day, hour, minute, second ) ;

  return t.modifiedJulianDay() ;
}

Int NROReader::getIndex( int irow )
{
  // DEBUG 
  //cout << "NROReader::getIndex()  start" << endl ;
  //
  if ( getData( irow ) != 0 ) {
    cout << "NROReader::getIndex()  error " << endl ;
    return -1 ; 
  }
  string str = data_->ARRYT ;
  // DEBUG
  //cout << "NROReader::getIndex()  str = " << str << endl ;
  //
  string substr = str.substr( 1, 2 ) ;
  uInt index = (uInt)(atoi( substr.c_str() ) - 1) ;
  // DEBUG 
  //cout << "NROReader::getIndex()  irow = " << irow << " index = " << index << endl ;
  //

  // DEBUG 
  //cout << "NROReader::getIndex()  end" << endl ;
  //
  return index ;
}

vector<double> NROReader::getFrequencies( int i )
{
  // return value
  // v[0]  reference channel
  // v[1]  reference frequency
  // v[2]  frequency increment
  vector<double> v( 3, 0.0 ) ;

  if ( getData( i ) != 0 ) {
    cout << "NROReader::getFrequendies()  error" << endl ;
    return v ;
  }
  string arryt = string( data_->ARRYT ) ;
  string sbeamno = arryt.substr( 1, arryt.size()-1 ) ;
  uInt ib = atoi( sbeamno.c_str() ) - 1 ; 


  v[0] = header_->getCHCAL()[ib][0] - 1 ; // 0-base

  int ia ;
  if ( strncmp( data_->ARRYT, "X", 1 ) == 0 )
    ia = 1 ;  // FX
  else
    ia = 2 ;  

  int iu ;
  if ( data_->FQIF1 > 0 )
    iu = 1 ;  // USB
  else 
    iu = 2 ;  // LSB

  int ivdef = -1 ;
  if ( strncmp( header_->getVDEF(), "RAD", 3 ) == 0 )
    ivdef = 0 ;
  else if ( strncmp( header_->getVDEF(), "OPT", 3 ) == 0 )
    ivdef = 1 ;
  // DEBUG
  //cout << "NROReader::getFrequencies() ivdef = " << ivdef << endl ;
  //
  double vel = header_->getURVEL() + data_->VRAD ;
  double cvel = 2.99792458e8 ; // speed of light [m/s]

  int ncal = header_->getNFCAL()[ib] ;
  double freqs[ncal] ;

  for ( int ii = 0 ; ii < ncal ; ii++ ) {
    freqs[ii] = header_->getFQCAL()[ib][ii] ;
    freqs[ii] -= header_->getF0CAL()[ib] ;
    if ( ia == 1 ) {
      if ( iu == 1 ) {
        freqs[ii] = data_->FREQ0 + freqs[ii] ;
      }
      else if ( iu == 2 ) {
        freqs[ii] = data_->FREQ0 - freqs[ii] ;
      }
    }
    else if ( ia == 2 ) {
      if ( iu == 1 ) {
        freqs[ii] = data_->FREQ0 - freqs[ii] ;
      }
      else if ( iu == 2 ) {
        freqs[ii] = data_->FREQ0 + freqs[ii] ;
      }
    }

    if ( ivdef == 0 ) {
      double factor = 1.0 / ( 1.0 - vel / cvel ) ;
      freqs[ii] = freqs[ii] * factor - data_->FQTRK * ( factor - 1.0 ) ;
    }
    else if ( ivdef == 1 ) {
      double factor = vel / cvel ;
      freqs[ii] = freqs[ii] * ( 1.0 + factor ) - data_->FQTRK * factor ;
    }
  }

  double cw = header_->getCWCAL()[ib][0] ;
  // DEBUG
  //if ( i == 25 ) {
  //cout << "NROReader::getFrequencies() FQCAL[0] = " << header_->getFQCAL()[ib][0] << " FQCAL[1] = " << header_->getFQCAL()[ib][1] << endl ;
  //cout << "NROReader::getFrequencies() freqs[0] = " << freqs[0] << " freqs[1] = " << freqs[1] << endl ;
  //}
  if ( cw == 0.0 ) {
    cw = ( freqs[1] - freqs[0] ) 
      / ( header_->getCHCAL()[ib][1] - header_->getCHCAL()[ib][0] ) ;
    if ( cw < 0.0 ) 
      cw = abs( cw ) ;
  }
  v[1] = freqs[0] ;
  v[2] = cw ;

  return v ;
}

vector<Bool> NROReader::getIFs()
{
  vector<Bool> v ;
  vector< vector<double> > fref ;
  vector< vector<double> > chcal = header_->getCHCAL() ;
  vector<double> f0cal = header_->getF0CAL() ;
  vector<double> beres = header_->getBERES() ;
  for ( int i = 0 ; i < rowNum_ ; i++ ) {
    vector<double> f( 4, 0 ) ;
    uInt index = getIndex( i ) ;
    f[0] = chcal[index][0] ;
    f[1] = f0cal[index] ;
    f[2] = beres[index] ;
    if ( f[0] != 0. ) {
      f[1] = f[1] - f[0] * f[2] ;
    }
    if ( getData( i ) != 0 ) {
      cout << "NROReader::getIFs()  error at " << i << endl ;
      return v ;
    }
    f[3] = data_->FREQ0 ;
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
  cout << "NROReader::getIFs()   number of IF is " << v.size() << endl ;
  //

  return v ;
}

vector<Bool> NROReader::getBeams() 
{
  vector<Bool> v ;
  vector<int> arry = header_->getARRY() ;
  for ( uInt i = 0 ; i < arry.size() ; i++ ) {
    if ( arry[i] != 0 ) {
      v.push_back( True ) ;
    }
  }

  // DEBUG
  cout << "NROReader::getBeams()   number of beam is " << v.size() << endl ;
  //

  return v ;
}

//////
void NROReader::convertEndian( int &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( int ) ;
  std::reverse( first, last ) ;
}

void NROReader::convertEndian( float &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( float ) ;
  std::reverse( first, last ) ;
}

void NROReader::convertEndian( double &value )
{
  char volatile *first = reinterpret_cast<char volatile *>( &value ) ;
  char volatile *last = first + sizeof( double ) ;
  std::reverse( first, last ) ;
}

void NROReader::convertEndian( NRODataset *d ) 
{
  convertEndian( d->ISCAN ) ;
  convertEndian( d->DSCX ) ;
  convertEndian( d->DSCY ) ;
  convertEndian( d->SCX ) ;
  convertEndian( d->SCY ) ;
  convertEndian( d->PAZ ) ;
  convertEndian( d->PEL ) ;
  convertEndian( d->RAZ ) ;
  convertEndian( d->REL ) ;
  convertEndian( d->XX ) ;
  convertEndian( d->YY ) ;
  convertEndian( d->TEMP ) ; 
  convertEndian( d->PATM ) ;
  convertEndian( d->PH2O ) ;
  convertEndian( d->VWIND ) ;
  convertEndian( d->DWIND ) ;
  convertEndian( d->TAU ) ;  
  convertEndian( d->TSYS ) ; 
  convertEndian( d->BATM ) ; 
  convertEndian( d->LINE ) ;
  for ( int i = 0 ; i < 4 ; i++ ) 
    convertEndian( d->IDMY1[i] ) ;
  convertEndian( d->VRAD ) ;
  convertEndian( d->FREQ0 ) ;
  convertEndian( d->FQTRK ) ;
  convertEndian( d->FQIF1 ) ;
  convertEndian( d->ALCV ) ; 
  for ( int i = 0 ; i < 2 ; i++ )
    for ( int j = 0 ; j < 2 ; j++ ) 
      convertEndian( d->OFFCD[i][j] ) ;
  convertEndian( d->IDMY0 ) ;
  convertEndian( d->IDMY2 ) ;
  convertEndian( d->DPFRQ ) ;
  convertEndian( d->SFCTR ) ;
  convertEndian( d->ADOFF ) ;
}

// Read specified data record.
Int NROReader::readData( int i ) 
{
  int status = 0 ;

  // fill NRODataset
  int offset = header_->getDataSize() + scanLen_ * i ;
  // DEBUG
  //cout << "NROReader::readData()  offset (header) = " << offset << endl ;
  //cout << "NROReader::readData()  sizeof(NRODataset) = " << sizeof( NRODataset ) << " byte" << endl ;
  fseek( fp_, offset, SEEK_SET ) ;
  int rsize = SCAN_HEADER_SIZE ;
  if ( (int)fread( data_, 1, rsize, fp_ ) != rsize ) {
    cerr << "Failed to read scan header: " << i << endl ;
    return -1 ;
  }
  //offset += SCAN_HEADER_SIZE ;
  //fseek( fp_, offset, SEEK_SET ) ;
  rsize = scanLen_ - SCAN_HEADER_SIZE ;
  //data_->LDATA = new char[rsize] ;
  if ( (int)fread( data_->LDATA, 1, rsize, fp_ ) != rsize ) {
    cerr << "Failed to read spectral data: " << i << endl ;
    return -1 ;
  }

  if ( !same_ ) {
    convertEndian( data_ ) ;
  } 

  return status ;
}

// Get specified scan
int NROReader::getData( int i )
{
  // DEBUG
  //cout << "NROReader::getData()  Start " << endl ;
  //
  if ( i < 0 || i >= rowNum_ ) {
    cerr << "NROReader::getData()  data index out of range." << endl ;
    return -1 ;
  }

  if ( i == dataid_ ) {
    return 0 ;
  }

  // DEBUG
  //cout << "NROReader::getData()  Get new dataset" << endl ;
  //
  // read data 
  //releaseData() ;
  //data_ = new NRODataset() ;
  int status = readData( i ) ;
  if ( status == 0 ) {
    dataid_ = i ;
  }
  else {
    cerr << "NROReader::getData()  error while reading data " << i << endl ;
    dataid_ = -1 ;
    //releaseData() ;
  }

  return status ;
}

// Get SRCDIRECTION in RADEC(J2000)
Vector<Double> NROReader::getSourceDirection()
{
  Vector<Double> v ;
  Double srcra = Double( header_->getRA0() ) ;
  Double srcdec = Double( header_->getDEC0() ) ;
  char *epoch = header_->getEPOCH() ;
  if ( strncmp( epoch, "B1950", 5 ) == 0 ) {
    // convert to J2000 value
    MDirection result = 
      MDirection::Convert( MDirection( Quantity( srcra, "rad" ),
				       Quantity( srcdec, "rad" ),
				       MDirection::Ref( MDirection::B1950 ) ),
			   MDirection::Ref( MDirection::J2000 ) ) () ;
    v = result.getAngle().getValue() ;
    Double srcra2 = v( 0 ) ;
    if ( srcra2 < 0.0 && srcra >= 0.0 )
      v( 0 ) = 2.0 * M_PI + srcra2 ;
    //cout << "NROReader::getSourceDirection()  SRCDIRECTION convert from (" 
    //<< srcra << "," << srcdec << ") B1950 to (" 
    //<< v( 0 ) << ","<< v( 1 ) << ") J2000" << endl ;
  }
  else if ( strncmp( epoch, "J2000", 5 ) == 0 ) {
    v.resize( 2 ) ;
    v( 0 ) = srcra ;
    v( 1 ) = srcdec ;
  }
    
  return v ;
}

// Get DIRECTION in RADEC(J2000)
Vector<Double> NROReader::getDirection( int i )
{
  Vector<Double> v ;
  if ( getData( i ) != 0 ) {
    cout << "NROReader::getDirection()  error " << endl ;
    return v ;
  }
  char *epoch = header_->getEPOCH() ;
  int icoord = header_->getSCNCD() ;
  Double dirx = Double( data_->SCX ) ;
  Double diry = Double( data_->SCY ) ;
  if ( icoord == 1 ) {
    // convert from LB to RADEC
    MDirection result = 
      MDirection::Convert( MDirection( Quantity( dirx, "rad" ), 
				       Quantity( diry, "rad" ),
				       MDirection::Ref( MDirection::GALACTIC ) ),
			   MDirection::Ref( MDirection::J2000 ) ) () ;
    v = result.getAngle().getValue() ;
    Double dirx2 = v( 0 ) ;
    if ( dirx2 < 0.0 && dirx >= 0.0 ) 
      v( 0 ) = 2.0 * M_PI + dirx2 ;
    //cout << "NROReader::getDirection()  DIRECTION convert from (" 
    //<< dirx << "," << diry << ") LB to (" 
    //<< v( 0 ) << ","<< v( 1 ) << ") RADEC" << endl ;
  }
  else if ( icoord == 2 ) {
    // convert from AZEL to RADEC
    MDirection result = 
      MDirection::Convert( MDirection( Quantity( dirx, "rad" ), 
				       Quantity( diry, "rad" ),
				       MDirection::Ref( MDirection::AZEL ) ),
			   MDirection::Ref( MDirection::J2000 ) ) () ;
    v = result.getAngle().getValue() ;
    //cout << "NROReader::getDirection()  DIRECTION convert from (" 
    //<< dirx << "," << diry << ") AZEL to (" 
    //<< v( 0 ) << ","<< v( 1 ) << ") RADEC" << endl ;
  }
  else if ( icoord == 0 ) {
    if ( strncmp( epoch, "B1950", 5 ) == 0 ) {
      // convert to J2000 value 
      MDirection result = 
	MDirection::Convert( MDirection( Quantity( dirx, "rad" ),
					 Quantity( diry, "rad" ),
					 MDirection::Ref( MDirection::B1950 ) ),
			     MDirection::Ref( MDirection::J2000 ) ) () ;
      v = result.getAngle().getValue() ;
      Double dirx2 = v( 0 ) ;
      if ( dirx2 < 0.0 && dirx >= 0.0 )
	v( 0 ) = 2.0 * M_PI + dirx2 ;
      //cout << "STFiller::readNRO()  DIRECTION convert from (" 
      //<< dirx << "," << diry << ") B1950 to (" 
      //<< v( 0 ) << ","<< v( 1 ) << ") J2000" << endl ;
    }
    else if ( strncmp( epoch, "J2000", 5 ) == 0 ) {
      v.resize( 2 ) ;
      v( 0 ) = dirx ;
      v( 1 ) = diry ;
    }
  }

  return v ;
}

void NROReader::releaseData()
{
  if ( data_ != NULL ) {
    if ( data_->LDATA != NULL ) {
      delete data_->LDATA ;
      data_->LDATA = NULL ;
    }
    delete data_ ;
    data_ = NULL ;
  }
  dataid_ = -1 ;
}

int NROReader::getHeaderInfo( Int &nchan,
                              Int &npol,
                              Int &nif,
                              Int &nbeam,
                              String &observer,
                              String &project,
                              String &obstype,
                              String &antname,
                              Vector<Double> &antpos,
                              Float &equinox,
                              String &freqref,
                              Double &reffreq,
                              Double &bw,
                              Double &utc,
                              String &fluxunit,
                              String &epoch,
                              String &poltype )
{
  
  nchan = header_->getNUMCH() ; 
  npol = getPolarizationNum() ;
  observer = header_->getOBSVR() ;
  project = header_->getPROJ() ;
  obstype = header_->getSWMOD() ;
  antname = header_->getSITE() ;
  // TODO: should be investigated antenna position since there are 
  //       no corresponding information in the header
  // 2008/11/13 Takeshi Nakazato
  // 
  // INFO: tentative antenna posiiton is obtained for NRO 45m from ITRF website
  // 2008/11/26 Takeshi Nakazato
  vector<double> pos = getAntennaPosition() ;
  antpos = pos ;
  char *eq = header_->getEPOCH() ;
  if ( strncmp( eq, "B1950", 5 ) == 0 )
    equinox = 1950.0 ;
  else if ( strncmp( eq, "J2000", 5 ) == 0 ) 
    equinox = 2000.0 ;
//   char *vref = header_->getVREF() ;
//   if ( strncmp( vref, "LSR", 3 ) == 0 ) {
//     strcat( vref, "K" ) ;
//   }
//   header_->freqref = vref ;
  getData( 0 ) ;
  reffreq = data_->FREQ0 ;
  bw = header_->getBEBW()[0] ;
  utc = getStartTime() ;
  fluxunit = "K" ;
  epoch = "UTC" ;  
  char *poltp = header_->getPOLTP()[0] ;
  if ( strcmp( poltp, "" ) == 0 ) 
    poltp = "None" ;
  poltype = poltp ;
  // DEBUG
  cout << "NROReader::getHeaderInfo()  poltype = " << poltype << endl ;
  //

  vector<Bool> ifs = getIFs() ;
  nif = ifs.size() ;

  vector<Bool> beams = getBeams() ;
  nbeam = beams.size() ;

  return 0 ;
}

string NROReader::getScanType( int i )
{
  if ( getData( i ) != 0 ) {
    cerr << "NROReader::getScanType()  error " << endl ;
    return NULL ;
  }
  string s = data_->SCANTP ;

  return s ;
}

int NROReader::getScanInfo( int irow,
                            uInt &scanno,
                            uInt &cycleno,
                            uInt &beamno,
                            uInt &polno,
                            vector<double> &freqs,   
                            Vector<Double> &restfreq,
                            uInt &refbeamno,
                            Double &scantime,
                            Double &interval,
                            String &srcname,
                            String &fieldname,
                            Array<Float> &spectra,
                            Array<uChar> &flagtra,
                            Array<Float> &tsys,
                            Array<Double> &direction,
                            Float &azimuth,
                            Float &elevation,
                            Float &parangle,
                            Float &opacity,
                            uInt &tcalid,
                            Int &fitid,
                            uInt &focusid,
                            Float &temperature,  
                            Float &pressure,     
                            Float &humidity,     
                            Float &windvel,      
                            Float &winddir,      
                            Double &srcvel,
                            Array<Double> &propermotion,
                            Vector<Double> &srcdir,
                            Array<Double> &scanrate )
{
  // get data record
  if ( getData( irow ) != 0 ) {
    cerr << "NROReader::getScanInfo() failed to get scan information." << endl ;
    return 1 ;
  }

  // scanno
  scanno = (uInt)(data_->ISCAN) ;

  // cycleno
  cycleno = 0 ;

  // beamno
  string arryt = string( data_->ARRYT ) ;
  string sbeamno = arryt.substr( 1, arryt.size()-1 ) ;
  uInt ibeamno = atoi( sbeamno.c_str() ) ; 
  beamno = ibeamno - 1 ;

  // polno
  polno = 0 ;

  // freqs (for IFNO and FREQ_ID)
  freqs = getFrequencies( irow ) ;

  // restfreq (for MOLECULE_ID)
  Vector<Double> rf( IPosition( 1, 1 ) ) ;
  rf( 0 ) = data_->FREQ0 ;
  restfreq = rf ;

  // refbeamno
  refbeamno = 0 ;

  // scantime
  scantime = Double( getStartIntTime( irow ) ) ;

  // interval
  interval = Double( header_->getIPTIM() ) ;

  // srcname
  srcname = String( header_->getOBJ() ) ;

  // fieldname
  fieldname = String( header_->getOBJ() ) ;

  // spectra
  vector<double> spec = getSpectrum( irow ) ;
  Array<Float> sp( IPosition( 1, spec.size() ) ) ;
  int index = 0 ;
  for ( Array<Float>::iterator itr = sp.begin() ; itr != sp.end() ; itr++ ) {
    *itr = spec[index++] ;
  }
  spectra = sp ;

  // flagtra
  Array<uChar> flag( spectra.shape() ) ;
  flag.set( 0 ) ;
  flagtra = flag ;

  // tsys
  Array<Float> tmp( IPosition( 1, 1 ), data_->TSYS ) ;
  tsys = tmp ;

  // direction
  direction = getDirection( irow ) ;

  // azimuth
  azimuth = data_->RAZ ;

  // elevation
  elevation = data_->REL ;

  // parangle
  parangle = 0.0 ;

  // opacity
  opacity = 0.0 ;

  // tcalid 
  tcalid = 0 ;

  // fitid
  fitid = -1 ;

  // focusid 
  focusid = 0 ;

  // temperature (for WEATHER_ID)
  temperature = Float( data_->TEMP ) ;

  // pressure (for WEATHER_ID)
  pressure = Float( data_->PATM ) ;

  // humidity (for WEATHER_ID) 
  humidity = Float( data_->PH2O ) ;

  // windvel (for WEATHER_ID)
  windvel = Float( data_->VWIND ) ;

  // winddir (for WEATHER_ID)
  winddir = Float( data_->DWIND ) ;

  // srcvel 
  srcvel = header_->getURVEL() ;

  // propermotion
  Array<Double> srcarr( IPosition( 1, 2 ) ) ;
  srcarr = 0.0 ;
  propermotion = srcarr ;

  // srcdir
  srcdir = getSourceDirection() ;

  // scanrate
  Array<Double> sr( IPosition( 1, 1 ) ) ;
  sr = 0.0 ;
  scanrate = sr ;

  return 0 ;
}


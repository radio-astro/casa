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
#include <atnf/PKSIO/NRO45FITSReader.h>
#include <atnf/PKSIO/NROOTFDataset.h>
#include <atnf/PKSIO/ASTEDataset.h>

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
    if ( string( buf ) == "XTEN" ) {
      // FITS data
      datatype = "NRO 45m FITS" ;
      reader = new NRO45FITSReader( filename ) ;
    }
    else if ( string( buf ) == "RW-F") {
      // ASTE-FX data
      datatype = "ASTE-FX";
      reader = new ASTEFXReader( filename );
    } else {
      // otherwise, read SITE0
      NRODataset *d = new NROOTFDataset( filename ) ;
      int size = d->getDataSize() - 188 ;
      delete d ;
      fseek( file, size, SEEK_SET ) ;
      fread( buf, 8, 1, file ) ;
      buf[8] = '\0' ;
      // DEBUG
      //cout << "getNROReader:: buf = " << buf << endl ;
      //
      if ( string( buf ) == "NRO" ) {
        // NRO 45m data
        datatype = "NRO 45m OTF" ;
        reader = new NRO45Reader( filename );
      }
      else {
        d = new ASTEDataset( filename ) ;
        size = d->getDataSize() - 188 ;
        delete d ;
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

  // return reader if exists
  if ( reader ) {
    reader->read() ;
    return reader ;
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
  dataset_ = NULL ;
}

// destructor
NROReader::~NROReader()
{
  if ( dataset_ != NULL ) {
    delete dataset_ ;
    dataset_ = NULL ;
  }
}

// get spectrum
vector< vector<double> > NROReader::getSpectrum()
{
  return dataset_->getSpectrum() ;
}

Int NROReader::getPolarizationNum() 
{
  return dataset_->getPolarizationNum() ;
}

double NROReader::getStartTime() 
{
  char *startTime = dataset_->getLOSTM() ;
  return getMJD( startTime ) ;
}

double NROReader::getEndTime() 
{
  char *endTime = dataset_->getLOETM() ;
  return getMJD( endTime ) ;
}

vector<double> NROReader::getStartIntTime()
{
  return dataset_->getStartIntTime() ;
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
  NRODataRecord *record = dataset_->getRecord( irow ) ;
  string str = record->ARRYT ;
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

  NRODataRecord *record = dataset_->getRecord( i ) ;
  string arryt = string( record->ARRYT ) ;
  string sbeamno = arryt.substr( 1, arryt.size()-1 ) ;
  uInt ib = atoi( sbeamno.c_str() ) - 1 ; 


  v[0] = dataset_->getCHCAL()[ib][0] - 1 ; // 0-base

  int ia ;
  if ( strncmp( record->ARRYT, "X", 1 ) == 0 )
    ia = 1 ;  // FX
  else
    ia = 2 ;  

  int iu ;
  if ( record->FQIF1 > 0 )
    iu = 1 ;  // USB
  else 
    iu = 2 ;  // LSB

  int ivdef = -1 ;
  if ( strncmp( dataset_->getVDEF(), "RAD", 3 ) == 0 )
    ivdef = 0 ;
  else if ( strncmp( dataset_->getVDEF(), "OPT", 3 ) == 0 )
    ivdef = 1 ;
  // DEBUG
  //cout << "NROReader::getFrequencies() ivdef = " << ivdef << endl ;
  //
  double vel = dataset_->getURVEL() + record->VRAD ;
  double cvel = 2.99792458e8 ; // speed of light [m/s]
  double fq0 = record->FREQ0 ;
  //double fq0 = record->FQTRK ;

  int ncal = dataset_->getNFCAL()[ib] ;
  double freqs[ncal] ;

  for ( int ii = 0 ; ii < ncal ; ii++ ) {
    freqs[ii] = dataset_->getFQCAL()[ib][ii] ;
    freqs[ii] -= dataset_->getF0CAL()[ib] ;
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

//     if ( ivdef == 0 ) {
//       double factor = 1.0 / ( 1.0 - vel / cvel ) ;
//       freqs[ii] = freqs[ii] * factor - record->FQTRK * ( factor - 1.0 ) ;
//     }
//     else if ( ivdef == 1 ) {
//       double factor = vel / cvel ;
//       freqs[ii] = freqs[ii] * ( 1.0 + factor ) - record->FQTRK * factor ;
//     }
  }

  double cw = dataset_->getCWCAL()[ib][0] ;
  // DEBUG
  //if ( i == 25 ) {
  //cout << "NROReader::getFrequencies() FQCAL[0] = " << dataset_->getFQCAL()[ib][0] << " FQCAL[1] = " << dataset_->getFQCAL()[ib][1] << endl ;
  //cout << "NROReader::getFrequencies() freqs[0] = " << freqs[0] << " freqs[1] = " << freqs[1] << endl ;
  //}
  if ( cw == 0.0 ) {
    cw = ( freqs[1] - freqs[0] ) 
      / ( dataset_->getCHCAL()[ib][1] - dataset_->getCHCAL()[ib][0] ) ;
    if ( cw < 0.0 ) 
      cw = abs( cw ) ;
  }
  v[1] = freqs[0] ;
  v[2] = cw ;

  return v ;
}

vector<Bool> NROReader::getIFs()
{
  return dataset_->getIFs() ;
}

vector<Bool> NROReader::getBeams() 
{
  vector<Bool> v ;
  vector<int> arry = dataset_->getARRY() ;
  for ( uInt i = 0 ; i < arry.size() ; i++ ) {
    if ( arry[i] != 0 ) {
      v.push_back( True ) ;
    }
  }

  // DEBUG
  //cout << "NROReader::getBeams()   number of beam is " << v.size() << endl ;
  //

  return v ;
}

// Get SRCDIRECTION in RADEC(J2000)
Vector<Double> NROReader::getSourceDirection()
{
  Vector<Double> v ;
  Double srcra = Double( dataset_->getRA0() ) ;
  Double srcdec = Double( dataset_->getDEC0() ) ;
  char *epoch = dataset_->getEPOCH() ;
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
  NRODataRecord *record = dataset_->getRecord( i ) ;
  char *epoch = dataset_->getEPOCH() ;
  int icoord = dataset_->getSCNCD() ;
  Double dirx = Double( record->SCX ) ;
  Double diry = Double( record->SCY ) ;
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
  
  nchan = dataset_->getNUMCH() ; 
  npol = getPolarizationNum() ;
  observer = dataset_->getOBSVR() ;
  project = dataset_->getPROJ() ;
  obstype = dataset_->getSWMOD() ;
  antname = dataset_->getSITE() ;
  // TODO: should be investigated antenna position since there are 
  //       no corresponding information in the header
  // 2008/11/13 Takeshi Nakazato
  // 
  // INFO: tentative antenna posiiton is obtained for NRO 45m from ITRF website
  // 2008/11/26 Takeshi Nakazato
  vector<double> pos = getAntennaPosition() ;
  antpos = pos ;
  char *eq = dataset_->getEPOCH() ;
  if ( strncmp( eq, "B1950", 5 ) == 0 )
    equinox = 1950.0 ;
  else if ( strncmp( eq, "J2000", 5 ) == 0 ) 
    equinox = 2000.0 ;
  char *vref = dataset_->getVREF() ;
  if ( strncmp( vref, "LSR", 3 ) == 0 ) {
    //strcat( vref, "K" ) ;
    if ( strlen( vref ) == 3 )
      strcat( vref, "K" ) ;
    else
      vref[3] = 'K' ;
  }
  freqref = vref ;
  NRODataRecord *record = dataset_->getRecord( 0 ) ;
  reffreq = record->FREQ0 ;
  bw = dataset_->getBEBW()[0] ;
  utc = getStartTime() ;
  fluxunit = "K" ;
  epoch = "UTC" ;  
  char *poltp = dataset_->getPOLTP()[0] ;
  if ( strcmp( poltp, "" ) == 0 ) 
    //poltp = "None" ;
    poltp = "linear" ;   // if no polarization type specified, set to "linear"
  //else if ( strcmp( poltp, "LINR" ) == 0 )
  else if ( strncmp( poltp, "LINR", 1 ) == 0 )
    poltp = "linear" ;
  //else if ( strcmp( poltp, "CIRL" ) == 0 )
  else if ( strncmp( poltp, "CIRL", 1 ) == 0 )
    poltp = "circular" ;
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
  NRODataRecord *record = dataset_->getRecord( i ) ;
  string s = record->SCANTP ;

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
  // DEBUG
  //cout << "NROReader::getScanInfo() " << endl ;
  //
  NRODataRecord *record = dataset_->getRecord( irow ) ;

  // scanno
  scanno = (uInt)(record->ISCAN) ;

  // cycleno
  cycleno = 0 ;

  // beamno
  string arryt = string( record->ARRYT ) ;
  string sbeamno = arryt.substr( 1, arryt.size()-1 ) ;
  uInt ibeamno = atoi( sbeamno.c_str() ) ; 
  beamno = ibeamno - 1 ;

  // polno
  polno = 0 ;

  // freqs (for IFNO and FREQ_ID)
  freqs = getFrequencies( irow ) ;

  // restfreq (for MOLECULE_ID)
  Vector<Double> rf( IPosition( 1, 1 ) ) ;
  rf( 0 ) = record->FREQ0 ;
  restfreq = rf ;

  // refbeamno
  refbeamno = 0 ;

  // scantime
  scantime = Double( dataset_->getStartIntTime( irow ) ) ;

  // interval
  interval = Double( dataset_->getIPTIM() ) ;

  // srcname
  srcname = String( dataset_->getOBJ() ) ;

  // fieldname
  fieldname = String( dataset_->getOBJ() ) ;

  // spectra
  vector<double> spec = dataset_->getSpectrum( irow ) ;
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
  Array<Float> tmp( IPosition( 1, 1 ), record->TSYS ) ;
  tsys = tmp ;

  // direction
  direction = getDirection( irow ) ;

  // azimuth
  azimuth = record->RAZ ;

  // elevation
  elevation = record->REL ;

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
  temperature = Float( record->TEMP ) ;

  // pressure (for WEATHER_ID)
  pressure = Float( record->PATM ) ;

  // humidity (for WEATHER_ID) 
  humidity = Float( record->PH2O ) ;

  // windvel (for WEATHER_ID)
  windvel = Float( record->VWIND ) ;

  // winddir (for WEATHER_ID)
  winddir = Float( record->DWIND ) ;

  // srcvel 
  srcvel = dataset_->getURVEL() ;

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

Int NROReader::getRowNum()
{
  return dataset_->getRowNum() ;
}

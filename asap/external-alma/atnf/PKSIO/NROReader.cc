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

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>

#include <casa/Logging/LogIO.h>
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
  LogIO os( LogOrigin( "", "getNROReader()", WHERE ) ) ;

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
    //os << LogIO::NORMAL << "getNROReader:: buf = " << String(buf) << LogIO::POST ;
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
      d->initialize() ;
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
        d->initialize() ;
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

  // DEBUG
  os << LogIO::NORMAL << "Data format of " << filename << ": " << datatype << LogIO::POST ;
  //

  // return reader if exists
  if ( reader ) {
    reader->read() ;
    return reader ;
  }

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
NROReader::NROReader( string name ) 
  : dataset_( NULL ),
    srcdir_( 0 ),
    msrcdir_( 0 ),
    freqRefFromVREF_( false ) // import frequency as REST
{
  // initialization
  filename_ = name ;
  coord_ = -1 ;
}

// destructor
NROReader::~NROReader()
{
}

// Read data header
Int NROReader::read() 
{
  LogIO os( LogOrigin( "NROReader", "read()", WHERE ) ) ;

  int status = 0 ;

  // initialize Dataset object
  initDataset();
  
  // fill Dataset
  status = dataset_->fillHeader() ;

  if ( status != 0 ) {
    os << LogIO::SEVERE << "Failed to fill data header." << LogIO::EXCEPTION ;
  }

  return status ;
}

// set frequency reference frame
void NROReader::setFreqRefFromVREF( bool fromVREF )
{
  os_.origin( LogOrigin( "NROReader", "setFreqRefFromVREF", WHERE ) ) ;
  os_ << ((fromVREF) ? "Take frequency reference frame from VREF" : 
          "Use frequency reference frame REST") << LogIO::POST ;

  freqRefFromVREF_ = fromVREF ; 
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
  //char *startTime = dataset_->getLOSTM() ;
  string startTime = dataset_->getLOSTM() ;
  //cout << "getStartTime()  startTime = " << startTime << endl ;
  return getMJD( startTime ) ;
}

double NROReader::getEndTime() 
{
  //char *endTime = dataset_->getLOETM() ;
  string endTime = dataset_->getLOETM() ;
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
  string strStartTime = string( time ) ;
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

double NROReader::getMJD( string strStartTime ) 
{
  // TODO: should be checked which time zone the time depends on
  // 2008/11/14 Takeshi Nakazato
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
  if ( msrcdir_.nelements() == 2 )
    return msrcdir_ ;

  srcdir_.resize( 2 ) ;
  srcdir_[0] = Double( dataset_->getRA0() ) ;
  srcdir_[1] = Double( dataset_->getDEC0() ) ;
  char epoch[5] ; 
  strncpy( epoch, (dataset_->getEPOCH()).c_str(), 5 ) ;
  if ( strncmp( epoch, "B1950", 5 ) == 0 ) {
    // convert to J2000 value
    MDirection result = 
      MDirection::Convert( MDirection( Quantity( srcdir_[0], "rad" ),
				       Quantity( srcdir_[1], "rad" ),
				       MDirection::Ref( MDirection::B1950 ) ),
			   MDirection::Ref( MDirection::J2000 ) ) () ;
    msrcdir_ = result.getAngle().getValue() ;
    if ( msrcdir_[0] < 0.0 && srcdir_[0] >= 0.0 )
      msrcdir_[0] = 2.0 * M_PI + msrcdir_[0] ;
    //cout << "NROReader::getSourceDirection()  SRCDIRECTION convert from (" 
    //<< srcra << "," << srcdec << ") B1950 to (" 
    //<< v( 0 ) << ","<< v( 1 ) << ") J2000" << endl ;
    //LogIO os( LogOrigin( "NROReader", "getSourceDirection()", WHERE ) ) ;
    //os << LogIO::NORMAL << "SRCDIRECTION convert from (" 
    //   << srcra << "," << srcdec << ") B1950 to (" 
    //   << v( 0 ) << ","<< v( 1 ) << ") J2000" << LogIO::POST ;
  }
  else if ( strncmp( epoch, "J2000", 5 ) == 0 ) {
    msrcdir_.reference( srcdir_ ) ;
  }
   
  return msrcdir_ ;
}

// Get DIRECTION in RADEC(J2000)
Vector<Double> NROReader::getDirection( int i )
{
  Vector<Double> v( 2 ) ;
  const NRODataRecord *record = dataset_->getRecord( i ) ;
  char epoch[5] ;
  strncpy( epoch, (dataset_->getEPOCH()).c_str(), 5 ) ;
  int icoord = dataset_->getSCNCD() ;
  double scantime = dataset_->getScanTime( i ) ;
  initConvert( icoord, scantime, epoch ) ;
  v[0]= Double( record->SCX ) ;
  v[1] = Double( record->SCY ) ;
  if ( converter_.null() )
    // no conversion 
    return v ;
  else {
    Vector<Double> v2 = (*converter_)( v ).getAngle().getValue() ; 
    if ( v2[0] < 0.0 && v[0] >= 0.0 ) 
      v2[0] = 2.0 * M_PI + v2[0] ;
    return v2 ;
  }
}

void NROReader::initConvert( int icoord, double t, char *epoch )
{
  if ( icoord == 0 && strncmp( epoch, "J2000", 5 ) == 0 )
    // no conversion 
    return ;

  if ( converter_.null() || icoord != coord_ ) {
    LogIO os( LogOrigin( "NROReader", "initConvert()", WHERE ) ) ;
    coord_ = icoord ;
    if ( coord_ == 0 ) {
      // RADEC (B1950) -> RADEC (J2000)
      os << "Creating converter from RADEC (B1950) to RADEC (J2000)" << LogIO::POST ;
      converter_ = new MDirection::Convert( MDirection::B1950,
                                            MDirection::J2000 ) ;
    }
    else if ( coord_ == 1 ) {
      // LB -> RADEC (J2000)
      os << "Creating converter from GALACTIC to RADEC (J2000)" << LogIO::POST ;
      converter_ = new MDirection::Convert( MDirection::GALACTIC,
                                            MDirection::J2000 ) ;
    }
    else {
      // coord_ == 2
      // AZEL -> RADEC (J2000)
      os << "Creating converter from AZEL to RADEC (J2000)" << LogIO::POST ;
      if ( mf_.null() ) {
        mf_ = new MeasFrame() ;
        vector<double> antpos = getAntennaPosition() ;
        Vector<Quantity> qantpos( 3 ) ;
        for ( int ip = 0 ; ip < 3 ; ip++ )
          qantpos[ip] = Quantity( antpos[ip], "m" ) ;
        mp_ = MPosition( MVPosition( qantpos ), MPosition::ITRF ) ;
        mf_->set( mp_ ) ;
      }
      converter_ = new MDirection::Convert( MDirection::AZEL,
                                            MDirection::Ref(MDirection::J2000,
                                                            *mf_ ) ) ;
    }
  }

  if ( coord_ == 2 ) {
    me_ = MEpoch( Quantity( t, "d" ), MEpoch::UTC ) ;
    mf_->set( me_ ) ;
  }
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
  //cout << "nchan = " << nchan << endl ;
  npol = getPolarizationNum() ;
  //cout << "npol = " << npol << endl ;
  observer = dataset_->getOBSVR() ;
  //cout << "observer = " << observer << endl ;
  project = dataset_->getPROJ() ;
  //cout << "project = " << project << endl ;
  obstype = dataset_->getSWMOD() ;
  //cout << "obstype = " << obstype << endl ;
  antname = dataset_->getSITE() ;
  //cout << "antname = " << antname << endl ;
  // TODO: should be investigated antenna position since there are 
  //       no corresponding information in the header
  // 2008/11/13 Takeshi Nakazato
  // 
  // INFO: tentative antenna posiiton is obtained for NRO 45m from ITRF website
  // 2008/11/26 Takeshi Nakazato
  vector<double> pos = getAntennaPosition() ;
  antpos = pos ;
  //cout << "antpos = " << antpos << endl ;
  string eq = dataset_->getEPOCH() ;
//   if ( eq.compare( 0, 5, "B1950" ) == 0 )
//     equinox = 1950.0 ;
//   else if ( eq.compare( 0, 5, "J2000" ) == 0 ) 
//     equinox = 2000.0 ;
  // equinox is always 2000.0
  equinox = 2000.0 ;
  //cout << "equinox = " << equinox << endl ;
  string vref = dataset_->getVREF() ;
  if ( vref.compare( 0, 3, "LSR" ) == 0 ) {
    if ( vref.size() == 3 ) {
      vref.append( "K" ) ;
    }
    else {
      vref[3] = 'K' ;
    }
  }
  else if ( vref.compare( 0, 3, "GAL" ) == 0 ) {
    vref = "GALACTO" ;
  }
  else if (vref.compare( 0, 3, "HEL" ) == 0 ) {
    os_.origin( LogOrigin( "NROReader", "getHeaderInfo", WHERE ) ) ;
    os_ << LogIO::WARN << "Heliocentric frame is not supported. Use Barycentric frame instead." << LogIO::POST ;
    vref = "BARY" ;
  }
  //freqref = vref ;
  //freqref = "LSRK" ;
  //freqref = "REST" ;
  freqref = freqRefFromVREF_ ? vref : "REST" ; 
  //cout << "freqref = " << freqref << endl ;
  const NRODataRecord *record = dataset_->getRecord( 0 ) ;
  reffreq = record->FREQ0 ; // rest frequency

  //cout << "reffreq = " << reffreq << endl ;
  bw = dataset_->getBEBW()[0] ;
  //cout << "bw = " << bw << endl ;
  utc = getStartTime() ;
  //cout << "utc = " << utc << endl ;
  fluxunit = "K" ;
  //cout << "fluxunit = " << fluxunit << endl ;
  epoch = "UTC" ;  
  //cout << "epoch = " << epoch << endl ;
  string poltp = dataset_->getPOLTP()[0] ;
  //cout << "poltp = '" << poltp << "'" << endl ;
  if ( poltp.empty() || poltp[0] == ' ' || poltp[0] == '\0' ) 
    //poltp = "None" ;
    poltp = "linear" ;   // if no polarization type specified, set to "linear"
  //else if ( strcmp( poltp, "LINR" ) == 0 )
  else if ( poltp.compare( 0, 1, "LINR", 0, 1 ) == 0 )
    poltp = "linear" ;
  //else if ( strcmp( poltp, "CIRL" ) == 0 )
  else if ( poltp.compare( 0, 1, "CIRL", 0, 1 ) == 0 )
    poltp = "circular" ;
  poltype = poltp ;
  //cout << "poltype = '" << poltype << "'" << endl ;

  //vector<Bool> ifs = getIFs() ;
  //nif = ifs.size() ;
  nif = getNumIF() ;
  //cout << "nif = " << nif << endl ;

  //vector<Bool> beams = getBeams() ;
  //nbeam = beams.size() ;
  nbeam = getNumBeam() ;
  //cout << "nbeam = " << nbeam << endl ;

  return 0 ;
}

string NROReader::getScanType( int i )
{
  const NRODataRecord *record = dataset_->getRecord( i ) ;
  string s = record->SCANTP ;

  return s ;
}

int NROReader::getScanInfo( int irow,
                            uInt &scanno,
                            uInt &cycleno,
                            uInt &ifno,
                            uInt &beamno,
                            uInt &polno,
                            vector<double> &freqs,   
                            Vector<Double> &restfreq,
                            uInt &refbeamno,
                            Double &scantime,
                            Double &interval,
                            String &srcname,
                            String &fieldname,
                            Vector<Float> &spectra,
                            Vector<uChar> &flagtra,
                            Vector<Float> &tsys,
                            Vector<Double> &direction,
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
                            Vector<Double> &/*propermotion*/,
                            Vector<Double> &srcdir,
                            Vector<Double> &/*scanrate*/ )
{
  static const IPosition oneByOne( 1, 1 );

  // DEBUG
  //cout << "NROReader::getScanInfo()  irow = " << irow << endl ;
  //
  NRODataRecord *record = dataset_->getRecord( irow ) ;

  // scanno
  scanno = (uInt)(record->ISCAN) ;
  //cout << "scanno = " << scanno << endl ;

  // cycleno
  cycleno = 0 ;
  //cout << "cycleno = " << cycleno << endl ;

  // beamno and ifno
  string rxname = dataset_->getRX()[0] ;
  if ( rxname.find("MULT2") != string::npos ) {
    string arryt = string( record->ARRYT ) ;
    beamno = dataset_->getSortedArrayId( arryt ) ;
    ifno = 0 ;
  }
  else {
    beamno = 0 ;
    string arryt = string( record->ARRYT ) ;
    ifno = dataset_->getSortedArrayId( arryt ) ;
  }
  //cout << "beamno = " << beamno << endl ;

  // polno
  polno = dataset_->getPolNo( irow ) ;
  //cout << "polno = " << polno << endl ;

  // freqs (for IFNO and FREQ_ID)
  //freqs = getFrequencies( irow ) ;
  freqs = dataset_->getFrequencies( irow ) ;
  //cout << "freqs = [" << freqs[0] << ", " << freqs[1] << ", " << freqs[2] << "]" << endl ;

  if ( freqRefFromVREF_ ) {
    freqs = shiftFrequency( freqs, 
                            dataset_->getURVEL(), 
                            dataset_->getVDEF() ) ;
  }

  // restfreq (for MOLECULE_ID)
  restfreq.resize( oneByOne ) ;
  restfreq[0] = record->FREQ0 ;
  //cout << "restfreq = " << restfreq[0] << endl ;

  // refbeamno
  refbeamno = 0 ;
  //cout << "refbeamno = " << refbeamno << endl ;

  // scantime
  //scantime = Double( dataset_->getStartIntTime( irow ) ) ;
  scantime = Double( dataset_->getScanTime( irow ) ) ;
  //cout << "scantime = " << scantime << endl ;

  // interval
  interval = Double( dataset_->getIPTIM() ) ;
  //cout << "interval = " << interval << endl ;

  // srcname
  srcname = String( dataset_->getOBJ() ) ;
  //cout << "srcname = " << srcname << endl ;

  // fieldname
  fieldname = String( dataset_->getOBJ() ) ;
  //cout << "fieldname = " << fieldname << endl ;

  // spectra
  vector<double> spec = dataset_->getSpectrum( irow ) ;
  spectra.resize( spec.size() ) ;
  //int index = 0 ;
  Bool b ;
  Float *fp = spectra.getStorage( b ) ;
  Float *wp = fp ;
  for ( vector<double>::iterator i = spec.begin() ;
        i != spec.end() ; i++ ) {
    *wp = *i ;
    wp++ ;
  }
  spectra.putStorage( fp, b ) ;
  //cout << "spec.size() = " << spec.size() << endl ;
  
  // flagtra
  bool setValue = !( flagtra.nelements() == spectra.nelements() ) ; 
  if ( setValue ) {
    //cout << "flagtra resized. reset values..." << endl ;
    flagtra.resize( spectra.nelements() ) ;
    flagtra.set( 0 ) ;
  }
  //cout << "flag.size() = " << flagtra.size() << endl ;

  // tsys
  tsys.resize( oneByOne ) ;
  tsys[0] = record->TSYS ;
  //cout << "tsys[0] = " << tsys[0] << endl ;

  // direction
  direction = getDirection( irow ) ;
  //cout << "direction = [" << direction[0] << ", " << direction[1] << "]" << endl ;

  // azimuth
  azimuth = record->RAZ ;
  //cout << "azimuth = " << azimuth << endl ;

  // elevation
  elevation = record->REL ;
  //cout << "elevation = " << elevation << endl ;

  // parangle
  parangle = 0.0 ;
  //cout << "parangle = " << parangle << endl ;

  // opacity
  opacity = 0.0 ;
  //cout << "opacity = " << opacity << endl ;

  // tcalid 
  tcalid = 0 ;
  //cout << "tcalid = " << tcalid << endl ;

  // fitid
  fitid = -1 ;
  //cout << "fitid = " << fitid << endl ;

  // focusid 
  focusid = 0 ;
  //cout << "focusid = " << focusid << endl ;

  // temperature (for WEATHER_ID)
  temperature = Float( record->TEMP ) ;
  //cout << "temperature = " << temperature << endl ;

  // pressure (for WEATHER_ID)
  pressure = Float( record->PATM ) ;
  //cout << "pressure = " << pressure << endl ;

  // humidity (for WEATHER_ID) 
  humidity = Float( record->PH2O ) ;
  //cout << "humidity = " << humidity << endl ;

  // windvel (for WEATHER_ID)
  windvel = Float( record->VWIND ) ;
  //cout << "windvel = " << windvel << endl ;

  // winddir (for WEATHER_ID)
  winddir = Float( record->DWIND ) ;
  //cout << "winddir = " << winddir << endl ;
  
  // srcvel 
  srcvel = dataset_->getURVEL() ;
  //cout << "srcvel = " << srcvel << endl ;

  // propermotion
  // do nothing

  // srcdir
  srcdir = getSourceDirection() ;
  //cout << "srcdir = [" << srcdir[0] << ", " << srcdir[1] << endl ;

  // scanrate
  // do nothing

  return 0 ;
}

Int NROReader::getRowNum()
{
  return dataset_->getRowNum() ;
}

vector<double> NROReader::shiftFrequency( const vector<double> &f, 
                                          const double &v, 
                                          const string &vdef )
{
  vector<double> r( f ) ;
  double factor = v / 2.99792458e8 ;
  if ( vdef.compare( 0, 3, "RAD" ) == 0 ) {
    factor = 1.0 / ( 1.0 + factor ) ;
    r[1] *= factor ;
    r[2] *= factor ;
  }
  else if ( vdef.compare( 0, 3, "OPT" ) == 0 ) {
    factor += 1.0 ;
    r[1] *= factor ;
    r[2] *= factor ;
  }
  else {
    cout << "vdef=" << vdef << " is not supported." << endl;
  }
  return r ;
}

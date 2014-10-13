//
// C++ Interface: NROFiller
//
// Description:
// This class is a concrete class that derives from FillerBase class.
// The class implements the following methods to be able to read NRO 
// data (45m and ASTE).
//
//    open()
//    close()
//    fill()
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "NROFiller.h"
#include "STHeader.h"
#include <casa/Containers/Record.h>
#include <casa/Quanta/MVTime.h>
#include <atnf/PKSIO/SrcType.h>
#include <casa/Logging/LogIO.h>

using namespace casa;

namespace asap
{
NROFiller::NROFiller(CountedPtr<Scantable> stable) 
  : FillerBase(stable)
{
}

NROFiller::~NROFiller()
{
  close() ;
}

bool NROFiller::open(const std::string& filename, const Record& rec) 
{
  bool status = true ;

  // Parse options
  String freqref = "DEFAULT (REST)" ;
  if ( rec.isDefined( "nro" ) ) {
    Record nrorec = rec.asRecord( "nro" ) ;
    if ( nrorec.isDefined( "freqref" ) ) {
      freqref = nrorec.asString( "freqref" ) ;
      freqref.upcase() ;
    }
    LogIO os( LogOrigin( "NROFiller", "open", WHERE ) ) ;
    os << "Parsing NRO options" << endl ;
    os << "   freqref = " << freqref << LogIO::POST ;
  }

  // get appropriate reader object
  String format ;
  reader_ = getNROReader( filename, format ) ;
  if ( reader_.null() == True ) {
    status = false ;
    return status ;
  }  

  // Apply options
  if ( freqref == "REST" || freqref == "DEFAULT (REST)" ) {
    reader_->setFreqRefFromVREF( false ) ;
  }
  else if ( freqref == "VREF" ) {
    reader_->setFreqRefFromVREF( true ) ;
  }
  
  // get header information
  STHeader hdr ;
  if ( reader_->getHeaderInfo( hdr.nchan,
                               hdr.npol,
                               hdr.nif, 
                               hdr.nbeam,
                               hdr.observer,
                               hdr.project,
                               hdr.obstype,
                               hdr.antennaname,
                               hdr.antennaposition,
                               hdr.equinox,
                               hdr.freqref,
                               hdr.reffreq,
                               hdr.bandwidth,
                               hdr.utc,
                               hdr.fluxunit,
                               hdr.epoch,
                               hdr.poltype ) ) {
    status = false ;
    return status ;
  }

  // FREQUENCIES table setup
  table_->frequencies().setFrame( hdr.freqref, true ) ;
  table_->frequencies().setFrame( hdr.freqref, false ) ;
  const string vref = reader_->dataset().getVREF() ;
  if ( vref.compare( 0, 3, "RAD" ) ) {
    table_->frequencies().setDoppler( "RADIO" ) ;
  }
  else if ( vref.compare( 0, 3, "OPT" ) ) {
    table_->frequencies().setDoppler( "OPTICAL" ) ;
  }
  else {
    LogIO os( LogOrigin( "NROFiller", "open", WHERE ) ) ;
    os << LogIO::WARN 
       << "VREF " << vref << " is not supported. Use default (RADIO)." 
       << LogIO::POST ; 
  }

  // set Header
  setHeader( hdr ) ;

  return status ;
}

void NROFiller::fill() 
{
  // for each row
  uInt nRow = reader_->getRowNum() ;
  vector< vector<double > > freqs ;
  uInt scanno ;
  uInt cycleno ;
  uInt ifno ;
  uInt beamno ;
  uInt polno ;
  vector<double> fqs ;
  Vector<Double> restfreq ;
  uInt refbeamno ;
  Double scantime ;
  Double interval ;
  String srcname ;
  String fieldname ;
  Vector<Float> spectra ;
  Vector<uChar> flagtra ;
  Vector<Float> tsys ;
  Vector<Double> direction ;
  Float azimuth ;
  Float elevation ;
  Float parangle = 0.0 ;
  Float opacity ;
  uInt tcalid ;
  Int fitid ;
  uInt focusid ;
  Float temperature ;
  Float pressure ;
  Float humidity ;
  Float windvel ;
  Float winddir ;
  Double srcvel ;
  Vector<Double> propermotion( 2, 0.0 ) ;
  Vector<Double> srcdir ;
  Vector<Double> scanrate( 2, 0.0 ) ;
  Int rowCount = 0 ;

  STHeader header = table_->getHeader() ;
  String obsType = header.obstype.substr( 0, 3 ) ;
  Vector<Float> defaultTcal( 1, 1.0 ) ;
  String tcalTime = MVTime( header.utc ).string( MVTime::YMD ) ;

  // TCAL subtable rows
  setTcal( tcalTime, defaultTcal ) ;

  // FOCUS subtable rows
  setFocus( parangle ) ;

  for ( Int irow = 0 ; irow < (Int)nRow ; irow++ ) {
    // check scan intent
    string scanType = reader_->getScanType( irow ) ;
    SrcType::type srcType = SrcType::NOTYPE ;

    // skip "ZERO" scan
    if ( scanType.compare( 0, 4, "ZERO" ) == 0 ) {
      continue ;
    }

    // set srcType 
    if ( obsType == "POS" || obsType == "BEA" ) {
      if ( scanType.compare( 0, 2, "ON" ) == 0 ) 
        srcType = SrcType::PSON ;
      else if ( scanType.compare( 0, 3, "OFF" ) == 0 ) 
        srcType = SrcType::PSOFF ;
    }
    else if ( obsType == "FRE" ) {
      if ( scanType.compare( 0, 2, "ON" ) == 0 ) 
        srcType = SrcType::FSON ;
      else if ( scanType.compare( 0, 3, "OFF" ) == 0 ) 
        srcType = SrcType::FSOFF ;
    }
 
    // get scan ifnromation
    if ( reader_->getScanInfo( irow,
                               scanno,
                               cycleno,
                               ifno,
                               beamno,
                               polno,
                               fqs,
                               restfreq,
                               refbeamno,
                               scantime,
                               interval,
                               srcname,
                               fieldname,
                               spectra,
                               flagtra,
                               tsys,
                               direction,
                               azimuth,
                               elevation,
                               parangle,
                               opacity,
                               tcalid,
                               fitid,
                               focusid,
                               temperature,
                               pressure,
                               humidity,
                               windvel,
                               winddir,
                               srcvel,
                               propermotion,
                               srcdir,
                               scanrate ) ) {
      throw AipsError( "Failed to read scan record" ) ;
      return ;
    }

    // set IDs and subtable rows
    // FREQUENCIES subtable row
    setFrequency( (Double)fqs[0], (Double)fqs[1], (Double)fqs[2] ) ;

    // MOLECULES subtable row
    setMolecule( restfreq ) ;

    // WEATHER subtable row
    float p = 7.5 * temperature / ( 273.3 + temperature ) ;
    float sh = 6.11 * powf( 10, p ) ;
    temperature += 273.15 ; // C->K
    winddir *= C::degree ; // deg->rad
    humidity /= sh ; // P_H2O->relative humidity
    setWeather2( temperature, pressure, humidity, windvel, winddir ) ;

    // set row attributes
    // SPECTRA, FLAGTRA, and TSYS
    setSpectrum( spectra, flagtra, tsys ) ;

    // SCANNO, CYCLENO, IFNO, POLNO, and BEAMNO
    setIndex( scanno, cycleno, ifno, polno, beamno ) ;

    // REFBEAMNO
    setReferenceBeam( (Int)refbeamno ) ;

    // DIRECTION
    setDirection( direction, azimuth, elevation ) ;

    // TIME and INTERVAL
    setTime( scantime, interval ) ;

    // SRCNAME, SRCTYPE, FIELDNAME, SRCDIRECTION, SRCPROPERMOTION, and SRCVELOCITY
    setSource( srcname, srcType, fieldname, srcdir, propermotion, srcvel ) ;

    // SCANRATE
    setScanRate( scanrate ) ;

    // OPACITY
    setOpacity( opacity ) ;

    // finally, commit row
    commitRow() ;

    // count up number of row committed 
    rowCount++ ;
  }
}

void NROFiller::close() 
{
  if (reader_.null() != False) {
    reader_ = 0;
  }
  table_ = 0;
}

};


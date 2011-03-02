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
#include <atnf/PKSIO/SrcType.h>

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

  // get appropriate reader object
  String format ;
  reader_ = getNROReader( filename, format ) ;
  if ( reader_.null() == True ) {
    status = false ;
    return status ;
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

  // 2010/07/30 TBD: Do we need to set frame here?
  table_->frequencies().setFrame( hdr.freqref, true ) ;
  table_->frequencies().setFrame( String("LSRK"), false ) ;

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
  uInt beamno ;
  uInt polno ;
  vector<double> fqs ;
  Vector<Double> restfreq ;
  uInt refbeamno ;
  Double scantime ;
  Double interval ;
  String srcname ;
  String fieldname ;
  Array<Float> spectra ;
  Array<uChar> flagtra ;
  Array<Float> tsys ;
  Array<Double> direction ;
  Float azimuth ;
  Float elevation ;
  Float parangle ;
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
  Array<Double> propermotion ;
  Vector<Double> srcdir ;
  Array<Double> scanrate ;
  Int rowCount = 0 ;

  STHeader header = table_->getHeader() ;
  String obsType = header.obstype.substr( 0, 3 ) ;
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

    // FOCUS subtable row
    setFocus( parangle ) ;

    // WEATHER subtable row 
    setWeather( temperature, pressure, humidity, windvel, winddir ) ;

    // TCAL subtable row
    // use default since NRO input is calibrated data
    setTcal() ;
    

    // set row attributes
    // SPECTRA, FLAGTRA, and TSYS
    Vector<Float> spectrum( spectra );
    Vector<uChar> flags( flagtra ) ;
    Vector<Float> Tsys( tsys ) ;
    setSpectrum( spectrum, flags, Tsys ) ;

    // SCANNO, CYCLENO, IFNO, POLNO, and BEAMNO
    uInt ifno = table_->frequencies().addEntry( (Double)fqs[0], (Double)fqs[1], (Double)fqs[2] ) ;
    setIndex( scanno, cycleno, ifno, polno, beamno ) ;

    // REFBEAMNO
    setReferenceBeam( (Int)refbeamno ) ;

    // DIRECTION
    Vector<Double> dir( direction ) ;
    setDirection(dir, azimuth, elevation ) ;

    // TIME and INTERVAL
    setTime( scantime, interval ) ;

    // SRCNAME, SRCTYPE, FIELDNAME, SRCDIRECTION, SRCPROPERMOTION, and SRCVELOCITY
    Vector<Double> propermot( propermotion ) ;
    setSource( srcname, srcType, fieldname, srcdir, propermot, srcvel ) ;

    // SCANRATE
    Vector<Double> srate( scanrate ) ;
    setScanRate( srate ) ;

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


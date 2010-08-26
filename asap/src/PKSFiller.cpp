//
// C++ Implementation: PKSFiller
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/iostream.h>
#include <casa/iomanip.h>

#include <casa/Exceptions.h>
#include <casa/OS/Path.h>
#include <casa/OS/File.h>
#include <casa/Quanta/Unit.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Utilities/Regex.h>
#include <casa/Logging/LogIO.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>

#include <atnf/PKSIO/PKSrecord.h>
#include <atnf/PKSIO/PKSreader.h>

#ifdef HAS_ALMA
 #include <casa/System/ProgressMeter.h>
#endif
#include <casa/Logging/LogIO.h>

#include <time.h>

#include "STDefs.h"
#include "STAttr.h"

#include "PKSFiller.h"
#include "STHeader.h"

using namespace casa;

namespace asap {


PKSFiller::PKSFiller( CountedPtr< Scantable > stbl ) :
  FillerBase(stbl),
  reader_(0)
{
  setReferenceRegex(".*(e|w|_R)$");
}

PKSFiller::~PKSFiller()
{
  close();
}

bool PKSFiller::open( const std::string& filename)
{
  Bool haveBase, haveSpectra;

  String inName(filename);
  Path path(inName);
  inName = path.expandedName();

  File file(inName);
  filename_ = inName;

  // Create reader and fill in values for arguments
  String format;
  Vector<Bool> beams, ifs;
  Vector<uInt> nchans,npols;

  String antenna("");

  reader_ = getPKSreader(inName, antenna, 0, 0, format, beams, ifs,
                         nchans, npols, haveXPol_, haveBase, haveSpectra);
  if (reader_.null() == True) {
    return False;
  }

  if (!haveSpectra) {
    reader_ = 0;
    throw(AipsError("No spectral data in file."));
  }

  LogIO os( LogOrigin( "PKSFiller", "open()", WHERE ) ) ;
  nBeam_ = beams.nelements();
  nIF_ = ifs.nelements();
  // Get basic parameters.
  if ( anyEQ(haveXPol_, True) ) {
    os <<  "Cross polarization present" << LogIO::POST;
    for (uInt i=0; i< npols.nelements();++i) {
      if (npols[i] < 3) npols[i] += 2;// Convert Complex -> 2 Floats
    }
  }
  STHeader header;
  header.nchan = max(nchans);
  header.npol = max(npols);
  header.nbeam = nBeam_;

  Int status = reader_->getHeader(header.observer, header.project,
                                  header.antennaname, header.antennaposition,
                                  header.obstype,
                                  header.fluxunit,
                                  header.equinox,
                                  header.freqref,
                                  header.utc, header.reffreq,
                                  header.bandwidth);

  if (status) {
    reader_ = 0;
    throw(AipsError("Failed to get header."));
  }
  os << "Found " << header.antennaname << " data." << LogIO::POST;
  if ((header.obstype).matches("*SW*")) {
    // need robust way here - probably read ahead of next timestamp
    os << "Header indicates frequency switched observation.\n"
       << "setting # of IFs = 1 " << LogIO::POST;

    nIF_ = 1;
    header.obstype = String("fswitch");
  }
  // Determine Telescope and set brightness unit
  Bool throwIt = False;
  Instrument inst = STAttr::convertInstrument(header.antennaname, throwIt);

  if (inst==ATMOPRA || inst==TIDBINBILLA) {
    header.fluxunit = "K";
  } else {
    // downcase for use with Quanta
    if (header.fluxunit == "JY") {
      header.fluxunit = "Jy";
    }
  }
  STAttr stattr;
  header.poltype = stattr.feedPolType(inst);
  header.nif = nIF_;
  header.epoch = "UTC";
  // *** header.frequnit = "Hz"
  // Apply selection criteria.
  Vector<Int> ref;

  Vector<Int> start(nIF_, 1);
  Vector<Int> end(nIF_, 0);
  Bool getPt = False;
  reader_->select(beams, ifs, start, end, ref, True, haveXPol_[0], False, getPt);
  setHeader(header);
  //For MS, add the location of POINTING of the input MS so one get
  //pointing data from there, if necessary.
  //Also find nrow in MS
  nInDataRow = 0;
  if (format == "MS2") {
    Path datapath(inName);
    String ptTabPath = datapath.absoluteName();
    Table inMS(ptTabPath);
    nInDataRow = inMS.nrow();
    ptTabPath.append("/POINTING");
    table_->table().rwKeywordSet().define("POINTING", ptTabPath);
    if (header.antennaname.matches("GBT")) {
      String GOTabPath = datapath.absoluteName();
      GOTabPath.append("/GBT_GO");
      table_->table().rwKeywordSet().define("GBT_GO", GOTabPath);
    }
  }
  String freqFrame = header.freqref;
  //translate frequency reference frame back to
  //MS style (as PKSMS2reader converts the original frame
  //in FITS standard style)
  if (freqFrame == "TOPOCENT") {
    freqFrame = "TOPO";
  } else if (freqFrame == "GEOCENER") {
    freqFrame = "GEO";
  } else if (freqFrame == "BARYCENT") {
    freqFrame = "BARY";
  } else if (freqFrame == "GALACTOC") {
    freqFrame = "GALACTO";
  } else if (freqFrame == "LOCALGRP") {
    freqFrame = "LGROUP";
  } else if (freqFrame == "CMBDIPOL") {
    freqFrame = "CMB";
  } else if (freqFrame == "SOURCE") {
    freqFrame = "REST";
  }
  // set both "FRAME" and "BASEFRAME"
  table_->frequencies().setFrame(freqFrame, false);
  table_->frequencies().setFrame(freqFrame, true);

  return true;
}

void PKSFiller::close( )
{
  if (reader_.null() != False) {
    reader_ = 0;
  }
  table_ = 0;
}

void PKSFiller::fill( )
{
  int status = 0;
  LogIO os( LogOrigin( "PKSFiller", "fill()", WHERE ) ) ;

/**
  Int    beamNo, IFno, refBeam, scanNo, cycleNo;
  Float  azimuth, elevation, focusAxi, focusRot, focusTan,
    humidity, parAngle, pressure, temperature, windAz, windSpeed;
  Double bandwidth, freqInc, interval, mjd, refFreq, srcVel;
  String          fieldName, srcName, tcalTime, obsType;
  Vector<Float>   calFctr, sigma, tcal, tsys;
  Matrix<Float>   baseLin, baseSub;
  Vector<Double>  direction(2), scanRate(2), srcDir(2), srcPM(2), restFreq(1);
  Matrix<Float>   spectra;
  Matrix<uChar>   flagtra;
  Complex         xCalFctr;
  Vector<Complex> xPol;
**/

  Double min = 0.0;
  Double max = nInDataRow;
#ifdef HAS_ALMA
  ProgressMeter fillpm(min, max, "Data importing progress");
#endif
  PKSrecord pksrec;
  pksrec.srcType=-1;
  int n = 0;
  bool isGBTFITS = false ;
  if ((table_->getAntennaName().find( "GBT" ) != String::npos)
       && File(filename_).isRegular()) {
    FILE *fp = fopen( filename_.c_str(), "r" ) ;
    fseek( fp, 640, SEEK_SET ) ;
    char buf[81] ;
    fread( buf, 80, 1, fp ) ;
    buf[80] = '\0' ;
    if ( strstr( buf, "NRAO_GBT" ) != NULL ) {
      isGBTFITS = true ;
    }
    fclose( fp ) ;
  }

  while ( status == 0 ) {

    status = reader_->read(pksrec);
    if ( status != 0 ) break;
    n += 1;
    Regex filterrx(".*[SL|PA]$");
    Regex obsrx("^AT.+");
    if ( table_->getAntennaName().matches(obsrx) &&
         pksrec.obsType.matches(filterrx)) {
        //cerr << "ignoring paddle scan" << endl;
        continue;
    }

    Vector<Double> sratedbl(pksrec.scanRate.nelements());
    convertArray(sratedbl, pksrec.scanRate);
    setScanRate(sratedbl);

    Regex rx(getReferenceRegex());
    Regex rx2("_S$");
    Int match = pksrec.srcName.matches(rx);
    std::string srcname;
    Int srctype = Int(SrcType::NOTYPE);
    if (match) {
      srcname = pksrec.srcName;
      srctype =  Int(SrcType::PSOFF);
    } else {
      srcname = pksrec.srcName.before(rx2);
      srctype =  Int(SrcType::PSON);
    }
    if ( pksrec.srcType != Int(SrcType::NOTYPE)) {
      srctype = pksrec.srcType ;
    }
    setTime(pksrec.mjd, pksrec.interval);
    setSource(srcname, srctype, pksrec.fieldName,
              pksrec.srcDir, pksrec.srcPM, pksrec.srcVel);

    if (nBeam_ > 1 ) {
      setReferenceBeam(pksrec.refBeam-1);
    }

    setMolecule(pksrec.restFreq);
    setTcal(pksrec.tcalTime, pksrec.tcal);
    setWeather(pksrec.temperature, pksrec.pressure,
                                    pksrec.humidity, pksrec.windSpeed,
                                    pksrec.windAz);
    setFocus(pksrec.parAngle, pksrec.focusAxi, pksrec.focusTan,
             pksrec.focusRot);
    setDirection(pksrec.direction, pksrec.azimuth, pksrec.elevation);

    setFrequency(Double(pksrec.spectra.nrow()/2),
                 pksrec.refFreq, pksrec.freqInc);

    // Note: this is only one value for all polarisations!
    setFlagrow(pksrec.flagrow);
    // Turn the (nchan,npol) matrix and possible complex xPol vector
    // into 2-4 rows in the scantable
    Vector<Float> tsysvec(1);
    // Why is spectra.ncolumn() == 3 for haveXPol_ == True
    uInt npol = (pksrec.spectra.ncolumn()==1 ? 1: 2);
    uInt polno =0;
    for ( uInt i=0; i< npol; ++i ) {
      tsysvec = pksrec.tsys(i);
      if (isGBTFITS) {
        polno = pksrec.polNo ;
      } else {
        polno = i;
      }
      setIndex(pksrec.scanNo-1, pksrec.cycleNo-1, pksrec.IFno-1, polno,
               pksrec.beamNo-1);
      setSpectrum(pksrec.spectra.column(i), pksrec.flagged.column(i), tsysvec);
      commitRow();
    }
    if ( haveXPol_[0] ) {
      // no tsys given for xpol, so emulate it
      tsysvec = sqrt(pksrec.tsys[0]*pksrec.tsys[1]);
      // add real part of cross pol
      polno = 2;
      Vector<Float> r(real(pksrec.xPol));
      // make up flags from linears
      /// @fixme this has to be a bitwise or of both pols
      /// pksrec.flagged.column(0) | pksrec.flagged.column(1);

      setIndex(pksrec.scanNo-1, pksrec.cycleNo-1, pksrec.IFno-1, polno,
               pksrec.beamNo-1);
      setSpectrum(r, pksrec.flagged.column(0), tsysvec);
      commitRow();

      // ad imaginary part of cross pol
      polno = 3;
      Vector<Float> im(imag(pksrec.xPol));
      setIndex(pksrec.scanNo-1, pksrec.cycleNo-1, pksrec.IFno-1, polno,
               pksrec.beamNo-1);
      setSpectrum(im, pksrec.flagged.column(0), tsysvec);
      commitRow();
    }
#ifdef HAS_ALMA
    fillpm._update(n);
#endif
  }
  if (status > 0) {
    close();
    throw(AipsError("Reading error occured, data possibly corrupted."));
  }
#ifdef HAS_ALMA
  fillpm.done();
#endif
}

}//namespace asap

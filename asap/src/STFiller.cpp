//
// C++ Implementation: STFiller
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
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

#include <casa/Containers/RecordField.h>

#include <tables/Tables/TableRow.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MeasConvert.h>

#include <atnf/PKSIO/PKSreader.h>
#include <casa/System/ProgressMeter.h>
#include <atnf/PKSIO/NROReader.h>

#include <time.h>

#include "STDefs.h"
#include "STAttr.h"

#include "STFiller.h"
#include "STHeader.h"

using namespace casa;

namespace asap {

STFiller::STFiller() :
  reader_(0),
  header_(0),
  table_(0),
  nreader_(0)
{
}

STFiller::STFiller( CountedPtr< Scantable > stbl ) :
  reader_(0),
  header_(0),
  table_(stbl),
  nreader_(0)
{
}

STFiller::STFiller(const std::string& filename, int whichIF, int whichBeam ) :
  reader_(0),
  header_(0),
  table_(0),
  nreader_(0)
{
  open(filename, whichIF, whichBeam);
}

STFiller::~STFiller()
{
  close();
}

void STFiller::open( const std::string& filename, int whichIF, int whichBeam, casa::Bool getPt )
{
  if (table_.null())  {
    table_ = new Scantable();
  }
  if (reader_)  { delete reader_; reader_ = 0; }
  Bool haveBase, haveSpectra;

  String inName(filename);
  Path path(inName);
  inName = path.expandedName();

  File file(inName);
  if ( !file.exists() ) {
     throw(AipsError("File does not exist"));
  }
  filename_ = inName;

  // Create reader and fill in values for arguments
  String format;
  Vector<Bool> beams, ifs;
  Vector<uInt> nchans,npols;

  //
  // if isNRO_ is true, try NROReader
  //
  // 2008/11/11 Takeshi Nakazato
  isNRO_ = fileCheck() ;
  if ( isNRO_ ) {
    if ( (nreader_ = getNROReader( inName, format )) == 0 ) {
      throw(AipsError("Creation of NROReader failed")) ;
    }
    else {
      openNRO( whichIF, whichBeam ) ;
      return ;
    }
  }
  //

  if ( (reader_ = getPKSreader(inName, 0, 0, format, beams, ifs,
                              nchans, npols, haveXPol_,haveBase, haveSpectra
                              )) == 0 )  {
    throw(AipsError("Creation of PKSreader failed"));
  }
  if (!haveSpectra) {
    delete reader_;
    reader_ = 0;
    throw(AipsError("No spectral data in file."));
    return;
  }
  nBeam_ = beams.nelements();
  nIF_ = ifs.nelements();
  // Get basic parameters.
  if ( anyEQ(haveXPol_, True) ) {
    pushLog("Cross polarization present");
    for (uInt i=0; i< npols.nelements();++i) {
      if (npols[i] < 3) npols[i] += 2;// Convert Complex -> 2 Floats
    }
  }
  if (header_) delete header_;
  header_ = new STHeader();
  header_->nchan = max(nchans);
  header_->npol = max(npols);
  header_->nbeam = nBeam_;

  Int status = reader_->getHeader(header_->observer, header_->project,
                                  header_->antennaname, header_->antennaposition,
                                  header_->obstype,header_->equinox,
                                  header_->freqref,
                                  header_->utc, header_->reffreq,
                                  header_->bandwidth,
                                  header_->fluxunit);

  if (status) {
    delete reader_;
    reader_ = 0;
    delete header_;
    header_ = 0;
    throw(AipsError("Failed to get header."));
  }
  if ((header_->obstype).matches("*SW*")) {
    // need robust way here - probably read ahead of next timestamp
    pushLog("Header indicates frequency switched observation.\n"
               "setting # of IFs = 1 ");
    nIF_ = 1;
    header_->obstype = String("fswitch");
  }
  // Determine Telescope and set brightness unit


  Bool throwIt = False;
  Instrument inst = STAttr::convertInstrument(header_->antennaname, throwIt);
  //header_->fluxunit = "Jy";
  if (inst==ATMOPRA || inst==TIDBINBILLA) {
     header_->fluxunit = "K";
  }
  STAttr stattr;
  header_->poltype = stattr.feedPolType(inst);
  header_->nif = nIF_;
  header_->epoch = "UTC";
  // *** header_->frequnit = "Hz"
  // Apply selection criteria.
  Vector<Int> ref;
  ifOffset_ = 0;
  if (whichIF>=0) {
    if (whichIF>=0 && whichIF<nIF_) {
      ifs = False;
      ifs(whichIF) = True;
      header_->nif = 1;
      nIF_ = 1;
      ifOffset_ = whichIF;
    } else {
      delete reader_;
      reader_ = 0;
      delete header_;
      header_ = 0;
      throw(AipsError("Illegal IF selection"));
    }
  }
  beamOffset_ = 0;
  if (whichBeam>=0) {
    if (whichBeam>=0 && whichBeam<nBeam_) {
      beams = False;
      beams(whichBeam) = True;
      header_->nbeam = 1;
      nBeam_ = 1;
      beamOffset_ = whichBeam;
    } else {
      delete reader_;
      reader_ = 0;
      delete header_;
      header_ = 0;
      throw(AipsError("Illegal Beam selection"));
    }
  }
  Vector<Int> start(nIF_, 1);
  Vector<Int> end(nIF_, 0);
  reader_->select(beams, ifs, start, end, ref, True, haveXPol_[0], False, getPt);
  table_->setHeader(*header_);
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
    if ((header_->antennaname).matches("GBT")) {
      String GOTabPath = datapath.absoluteName();
      GOTabPath.append("/GBT_GO");
      table_->table().rwKeywordSet().define("GBT_GO", GOTabPath);
    }
  }
  String freqFrame = header_->freqref;
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
  table_->frequencies().setFrame(freqFrame);
     
}

void STFiller::close( )
{
  delete reader_;reader_=0;
  delete nreader_;nreader_=0;
  delete header_;header_=0;
  table_ = 0;
}

int asap::STFiller::read( )
{
  int status = 0;

  // 
  // for NRO data
  //
  // 2008/11/12 Takeshi Nakazato
  if ( isNRO_ ) {
    status = readNRO() ;
    return status ;
  }
  //

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
  Double min = 0.0;
  Double max = nInDataRow;
  ProgressMeter fillpm(min, max, "Data importing progress");
  int n = 0;
  while ( status == 0 ) {
    status = reader_->read(scanNo, cycleNo, mjd, interval, fieldName,
                          srcName, srcDir, srcPM, srcVel, obsType, IFno,
                          refFreq, bandwidth, freqInc, restFreq, tcal, tcalTime,
                          azimuth, elevation, parAngle, focusAxi,
                          focusTan, focusRot, temperature, pressure,
                          humidity, windSpeed, windAz, refBeam,
                          beamNo, direction, scanRate,
                          tsys, sigma, calFctr, baseLin, baseSub,
                          spectra, flagtra, xCalFctr, xPol);
    if ( status != 0 ) break;
    n += 1;
    
    Regex filterrx(".*[SL|PA]$");
    Regex obsrx("^AT.+");
    if ( header_->antennaname.matches(obsrx) && obsType.matches(filterrx)) {
        //cerr << "ignoring paddle scan" << endl;
        continue;
    }
    TableRow row(table_->table());
    TableRecord& rec = row.record();
    // fields that don't get used and are just passed through asap
    RecordFieldPtr<Array<Double> > srateCol(rec, "SCANRATE");
    *srateCol = scanRate;
    RecordFieldPtr<Array<Double> > spmCol(rec, "SRCPROPERMOTION");
    *spmCol = srcPM;
    RecordFieldPtr<Array<Double> > sdirCol(rec, "SRCDIRECTION");
    *sdirCol = srcDir;
    RecordFieldPtr<Double> svelCol(rec, "SRCVELOCITY");
    *svelCol = srcVel;
    // the real stuff
    RecordFieldPtr<Int> fitCol(rec, "FIT_ID");
    *fitCol = -1;
    RecordFieldPtr<uInt> scanoCol(rec, "SCANNO");
    *scanoCol = scanNo-1;
    RecordFieldPtr<uInt> cyclenoCol(rec, "CYCLENO");
    *cyclenoCol = cycleNo-1;
    RecordFieldPtr<Double> mjdCol(rec, "TIME");
    *mjdCol = mjd;
    RecordFieldPtr<Double> intCol(rec, "INTERVAL");
    *intCol = interval;
    RecordFieldPtr<String> srcnCol(rec, "SRCNAME");
    RecordFieldPtr<Int> srctCol(rec, "SRCTYPE");
    RecordFieldPtr<String> fieldnCol(rec, "FIELDNAME");
    *fieldnCol = fieldName;
    // try to auto-identify if it is on or off.
    Regex rx(".*[e|w|_R]$");
    Regex rx2("_S$");
    Int match = srcName.matches(rx);
    if (match) {
      *srcnCol = srcName;
    } else {
      *srcnCol = srcName.before(rx2);
    }
    //*srcnCol = srcName;//.before(rx2);
    *srctCol = match;
    RecordFieldPtr<uInt> beamCol(rec, "BEAMNO");
    *beamCol = beamNo-beamOffset_-1;
    RecordFieldPtr<Int> rbCol(rec, "REFBEAMNO");
    Int rb = -1;
    if (nBeam_ > 1 ) rb = refBeam-1;
    *rbCol = rb;
    RecordFieldPtr<uInt> ifCol(rec, "IFNO");
    *ifCol = IFno-ifOffset_- 1;
    uInt id;
    /// @todo this has to change when nchan isn't global anymore
    id = table_->frequencies().addEntry(Double(header_->nchan/2),
                                            refFreq, freqInc);
    RecordFieldPtr<uInt> mfreqidCol(rec, "FREQ_ID");
    *mfreqidCol = id;

    id = table_->molecules().addEntry(restFreq);
    RecordFieldPtr<uInt> molidCol(rec, "MOLECULE_ID");
    *molidCol = id;

    id = table_->tcal().addEntry(tcalTime, tcal);
    RecordFieldPtr<uInt> mcalidCol(rec, "TCAL_ID");
    *mcalidCol = id;
    id = table_->weather().addEntry(temperature, pressure, humidity,
                                    windSpeed, windAz);
    RecordFieldPtr<uInt> mweatheridCol(rec, "WEATHER_ID");
    *mweatheridCol = id;
    RecordFieldPtr<uInt> mfocusidCol(rec, "FOCUS_ID");
    id = table_->focus().addEntry(focusAxi, focusTan, focusRot);
    *mfocusidCol = id;
    RecordFieldPtr<Array<Double> > dirCol(rec, "DIRECTION");
    *dirCol = direction;
    RecordFieldPtr<Float> azCol(rec, "AZIMUTH");
    *azCol = azimuth;
    RecordFieldPtr<Float> elCol(rec, "ELEVATION");
    *elCol = elevation;

    RecordFieldPtr<Float> parCol(rec, "PARANGLE");
    *parCol = parAngle;

    RecordFieldPtr< Array<Float> > specCol(rec, "SPECTRA");
    RecordFieldPtr< Array<uChar> > flagCol(rec, "FLAGTRA");
    RecordFieldPtr< uInt > polnoCol(rec, "POLNO");

    RecordFieldPtr< Array<Float> > tsysCol(rec, "TSYS");
    // Turn the (nchan,npol) matrix and possible complex xPol vector
    // into 2-4 rows in the scantable
    Vector<Float> tsysvec(1);
    // Why is spectra.ncolumn() == 3 for haveXPol_ == True
    uInt npol = (spectra.ncolumn()==1 ? 1: 2);
    for ( uInt i=0; i< npol; ++i ) {
      tsysvec = tsys(i);
      *tsysCol = tsysvec;
      *polnoCol = i;

      *specCol = spectra.column(i);
      *flagCol = flagtra.column(i);
      table_->table().addRow();
      row.put(table_->table().nrow()-1, rec);
    }
    if ( haveXPol_[0] ) {
      // no tsys given for xpol, so emulate it
      tsysvec = sqrt(tsys[0]*tsys[1]);
      *tsysCol = tsysvec;
      // add real part of cross pol
      *polnoCol = 2;
      Vector<Float> r(real(xPol));
      *specCol = r;
      // make up flags from linears
      /// @fixme this has to be a bitwise or of both pols
      *flagCol = flagtra.column(0);// | flagtra.column(1);
      table_->table().addRow();
      row.put(table_->table().nrow()-1, rec);
      // ad imaginary part of cross pol
      *polnoCol = 3;
      Vector<Float> im(imag(xPol));
      *specCol = im;
      table_->table().addRow();
      row.put(table_->table().nrow()-1, rec);
    }
    fillpm._update(n);
  }
  if (status > 0) {
    close();
    throw(AipsError("Reading error occured, data possibly corrupted."));
  }
  fillpm.done();
  return status;
}

/**
 * For NRO data
 *
 * 2008/11/11 Takeshi Nakazato
 **/
void STFiller::openNRO( int whichIF, int whichBeam )
{
  // open file
  // DEBUG
  time_t t0 ;
  time( &t0 ) ;
  tm *ttm = localtime( &t0 ) ;
  
  cout << "STFiller::openNRO()  Start time = " << t0 
       << " (" 
       << ttm->tm_year + 1900 << "/" << ttm->tm_mon + 1 << "/" << ttm->tm_mday 
       << " " 
       << ttm->tm_hour << ":" << ttm->tm_min << ":" << ttm->tm_sec 
       << ")" << endl ;

  // fill STHeader
  header_ = new STHeader() ;

  if ( nreader_->getHeaderInfo( header_->nchan,
                                header_->npol,
                                nIF_,
                                nBeam_,
                                header_->observer,
                                header_->project,
                                header_->obstype,
                                header_->antennaname,
                                header_->antennaposition,
                                header_->equinox,
                                header_->freqref,
                                header_->reffreq,
                                header_->bandwidth,
                                header_->utc,
                                header_->fluxunit,
                                header_->epoch,
                                header_->poltype ) ) {
    cout << "STFiller::openNRO()  Failed to get header information." << endl ;
    return ;
  }

  // DEBUG
  //cout << "STFiller::openNRO()  getHeaderInfo " << endl ;
  //

  ifOffset_ = 0;
  vector<Bool> ifs = nreader_->getIFs() ;
  if ( whichIF >= 0 ) {
    if ( whichIF >= 0 && whichIF < nIF_ ) {
      for ( int i = 0 ; i < nIF_ ; i++ ) 
        ifs[i] = False ;
      ifs[whichIF] = True ;
      header_->nif = 1;
      nIF_ = 1;
      ifOffset_ = whichIF;
    } else {
      delete reader_;
      reader_ = 0;
      delete header_;
      header_ = 0;
      throw(AipsError("Illegal IF selection"));
    }
  }

  // DEBUG
  //cout << "STFiller::openNRO()  nIF " << endl ;
  //

  beamOffset_ = 0;
  vector<Bool> beams = nreader_->getBeams() ;
  if (whichBeam>=0) {
    if (whichBeam>=0 && whichBeam<nBeam_) {
      for ( int i = 0 ; i < nBeam_ ; i++ ) 
        beams[i] = False ;
      beams[whichBeam] = True;
      header_->nbeam = 1;
      nBeam_ = 1;
      beamOffset_ = whichBeam;
    } else {
      delete reader_;
      reader_ = 0;
      delete header_;
      header_ = 0;
      throw(AipsError("Illegal Beam selection"));
    }
  }

  // DEBUG
  //cout << "STFiller::openNRO()  nBeam " << endl ;
  //

  header_->nbeam = nBeam_ ;
  header_->nif = nIF_ ;

  // set header
  table_->setHeader( *header_ ) ;

  // DEBUG
  //cout << "STFiller::openNRO() Velocity Definition = " << nheader->getVDEF() << endl ;

  // DEBUG
  time_t t1 ;
  time( &t1 ) ;
  ttm = localtime( &t1 ) ;
  cout << "STFiller::openNRO()  End time = " << t1 
       << " (" 
       << ttm->tm_year + 1900 << "/" << ttm->tm_mon + 1 << "/" << ttm->tm_mday 
       << " " 
       << ttm->tm_hour << ":" << ttm->tm_min << ":" << ttm->tm_sec 
       << ")" << endl ;
  cout << "STFiller::openNRO()  Elapsed time = " << t1 - t0 << " sec" << endl ;
  //

  return ;
}

int STFiller::readNRO()
{
  // DEBUG
  time_t t0 ;
  time( &t0 ) ;
  tm *ttm = localtime( &t0 ) ;
  cout << "STFiller::readNRO()  Start time = " << t0 
       << " (" 
       << ttm->tm_year + 1900 << "/" << ttm->tm_mon + 1 << "/" << ttm->tm_mday 
       << " " 
       << ttm->tm_hour << ":" << ttm->tm_min << ":" << ttm->tm_sec 
       << ")" << endl ;
  //

  // fill row
  uInt id ;
  uInt imax = nreader_->getRowNum() ;
  vector< vector<double > > freqs ;
  uInt i = 0 ;
  int count = 0 ;
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
  for ( i = 0 ; i < imax ; i++ ) {
    if( nreader_->getData( i ) != 0 ) {
      cerr << "STFiller::readNRO()  error while reading row " << i << endl ;
      return -1 ;
    }

    string scanType = nreader_->getScanType( i ) ;
    Int srcType = -1 ;
    if ( scanType.compare( 0, 2, "ON") == 0 ) {
      srcType = 0 ;
    }
    else if ( scanType.compare( 0, 3, "OFF" ) == 0 ) {
      //cout << "OFF srcType: " << i << endl ;
      srcType = 1 ;
    }
    else if ( scanType.compare( 0, 4, "ZERO" ) == 0 ) {
      //cout << "ZERO srcType: " << i << endl ;
      srcType = 2 ;
    }
    else {
      //cout << "Undefined srcType: " << i << endl ;
      srcType = 3 ;
    }
 
    // if srcType is 2 (ZERO scan), ignore scan
    if ( srcType != 2 && srcType != -1 && srcType != 3 ) { 
      TableRow row( table_->table() ) ;
      TableRecord& rec = row.record();

      if ( nreader_->getScanInfo( i,
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
        cerr << "STFiller::readNRO()  Failed to get scan information." << endl ;
        return 1 ;
      }

      RecordFieldPtr<uInt> scannoCol( rec, "SCANNO" ) ;
      *scannoCol = scanno ;
      RecordFieldPtr<uInt> cyclenoCol(rec, "CYCLENO") ;
      *cyclenoCol = cycleno ;
      RecordFieldPtr<uInt> beamCol(rec, "BEAMNO") ;
      *beamCol = beamno ;
      RecordFieldPtr<uInt> ifCol(rec, "IFNO") ;
      RecordFieldPtr< uInt > polnoCol(rec, "POLNO") ;
      *polnoCol = polno ;
      RecordFieldPtr<uInt> mfreqidCol(rec, "FREQ_ID") ;
      if ( freqs.size() == 0 ) {
        id = table_->frequencies().addEntry( Double( fqs[0] ),
                                             Double( fqs[1] ),
                                             Double( fqs[2] ) ) ;
        *mfreqidCol = id ;
        *ifCol = id ;
        freqs.push_back( fqs ) ;
      }
      else {
        int iadd = -1 ;
        for ( uInt iif = 0 ; iif < freqs.size() ; iif++ ) {
          //cout << "STFiller::readNRO()  freqs[" << iif << "][1] = " << freqs[iif][1] << endl ;
          double fdiff = abs( freqs[iif][1] - fqs[1] ) / freqs[iif][1] ;
          //cout << "STFiller::readNRO()  fdiff = " << fdiff << endl ;
          if ( fdiff < 1.0e-8 ) {
            iadd = iif ;
            break ;
          }
        }
        if ( iadd == -1 ) {
          id = table_->frequencies().addEntry( Double( fqs[0] ),
                                               Double( fqs[1] ),
                                               Double( fqs[2] ) ) ;
          *mfreqidCol = id ;
          *ifCol = id ;
          freqs.push_back( fqs ) ;
        }
        else {
          *mfreqidCol = iadd ;
          *ifCol = iadd ;
        }
      }
      RecordFieldPtr<uInt> molidCol(rec, "MOLECULE_ID") ;
      id = table_->molecules().addEntry( restfreq ) ;
      *molidCol = id ;
      RecordFieldPtr<Int> rbCol(rec, "REFBEAMNO") ;
      *rbCol = refbeamno ;
      RecordFieldPtr<Double> mjdCol( rec, "TIME" ) ;
      *mjdCol = scantime ;
      RecordFieldPtr<Double> intervalCol( rec, "INTERVAL" ) ;
      *intervalCol = interval ;
      RecordFieldPtr<String> srcnCol(rec, "SRCNAME") ;
      *srcnCol = srcname ;
      RecordFieldPtr<Int> srctCol(rec, "SRCTYPE") ;
      *srctCol = srcType ;     
      RecordFieldPtr<String> fieldnCol(rec, "FIELDNAME"); 
      *fieldnCol = fieldname ;
      RecordFieldPtr< Array<Float> > specCol(rec, "SPECTRA") ;
      *specCol = spectra ;
      RecordFieldPtr< Array<uChar> > flagCol(rec, "FLAGTRA") ;
      *flagCol = flagtra ;
      RecordFieldPtr< Array<Float> > tsysCol(rec, "TSYS") ;
      *tsysCol = tsys ;
      RecordFieldPtr< Array<Double> > dirCol(rec, "DIRECTION") ;
      *dirCol = direction ;
      RecordFieldPtr<Float> azCol(rec, "AZIMUTH") ;
      *azCol = azimuth ;
      RecordFieldPtr<Float> elCol(rec, "ELEVATION") ;
      *elCol = elevation ;
      RecordFieldPtr<Float> parCol(rec, "PARANGLE") ;
      *parCol = parangle ;
      RecordFieldPtr<Float> tauCol(rec, "OPACITY") ;
      *tauCol = opacity ;
      RecordFieldPtr<uInt> mcalidCol(rec, "TCAL_ID") ;
      *mcalidCol = tcalid ;
      RecordFieldPtr<Int> fitCol(rec, "FIT_ID") ;
      *fitCol = fitid ;
      RecordFieldPtr<uInt> mfocusidCol(rec, "FOCUS_ID") ;
      *mfocusidCol = focusid ;
      RecordFieldPtr<uInt> mweatheridCol(rec, "WEATHER_ID") ;
      id = table_->weather().addEntry( temperature,
                                       pressure,
                                       humidity,
                                       windvel,
                                       winddir ) ;
      *mweatheridCol = id ;          
      RecordFieldPtr<Double> svelCol(rec, "SRCVELOCITY") ;
      *svelCol = srcvel ;
      RecordFieldPtr<Array<Double> > spmCol(rec, "SRCPROPERMOTION") ;
      *spmCol = propermotion ;
      RecordFieldPtr<Array<Double> > sdirCol(rec, "SRCDIRECTION") ;
      *sdirCol = srcdir ;
      RecordFieldPtr<Array<Double> > srateCol(rec, "SCANRATE");
      *srateCol = scanrate ;

      table_->table().addRow() ;
      row.put(table_->table().nrow()-1, rec) ;
    }
    else {
      count++ ;
    }
    // DEBUG
    //int rownum = nreader_->getRowNum() ;
    //cout << "STFiller::readNRO() Finished row " << i << "/" << rownum << endl ;
    //
  }

  // DEBUG
  time_t t1 ;
  time( &t1 ) ;
  ttm = localtime( &t1 ) ;
  cout << "STFiller::readNRO()  Processed " << i << " rows" << endl ;
  cout << "STFiller::readNRO()  Added " << i - count << " rows (ignored " 
       << count << " \"ZERO\" scans)" << endl ;
  cout << "STFiller::readNRO()  End time = " << t1 
       << " (" 
       << ttm->tm_year + 1900 << "/" << ttm->tm_mon + 1 << "/" << ttm->tm_mday 
       << " " 
       << ttm->tm_hour << ":" << ttm->tm_min << ":" << ttm->tm_sec 
       << ")" << endl ;
  cout << "STFiller::readNRO()  Elapsed time = " << t1 - t0 << " sec" << endl ;
  //

  return 0 ;
} 

Bool STFiller::fileCheck()
{
  // if filename_ is directory, return false
  File inFile( filename_ ) ;
  if ( inFile.isDirectory() )
    return false ;
  
  // if beginning of header data is "RW", return true
  // otherwise, return false ; 
  FILE *fp = fopen( filename_.c_str(), "r" ) ;
  char buf[9] ;
  fread( buf, 4, 1, fp ) ;
  buf[4] = '\0' ;
  if ( ( strncmp( buf, "RW", 2 ) == 0 ) )
    return true ;

  return false ;
}

}//namespace asap

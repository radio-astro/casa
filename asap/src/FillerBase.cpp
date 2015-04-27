//
// C++ Interface: FillerBase
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <casa/Containers/RecordField.h>
#include <tables/Tables/ExprNode.h>

#include "FillerBase.h"

using namespace casa;

namespace asap {

FillerBase::FillerBase(casa::CountedPtr<Scantable> stable) :
  table_(stable)
{
    row_ = TableRow(table_->table());

    // FIT_ID is -1 by default
    RecordFieldPtr<Int> fitIdCol( row_.record(), "FIT_ID" ) ;
    *fitIdCol = -1 ;

    mEntry_.resize( 0 ) ;
    mIdx_.resize( 0 ) ;
    fEntry_.resize( 0 ) ;
    fIdx_.resize( 0 ) ;
    wEntry_.resize( 0 ) ;
    wIdx_.resize( 0 ) ;
}

void FillerBase::setHeader(const STHeader& header)
{
  table_->setHeader(header);
}

void FillerBase::setSpectrum(const Vector<Float>& spectrum,
                             const Vector<uChar>& flags,
                             const Vector<Float>& tsys)
{
  RecordFieldPtr< Array<Float> > specCol(row_.record(), "SPECTRA");
  RecordFieldPtr< Array<uChar> > flagCol(row_.record(), "FLAGTRA");
  RecordFieldPtr< Array<Float> > tsysCol(row_.record(), "TSYS");

  //*specCol = spectrum;
  //*flagCol = flags;
  //*tsysCol = tsys;
  specCol.define(spectrum);
  flagCol.define(flags);
  tsysCol.define(tsys);
}

void FillerBase::setFlagrow(uInt flag)
{
  RecordFieldPtr<uInt> flagrowCol(row_.record(), "FLAGROW");
  *flagrowCol = flag;
}

void FillerBase::setOpacity(Float opacity)
{
  RecordFieldPtr<Float> tauCol(row_.record(), "OPACITY") ;
  *tauCol = opacity ;
}

void FillerBase::setIndex(uInt scanno, uInt cycleno, uInt ifno, uInt polno,
                          uInt beamno)
{
  RecordFieldPtr<uInt> beamCol(row_.record(), "BEAMNO");
  RecordFieldPtr<uInt> ifCol(row_.record(), "IFNO");
  RecordFieldPtr<uInt> polCol(row_.record(), "POLNO");
  RecordFieldPtr<uInt> cycleCol(row_.record(), "CYCLENO");
  RecordFieldPtr<uInt> scanCol(row_.record(), "SCANNO");
  *beamCol = beamno;
  *cycleCol = cycleno;
  *ifCol = ifno;
  *polCol = polno;
  *scanCol = scanno;
}

void FillerBase::setFrequency(Double refpix, Double refval,
                              Double incr)
{
  /// @todo this has to change when nchan isn't global anymore
  uInt nEntry = fEntry_.size() ;
  Int idx = -1 ;
  Vector<Double> entry( 3 ) ;
  entry[0] = refpix ;
  entry[1] = refval ;
  entry[2] = incr ;
  for ( uInt i = 0 ; i < nEntry ; i++ ) {
    if ( allEQ( entry, fEntry_[i] ) ) {
      idx = i ;
      break ;
    }
  }
  uInt id ;
  if ( idx != -1 )
    id = fIdx_[idx] ;
  else {
    id= table_->frequencies().addEntry(refpix, refval, incr);
    RecordFieldPtr<uInt> mfreqidCol(row_.record(), "FREQ_ID");
    fEntry_.push_back( entry ) ;
    fIdx_.push_back( id ) ;
  }
  RecordFieldPtr<uInt> mfreqidCol(row_.record(), "FREQ_ID");
  *mfreqidCol = id;

}


void FillerBase::setMolecule(const Vector<Double>& restfreq)
{
  uInt nEntry = mEntry_.size() ;
  Int idx = -1 ;
  for ( uInt i = 0 ; i < nEntry ; i++ ) {
    if ( restfreq.conform( mEntry_[i] ) && 
         allEQ( restfreq, mEntry_[i] ) ) {
      idx = i ;
      break ;
    }
  }
  uInt id ;
  if ( idx != -1 )
    id = mIdx_[idx] ;
  else {
    Vector<String> tmp ;
    id = table_->molecules().addEntry(restfreq,tmp,tmp) ;
    mEntry_.push_back( restfreq.copy() ) ;
    mIdx_.push_back( id ) ;
  }
  RecordFieldPtr<uInt> molidCol(row_.record(), "MOLECULE_ID");
  *molidCol = id;
}

void FillerBase::setDirection(const Vector<Double>& dir,
                              Float az, Float el)
{
  RecordFieldPtr<Array<Double> > dirCol(row_.record(), "DIRECTION");
  *dirCol = dir;
  RecordFieldPtr<Float> azCol(row_.record(), "AZIMUTH");
  *azCol = az;
  RecordFieldPtr<Float> elCol(row_.record(), "ELEVATION");
  *elCol = el;
}

void FillerBase::setFocus(Float pa, Float faxis,
                      Float ftan, Float frot)
{
  RecordFieldPtr<uInt> mfocusidCol(row_.record(), "FOCUS_ID");
  uInt id = table_->focus().addEntry(pa, faxis, ftan, frot);
  *mfocusidCol = id;
}

void FillerBase::setTime(Double mjd, Double interval)
{
    RecordFieldPtr<Double> mjdCol(row_.record(), "TIME");
    *mjdCol = mjd;
    RecordFieldPtr<Double> intCol(row_.record(), "INTERVAL");
    *intCol = interval;

}

void FillerBase::setWeather(Float temperature, Float pressure,
                        Float humidity,
                        Float windspeed, Float windaz)
{
  uInt nEntry = wEntry_.size() ;
  Int idx = -1 ;
  Vector<Float> entry( 5 ) ;
  entry[0] = temperature ;
  entry[1] = pressure ;
  entry[2] = humidity ;
  entry[3] = windspeed ;
  entry[4] = windaz ;
  for ( uInt i = 0 ; i < nEntry ; i++ ) {
    if ( allEQ( entry, wEntry_[i] ) ) {
      idx = i ;
      break ;
    }
  }
  uInt id ;
  if ( idx != -1 )
    id = wIdx_[idx] ;
  else {
    id = table_->weather().addEntry(temperature, pressure,
                                    humidity, windspeed, windaz);
    wEntry_.push_back( entry ) ;
    wIdx_.push_back( id ) ;
  }
  RecordFieldPtr<uInt> mweatheridCol(row_.record(), "WEATHER_ID");
  *mweatheridCol = id;
}

void FillerBase::setTcal(const String& tcaltime,
                     const Vector<Float>& tcal)
{
    uInt id = table_->tcal().addEntry(tcaltime, tcal);
    RecordFieldPtr<uInt> mcalidCol(row_.record(), "TCAL_ID");
    *mcalidCol = id;
}

void FillerBase::setScanRate(const Vector<Double>& srate)
{
    RecordFieldPtr<Array<Double> > srateCol(row_.record(), "SCANRATE");
    *srateCol = srate;
}

void FillerBase::setReferenceBeam(Int beamno)
{
  RecordFieldPtr<Int> rbCol(row_.record(), "REFBEAMNO");
  *rbCol = beamno;
}

void FillerBase::setSource(const std::string& name, Int type,
                           const std::string& fieldname,
                           const Vector<Double>& dir,
                           const Vector<Double>& propermot,
                           Double velocity)
{
    RecordFieldPtr<String> srcnCol(row_.record(), "SRCNAME");
    *srcnCol = name;
    RecordFieldPtr<Int> srctCol(row_.record(), "SRCTYPE");
    *srctCol = type;
    RecordFieldPtr<String> fieldnCol(row_.record(), "FIELDNAME");
    *fieldnCol = fieldname;
    RecordFieldPtr<Array<Double> > spmCol(row_.record(), "SRCPROPERMOTION");
    *spmCol = propermot;
    RecordFieldPtr<Array<Double> > sdirCol(row_.record(), "SRCDIRECTION");
    *sdirCol = dir;
    RecordFieldPtr<Double> svelCol(row_.record(), "SRCVELOCITY");
    *svelCol = velocity;
}

void FillerBase::commitRow()
{
  table_->table().addRow();
  row_.put(table_->table().nrow()-1);
}

void FillerBase::setWeather2(Float temperature, 
                             Float pressure,
                             Float humidity,
                             Float windspeed, 
                             Float windaz)
{
  uInt nEntry = wEntry_.size() ;
  Int idx = -1 ;
  Vector<Float> entry( 5 ) ;
  entry[0] = temperature ;
  entry[1] = pressure ;
  entry[2] = humidity ;
  entry[3] = windspeed ;
  entry[4] = windaz ;
  for ( uInt i = 0 ; i < nEntry ; i++ ) {
    if ( allEQ( entry, wEntry_[i] ) ) {
      idx = i ;
      break ;
    }
  }
  uInt id ;
  if ( idx != -1 )
    id = wIdx_[idx] ;
  else {
    Table tab = table_->weather().table() ;
    Table subt = tab( tab.col("TEMPERATURE") == temperature     \
                      && tab.col("PRESSURE") == pressure        \
                      && tab.col("HUMIDITY") == humidity        \
                      && tab.col("WINDSPEED") == windspeed      \
                      && tab.col("WINDAZ") == windaz, 1 ) ;
    Int nrow = tab.nrow() ;
    Int nrowSel = subt.nrow() ;
    if ( nrowSel == 0 ) {
      tab.addRow( 1, True ) ;
      TableRow row( tab ) ;
      TableRecord &rec = row.record() ;
      RecordFieldPtr<casa::uInt> rfpi ;
      rfpi.attachToRecord( rec, "ID" ) ;
      *rfpi = (uInt)nrow ;
      RecordFieldPtr<casa::Float> rfp ;
      rfp.attachToRecord( rec, "TEMPERATURE" ) ;
      *rfp = temperature ;
      rfp.attachToRecord( rec, "PRESSURE" ) ;
      *rfp = pressure ;
      rfp.attachToRecord( rec, "HUMIDITY" ) ;
      *rfp = humidity ;
      rfp.attachToRecord( rec, "WINDSPEED" ) ;
      *rfp = windspeed ;
      rfp.attachToRecord( rec, "WINDAZ" ) ;
      *rfp = windaz ;
      row.put( nrow, rec ) ;
      id = (uInt)nrow ;
    }
    else {
      ROTableColumn tc( subt, "ID" ) ;
      id = tc.asuInt( 0 ) ;
    }
    wEntry_.push_back( entry ) ;
    wIdx_.push_back( id ) ;
  }
  RecordFieldPtr<uInt> mweatheridCol(row_.record(), "WEATHER_ID");
  *mweatheridCol = id;
}

void FillerBase::setTcal2(const String& tcaltime,
                          const Vector<Float>& tcal)
{
  uInt id = 0 ;
  Table tab = table_->tcal().table() ;
  Table result =
    //tab( nelements(tab.col("TCAL")) == uInt (tcal.size()) && 
    //     all(tab.col("TCAL")== tcal) &&
    //     tab.col("TIME") == tcaltime, 1 ) ;
    tab( nelements(tab.col("TCAL")) == uInt (tcal.size()) && 
         all(tab.col("TCAL")== tcal), 1 ) ;

  if ( result.nrow() > 0 ) {
    ROTableColumn tmpCol( result, "ID" ) ;
    tmpCol.getScalar( 0, id ) ;
  }
  else {
    uInt rno = tab.nrow();
    tab.addRow();
    TableColumn idCol( tab, "ID" ) ;
    TableColumn tctimeCol( tab, "TIME" ) ;
    ArrayColumn<Float> tcalCol( tab, "TCAL" ) ;
    // get last assigned _id and increment
    if ( rno > 0 ) {
      idCol.getScalar(rno-1, id);
      id++;
    }
    tctimeCol.putScalar(rno, tcaltime);
    tcalCol.put(rno, tcal);
    idCol.putScalar(rno, id);
  }

  RecordFieldPtr<uInt> mcalidCol(row_.record(), "TCAL_ID");
  *mcalidCol = id;
}

};

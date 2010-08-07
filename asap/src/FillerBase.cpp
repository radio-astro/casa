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

#include "FillerBase.h"

using namespace casa;

namespace asap {

FillerBase::FillerBase(casa::CountedPtr<Scantable> stable) :
  table_(stable)
{
    row_ = TableRow(table_->table());
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

  *specCol = spectrum;
  *flagCol = flags;
  *tsysCol = tsys;
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
  uInt id= table_->frequencies().addEntry(refpix, refval, incr);
  RecordFieldPtr<uInt> mfreqidCol(row_.record(), "FREQ_ID");
  *mfreqidCol = id;

}


void FillerBase::setMolecule(const Vector<Double>& restfreq)
{
  Vector<String> tmp;
  uInt id = table_->molecules().addEntry(restfreq, tmp, tmp);
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
    uInt id = table_->weather().addEntry(temperature, pressure,
                                         humidity, windspeed, windaz);
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

};

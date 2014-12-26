//#---------------------------------------------------------------------------
//# SDMemTable.cc: A MemoryTable container for single dish integrations
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------

#include <map>

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayAccessor.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta/MVAngle.h>

#include <tables/TaQL/TableParse.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>

#include <tables/TaQL/ExprNode.h>
#include <tables/Tables/TableRecord.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MeasTable.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>

#include <singledish/SDDefs.h>
#include <singledish/SDContainer.h>
#include <singledish/MathUtils.h>
#include <singledish/SDPol.h>
#include <singledish/SDAttr.h>

#include <singledish/SDMemTable.h>

using namespace casa;
using namespace asap;

SDMemTable::SDMemTable() :
  IFSel_(0),
  beamSel_(0),
  polSel_(0)
{
  setup();
  attach();
}

SDMemTable::SDMemTable(const std::string& name) :
  IFSel_(0),
  beamSel_(0),
  polSel_(0)
{
  Table tab(name);
  Int version;
  tab.keywordSet().get("VERSION", version);
  if (version != version_) {
    throw(AipsError("Unsupported version of ASAP file."));
  }
  table_ = tab.copyToMemoryTable("dummy");
  attach();
}

SDMemTable::SDMemTable(const SDMemTable& other, Bool clear)
{
  table_ = other.table_.copyToMemoryTable(String("dummy"));
  // clear all rows()
  if (clear) {
    table_.removeRow(this->table_.rowNumbers());
    IFSel_= 0;
    beamSel_= 0;
    polSel_= 0;
  } else {
    IFSel_= other.IFSel_;
    beamSel_= other.beamSel_;
    polSel_= other.polSel_;
  }

  attach();
}

SDMemTable::SDMemTable(const Table& tab, const std::string& exprs) :
  IFSel_(0),
  beamSel_(0),
  polSel_(0)
{
  Table t = tableCommand(exprs,tab);
  if (t.nrow() == 0)
      throw(AipsError("Query unsuccessful."));
  table_ = t.copyToMemoryTable("dummy");
  attach();
  renumber();
}

SDMemTable::~SDMemTable()
{
  //cerr << "goodbye from SDMemTable @ " << this << endl;
}

SDMemTable SDMemTable::getScan(Int scanID) const
{
  String cond("SELECT * from $1 WHERE SCANID == ");
  cond += String::toString(scanID);
  return SDMemTable(table_, cond);
}

SDMemTable &SDMemTable::operator=(const SDMemTable& other)
{
  if (this != &other) {
     IFSel_= other.IFSel_;
     beamSel_= other.beamSel_;
     polSel_= other.polSel_;
     table_ = other.table_.copyToMemoryTable(String("dummy"));
     attach();
  }
  return *this;
}

SDMemTable SDMemTable::getSource(const std::string& source) const
{
  String cond("SELECT * from $1 WHERE SRCNAME == ");
  cond += source;
  return SDMemTable(table_, cond);
}

void SDMemTable::setup()
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.comment() = "A SDMemTable";
  td.rwKeywordSet().define("VERSION", Int(version_));

  td.addColumn(ScalarColumnDesc<Double>("TIME"));
  td.addColumn(ScalarColumnDesc<String>("SRCNAME"));
  td.addColumn(ArrayColumnDesc<Float>("SPECTRA"));
  td.addColumn(ArrayColumnDesc<uChar>("FLAGTRA"));
  td.addColumn(ArrayColumnDesc<Float>("TSYS"));
  td.addColumn(ArrayColumnDesc<Float>("STOKES"));
  td.addColumn(ScalarColumnDesc<Int>("SCANID"));
  td.addColumn(ScalarColumnDesc<Double>("INTERVAL"));
  td.addColumn(ArrayColumnDesc<uInt>("FREQID"));
  td.addColumn(ArrayColumnDesc<uInt>("RESTFREQID"));
  td.addColumn(ArrayColumnDesc<Double>("DIRECTION"));
  td.addColumn(ScalarColumnDesc<String>("FIELDNAME"));
  td.addColumn(ScalarColumnDesc<String>("TCALTIME"));
  td.addColumn(ArrayColumnDesc<Float>("TCAL"));
  td.addColumn(ScalarColumnDesc<Float>("AZIMUTH"));
  td.addColumn(ScalarColumnDesc<Float>("ELEVATION"));
  td.addColumn(ScalarColumnDesc<Float>("PARANGLE"));
  td.addColumn(ScalarColumnDesc<Int>("REFBEAM"));
  td.addColumn(ArrayColumnDesc<Int>("FITID"));

  // Now create Table SetUp from the description.
  SetupNewTable aNewTab("dummy", td, Table::New);

  // Bind the Stokes Virtual machine to the STOKES column Because we
  // don't know how many polarizations will be in the data at this
  // point, we must bind the Virtual Engine regardless.  The STOKES
  // column won't be accessed if not appropriate (nPol=4)
  SDStokesEngine::registerClass();
  SDStokesEngine stokesEngine(String("STOKES"), String("SPECTRA"));
  aNewTab.bindColumn("STOKES", stokesEngine);

  // Create Table
  table_ = Table(aNewTab, Table::Memory, 0);
  // add subtable
  TableDesc tdf("", "1", TableDesc::Scratch);
  tdf.addColumn(ArrayColumnDesc<String>("FUNCTIONS"));
  tdf.addColumn(ArrayColumnDesc<Int>("COMPONENTS"));
  tdf.addColumn(ArrayColumnDesc<Double>("PARAMETERS"));
  tdf.addColumn(ArrayColumnDesc<Bool>("PARMASK"));
  tdf.addColumn(ArrayColumnDesc<String>("FRAMEINFO"));
  SetupNewTable fittab("fits", tdf, Table::New);
  Table fitTable(fittab, Table::Memory);
  table_.rwKeywordSet().defineTable("FITS", fitTable);

  TableDesc tdh("", "1", TableDesc::Scratch);
  tdh.addColumn(ScalarColumnDesc<String>("ITEM"));
  SetupNewTable histtab("hist", tdh, Table::New);
  Table histTable(histtab, Table::Memory);
  table_.rwKeywordSet().defineTable("HISTORY", histTable);
}

void SDMemTable::attach()
{
  timeCol_.attach(table_, "TIME");
  srcnCol_.attach(table_, "SRCNAME");
  specCol_.attach(table_, "SPECTRA");
  flagsCol_.attach(table_, "FLAGTRA");
  tsCol_.attach(table_, "TSYS");
  stokesCol_.attach(table_, "STOKES");
  scanCol_.attach(table_, "SCANID");
  integrCol_.attach(table_, "INTERVAL");
  freqidCol_.attach(table_, "FREQID");
  restfreqidCol_.attach(table_, "RESTFREQID");
  dirCol_.attach(table_, "DIRECTION");
  fldnCol_.attach(table_, "FIELDNAME");
  tcaltCol_.attach(table_, "TCALTIME");
  tcalCol_.attach(table_, "TCAL");
  azCol_.attach(table_, "AZIMUTH");
  elCol_.attach(table_, "ELEVATION");
  paraCol_.attach(table_, "PARANGLE");
  rbeamCol_.attach(table_, "REFBEAM");
  fitCol_.attach(table_,"FITID");
}


std::string SDMemTable::getSourceName(Int whichRow) const
{
  String name;
  srcnCol_.get(whichRow, name);
  return name;
}

float SDMemTable::getElevation(Int whichRow) const
{
  float elevation;
  elCol_.get(whichRow, elevation);
  return elevation;
}

float SDMemTable::getAzimuth(Int whichRow) const
{
  float azimuth;
  azCol_.get(whichRow, azimuth);
  return azimuth;
}

float SDMemTable::getParAngle(Int whichRow) const
{
  float parangle;
  paraCol_.get(whichRow, parangle);
  return parangle;
}

std::string SDMemTable::getTime(Int whichRow, Bool showDate) const
{
  Double tm;
  if (whichRow > -1) {
    timeCol_.get(whichRow, tm);
  } else {
    table_.keywordSet().get("UTC",tm);
  }
  MVTime mvt(tm);
  if (showDate)
    mvt.setFormat(MVTime::YMD);
  else
    mvt.setFormat(MVTime::TIME);
  ostringstream oss;
  oss << mvt;
  return String(oss);
}

double SDMemTable::getInterval(Int whichRow) const
{
  Double intval;
  integrCol_.get(whichRow, intval);
  return intval;
}

bool SDMemTable::setIF(Int whichIF)
{
  if ( whichIF >= 0 && whichIF < nIF()) {
    IFSel_ = whichIF;
    return true;
  }
  return false;
}

bool SDMemTable::setBeam(Int whichBeam)
{
  if ( whichBeam >= 0 && whichBeam < nBeam()) {
    beamSel_ = whichBeam;
    return true;
  }
  return false;
}

bool SDMemTable::setPol(Int whichPol)
{
  if ( whichPol >= 0 && whichPol < nPol()) {
    polSel_ = whichPol;
    return true;
  }
  return false;
}

void SDMemTable::resetCursor()
{
   polSel_ = 0;
   IFSel_ = 0;
   beamSel_ = 0;
}

std::vector<bool> SDMemTable::getMask(Int whichRow) const
{

  std::vector<bool> mask;

  Array<uChar> arr;
  flagsCol_.get(whichRow, arr);

  ArrayAccessor<uChar, Axis<asap::BeamAxis> > aa0(arr);
  aa0.reset(aa0.begin(uInt(beamSel_)));//go to beam
  ArrayAccessor<uChar, Axis<asap::IFAxis> > aa1(aa0);
  aa1.reset(aa1.begin(uInt(IFSel_)));// go to IF
  ArrayAccessor<uChar, Axis<asap::PolAxis> > aa2(aa1);
  aa2.reset(aa2.begin(uInt(polSel_)));// go to pol

  for (ArrayAccessor<uChar, Axis<asap::ChanAxis> > i(aa2); i != i.end(); ++i) {
    bool out =!static_cast<bool>(*i);
    mask.push_back(out);
  }
  return mask;
}



std::vector<float> SDMemTable::getSpectrum(Int whichRow) const
{
  Array<Float> arr;
  specCol_.get(whichRow, arr);
  return getFloatSpectrum(arr);
}


int SDMemTable::nStokes() const
{
   return stokesCol_.shape(0).nelements();        // All rows same shape
}


std::vector<float> SDMemTable::getStokesSpectrum(Int whichRow,
                                                 Bool doPol) const
  //
  // Gets one STokes parameter depending on cursor polSel location
  //  doPol=False  : I,Q,U,V
  //  doPol=True   : I,P,PA,V   ; P = sqrt(Q**2+U**2), PA = 0.5*atan2(Q,U)
  //
{
  AlwaysAssert(asap::nAxes==4,AipsError);
  if (nPol()!=1 && nPol()!=2 && nPol()!=4) {
    throw (AipsError("You must have 1,2 or 4 polarizations to get the Stokes parameters"));
  }

  // For full conversion we are only supporting linears at the moment

  if (nPol() > 2) {
    String antName;
    table_.keywordSet().get("AntennaName", antName);
    Instrument inst = SDAttr::convertInstrument (antName, True);
    SDAttr sdAtt;
    if (sdAtt.feedPolType(inst) != LINEAR) {
      throw(AipsError("Only linear polarizations are supported"));
    }
  }

  Array<Float> arr;
  stokesCol_.get(whichRow, arr);

  if (doPol && (polSel_==1 || polSel_==2)) {       //   Q,U --> P, P.A.

    // Set current cursor location

    const IPosition& shape = arr.shape();
    IPosition start, end;
    getCursorSlice(start, end, shape);

    // Get Q and U slices

    Array<Float> Q = SDPolUtil::getStokesSlice(arr,start,end,"Q");
    Array<Float> U = SDPolUtil::getStokesSlice(arr,start,end,"U");

    // Compute output

    Array<Float> out;
    if (polSel_==1) {                                        // P
      out = SDPolUtil::polarizedIntensity(Q,U);
    } else if (polSel_==2) {                                 // P.A.
      out = SDPolUtil::positionAngle(Q,U);
    }

    // Copy to output

    IPosition vecShape(1,shape(asap::ChanAxis));
    Vector<Float> outV = out.reform(vecShape);
    std::vector<float> stlout;
    outV.tovector(stlout);
    return stlout;

  } else {
    // Selects at the cursor location
    return getFloatSpectrum(arr);
  }
}

std::string SDMemTable::getPolarizationLabel(Bool linear, Bool stokes,
                                              Bool linPol, Int polIdx) const
{
   uInt idx = polSel_;
   if (polIdx >=0) idx = polIdx;
   return SDPolUtil::polarizationLabel(idx, linear, stokes, linPol);
}



std::vector<float> SDMemTable::stokesToPolSpectrum(Int whichRow,
                                                   Bool toLinear,
                                                   Int polIdx) const
//
// polIdx
//   0:3 -> RR,LL,Real(RL),Imag(RL)
//          XX,YY,Real(XY),Image(XY)
//
// Gets only
//  RR = I + V
//  LL = I - V
// at the moment
//
{
  AlwaysAssert(asap::nAxes==4,AipsError);
  if (nStokes()!=4) {
     throw (AipsError("You must have 4 Stokes to convert to linear or circular"));
  }
//
  Array<Float> arr, out;
  stokesCol_.get(whichRow, arr);

// Set current cursor location

  const IPosition& shape = arr.shape();
  IPosition start, end;
  getCursorSlice(start, end, shape);

// Get the slice

  if (toLinear) {
     throw(AipsError("Conversion to linears not yet supported"));
  } else {
    uInt selection = polSel_;
    if (polIdx > -1) selection = polIdx;
    Bool doRR = (selection==0);
    if (selection>1) {
      throw(AipsError("Only conversion to RR & LL is currently supported"));
    }

    // Get I and V slices
    Array<Float> I = SDPolUtil::getStokesSlice(arr,start,end,"I");
    Array<Float> V = SDPolUtil::getStokesSlice(arr,start,end,"V");

    // Compute output
    out = SDPolUtil::circularPolarizationFromStokes(I, V, doRR);
  }

  // Copy to output
   IPosition vecShape(1,shape(asap::ChanAxis));
   Vector<Float> outV = out.reform(vecShape);
   std::vector<float> stlout;
   outV.tovector(stlout);
//
   return stlout;
}




Array<Float> SDMemTable::getStokesSpectrum(Int whichRow, Int iBeam, Int iIF) const
{

// Get data

  Array<Float> arr;
  stokesCol_.get(whichRow, arr);

// Set current cursor location and overwrite polarization axis

  const IPosition& shape = arr.shape();
  IPosition start(shape.nelements(),0);
  IPosition end(shape-1);
  if (iBeam!=-1) {
     start(asap::BeamAxis) = iBeam;
     end(asap::BeamAxis) = iBeam;
  }
  if (iIF!=-1) {
     start(asap::IFAxis) = iIF;
     end(asap::IFAxis) = iIF;
  }

// Get slice

  return arr(start,end);
}


std::vector<string> SDMemTable::getCoordInfo() const
{
  String un;
  Table t = table_.keywordSet().asTable("FREQUENCIES");
  String sunit;
  t.keywordSet().get("UNIT",sunit);
  String dpl;
  t.keywordSet().get("DOPPLER",dpl);
  if (dpl == "") dpl = "RADIO";
  String rfrm;
  t.keywordSet().get("REFFRAME",rfrm);
  std::vector<string> inf;
  inf.push_back(sunit);
  inf.push_back(rfrm);
  inf.push_back(dpl);
  t.keywordSet().get("BASEREFFRAME",rfrm);
  inf.push_back(rfrm);
  return inf;
}

void SDMemTable::setCoordInfo(std::vector<string> theinfo)
{
  std::vector<string>::iterator it;
  String un,rfrm, brfrm,dpl;
  un = theinfo[0];              // Abcissa unit
  rfrm = theinfo[1];            // Active (or conversion) frame
  dpl = theinfo[2];             // Doppler
  brfrm = theinfo[3];           // Base frame
  Table t = table_.rwKeywordSet().asTable("FREQUENCIES");

  Vector<Double> rstf;
  t.keywordSet().get("RESTFREQS",rstf);

  Bool canDo = True;
  Unit u1("km/s");Unit u2("Hz");
  if (Unit(un) == u1) {
    Vector<Double> rstf;
    t.keywordSet().get("RESTFREQS",rstf);
    if (rstf.nelements() == 0) {
        throw(AipsError("Can't set unit to km/s if no restfrequencies are specified"));
    }
  } else if (Unit(un) != u2 && un != "") {
        throw(AipsError("Unit not conformant with Spectral Coordinates"));
  }
  t.rwKeywordSet().define("UNIT", un);

  MFrequency::Types mdr;
  if (!MFrequency::getType(mdr, rfrm)) {

    Int a,b;const uInt* c;
    const String* valid = MFrequency::allMyTypes(a, b, c);
    String pfix = "Please specify a legal frame type. Types are\n";
    throw(AipsError(pfix+(*valid)));
  } else {
    t.rwKeywordSet().define("REFFRAME",rfrm);
  }

  MDoppler::Types dtype;
  dpl.upcase();
  if (!MDoppler::getType(dtype, dpl)) {
    throw(AipsError("Doppler type unknown"));
  } else {
    t.rwKeywordSet().define("DOPPLER",dpl);
  }

  if (!MFrequency::getType(mdr, brfrm)) {
     Int a,b;const uInt* c;
     const String* valid = MFrequency::allMyTypes(a, b, c);
     String pfix = "Please specify a legal frame type. Types are\n";
     throw(AipsError(pfix+(*valid)));
   } else {
    t.rwKeywordSet().define("BASEREFFRAME",brfrm);
   }
}


std::vector<double> SDMemTable::getAbcissa(Int whichRow) const
{
  std::vector<double> abc(nChan());

  // Get header units keyword
  Table t = table_.keywordSet().asTable("FREQUENCIES");
  String sunit;
  t.keywordSet().get("UNIT",sunit);
  if (sunit == "") sunit = "pixel";
  Unit u(sunit);

  // Easy if just wanting pixels
  if (sunit==String("pixel")) {
    // assume channels/pixels
    std::vector<double>::iterator it;
    uInt i=0;
    for (it = abc.begin(); it != abc.end(); ++it) {
      (*it) = Double(i++);
    }
    return abc;
  }

  // Continue with km/s or Hz.  Get FreqIDs
  Vector<uInt> freqIDs;
  freqidCol_.get(whichRow, freqIDs);
  uInt freqID = freqIDs(IFSel_);
  restfreqidCol_.get(whichRow, freqIDs);
  uInt restFreqID = freqIDs(IFSel_);

  // Get SpectralCoordinate, set reference frame conversion,
  // velocity conversion, and rest freq state

  SpectralCoordinate spc = getSpectralCoordinate(freqID, restFreqID, whichRow);
  Vector<Double> pixel(nChan());
  indgen(pixel);

  if (u == Unit("km/s")) {
     Vector<Double> world;
     spc.pixelToVelocity(world,pixel);
     std::vector<double>::iterator it;
     uInt i = 0;
     for (it = abc.begin(); it != abc.end(); ++it) {
       (*it) = world[i];
       i++;
     }
  } else if (u == Unit("Hz")) {

    // Set world axis units
    Vector<String> wau(1); wau = u.getName();
    spc.setWorldAxisUnits(wau);

    std::vector<double>::iterator it;
    Double tmp;
    uInt i = 0;
    for (it = abc.begin(); it != abc.end(); ++it) {
      spc.toWorld(tmp,pixel[i]);
      (*it) = tmp;
      i++;
    }
  }
  return abc;
}

std::string SDMemTable::getAbcissaString(Int whichRow) const
{
  Table t = table_.keywordSet().asTable("FREQUENCIES");

  String sunit;
  t.keywordSet().get("UNIT",sunit);
  if (sunit == "") sunit = "pixel";
  Unit u(sunit);

  Vector<uInt> freqIDs;
  freqidCol_.get(whichRow, freqIDs);
  uInt freqID = freqIDs(IFSel_);
  restfreqidCol_.get(whichRow, freqIDs);
  uInt restFreqID = freqIDs(IFSel_);

  // Get SpectralCoordinate, with frame, velocity, rest freq state set
  SpectralCoordinate spc = getSpectralCoordinate(freqID, restFreqID, whichRow);

  String s = "Channel";
  if (u == Unit("km/s")) {
    s = CoordinateUtil::axisLabel(spc,0,True,True,True);
  } else if (u == Unit("Hz")) {
    Vector<String> wau(1);wau = u.getName();
    spc.setWorldAxisUnits(wau);

    s = CoordinateUtil::axisLabel(spc,0,True,True,False);
  }
  return s;
}

void SDMemTable::setSpectrum(std::vector<float> spectrum, int whichRow)
{
  Array<Float> arr;
  specCol_.get(whichRow, arr);
  if (spectrum.size() != arr.shape()(asap::ChanAxis)) {
    throw(AipsError("Attempting to set spectrum with incorrect length."));
  }

  // Setup accessors
  ArrayAccessor<Float, Axis<asap::BeamAxis> > aa0(arr);
  aa0.reset(aa0.begin(uInt(beamSel_)));                   // Beam selection
  ArrayAccessor<Float, Axis<asap::IFAxis> > aa1(aa0);
  aa1.reset(aa1.begin(uInt(IFSel_)));                     // IF selection
  ArrayAccessor<Float, Axis<asap::PolAxis> > aa2(aa1);
  aa2.reset(aa2.begin(uInt(polSel_)));                    // Pol selection

  // Iterate
  std::vector<float>::iterator it = spectrum.begin();
  for (ArrayAccessor<Float, Axis<asap::ChanAxis> > i(aa2); i != i.end(); ++i) {
    (*i) = Float(*it);
    it++;
  }
  specCol_.put(whichRow, arr);
}

void SDMemTable::getSpectrum(Vector<Float>& spectrum, Int whichRow) const
{
  Array<Float> arr;
  specCol_.get(whichRow, arr);

  // Iterate and extract
  spectrum.resize(arr.shape()(3));
  ArrayAccessor<Float, Axis<asap::BeamAxis> > aa0(arr);
  aa0.reset(aa0.begin(uInt(beamSel_)));//go to beam
  ArrayAccessor<Float, Axis<asap::IFAxis> > aa1(aa0);
  aa1.reset(aa1.begin(uInt(IFSel_)));// go to IF
  ArrayAccessor<Float, Axis<asap::PolAxis> > aa2(aa1);
  aa2.reset(aa2.begin(uInt(polSel_)));// go to pol

  ArrayAccessor<Float, Axis<asap::BeamAxis> > va(spectrum);
  for (ArrayAccessor<Float, Axis<asap::ChanAxis> > i(aa2); i != i.end(); ++i) {
    (*va) = (*i);
    va++;
  }
}


/*
void SDMemTable::getMask(Vector<Bool>& mask, Int whichRow) const {
  Array<uChar> arr;
  flagsCol_.get(whichRow, arr);
  mask.resize(arr.shape()(3));

  ArrayAccessor<uChar, Axis<asap::BeamAxis> > aa0(arr);
  aa0.reset(aa0.begin(uInt(beamSel_)));//go to beam
  ArrayAccessor<uChar, Axis<asap::IFAxis> > aa1(aa0);
  aa1.reset(aa1.begin(uInt(IFSel_)));// go to IF
  ArrayAccessor<uChar, Axis<asap::PolAxis> > aa2(aa1);
  aa2.reset(aa2.begin(uInt(polSel_)));// go to pol

  Bool useUserMask = ( chanMask_.size() == arr.shape()(3) );

  ArrayAccessor<Bool, Axis<asap::BeamAxis> > va(mask);
  std::vector<bool> tmp;
  tmp = chanMask_; // WHY the fxxx do I have to make a copy here. The
                   // iterator should work on chanMask_??
  std::vector<bool>::iterator miter;
  miter = tmp.begin();

  for (ArrayAccessor<uChar, Axis<asap::ChanAxis> > i(aa2); i != i.end(); ++i) {
    bool out =!static_cast<bool>(*i);
    if (useUserMask) {
      out = out && (*miter);
      miter++;
    }
    (*va) = out;
    va++;
  }
}
*/

MaskedArray<Float> SDMemTable::rowAsMaskedArray(uInt whichRow,
                                                Bool toStokes) const
{
  // Get flags
  Array<uChar> farr;
  flagsCol_.get(whichRow, farr);

  // Get data and convert mask to Bool
  Array<Float> arr;
  Array<Bool> mask;
  if (toStokes) {
     stokesCol_.get(whichRow, arr);

     Array<Bool> tMask(farr.shape());
     convertArray(tMask, farr);
     mask = SDPolUtil::stokesData (tMask, True);
  } else {
     specCol_.get(whichRow, arr);
     mask.resize(farr.shape());
     convertArray(mask, farr);
  }

  return MaskedArray<Float>(arr,!mask);
}

Float SDMemTable::getTsys(Int whichRow) const
{
  Array<Float> arr;
  tsCol_.get(whichRow, arr);
  Float out;

  IPosition ip(arr.shape());
  ip(asap::BeamAxis) = beamSel_;
  ip(asap::IFAxis) = IFSel_;
  ip(asap::PolAxis) = polSel_;
  ip(asap::ChanAxis)=0;               // First channel only

  out = arr(ip);
  return out;
}

MDirection SDMemTable::getDirection(Int whichRow, Bool refBeam) const
{
  MDirection::Types mdr = getDirectionReference();
  Array<Double> posit;
  dirCol_.get(whichRow,posit);
  Vector<Double> wpos(2);
  Int rb;
  rbeamCol_.get(whichRow,rb);
  wpos[0] = posit(IPosition(2,beamSel_,0));
  wpos[1] = posit(IPosition(2,beamSel_,1));
  if (refBeam && rb > -1) {  // use refBeam instead if it exists
    wpos[0] = posit(IPosition(2,rb,0));
    wpos[1] = posit(IPosition(2,rb,1));
  }

  Quantum<Double> lon(wpos[0],Unit(String("rad")));
  Quantum<Double> lat(wpos[1],Unit(String("rad")));
  return MDirection(lon, lat, mdr);
}

MEpoch SDMemTable::getEpoch(Int whichRow) const
{
  MEpoch::Types met = getTimeReference();

  Double obstime;
  timeCol_.get(whichRow,obstime);
  MVEpoch tm2(Quantum<Double>(obstime, Unit(String("d"))));
  return MEpoch(tm2, met);
}

MPosition SDMemTable::getAntennaPosition () const
{
  Vector<Double> antpos;
  table_.keywordSet().get("AntennaPosition", antpos);
  MVPosition mvpos(antpos(0),antpos(1),antpos(2));
  return MPosition(mvpos);
}


SpectralCoordinate SDMemTable::getSpectralCoordinate(uInt freqID) const
{

  Table t = table_.keywordSet().asTable("FREQUENCIES");
  if (freqID> t.nrow() ) {
    throw(AipsError("SDMemTable::getSpectralCoordinate - freqID out of range"));
  }

  Double rp,rv,inc;
  String rf;
  ROScalarColumn<Double> rpc(t, "REFPIX");
  ROScalarColumn<Double> rvc(t, "REFVAL");
  ROScalarColumn<Double> incc(t, "INCREMENT");
  t.keywordSet().get("BASEREFFRAME",rf);

  // Create SpectralCoordinate (units Hz)
  MFrequency::Types mft;
  if (!MFrequency::getType(mft, rf)) {
    ostringstream oss;
    pushLog("WARNING: Frequency type unknown assuming TOPO");
    mft = MFrequency::TOPO;
  }
  rpc.get(freqID, rp);
  rvc.get(freqID, rv);
  incc.get(freqID, inc);

  SpectralCoordinate spec(mft,rv,inc,rp);
  return spec;
}


SpectralCoordinate SDMemTable::getSpectralCoordinate(uInt freqID,
                                                     uInt restFreqID,
                                                     uInt whichRow) const
{

  // Create basic SC
  SpectralCoordinate spec = getSpectralCoordinate (freqID);

  Table t = table_.keywordSet().asTable("FREQUENCIES");

  // Set rest frequency
  Vector<Double> restFreqIDs;
  t.keywordSet().get("RESTFREQS",restFreqIDs);
  if (restFreqID < restFreqIDs.nelements()) {    // Should always be true
    spec.setRestFrequency(restFreqIDs(restFreqID),True);
  }

  // Set up frame conversion layer
  String frm;
  t.keywordSet().get("REFFRAME",frm);
  if (frm == "") frm = "TOPO";
  MFrequency::Types mtype;
  if (!MFrequency::getType(mtype, frm)) {
    // Should never happen
    pushLog("WARNING: Frequency type unknown assuming TOPO");
    mtype = MFrequency::TOPO;
  }

  // Set reference frame conversion  (requires row)
  MDirection dir = getDirection(whichRow);
  MEpoch epoch = getEpoch(whichRow);
  MPosition pos = getAntennaPosition();

  if (!spec.setReferenceConversion(mtype,epoch,pos,dir)) {
    throw(AipsError("Couldn't convert frequency frame."));
  }

  // Now velocity conversion if appropriate
  String unitStr;
  t.keywordSet().get("UNIT",unitStr);

  String dpl;
  t.keywordSet().get("DOPPLER",dpl);
  MDoppler::Types dtype;
  MDoppler::getType(dtype, dpl);

  // Only set velocity unit if non-blank and non-Hz
  if (!unitStr.empty()) {
     Unit unitU(unitStr);
     if (unitU==Unit("Hz")) {
     } else {
        spec.setVelocity(unitStr, dtype);
     }
  }

  return spec;
}


Bool SDMemTable::setCoordinate(const SpectralCoordinate& speccord,
                               uInt freqID) {
  Table t = table_.rwKeywordSet().asTable("FREQUENCIES");
  if (freqID > t.nrow() ) {
    throw(AipsError("SDMemTable::setCoordinate - coord no out of range"));
  }
  ScalarColumn<Double> rpc(t, "REFPIX");
  ScalarColumn<Double> rvc(t, "REFVAL");
  ScalarColumn<Double> incc(t, "INCREMENT");

  rpc.put(freqID, speccord.referencePixel()[0]);
  rvc.put(freqID, speccord.referenceValue()[0]);
  incc.put(freqID, speccord.increment()[0]);

  return True;
}

Int SDMemTable::nCoordinates() const
{
  return table_.keywordSet().asTable("FREQUENCIES").nrow();
}


std::vector<double> SDMemTable::getRestFreqs() const
{
  Table t = table_.keywordSet().asTable("FREQUENCIES");
  Vector<Double> tvec;
  t.keywordSet().get("RESTFREQS",tvec);
  std::vector<double> stlout;
  tvec.tovector(stlout);
  return stlout;
}

bool SDMemTable::putSDFreqTable(const SDFrequencyTable& sdft)
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.addColumn(ScalarColumnDesc<Double>("REFPIX"));
  td.addColumn(ScalarColumnDesc<Double>("REFVAL"));
  td.addColumn(ScalarColumnDesc<Double>("INCREMENT"));
  SetupNewTable aNewTab("freqs", td, Table::New);
  Table aTable (aNewTab, Table::Memory, sdft.length());
  ScalarColumn<Double> sc0(aTable, "REFPIX");
  ScalarColumn<Double> sc1(aTable, "REFVAL");
  ScalarColumn<Double> sc2(aTable, "INCREMENT");
  for (uInt i=0; i < sdft.length(); ++i) {
    sc0.put(i,sdft.referencePixel(i));
    sc1.put(i,sdft.referenceValue(i));
    sc2.put(i,sdft.increment(i));
  }
  String rf = sdft.refFrame();
  if (rf.contains("TOPO")) rf = "TOPO";
  String brf = sdft.baseRefFrame();
  if (brf.contains("TOPO")) brf = "TOPO";

  aTable.rwKeywordSet().define("BASEREFFRAME", brf);
  aTable.rwKeywordSet().define("REFFRAME", rf);
  aTable.rwKeywordSet().define("EQUINOX", sdft.equinox());
  aTable.rwKeywordSet().define("UNIT", sdft.unit());
  aTable.rwKeywordSet().define("UNIT", String(""));
  aTable.rwKeywordSet().define("DOPPLER", String("RADIO"));
  Vector<Double> rfvec;
  String rfunit;
  sdft.restFrequencies(rfvec,rfunit);
  Quantum<Vector<Double> > q(rfvec, rfunit);
  rfvec.resize();
  rfvec = q.getValue("Hz");
  aTable.rwKeywordSet().define("RESTFREQS", rfvec);
  table_.rwKeywordSet().defineTable("FREQUENCIES", aTable);
  return true;
}

bool SDMemTable::putSDFitTable(const SDFitTable& sdft)
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.addColumn(ArrayColumnDesc<String>("FUNCTIONS"));
  td.addColumn(ArrayColumnDesc<Int>("COMPONENTS"));
  td.addColumn(ArrayColumnDesc<Double>("PARAMETERS"));
  td.addColumn(ArrayColumnDesc<Bool>("PARMASK"));
  td.addColumn(ArrayColumnDesc<String>("FRAMEINFO"));
  SetupNewTable aNewTab("fits", td, Table::New);
  Table aTable(aNewTab, Table::Memory);
  ArrayColumn<String> sc0(aTable, "FUNCTIONS");
  ArrayColumn<Int> sc1(aTable, "COMPONENTS");
  ArrayColumn<Double> sc2(aTable, "PARAMETERS");
  ArrayColumn<Bool> sc3(aTable, "PARMASK");
  ArrayColumn<String> sc4(aTable, "FRAMEINFO");
  for (uInt i; i<sdft.length(); ++i) {
    const Vector<Double>& parms = sdft.getParameters(i);
    const Vector<Bool>& parmask = sdft.getParameterMask(i);
    const Vector<String>& funcs = sdft.getFunctions(i);
    const Vector<Int>& comps = sdft.getComponents(i);
    const Vector<String>& finfo = sdft.getFrameInfo(i);
    sc0.put(i,funcs);
    sc1.put(i,comps);
    sc3.put(i,parmask);
    sc2.put(i,parms);
    sc4.put(i,finfo);
  }
  table_.rwKeywordSet().defineTable("FITS", aTable);
  return true;
}

SDFitTable SDMemTable::getSDFitTable() const
{
  const Table& t = table_.keywordSet().asTable("FITS");
  Vector<Double> parms;
  Vector<Bool> parmask;
  Vector<String> funcs;
  Vector<String> finfo;
  Vector<Int> comps;
  ROArrayColumn<Double> parmsCol(t, "PARAMETERS");
  ROArrayColumn<Bool> parmaskCol(t, "PARMASK");
  ROArrayColumn<Int> compsCol(t, "COMPONENTS");
  ROArrayColumn<String> funcsCol(t, "FUNCTIONS");
  ROArrayColumn<String> finfoCol(t, "FRAMEINFO");
  uInt n = t.nrow();
  SDFitTable sdft;
  for (uInt i=0; i<n; ++i) {
    parmaskCol.get(i, parmask);
    parmsCol.get(i, parms);
    funcsCol.get(i, funcs);
    compsCol.get(i, comps);
    finfoCol.get(i, finfo);
    sdft.addFit(parms, parmask, funcs, comps, finfo);
  }
  return sdft;
}

SDFitTable SDMemTable::getSDFitTable(uInt whichRow) const {
  const Table& t = table_.keywordSet().asTable("FITS");
  if (t.nrow() == 0 || whichRow >= t.nrow()) return SDFitTable();
  Array<Int> fitid;
  fitCol_.get(whichRow, fitid);
  if (fitid.nelements() == 0) return SDFitTable();

  IPosition shp = fitid.shape();
  IPosition start(4, beamSel_, IFSel_, polSel_,0);
  IPosition end(4, beamSel_, IFSel_, polSel_, shp[3]-1);

  // reform the output array slice to be of dim=1
  Vector<Int> tmp = (fitid(start, end)).reform(IPosition(1,shp[3]));

  Vector<Double> parms;
  Vector<Bool> parmask;
  Vector<String> funcs;
  Vector<String> finfo;
  Vector<Int> comps;
  ROArrayColumn<Double> parmsCol(t, "PARAMETERS");
  ROArrayColumn<Bool> parmaskCol(t, "PARMASK");
  ROArrayColumn<Int> compsCol(t, "COMPONENTS");
  ROArrayColumn<String> funcsCol(t, "FUNCTIONS");
  ROArrayColumn<String> finfoCol(t, "FRAMEINFO");
  SDFitTable sdft;
  Int k=-1;
  for (uInt i=0; i< tmp.nelements(); ++i) {
    k = tmp[i];
    if ( k > -1 && k < t.nrow() ) {
      parms.resize();
      parmsCol.get(k, parms);
      parmask.resize();
      parmaskCol.get(k, parmask);
      funcs.resize();
      funcsCol.get(k, funcs);
      comps.resize();
      compsCol.get(k, comps);
      finfo.resize();
      finfoCol.get(k, finfo);
      sdft.addFit(parms, parmask, funcs, comps, finfo);
    }
  }
  return sdft;
}

void SDMemTable::addFit(uInt whichRow,
                        const Vector<Double>& p, const Vector<Bool>& m,
                        const Vector<String>& f, const Vector<Int>& c)
{
  if (whichRow >= nRow()) {
    throw(AipsError("Specified row out of range"));
  }
  Table t = table_.keywordSet().asTable("FITS");
  uInt nrow = t.nrow();
  t.addRow();
  ArrayColumn<Double> parmsCol(t, "PARAMETERS");
  ArrayColumn<Bool> parmaskCol(t, "PARMASK");
  ArrayColumn<Int> compsCol(t, "COMPONENTS");
  ArrayColumn<String> funcsCol(t, "FUNCTIONS");
  ArrayColumn<String> finfoCol(t, "FRAMEINFO");
  parmsCol.put(nrow, p);
  parmaskCol.put(nrow, m);
  compsCol.put(nrow, c);
  funcsCol.put(nrow, f);
  Vector<String> fi = mathutil::toVectorString(getCoordInfo());
  finfoCol.put(nrow, fi);

  Array<Int> fitarr;
  fitCol_.get(whichRow, fitarr);

  Array<Int> newarr;               // The new Array containing the fitid
  Int pos =-1;                     // The fitid position in the array
  if ( fitarr.nelements() == 0 ) { // no fits at all in this row
    Array<Int> arr(IPosition(4,nBeam(),nIF(),nPol(),1));
    arr = -1;
    newarr.reference(arr);
    pos = 0;
  } else {
    IPosition shp = fitarr.shape();
    IPosition start(4, beamSel_, IFSel_, polSel_,0);
    IPosition end(4, beamSel_, IFSel_, polSel_, shp[3]-1);
    // reform the output array slice to be of dim=1
    Array<Int> tmp = (fitarr(start, end)).reform(IPosition(1,shp[3]));
    const Vector<Int>& fits = tmp;
    VectorSTLIterator<Int> it(fits);
    Int i = 0;
    while (it != fits.end()) {
      if (*it == -1) {
        pos = i;
        break;
      }
      ++i;
      ++it;
    };
  }
  if (pos == -1) {
      mathutil::extendLastArrayAxis(newarr,fitarr, -1);
      pos = fitarr.shape()[3];     // the new element position
  } else {
    if (fitarr.nelements() > 0)
      newarr = fitarr;
  }
  newarr(IPosition(4, beamSel_, IFSel_, polSel_, pos)) = Int(nrow);
  fitCol_.put(whichRow, newarr);

}

SDFrequencyTable SDMemTable::getSDFreqTable() const
{
  const Table& t = table_.keywordSet().asTable("FREQUENCIES");
  SDFrequencyTable sdft;

  // Add refpix/refval/incr.  What are the units ? Hz I suppose
  // but it's nowhere described...
  Vector<Double> refPix, refVal, incr;
  ScalarColumn<Double> refPixCol(t, "REFPIX");
  ScalarColumn<Double> refValCol(t, "REFVAL");
  ScalarColumn<Double> incrCol(t, "INCREMENT");
  refPix = refPixCol.getColumn();
  refVal = refValCol.getColumn();
  incr = incrCol.getColumn();

  uInt n = refPix.nelements();
  for (uInt i=0; i<n; i++) {
     sdft.addFrequency(refPix[i], refVal[i], incr[i]);
  }

  // Frequency reference frame.  I don't know if this
  // is the correct frame.  It might be 'REFFRAME'
  // rather than 'BASEREFFRAME' ?
  String frame;
  t.keywordSet().get("REFFRAME",frame);
  sdft.setRefFrame(frame);
  t.keywordSet().get("BASEREFFRAME",frame);
  //sdft.setBaseRefFrame(frame);

  // Equinox
  Float equinox;
  t.keywordSet().get("EQUINOX", equinox);
  sdft.setEquinox(equinox);

  String unit;
  t.keywordSet().get("UNIT", unit);
  sdft.setUnit(unit);

  // Rest Frequency
  Vector<Double> restFreqs;
  t.keywordSet().get("RESTFREQS", restFreqs);
  for (uInt i=0; i<restFreqs.nelements(); i++) {
     sdft.addRestFrequency(restFreqs[i]);
  }
  sdft.setRestFrequencyUnit(String("Hz"));

  return sdft;
}

bool SDMemTable::putSDContainer(const SDContainer& sdc)
{
  uInt rno = table_.nrow();
  table_.addRow();

  timeCol_.put(rno, sdc.timestamp);
  srcnCol_.put(rno, sdc.sourcename);
  fldnCol_.put(rno, sdc.fieldname);
  specCol_.put(rno, sdc.getSpectrum());
  flagsCol_.put(rno, sdc.getFlags());
  tsCol_.put(rno, sdc.getTsys());
  scanCol_.put(rno, sdc.scanid);
  integrCol_.put(rno, sdc.interval);
  freqidCol_.put(rno, sdc.getFreqMap());
  restfreqidCol_.put(rno, sdc.getRestFreqMap());
  dirCol_.put(rno, sdc.getDirection());
  rbeamCol_.put(rno, sdc.refbeam);
  tcalCol_.put(rno, sdc.tcal);
  tcaltCol_.put(rno, sdc.tcaltime);
  azCol_.put(rno, sdc.azimuth);
  elCol_.put(rno, sdc.elevation);
  paraCol_.put(rno, sdc.parangle);
  fitCol_.put(rno, sdc.getFitMap());
  return true;
}

SDContainer SDMemTable::getSDContainer(uInt whichRow) const
{
  SDContainer sdc(nBeam(),nIF(),nPol(),nChan());
  timeCol_.get(whichRow, sdc.timestamp);
  srcnCol_.get(whichRow, sdc.sourcename);
  integrCol_.get(whichRow, sdc.interval);
  scanCol_.get(whichRow, sdc.scanid);
  fldnCol_.get(whichRow, sdc.fieldname);
  rbeamCol_.get(whichRow, sdc.refbeam);
  azCol_.get(whichRow, sdc.azimuth);
  elCol_.get(whichRow, sdc.elevation);
  paraCol_.get(whichRow, sdc.parangle);
  Vector<Float> tc;
  tcalCol_.get(whichRow, tc);
  sdc.tcal[0] = tc[0];sdc.tcal[1] = tc[1];
  tcaltCol_.get(whichRow, sdc.tcaltime);

  Array<Float> spectrum;
  Array<Float> tsys;
  Array<uChar> flagtrum;
  Vector<uInt> fmap;
  Array<Double> direction;
  Array<Int> fits;

  specCol_.get(whichRow, spectrum);
  sdc.putSpectrum(spectrum);
  flagsCol_.get(whichRow, flagtrum);
  sdc.putFlags(flagtrum);
  tsCol_.get(whichRow, tsys);
  sdc.putTsys(tsys);
  freqidCol_.get(whichRow, fmap);
  sdc.putFreqMap(fmap);
  restfreqidCol_.get(whichRow, fmap);
  sdc.putRestFreqMap(fmap);
  dirCol_.get(whichRow, direction);
  sdc.putDirection(direction);
  fitCol_.get(whichRow, fits);
  sdc.putFitMap(fits);
  return sdc;
}

bool SDMemTable::putSDHeader(const SDHeader& sdh)
{
  table_.rwKeywordSet().define("nIF", sdh.nif);
  table_.rwKeywordSet().define("nBeam", sdh.nbeam);
  table_.rwKeywordSet().define("nPol", sdh.npol);
  table_.rwKeywordSet().define("nChan", sdh.nchan);
  table_.rwKeywordSet().define("Observer", sdh.observer);
  table_.rwKeywordSet().define("Project", sdh.project);
  table_.rwKeywordSet().define("Obstype", sdh.obstype);
  table_.rwKeywordSet().define("AntennaName", sdh.antennaname);
  table_.rwKeywordSet().define("AntennaPosition", sdh.antennaposition);
  table_.rwKeywordSet().define("Equinox", sdh.equinox);
  table_.rwKeywordSet().define("FreqRefFrame", sdh.freqref);
  table_.rwKeywordSet().define("FreqRefVal", sdh.reffreq);
  table_.rwKeywordSet().define("Bandwidth", sdh.bandwidth);
  table_.rwKeywordSet().define("UTC", sdh.utc);
  table_.rwKeywordSet().define("FluxUnit", sdh.fluxunit);
  table_.rwKeywordSet().define("Epoch", sdh.epoch);
  return true;
}

SDHeader SDMemTable::getSDHeader() const
{
  SDHeader sdh;
  table_.keywordSet().get("nBeam",sdh.nbeam);
  table_.keywordSet().get("nIF",sdh.nif);
  table_.keywordSet().get("nPol",sdh.npol);
  table_.keywordSet().get("nChan",sdh.nchan);
  table_.keywordSet().get("Observer", sdh.observer);
  table_.keywordSet().get("Project", sdh.project);
  table_.keywordSet().get("Obstype", sdh.obstype);
  table_.keywordSet().get("AntennaName", sdh.antennaname);
  table_.keywordSet().get("AntennaPosition", sdh.antennaposition);
  table_.keywordSet().get("Equinox", sdh.equinox);
  table_.keywordSet().get("FreqRefFrame", sdh.freqref);
  table_.keywordSet().get("FreqRefVal", sdh.reffreq);
  table_.keywordSet().get("Bandwidth", sdh.bandwidth);
  table_.keywordSet().get("UTC", sdh.utc);
  table_.keywordSet().get("FluxUnit", sdh.fluxunit);
  table_.keywordSet().get("Epoch", sdh.epoch);
  return sdh;
}
void SDMemTable::makePersistent(const std::string& filename)
{
  table_.deepCopy(filename,Table::New);

}

Int SDMemTable::nScan() const {
  Int n = 0;
  Int previous = -1;Int current=0;
  for (uInt i=0; i< scanCol_.nrow();i++) {
    scanCol_.getScalar(i,current);
    if (previous != current) {
      previous = current;
      n++;
    }
  }
  return n;
}

String SDMemTable::formatSec(Double x) const
{
  Double xcop = x;
  MVTime mvt(xcop/24./3600.);  // make days

  if (x < 59.95)
    return  String("      ") + mvt.string(MVTime::TIME_CLEAN_NO_HM, 7)+"s";
  else if (x < 3599.95)
    return String("   ") + mvt.string(MVTime::TIME_CLEAN_NO_H,7)+" ";
  else {
    ostringstream oss;
    oss << setw(2) << std::right << setprecision(1) << mvt.hour();
    oss << ":" << mvt.string(MVTime::TIME_CLEAN_NO_H,7) << " ";
    return String(oss);
  }
};

String SDMemTable::formatDirection(const MDirection& md) const
{
  Vector<Double> t = md.getAngle(Unit(String("rad"))).getValue();
  Int prec = 7;

  MVAngle mvLon(t[0]);
  String sLon = mvLon.string(MVAngle::TIME,prec);
  MVAngle mvLat(t[1]);
  String sLat = mvLat.string(MVAngle::ANGLE+MVAngle::DIG2,prec);
  return sLon + String(" ") + sLat;
}


std::string SDMemTable::getFluxUnit() const
{
  String tmp;
  table_.keywordSet().get("FluxUnit", tmp);
  return tmp;
}

void SDMemTable::setFluxUnit(const std::string& unit)
{
  String tmp(unit);
  Unit tU(tmp);
  if (tU==Unit("K") || tU==Unit("Jy")) {
     table_.rwKeywordSet().define(String("FluxUnit"), tmp);
  } else {
     throw AipsError("Illegal unit - must be compatible with Jy or K");
  }
}


void SDMemTable::setInstrument(const std::string& name)
{
  Bool throwIt = True;
  Instrument ins = SDAttr::convertInstrument(name, throwIt);
  String nameU(name);
  nameU.upcase();
  table_.rwKeywordSet().define(String("AntennaName"), nameU);
}

std::string SDMemTable::summary(bool verbose) const  {

  // Format header info
  ostringstream oss;
  oss << endl;
  oss << "--------------------------------------------------------------------------------" << endl;
  oss << " Scan Table Summary" << endl;
  oss << "--------------------------------------------------------------------------------" << endl;
  oss.flags(std::ios_base::left);
  oss << setw(15) << "Beams:" << setw(4) << nBeam() << endl
      << setw(15) << "IFs:" << setw(4) << nIF() << endl
      << setw(15) << "Polarisations:" << setw(4) << nPol() << endl
      << setw(15) << "Channels:"  << setw(4) << nChan() << endl;
  oss << endl;
  String tmp;
  table_.keywordSet().get("Observer", tmp);
  oss << setw(15) << "Observer:" << tmp << endl;
  oss << setw(15) << "Obs Date:" << getTime(-1,True) << endl;
  table_.keywordSet().get("Project", tmp);
  oss << setw(15) << "Project:" << tmp << endl;
  table_.keywordSet().get("Obstype", tmp);
  oss << setw(15) << "Obs. Type:" << tmp << endl;
  table_.keywordSet().get("AntennaName", tmp);
  oss << setw(15) << "Antenna Name:" << tmp << endl;
  table_.keywordSet().get("FluxUnit", tmp);
  oss << setw(15) << "Flux Unit:" << tmp << endl;
  Table t = table_.keywordSet().asTable("FREQUENCIES");
  Vector<Double> vec;
  t.keywordSet().get("RESTFREQS",vec);
  oss << setw(15) << "Rest Freqs:";
  if (vec.nelements() > 0) {
      oss << setprecision(10) << vec << " [Hz]" << endl;
  } else {
      oss << "None set" << endl;
  }
  oss << setw(15) << "Abcissa:" << getAbcissaString() << endl;
  oss << setw(15) << "Cursor:" << "Beam[" << getBeam() << "] "
      << "IF[" << getIF() << "] " << "Pol[" << getPol() << "]" << endl;
  oss << endl;

  String dirtype ="Position ("+ MDirection::showType(getDirectionReference()) + ")";
  oss << setw(5) << "Scan"
      << setw(15) << "Source"
      << setw(24) << dirtype
      << setw(10) << "Time"
      << setw(18) << "Integration"
      << setw(7) << "FreqIDs" << endl;
  oss << "--------------------------------------------------------------------------------" << endl;

  // Generate list of scan start and end integrations
  Vector<Int> scanIDs = scanCol_.getColumn();
  Vector<uInt> startInt, endInt;
  mathutil::scanBoundaries(startInt, endInt, scanIDs);

  const uInt nScans = startInt.nelements();
  String name;
  Vector<uInt> freqIDs, listFQ;
  Vector<uInt> restFreqIDs, listRestFQ;
  uInt nInt;

  for (uInt i=0; i<nScans; i++) {
    // Get things from first integration of scan
    String time = getTime(startInt(i),False);
    String tInt = formatSec(Double(getInterval(startInt(i))));
    String posit = formatDirection(getDirection(startInt(i),True));
    srcnCol_.getScalar(startInt(i),name);

    // Find all the FreqIDs in this scan
    listFQ.resize(0);
    listRestFQ.resize(0);
    for (uInt j=startInt(i); j<endInt(i)+1; j++) {
      freqidCol_.get(j, freqIDs);
      for (uInt k=0; k<freqIDs.nelements(); k++) {
        mathutil::addEntry(listFQ, freqIDs(k));
      }
//
      restfreqidCol_.get(j, restFreqIDs);
      for (uInt k=0; k<restFreqIDs.nelements(); k++) {
        mathutil::addEntry(listRestFQ, restFreqIDs(k));
      }
    }

    nInt = endInt(i) - startInt(i) + 1;
    oss << setw(3) << std::right << i << std::left << setw(2) << "  "
        << setw(15) << name
        << setw(24) << posit
        << setw(10) << time
        << setw(3) << std::right << nInt  << setw(3) << " x " << std::left
        << setw(6) <<  tInt
        << " " << listFQ << " " << listRestFQ << endl;
  }
  oss << endl;
  oss << "Table contains " << table_.nrow() << " integration(s) in "
      << nScans << " scan(s)." << endl;

  // Frequency Table
  if (verbose) {
    std::vector<string> info = getCoordInfo();
    SDFrequencyTable sdft = getSDFreqTable();
    oss << endl << endl;
    oss << "FreqID  Frame   RefFreq(Hz)     RefPix   Increment(Hz)" << endl;
    oss << "--------------------------------------------------------------------------------" << endl;
    for (uInt i=0; i<sdft.length(); i++) {
      oss << setw(8) << i << setw(8)
          << info[3] << setw(16) << setprecision(8)
          << sdft.referenceValue(i) << setw(10)
          << sdft.referencePixel(i) << setw(12)
          << sdft.increment(i) << endl;
    }
    oss << "--------------------------------------------------------------------------------" << endl;
  }
  return String(oss);
}
/*
std::string SDMemTable::scanSummary(const std::vector<int>& whichScans) {
  ostringstream oss;
  Vector<Int> scanIDs = scanCol_.getColumn();
  Vector<uInt> startInt, endInt;
  mathutil::scanBoundaries(startInt, endInt, scanIDs);
  const uInt nScans = startInt.nelements();
  std::vector<int>::const_iterator it(whichScans);
  return String(oss);
}
*/
Int SDMemTable::nBeam() const
{
  Int n;
  table_.keywordSet().get("nBeam",n);
  return n;
}

Int SDMemTable::nIF() const {
  Int n;
  table_.keywordSet().get("nIF",n);
  return n;
}

Int SDMemTable::nPol() const {
  Int n;
  table_.keywordSet().get("nPol",n);
  return n;
}

Int SDMemTable::nChan() const {
  Int n;
  table_.keywordSet().get("nChan",n);
  return n;
}

Table SDMemTable::getHistoryTable() const
{
  return table_.keywordSet().asTable("HISTORY");
}

void SDMemTable::appendToHistoryTable(const Table& otherHist)
{
  Table t = table_.rwKeywordSet().asTable("HISTORY");
  const String sep = "--------------------------------------------------------------------------------";
  addHistory(sep);
  TableCopy::copyRows(t, otherHist, t.nrow(), 0, otherHist.nrow());
  addHistory(sep);
}

void SDMemTable::addHistory(const std::string& hist)
{
  Table t = table_.rwKeywordSet().asTable("HISTORY");
  uInt nrow = t.nrow();
  t.addRow();
  ScalarColumn<String> itemCol(t, "ITEM");
  itemCol.put(nrow, hist);
}

std::vector<std::string> SDMemTable::getHistory() const
{
  Vector<String> history;
  const Table& t = table_.keywordSet().asTable("HISTORY");
  uInt nrow = t.nrow();
  ROScalarColumn<String> itemCol(t, "ITEM");
  std::vector<std::string> stlout;
  String hist;
  for (uInt i=0; i<nrow; ++i) {
    itemCol.get(i, hist);
    stlout.push_back(hist);
  }
  return stlout;
}


/*
  void SDMemTable::maskChannels(const std::vector<Int>& whichChans ) {

  std::vector<int>::iterator it;
  ArrayAccessor<uChar, Axis<asap::PolAxis> > j(flags_);
  for (it = whichChans.begin(); it != whichChans.end(); it++) {
    j.reset(j.begin(uInt(*it)));
    for (ArrayAccessor<uChar, Axis<asap::BeamAxis> > i(j); i != i.end(); ++i) {
      for (ArrayAccessor<uChar, Axis<asap::IFAxis> > ii(i); ii != ii.end(); ++ii) {
        for (ArrayAccessor<uChar, Axis<asap::ChanAxis> > iii(ii);
             iii != iii.end(); ++iii) {
          (*iii) =
        }
      }
    }
  }

}
*/
void SDMemTable::flag(int whichRow)
  {
  Array<uChar> arr;
  flagsCol_.get(whichRow, arr);

  ArrayAccessor<uChar, Axis<asap::BeamAxis> > aa0(arr);
  aa0.reset(aa0.begin(uInt(beamSel_)));//go to beam
  ArrayAccessor<uChar, Axis<asap::IFAxis> > aa1(aa0);
  aa1.reset(aa1.begin(uInt(IFSel_)));// go to IF
  ArrayAccessor<uChar, Axis<asap::PolAxis> > aa2(aa1);
  aa2.reset(aa2.begin(uInt(polSel_)));// go to pol

  for (ArrayAccessor<uChar, Axis<asap::ChanAxis> > i(aa2); i != i.end(); ++i) {
    (*i) = uChar(True);
  }

  flagsCol_.put(whichRow, arr);
}

MDirection::Types SDMemTable::getDirectionReference() const
{
  Float eq;
  table_.keywordSet().get("Equinox",eq);
  std::map<float,string> mp;
  mp[2000.0] = "J2000";
  mp[1950.0] = "B1950";
  MDirection::Types mdr;
  if (!MDirection::getType(mdr, mp[eq])) {
    mdr = MDirection::J2000;
    pushLog("WARNING: Unknown equinox using J2000");
  }

  return mdr;
}

MEpoch::Types SDMemTable::getTimeReference() const
{
  MEpoch::Types met;
  String ep;
  table_.keywordSet().get("Epoch",ep);
  if (!MEpoch::getType(met, ep)) {
     pushLog("WARNING: Epoch type unknown - using UTC");
    met = MEpoch::UTC;
  }

  return met;
}


Bool SDMemTable::setRestFreqs(const Vector<Double>& restFreqsIn,
                              const String& sUnit,
                              const vector<string>& lines,
                              const String& source,
                              Int whichIF)
{
   const Int nIFs = nIF();
   if (whichIF>=nIFs) {
      throw(AipsError("Illegal IF index"));
   }

   // Find vector of restfrequencies
   // Double takes precedence over String
   Unit unit;
   Vector<Double> restFreqs;
   if (restFreqsIn.nelements()>0) {
      restFreqs.resize(restFreqsIn.nelements());
      restFreqs = restFreqsIn;
      unit = Unit(sUnit);
   } else if (lines.size()>0) {
      const uInt nLines = lines.size();
      unit = Unit("Hz");
      restFreqs.resize(nLines);
      MFrequency lineFreq;
      for (uInt i=0; i<nLines; i++) {
         String tS(lines[i]);
         tS.upcase();
         if (MeasTable::Line(lineFreq, tS)) {
            restFreqs[i] = lineFreq.getValue().getValue();          // Hz
         } else {
            String s = String(lines[i]) +
              String(" is an unrecognized spectral line");
            throw(AipsError(s));
         }
      }
   } else {
      throw(AipsError("You have not specified any rest frequencies or lines"));
   }

   // If multiple restfreqs, must be length nIF. In this
   // case we will just replace the rest frequencies
   // We can't disinguish scalar and vector of length 1
   const uInt nRestFreqs = restFreqs.nelements();
   Int idx = -1;
   SDFrequencyTable sdft = getSDFreqTable();

   ostringstream oss;
   if (nRestFreqs>1) {
     // Replace restFreqs, one per IF
     if (nRestFreqs != nIFs) {
       throw (AipsError("Number of rest frequencies must be equal to the number of IFs"));
     }
     ostringstream oss;
     oss << "Replaced rest frequencies, one per IF, with given list : " << restFreqs;
     sdft.deleteRestFrequencies();
     for (uInt i=0; i<nRestFreqs; i++) {
       Quantum<Double> rf(restFreqs[i], unit);
       sdft.addRestFrequency(rf.getValue("Hz"));
     }
   } else {

     // Add new rest freq
      Quantum<Double> rf(restFreqs[0], unit);
      idx = sdft.addRestFrequency(rf.getValue("Hz"));
      if (whichIF>=0) {
         oss << "Selected given rest frequency (" << restFreqs[0] << ") for IF " << whichIF << endl;
      } else {
         oss << "Selected given rest frequency (" << restFreqs[0] << ") for all IFs" << endl;
      }
   }
   pushLog(String(oss));
   // Replace
   Bool empty = source.empty();
   Bool ok = False;
   if (putSDFreqTable(sdft)) {
      const uInt nRow = table_.nrow();
      String srcName;
      Vector<uInt> restFreqIDs;
      for (uInt i=0; i<nRow; i++) {
         srcnCol_.get(i, srcName);
         restfreqidCol_.get(i,restFreqIDs);
         if (idx==-1) {
           // Replace vector of restFreqs; one per IF.
           // No selection possible
            for (uInt i=0; i<nIFs; i++) restFreqIDs[i] = i;
         } else {
           // Set RestFreqID for selected data
            if (empty || source==srcName) {
               if (whichIF<0) {
                  restFreqIDs = idx;
               } else {
                  restFreqIDs[whichIF] = idx;
               }
            }
         }
         restfreqidCol_.put(i,restFreqIDs);
      }
      ok = True;
   } else {
     ok = False;
   }

   return ok;
}

std::string SDMemTable::spectralLines() const
{
   Vector<String> lines = MeasTable::Lines();
   MFrequency lineFreq;
   Double freq;
   ostringstream oss;

   oss.flags(std::ios_base::left);
   oss << "Line      Frequency (Hz)" << endl;
   oss << "-----------------------" << endl;
   for (uInt i=0; i<lines.nelements(); i++) {
     MeasTable::Line(lineFreq, lines[i]);
     freq = lineFreq.getValue().getValue();          // Hz
     oss << setw(11) << lines[i] << setprecision(10) << freq << endl;
   }
   return String(oss);
}

void SDMemTable::renumber()
{
  uInt nRow = scanCol_.nrow();
  Int newscanid = 0;
  Int cIdx;// the current scanid
  // get the first scanid
  scanCol_.getScalar(0,cIdx);
  Int pIdx = cIdx;// the scanid of the previous row
  for (uInt i=0; i<nRow;++i) {
    scanCol_.getScalar(i,cIdx);
    if (pIdx == cIdx) {
      // renumber
      scanCol_.put(i,newscanid);
    } else {
      ++newscanid;
      pIdx = cIdx;   // store scanid
      --i;           // don't increment next loop
    }
  }
}


void SDMemTable::getCursorSlice(IPosition& start, IPosition& end,
                                const IPosition& shape) const
{
  const uInt nDim = shape.nelements();
  start.resize(nDim);
  end.resize(nDim);

  start(asap::BeamAxis) = beamSel_;
  end(asap::BeamAxis) = beamSel_;
  start(asap::IFAxis) = IFSel_;
  end(asap::IFAxis) = IFSel_;

  start(asap::PolAxis) = polSel_;
  end(asap::PolAxis) = polSel_;

  start(asap::ChanAxis) = 0;
  end(asap::ChanAxis) = shape(asap::ChanAxis) - 1;
}


std::vector<float> SDMemTable::getFloatSpectrum(const Array<Float>& arr) const
  // Get spectrum at cursor location
{

  // Setup accessors
  ArrayAccessor<Float, Axis<asap::BeamAxis> > aa0(arr);
  aa0.reset(aa0.begin(uInt(beamSel_)));                    // Beam selection

  ArrayAccessor<Float, Axis<asap::IFAxis> > aa1(aa0);
  aa1.reset(aa1.begin(uInt(IFSel_)));                      // IF selection

  ArrayAccessor<Float, Axis<asap::PolAxis> > aa2(aa1);
  aa2.reset(aa2.begin(uInt(polSel_)));                     // Pol selection

  std::vector<float> spectrum;
  for (ArrayAccessor<Float, Axis<asap::ChanAxis> > i(aa2); i != i.end(); ++i) {
    spectrum.push_back(*i);
  }
  return spectrum;
}

void SDMemTable::calculateAZEL()
{
  MPosition mp = getAntennaPosition();
  ostringstream oss;
  oss << "Computed azimuth/elevation using " << endl
      << mp << endl;
  for (uInt i=0; i<nRow();++i) {
    MEpoch me = getEpoch(i);
    MDirection md = getDirection(i,False);
    oss  << " Time: " << getTime(i,False) << " Direction: " << formatDirection(md)
         << endl << "     => ";
    MeasFrame frame(mp, me);
    Vector<Double> azel =
        MDirection::Convert(md, MDirection::Ref(MDirection::AZEL,
                                                frame)
                            )().getAngle("rad").getValue();
    azCol_.put(i,azel[0]);
    elCol_.put(i,azel[1]);
    oss << "azel: " << azel[0]/C::pi*180.0 << " "
        << azel[1]/C::pi*180.0 << " (deg)" << endl;
  }
  pushLog(String(oss));
}

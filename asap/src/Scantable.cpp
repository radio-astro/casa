//
// C++ Implementation: Scantable
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <map>
#include <fstream>

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/OS/Path.h>
#include <casa/OS/File.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayAccessor.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <casa/Arrays/Slice.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Containers/RecordField.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Logging/LogIO.h>

#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableVector.h>
#include <tables/Tables/TableIter.h>

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/MVAngle.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/MeasTable.h>
// needed to avoid error in .tcc
#include <measures/Measures/MCDirection.h>
//
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MEpoch.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

#include "Scantable.h"
#include "STPolLinear.h"
#include "STPolCircular.h"
#include "STPolStokes.h"
#include "STAttr.h"
#include "MathUtils.h"

using namespace casa;

namespace asap {

std::map<std::string, STPol::STPolFactory *> Scantable::factories_;

void Scantable::initFactories() {
  if ( factories_.empty() ) {
    Scantable::factories_["linear"] = &STPolLinear::myFactory;
    Scantable::factories_["circular"] = &STPolCircular::myFactory;
    Scantable::factories_["stokes"] = &STPolStokes::myFactory;
  }
}

Scantable::Scantable(Table::TableType ttype) :
  type_(ttype)
{
  initFactories();
  setupMainTable();
  freqTable_ = STFrequencies(*this);
  table_.rwKeywordSet().defineTable("FREQUENCIES", freqTable_.table());
  weatherTable_ = STWeather(*this);
  table_.rwKeywordSet().defineTable("WEATHER", weatherTable_.table());
  focusTable_ = STFocus(*this);
  table_.rwKeywordSet().defineTable("FOCUS", focusTable_.table());
  tcalTable_ = STTcal(*this);
  table_.rwKeywordSet().defineTable("TCAL", tcalTable_.table());
  moleculeTable_ = STMolecules(*this);
  table_.rwKeywordSet().defineTable("MOLECULES", moleculeTable_.table());
  historyTable_ = STHistory(*this);
  table_.rwKeywordSet().defineTable("HISTORY", historyTable_.table());
  fitTable_ = STFit(*this);
  table_.rwKeywordSet().defineTable("FIT", fitTable_.table());
  originalTable_ = table_;
  attach();
}

Scantable::Scantable(const std::string& name, Table::TableType ttype) :
  type_(ttype)
{
  initFactories();

  Table tab(name, Table::Update);
  uInt version = tab.keywordSet().asuInt("VERSION");
  if (version != version_) {
    throw(AipsError("Unsupported version of ASAP file."));
  }
  if ( type_ == Table::Memory ) {
    table_ = tab.copyToMemoryTable(generateName());
  } else {
    table_ = tab;
  }

  attachSubtables();
  originalTable_ = table_;
  attach();
}
/*
Scantable::Scantable(const std::string& name, Table::TableType ttype) :
  type_(ttype)
{
  initFactories();
  Table tab(name, Table::Update);
  uInt version = tab.keywordSet().asuInt("VERSION");
  if (version != version_) {
    throw(AipsError("Unsupported version of ASAP file."));
  }
  if ( type_ == Table::Memory ) {
    table_ = tab.copyToMemoryTable(generateName());
  } else {
    table_ = tab;
  }

  attachSubtables();
  originalTable_ = table_;
  attach();
}
*/

Scantable::Scantable( const Scantable& other, bool clear )
{
  // with or without data
  String newname = String(generateName());
  type_ = other.table_.tableType();
  if ( other.table_.tableType() == Table::Memory ) {
      if ( clear ) {
        table_ = TableCopy::makeEmptyMemoryTable(newname,
                                                 other.table_, True);
      } else
        table_ = other.table_.copyToMemoryTable(newname);
  } else {
      other.table_.deepCopy(newname, Table::New, False,
                            other.table_.endianFormat(),
                            Bool(clear));
      table_ = Table(newname, Table::Update);
      table_.markForDelete();
  }
  /// @todo reindex SCANNO, recompute nbeam, nif, npol
  if ( clear ) copySubtables(other);
  attachSubtables();
  originalTable_ = table_;
  attach();
}

void Scantable::copySubtables(const Scantable& other) {
  Table t = table_.rwKeywordSet().asTable("FREQUENCIES");
  TableCopy::copyRows(t, other.freqTable_.table());
  t = table_.rwKeywordSet().asTable("FOCUS");
  TableCopy::copyRows(t, other.focusTable_.table());
  t = table_.rwKeywordSet().asTable("WEATHER");
  TableCopy::copyRows(t, other.weatherTable_.table());
  t = table_.rwKeywordSet().asTable("TCAL");
  TableCopy::copyRows(t, other.tcalTable_.table());
  t = table_.rwKeywordSet().asTable("MOLECULES");
  TableCopy::copyRows(t, other.moleculeTable_.table());
  t = table_.rwKeywordSet().asTable("HISTORY");
  TableCopy::copyRows(t, other.historyTable_.table());
  t = table_.rwKeywordSet().asTable("FIT");
  TableCopy::copyRows(t, other.fitTable_.table());
}

void Scantable::attachSubtables()
{
  freqTable_ = STFrequencies(table_);
  focusTable_ = STFocus(table_);
  weatherTable_ = STWeather(table_);
  tcalTable_ = STTcal(table_);
  moleculeTable_ = STMolecules(table_);
  historyTable_ = STHistory(table_);
  fitTable_ = STFit(table_);
}

Scantable::~Scantable()
{
  //cout << "~Scantable() " << this << endl;
}

void Scantable::setupMainTable()
{
  TableDesc td("", "1", TableDesc::Scratch);
  td.comment() = "An ASAP Scantable";
  td.rwKeywordSet().define("VERSION", uInt(version_));

  // n Cycles
  td.addColumn(ScalarColumnDesc<uInt>("SCANNO"));
  // new index every nBeam x nIF x nPol
  td.addColumn(ScalarColumnDesc<uInt>("CYCLENO"));

  td.addColumn(ScalarColumnDesc<uInt>("BEAMNO"));
  td.addColumn(ScalarColumnDesc<uInt>("IFNO"));
  // linear, circular, stokes
  td.rwKeywordSet().define("POLTYPE", String("linear"));
  td.addColumn(ScalarColumnDesc<uInt>("POLNO"));

  td.addColumn(ScalarColumnDesc<uInt>("FREQ_ID"));
  td.addColumn(ScalarColumnDesc<uInt>("MOLECULE_ID"));

  ScalarColumnDesc<Int> refbeamnoColumn("REFBEAMNO");
  refbeamnoColumn.setDefault(Int(-1));
  td.addColumn(refbeamnoColumn);

  ScalarColumnDesc<uInt> flagrowColumn("FLAGROW");
  flagrowColumn.setDefault(uInt(0));
  td.addColumn(flagrowColumn);

  td.addColumn(ScalarColumnDesc<Double>("TIME"));
  TableMeasRefDesc measRef(MEpoch::UTC); // UTC as default
  TableMeasValueDesc measVal(td, "TIME");
  TableMeasDesc<MEpoch> mepochCol(measVal, measRef);
  mepochCol.write(td);

  td.addColumn(ScalarColumnDesc<Double>("INTERVAL"));

  td.addColumn(ScalarColumnDesc<String>("SRCNAME"));
  // Type of source (on=0, off=1, other=-1)
  ScalarColumnDesc<Int> stypeColumn("SRCTYPE");
  stypeColumn.setDefault(Int(-1));
  td.addColumn(stypeColumn);
  td.addColumn(ScalarColumnDesc<String>("FIELDNAME"));

  //The actual Data Vectors
  td.addColumn(ArrayColumnDesc<Float>("SPECTRA"));
  td.addColumn(ArrayColumnDesc<uChar>("FLAGTRA"));
  td.addColumn(ArrayColumnDesc<Float>("TSYS"));

  td.addColumn(ArrayColumnDesc<Double>("DIRECTION",
                                       IPosition(1,2),
                                       ColumnDesc::Direct));
  TableMeasRefDesc mdirRef(MDirection::J2000); // default
  TableMeasValueDesc tmvdMDir(td, "DIRECTION");
  // the TableMeasDesc gives the column a type
  TableMeasDesc<MDirection> mdirCol(tmvdMDir, mdirRef);
  // a uder set table type e.g. GALCTIC, B1950 ...
  td.rwKeywordSet().define("DIRECTIONREF", String("J2000"));
  // writing create the measure column
  mdirCol.write(td);
  td.addColumn(ScalarColumnDesc<Float>("AZIMUTH"));
  td.addColumn(ScalarColumnDesc<Float>("ELEVATION"));
  td.addColumn(ScalarColumnDesc<Float>("OPACITY"));

  td.addColumn(ScalarColumnDesc<uInt>("TCAL_ID"));
  ScalarColumnDesc<Int> fitColumn("FIT_ID");
  fitColumn.setDefault(Int(-1));
  td.addColumn(fitColumn);

  td.addColumn(ScalarColumnDesc<uInt>("FOCUS_ID"));
  td.addColumn(ScalarColumnDesc<uInt>("WEATHER_ID"));

  // columns which just get dragged along, as they aren't used in asap
  td.addColumn(ScalarColumnDesc<Double>("SRCVELOCITY"));
  td.addColumn(ArrayColumnDesc<Double>("SRCPROPERMOTION"));
  td.addColumn(ArrayColumnDesc<Double>("SRCDIRECTION"));
  td.addColumn(ArrayColumnDesc<Double>("SCANRATE"));

  td.rwKeywordSet().define("OBSMODE", String(""));

  // Now create Table SetUp from the description.
  SetupNewTable aNewTab(generateName(), td, Table::Scratch);
  table_ = Table(aNewTab, type_, 0);
  originalTable_ = table_;
}

void Scantable::attach()
{
  timeCol_.attach(table_, "TIME");
  srcnCol_.attach(table_, "SRCNAME");
  srctCol_.attach(table_, "SRCTYPE");
  specCol_.attach(table_, "SPECTRA");
  flagsCol_.attach(table_, "FLAGTRA");
  tsysCol_.attach(table_, "TSYS");
  cycleCol_.attach(table_,"CYCLENO");
  scanCol_.attach(table_, "SCANNO");
  beamCol_.attach(table_, "BEAMNO");
  ifCol_.attach(table_, "IFNO");
  polCol_.attach(table_, "POLNO");
  integrCol_.attach(table_, "INTERVAL");
  azCol_.attach(table_, "AZIMUTH");
  elCol_.attach(table_, "ELEVATION");
  dirCol_.attach(table_, "DIRECTION");
  fldnCol_.attach(table_, "FIELDNAME");
  rbeamCol_.attach(table_, "REFBEAMNO");

  mweatheridCol_.attach(table_,"WEATHER_ID");
  mfitidCol_.attach(table_,"FIT_ID");
  mfreqidCol_.attach(table_, "FREQ_ID");
  mtcalidCol_.attach(table_, "TCAL_ID");
  mfocusidCol_.attach(table_, "FOCUS_ID");
  mmolidCol_.attach(table_, "MOLECULE_ID");

  //Add auxiliary column for row-based flagging (CAS-1433 Wataru Kawasaki)
  attachAuxColumnDef(flagrowCol_, "FLAGROW", 0);

}

template<class T, class T2>
void Scantable::attachAuxColumnDef(ScalarColumn<T>& col,
				   const String& colName,
				   const T2& defValue)
{
  try {
    col.attach(table_, colName);
  } catch (TableError& err) {
    String errMesg = err.getMesg();
    if (errMesg == "Table column " + colName + " is unknown") {
      table_.addColumn(ScalarColumnDesc<T>(colName));
      col.attach(table_, colName);
      col.fillColumn(static_cast<T>(defValue));
    } else {
      throw;
    }
  } catch (...) {
    throw;
  }
}

template<class T, class T2>
void Scantable::attachAuxColumnDef(ArrayColumn<T>& col,
				   const String& colName,
				   const Array<T2>& defValue)
{
  try {
    col.attach(table_, colName);
  } catch (TableError& err) {
    String errMesg = err.getMesg();
    if (errMesg == "Table column " + colName + " is unknown") {
      table_.addColumn(ArrayColumnDesc<T>(colName));
      col.attach(table_, colName);

      int size = 0;
      ArrayIterator<T2>& it = defValue.begin();
      while (it != defValue.end()) {
	++size;
	++it;
      }
      IPosition ip(1, size);
      Array<T>& arr(ip);
      for (int i = 0; i < size; ++i)
	arr[i] = static_cast<T>(defValue[i]);

      col.fillColumn(arr);
    } else {
      throw;
    }
  } catch (...) {
    throw;
  }
}

void Scantable::setHeader(const STHeader& sdh)
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
  table_.rwKeywordSet().define("POLTYPE", sdh.poltype);
}

STHeader Scantable::getHeader() const
{
  STHeader sdh;
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
  table_.keywordSet().get("POLTYPE", sdh.poltype);
  return sdh;
}

void Scantable::setSourceType( int stype )
{
  if ( stype < 0 || stype > 1 )
    throw(AipsError("Illegal sourcetype."));
  TableVector<Int> tabvec(table_, "SRCTYPE");
  tabvec = Int(stype);
}

bool Scantable::conformant( const Scantable& other )
{
  return this->getHeader().conformant(other.getHeader());
}



std::string Scantable::formatSec(Double x) const
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

std::string Scantable::formatDirection(const MDirection& md) const
{
  Vector<Double> t = md.getAngle(Unit(String("rad"))).getValue();
  Int prec = 7;

  MVAngle mvLon(t[0]);
  String sLon = mvLon.string(MVAngle::TIME,prec);
  uInt tp = md.getRef().getType();
  if (tp == MDirection::GALACTIC ||
      tp == MDirection::SUPERGAL ) {
    sLon = mvLon(0.0).string(MVAngle::ANGLE_CLEAN,prec);
  }
  MVAngle mvLat(t[1]);
  String sLat = mvLat.string(MVAngle::ANGLE+MVAngle::DIG2,prec);
  return sLon + String(" ") + sLat;
}


std::string Scantable::getFluxUnit() const
{
  return table_.keywordSet().asString("FluxUnit");
}

void Scantable::setFluxUnit(const std::string& unit)
{
  String tmp(unit);
  Unit tU(tmp);
  if (tU==Unit("K") || tU==Unit("Jy")) {
     table_.rwKeywordSet().define(String("FluxUnit"), tmp);
  } else {
     throw AipsError("Illegal unit - must be compatible with Jy or K");
  }
}

void Scantable::setInstrument(const std::string& name)
{
  bool throwIt = true;
  // create an Instrument to see if this is valid
  STAttr::convertInstrument(name, throwIt);
  String nameU(name);
  nameU.upcase();
  table_.rwKeywordSet().define(String("AntennaName"), nameU);
}

void Scantable::setFeedType(const std::string& feedtype)
{
  if ( Scantable::factories_.find(feedtype) ==  Scantable::factories_.end() ) {
    std::string msg = "Illegal feed type "+ feedtype;
    throw(casa::AipsError(msg));
  }
  table_.rwKeywordSet().define(String("POLTYPE"), feedtype);
}

MPosition Scantable::getAntennaPosition() const
{
  Vector<Double> antpos;
  table_.keywordSet().get("AntennaPosition", antpos);
  MVPosition mvpos(antpos(0),antpos(1),antpos(2));
  return MPosition(mvpos);
}

void Scantable::makePersistent(const std::string& filename)
{
  String inname(filename);
  Path path(inname);
  /// @todo reindex SCANNO, recompute nbeam, nif, npol
  inname = path.expandedName();
  // WORKAROUND !!! for Table bug
  // Remove when fixed in casacore
  if ( table_.tableType() == Table::Memory  && !selector_.empty() ) {
    Table tab = table_.copyToMemoryTable(generateName());
    tab.deepCopy(inname, Table::New);
    tab.markForDelete();

  } else {
    table_.deepCopy(inname, Table::New);
  }
}

int Scantable::nbeam( int scanno ) const
{
  if ( scanno < 0 ) {
    Int n;
    table_.keywordSet().get("nBeam",n);
    return int(n);
  } else {
    // take the first POLNO,IFNO,CYCLENO as nbeam shouldn't vary with these
    Table t = table_(table_.col("SCANNO") == scanno);
    ROTableRow row(t);
    const TableRecord& rec = row.get(0);
    Table subt = t( t.col("IFNO") == Int(rec.asuInt("IFNO"))
                    && t.col("POLNO") == Int(rec.asuInt("POLNO"))
                    && t.col("CYCLENO") == Int(rec.asuInt("CYCLENO")) );
    ROTableVector<uInt> v(subt, "BEAMNO");
    return int(v.nelements());
  }
  return 0;
}

int Scantable::nif( int scanno ) const
{
  if ( scanno < 0 ) {
    Int n;
    table_.keywordSet().get("nIF",n);
    return int(n);
  } else {
    // take the first POLNO,BEAMNO,CYCLENO as nbeam shouldn't vary with these
    Table t = table_(table_.col("SCANNO") == scanno);
    ROTableRow row(t);
    const TableRecord& rec = row.get(0);
    Table subt = t( t.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
                    && t.col("POLNO") == Int(rec.asuInt("POLNO"))
                    && t.col("CYCLENO") == Int(rec.asuInt("CYCLENO")) );
    if ( subt.nrow() == 0 ) return 0;
    ROTableVector<uInt> v(subt, "IFNO");
    return int(v.nelements());
  }
  return 0;
}

int Scantable::npol( int scanno ) const
{
  if ( scanno < 0 ) {
    Int n;
    table_.keywordSet().get("nPol",n);
    return n;
  } else {
    // take the first POLNO,IFNO,CYCLENO as nbeam shouldn't vary with these
    Table t = table_(table_.col("SCANNO") == scanno);
    ROTableRow row(t);
    const TableRecord& rec = row.get(0);
    Table subt = t( t.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
                    && t.col("IFNO") == Int(rec.asuInt("IFNO"))
                    && t.col("CYCLENO") == Int(rec.asuInt("CYCLENO")) );
    if ( subt.nrow() == 0 ) return 0;
    ROTableVector<uInt> v(subt, "POLNO");
    return int(v.nelements());
  }
  return 0;
}

int Scantable::ncycle( int scanno ) const
{
  if ( scanno < 0 ) {
    Block<String> cols(2);
    cols[0] = "SCANNO";
    cols[1] = "CYCLENO";
    TableIterator it(table_, cols);
    int n = 0;
    while ( !it.pastEnd() ) {
      ++n;
      ++it;
    }
    return n;
  } else {
    Table t = table_(table_.col("SCANNO") == scanno);
    ROTableRow row(t);
    const TableRecord& rec = row.get(0);
    Table subt = t( t.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
                    && t.col("POLNO") == Int(rec.asuInt("POLNO"))
                    && t.col("IFNO") == Int(rec.asuInt("IFNO")) );
    if ( subt.nrow() == 0 ) return 0;
    return int(subt.nrow());
  }
  return 0;
}


int Scantable::nrow( int scanno ) const
{
  return int(table_.nrow());
}

int Scantable::nchan( int ifno ) const
{
  if ( ifno < 0 ) {
    Int n;
    table_.keywordSet().get("nChan",n);
    return int(n);
  } else {
    // take the first SCANNO,POLNO,BEAMNO,CYCLENO as nbeam shouldn't
    // vary with these
    Table t = table_(table_.col("IFNO") == ifno);
    if ( t.nrow() == 0 ) return 0;
    ROArrayColumn<Float> v(t, "SPECTRA");
    return v.shape(0)(0);
  }
  return 0;
}

int Scantable::nscan() const {
  Vector<uInt> scannos(scanCol_.getColumn());
  uInt nout = genSort( scannos, Sort::Ascending,
                       Sort::QuickSort|Sort::NoDuplicates );
  return int(nout);
}

int Scantable::getChannels(int whichrow) const
{
  return specCol_.shape(whichrow)(0);
}

int Scantable::getBeam(int whichrow) const
{
  return beamCol_(whichrow);
}

std::vector<uint> Scantable::getNumbers(const ScalarColumn<uInt>& col) const
{
  Vector<uInt> nos(col.getColumn());
  uInt n = genSort( nos, Sort::Ascending, Sort::QuickSort|Sort::NoDuplicates );
  nos.resize(n, True);
  std::vector<uint> stlout;
  nos.tovector(stlout);
  return stlout;
}

int Scantable::getIF(int whichrow) const
{
  return ifCol_(whichrow);
}

int Scantable::getPol(int whichrow) const
{
  return polCol_(whichrow);
}

std::string Scantable::formatTime(const MEpoch& me, bool showdate) const
{
  MVTime mvt(me.getValue());
  if (showdate)
    mvt.setFormat(MVTime::YMD);
  else
    mvt.setFormat(MVTime::TIME);
  ostringstream oss;
  oss << mvt;
  return String(oss);
}

void Scantable::calculateAZEL()
{
  MPosition mp = getAntennaPosition();
  MEpoch::ROScalarColumn timeCol(table_, "TIME");
  ostringstream oss;
  oss << "Computed azimuth/elevation using " << endl
      << mp << endl;
  for (Int i=0; i<nrow(); ++i) {
    MEpoch me = timeCol(i);
    MDirection md = getDirection(i);
    oss  << " Time: " << formatTime(me,False) << " Direction: " << formatDirection(md)
         << endl << "     => ";
    MeasFrame frame(mp, me);
    Vector<Double> azel =
        MDirection::Convert(md, MDirection::Ref(MDirection::AZEL,
                                                frame)
                            )().getAngle("rad").getValue();
    azCol_.put(i,Float(azel[0]));
    elCol_.put(i,Float(azel[1]));
    oss << "azel: " << azel[0]/C::pi*180.0 << " "
        << azel[1]/C::pi*180.0 << " (deg)" << endl;
  }
  pushLog(String(oss));
}

void Scantable::clip(const Float uthres, const Float dthres, bool clipoutside, bool unflag)
{
  for (uInt i=0; i<table_.nrow(); ++i) {
    Vector<uChar> flgs = flagsCol_(i);
    srchChannelsToClip(i, uthres, dthres, clipoutside, unflag, flgs);
    flagsCol_.put(i, flgs);
  }
}

std::vector<bool> Scantable::getClipMask(int whichrow, const Float uthres, const Float dthres, bool clipoutside, bool unflag)
{
  Vector<uChar> flags;
  flagsCol_.get(uInt(whichrow), flags);
  srchChannelsToClip(uInt(whichrow), uthres, dthres, clipoutside, unflag, flags);
  Vector<Bool> bflag(flags.shape());
  convertArray(bflag, flags);
  //bflag = !bflag;

  std::vector<bool> mask;
  bflag.tovector(mask);
  return mask;
}

void Scantable::srchChannelsToClip(uInt whichrow, const Float uthres, const Float dthres, bool clipoutside, bool unflag,
				   Vector<uChar> flgs)
{
    Vector<Float> spcs = specCol_(whichrow);
    uInt nchannel = nchan();
    if (spcs.nelements() != nchannel) {
      throw(AipsError("Data has incorrect number of channels"));
    }
    uChar userflag = 1 << 7;
    if (unflag) {
      userflag = 0 << 7;
    }
    if (clipoutside) {
      for (uInt j = 0; j < nchannel; ++j) {
        Float spc = spcs(j);
        if ((spc >= uthres) || (spc <= dthres)) {
	  flgs(j) = userflag;
	}
      }
    } else {
      for (uInt j = 0; j < nchannel; ++j) {
        Float spc = spcs(j);
        if ((spc < uthres) && (spc > dthres)) {
	  flgs(j) = userflag;
	}
      }
    }
}

void Scantable::flag(const std::vector<bool>& msk, bool unflag)
{
  std::vector<bool>::const_iterator it;
  uInt ntrue = 0;
  for (it = msk.begin(); it != msk.end(); ++it) {
    if ( *it ) {
      ntrue++;
    }
  }
  if ( selector_.empty()  && (msk.size() == 0 || msk.size() == ntrue) )
    throw(AipsError("Trying to flag whole scantable."));
  if ( msk.size() == 0 ) {
    uChar userflag = 1 << 7;
    if ( unflag ) {
      userflag = 0 << 7;
    }
    for ( uInt i=0; i<table_.nrow(); ++i) {
      Vector<uChar> flgs = flagsCol_(i);
      flgs = userflag;
      flagsCol_.put(i, flgs);
    }
    return;
  }
  if ( int(msk.size()) != nchan() ) {
    throw(AipsError("Mask has incorrect number of channels."));
  }
  for ( uInt i=0; i<table_.nrow(); ++i) {
    Vector<uChar> flgs = flagsCol_(i);
    if ( flgs.nelements() != msk.size() ) {
      throw(AipsError("Mask has incorrect number of channels."
                      " Probably varying with IF. Please flag per IF"));
    }
    std::vector<bool>::const_iterator it;
    uInt j = 0;
    uChar userflag = 1 << 7;
    if ( unflag ) {
      userflag = 0 << 7;
    }
    for (it = msk.begin(); it != msk.end(); ++it) {
      if ( *it ) {
        flgs(j) = userflag;
      }
      ++j;
    }
    flagsCol_.put(i, flgs);
  }
}

void Scantable::flagRow(const std::vector<uInt>& rows, bool unflag)
{
  if ( selector_.empty() && (rows.size() == table_.nrow()) )
    throw(AipsError("Trying to flag whole scantable."));

  uInt rowflag = (unflag ? 0 : 1);
  std::vector<uInt>::const_iterator it;
  for (it = rows.begin(); it != rows.end(); ++it)
    flagrowCol_.put(*it, rowflag);
}

std::vector<bool> Scantable::getMask(int whichrow) const
{
  Vector<uChar> flags;
  flagsCol_.get(uInt(whichrow), flags);
  Vector<Bool> bflag(flags.shape());
  convertArray(bflag, flags);
  bflag = !bflag;
  std::vector<bool> mask;
  bflag.tovector(mask);
  return mask;
}

std::vector<float> Scantable::getSpectrum( int whichrow,
                                           const std::string& poltype ) const
{
  String ptype = poltype;
  if (poltype == "" ) ptype = getPolType();
  if ( whichrow  < 0 || whichrow >= nrow() )
    throw(AipsError("Illegal row number."));
  std::vector<float> out;
  Vector<Float> arr;
  uInt requestedpol = polCol_(whichrow);
  String basetype = getPolType();
  if ( ptype == basetype ) {
    specCol_.get(whichrow, arr);
  } else {
    CountedPtr<STPol> stpol(STPol::getPolClass(Scantable::factories_,
                                               basetype));
    uInt row = uInt(whichrow);
    stpol->setSpectra(getPolMatrix(row));
    Float fang,fhand,parang;
    fang = focusTable_.getTotalAngle(mfocusidCol_(row));
    fhand = focusTable_.getFeedHand(mfocusidCol_(row));
    stpol->setPhaseCorrections(fang, fhand);
    arr = stpol->getSpectrum(requestedpol, ptype);
  }
  if ( arr.nelements() == 0 )
    pushLog("Not enough polarisations present to do the conversion.");
  arr.tovector(out);
  return out;
}

void Scantable::setSpectrum( const std::vector<float>& spec,
                                   int whichrow )
{
  Vector<Float> spectrum(spec);
  Vector<Float> arr;
  specCol_.get(whichrow, arr);
  if ( spectrum.nelements() != arr.nelements() )
    throw AipsError("The spectrum has incorrect number of channels.");
  specCol_.put(whichrow, spectrum);
}


String Scantable::generateName()
{
  return (File::newUniqueName("./","temp")).baseName();
}

const casa::Table& Scantable::table( ) const
{
  return table_;
}

casa::Table& Scantable::table( )
{
  return table_;
}

std::string Scantable::getPolType() const
{
  return table_.keywordSet().asString("POLTYPE");
}

void Scantable::unsetSelection()
{
  table_ = originalTable_;
  attach();
  selector_.reset();
}

void Scantable::setSelection( const STSelector& selection )
{
  Table tab = const_cast<STSelector&>(selection).apply(originalTable_);
  if ( tab.nrow() == 0 ) {
    throw(AipsError("Selection contains no data. Not applying it."));
  }
  table_ = tab;
  attach();
  selector_ = selection;
}

std::string Scantable::summary( bool verbose )
{
  // Format header info
  ostringstream oss;
  oss << endl;
  oss << asap::SEPERATOR << endl;
  oss << " Scan Table Summary" << endl;
  oss << asap::SEPERATOR << endl;
  oss.flags(std::ios_base::left);
  oss << setw(15) << "Beams:" << setw(4) << nbeam() << endl
      << setw(15) << "IFs:" << setw(4) << nif() << endl
      << setw(15) << "Polarisations:" << setw(4) << npol()
      << "(" << getPolType() << ")" << endl
      << setw(15) << "Channels:" << nchan() << endl;
  String tmp;
  oss << setw(15) << "Observer:"
      << table_.keywordSet().asString("Observer") << endl;
  oss << setw(15) << "Obs Date:" << getTime(-1,true) << endl;
  table_.keywordSet().get("Project", tmp);
  oss << setw(15) << "Project:" << tmp << endl;
  table_.keywordSet().get("Obstype", tmp);
  oss << setw(15) << "Obs. Type:" << tmp << endl;
  table_.keywordSet().get("AntennaName", tmp);
  oss << setw(15) << "Antenna Name:" << tmp << endl;
  table_.keywordSet().get("FluxUnit", tmp);
  oss << setw(15) << "Flux Unit:" << tmp << endl;
  //Vector<Double> vec(moleculeTable_.getRestFrequencies());
  int nid = moleculeTable_.nrow();
  Bool firstline = True;
  oss << setw(15) << "Rest Freqs:";
  for (int i=0; i<nid; i++) {
      Table t = table_(table_.col("MOLECULE_ID") == i);
      if (t.nrow() >  0) {
          Vector<Double> vec(moleculeTable_.getRestFrequency(i));
          if (vec.nelements() > 0) {
               if (firstline) {
                   oss << setprecision(10) << vec << " [Hz]" << endl;
                   firstline=False;
               }
               else{
                   oss << setw(15)<<" " << setprecision(10) << vec << " [Hz]" << endl;
               }
          } else {
              oss << "none" << endl;
          }
      }
  }

  oss << setw(15) << "Abcissa:" << getAbcissaLabel(0) << endl;
  oss << selector_.print() << endl;
  oss << endl;
  // main table
  String dirtype = "Position ("
                  + getDirectionRefString()
                  + ")";
  oss << setw(5) << "Scan" << setw(15) << "Source"
      << setw(10) << "Time" << setw(18) << "Integration" << endl;
  oss << setw(5) << "" << setw(5) << "Beam" << setw(3) << "" << dirtype << endl;
  oss << setw(10) << "" << setw(3) << "IF" << setw(3) << ""
      << setw(8) << "Frame" << setw(16)
      << "RefVal" << setw(10) << "RefPix" << setw(12) << "Increment"
      << setw(7) << "Channels"
      << endl;
  oss << asap::SEPERATOR << endl;
  TableIterator iter(table_, "SCANNO");
  while (!iter.pastEnd()) {
    Table subt = iter.table();
    ROTableRow row(subt);
    MEpoch::ROScalarColumn timeCol(subt,"TIME");
    const TableRecord& rec = row.get(0);
    oss << setw(4) << std::right << rec.asuInt("SCANNO")
        << std::left << setw(1) << ""
        << setw(15) << rec.asString("SRCNAME")
        << setw(10) << formatTime(timeCol(0), false);
    // count the cycles in the scan
    TableIterator cyciter(subt, "CYCLENO");
    int nint = 0;
    while (!cyciter.pastEnd()) {
      ++nint;
      ++cyciter;
    }
    oss << setw(3) << std::right << nint  << setw(3) << " x " << std::left
        << setw(6) <<  formatSec(rec.asFloat("INTERVAL")) << endl;

    TableIterator biter(subt, "BEAMNO");
    while (!biter.pastEnd()) {
      Table bsubt = biter.table();
      ROTableRow brow(bsubt);
      const TableRecord& brec = brow.get(0);
      uInt row0 = bsubt.rowNumbers(table_)[0];
      oss << setw(5) << "" <<  setw(4) << std::right << brec.asuInt("BEAMNO")<< std::left;
      oss  << setw(4) << ""  << formatDirection(getDirection(row0)) << endl;
      TableIterator iiter(bsubt, "IFNO");
      while (!iiter.pastEnd()) {
        Table isubt = iiter.table();
        ROTableRow irow(isubt);
        const TableRecord& irec = irow.get(0);
        oss << setw(9) << "";
        oss << setw(3) << std::right << irec.asuInt("IFNO") << std::left
            << setw(1) << "" << frequencies().print(irec.asuInt("FREQ_ID"))
            << setw(3) << "" << nchan(irec.asuInt("IFNO"))
            << endl;

        ++iiter;
      }
      ++biter;
    }
    ++iter;
  }
  /// @todo implement verbose mode
  return String(oss);
}

std::string Scantable::getTime(int whichrow, bool showdate) const
{
  MEpoch::ROScalarColumn timeCol(table_, "TIME");
  MEpoch me;
  if (whichrow > -1) {
    me = timeCol(uInt(whichrow));
  } else {
    Double tm;
    table_.keywordSet().get("UTC",tm);
    me = MEpoch(MVEpoch(tm));
  }
  return formatTime(me, showdate);
}

MEpoch Scantable::getEpoch(int whichrow) const
{
  if (whichrow > -1) {
    return timeCol_(uInt(whichrow));
  } else {
    Double tm;
    table_.keywordSet().get("UTC",tm);
    return MEpoch(MVEpoch(tm));
  }
}

std::string Scantable::getDirectionString(int whichrow) const
{
  return formatDirection(getDirection(uInt(whichrow)));
}


SpectralCoordinate Scantable::getSpectralCoordinate(int whichrow) const {
  const MPosition& mp = getAntennaPosition();
  const MDirection& md = getDirection(whichrow);
  const MEpoch& me = timeCol_(whichrow);
  //Double rf = moleculeTable_.getRestFrequency(mmolidCol_(whichrow));
  Vector<Double> rf = moleculeTable_.getRestFrequency(mmolidCol_(whichrow));
  return freqTable_.getSpectralCoordinate(md, mp, me, rf,
                                          mfreqidCol_(whichrow));
}

std::vector< double > Scantable::getAbcissa( int whichrow ) const
{
  if ( whichrow > int(table_.nrow()) ) throw(AipsError("Illegal row number"));
  std::vector<double> stlout;
  int nchan = specCol_(whichrow).nelements();
  String us = freqTable_.getUnitString();
  if ( us == "" || us == "pixel" || us == "channel" ) {
    for (int i=0; i<nchan; ++i) {
      stlout.push_back(double(i));
    }
    return stlout;
  }
  SpectralCoordinate spc = getSpectralCoordinate(whichrow);
  Vector<Double> pixel(nchan);
  Vector<Double> world;
  indgen(pixel);
  if ( Unit(us) == Unit("Hz") ) {
    for ( int i=0; i < nchan; ++i) {
      Double world;
      spc.toWorld(world, pixel[i]);
      stlout.push_back(double(world));
    }
  } else if ( Unit(us) == Unit("km/s") ) {
    Vector<Double> world;
    spc.pixelToVelocity(world, pixel);
    world.tovector(stlout);
  }
  return stlout;
}
void Scantable::setDirectionRefString( const std::string & refstr )
{
  MDirection::Types mdt;
  if (refstr != "" && !MDirection::getType(mdt, refstr)) {
    throw(AipsError("Illegal Direction frame."));
  }
  if ( refstr == "" ) {
    String defaultstr = MDirection::showType(dirCol_.getMeasRef().getType());
    table_.rwKeywordSet().define("DIRECTIONREF", defaultstr);
  } else {
    table_.rwKeywordSet().define("DIRECTIONREF", String(refstr));
  }
}

std::string Scantable::getDirectionRefString( ) const
{
  return table_.keywordSet().asString("DIRECTIONREF");
}

MDirection Scantable::getDirection(int whichrow ) const
{
  String usertype = table_.keywordSet().asString("DIRECTIONREF");
  String type = MDirection::showType(dirCol_.getMeasRef().getType());
  if ( usertype != type ) {
    MDirection::Types mdt;
    if (!MDirection::getType(mdt, usertype)) {
      throw(AipsError("Illegal Direction frame."));
    }
    return dirCol_.convert(uInt(whichrow), mdt);
  } else {
    return dirCol_(uInt(whichrow));
  }
}

std::string Scantable::getAbcissaLabel( int whichrow ) const
{
  if ( whichrow > int(table_.nrow()) ) throw(AipsError("Illegal ro number"));
  const MPosition& mp = getAntennaPosition();
  const MDirection& md = getDirection(whichrow);
  const MEpoch& me = timeCol_(whichrow);
  //const Double& rf = mmolidCol_(whichrow);
  const Vector<Double> rf = moleculeTable_.getRestFrequency(mmolidCol_(whichrow));
  SpectralCoordinate spc =
    freqTable_.getSpectralCoordinate(md, mp, me, rf, mfreqidCol_(whichrow));

  String s = "Channel";
  Unit u = Unit(freqTable_.getUnitString());
  if (u == Unit("km/s")) {
    s = CoordinateUtil::axisLabel(spc, 0, True,True,  True);
  } else if (u == Unit("Hz")) {
    Vector<String> wau(1);wau = u.getName();
    spc.setWorldAxisUnits(wau);
    s = CoordinateUtil::axisLabel(spc, 0, True, True, False);
  }
  return s;

}

/**
void asap::Scantable::setRestFrequencies( double rf, const std::string& name,
                                          const std::string& unit )
**/
void Scantable::setRestFrequencies( vector<double> rf, const vector<std::string>& name,
                                          const std::string& unit )

{
  ///@todo lookup in line table to fill in name and formattedname
  Unit u(unit);
  //Quantum<Double> urf(rf, u);
  Quantum<Vector<Double> >urf(rf, u);
  Vector<String> formattedname(0);
  //cerr<<"Scantable::setRestFrequnecies="<<urf<<endl;

  //uInt id = moleculeTable_.addEntry(urf.getValue("Hz"), name, "");
  uInt id = moleculeTable_.addEntry(urf.getValue("Hz"), mathutil::toVectorString(name), formattedname);
  TableVector<uInt> tabvec(table_, "MOLECULE_ID");
  tabvec = id;
}

/**
void asap::Scantable::setRestFrequencies( const std::string& name )
{
  throw(AipsError("setRestFrequencies( const std::string& name ) NYI"));
  ///@todo implement
}
**/
void Scantable::setRestFrequencies( const vector<std::string>& name )
{
  throw(AipsError("setRestFrequencies( const vector<std::string>& name ) NYI"));
  ///@todo implement
}

std::vector< unsigned int > Scantable::rownumbers( ) const
{
  std::vector<unsigned int> stlout;
  Vector<uInt> vec = table_.rowNumbers();
  vec.tovector(stlout);
  return stlout;
}


Matrix<Float> Scantable::getPolMatrix( uInt whichrow ) const
{
  ROTableRow row(table_);
  const TableRecord& rec = row.get(whichrow);
  Table t =
    originalTable_( originalTable_.col("SCANNO") == Int(rec.asuInt("SCANNO"))
                    && originalTable_.col("BEAMNO") == Int(rec.asuInt("BEAMNO"))
                    && originalTable_.col("IFNO") == Int(rec.asuInt("IFNO"))
                    && originalTable_.col("CYCLENO") == Int(rec.asuInt("CYCLENO")) );
  ROArrayColumn<Float> speccol(t, "SPECTRA");
  return speccol.getColumn();
}

std::vector< std::string > Scantable::columnNames( ) const
{
  Vector<String> vec = table_.tableDesc().columnNames();
  return mathutil::tovectorstring(vec);
}

MEpoch::Types Scantable::getTimeReference( ) const
{
  return MEpoch::castType(timeCol_.getMeasRef().getType());
}

void Scantable::addFit( const STFitEntry& fit, int row )
{
  //cout << mfitidCol_(uInt(row)) << endl;
  LogIO os( LogOrigin( "Scantable", "addFit()", WHERE ) ) ;
  os << mfitidCol_(uInt(row)) << LogIO::POST ;
  uInt id = fitTable_.addEntry(fit, mfitidCol_(uInt(row)));
  mfitidCol_.put(uInt(row), id);
}

void Scantable::shift(int npix)
{
  Vector<uInt> fids(mfreqidCol_.getColumn());
  genSort( fids, Sort::Ascending,
	   Sort::QuickSort|Sort::NoDuplicates );
  for (uInt i=0; i<fids.nelements(); ++i) {
    frequencies().shiftRefPix(npix, fids[i]);
  }
}

String Scantable::getAntennaName() const
{
  String out;
  table_.keywordSet().get("AntennaName", out);
  return out;
}

int Scantable::checkScanInfo(const std::vector<int>& scanlist) const
{
  String tbpath;
  int ret = 0;
  if ( table_.keywordSet().isDefined("GBT_GO") ) {
    table_.keywordSet().get("GBT_GO", tbpath);
    Table t(tbpath,Table::Old);
    // check each scan if other scan of the pair exist
    int nscan = scanlist.size();
    for (int i = 0; i < nscan; i++) {
      Table subt = t( t.col("SCAN") == scanlist[i]+1 );
      if (subt.nrow()==0) {
        //cerr <<"Scan "<<scanlist[i]<<" cannot be found in the scantable."<<endl;
        LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
        os <<LogIO::WARN<<"Scan "<<scanlist[i]<<" cannot be found in the scantable."<<LogIO::POST;
        ret = 1;
        break;
      }
      ROTableRow row(subt);
      const TableRecord& rec = row.get(0);
      int scan1seqn = rec.asuInt("PROCSEQN");
      int laston1 = rec.asuInt("LASTON");
      if ( rec.asuInt("PROCSIZE")==2 ) {
        if ( i < nscan-1 ) {
          Table subt2 = t( t.col("SCAN") == scanlist[i+1]+1 );
          if ( subt2.nrow() == 0) {
            LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;

            //cerr<<"Scan "<<scanlist[i+1]<<" cannot be found in the scantable."<<endl;
            os<<LogIO::WARN<<"Scan "<<scanlist[i+1]<<" cannot be found in the scantable."<<LogIO::POST;
            ret = 1;
            break;
          }
          ROTableRow row2(subt2);
          const TableRecord& rec2 = row2.get(0);
          int scan2seqn = rec2.asuInt("PROCSEQN");
          int laston2 = rec2.asuInt("LASTON");
          if (scan1seqn == 1 && scan2seqn == 2) {
            if (laston1 == laston2) {
              LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
              //cerr<<"A valid scan pair ["<<scanlist[i]<<","<<scanlist[i+1]<<"]"<<endl;
              os<<"A valid scan pair ["<<scanlist[i]<<","<<scanlist[i+1]<<"]"<<LogIO::POST;
              i +=1;
            }
            else {
              LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
              //cerr<<"Incorrect scan pair ["<<scanlist[i]<<","<<scanlist[i+1]<<"]"<<endl;
              os<<LogIO::WARN<<"Incorrect scan pair ["<<scanlist[i]<<","<<scanlist[i+1]<<"]"<<LogIO::POST;
            }
          }
          else if (scan1seqn==2 && scan2seqn == 1) {
            if (laston1 == laston2) {
              LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
              //cerr<<"["<<scanlist[i]<<","<<scanlist[i+1]<<"] is a valid scan pair but in incorrect order."<<endl;
              os<<LogIO::WARN<<"["<<scanlist[i]<<","<<scanlist[i+1]<<"] is a valid scan pair but in incorrect order."<<LogIO::POST;
              ret = 1;
              break;
            }
          }
          else {
            LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
            //cerr<<"The other scan for  "<<scanlist[i]<<" appears to be missing. Check the input scan numbers."<<endl;
            os<<LogIO::WARN<<"The other scan for  "<<scanlist[i]<<" appears to be missing. Check the input scan numbers."<<LogIO::POST;
            ret = 1;
            break;
          }
        }
      }
      else {
        LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
        //cerr<<"The scan does not appear to be standard obsevation."<<endl;
        os<<LogIO::WARN<<"The scan does not appear to be standard obsevation."<<LogIO::POST;
      }
    //if ( i >= nscan ) break;
    }
  }
  else {
    LogIO os( LogOrigin( "Scantable", "checkScanInfo()", WHERE ) ) ;
    //cerr<<"No reference to GBT_GO table."<<endl;
    os<<LogIO::WARN<<"No reference to GBT_GO table."<<LogIO::POST;
    ret = 1;
  }
  return ret;
}

std::vector<double> Scantable::getDirectionVector(int whichrow) const
{
  Vector<Double> Dir = dirCol_(whichrow).getAngle("rad").getValue();
  std::vector<double> dir;
  Dir.tovector(dir);
  return dir;
}

void asap::Scantable::reshapeSpectrum( int nmin, int nmax )
  throw( casa::AipsError )
{
  // assumed that all rows have same nChan
  Vector<Float> arr = specCol_( 0 ) ;
  int nChan = arr.nelements() ;

  // if nmin < 0 or nmax < 0, nothing to do
  if (  nmin < 0 ) {
    throw( casa::indexError<int>( nmin, "asap::Scantable::reshapeSpectrum: Invalid range. Negative index is specified." ) ) ;
    }
  if (  nmax < 0  ) {
    throw( casa::indexError<int>( nmax, "asap::Scantable::reshapeSpectrum: Invalid range. Negative index is specified." ) ) ;
  }

  // if nmin > nmax, exchange values
  if ( nmin > nmax ) {
    int tmp = nmax ;
    nmax = nmin ;
    nmin = tmp ;
    LogIO os( LogOrigin( "Scantable", "reshapeSpectrum()", WHERE ) ) ;
    os << "Swap values. Applied range is ["
       << nmin << ", " << nmax << "]" << LogIO::POST ;
  }

  // if nmin exceeds nChan, nothing to do
  if ( nmin >= nChan ) {
    throw( casa::indexError<int>( nmin, "asap::Scantable::reshapeSpectrum: Invalid range. Specified minimum exceeds nChan." ) ) ;
  }

  // if nmax exceeds nChan, reset nmax to nChan
  if ( nmax >= nChan ) {
    if ( nmin == 0 ) {
      // nothing to do
      LogIO os( LogOrigin( "Scantable", "reshapeSpectrum()", WHERE ) ) ;
      os << "Whole range is selected. Nothing to do." << LogIO::POST ;
      return ;
    }
    else {
      LogIO os( LogOrigin( "Scantable", "reshapeSpectrum()", WHERE ) ) ;
      os << "Specified maximum exceeds nChan. Applied range is ["
         << nmin << ", " << nChan-1 << "]." << LogIO::POST ;
      nmax = nChan - 1 ;
    }
  }

  // reshape specCol_ and flagCol_
  for ( int irow = 0 ; irow < nrow() ; irow++ ) {
    reshapeSpectrum( nmin, nmax, irow ) ;
  }

  // update FREQUENCIES subtable
  Double refpix ;
  Double refval ;
  Double increment ;
  int freqnrow = freqTable_.table().nrow() ;
  Vector<uInt> oldId( freqnrow ) ;
  Vector<uInt> newId( freqnrow ) ;
  for ( int irow = 0 ; irow < freqnrow ; irow++ ) {
    freqTable_.getEntry( refpix, refval, increment, irow ) ;
    /***
     * need to shift refpix to nmin
     * note that channel nmin in old index will be channel 0 in new one
     ***/
    refval = refval - ( refpix - nmin ) * increment ;
    refpix = 0 ;
    freqTable_.setEntry( refpix, refval, increment, irow ) ;
  }

  // update nchan
  int newsize = nmax - nmin + 1 ;
  table_.rwKeywordSet().define( "nChan", newsize ) ;

  // update bandwidth
  // assumed all spectra in the scantable have same bandwidth
  table_.rwKeywordSet().define( "Bandwidth", increment * newsize ) ;

  return ;
}

void asap::Scantable::reshapeSpectrum( int nmin, int nmax, int irow )
{
  // reshape specCol_ and flagCol_
  Vector<Float> oldspec = specCol_( irow ) ;
  Vector<uChar> oldflag = flagsCol_( irow ) ;
  uInt newsize = nmax - nmin + 1 ;
  specCol_.put( irow, oldspec( Slice( nmin, newsize, 1 ) ) ) ;
  flagsCol_.put( irow, oldflag( Slice( nmin, newsize, 1 ) ) ) ;

  return ;
}

void asap::Scantable::regridChannel( int nChan, double dnu )
{
  LogIO os( LogOrigin( "Scantable", "regridChannel()", WHERE ) ) ;
  os << "Regrid abcissa with channel number " << nChan << " and spectral resoultion " << dnu << "Hz." << LogIO::POST ;
  // assumed that all rows have same nChan
  Vector<Float> arr = specCol_( 0 ) ;
  int oldsize = arr.nelements() ;

  // if oldsize == nChan, nothing to do
  if ( oldsize == nChan ) {
    os << "Specified channel number is same as current one. Nothing to do." << LogIO::POST ;
    return ;
  }

  // if oldChan < nChan, unphysical operation
  if ( oldsize < nChan ) {
    os << "Unphysical operation. Nothing to do." << LogIO::POST ;
    return ;
  }

  // change channel number for specCol_ and flagCol_
  Vector<Float> newspec( nChan, 0 ) ;
  Vector<uChar> newflag( nChan, false ) ;
  vector<string> coordinfo = getCoordInfo() ;
  string oldinfo = coordinfo[0] ;
  coordinfo[0] = "Hz" ;
  setCoordInfo( coordinfo ) ;
  for ( int irow = 0 ; irow < nrow() ; irow++ ) {
    regridChannel( nChan, dnu, irow ) ;
  }
  coordinfo[0] = oldinfo ;
  setCoordInfo( coordinfo ) ;


  // NOTE: this method does not update metadata such as
  //       FREQUENCIES subtable, nChan, Bandwidth, etc.

  return ;
}

void asap::Scantable::regridChannel( int nChan, double dnu, int irow )
{
  // logging
  //ofstream ofs( "average.log", std::ios::out | std::ios::app ) ;
  //ofs << "IFNO = " << getIF( irow ) << " irow = " << irow << endl ;

  Vector<Float> oldspec = specCol_( irow ) ;
  Vector<uChar> oldflag = flagsCol_( irow ) ;
  Vector<Float> newspec( nChan, 0 ) ;
  Vector<uChar> newflag( nChan, false ) ;

  // regrid
  vector<double> abcissa = getAbcissa( irow ) ;
  int oldsize = abcissa.size() ;
  double olddnu = abcissa[1] - abcissa[0] ;
  //int refChan = 0 ;
  //double frac = 0.0 ;
  //double wedge = 0.0 ;
  //double pile = 0.0 ;
  int ichan = 0 ;
  double wsum = 0.0 ;
  Vector<Float> z( nChan ) ;
  z[0] = abcissa[0] - 0.5 * olddnu + 0.5 * dnu ;
  for ( int ii = 1 ; ii < nChan ; ii++ )
    z[ii] = z[ii-1] + dnu ;
  Vector<Float> zi( nChan+1 ) ;
  Vector<Float> yi( oldsize + 1 ) ;
  zi[0] = z[0] - 0.5 * dnu ;
  zi[1] = z[0] + 0.5 * dnu ;
  for ( int ii = 2 ; ii < nChan ; ii++ )
    zi[ii] = zi[ii-1] + dnu ;
  zi[nChan] = z[nChan-1] + 0.5 * dnu ;
  yi[0] = abcissa[0] - 0.5 * olddnu ;
  yi[1] = abcissa[1] + 0.5 * olddnu ;
  for ( int ii = 2 ; ii < oldsize ; ii++ )
    yi[ii] = abcissa[ii-1] + olddnu ;
  yi[oldsize] = abcissa[oldsize-1] + 0.5 * olddnu ;
  if ( dnu > 0.0 ) {
    for ( int ii = 0 ; ii < nChan ; ii++ ) {
      double zl = zi[ii] ;
      double zr = zi[ii+1] ;
      for ( int j = ichan ; j < oldsize ; j++ ) {
        double yl = yi[j] ;
        double yr = yi[j+1] ;
        if ( yl <= zl ) {
          if ( yr <= zl ) {
            continue ;
          }
          else if ( yr <= zr ) {
            newspec[ii] += oldspec[j] * ( yr - zl ) ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += ( yr - zl ) ;
          }
          else {
            newspec[ii] += oldspec[j] * dnu ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += dnu ;
            ichan = j ;
            break ;
          }
        }
        else if ( yl < zr ) {
          if ( yr <= zr ) {
              newspec[ii] += oldspec[j] * ( yr - yl ) ;
              newflag[ii] = newflag[ii] || oldflag[j] ;
              wsum += ( yr - yl ) ;
          }
          else {
            newspec[ii] += oldspec[j] * ( zr - yl ) ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += ( zr - yl ) ;
            ichan = j ;
            break ;
          }
        }
        else {
          ichan = j - 1 ;
          break ;
        }
      }
      newspec[ii] /= wsum ;
      wsum = 0.0 ;
    }
  }
  else if ( dnu < 0.0 ) {
    for ( int ii = 0 ; ii < nChan ; ii++ ) {
      double zl = zi[ii] ;
      double zr = zi[ii+1] ;
      for ( int j = ichan ; j < oldsize ; j++ ) {
        double yl = yi[j] ;
        double yr = yi[j+1] ;
        if ( yl >= zl ) {
          if ( yr >= zl ) {
            continue ;
          }
          else if ( yr >= zr ) {
            newspec[ii] += oldspec[j] * abs( yr - zl ) ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += abs( yr - zl ) ;
          }
          else {
            newspec[ii] += oldspec[j] * abs( dnu ) ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += abs( dnu ) ;
            ichan = j ;
            break ;
          }
        }
        else if ( yl > zr ) {
          if ( yr >= zr ) {
            newspec[ii] += oldspec[j] * abs( yr - yl ) ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += abs( yr - yl ) ;
          }
          else {
            newspec[ii] += oldspec[j] * abs( zr - yl ) ;
            newflag[ii] = newflag[ii] || oldflag[j] ;
            wsum += abs( zr - yl ) ;
            ichan = j ;
            break ;
          }
        }
        else {
          ichan = j - 1 ;
          break ;
        }
      }
      newspec[ii] /= wsum ;
      wsum = 0.0 ;
    }
  }
//    * ichan = 0
//    ***/
//   //ofs << "olddnu = " << olddnu << ", dnu = " << dnu << endl ;
//   pile += dnu ;
//   wedge = olddnu * ( refChan + 1 ) ;
//   while ( wedge < pile ) {
//     newspec[0] += olddnu * oldspec[refChan] ;
//     newflag[0] = newflag[0] || oldflag[refChan] ;
//     //ofs << "channel " << refChan << " is included in new channel 0" << endl ;
//     refChan++ ;
//     wedge += olddnu ;
//     wsum += olddnu ;
//     //ofs << "newspec[0] = " << newspec[0] << " wsum = " << wsum << endl ;
//   }
//   frac = ( wedge - pile ) / olddnu ;
//   wsum += ( 1.0 - frac ) * olddnu ;
//   newspec[0] += ( 1.0 - frac ) * olddnu * oldspec[refChan] ;
//   newflag[0] = newflag[0] || oldflag[refChan] ;
//   //ofs << "channel " << refChan << " is partly included in new channel 0" << " with fraction of " << ( 1.0 - frac ) << endl ;
//   //ofs << "newspec[0] = " << newspec[0] << " wsum = " << wsum << endl ;
//   newspec[0] /= wsum ;
//   //ofs << "newspec[0] = " << newspec[0] << endl ;
//   //ofs << "wedge = " << wedge << ", pile = " << pile << endl ;

//   /***
//    * ichan = 1 - nChan-2
//    ***/
//   for ( int ichan = 1 ; ichan < nChan - 1 ; ichan++ ) {
//     pile += dnu ;
//     newspec[ichan] += frac * olddnu * oldspec[refChan] ;
//     newflag[ichan] = newflag[ichan] || oldflag[refChan] ;
//     //ofs << "channel " << refChan << " is partly included in new channel " << ichan << " with fraction of " << frac << endl ;
//     refChan++ ;
//     wedge += olddnu ;
//     wsum = frac * olddnu ;
//     //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << " wsum = " << wsum << endl ;
//     while ( wedge < pile ) {
//       newspec[ichan] += olddnu * oldspec[refChan] ;
//       newflag[ichan] = newflag[ichan] || oldflag[refChan] ;
//       //ofs << "channel " << refChan << " is included in new channel " << ichan << endl ;
//       refChan++ ;
//       wedge += olddnu ;
//       wsum += olddnu ;
//       //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << " wsum = " << wsum << endl ;
//     }
//     frac = ( wedge - pile ) / olddnu ;
//     wsum += ( 1.0 - frac ) * olddnu ;
//     newspec[ichan] += ( 1.0 - frac ) * olddnu * oldspec[refChan] ;
//     newflag[ichan] = newflag[ichan] || oldflag[refChan] ;
//     //ofs << "channel " << refChan << " is partly included in new channel " << ichan << " with fraction of " << ( 1.0 - frac ) << endl ;
//     //ofs << "wedge = " << wedge << ", pile = " << pile << endl ;
//     //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << " wsum = " << wsum << endl ;
//     newspec[ichan] /= wsum ;
//     //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << endl ;
//   }

//   /***
//    * ichan = nChan-1
//    ***/
//   // NOTE: Assumed that all spectra have the same bandwidth
//   pile += dnu ;
//   newspec[nChan-1] += frac * olddnu * oldspec[refChan] ;
//   newflag[nChan-1] = newflag[nChan-1] || oldflag[refChan] ;
//   //ofs << "channel " << refChan << " is partly included in new channel " << nChan-1 << " with fraction of " << frac << endl ;
//   refChan++ ;
//   wedge += olddnu ;
//   wsum = frac * olddnu ;
//   //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << " wsum = " << wsum << endl ;
//   for ( int jchan = refChan ; jchan < oldsize ; jchan++ ) {
//     newspec[nChan-1] += olddnu * oldspec[jchan] ;
//     newflag[nChan-1] = newflag[nChan-1] || oldflag[jchan] ;
//     wsum += olddnu ;
//     //ofs << "channel " << jchan << " is included in new channel " << nChan-1 << " with fraction of " << frac << endl ;
//     //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << " wsum = " << wsum << endl ;
//   }
//   //ofs << "wedge = " << wedge << ", pile = " << pile << endl ;
//   //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << " wsum = " << wsum << endl ;
//   newspec[nChan-1] /= wsum ;
//   //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << endl ;

//   specCol_.put( irow, newspec ) ;
//   flagsCol_.put( irow, newflag ) ;

//   // ofs.close() ;


  return ;
}

std::vector<float> Scantable::getWeather(int whichrow) const
{
  std::vector<float> out(5);
  //Float temperature, pressure, humidity, windspeed, windaz;
  weatherTable_.getEntry(out[0], out[1], out[2], out[3], out[4],
                         mweatheridCol_(uInt(whichrow)));


  return out;
}

}
//namespace asap

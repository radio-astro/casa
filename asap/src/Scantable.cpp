//
// C++ Implementation: Scantable
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2005-2013
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <map>
#include <sys/time.h>

#include <atnf/PKSIO/SrcType.h>

#include <casa/aips.h>
#include <casa/iomanip.h>
#include <casa/iostream.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <casa/Logging/LogIO.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayAccessor.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MaskArrMath.h>
#include <casa/Arrays/Slice.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/VectorSTLIterator.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Containers/RecordField.h>
#include <casa/Logging/LogIO.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Utilities/GenSort.h>

#include <coordinates/Coordinates/CoordinateUtil.h>

// needed to avoid error in .tcc
#include <measures/Measures/MCDirection.h>
//
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MeasRef.h>
#include <measures/Measures/MeasTable.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/TableMeasDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasValueDesc.h>

#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableCopy.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableVector.h>

#include "MathUtils.h"
#include "STAttr.h"
#include "STBaselineTable.h"
#include "STLineFinder.h"
#include "STPolCircular.h"
#include "STPolLinear.h"
#include "STPolStokes.h"
#include "STUpgrade.h"
#include "STFitter.h"
#include "Scantable.h"

#define debug 1

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
  table_.tableInfo().setType( "Scantable" ) ;
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
      STUpgrade upgrader(version_);
      LogIO os( LogOrigin( "Scantable" ) ) ;
      os << LogIO::WARN
         << name << " data format version " << version 
	 << " is deprecated" << endl 
         << "Running upgrade."<< endl  
         << LogIO::POST ;  
      std::string outname = upgrader.upgrade(name);
      if ( outname != name ) {
        os << LogIO::WARN
           << "Data will be loaded from " << outname << " instead of " 
           << name << LogIO::POST ;
        tab = Table(outname, Table::Update ) ;
      }
  }
  if ( type_ == Table::Memory ) {
    table_ = tab.copyToMemoryTable(generateName());
  } else {
    table_ = tab;
  }
  table_.tableInfo().setType( "Scantable" ) ;

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
      } else {
        table_ = other.table_.copyToMemoryTable(newname);
      }
  } else {
      other.table_.deepCopy(newname, Table::New, False,
                            other.table_.endianFormat(),
                            Bool(clear));
      table_ = Table(newname, Table::Update);
      table_.markForDelete();
  }
  table_.tableInfo().setType( "Scantable" ) ;
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

void Scantable::setSourceName( const std::string& name )
{
  TableVector<String> tabvec(table_, "SRCNAME");
  tabvec = name;
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

  std::string Scantable::formatDirection(const MDirection& md, Int prec) const
{
  Vector<Double> t = md.getAngle(Unit(String("rad"))).getValue();
  if (prec<0)
    prec = 7;

  String ref = md.getRefString();
  MVAngle mvLon(t[0]);
  String sLon = mvLon.string(MVAngle::TIME,prec);
  uInt tp = md.getRef().getType();
  if (tp == MDirection::GALACTIC ||
      tp == MDirection::SUPERGAL ) {
    sLon = mvLon(0.0).string(MVAngle::ANGLE_CLEAN,prec);
  }
  MVAngle mvLat(t[1]);
  String sLat = mvLat.string(MVAngle::ANGLE+MVAngle::DIG2,prec);
  return  ref + String(" ") + sLon + String(" ") + sLat;
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
  // 2011/03/04 TN
  // We can comment out this workaround since the essential bug is 
  // fixed in casacore (r20889 in google code).
  table_.deepCopy(inname, Table::New);
//   // WORKAROUND !!! for Table bug
//   // Remove when fixed in casacore
//   if ( table_.tableType() == Table::Memory  && !selector_.empty() ) {
//     Table tab = table_.copyToMemoryTable(generateName());
//     tab.deepCopy(inname, Table::New);
//     tab.markForDelete();
//
//   } else {
//     table_.deepCopy(inname, Table::New);
//   }
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
    Table t = table_(table_.col("IFNO") == ifno, 1);
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
  return formatTime(me, showdate, 0);
}

std::string Scantable::formatTime(const MEpoch& me, bool showdate, uInt prec) const
{
  MVTime mvt(me.getValue());
  if (showdate)
    //mvt.setFormat(MVTime::YMD);
    mvt.setFormat(MVTime::YMD, prec);
  else
    //mvt.setFormat(MVTime::TIME);
    mvt.setFormat(MVTime::TIME, prec);
  ostringstream oss;
  oss << mvt;
  return String(oss);
}

void Scantable::calculateAZEL()
{  
  LogIO os( LogOrigin( "Scantable", "calculateAZEL()", WHERE ) ) ;
  MPosition mp = getAntennaPosition();
  MEpoch::ROScalarColumn timeCol(table_, "TIME");
  ostringstream oss;
  oss << mp;
  os << "Computed azimuth/elevation using " << endl
     << String(oss) << endl;
  for (Int i=0; i<nrow(); ++i) {
    MEpoch me = timeCol(i);
    MDirection md = getDirection(i);
    os  << " Time: " << formatTime(me,False) 
	<< " Direction: " << formatDirection(md)
         << endl << "     => ";
    MeasFrame frame(mp, me);
    Vector<Double> azel =
        MDirection::Convert(md, MDirection::Ref(MDirection::AZEL,
                                                frame)
                            )().getAngle("rad").getValue();
    azCol_.put(i,Float(azel[0]));
    elCol_.put(i,Float(azel[1]));
    os << "azel: " << azel[0]/C::pi*180.0 << " "
       << azel[1]/C::pi*180.0 << " (deg)" << LogIO::POST;
  }
}

void Scantable::clip(const Float uthres, const Float dthres, bool clipoutside, bool unflag)
{
  Vector<uInt> flagrow = flagrowCol_.getColumn();
  for (uInt i=0; i<table_.nrow(); ++i) {
    // apply flag only when specified row is vaild
    if (flagrow[i] == 0) {
      Vector<uChar> flgs = flagsCol_(i);
      srchChannelsToClip(i, uthres, dthres, clipoutside, unflag, flgs);
      flagsCol_.put(i, flgs);
    }
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
    uInt nchannel = spcs.nelements();
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


void Scantable::flag( int whichrow, const std::vector<bool>& msk, bool unflag ) {
  std::vector<bool>::const_iterator it;
  uInt ntrue = 0;
  if (whichrow >= int(table_.nrow()) ) {
    throw(AipsError("Invalid row number"));
  }
  for (it = msk.begin(); it != msk.end(); ++it) {
    if ( *it ) {
      ntrue++;
    }
  }
  //if ( selector_.empty()  && (msk.size() == 0 || msk.size() == ntrue) )
  if ( whichrow == -1 && !unflag && selector_.empty() && (msk.size() == 0 || msk.size() == ntrue) )
    throw(AipsError("Trying to flag whole scantable."));
  uChar userflag = 1 << 7;
  if ( unflag ) {
    userflag = 0 << 7;
  }
  if (whichrow > -1 ) {
    // apply flag only when specified row is vaild
    if (flagrowCol_(whichrow) == 0) {
      applyChanFlag(uInt(whichrow), msk, userflag);
    }
  } else {
    Vector<uInt> flagrow = flagrowCol_.getColumn();
    for ( uInt i=0; i<table_.nrow(); ++i) {
      // apply flag only when specified row is vaild
      if (flagrow[i] == 0) {
        applyChanFlag(i, msk, userflag);
      }
    }
  }
}

void Scantable::applyChanFlag( uInt whichrow, const std::vector<bool>& msk, uChar flagval )
{
  if (whichrow >= table_.nrow() ) {
    throw( casa::indexError<int>( whichrow, "asap::Scantable::applyChanFlag: Invalid row number" ) );
  }
  Vector<uChar> flgs = flagsCol_(whichrow);
  if ( msk.size() == 0 ) {
    flgs = flagval;
    flagsCol_.put(whichrow, flgs);
    return;
  }
  if ( int(msk.size()) != nchan( getIF(whichrow) ) ) {
    throw(AipsError("Mask has incorrect number of channels."));
  }
  if ( flgs.nelements() != msk.size() ) {
    throw(AipsError("Mask has incorrect number of channels."
		    " Probably varying with IF. Please flag per IF"));
  }
  std::vector<bool>::const_iterator it;
  uInt j = 0;
  for (it = msk.begin(); it != msk.end(); ++it) {
    if ( *it ) {
      flgs(j) = flagval;
    }
    ++j;
  }
  flagsCol_.put(whichrow, flgs);
}

void Scantable::flagRow(const std::vector<uInt>& rows, bool unflag)
{
  if (selector_.empty() && (rows.size() == table_.nrow()) && !unflag)
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
  LogIO os( LogOrigin( "Scantable", "getSpectrum()", WHERE ) ) ;

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
    Float fang,fhand;
    fang = focusTable_.getTotalAngle(mfocusidCol_(row));
    fhand = focusTable_.getFeedHand(mfocusidCol_(row));
    stpol->setPhaseCorrections(fang, fhand);
    arr = stpol->getSpectrum(requestedpol, ptype);
  }
  if ( arr.nelements() == 0 )
    
    os << "Not enough polarisations present to do the conversion." 
       << LogIO::POST;
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
//   tab.rwKeywordSet().define("nBeam",(Int)(getBeamNos().size())) ;
//   vector<uint> selectedIFs = getIFNos() ;
//   Int newnIF = selectedIFs.size() ;
//   tab.rwKeywordSet().define("nIF",newnIF) ;
//   if ( newnIF != 0 ) {
//     Int newnChan = 0 ;
//     for ( Int i = 0 ; i < newnIF ; i++ ) {
//       Int nChan = nchan( selectedIFs[i] ) ;
//       if ( newnChan > nChan )
//         newnChan = nChan ;
//     }
//     tab.rwKeywordSet().define("nChan",newnChan) ;
//   }
//   tab.rwKeywordSet().define("nPol",(Int)(getPolNos().size())) ;
  selector_ = selection;
}


std::string Scantable::headerSummary()
{
  // Format header info
//   STHeader sdh;
//   sdh = getHeader();
//   sdh.print();
  ostringstream oss;
  oss.flags(std::ios_base::left);
  String tmp;
  // Project
  table_.keywordSet().get("Project", tmp);
  oss << setw(15) << "Project:" << tmp << endl;
  // Observation date
  oss << setw(15) << "Obs Date:" << getTime(-1,true) << endl;
  // Observer
  oss << setw(15) << "Observer:"
      << table_.keywordSet().asString("Observer") << endl;
  // Antenna Name
  table_.keywordSet().get("AntennaName", tmp);
  oss << setw(15) << "Antenna Name:" << tmp << endl;
  // Obs type
  table_.keywordSet().get("Obstype", tmp);
  // Records (nrow)
  oss << setw(15) << "Data Records:" << table_.nrow() << " rows" << endl;
  oss << setw(15) << "Obs. Type:" << tmp << endl;
  // Beams, IFs, Polarizations, and Channels
  oss << setw(15) << "Beams:" << setw(4) << nbeam() << endl
      << setw(15) << "IFs:" << setw(4) << nif() << endl
      << setw(15) << "Polarisations:" << setw(4) << npol()
      << "(" << getPolType() << ")" << endl
      << setw(15) << "Channels:" << nchan() << endl;
  // Flux unit
  table_.keywordSet().get("FluxUnit", tmp);
  oss << setw(15) << "Flux Unit:" << tmp << endl;
  // Abscissa Unit
  oss << setw(15) << "Abscissa:" << getAbcissaLabel(0) << endl;
  // Selection
  oss << selector_.print() << endl;

  return String(oss);
}

void Scantable::summary( const std::string& filename )
{
  ostringstream oss;
  ofstream ofs;
  LogIO ols(LogOrigin("Scantable", "summary", WHERE));

  if (filename != "")
    ofs.open( filename.c_str(),  ios::out );

  oss << endl;
  oss << asap::SEPERATOR << endl;
  oss << " Scan Table Summary" << endl;
  oss << asap::SEPERATOR << endl;

  // Format header info
  oss << headerSummary();
  oss << endl;

  if (table_.nrow() <= 0){
    oss << asap::SEPERATOR << endl;
    oss << "The MAIN table is empty: there are no data!!!" << endl;
    oss << asap::SEPERATOR << endl;

    ols << String(oss) << LogIO::POST;
    if (ofs) {
      ofs << String(oss) << flush;
      ofs.close();
    }
    return;
  }



  // main table
  String dirtype = "Position ("
                  + getDirectionRefString()
                  + ")";
  oss.flags(std::ios_base::left);
  oss << setw(5) << "Scan" 
      << setw(15) << "Source"
      << setw(35) << "Time range"
      << setw(2) << "" << setw(7) << "Int[s]"
      << setw(7) << "Record"
      << setw(8) << "SrcType"
      << setw(8) << "FreqIDs"
      << setw(7) << "MolIDs" << endl;
  oss << setw(7)<< "" << setw(6) << "Beam"
      << setw(23) << dirtype << endl;

  oss << asap::SEPERATOR << endl;

  // Flush summary and clear up the string
  ols << String(oss) << LogIO::POST;
  if (ofs) ofs << String(oss) << flush;
  oss.str("");
  oss.clear();


  // Get Freq_ID map
  ROScalarColumn<uInt> ftabIds(frequencies().table(), "ID");
  Int nfid = ftabIds.nrow();
  if (nfid <= 0){
    oss << "FREQUENCIES subtable is empty: there are no data!!!" << endl;
    oss << asap::SEPERATOR << endl;

    ols << String(oss) << LogIO::POST;
    if (ofs) {
      ofs << String(oss) << flush;
      ofs.close();
    }
    return;
  }
  // Storages of overall IFNO, POLNO, and nchan per FREQ_ID
  // the orders are identical to ID in FREQ subtable
  Block< Vector<uInt> > ifNos(nfid), polNos(nfid);
  Vector<Int> fIdchans(nfid,-1);
  Vector<Double> fIdfreq0(nfid,-1);
  Vector<Double> fIdfcent(nfid,-1);
  map<uInt, Int> fidMap;  // (FREQ_ID, row # in FREQ subtable) pair
  for (Int i=0; i < nfid; i++){
   // fidMap[freqId] returns row number in FREQ subtable
   fidMap.insert(pair<uInt, Int>(ftabIds(i),i));
   ifNos[i] = Vector<uInt>();
   polNos[i] = Vector<uInt>();
  }

  TableIterator iter(table_, "SCANNO");

  // Vars for keeping track of time, freqids, molIds in a SCANNO
  //Vector<uInt> freqids;
  //Vector<uInt> molids;
  Vector<uInt> beamids(1,0);
  Vector<MDirection> beamDirs;
  Vector<Int> stypeids(1,0);
  Vector<String> stypestrs;
  Int nfreq(1);
  Int nmol(1);
  uInt nbeam(1);
  uInt nstype(1);

  Double btime(0.0), etime(0.0);
  Double meanIntTim(0.0);

  uInt currFreqId(0), ftabRow(0);
  Int iflen(0), pollen(0);

  while (!iter.pastEnd()) {
    Table subt = iter.table();
    uInt snrow = subt.nrow();
    ROTableRow row(subt);
    const TableRecord& rec = row.get(0);

    // relevant columns
    ROScalarColumn<Double> mjdCol(subt,"TIME");
    ROScalarColumn<Double> intervalCol(subt,"INTERVAL");
    MDirection::ROScalarColumn dirCol(subt,"DIRECTION");

    ScalarColumn<uInt> freqIdCol(subt,"FREQ_ID");
    ScalarColumn<uInt> molIdCol(subt,"MOLECULE_ID");
    ROScalarColumn<uInt> beamCol(subt,"BEAMNO");
    ROScalarColumn<Int> stypeCol(subt,"SRCTYPE");

    ROScalarColumn<uInt> ifNoCol(subt,"IFNO");
    ROScalarColumn<uInt> polNoCol(subt,"POLNO");


    // Times
    meanIntTim = sum(intervalCol.getColumn()) / (double) snrow;
    minMax(btime, etime, mjdCol.getColumn());
    double shiftInDay(0.5*meanIntTim/C::day);
    btime -= shiftInDay;
    etime += shiftInDay;

    // MOLECULE_ID and FREQ_ID
    Vector<uInt> molids(getNumbers(molIdCol));
    molids.shape(nmol);

    Vector<uInt> freqids(getNumbers(freqIdCol));
    freqids.shape(nfreq);

    // Add first beamid, and srcNames
    beamids.resize(1,False);
    beamDirs.resize(1,False);
    beamids(0)=beamCol(0);
    beamDirs(0)=dirCol(0);
    nbeam = 1;

    stypeids.resize(1,False);
    stypeids(0)=stypeCol(0);
    nstype = 1;

    // Global listings of nchan/IFNO/POLNO per FREQ_ID
    currFreqId=freqIdCol(0);
    ftabRow = fidMap[currFreqId];
    // Assumes an identical number of channels per FREQ_ID
    if (fIdchans(ftabRow) < 0 ) {
      RORecordFieldPtr< Array<Float> > spec(rec, "SPECTRA");
      fIdchans(ftabRow)=(*spec).shape()(0);
    }
    if (fIdfreq0(ftabRow) < 0 ) {
      SpectralCoordinate spc = frequencies().getSpectralCoordinate(ftabRow);
      Double fs, fe;
      spc.toWorld(fs, 0);
      spc.toWorld(fe, fIdchans(ftabRow)-1);
      fIdfreq0(ftabRow) = fs;
      fIdfcent(ftabRow) = 0.5 * ( fs + fe );
    }
    // Should keep ifNos and polNos form the previous SCANNO
    if ( !anyEQ(ifNos[ftabRow],ifNoCol(0)) ) {
      ifNos[ftabRow].shape(iflen);
      iflen++;
      ifNos[ftabRow].resize(iflen,True);
      ifNos[ftabRow](iflen-1) = ifNoCol(0);
    }
    if ( !anyEQ(polNos[ftabRow],polNoCol(0)) ) {
      polNos[ftabRow].shape(pollen);
      pollen++;
      polNos[ftabRow].resize(pollen,True);
      polNos[ftabRow](pollen-1) = polNoCol(0);
    }

    for (uInt i=1; i < snrow; i++){
      // Need to list BEAMNO and DIRECTION in the same order
      if ( !anyEQ(beamids,beamCol(i)) ) {
	nbeam++;
	beamids.resize(nbeam,True);
	beamids(nbeam-1)=beamCol(i);
	beamDirs.resize(nbeam,True);
	beamDirs(nbeam-1)=dirCol(i);
      }

      // SRCTYPE is Int (getNumber takes only uInt)
      if ( !anyEQ(stypeids,stypeCol(i)) ) {
	nstype++;
	stypeids.resize(nstype,True);
	stypeids(nstype-1)=stypeCol(i);
      }

      // Global listings of nchan/IFNO/POLNO per FREQ_ID
      currFreqId=freqIdCol(i);
      ftabRow = fidMap[currFreqId];
      if (fIdchans(ftabRow) < 0 ) {
	const TableRecord& rec = row.get(i);
	RORecordFieldPtr< Array<Float> > spec(rec, "SPECTRA");
	fIdchans(ftabRow) = (*spec).shape()(0);
      }
      if (fIdfreq0(ftabRow) < 0 ) {
	SpectralCoordinate spc = frequencies().getSpectralCoordinate(ftabRow);
	Double fs, fe;
	spc.toWorld(fs, 0);
	spc.toWorld(fe, fIdchans(ftabRow)-1);
	fIdfreq0(ftabRow) = fs;
	fIdfcent(ftabRow) = 5.e-1 * ( fs + fe );
      }
      if ( !anyEQ(ifNos[ftabRow],ifNoCol(i)) ) {
	ifNos[ftabRow].shape(iflen);
	iflen++;
	ifNos[ftabRow].resize(iflen,True);
	ifNos[ftabRow](iflen-1) = ifNoCol(i);
      }
      if ( !anyEQ(polNos[ftabRow],polNoCol(i)) ) {
	polNos[ftabRow].shape(pollen);
	pollen++;
	polNos[ftabRow].resize(pollen,True);
	polNos[ftabRow](pollen-1) = polNoCol(i);
      }
    } // end of row iteration

    stypestrs.resize(nstype,False);
    for (uInt j=0; j < nstype; j++)
      stypestrs(j) = SrcType::getName(stypeids(j));

    // Format Scan summary
    oss << setw(4) << std::right << rec.asuInt("SCANNO")
	<< std::left << setw(1) << ""
	<< setw(15) << rec.asString("SRCNAME")
	<< setw(21) << MVTime(btime).string(MVTime::YMD,8)
	<< setw(3) << " - " << MVTime(etime).string(MVTime::TIME,8)
	<< setw(3) << "" << setw(6) << meanIntTim << setw(1) << "" 
	<< std::right << setw(5) << snrow << setw(2) << ""
	<< std::left << stypestrs << setw(1) << ""
	<< freqids << setw(1) << ""
	<< molids  << endl;
    // Format Beam summary
    for (uInt j=0; j < nbeam; j++) {
      oss << setw(7) << "" << setw(6) << beamids(j) << setw(1) << ""
	  << formatDirection(beamDirs(j),9) << endl;
    }
    // Flush summary every scan and clear up the string
    ols << String(oss) << LogIO::POST;
    if (ofs) ofs << String(oss) << flush;
    oss.str("");
    oss.clear();

    ++iter;
  } // end of scan iteration
  oss << asap::SEPERATOR << endl;
  
  // List FRECUENCIES Table (using STFrequencies.print may be slow)
  oss << "FREQUENCIES: " << nfreq << endl;
//   oss << std::right << setw(5) << "ID" << setw(2) << ""
//       << std::left  << setw(5) << "IFNO" << setw(2) << ""
//       << setw(8) << "Frame"
//       << setw(16) << "RefVal"
//       << setw(7) << "RefPix"
//       << setw(15) << "Increment"
//       << setw(9) << "Channels"
//       << setw(6) << "POLNOs" << endl;
//   Int tmplen;
//   for (Int i=0; i < nfid; i++){
//     // List row=i of FREQUENCIES subtable
//     ifNos[i].shape(tmplen);
//     if (tmplen >= 1) {
//       oss << std::right << setw(5) << ftabIds(i) << setw(2) << ""
// 	  << setw(3) << ifNos[i](0) << setw(1) << ""
// 	  << std::left << setw(46) << frequencies().print(ftabIds(i)) 
// 	  << setw(2) << ""
// 	  << std::right << setw(8) << fIdchans[i] << setw(2) << ""
// 	  << std::left << polNos[i];
//       if (tmplen > 1) {
// 	oss  << " (" << tmplen << " chains)";
//       }
//       oss << endl;
//     }
  oss << std::right << setw(4) << "ID" << setw(2) << ""
      << std::left  << setw(9) << "IFNO(SPW)" << setw(2) << ""
      << setw(8) << "#Chans"
      << setw(8) << "Frame"
      << setw(12) << "Ch0[MHz]"
      << setw(14) << "ChanWid[kHz]"
      << setw(14) << "Center[MHz]"
      << setw(6) << "POLNOs" << endl;
  Int tmplen;
  for (Int i=0; i < nfid; i++){
    // List row=i of FREQUENCIES subtable
    ifNos[i].shape(tmplen);
    Double refpix, refval, increment ;
    if (tmplen >= 1) {
      freqTable_.getEntry( refpix, refval, increment, ftabIds(i) ) ;
      oss << std::right << setw(4) << ftabIds(i) << setw(2) << "" 
	  << std::left << setw(9) << ifNos[i](0) << setw(2) << ""
	  << std::right << setw(6) << fIdchans[i] << setw(2) << ""
	  << setw(6) << frequencies().getFrameString(true)
	  << setw(2) << "" 
	  << setw(10) << std::setprecision(9) << (fIdfreq0[i]*1.e-6) << setw(2) << ""
	  << setw(12) << (increment*1.e-3) << setw(2) << ""
	  << setw(12) << (fIdfcent[i]*1.e-6) << setw(2) << ""
	  << std::left << polNos[i];
      if (tmplen > 1) {
	oss  << " (" << tmplen << " chains)";
      }
      oss << endl;
    }
    
  }
  oss << asap::SEPERATOR << endl;

  // List MOLECULES Table (currently lists all rows)
  oss << "MOLECULES: " << endl;
  if (molecules().nrow() <= 0) {
    oss << "   MOLECULES subtable is empty: there are no data" << endl;
  } else {
    ROTableRow row(molecules().table());
    oss << std::right << setw(5) << "ID" 
	<< std::left << setw(3) << ""
	<< setw(18) << "RestFreq"
	<< setw(15) << "Name" << endl;
    for (Int i=0; i < molecules().nrow(); i++){
      const TableRecord& rec=row.get(i);
      oss << std::right << setw(5) << rec.asuInt("ID")
	  << std::left << setw(3) << ""
	  << rec.asArrayDouble("RESTFREQUENCY") << setw(1) << ""
	  << rec.asArrayString("NAME") << endl;
    }
  }
  oss << asap::SEPERATOR << endl;
  ols << String(oss) << LogIO::POST;
  if (ofs) {
    ofs << String(oss) << flush;
    ofs.close();
  }
  //  return String(oss);
}


std::string Scantable::oldheaderSummary()
{
  // Format header info
//   STHeader sdh;
//   sdh = getHeader();
//   sdh.print();
  ostringstream oss;
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
  int nid = moleculeTable_.nrow();
  Bool firstline = True;
  oss << setw(15) << "Rest Freqs:";
  for (int i=0; i<nid; i++) {
    Table t = table_(table_.col("MOLECULE_ID") == i, 1);
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
  return String(oss);
}

  //std::string Scantable::summary( const std::string& filename )
void Scantable::oldsummary( const std::string& filename )
{
  ostringstream oss;
  ofstream ofs;
  LogIO ols(LogOrigin("Scantable", "summary", WHERE));

  if (filename != "")
    ofs.open( filename.c_str(),  ios::out );

  oss << endl;
  oss << asap::SEPERATOR << endl;
  oss << " Scan Table Summary" << endl;
  oss << asap::SEPERATOR << endl;

  // Format header info
  oss << oldheaderSummary();
  oss << endl;

  // main table
  String dirtype = "Position ("
                  + getDirectionRefString()
                  + ")";
  oss.flags(std::ios_base::left);
  oss << setw(5) << "Scan" << setw(15) << "Source"
      << setw(10) << "Time" << setw(18) << "Integration" 
      << setw(15) << "Source Type" << endl;
  oss << setw(5) << "" << setw(5) << "Beam" << setw(3) << "" << dirtype << endl;
  oss << setw(10) << "" << setw(3) << "IF" << setw(3) << ""
      << setw(8) << "Frame" << setw(16)
      << "RefVal" << setw(10) << "RefPix" << setw(12) << "Increment"
      << setw(7) << "Channels"
      << endl;
  oss << asap::SEPERATOR << endl;

  // Flush summary and clear up the string
  ols << String(oss) << LogIO::POST;
  if (ofs) ofs << String(oss) << flush;
  oss.str("");
  oss.clear();

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
        << setw(11) <<  formatSec(rec.asFloat("INTERVAL")) << setw(1) << ""
	<< setw(15) << SrcType::getName(rec.asInt("SRCTYPE")) << endl;

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
    // Flush summary every scan and clear up the string
    ols << String(oss) << LogIO::POST;
    if (ofs) ofs << String(oss) << flush;
    oss.str("");
    oss.clear();

    ++iter;
  }
  oss << asap::SEPERATOR << endl;
  ols << String(oss) << LogIO::POST;
  if (ofs) {
    ofs << String(oss) << flush;
    ofs.close();
  }
  //  return String(oss);
}

// std::string Scantable::getTime(int whichrow, bool showdate) const
// {
//   MEpoch::ROScalarColumn timeCol(table_, "TIME");
//   MEpoch me;
//   if (whichrow > -1) {
//     me = timeCol(uInt(whichrow));
//   } else {
//     Double tm;
//     table_.keywordSet().get("UTC",tm);
//     me = MEpoch(MVEpoch(tm));
//   }
//   return formatTime(me, showdate);
// }

std::string Scantable::getTime(int whichrow, bool showdate, uInt prec) const
{
  MEpoch me;
  me = getEpoch(whichrow);
  return formatTime(me, showdate, prec);
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
  (void) name; // suppress unused warning
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
  String::size_type pos1 = out.find("@") ;
  String::size_type pos2 = out.find("//") ;
  if ( pos2 != String::npos ) 
    out = out.substr(pos2+2,pos1-pos2-2) ;
  else if ( pos1 != String::npos )
    out = out.substr(0,pos1) ;
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
      Table subt = t( t.col("SCAN") == scanlist[i] );
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
	  Table subt2 = t( t.col("SCAN") == scanlist[i+1] );
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
  if ( nmax >= nChan-1 ) {
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
  Vector<uInt> freqIdArray = mfreqidCol_.getColumn();
  uInt numFreqId = GenSort<uInt>::sort(freqIdArray, Sort::Ascending,
				       Sort::HeapSort | Sort::NoDuplicates);
  Double refpix ;
  Double refval ;
  Double increment ;
  for (uInt irow  = 0; irow < numFreqId; irow++) { 
    freqTable_.getEntry( refpix, refval, increment, freqIdArray[irow] ) ;
    /***
     * need to shift refpix to nmin
     * note that channel nmin in old index will be channel 0 in new one
     ***/
    refval = refval - ( refpix - nmin ) * increment ;
    refpix = 0 ;
    freqTable_.setEntry( refpix, refval, increment, freqIdArray[irow] ) ;
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
  Vector<Float> oldtsys = tsysCol_( irow ) ;
  uInt newsize = nmax - nmin + 1 ;
  Slice slice( nmin, newsize, 1 ) ;
  specCol_.put( irow, oldspec( slice ) ) ;
  flagsCol_.put( irow, oldflag( slice ) ) ;
  if ( oldspec.size() == oldtsys.size() )
    tsysCol_.put( irow, oldtsys( slice ) ) ;

  return ;
}

void asap::Scantable::regridSpecChannel( double dnu, int nChan )
{
  LogIO os( LogOrigin( "Scantable", "regridChannel()", WHERE ) ) ;
  os << "Regrid abcissa with spectral resoultion " << dnu << " " << freqTable_.getUnitString() << " with channel number " << ((nChan>0)? String(nChan) : "covering band width")<< LogIO::POST ;
  int freqnrow = freqTable_.table().nrow() ;
  Vector<bool> firstTime( freqnrow, true ) ;
  double oldincr, factor;
  uInt currId;
  Double refpix ;
  Double refval ;
  Double increment ;
  for ( int irow = 0 ; irow < nrow() ; irow++ ) {
    currId = mfreqidCol_(irow);
    vector<double> abcissa = getAbcissa( irow ) ;
    if (nChan < 0) {
      int oldsize = abcissa.size() ;
      double bw = (abcissa[oldsize-1]-abcissa[0]) +			\
	0.5 * (abcissa[1]-abcissa[0] + abcissa[oldsize-1]-abcissa[oldsize-2]) ;
      nChan = int( ceil( abs(bw/dnu) ) ) ;
    }
    // actual regridding
    regridChannel( nChan, dnu, irow ) ;

    // update FREQUENCIES subtable
    if (firstTime[currId]) {
      oldincr = abcissa[1]-abcissa[0] ;
      factor = dnu/oldincr ;
      firstTime[currId] = false ;
      freqTable_.getEntry( refpix, refval, increment, currId ) ;

      //refval = refval - ( refpix + 0.5 * (1 - factor) ) * increment ;
      if (factor > 0 ) {
	refpix = (refpix + 0.5)/factor - 0.5;
      } else {
	refpix = (abcissa.size() - 0.5 - refpix)/abs(factor) - 0.5;
      }
      freqTable_.setEntry( refpix, refval, increment*factor, currId ) ;
      //os << "ID" << currId << ": channel width (Orig) = " << oldincr << " [" << freqTable_.getUnitString() << "], scale factor = " << factor << LogIO::POST ;
      //os << "     frequency increment (Orig) = " << increment << "-> (New) " << increment*factor << LogIO::POST ;
    }
  }
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

  // change channel number for specCol_, flagCol_, and tsysCol_ (if necessary)
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
  Vector<Float> oldtsys = tsysCol_( irow ) ;
  Vector<Float> newspec( nChan, 0 ) ;
  Vector<uChar> newflag( nChan, true ) ;
  Vector<Float> newtsys ;
  bool regridTsys = false ;
  if (oldtsys.size() == oldspec.size()) {
    regridTsys = true ;
    newtsys.resize(nChan,false) ;
    newtsys = 0 ;
  }

  // regrid
  vector<double> abcissa = getAbcissa( irow ) ;
  int oldsize = abcissa.size() ;
  double olddnu = abcissa[1] - abcissa[0] ;
  //int ichan = 0 ;
  double wsum = 0.0 ;
  Vector<double> zi( nChan+1 ) ;
  Vector<double> yi( oldsize + 1 ) ;
  yi[0] = abcissa[0] - 0.5 * olddnu ;
  for ( int ii = 1 ; ii < oldsize ; ii++ )
    yi[ii] = 0.5* (abcissa[ii-1] + abcissa[ii]) ;
  yi[oldsize] = abcissa[oldsize-1] \
    + 0.5 * (abcissa[oldsize-1] - abcissa[oldsize-2]) ;
  //zi[0] = abcissa[0] - 0.5 * olddnu ;
  zi[0] = ((olddnu*dnu > 0) ? yi[0] : yi[oldsize]) ;
  for ( int ii = 1 ; ii < nChan ; ii++ )
    zi[ii] = zi[0] + dnu * ii ;
  zi[nChan] = zi[nChan-1] + dnu ;
  // Access zi and yi in ascending order
  int izs = ((dnu > 0) ? 0 : nChan ) ;
  int ize = ((dnu > 0) ? nChan : 0 ) ;
  int izincr = ((dnu > 0) ? 1 : -1 ) ;
  int ichan =  ((olddnu > 0) ? 0 : oldsize ) ;
  int iye = ((olddnu > 0) ? oldsize : 0 ) ;
  int iyincr = ((olddnu > 0) ? 1 : -1 ) ;
  //for ( int ii = izs ; ii != ize ; ii+=izincr ){
  int ii = izs ;
  while (ii != ize) {
    // always zl < zr
    double zl = zi[ii] ;
    double zr = zi[ii+izincr] ;
    // Need to access smaller index for the new spec, flag, and tsys.
    // Values between zi[k] and zi[k+1] should be stored in newspec[k], etc.
    int i = min(ii, ii+izincr) ;
    //for ( int jj = ichan ; jj != iye ; jj+=iyincr ) {
    int jj = ichan ;
    while (jj != iye) {
      // always yl < yr
      double yl = yi[jj] ;
      double yr = yi[jj+iyincr] ;
      // Need to access smaller index for the original spec, flag, and tsys.
      // Values between yi[k] and yi[k+1] are stored in oldspec[k], etc.
      int j = min(jj, jj+iyincr) ;
      if ( yr <= zl ) {
	jj += iyincr ;
	continue ;
      }
      else if ( yl <= zl ) {
	if ( yr < zr ) {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * ( yr - zl ) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * ( yr - zl ) ;
	    wsum += ( yr - zl ) ;
	  }
	  newflag[i] = (newflag[i] && oldflag[j]) ? 1 << 7 : 0 ;
	}
	else {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * abs(dnu) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * abs(dnu) ;
	    wsum += abs(dnu) ;
	  }
	  newflag[i] = (newflag[i] && oldflag[j]) ? 1 << 7 : 0 ;
	  ichan = jj ;
	  break ;
	}
      }
      else if ( yl < zr ) {
	if ( yr <= zr ) {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * ( yr - yl ) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * ( yr - yl ) ;
	    wsum += ( yr - yl ) ;
	  }
	  newflag[i] = (newflag[i] && oldflag[j]) ? 1 << 7 : 0 ;
	}
	else {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * ( zr - yl ) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * ( zr - yl ) ;
	    wsum += ( zr - yl ) ;
	  }
	  newflag[i] = (newflag[i] && oldflag[j]) ? 1 << 7 : 0 ;
	  ichan = jj ;
	  break ;
	}
      }
      else {
	ichan = jj - iyincr ;
	break ;
      }
      jj += iyincr ;
    }
    if ( wsum != 0.0 ) {
      newspec[i] /= wsum ;
      if (regridTsys) newtsys[i] /= wsum ;
    }
    wsum = 0.0 ;
    ii += izincr ;
  }
//   if ( dnu > 0.0 ) {
//     for ( int ii = 0 ; ii < nChan ; ii++ ) {
//       double zl = zi[ii] ;
//       double zr = zi[ii+1] ;
//       for ( int j = ichan ; j < oldsize ; j++ ) {
//         double yl = yi[j] ;
//         double yr = yi[j+1] ;
//         if ( yl <= zl ) {
//           if ( yr <= zl ) {
//             continue ;
//           }
//           else if ( yr <= zr ) {
// 	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * ( yr - zl ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * ( yr - zl ) ;
// 	      wsum += ( yr - zl ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//           }
//           else {
// 	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * dnu ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * dnu ;
// 	      wsum += dnu ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//             ichan = j ;
//             break ;
//           }
//         }
//         else if ( yl < zr ) {
//           if ( yr <= zr ) {
//  	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * ( yr - yl ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * ( yr - yl ) ;
//               wsum += ( yr - yl ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//           }
//           else {
//  	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * ( zr - yl ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * ( zr - yl ) ;
// 	      wsum += ( zr - yl ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//             ichan = j ;
//             break ;
//           }
//         }
//         else {
//           ichan = j - 1 ;
//           break ;
//         }
//       }
//       if ( wsum != 0.0 ) {
//         newspec[ii] /= wsum ;
// 	if (regridTsys) newtsys[ii] /= wsum ;
//       }
//       wsum = 0.0 ;
//     }
//   }
//   else if ( dnu < 0.0 ) {
//     for ( int ii = 0 ; ii < nChan ; ii++ ) {
//       double zl = zi[ii] ;
//       double zr = zi[ii+1] ;
//       for ( int j = ichan ; j < oldsize ; j++ ) {
//         double yl = yi[j] ;
//         double yr = yi[j+1] ;
//         if ( yl >= zl ) {
//           if ( yr >= zl ) {
//             continue ;
//           }
//           else if ( yr >= zr ) {
//  	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * abs( yr - zl ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * abs( yr - zl ) ;
// 	      wsum += abs( yr - zl ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//           }
//           else {
//  	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * abs( dnu ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * abs( dnu ) ;
// 	      wsum += abs( dnu ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//             ichan = j ;
//             break ;
//           }
//         }
//         else if ( yl > zr ) {
//           if ( yr >= zr ) {
//  	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * abs( yr - yl ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * abs( yr - yl ) ;
// 	      wsum += abs( yr - yl ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//           }
//           else {
//  	    if (!oldflag[j]) {
// 	      newspec[ii] += oldspec[j] * abs( zr - yl ) ;
// 	      if (regridTsys) newtsys[ii] += oldtsys[j] * abs( zr - yl ) ;
// 	      wsum += abs( zr - yl ) ;
// 	    }
// 	    newflag[ii] = newflag[ii] && oldflag[j] ;
//             ichan = j ;
//             break ;
//           }
//         }
//         else {
//           ichan = j - 1 ;
//           break ;
//         }
//       }
//       if ( wsum != 0.0 ) {
//         newspec[ii] /= wsum ;
// 	if (regridTsys) newtsys[ii] /= wsum ;
//       }
//       wsum = 0.0 ;
//     }
//   }
// //   //ofs << "olddnu = " << olddnu << ", dnu = " << dnu << endl ;
// //   pile += dnu ;
// //   wedge = olddnu * ( refChan + 1 ) ;
// //   while ( wedge < pile ) {
// //     newspec[0] += olddnu * oldspec[refChan] ;
// //     newflag[0] = newflag[0] || oldflag[refChan] ;
// //     //ofs << "channel " << refChan << " is included in new channel 0" << endl ;
// //     refChan++ ;
// //     wedge += olddnu ;
// //     wsum += olddnu ;
// //     //ofs << "newspec[0] = " << newspec[0] << " wsum = " << wsum << endl ;
// //   }
// //   frac = ( wedge - pile ) / olddnu ;
// //   wsum += ( 1.0 - frac ) * olddnu ;
// //   newspec[0] += ( 1.0 - frac ) * olddnu * oldspec[refChan] ;
// //   newflag[0] = newflag[0] || oldflag[refChan] ;
// //   //ofs << "channel " << refChan << " is partly included in new channel 0" << " with fraction of " << ( 1.0 - frac ) << endl ;
// //   //ofs << "newspec[0] = " << newspec[0] << " wsum = " << wsum << endl ;
// //   newspec[0] /= wsum ;
// //   //ofs << "newspec[0] = " << newspec[0] << endl ;
// //   //ofs << "wedge = " << wedge << ", pile = " << pile << endl ;

// //   /***
// //    * ichan = 1 - nChan-2
// //    ***/
// //   for ( int ichan = 1 ; ichan < nChan - 1 ; ichan++ ) {
// //     pile += dnu ;
// //     newspec[ichan] += frac * olddnu * oldspec[refChan] ;
// //     newflag[ichan] = newflag[ichan] || oldflag[refChan] ;
// //     //ofs << "channel " << refChan << " is partly included in new channel " << ichan << " with fraction of " << frac << endl ;
// //     refChan++ ;
// //     wedge += olddnu ;
// //     wsum = frac * olddnu ;
// //     //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << " wsum = " << wsum << endl ;
// //     while ( wedge < pile ) {
// //       newspec[ichan] += olddnu * oldspec[refChan] ;
// //       newflag[ichan] = newflag[ichan] || oldflag[refChan] ;
// //       //ofs << "channel " << refChan << " is included in new channel " << ichan << endl ;
// //       refChan++ ;
// //       wedge += olddnu ;
// //       wsum += olddnu ;
// //       //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << " wsum = " << wsum << endl ;
// //     }
// //     frac = ( wedge - pile ) / olddnu ;
// //     wsum += ( 1.0 - frac ) * olddnu ;
// //     newspec[ichan] += ( 1.0 - frac ) * olddnu * oldspec[refChan] ;
// //     newflag[ichan] = newflag[ichan] || oldflag[refChan] ;
// //     //ofs << "channel " << refChan << " is partly included in new channel " << ichan << " with fraction of " << ( 1.0 - frac ) << endl ;
// //     //ofs << "wedge = " << wedge << ", pile = " << pile << endl ;
// //     //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << " wsum = " << wsum << endl ;
// //     newspec[ichan] /= wsum ;
// //     //ofs << "newspec[" << ichan << "] = " << newspec[ichan] << endl ;
// //   }

// //   /***
// //    * ichan = nChan-1
// //    ***/
// //   // NOTE: Assumed that all spectra have the same bandwidth
// //   pile += dnu ;
// //   newspec[nChan-1] += frac * olddnu * oldspec[refChan] ;
// //   newflag[nChan-1] = newflag[nChan-1] || oldflag[refChan] ;
// //   //ofs << "channel " << refChan << " is partly included in new channel " << nChan-1 << " with fraction of " << frac << endl ;
// //   refChan++ ;
// //   wedge += olddnu ;
// //   wsum = frac * olddnu ;
// //   //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << " wsum = " << wsum << endl ;
// //   for ( int jchan = refChan ; jchan < oldsize ; jchan++ ) {
// //     newspec[nChan-1] += olddnu * oldspec[jchan] ;
// //     newflag[nChan-1] = newflag[nChan-1] || oldflag[jchan] ;
// //     wsum += olddnu ;
// //     //ofs << "channel " << jchan << " is included in new channel " << nChan-1 << " with fraction of " << frac << endl ;
// //     //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << " wsum = " << wsum << endl ;
// //   }
// //   //ofs << "wedge = " << wedge << ", pile = " << pile << endl ;
// //   //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << " wsum = " << wsum << endl ;
// //   newspec[nChan-1] /= wsum ;
// //   //ofs << "newspec[" << nChan - 1 << "] = " << newspec[nChan-1] << endl ;

// //   // ofs.close() ;

  specCol_.put( irow, newspec ) ;
  flagsCol_.put( irow, newflag ) ;
  if (regridTsys) tsysCol_.put( irow, newtsys );

  return ;
}

void Scantable::regridChannel( int nChan, double dnu, double fmin, int irow ) 
{
  Vector<Float> oldspec = specCol_( irow ) ;
  Vector<uChar> oldflag = flagsCol_( irow ) ;
  Vector<Float> oldtsys = tsysCol_( irow ) ;
  Vector<Float> newspec( nChan, 0 ) ;
  Vector<uChar> newflag( nChan, true ) ;
  Vector<Float> newtsys ;
  bool regridTsys = false ;
  if (oldtsys.size() == oldspec.size()) {
    regridTsys = true ;
    newtsys.resize(nChan,false) ;
    newtsys = 0 ;
  }
  
  // regrid
  vector<double> abcissa = getAbcissa( irow ) ;
  int oldsize = abcissa.size() ;
  double olddnu = abcissa[1] - abcissa[0] ;
  //int ichan = 0 ;
  double wsum = 0.0 ;
  Vector<double> zi( nChan+1 ) ;
  Vector<double> yi( oldsize + 1 ) ;
  Block<uInt> count( nChan, 0 ) ;
  yi[0] = abcissa[0] - 0.5 * olddnu ;
  for ( int ii = 1 ; ii < oldsize ; ii++ )
    yi[ii] = 0.5* (abcissa[ii-1] + abcissa[ii]) ;
  yi[oldsize] = abcissa[oldsize-1] \
    + 0.5 * (abcissa[oldsize-1] - abcissa[oldsize-2]) ;
//   cout << "olddnu=" << olddnu << ", dnu=" << dnu << " (diff=" << olddnu-dnu << ")" << endl ;
//   cout << "yi[0]=" << yi[0] << ", fmin=" << fmin << " (diff=" << yi[0]-fmin << ")" << endl ;
//   cout << "oldsize=" << oldsize << ", nChan=" << nChan << endl ;

  // do not regrid if input parameters are almost same as current 
  // spectral setup
  double dnuDiff = abs( ( dnu - olddnu ) / olddnu ) ;
  double oldfmin = min( yi[0], yi[oldsize] ) ;
  double fminDiff = abs( ( fmin - oldfmin ) / oldfmin ) ;
  double nChanDiff = nChan - oldsize ;
  double eps = 1.0e-8 ;
  if ( nChanDiff == 0 && dnuDiff < eps && fminDiff < eps )
    return ;

  //zi[0] = abcissa[0] - 0.5 * olddnu ;
  //zi[0] = ((olddnu*dnu > 0) ? yi[0] : yi[oldsize]) ;
  if ( dnu > 0 )
    zi[0] = fmin - 0.5 * dnu ;
  else
    zi[0] = fmin + nChan * abs(dnu) ;
  for ( int ii = 1 ; ii < nChan ; ii++ )
    zi[ii] = zi[0] + dnu * ii ;
  zi[nChan] = zi[nChan-1] + dnu ;
  // Access zi and yi in ascending order
  int izs = ((dnu > 0) ? 0 : nChan ) ;
  int ize = ((dnu > 0) ? nChan : 0 ) ;
  int izincr = ((dnu > 0) ? 1 : -1 ) ;
  int ichan =  ((olddnu > 0) ? 0 : oldsize ) ;
  int iye = ((olddnu > 0) ? oldsize : 0 ) ;
  int iyincr = ((olddnu > 0) ? 1 : -1 ) ;
  //for ( int ii = izs ; ii != ize ; ii+=izincr ){
  int ii = izs ;
  while (ii != ize) {
    // always zl < zr
    double zl = zi[ii] ;
    double zr = zi[ii+izincr] ;
    // Need to access smaller index for the new spec, flag, and tsys.
    // Values between zi[k] and zi[k+1] should be stored in newspec[k], etc.
    int i = min(ii, ii+izincr) ;
    //for ( int jj = ichan ; jj != iye ; jj+=iyincr ) {
    int jj = ichan ;
    while (jj != iye) {
      // always yl < yr
      double yl = yi[jj] ;
      double yr = yi[jj+iyincr] ;
      // Need to access smaller index for the original spec, flag, and tsys.
      // Values between yi[k] and yi[k+1] are stored in oldspec[k], etc.
      int j = min(jj, jj+iyincr) ;
      if ( yr <= zl ) {
	jj += iyincr ;
	continue ;
      }
      else if ( yl <= zl ) {
	if ( yr < zr ) {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * ( yr - zl ) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * ( yr - zl ) ;
	    wsum += ( yr - zl ) ;
            count[i]++ ;
	  }
	  newflag[i] = newflag[i] && oldflag[j] ;
	}
	else {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * abs(dnu) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * abs(dnu) ;
	    wsum += abs(dnu) ;
            count[i]++ ;
	  }
	  newflag[i] = newflag[i] && oldflag[j] ;
	  ichan = jj ;
	  break ;
	}
      }
      else if ( yl < zr ) {
	if ( yr <= zr ) {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * ( yr - yl ) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * ( yr - yl ) ;
	    wsum += ( yr - yl ) ;
            count[i]++ ;
	  }
	  newflag[i] = newflag[i] && oldflag[j] ;
	}
	else {
	  if (!oldflag[j]) {
	    newspec[i] += oldspec[j] * ( zr - yl ) ;
	    if (regridTsys) newtsys[i] += oldtsys[j] * ( zr - yl ) ;
	    wsum += ( zr - yl ) ;
            count[i]++ ;
	  }
	  newflag[i] = newflag[i] && oldflag[j] ;
	  ichan = jj ;
	  break ;
	}
      }
      else {
	//ichan = jj - iyincr ;
	break ;
      }
      jj += iyincr ;
    }
    if ( wsum != 0.0 ) {
      newspec[i] /= wsum ;
      if (regridTsys) newtsys[i] /= wsum ;
    }
    wsum = 0.0 ;
    ii += izincr ;
  }

  // flag out channels without data
  // this is tentative since there is no specific definition 
  // on bit flag...
  uChar noData = 1 << 7 ; 
  for ( Int i = 0 ; i < nChan ; i++ ) {
    if ( count[i] == 0 ) 
      newflag[i] = noData ;
  }

  specCol_.put( irow, newspec ) ;
  flagsCol_.put( irow, newflag ) ;
  if (regridTsys) tsysCol_.put( irow, newtsys );

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

bool Scantable::isAllChannelsFlagged(uInt whichrow)
{
  uInt rflag;
  flagrowCol_.get(whichrow, rflag);
  if (rflag > 0)
    return true;
  uChar flag;
  Vector<uChar> flags;
  flagsCol_.get(whichrow, flags);
  flag = flags[0];
  for (uInt i = 1; i < flags.size(); ++i) {
    flag &= flags[i];
  }
  //  return ((flag >> 7) == 1);
  return (flag > 0);
}

std::vector<std::string> Scantable::applyBaselineTable(const std::string& bltable, const bool returnfitresult, const std::string& outbltable, const bool outbltableexists, const bool overwrite)
{
  STBaselineTable btin = STBaselineTable(bltable);

  Vector<Bool> applyCol = btin.getApply();
  int nRowBl = applyCol.size();
  if (nRowBl != nrow()) {
    throw(AipsError("Scantable and bltable have different number of rows."));
  }

  std::vector<std::string> res;
  res.clear();

  bool outBaselineTable = ((outbltable != "") && (!outbltableexists || overwrite));
  bool bltableidentical = (bltable == outbltable);
  STBaselineTable btout = STBaselineTable(*this);
  ROScalarColumn<Double> tcol = ROScalarColumn<Double>(table_, "TIME");
  Vector<Double> timeSecCol = tcol.getColumn();

  for (int whichrow = 0; whichrow < nRowBl; ++whichrow) {
    if (applyCol[whichrow]) {
      std::vector<float> spec = getSpectrum(whichrow);

      std::vector<bool> mask = btin.getMask(whichrow);  //use mask_bltable only

      STBaselineFunc::FuncName ftype = btin.getFunctionName(whichrow);
      std::vector<int> fpar = btin.getFuncParam(whichrow);
      std::vector<float> params;
      float rms;
      std::vector<float> resfit = doApplyBaselineTable(spec, mask, ftype, fpar, params, rms);
      setSpectrum(resfit, whichrow);

      if (returnfitresult) {
	res.push_back(packFittingResults(whichrow, params, rms));
      }

      if (outBaselineTable) {
	if (outbltableexists) {
	  if (overwrite) {
	    if (bltableidentical) {
	      btin.setresult(uInt(whichrow), Vector<Float>(params), Float(rms));
	    } else {
	      btout.setresult(uInt(whichrow), Vector<Float>(params), Float(rms));
	    }
	  }
	} else {
	  btout.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
			   getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
			   true, ftype, fpar, std::vector<float>(), 
			   getMaskListFromMask(mask), params, rms, spec.size(), 
			   3.0, 0, 0.0, 0, std::vector<int>());
	}
      }
    }
  }

  if (outBaselineTable) {
    if (bltableidentical) {
      btin.save(outbltable);
    } else {
      btout.save(outbltable);
    }
  }

  return res;
}

std::vector<std::string> Scantable::subBaseline(const std::vector<std::string>& blInfoList, const bool returnfitresult, const std::string& outbltable, const bool outbltableexists, const bool overwrite)
{
  int nRowBl = blInfoList.size();
  int nRowSt = nrow();

  std::vector<std::string> res;
  res.clear();

  bool outBaselineTable = ((outbltable != "") && (!outbltableexists || overwrite));
  if ((outbltable != "") && outbltableexists && !overwrite) {
    throw(AipsError("Cannot overwrite bltable. Set overwrite=True."));
  }

  STBaselineTable* btp;
  ROScalarColumn<Double> tcol = ROScalarColumn<Double>(table_, "TIME");
  Vector<Double> timeSecCol = tcol.getColumn();

  if (outBaselineTable) {
    if (outbltableexists) {
      btp = new STBaselineTable((String)outbltable);
    } else {
      btp = new STBaselineTable(*this);
      // for (int i = 0; i < nRowSt; ++i) {
      //   btp->appendbasedata(getScan(i), getCycle(i), getBeam(i), getIF(i), getPol(i), 
      //   		   0, timeSecCol[i]);
      //   btp->setApply(i, false);
      // }
    }
    int nrow = btp->nrow();
    for (int i = nrow; i < nRowSt; ++i) {
      btp->appendbasedata(getScan(i), getCycle(i), getBeam(i), getIF(i), getPol(i), 
                          0, timeSecCol[i]);
      btp->setApply(i, false);
    }
  }

  for (int i = 0; i < nRowBl; ++i) {
    int irow;
    STBaselineFunc::FuncName ftype;
    std::vector<bool> mask;
    std::vector<int> fpar;
    float clipth;
    int clipn;
    bool uself;
    float lfth;
    std::vector<int> lfedge;
    int lfavg;
    parseBlInfo(blInfoList[i], irow, ftype, fpar, mask, clipth, clipn, uself, lfth, lfedge, lfavg);

    if (irow < nRowSt) {
      std::vector<float> spec = getSpectrum(irow);
      std::vector<float> params;
      float rms;
      std::vector<bool> finalmask;
      Bool doApply = True;
      
      if (flagrowCol_(irow) == 0) {
        std::vector<float> resfit = doSubtractBaseline(spec, mask, ftype, fpar, params, rms, finalmask, clipth, clipn, uself, irow, lfth, lfedge, lfavg);
        setSpectrum(resfit, irow);
      }
      else {
        doApply = False;
      }

      if (returnfitresult) {
	res.push_back(packFittingResults(irow, params, rms));
      }

      if (outBaselineTable) {
	Vector<Int> fparam(fpar.size());
	for (uInt j = 0; j < fparam.size(); ++j) {
	  fparam[j] = (Int)fpar[j];
	}

	btp->setdata(uInt(irow), 
		    uInt(getScan(irow)), uInt(getCycle(irow)), 
		    uInt(getBeam(irow)), uInt(getIF(irow)), uInt(getPol(irow)), 
		    uInt(0), timeSecCol[irow], doApply, ftype, fparam, 
	            Vector<Float>(), getMaskListFromMask(finalmask), Vector<Float>(params), 
	            Float(rms), uInt(spec.size()), Float(clipth), uInt(clipn), 
	            Float(0.0), uInt(0), Vector<uInt>());
      }

    }
  }

  if (outBaselineTable) {
    btp->save(outbltable);
  }

  delete btp;
  return res;
}

std::vector<float> Scantable::doApplyBaselineTable(std::vector<float>& spec, 
						   std::vector<bool>& mask, 
						   const STBaselineFunc::FuncName ftype, 
						   std::vector<int>& fpar, 
						   std::vector<float>& params, 
						   float&rms)
{
  std::vector<bool> finalmask;
  std::vector<int> lfedge;
  return doSubtractBaseline(spec, mask, ftype, fpar, params, rms, finalmask, 0.0, 0, false, 0, 0.0, lfedge, 0);
}

std::vector<float> Scantable::doSubtractBaseline(std::vector<float>& spec,
						 std::vector<bool>& mask,
						 const STBaselineFunc::FuncName ftype,
						 std::vector<int>& fpar,
						 std::vector<float>& params,
						 float&rms,
						 std::vector<bool>& finalmask,
						 float clipth,
						 int clipn,
						 bool uself,
						 int irow,
						 float lfth,
						 std::vector<int>& lfedge,
						 int lfavg)
{
  if (uself) {
    STLineFinder lineFinder = STLineFinder();
    initLineFinder(lfedge, lfth, lfavg, lineFinder);
    std::vector<int> currentEdge;
    mask = getCompositeChanMask(irow, mask, lfedge, currentEdge, lineFinder);
  } else {
    mask = getCompositeChanMask(irow, mask);
  }

  std::vector<float> res;
  if (ftype == STBaselineFunc::Polynomial) {
    res = doPolynomialFitting(spec, mask, fpar[0], params, rms, finalmask, clipth, clipn);
  } else if (ftype == STBaselineFunc::Chebyshev) {
    res = doChebyshevFitting(spec, mask, fpar[0], params, rms, finalmask, clipth, clipn);
  } else if (ftype == STBaselineFunc::CSpline) {
    if (fpar.size() > 1) { // reading from baseline table in which pieceEdges are already calculated and stored.
      res = doCubicSplineFitting(spec, mask, fpar, params, rms, finalmask, clipth, clipn);
    } else {               // usual cspline fitting by giving nPiece only. fpar will be replaced with pieceEdges. 
      res = doCubicSplineFitting(spec, mask, fpar[0], fpar, params, rms, finalmask, clipth, clipn);
    }
  } else if (ftype == STBaselineFunc::Sinusoid) {
    res = doSinusoidFitting(spec, mask, fpar, params, rms, finalmask, clipth, clipn);
  }

  return res;
}

std::string Scantable::packFittingResults(const int irow, const std::vector<float>& params, const float rms)
{
  // returned value: "irow:params[0],params[1],..,params[n-1]:rms"
  ostringstream os;
  os << irow << ':';
  for (uInt i = 0; i < params.size(); ++i) {
    if (i > 0) {
      os << ',';
    }
    os << params[i];
  }
  os << ':' << rms;

  return os.str();
}

void Scantable::parseBlInfo(const std::string& blInfo, int& irow, STBaselineFunc::FuncName& ftype, std::vector<int>& fpar, std::vector<bool>& mask, float& thresClip, int& nIterClip, bool& useLineFinder, float& thresLF, std::vector<int>& edgeLF, int& avgLF)
{
  // The baseline info to be parsed must be column-delimited string like 
  // "0:chebyshev:5:3,5,169,174,485,487" where the elements are 
  // row number, funcType, funcOrder, maskList, clipThreshold, clipNIter, 
  // useLineFinder, lfThreshold, lfEdge and lfChanAvgLimit.

  std::vector<string> res = splitToStringList(blInfo, ':');
  if (res.size() < 4) {
    throw(AipsError("baseline info has bad format")) ;
  }

  string ftype0, fpar0, masklist0, uself0, edge0;
  std::vector<int> masklist;

  stringstream ss;
  ss << res[0];
  ss >> irow;
  ss.clear(); ss.str("");

  ss << res[1];
  ss >> ftype0;
  if (ftype0 == "poly") {
    ftype = STBaselineFunc::Polynomial;
  } else if (ftype0 == "cspline") {
    ftype = STBaselineFunc::CSpline;
  } else if (ftype0 == "sinusoid") {
    ftype = STBaselineFunc::Sinusoid;
  } else if (ftype0 == "chebyshev") {
    ftype = STBaselineFunc::Chebyshev;
  } else {
    throw(AipsError("invalid function type."));
  }
  ss.clear(); ss.str("");

  ss << res[2];
  ss >> fpar0;
  fpar = splitToIntList(fpar0, ',');
  ss.clear(); ss.str("");

  ss << res[3];
  ss >> masklist0;
  mask = getMaskFromMaskList(nchan(getIF(irow)), splitToIntList(masklist0, ','));
  ss.clear(); ss.str("");

  ss << res[4];
  ss >> thresClip;
  ss.clear(); ss.str("");

  ss << res[5];
  ss >> nIterClip;
  ss.clear(); ss.str("");

  ss << res[6];
  ss >> uself0;
  if (uself0 == "true") {
    useLineFinder = true;
  } else {
    useLineFinder = false;
  }
  ss.clear(); ss.str("");

  if (useLineFinder) {
    ss << res[7];
    ss >> thresLF;
    ss.clear(); ss.str("");

    ss << res[8];
    ss >> edge0;
    edgeLF = splitToIntList(edge0, ',');
    ss.clear(); ss.str("");

    ss << res[9];
    ss >> avgLF;
    ss.clear(); ss.str("");
  }

}

std::vector<int> Scantable::splitToIntList(const std::string& s, const char delim)
{
  istringstream iss(s);
  string tmp;
  int tmpi;
  std::vector<int> res;
  stringstream ss;
  while (getline(iss, tmp, delim)) {
    ss << tmp;
    ss >> tmpi;
    res.push_back(tmpi);
    ss.clear(); ss.str("");
  }

  return res;
}

std::vector<string> Scantable::splitToStringList(const std::string& s, const char delim)
{
  istringstream iss(s);
  std::string tmp;
  std::vector<string> res;
  while (getline(iss, tmp, delim)) {
    res.push_back(tmp);
  }

  return res;
}

std::vector<bool> Scantable::getMaskFromMaskList(const int nchan, const std::vector<int>& masklist)
{
  if (masklist.size() % 2 != 0) {
    throw(AipsError("masklist must have even number of elements."));
  }

  std::vector<bool> res(nchan);

  for (int i = 0; i < nchan; ++i) {
    res[i] = false;
  }
  for (uInt j = 0; j < masklist.size(); j += 2) {
    for (int i = masklist[j]; i <= min(nchan-1, masklist[j+1]); ++i) {
      res[i] = true;
    }
  }

  return res;
}

Vector<uInt> Scantable::getMaskListFromMask(const std::vector<bool>& mask)
{
  std::vector<int> masklist;
  masklist.clear();

  for (uInt i = 0; i < mask.size(); ++i) {
    if (mask[i]) {
      if ((i == 0)||(i == mask.size()-1)) {
	masklist.push_back(i);
      } else {
	if ((mask[i])&&(!mask[i-1])) {
	  masklist.push_back(i);
	}
	if ((mask[i])&&(!mask[i+1])) {
	  masklist.push_back(i);
	}
      }
    }
  }

  Vector<uInt> res(masklist.size());
  for (uInt i = 0; i < masklist.size(); ++i) {
    res[i] = (uInt)masklist[i];
  }

  return res;
}

void Scantable::initialiseBaselining(const std::string& blfile, 
				     ofstream& ofs, 
				     const bool outLogger, 
				     bool& outTextFile, 
				     bool& csvFormat, 
				     String& coordInfo, 
				     bool& hasSameNchan, 
				     const std::string& progressInfo, 
				     bool& showProgress, 
				     int& minNRow, 
				     Vector<Double>& timeSecCol)
{
  csvFormat = false;
  outTextFile = false;

  if (blfile != "") {
    csvFormat = (blfile.substr(0, 1) == "T");
    ofs.open(blfile.substr(1).c_str(), ios::out | ios::app);
    if (ofs) outTextFile = true;
  }

  coordInfo = "";
  hasSameNchan = true;

  if (outLogger || outTextFile) {
    coordInfo = getCoordInfo()[0];
    if (coordInfo == "") coordInfo = "channel";
    hasSameNchan = hasSameNchanOverIFs();
  }

  parseProgressInfo(progressInfo, showProgress, minNRow);

  ROScalarColumn<Double> tcol = ROScalarColumn<Double>(table_, "TIME");
  timeSecCol = tcol.getColumn();
}

void Scantable::finaliseBaselining(const bool outBaselineTable, 
				   STBaselineTable* pbt, 
				   const string& bltable, 
				   const bool outTextFile, 
				   ofstream& ofs)
{
  if (outBaselineTable) {
    pbt->save(bltable);
  }

  if (outTextFile) ofs.close();
}

void Scantable::initLineFinder(const std::vector<int>& edge, 
			       const float threshold, 
			       const int chanAvgLimit,
			       STLineFinder& lineFinder)
{
  if ((edge.size() > 2) && (edge.size() < getIFNos().size()*2)) {
    throw(AipsError("Length of edge element info is less than that of IFs"));
  }

  lineFinder.setOptions(threshold, 3, chanAvgLimit);
}

void Scantable::polyBaseline(const std::vector<bool>& mask, int order, 
			     float thresClip, int nIterClip, 
			     bool getResidual, 
			     const std::string& progressInfo, 
			     const bool outLogger, const std::string& blfile, 
			     const std::string& bltable)
{
  /****
  double TimeStart = mathutil::gettimeofday_sec();
  ****/

  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    modelReservoir = getPolynomialModelReservoir(order, 
						 &Scantable::getNormalPolynomial,
						 nChanNos);
    int nModel = modelReservoir.size();

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      chanMask = getCompositeChanMask(whichrow, mask);
      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doLeastSquareFitting(sp, chanMask, 
                                   modelReservoir[getIdxOfNchan(sp.size(), nChanNos)], 
                                   params, rms, finalChanMask, 
                                   nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Polynomial, order, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
	  		    coordInfo, hasSameNchan, ofs, "polyBaseline()", 
			    params, nClipped);
      } else {
        if (outBaselineTable) {
	  params.resize(nModel);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Polynomial, order, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        }
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }

    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }

  /****
  double TimeEnd = mathutil::gettimeofday_sec();
  double elapse1 = TimeEnd - TimeStart;
  std::cout << "poly-new   : " << elapse1 << " (sec.)" << endl;
  ****/
}

void Scantable::autoPolyBaseline(const std::vector<bool>& mask, int order, 
				 float thresClip, int nIterClip, 
				 const std::vector<int>& edge, 
				 float threshold, int chanAvgLimit, 
				 bool getResidual, 
				 const std::string& progressInfo, 
				 const bool outLogger, const std::string& blfile, 
				 const std::string& bltable)
{
  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;
    STLineFinder lineFinder = STLineFinder();

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    initLineFinder(edge, threshold, chanAvgLimit, lineFinder);

    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    modelReservoir = getPolynomialModelReservoir(order, 
						 &Scantable::getNormalPolynomial,
						 nChanNos);
    int nModel = modelReservoir.size();

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      std::vector<int> currentEdge;
      chanMask = getCompositeChanMask(whichrow, mask, edge, currentEdge, lineFinder);
      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doLeastSquareFitting(sp, chanMask, 
                                   modelReservoir[getIdxOfNchan(sp.size(), nChanNos)], 
                                   params, rms, finalChanMask,
                                   nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Polynomial, order, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
	  		    coordInfo, hasSameNchan, ofs, "autoPolyBaseline()", 
			    params, nClipped);
      } else {
        if (outBaselineTable) {
          params.resize(nModel);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Polynomial, order, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
	}
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }

    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }
}

void Scantable::chebyshevBaseline(const std::vector<bool>& mask, int order, 
				  float thresClip, int nIterClip, 
				  bool getResidual, 
				  const std::string& progressInfo, 
				  const bool outLogger, const std::string& blfile, 
				  const std::string& bltable)
{
  /*
  double TimeStart = mathutil::gettimeofday_sec();
  */

  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    modelReservoir = getPolynomialModelReservoir(order, 
						 &Scantable::getChebyshevPolynomial,
						 nChanNos);
    int nModel = modelReservoir.size();

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      chanMask = getCompositeChanMask(whichrow, mask);
      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doLeastSquareFitting(sp, chanMask, 
                                   modelReservoir[getIdxOfNchan(sp.size(), nChanNos)], 
                                   params, rms, finalChanMask, 
                                   nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Chebyshev, order, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
			    coordInfo, hasSameNchan, ofs, "chebyshevBaseline()", 
			    params, nClipped);
      } else {
        if (outBaselineTable) {
	  params.resize(nModel);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Chebyshev, order, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        }
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }
    
    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }

  /*
  double TimeEnd = mathutil::gettimeofday_sec();
  double elapse1 = TimeEnd - TimeStart;
  std::cout << "cheby   : " << elapse1 << " (sec.)" << endl;
  */
}

void Scantable::autoChebyshevBaseline(const std::vector<bool>& mask, int order, 
				      float thresClip, int nIterClip, 
				      const std::vector<int>& edge, 
				      float threshold, int chanAvgLimit, 
				      bool getResidual, 
				      const std::string& progressInfo, 
				      const bool outLogger, const std::string& blfile, 
				      const std::string& bltable)
{
  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;
    STLineFinder lineFinder = STLineFinder();

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    initLineFinder(edge, threshold, chanAvgLimit, lineFinder);

    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    modelReservoir = getPolynomialModelReservoir(order, 
						 &Scantable::getChebyshevPolynomial,
						 nChanNos);
    int nModel = modelReservoir.size();

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      std::vector<int> currentEdge;
      chanMask = getCompositeChanMask(whichrow, mask, edge, currentEdge, lineFinder);
      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doLeastSquareFitting(sp, chanMask, 
                                   modelReservoir[getIdxOfNchan(sp.size(), nChanNos)], 
                                   params, rms, finalChanMask, 
                                   nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Chebyshev, order, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
			    coordInfo, hasSameNchan, ofs, "autoChebyshevBaseline()", 
			    params, nClipped);
      } else {
        if (outBaselineTable) {
          params.resize(nModel);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Chebyshev, order, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
	}
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }

    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }
}

double Scantable::calculateModelSelectionCriteria(const std::string& valname, 
						  const std::string& blfunc, 
						  int order, 
						  const std::vector<bool>& inMask, 
						  int whichrow, 
						  bool useLineFinder, 
						  const std::vector<int>& edge, 
						  float threshold, 
						  int chanAvgLimit)
{
  std::vector<float> sp = getSpectrum(whichrow);
  std::vector<bool> chanMask;
  chanMask.clear();

  if (useLineFinder) {
    STLineFinder lineFinder = STLineFinder();
    initLineFinder(edge, threshold, chanAvgLimit, lineFinder);
    std::vector<int> currentEdge;
    chanMask = getCompositeChanMask(whichrow, inMask, edge, currentEdge, lineFinder);
  } else {
    chanMask = getCompositeChanMask(whichrow, inMask);
  }

  return doCalculateModelSelectionCriteria(valname, sp, chanMask, blfunc, order);
}

double Scantable::doCalculateModelSelectionCriteria(const std::string& valname, const std::vector<float>& spec, const std::vector<bool>& mask, const std::string& blfunc, int order)
{
  int nparam;
  std::vector<float> params;
  std::vector<bool> finalChanMask;
  float rms;
  int nClipped = 0;
  std::vector<float> res;
  if (blfunc == "poly") {
    nparam = order + 1;
    res = doPolynomialFitting(spec, mask, order, params, rms, finalChanMask, nClipped);
  } else if (blfunc == "chebyshev") {
    nparam = order + 1;
    res = doChebyshevFitting(spec, mask, order, params, rms, finalChanMask, nClipped);
  } else if (blfunc == "cspline") {
    std::vector<int> pieceEdges;//(order+1);  //order = npiece
    nparam = order + 3;
    res = doCubicSplineFitting(spec, mask, order, false, pieceEdges, params, rms, finalChanMask, nClipped);
  } else if (blfunc == "sinusoid") {
    std::vector<int> nWaves;
    nWaves.clear();
    for (int i = 0; i <= order; ++i) {
      nWaves.push_back(i);
    }
    nparam = 2*order + 1;  // order = nwave
    res = doSinusoidFitting(spec, mask, nWaves, params, rms, finalChanMask, nClipped);
  } else {
    throw(AipsError("blfunc must be poly, chebyshev, cspline or sinusoid."));
  }

  double msq = 0.0;
  int nusedchan = 0;
  int nChan = res.size();
  for (int i = 0; i < nChan; ++i) {
    if (mask[i]) {
      msq += (double)res[i]*(double)res[i];
      nusedchan++;
    }
  }
  if (nusedchan == 0) {
    throw(AipsError("all channels masked."));
  }
  msq /= (double)nusedchan;

  nparam++;  //add 1 for sigma of Gaussian distribution
  const double PI = 6.0 * asin(0.5); // PI (= 3.141592653...)

  if (valname.find("aic") == 0) {
    // Original Akaike Information Criterion (AIC)
    double aic = nusedchan * (log(2.0 * PI * msq) + 1.0) + 2.0 * nparam;

    // Corrected AIC by Sugiura(1978) (AICc)
    if (valname == "aicc") {
      if (nusedchan - nparam - 1 <= 0) {
	throw(AipsError("channel size is too small to calculate AICc."));
      }
      aic += 2.0*nparam*(nparam + 1)/(double)(nusedchan - nparam - 1);
    }

    return aic;

  } else if (valname == "bic") {
    // Bayesian Information Criterion (BIC)
    double bic = nusedchan * log(msq) + nparam * log((double)nusedchan);
    return bic;

  } else if (valname == "gcv") {
    // Generalised Cross Validation
    double x = 1.0 - (double)nparam / (double)nusedchan;
    double gcv = msq / (x * x);
    return gcv;

  } else {
    throw(AipsError("valname must be aic, aicc, bic or gcv."));
  }
}

double Scantable::getNormalPolynomial(int n, double x) {
  if (n == 0) {
    return 1.0;
  } else if (n > 0) {
    double res = 1.0;
    for (int i = 0; i < n; ++i) {
      res *= x;
    }
    return res;
  } else {
    if (x == 0.0) {
      throw(AipsError("infinity result: x=0 given for negative power."));
    } else {
      return pow(x, (double)n);
    }
  }
}

double Scantable::getChebyshevPolynomial(int n, double x) {
  if ((x < -1.0)||(x > 1.0)) {
    throw(AipsError("out of definition range (-1 <= x <= 1)."));
  } else if (x == 1.0) {
    return 1.0;
  } else if (x == 0.0) {
    double res;
    if (n%2 == 0) {
      if (n%4 == 0) {
	res = 1.0;
      } else {
	res = -1.0;
      }
    } else {
      res = 0.0;
    }
    return res;
  } else if (x == -1.0) {
    double res = (n%2 == 0 ? 1.0 : -1.0);
    return res;
  } else if (n < 0) {
    throw(AipsError("the order must be zero or positive."));
  } else if (n == 0) {
    return 1.0;
  } else if (n == 1) {
    return x;
  } else {
    double res[n+1];
    for (int i = 0; i < n+1; ++i) {
      double res0 = 0.0;
      if (i == 0) {
	res0 = 1.0;
      } else if (i == 1) {
	res0 = x;
      } else {
	res0 = 2.0 * x * res[i-1] - res[i-2];
      }
      res[i] = res0;
    }
    return res[n];
  }
}

std::vector<float> Scantable::doPolynomialFitting(const std::vector<float>& data, 
						  const std::vector<bool>& mask, 
						  int order, 
						  std::vector<float>& params, 
						  float& rms, 
						  std::vector<bool>& finalmask, 
						  float clipth, 
						  int clipn)
{
  int nClipped = 0;
  return doPolynomialFitting(data, mask, order, params, rms, finalmask, nClipped, clipth, clipn);
}

std::vector<float> Scantable::doPolynomialFitting(const std::vector<float>& data, 
						  const std::vector<bool>& mask, 
						  int order, 
						  std::vector<float>& params, 
						  float& rms, 
						  std::vector<bool>& finalMask, 
						  int& nClipped, 
						  float thresClip, 
						  int nIterClip, 
						  bool getResidual)
{
  return doLeastSquareFitting(data, mask, 
			      getPolynomialModel(order, data.size(), &Scantable::getNormalPolynomial), 
			      params, rms, finalMask, 
			      nClipped, thresClip, nIterClip, 
			      getResidual);
}

std::vector<float> Scantable::doChebyshevFitting(const std::vector<float>& data, 
						 const std::vector<bool>& mask, 
						 int order, 
						 std::vector<float>& params, 
						 float& rms, 
						 std::vector<bool>& finalmask, 
						 float clipth, 
						 int clipn)
{
  int nClipped = 0;
  return doChebyshevFitting(data, mask, order, params, rms, finalmask, nClipped, clipth, clipn);
}

std::vector<float> Scantable::doChebyshevFitting(const std::vector<float>& data, 
						 const std::vector<bool>& mask, 
						 int order, 
						 std::vector<float>& params, 
						 float& rms, 
						 std::vector<bool>& finalMask, 
						 int& nClipped, 
						 float thresClip, 
						 int nIterClip, 
						 bool getResidual)
{
  return doLeastSquareFitting(data, mask, 
			      getPolynomialModel(order, data.size(), &Scantable::getChebyshevPolynomial), 
			      params, rms, finalMask, 
			      nClipped, thresClip, nIterClip, 
			      getResidual);
}

std::vector<std::vector<double> > Scantable::getPolynomialModel(int order, int nchan, double (Scantable::*pfunc)(int, double))
{
  // model  : contains model values for computing the least-square matrix. 
  //          model.size() is nmodel and model[*].size() is nchan. 
  //          Each model element are as follows:
  //
  //          (for normal polynomials)
  //          model[0]   = {1.0,   1.0,   1.0,   ..., 1.0},
  //          model[1]   = {0.0,   1.0,   2.0,   ..., (nchan-1)}
  //          model[n-1] = ...,
  //          model[n]   = {0.0^n, 1.0^n, 2.0^n, ..., (nchan-1)^n}
  //          where (0 <= n <= order)
  //
  //          (for Chebyshev polynomials)
  //          model[0]   = {T0(-1), T0(2/(nchan-1)-1), T0(4/(nchan-1)-1), ..., T0(1)}, 
  //          model[n-1] = ...,
  //          model[n]   = {Tn(-1), Tn(2/(nchan-1)-1), Tn(4/(nchan-1)-1), ..., Tn(1)}
  //          where (0 <= n <= order),

  int nmodel = order + 1;
  std::vector<std::vector<double> > model(nmodel, std::vector<double>(nchan));

  double stretch, shift;
  if (pfunc == &Scantable::getChebyshevPolynomial) {
    stretch = 2.0/(double)(nchan - 1);
    shift   = -1.0;
  } else {
    stretch = 1.0;
    shift   = 0.0;
  }

  for (int i = 0; i < nmodel; ++i) {
    for (int j = 0; j < nchan; ++j) {
      model[i][j] = (this->*pfunc)(i, stretch*(double)j + shift);
    }
  }

  return model;
}

std::vector<std::vector<std::vector<double> > > Scantable::getPolynomialModelReservoir(int order,
										       double (Scantable::*pfunc)(int, double),
										       std::vector<int>& nChanNos)
{
  std::vector<std::vector<std::vector<double> > > res;
  res.clear();
  nChanNos.clear();

  std::vector<uint> ifNos = getIFNos();
  for (uint i = 0; i < ifNos.size(); ++i) {
    int currNchan = nchan(ifNos[i]);
    bool hasDifferentNchan = (i == 0);
    for (uint j = 0; j < i; ++j) {
      if (currNchan != nchan(ifNos[j])) {
	hasDifferentNchan = true;
	break;
      }
    }
    if (hasDifferentNchan) {
      res.push_back(getPolynomialModel(order, currNchan, pfunc));
      nChanNos.push_back(currNchan);
    }
  }

  return res;
}

std::vector<float> Scantable::doLeastSquareFitting(const std::vector<float>& data, 
						   const std::vector<bool>& mask, 
						   const std::vector<std::vector<double> >& model, 
						   std::vector<float>& params, 
						   float& rms, 
						   std::vector<bool>& finalMask, 
						   int& nClipped, 
						   float thresClip, 
						   int nIterClip, 
						   bool getResidual)
{
  int nDOF = model.size();
  int nChan = data.size();

  if (nDOF == 0) {
    throw(AipsError("no model data given"));
  }
  if (nChan < 2) {
    throw(AipsError("data size is too few"));
  }
  if (nChan != (int)mask.size()) {
    throw(AipsError("data and mask sizes are not identical"));
  }
  for (int i = 0; i < nDOF; ++i) {
    if (nChan != (int)model[i].size()) {
      throw(AipsError("data and model sizes are not identical"));
    }
  }

  params.clear();
  params.resize(nDOF);

  finalMask.clear();
  finalMask.resize(nChan);

  std::vector<int> maskArray(nChan);
  int j = 0;
  for (int i = 0; i < nChan; ++i) {
    maskArray[i] = mask[i] ? 1 : 0;
    if (isnan(data[i])) maskArray[i] = 0;
    if (isinf(data[i])) maskArray[i] = 0;

    finalMask[i] = (maskArray[i] == 1);
    if (finalMask[i]) {
      j++;
    }

    /*
    maskArray[i] = mask[i] ? 1 : 0;
    if (mask[i]) {
      j++;
    }
    finalMask[i] = mask[i];
    */
  }

  int initNData = j;
  int nData = initNData;

  std::vector<double> z1(nChan), r1(nChan), residual(nChan);
  for (int i = 0; i < nChan; ++i) {
    z1[i] = (double)data[i];
    r1[i] = 0.0;
    residual[i] = 0.0;
  }

  for (int nClip = 0; nClip < nIterClip+1; ++nClip) {
    // xMatrix : horizontal concatenation of 
    //           the least-sq. matrix (left) and an 
    //           identity matrix (right).
    // the right part is used to calculate the inverse matrix of the left part. 
    double xMatrix[nDOF][2*nDOF];
    double zMatrix[nDOF];
    for (int i = 0; i < nDOF; ++i) {
      for (int j = 0; j < 2*nDOF; ++j) {
	xMatrix[i][j] = 0.0;
      }
      xMatrix[i][nDOF+i] = 1.0;
      zMatrix[i] = 0.0;
    }

    int nUseData = 0;
    for (int k = 0; k < nChan; ++k) {
      if (maskArray[k] == 0) continue;

      for (int i = 0; i < nDOF; ++i) {
	for (int j = i; j < nDOF; ++j) {
	  xMatrix[i][j] += model[i][k] * model[j][k];
	}
	zMatrix[i] += z1[k] * model[i][k];
      }

      nUseData++;
    }

    if (nUseData < 1) {
	throw(AipsError("all channels clipped or masked. can't execute fitting anymore."));      
    }

    for (int i = 0; i < nDOF; ++i) {
      for (int j = 0; j < i; ++j) {
	xMatrix[i][j] = xMatrix[j][i];
      }
    }

    //compute inverse matrix of the left half of xMatrix
    std::vector<double> invDiag(nDOF);
    for (int i = 0; i < nDOF; ++i) {
      invDiag[i] = 1.0 / xMatrix[i][i];
      for (int j = 0; j < nDOF; ++j) {
	xMatrix[i][j] *= invDiag[i];
      }
    }

    for (int k = 0; k < nDOF; ++k) {
      for (int i = 0; i < nDOF; ++i) {
	if (i != k) {
	  double factor1 = xMatrix[k][k];
	  double invfactor1 = 1.0 / factor1;
	  double factor2 = xMatrix[i][k];
	  for (int j = k; j < 2*nDOF; ++j) {
	    xMatrix[i][j] *= factor1;
	    xMatrix[i][j] -= xMatrix[k][j]*factor2;
	    xMatrix[i][j] *= invfactor1;
	  }
	}
      }
      double invXDiag = 1.0 / xMatrix[k][k];
      for (int j = k; j < 2*nDOF; ++j) {
	xMatrix[k][j] *= invXDiag;
      }
    }
    
    for (int i = 0; i < nDOF; ++i) {
      for (int j = 0; j < nDOF; ++j) {
	xMatrix[i][nDOF+j] *= invDiag[j];
      }
    }
    //compute a vector y in which coefficients of the best-fit 
    //model functions are stored.
    //in case of polynomials, y consists of (a0,a1,a2,...) 
    //where ai is the coefficient of the term x^i.
    //in case of sinusoids, y consists of (a0,s1,c1,s2,c2,...)
    //where a0 is constant term and s* and c* are of sine 
    //and cosine functions, respectively. 
    std::vector<double> y(nDOF);
    for (int i = 0; i < nDOF; ++i) {
      y[i] = 0.0;
      for (int j = 0; j < nDOF; ++j) {
	y[i] += xMatrix[i][nDOF+j]*zMatrix[j];
      }
      params[i] = (float)y[i];
    }

    for (int i = 0; i < nChan; ++i) {
      r1[i] = y[0];
      for (int j = 1; j < nDOF; ++j) {
	r1[i] += y[j]*model[j][i];
      }
      residual[i] = z1[i] - r1[i];
    }

    double stdDev = 0.0;
    for (int i = 0; i < nChan; ++i) {
      if (maskArray[i] == 0) continue;
      stdDev += residual[i]*residual[i];
    }
    stdDev = sqrt(stdDev/(double)nData);
    rms = (float)stdDev;

    if ((nClip == nIterClip) || (thresClip <= 0.0)) {
      break;
    } else {

      double thres = stdDev * thresClip;
      int newNData = 0;
      for (int i = 0; i < nChan; ++i) {
	if (abs(residual[i]) >= thres) {
	  maskArray[i] = 0;
          finalMask[i] = false;
	}
	if (maskArray[i] > 0) {
	  newNData++;
	}
      }
      if (newNData == nData) {
	break; //no more flag to add. stop iteration.
      } else {
	nData = newNData;
      }

    }
  }

  nClipped = initNData - nData;

  std::vector<float> result(nChan);
  if (getResidual) {
    for (int i = 0; i < nChan; ++i) {
      result[i] = (float)residual[i];
    }
  } else {
    for (int i = 0; i < nChan; ++i) {
      result[i] = (float)r1[i];
    }
  }

  return result;
} //xMatrix

void Scantable::cubicSplineBaseline(const std::vector<bool>& mask, int nPiece, 
				    float thresClip, int nIterClip, 
				    bool getResidual, 
				    const std::string& progressInfo, 
				    const bool outLogger, const std::string& blfile, 
				    const std::string& bltable)
{
  /****
  double TimeStart = mathutil::gettimeofday_sec();
  ****/

  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    modelReservoir = getPolynomialModelReservoir(3, 
						 &Scantable::getNormalPolynomial,
						 nChanNos);
    int nDOF = nPiece + 3;

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      chanMask = getCompositeChanMask(whichrow, mask);
      std::vector<int> pieceEdges;
      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doCubicSplineLeastSquareFitting(sp, chanMask,
                modelReservoir[getIdxOfNchan(sp.size(), nChanNos)], 
                nPiece, false, pieceEdges, params, rms, finalChanMask, 
                nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::CSpline, pieceEdges, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
			    coordInfo, hasSameNchan, ofs, "cubicSplineBaseline()", 
			    pieceEdges, params, nClipped);
      } else {
        if (outBaselineTable) {
          pieceEdges.resize(nPiece+1);
          for (uInt i = 0; i < pieceEdges.size(); ++i) {
            pieceEdges[i] = 0;
	  }
	  params.resize(nDOF);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::CSpline, pieceEdges, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        }
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }
    
    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }

  /****
  double TimeEnd = mathutil::gettimeofday_sec();
  double elapse1 = TimeEnd - TimeStart;
  std::cout << "cspline-new   : " << elapse1 << " (sec.)" << endl;
  ****/
}

void Scantable::autoCubicSplineBaseline(const std::vector<bool>& mask, int nPiece, 
					float thresClip, int nIterClip, 
					const std::vector<int>& edge, 
					float threshold, int chanAvgLimit, 
					bool getResidual, 
					const std::string& progressInfo, 
					const bool outLogger, const std::string& blfile, 
					const std::string& bltable)
{
  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;
    STLineFinder lineFinder = STLineFinder();

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    initLineFinder(edge, threshold, chanAvgLimit, lineFinder);

    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    modelReservoir = getPolynomialModelReservoir(3, 
						 &Scantable::getNormalPolynomial,
						 nChanNos);
    int nDOF = nPiece + 3;

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      std::vector<int> currentEdge;
      chanMask = getCompositeChanMask(whichrow, mask, edge, currentEdge, lineFinder);
      std::vector<int> pieceEdges;
      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doCubicSplineLeastSquareFitting(sp, chanMask, 
                modelReservoir[getIdxOfNchan(sp.size(), nChanNos)], 
                nPiece, false, pieceEdges, params, rms, finalChanMask, 
                nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::CSpline, pieceEdges, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
			    coordInfo, hasSameNchan, ofs, "autoCubicSplineBaseline()", 
			    pieceEdges, params, nClipped);
      } else {
        if (outBaselineTable) {
          pieceEdges.resize(nPiece+1);
          for (uInt i = 0; i < pieceEdges.size(); ++i) {
            pieceEdges[i] = 0;
	  }
          params.resize(nDOF);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::CSpline, pieceEdges, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
	}
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }

    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }
}

std::vector<float> Scantable::doCubicSplineFitting(const std::vector<float>& data, 
						   const std::vector<bool>& mask, 
						   std::vector<int>& idxEdge, 
						   std::vector<float>& params, 
						   float& rms, 
						   std::vector<bool>& finalmask, 
						   float clipth, 
						   int clipn)
{
  int nClipped = 0;
  return doCubicSplineFitting(data, mask, idxEdge.size()-1, true, idxEdge, params, rms, finalmask, nClipped, clipth, clipn);
}

std::vector<float> Scantable::doCubicSplineFitting(const std::vector<float>& data, 
						   const std::vector<bool>& mask, 
						   int nPiece, 
						   std::vector<int>& idxEdge, 
						   std::vector<float>& params, 
						   float& rms, 
						   std::vector<bool>& finalmask, 
						   float clipth, 
						   int clipn)
{
  int nClipped = 0;
  return doCubicSplineFitting(data, mask, nPiece, false, idxEdge, params, rms, finalmask, nClipped, clipth, clipn);
}

std::vector<float> Scantable::doCubicSplineFitting(const std::vector<float>& data, 
						   const std::vector<bool>& mask, 
						   int nPiece, 
						   bool useGivenPieceBoundary, 
						   std::vector<int>& idxEdge, 
						   std::vector<float>& params, 
						   float& rms, 
						   std::vector<bool>& finalMask, 
						   int& nClipped, 
						   float thresClip, 
						   int nIterClip, 
						   bool getResidual)
{
  return doCubicSplineLeastSquareFitting(data, mask, 
					 getPolynomialModel(3, data.size(), &Scantable::getNormalPolynomial), 
					 nPiece, useGivenPieceBoundary, idxEdge, 
					 params, rms, finalMask, 
					 nClipped, thresClip, nIterClip, 
					 getResidual);
}

std::vector<float> Scantable::doCubicSplineLeastSquareFitting(const std::vector<float>& data, 
							      const std::vector<bool>& mask, 
							      const std::vector<std::vector<double> >& model, 
							      int nPiece, 
							      bool useGivenPieceBoundary, 
							      std::vector<int>& idxEdge, 
							      std::vector<float>& params, 
							      float& rms, 
							      std::vector<bool>& finalMask, 
							      int& nClipped, 
							      float thresClip, 
							      int nIterClip, 
							      bool getResidual)
{
  int nDOF = nPiece + 3;  //number of independent parameters to solve, namely, 4+(nPiece-1).
  int nModel = model.size();
  int nChan = data.size();

  if (nModel != 4) {
    throw(AipsError("model size must be 4."));
  }
  if (nPiece < 1) {
    throw(AipsError("number of the sections must be one or more"));
  }
  if (nChan < 2*nPiece) {
    throw(AipsError("data size is too few"));
  }
  if (nChan != (int)mask.size()) {
    throw(AipsError("data and mask sizes are not identical"));
  }
  for (int i = 0; i < nModel; ++i) {
    if (nChan != (int)model[i].size()) {
      throw(AipsError("data and model sizes are not identical"));
    }
  }

  params.clear();
  params.resize(nPiece*nModel);

  finalMask.clear();
  finalMask.resize(nChan);

  std::vector<int> maskArray(nChan);
  std::vector<int> x(nChan);
  int j = 0;
  for (int i = 0; i < nChan; ++i) {
    maskArray[i] = mask[i] ? 1 : 0;
    if (isnan(data[i])) maskArray[i] = 0;
    if (isinf(data[i])) maskArray[i] = 0;

    finalMask[i] = (maskArray[i] == 1);
    if (finalMask[i]) {
      x[j] = i;
      j++;
    }

    /*
    maskArray[i] = mask[i] ? 1 : 0;
    if (mask[i]) {
      x[j] = i;
      j++;
    }
    finalMask[i] = mask[i];
    */
  }

  int initNData = j;
  int nData = initNData;

  if (initNData < nPiece) {
    throw(AipsError("too few non-flagged channels"));
  }

  int nElement = (int)(floor(floor((double)(initNData/nPiece))+0.5));
  std::vector<double> invEdge(nPiece-1);

  if (useGivenPieceBoundary) {
    if ((int)idxEdge.size() != nPiece+1) {
      throw(AipsError("pieceEdge.size() must be equal to nPiece+1."));
    }
  } else {
    idxEdge.clear();
    idxEdge.resize(nPiece+1);
    idxEdge[0] = x[0];
  }
  for (int i = 1; i < nPiece; ++i) {
    int valX = x[nElement*i];
    if (!useGivenPieceBoundary) {
      idxEdge[i] = valX;
    }
    invEdge[i-1] = 1.0/(double)valX;
  }
  if (!useGivenPieceBoundary) {
    idxEdge[nPiece] = x[initNData-1]+1;
  }

  std::vector<double> z1(nChan), r1(nChan), residual(nChan);
  for (int i = 0; i < nChan; ++i) {
    z1[i] = (double)data[i];
    r1[i] = 0.0;
    residual[i] = 0.0;
  }

  for (int nClip = 0; nClip < nIterClip+1; ++nClip) {
    // xMatrix : horizontal concatenation of 
    //           the least-sq. matrix (left) and an 
    //           identity matrix (right).
    // the right part is used to calculate the inverse matrix of the left part. 

    double xMatrix[nDOF][2*nDOF];
    double zMatrix[nDOF];
    for (int i = 0; i < nDOF; ++i) {
      for (int j = 0; j < 2*nDOF; ++j) {
	xMatrix[i][j] = 0.0;
      }
      xMatrix[i][nDOF+i] = 1.0;
      zMatrix[i] = 0.0;
    }

    for (int n = 0; n < nPiece; ++n) {
      int nUseDataInPiece = 0;
      for (int k = idxEdge[n]; k < idxEdge[n+1]; ++k) {

	if (maskArray[k] == 0) continue;

	for (int i = 0; i < nModel; ++i) {
	  for (int j = i; j < nModel; ++j) {
	    xMatrix[i][j] += model[i][k] * model[j][k];
	  }
	  zMatrix[i] += z1[k] * model[i][k];
	}

	for (int i = 0; i < n; ++i) {
	  double q = 1.0 - model[1][k]*invEdge[i];
	  q = q*q*q;
	  for (int j = 0; j < nModel; ++j) {
	    xMatrix[j][i+nModel] += q * model[j][k];
	  }
	  for (int j = 0; j < i; ++j) {
	    double r = 1.0 - model[1][k]*invEdge[j];
	    r = r*r*r;
	    xMatrix[j+nModel][i+nModel] += r*q;
	  }
	  xMatrix[i+nModel][i+nModel] += q*q;
	  zMatrix[i+nModel] += q*z1[k];
	}

	nUseDataInPiece++;
      }

      if (nUseDataInPiece < 1) {
	std::vector<string> suffixOfPieceNumber(4);
	suffixOfPieceNumber[0] = "th";
	suffixOfPieceNumber[1] = "st";
	suffixOfPieceNumber[2] = "nd";
	suffixOfPieceNumber[3] = "rd";
	int idxNoDataPiece = (n % 10 <= 3) ? n : 0;
	ostringstream oss;
	oss << "all channels clipped or masked in " << n << suffixOfPieceNumber[idxNoDataPiece];
	oss << " piece of the spectrum. can't execute fitting anymore.";
	throw(AipsError(String(oss)));
      }
    }

    for (int i = 0; i < nDOF; ++i) {
      for (int j = 0; j < i; ++j) {
	xMatrix[i][j] = xMatrix[j][i];
      }
    }

    std::vector<double> invDiag(nDOF);
    for (int i = 0; i < nDOF; ++i) {
      invDiag[i] = 1.0 / xMatrix[i][i];
      for (int j = 0; j < nDOF; ++j) {
	xMatrix[i][j] *= invDiag[i];
      }
    }

    for (int k = 0; k < nDOF; ++k) {
      for (int i = 0; i < nDOF; ++i) {
	if (i != k) {
	  double factor1 = xMatrix[k][k];
	  double invfactor1 = 1.0 / factor1;
	  double factor2 = xMatrix[i][k];
	  for (int j = k; j < 2*nDOF; ++j) {
	    xMatrix[i][j] *= factor1;
	    xMatrix[i][j] -= xMatrix[k][j]*factor2;
	    xMatrix[i][j] *= invfactor1;
	  }
	}
      }
      double invXDiag = 1.0 / xMatrix[k][k];
      for (int j = k; j < 2*nDOF; ++j) {
	xMatrix[k][j] *= invXDiag;
      }
    }
    
    for (int i = 0; i < nDOF; ++i) {
      for (int j = 0; j < nDOF; ++j) {
	xMatrix[i][nDOF+j] *= invDiag[j];
      }
    }

    //compute a vector y which consists of the coefficients of the best-fit spline curves 
    //(a0,a1,a2,a3(,b3,c3,...)), namely, the ones for the leftmost piece and the ones of 
    //cubic terms for the other pieces (in case nPiece>1).
    std::vector<double> y(nDOF);
    for (int i = 0; i < nDOF; ++i) {
      y[i] = 0.0;
      for (int j = 0; j < nDOF; ++j) {
	y[i] += xMatrix[i][nDOF+j]*zMatrix[j];
      }
    }

    std::vector<double> a(nModel);
    for (int i = 0; i < nModel; ++i) {
      a[i] = y[i];
    }

    int j = 0;
    for (int n = 0; n < nPiece; ++n) {
      for (int i = idxEdge[n]; i < idxEdge[n+1]; ++i) {
	r1[i] = 0.0;
	for (int j = 0; j < nModel; ++j) {
	  r1[i] += a[j] * model[j][i];
	}
      }
      for (int i = 0; i < nModel; ++i) {
	params[j+i] = a[i];
      }
      j += nModel;

      if (n == nPiece-1) break;

      double d = y[n+nModel];
      double iE = invEdge[n];
      a[0] +=       d;
      a[1] -= 3.0 * d * iE;
      a[2] += 3.0 * d * iE * iE;
      a[3] -=       d * iE * iE * iE;
    }

    //subtract constant value for masked regions at the edge of spectrum
    if (idxEdge[0] > 0) {
      int n = idxEdge[0];
      for (int i = 0; i < idxEdge[0]; ++i) {
	//--cubic extrapolate--
	//r1[i] = params[0] + params[1]*x1[i] + params[2]*x2[i] + params[3]*x3[i];
	//--linear extrapolate--
	//r1[i] = (r1[n+1] - r1[n])/(x1[n+1] - x1[n])*(x1[i] - x1[n]) + r1[n];
	//--constant--
	r1[i] = r1[n];
      }
    }

    if (idxEdge[nPiece] < nChan) {
      int n = idxEdge[nPiece]-1;
      for (int i = idxEdge[nPiece]; i < nChan; ++i) {
	//--cubic extrapolate--
	//int m = 4*(nPiece-1);
	//r1[i] = params[m] + params[m+1]*x1[i] + params[m+2]*x2[i] + params[m+3]*x3[i];
	//--linear extrapolate--
	//r1[i] = (r1[n-1] - r1[n])/(x1[n-1] - x1[n])*(x1[i] - x1[n]) + r1[n];
	//--constant--
	r1[i] = r1[n];
      }
    }

    for (int i = 0; i < nChan; ++i) {
      residual[i] = z1[i] - r1[i];
    }

    double stdDev = 0.0;
    for (int i = 0; i < nChan; ++i) {
      if (maskArray[i] == 0) continue;
      stdDev += residual[i]*residual[i];
    }
    stdDev = sqrt(stdDev/(double)nData);
    rms = (float)stdDev;

    if ((nClip == nIterClip) || (thresClip <= 0.0)) {
      break;
    } else {
      
      double thres = stdDev * thresClip;
      int newNData = 0;
      for (int i = 0; i < nChan; ++i) {
	if (abs(residual[i]) >= thres) {
	  maskArray[i] = 0;
	  finalMask[i] = false;
	}
	if (maskArray[i] > 0) {
	  newNData++;
	}
      }
      if (newNData == nData) {
	break; //no more flag to add. iteration stops.
      } else {
	nData = newNData;
      }

    }
  }

  nClipped = initNData - nData;

  std::vector<float> result(nChan);
  if (getResidual) {
    for (int i = 0; i < nChan; ++i) {
      result[i] = (float)residual[i];
    }
  } else {
    for (int i = 0; i < nChan; ++i) {
      result[i] = (float)r1[i];
    }
  }

  return result;
}

std::vector<int> Scantable::selectWaveNumbers(const std::vector<int>& addNWaves, 
				  const std::vector<int>& rejectNWaves)
{
  std::vector<bool> chanMask;
  std::string fftMethod;
  std::string fftThresh;

  return selectWaveNumbers(0, chanMask, false, fftMethod, fftThresh, addNWaves, rejectNWaves);
}

std::vector<int> Scantable::selectWaveNumbers(const int whichrow, 
				  const std::vector<bool>& chanMask, 
				  const bool applyFFT, 
				  const std::string& fftMethod, 
				  const std::string& fftThresh, 
				  const std::vector<int>& addNWaves, 
				  const std::vector<int>& rejectNWaves)
{
  std::vector<int> nWaves;
  nWaves.clear();

  if (applyFFT) {
    string fftThAttr;
    float fftThSigma;
    int fftThTop;
    parseFFTThresholdInfo(fftThresh, fftThAttr, fftThSigma, fftThTop);
    doSelectWaveNumbers(whichrow, chanMask, fftMethod, fftThSigma, fftThTop, fftThAttr, nWaves);
  }

  addAuxWaveNumbers(whichrow, addNWaves, rejectNWaves, nWaves);

  return nWaves;
}

int Scantable::getIdxOfNchan(const int nChan, const std::vector<int>& nChanNos)
{
  int idx = -1;
  for (uint i = 0; i < nChanNos.size(); ++i) {
    if (nChan == nChanNos[i]) {
      idx = i;
      break;
    }
  }

  if (idx < 0) {
    throw(AipsError("nChan not found in nChhanNos."));
  }

  return idx;
}

void Scantable::parseFFTInfo(const std::string& fftInfo, bool& applyFFT, std::string& fftMethod, std::string& fftThresh)
{
  istringstream iss(fftInfo);
  std::string tmp;
  std::vector<string> res;
  while (getline(iss, tmp, ',')) {
    res.push_back(tmp);
  }
  if (res.size() < 3) {
    throw(AipsError("wrong value in 'fftinfo' parameter")) ;
  }
  applyFFT = (res[0] == "true");
  fftMethod = res[1];
  fftThresh = res[2];
}

void Scantable::parseFFTThresholdInfo(const std::string& fftThresh, std::string& fftThAttr, float& fftThSigma, int& fftThTop)
{
  uInt idxSigma = fftThresh.find("sigma");
  uInt idxTop   = fftThresh.find("top");

  if (idxSigma == fftThresh.size() - 5) {
    std::istringstream is(fftThresh.substr(0, fftThresh.size() - 5));
    is >> fftThSigma;
    fftThAttr = "sigma";
  } else if (idxTop == 0) {
    std::istringstream is(fftThresh.substr(3));
    is >> fftThTop;
    fftThAttr = "top";
  } else {
    bool isNumber = true;
    for (uInt i = 0; i < fftThresh.size()-1; ++i) {
      char ch = (fftThresh.substr(i, 1).c_str())[0];
      if (!(isdigit(ch) || (fftThresh.substr(i, 1) == "."))) {
	isNumber = false;
	break;
      }
    }
    if (isNumber) {
      std::istringstream is(fftThresh);
      is >> fftThSigma;
      fftThAttr = "sigma";
    } else {
      throw(AipsError("fftthresh has a wrong value"));
    }
  }
}

void Scantable::doSelectWaveNumbers(const int whichrow, const std::vector<bool>& chanMask, const std::string& fftMethod, const float fftThSigma, const int fftThTop, const std::string& fftThAttr, std::vector<int>& nWaves)
{
  std::vector<float> fspec;
  if (fftMethod == "fft") {
    fspec = execFFT(whichrow, chanMask, false, true);
  //} else if (fftMethod == "lsp") {
  //  fspec = lombScarglePeriodogram(whichrow);
  }

  if (fftThAttr == "sigma") {
    float mean  = 0.0;
    float mean2 = 0.0;
    for (uInt i = 0; i < fspec.size(); ++i) {
      mean  += fspec[i];
      mean2 += fspec[i]*fspec[i];
    }
    mean  /= float(fspec.size());
    mean2 /= float(fspec.size());
    float thres = mean + fftThSigma * float(sqrt(mean2 - mean*mean));

    for (uInt i = 0; i < fspec.size(); ++i) {
      if (fspec[i] >= thres) {
	nWaves.push_back(i);
      }
    }

  } else if (fftThAttr == "top") {
    for (int i = 0; i < fftThTop; ++i) {
      float max = 0.0;
      int maxIdx = 0;
      for (uInt j = 0; j < fspec.size(); ++j) {
	if (fspec[j] > max) {
	  max = fspec[j];
	  maxIdx = j;
	}
      }
      nWaves.push_back(maxIdx);
      fspec[maxIdx] = 0.0;
    }

  }

  if (nWaves.size() > 1) {
    sort(nWaves.begin(), nWaves.end());
  }
}

void Scantable::addAuxWaveNumbers(const int whichrow, const std::vector<int>& addNWaves, const std::vector<int>& rejectNWaves, std::vector<int>& nWaves)
{
  std::vector<int> tempAddNWaves, tempRejectNWaves;
  tempAddNWaves.clear();
  tempRejectNWaves.clear();

  for (uInt i = 0; i < addNWaves.size(); ++i) {
    tempAddNWaves.push_back(addNWaves[i]);
  }
  if ((tempAddNWaves.size() == 2) && (tempAddNWaves[1] == -999)) {
    setWaveNumberListUptoNyquistFreq(whichrow, tempAddNWaves);
  }

  for (uInt i = 0; i < rejectNWaves.size(); ++i) {
    tempRejectNWaves.push_back(rejectNWaves[i]);
  }
  if ((tempRejectNWaves.size() == 2) && (tempRejectNWaves[1] == -999)) {
    setWaveNumberListUptoNyquistFreq(whichrow, tempRejectNWaves);
  }

  for (uInt i = 0; i < tempAddNWaves.size(); ++i) {
    bool found = false;
    for (uInt j = 0; j < nWaves.size(); ++j) {
      if (nWaves[j] == tempAddNWaves[i]) {
	found = true;
	break;
      }
    }
    if (!found) nWaves.push_back(tempAddNWaves[i]);
  }

  for (uInt i = 0; i < tempRejectNWaves.size(); ++i) {
    for (std::vector<int>::iterator j = nWaves.begin(); j != nWaves.end(); ) {
      if (*j == tempRejectNWaves[i]) {
	j = nWaves.erase(j);
      } else {
	++j;
      }
    }
  }

  if (nWaves.size() > 1) {
    sort(nWaves.begin(), nWaves.end());
    unique(nWaves.begin(), nWaves.end());
  }
}

void Scantable::setWaveNumberListUptoNyquistFreq(const int whichrow, std::vector<int>& nWaves)
{
  int val = nWaves[0];
  int nyquistFreq = nchan(getIF(whichrow))/2+1;
  nWaves.clear();
  if (val > nyquistFreq) {  // for safety, at least nWaves contains a constant; CAS-3759
    nWaves.push_back(0);
  }
  while (val <= nyquistFreq) {
    nWaves.push_back(val);
    val++;
  }
}

void Scantable::sinusoidBaseline(const std::vector<bool>& mask, const std::string& fftInfo, 
				 const std::vector<int>& addNWaves, 
				 const std::vector<int>& rejectNWaves, 
				 float thresClip, int nIterClip, 
				 bool getResidual, 
				 const std::string& progressInfo, 
				 const bool outLogger, const std::string& blfile, 
				 const std::string& bltable)
{
  /****
  double TimeStart = mathutil::gettimeofday_sec();
  ****/

  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    bool applyFFT;
    std::string fftMethod, fftThresh;
    parseFFTInfo(fftInfo, applyFFT, fftMethod, fftThresh);

    std::vector<int> nWaves;
    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    if (!applyFFT) {
      nWaves = selectWaveNumbers(addNWaves, rejectNWaves);
      modelReservoir = getSinusoidModelReservoir(nWaves, nChanNos);
    }

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      chanMask = getCompositeChanMask(whichrow, mask);
      std::vector<std::vector<double> > model;
      if (applyFFT) {
	nWaves = selectWaveNumbers(whichrow, chanMask, true, fftMethod, fftThresh, 
				   addNWaves, rejectNWaves);
	model = getSinusoidModel(nWaves, sp.size());
      } else {
	model = modelReservoir[getIdxOfNchan(sp.size(), nChanNos)];
      }
      int nModel = modelReservoir.size();

      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doLeastSquareFitting(sp, chanMask, model, 
                                   params, rms, finalChanMask, 
                                   nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Sinusoid, nWaves, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
			    coordInfo, hasSameNchan, ofs, "sinusoidBaseline()", 
			    params, nClipped);
      } else {
        if (outBaselineTable) {
	  params.resize(nModel);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Sinusoid, nWaves, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, 0.0, 0, std::vector<int>());
        }
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }

    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }

  /****
  double TimeEnd = mathutil::gettimeofday_sec();
  double elapse1 = TimeEnd - TimeStart;
  std::cout << "sinusoid-old   : " << elapse1 << " (sec.)" << endl;
  ****/
}

void Scantable::autoSinusoidBaseline(const std::vector<bool>& mask, const std::string& fftInfo, 
				     const std::vector<int>& addNWaves, 
				     const std::vector<int>& rejectNWaves, 
				     float thresClip, int nIterClip, 
				     const std::vector<int>& edge, 
				     float threshold, int chanAvgLimit, 
				     bool getResidual, 
				     const std::string& progressInfo, 
				     const bool outLogger, const std::string& blfile, 
				     const std::string& bltable)
{
  try {
    ofstream ofs;
    String coordInfo;
    bool hasSameNchan, outTextFile, csvFormat, showProgress;
    int minNRow;
    int nRow = nrow();
    std::vector<bool> chanMask, finalChanMask;
    float rms;
    bool outBaselineTable = (bltable != "");
    STBaselineTable bt = STBaselineTable(*this);
    Vector<Double> timeSecCol;
    STLineFinder lineFinder = STLineFinder();

    initialiseBaselining(blfile, ofs, outLogger, outTextFile, csvFormat, 
			 coordInfo, hasSameNchan, 
			 progressInfo, showProgress, minNRow, 
			 timeSecCol);

    initLineFinder(edge, threshold, chanAvgLimit, lineFinder);

    bool applyFFT;
    string fftMethod, fftThresh;
    parseFFTInfo(fftInfo, applyFFT, fftMethod, fftThresh);

    std::vector<int> nWaves;
    std::vector<int> nChanNos;
    std::vector<std::vector<std::vector<double> > > modelReservoir;
    if (!applyFFT) {
      nWaves = selectWaveNumbers(addNWaves, rejectNWaves);
      modelReservoir = getSinusoidModelReservoir(nWaves, nChanNos);
    }

    for (int whichrow = 0; whichrow < nRow; ++whichrow) {
      std::vector<float> sp = getSpectrum(whichrow);
      std::vector<int> currentEdge;
      chanMask = getCompositeChanMask(whichrow, mask, edge, currentEdge, lineFinder);
      std::vector<std::vector<double> > model;
      if (applyFFT) {
	nWaves = selectWaveNumbers(whichrow, chanMask, true, fftMethod, fftThresh, 
				   addNWaves, rejectNWaves);
	model = getSinusoidModel(nWaves, sp.size());
      } else {
	model = modelReservoir[getIdxOfNchan(sp.size(), nChanNos)];
      }
      int nModel = modelReservoir.size();

      std::vector<float> params;

      if (flagrowCol_(whichrow) == 0) {
        int nClipped = 0;
        std::vector<float> res;
        res = doLeastSquareFitting(sp, chanMask, model, 
                                   params, rms, finalChanMask, 
                                   nClipped, thresClip, nIterClip, getResidual);

        if (outBaselineTable) {
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Sinusoid, nWaves, std::vector<float>(),
	                getMaskListFromMask(finalChanMask), params, rms, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
        } else {
	  setSpectrum(res, whichrow);
        }

        outputFittingResult(outLogger, outTextFile, csvFormat, chanMask, whichrow, 
			    coordInfo, hasSameNchan, ofs, "autoSinusoidBaseline()", 
			    params, nClipped);
      } else {
        if (outBaselineTable) {
	  params.resize(nModel);
          for (uInt i = 0; i < params.size(); ++i) {
            params[i] = 0.0;
	  }
	  bt.appenddata(getScan(whichrow), getCycle(whichrow), getBeam(whichrow), 
		        getIF(whichrow), getPol(whichrow), 0, timeSecCol[whichrow], 
	                true, STBaselineFunc::Sinusoid, nWaves, std::vector<float>(),
	                getMaskListFromMask(chanMask), params, 0.0, sp.size(), 
	                thresClip, nIterClip, threshold, chanAvgLimit, currentEdge);
        }
      }

      showProgressOnTerminal(whichrow, nRow, showProgress, minNRow);
    }

    finaliseBaselining(outBaselineTable, &bt, bltable, outTextFile, ofs);

  } catch (...) {
    throw;
  }
}

std::vector<float> Scantable::doSinusoidFitting(const std::vector<float>& data, 
						const std::vector<bool>& mask, 
						const std::vector<int>& waveNumbers, 
						std::vector<float>& params, 
						float& rms, 
						std::vector<bool>& finalmask, 
						float clipth, 
						int clipn)
{
  int nClipped = 0;
  return doSinusoidFitting(data, mask, waveNumbers, params, rms, finalmask, nClipped, clipth, clipn);
}

std::vector<float> Scantable::doSinusoidFitting(const std::vector<float>& data, 
						const std::vector<bool>& mask, 
						const std::vector<int>& waveNumbers, 
						std::vector<float>& params, 
						float& rms, 
						std::vector<bool>& finalMask, 
						int& nClipped, 
						float thresClip, 
						int nIterClip, 
						bool getResidual)
{
  return doLeastSquareFitting(data, mask, 
			      getSinusoidModel(waveNumbers, data.size()), 
			      params, rms, finalMask, 
			      nClipped, thresClip, nIterClip, 
			      getResidual);
}

std::vector<std::vector<std::vector<double> > > Scantable::getSinusoidModelReservoir(const std::vector<int>& waveNumbers, 
										     std::vector<int>& nChanNos)
{
  std::vector<std::vector<std::vector<double> > > res;
  res.clear();
  nChanNos.clear();

  std::vector<uint> ifNos = getIFNos();
  for (uint i = 0; i < ifNos.size(); ++i) {
    int currNchan = nchan(ifNos[i]);
    bool hasDifferentNchan = (i == 0);
    for (uint j = 0; j < i; ++j) {
      if (currNchan != nchan(ifNos[j])) {
	hasDifferentNchan = true;
	break;
      }
    }
    if (hasDifferentNchan) {
      res.push_back(getSinusoidModel(waveNumbers, currNchan));
      nChanNos.push_back(currNchan);
    }
  }

  return res;
}

std::vector<std::vector<double> > Scantable::getSinusoidModel(const std::vector<int>& waveNumbers, int nchan)
{
  // model  : contains elemental values for computing the least-square matrix. 
  //          model.size() is nmodel and model[*].size() is nchan. 
  //          Each model element are as follows:
  //          model[0]    = {1.0, 1.0, 1.0, ..., 1.0}, 
  //          model[2n-1] = {sin(nPI/L*x[0]), sin(nPI/L*x[1]), ..., sin(nPI/L*x[nchan])}, 
  //          model[2n]   = {cos(nPI/L*x[0]), cos(nPI/L*x[1]), ..., cos(nPI/L*x[nchan])}, 
  //          where (1 <= n <= nMaxWavesInSW),
  //          or, 
  //          model[2n-1] = {sin(wn[n]PI/L*x[0]), sin(wn[n]PI/L*x[1]), ..., sin(wn[n]PI/L*x[nchan])}, 
  //          model[2n]   = {cos(wn[n]PI/L*x[0]), cos(wn[n]PI/L*x[1]), ..., cos(wn[n]PI/L*x[nchan])}, 
  //          where wn[n] denotes waveNumbers[n] (1 <= n <= waveNumbers.size()).

  std::vector<int> nWaves;  // sorted and uniqued array of wave numbers
  nWaves.reserve(waveNumbers.size());
  copy(waveNumbers.begin(), waveNumbers.end(), back_inserter(nWaves));
  sort(nWaves.begin(), nWaves.end());
  std::vector<int>::iterator end_it = unique(nWaves.begin(), nWaves.end());
  nWaves.erase(end_it, nWaves.end());

  int minNWaves = nWaves[0];
  if (minNWaves < 0) {
    throw(AipsError("wave number must be positive or zero (i.e. constant)"));
  }
  bool hasConstantTerm = (minNWaves == 0);
  int nmodel = nWaves.size() * 2 - (hasConstantTerm ? 1 : 0);  //number of parameters to solve.

  std::vector<std::vector<double> > model(nmodel, std::vector<double>(nchan));

  if (hasConstantTerm) {
    for (int j = 0; j < nchan; ++j) {
      model[0][j] = 1.0;
    }
  }

  const double PI = 6.0 * asin(0.5); // PI (= 3.141592653...)
  double stretch0 = 2.0*PI/(double)(nchan-1);

  for (uInt i = (hasConstantTerm ? 1 : 0); i < nWaves.size(); ++i) {
    int sidx = hasConstantTerm ? 2*i-1 : 2*i;
    int cidx = sidx + 1;
    double stretch = stretch0*(double)nWaves[i];

    for (int j = 0; j < nchan; ++j) {
      model[sidx][j] = sin(stretch*(double)j);
      model[cidx][j] = cos(stretch*(double)j);
    }
  }

  return model;
}

std::vector<bool> Scantable::getCompositeChanMask(int whichrow, 
						  const std::vector<bool>& inMask)
{
  std::vector<bool> mask = getMask(whichrow);
  uInt maskSize = mask.size();
  if (inMask.size() != 0) {
    if (maskSize != inMask.size()) {
      throw(AipsError("mask sizes are not the same."));
    }
    for (uInt i = 0; i < maskSize; ++i) {
      mask[i] = mask[i] && inMask[i];
    }
  }

  return mask;
}

std::vector<bool> Scantable::getCompositeChanMask(int whichrow, 
						  const std::vector<bool>& inMask, 
						  const std::vector<int>& edge, 
						  std::vector<int>& currEdge, 
						  STLineFinder& lineFinder)
{
  std::vector<uint> ifNos = getIFNos();
  if ((edge.size() > 2) && (edge.size() < ifNos.size()*2)) {
    throw(AipsError("Length of edge element info is less than that of IFs"));
  }

  uint idx = 0;
  if (edge.size() > 2) {
    int ifVal = getIF(whichrow);
    bool foundIF = false;
    for (uint i = 0; i < ifNos.size(); ++i) {
      if (ifVal == (int)ifNos[i]) {
	idx = 2*i;
	foundIF = true;
	break;
      }
    }
    if (!foundIF) {
      throw(AipsError("bad IF number"));
    }
  }

  currEdge.clear();
  currEdge.resize(2);
  currEdge[0] = edge[idx];
  currEdge[1] = edge[idx+1];

  lineFinder.setData(getSpectrum(whichrow));
  lineFinder.findLines(getCompositeChanMask(whichrow, inMask), currEdge, whichrow);

  return lineFinder.getMask();
}

/* for cspline. will be merged once cspline is available in fitter (2011/3/10 WK) */
void Scantable::outputFittingResult(bool outLogger, 
				    bool outTextFile, 
				    bool csvFormat, 
				    const std::vector<bool>& chanMask, 
				    int whichrow, 
				    const casa::String& coordInfo, 
				    bool hasSameNchan, 
				    ofstream& ofs, 
				    const casa::String& funcName, 
				    const std::vector<int>& edge, 
				    const std::vector<float>& params, 
				    const int nClipped)
{
  if (outLogger || outTextFile) {
    float rms = getRms(chanMask, whichrow);
    String masklist = getMaskRangeList(chanMask, whichrow, coordInfo, hasSameNchan);
    std::vector<bool> fixed;
    fixed.clear();

    if (outLogger) {
      LogIO ols(LogOrigin("Scantable", funcName, WHERE));
      ols << formatPiecewiseBaselineParams(edge, params, fixed, rms, nClipped, 
					   masklist, whichrow, false, csvFormat) << LogIO::POST ;
    }
    if (outTextFile) {
      ofs << formatPiecewiseBaselineParams(edge, params, fixed, rms, nClipped, 
					   masklist, whichrow, true, csvFormat) << flush;
    }
  }
}

/* for poly/chebyshev/sinusoid. */
void Scantable::outputFittingResult(bool outLogger, 
				    bool outTextFile, 
				    bool csvFormat, 
				    const std::vector<bool>& chanMask, 
				    int whichrow, 
				    const casa::String& coordInfo, 
				    bool hasSameNchan, 
				    ofstream& ofs, 
				    const casa::String& funcName, 
				    const std::vector<float>& params, 
				    const int nClipped)
{
  if (outLogger || outTextFile) {
    float rms = getRms(chanMask, whichrow);
    String masklist = getMaskRangeList(chanMask, whichrow, coordInfo, hasSameNchan);
    std::vector<bool> fixed;
    fixed.clear();

    if (outLogger) {
      LogIO ols(LogOrigin("Scantable", funcName, WHERE));
      ols << formatBaselineParams(params, fixed, rms, nClipped, 
				  masklist, whichrow, false, csvFormat) << LogIO::POST ;
    }
    if (outTextFile) {
      ofs << formatBaselineParams(params, fixed, rms, nClipped, 
				  masklist, whichrow, true, csvFormat) << flush;
    }
  }
}

void Scantable::parseProgressInfo(const std::string& progressInfo, bool& showProgress, int& minNRow)
{
  int idxDelimiter = progressInfo.find(",");
  if (idxDelimiter < 0) {
    throw(AipsError("wrong value in 'showprogress' parameter")) ;
  }
  showProgress = (progressInfo.substr(0, idxDelimiter) == "true");
  std::istringstream is(progressInfo.substr(idxDelimiter+1));
  is >> minNRow;
}

void Scantable::showProgressOnTerminal(const int nProcessed, const int nTotal, const bool showProgress, const int nTotalThreshold)
{
  if (showProgress && (nTotal >= nTotalThreshold)) {
    int nInterval = int(floor(double(nTotal)/100.0));
    if (nInterval == 0) nInterval++;

    if (nProcessed % nInterval == 0) {
      printf("\r");                          //go to the head of line
      printf("\x1b[31m\x1b[1m");             //set red color, highlighted
      printf("[%3d%%]", (int)(100.0*(double(nProcessed+1))/(double(nTotal))) );
      printf("\x1b[39m\x1b[0m");             //set default attributes
      fflush(NULL);
    }

    if (nProcessed == nTotal - 1) {
      printf("\r\x1b[K");                    //clear
      fflush(NULL);
    }

  }
}

std::vector<float> Scantable::execFFT(const int whichrow, const std::vector<bool>& inMask, bool getRealImag, bool getAmplitudeOnly)
{
  std::vector<bool>  mask = getMask(whichrow);

  if (inMask.size() > 0) {
    uInt maskSize = mask.size();
    if (maskSize != inMask.size()) {
      throw(AipsError("mask sizes are not the same."));
    }
    for (uInt i = 0; i < maskSize; ++i) {
      mask[i] = mask[i] && inMask[i];
    }
  }

  Vector<Float> spec = getSpectrum(whichrow);
  mathutil::doZeroOrderInterpolation(spec, mask);

  FFTServer<Float,Complex> ffts;
  Vector<Complex> fftres;
  ffts.fft0(fftres, spec);

  std::vector<float> res;
  float norm = float(2.0/double(spec.size()));

  if (getRealImag) {
    for (uInt i = 0; i < fftres.size(); ++i) {
      res.push_back(real(fftres[i])*norm);
      res.push_back(imag(fftres[i])*norm);
    }
  } else {
    for (uInt i = 0; i < fftres.size(); ++i) {
      res.push_back(abs(fftres[i])*norm);
      if (!getAmplitudeOnly) res.push_back(arg(fftres[i]));
    }
  }

  return res;
}


float Scantable::getRms(const std::vector<bool>& mask, int whichrow)
{
  /****
  double ms1TimeStart, ms1TimeEnd;
  double elapse1 = 0.0;
  ms1TimeStart = mathutil::gettimeofday_sec();
  ****/

  Vector<Float> spec;
  specCol_.get(whichrow, spec);

  /****
  ms1TimeEnd = mathutil::gettimeofday_sec();
  elapse1 = ms1TimeEnd - ms1TimeStart;
  std::cout << "rm1   : " << elapse1 << " (sec.)" << endl;
  ****/

  return (float)doGetRms(mask, spec);
}

double Scantable::doGetRms(const std::vector<bool>& mask, const Vector<Float>& spec)
{
  double mean = 0.0;
  double smean = 0.0;
  int n = 0;
  for (uInt i = 0; i < spec.nelements(); ++i) {
    if (mask[i]) {
      double val = (double)spec[i];
      mean += val;
      smean += val*val;
      n++;
    }
  }

  mean /= (double)n;
  smean /= (double)n;

  return sqrt(smean - mean*mean);
}

std::string Scantable::formatBaselineParamsHeader(int whichrow, const std::string& masklist, bool verbose, bool csvformat) const
{
  if (verbose) {
    ostringstream oss;

    if (csvformat) {
      oss << getScan(whichrow)  << ",";
      oss << getBeam(whichrow)  << ",";
      oss << getIF(whichrow)    << ",";
      oss << getPol(whichrow)   << ",";
      oss << getCycle(whichrow) << ",";
      String commaReplacedMasklist = masklist;
      string::size_type pos = 0;
      while (pos = commaReplacedMasklist.find(","), pos != string::npos) {
	commaReplacedMasklist.replace(pos, 1, ";");
	pos++;
      }
      oss << commaReplacedMasklist << ",";
    } else {
      oss <<  " Scan[" << getScan(whichrow)  << "]";
      oss <<  " Beam[" << getBeam(whichrow)  << "]";
      oss <<    " IF[" << getIF(whichrow)    << "]";
      oss <<   " Pol[" << getPol(whichrow)   << "]";
      oss << " Cycle[" << getCycle(whichrow) << "]: " << endl;
      oss << "Fitter range = " << masklist << endl;
      oss << "Baseline parameters" << endl;
    }
    oss << flush;

    return String(oss);
  }

  return "";
}

std::string Scantable::formatBaselineParamsFooter(float rms, int nClipped, bool verbose, bool csvformat) const
{
  if (verbose) {
    ostringstream oss;

    if (csvformat) {
      oss << rms << ",";
      if (nClipped >= 0) {
	oss << nClipped;
      }
    } else {
      oss << "Results of baseline fit" << endl;
      oss << "  rms = " << setprecision(6) << rms << endl;
      if (nClipped >= 0) {
	oss << "  Number of clipped channels = " << nClipped << endl;
      }
      for (int i = 0; i < 60; ++i) {
	oss << "-";
      }
    }
    oss << endl;
    oss << flush;

    return String(oss);
  }

  return "";
}

std::string Scantable::formatBaselineParams(const std::vector<float>& params, 
					    const std::vector<bool>& fixed, 
					    float rms, 
					    int nClipped,
					    const std::string& masklist, 
					    int whichrow, 
					    bool verbose, 
					    bool csvformat, 
					    int start, int count,
					    bool resetparamid) const
{
  int nParam = (int)(params.size());

  if (nParam < 1) {
    return("  Not fitted");
  } else {

    ostringstream oss;
    oss << formatBaselineParamsHeader(whichrow, masklist, verbose, csvformat);

    if (start < 0) start = 0;
    if (count < 0) count = nParam;
    int end = start + count;
    if (end > nParam) end = nParam;
    int paramidoffset = (resetparamid) ? (-start) : 0;

    for (int i = start; i < end; ++i) {
      if (i > start) {
	oss << ",";
      }
      std::string sFix = ((fixed.size() > 0) && (fixed[i]) && verbose) ? "(fixed)" : "";
      if (csvformat) {
	oss << params[i] << sFix;
      } else {
	oss << "  p" << (i+paramidoffset) << sFix << "= " << right << setw(13) << setprecision(6) << params[i];
      }
    }

    if (csvformat) {
      oss << ",";
    } else {
      oss << endl;
    }
    oss << formatBaselineParamsFooter(rms, nClipped, verbose, csvformat);

    return String(oss);
  }

}

std::string Scantable::formatPiecewiseBaselineParams(const std::vector<int>& ranges, const std::vector<float>& params, const std::vector<bool>& fixed, float rms, int nClipped, const std::string& masklist, int whichrow, bool verbose, bool csvformat) const
{
  int nOutParam = (int)(params.size());
  int nPiece = (int)(ranges.size()) - 1;

  if (nOutParam < 1) {
    return("  Not fitted");
  } else if (nPiece < 0) {
    return formatBaselineParams(params, fixed, rms, nClipped, masklist, whichrow, verbose, csvformat);
  } else if (nPiece < 1) {
    return("  Bad count of the piece edge info");
  } else if (nOutParam % nPiece != 0) {
    return("  Bad count of the output baseline parameters");
  } else {

    int nParam = nOutParam / nPiece;

    ostringstream oss;
    oss << formatBaselineParamsHeader(whichrow, masklist, verbose, csvformat);

    if (csvformat) {
      for (int i = 0; i < nPiece; ++i) {
	oss << ranges[i] << "," << (ranges[i+1]-1) << ","; 
	oss << formatBaselineParams(params, fixed, rms, 0, masklist, whichrow, false, csvformat, i*nParam, nParam, true);
      }
    } else {
      stringstream ss;
      ss << ranges[nPiece] << flush;
      int wRange = ss.str().size() * 2 + 5;

      for (int i = 0; i < nPiece; ++i) {
	ss.str("");
	ss << "  [" << ranges[i] << "," << (ranges[i+1]-1) << "]";
	oss << left << setw(wRange) << ss.str();
	oss << formatBaselineParams(params, fixed, rms, 0, masklist, whichrow, false, csvformat, i*nParam, nParam, true);
	//oss << endl;
      }
    }

    oss << formatBaselineParamsFooter(rms, nClipped, verbose, csvformat);

    return String(oss);
  }

}

bool Scantable::hasSameNchanOverIFs()
{
  int nIF = nif(-1);
  int nCh;
  int totalPositiveNChan = 0;
  int nPositiveNChan = 0;

  for (int i = 0; i < nIF; ++i) {
    nCh = nchan(i);
    if (nCh > 0) {
      totalPositiveNChan += nCh;
      nPositiveNChan++;
    }
  }

  return (totalPositiveNChan == (nPositiveNChan * nchan(0)));
}

std::string Scantable::getMaskRangeList(const std::vector<bool>& mask, int whichrow, const casa::String& coordInfo, bool hasSameNchan, bool verbose)
{
  if (mask.size() <= 0) {
    throw(AipsError("The mask elements should be > 0"));
  }
  int IF = getIF(whichrow);
  if (mask.size() != (uInt)nchan(IF)) {
    throw(AipsError("Number of channels in scantable != number of mask elements"));
  }

  if (verbose) {
    LogIO logOs(LogOrigin("Scantable", "getMaskRangeList()", WHERE));
    logOs << LogIO::WARN << "The current mask window unit is " << coordInfo;
    if (!hasSameNchan) {
      logOs << endl << "This mask is only valid for IF=" << IF;
    }
    logOs << LogIO::POST;
  }

  std::vector<double> abcissa = getAbcissa(whichrow);
  std::vector<int> edge = getMaskEdgeIndices(mask);

  ostringstream oss;
  oss.setf(ios::fixed);
  oss << setprecision(1) << "[";
  for (uInt i = 0; i < edge.size(); i+=2) {
    if (i > 0) oss << ",";
    oss << "[" << (float)abcissa[edge[i]] << "," << (float)abcissa[edge[i+1]] << "]";
  }
  oss << "]" << flush;

  return String(oss);
}

std::vector<int> Scantable::getMaskEdgeIndices(const std::vector<bool>& mask)
{
  if (mask.size() <= 0) {
    throw(AipsError("The mask elements should be > 0"));
  }

  std::vector<int> out, startIndices, endIndices;
  int maskSize = mask.size();

  startIndices.clear();
  endIndices.clear();

  if (mask[0]) {
    startIndices.push_back(0);
  }
  for (int i = 1; i < maskSize; ++i) {
    if ((!mask[i-1]) && mask[i]) {
      startIndices.push_back(i);
    } else if (mask[i-1] && (!mask[i])) {
      endIndices.push_back(i-1);
    }
  }
  if (mask[maskSize-1]) {
    endIndices.push_back(maskSize-1);
  }

  if (startIndices.size() != endIndices.size()) {
    throw(AipsError("Inconsistent Mask Size: bad data?"));
  }
  for (uInt i = 0; i < startIndices.size(); ++i) {
    if (startIndices[i] > endIndices[i]) {
      throw(AipsError("Mask start index > mask end index"));
    }
  }

  out.clear();
  for (uInt i = 0; i < startIndices.size(); ++i) {
    out.push_back(startIndices[i]);
    out.push_back(endIndices[i]);
  }

  return out;
}

void Scantable::setTsys(const std::vector<float>& newvals, int whichrow) {
  Vector<Float> tsys(newvals);
  if (whichrow > -1) {
    if (tsysCol_.shape(whichrow) != tsys.shape())
      throw(AipsError("Given Tsys values are not of the same shape"));
    tsysCol_.put(whichrow, tsys);
  } else {
    tsysCol_.fillColumn(tsys);
  }
}

vector<float> Scantable::getTsysSpectrum( int whichrow ) const
{
  Vector<Float> tsys( tsysCol_(whichrow) ) ;
  vector<float> stlTsys ;
  tsys.tovector( stlTsys ) ;
  return stlTsys ;
}

vector<uint> Scantable::getMoleculeIdColumnData() const
{
  Vector<uInt> molIds(mmolidCol_.getColumn());
  vector<uint> res;
  molIds.tovector(res);
  return res;
}

void Scantable::setMoleculeIdColumnData(const std::vector<uint>& molids)
{
  Vector<uInt> molIds(molids);
  Vector<uInt> arr(mmolidCol_.getColumn());
  if ( molIds.nelements() != arr.nelements() )
    throw AipsError("The input data size must be the number of rows.");
  mmolidCol_.putColumn(molIds);
}


std::vector<uint> Scantable::getRootTableRowNumbers() const
{
  Vector<uInt> rowIds(table_.rowNumbers());
  vector<uint> res;
  rowIds.tovector(res);
  return res;
}


void Scantable::dropXPol()
{
  if (npol() <= 2) {
    return;
  }
  if (!selector_.empty()) {
    throw AipsError("Can only operate with empty selection");
  }
  std::string taql = "SELECT FROM $1 WHERE POLNO IN [0,1]";
  Table tab = tableCommand(taql, table_);
  table_ = tab;
  table_.rwKeywordSet().define("nPol", Int(2));
  originalTable_ = table_;
  attach();
}

}
//namespace asap

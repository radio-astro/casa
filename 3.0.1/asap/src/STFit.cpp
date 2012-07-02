
//
// C++ Implementation: STFit
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/Exceptions/Error.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>
#include <casa/Containers/RecordField.h>

#include "MathUtils.h"
#include "STFitEntry.h"
#include "STFit.h"


using namespace casa;

namespace asap {

const casa::String STFit::name_ = "FIT";

STFit::STFit(const Scantable& parent) :
  STSubTable( parent, name_ )
{
  setup();
}

STFit& asap::STFit::operator =( const STFit & other )
{
  if ( this != &other ) {
    static_cast<STSubTable&>(*this) = other;
    funcCol_.attach(table_,"FUNCTIONS");
    compCol_.attach(table_,"COMPONENTS");
    parCol_.attach(table_,"PARAMETERS");
    maskCol_.attach(table_,"PARMASKS");
    frameCol_.attach(table_,"FRAMEINFO");
  }
  return *this;
}

asap::STFit::STFit( casa::Table tab ) : STSubTable(tab, name_)
{
    funcCol_.attach(table_,"FUNCTIONS");
    compCol_.attach(table_,"COMPONENTS");
    parCol_.attach(table_,"PARAMETERS");
    maskCol_.attach(table_,"PARMASKS");
    frameCol_.attach(table_,"FRAMEINFO");
}

STFit::~STFit()
{
}

void asap::STFit::setup( )
{
  // add to base class table
  table_.addColumn(ArrayColumnDesc<String>("FUNCTIONS"));
  table_.addColumn(ArrayColumnDesc<Int>("COMPONENTS"));
  table_.addColumn(ArrayColumnDesc<Double>("PARAMETERS"));
  table_.addColumn(ArrayColumnDesc<Bool>("PARMASKS"));
  table_.addColumn(ArrayColumnDesc<String>("FRAMEINFO"));

  // new cached columns
  funcCol_.attach(table_,"FUNCTIONS");
  compCol_.attach(table_,"COMPONENTS");
  parCol_.attach(table_,"PARAMETERS");
  maskCol_.attach(table_,"PARMASKS");
  frameCol_.attach(table_,"FRAMEINFO");
}

uInt STFit::addEntry( const STFitEntry& fit, Int id )
{
  uInt rno = table_.nrow();
  uInt resultid = 0;
  bool foundentry = false;
  // replace
  if ( id > -1 ) {
    Table t = table_(table_.col("ID") == id );
    if (t.nrow() > 0) {
      rno = t.rowNumbers(table_)[0];
      resultid = id;
      foundentry = true;
    }
  }
  // doesn't exist
  if ( rno > 0  && !foundentry ) {
    idCol_.get(rno-1, resultid);
    resultid++;
  }
  // add new row if new id
  if ( !foundentry ) table_.addRow();
  funcCol_.put(rno, mathutil::toVectorString(fit.getFunctions()));
  compCol_.put(rno, Vector<Int>(fit.getComponents()));
  parCol_.put(rno, Vector<Double>(fit.getParameters()));
  maskCol_.put(rno, Vector<Bool>(fit.getParmasks()));
  frameCol_.put(rno, mathutil::toVectorString(fit.getFrameinfo()));
  idCol_.put(rno, resultid);
  return resultid;
}

void STFit::getEntry( STFitEntry& fit, uInt id ) const
{
  Table t = table_(table_.col("ID") == Int(id) );
  if (t.nrow() == 0 ) {
    throw(AipsError("STFit::getEntry - id out of range"));
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id
  const TableRecord& rec = row.get(0);
  std::vector<std::string> outstr;
  Vector<String> vec;
  rec.get("FUNCTIONS", vec);
  fit.setFunctions(mathutil::tovectorstring(vec));
  Vector<Int> ivec;
  std::vector<int> istl;
  rec.get("COMPONENTS", ivec);
  ivec.tovector(istl);
  fit.setComponents(istl);
  Vector<Double> dvec;
  std::vector<double> dstl;
  rec.get("PARAMETERS", dvec);
  dvec.tovector(dstl);
  fit.setParameters(dstl);
  Vector<Bool> bvec;
  std::vector<bool> bstl;
  rec.get("PARMASKS", bvec);
  bvec.tovector(bstl);
  fit.setParmasks(bstl);
  vec.resize();
  rec.get("FRAMEINFO", vec);
  fit.setFrameinfo(mathutil::tovectorstring(vec));
}

} //namespace

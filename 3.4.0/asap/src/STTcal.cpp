
//
// C++ Implementation: STTcal
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

#include "STTcal.h"


using namespace casa;

namespace asap {

const casa::String STTcal::name_ = "TCAL";

STTcal::STTcal(const Scantable& parent) :
  STSubTable( parent, name_ )
{
  setup();
}

STTcal& asap::STTcal::operator =( const STTcal & other )
{
  if ( this != &other ) {
    static_cast<STSubTable&>(*this) = other;
    timeCol_.attach(table_,"TIME");
    tcalCol_.attach(table_,"TCAL");
  }
  return *this;
}

asap::STTcal::STTcal( casa::Table tab ) : STSubTable(tab, name_)
{
  timeCol_.attach(table_,"TIME");
  tcalCol_.attach(table_,"TCAL");

}

STTcal::~STTcal()
{
}

void asap::STTcal::setup( )
{
  // add to base class table
  table_.addColumn(ScalarColumnDesc<String>("TIME"));
  table_.addColumn(ArrayColumnDesc<Float>("TCAL"));

  // new cached columns
  timeCol_.attach(table_,"TIME");
  tcalCol_.attach(table_,"TCAL");
}

/*** rewrite this for handling of GBT data 
uInt STTcal::addEntry( const String& time, const Vector<Float>& cal)
{
  // test if this already exists
  Table result = table_( table_.col("TIME") == time );
  uInt resultid = 0;
  if ( result.nrow() > 0) {
    ROScalarColumn<uInt> c(result, "ID");
    c.get(0, resultid);
  } else {
    uInt rno = table_.nrow();
    table_.addRow();
    // get last assigned tcal_id and increment
    if ( rno > 0 ) {
      idCol_.get(rno-1, resultid);
      resultid++;
    }
    tcalCol_.put(rno, cal);
    timeCol_.put(rno, time);
    idCol_.put(rno, resultid);
  }
  return resultid;
}
***/

uInt STTcal::addEntry( const String& time, const Vector<Float>& cal)
{
  // test if this already exists
  // TT - different Tcal values for each polarization, feed, and
  // data description. So there may be multiple entries for the same
  // time stamp.
  uInt resultid;
  uInt rno = table_.nrow();
  //table_.addRow();
  // get last assigned tcal_id and increment
  if ( rno == 0 ) {
    resultid = 0;
  }
  else {
    idCol_.get(rno-1, resultid);
    resultid++;
  }
  table_.addRow();
  tcalCol_.put(rno, cal);
  timeCol_.put(rno, time);
  idCol_.put(rno, resultid);
  return resultid;
}


void STTcal::getEntry( String& time, Vector<Float>& tcal, uInt id )
{
  Table t = table_(table_.col("ID") == Int(id),1);
  if (t.nrow() == 0 ) {
    throw(AipsError("STTcal::getEntry - id out of range"));
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id
  const TableRecord& rec = row.get(0);
  time = rec.asString("TIME");
  tcal.resize();
  Vector<Float> out;
  rec.get("TCAL",out);
  tcal = out;
}

} //namespace

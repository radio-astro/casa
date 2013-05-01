//
// C++ Implementation: STFocus
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
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>
#include <casa/Containers/RecordField.h>

#include "STFocus.h"


using namespace casa;

namespace asap {

const casa::String STFocus::name_ = "FOCUS";

STFocus::STFocus(const Scantable& parent ) :
  STSubTable( parent, name_ )
{
  setup();
}

STFocus::STFocus( casa::Table tab ) : 
  STSubTable(tab, name_)
{
  parangleCol_.attach(table_,"PARANGLE");
  rotationCol_.attach(table_,"ROTATION");
  axisCol_.attach(table_,"AXIS");
  tanCol_.attach(table_,"TAN");
  handCol_.attach(table_,"HAND");
  userCol_.attach(table_,"USERPHASE");
  mountCol_.attach(table_,"MOUNT");
  xyphCol_.attach(table_,"XYPHASE");
  xyphoffCol_.attach(table_,"XYPHASEOFFSET");
}

STFocus::~STFocus()
{
}

STFocus& STFocus::operator =( const STFocus & other )
{
  if (this != &other) {
    static_cast<STSubTable&>(*this) = other;
    parangleCol_.attach(table_,"PARANGLE");
    rotationCol_.attach(table_,"ROTATION");
    axisCol_.attach(table_,"AXIS");
    tanCol_.attach(table_,"TAN");
    handCol_.attach(table_,"HAND");
    userCol_.attach(table_,"USERPHASE");
    mountCol_.attach(table_,"MOUNT");
    xyphCol_.attach(table_,"XYPHASE");
    xyphoffCol_.attach(table_,"XYPHASEOFFSET");
  }
  return *this;
}
void STFocus::setup( )
{
  // add to base class table
  table_.addColumn(ScalarColumnDesc<Float>("PARANGLE"));
  table_.addColumn(ScalarColumnDesc<Float>("ROTATION"));
  table_.addColumn(ScalarColumnDesc<Float>("AXIS"));
  table_.addColumn(ScalarColumnDesc<Float>("TAN"));
  table_.addColumn(ScalarColumnDesc<Float>("HAND"));
  table_.addColumn(ScalarColumnDesc<Float>("USERPHASE"));
  table_.addColumn(ScalarColumnDesc<Float>("MOUNT"));
  table_.addColumn(ScalarColumnDesc<Float>("XYPHASE"));
  table_.addColumn(ScalarColumnDesc<Float>("XYPHASEOFFSET"));
  table_.rwKeywordSet().define("PARALLACTIFY", False);

  // new cached columns
  parangleCol_.attach(table_,"PARANGLE");
  rotationCol_.attach(table_,"ROTATION");
  axisCol_.attach(table_,"AXIS");
  tanCol_.attach(table_,"TAN");
  handCol_.attach(table_,"HAND");
  userCol_.attach(table_,"USERPHASE");
  mountCol_.attach(table_,"MOUNT");
  xyphCol_.attach(table_,"XYPHASE");
  xyphoffCol_.attach(table_,"XYPHASEOFFSET");
}

  uInt STFocus::addEntry( Float pa, Float fax, Float ftan, Float frot, Float hand,
                          Float user, Float mount,
                          Float xyphase, Float xyphaseoffset)
{
  Table result = table_(  near(table_.col("PARANGLE"), pa)
                          && near(table_.col("ROTATION"), frot)
                          && near(table_.col("AXIS"), fax)
                          && near(table_.col("TAN"), ftan)
                          && near(table_.col("HAND"), hand)
                          && near(table_.col("USERPHASE"), user)
                          && near(table_.col("MOUNT"), mount)
                          && near(table_.col("XYPHASE"), xyphase)
                          && near(table_.col("XYPHASEOFFSET"), xyphaseoffset), 1
                          );
  uInt resultid = 0;
  if ( result.nrow() > 0) {
    ROScalarColumn<uInt> c(result, "ID");
    c.get(0, resultid);
  } else {
    uInt rno = table_.nrow();
    table_.addRow();
    // get last assigned _id and increment
    if ( rno > 0 ) {
      idCol_.get(rno-1, resultid);
      resultid++;
    }
    parangleCol_.put(rno, pa);
    rotationCol_.put(rno, frot);
    axisCol_.put(rno, fax);
    tanCol_.put(rno, ftan);
    handCol_.put(rno, hand);
    userCol_.put(rno, user);
    mountCol_.put(rno, mount);
    xyphCol_.put(rno, xyphase);
    xyphoffCol_.put(rno, xyphaseoffset);
    idCol_.put(rno, resultid);
  }
  return resultid;
}

  void STFocus::getEntry( Float& pa, Float& rotation, Float& angle, Float& ftan,
                                Float& hand, Float& user, Float& mount,
                                Float& xyphase, Float& xyphaseoffset,
                                uInt id) const
{
  Table t = table_(table_.col("ID") == Int(id), 1 );
  if (t.nrow() == 0 ) {
    throw(AipsError("STFocus::getEntry - id out of range"));
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id
  const TableRecord& rec = row.get(0);
  pa = rec.asFloat("PARANGLE");
  rotation = rec.asFloat("ROTATION");
  angle = rec.asFloat("AXIS");
  ftan = rec.asFloat("TAN");
  hand = rec.asFloat("HAND");
  user = rec.asFloat("USERPHASE");
  mount = rec.asFloat("MOUNT");
  xyphase = rec.asFloat("XYPHASE");
  xyphaseoffset = rec.asFloat("XYPHASEOFFSET");
}


casa::Float STFocus::getTotalAngle( casa::uInt id ) const
{
  Float total = 0.0f;
  Table t = table_(table_.col("ID") == Int(id), 1 );
  if (t.nrow() == 0 ) {
    throw(AipsError("STFocus::getTotalAngle - id out of range"));
  }
  if (table_.keywordSet().asBool("PARALLACTIFY")) {
    return 0.0f;
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id
  const TableRecord& rec = row.get(0);
  total += rec.asFloat("PARANGLE");  
  total += rec.asFloat("ROTATION");
  total += rec.asFloat("USERPHASE");
  total += rec.asFloat("MOUNT");
  return total;
}


casa::Float STFocus::getFeedHand( casa::uInt id ) const
{
  Table t = table_(table_.col("ID") == Int(id), 1 );
  if (t.nrow() == 0 ) {
    throw(AipsError("STFocus::getEntry - id out of range"));
  }
  ROTableRow row(t);
  const TableRecord& rec = row.get(0);
  return rec.asFloat("HAND");
}

void STFocus::setParallactify(bool istrue) {
  table_.rwKeywordSet().define("PARALLACTIFY", Bool(istrue));
}

}

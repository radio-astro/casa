//
// C++ Implementation: STMolecules
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

#include <tables/Tables/TableProxy.h>

#include "STMolecules.h"


using namespace casa;

namespace asap {

const casa::String STMolecules::name_ = "MOLECULES";

STMolecules::STMolecules(const Scantable& parent) :
  STSubTable( parent, name_ )
{
  setup();
}

asap::STMolecules::STMolecules( casa::Table tab ) : STSubTable(tab, name_)
{
  restfreqCol_.attach(table_,"RESTFREQUENCY");
  nameCol_.attach(table_,"NAME");
  formattednameCol_.attach(table_,"FORMATTEDNAME");
}

STMolecules::~STMolecules()
{
}

STMolecules & asap::STMolecules::operator =( const STMolecules & other )
{
  if ( this != &other ) {
    static_cast<STSubTable&>(*this) = other;
    restfreqCol_.attach(table_,"RESTFREQUENCY");
    nameCol_.attach(table_,"NAME");
    formattednameCol_.attach(table_,"FORMATTEDNAME");
  }
  return *this;
}

void asap::STMolecules::setup( )
{
  // add to base class table
  //table_.addColumn(ScalarColumnDesc<Double>("RESTFREQUENCY"));
  table_.addColumn(ArrayColumnDesc<Double>("RESTFREQUENCY"));
  //table_.addColumn(ScalarColumnDesc<String>("NAME"));
  table_.addColumn(ArrayColumnDesc<String>("NAME"));
  //table_.addColumn(ScalarColumnDesc<String>("FORMATTEDNAME"));
  table_.addColumn(ArrayColumnDesc<String>("FORMATTEDNAME"));
  table_.rwKeywordSet().define("UNIT", String("Hz"));
  // new cached columns
  restfreqCol_.attach(table_,"RESTFREQUENCY");
  nameCol_.attach(table_,"NAME");
  formattednameCol_.attach(table_,"FORMATTEDNAME");
}

/***
uInt STMolecules::addEntry( Double restfreq, const String& name,
                            const String& formattedname )
{

  Table result =
    table_( near(table_.col("RESTFREQUENCY"), restfreq) );
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
    restfreqCol_.put(rno, restfreq);
    nameCol_.put(rno, name);
    formattednameCol_.put(rno, formattedname);
    idCol_.put(rno, resultid);
  }
  return resultid;
}
***/
uInt STMolecules::addEntry( Vector<Double> restfreq, const Vector<String>& name,
                            const Vector<String>& formattedname )
{
// How to handle this...?
  Table result =
    table_( nelements(table_.col("RESTFREQUENCY")) == uInt (restfreq.size()) && 
            all(table_.col("RESTFREQUENCY")== restfreq) );
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
    restfreqCol_.put(rno, restfreq);
    nameCol_.put(rno, name);
    formattednameCol_.put(rno, formattedname);
    idCol_.put(rno, resultid);
  }
  return resultid;
}




/***
void STMolecules::getEntry( Double& restfreq, String& name,
                            String& formattedname, uInt id ) const
{
  Table t = table_(table_.col("ID") == Int(id) );
  if (t.nrow() == 0 ) {
    throw(AipsError("STMolecules::getEntry - id out of range"));
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id

  const TableRecord& rec = row.get(0);
  restfreq = rec.asDouble("RESTFREQUENCY");
  name = rec.asString("NAME");
  formattedname = rec.asString("FORMATTEDNAME");
}
***/
void STMolecules::getEntry( Vector<Double>& restfreq, Vector<String>& name,
                            Vector<String>& formattedname, uInt id ) const
{
  Table t = table_(table_.col("ID") == Int(id) );
  if (t.nrow() == 0 ) {
    throw(AipsError("STMolecules::getEntry - id out of range"));
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id

  const TableRecord& rec = row.get(0);
  //restfreq = rec.asDouble("RESTFREQUENCY");
  restfreq = rec.asArrayDouble("RESTFREQUENCY");
  //name = rec.asString("NAME");
  name = rec.asArrayString("NAME");
  //formattedname = rec.asString("FORMATTEDNAME");
  formattedname = rec.asArrayString("FORMATTEDNAME");
}

std::vector< double > asap::STMolecules::getRestFrequencies( ) const
{
  std::vector<double> out;
  //TableProxy itsTable(table_);
  //Record rec;
  Vector<Double> rfs = restfreqCol_.getColumn();
  rfs.tovector(out);
  //rec = itsTable.getVarColumn("RESTFREQUENCY", 0, 1, 1);
  return out;
}

std::vector< double > asap::STMolecules::getRestFrequency( uInt id ) const
{
  std::vector<double> out;
  Table t = table_(table_.col("ID") == Int(id) );
  if (t.nrow() == 0 ) {
    throw(AipsError("STMolecules::getRestFrequency - id out of range"));
  }
  ROTableRow row(t);
  const TableRecord& rec = row.get(0);
  //return double(rec.asDouble("RESTFREQUENCY"));
  Vector<Double> rfs = rec.asArrayDouble("RESTFREQUENCY");
  rfs.tovector(out);
  return out;
}

int asap::STMolecules::nrow() const
{
  return int(table_.nrow());
}

}//namespace


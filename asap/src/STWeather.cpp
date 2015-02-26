//
// C++ Implementation: STWeather
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

#include "STWeather.h"


using namespace casa;

namespace asap {

const casa::String STWeather::name_ = "WEATHER";

STWeather::STWeather(const Scantable& parent) :
  STSubTable( parent, name_ )
{
  setup();
}


asap::STWeather::STWeather( casa::Table tab ) : STSubTable(tab, name_)
{
  temperatureCol_.attach(table_,"TEMPERATURE");
  pressureCol_.attach(table_,"PRESSURE");
  humidityCol_.attach(table_,"HUMIDITY");
  windspeedCol_.attach(table_,"WINDSPEED");
  windazCol_.attach(table_,"WINDAZ");

}

STWeather::~STWeather()
{
}

STWeather & asap::STWeather::operator =( const STWeather & other )
{
  if ( this != &other ) {
    static_cast<STSubTable&>(*this) = other;
    temperatureCol_.attach(table_,"TEMPERATURE");
    pressureCol_.attach(table_,"PRESSURE");
    humidityCol_.attach(table_,"HUMIDITY");
    windspeedCol_.attach(table_,"WINDSPEED");
    windazCol_.attach(table_,"WINDAZ");
  }
  return *this;
}


void asap::STWeather::setup( )
{
  // add to base class table
  table_.addColumn(ScalarColumnDesc<Float>("TEMPERATURE"));
  table_.addColumn(ScalarColumnDesc<Float>("PRESSURE"));
  table_.addColumn(ScalarColumnDesc<Float>("HUMIDITY"));
  table_.addColumn(ScalarColumnDesc<Float>("WINDSPEED"));
  table_.addColumn(ScalarColumnDesc<Float>("WINDAZ"));

  // new cached columns
  temperatureCol_.attach(table_,"TEMPERATURE");
  pressureCol_.attach(table_,"PRESSURE");
  humidityCol_.attach(table_,"HUMIDITY");
  windspeedCol_.attach(table_,"WINDSPEED");
  windazCol_.attach(table_,"WINDAZ");
}

uInt STWeather::addEntry( Float temp, Float pressure, Float humidity,
                          Float wspeed, Float waz )
{
  /// @todo this is a zero implementation as none of the telescopes
  /// fills in this information (yet)
// test if this already exists
  Table result = table_( near(table_.col("TEMPERATURE"), temp)
                         && near(table_.col("PRESSURE"), pressure)
                         && near(table_.col("WINDSPEED"), wspeed)
                         && near(table_.col("WINDAZ"), waz)
                         && near(table_.col("HUMIDITY"), humidity), 1 );
  uInt resultid = 0;
  if ( result.nrow() > 0) {
    ROScalarColumn<uInt> c(result, "ID");
    c.get(0, resultid);
  } else {
    uInt rno = table_.nrow();
    table_.addRow();
    // get last assigned freq_id and increment
    if ( rno > 0 ) {
      idCol_.get(rno-1, resultid);
      resultid++;
    }
    temperatureCol_.put(rno, temp);
    pressureCol_.put(rno, pressure);
    windspeedCol_.put(rno, wspeed);
    windazCol_.put(rno, waz);
    humidityCol_.put(rno, humidity);
    idCol_.put(rno, resultid);
  }
  return resultid;
}

void STWeather::getEntry( Float& temperature, Float& pressure,
                          Float& humidity, Float& windspeed, Float& windaz,
                          uInt id ) const
{
  Table t = table_(table_.col("ID") == Int(id), 1 );
  if (t.nrow() == 0 ) {
    throw(AipsError("STWeather::getEntry - id out of range"));
  }
  ROTableRow row(t);
  // get first row - there should only be one matching id
  const TableRecord& rec = row.get(0);
  temperature = rec.asFloat("TEMPERATURE");
  pressure = rec.asDouble("PRESSURE");
  humidity = rec.asDouble("HUMIDITY");
  windspeed = rec.asDouble("WINDSPEED");
  windaz = rec.asDouble("WINDAZ");
}

}


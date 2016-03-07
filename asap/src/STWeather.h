//
// C++ Interface: STWeather
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTWEATHER_H
#define ASAPSTWEATHER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>

#include "STSubTable.h"

namespace asap {

/**
The Weather subtable of the Scantable

@author Malte Marquarding
*/
class STWeather : public STSubTable {
public:
  STWeather() {;}
  explicit STWeather(casa::Table tab);
  explicit STWeather(const Scantable& parent);

  virtual ~STWeather();

  STWeather& operator=(const STWeather& other);

  casa::uInt addEntry( casa::Float temperature, casa::Float pressure,
                       casa::Float humidity,
                       casa::Float windspeed, casa::Float windaz);

  void getEntry( casa::Float& temperature, casa::Float& pressure,
                       casa::Float& humidity,
                       casa::Float& windspeed, casa::Float& windaz,
                       casa::uInt id) const;

  const casa::String& name() const { return name_; }

private:
  void setup();
  static const casa::String name_;
  //casa::Table table_;
  //casa::ScalarColumn<casa::uInt> freqidCol_;
  casa::ScalarColumn<casa::Float> pressureCol_, temperatureCol_,
                                  humidityCol_,
                                  windspeedCol_, windazCol_;
};

}

#endif

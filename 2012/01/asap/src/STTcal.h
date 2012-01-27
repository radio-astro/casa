//
// C++ Interface: STTcal
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTTCAL_H
#define ASAPSTTCAL_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include "STSubTable.h"

namespace asap {

/**
The Tcal subtable of the Scantable

@author Malte Marquarding
*/
class STTcal : public STSubTable {
public:
  STTcal() {;}
  explicit STTcal(casa::Table tab);
  explicit STTcal( const Scantable& parent);

  virtual ~STTcal();

  STTcal& operator=(const STTcal& other);

  casa::uInt addEntry( const casa::String& time,
                       const casa::Vector<casa::Float>& tcal);
  void getEntry( casa::String& time, casa::Vector<casa::Float>& tcal,
                 casa::uInt id );

  const casa::String& name() const { return name_; }

private:
  void setup();
  static const casa::String name_;
  //casa::Table table_;
  casa::ArrayColumn<casa::Float> tcalCol_;
  casa::ScalarColumn<casa::String> timeCol_;
};

}

#endif

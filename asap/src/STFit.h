//
// C++ Interface: STFit
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTFIT_H
#define ASAPSTFIT_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include "STSubTable.h"
namespace asap {

class STFitEntry;
/**
The Fit subtable of the Scantable

@author Malte Marquarding
*/
class STFit : public STSubTable {
public:
  STFit() {;}
  explicit STFit(casa::Table tab);
  explicit STFit( const Scantable& parent);

  virtual ~STFit();

  STFit& operator=(const STFit& other);

  casa::uInt addEntry( const STFitEntry& fit, casa::Int id=-1 );
  void getEntry( STFitEntry& fit, casa::uInt id ) const;

  const casa::String& name() const { return name_; }

private:
  void setup();
  static const casa::String name_;
  casa::ArrayColumn<casa::String> funcCol_;
  casa::ArrayColumn<casa::Int> compCol_;
  casa::ArrayColumn<casa::Double> parCol_;
  casa::ArrayColumn<casa::Bool> maskCol_;
  casa::ArrayColumn<casa::String> frameCol_;
};

}

#endif

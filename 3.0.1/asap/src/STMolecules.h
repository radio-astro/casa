//
// C++ Interface: STMolecules
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTMOLECULES_H
#define ASAPSTMOLECULES_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>

#include "STSubTable.h"

namespace asap {

/**
The Molecules subtable of the Scantable

@author Malte Marquarding
*/
class STMolecules : public STSubTable {
public:
  STMolecules() {;}
  explicit STMolecules(casa::Table tab);
  explicit STMolecules( const Scantable& parent);

  virtual ~STMolecules();

  STMolecules& operator=(const STMolecules& other);

/***
  casa::uInt addEntry( casa::Double restfreq, const casa::String& name="",
                       const casa::String& formattedname="");
***/

  casa::uInt addEntry( casa::Vector<casa::Double> restfreq, const casa::Vector<casa::String>& name=casa::Vector<casa::String>(0),
                       const casa::Vector<casa::String>& formattedname=casa::Vector<casa::String>(0));

/***
  void getEntry( casa::Double& restfreq, casa::String& name,
                 casa::String& formattedname, casa::uInt id) const;
***/
  void getEntry( casa::Vector<casa::Double>& restfreq, casa::Vector<casa::String>& name,
                 casa::Vector<casa::String>& formattedname, casa::uInt id) const;

  std::vector<double> getRestFrequencies() const;
  std::vector<double> getRestFrequency( casa::uInt id ) const;
  const casa::String& name() const { return name_; }
  int nrow() const;

private:
  void setup();
  static const casa::String name_;
  //casa::Table table_;
  //casa::ScalarColumn<casa::uInt> freqidCol_;
  //casa::ScalarColumn<casa::Double> restfreqCol_;
  casa::ArrayColumn<casa::Double> restfreqCol_;
  //casa::ScalarColumn<casa::String> nameCol_;
  casa::ArrayColumn<casa::String> nameCol_;
  //casa::ScalarColumn<casa::String> formattednameCol_; // e.g. latex
  casa::ArrayColumn<casa::String> formattednameCol_; // e.g. latex

};

}

#endif

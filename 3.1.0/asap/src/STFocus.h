//
// C++ Interface: STFocus
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTFOCUS_H
#define ASAPSTFOCUS_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>

#include "STSubTable.h"

namespace asap {

/**
The Focus subtable of the Scantable

@author Malte Marquarding
*/
class STFocus : public STSubTable {
public:
  STFocus() {;}
  explicit STFocus(casa::Table tab);
  explicit STFocus( const Scantable& parent );

  virtual ~STFocus();

  STFocus& operator=(const STFocus& other);

  casa::uInt addEntry( casa::Float pa, casa::Float faxis, casa::Float ftan,
                       casa::Float frot, casa::Float hand=1.0f,
                       casa::Float mount=0.0f, casa::Float user=0.0f,
                       casa::Float xyphase=0.0f, 
                       casa::Float xyphaseoffset=0.0f);

  void getEntry( casa::Float& pa, casa::Float& fax, casa::Float& ftan,
                 casa::Float& frot, casa::Float& hand,
                 casa::Float& mount, casa::Float& user,
                 casa::Float& xyphase, casa::Float& xyphaseoffset,
                 casa::uInt id) const;

  casa::Float getTotalAngle(casa::uInt id) const;

  casa::Float getParAngle(casa::uInt id) const {
    return parangleCol_(id);
  }
  casa::Float getFeedHand(casa::uInt id) const;

  void setParallactify(bool istrue=false);

  const casa::String& name() const { return name_; }

private:
  void setup();
  static const casa::String name_;
  casa::ScalarColumn<casa::Float> rotationCol_, axisCol_,
    tanCol_,handCol_, parangleCol_,
    mountCol_,userCol_, xyphCol_,xyphoffCol_,;
};

}

#endif

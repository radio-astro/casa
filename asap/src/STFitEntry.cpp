//
// C++ Implementation: STFitEntry
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "STFitEntry.h"

namespace asap {

STFitEntry::STFitEntry()
{
}
STFitEntry::STFitEntry(const STFitEntry& other)
{
  if ( this != &other ) {
    this->functions_ = other.functions_;
    this->components_ = other.components_;
    this->parameters_ = other.parameters_;
    this->parmasks_ = other.parmasks_;
    this->frameinfo_ = other.frameinfo_;
  }
}


STFitEntry::~STFitEntry()
{
}


}

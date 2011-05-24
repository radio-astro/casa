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
#include <casa/iostream.h>

using namespace casa;
namespace asap {

  STFitEntry::STFitEntry()
  /* : 
    functions_( std::vector<std::string>()),
    components_(std::vector<int>()),
    parameters_(std::vector<float>()),
    errors_(std::vector<float>()),
    parmasks_(std::vector<bool>()),
    frameinfo_(std::vector<std::string>())
  */
{
}
STFitEntry::STFitEntry(const STFitEntry& other)
{
  if ( this != &other ) {
    this->functions_ = std::vector<std::string>();
    this->components_ = other.components_;
    this->parameters_ = other.parameters_;
    this->errors_ = other.errors_;
    this->frameinfo_ = other.frameinfo_;
  }
}

STFitEntry& STFitEntry::operator=(const STFitEntry& other)
{
  if ( this != &other ) {
    this->functions_ = other.functions_;
    this->components_ = other.components_;
    this->parameters_ = other.parameters_;
    this->errors_ = other.errors_;
    this->parmasks_ = other.parmasks_;
    this->frameinfo_ = other.frameinfo_;
  }
  return *this;
}

STFitEntry::~STFitEntry()
{
}


}

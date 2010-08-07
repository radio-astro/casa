//
// C++ Interface: Filler
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPFILLER_H
#define ASAPFILLER_H

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/OS/File.h>

#include <string>

#include "ScantableWrapper.h"
#include "FillerBase.h"
#include "PKSFiller.h"
#include "NROFiller.h"


namespace asap
{
class FillerWrapper
{
public:

  explicit FillerWrapper(ScantableWrapper tbl) : filler_(0), attached_(false)
  { stable_ = tbl.getCP(); }

  virtual ~FillerWrapper() { close(); }


//  void open(const std::string& filename, casa::Record rec) {
  void open(const std::string& filename) {
    casa::File file(filename);
    if ( !file.exists() ) {
      throw(AipsError("File does not exist"));
    }
    filler_ = new PKSFiller(stable_);
//    if (filler_->open(filename, rec)) {
    if (filler_->open(filename)) {
      attached_ = true;
      return;
    }
    filler_ = new NROFiller(stable_);
//    if (filler_->open(filename, rec)) {
    if (filler_->open(filename)) {
      attached_ = true;
      return;
    }
    filler_ = 0;
    attached_ = false;
    throw casa::AipsError("Unknown Data Format");
  }
  void close() {
    if (attached_) {
      filler_->close();
    }
  }

  void fill() {
    if (attached_) {
      filler_->fill();
    }
  }

  void setReferenceRegex(const std::string& rx) {
    if (attached_) {
      filler_->setReferenceRegex(rx);
    }
  }

private:

  FillerWrapper();
  FillerWrapper(const FillerWrapper&);
  FillerWrapper& operator=(const FillerWrapper&);

  casa::CountedPtr<FillerBase> filler_;
  bool attached_;
  casa::CountedPtr<Scantable> stable_;
};

};
#endif

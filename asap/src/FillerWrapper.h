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
#include <casa/Containers/Record.h>
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


  void open(const std::string& filename, const casa::Record& rec) {
    //  void open(const std::string& filename) {
    casa::File file(filename);
    if ( !file.exists() ) {
      throw(AipsError("File does not exist"));
    }
    int fileType = dataType( filename ) ;
    if ( fileType == 0 ) {
      filler_ = new PKSFiller(stable_);
      if (filler_->open(filename, rec)) {
        attached_ = true;
        return;
      }
    }
    else if ( fileType == 1 ) {
      filler_ = new NROFiller(stable_);
      if (filler_->open(filename, rec)) {
        attached_ = true;
        return;
      }
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

  int dataType( const std::string &filename ) {
    int ret = -1 ;
    int pks = 0 ;
    int nro = 1 ;
    casa::File file( filename ) ;
    if ( file.isDirectory() )
      ret = pks ;
    else if ( file.isReadable() ) {
      // if we want to compare to 6 characters we should only read in 6
      FILE *f = fopen( filename.c_str(), "r") ;
      char buf[7] ;
      fread( buf, 6, 1, f ) ;
      fclose( f ) ;
      buf[6]='\0' ;
      // NRO data has two types:
      //  1) specific binary data for OTF observation
      //  2) (pseudo-)FITS data that doesn't have primary HDU
      // So, one can distinguish NRO and non-NRO data by examining 
      // first keyword name.
      if ( casa::String( buf ) == "SIMPLE" ) {
        ret = pks ;
      }
      else {
        ret = nro ;
      }
    }
    return ret ;
  }

  FillerWrapper();
  FillerWrapper(const FillerWrapper&);
  FillerWrapper& operator=(const FillerWrapper&);

  casa::CountedPtr<FillerBase> filler_;
  bool attached_;
  casa::CountedPtr<Scantable> stable_;
};

};
#endif

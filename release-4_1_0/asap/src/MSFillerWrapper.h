//
// C++ Interface: MSFillerWrapper
//
// Description:
//
// This class is wrapper class for MSFiller
// 
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPMSFILLER_WRAPPER_H
#define ASAPMSFILLER_WRAPPER_H

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/Record.h>
#include <casa/OS/File.h>

#include <string>

#include "ScantableWrapper.h"
#include "MSFiller.h" 

namespace asap
{

class MSFillerWrapper
{
public:
  explicit MSFillerWrapper( ScantableWrapper tbl )
    : filler_( 0 ),
      attached_( false )
  { stable_ = tbl.getCP() ; }

  virtual ~MSFillerWrapper() { close() ; }

  void open(const std::string& filename, const casa::Record& rec) 
  {
    casa::File file( filename ) ;
    if ( !file.exists() ) {
      throw casa::AipsError( "File does not exist" ) ;
    }
    filler_ = new MSFiller( stable_ ) ;
    if ( filler_->open( filename, rec ) ) {
      attached_ = true ;
      return ;
    }
    else {
      throw casa::AipsError( "Failed to open file" ) ;
    }
  }

  void close() 
  {
    if ( attached_ ) {
      filler_->close() ;
    }
  }

  void fill()
  {
    if ( attached_ ) {
      filler_->fill() ;
    }
  }

  // add dummy method for consistency
  void setReferenceRegex(const std::string& rx) {
    // do nothing
  }  

private:

  MSFillerWrapper() ;
  MSFillerWrapper(const MSFillerWrapper&) ;
  MSFillerWrapper& operator=(const MSFillerWrapper&) ;

  casa::CountedPtr<MSFiller> filler_ ;
  bool attached_ ;
  casa::CountedPtr<Scantable> stable_ ;
};


};
#endif

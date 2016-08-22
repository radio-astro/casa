//
// C++ Interface: MSWriterWrapper
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
#ifndef ASAPMSWRITER_WRAPPER_H
#define ASAPMSWRITER_WRAPPER_H

#include <casa/aips.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Containers/Record.h>
#include <casa/OS/File.h>

#include <string>

#include "ScantableWrapper.h"
#include "MSWriter.h" 
#include "GILHandler.h"

namespace asap
{

class MSWriterWrapper
{
public:
  explicit MSWriterWrapper( ScantableWrapper tbl )
    : writer_( 0 )
  { stable_ = tbl.getCP() ; }

  virtual ~MSWriterWrapper() {}

  void write(const std::string& filename, const casa::Record& rec) 
  {
    GILHandler scopedRelease;

    casa::File file( filename ) ;
    writer_ = new MSWriter( stable_ ) ;
    if ( writer_->write( filename, rec ) ) {
      return ;
    }
    else {
      throw casa::AipsError( "Failed to write data" ) ;
    }
  }

private:

  MSWriterWrapper() ;
  MSWriterWrapper(const MSWriterWrapper&) ;
  MSWriterWrapper& operator=(const MSWriterWrapper&) ;

  casa::CountedPtr<MSWriter> writer_ ;
  casa::CountedPtr<Scantable> stable_ ;
};


};
#endif

//
// C++ Interface: NROFiller
//
// Description:
//
// This class is a concrete class that derives from FillerBase class.
// The class implements the following methods to be able to read NRO 
// data (45m and ASTE).
//
//    open()
//    close()
//    fill()
//
// The fill() method usually iterates over the source data and calls
// the setXYZ() methods for. After all the data for a row has been set via
// these methods, the fill() method needs to call commitRow() to write the
// data to the scantable.
// All arguments which are defaulted in the setXYZ() methods are optional. All
// others should be set explicitly.
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPNROFILLER_H
#define ASAPNROFILLER_H

#include "FillerBase.h"

// STL
#include <string>
#include <vector>
// AIPS++
#include <casa/aips.h>
#include <atnf/PKSIO/NROReader.h>

namespace asap
{

class NROFiller : public FillerBase
{
  public:
    explicit NROFiller(casa::CountedPtr<Scantable> stable);
    virtual ~NROFiller();

    bool open(const std::string& filename) ;
    void fill() ;
    void close() ;

  protected:

  private:
    NROFiller();
    NROFiller(const NROFiller&);
    NROFiller& operator=(const NROFiller&);

    // pointer to the reader
    //NROReader *reader_ ;
    casa::CountedPtr<NROReader> reader_ ;
};


}
#endif

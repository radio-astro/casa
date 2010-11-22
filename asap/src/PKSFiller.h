//
// C++ Interface: PKSFiller
//
// Description:
//

// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPPKSFILLER_H
#define ASAPPKSFILLER_H

// STL
#include <string>
// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>


#include "FillerBase.h"
#include "Scantable.h"

class casa::Record;
class PKSreader;

namespace asap
{

class PKSFiller : public FillerBase
{
  public:
    explicit PKSFiller(casa::CountedPtr<Scantable> stable);
    virtual ~PKSFiller();

    bool open(const std::string& filename, const casa::Record& rec);
    //    bool open(const std::string& filename);
    void fill();
    void close();


  private:

    PKSFiller();
    PKSFiller(const PKSFiller&);
    PKSFiller& operator=(const PKSFiller&);

    casa::CountedPtr<PKSreader> reader_;
    casa::String filename_;
    casa::Int nIF_, nBeam_, nPol_, nChan_, nInDataRow;
    casa::Vector<casa::Bool> haveXPol_;
};


};
#endif

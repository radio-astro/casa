//
// C++ Interface: FillerBase
//
// Description:
//
// This class is the Base class for all data fillers.
// The derived filler needs to implement
// open()
// close()
// fill()
//
// The fill() method usually iterates over the source data and calls
// the setXYZ() methods for. After all the data for a row has been set via
// these methods, the fill() method needs to call commitRow() to write the
// data to the scantable.
// All arguments which are defaulted in the setXYZ() methods are optional. All
// others should be set explicitly.
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPFILLERBASE_H
#define ASAPFILLERBASE_H

// STL
#include <string>
// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <tables/Tables/TableRow.h>
#include "Scantable.h"

namespace asap
{

class FillerBase
{
  public:
    explicit FillerBase(casa::CountedPtr<Scantable> stable);
    virtual ~FillerBase() {;}

//    virtual bool open(const std::string& filename, const Record& rec) = 0;
    virtual bool open(const std::string& filename) = 0;
    virtual void fill() = 0;
    virtual void close() = 0;

    void setReferenceRegex(const std::string& rx) { referenceRx_ = rx; }
    std::string getReferenceRegex() { return referenceRx_;  }

  protected:

    void commitRow();
    void setHeader(const STHeader& header);
    void setSpectrum(const casa::Vector<casa::Float>& spectrum,
                             const casa::Vector<casa::uChar>& flags,
                             const casa::Vector<casa::Float>& tsys);
    void setFlagrow(casa::uInt flag);
    void setOpacity(casa::Float opacity=0.0f);
    void setIndex(casa::uInt scanno, casa::uInt cycleno,
                          casa::uInt ifno, casa::uInt polno,
                          casa::uInt beamno=0);
    void setFrequency(casa::Double refpix, casa::Double refval,
                              casa::Double incr);
    void setMolecule(const casa::Vector<casa::Double>& restfreq);
    void setDirection(const casa::Vector<casa::Double>& dir,
                              casa::Float az=0.0f, casa::Float el=0.0f);

    void setFocus(casa::Float pa=0.0f, casa::Float faxis=0.0f,
                          casa::Float ftan=0.0f, casa::Float frot=0.0f);
    void setTime(casa::Double mjd, casa::Double integration);
    void setWeather(casa::Float temperature=0.0f,
                            casa::Float pressure=0.0f,
                            casa::Float humidity=0.0f,
                            casa::Float windspeed=0.0f,
                            casa::Float windaz=0.0f);
    void setTcal(const casa::String& caltime="",
                         const casa::Vector<casa::Float>& tcal=casa::Vector<casa::Float>());
    void setScanRate(const casa::Vector<casa::Double>& srate=casa::Vector<casa::Double>());
    void setReferenceBeam(casa::Int beamno=-1);
    void setSource(const std::string& name, casa::Int type,
                           const std::string& fieldname="",
                           const casa::Vector<casa::Double>& dir=casa::Vector<casa::Double>(),
                           const casa::Vector<casa::Double>& propermot=casa::Vector<casa::Double>(),
                           casa::Double velocity=0.0);

    casa::CountedPtr< Scantable > table_;

  private:

    FillerBase();
    FillerBase(const FillerBase&);
    FillerBase& operator=(const FillerBase&);
    casa::String referenceRx_;
    casa::TableRow row_;
};


};
#endif

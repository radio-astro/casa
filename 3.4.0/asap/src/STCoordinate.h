//
// C++ Interface: STCoordinate
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTCOORDINATE_H
#define ASAPSTCOORDINATE_H

#include <vector>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>

namespace asap {

class STCoordinate {
  public:
    STCoordinate() {};

    STCoordinate(const STCoordinate& other) : spec_(other.spec_) {};

    STCoordinate(const casa::SpectralCoordinate& spec) :
      spec_(spec) {};

    virtual ~STCoordinate() {};

    double getReferencePixel() {
      return spec_.referencePixel()(0);
    };

    double getReferenceValue() {
      return spec_.referenceValue()(0);
    };

    double getIncrement() {
      return spec_.increment()(0);
    }

    double toFrequency(double pix) {
      casa::Double world;
      spec_.toWorld(world, pix);
      return world;
    }

    double toVelocity(double pix) {
      casa::Double vel;
      spec_.pixelToVelocity(vel, pix);
      return vel;
    }

    double toPixel(double world) {
      casa::Double pix;
      spec_.toPixel(pix, world);
      return pix;
    }

  private:
    casa::SpectralCoordinate spec_;
};

}

#endif

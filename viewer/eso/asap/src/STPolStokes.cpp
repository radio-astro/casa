//
// C++ Implementation: STPolStokes
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicMath/Math.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/Constants.h>
#include "STPolStokes.h"

using namespace casa;

namespace asap {

Factory<STPol,STPolStokes> STPolStokes::myFactory;

STPolStokes::~STPolStokes()
{
}


Vector<Float> asap::STPolStokes::getStokes( uint index )
{
  return getSpectrum(index);
}

Vector<Float> asap::STPolStokes::getLinPol( uInt index )
{
  if ( index < 0 || index >4 ) throw(AipsError("LinPol index out of range"));
  Vector<Float> out;
  if ( nspec() == 4) {
    switch(index) {
      case 1:
        out = Vector<Float>(sqrt(pow(getSpectrum(1),Float(2.0))
                                 +pow(getSpectrum(2), Float(2.0))));
        break;
      case 2:
        out = Vector<Float>(Float(180.0/C::pi/2.0)
                            * atan2(getSpectrum(2),getSpectrum(1)));
        break;
      default:
        out = getSpectrum(index);
    }
  }
  return out;
}

Vector<Float> asap::STPolStokes::getLinear(uInt index )
{
  Vector<Float> out;
  switch(index) {
  case 0:
    out = (getSpectrum(0)+getSpectrum(1))/Float(2.0);
    break;
  case 1:
    out = (getSpectrum(0)-getSpectrum(1))/Float(2.0);
    break;
  case 2:
    out = getSpectrum(2)/Float(2.0);
    break;
  case 3:
    out = getSpectrum(3)/Float(2.0);
    break;
  default:
    out = Vector<Float>();
  }
  return out;

}

Vector<Float> asap::STPolStokes::getCircular(uInt index )
{
  // convert to stokes I/ V first
  //
  //   We use the convention
  //    I = (RR+LL)  // definition changed

  if ( index == 2 || index ==3  ) throw(AipsError("Re/Imag RL not implemented"));
  Vector<Float> out;
  switch(index) {
  case 0:
    out = (getSpectrum(0) + getSpectrum(3))/Float(2.0);
    break;
  case 1:
    out = (getSpectrum(0) - getSpectrum(3))/Float(2.0);
    break;
  default:
    out = Vector<Float>();
  }
  return out;
}

}

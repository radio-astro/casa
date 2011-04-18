//
// C++ Implementation: STPolCircular
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
#include "STPolCircular.h"

using namespace casa;

namespace asap {

Factory<STPol,STPolCircular> STPolCircular::myFactory;

STPolCircular::~STPolCircular()
{
}


Vector<Float> asap::STPolCircular::getStokes( uint index )
{
  /// @todo implement full stokes support when circular cross pols are available
  Vector<Float> out;
  if (nspec() == 2) {
    if ( index == 0 )
      out = Vector<Float>(getSpectrum(0) + getSpectrum(1));
    else if ( index == 3 )
      out = Vector<Float>(getSpectrum(0) - getSpectrum(1));
  }
  return out;
}

Vector<Float> asap::STPolCircular::getLinPol( uInt index )
{
  if (nspec() != 4) {
    throw(AipsError("You must have 4 circular polarisations to run this function"));
  }
  if ( index < 0 || index >4 ) throw(AipsError("LinPol index out of range"));
  Vector<Float> out,q,u;
  if ( nspec() == 4) {
    switch(index) {
      case 1:
        q = getStokes(1);
        u = getStokes(2);
        out = Vector<Float>(sqrt(pow(q,Float(2.0))+pow(u, Float(2.0))));
        break;
      case 2:
        q = getStokes(1);
        u = getStokes(2);
        out = Vector<Float>(Float(180.0/C::pi/2.0) * atan2(u,q));
        break;
      default:
        out = getStokes(index);
    }
  }
  return out;
}

Vector<Float> asap::STPolCircular::getCircular(uInt index )
{
  return getSpectrum(index);
}

Vector<Float> asap::STPolCircular::getLinear(uInt index )
{
  // convert to stokes I/ V first
  //
  //   We use the convention
  //    I = (XX+YY)  // definition changed

  if (nspec() != 4) {
    throw(AipsError("You must have 4 circular polarisations to run this function"));
  }
  if ( index == 2 || index ==3  ) throw(AipsError("Re/Imag XY not implemented"));
  Vector<Float> I,V,out;
  I = getStokes(0);
  V = getStokes(3);
  switch(index) {
  case 0:
    out = (I + V)/Float(2.0);
    break;
  case 1:
    out = (I - V)/Float(2.0);
    break;
  default:
    out = Vector<Float>();
  }
  return out;
}

}

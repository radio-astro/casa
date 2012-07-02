//
// C++ Implementation: STPolLinear
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
#include "STPolLinear.h"

using namespace casa;

namespace asap {

Factory<STPol,STPolLinear> STPolLinear::myFactory;

STPolLinear::~STPolLinear()
{
}


Vector<Float> asap::STPolLinear::getStokes( uint index )
{
  if ( index < 0 || index >4 ) throw(AipsError("Stokes index out of range"));
  Vector<Float> out;
  Float phase = getTotalPhase();
  Vector<Float> q(getSpectrum(0) - getSpectrum(1));
  if ( nspec() == 4 ) {
    switch(index) {
      case 0:
        out = Vector<Float>(getSpectrum(0) + getSpectrum(1));
        break;
      case 1:
        out = Vector<Float>(q * cos(phase) - Float(2.0)*getSpectrum(2) * sin(phase));
        break;
      case 2:
        out = Vector<Float>(q * sin(phase) + Float(2.0)*getSpectrum(2) * cos(phase));
        break;
      case 3:
        out = getFeedHand() * Float(2.0) * Vector<Float>(getSpectrum(3));
        break;
    }
  } else if (nspec() == 2) {
    if ( index == 0 )
      out = Vector<Float>(getSpectrum(0) + getSpectrum(1));
  }
  return out;
}

Vector<Float> asap::STPolLinear::getLinPol( uInt index )
{
  if (nspec() != 4) {
    throw(AipsError("You must have 4 linear polarizations to run this function"));
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

Vector<Float> asap::STPolLinear::getLinear(uInt index )
{
  return getSpectrum(index);
}

Vector<Float> asap::STPolLinear::getCircular(uInt index )
{
  // convert to stokes I/ V first
  //
  //   We use the convention
  //    I = (RR+LL)  // definition changed
  if (nspec() != 4) {
    throw(AipsError("You must have 4 linear polarizations to run this function"));
  }
  if ( index == 2 || index ==3  ) throw(AipsError("Re/Imag RL not implemented"));
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

void asap::STPolLinear::rotatePhase( Float phase )
{
  if (nspec() != 4) {
    throw(AipsError("You must have 4 linear polarizations to run this function"));
  }
  Float cosVal = cos(C::pi/180.0*phase);
  Float sinVal = sin(C::pi/180.0*phase);
  Matrix<Float>& specs = getSpectra();
  Vector<Float> R2 = specs.column(2)*cosVal - specs.column(3)*sinVal;
  specs.column(3) =  specs.column(2)*sinVal + specs.column(3)*cosVal;
  specs.column(2) = R2;
}

void asap::STPolLinear::invertPhase( Float phase )
{
  // phase isnt used, just ro keep interface the same for all pol operations
  if (nspec() != 4) {
    throw(AipsError("You must have 4 linear polarizations to run this function"));
  }
  Matrix<Float>& specs = getSpectra();
  Vector<Float> I = specs.column(3);
  I *= Float(-1.0);
}

void asap::STPolLinear::rotateLinPolPhase( casa::Float phase )
{
//
// Rotate P = Q + iU but do it directly on the  linear
// correlations.
//
// We are using I=(XX+YY)/2 convention
// C1 = XX; C2 = YY, C3 = Real(XY)
//
  if (nspec() != 4) {
    throw(AipsError("You must have 4 linear polarizations to run this function"));
  }
  Vector<Float> I,Q,U;
  I = getStokes(0);
  Q = getStokes(1);
  U = getStokes(2);
  // Rotate Q & U (factor of 2 for polarization)
  Float cosVal = cos(C::pi/180.0*2.0*phase);
  Float sinVal = sin(C::pi/180.0*2.0*phase);
  Vector<Float> Q2 = Q*cosVal - U*sinVal;
  U =  Q*sinVal + U*cosVal;
  Q = Q2;
  Matrix<Float>& specs = getSpectra();
  specs.column(0) = (I+Q)/Float(2.0);
  specs.column(1) = (I-Q)/Float(2.0);
  specs.column(2) = U/Float(2.0);

}

}

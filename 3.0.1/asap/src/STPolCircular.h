//
// C++ Interface: STPolLinear
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTPOLCircular_H
#define ASAPSTPOLCircular_H

#include "Factory.h"
#include "STPol.h"

namespace asap {

/**
  The Circular representation of polarisation.
  NOTE  U and V are probably wrong
  We are using the following convention:
  @li I = RR + LL
  @li Q = RR - LL
  @li U = 2*Real(RL)
  @li V = 2*Imag(RL)

  resulting in:
  @li I' = I
  @li Q' = Q * cos(theta) - V *sin(theta)
  @li U' = Q * sin(theta) + U * cos(theta)
  @li V' = V
  @author Malte Marquarding

*/
class STPolCircular : public STPol
{
public:
  STPolCircular() {}

  explicit STPolCircular(const casa::Matrix<casa::Float>& specs)
    { setSpectra(specs); }

  ~STPolCircular();

  static Factory<STPol,STPolCircular> myFactory;

  virtual casa::Vector<casa::Float> getCircular( casa::uInt index );

  virtual casa::Vector<casa::Float> getStokes( casa::uInt index);

  virtual casa::Vector<casa::Float> getLinPol( casa::uInt index);

  virtual casa::Vector<casa::Float> getLinear( casa::uInt index );

};

}

#endif

//
// C++ Interface: STPolStokes
//
// Description:
//
//
// Author: Malte Marquarding <Malte.Marquarding@csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPSTPOLSTOKES_H
#define ASAPSTPOLSTOKES_H

#include "Factory.h"
#include "STPol.h"

namespace asap {

/**
The stokes representation of polarisation

@author Malte Marquarding
*/
class STPolStokes : public STPol
{
public:
  STPolStokes() {}

  explicit STPolStokes(const casa::Matrix<casa::Float>& specs)
    { setSpectra(specs); }

  ~STPolStokes();

  static Factory<STPol,STPolStokes> myFactory;

  virtual casa::Vector<casa::Float> getCircular( casa::uInt index );

  virtual casa::Vector<casa::Float> getStokes( casa::uInt index);

  virtual casa::Vector<casa::Float> getLinPol( casa::uInt index);

  virtual casa::Vector<casa::Float> getLinear( casa::uInt index );

  //virtual void rotatePhase( casa::Float phase );
  //virtual void rotateLinPolPhase( casa::Float phase );
  //virtual void invertPhase( casa::Float phase );

};

}

#endif

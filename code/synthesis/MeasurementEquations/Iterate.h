//# Iterate.h:
//# Copyright (C) 1996,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_ITERATE_H
#define SYNTHESIS_ITERATE_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Fallible.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// Base class for Iteration
class Iterate {
public:
  // Constructor
  Iterate() : numberIterations_(100), gain_(1.0),
       tolerance_(0.000001), threshold_(0.0), free_(False), mode_("") {};

  virtual ~Iterate() {};

  // Is this a free variable?
  void setFree()
    {free_=True;};
  void setNotFree()
    {free_=False;};
  Bool free()
    {return(free_);};

  void setNumberIterations(const Int n) {numberIterations_=n;};
  void setGain(const Float g) {gain_=g;};
  void setTolerance(const Float t) {tolerance_=t;};
  void setThreshold(const Float t) {threshold_=t;};
  void setMode(const String m) {mode_=m;};

  Int numberIterations() {return numberIterations_;};
  Float gain() {return gain_;};
  Float tolerance() {return tolerance_;};
  virtual Float threshold() {return threshold_;};
  const String mode() {return mode_;};

private:    
  Int numberIterations_;
  Double gain_;
  Double tolerance_;
  Double threshold_;
  Bool free_;
  String mode_;
};


} //# NAMESPACE CASA - END

#endif

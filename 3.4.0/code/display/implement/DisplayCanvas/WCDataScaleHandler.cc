//# WCDataScaleHandler.cc: data scale handling for WorldCanvas
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//# 
//# $Id$

#include <casa/BasicMath/Math.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/DisplayCanvas/WCDataScaleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
WCDataScaleHandler::WCDataScaleHandler() :
  domainMin_(0), 
  domainMax_(1.0), 
  rangeMax_(255) {
}

// Destructor
WCDataScaleHandler::~WCDataScaleHandler() {;}

// User Constructor
WCDataScaleHandler::WCDataScaleHandler(Double min, Double max) : 
  domainMin_(min), 
  domainMax_(max), 
  rangeMax_(255) {
}

// Install the default options for this object.
void WCDataScaleHandler::setDefaultOptions() {
  // nothing yet
}

// Apply new options to this object.
Bool WCDataScaleHandler::setOptions(Record &, Record &) {
  // nothing yet
  return False;
}

// Retrieve the current and default options for this object.
Record WCDataScaleHandler::getOptions() {
  // nothing yet
  Record rec;
  return rec;
}

// Default Boolean definition maps True to the max, False to zero
Bool WCDataScaleHandler::operator()(Array<uInt> & out, const Array<Bool> & in)
{
  out.resize(in.shape());
  Bool inDel;
  const Bool * inp = in.getStorage(inDel);
  Bool outDel;
  uInt * outp = out.getStorage(outDel);
  uInt * endp = outp + out.nelements();
  uInt * p = outp;
  const Bool * q = inp;
  while (p < endp)
    *p++ = (*q++) ? rangeMax() : 0;
  in.freeStorage(inp, inDel);
  out.putStorage(outp, outDel);
  return True;
}

// Default floating-point definitions not implemented
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<uChar> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<Char> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<uShort> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<Short> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<uInt> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<Int> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<uLong> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<Long> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<Float> &) {
  return False;
}
Bool WCDataScaleHandler::operator()(Array<uInt> &, const Array<Double> &) {
  return False;
}

// Default Complex definitions plot amplitude
Bool WCDataScaleHandler::operator()(Array<uInt> & out, 
				    const Array<Complex> & in) {
  out.resize(in.shape());
  Array<Float> tmp(in.shape());
  amplitude(tmp, in);
  return (*this)(out, tmp);
}
Bool WCDataScaleHandler::operator()(Array<uInt> & out, 
				    const Array<DComplex> & in) {
  out.resize(in.shape());
  Array<Double> tmp(in.shape());
  amplitude(tmp, in);
  return (*this)(out, tmp);
}

void WCDataScaleHandler::setDomainMinMax(Double min, Double max) {
  domainMin_ = min;
  domainMax_ = max;
  if (near(domainMin_, domainMax_)) {
    domainMin_ -= 0.5;
    domainMax_ += 0.5;
  }
}



} //# NAMESPACE CASA - END


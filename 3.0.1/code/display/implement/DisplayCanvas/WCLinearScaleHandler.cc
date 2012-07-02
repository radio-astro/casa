//# WCLinearScaleHandler.cc: linear scaling of data values
//# Copyright (C) 1993,1994,1995,1996,1999,2000,2001
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

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/IPosition.h>
#include <casa/BasicSL/Constants.h>
#include <display/DisplayCanvas/WCLinearScaleHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
WCLinearScaleHandler::WCLinearScaleHandler() {
}

// Destructor
WCLinearScaleHandler::~WCLinearScaleHandler() {
}


#define WCLinearScaleHandlerSCALE(Type, TypeEpsilon) \
  out.resize(in.shape());                             \
  Bool inDel;                                          \
  const Type * inp = in.getStorage(inDel);              \
  Bool outDel;                                           \
  uInt * outp = out.getStorage(outDel);                   \
  Double f = (rangeMax()+1)/(domainMax() - domainMin() + TypeEpsilon); \
  uInt * endp = outp + out.nelements();                    \
  uInt * p = outp;                                         \
  const Type * q = inp;                                    \
  while (p < endp)                                         \
    *p++ = (uInt) (f*((*q++) - domainMin()));              \
  in.freeStorage(inp, inDel);                              \
  out.putStorage(outp, outDel);                            \
  return True;

// I think this version will not go outside the limits!

#define WCLinearScaleHandlerSCALE2(Type, TypeEpsilon) \
out.resize(in.shape()); \
Bool inDel; \
const Type * inp = in.getStorage(inDel); \
Bool outDel; \
uInt * outp = out.getStorage(outDel); \
uInt * endp = outp + out.nelements(); \
uInt * p = outp; \
const Type * q = inp; \
Double frac; \
Double precalc0 = domainMin(); \
Double precalc1 = 1.0 / (domainMax() - precalc0 + TypeEpsilon); \
Double precalc2 = rangeMax() + 1; \
while (p < endp) { \
  frac = (Double)(*q++ - precalc0) * precalc1; \
  if (frac < 0.0) { \
    frac = 0.0; \
  } else if (frac > 1.0) { \
    frac = 1.0; \
  } \
  *p++ = (uInt)(frac * precalc2); \
} \
in.freeStorage(inp, inDel); \
out.putStorage(outp, outDel); \
return True;

Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Bool> & in)
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

Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<uChar> & in)
{
  WCLinearScaleHandlerSCALE(uChar, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Char> & in)
{
  WCLinearScaleHandlerSCALE(Char, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<uShort> & in)
{
  WCLinearScaleHandlerSCALE(uShort, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Short> & in)
{
  WCLinearScaleHandlerSCALE(Short, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<uInt> & in)
{
  WCLinearScaleHandlerSCALE(uInt, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Int> & in)
{
  WCLinearScaleHandlerSCALE(Int, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<uLong> & in)
{
  WCLinearScaleHandlerSCALE(uLong, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Long> & in)
{
  WCLinearScaleHandlerSCALE(Long, 1);
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Float> & in)
{
  WCLinearScaleHandlerSCALE2(Float, FLT_EPSILON*(rangeMax()+1));
}
Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Double> & in)
{
  WCLinearScaleHandlerSCALE(Double, DBL_EPSILON*(rangeMax()+1));
}

Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<Complex> & in)
{
   return WCDataScaleHandler::operator()(out, in);
}

Bool WCLinearScaleHandler::operator()(Array<uInt> & out, const Array<DComplex> & in)
{
   return WCDataScaleHandler::operator()(out, in);
}




} //# NAMESPACE CASA - END


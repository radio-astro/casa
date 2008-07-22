//# TabulatedConvFunc.h: Definition for TabulatedConvFunc
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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

#ifndef SYNTHESIS_TABULATEDCONVFUNC_H
#define SYNTHESIS_TABULATEDCONVFUNC_H

#include <synthesis/MeasurementComponents/ConvolutionFunction.h>

namespace casa{

  class TabulatedConvFunc: public ConvolutionFunction
  {
  public:
    TabulatedConvFunc(): ConvolutionFunction()   {};
    TabulatedConvFunc(Int n): ConvolutionFunction(n) {};
    ~TabulatedConvFunc() {};

    virtual void setMaximumCacheSize(const Long howManyRealPixels=-1) {cacheSizeInPixels_p=howManyRealPixels;}
    virtual Long maximumCacheSize() {return cacheSizeInPixels_p;}
    virtual void initTables(Int n){};
  private:
    Long cacheSizeInPixels_p;
  };

};
#endif

//# PixelatedConvFunc.h: Definition for PixelatedConvFunc
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

#ifndef SYNTHESIS_PIXELATEDCONVFUNC_H
#define SYNTHESIS_PIXELATEDCONVFUNC_H

#include <casa/Arrays/Vector.h>
//#include <synthesis/MeasurementComponents/Exp.h>
#include <synthesis/MeasurementComponents/CExp.h>
#include <synthesis/TransformMachines/ConvolutionFunction.h>

namespace casa{

  template <class T> class PixelatedConvFunc: public ConvolutionFunction
  {
  public:
    PixelatedConvFunc():ConvolutionFunction() {};
    PixelatedConvFunc(casacore::IPosition& shape):ConvolutionFunction(shape.nelements()) 
    {setSize(shape);}
    ~PixelatedConvFunc() {};
    
    virtual void setSize(casacore::IPosition& size)
    {
      nDim=size.nelements(); 
      shape=size;
      cache.resize(shape);
    };

    virtual T getValue(casacore::Vector<T>& coord, casacore::Vector<T>& offset) { return (T)(1.0);};
    virtual int getVisParams(const VisBuffer& vb,const casacore::CoordinateSystem& skyCoord=casacore::CoordinateSystem()) {return 0;};
    virtual void makeConvFunction(const casacore::ImageInterface<casacore::Complex>& image,
				  const VisBuffer& vb,
				  const casacore::Int wConvSize,
				  const casacore::Float pa,
				  const casacore::Float dpa,
				  CFStore& cfs,
				  CFStore& cfwts,casacore::Bool fillCF=true) {};
    virtual void setPolMap(const casacore::Vector<casacore::Int>& polMap) {(void)polMap;};
    virtual void setFeedStokes(const casacore::Vector<casacore::Int>& feedStokes){(void)feedStokes;};
    PixelatedConvFunc& operator=(const PixelatedConvFunc& other)
    {
      cerr << "******* PixelatedConvFunc& operator=(PixelatedConvFunc&) called!" << endl;
      return *this;
    }
    PixelatedConvFunc& operator=(const ConvolutionFunction& other)
    {
      cerr << "******* PixelatedConvFunc& operator=(ConvolutionFunction&) called!" << endl;
      return *this;
    }
    virtual casacore::Bool makeAverageResponse(const VisBuffer& vb, 
				     const casacore::ImageInterface<casacore::Complex>& image,
				     casacore::ImageInterface<casacore::Complex>& theavgPB,
				     casacore::Bool reset=true)
    {throw(casacore::AipsError("PixelatedConvFunc::makeAverageRes(casacore::Complex) called"));};
    virtual void prepareConvFunction(const VisBuffer& vb, CFStore& cfs) {};

  private:
    casacore::Int nDim;
    casacore::IPosition shape;
    casacore::Array<T> cache;
  };

};

#endif

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
    PixelatedConvFunc(IPosition& shape):ConvolutionFunction(shape.nelements()) 
    {setSize(shape);}
    ~PixelatedConvFunc() {};
    
    virtual void setSize(IPosition& size)
    {
      nDim=size.nelements(); 
      shape=size;
      cache.resize(shape);
    };

    virtual T getValue(Vector<T>& coord, Vector<T>& offset) { return (T)(1.0);};
    virtual int getVisParams(const VisBuffer& vb,const CoordinateSystem& skyCoord=CoordinateSystem()) {return 0;};
    virtual void makeConvFunction(const ImageInterface<Complex>& image,
				  const VisBuffer& vb,
				  const Int wConvSize,
				  const Float pa,
				  const Float dpa,
				  CFStore& cfs,
				  CFStore& cfwts,Bool fillCF=True) {};
    virtual void setPolMap(const Vector<Int>& polMap) {(void)polMap;};
    virtual void setFeedStokes(const Vector<Int>& feedStokes){(void)feedStokes;};
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
    virtual Bool makeAverageResponse(const VisBuffer& vb, 
				     const ImageInterface<Complex>& image,
				     ImageInterface<Complex>& theavgPB,
				     Bool reset=True)
    {throw(AipsError("PixelatedConvFunc::makeAverageRes(Complex) called"));};
    virtual void prepareConvFunction(const VisBuffer& vb, CFStore& cfs) {};

  private:
    Int nDim;
    IPosition shape;
    Array<T> cache;
  };

};

#endif

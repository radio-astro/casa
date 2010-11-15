//# ConvolutionFunction.h: Definition for ConvolutionFunction
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

#ifndef SYNTHESIS_CONVOLUTIONFUNCTION_H
#define SYNTHESIS_CONVOLUTIONFUNCTION_H

#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/CFStore.h>
#include <casa/Arrays/Vector.h>
#define CF_TYPE Double

namespace casa{

  class ConvolutionFunction
  {
  public:
    ConvolutionFunction() {};
    ConvolutionFunction(Int dim) {nDim=dim;};
    virtual ~ConvolutionFunction();
    
    virtual void setDimension(Int n){nDim = n;};
    virtual CF_TYPE getValue(Vector<CF_TYPE>& coord, Vector<CF_TYPE>& offset) {return 0.0;};

    //    virtual ConvolutionFunction& operator=(const ConvolutionFunction& other) = 0;
    virtual int getVisParams(const VisBuffer& vb)=0;
    virtual void makeConvFunction(const ImageInterface<Complex>& image,
				  const VisBuffer& vb,
				  const Int wConvSize,
				  const Float pa,
				  CFStore& cfs,
				  CFStore& cfwts) = 0;// {};
    virtual void setPolMap(const Vector<Int>& polMap) = 0;// {};
    virtual void setFeedStokes(const Vector<Int>& feedStokes) = 0;// {};
    virtual void setParams(const Vector<Int>& polMap, const Vector<Int>& feedStokes)
    {setPolMap(polMap); setFeedStokes(feedStokes);};
  private:
    Int nDim;
  };

};

#endif

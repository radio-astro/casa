// -*- C++ -*-
//# AWConvFunc.h: Definition of the AWConvFunc class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
//
#ifndef SYNTHESIS_AWCONVFUNC_H
#define SYNTHESIS_AWCONVFUNC_H

#include <images/Images/ImageInterface.h>
// #include <synthesis/MeasurementComponents/Utils.h>
// #include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/CFStore.h>
//#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
//#include <synthesis/MeasurementComponents/IlluminationConvFunc.h>
//#include <synthesis/MeasurementComponents/PixelatedConvFunc.h>
#include <synthesis/MeasurementComponents/ConvolutionFunction.h>
#include <synthesis/MeasurementComponents/ATerm.h>
// #include <coordinates/Coordinates/DirectionCoordinate.h>
// #include <coordinates/Coordinates/SpectralCoordinate.h>
// #include <coordinates/Coordinates/StokesCoordinate.h>
// #include <lattices/Lattices/LatticeFFT.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  //
  //-------------------------------------------------------------------------------------------
  //
  class AWConvFunc : public ConvolutionFunction
  //: public PixelatedConvFunc<Complex>
  {
  public:
    AWConvFunc(const CountedPtr<ATerm> ATerm):
      ConvolutionFunction(),ATerm_p(ATerm)
    {};
    ~AWConvFunc() {};
    AWConvFunc& operator=(const AWConvFunc& other);
    virtual void makeConvFunction(const ImageInterface<Complex>& image,
				  const VisBuffer& vb,
				  const Int wConvSize,
				  const Float pa,
				  CFStore& cfs,
				  CFStore& cfwts);
    virtual int getVisParams(const VisBuffer& vb) {return ATerm_p->getVisParams(vb);};
    virtual void setPolMap(const Vector<Int>& polMap) {ATerm_p->setPolMap(polMap);};
    virtual void setFeedStokes(const Vector<Int>& feedStokes) {ATerm_p->setFeedStokes(feedStokes);};

  private:
    CountedPtr<ATerm> ATerm_p;
  };
  //
  //-------------------------------------------------------------------------------------------
  //
};
#endif

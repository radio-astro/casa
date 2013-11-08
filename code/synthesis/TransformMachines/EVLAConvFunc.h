// -*- C++ -*-
//# EVLAConvFunc.h: Definition of the EVLAConvFunc class
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
#ifndef SYNTHESIS_EVLACONVFUNC_H
#define SYNTHESIS_EVLACONVFUNC_H

#include <images/Images/ImageInterface.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/BeamCalc.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/VLACalcIlluminationConvFunc.h>
//#include <synthesis/MeasurementComponents/IlluminationConvFunc.h>
//#include <synthesis/MeasurementComponents/PixelatedConvFunc.h>
#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  class EVLAConvFunc : public ConvolutionFunction
  //: public PixelatedConvFunc<Complex>
  {
  public:
    //    EVLAConvFunc(const CountedPtr<IlluminationConvFunc> ATerm):
    //      ConvolutionFunction(),bandID_p(-1), polMap_p(), feedStokes_p(), ATerm_p(ATerm)
    EVLAConvFunc():     
      ConvolutionFunction(),bandID_p(-1), polMap_p(), feedStokes_p()
    {};
    ~EVLAConvFunc() {};
    EVLAConvFunc& operator=(const EVLAConvFunc& other);
    Int getVLABandID(Double& freq,String&telescopeName);
    Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R);
    void makeConvFunction(const ImageInterface<Complex>& image,
			  const VisBuffer& vb,
			  const Int wConvSize,
			  const Float pa,
			  const Float dpa,
			  CFStore& cfs,
			  CFStore& cfwts);
    int getVisParams(const VisBuffer& vb);
    Int makePBPolnCoords(const VisBuffer&vb,
			 const Vector<Int>& polMap,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord,
			 Vector<Int>& cfStokes);
    //
    // Overloading these functions from ConvolutionFunction class
    //
    void setPolMap(const Vector<Int>& polMap);
    void setFeedStokes(const Vector<Int>& feedStokes);
  private:
    Int bandID_p;
    Float Diameter_p, Nant_p, HPBW, sigma;
    
    LogIO& logIO() {return logIO_p;}
    LogIO logIO_p;
    Vector<Int> polMap_p;
    Vector<Int> feedStokes_p;
    CountedPtr<IlluminationConvFunc> ATerm_p;
  };
};
#endif

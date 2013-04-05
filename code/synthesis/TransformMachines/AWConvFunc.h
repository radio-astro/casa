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

#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/CFStore2.h>
#include <synthesis/TransformMachines/CFBuffer.h>
#include <synthesis/TransformMachines/PSTerm.h>
#include <synthesis/TransformMachines/WTerm.h>
#include <synthesis/TransformMachines/ATerm.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>
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
  {
  public:
    AWConvFunc(const CountedPtr<ATerm> ATerm,
	       const CountedPtr<PSTerm> psTerm,
	       const CountedPtr<WTerm> wTerm,
	       const Bool wbAWP=False):
      ConvolutionFunction(),aTerm_p(ATerm),psTerm_p(psTerm), wTerm_p(wTerm), pixFieldGrad_p(), 
      wbAWP_p(wbAWP)
    {pixFieldGrad_p.resize(2);pixFieldGrad_p=0.0;}

    ~AWConvFunc() {};
    AWConvFunc& operator=(const AWConvFunc& other);
    virtual void makeConvFunction(const ImageInterface<Complex>& image,
				  const VisBuffer& vb,
				  const Int wConvSize,
				  const CountedPtr<PolOuterProduct>& pop,
				  const Float pa,
				  const Float dpa,
				  const Vector<Double>& uvScale, const Vector<Double>& uvOffset,
				  const Matrix<Double>& vbFreqSelection,
				  CFStore2& cfs,
				  CFStore2& cfwts);
  void fillConvFuncBuffer(CFBuffer& cfb, CFBuffer& cfWtb,
			  const Int& nx, const Int& ny,
			  const Vector<Double>& freqValues,
			  const Vector<Double>& wValues,
			  const Double& wScale,
			  const PolMapType& muellerElements,
			  const PolMapType& muellerElementsIndex,
			  const VisBuffer& vb, const Float& psScale,
			  PSTerm& psTerm, WTerm& wTerm, ATerm& aTerm);

    virtual Bool makeAverageResponse(const VisBuffer& vb, 
				     const ImageInterface<Complex>& image,
				     ImageInterface<Float>& theavgPB,
				     Bool reset=True);
    virtual Bool makeAverageResponse(const VisBuffer& vb, 
				     const ImageInterface<Complex>& image,
				     ImageInterface<Complex>& theavgPB,
				     Bool reset=True);
    virtual int getVisParams(const VisBuffer& vb,const CoordinateSystem& skyCoord=CoordinateSystem())
    {return aTerm_p->getVisParams(vb,skyCoord);};
    virtual void setPolMap(const Vector<Int>& polMap) {aTerm_p->setPolMap(polMap);};
    //    virtual void setFeedStokes(const Vector<Int>& feedStokes) {aTerm_p->setFeedStokes(feedStokes);};
    virtual Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R);
    virtual Vector<Double> findPointingOffset(const ImageInterface<Complex>& /*image*/,
					      const VisBuffer& /*vb*/) {Vector<Double> tt(2); tt=0;return tt;};
    virtual Bool setUpCFSupport(Array<Complex>& func, 
				Int& xSupport, Int& ySupport,
				const Float& sampling,
				const Complex& peak);
    virtual Bool resizeCF(Array<Complex>& func, 
			  Int& xSupport, Int& ySupport,
			  const Float& sampling,
			  const Complex& peak);
    virtual void prepareConvFunction(const VisBuffer& vb, VBRow2CFBMapType& cfs);
    Int mapAntIDToAntType(const Int& ant) {return aTerm_p->mapAntIDToAntType(ant);};

    virtual Vector<Double> makeFreqValList(Double& freqScale,
					   const VisBuffer& vb, 
					   const ImageInterface<Complex>& uvGrid);
    virtual Vector<Double> makeWValList(const Double &dW, const Int &nW);

    virtual void setMiscInfo(const RecordInterface& params);
    virtual Matrix<Double> getFreqRangePerSpw(const VisBuffer& vb);

  protected:
    void normalizeAvgPB(ImageInterface<Complex>& inImage,
			ImageInterface<Float>& outImage);
    Bool makeAverageResponse_org(const VisBuffer& vb, 
				 const ImageInterface<Complex>& image,
				 ImageInterface<Float>& theavgPB,
				 Bool reset=True);
    void makePBSq(ImageInterface<Complex>& inImage);
    void makeConjPolAxis(CoordinateSystem& cs);

    Complex cfArea(Matrix<Complex>& cf, const Int& xSupport, const Int& ySupport, const Float& sampling);

    CountedPtr<ATerm> aTerm_p;
    CountedPtr<PSTerm> psTerm_p;
    CountedPtr<WTerm> wTerm_p;
    Vector<Double> thePix_p, pixFieldGrad_p;
    Double imRefFreq_p;
    Bool wbAWP_p;
  };
  //
  //-------------------------------------------------------------------------------------------
  //
};
#endif

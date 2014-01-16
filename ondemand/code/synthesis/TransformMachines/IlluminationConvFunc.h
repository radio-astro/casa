//# IlluminationConvFunc.h: Definition for IlluminationConvFunc
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

#ifndef SYNTHESIS_ILLUMINATIONCONVFUNC_H
#define SYNTHESIS_ILLUMINATIONCONVFUNC_H

#include <casa/Exceptions.h>
#include <synthesis/MeasurementComponents/TabulatedConvFunc.h>
#include <synthesis/MeasurementComponents/ExpCache.h>
#include <synthesis/MeasurementComponents/CExp.new3.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>

namespace casa{

  class IlluminationConvFunc: public TabulatedConvFunc
  {
  public:
    IlluminationConvFunc(): TabulatedConvFunc() 
    {expTableReady=cexpTableReady=sigmaReady=False;pa_p=1000.0;};
    IlluminationConvFunc(Int n): TabulatedConvFunc(n)
    {expTableReady=cexpTableReady=sigmaReady=False;pa_p=1000.0;};
    ~IlluminationConvFunc() {};

    IlluminationConvFunc& operator=(const IlluminationConvFunc& /*other*/)
    {
      return *this;
    }
    IlluminationConvFunc& operator=(const ConvolutionFunction& /*other*/)
    {
      return *this;
    }
    void initExpTable(Int n, CF_TYPE step)  {ExpTable.build(n,step);expTableReady=True;};
    void initCExpTable(Int n)               {CExpTable.build(n);cexpTableReady=True;};
    void setSigma(Double s)                 {sigma = s;sigmaReady=True;}

    Bool isReady() {return (expTableReady & cexpTableReady & sigmaReady);}

    CF_TYPE area(Vector<Int>& convSupport, Vector<Double>& uvScale);
    Vector<Int> supportSize(Vector<Double>& uvScale);

    CF_TYPE getValue(Vector<CF_TYPE>& coord, Vector<CF_TYPE>& offset) 
    {
      (void)coord;(void)offset;
      throw(AipsError("This version of IlluminationConvFunc::getValue() is not useful."));
      return 0;
    };
    virtual CF_TYPE getValue(Double *coord,
			     Double *raoff1,  Double *raoff2, 
			     Double *decoff1, Double *decoff2,
			     Double *area,
			     Int *doGrad,
			     Complex& weight,
			     Complex& dweight1,
			     Complex& dweight2,
			     Double& currentCFPA
			     //		  ,Double lsigma
			     );
    int getVisParams(const casa::VisBuffer& vb, const CoordinateSystem& skyCoord=CoordinateSystem()) 
    {(void)vb;(void)skyCoord;return 0;};
    void makeConvFunction(const casa::ImageInterface<std::complex<float> >&, 
			  const casa::VisBuffer&, casa::Int, 
			  const CountedPtr<PolOuterProduct>& ,//pop,
			  casa::Float, casa::Float,
			  const Vector<Double>& /*uvScale*/, const Vector<Double>& /*uvOffset*/,
			  const Matrix<Double>& /*freqSelection*/,
			  casa::CFStore2&, casa::CFStore2&) {};
    void setPolMap(const casa::Vector<int>&polMap) {(void)polMap;};
    void setFeedStokes(const casa::Vector<int>&feedStokes) {(void)feedStokes;};
    void setParams(const casa::Vector<int>& polMap, const casa::Vector<int>& feedStokes)
    {(void)polMap;(void)feedStokes;};

    Bool findSupport(Array<Complex>&, Float& ,Int&, Int&) 
    {throw(AipsError("IlluminationConvFunc::findSupport() not implemented"));};
    virtual Vector<Double> findPointingOffset(const ImageInterface<Complex>& /*image*/,
					      const VisBuffer& /*vb*/)
    {throw(AipsError("IlluminationConvFunc::findPointingOffset() not implemented"));};
    virtual void prepareConvFunction(const VisBuffer& /*vb*/, VBRow2CFBMapType& /*theMap*/)
    {throw(AipsError("IlluminationConvFunc::prepareConvFunction() not implemented"));};

    virtual Bool makeAverageResponse(const VisBuffer&,
				     const ImageInterface<Complex>&,
				     ImageInterface<Float>&,
				     Bool)
      {throw(AipsError("IlluminationConvFunc::makeAverageRes() called"));};

    virtual Bool makeAverageResponse(const VisBuffer&,
				     const ImageInterface<Complex>&,
				     ImageInterface<Complex>&,
				     Bool)
      {throw(AipsError("IlluminationConvFunc::makeAverageRes() called"));};

		  
  private:
    ExpCache<CF_TYPE> ExpTable;
    CExp3<CF_TYPE> CExpTable;
    Double sigma, pa_p;
    Bool expTableReady,cexpTableReady,sigmaReady;
  };

};
#endif

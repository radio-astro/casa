// -*- C++ -*-
//# AWConvFunc2.h: Definition of the AWConvFunc2 class
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
#ifndef SYNTHESIS_AWCONVFUNC2_H
#define SYNTHESIS_AWCONVFUNC2_H

#include <synthesis/TransformMachines/ConvolutionFunction.h>
#include <synthesis/TransformMachines/PolOuterProduct.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/TransformMachines/AWConvFunc.h>
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
  //class AWConvFunc2 : public ConvolutionFunction
  class AWConvFunc2 : public AWConvFunc
  {
  public:
    AWConvFunc2(const CountedPtr<ATerm> ATerm,
	       const CountedPtr<PSTerm> psTerm,
	       const CountedPtr<WTerm> wTerm,
	       const Bool wbAWP=False):
      AWConvFunc(ATerm, psTerm, wTerm, wbAWP)
    {}

    ~AWConvFunc2() {};
    virtual void fillConvFuncBuffer(CFBuffer& cfb, CFBuffer& cfWtb,
			  const Int& nx, const Int& ny,
			  const Vector<Double>& freqValues,
			  const Vector<Double>& wValues,
			  const Double& wScale,
			  const PolMapType& muellerElements,
			  const PolMapType& muellerElementsIndex,
			  const VisBuffer& vb, const Float& psScale,
			  PSTerm& psTerm, WTerm& wTerm, ATerm& aTerm);
  };
  //
  //-------------------------------------------------------------------------------------------
  //
};
#endif

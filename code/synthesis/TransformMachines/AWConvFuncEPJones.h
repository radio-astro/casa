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
#ifndef SYNTHESIS_AWCONVFUNCEPJONES_H
#define SYNTHESIS_AWCONVFUNCEPJONES_H

#include <synthesis/TransformMachines/AWConvFunc.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/TransformMachines/PSTerm.h>
#include <synthesis/TransformMachines/WTerm.h>
#include <synthesis/TransformMachines/ATerm.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  //
  //-------------------------------------------------------------------------------------------
  //
  class AWConvFuncEPJones : public AWConvFunc
  {
  public:
    AWConvFuncEPJones(const CountedPtr<ATerm> ATerm,
		      const CountedPtr<PSTerm> psTerm,
		      const CountedPtr<WTerm> wTerm,
		      const Bool wbAWP=False):
      AWConvFunc(ATerm,psTerm,wTerm,wbAWP), imageDC_p(),imageObsInfo_p()
    {};
    ~AWConvFuncEPJones() {};
    AWConvFuncEPJones& operator=(const AWConvFuncEPJones& other);

    // MosaicFT related
    virtual void makeConvFunction(const ImageInterface<Complex>& image,
				  const VisBuffer& vb,
				  const Int wConvSize,
				  const CountedPtr<PolOuterProduct>& pop,
				  const Float pa,
				  const Float dpa,
				  const Vector<Double>& uvScale, const Vector<Double>& uvOffset,
				  const Matrix<Double>& vbFreqSelection,
				  CFStore2& cfs,
				  CFStore2& cfwts,
				  Bool fillCF=True);
    virtual Vector<Double> findPointingOffset(const ImageInterface<Complex>& image,
					      const VisBuffer& vb);

    void toPix(const VisBuffer& vb);
    void storeImageParams(const ImageInterface<Complex>& iimage,
			  const VisBuffer& vb);
  private:
    // MosaicFT related
    DirectionCoordinate imageDC_p;
    ObsInfo imageObsInfo_p;
    Int nx_p; 
    Int ny_p;
    Int nchan_p;
    Int npol_p;
    CoordinateSystem csys_p;
    DirectionCoordinate dc_p;
    MDirection::Convert pointToPix_p;
    MeasFrame pointFrame_p;
    MEpoch::Types timeMType_p;
    Unit timeUnit_p;
    Int directionIndex_p;
    MDirection direction1_p;
    MDirection direction2_p;
  };
  //
  //-------------------------------------------------------------------------------------------
  //
};
#endif

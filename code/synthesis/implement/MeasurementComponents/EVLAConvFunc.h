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
//
//TEMPS
#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/CFStore.h>
#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogOrigin.h>
//TEMPS
//
// Temp. function for staged re-factoring
//    
namespace casa { //# NAMESPACE CASA - BEGIN
  class EVLAConvFunc
  {
  public:
    EVLAConvFunc():bandID_p(-1){};
    Int getVLABandID(Double& freq,String&telescopeName);
    Bool findSupport(Array<Complex>& func, Float& threshold,Int& origin, Int& R);
    void makeConvFunction(const ImageInterface<Complex>& image,
			  const Int wConvSize,
			  const VisBuffer& vb,Float pa,
			  const Vector<Int>& polMap,
			  Vector<Int>& cfStokes,
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
    Int bandID_p;
    Float Diameter_p, Nant_p, HPBW, sigma;
    
    LogIO& logIO() {return logIO_p;}
    LogIO logIO_p;

  };
};
#endif

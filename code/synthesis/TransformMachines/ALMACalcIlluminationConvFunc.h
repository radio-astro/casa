//# ALMAIlluminationConvFunc.h: Definition for ALMAIlluminationConvFunc
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//# Copyright by ESO (in the framework of the ALMA collaboration)
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

#ifndef SYNTHESIS_ALMACALCILLUMINATIONCONVFUNC_H
#define SYNTHESIS_ALMACALCILLUMINATIONCONVFUNC_H

#include <synthesis/TransformMachines/IlluminationConvFunc.h>
#include <synthesis/TransformMachines/BeamCalc.h>
#include <synthesis/TransformMachines/ALMAAperture.h>
#include <synthesis/MeasurementComponents/CExp.new3.h>
#include <synthesis/MeasurementComponents/ExpCache.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/sstream.h>
#include <synthesis/TransformMachines/Utils.h>
namespace casa{

  class ALMACalcIlluminationConvFunc: public IlluminationConvFunc
  {
  public:

    ALMACalcIlluminationConvFunc();
    ALMACalcIlluminationConvFunc(Int n):IlluminationConvFunc(n)
      {pbRead_p=False;};
    ~ALMACalcIlluminationConvFunc() 
      {delete ap.aperture;};

    void setBandID(Int /*bandID*/) {/*ap.band=(ALMABeamCalcBandCode)bandID;*/}

    void setAntRayPath(const String& antRayPath) {otherAntRayPath_p = antRayPath;}

    void loadFromImage(String &fileName);
    void getIdealConvFunc(Array<Complex>& buf);
    void ftAperture(TempImage<Complex>& uvgrid);
    void ftAperture() {ftAperture(convFunc_p); pbRead_p=True;};
    void storePB(String& fileName);

    Bool pbReady() {return pbRead_p;}

    CoordinateSystem makeUVCoords(CoordinateSystem& imageCoordSys,
				  IPosition& shape, Double refFreq=-1.0);
    void regridAperture(CoordinateSystem& skyCS, 
			IPosition& skyShape, 
			TempImage<Complex>& uvGrid, 
			const VisBuffer& vb,
			Bool doSquint=True,Int bandID=-1);
    void regridAperture(CoordinateSystem& skyCS,
			IPosition& skyShape,
			TempImage<Complex>& uvGrid,
			const VisBuffer &vb,
			const Vector<Float>& paList,
			Bool doSquint, Int bandID);

    void regridAperture(CoordinateSystem& skyCS, 
			IPosition& skyShape, 
			TempImage<Complex>& uvGrid, 
			const String& telescope,
			const MVFrequency& freqQ,
			Float pa = 0.,
			Bool doSquint=True,
			Int bandID=-1);

    void applyPB(ImageInterface<Float>& pbImage, 
		 const VisBuffer& vb, Bool doSquint=False, Int cfKey=0);

    void applyPB(ImageInterface<Complex>& pbImage, 
		 const VisBuffer& vb, Bool doSquint=True, Int cfKey=0);

    void applyPB(ImageInterface<Float>& pbImage, 
		 const String& telescope, const MEpoch& obsTime, 
		 const String& antType0, const String& antType1,
		 const MVFrequency& freqQ, Double pa=0.,
		 Bool doSquint=False);

    void applyPB(ImageInterface<Complex>& pbImage, 
		 const String& telescope, const MEpoch& obsTime,
		 const String& antType0, const String& antType1,
		 const MVFrequency& freqQ, Double pa=0.,
		 Bool doSquint=True);

    void applyVP(ImageInterface<Complex>& pbImage, 
		 const String& telescope, const MEpoch& obsTime,
		 const String& antType0, const String& antType1,
		 const MVFrequency& freqQ, Double pa=0.,
		 Bool doSquint=True);

    void skyMuller(ImageInterface<Complex>& skyJones);

    //    Int getALMABandId(const Double& freq);
    virtual void prepareConvFunction(const VisBuffer& /*vb*/, CFStore& /*cfs*/){};

    

  private:
    
    void fillPB(ImageInterface<Complex>& inImg, ImageInterface<Float>& outImg, Bool Square=False);
    void fillPB(ImageInterface<Complex>& inImg, ImageInterface<Complex>& outImg, Bool Square=False);
    void fillVP(ImageInterface<Complex>& inImg, ImageInterface<Complex>& outImg, Bool Square=False);

    TempImage<Complex> convFunc_p;
    Vector<Double> resolution;
    Bool pbRead_p;
    Float freq_p,lastPA;
    ApertureCalcParams ap;

    Bool haveCannedResponses;

    String otherAntRayPath_p;
  };

};
#endif

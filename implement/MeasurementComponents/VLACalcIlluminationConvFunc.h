//# VLAIlluminationConvFunc.h: Definition for VLAIlluminationConvFunc
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

#ifndef SYNTHESIS_VLACALCILLUMINATIONCONVFUNC_H
#define SYNTHESIS_VLACALCILLUMINATIONCONVFUNC_H

#include <synthesis/MeasurementComponents/IlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/BeamCalcConstants.h>
#include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/BeamCalcAntenna.h>
#include <synthesis/MeasurementComponents/CExp.new3.h>
//#include <synthesis/MeasurementComponents/Exp.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/sstream.h>
namespace casa{

  Int getVLABandID(Double& freq,String&telescopeName);
  class VLACalcIlluminationConvFunc: public IlluminationConvFunc
  {
  public:

    VLACalcIlluminationConvFunc();
    VLACalcIlluminationConvFunc(Int n):IlluminationConvFunc(n)    {pbRead_p=False;};
    ~VLACalcIlluminationConvFunc() {delete ap.aperture;};

    void setBandID(Int bandID) {ap.band=(BeamCalcBandCode)bandID;}
    void storeImg(String &fileName,ImageInterface<Complex>& img);
    void storeImg(String &fileName,ImageInterface<Float>& img);
    void store(String &fileName);
    void loadFromImage(String &fileName);
    void getIdealConvFunc(Array<Complex>& buf);
    void ftAperture(TempImage<Complex>& uvgrid);
    void ftAperture() {ftAperture(convFunc_p); pbRead_p=True;};
    void storePB(String& fileName);

    Bool pbReady() {return pbRead_p;}

    CoordinateSystem makeUVCoords(CoordinateSystem& imageCoordSys,
				  IPosition& shape);
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
    void applyPB(ImageInterface<Float>& pbImage, const VisBuffer& vb, 
		 const Vector<Float>& paList, Int bandID);
    void applyPB(ImageInterface<Float>& pbImage, const VisBuffer& vb, Int bandID=-1);
    void applyPB(ImageInterface<Complex>& pbImage, const VisBuffer& vb, Int bandID=-1);
    void skyMuller(ImageInterface<Complex>& skyJones);


  private:
    
    void fillPB(ImageInterface<Complex>& inImg, ImageInterface<Float>& outImg);
    void fillPB(ImageInterface<Complex>& inImg, ImageInterface<Complex>& outImg);

    TempImage<Complex> convFunc_p;
    //    TempImage<Float> reAperture_p, imAperture_p;
    Vector<Double> resolution;
    Bool pbRead_p;
    Float freq_p,lastPA;
    struct ApertureCalcParams ap;
//  =
//       {
// 	1,
// 	{0, 0, 0, 0},
// 	-13.0, -13.0, 
// 	0.5, 0.5,
// 	52, 52,
// 	15.0*M_PI/180.0,
// 	1.4,
// 	BeamCalc_VLA_L
//       };

  };

};
#endif

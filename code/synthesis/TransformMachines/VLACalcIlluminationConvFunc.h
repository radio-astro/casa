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

#include <synthesis/TransformMachines/IlluminationConvFunc.h>
//#include <synthesis/MeasurementComponents/BeamCalcConstants.h>
#include <synthesis/TransformMachines/BeamCalc.h>
//#include <synthesis/MeasurementComponents/BeamCalcAntenna.h>
#include <synthesis/MeasurementComponents/CExp.new3.h>
#include <synthesis/MeasurementComponents/ExpCache.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/sstream.h>
#include <synthesis/TransformMachines/Utils.h>
namespace casa{

  //  Int getVLABandID(Double& freq,String&telescopeName);
  class VLACalcIlluminationConvFunc: public IlluminationConvFunc
  {
  public:

    VLACalcIlluminationConvFunc();
    VLACalcIlluminationConvFunc(Int n):IlluminationConvFunc(n)    {pbRead_p=False;};
    ~VLACalcIlluminationConvFunc() {delete ap.aperture;};

    void setBandID(Int bandID) {ap.band=bandID;}
    // void storeImg(String &fileName,ImageInterface<Complex>& img);
    // void storeImg(String &fileName,ImageInterface<Float>& img);
    // void store(String &fileName);
    void loadFromImage(String &fileName);
    void getIdealConvFunc(Array<Complex>& buf);
    void ftAperture(TempImage<Complex>& uvgrid, Bool makeMueller=True);
    void ftAperture() {ftAperture(convFunc_p); pbRead_p=True;};
    void storePB(String& fileName);

    Bool pbReady() {return pbRead_p;}

    CoordinateSystem makeUVCoords(CoordinateSystem& imageCoordSys,
				  IPosition& shape, Double refFreq=-1.0);
    void regridAperture(CoordinateSystem& skyCS, 
			IPosition& skyShape, 
			TempImage<Complex>& uvGrid, 
			const VisBuffer& vb,
			Bool doSquint=True,Int bandID=-1, Double freqVal=-1.0);
    void regridAperture(CoordinateSystem& skyCS,
			IPosition& skyShape,
			TempImage<Complex>& uvGrid,
			const VisBuffer &vb,
			const Vector<Float>& paList,
			Bool doSquint, Int bandID);
    void regridApertureEngine(ApertureCalcParams& ap,
			      const Int& inStokes);
    void setApertureParams(ApertureCalcParams& ap,
			   const Float& Freq, const Float& pa, 
			   const Int& bandID,
			   const Int& inStokes,
			   const IPosition& skyShape,
			   const Vector<Double>& uvIncr);


    void applyPB(ImageInterface<Float>& pbImage, const VisBuffer& vb, 
		 const Vector<Float>& paList, Int bandID, Bool doSquint=False);
    void applyPB(ImageInterface<Float>& pbImage, const VisBuffer& vb, Int bandID=-1, 
		 Bool doSquint=False, Double freqVal=-1.0);
    void applyPB(ImageInterface<Complex>& pbImage, const VisBuffer& vb, Int bandID=-1, 
		 Bool doSquint=True, Double freqVal=-1.0);
    void applyPBSq(ImageInterface<Float>& pbImage, const VisBuffer& vb, 
		   const Vector<Float>& paList, Int bandID, Bool doSquint=False);
    void applyPBSq(ImageInterface<Float>& pbImage, const VisBuffer& vb, Int bandID=-1, 
		   Bool doSquint=False);
    void applyPBSq(ImageInterface<Complex>& pbImage, const VisBuffer& vb, Int bandID=-1, 
		   Bool doSquint=True);
    void makeFullJones(ImageInterface<Complex>& pbImage, const VisBuffer& vb, Bool doSquint, 
		       Int bandID, Double freqVal=-1.0);
    void skyMuller(ImageInterface<Complex>& skyJones);
    void skyMuller(Array<Complex>& skyJones, const IPosition& shape, const Int& inStokes);

    Bool findSupport(Array<Complex>& /*func*/, Float& /*threshold*/,Int& /*origin*/, Int& /*R*/)
    {throw(AipsError("VLACalcIlluminationConvFunc::findSupport() not implemented"));};
    virtual Bool makeAverageResponse(const VisBuffer& /*vb*/,
				     const ImageInterface<Complex>& /*image*/,
				     //				     TempImage<Float>& theavgPB,
				     ImageInterface<Float>& /*theavgPB*/,
				     Bool /*reset*/=True)
    {throw(AipsError("VLACalc::makeAverageRes() called"));};


  private:
    
    void fillPB(ImageInterface<Complex>& inImg, ImageInterface<Float>& outImg, Bool Square=False);
    void fillPB(ImageInterface<Complex>& inImg, ImageInterface<Complex>& outImg, Bool Square=False);

    TempImage<Complex> convFunc_p;
    //    TempImage<Float> reAperture_p, imAperture_p;
    Vector<Double> resolution;
    Bool pbRead_p;
    Float freq_p,lastPA;
    ApertureCalcParams ap;
  };

};
#endif

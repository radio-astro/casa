/*
 * tSDMaskHandler.cc
 *tSDMaskHandler: test of SDMaskHandler 
//# Copyright (C) 2013
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
 *  Created on: 2014.08.23 
 *      Author: ttsutsum
 */





#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <casa/Utilities/Assert.h>

#include <measures/Measures/MRadialVelocity.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/Logging/LogIO.h>
//#include <synthesis/ImagerObjects/SynthesisImager.h>
//#include <synthesis/ImagerObjects/SIImageStore.h>
//#include <synthesis/Utilities/SpectralImageUtil.h>
#include <lattices/Lattices/LatticeConcat.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/SubImage.h>
#include <casa/namespace.h>
#include <images/Images/TempImage.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/ImageRegion.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <ms/MSSel/MSSourceIndex.h>
#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>
#include <synthesis/ImagerObjects/SDMaskHandler.h>

// test makeMask
void testMakeMask()
{

    cout <<" Test makeMask()"<<endl;
    String maskname("mymask");
    Double thresval = 2.0;
    Quantity threshold(thresval, "Jy");
    IPosition shape(4, 100, 100, 1, 5);
    CoordinateSystem csys=CoordinateUtil::defaultCoords4D();
    //TempImage<Float> templateImage(TiledShape(shape),csys);
    PagedImage<Float> templateImage(TiledShape(shape),csys, String("mytempim"));
    templateImage.setUnits(Unit("Jy/pixel"));
    templateImage.set(0.0);
    for (uInt i=0; i < 5; i++) {
      for (uInt j=0; j < 3; j++ ) {
        //Int selpx = 45 + j;
        IPosition loc(4, 50, 45+j, 0, Int(i));
        IPosition loc2(4, 51, 45+j, 0, Int(i));
        IPosition loc3(4, 52, 45+j, 0, Int(i));
        Float val = 2.0 + Float(i);
        templateImage.putAt(val, loc);
        templateImage.putAt(val, loc2);
        templateImage.putAt(val, loc3);
      }
    } 
    //CountedPtr<ImageInterface<Float> > outmaskimage;
    SHARED_PTR<ImageInterface<Float> > outmaskimage;
    SDMaskHandler maskhandler;
    outmaskimage = maskhandler.makeMask(maskname, threshold, templateImage);
    cerr<<"maskimage.shape()="<< outmaskimage->shape()<<endl;
    for (uInt i=0; i < 5; i++) {
      for (uInt j=0; j < 3; j++) {
        IPosition loc(4, 50, 45+j, 0, Int(i));
        IPosition loc2(4, 51, 45+j, 0, Int(i));
        IPosition loc3(4, 52, 45+j, 0, Int(i));
        // mask image pix values are 1 or 0
        //chan = 0 plane should be zero by threshold 
        if (i==0) {
          AlwaysAssert(outmaskimage->getAt(loc)==Float(0.0),AipsError);
          AlwaysAssert(outmaskimage->getAt(loc2)==Float(0.0),AipsError);
          AlwaysAssert(outmaskimage->getAt(loc3)==Float(0.0),AipsError);
        }
        else {
          AlwaysAssert(outmaskimage->getAt(loc)==Float(1.0),AipsError);
          AlwaysAssert(outmaskimage->getAt(loc2)==Float(1.0),AipsError);
          AlwaysAssert(outmaskimage->getAt(loc3)==Float(1.0),AipsError);
       }
     }
   }
}

//test regionToMaskImage
void testRegionToMaskImage()
{
  cout <<" Test regionToImageMask()"<<endl;
  //static Bool regionToImageMask(const String& maskimage, Record* regionRec, Matrix<Quantity> & blctrcs,
  //                  Matrix<Float>& circles, const Float& value=1.0);
  String maskname("mymaskfromregions");
  IPosition shape(4, 100, 100, 1, 10);
  CoordinateSystem csys=CoordinateUtil::defaultCoords4D();
  PagedImage<Float> maskImage(TiledShape(shape),csys, maskname);
  //SDMaskHandler::cloneImShape(templateImage, maskname);
  // region record
  Vector<Quantum<Double> > qblc(4);
  Vector<Quantum<Double> > qtrc(4);
  // blctrcs box 
  Vector<Quantum<Double> > qblcbox(2);
  Vector<Quantum<Double> > qtrcbox(2);
  Vector<Double> pBlc(4);
  Vector<Double> pTrc(4);
  Vector<Double> wBlc(4);
  Vector<Double> wTrc(4);
  Vector<Double> pBlcBox(4);
  Vector<Double> pTrcBox(4);
  Vector<Double> wBlcBox(4);
  Vector<Double> wTrcBox(4);
  pBlc(0)=25.0; pBlc(1)=65.0; pBlc(2)=0; pBlc(3)=2.0;
  pTrc(0)=42.0; pTrc(1)=75.0; pTrc(2)=0; pTrc(3)=9.0;
  pBlcBox(0)=50.0; pBlcBox(1)=10.0; pBlcBox(2)=0; pBlcBox(3)=0;
  pTrcBox(0)=65.0; pTrcBox(1)=40.0; pBlcBox(2)=0; pBlcBox(3)=0;
  csys.toWorld(wBlc,pBlc);
  csys.toWorld(wTrc,pTrc);
  csys.toWorld(wBlcBox,pBlcBox);
  csys.toWorld(wTrcBox,pTrcBox);
  for (Int i = 0; i< 4; i++) {
    Int iaxis = csys.pixelAxisToWorldAxis(i);
    qblc(i) = Quantum<Double>(wBlc(i), csys.worldAxisUnits()(iaxis));
    qtrc(i) = Quantum<Double>(wTrc(i), csys.worldAxisUnits()(iaxis));
    if (i<2) {
      qblcbox(i) = Quantum<Double>(wBlcBox(i), csys.worldAxisUnits()(iaxis));
      qtrcbox(i) = Quantum<Double>(wTrcBox(i), csys.worldAxisUnits()(iaxis));
    }
  }
  Vector<Int> absRel; // null = abosolute
  WCBox wbox(qblc,qtrc,csys,absRel);   
  LCBox box(pBlc,pTrc,shape);
  Record rec=wbox.toRecord("");
  Record *regionRec=0;
  regionRec = new Record();
  regionRec->assign(rec);
  // a box by blc and trc
  Matrix<Quantity> blctrcs(1,4);
  blctrcs(0,0)=qblcbox(0);
  blctrcs(0,1)=qblcbox(1);
  blctrcs(0,2)=qtrcbox(0);
  blctrcs(0,3)=qtrcbox(1);
  // define two circles with r=5 and r=12
  Matrix<Float> circles(2,3);
  circles(0,0) = 5;
  circles(0,1) = 10;
  circles(0,2) = 8;
  circles(1,0) = 12;
  circles(1,1) = 70;
  circles(1,2) = 65;
  //SDMaskHandler::regionToImageMask(maskname, regionRec, blctrcs, circles);
  SDMaskHandler::regionToImageMask(maskImage, regionRec, blctrcs, circles);
  delete regionRec;
}

void testRegionText()
{
  cout <<" Test regionTextToImageRegion()"<<endl;
  String maskname("mymaskfromregions2");
  IPosition shape(4, 100, 100, 1, 10);
  CoordinateSystem csys=CoordinateUtil::defaultCoords4D();
  //TempImage<Float> templateImage(TiledShape(shape),csys);
  PagedImage<Float> regionImage(TiledShape(shape),csys, maskname);
  //String crtfFile="/home/casa-dev-08/ttsutsum/testcrtf.txt"; 
  String crtfFile="box [[45pix,50pix],[85pix,65pix]]"; 
  ImageRegion* imageRegion=0;
  SDMaskHandler::regionTextToImageRegion(crtfFile, regionImage, imageRegion);
  if (imageRegion!=0)
    SDMaskHandler::regionToMask(regionImage,*imageRegion, Float(1.0));
  delete imageRegion;
}

int main(int argc, char **argv)
{
  using namespace std;
  using namespace casa;
  try{
      testMakeMask();
      testRegionToMaskImage();
      testRegionText();
  }catch( AipsError e ){
    cout << "Exception ocurred." << endl;
    cout << e.getMesg() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
};

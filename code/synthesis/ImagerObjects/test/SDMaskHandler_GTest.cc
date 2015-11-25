//# SDMaskHandler_GTest.cc: implementation of SDMaskHandler google test
//#
//# Copyright (C) 2015
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
//# 51 Franklin Street, Fifth FloorBoston, MA 02110-1335, USA 
//#

#include <synthesis/ImagerObjects/test/SDMaskHandler_GTest.h>
#include <synthesis/ImagerObjects/SDMaskHandler.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/ImageRegion.h>

using namespace casa;
using namespace std;

namespace test {

SDMaskHandlerTest::SDMaskHandlerTest() {};
SDMaskHandlerTest::~SDMaskHandlerTest() { }; 

void SDMaskHandlerTest::SetUp() {
    outMaskName="";
}

void SDMaskHandlerTest::TearDown() {
     
}

void SDMaskHandlerTest::generateBoxMaskImage(String imagename, Int nchan, IPosition blc, IPosition trc)
{
    //blc and trc --- 4 elements vect.
    IPosition shape(4, 100, 100, 1, nchan);
    csys=CoordinateUtil::defaultCoords4D();
    PagedImage<Float> maskImage(TiledShape(shape), csys, imagename);
    maskImage.setUnits(Unit("Jy/pixel"));
    maskImage.set(0.0);
    // sanity check
    if (blc(0) <= trc(0) && blc(1) <= trc(1) && blc(3) < nchan) {
      Int dx = trc(0) - blc(0);
      Int dy = trc(1) - blc(1);
      for (uInt i=0+blc(3); i < trc(3)+1; i++) {
        for (uInt j=0; j < (uInt)dx; j++) {
          for (uInt k=0; k < (uInt)dy; k++) {
            IPosition loc(4,blc(0)+j, blc(1)+k, 0, Int(i));
            Float val = 1.0;
            maskImage.putAt(val, loc);
          }
        }
      }
    }
}

ImageInterfaceTest::ImageInterfaceTest() {}
ImageInterfaceTest::~ImageInterfaceTest() {}


void ImageInterfaceTest::testMakeMaskByThreshold() 
{
    cout <<" Test makeMask()"<<endl;
    outMaskName="testMakeMask.im"; 
    Double thresval = 2.0;
    Quantity threshold(thresval, "Jy");
    IPosition shape(4, 100, 100, 1, 5);
    csys=CoordinateUtil::defaultCoords4D();
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
    SHARED_PTR<ImageInterface<Float> > outmaskimage;
    SDMaskHandler maskhandler;
    outmaskimage = maskhandler.makeMask(outMaskName, threshold, templateImage);
    cerr<<"maskimage.shape()="<< outmaskimage->shape()<<endl;
    for (uInt i=0; i < 5; i++) {
      for (uInt j=0; j < 3; j++) {
        IPosition loc(4, 50, 45+j, 0, Int(i));
        IPosition loc2(4, 51, 45+j, 0, Int(i));
        IPosition loc3(4, 52, 45+j, 0, Int(i));
        // mask image pix values are 1 or 0
        //chan = 0 plane should be zero by threshold
        if (i==0) {
          ASSERT_TRUE(outmaskimage->getAt(loc)==Float(0.0));
          ASSERT_TRUE(outmaskimage->getAt(loc2)==Float(0.0));
          ASSERT_TRUE(outmaskimage->getAt(loc3)==Float(0.0));
        }
        else {
          ASSERT_TRUE(outmaskimage->getAt(loc)==Float(1.0));
          ASSERT_TRUE(outmaskimage->getAt(loc2)==Float(1.0));
          ASSERT_TRUE(outmaskimage->getAt(loc3)==Float(1.0));
       }
     }
   }
}//testMakeMaskByThreshold

void ImageInterfaceTest::testRegionToMaskImage()
{
  cout <<" Test regionToImageMask()"<<endl;
  //static Bool regionToImageMask(const String& maskimage, Record* regionRec, Matrix<Quantity> & blctrcs,
  //                  Matrix<Float>& circles, const Float& value=1.0);
  outMaskName="testRegionToImageMask.im";
  IPosition shape(4, 100, 100, 1, 10);
  csys=CoordinateUtil::defaultCoords4D();
  PagedImage<Float> maskImage(TiledShape(shape),csys, outMaskName);
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
}//testRegionToMaskImage

void ImageInterfaceTest::testRegionText()
{
  cout <<" Test regionTextToImageRegion()"<<endl;
  outMaskName="testRegionText.im";
  IPosition shape(4, 100, 100, 1, 10);
  csys=CoordinateUtil::defaultCoords4D();
  //TempImage<Float> templateImage(TiledShape(shape),csys);
  PagedImage<Float> regionImage(TiledShape(shape),csys, outMaskName);
  //String crtfFile="/home/casa-dev-08/ttsutsum/testcrtf.txt";
  String crtfFile="box [[45pix,50pix],[85pix,65pix]]";
  ImageRegion* imageRegion=0;
  SDMaskHandler::regionTextToImageRegion(crtfFile, regionImage, imageRegion);
  if (imageRegion!=0)
    SDMaskHandler::regionToMask(regionImage,*imageRegion, Float(1.0));
  delete imageRegion;
}

// Tests
TEST_F(ImageInterfaceTest, testMakeMaskByThreshold) {
   testMakeMaskByThreshold();
}

TEST_F(ImageInterfaceTest, testRegionToMaskImage) {
   testRegionToMaskImage();
}

TEST_F(ImageInterfaceTest, testRegionText) {
  testRegionText();
}

}//test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


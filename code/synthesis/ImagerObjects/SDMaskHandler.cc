//# SDMaskHandler.cc: Implementation of SDMaskHandler classes
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <casa/Arrays/ArrayMath.h>
#include <casa/OS/HostInfo.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/TempImage.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/RegionManager.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCUnion.h>
#include <imageanalysis/ImageAnalysis/CasaImageBeamSet.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <imageanalysis/ImageAnalysis/Image2DConvolver.h>
#include <casa/OS/File.h>
#include <lattices/LEL/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/LRegions/LCEllipsoid.h>
#include <lattices/LRegions/LCUnion.h>
#include <lattices/LRegions/LCExtension.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <tables/Tables/TableLock.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

#include <imageanalysis/Annotations/RegionTextList.h>
#include <synthesis/ImagerObjects/SDMaskHandler.h>


namespace casa { //# NAMESPACE CASA - BEGIN


  SDMaskHandler::SDMaskHandler()
  {
    interactiveMasker_p = new InteractiveMasking();
  }
  
  SDMaskHandler::~SDMaskHandler()
  {
    if (interactiveMasker_p != 0)
      delete interactiveMasker_p;
  }
  
  void SDMaskHandler::resetMask(SHARED_PTR<SIImageStore> imstore)
  {
    imstore->mask()->set(1.0);
    imstore->mask()->unlock();
  }

/***
  void SDMaskHandler::fillMask(SHARED_PTR<SIImageStore> imstore, Vector<String> maskStrings)
  {
      String maskString;
      if (maskStrings.nelements()) {
        for (uInt imsk = 0; imsk < maskStrings.nelements(); imsk++) {
          maskString = maskStrings[imsk];
          fillMask(imstore, maskString);
        }
      }
      else {
        fillMask(imstore, String(""));
      }
  }
***/

  void SDMaskHandler::fillMask(SHARED_PTR<SIImageStore> imstore, Vector<String> maskStrings)
  {
    LogIO os( LogOrigin("SDMaskHandler","fillMask",WHERE) );
    String maskString;
    try {
      TempImage<Float> tempAllMaskImage(imstore->mask()->shape(), imstore->mask()->coordinates());
      if (maskStrings.nelements()) {
        //working temp mask image
        TempImage<Float> tempMaskImage(imstore->mask()->shape(), imstore->mask()->coordinates());
        copyMask(*(imstore->mask()), tempMaskImage);
        for (uInt imsk = 0; imsk < maskStrings.nelements(); imsk++) {
          maskString = maskStrings[imsk];
          if (maskString!="") {
            if ( Table::isReadable(maskString) ) {
              Table imtab = Table(maskString, Table::Old);
              Vector<String> colnames = imtab.tableDesc().columnNames();
              if ( colnames[0]=="map" ) {
                // looks like a CASA image ... probably should check coord exists in the keyword also...
                //          cout << "copy this input mask...."<<endl;
                PagedImage<Float> inmask(maskString);
                IPosition inShape = inmask.shape();
                IPosition outShape = imstore->mask()->shape();
                Int specAxis = CoordinateUtil::findSpectralAxis(inmask.coordinates());
                Int outSpecAxis = CoordinateUtil::findSpectralAxis(imstore->mask()->coordinates());
                if (inShape(specAxis) == 1 && outShape(outSpecAxis)>1) {
                  os << "Expanding mask image: " << maskString << LogIO::POST;
                  expandMask(inmask, tempMaskImage);
                }
                else {
                  os << "Copying mask image: " << maskString << LogIO::POST;
                  copyMask(inmask, tempMaskImage);
               }
              }// end of ''map''
              else {
                throw(AipsError(maskString+" does not appear to be valid image mask"));
              }
            }// end of readable table
            else {
              //
              Record* myrec = 0;
              try {
                myrec = RegionManager::readImageFile(maskString,String("temprgrec"));
                if (myrec!=0) {
                  Bool ret(false);
                  Matrix<Quantity> dummyqmat;
                  Matrix<Float> dummyfmat;
                  ret=SDMaskHandler::regionToImageMask(tempMaskImage, myrec, dummyqmat, dummyfmat);
                  if (!ret) cout<<"regionToImageMask failed..."<<endl;
                    os << "Reading region record mask: " << maskString << LogIO::POST;

                  //debug
                  //PagedImage<Float> testtempim(tempMaskImage.shape(), tempMaskImage.coordinates(), "_testTempim");
                  //ret=SDMaskHandler::regionToImageMask(testtempim, myrec, dummyqmat, dummyfmat);
                  //if (!ret) cout<<"regionToImageMask 2nd failed..."<<endl;
                }
              }
              catch (...) {
                try {
                  ImageRegion* imageRegion=0;
                  os << "Reading text mask: " << maskString << LogIO::POST;
                  SDMaskHandler::regionTextToImageRegion(maskString, tempMaskImage, imageRegion);
                  if (imageRegion!=0)
                   SDMaskHandler::regionToMask(tempMaskImage,*imageRegion, Float(1.0));
                }
                catch (...) {
                  os << LogIO::WARN << maskString << "is invalid mask. Skipping this mask..." << LogIO::POST;
                }
              }
            }// end of region string
          }// end of non-emtpy maskstring
         
          LatticeExpr<Float> addedmask(tempMaskImage+tempAllMaskImage); 
          tempAllMaskImage.copyData( LatticeExpr<Float>( iif(addedmask > 0.0, 1.0, 0.0) ) );
        }
        imstore->mask()->copyData(tempAllMaskImage);
        imstore->mask()->unlock();
      }
    } catch( AipsError &x )
      {
	throw(AipsError("Error in constructing "+ imstore->getName() +".mask from " + maskString + " : " + x.getMesg()));
      }
  }


  
  void SDMaskHandler::fillMask(SHARED_PTR<SIImageStore> imstore, String maskString)
  {

    try {
      
      //// imstore->mask() will return a pointer to an ImageInterface (allocation happens on first access). 
      
      //    cout << "Call makeMask here to fill in " << imstore->mask()->name() << " from " << maskString <<  ". For now, set mask to 1 inside a central box" << endl;
      
      //interpret maskString 
      if (maskString !="") {
	if ( Table::isReadable(maskString) ) {
	  Table imtab = Table(maskString, Table::Old);
	  Vector<String> colnames = imtab.tableDesc().columnNames();
	  if ( colnames[0]=="map" ) {
	    
	    // looks like a CASA image ... probably should check coord exists in the keyword also...
	    //          cout << "copy this input mask...."<<endl;
	    PagedImage<Float> inmask(maskString); 
	    IPosition inShape = inmask.shape();
	    IPosition outShape = imstore->mask()->shape();
	    Int specAxis = CoordinateUtil::findSpectralAxis(inmask.coordinates());
	    Int outSpecAxis = CoordinateUtil::findSpectralAxis(imstore->mask()->coordinates());
	    if (inShape(specAxis) == 1 && outShape(outSpecAxis)>1) {
	      expandMask(inmask, *(imstore->mask()));
	    }
	    else {
	      copyMask(inmask, *(imstore->mask()));
	    }
	  }// end of ''map''
	}// end of readable table
	else {
	  //cout << maskString << " is not image..."<<endl;
	  ImageRegion* imageRegion=0;
	  SDMaskHandler::regionTextToImageRegion(maskString, *(imstore->mask()), imageRegion);
	  if (imageRegion!=0)
	    SDMaskHandler::regionToMask(*(imstore->mask()),*imageRegion, Float(1.0));
	}// end of region string
      }
      else { 
	/////// Temporary code to set a mask in the inner quarter.
	/////// This is only for testing... should go when 'maskString' can be used to fill it in properly. 
	IPosition imshp = imstore->mask()->shape();
	AlwaysAssert( imshp.nelements() >=2 , AipsError );
	
	Slicer themask;
	IPosition blc(imshp.nelements(), 0);
	IPosition trc = imshp-1;
	IPosition inc(imshp.nelements(), 1);
	
	blc(0)=int(imshp[0]*0.25);
	blc(1)=int(imshp[1]*0.25);
	trc(0)=int(imshp[0]*0.75);
	trc(1)=int(imshp[1]*0.75);
	
	LCBox::verify(blc, trc, inc, imshp);
	Slicer imslice(blc, trc, inc, Slicer::endIsLast);
	
	SHARED_PTR<ImageInterface<Float> >  referenceImage( new SubImage<Float>(*(imstore->mask()), imslice, True) );
	referenceImage->set(1.0);
      }

      imstore->mask()->unlock();
   
    } catch( AipsError &x )
      {
	throw(AipsError("Error in constructing "+ imstore->getName() +".mask from " + maskString + " : " + x.getMesg()));
      }
  }
  
  //void SDMaskHandler::makeMask()
   SHARED_PTR<ImageInterface<Float> > SDMaskHandler::makeMask(const String& maskName, const Quantity threshold,
   //void SDMaskHandler::makeMask(const String& maskName, const Quantity threshold,
                               ImageInterface<Float>& tempim)
   //                             ImageInterface<Float>& tempim,
   //                           ImageInterface<Float> *newMaskImage)
  {
    LogIO os( LogOrigin("SDMaskHandler","makeMask",WHERE) );
    //
    // create mask from a threshold... Imager::mask()...
    //default handling?
    String maskFileName(maskName);
    if ( maskFileName=="" ) { 
      maskFileName = tempim.name() + ".mask";
    }
    if (!cloneImShape(tempim, maskFileName)) {
      throw(AipsError("Cannot make a mask from "+tempim.name()));
    }
    PagedImage<Float> *newMaskImage = new PagedImage<Float>(maskFileName, TableLock::AutoNoReadLocking);
    //newMaskImage = PagedImage<Float>(maskFileName, TableLock::AutoNoReadLocking);
    //PagedImage<Float>(maskFileName, TableLock::AutoNoReadLocking);
    StokesImageUtil::MaskFrom(*newMaskImage, tempim, threshold);
    return SHARED_PTR<ImageInterface<Float> >(newMaskImage);
  }

  //Bool SDMaskHandler::regionToImageMask(const String& maskName, Record* regionRec, Matrix<Quantity>& blctrcs,
  Bool SDMaskHandler::regionToImageMask(ImageInterface<Float>& maskImage, Record* regionRec, Matrix<Quantity>& blctrcs,
            Matrix<Float>& circles, const Float& value) {

    LogIO os(LogOrigin("imager", "regionToImageMask", WHERE));

    try {
      //PagedImage<Float> tempmask(TiledShape(maskImage->shape(),
      //                                    maskImage->niceCursorShape()), maskImage->coordinates(), tempfname);
      SHARED_PTR<ImageInterface<Float> > tempmask;
      tempmask.reset( new TempImage<Float>(TiledShape(maskImage.shape(),maskImage.niceCursorShape()), maskImage.coordinates()) );
      //tempmask = new PagedImage<Float>(maskImage.shape(), maskImage.coordinates(),"__tmp_rgmask");
      tempmask->copyData(maskImage);

      CoordinateSystem cSys=tempmask->coordinates();
      //maskImage.table().markForDelete();
      ImageRegion *boxregions=0;
      ImageRegion *circleregions=0;
      RegionManager regMan;
      regMan.setcoordsys(cSys);
      if (blctrcs.nelements()!=0){
        boxRegionToImageRegion(*tempmask, blctrcs, boxregions);
      }
      if (circles.nelements()!=0) {
        circleRegionToImageRegion(*tempmask, circles, circleregions);
      } 
      ImageRegion* recordRegion=0;
      if(regionRec !=0){
      //if(regionRec.nfields() !=0){
        recordRegionToImageRegion(regionRec, recordRegion);
      }
   
      ImageRegion *unionReg=0;
      if(boxregions!=0 && recordRegion!=0){
        unionReg=regMan.doUnion(*boxregions, *recordRegion);
        delete boxregions; boxregions=0;
        delete recordRegion; recordRegion=0;
      }
      else if(boxregions !=0){
        unionReg=boxregions;
      }
      else if(recordRegion !=0){
        unionReg=recordRegion;
      } 

      if(unionReg !=0){
        regionToMask(*tempmask, *unionReg, value);
        delete unionReg; unionReg=0;
      }
      //As i can't unionize LCRegions and WCRegions;  do circles seperately
      if(circleregions !=0){
        regionToMask(*tempmask, *circleregions, value);
        delete circleregions;
        circleregions=0;
      }
      //maskImage.table().unmarkForDelete();
      maskImage.copyData(*tempmask); 
      //PagedImage<Float> mytest1(tempmask->shape(), tempmask->coordinates(), "mytempmaskcopy");
      //mytest1.copyData(*tempmask);
      //PagedImage<Float> mytest2(maskImage->shape(), maskImage->coordinates(), "mymaskimagecopy");
      //PagedImage<Float> mytest2(maskImage.shape(), maskImage.coordinates(), "mymaskimagecopy");
      //mytest2.copyData(*maskImage);
      //mytest2.copyData(LatticeExpr<Float>(maskImage));
      //cerr<<"copying DONE..."<<endl;
    }
    catch (AipsError& x) {
      os << "Error in regionToMaskImage() : " << x.getMesg() << LogIO::EXCEPTION;
    }
    return True;
  }

  Bool SDMaskHandler::regionToMask(ImageInterface<Float>& maskImage, ImageRegion& imageregion, const Float& value) 
  {
    SubImage<Float> partToMask(maskImage, imageregion, True);
    LatticeRegion latReg=imageregion.toLatticeRegion(maskImage.coordinates(), maskImage.shape());
    ArrayLattice<Bool> pixmask(latReg.get());
    LatticeExpr<Float> myexpr(iif(pixmask, value, partToMask) );
    partToMask.copyData(myexpr);
    return True;
  }

  void SDMaskHandler::boxRegionToImageRegion(const ImageInterface<Float>& maskImage, const Matrix<Quantity>& blctrcs, ImageRegion*& boxImageRegions)
  {
    if(blctrcs.shape()(1) != 4)
      throw(AipsError("Need a list of 4 elements to define a box"));

    CoordinateSystem cSys=maskImage.coordinates();
    RegionManager regMan;
    regMan.setcoordsys(cSys);
    Vector<Quantum<Double> > blc(2);
    Vector<Quantum<Double> > trc(2);
    Int nrow=blctrcs.shape()(0);
    Vector<Int> absRel(2, RegionType::Abs);
    PtrBlock<const WCRegion *> lesbox(nrow);
    for (Int k=0; k < nrow; ++k){
      blc(0) = blctrcs(k,0);
      blc(1) = blctrcs(k,1);
      trc(0) = blctrcs(k,2);
      trc(1) = blctrcs(k,3);
      lesbox[k]= new WCBox (blc, trc, cSys, absRel);
    }
    boxImageRegions=regMan.doUnion(lesbox);
    if (boxImageRegions!=0) {
    }
    for (Int k=0; k < nrow; ++k){
      delete lesbox[k];
    }
  }

  void SDMaskHandler::circleRegionToImageRegion(const ImageInterface<Float>& maskImage, const Matrix<Float>& circles, 
                                         ImageRegion*& circleImageRegions)
  {
    if(circles.shape()(1) != 3)
      throw(AipsError("Need a list of 3 elements to define a circle"));

    CoordinateSystem cSys=maskImage.coordinates();
    RegionManager regMan;
    regMan.setcoordsys(cSys);
    Int nrow=circles.shape()(0);
    Vector<Float> cent(2);
    cent(0)=circles(0,1); cent(1)=circles(0,2);
    Float radius=circles(0,0);
    IPosition xyshape(2,maskImage.shape()(0),maskImage.shape()(1));
    LCEllipsoid *circ= new LCEllipsoid(cent, radius, xyshape);
    //Tell LCUnion to delete the pointers
    LCUnion *elunion= new LCUnion(True, circ);
    //now lets do the remainder
    for (Int k=1; k < nrow; ++k){
      cent(0)=circles(k,1); cent(1)=circles(k,2);
      radius=circles(k,0);
      circ= new LCEllipsoid(cent, radius, xyshape);
      elunion=new LCUnion(True, elunion, circ);
    }
    //now lets extend that to the whole image
    IPosition trc(2);
    trc(0)=maskImage.shape()(2)-1;
    trc(1)=maskImage.shape()(3)-1;
    LCBox lbox(IPosition(2,0,0), trc,
               IPosition(2,maskImage.shape()(2),maskImage.shape()(3)) );
    LCExtension linter(*elunion, IPosition(2,2,3),lbox);
    circleImageRegions=new ImageRegion(linter);
    delete elunion;
  }
 
  void SDMaskHandler::recordRegionToImageRegion(Record* imageRegRec, ImageRegion*& imageRegion ) 
  //void SDMaskHandler::recordRegionToImageRegion(Record& imageRegRec, ImageRegion*& imageRegion ) 
  {
    if(imageRegRec !=0){
      TableRecord rec1;
      rec1.assign(*imageRegRec);
      imageRegion=ImageRegion::fromRecord(rec1,"");
    }
  }


  void SDMaskHandler::regionTextToImageRegion(const String& text, const ImageInterface<Float>& regionImage,
                                            ImageRegion*& imageRegion)
  {
    LogIO os( LogOrigin("SDMaskHandler", "regionTextToImageRegion",WHERE) );

     try {
       IPosition imshape = regionImage.shape();
       CoordinateSystem csys = regionImage.coordinates();
       File fname(text); 
       Record* imageRegRec=0;
       Record myrec;
       //Record imageRegRec;
       if (fname.exists() && fname.isRegular()) {
         RegionTextList  CRTFList(text, csys, imshape);
         myrec = CRTFList.regionAsRecord();
       }
       else { // direct text input....
         Regex rx (Regex::fromPattern("*\\[*\\]*"));
         if (text.matches(rx)) {
           RegionTextList CRTFList(csys, text, imshape);
           myrec = CRTFList.regionAsRecord();
           //cerr<<"myrec.nfields()="<<myrec.nfields()<<endl;
         }
         else {
           throw(AipsError("Input mask, '"+text+"' does not exist if it is inteded as a mask file name."+
                 "Or invalid CRTF syntax if it is intended as a direct region specification."));
         }
       }
       imageRegRec = new Record();
       imageRegRec->assign(myrec);
       recordRegionToImageRegion(imageRegRec, imageRegion);
       delete imageRegRec;
     }
     catch (AipsError& x) {
       os << LogIO::SEVERE << "Exception: "<< x.getMesg() << LogIO::POST;
     }  
  }

  void SDMaskHandler::copyAllMasks(const Vector< SHARED_PTR<ImageInterface<Float> > > inImageMasks, ImageInterface<Float>& outImageMask)
  {
     LogIO os( LogOrigin("SDMaskHandler", "copyAllMasks", WHERE) );

     TempImage<Float> tempoutmask(outImageMask.shape(), outImageMask.coordinates());
     
     for (uInt i = 0; i < inImageMasks.nelements(); i++) {
       copyMask(*inImageMasks(i), tempoutmask);
        outImageMask.copyData( (LatticeExpr<Float>)(tempoutmask+outImageMask) );
     }
  }

  void SDMaskHandler::copyMask(const ImageInterface<Float>& inImageMask, ImageInterface<Float>& outImageMask) 
  {
    LogIO os( LogOrigin("SDMaskHandler", "copyMask", WHERE) );
  
    //output mask coords
    IPosition outshape = outImageMask.shape();
    CoordinateSystem outcsys = outImageMask.coordinates();
    DirectionCoordinate outDirCsys = outcsys.directionCoordinate();
    SpectralCoordinate outSpecCsys = outcsys.spectralCoordinate();
     
    // do regrid   
    IPosition axes(3,0, 1, 2);
    IPosition inshape = inImageMask.shape();
    CoordinateSystem incsys = inImageMask.coordinates(); 
    DirectionCoordinate inDirCsys = incsys.directionCoordinate();
    SpectralCoordinate inSpecCsys = incsys.spectralCoordinate();

    Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(incsys);
    axes(0) = dirAxes(0); 
    axes(1) = dirAxes(1);
    axes(2) = CoordinateUtil::findSpectralAxis(incsys);
    try {
      ImageRegrid<Float> imregrid;
      imregrid.regrid(outImageMask, Interpolate2D::LINEAR, axes, inImageMask); 
    } catch (AipsError &x) {
	throw(AipsError("ImageRegrid error : "+ x.getMesg()));
      }
  } 

  void SDMaskHandler::expandMask(const ImageInterface<Float>& inImageMask, ImageInterface<Float>& outImageMask)
  {
    LogIO os( LogOrigin("SDMaskHandler", "expandMask", WHERE) );

    // expand mask with input range (in spectral axis and stokes?) ... to output range on outimage
    // current expand a continuum mask to a cube mask in channels only (to all channels) 
    IPosition inShape = inImageMask.shape();
    CoordinateSystem inCsys = inImageMask.coordinates();
    Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(inCsys);
    Int inSpecAxis = CoordinateUtil::findSpectralAxis(inCsys);
    Int inNchan = inShape(inSpecAxis); 
    Vector<Stokes::StokesTypes> inWhichPols;
    Int inStokesAxis = CoordinateUtil::findStokesAxis(inWhichPols,inCsys);
    //
    // Single channel(continuum) input mask to output cube mask case:
    //  - It can be different shape in direction axes and will be regridded.
    if (inNchan==1) {
      IPosition outShape = outImageMask.shape();
      CoordinateSystem outCsys = outImageMask.coordinates();
      Vector<Int> outDirAxes = CoordinateUtil::findDirectionAxes(outCsys);
      Int outSpecAxis = CoordinateUtil::findSpectralAxis(outCsys);
      Int outNchan = outShape(outSpecAxis);
      Vector<Stokes::StokesTypes> outWhichPols;
      Int outStokesAxis = CoordinateUtil::findStokesAxis(outWhichPols,outCsys);

      Int stokesInc = 1;
      if (inShape(inStokesAxis)==outShape(outStokesAxis)) {
        stokesInc = inShape(inStokesAxis);
      }
      IPosition start(4,0,0,0,0);
      IPosition length(4,outShape(outDirAxes(0)), outShape(outDirAxes(1)),1,1);
      length(outStokesAxis) = stokesInc;
      Slicer sl(start, length); 

      // make a subImage for regridding output       
      SubImage<Float> chanMask(outImageMask, sl, True);
      
      ImageRegrid<Float> imregrid;
      try {
        imregrid.regrid(chanMask, Interpolate2D::LINEAR, dirAxes, inImageMask);
      } catch (AipsError& x) {
        cerr<<"Attempt to regrid the input mask image failed: "<<x.getMesg()<<endl;
      }
      Array<Float> inMaskData;
      IPosition end2(4,outShape(outDirAxes(0)), outShape(outDirAxes(1)), 1, 1);
      chanMask.doGetSlice(inMaskData, Slicer(start,end2));
      for (Int ich = 1; ich < outNchan; ich++) {
        start(outSpecAxis) = ich;
        IPosition stride(4,1,1,1,1);
        stride(outSpecAxis) = stokesInc; 
        outImageMask.putSlice(inMaskData,start,stride); 
      }
    }
    else {
      throw(AipsError("Input mask,"+inImageMask.name()+" does not conform with the number of channels in output mask"));
    }
  }

  // was Imager::clone()...
  //static Bool cloneImShape(const ImageInterface<Float>& inImage, ImageInterface<Float>& outImage)
  Bool SDMaskHandler::cloneImShape(const ImageInterface<Float>& inImage, const String& outImageName)
  { 
    LogIO os( LogOrigin("SDMaskHandler", "cloneImShape",WHERE) );
    
    try {
      PagedImage<Float> newImage(TiledShape(inImage.shape(),
                                          inImage.niceCursorShape()), inImage.coordinates(),
    //                           outImage.name());
                               outImageName);
      newImage.set(0.0);
      newImage.table().flush(True, True);
    } catch (AipsError& x) {
      os << LogIO::SEVERE << "Exception: " << x.getMesg() << LogIO::POST;
      return False;
    }
    return True;
  }


  Int SDMaskHandler::makeInteractiveMask(SHARED_PTR<SIImageStore>& imstore,
					 Int& niter, Int& cycleniter, 
					 String& threshold, String& cyclethreshold)
  {
    Int ret;
    // Int niter=1000, ncycles=100;
    // String thresh="0.001mJy";
    String imageName = imstore->getName()+".residual"+(imstore->getNTaylorTerms()>1?".tt0":"");
    String maskName = imstore->getName() + ".mask";
    imstore->mask()->unlock();
    cout << "Before interaction : niter : " << niter << " cycleniter : " << cycleniter << " thresh : " << threshold << "  cyclethresh : " << cyclethreshold << endl;
    //    ret = interactiveMasker_p->interactivemask(imageName, maskName,
    //                                            niter, ncycles, threshold);
    ret = interactiveMasker_p->interactivemask(imageName, maskName,
                                               niter, cycleniter, threshold, cyclethreshold);
    cout << "After interaction : niter : " << niter << " cycleniter : " << cycleniter << " thresh : " << threshold << " cyclethresh : " << cyclethreshold << "  ------ ret : " << ret << endl;
    return ret;
  }

  void SDMaskHandler::makeAutoMask(SHARED_PTR<SIImageStore> imstore)
  {
    LogIO os( LogOrigin("SDMaskHandler","makeAutoMask",WHERE) );

    Array<Float> localres;
    // Modification to be able to work with a cube (TT 2014-12-09)
    //imstore->residual()->get( localres , True );
    imstore->residual()->get( localres );

    Array<Float> localmask;
    //imstore->mask()->get( localmask , True );
    imstore->mask()->get( localmask );
   
    Int specAxis = CoordinateUtil::findSpectralAxis(imstore->mask()->coordinates());
    IPosition maskShape = localmask.shape();
    Int ndim = maskShape.nelements();
    IPosition pos(ndim,0);
    IPosition blc(ndim,0);
    IPosition trc(ndim,0);
    trc[0] = maskShape[0]-1; 
    trc[1] = maskShape[1]-1;
    // added per channel mask setting
    for (pos[specAxis] = 0; pos[specAxis]<localmask.shape()[specAxis]; pos[specAxis]++) 
      { 
        IPosition posMaxAbs( localmask.shape().nelements(), 0);
        blc[specAxis]=pos[specAxis];
        trc[specAxis]=pos[specAxis];
        Float maxAbs=0.0;
        Float minVal;
        IPosition posmin(localmask.shape().nelements(), 0);
        //minMax(minVal, maxAbs, posmin, posMaxAbs, localres);
        minMax(minVal, maxAbs, posmin, posMaxAbs, localres(blc,trc));

    //    cout << "Max position : " << posMaxAbs << endl;

        Int dist=5;
     
        //IPosition pos(2,0,0); // Deal with the input shapes properly......
        for (pos[0]=posMaxAbs[0]-dist; pos[0]<posMaxAbs[0]+dist; pos[0]++)
          {
	    for (pos[1]=posMaxAbs[1]-dist; pos[1]<posMaxAbs[1]+dist; pos[1]++)
	      {
	        if( pos[0]>0 && pos[0]<localmask.shape()[0] && pos[1]>0 && pos[1]<localmask.shape()[1] )
	          {
		    localmask( pos ) = 1.0;
	          }
	      }
          }
      } // over channels
    //cout << "Sum of mask : " << sum(localmask) << endl;
    Float summask = sum(localmask);
    if( summask==0.0 ) { localmask=1.0; summask = sum(localmask); }
    os << LogIO::NORMAL1 << "Make Autobox mask with " << summask << " available pixels " << LogIO::POST;

    imstore->mask()->put( localmask );

    //    imstore->mask()->get( localmask , True );
    //    cout << "Sum of imstore mask : " << sum( localmask ) << endl;

  }

  void SDMaskHandler::autoMask(SHARED_PTR<SIImageStore> imstore, 
                               const String& alg, 
                               const String& threshold, 
                               const Float& fracofpeak, 
                               const String& resolution) 
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMask",WHERE) );
    
    cerr<<"autoMask alg="<<alg<<endl;
    TempImage<Float>* tempres = new TempImage<Float>(imstore->residual()->shape(), imstore->residual()->coordinates()); 
    Array<Float> resdata;
    Array<Float> maskdata;
    Array<Float> psfdata;
    imstore->residual()->get(resdata);
    tempres->put(resdata);
    tempres->setImageInfo(imstore->residual()->imageInfo());

    TempImage<Float>* temppsf = new TempImage<Float>(imstore->psf()->shape(), imstore->psf()->coordinates()); 
    imstore->psf()->get(psfdata);
    temppsf->put(psfdata);
    temppsf->setImageInfo(imstore->psf()->imageInfo());

    TempImage<Float>* tempmask = new TempImage<Float>(imstore->mask()->shape(), imstore->mask()->coordinates());
    // get current mask
    imstore->mask()->get(maskdata);
    String maskname = imstore->getName()+".mask";
    tempmask->put(maskdata);
    //
    // create pixel mask (set to False for the previous selected region(s))
    LatticeExpr<Bool> pixmask( iif(*tempmask > 0.0, False, True) );
    TempImage<Float>* dummy = new TempImage<Float>(tempres->shape(), tempres->coordinates());
    dummy->attachMask(pixmask);
    if (ntrue(dummy->getMask())) tempres->attachMask(pixmask);
    delete dummy; dummy=0;
    //input 
    Quantity qthresh(0,"");
    Quantity qreso(0,"");
    Quantity::read(qreso,resolution);
    //cerr<<"input resolution : qreso.getValue()="<<qreso.getValue()<<endl;
    os<<"Input resolution : "<<qreso.getValue()<<" "<<qreso.getUnit()<<LogIO::POST;
    Float sigma;
    // if fracofpeak (fraction of a peak) is specified, use it to set a threshold
    if ( fracofpeak != 0.0 ) {
      if (fracofpeak > 1.0 )
         throw(AipsError("Fracofpeak must be < 1.0"));
      sigma = 0.0;
    }
    else if(Quantity::read(qthresh,threshold) ) {
      // evaluate threshold input 
      //cerr<<"qthresh="<<qthresh.get().getValue()<<" unit="<<qthresh.getUnit()<<endl;
      os << "Input threshold (qthresh) ="<<qthresh.get().getValue()<<" "<<qthresh.getUnit()<<LogIO::POST;
      if (qthresh.getUnit()!="") {
        // use qthresh and set sigma =0.0 to ignore
        sigma = 0.0;
      }
      else {
        sigma = String::toFloat(threshold);
        if (sigma==0.0) {
            // default case: threshold, fracofpeak unset => use default (3sigma)
            sigma = 3.0;
        }
      }
    }
    else {
      if (!sigma) {
        throw(AipsError("Unrecognized automask threshold="+threshold));
      }
    }
       
    //do statistics
    SHARED_PTR<casa::ImageInterface<float> > tempres_ptr(tempres);
    ImageStatsCalculator imcalc( tempres_ptr, 0, "", False); 
    Vector<Int> axes(2);
    axes[0] = 0;
    axes[1] = 1;
    imcalc.setAxes(axes);
    Record thestats = imcalc.statistics();
    Array<Double> max, min, rms;
    thestats.get(RecordFieldId("max"), max);
    thestats.get(RecordFieldId("rms"), rms);
    os<<"Stats -- rms.nelements()="<<rms.nelements()<<" rms(0)="<<rms[0]<<LogIO::DEBUG1;
    os<<"Stats -- max.nelements()="<<max.nelements()<<" max(0)="<<max[0]<<LogIO::DEBUG1;
    if (alg==String("")) {
      cerr<<" calling makeAutoMask(), simple 1 cleanbox around the max"<<endl;
      makeAutoMask(imstore);
    }
    else if (alg==String("thresh")) {
      autoMaskByThreshold(*tempmask, *tempres, *temppsf, qreso, qthresh, fracofpeak, thestats, sigma);
      //cerr<<" automaskbyThreshold...."<<endl;
      tempmask->get(maskdata);
      imstore->mask()->put(maskdata);
    } 
    delete tempmask; tempmask=0;
  }

  void SDMaskHandler::autoMaskByThreshold(ImageInterface<Float>& mask,
                                          const ImageInterface<Float>& res, 
                                          const ImageInterface<Float>& psf, 
                                          const Quantity& resolution, 
                                          const Quantity& qthresh, 
                                          const Float& fracofpeak,
                                          const Record& stats, 
                                          const Float& sigma) 
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMaskByThreshold",WHERE) );
     Array<Double> rms, max;
     Double thresh, rmsthresh, maxval;
     Int npix;
     // taking account for beam or input resolution
     TempImage<Float> tempmask(mask.shape(), mask.coordinates());
     CoordinateSystem incsys = res.coordinates();
     Vector<Double> incVal = incsys.increment(); 
     Vector<String> incUnit = incsys.worldAxisUnits();
     Quantity qinc(incVal[0],incUnit[0]);
     if (resolution.get().getValue() ) {
       //npix = 2*Int(abs( resolution.getValue()/qinc.getValue(resolution.getUnit()) ) );
       npix = 2*Int(abs( resolution/(qinc.convert(resolution),qinc) ).getValue() );
     }
     else {
       //use beam from residual or psf
       ImageInfo resInfo = res.imageInfo();
       ImageInfo psfInfo = psf.imageInfo();
       GaussianBeam beam;
       if (resInfo.hasBeam() || psfInfo.hasBeam()) {
         if (resInfo.hasSingleBeam()) {
           beam = resInfo.restoringBeam();  
         }
         else if (resInfo.hasMultipleBeams()) {
           beam = CasaImageBeamSet(resInfo.getBeamSet()).getCommonBeam(); 
         }
         else if (psfInfo.hasSingleBeam()) {
           beam = psfInfo.restoringBeam();  
         }
         else {
           beam = CasaImageBeamSet(psfInfo.getBeamSet()).getCommonBeam(); 
         }
         Quantity bmaj = beam.getMajor();
         npix = 2*Int( abs( (bmaj/(qinc.convert(bmaj),bmaj)).get().getValue() ) );
       }
       else {
          throw(AipsError("No restoring beam(s) in the input image/psf or resolution is given"));
       }
     }
     //cerr<<"npix="<<npix<<endl;
     if (npix==0) {
       os << "Resolution too small. No binning (nbin=1)  is applied input image to evaluate the threshold." << LogIO::POST;
       npix=1;
     }
     RebinImage<Float> tempRebinnedIm( res, IPosition(4,npix,npix,1,1) );
     PagedImage<Float> temprebincopy(TiledShape(tempRebinnedIm.shape()),tempRebinnedIm.coordinates(), String("mytemp_rebinim"));
     temprebincopy.copyData(tempRebinnedIm);

     // Determine threshold 
     Double minval;
     if (fracofpeak) {
       stats.get(RecordFieldId("max"), max);
       minMax(minval,maxval,max);
       //cerr<<"minval="<<minval<<" maxval="<<maxval<<endl;
       rmsthresh = maxval * fracofpeak; 
     }
     else if (sigma) {
       stats.get(RecordFieldId("rms"), rms);
       minMax(minval,maxval,rms); 
       //cerr<<"minval="<<minval<<" maxval="<<maxval<<endl;
       rmsthresh = maxval * sigma;
     }      
     else {
       rmsthresh = qthresh.getValue(Unit("Jy"));
       if ( rmsthresh==0.0 ) 
         { throw(AipsError("Threshold for automask is not set"));}
     }
     os<<"sigma="<<sigma<<" rmsthresh="<<rmsthresh<<LogIO::POST;

     thresh = rmsthresh / sqrt(npix);
     //cerr<<" thresh="<<thresh<<endl;
     // apply threshold to rebinned image to generate a temp image mask
     LatticeExpr<Float> tempthresh( iif( tempRebinnedIm > thresh, 1.0, 0.0) );
     TempImage<Float> tempthreshIm(tempRebinnedIm.shape(), tempRebinnedIm.coordinates() );
     tempthreshIm.copyData(tempthresh);
     PagedImage<Float> tempthreshimcopy(TiledShape(tempthreshIm.shape()),tempthreshIm.coordinates(), String("mytemp_threshim"));
     tempthreshimcopy.copyData(tempthreshIm);

     //regrid
     IPosition axes(3,0, 1, 2);
     Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(incsys);
     axes(0) = dirAxes(0);
     axes(1) = dirAxes(1);
     axes(2) = CoordinateUtil::findSpectralAxis(incsys);
     TempImage<Float> tempIm2(res.shape(), res.coordinates() );
     ImageRegrid<Float> imRegrid; 
     imRegrid.regrid(tempIm2, Interpolate2D::LINEAR, axes, tempthreshIm);

     // convolve to a beam = npix
     TempImage<Float>* tempIm3 = new TempImage<Float>(res.shape(), res.coordinates() );
     tempIm3->copyData(tempIm2);
     SHARED_PTR<casa::ImageInterface<float> > tempIm3_ptr(tempIm3);
     Vector<Quantity> convbeam(3);
     convbeam[0] = Quantity(npix, "pix");
     convbeam[1] = Quantity(npix, "pix");
     convbeam[2] = Quantity(0.0, "deg");
     Record dammyRec=Record();
     String convimname("temp_convim");
     Image2DConvolver<Float> convolver(tempIm3_ptr, &dammyRec, String(""), convimname, True); 
     convolver.setKernel("GAUSSIAN", convbeam[0], convbeam[1], convbeam[2]);
     convolver.setAxes(std::make_pair(0,1));
     convolver.setScale(Double(-1.0));
     convolver.setSuppressWarnings(True);
     convolver.convolve();
     // replaced the deprecated static method with object-oriented one
     //Image2DConvolver<Float>::convolve(os, tempIm3_ptr, tempIm2, VectorKernel::GAUSSIAN, IPosition(2, 0, 1), convbeam, True, Double(-1.0), True, False);   
     //PagedImage<Float> tempconvim(TiledShape(res.shape()), res.coordinates(), String("mytemp_convim"));
     PagedImage<Float> tempconvim(convimname);
     //tempconvim.copyData(*tempIm3_ptr);

     // fudge factor?  
     Float afactor = 2.0;
     //cerr<<"thresh/afactor="<<thresh/afactor<<endl;
     //LatticeExpr<Float> themask( iif( *(tempIm3_ptr) > thresh/afactor, 1.0, 0.0 ));
     LatticeExpr<Float> themask( iif( tempconvim > thresh/afactor, 1.0, 0.0 ));
     mask.copyData( (LatticeExpr<Float>)( iif((mask + themask) > 0.0, 1.0, 0.0  ) ) );
  }
  
  void SDMaskHandler::makePBMask(SHARED_PTR<SIImageStore> imstore, Float pblimit)
  {
    LogIO os( LogOrigin("SDMaskHandler","makePBMask",WHERE) );

    if( imstore->hasPB() ) // Projection algorithms will have this.
      {
	LatticeExpr<Float> themask( iif( (*(imstore->pb())) > pblimit , 1.0, 0.0 ) );
	imstore->mask()->copyData( themask );
      }
    else // Calculate it here...
      {
	// Get antenna diameter
	// Get frequency
	// Assuming a Gaussian, construct a circle region at pblimit.

	// But for now...
	throw(AipsError("Need PB/Sensitivity/Weight image before a PB-based mask can be made for "+imstore->getName())); 
      }
    // Also add option to just use the vpmanager or whatever centralized PB repository there will be (sometime in the distant future...).

  }// end of makePBMask

  void SDMaskHandler::autoMaskWithinPB(SHARED_PTR<SIImageStore> imstore, Float pblimit)
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMaskWithinPB",WHERE) );

    autoMask( imstore, "thresh" );

    if( imstore->hasPB() ) // Projection algorithms will have this.
      {
	LatticeExpr<Float> themask( iif( (*(imstore->pb())) > pblimit , (*(imstore->mask())), 0.0 ) );
	imstore->mask()->copyData( themask );
      }
    // else... same options as makePBMask (put it into a helper function)
  }// end of autoMaskWithinPB

} //# NAMESPACE CASA - END


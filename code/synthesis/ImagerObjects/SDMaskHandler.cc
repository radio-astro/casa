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
#include <images/Regions/RegionHandler.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCUnion.h>
#include <imageanalysis/ImageAnalysis/CasaImageBeamSet.h>
#include <imageanalysis/ImageAnalysis/ImageDecomposer.h>
#include <imageanalysis/ImageAnalysis/ImageStatsCalculator.h>
#include <imageanalysis/ImageAnalysis/Image2DConvolver.h>
#include <imageanalysis/ImageAnalysis/ImageRegridder.h>
#include <casa/OS/File.h>
#include <lattices/LEL/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/LRegions/LCEllipsoid.h>
#include <lattices/LRegions/LCUnion.h>
#include <lattices/LRegions/LCExtension.h>
#include <lattices/LRegions/LCPagedMask.h>
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


using namespace casacore;
namespace casa { //# NAMESPACE CASA - BEGIN


  SDMaskHandler::SDMaskHandler()
  {
    interactiveMasker_p = new InteractiveMasking();
    itsMax = DBL_MAX;
    itsRms = DBL_MAX;
    itsSidelobeLevel = 0.0;
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
      tempAllMaskImage.set(0.0);
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
         
          LatticeExpr<Float> addedmask( tempMaskImage + tempAllMaskImage ); 
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
	
	SHARED_PTR<ImageInterface<Float> >  referenceImage( new SubImage<Float>(*(imstore->mask()), imslice, true) );
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
    }
    catch (AipsError& x) {
      os << "Error in regionToMaskImage() : " << x.getMesg() << LogIO::EXCEPTION;
    }
    return true;
  }

  Bool SDMaskHandler::regionToMask(ImageInterface<Float>& maskImage, ImageRegion& imageregion, const Float& value) 
  {
    SubImage<Float> partToMask(maskImage, imageregion, true);
    LatticeRegion latReg=imageregion.toLatticeRegion(maskImage.coordinates(), maskImage.shape());
    ArrayLattice<Bool> pixmask(latReg.get());
    LatticeExpr<Float> myexpr(iif(pixmask, value, partToMask) );
    partToMask.copyData(myexpr);
    return true;
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
    LCUnion *elunion= new LCUnion(true, circ);
    //now lets do the remainder
    for (Int k=1; k < nrow; ++k){
      cent(0)=circles(k,1); cent(1)=circles(k,2);
      radius=circles(k,0);
      circ= new LCEllipsoid(cent, radius, xyshape);
      elunion=new LCUnion(true, elunion, circ);
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
    IPosition axes(3,0,1,2);
    IPosition inshape = inImageMask.shape();
    CoordinateSystem incsys = inImageMask.coordinates(); 
    DirectionCoordinate inDirCsys = incsys.directionCoordinate();
    SpectralCoordinate inSpecCsys = incsys.spectralCoordinate();
    //Check the conversion layer consistentcy between input and output.
    //Change the frame of the convesion layer in incsys to that of outcsys if different.
    if (inSpecCsys.frequencySystem(True)!=outSpecCsys.frequencySystem(True)) {
      incsys.setSpectralConversion(MFrequency::showType(outSpecCsys.frequencySystem(True)));
    }

    Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(incsys);
    axes(0) = dirAxes(0); 
    axes(1) = dirAxes(1);
    axes(2) = CoordinateUtil::findSpectralAxis(incsys);

    //const String outfilename = outImageMask.name()+"_"+String::toString(HostInfo::processID());

    try {
      // Since regrid along the spectral axis does not seem to work
      // properly, replacing with ImageRegridder 
      //ImageRegrid<Float> imregrid;
      //imregrid.showDebugInfo(1);
      //imregrid.regrid(outImageMask, Interpolate2D::LINEAR, axes, inImageMask); 
      //
      TempImage<Float>* inImageMaskptr = new TempImage<Float>(inshape,incsys);
      ArrayLattice<Bool> inimmask(inImageMask.getMask());
      inImageMaskptr->copyData((LatticeExpr<Float>)(inImageMask * iif(inimmask,1.0,0.0)) );
      //
      SPCIIF tempim(inImageMaskptr);
      SPCIIF templateim(new TempImage<Float>(outshape,outcsys));
      Record* dummyrec = 0;
      //ImageRegridder regridder(tempim, outfilename, templateim, axes, dummyrec, "", true, outshape);
      ImageRegridder regridder(tempim, "", templateim, axes, dummyrec, "", true, outshape);
      regridder.setMethod(Interpolate2D::LINEAR);
      SPIIF retim = regridder.regrid();
      //outImageMask.copyData( (LatticeExpr<Float>) iif(*retim > 0.1, 1.0, 0.0)  );
      ArrayLattice<Bool> retimmask(retim->getMask());
      //LatticeExpr<Float> withmask( (*retim) * iif(retimmask,1.0,0.0) );
      //outImageMask.copyData( withmask );
      outImageMask.copyData( (LatticeExpr<Float>)((*retim) * iif(retimmask,1.0,0.0))  );
      //LatticeUtilities::copyDataAndMask(os, outImageMask, *retim );
    } catch (AipsError &x) {
	throw(AipsError("Image regrid error : "+ x.getMesg()));
      }

    // no longer needed (output file in regrid is now set to "" so no need of this clean-up)
    //try
    //  {
	// delete the outfilename image on disk
    //	Directory dd(outfilename);
    //	dd.removeRecursive();
    //  }
    //catch (AipsError &x) {
      //      throw(AipsError("Cannot delete temporary mask image : "+ x.getMesg()));
    //  os << LogIO::WARN << "Cannot  delete temporary mask image : " << x.getMesg() << LogIO::POST;
    //}
    
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
      SubImage<Float> chanMask(outImageMask, sl, true);
      
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
      newImage.table().flush(true, true);
    } catch (AipsError& x) {
      os << LogIO::SEVERE << "Exception: " << x.getMesg() << LogIO::POST;
      return false;
    }
    return true;
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
    //imstore->residual()->get( localres , true );
    imstore->residual()->get( localres );

    Array<Float> localmask;
    //imstore->mask()->get( localmask , true );
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

    //    imstore->mask()->get( localmask , true );
    //    cout << "Sum of imstore mask : " << sum( localmask ) << endl;

  }

  void SDMaskHandler::autoMask(SHARED_PTR<SIImageStore> imstore, 
                               const String& alg, 
                               const String& threshold, 
                               const Float& fracofpeak, 
                               const String& resolution,
                               const Float& resbybeam,
                               const Int nmask,
                               const Bool autoadjust,
                               Float pblimit)
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMask",WHERE) );
    
    //currently supported alg:
    //  onebox: a box around a max (box size +/-5pix around max position)
    //  thresh: threshold based auto masking (uses threshold or fracofpeak, and resolution)
    //      
    os << LogIO::NORMAL2 <<"algorithm:"<<alg<<LogIO::POST;
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

    if (pblimit>0.0 && imstore->hasPB()) {
      //cerr<<" applying pb mask ..."<<endl;
      LatticeExpr<Bool> pixmask( iif(*tempmask > 0.0, True, False));
      TempImage<Float>* dummy = new TempImage<Float>(tempres->shape(), tempres->coordinates());
      dummy->attachMask(pixmask);
      LatticeExpr<Float> themask;
      if (!ntrue(dummy->getMask())) { // initial zero mask
        //themask = LatticeExpr<Float>( iif( (*(imstore->pb())) > pblimit , 1.0 , 0.0 ));
        themask = LatticeExpr<Float>( *tempmask);
      }
      else {
        themask = LatticeExpr<Float>( iif( (*(imstore->pb())) > pblimit, *(imstore->mask()), 0.0));
      } 
      // attache pixmask to temp res image to be used in stats etc
      //cerr<<"attaching pixmask to res.."<<endl;
      tempres->attachMask(LatticeExpr<Bool> ( iif(*(imstore->pb()) > pblimit, True, False)));
      //cerr<<"Has pixmask now?= "<<tempres->hasPixelMask()<<endl;
      imstore->mask()->copyData( themask );
      imstore->mask()->get(maskdata);
      tempmask->put(maskdata);
      delete dummy;
    }
     
    // Not use this way for now. Got an issue on removing pixel mask from *.mask image
    // retrieve pixelmask (i.e.  pb mask)
    //LatticeExpr<Bool> pixmasyyk;
    //if (imstore->mask()->hasPixelMask()) {
    //  pixmask = LatticeExpr<Bool> (imstore->mask()->pixelMask()); 
      //
      // create pixel mask (set to True for the previous selected region(s) to exclude the region from the stats/masking )
      //LatticeExpr<Bool> prevmask( iif(*tempmask > 0.0 || pixmask, True, False) );
    //  TempImage<Float>* dummy = new TempImage<Float>(tempres->shape(), tempres->coordinates());
    //  dummy->attachMask(pixmask);
      //if (ntrue(dummy->getMask())) tempres->attachMask(pixmask);
    //  if (ntrue(dummy->getMask())) {
        //tempmask->removeMask();
    //  }
    //  else {
    //    os<<LogIO::DEBUG1<<"No pixel mask"<<LogIO::POST;
    //  }  
    //  delete dummy; dummy=0;
    //}
    //
    //input 
    Quantity qthresh(0,"");
    Quantity qreso(0,"");
    Quantity::read(qreso,resolution);
    Float sigma = 0.0;
    // if fracofpeak (fraction of a peak) is specified, use it to set a threshold
    if ( fracofpeak != 0.0 ) {
      if (fracofpeak > 1.0 )
         throw(AipsError("Fracofpeak must be < 1.0"));
      sigma = 0.0;
    }
    else if(Quantity::read(qthresh,threshold) ) {
      // evaluate threshold input 
      //cerr<<"qthresh="<<qthresh.get().getValue()<<" unit="<<qthresh.getUnit()<<endl;
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
       
    //TempImage<Float>* resforstats = new TempImage<Float>(imstore->residual()->shape(), imstore->residual()->coordinates()); 
    //Array<Float> resdata2;
    //imstore->residual()->get(resdata2);
    //resforstats->put(resdata2);
    //resforstats->setImageInfo(imstore->residual()->imageInfo());
    //LatticeExpr<Bool> prevmask( iif(*tempmask > 0.0 , True, False) );
    //resforstats->attachMask(prevmask);
    //SHARED_PTR<casacore::ImageInterface<float> > tempres_ptr(resforstats);
    /**
    SHARED_PTR<casacore::ImageInterface<float> > tempres_ptr(tempres);
    //cerr<<" tempres->hasPixelMask? "<<tempres->hasPixelMask()<<endl;
    ImageStatsCalculator imcalc( tempres_ptr, 0, "", False); 
    Vector<Int> axes(2);
    axes[0] = 0;
    axes[1] = 1;
    imcalc.setAxes(axes);
    // for now just for new autobox alg.
    if (alg.contains("newauto") ) {
       imcalc.setRobust(true);
    }
    Record thestats = imcalc.statistics();
    ***/

    Record*  region_ptr=0;
    String LELmask("");
    // note: tempres is res image with pblimit applied
    Bool robust(false);
    if (alg.contains("newauto") ) {
       robust=true;
       // define an annulus 
       Float outerRadius=0.0;
       Float innerRadius=1.0;
       if (imstore->hasPB()) {
          //use pb based annulus for statistics
          outerRadius = 0.2;
          innerRadius = 0.3; 
          LatticeExpr<Bool> testlat( iif(( *imstore->pb() < innerRadius && *imstore->pb() > outerRadius) , True, False) );
          TempImage<Float>* testres = new TempImage<Float>(imstore->residual()->shape(), imstore->residual()->coordinates()); 
          testres->set(1.0);
          testres->attachMask(testlat);
          cerr<<"ntrue(testlat) = "<<ntrue(testres->getMask())<<endl;
          if (ntrue(testres->getMask()) > 0) { 
              String pbname = imstore->getName()+".pb";
              LELmask=pbname+"<"+String::toString(innerRadius)+" && "+pbname+">"+String::toString(outerRadius);   
          }
       }
    } 
    cerr<<"LELmask="<<LELmask<<endl;
    Record thestats = calcImageStatistics(*tempres, *tempmask, LELmask, region_ptr, robust);
    Array<Double> maxs, mins, rmss, mads;
    thestats.get(RecordFieldId("max"), maxs);
    thestats.get(RecordFieldId("rms"), rmss);
    os<< LogIO::DEBUG1 << "All rms's on the input image -- rms.nelements()="<<rmss.nelements()<<" rms="<<rmss<<LogIO::POST;
    os<< LogIO::DEBUG1 << "All max's on the input image -- max.nelements()="<<maxs.nelements()<<" max="<<maxs<<LogIO::POST;
    if (alg.contains("newauto")) {
       thestats.get(RecordFieldId("medabsdevmed"), mads);
       os<< LogIO::DEBUG1 << "All max's on the input image -- mad.nelements()="<<mads.nelements()<<" mad="<<mads<<LogIO::POST;
    }

    os<<"SidelobeLevel = "<<imstore->getPSFSidelobeLevel()<<LogIO::POST;
    itsSidelobeLevel = imstore->getPSFSidelobeLevel();
    //os<< "mask algortihm ="<<alg<< LogIO::POST;
    if (alg==String("") || alg==String("onebox")) {
      //cerr<<" calling makeAutoMask(), simple 1 cleanbox around the max"<<endl;
      makeAutoMask(imstore);
    }
    else if (alg==String("thresh")) {
      autoMaskByThreshold(*tempmask, *tempres, *temppsf, qreso, resbybeam, qthresh, fracofpeak, thestats, sigma, nmask, autoadjust);
    }
    else if (alg==String("thresh2")) {
      autoMaskByThreshold2(*tempmask, *tempres, *temppsf, qreso, resbybeam, qthresh, fracofpeak, thestats, sigma, nmask);
    }
    /***
    // this did not work (it won't physically remove the mask from the image 
    if (imstore->mask()->hasPixelMask()) {
      imstore.get()->mask()->removeRegion(fname, RegionHandler::Any, False);
      cerr<<"imstore->mask()->name()="<<imstore->mask()->name()<<endl;
      cerr<<" mask: "<<fname<<" exist on disk? ="<<File(fname).isDirectory()<<endl;
    }
    ***/
    tempmask->get(maskdata);
    imstore->mask()->put(maskdata);
    delete tempmask; tempmask=0;
  }

  Record SDMaskHandler::calcImageStatistics(ImageInterface<Float>& res, ImageInterface<Float>& /*  prevmask */, String& LELmask,  Record* regionPtr, const Bool robust )
  { 
    TempImage<Float>* tempres = new TempImage<Float>(res.shape(), res.coordinates()); 
    Array<Float> resdata;
    res.get(resdata);
    tempres->put(resdata);
    tempres->setImageInfo(res.imageInfo());
    SHARED_PTR<casacore::ImageInterface<Float> > tempres_ptr(tempres);
    
    // 2nd arg is regionRecord, 3rd is LELmask expression and those will be AND 
    // to define a region to be get statistics
    //ImageStatsCalculator imcalc( tempres_ptr, 0, "", False); 
    //String lelstring = pbname+">0.92 && "+pbname+"<0.98";
    //cerr<<"lelstring = "<<lelstring<<endl;
    //cerr<<"LELMask="<<LELmask<<endl;
    ImageStatsCalculator imcalc( tempres_ptr, regionPtr, LELmask, False); 
    Vector<Int> axes(2);
    axes[0] = 0;
    axes[1] = 1;
    imcalc.setAxes(axes);
    imcalc.setRobust(robust);
    Record thestats = imcalc.statistics();
    //cerr<<"thestats="<<thestats<<endl;
    //Array<Double> max, min, rms, mad;
    //thestats.get(RecordFieldId("max"), max);
    return thestats;
  }

  void SDMaskHandler::autoMaskByThreshold(ImageInterface<Float>& mask,
                                          const ImageInterface<Float>& res, 
                                          const ImageInterface<Float>& psf, 
                                          const Quantity& resolution,
                                          const Float& resbybeam,
                                          const Quantity& qthresh, 
                                          const Float& fracofpeak,
                                          const Record& stats, 
                                          const Float& sigma, 
                                          const Int nmask,
                                          const Bool autoadjust) 
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMaskByThreshold",WHERE) );
    Array<Double> rms, max;
    Double rmsthresh, minrmsval, maxrmsval, minmaxval, maxmaxval;
    IPosition minrmspos, maxrmspos, minmaxpos, maxmaxpos;
    Int npix;
    //for debug set to True to save intermediate mask images on disk
    Bool debug(False);

    //automask stage selecitons
    Bool dobin(True);
    Bool doregrid(True);
    Bool doconvolve(True);

    // taking account for beam or input resolution
    TempImage<Float> tempmask(mask.shape(), mask.coordinates());
    IPosition shp = mask.shape();
    CoordinateSystem incsys = res.coordinates();
    Vector<Double> incVal = incsys.increment(); 
    Vector<String> incUnit = incsys.worldAxisUnits();
    Quantity qinc(incVal[0],incUnit[0]);
    if (resolution.get().getValue() ) {
      //npix = 2*Int(abs( resolution/(qinc.convert(resolution),qinc) ).getValue() );
      npix = Int(abs( resolution/(qinc.convert(resolution),qinc) ).getValue() );
      os << LogIO::NORMAL2 << "Use the input resolution:"<<resolution<<" fo binning "<< LogIO::POST;
      os << LogIO::DEBUG1 << "inc = "<<qinc.getValue(resolution.getUnit())<<LogIO::POST;
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
        if (resbybeam > 0.0 ) {
          //npix = 2*Int( Double(resbybeam) * abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
          npix = Int( Double(resbybeam) * abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
          os << LogIO::NORMAL2 << "Use "<< resbybeam <<" x  beam size(maj)="<< Double(resbybeam)*bmaj <<" for binning."<< LogIO::POST;
        }
        else {
          //npix = 2*Int( abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
          npix = Int( abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
          os << LogIO::NORMAL2 << "Use a beam size(maj):"<<bmaj<<" for binning."<< LogIO::POST;
        } 
      }
      else {
         throw(AipsError("No restoring beam(s) in the input image/psf or resolution is given"));
      }
    }
    os << LogIO::DEBUG1 << "Acutal bin size used: npix="<<npix<< LogIO::POST;
    if (npix==0) {
      os << "Resolution too small. No binning (nbin=1)  is applied input image to evaluate the threshold." << LogIO::POST;
      npix=1;
    }


    // Determine threshold from input image stats
    stats.get(RecordFieldId("max"), max);
    stats.get(RecordFieldId("rms"), rms);
    minMax(minmaxval,maxmaxval,minmaxpos, maxmaxpos, max);
    minMax(minrmsval,maxrmsval,minrmspos, maxrmspos, rms); 
    os << LogIO::DEBUG1 <<"stats on the image: max="<<maxmaxval<<" rms="<<maxrmsval<<endl;
    if (fracofpeak) {
      rmsthresh = maxmaxval * fracofpeak; 
      //os << LogIO::NORMAL <<"Threshold by fraction of the peak(="<<fracofpeak<<") * max: "<<rmsthresh<< LogIO::POST;
      os << LogIO::DEBUG1 <<"max at "<<maxmaxpos<<", dynamic range = "<<maxmaxval/rms(maxmaxpos) << LogIO::POST;
    }
    else if (sigma) {
      //cerr<<"minval="<<minval<<" maxval="<<maxval<<endl;
      rmsthresh = maxrmsval * sigma;
      //os << LogIO::NORMAL <<"Threshold by sigma(="<<sigma<<")* rms (="<<maxrmsval<<") :"<<rmsthresh<< LogIO::POST;
      os << LogIO::DEBUG1 <<"max rms at "<<maxrmspos<<", dynamic range = "<<max(maxrmspos)/maxrmsval << LogIO::POST;
    }      
    else {
      rmsthresh = qthresh.getValue(Unit("Jy"));
      if ( rmsthresh==0.0 ) 
        { throw(AipsError("Threshold for automask is not set"));}
    }
    //os << LogIO::NORMAL2 <<" thresh="<<rmsthresh<<LogIO::POST;


    TempImage<Float>* tempIm2 = new TempImage<Float>(res.shape(), res.coordinates() );
    TempImage<Float>* tempIm = new TempImage<Float>(res.shape(), res.coordinates() );
    tempIm->copyData(res);    

    SPCIIF tempIm2_ptr(tempIm2);
    SPIIF tempIm3_ptr(tempIm);
    SPIIF tempIm_ptr;
    //
    //binning stage
    if (dobin) {
      tempIm_ptr =  makeMaskFromBinnedImage(res, npix, npix, fracofpeak, sigma, nmask, autoadjust, rmsthresh);
      //for debugging: save the mask at this stage
      if (debug) {
        PagedImage<Float> tempBinIm(TiledShape(tempIm_ptr.get()->shape()), tempIm_ptr.get()->coordinates(), "binnedThresh.Im");
        tempBinIm.copyData(*(tempIm_ptr.get()));
      }
    }
    if (doregrid) {
      //regrid
      os << LogIO::DEBUG1 <<" now regridding..."<<LogIO::POST;
      IPosition axes(3,0, 1, 2);
      Vector<Int> dirAxes = CoordinateUtil::findDirectionAxes(incsys);
      axes(0) = dirAxes(0);
      axes(1) = dirAxes(1);
      axes(2) = CoordinateUtil::findSpectralAxis(incsys);
      Record* dummyrec = 0;
      SPCIIF inmask_ptr(tempIm_ptr);
      ImageRegridder regridder(inmask_ptr, "", tempIm2_ptr, axes, dummyrec, "", True, shp);
      regridder.setMethod(Interpolate2D::LINEAR);
      tempIm_ptr = regridder.regrid();
      //for debugging: save the mask at this stage
      if (debug) {
        PagedImage<Float> tempGridded(TiledShape(tempIm_ptr.get()->shape()), tempIm_ptr.get()->coordinates(), "binAndGridded.Im");
        tempGridded.copyData(*(tempIm_ptr.get()));
      }
    }
    else {
      tempIm_ptr = tempIm3_ptr;
    }
    if (doconvolve) {
    //
      SPIIF outmask = convolveMask(*(tempIm_ptr.get()), npix, npix);
      tempIm_ptr = outmask;
      //
      //for debugging: save the mask at this stage
      if (debug) { 
        PagedImage<Float> tempconvIm(TiledShape(tempIm_ptr.get()->shape()), tempIm_ptr.get()->coordinates(),"convolved.Im");
        tempconvIm.copyData(*(tempIm_ptr.get()));
      }
      //os<<"done convolving the mask "<<LogIO::POST;
    }

    //os <<"Final thresholding with rmsthresh/afactor="<< rmsthresh/afactor <<LogIO::POST;
    //LatticeExpr<Float> themask( iif( *(tempIm_ptr.get()) > rmsthresh/afactor, 1.0, 0.0 ));
    // previous 1/0 mask (regridded), max pix value should be <1.0, take arbitary cut off at 0.1
    LatticeExpr<Float> themask( iif( *(tempIm_ptr.get()) > 0.1, 1.0, 0.0 ));
    if (res.hasPixelMask()) {
      LatticeExpr<Bool>  pixmask(res.pixelMask()); 
      mask.copyData( (LatticeExpr<Float>)( iif((mask + themask) > 0.0 && pixmask, 1.0, 0.0  ) ) );
      os <<LogIO::DEBUG1 <<"add previous mask, pbmask and the new mask.."<<LogIO::POST;
    }
    else {
      //for debug
      /***
      PagedImage<Float> tempthemask(TiledShape(tempIm_ptr.get()->shape()), tempIm_ptr.get()->coordinates(),"tempthemask.Im");
      tempthemask.copyData(themask);
      ***/

      //os <<"Lattice themask is created..."<<LogIO::POST;
      //LatticeExpr<Float> themask( iif( tempconvim > rmsthresh/afactor, 1.0, 0.0 ));
      mask.copyData( (LatticeExpr<Float>)( iif((mask + themask) > 0.0, 1.0, 0.0  ) ) );
      os <<LogIO::DEBUG1 <<"add previous mask and the new mask.."<<LogIO::POST;
    }
  }

  void SDMaskHandler::autoMaskByThreshold2(ImageInterface<Float>& mask,
                                          const ImageInterface<Float>& res, 
                                          const ImageInterface<Float>& psf, 
                                          const Quantity& resolution,
                                          const Float& resbybeam,
                                          const Quantity& qthresh, 
                                          const Float& fracofpeak,
                                          const Record& stats, 
                                          const Float& sigma, 
                                          const Int nmask) 
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMaskByThreshold2",WHERE) );
    Array<Double> rms, max;
    Double rmsthresh, minrmsval, maxrmsval, minmaxval, maxmaxval;
    IPosition minrmspos, maxrmspos, minmaxpos, maxmaxpos;
    Int npix;

    //for debug set to True to save intermediate mask images on disk
    //Bool debug(False);

    // taking account for beam or input resolution
    TempImage<Float> tempmask(mask.shape(), mask.coordinates());
    IPosition shp = mask.shape();
    CoordinateSystem incsys = res.coordinates();
    Vector<Double> incVal = incsys.increment(); 
    Vector<String> incUnit = incsys.worldAxisUnits();
    Quantity qinc(incVal[0],incUnit[0]);
    if (resolution.get().getValue() ) {
      npix = Int(abs( resolution/(qinc.convert(resolution),qinc) ).getValue() );
      os << LogIO::NORMAL2 << "Use the input resolution:"<<resolution<<" for pruning "<< LogIO::POST;
      os << LogIO::DEBUG1 << "inc = "<<qinc.getValue(resolution.getUnit())<<LogIO::POST;
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
        if (resbybeam > 0.0 ) {
          npix = Int( Double(resbybeam) * abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
          os << LogIO::NORMAL2 << "Use "<< resbybeam <<" x  beam size(maj)="<< Double(resbybeam)*bmaj <<" for pruning."<< LogIO::POST;
        }
        else {
          npix = Int( abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
          os << LogIO::NORMAL2 << "Use a beam size(maj):"<<bmaj<<" for pruning."<< LogIO::POST;
        } 
      }
      else {
         throw(AipsError("No restoring beam(s) in the input image/psf or resolution is given"));
      }
    }
    os << LogIO::DEBUG1 << "Acutal bin size used: npix="<<npix<< LogIO::POST;

    // Determine threshold from input image stats
    stats.get(RecordFieldId("max"), max);
    stats.get(RecordFieldId("rms"), rms);
    minMax(minmaxval,maxmaxval,minmaxpos, maxmaxpos, max);
    minMax(minrmsval,maxrmsval,minrmspos, maxrmspos, rms); 
    os << LogIO::DEBUG1 <<"stats on the image: max="<<maxmaxval<<" rms="<<maxrmsval<<endl;
    if (fracofpeak) {
      rmsthresh = maxmaxval * fracofpeak; 
      os << LogIO::NORMAL <<"Threshold by fraction of the peak(="<<fracofpeak<<") * max: "<<rmsthresh<< LogIO::POST;
      os << LogIO::DEBUG1 <<"max at "<<maxmaxpos<<", dynamic range = "<<maxmaxval/rms(maxmaxpos) << LogIO::POST;
    }
    else if (sigma) {
      //cerr<<"minval="<<minval<<" maxval="<<maxval<<endl;
      rmsthresh = maxrmsval * sigma;
      os << LogIO::NORMAL <<"Threshold by sigma(="<<sigma<<")* rms (="<<maxrmsval<<") :"<<rmsthresh<< LogIO::POST;
      os << LogIO::DEBUG1 <<"max rms at "<<maxrmspos<<", dynamic range = "<<max(maxrmspos)/maxrmsval << LogIO::POST;
    }      
    else {
      rmsthresh = qthresh.getValue(Unit("Jy"));
      if ( rmsthresh==0.0 ) 
        { throw(AipsError("Threshold for automask is not set"));}
    }
    os << LogIO::NORMAL2 <<" thresh="<<rmsthresh<<LogIO::POST;

    SHARED_PTR<ImageInterface<Float> > tempIm_ptr = pruneRegions(res, rmsthresh, nmask, npix);
    LatticeExpr<Float> themask( iif( *(tempIm_ptr.get()) > rmsthresh, 1.0, 0.0 ));

    //for debug
    /***
    PagedImage<Float> tempthemask(TiledShape(tempIm_ptr.get()->shape()), tempIm_ptr.get()->coordinates(),"tempthemask.Im");
    tempthemask.copyData(themask);
    ***/
    if (res.hasPixelMask()) {
      LatticeExpr<Bool>  pixmask(res.pixelMask()); 
      mask.copyData( (LatticeExpr<Float>)( iif((mask + themask) > 0.0 && pixmask, 1.0, 0.0  ) ) );
      mask.clearCache();
      mask.unlock();
      mask.tempClose();
      os <<LogIO::DEBUG1 <<"Add previous mask, pbmask and the new mask.."<<LogIO::POST;
    }
    else {
      //os <<"Lattice themask is created..."<<LogIO::POST;
      //LatticeExpr<Float> themask( iif( tempconvim > rmsthresh/afactor, 1.0, 0.0 ));
      mask.copyData( (LatticeExpr<Float>)( iif((mask + themask) > 0.0, 1.0, 0.0  ) ) );
      os <<LogIO::DEBUG1 <<"Add previous mask and the new mask.."<<LogIO::POST;
    }
  }//end of makeAutoMaskByThreshold2

  // for implemtation of Amanda's algorithm
  void SDMaskHandler::autoMaskByMultiThreshold(ImageInterface<Float>& mask,
                                          const ImageInterface<Float>& res, 
                                          const ImageInterface<Float>& psf, 
                                          const Record& stats, 
                                          const Int nmask,
                                          const Float& sidelobeLevel,
                                          const Float& sidelobeThresholdFactor,
                                          const Float& noiseThresholdFactor,
                                          const Float& lowNoiseThresholdFactor,
                                          const Float& cutThreshold,
                                          const Float& smoothFactor,
                                          const Float& minBeamFrac) 
  {
    LogIO os( LogOrigin("SDMaskHandler","autoMaskByThreshold3",WHERE) );
    Array<Double> rmss, maxs, mads;
    Float resPeak, resRms;
    Array<Double> resRmss;
    Double minrmsval, maxrmsval, minmaxval, maxmaxval, minmadval, maxmadval;
    IPosition minrmspos, maxrmspos, minmaxpos, maxmaxpos, minmadpos, maxmadpos;
    Int npix, nxpix, nypix;

    //for debug set to True to save intermediate mask images on disk
    //Bool debug(False);

    TempImage<Float> tempmask(mask.shape(), mask.coordinates());
    // taking account for beam or input resolution
    IPosition shp = mask.shape();
    CoordinateSystem incsys = res.coordinates();
    Vector<Double> incVal = incsys.increment(); 
    Vector<String> incUnit = incsys.worldAxisUnits();
    Quantity qinc(incVal[0],incUnit[0]);
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
      Quantity bmin = beam.getMinor();
      //for pruning for now
      npix = Int( Double(minBeamFrac) * abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
      // make npix beam area in pixels? (and need to modify pruneRegions)...
      //beam in pixels
      nxpix = Int( Double(smoothFactor) * abs( (bmaj/(qinc.convert(bmaj),qinc)).get().getValue() ) );
      nypix = Int( Double(smoothFactor) * abs( (bmin/(qinc.convert(bmin),qinc)).get().getValue() ) );
    }
    else {
       throw(AipsError("No restoring beam(s) in the input image/psf"));
    }


    // Determine threshold from input image stats
    stats.get(RecordFieldId("max"), maxs);
    stats.get(RecordFieldId("rms"), rmss);
    stats.get(RecordFieldId("medabsdevmed"), mads);
    
    // only useful if single threshold value are used for all spectral planes... 
    minMax(minmaxval,maxmaxval,minmaxpos, maxmaxpos, maxs);
    minMax(minrmsval,maxrmsval,minrmspos, maxrmspos, rmss); 
    minMax(minmadval,maxmadval,minmadpos, maxmadpos, mads); 
    // use max of the list of peak values (for multiple channel plane)
    resPeak = maxmaxval;
    // use MAD and convert to rms 
    resRms = maxmadval * 1.4826; 
    resRmss = mads * 1.4826;

    //define mask threshold  
    Float sidelobeThreshold = sidelobeLevel * sidelobeThresholdFactor * resPeak;
    Float noiseThreshold = noiseThresholdFactor * resRms;
    Float lowNoiseThreshold = lowNoiseThresholdFactor * resRms; 
    Float maskThreshold = max(sidelobeThreshold, noiseThreshold);
    Float lowMaskThreshold = max(sidelobeThreshold, lowNoiseThreshold);

    String ThresholdType = (maskThreshold == sidelobeThreshold? "sidelobe": "noise");

    os << LogIO::NORMAL1 <<" Using "<<ThresholdType<<" threshold:"<<maskThreshold<<LogIO::POST;

    // branch out if just need to grow mask, obviously no 'grow' mask for the beginning of the first iteration
    // but how should detect if it is the first iteration... the original python prototype code has
    // a seperate createTHresholdMask... save a state in iterBot or get ncycle info from there?

    LatticeExpr<Float> themask; 
    Bool firstIter(false);
    // do thresholding via pruneRegions()
    if (minBeamFrac > 0.0 ) {
        // make temp mask image consist of the original pix value and below the threshold is set to 0 
        TempImage<Float> maskedRes(res.shape(), res.coordinates());
        maskedRes.copyData( (LatticeExpr<Float>)( iif(res > maskThreshold, res, 0.0)) );
        double tempthresh=0.0;
        SHARED_PTR<ImageInterface<Float> > tempIm_ptr = pruneRegions(maskedRes, tempthresh,  nmask, npix);
        themask = LatticeExpr<Float> ( iif( *(tempIm_ptr.get()) > maskThreshold, 1.0, 0.0 ));
    }
    else {
      themask = LatticeExpr<Float> ( iif( res > maskThreshold, 1.0, 0.0 ));
    }  
    tempmask.copyData(themask);
   
    //smooth
    SPIIF outmask = convolveMask(tempmask, nxpix, nypix);

    //clean up (appy cutThreshold to convolved mask image)
    LatticeExpr<Float> thenewmask( iif( *(outmask.get()) > cutThreshold, 1.0, 0.0 ));

    if (!firstIter) {
       //call growMask
       cerr<<"create constraint mask using lowNoiseThreshold: "<<lowMaskThreshold<<endl;
    }

    //for debug
    /***
    PagedImage<Float> tempthemask(TiledShape(tempIm_ptr.get()->shape()), tempIm_ptr.get()->coordinates(),"tempthemask.Im");
    tempthemask.copyData(themask);
    ***/
    if (res.hasPixelMask()) {
      LatticeExpr<Bool>  pixmask(res.pixelMask()); 
      mask.copyData( (LatticeExpr<Float>)( iif((mask + thenewmask) > 0.0 && pixmask, 1.0, 0.0  ) ) );
      mask.clearCache();
      mask.unlock();
      mask.tempClose();
      os <<LogIO::DEBUG1 <<"Add previous mask, pbmask and the new mask.."<<LogIO::POST;
    }
    else {
      //os <<"Lattice themask is created..."<<LogIO::POST;
      //LatticeExpr<Float> themask( iif( tempconvim > rmsthresh/afactor, 1.0, 0.0 ));
      mask.copyData( (LatticeExpr<Float>)( iif((mask + thenewmask) > 0.0, 1.0, 0.0  ) ) );
      os <<LogIO::DEBUG1 <<"Add previous mask and the new mask.."<<LogIO::POST;
    }
  }//end of makeAutoMaskByThreshold3

  SHARED_PTR<ImageInterface<Float> >  SDMaskHandler::makeMaskFromBinnedImage(const ImageInterface<Float>& image, 
                                                                             const Int nx, 
                                                                             const Int ny,  
                                                                             const Float& fracofpeak,
                                                                             const Float& sigma, 
                                                                             const Int nmask,
                                                                             const Bool autoadjust,
                                                                             Double thresh)
  {
    Bool debug(False);
    Bool firstrun(False);
    LogIO os( LogOrigin("SDMaskHandler","makeMaskfromBinnedImage",WHERE) );
    RebinImage<Float> tempRebinnedIm( image, IPosition(4,nx, ny,1,1) );
    // for debug
    if (debug) {
      PagedImage<Float> copyRebinnedIm(TiledShape(tempRebinnedIm.shape()), tempRebinnedIm.coordinates(), "binned.Im");
      copyRebinnedIm.copyData(tempRebinnedIm);
    }

    // modified threshold
    // original algortihm
    //thresh = 3.0*thresh / sqrt(npix);
    // modified by bin size only
    //thresh = thresh / sqrt(nx);

    //stats on the binned image (the info not used for mask criteria yet)
    Array<Double> rmses, maxes, mins;
    // vars to store min,max values of extrema and rms in all planes
    Double minRmsVal, maxRmsVal, minMaxVal, maxMaxVal, minMinVal, maxMinVal;
    IPosition minRmsPos, maxRmsPos, minMaxPos, maxMaxPos, minMinPos, maxMinPos;
    TempImage<Float>* tempImForStat = new TempImage<Float>(tempRebinnedIm.shape(), tempRebinnedIm.coordinates() );
    tempImForStat->copyData(tempRebinnedIm);
    SHARED_PTR<casacore::ImageInterface<float> > temprebin_ptr(tempImForStat);
    //os<<" temprebin_ptr.get()->hasPixelMask()="<<temprebin_ptr.get()->hasPixelMask()<<LogIO::POST;
    ImageStatsCalculator imcalc( temprebin_ptr, 0, "", False);
    Vector<Int> stataxes(2);
    stataxes[0] = 0;
    stataxes[1] = 1;
    imcalc.setAxes(stataxes);
    Record imstats = imcalc.statistics();
    imstats.get(RecordFieldId("rms"),rmses);
    imstats.get(RecordFieldId("max"),maxes);
    imstats.get(RecordFieldId("min"),mins);
    minMax(minRmsVal,maxRmsVal,minRmsPos, maxRmsPos,rmses);
    minMax(minMaxVal,maxMaxVal,minMaxPos, maxMaxPos,maxes);
    minMax(minMinVal,maxMinVal,minMinPos, maxMinPos,mins);
    

    //os << LogIO::NORMAL <<"Statistics on binned image: max="<<maxMaxVal<<" rms="<<maxRmsVal<<LogIO::POST;
    os << LogIO::NORMAL <<"Statistics on binned image: Peak (max)="<<maxMaxVal<<"@"<<maxMaxPos
                        <<"rms (max) ="<<maxRmsVal<<"@"<<maxRmsPos<<LogIO::POST;
    //os << LogIO::NORMAL <<"Statistics on binned image: min of Max="<<minMaxVal<<"@"<<minMaxPos<<LogIO::POST;
    //os << LogIO::NORMAL <<"Statistics on binned image: min of rms="<<minRmsVal<<"@"<<minRmsPos<<LogIO::POST;

    TempImage<Float>* tempMask = new TempImage<Float> (tempRebinnedIm.shape(), tempRebinnedIm.coordinates() );


    //Double dr =  maxMaxVal/rmses(maxMaxPos);
    
    // save only the first time
    if (itsMax==DBL_MAX) { 
        itsMax = maxMaxVal;
        firstrun = True;
    }
    else {
        firstrun = False;
    }

    Float adjFact = 0.0;

    //Float fracDiffMax = (itsMax - maxMaxVal)/itsMax;
    Float fracDiffRms = (itsRms - maxRmsVal)/itsRms;
    //os<<"fractional changes in max:"<<fracDiffMax<<" in rms:"<<fracDiffRms<<LogIO::POST;
    os<<LogIO::DEBUG1<<"fractional changes in rms from previous one:"<<fracDiffRms<<LogIO::POST;
    if (autoadjust) {
      //os <<"autoAdjust is on "<<LogIO::POST;
      // automatically adjust threshold for the initial round and when fractional rms change is
      // is less than 10%
      if (fracDiffRms < 0.1 ) {
        adjFact = (Float) Int(log(1.0/abs(fracDiffRms))-1.0);
      }
     // else if (dr < 10.0) {
     //   os<<LogIO::DEBUG1<<"dynamic range:max/rms = "<<dr<<LogIO::POST;
     //   adjFact = sigma!=0 && sigma <= 3.0? 2: 0;
     // }
      else if (firstrun) {
        adjFact = 3;
      }
    }
    if (fracofpeak) {
      thresh = fracofpeak * maxMaxVal;
      Double prevthresh = thresh;
      if (adjFact >0.0 ) {
        thresh = max((adjFact + 3) * maxRmsVal,thresh);
        if (firstrun) {
          // adjustment at 1st iteration cycle, if threshold is too big, make cutoff at 50% peak 
          if (thresh < itsMax) {
            if (prevthresh != thresh) {
              os << LogIO::NORMAL <<"first iteration automatically adjusted thresh="<<thresh<<"( "<<" * (adj fact.="<<adjFact+3<<") * rms )"<<LogIO::POST;
            }
          }
          else {
            thresh=prevthresh;
          }
        } 
        else {
          if (prevthresh != thresh) {
            os << LogIO::NORMAL <<"thresh="<<thresh<<" ( adj fact.="<<adjFact+3<<") * rms )"<<LogIO::POST;
          }
        }
      }
      // if sidelobe level is set and if it is larger than the current thresh use that value
      //thresh = max( itsSidelobeLevel*itsMax, thresh );
      if (adjFact != 0.0) {
      }
      else {
        os << LogIO::NORMAL <<"thresh="<<thresh<<" ( "<<fracofpeak<<"* max peak )"<<LogIO::POST;
      }
    } 
    else if (sigma) {
      thresh = (sigma + adjFact)* maxRmsVal;
      // if sidelobe level is set and if it is larger than the current thresh use that value
      //thresh = max( itsSidelobeLevel*itsMax, thresh);
      if (firstrun && adjFact != 0.0) {
        if (thresh < itsMax) { 
          os << LogIO::NORMAL <<"first iteration automatically adjusted thresh="<<thresh
             <<" ( "<<sigma<<"+adjustment factor:"<<adjFact<<")* rms )"<<LogIO::POST;
        }
        else {
          thresh = 0.5*itsMax;
          os << LogIO::NORMAL <<"first iteration automatically adjusted thresh="<<thresh
             <<" (0.5*peak )"<<LogIO::POST;
        }
      } 
      if (adjFact != 0.0) {
        os << LogIO::NORMAL <<"thresh="<<thresh<<" ( "<<sigma<<"+adjustment factor:"<<adjFact<<")* rms )"<<LogIO::POST;
      }
      else {
        os << LogIO::NORMAL <<"thresh="<<thresh<<" ( "<<sigma<<"* rms )"<<LogIO::POST;
      }
    }


    itsRms = maxRmsVal;

    if (thresh > maxMaxVal) {
      os << LogIO::WARN <<" The threshold value,"<<thresh<<" for making a mask is greater than max value in the image. No new mask will be added by automask."<< LogIO::POST;
      tempMask->set(0.0);
    }
    else {
    
      // apply threshold to rebinned image to generate a temp image mask
      // first run pruning by limiting n masks (npix=1 as it is already binned)
      SHARED_PTR<ImageInterface<Float> > dummyim = pruneRegions(tempRebinnedIm, thresh, nmask, 1);

      os << LogIO::DEBUG1<<" threshold applied ="<<thresh<<LogIO::POST;
      //cerr<<"dummyim shape="<<dummyim.get()->shape()<<endl;
      //cerr<<"temprebinned shape="<<tempRebinnedIm.shape()<<endl;
      //
      //LatticeExpr<Float> tempthresh( iif( abs(tempRebinnedIm) > thresh, 1.0, 0.0) );
      LatticeExpr<Float> tempthresh( iif( abs( *(dummyim.get()) ) > thresh, 1.0, 0.0) );
      //os << LogIO::DEBUG1<<" copying the threshold image....."<<LogIO::POST;
      tempMask->copyData(tempthresh);
    }
    return SHARED_PTR<ImageInterface<Float> >(tempMask);
  }

  SHARED_PTR<ImageInterface<Float> > SDMaskHandler::convolveMask(const ImageInterface<Float>& inmask, Int nxpix, Int nypix)
  {
    LogIO os( LogOrigin("SDMaskHandler","convolveMask",WHERE) );
    TempImage<Float>* tempIm = new TempImage<Float>(inmask.shape(), inmask.coordinates() );
    tempIm->copyData(inmask);
    SHARED_PTR<casacore::ImageInterface<float> > tempIm2_ptr(tempIm);
    //DEBUG will be removed 
    os << LogIO::DEBUG1<<"convolve with "<<nxpix<<" pix x "<<nypix<<" pix gaussian"<<LogIO::POST;

    Vector<Quantity> convbeam(3);
    convbeam[0] = Quantity(nxpix, "pix");
    convbeam[1] = Quantity(nypix, "pix");
    convbeam[2] = Quantity(0.0, "deg");
    Record dammyRec=Record();
    //String convimname("temp_convim");
    Image2DConvolver<Float> convolver(tempIm2_ptr, &dammyRec, String(""), String(""), True);
    convolver.setKernel("GAUSSIAN", convbeam[0], convbeam[1], convbeam[2]);
    convolver.setAxes(std::make_pair(0,1));
    convolver.setScale(Double(-1.0));
    convolver.setSuppressWarnings(True);
    auto outmask = convolver.convolve();
    return outmask;
  } 

  SHARED_PTR<casacore::ImageInterface<Float> >  SDMaskHandler::pruneRegions(const ImageInterface<Float>& image, Double& thresh, Int nmask, Int npix)
  {
    LogIO os( LogOrigin("SDMaskHnadler", "pruneRegions",WHERE) );
    Bool debug(False);

    IPosition fullimShape=image.shape();
    TempImage<Float>* fullIm = new TempImage<Float>(TiledShape(fullimShape, image.niceCursorShape()), image.coordinates());

    if (nmask==0 && npix==0 ) {
      //No-op
      os<<LogIO::DEBUG1<<"Skip pruning of mask regions"<<LogIO::POST;
      fullIm->copyData(image);
      return SHARED_PTR<ImageInterface<Float> >(fullIm);
    }  
    os <<LogIO::DEBUG1<< "pruneRegions with nmask="<<nmask<<", size="<<npix<<" is applied"<<LogIO::POST;
    
    IPosition shp = image.shape();
    //cerr<<"shp = "<<shp<<endl;
    IPosition blc(4,0);
    IPosition trc = shp-1; 
    Slicer sl(blc,trc,Slicer::endIsLast);
    AxesSpecifier aspec(False);
    // decomposer can only work for 2 and 3-dim images so need
    // some checks the case for stokes axis > 1
    // following works if stokes axis dim = 1
    SubImage<Float>* subIm = new SubImage<Float>(image, sl, aspec, True); 
    RegionManager regMan;
    CoordinateSystem cSys=subIm->coordinates(); 
    regMan.setcoordsys(cSys);
    //String strReg = "box[["+String::toString(blc[0])+"pix,"+String::toString(blc[1])+"pix], ["+String::toString(shp[0])+"pix,"+String::toString(shp[1])+"pix]]";
    //cerr<<"strReg="<<strReg<<endl;
    //RegionTextList CRTFList(cSys, strReg, shp);
    //Record regRec = CRTFList.regionAsRecord();
    //SHARED_PTR<casacore::SubImage<Float> > subIm = SubImageFactory<Float>::createSubImageRW(image, regRec, "", &os, aspec, False, False);

    //cerr <<" subIm.shape="<<subIm->shape()<<endl;
    IPosition subimShape=subIm->shape();
    uInt ndim = subimShape.nelements();
    //SHARED_PTR<casacore::ImageInterface<float> > tempIm_ptr(subIm);
    TempImage<Float>* tempIm = new TempImage<Float> (TiledShape(subIm->shape(), subIm->niceCursorShape()), subIm->coordinates() );
    // to search for both positive and negative components
    tempIm->copyData(LatticeExpr<Float> (abs(*subIm)));
    os << LogIO::NORMAL2 <<"Finding regions by ImageDecomposer..."<<LogIO::POST;
    //use ImageDecomposer
    Matrix<Quantity> blctrcs;
    ImageDecomposer<Float> id(*tempIm);
    id.setDeblend(True);
    os << LogIO::DEBUG1<< "Deblend threshold="<<thresh<<LogIO::POST;
    id.setDeblendOptions(thresh, 3, 1, 2); //nContour=3
    id.setFit(False);
    os << LogIO::DEBUG1<<"Now calling decomposeImage()..."<<LogIO::POST;
    id.decomposeImage();
    if (debug) 
      id.printComponents();
    uInt nRegion = id.numRegions();
    os << "Found " << nRegion <<" regions"<<LogIO::POST;
    Block<IPosition> blcs(nRegion);
    Block<IPosition> trcs(nRegion);
    id.boundRegions(blcs, trcs);
    //os << "Get comp list.."<<LogIO::POST;
    Matrix<Float> clmat=id.componentList();
    //os << "Get peaks.."<<LogIO::POST;
    Vector<Float> peaks = clmat.column(0);
    //cerr<<"peaks="<<peaks<<endl;
    os << LogIO::DEBUG1<< "Sorting by peak fluxes..."<<LogIO::POST;
    // sort 
    Vector<uInt> sortedindx;
    Sort sorter;
    //cerr<<"Setup sortKey..."<<endl;
    sorter.sortKey(peaks.data(),TpFloat,0, Sort::Descending);
    //cerr<<"do sort..."<<endl;
    sorter.sort(sortedindx, peaks.nelements());
    //os << "Sorting py peak flux DONE..."<<LogIO::POST;
    os<< LogIO::DEBUG1<<"sortedindx="<<sortedindx<<LogIO::POST;    
    // FOR DEBUGGING
    //for (uInt j = 0; j < blcs.nelements(); j++) {
    //  os<<" j="<<j<<" blcs="<<blcs[j]<<" trcs="<<trcs[j]<<LogIO::POST;
    //}
    Vector<Int> absRel(ndim, RegionType::Abs);
    PtrBlock<const WCRegion *> wbox;
    uInt iSubComp=0;
    uInt removeByNMask=0;
    uInt removeBySize=0;
    for (uInt icomp=0; icomp < sortedindx.nelements(); icomp++) {
      Bool removeit(False);
      Vector<Quantum<Double> > qblc(ndim);
      Vector<Quantum<Double> > qtrc(ndim);      
      Vector<Double> wblc(ndim);
      Vector<Double> wtrc(ndim);
      Vector<Double> pblc(ndim);
      Vector<Double> ptrc(ndim);
      // pixel blcs and trcs
      for (uInt i=0; i < ndim; i++) {
        pblc[i] = (Double) blcs[sortedindx[icomp]][i]; 
        ptrc[i] = (Double) trcs[sortedindx[icomp]][i];
      }
      // get blcs and trcs in world coord.
      cSys.toWorld(wblc,pblc);
      cSys.toWorld(wtrc,ptrc);
      for (uInt i=0; i < ndim; i++) {
        qblc[i] = Quantum<Double>(wblc[i], cSys.worldAxisUnits()(cSys.pixelAxisToWorldAxis(i)));
        qtrc[i] = Quantum<Double>(wtrc[i], cSys.worldAxisUnits()(cSys.pixelAxisToWorldAxis(i)));
      }

      if (npix > 0) {
        //os<<"pruning regions by size < "<<npix<<LogIO::POST;
        Int xboxdim = ptrc[0] - pblc[0];
        Int yboxdim = ptrc[1] - pblc[1];
        //cerr<<"xboxdim = "<<xboxdim<<" yboxdim="<<yboxdim<<endl;
        if (( xboxdim < npix || yboxdim < npix ) && xboxdim*yboxdim < npix*npix ) {
          removeit = True;
          removeBySize++;
        }
      }
      if (nmask > 0 && icomp >= (uInt)nmask ) {
        removeit=True; 
        removeByNMask++;
      }
      
      if (removeit) {
        wbox.resize(iSubComp+1);
        wbox[iSubComp]= new WCBox (qblc, qtrc, cSys, absRel);
        iSubComp++;
        os << LogIO::DEBUG1<<"*** Removed region: "<<icomp<<" pblc="<<pblc<<" ptrc="<<ptrc<<LogIO::POST;
      }
      else {
        os << LogIO::DEBUG1<<"Saved region: "<<icomp<<" pblc="<<pblc<<" ptrc="<<ptrc<<LogIO::POST;
      }
    } // for icomp  

    //cerr<<"iSubComp="<<iSubComp<<endl;
    //cerr<<"wbox.nelements="<<wbox.nelements()<<endl;
    if (iSubComp>0) {
      ImageRegion* boxImageRegion=regMan.doUnion(wbox);
      //cerr<<"regionToMask ..."<<endl;
      tempIm->copyData(*subIm);
      regionToMask(*tempIm,*boxImageRegion, Float(0.0)); 
      //cerr<<"Done regionToMask..."<<endl;
      os <<"pruneRegions removed "<<iSubComp<<" regions from the mask image"<<LogIO::POST;
      os <<"  (reasons: "<< removeBySize<<" by size(maskresolution) "<<", "<<removeByNMask<<" by nmask)" <<LogIO::POST;
      for (uInt k=0; k < wbox.nelements(); k++) {
        delete wbox[k];
      }
    }
    else {
      os <<"No regions are removed by pruning" << LogIO::POST;
    }
    // Debug
    if (debug) {
      PagedImage<Float> debugPrunedIm(tempIm->shape(),tempIm->coordinates(),"prunedSub.Im");
      debugPrunedIm.copyData(*tempIm);
    }
    //
    // Inserting pruned result image to the input image
    Array<Float> subimData;
    //IPosition fullimShape=image.shape();
    //TempImage<Float>* fullIm = new TempImage<Float>(TiledShape(fullimShape, image.niceCursorShape()), image.coordinates());
    fullIm->set(0);
    IPosition start(fullimShape.nelements(),0);
    IPosition stride(fullimShape.nelements(),1);
    if (ndim ==3) {
      IPosition substart(3,0);
      IPosition subshape(3,subimShape(0),subimShape(1),1);
      IPosition substride(3,1,1,1);
      uInt nchan=subimShape(2);
      //cerr<<"shape tempIm ="<<tempIm->shape()<<endl;
      //cerr<<"shape fullIm ="<<fullIm->shape()<<endl;
      for (uInt ich=0; ich < nchan; ich++) {
        substart(2) = ich;
        //tempIm->getSlice(subimData,Slicer(substart,subend),True);
        tempIm->getSlice(subimData,substart,subshape,substride,True);
        start(3) = ich;
        fullIm->putSlice(subimData,start,stride);  
      }
    }
    else if (ndim==2) {
      subimData = tempIm->get();
      //cerr<<"subimData shape="<<subimData.shape()<<endl;
      //cerr<<"shape tempIm ="<<tempIm->shape()<<endl;
      fullIm->putSlice(subimData,start,stride);
      //cerr<<"shape fullIm ="<<fullIm->shape()<<endl;
    }

    return SHARED_PTR<ImageInterface<Float> >(fullIm);
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

  //apply per channel plane threshold
  void SDMaskHandler::maskWithPerPlaneThreshold(ImageInterface<Float>& image, ImageInterface<Float>& mask, Vector<Float>& thresholds) 
  {
    IPosition imshape = image.shape();
    CoordinateSystem imcsys = image.coordinates();
    Vector<Int> diraxes = CoordinateUtil::findDirectionAxes(imcsys);
    Int specaxis = CoordinateUtil::findSpectralAxis(imcsys);
    uInt nchan = imshape (specaxis); 
    if (nchan != thresholds.nelements()) {
      throw(AipsError("Mismatch in the number of threshold values and the number of chan planes."));
    }
    for (uInt ich=0; ich < nchan; ich++) {
      IPosition start(4, 0, 0, 0,ich);
      IPosition length(4, imshape(diraxes(0)),imshape(diraxes(1)),imshape(2),1);
      Slicer sl(start, length);

      // make a subImage for  a channel slice      
      SubImage<Float> chanImage(image, sl, true);
      TempImage<Float>* tempChanImage = new TempImage<Float> (chanImage.shape(), chanImage.coordinates() );
      Array<Float> chanImageArr;
      LatticeExpr<Float> chanMask(iif(chanImage > thresholds(ich),1.0, 0.0)); 
      tempChanImage->copyData(chanMask);
      tempChanImage->getSlice(chanImageArr, IPosition(4,0), chanImage.shape(),IPosition(4,1,1,1,1));
      mask.putSlice(chanImageArr,start,IPosition(4,1,1,1,1)); 
    } // loop over chans
  }

  void SDMaskHandler::binaryDilationCore(Lattice<Float>& inlattice,
                      Array<Float>& structure,
                      Lattice<Float>& mask,
                      Lattice<Float>& outlattice)
  {
    LogIO os( LogOrigin("SDMaskHandler", "binaryDilation", WHERE) );
    //
    //IPosition cursorShape=inlattice.niceCursorShape();
    IPosition inshape = inlattice.shape();
    Int nx = inshape(0);
    Int ny = inshape(0);
    // assume here 3x3 structure elements (connectivity of either 1 or 2)
    IPosition seshape = structure.shape();
    Int se_nx =seshape(0); 
    Int se_ny =seshape(1); 

    if (mask.shape()!=inshape) {
      throw(AipsError("Incompartible mask shape. Need to be the same as the input image."));
    } 
    // assume the origin of structure element is the center  se(1,1)
    IPosition cursorShape(4, nx, ny, 1, 1);
    IPosition axisPath(4, 0, 1, 3, 2);
    //cerr<<"cursorShape="<<cursorShape<<endl;
    //cerr<<"structure="<<structure<<endl;
    LatticeStepper tls(inlattice.shape(), cursorShape, axisPath); 
    RO_LatticeIterator<Float> li(inlattice, tls);
    RO_LatticeIterator<Float> mi(mask, tls);
    LatticeIterator<Float> oli(outlattice,tls);
    for (li.reset(), oli.reset(); !li.atEnd(); li++, oli++) {
      Array<Float> planeImage(li.cursor());
      //cerr<<"planeImage.shape()="<<planeImage.shape()<<endl;
      for (Int i=0; i < nx; i++) {
        for (Int j=0; j < ny; j++) {
          if (planeImage(IPosition(4,i,j,0,0))==1.0) {
            //cerr<<"if planeImage ==1 i="<<i<<" j="<<j<<endl;
            // Set the value for se(1,1)
            planeImage(IPosition(4,i,j,0,0)) = 2.0;
            for (Int ise=0; ise < se_nx; ise++) {
              for (Int jse = 0; jse < se_ny; jse++) {
                Int relx_se = ise - 1;
                Int rely_se = jse - 1;
                if (structure(IPosition(2,ise,jse)) && !(ise==1.0 && jse==1.0)) {
                  //cerr<<"structure("<<ise<<","<<jse<<")="<<structure(IPosition(2,ise,jse))<<endl; 
                  if ((i + relx_se) >= 0 && (i + relx_se) < nx &&
                      (j + rely_se) >= 0 && (j + rely_se) < ny) {
                    if (planeImage(IPosition(4,i+relx_se,j+rely_se,0,0))==0 ) {
                      // set to 2.0 to make distinction with the orignal 1.0 pixels
                      planeImage(IPosition(4, i+relx_se, j+rely_se,0,0))=2.0;
                      //cerr<<" i+relx_se="<<i+relx_se<<" j+rely_se="<<j+rely_se<<endl;
                    }                   
                  }
                } // if(se(ise,jse)
              } // if planeImage(i,j)=1.0
            } // S.E. col loop
          } // S.E. row loop
        } // image col loop
      } //inage row loop
      for (Int ii=0; ii < nx; ii++) {
        for (Int jj=0; jj < ny; jj++) {
          if (planeImage(IPosition(4,ii,jj,0,0))==2) 
            planeImage(IPosition(4,ii,jj,0,0))=1;
        }
      }
      oli.rwCursor() = planeImage;
    }
  }

  /***
  void SDMaskHandler::binaryDilation(Lattice<Float>& inlattice,
                      Array<Float>& structure,
                      Int niteration,
                      Lattice<Float>& mask,
                      Lattice<Float>& outlattice)
  ***/
  void SDMaskHandler::binaryDilation(ImageInterface<Float>& inImage,
                      Array<Float>& structure,
                      Int niteration,
                      ImageInterface<Float>& mask,
                      ImageInterface<Float>& outImage)
  {

      binaryDilationCore(inImage,structure,mask,outImage);
      Int iter = 1;
      while (iter < niteration) {
        ArrayLattice<Float> templattice(inImage.shape());
        templattice.copyData(outImage);
        binaryDilationCore(templattice,structure,mask,outImage); 
        iter++;
      }
  }

 
  void SDMaskHandler::autoMaskWithinPB(SHARED_PTR<SIImageStore> imstore, 
                                       const String& alg, 
                                       const String& threshold, 
                                       const Float& fracofpeak, 
                                       const String& resolution,
                                       const Float& resbybeam, 
                                       const Int nmask,
                                       const Bool autoadjust,
                                       Float pblimit)
  { 
    LogIO os( LogOrigin("SDMaskHandler","autoMaskWithinPB",WHERE) );
    //Bool debug(False);

    os <<LogIO::DEBUG1<<"Calling autoMaskWithinPB .."<<LogIO::POST;
    // changed to do automask ater pb mask is generated so automask do stats within pb mask
    autoMask( imstore, alg, threshold, fracofpeak, resolution, resbybeam, nmask, autoadjust, pblimit);

    if( imstore->hasPB() )
      {
        LatticeExpr<Float> themask( iif( (*(imstore->pb())) > pblimit , (*(imstore->mask())), 0.0 ) );
	imstore->mask()->copyData( themask );

        /**** 
        // apply pb mask as pixel mask for now. This will be converted to 1/0 image later
        LatticeExpr<Bool> mask( iif(*(imstore->pb()) > pblimit, True, False));
        os <<"calling MakeMask, hasPixelMask? "<<imstore->mask()->hasPixelMask()<<LogIO::POST;
        os <<"calling MakeMask, hasRegion mask0? "<<imstore->mask()->hasRegion("mask0")<<LogIO::POST;
        os <<"defaultMask "<<imstore->mask()->getDefaultMask()<<LogIO::POST;
        //ImageRegion outreg=imstore->mask()->makeMask("mask0", False, True);
        ImageRegion outreg=imstore.get()->mask()->makeMask("mask0", True, True);
        LCRegion& outmask=outreg.asMask();
        outmask.copyData(mask);
        os <<"Before defineRegion"<<LogIO::POST;
        imstore.get()->mask()->defineRegion("mask0", outreg, RegionHandler::Masks, True);
        os <<"setDefMask"<<LogIO::POST;
        imstore.get()->mask()->setDefaultMask("mask0");
        imstore.get()->releaseImage(imstore.get()->mask());
        if (debug) {
	  cerr<<"Make a copy"<<endl;
          PagedImage<Float> temppb(imstore->mask()->shape(), imstore->mask()->coordinates(),"tempPB.Im");
          temppb.copyData(*(imstore->mask()));
          temppb.defineRegion("mask0", outreg, RegionHandler::Masks, True);
          temppb.setDefaultMask("mask0");
        }
        ***/
        
      }
    //autoMask( imstore, alg, threshold, fracofpeak, resolution, resbybeam, nmask);

    // else... same options as makePBMask (put it into a helper function)
  }// end of autoMaskWithinPB

} //# NAMESPACE CASA - END

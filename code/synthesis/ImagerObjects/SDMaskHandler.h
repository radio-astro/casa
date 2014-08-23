//# SDMaskHandler.h: Definition for SDMaskHandler
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

#ifndef SYNTHESIS_SDMASKHANDLER_H
#define SYNTHESIS_SDMASKHANDLER_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include<synthesis/ImagerObjects/SIImageStore.h>
#include<synthesis/ImagerObjects/SIImageStoreMultiTerm.h>
#include <synthesis/ImagerObjects/InteractiveMasking.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SDMaskHandler 
{
public:

  // Empty constructor
  SDMaskHandler();
  ~SDMaskHandler();

  void resetMask(CountedPtr<SIImageStore> imstore);

  // Collection of methods translate mask description (text, record, threshold, etc) to
  // mask image where the region(s) of interest are represented by the value (default = 1.0)
  // and the rest of the image is set to 0.0.
  //
  //void makeMask();
  // Create a mask image with maskName from tempim with a threshold applied to the pixel intensity
  CountedPtr<ImageInterface<Float> > makeMask(const String& maskName, const Quantity threshold, ImageInterface<Float>& tempim);
  
  // Make a mask image from Record, Matrix of (nboxes,4) where each row contains [blc_x,blc_y, trc_x,trc_y], 
  // and Matrix of (ncircles, 3) with the specified 'value'. Each row of circles are radius, x_center, y_center in pixels.
  // blctrcs and circles are applied to all the spectral and stokes while regions specified in record can specify selections 
  // in spectral and stokes axes.
  static Bool regionToImageMask(const String& maskimage, Record* regionRec, Matrix<Quantity> & blctrcs,
                    Matrix<Float>& circles, const Float& value=1.0);
  
  // Convert boxes defined with blcs and trcs to ImageRegion
  static void boxRegionToImageRegion(const ImageInterface<Float>& maskImage, const Matrix<Quantity>& blctrcs, ImageRegion*& boxImageRegions);
  // Convert circles (in pixels)  to ImageRegion
  static void circleRegionToImageRegion(const ImageInterface<Float>& maskImage, const Matrix<Float>& circles, ImageRegion*& circleImageRegions);
  // Convert region defined by record to Imageregion
  static void recordRegionToImageRegion(Record* imageRegRec, ImageRegion*& imageRegion );
  // Convert ImageRegion to a mask image with the value
  static Bool regionToMask(ImageInterface<Float>& maskImage, ImageRegion& imageregion, const Float& value);
  // Read CRTF format text or the text file contains CRTF definitions and convert it to a ImageRegion
  static void regionTextToImageRegion(const String& text, const ImageInterface<Float>& regionImage, ImageRegion*& imageRegion);

  // merge mask images to outimage
  void copyAllMasks(const Vector< ImageInterface<Float> >& inimage, ImageInterface<Float>& outimage);
  // copy and regrid a mask image to outimage
  void copyMask(const ImageInterface<Float>& inimage, ImageInterface<Float>& outimage);
  // expand smaller chan mask image to larger one 
  void expandMask(const ImageInterface<Float>& smallchanmask, ImageInterface<Float>& outimage);
  // convert internal mask to imageRegion
  void InMaskToImageRegion(const ImageInterface<Float>& inimage );

  int makeInteractiveMask(CountedPtr<SIImageStore>& imstore,
			  Int& niter, Int& ncycles, 
			  String& threshold);

  // Return a reference to an imageinterface for the mask.
  void makeAutoMask(CountedPtr<SIImageStore> imstore);

  void makePBMask(CountedPtr<SIImageStore> imstore, Float weightlimit);

  // check if input image is a mask image with 0 or a value (if normalize=true, 1)
  Bool checkMaskImage(ImageInterface<Float>& maskiamge, Bool normalize=True);

// 
  static Bool cloneImShape(const ImageInterface<Float>& inImage, const String& outImageName);

protected:
  InteractiveMasking *interactiveMasker_p;
};

} //# NAMESPACE CASA - END


#endif



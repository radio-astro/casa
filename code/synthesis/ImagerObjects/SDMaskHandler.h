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

  void resetMask(SHARED_PTR<SIImageStore> imstore);

  void fillMask(SHARED_PTR<SIImageStore> imstore, casacore::Vector<casacore::String> maskStrings);
  void fillMask(SHARED_PTR<SIImageStore> imstore, casacore::String maskString);

  // Collection of methods translate mask description (text, record, threshold, etc) to
  // mask image where the region(s) of interest are represented by the value (default = 1.0)
  // and the rest of the image is set to 0.0.
  //
  //void makeMask();
  // Create a mask image with maskName from tempim with a threshold applied to the pixel intensity
  SHARED_PTR<casacore::ImageInterface<casacore::Float> > makeMask(const casacore::String& maskName, const casacore::Quantity threshold, casacore::ImageInterface<casacore::Float>& tempim);
  
  // Make a mask image from casacore::Record, casacore::Matrix of (nboxes,4) where each row contains [blc_x,blc_y, trc_x,trc_y], 
  // and casacore::Matrix of (ncircles, 3) with the specified 'value'. Each row of circles are radius, x_center, y_center in pixels.
  // blctrcs and circles are applied to all the spectral and stokes while regions specified in record can specify selections 
  // in spectral and stokes axes.
  //static casacore::Bool regionToImageMask(const casacore::String& maskimage, casacore::Record* regionRec, casacore::Matrix<casacore::Quantity> & blctrcs,
  static casacore::Bool regionToImageMask(casacore::ImageInterface<casacore::Float>&  maskImage, casacore::Record* regionRec, casacore::Matrix<casacore::Quantity> & blctrcs,
                    casacore::Matrix<casacore::Float>& circles, const casacore::Float& value=1.0);
  
  // Convert boxes defined with blcs and trcs to ImageRegion
  static void boxRegionToImageRegion(const casacore::ImageInterface<casacore::Float>& maskImage, const casacore::Matrix<casacore::Quantity>& blctrcs, casacore::ImageRegion*& boxImageRegions);
  // Convert circles (in pixels)  to ImageRegion
  static void circleRegionToImageRegion(const casacore::ImageInterface<casacore::Float>& maskImage, const casacore::Matrix<casacore::Float>& circles, casacore::ImageRegion*& circleImageRegions);
  // Convert region defined by record to Imageregion
  static void recordRegionToImageRegion(casacore::Record* imageRegRec, casacore::ImageRegion*& imageRegion );
  // Convert casacore::ImageRegion to a mask image with the value
  static casacore::Bool regionToMask(casacore::ImageInterface<casacore::Float>& maskImage, casacore::ImageRegion& imageregion, const casacore::Float& value);
  // Read CRTF format text or the text file contains CRTF definitions and convert it to a ImageRegion
  static void regionTextToImageRegion(const casacore::String& text, const casacore::ImageInterface<casacore::Float>& regionImage, casacore::ImageRegion*& imageRegion);

  // merge mask images to outimage
  void copyAllMasks(const casacore::Vector< SHARED_PTR<casacore::ImageInterface<casacore::Float> > > inImageMasks, casacore::ImageInterface<casacore::Float>& outImageMask);
  // copy and regrid a mask image to outimage
  void copyMask(const casacore::ImageInterface<casacore::Float>& inimage, casacore::ImageInterface<casacore::Float>& outimage);
  // expand smaller chan mask image to larger one. - currently only works for a single channel (continuum) input mask 
  void expandMask(const casacore::ImageInterface<casacore::Float>& inImageMask, casacore::ImageInterface<casacore::Float>& outImageMask);
  // convert internal mask to imageRegion
  void InMaskToImageRegion(const casacore::ImageInterface<casacore::Float>& inimage );

  int makeInteractiveMask(SHARED_PTR<SIImageStore>& imstore,
			  casacore::Int& niter, casacore::Int& cycleniter,
			  casacore::String& threshold, casacore::String& cyclethreshold);

  // Return a reference to an imageinterface for the mask.
  void makeAutoMask(SHARED_PTR<SIImageStore> imstore);
  // Top level autoMask interface...
  void autoMask(SHARED_PTR<SIImageStore> imstore, 
                const casacore::String& alg="",
                const casacore::String& threshold="",
                const casacore::Float& fracpeak=0.0,
                const casacore::String& resolution="",
                const casacore::Float& resbybeam=0.0,
                const casacore::Int nmask=0, 
                const casacore::Bool autoadjust=false,
                casacore::Float pblimit=0.0);
  // automask algorithms...  
  void autoMaskByThreshold (casacore::ImageInterface<casacore::Float>& mask,
                           const casacore::ImageInterface<casacore::Float>& res, 
                           const casacore::ImageInterface<casacore::Float>& psf, 
                           const casacore::Quantity& resolution, 
                           const casacore::Float& resbybeam, 
                           const casacore::Quantity& qthreshold, 
                           const casacore::Float& fracofpeak, 
                           const casacore::Record& theStatsffff,
                           const casacore::Float& sigma=3.0,
                           const casacore::Int nmask=0, 
                           const casacore::Bool autoadjust=casacore::False);

  // no binning version
  void autoMaskByThreshold2 (casacore::ImageInterface<float>& mask,
                           const casacore::ImageInterface<float>& res,
                           const casacore::ImageInterface<float>& psf,
                           const casacore::Quantity& resolution,
                           const casacore::Float& resbybeam,
                           const casacore::Quantity& qthreshold,
                           const casacore::Float& fracofpeak,
                           const casacore::Record& theStats,
                           const casacore::Float& sigma=3.0,
                           const casacore::Int nmask=0);

  // implementation of Amanda's autoboxing algorithm
  void autoMaskByThreshold3(casacore::ImageInterface<float>& mask,
                                          const casacore::ImageInterface<float>& res, 
                                          const casacore::ImageInterface<float>& psf, 
                                          const casacore::Record& stats, 
                                          const casacore::Float& sigma=3.0, 
                                          const casacore::Int nmask=0,
                                          const casacore::Float& sidelobeThreshold=3.0,
                                          const casacore::Float& cutThreshold=0.01,
                                          const casacore::Float& smoothFactor=1.0,
                                          const casacore::Float& minBeamFrac=-1); 
                           
  casacore::Record calcImageStatistics(casacore::ImageInterface<float>& res, 
                                       casacore::ImageInterface<float>& prevmask, 
                                       const casacore::Bool robust);

  SHARED_PTR<casacore::ImageInterface<float> > makeMaskFromBinnedImage (
                               const casacore::ImageInterface<float>& image,
                               const casacore::Int nx,
                               const casacore::Int ny,
                               const casacore::Float& fracofpeak,
                               const casacore::Float& sigma,
                               const casacore::Int nmask,
                               const casacore::Bool autoadjust,
                               casacore::Double thresh=0.0);

  SHARED_PTR<casacore::ImageInterface<float> > convolveMask(const casacore::ImageInterface<float>& inmask,
                                                  int nxpix, int nypix);

  SHARED_PTR<casacore::ImageInterface<float> >  pruneRegions(const casacore::ImageInterface<float>& image,
                                                   double& thresh,
                                                   int nmask=0,
                                                   int npix=0);

  void makePBMask(SHARED_PTR<SIImageStore> imstore, casacore::Float pblimit=0.1);
  void autoMaskWithinPB(SHARED_PTR<SIImageStore> imstore, 
                        const casacore::String& alg="",
                        const casacore::String& threshold="",
                        const casacore::Float& fracpeak=0.0,
                        const casacore::String& resolution="",
                        const casacore::Float& resbybeam=0.0,
                        const casacore::Int nmask=0,
                        const casacore::Bool autoadjust=false,
                        casacore::Float pblimit=0.1);

  // check if input image is a mask image with 0 or a value (if normalize=true, 1)
  casacore::Bool checkMaskImage(casacore::ImageInterface<casacore::Float>& maskiamge, casacore::Bool normalize=true);

// 
  static casacore::Bool cloneImShape(const casacore::ImageInterface<casacore::Float>& inImage, const casacore::String& outImageName);

protected:
  InteractiveMasking *interactiveMasker_p;

private:
  double itsRms;
  double itsMax;
  float itsSidelobeLevel;
};



} //# NAMESPACE CASA - END


#endif



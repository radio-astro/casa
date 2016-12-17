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

  void fillMask(SHARED_PTR<SIImageStore> imstore, Vector<String> maskStrings);
  void fillMask(SHARED_PTR<SIImageStore> imstore, String maskString);

  // Collection of methods translate mask description (text, record, threshold, etc) to
  // mask image where the region(s) of interest are represented by the value (default = 1.0)
  // and the rest of the image is set to 0.0.
  //
  //void makeMask();
  // Create a mask image with maskName from tempim with a threshold applied to the pixel intensity
  SHARED_PTR<ImageInterface<Float> > makeMask(const String& maskName, const Quantity threshold, ImageInterface<Float>& tempim);
  
  // Make a mask image from Record, Matrix of (nboxes,4) where each row contains [blc_x,blc_y, trc_x,trc_y], 
  // and Matrix of (ncircles, 3) with the specified 'value'. Each row of circles are radius, x_center, y_center in pixels.
  // blctrcs and circles are applied to all the spectral and stokes while regions specified in record can specify selections 
  // in spectral and stokes axes.
  //static Bool regionToImageMask(const String& maskimage, Record* regionRec, Matrix<Quantity> & blctrcs,
  static Bool regionToImageMask(ImageInterface<Float>&  maskImage, Record* regionRec, Matrix<Quantity> & blctrcs,
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
  void copyAllMasks(const Vector< SHARED_PTR<ImageInterface<Float> > > inImageMasks, ImageInterface<Float>& outImageMask);
  // copy and regrid a mask image to outimage
  void copyMask(const ImageInterface<Float>& inimage, ImageInterface<Float>& outimage);
  // expand smaller chan mask image to larger one. - currently only works for a single channel (continuum) input mask 
  void expandMask(const ImageInterface<Float>& inImageMask, ImageInterface<Float>& outImageMask);
  // convert internal mask to imageRegion
  void InMaskToImageRegion(const ImageInterface<Float>& inimage );

  int makeInteractiveMask(SHARED_PTR<SIImageStore>& imstore,
			  Int& niter, Int& cycleniter,
			  String& threshold, String& cyclethreshold);

  // Return a reference to an imageinterface for the mask.
  void makeAutoMask(SHARED_PTR<SIImageStore> imstore);
  // Top level autoMask interface...
  void autoMask(SHARED_PTR<SIImageStore> imstore, 
                const Int iterdone,
                const String& alg="",
                const String& threshold="",
                const Float& fracpeak=0.0,
                const String& resolution="",
                const Float& resbybeam=0.0,
                const Int nmask=0, 
                const Bool autoadjust=False,
                const Float& sidelobethreshold=0.0,
                const Float& noisethreshold=0.0,
                const Float& lownoisethreshold=0.0,
                const Float& cutthreshold=0.0,
                const Float& smoothfactor=0.0,
                const Float& minbeamfrac=0.0,
                Float pblimit=0.0);
  // automask by threshold with binning before applying it
  void autoMaskByThreshold (ImageInterface<Float>& mask,
                           const ImageInterface<Float>& res, 
                           const ImageInterface<Float>& psf, 
                           const Quantity& resolution, 
                           const Float& resbybeam, 
                           const Quantity& qthreshold, 
                           const Float& fracofpeak, 
                           const Record& theStats,
                           const Float& sigma=3.0,
                           const Int nmask=0, 
                           const Bool autoadjust=False);

  // automask by threshold : no binning version
  void autoMaskByThreshold2 (ImageInterface<Float>& mask,
                           const ImageInterface<Float>& res, 
                           const ImageInterface<Float>& psf, 
                           const Quantity& resolution, 
                           const Float& resbybeam, 
                           const Quantity& qthreshold, 
                           const Float& fracofpeak, 
                           const Record& theStats,
                           const Float& sigma=3.0,
                           const Int nmask=0);

  // implementation of Amanda's automasking algorithm using multiple thresholds
  void autoMaskByMultiThreshold(ImageInterface<Float>& mask,
                                          const ImageInterface<Float>& res,
                                          const ImageInterface<Float>& psf,
                                          const Record& stats,
                                          const Int iterdone,
                                          const Float& sidelobeLevel=0.0,
                                          const Float& sidelobeThresholdFactor=3.0,
                                          const Float& noiseThresholdFactor=3.0,
                                          const Float& lowNoiseThresholdFactor=2.0,
                                          const Float& cutThreshold=0.01,
                                          const Float& smoothFactor=1.0,
                                          const Float& minBeamFrac=-1.0);
                           
  // Calculate statistics on a residual image with additional region and LEL mask specificaations
  Record calcImageStatistics(ImageInterface<Float>& res,
                                       ImageInterface<Float>& prevmask,
                                       String& lelmask,
                                       Record* regionPtr,
                                       const Bool robust);

  SHARED_PTR<ImageInterface<Float> > makeMaskFromBinnedImage (
                               const ImageInterface<Float>& image, 
                               const Int nx, 
                               const Int ny,
                               const Float& fracofpeak,
                               const Float& sigma, 
                               const Int nmask,
                               const Bool autoadjust,
                               Double thresh=0.0);

  // Convolve mask image with nx pixel by ny pixel
  SHARED_PTR<ImageInterface<Float> > convolveMask(const ImageInterface<Float>& inmask, 
                                                  Int nxpix, Int nypix);

  // Convolve mask image by a gaussian
  SHARED_PTR<ImageInterface<Float> > convolveMask(const ImageInterface<Float>& inmask,
                                                  const GaussianBeam& beam);
  //
  // Prune the mask regions found
  SHARED_PTR<ImageInterface<Float> >  pruneRegions(const ImageInterface<Float>& image, 
                                                   Double& thresh, 
                                                   Int nmask=0, 
                                                   Int npix=0);

  // Prune the mask regions per spectral plane
  SHARED_PTR<ImageInterface<Float> >  pruneRegions2(const ImageInterface<Float>& image,
                                                   Double& thresh,
                                                   Int nmask=0,
                                                   Double prunesize=0.0);

  // create a mask image (1/0 image) applying a different threshold for each channel plane
  void makeMaskByPerChanThreshold(const ImageInterface<Float>& image,
                                 ImageInterface<Float>& mask,
                                 Vector<Float>& thresholds);

  // A core method for binary dilation of the input lattice
  void binaryDilationCore(Lattice<Float>& inlattice,
                      Array<Float>& structure,
                      Lattice<Bool>& mask,
                      Array<Bool>& chanmask,
                      Lattice<Float>& outlattice);

  // Multiple Binary dilation application of an image with a constraint mask and channel plane based flags
  void binaryDilation(ImageInterface<Float>& inImage,
                      Array<Float>& structure,
                      Int niteration,
                      Lattice<Bool>& mask,
                      Array<Bool>& chanmask,
                      ImageInterface<Float>& outImage);
 
  // return beam area in pixel unit
  Float pixelBeamArea(const GaussianBeam& beam, const CoordinateSystem& csys);

  // Create a mask image applying PB level
  void makePBMask(SHARED_PTR<SIImageStore> imstore, Float pblimit=0.1);
  void autoMaskWithinPB(SHARED_PTR<SIImageStore> imstore, 
                        const Int iterdone,
                        const String& alg="",
                        const String& threshold="",
                        const Float& fracpeak=0.0,
                        const String& resolution="",
                        const Float& resbybeam=0.0,
                        const Int nmask=0,
                        const Bool autoadjust=False,
                        const Float& sidelobethreshold=0.0,
                        const Float& noisethreshold=0.0,
                        const Float& lownoisethreshold=0.0,
                        const Float& cutthreshold=0.0,
                        const Float& smoothfactor=0.0,
                        const Float& minbeamfrac=0.0,
                        Float pblimit=0.1);

  // check if input image is a mask image with 0 or a value (if normalize=true, 1)
  Bool checkMaskImage(ImageInterface<Float>& maskiamge, Bool normalize=True);

// 
  static Bool cloneImShape(const ImageInterface<Float>& inImage, const String& outImageName);
  // max MB of memory to use in TempImage
  static inline casacore::Double memoryToUse() {return 1.0;};


protected:
  InteractiveMasking *interactiveMasker_p;

private:
  Double itsRms;
  Double itsMax;
  Float itsSidelobeLevel;
};



} //# NAMESPACE CASA - END


#endif



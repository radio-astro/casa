//# Feather.h: Helper for feathering
//# Copyright (C) 1996-2012
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
#ifndef SYNTHESIS_FEATHER_H
#define SYNTHESIS_FEATHER_H

#include <casa/BasicSL/String.h>
#include <casa/BasicSL.h>
#include <casa/Quanta/Quantum.h>
#include <scimath/Mathematics/GaussianBeam.h>
#include <coordinates/Coordinates/CoordinateSystem.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  //Forward declaration
  template<class T> class ImageInterface;
  template<class T> class Vector;
  // <summary> Class that contains functions needed for feathering</summary>

  class Feather{

  public:

    //default constructor
    Feather();
    //Constructor 
    Feather(const ImageInterface<Float>& SDImage, const ImageInterface<Float>& INTImage, Float sdScale=1.0);
    //Destructor
    virtual ~Feather();

    //Clear the weight flags.  Used when changing between the
    //SDimage and the dirty image.
    void clearWeightFlags();

    //set the SDimage and Int images 
    void setSDScale(Float sdscale=1.0);
    void setSDImage(const ImageInterface<Float>& SDImage);
    void setINTImage(const ImageInterface<Float>& IntImage);
    //set and get effective dish diameter to be used in feathering function
    //setEffectiveDishDiam will return False if you are trying to assign a finer resolution than what 
    //the original data came with 
    Bool setEffectiveDishDiam(const Float xdiam, const Float ydiam=-1.0);
    void getEffectiveDishDiam(Float& xdiam, Float& ydiam);
    //Get the 1-D slices of amplitude  along the x and y axis of the FFT of images  
    //if radial is set to True then the 1D slice is the the circular average rather that X and Y cuts...only the x-values are valid then 
    //Note the SD image is already feathered by its beam..you cannot get unfeathered SD data as it implies deconvolution
    void getFTCutSDImage(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp, const Bool radial=False );
    void getFTCutIntImage(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp, Bool radial=False);
    //Get the 1-D slices of the feathering function that will be applied on SD and INTerf data
    //If normalize=False for the SD then the value for Jy/beam correction to final beam size is multiplied to 
    //the function
    void getFeatherSD(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp, Bool radial=False, Bool normalize=True);
    void getFeatherINT(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp, Bool radial=False);
    //Get 1-D slices of the feathered data ...
    //note for SD  this
    //should return the same values as getFTCutSDImage
    void getFeatheredCutSD(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp, Bool radial=False);
    void getFeatheredCutINT(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp, Bool radial=False);
    //This function convolves the INT image to the new GaussianBeam
    //So the INT image stored in this object is going to be replaced by the new convolved image
    void convolveINT(const GaussianBeam& newHighBeam);

    //write the feathered image to disk
    Bool saveFeatheredImage(const String& imagename);
    ///////Helper function to get XY slices and radial cuts for any generic image
    static void getCutXY(Vector<Float>& ux, Vector<Float>& xamp, 
			 Vector<Float>& uy, Vector<Float>& yamp, 
			 const ImageInterface<Float>& image);
    static void getRadialCut(Vector<Float>& radius, Vector<Float>& radialAmp, 
			  const ImageInterface<Float>& image);


    ///////////////Old methods left for now till new version is implemented
    static void feather(const String& image, const ImageInterface<Float>& high, const ImageInterface<Float>& low, const Float& sdScale=1.0, const String& lowPSF="", const Bool useDefault=True, const String& vpTable="" , Float effSDDiam=-1.0, const Bool lowpassfiltersd=False);

    static Double worldFreq(const CoordinateSystem& cs, Int spectralpix=0);

    
  private:
    void fillXVectors( Vector<Float>& ux, Vector<Float>& uy ) const;
    static void applyDishDiam(ImageInterface<Complex>& image, GaussianBeam& beam, Float effDiam, ImageInterface<Float>& newbeam, Vector<Quantity>& extraconv);


    
  
    CountedPtr<ImageInterface<Float> > lowIm_p;
    CountedPtr<ImageInterface<Float> > lowImOrig_p;
    CountedPtr<ImageInterface<Float> > highIm_p;
    CountedPtr<ImageInterface<Complex> > cwImage_p;
    CountedPtr<ImageInterface<Complex> > cwHighIm_p;
    static void getCutXY(Vector<Float>& ux, Vector<Float>& xamp, 
		  Vector<Float>& uy, Vector<Float>& yamp, ImageInterface<Complex>& ftimage);
    static void getRadialCut(Vector<Float>& radialAmp, ImageInterface<Complex>& ftimage);
    //calculate the complex weight image to apply on the interf image
    static void getLowBeam(const ImageInterface<Float>& low0, const String& lowPSF, const Bool useDefaultPB, const String& vpTableStr, GaussianBeam& lBeam);
    void calcCWeightImage();
    void applyFeather();
    static void getRadialUVval(const Int npix, const IPosition& imshape, const CoordinateSystem& csys, Vector<Float>& radius);
    GaussianBeam hBeam_p;
    GaussianBeam lBeam_p;
    GaussianBeam lBeamOrig_p;
    Float dishDiam_p;
    Bool cweightCalced_p;
    Bool cweightApplied_p;
    Float sdScale_p;
    CoordinateSystem csysHigh_p;
  };



} //# NAMESPACE CASA - END

#endif

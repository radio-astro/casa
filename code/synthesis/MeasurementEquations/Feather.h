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

namespace casa { //# NAMESPACE CASA - BEGIN
  //Forward declaration
  template<class T> class ImageInterface;
  template<class T> class Vector;
  class GaussianBeam;
  class CoordinateSystem;
  // <summary> Class that contains functions needed for feathering</summary>

  class Feather{

  public:

    //default constructor
    Feather();
    //Constructor 
    Feather(const ImageInterface<Float>& SDImage, const ImageInterface<Float>& INTImage);
    //Destructor
    virtual ~Feather();
    //set the SDimage and Int images 
    void setSDImage(const ImageInterface<Float>& SDImage);
    void setINTImage(const ImageInterface<Float>& IntImage);
    //set and get effective dish diameter to be used in feathering function
    //setEffectiveDishDiam will return False if you are trying to assign a finer resolution than what 
    //the original data came with 
    Bool setEffectiveDishDiam(const Float xdiam, const Float ydiam=-1.0);
    void getEffectiveDishDiam(Float& xdiam, Float& ydiam);
    //Get the 1-D slices of amplitude  along the x and y axis of the FFT of images  
    //Note the SD image is already feathered by its beam..you cannot get unfeathered SD data as it implies deconvolution
    void getFTCutSDImage(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp);
    void getFTCutIntImage(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp);
    //Get the 1-D slices of the feathering function that will be applied on SD and INTerf data
    void getFeatherSD(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp);
    void getFeatherINT(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp);
    //Get 1-D slices of the feathered data ...note for SD is effectiveDishdiam is left to the original of image this
    //should return the same values as getFTCutSDImage
    void getFeatheredCutSD(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp);
    void getFeatheredCutINT(Vector<Float>& ux, Vector<Float>& xamp, Vector<Float>& uy, Vector<Float>& yamp);
    //write the feathered image to disk
    Bool saveFeatheredImage(const String& imagename);

    ///////////////Old methods left for now till new version is implemented
    static void feather(const String& image, const ImageInterface<Float>& high, const ImageInterface<Float>& low, const Float& sdScale=1.0, const String& lowPSF="", const Bool useDefault=True, const String& vpTable="" , Float effSDDiam=-1.0, const Bool doPlot=False);

    static Double worldFreq(const CoordinateSystem& cs, Int spectralpix=0);
  private:
    
    static void applyDishDiam(ImageInterface<Complex>& image, GaussianBeam& beam, Float effDiam, ImageInterface<Float>& newbeam, Vector<Quantity>& extraconv);


  };



} //# NAMESPACE CASA - END

#endif

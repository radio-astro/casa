//# WBCleanImageSkyModel.h: Definition for WBCleanImageSkyModel
//# Copyright (C) 1996,1997,1998,1999,2000
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

#ifndef SYNTHESIS_WBCLEANIMAGESKYMODEL_H
#define SYNTHESIS_WBCLEANIMAGESKYMODEL_H

#include <synthesis/MeasurementComponents/CleanImageSkyModel.h>
#include <lattices/Lattices/LatticeCleanProgress.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/MultiTermLatticeCleaner.h>
#include <casa/OS/Timer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//forward
class SkyEquation;

// <summary> 
// WB Clean Image Sky Model: Image Sky Model implementing a Wide-Band
// multi frequency synthesis algorithm
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=LatticeCleaner>LatticeCleaner</linkto> module
//   <li> <linkto class=ImageSkyModel>ImageSkyModel</linkto> module
//   <li> <linkto class=LinearModel>LinearModel</linkto> module
// </prerequisite>
//
// <etymology>
// WBCleanImageSkyModel implements the Wide Band Clean algorithm.
// It is derived from <linkto class=SkyModel>SkyModel</linkto>.
// </etymology>
//
// <synopsis> 
// The WB Clean is the multi-frequency synthesis deconvolution
// algorithm. It decomposes an image into a linear compbination
// of models convolved with spectral dirty beams of various
// order. A multiscale variant can be invoked by supplying a
// user vector of scale sizes. Default is 1, corresponding to a
// scale insensitive mfs deconvolution.
//
// Masking is optionally performed using a mask image: only points
// where the mask is non-zero are searched for Gaussian components. 
// This can cause some difficulty, as the different Gaussian scale
// sizes will extend beyond the mask by different amounts.
// If no mask is specified
// all points in the inner quarter of the image are cleaned.
// </synopsis> 
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// </motivation>
//
// <todo asof="99/04/07">
// <ul> Improve the masking.
// </todo>

class WBCleanImageSkyModel : public CleanImageSkyModel {
public:

  // Create a WBCleanImageSkyModel - default scale size = 1 pixel
  WBCleanImageSkyModel();
  WBCleanImageSkyModel(const Int ntaylor,const Int nscales,const Double reffreq);
  WBCleanImageSkyModel(const Int ntaylor,const Vector<Float>& userScaleSizes, const Double reffreq);

  // destructor
  ~WBCleanImageSkyModel();

  // Solve for this SkyModel
  Bool solve (SkyEquation& se);
  Bool copyLatToImInt(TempLattice<Float>& lat, ImageInterface<Float>& im);
  Bool copyImIntToLat(TempLattice<Float>& lat, ImageInterface<Float>& im);
  
//  Int nmodels_p; // Number of image models = nfields * ntaylor
  Int ntaylor_p; // Number of terms in the Taylor expansion to use.
//  Int nfields_p; // Number of image fields/pointings.
  Int nscales_p; // Number of scales to use for the multiscale part.
  
  Double refFrequency_p;

//   Int add(ImageInterface<Float>& iimage, const Int maxNumXfr=100);
//   Bool addResidual(Int thismodel, ImageInterface<Float>& iresidual);
//   void initializeGradients();
   Bool solveResiduals(SkyEquation& se);
   Bool makeNewtonRaphsonStep(SkyEquation& se, Bool incremental=False, Bool modelToMS=False);

   Int numberOfTaylorTerms(){return ntaylor_p;};
   Double getReferenceFrequency(){return refFrequency_p;};

   // Major axis for ordering : Taylor
   Int getModelIndex(uInt model, uInt taylor){return taylor * (nfields_p) + (model);};
   Int getTaylorIndex(uInt index){return Int(index/nfields_p);};
   Int getFieldIndex(uInt index){return index%nfields_p;};

   // Major axis for ordering : Models
   //inline Int getModelIndex(uInt model, uInt taylor){return model * (ntaylor_p) + (taylor);};
   //inline Int getTaylorIndex(uInt index){return index%ntaylor_p;};
   //inline Int getModelIndex(uInt index){return index/ntaylor_p;};
 
   Vector<String> imageNames;
   
private:

  PtrBlock<MultiTermLatticeCleaner<Float>* > lc_p;
   
  Vector<Float> scaleSizes_p; // Vector of scale sizes in pixels.
  Vector<Float> scaleBias_p; // Vector of scale biases !!
  Float maxPsf_p;

  IPosition gip,imshape;
  Bool donePSF_p;
  Int nx,ny;
  
  // Memory to be allocated per TempLattice
  Double memoryMB_p;
  
  LogIO os;
  
  void initVars();
  
  Int storeAsImg(String fileName, ImageInterface<Float>& theImg);
  Int storeTLAsImg(String fileName, TempLattice<Float> &TL, ImageInterface<Float>& theImg);
  Int storeTLAsImg(String fileName, TempLattice<Complex> &TL, ImageInterface<Float>& theImg);

  Bool resizeWorkArrays(Int length);
  
  Int makeSpectralPSFs(SkyEquation& se);
  Bool findMaxAbsLattice(const TempLattice<Float>& masklat,const Lattice<Float>& lattice,Float& maxAbs,IPosition& posMaxAbs, Bool flip=False);
  Int addTo(Lattice<Float>& to, const Lattice<Float>& add, Float multiplier);
  Int writeResultsToDisk();
  
  Timer tmr1,tmr2;
  Int adbg;
  Int tdbg;
  Int ddbg;
  // Put in some progress metre here...
  
  
};


} //# NAMESPACE CASA - END

#endif



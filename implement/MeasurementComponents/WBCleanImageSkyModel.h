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
  WBCleanImageSkyModel(const Int ntaylor,const Int nscales);
  WBCleanImageSkyModel(const Int ntaylor,const Vector<Float>& userScaleSizes);

  // destructor
  ~WBCleanImageSkyModel();

  // Solve for this SkyModel
   Bool solve (SkyEquation& se);
  Bool solveResiduals(SkyEquation& se, Float &maxres, IPosition &maxip);
  Bool copyLatToImInt(TempLattice<Float>& lat, ImageInterface<Float>& im);
  Bool copyImIntToLat(TempLattice<Float>& lat, ImageInterface<Float>& im);
 
  // Complex image (needed for e.g. RR,RL,LR,LL)
//   ImageInterface<Complex>& cImage1(Int model=0);
//   ImageInterface<Float>& image1(Int model=0);
//   ImageInterface<Float>& deltaImage1(Int model=0);
//   ImageInterface<Float>& gS1(Int model=0);
//   ImageInterface<Float>& PSF1(Int model=0);
   
   ImageInterface<Float>& PSFspec(Int model=0, Int taylor=1);
   ImageInterface<Float>& gSspec(Int model=0, Int taylor=1);
   ImageInterface<Complex>& cImagespec(Int model=0, Int taylor=1);
   ImageInterface<Float>& imagespec(Int model=0, Int taylor=1);
   ImageInterface<Float>& deltaImagespec(Int model=0, Int taylor=1);
  
  Int ntaylor_p; // Number of terms in the Taylor expansion to use.
  Int nscales_p; // Number of scales to use for the multiscale part.
  
  Float useRefFrequency_p;

//  PtrBlock<ImageInterface<Complex> * > cimage1_p;
//  PtrBlock<ImageInterface<Float> * > image1_p;
//  PtrBlock<TempImage<Float> * > deltaimage1_p;
//  PtrBlock<TempImage<Float> * > gS1_p;
//  PtrBlock<TempImage<Float> * > psf1_p;
  
  PtrBlock<TempImage<Float>* > psfspec_p;
  PtrBlock<TempImage<Float>* > gSspec_p;
  PtrBlock<ImageInterface<Complex> * > cimagespec_p;
  PtrBlock<ImageInterface<Float> * > imagespec_p;
  PtrBlock<TempImage<Float>* > deltaimagespec_p;

   //Int add(ImageInterface<Float>& image,ImageInterface<Float>& image1, const Int maxNumXfr=100);
   Int add(ImageInterface<Float>& image, const Int maxNumXfr=100);
   void initializeGradients();
 
   Vector<String> imageNames;
   
private:
  // Dirty image and image mask
  TempLattice<Float>* dirty_p;
  TempLattice<Complex>* dirtyFT_p;
  TempLattice<Float>* mask_p;
  TempLattice<Float>* fftmask_p;
  
  Vector<Float> scaleSizes_p; // Vector of scale sizes in pixels.
  Vector<Float> scaleBias_p; // Vector of scale biases !!
  Vector<Float> totalScaleFlux_p; // Vector of total scale fluxes.
  Vector<Float> totalTaylorFlux_p; // Vector of total flux in each taylor term.
  Float weightScaleFactor_p;
  Float maxPsf_p;

  IPosition gip,imshape;
  Bool donePSF_p,donePSP_p,doneCONV_p;
  Int nx,ny,npol_p,nchan;
  
  // h(s) [nx,ny,nscales]
  PtrBlock<TempLattice<Float>* > vecScales_p; 
  PtrBlock<TempLattice<Complex>* > vecScalesFT_p; 
  
  // B_k  [nx,ny,ntaylor]
  PtrBlock<TempLattice<Float>* > vecPsf_p; 
  PtrBlock<TempLattice<Complex>* > vecPsfFT_p; 
 
  // M_k [nx,ny,ntaylor]
  //PtrBlock<TempLattice<Float>* > vecModel_p; 
  
  // A_{smn} = B_{sm} * B{sn} [nx,ny,ntaylor,ntaylor,nscales,nscales]
  // A_{s1s2mn} = B_{s1m} * B{s2n} [nx,ny,ntaylor,ntaylor,nscales,nscales]
  PtrBlock<TempLattice<Float>* > cubeA_p; 
  PtrBlock<LatticeIterator<Float>* > itercubeA_p;
  
  // R_{sk} = I_D * B_{sk} [nx,ny,ntaylor,nscales]
  PtrBlock<TempLattice<Float>* > matR_p; 
  PtrBlock<LatticeIterator<Float>* > itermatR_p;
  
  // a_{sk} = Solution vectors. [nx,ny,ntaylor,nscales]
  PtrBlock<TempLattice<Float>* > matCoeffs_p; 
  PtrBlock<LatticeIterator<Float>* > itermatCoeffs_p;

  // Memory to be allocated per TempLattice
  Double memoryMB_p;
  
  // Solve [A][Coeffs] = [I_D * B]
  // Shape of A : [ntaylor,ntaylor]
  PtrBlock<Matrix<Double>*> matA_p;    // 2D matrix to be inverted.
  PtrBlock<Matrix<Double>*> invMatA_p; // Inverse of matA_p;

  TempLattice<Complex>* cWork_p;
  TempLattice<Float>* tWork_p;
  LatticeIterator<Float>* itertWork_p;
  
  LatticeExprNode len_p;

  Float lambda_p;
  
  LogIO os;
  
  void initVars();
  
  Int numberOfTempLattices(Int nscales,Int ntaylor);
  Int manageMemory(Bool allocate);
  Int storeAsImg(String fileName, ImageInterface<Float>& theImg);
  Int storeTLAsImg(String fileName, TempLattice<Float> &TL, ImageInterface<Float>& theImg);
//  Float modifyWeights(SkyEquation& se, Float power, Bool direction);

  Int makeScale(Lattice<Float>& scale, const Float& scaleSize);
  Float spheroidal(Float nu);
  
  Int makeSpectralPSFs(SkyEquation& se);
  Bool findMaxAbsLattice(const TempLattice<Float>& masklat,const Lattice<Float>& lattice,Float& maxAbs,IPosition& posMaxAbs, Bool flip=False);
  Int addTo(Lattice<Float>& to, const Lattice<Float>& add, Float multiplier);
  Int writeResultsToDisk();
  Int calcWeightScaleFactor(SkyEquation& se);
  Int setupMasks();
  Int setupBlobs();
  Int readInModels();
  Int computeFluxLimit(Float &fluxlimit, Float threshold);
  
  Int computeMatrixA();
  Int computeRHS();
  Int solveMatrixEqn(Int scale);
 
  Int computePenaltyFunction(Int scale, Int totaliters, Float &loopgain, Bool choosespec);
 
  Int updateSolution(IPosition globalmaxpos, Int maxscaleindex, Float loopgain);
  
  Int IND2(Int taylor,Int scale);
  Int IND4(Int taylor1, Int taylor2, Int scale1, Int scale2);
  
  Timer tmr1,tmr2;
  Int adbg;
  Int tdbg;
  Int ddbg;
  // Put in some progress metre here...
  
  
};


} //# NAMESPACE CASA - END

#endif



//# MultiTermMatrixCleaner.h: Minor Cycle for MSMFS deconvolution
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
//# $Id: MultiTermMatrixCleaner Urvashi R.V.  2010-12-04 <rurvashi@aoc.nrao.edu$

#ifndef SYNTHESIS_MULTITERMLATTICECLEANER_H
#define SYNTHESIS_MULTITERMLATTICECLEANER_H

#include <scimath/Mathematics/FFTServer.h>
#include <synthesis/MeasurementEquations/MatrixCleaner.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MultiTermMatrixCleaner : public MatrixCleaner
{
public:
  // Create a cleaner 
  MultiTermMatrixCleaner();

  // The copy constructor uses reference semantics
  //  MultiTermMatrixCleaner(const MultiTermMatrixCleaner & other);

  // The assignment operator also uses reference semantics
  //  MultiTermMatrixCleaner & operator=(const MultiTermMatrixCleaner & other); 

  // The destructor resizes arrays to empty before destruction.
  ~MultiTermMatrixCleaner();

  // Input : number of Taylor terms
  //         Reshapes PtrBlocks to hold the correct number of PSFs and Residual images
  Bool setntaylorterms(const int & nterms);
  
  // Input : scales
  Bool setscales(const Vector<Float> & scales);

  // Initialize all the memory being used.
  Bool initialise(Int nx,Int ny);

  // Calculate Hessian elements and check for invertibility
  // Does not have to be called externally, but can be. Either way, it executes only once.
  Int computeHessianPeak();

  // Input : psfs and dirty images
  Bool setpsf(int order, Matrix<Float> & psf);
  
  // Input : psfs and dirty images
  Bool setresidual(int order, Matrix<Float> & dirty);
 
  // Input : model images
  Bool setmodel(int order, Matrix<Float> & model);

  // Input : mask
  Bool setmask(Matrix<Float> & mask);
 
  // Run the minor cycle
  Int mtclean(Int maxniter, Float stopfraction, Float inputgain, Float userthreshold);

  // Output : Model images
  Bool getmodel(int order, Matrix<Float> & model);
  
  // Output : psfs and dirty images
  Bool getresidual(int order, Matrix<Float> & residual);
  
  // Compute principal solution - in-place on the residual images in vecDirty. 
  Bool computeprincipalsolution();
 
  // Output : Hessian matrix
  Bool getinvhessian(Matrix<Double> & invhessian);

  // Output : Peak residual computed from matR_p (residual convolved with PSF).
  Float getpeakresidual(){return rmaxval_p;}


private:
  LogIO os;

  using MatrixCleaner::itsCleanType;
  using MatrixCleaner::itsMaxNiter;
  using MatrixCleaner::itsGain;
  using MatrixCleaner::itsThreshold;
  using MatrixCleaner::itsMask;
  using MatrixCleaner::itsPositionPeakPsf;
  //  using MatrixCleaner::itsScaleMasks;
  //  using MatrixCleaner::itsScaleXfrs;
  //  using MatrixCleaner::itsNscales;
  //  using MatrixCleaner::itsScalesValid;
  using MatrixCleaner::itsMaskThreshold;

  using MatrixCleaner::findMaxAbs;
  using MatrixCleaner::findMaxAbsMask;
  using MatrixCleaner::makeScale;
  //  using MatrixCleaner::addTo;
  using MatrixCleaner::makeBoxesSameSize;
  using MatrixCleaner::validatePsf;
  //using MatrixCleaner::makeScaleMasks;

  Int ntaylor_p; // Number of terms in the Taylor expansion to use.
  Int psfntaylor_p; // Number of terms in the Taylor expansion for PSF.
  Int nscales_p; // Number of scales to use for the multiscale part.
  Int nx_p;
  Int ny_p;
  Int totalIters_p; // Total number of minor-cycle iterations
  Float globalmaxval_p;
  Int maxscaleindex_p;
  IPosition globalmaxpos_p;
  Int itercount_p; // Number of minor cycle iterations
  Int maxniter_p;
  Float stopfraction_p;
  Float inputgain_p;
  Float userthreshold_p;
  Float prev_max_p;
  Float min_max_p;
  Float rmaxval_p;

  IPosition psfsupport_p;
  IPosition psfpeak_p;
  IPosition blc_p, trc_p, blcPsf_p, trcPsf_p;

  Vector<Float> scaleSizes_p; // Vector of scale sizes in pixels.
  Vector<Float> scaleBias_p; // Vector of scale biases !!
  Vector<Float> totalScaleFlux_p; // Vector of total scale fluxes.
  Vector<Float> totalTaylorFlux_p; // Vector of total flux in each taylor term.
  Vector<Float> maxScaleVal_p; // Vector for peaks at each scale size
  Vector<IPosition> maxScalePos_p; // Vector of peak positions at each scale size.

  IPosition gip;
  Int nx,ny;
  Bool donePSF_p,donePSP_p,doneCONV_p;
 
  Matrix<Complex> dirtyFT_p;
  Block<Matrix<Float> > vecScaleMasks_p;
  
  Matrix<Complex> cWork_p;
  Block<Matrix<Float> > vecWork_p;
  
  // h(s) [nx,ny,nscales]
  Block<Matrix<Float> > vecScales_p; 
  Block<Matrix<Complex> > vecScalesFT_p; 
  
  // B_k  [nx,ny,ntaylor]
  // Block<Matrix<Float> > vecPsf_p; 
  Block<Matrix<Complex> > vecPsfFT_p; 
  
  // I_D : Residual/Dirty Images [nx,ny,ntaylor]
  Block<Matrix<Float> > vecDirty_p; 
 
  // I_M : Model Images [nx,ny,ntaylor]
  Block<Matrix<Float> > vecModel_p; 
  //  Block <Matrix<Float> > vecScaleModel_p;
 
  // A_{smn} = B_{sm} * B{sn} [nx,ny,ntaylor,ntaylor,nscales,nscales]
  // A_{s1s2mn} = B_{s1m} * B{s2n} [nx,ny,ntaylor,ntaylor,nscales,nscales]
  Block<Matrix<Float> > cubeA_p; 

  // R_{sk} = I_D * B_{sk} [nx,ny,ntaylor,nscales]
  Block<Matrix<Float> > matR_p; 
  
  // a_{sk} = Solution vectors. [nx,ny,ntaylor,nscales]
  Block<Matrix<Float> > matCoeffs_p; 

  // Memory to be allocated per Matrix
  Double memoryMB_p;
  
  // Solve [A][Coeffs] = [I_D * B]
  // Shape of A : [ntaylor,ntaylor]
  Block<Matrix<Double> > matA_p;    // 2D matrix to be inverted.
  Block<Matrix<Double> > invMatA_p; // Inverse of matA_p;

  // FFTserver
  FFTServer<Float,Complex> fftcomplex;

  // Initial setup functions  
  Int verifyScaleSizes();
  Int allocateMemory();
  Int setupScaleFunctions();

  // Setup per major cycle
  Int setupUserMask();
  Int computeFluxLimit(Float &fluxlimit, Float threshold);
  Int computeRHS();

  // Solver functions : minor-cycle iterations. Need to be efficient.
  Int solveMatrixEqn(Int ntaylor,Int scale, IPosition blc, IPosition trc);
  Int chooseComponent(Int ntaylor,Int scale, Int criterion, IPosition blc, IPosition trc);
  Int updateModelAndRHS(Float loopgain);
  Int updateRHS(Int ntaylor, Int scale, Float loopgain,Vector<Float> coeffs, IPosition blc, IPosition trc, IPosition blcPsf, IPosition trcPsf);
  Int checkConvergence(Int updatetype, Float &fluxlimit, Float &loopgain); 
  Bool buildImagePatches();

  // Helper functions
  Int writeMatrixToDisk(String imagename, Matrix<Float> &themat);
  Int IND2(Int taylor,Int scale);
  Int IND4(Int taylor1, Int taylor2, Int scale1, Int scale2);
  
  Bool adbg;
};

} //# NAMESPACE CASA - END

#endif


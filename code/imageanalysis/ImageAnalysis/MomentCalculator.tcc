//# MomentCalculator.cc: 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2004
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
//# $Id: MomentCalculator.tcc 19940 2007-02-27 05:35:22Z Malte.Marquarding $
//
#include <imageanalysis/ImageAnalysis/MomentCalculator.h>

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <scimath/Fitting/NonLinearFitLM.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Functionals/CompoundFunction.h>
#include <imageanalysis/ImageAnalysis/ImageMoments.h>
#include <lattices/LatticeMath/LatticeStatsBase.h>
#include <casa/BasicMath/Math.h>
#include <casa/Logging/LogIO.h> 
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>



namespace casa { //# NAMESPACE CASA - BEGIN

// Derived class MomentWindow

template <class T>
MomentWindow<T>::MomentWindow(shared_ptr<Lattice<T>> pAncilliaryLattice,
                              MomentsBase<T>& iMom,
                              LogIO& os,
                              const uInt nLatticeOut)
: _ancilliaryLattice(pAncilliaryLattice),
  iMom_p(iMom),
  os_p(os)
{
// Set moment selection vector

   selectMoments_p = this->selectMoments(iMom_p);

// Set/check some dimensionality

   constructorCheck(calcMoments_p, calcMomentsMask_p, selectMoments_p, nLatticeOut);

// Fish out moment axis

   Int momAxis = this->momentAxis(iMom_p);

// Set up slice shape for extraction from masking lattice

   if (_ancilliaryLattice != 0) {
      sliceShape_p.resize(_ancilliaryLattice->ndim());
      sliceShape_p = 1;
      sliceShape_p(momAxis) = _ancilliaryLattice->shape()(momAxis);
   }

// Make all plots with same y range ?

   //fixedYLimits_p = this->fixedYLimits(iMom_p);
   this->yAutoMinMax(yMinAuto_p, yMaxAuto_p, iMom_p);

// Are we computing the expensive moments ?

   this->costlyMoments(iMom_p, doMedianI_p, doMedianV_p, doAbsDev_p);

// Are we plotting ?

   //plotter_p = this->device(iMom_p);

// Are we computing coordinate-dependent moments.  If
// so precompute coordinate vector is momebt axis separable

   this->setCoordinateSystem (cSys_p, iMom_p);
   this->doCoordCalc(doCoordProfile_p, doCoordRandom_p, iMom_p);
   this->setUpCoords(iMom_p, pixelIn_p, worldOut_p, sepWorldCoord_p, os_p,
               integratedScaleFactor_p, cSys_p, doCoordProfile_p, 
               doCoordRandom_p);

// What is the axis type of the moment axis
   
   momAxisType_p = this->momentAxisName(cSys_p, iMom_p);

// Are we fitting, automatically or interactively ?

   doAuto_p = this->doAuto(iMom_p);
   doFit_p = this->doFit(iMom_p);

// Values to assess if spectrum is all noise or not

   peakSNR_p = this->peakSNR(iMom_p);
   stdDeviation_p = this->stdDeviation(iMom_p);

// Number of failed Gaussian fits 

   nFailed_p = 0;
}


template <class T>
MomentWindow<T>::~MomentWindow()
{;}

template <class T>
void MomentWindow<T>::process(T&,
                              Bool&,
                              const Vector<T>&,
                              const Vector<Bool>&,
                              const IPosition&)
{
   throw(AipsError("MomentWindow<T>::process not implemented"));
}


template <class T> 
void MomentWindow<T>::multiProcess(Vector<T>& moments,
                                   Vector<Bool>& momentsMask,
                                   const Vector<T>& profileIn,
                                   const Vector<Bool>& profileInMask,
                                   const IPosition& inPos)
//
// Generate windowed moments of this profile.
// The profile comes with its own mask (or a null mask
// which means all good).  In addition, we create
// a further mask by applying the clip range to either
// the primary lattice, or the ancilliary lattice (e.g. 
// the smoothed lattice)
//
{

// Fish out the ancilliary image slice if needed.  Stupid slice functions 
// require me to create the slice empty every time so degenerate
// axes can be chucked out.  We set up a pointer to the primary or 
// ancilliary vector object  that we can use for fast access 

   const T* pProfileSelect = 0;      
   Bool deleteIt;
   if (_ancilliaryLattice) {
      Array<T> ancilliarySlice;
      IPosition stride(_ancilliaryLattice->ndim(),1);
      _ancilliaryLattice->getSlice(ancilliarySlice, inPos,
                               sliceShape_p, stride, True);
      ancilliarySliceRef_p.reference(ancilliarySlice);

      pProfileSelect_p = &ancilliarySliceRef_p;
      pProfileSelect = ancilliarySliceRef_p.getStorage(deleteIt);
   } else {
      pProfileSelect_p = &profileIn;
      pProfileSelect = profileIn.getStorage(deleteIt);
   }


// Make abcissa and labels
   
   //static Bool allSubsequent = False;
   static Vector<Int> window(2);  
   static Int nPts = 0;
      
   this->makeAbcissa (abcissa_p, pProfileSelect_p->nelements());
   String xLabel;
   if (momAxisType_p.empty()) {
      xLabel = "x (pixels)";
   } else {
      xLabel = momAxisType_p + " (pixels)";
   }
   const String yLabel("Intensity");
   String title;
   setPosLabel(title, inPos);


// Do the window selection

   if (doAuto_p) {
   
// Define the window automatically

      Vector<T> gaussPars;
      if (getAutoWindow(nFailed_p, window,  abcissa_p, *pProfileSelect_p, profileInMask,
                        peakSNR_p, stdDeviation_p, doFit_p/*, plotter_p, 
                        fixedYLimits_p, yMinAuto_p, yMaxAuto_p, xLabel,
                        yLabel, title*/)) {
         nPts = window(1) - window(0) + 1;
      } else {
         nPts = 0;
      }
   }


// If no points make moments zero and mask
               
   if (nPts==0) {
      moments = 0.0;
      momentsMask = False;

      if (_ancilliaryLattice) {
         ancilliarySliceRef_p.freeStorage(pProfileSelect, deleteIt);
      } else {
         profileIn.freeStorage(pProfileSelect, deleteIt);
      }
      return;
   }        


// Resize array for median.  Is resized correctly later
 
   selectedData_p.resize(nPts);
      

// Were the profile coordinates precomputed ?
      
   Bool preComp = (sepWorldCoord_p.nelements() > 0);

// 
// We must fill in the input pixel coordinate if we need
// coordinates, but did not pre compute them
//
   if (!preComp) {
      if (doCoordRandom_p || doCoordProfile_p) {
         for (uInt i=0; i<inPos.nelements(); i++) {
            pixelIn_p(i) = Double(inPos(i));
         }
      }
   }


// Set up a pointer for fast access to the profile mask
// if it exists.

   Bool deleteIt2;
   const Bool* pProfileInMask = profileInMask.getStorage(deleteIt2);


// Accumulate sums and acquire selected data from primary lattice 
            
   typename NumericTraits<T>::PrecisionType s0  = 0.0;
   typename NumericTraits<T>::PrecisionType s0Sq = 0.0;
   typename NumericTraits<T>::PrecisionType s1  = 0.0;
   typename NumericTraits<T>::PrecisionType s2  = 0.0;
   Int iMin = -1;
   Int iMax = -1;
   T dMin =  1.0e30;
   T dMax = -1.0e30;
   Double coord = 0.0;

   Int i,j;
   for (i=window(0),j=0; i<=window(1); i++) {
      if (pProfileInMask[i]) {
         if (preComp) {
            coord = sepWorldCoord_p(i);
         } else if (doCoordProfile_p) {
            coord = this->getMomentCoord(iMom_p, pixelIn_p,
                                   worldOut_p, Double(i));
         }
         this->accumSums(s0, s0Sq, s1, s2, iMin, iMax,
                   dMin, dMax, i, profileIn(i), coord);
         selectedData_p(j) = profileIn(i);
         j++;
      }
   }
   nPts = j;

         
// Absolute deviations of I from mean needs an extra pass.
   
   typename NumericTraits<T>::PrecisionType sumAbsDev = 0.0;
   if (doAbsDev_p) {
      T iMean = s0 / nPts;
      for (i=0; i<nPts; i++) sumAbsDev += abs(selectedData_p(i) - iMean);
   }



// Delete memory associated with pointers

   if (_ancilliaryLattice) {
      ancilliarySliceRef_p.freeStorage(pProfileSelect, deleteIt);
   } else {
      profileIn.freeStorage(pProfileSelect, deleteIt);
   }
   profileInMask.freeStorage(pProfileInMask, deleteIt2);

 
// Median of I
         
   T dMedian = 0.0;
   if (doMedianI_p) {
      selectedData_p.resize(nPts,True);
      dMedian = median(selectedData_p);
   }
       
// Fill all moments array
   
   T vMedian = 0;   
   this->setCalcMoments(iMom_p, calcMoments_p, calcMomentsMask_p, pixelIn_p, 
                  worldOut_p, doCoordRandom_p, integratedScaleFactor_p,
                  dMedian, vMedian, nPts, s0, s1, s2, s0Sq, 
                  sumAbsDev, dMin, dMax, iMin, iMax);


// Fill selected moments 

   for (i=0; i<Int(selectMoments_p.nelements()); i++) {
      moments(i) = calcMoments_p(selectMoments_p(i));
      momentsMask(i) = True;
      momentsMask(i) = calcMomentsMask_p(selectMoments_p(i));
   }
}

template <class T>
Bool MomentWindow<T>::getAutoWindow (uInt& nFailed,
                                     Vector<Int>& window,
                                     const Vector<T>& x,
                                     const Vector<T>& y,
                                     const Vector<Bool>& mask,
                                     const T peakSNR,
                                     const T stdDeviation,
                                     const Bool doFit) const
//
// Automatically fit a Gaussian and return the +/- 3-sigma window or
// invoke Bosma's method to set a window.  If a plotting device is
// active, we also plot the spectra and fits
//
// Inputs:
//   x,y        Spectrum
//   mask       Mask associated with spectrum. True is good.
//   plotter    Plot spectrum and optionally the  window
//   x,yLabel   x label for plots
//   title 
// Input/output
//   nFailed    Cumulative number of failed fits
// Output:
//   window     The window (pixels).  If both 0,  then discard this spectrum
//              and mask moments    
//
{
   if (doFit) {
      Vector<T> gaussPars(4);
      if (!this->getAutoGaussianFit (nFailed, gaussPars, x, y, mask, peakSNR, stdDeviation)) {
         window = 0;
         return False;
      } else {
   
// Set 3-sigma limits.  This assumes that there are some unmasked
// points in the window !
 
         if (!setNSigmaWindow (window, gaussPars(1), gaussPars(2),
                               y.nelements(), 3)) {
            window = 0;
            return False;
         }
      }
   } else {
// Invoke Albert's method (see AJ, 86, 1791)

      if (!getBosmaWindow (window, y, mask, peakSNR, stdDeviation)) {
         window = 0;
         return False;
      }
   }
   return True;
}

template <class T>
Bool MomentWindow<T>::getBosmaWindow (Vector<Int>& window,
                                      const Vector<T>& y,
                                      const Vector<Bool>& mask,
                                      const T peakSNR,
                                      const T stdDeviation) const
//
// Automatically work out the spectral window
// with Albert Bosma's algorithm.
//    
// Inputs: 
//   x,y       Spectrum
//   plotter   Plot device active if True
//   x,yLabel  Labels for plots
// Output:
//   window    The window
//   Bool      False if we reject this spectrum.  This may
//             be because it is all noise, or all masked
//
{
// See if this spectrum is all noise first.  If so, forget it.
// Return straight away if all maske

   T dMean;
   uInt iNoise = this->allNoise(dMean, y, mask, peakSNR, stdDeviation);
   if (iNoise == 2) return False;

   if (iNoise==1) {
      window = 0;
      return False;   
   }


// Find peak

   uInt minPos, maxPos;
   T yMin, yMax, yMean;
   this->stats(yMin, yMax, minPos, maxPos, yMean, y, mask);

   const Int nPts = y.nelements(); 
   Int iMin = max(0,Int(maxPos)-2);   
   Int iMax = min(nPts-1,Int(maxPos)+2);
   T tol = stdDeviation / (nPts - (iMax-iMin-1));
          
       
// Iterate to convergence
   
   Bool first = True;
   Bool converged = False;
   Bool more = True;
   yMean = 0;
   T oldYMean = 0;
   while (more) {
   
// Find mean outside of peak region

      typename NumericTraits<T>::PrecisionType sum = 0;
      Int i,j;
      for (i=0,j=0; i<nPts; i++) {
         if (mask(i) && (i < iMin || i > iMax)) {
            sum += y(i);
            j++;
         }
      }
      if (j>0) yMean = sum / j;
   

// Interpret result

      if (!first && j>0 && abs(yMean-oldYMean) < tol) {
         converged = True;
         more = False;
      } else if (iMin==0 && iMax==nPts-1)
         more = False;
      else {
   
// Widen window and redetermine tolerance

         oldYMean = yMean;
         iMin = max(0,iMin - 2);
         iMax = min(nPts-1,iMax+2); 
         tol = stdDeviation / (nPts - (iMax-iMin-1));
      }
      first = False;
   }   
      
// Return window

   if (converged) {
      window(0) = iMin;
      window(1) = iMax;
      return True;
   } else {
      window = 0;
      return False;   
   }
}  


template <class T> 
Bool MomentWindow<T>::setNSigmaWindow (Vector<Int>& window,
                                       const T pos,
                                       const T width,
                                       const Int nPts,
                                       const Int N) const
// 
// Take the fitted Gaussian position and width and
// set an N-sigma window.  If the window is too small
// return a Fail condition.
//
// Inputs:
//   pos,width   The position and width in pixels
//   nPts        The number of points in the spectrum that was fit
//   N           The N-sigma
// Outputs:
//   window      The window in pixels
//   Bool        False if window too small to be sensible
//
{
   window(0) = Int((pos-N*width)+0.5);
   window(0) = min(nPts-1,max(0,window(0)));
   window(1) = Int((pos+N*width)+0.5);
   window(1) = min(nPts-1,max(0,window(1)));
   // FIXME this was
   // if ( abs(window(1)-window(0)) < 3) return False;
   // return True;
   // but because window(1) - window(0) could be negative and True could be
   // returned, an allocation error was occuring because in another function a
   // vector was being resized to (window(1) - window(0)). It is possible that
   // in that case the absolute value should be calculated but I don't have time
   // at the moment to trace through the code and make sure that is really the
   // correct thing to do. Thus, making this function return false if window(1) - window(0)
   // seems the more conservative approach, so I'm doing that for now.
   return window(1)-window(0) >= 3;

} 




// Derived class MomentFit

template <class T>
MomentFit<T>::MomentFit(MomentsBase<T>& iMom,
                        LogIO& os,
                        const uInt nLatticeOut)
: iMom_p(iMom),
  os_p(os)
{
// Set moment selection vector

   selectMoments_p = this->selectMoments(iMom_p);

// Set/check some dimensionality

   constructorCheck(calcMoments_p, calcMomentsMask_p, selectMoments_p, nLatticeOut);

// Make all plots with same y range ?

   //fixedYLimits_p = this->fixedYLimits(iMom_p);
   this->yAutoMinMax(yMinAuto_p, yMaxAuto_p, iMom_p);

// Are we computing the expensive moments ?

   this->costlyMoments(iMom_p, doMedianI_p, doMedianV_p, doAbsDev_p);

// Are we plotting ?
   //plotter_p = this->device(iMom_p);

// Are we computing coordinate-dependent moments.  If so
// precompute coordinate vector if moment axis is separable

   this->setCoordinateSystem (cSys_p, iMom_p);
   this->doCoordCalc(doCoordProfile_p, doCoordRandom_p, iMom_p);
   this->setUpCoords(iMom_p, pixelIn_p, worldOut_p, sepWorldCoord_p, os_p,
               integratedScaleFactor_p, cSys_p, doCoordProfile_p, 
               doCoordRandom_p);

// What is the axis type of the moment axis

   momAxisType_p = this->momentAxisName(cSys_p, iMom_p);

// Are we fitting, automatically or interactively ?
   
   doAuto_p = this->doAuto(iMom_p);
   doFit_p = this->doFit(iMom_p);
         
// Values to assess if spectrum is all noise or not

   peakSNR_p = this->peakSNR(iMom_p);
   stdDeviation_p = this->stdDeviation(iMom_p);

// Number of failed Gaussian fits 

   nFailed_p = 0;

}


template <class T>
MomentFit<T>::~MomentFit()
{;}


template <class T> 
void MomentFit<T>::process(T&,
                            Bool&,
                            const Vector<T>&,
                            const Vector<Bool>&,
                            const IPosition&)
{
   throw(AipsError("MomentFit<T>::process not implemented"));
}



template <class T> 
void MomentFit<T>::multiProcess(Vector<T>& moments,
                                Vector<Bool>& momentsMask,
                                const Vector<T>& profileIn,
                                const Vector<Bool>& profileInMask,
                                const IPosition& inPos)
//
// Generate moments from a Gaussian fit of this profile
//
{

// Create the abcissa array and some labels
       
   Int nPts = profileIn.nelements();
   Vector<T> gaussPars(4);
   this->makeAbcissa (abcissa_p, nPts);
   String xLabel;
   if (momAxisType_p.empty())
      xLabel = "x (pixels)";
   else
      xLabel = momAxisType_p + " (pixels)";
   String yLabel("Intensity"); 
   String title;
   setPosLabel(title, inPos);


// Fit the profile

   if (doAuto_p) {

// Automatic

      if (!this->getAutoGaussianFit (nFailed_p, gaussPars, abcissa_p, profileIn, profileInMask, 
                               peakSNR_p, stdDeviation_p/*, plotter_p, fixedYLimits_p,
                               yMinAuto_p, yMaxAuto_p, xLabel, yLabel, title*/)) {
         moments = 0;   
         momentsMask = False;
         return;
      }

   } else {
/*
// Interactive
   
       if (!this->getInterGaussianFit(nFailed_p, gaussPars, os_p, abcissa_p, profileIn, profileInMask,
                                fixedYLimits_p, yMinAuto_p, yMaxAuto_p,
                                xLabel, yLabel, title, plotter_p)) {
         moments = 0;   
         momentsMask = False;
         return;
      }
  */ 
   }

// Were the profile coordinates precomputed ?
      
   Bool preComp = (sepWorldCoord_p.nelements() > 0);
// 
// We must fill in the input pixel coordinate if we need
// coordinates, but did not pre compute them
//
   if (!preComp) {
      if (doCoordRandom_p || doCoordProfile_p) {
         for (uInt i=0; i<inPos.nelements(); i++) {
            pixelIn_p(i) = Double(inPos(i));
         }
      }
   }

// Set Gaussian functional values.  We reuse the same functional that
// was used in the interactive fitting display process.

   gauss_p.setHeight(gaussPars(0));
   gauss_p.setCenter(gaussPars(1));
   gauss_p.setWidth(gaussPars(2));


// Compute moments from the fitted Gaussian
            
   typename NumericTraits<T>::PrecisionType s0  = 0.0;
   typename NumericTraits<T>::PrecisionType s0Sq = 0.0;
   typename NumericTraits<T>::PrecisionType s1  = 0.0;
   typename NumericTraits<T>::PrecisionType s2  = 0.0;
   Int iMin = -1;
   Int iMax = -1;
   T dMin =  1.0e30;
   T dMax = -1.0e30;
   Double coord = 0.0;
   T xx;
   Vector<T> gData(nPts);

   Int i,j;
   for (i=0,j=0; i<nPts; i++) {
      if (profileInMask(i)) {
         xx = i;
         gData(j) = gauss_p(xx) + gaussPars(3);
      
         if (preComp) {
            coord = sepWorldCoord_p(i);
         } else if (doCoordProfile_p) {
            coord = this->getMomentCoord(iMom_p, pixelIn_p, 
                                   worldOut_p,Double(i));
         }
         this->accumSums(s0, s0Sq, s1, s2, iMin, iMax,
                   dMin, dMax, i, gData(j), coord);
         j++;
      }
   }

// If no unmasked points go home.  This shouldn't happen
// as we can't have done a fit under these conditions.

   nPts = j;
   if (nPts == 0) {
      moments = 0;   
      momentsMask = False;
      return;
   }


// Absolute deviations of I from mean needs an extra pass.
         
   typename NumericTraits<T>::PrecisionType sumAbsDev = 0.0;
   if (doAbsDev_p) {
      T iMean = s0 / nPts;
      for (i=0; i<nPts; i++) sumAbsDev += abs(gData(i) - iMean);
   }


// Median of I
         
   T dMedian = 0.0;
   if (doMedianI_p) {
      gData.resize(nPts, True);
      dMedian = median(gData);
   }
   T vMedian = 0.0;
       
// Fill all moments array
   
   this->setCalcMoments(iMom_p, calcMoments_p, calcMomentsMask_p, pixelIn_p, 
                  worldOut_p, doCoordRandom_p, integratedScaleFactor_p,
                  dMedian, vMedian, nPts, s0, s1, s2, s0Sq,
                  sumAbsDev, dMin, dMax, iMin, iMax);


// Fill vector of selected moments 

   for (i=0; i<Int(selectMoments_p.nelements()); i++) {
      moments(i) = calcMoments_p(selectMoments_p(i));
      momentsMask(i) = True;
      momentsMask(i) = calcMomentsMask_p(selectMoments_p(i));
   }
}


} //# NAMESPACE CASA - END


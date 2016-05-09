//# MomentCalcBase.tcc:
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

// Base class MomentCalcBase 

template <class T> 
MomentCalcBase<T>::~MomentCalcBase()
{;}

template <class T>
void MomentCalcBase<T>::init (uInt nOutPixelsPerCollapse)
{
   AlwaysAssert (nOutPixelsPerCollapse == 1, AipsError);
}


template <class T>
uInt MomentCalcBase<T>::allNoise (T& dMean, 
                                  const Vector<T>& data,
                                  const Vector<Bool>& mask,
                                  const T peakSNR,
                                  const T stdDeviation) const
//
// Try and work out whether this spectrum is all noise
// or not.  We don't bother with it if it is noise.
// We compare the peak with sigma and a cutoff SNR
//
// Returns 1 if all noise
// Returns 2 if all masked
// Returns 0 otherwise
//
{
   T dMin, dMax;
   uInt minPos, maxPos;
   if (!this->stats(dMin, dMax, minPos, maxPos, dMean, data, mask)) return 2;


// Assume we are continuum subtracted so outside of line mean=0

   const T rat = max(abs(dMin),abs(dMax)) / stdDeviation;

   if (rat < peakSNR) {
      return 1;
   } else {
      return 0;
   }
}

template <class T>
void MomentCalcBase<T>::constructorCheck(Vector<T>& calcMoments, 
                                         Vector<Bool>& calcMomentsMask,
                                         const Vector<Int>& selectMoments,
                                         const uInt nLatticeOut) const
 {
// Number of output lattices must equal the number of moments
// the user asked to calculate

   AlwaysAssert(nLatticeOut == selectMoments.nelements(), AipsError);

// Number of requested moments must be in allowed range

   AlwaysAssert(selectMoments.nelements() <= nMaxMoments(), AipsError);
   AlwaysAssert(selectMoments.nelements() > 0, AipsError);

// Resize the vector that will hold ALL possible moments
   
   calcMoments.resize(nMaxMoments());
   calcMomentsMask.resize(nMaxMoments());
}





template <class T>
void MomentCalcBase<T>::costlyMoments(MomentsBase<T>& iMom,
                                      Bool& doMedianI,
                                      Bool& doMedianV,
                                      Bool& doAbsDev) const
{
   doMedianI = False;
   doMedianV = False;
   doAbsDev = False;
   using IM = MomentsBase<Float>;
//
   for (uInt i=0; i<iMom.moments_p.nelements(); i++) {
      if (iMom.moments_p(i) == IM::MEDIAN) doMedianI = True;
      if (iMom.moments_p(i) == IM::MEDIAN_COORDINATE) doMedianV = True;
      if (iMom.moments_p(i) == IM::ABS_MEAN_DEVIATION) doAbsDev = True;
   }      
}

template <class T>
Bool MomentCalcBase<T>::doAuto(const MomentsBase<T>& iMom) const
{
// Get it from ImageMoments private data

   return iMom.doAuto_p;
}


template <class T>
Bool MomentCalcBase<T>::doFit(const MomentsBase<T>& iMom) const
{
// Get it from ImageMoments private data

   return iMom.doFit_p;
}

template <class T>
void MomentCalcBase<T>::doCoordCalc(Bool& doCoordProfile,
                                    Bool& doCoordRandom,
                                    const MomentsBase<T>& iMom) const
//
// doCoordProfile - we need the coordinate for each pixel of the profile
// doCoordRandom  - we need the coordinate for occaisional use
//
{
// Figure out if we need to compute the coordinate of each profile pixel index
// for each profile.  This is very expensive for non-separable axes.

   doCoordProfile = False;
   doCoordRandom  = False;
   using IM = MomentsBase<Float>;
//
   for (uInt i=0; i<iMom.moments_p.nelements(); i++) {
      if (iMom.moments_p(i) == IM::WEIGHTED_MEAN_COORDINATE ||
          iMom.moments_p(i) == IM::WEIGHTED_DISPERSION_COORDINATE) {
         doCoordProfile = True;
      }
      if (iMom.moments_p(i) == IM::MAXIMUM_COORDINATE ||
          iMom.moments_p(i) == IM::MINIMUM_COORDINATE ||
          iMom.moments_p(i) == IM::MEDIAN_COORDINATE) {
         doCoordRandom = True;
      }
   }
}

template <class T>
Bool MomentCalcBase<T>::findNextDatum (uInt& iFound,
                                       const uInt& n,
                                       const Vector<Bool>& mask,
                                       const uInt& iStart,
                                       const Bool& findGood) const
//
// Find the next good (or bad) point in an array.
// A good point in the array has a non-zero value.
//
// Inputs:
//  n        Number of points in array
//  mask     Vector containing counts.  
//  iStart   The index of the first point to consider
//  findGood If True look for next good point.
//           If False look for next bad point
// Outputs:
//  iFound   Index of found point
//  Bool     False if didn't find another valid datum
{
   for (uInt i=iStart; i<n; i++) {
      if ( (findGood && mask(i)) ||
           (!findGood && !mask(i)) ) {
        iFound = i;
        return True;
      }
   }
   return False;
}


template <class T>
Bool MomentCalcBase<T>::fitGaussian (uInt& nFailed, 
                                     T& peak,
                                     T& pos,
                                     T& width,
                                     T& level,
                                     const Vector<T>& x,
                                     const Vector<T>& y,
                                     const Vector<Bool>& mask,
                                     const T peakGuess,
                                     const T posGuess,
                                     const T widthGuess,
                                     const T levelGuess) const
// 
// Fit Gaussian pos * exp(-4ln2*(x-pos)**2/width**2)
// width = fwhm
// 
// Returns false if fit fails or all masked
//
{

// Select unmasked pixels

   uInt j = 0;
   Vector<T> xSel(y.nelements());
   Vector<T> ySel(y.nelements());
   for (uInt i=0; i<y.nelements(); i++) {
     if (mask(i)) {
       xSel(j) = x(i);
       ySel(j) = y(i);
       j++;
     }
   }
   uInt nPts = j;
   if (nPts == 0) return False;

   xSel.resize(nPts,True);
   ySel.resize(nPts,True);

      
// Create fitter

   NonLinearFitLM<T> fitter;

// Create and set the functionals

   Gaussian1D<AutoDiff<T> > gauss; 
   Polynomial<AutoDiff<T> > poly;  
   CompoundFunction<AutoDiff<T> > func;
   func.addFunction(gauss);
   func.addFunction(poly);         
   
   fitter.setFunction(func);
   
   
// Initial guess

   Vector<T> v(4);
   v(0) = peakGuess;             // peak
   v(1) = posGuess;              // position
   v(2) = widthGuess;            // width
   v(3) = levelGuess;            // level
  
   fitter.setParameterValues(v);
   
   
// Set maximum number of iterations to 50.  Default is 10

   fitter.setMaxIter(50);


// Set converge criteria.

   T tol = 0.001;
   fitter.setCriteria(tol);
                                   

// Perform fit on unmasked data

   Vector<T> resultSigma(nPts);
   Vector<T> solution;
   resultSigma = 1;
   try {
     solution = fitter.fit(xSel, ySel, resultSigma);
   } catch (AipsError x1) {
      nFailed++;
      return False;
   } 


// Return values of fit
   
   peak  = solution(0);
   pos   = solution(1);
   width = abs(solution(2));
   level = solution(3);

// Return status

   if (!fitter.converged()) nFailed++;
   return fitter.converged();
                                   
}

template <class T>
Bool MomentCalcBase<T>::getAutoGaussianFit (uInt& nFailed,
                                            Vector<T>& gaussPars,
                                            const Vector<T>& x,
                                            const Vector<T>& y,
                                            const Vector<Bool>& mask,
                                            const T peakSNR,
                                            const T stdDeviation
                                            ) const
//
// Automatically fit a Gaussian and return the Gaussian parameters.
// If a plotting device is active, we also plot the spectra and fits
//
// Inputs:
//   x,y        Vector containing the data
//   mask       True is good
//   plotter    Plot spectrum and optionally the  window
//   x,yLabel   Labels
//   title
// Input/output
//   nFailed    Cumulative number of failed fits
// Output:
//   gaussPars  The gaussian parameters, peak, pos, fwhm
//   Bool       If False then this spectrum has been rejected (all
//              masked, all noise, failed fit)
//
{
    
// See if this spectrum is all noise.  If so, forget it.
// Return straight away if all masked
   
   T dMean;
   uInt iNoise = this->allNoise(dMean, y, mask, peakSNR, stdDeviation);
   if (iNoise == 2) return False;
 
   if (iNoise==1) {
      gaussPars = 0;  
      return False;
   }

// Work out guesses for Gaussian

   T peakGuess, posGuess, widthGuess, levelGuess;
   T pos, width, peak, level;
   if (!getAutoGaussianGuess(peakGuess, posGuess, widthGuess, 
                             levelGuess, x, y, mask)) return False;
   peakGuess = peakGuess - levelGuess;


// Fit gaussian. Do it twice.
  
   if (!fitGaussian (nFailed, peak, pos, width, level, x, y, mask, peakGuess, 
                     posGuess, widthGuess, levelGuess)) {
      gaussPars = 0;
      return False;
   }  
   gaussPars(0) = peak;
   gaussPars(1) = pos;
   gaussPars(2) = width;
   gaussPars(3) = level;

   return True;
}



template <class T>
Bool MomentCalcBase<T>::getAutoGaussianGuess (T& peakGuess,
                                              T& posGuess,
                                              T& widthGuess,
                                              T& levelGuess,
                                              const Vector<T>& x,
                                              const Vector<T>& y,
                                              const Vector<Bool>& mask) const
//
// Make a wild stab in the dark as to what the Gaussian
// parameters of this spectrum might be
//    
// Returns False if all masked
{

// Find peak and position of peak

   uInt minPos, maxPos;
   T dMin, dMax, dMean;
   if (!this->stats(dMin, dMax, minPos, maxPos, dMean, y, mask)) return False;

   posGuess = x(maxPos);
   peakGuess = dMax;
   levelGuess = dMean;

// Nothing much is very robust.  Assume the line is reasonably
// sampled and set its width to a few pixels.  Totally ridiculous.
                                            
   widthGuess = 5;
   return True;
}

template <class T>
Bool MomentCalcBase<T>::getInterGaussianFit (uInt& nFailed,
                                             Vector<T>& gaussPars,
                                             LogIO& os,
                                             const Vector<T>& x,
                                             const Vector<T>& y,
                                             const Vector<Bool>& mask,
                                             Bool fixedYLimits,
                                             T yMinAuto,
                                             T yMaxAuto,
                                             const String& xLabel,
                                             const String& yLabel,
                                             const String& title ) const
//
// With the cursor, define a guess for a Gaussian fit,
// and do the fit over and over until they are happy.
// Then return the Gaussian parameters.
//
// Inputs:
//   x,y       The abcissa and spectrum
//   mask      Mask.  True is good.
//   x,yLabel  Labels
//   title     Title of plot
// Input/output
//   nFailed   Cumualtive number of failures in fitting
// Outputs:
//   gaussPars The gaussian parameters (peak, pos, width, level)
//   Bool      True if all successful, False if spectrum rejected
//             because all noise or all masked
//
{
   
// Get user's guess and fit until satisfied

   Bool more = True;
   Bool ditch, redo;
   Vector<Int> window(2);
   os << endl;


   while (more) {

// Get users guess for position, peak, width and fit window

      T peakGuess, posGuess, widthGuess, levelGuess, level;
      Bool reject;
      getInterGaussianGuess (peakGuess, posGuess, widthGuess, window,
                             reject, os, y.nelements()/*, plotter*/);
      if (reject) {
         gaussPars = 0;   
         return False;
      }
  

// Get guess for level and adjust peak

      T dMin, dMax, dMean;
      uInt minPos, maxPos;
      this->stats (dMin, dMax, minPos, maxPos, dMean, y, mask);
      levelGuess = dMean;
      peakGuess = peakGuess - levelGuess;

   
// Fit a Gaussian
   
      Int n = window(1) - window(0) + 1;
      Vector<T> xFit(n);
      Vector<T> yFit(n); 
      Vector<Bool> maskFit(n);
      for (Int i=0; i<n; i++) {
         xFit(i) = x(i+window(0));
         yFit(i) = y(i+window(0));
         maskFit(i) = mask(i+window(0));
      }
      T pos, width, peak;
      if (fitGaussian (nFailed, peak, pos, width, level, xFit, yFit, maskFit,
                       peakGuess, posGuess, widthGuess, levelGuess)) {
       
// Show fit

         //showGaussFit (peak, pos, width, level, x, y, mask, plotter);
      } else {
         os << LogIO::NORMAL << "Fit failed" << LogIO::POST;
      }


// Are we happy ?

   
      //plotter.message("Accept (left),  redo (middle), reject (right)");
      //getButton(ditch, redo, plotter);
      if (ditch) {
         //plotter.message("Rejecting spectrum");
         gaussPars = 0;
         return False;
      } else if (redo) {
   
      } else {
         
// OK, set parameters of fit
         
         more = False;
         gaussPars(0) = peak;
         gaussPars(1) = pos;
         gaussPars(2) = width;
         gaussPars(3) = level;
         return True;
      }
   }
   return True;
}


/*
template <class T>
void MomentCalcBase<T>::getInterGaussianGuess(T& peakGuess,
                                              T& posGuess,
                                              T& widthGuess,
                                              Vector<Int>& window,
                                              Bool& reject,
                                              LogIO& os,
                                              const Int nPts) const

//
// Use the cursor to get the user's guess for the
// Gaussian peak, position and width (fwhm)
// and fitting window 
//
{
   //plotter.message("Mark location of peak & position - click right to reject spectrum");

   Vector<Float> minMax(4);
   //minMax = plotter.qwin();
    
   
// Peak/pos first

   String str;
   Float xCurLocF = (minMax(0)+minMax(1))/2;
   Float yCurLocF = (minMax(2)+minMax(3))/2;
   Bool miss=True;
   while (miss) {
     str.upcase();
     if (str == "X") {
        miss = False;
     } else {
        miss = (xCurLocF<minMax(0) || xCurLocF>minMax(1) ||
                      yCurLocF<minMax(2) || yCurLocF>minMax(3));
     }
   }

   reject = False;
   if (str == "X") {
     reject = True;
     return;
   }

   Vector<Float> xDataF(1), yDataF(1);
   xDataF(0) = xCurLocF;
   yDataF(0) = yCurLocF;
   posGuess = convertT(xCurLocF);
   peakGuess = convertT(yCurLocF);
   
// Now FWHM
     
   os << endl;
   miss = True;   
   while (miss) {
     str.upcase();
     if (str == "X") {
        miss = False;
     } else {
        miss = (xCurLocF<minMax(0) || xCurLocF>minMax(1) ||
                      yCurLocF<minMax(2) || yCurLocF>minMax(3));
     }
   }
   if (str == "X") {
     reject = True;
   }
   xDataF(0) = xCurLocF;
   yDataF(0) = yCurLocF;
   yCurLocF = convertT(peakGuess)/2;
   widthGuess = 2*abs(posGuess-T(xCurLocF));
  
   miss=True;
   while (miss) {
     str.upcase();
     if (str == "X") {
        miss = False;
     } else {
        miss = (xCurLocF<minMax(0) || xCurLocF>minMax(1) || 
                      yCurLocF<minMax(2) || yCurLocF>minMax(3));
     }
   }
   if (str == "X") {
     reject = True;
     return;
   } else if (str == "D") {
     window(0) = 0;
     window(1) = nPts-1;
     return; 
   }
   T tX = convertF(xCurLocF);
   T tY1 = minMax(2);
   T tY2 = minMax(3);
   window(0) = Int(xCurLocF+0.5);
   
   miss = True;
   while (miss) {
     str.upcase();
     if (str == "X") {
        miss = False;
     } else {
        miss = (xCurLocF<minMax(0) || xCurLocF>minMax(1) || 
                      yCurLocF<minMax(2) || yCurLocF>minMax(3));
     }
   }
   if (str == "X") {
     reject = True;
     return;
   } else if (str == "D") {
     window(0) = 0;
     window(1) = nPts-1;
     return;
   }
   tX = convertT(xCurLocF);
   tY1 = minMax(2);
   tY2 = minMax(3);
   window(1) = Int(xCurLocF+0.5);
   Int iTemp = window(0);
   window(0) = min(iTemp, window(1));
   window(1) = max(iTemp, window(1));
   window(0) = max(0,window(0));
   window(1) = min(nPts-1,window(1));
}
   */
template <class T>
void MomentCalcBase<T>::lineSegments (uInt& nSeg,
                                      Vector<uInt>& start, 
                                      Vector<uInt>& nPts,
                                      const Vector<Bool>& mask) const
//
// Examine an array and determine how many segments
// of good points it consists of.    A good point
// occurs if the array value is greater than zero.
//
// Inputs:
//   mask  The array mask. True is good.
// Outputs:
//   nSeg  Number of segments  
//   start Indices of start of each segment
//   nPts  Number of points in segment
//
{ 
   Bool finish = False;
   nSeg = 0;
   uInt iGood, iBad;
   const uInt n = mask.nelements();
   start.resize(n);
   nPts.resize(n);
 
   for (uInt i=0; !finish;) {
      if (!findNextDatum (iGood, n, mask, i, True)) {
         finish = True;
      } else {
         nSeg++;
         start(nSeg-1) = iGood;        
   
         if (!findNextDatum (iBad, n, mask, iGood, False)) {
            nPts(nSeg-1) = n - start(nSeg-1);
            finish = True;
         } else {
            nPts(nSeg-1) = iBad - start(nSeg-1);
            i = iBad + 1;
         }
      }
   }
   start.resize(nSeg,True);
   nPts.resize(nSeg,True);
}

template <class T>
Int& MomentCalcBase<T>::momentAxis(MomentsBase<T>& iMom) const
{
   return iMom.momentAxis_p;
}

template <class T>
String MomentCalcBase<T>::momentAxisName(const CoordinateSystem& cSys,
                                         const MomentsBase<T>& iMom) const 
{
// Return the name of the moment/profile axis

   Int worldMomentAxis = cSys.pixelAxisToWorldAxis(iMom.momentAxis_p);
   return cSys.worldAxisNames()(worldMomentAxis);
}


template <class T>
uInt MomentCalcBase<T>::nMaxMoments() const
{

// Get it from ImageMoments enum

   uInt i = MomentsBase<T>::NMOMENTS;
   return i;
}


template <class T>
T& MomentCalcBase<T>::peakSNR(MomentsBase<T>& iMom) const
{
// Get it from ImageMoments private data

   return iMom.peakSNR_p;
}


template <class T>
void MomentCalcBase<T>::selectRange(Vector<T>& pixelRange,
                                    Bool& doInclude,
                                    Bool& doExclude, 
                                    MomentsBase<T>& iMom) const
{
// Get it from ImageMoments private data

   pixelRange = iMom.selectRange_p;
   doInclude = (!(iMom.noInclude_p));
   doExclude = (!(iMom.noExclude_p));
}


template <class T>
Vector<Int> MomentCalcBase<T>::selectMoments(MomentsBase<T>& iMom) const
//
// Fill the moment selection vector according to what the user requests
//
{
   using IM = MomentsBase<Float>;
   Vector<Int> sel(IM::NMOMENTS);

   uInt j = 0;
   for (uInt i=0; i<iMom.moments_p.nelements(); i++) {
      if (iMom.moments_p(i) == IM::AVERAGE) {
         sel(j++) = IM::AVERAGE;
      } else if (iMom.moments_p(i) == IM::INTEGRATED) {
         sel(j++) = IM::INTEGRATED;
      } else if (iMom.moments_p(i) == IM::WEIGHTED_MEAN_COORDINATE) {
         sel(j++) = IM::WEIGHTED_MEAN_COORDINATE;
      } else if (iMom.moments_p(i) == IM::WEIGHTED_DISPERSION_COORDINATE) {
         sel(j++) = IM::WEIGHTED_DISPERSION_COORDINATE;
      } else if (iMom.moments_p(i) == IM::MEDIAN) {
         sel(j++) = IM::MEDIAN;
      } else if (iMom.moments_p(i) == IM::STANDARD_DEVIATION) {
         sel(j++) = IM::STANDARD_DEVIATION;
      } else if (iMom.moments_p(i) == IM::RMS) {
         sel(j++) = IM::RMS;
      } else if (iMom.moments_p(i) == IM::ABS_MEAN_DEVIATION) { 
         sel(j++) = IM::ABS_MEAN_DEVIATION;
      } else if (iMom.moments_p(i) == IM::MAXIMUM) {
         sel(j++) = IM::MAXIMUM;
       } else if (iMom.moments_p(i) == IM::MAXIMUM_COORDINATE) {
         sel(j++) = IM::MAXIMUM_COORDINATE;
      } else if (iMom.moments_p(i) == IM::MINIMUM) {
         sel(j++) = IM::MINIMUM;
      } else if (iMom.moments_p(i) == IM::MINIMUM_COORDINATE) {
         sel(j++) = IM::MINIMUM_COORDINATE;
      } else if (iMom.moments_p(i) == IM::MEDIAN_COORDINATE) {
         sel(j++) = IM::MEDIAN_COORDINATE;
      }
   }
   sel.resize(j,True);
   return sel;
}


template <class T> 
void MomentCalcBase<T>::setPosLabel (String& title,
                                     const IPosition& pos) const
{  
   ostringstream oss;

   oss << "Position = " << pos+1;
   String temp(oss);
   title = temp;
}


template <class T>
void MomentCalcBase<T>::setCoordinateSystem (CoordinateSystem& cSys, 
                                             MomentsBase<T>& iMom) 
{
  cSys = iMom.coordinates() ;
}

template <class T>
void MomentCalcBase<T>::setUpCoords (const MomentsBase<T>& iMom,
                                     Vector<Double>& pixelIn,
                                     Vector<Double>& worldOut,
                                     Vector<Double>& sepWorldCoord,
                                     LogIO& os, 
                                     Double& integratedScaleFactor,
                                     const CoordinateSystem& cSys,
                                     Bool doCoordProfile, 
                                     Bool doCoordRandom) const
// 
// Input:
// doCoordProfile - we need the coordinate for each pixel of the profile
//                  and we precompute it if we can
// doCoordRandom  - we need the coordinate for occaisional use
//
// This function does two things.  It sets up the pixelIn
// and worldOut vectors needed by getMomentCoord. It also
// precomputes the vector of coordinates for the moment axis
// profile if it is separable
//
{

// Do we need the scale factor for the integrated moment

   Int axis =  iMom.momentAxis_p;
   Bool doIntScaleFactor = False;
   integratedScaleFactor = 1.0;
   for (uInt i=0; i<iMom.moments_p.nelements(); i++) {
      if (iMom.moments_p(i) == MomentsBase<Float>::INTEGRATED) {
         doIntScaleFactor = True;
         break;
      }
   }
//
   sepWorldCoord.resize(0);
   if (!doCoordProfile && !doCoordRandom && !doIntScaleFactor) return;

// Resize these vectors used for occaisional coordinate transformations

   pixelIn.resize(cSys.nPixelAxes());
   worldOut.resize(cSys.nWorldAxes());
   if (!doCoordProfile && !doIntScaleFactor) return;

// Find the coordinate for the moment axis
   
   Int coordinate, axisInCoordinate;
   cSys.findPixelAxis(coordinate, axisInCoordinate, axis);  
  
// Find out whether this coordinate is separable or not
  
   Int nPixelAxes = cSys.coordinate(coordinate).nPixelAxes();
   Int nWorldAxes = cSys.coordinate(coordinate).nWorldAxes();

// Precompute the profile coordinates if it is separable and needed
// The Integrated moment scale factor is worked out here as well so the 
// logic is a bit contorted

   Bool doneIntScale = False;      
   if (nPixelAxes == 1 && nWorldAxes == 1) {
      pixelIn = cSys_p.referencePixel();
//
      Vector<Double> frequency(iMom.getShape()(axis));
      if (doCoordProfile) {
         for (uInt i=0; i<frequency.nelements(); i++) {
            frequency(i) = getMomentCoord(iMom, pixelIn, worldOut, Double(i));
         }
      }

// If the coordinate of the moment axis is Spectral convert to km/s
// Although I could work this out here, it would be decoupled from
// ImageMoments which works the same thing out and sets the units.
// So to ensure coupling, i pass in this switch via the IM object

      if (iMom.convertToVelocity_p) {
         AlwaysAssert(cSys.type(coordinate)==Coordinate::SPECTRAL, AipsError);  // Should never fail !
//
         const SpectralCoordinate& sc = cSys.spectralCoordinate(coordinate);
         SpectralCoordinate sc0(sc);

// Convert

         sc0.setVelocity (String("km/s"), iMom.velocityType_p);
         if (doCoordProfile) {
            sc0.frequencyToVelocity (sepWorldCoord, frequency);
         }

// Find increment in world units at reference pixel if needed

         if (doIntScaleFactor) {
            Quantum<Double> vel0, vel1;
            Double pix0 = sc0.referencePixel()(0) - 0.5;
            Double pix1 = sc0.referencePixel()(0) + 0.5;
            sc0.pixelToVelocity (vel0, pix0);
            sc0.pixelToVelocity (vel1, pix1);
            integratedScaleFactor = abs(vel1.getValue() - vel0.getValue());
            doneIntScale = True;
         }
      } 
   } else {
      os << LogIO::NORMAL
           << "You have asked for a coordinate moment from a non-separable " << endl;
      os << "axis.  This means a coordinate must be computed for each pixel " << endl;
      os << "of each profile which will cause performance degradation" << LogIO::POST;
   }
//
   if (doIntScaleFactor && !doneIntScale) {

// We need the Integrated moment scale factor but the moment
// axis is non-separable

      const Coordinate& c = cSys.coordinate(coordinate);
      Double inc = c.increment()(axisInCoordinate);
      integratedScaleFactor = abs(inc*inc);
      doneIntScale = True;
   }
}

template <class T>      
Bool MomentCalcBase<T>::stats(T& dMin, 
                              T& dMax,  
                              uInt& minPos,
                              uInt& maxPos,
                              T& dMean,
                              const Vector<T>& profile,
                              const Vector<Bool>& mask) const
//
// Returns False if no unmasked points
//
{
   Bool deleteIt1, deleteIt2;
   const T* pProfile = profile.getStorage(deleteIt1);
   const Bool* pMask = mask.getStorage(deleteIt2);

   Int iStart = -1;
   uInt i = 0;
   uInt nPts = 0;
   typename NumericTraits<T>::PrecisionType sum = 0;

   while (i<profile.nelements() && iStart==-1) {
      if (pMask[i]) {
        dMax = pProfile[i];
        dMin = dMax;
        minPos = i;
        maxPos = i;
        sum = pProfile[i];
        nPts++;
        iStart = i+1;
      }
      i++;
   }
   if (iStart == -1) return False;

   for (i=iStart; i<profile.nelements(); i++) {
      if (pMask[i]) {
         dMin = min(dMin,pProfile[i]);
         dMax = max(dMax,pProfile[i]);
         minPos = i;
         maxPos = i;
         sum += pProfile[i];
         nPts++;
      }
   }
   dMean = sum / nPts;
   profile.freeStorage(pProfile, deleteIt1);
   mask.freeStorage(pMask, deleteIt2);

   return True;  
}


template <class T>
T& MomentCalcBase<T>::stdDeviation(MomentsBase<T>& iMom) const
{
   return iMom.stdDeviation_p;
}
      
/*
template <class T>
void MomentCalcBase<T>::yAutoMinMax(T& yMin, 
                                    T& yMax, 
                                    MomentsBase<T>& iMom) const
{
   yMin = iMom.yMin_p;
   yMax = iMom.yMax_p;
}
 */



// Fill the ouput moments array
template<class T>
void MomentCalcBase<T>::setCalcMoments
                       (const MomentsBase<T>& iMom,
                        Vector<T>& calcMoments,
                        Vector<Bool>& calcMomentsMask,
                        Vector<Double>& pixelIn,
                        Vector<Double>& worldOut,
                        Bool doCoord,
                        Double integratedScaleFactor,
                        T dMedian,
                        T vMedian,
                        Int nPts,
                        typename NumericTraits<T>::PrecisionType s0,
                        typename NumericTraits<T>::PrecisionType s1,
                        typename NumericTraits<T>::PrecisionType s2,
                        typename NumericTraits<T>::PrecisionType s0Sq,
                        typename NumericTraits<T>::PrecisionType sumAbsDev,
                        T dMin,
                        T dMax,
                        Int iMin,
                        Int iMax) const
//
// Fill the moments vector
//
// Inputs
//   integratedScaleFactor  width of a channel in km/s or Hz or whatever
// Outputs:
//   calcMoments The moments
//
{
// Short hand to fish ImageMoments enum values out   
// Despite being our friend, we cannot refer to the
// enum values as just, say, "AVERAGE"
     
   using IM = MomentsBase<Float>;
           
// Normalize and fill moments

   calcMomentsMask = True;
   calcMoments(IM::AVERAGE) = s0 / nPts;
   calcMoments(IM::INTEGRATED) = s0 * integratedScaleFactor; 
   if (abs(s0) > 0.0) {
      calcMoments(IM::WEIGHTED_MEAN_COORDINATE) = s1 / s0;
//
      calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE) = 
        (s2 / s0) - calcMoments(IM::WEIGHTED_MEAN_COORDINATE) *
                    calcMoments(IM::WEIGHTED_MEAN_COORDINATE);
      calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE) =
         abs(calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE));
      if (calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE) > 0.0) {
         calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE) =
            sqrt(calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE));
      } else {
         calcMoments(IM::WEIGHTED_DISPERSION_COORDINATE) = 0.0;
         calcMomentsMask(IM::WEIGHTED_DISPERSION_COORDINATE) = False;
      }
   } else {
      calcMomentsMask(IM::WEIGHTED_MEAN_COORDINATE) = False;
      calcMomentsMask(IM::WEIGHTED_DISPERSION_COORDINATE) = False;
   }

// Standard deviation about mean of I
                 
   if (nPts>1 && Float((s0Sq - s0*s0/nPts)/(nPts-1)) > 0) {
      calcMoments(IM::STANDARD_DEVIATION) = sqrt((s0Sq - s0*s0/nPts)/(nPts-1));
   } else {
      calcMoments(IM::STANDARD_DEVIATION) = 0;
      calcMomentsMask(IM::STANDARD_DEVIATION) = False;
   }

// Rms of I

   calcMoments(IM::RMS) = sqrt(s0Sq/nPts);
     
// Absolute mean deviation

   calcMoments(IM::ABS_MEAN_DEVIATION) = sumAbsDev / nPts;

// Maximum value

   calcMoments(IM::MAXIMUM) = dMax;
                                      
// Coordinate of min/max value

   if (doCoord) {
      calcMoments(IM::MAXIMUM_COORDINATE) = getMomentCoord(
    		  iMom, pixelIn, worldOut, Double(iMax),
    		  iMom.convertToVelocity_p
      );
      calcMoments(IM::MINIMUM_COORDINATE) = getMomentCoord(
    		  iMom, pixelIn, worldOut, Double(iMin),
    		  iMom.convertToVelocity_p
      );
   } else {
      calcMoments(IM::MAXIMUM_COORDINATE) = 0.0;
      calcMoments(IM::MINIMUM_COORDINATE) = 0.0;
      calcMomentsMask(IM::MAXIMUM_COORDINATE) = False;
      calcMomentsMask(IM::MINIMUM_COORDINATE) = False;
   }

// Minimum value
   calcMoments(IM::MINIMUM) = dMin;

// Medians

   calcMoments(IM::MEDIAN) = dMedian;
   calcMoments(IM::MEDIAN_COORDINATE) = vMedian;
}

} //# NAMESPACE CASA - END


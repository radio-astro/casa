//# Cleaner.h: this defines Cleaner a class for doing convolution
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
//#
//# $Id: LatticeCleaner.h 20739 2009-09-29 01:15:15Z Malte.Marquarding $

#ifndef SYNTHESIS_MATRIXCLEANER_H
#define SYNTHESIS_MATRIXCLEANER_H

//# Includes
#include <casa/aips.h>
#include <casa/Quanta/Quantum.h>
#include <lattices/Lattices/TempLattice.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <lattices/Lattices/LatticeCleaner.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
template <class T> class Matrix;

// <summary>A copy of LatticeCleaner but just using 2-D matrices</summary>
// <synopsis> It is a mimic of the LatticeCleaner class but avoid a lot of 
// of the lattice to array and back copies and uses openmp in the obvious places
// </synopsis>

// <summary>A class for doing multi-dimensional cleaning</summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="tLatticeCleaner">
// </reviewed>

// <prerequisite>
//  <li> The mathematical concept of deconvolution
// </prerequisite>
//
// <etymology>

// The MatrixCleaner class will deconvolve 2-D arrays of floats.

// </etymology>
//
// <synopsis>
// This class will perform various types of Clean deconvolution
// on Lattices.
//
// </synopsis>
//
// <example>
// <srcblock>
// </srcblock> 
// </example>
//
// <motivation>
// </motivation>
//
// <thrown>
// <li> AipsError: if psf has more dimensions than the model. 
// </thrown>
//
// <todo asof="yyyy/mm/dd">
// </todo>

class MatrixCleaner
{
public:

  // Create a cleaner : default constructor
  MatrixCleaner();

  // Create a cleaner for a specific dirty image and PSF
  MatrixCleaner(const Matrix<Float> & psf, const Matrix<Float> & dirty);

  // The copy constructor uses reference semantics
  MatrixCleaner(const MatrixCleaner& other);

  // The assignment operator also uses reference semantics
  MatrixCleaner & operator=(const MatrixCleaner & other); 

  // The destructor does nothing special.
  ~MatrixCleaner();
 
  //just define the scales...nothing else is done
  //the user will need to call setPsf+makePsfScales+setDirty+makeDirtyScales
  //to be in a good state to clean.
  void defineScales(const Vector<Float>& scales);
  
  //Set the dirty image without calculating convolutions..
  //can be done by calling  makeDirtyScales or setscales if one want to redo the 
  //psfscales too.
  void setDirty(const Matrix<Float>& dirty);
  //Calculate the convolutions for the dirt
  //Obviously the 
  void makeDirtyScales();
  // Update the dirty image only (equiv of setDirty + makeDirtyScales)
  void update(const Matrix<Float> & dirty);

  //change the psf
  //don't forget to redo the setscales or run makePsfScales, 
  //followed by makeDirtyScales 
  void setPsf(const Matrix<Float>& psf);
  //calculate the convolutions of the psf 
  void makePsfScales();
  
  // Set a number of scale sizes. The units of the scale are pixels.
  // The 2 functions below assume you have the dirty image and the psf set
  // the convolutions are calculated automatically and the masks ones too 
  // if it is set ....kept so as to be compatible function wise with LatticeCleaner
  Bool setscales(const Int nscales, const Float scaleInc=1.0);

  // Set a specific set of scales
  Bool setscales(const Vector<Float> & scales);

  
 
  // Set up control parameters
  // cleanType - type of the cleaning algorithm to use (HOGBOM, MULTISCALE)
  // niter - number of iterations
  // gain - loop gain used in cleaning (a fraction of the maximum 
  //        subtracted at every iteration)
  // aThreshold - absolute threshold to stop iterations
  // fThreshold - fractional threshold (i.e. given w.r.t. maximum residual)
  //              to stop iterations. This parameter is specified as
  //              Quantity so it can be given in per cents. 
  // choose - unused at the moment, specify False. Original meaning is
  // to allow interactive decision on whether to continue iterations.
  // This method always returns True.
  Bool setcontrol(CleanEnums::CleanType cleanType, const Int niter,
		  const Float gain, const Quantity& aThreshold,
		  const Quantity& fThreshold);

  // This version of the method disables stopping on fractional threshold
  Bool setcontrol(CleanEnums::CleanType cleanType, const Int niter,
		  const Float gain, const Quantity& threshold);

  // return how many iterations we did do
  Int iteration() const { return itsIteration; }
  Int numberIterations() const { return itsIteration; }

  // what iteration number to start on
  void startingIteration(const Int starting = 0) {itsStartingIter = starting; }

  // Clean an image. 
  //return value gives you a hint of what's happening
  //  1 = converged
  //  0 = not converged but behaving normally
  // -1 = not converged and stopped on cleaning consecutive smallest scale
  // -2 = not converged and either large scale hit negative or diverging 
  // -3 = clean is diverging rather than converging 
  Int clean(Matrix<Float> & model, Bool doPlotProgress=False);

  // Set the mask
  // mask - input mask lattice
  // maskThreshold - if positive, the value is treated as a threshold value to determine
  // whether a pixel is good (mask value is greater than the threshold) or has to be 
  // masked (mask value is below the threshold). Negative threshold switches mask clipping
  // off. The mask value is used to weight the flux during cleaning. This mode is used
  // to implement cleaning based on the signal-to-noise as opposed to the standard cleaning
  // based on the flux. The default threshold value is 0.9, which ensures the behavior of the
  // code is exactly the same as before this parameter has been introduced.
  void setMask(Matrix<Float> & mask, const Float& maskThreshold = 0.9);
  // Call the function below if the psf is changed ..no need to setMask again
  Bool makeScaleMasks();

  // remove the mask;
  // useful when keeping object and sending a new dirty image to clean 
  // one can set another mask then 
  void unsetMask();

  // Tell the algorithm to NOT clean just the inner quarter
  // (This is useful when multiscale clean is being used
  // inside a major cycle for MF or WF algorithms)
  // if True, the full image deconvolution will be attempted
  void ignoreCenterBox(Bool huh) { itsIgnoreCenterBox = huh; }

  // Consider the case of a point source: 
  // the flux on all scales is the same, and the first scale will be chosen.
  // Now, consider the case of a point source with a *little* bit of extended structure:
  // thats right, the largest scale will be chosen.  In this case, we should provide some
  // bias towards the small scales, or against the large scales.  We do this in
  // an ad hoc manner, multiplying the maxima found at each scale by
  // 1.0 - itsSmallScaleBias * itsScaleSizes(scale)/itsScaleSizes(nScalesToClean-1);
  // Typical bias values range from 0.2 to 1.0.
  void setSmallScaleBias(const Float x=0.5) { itsSmallScaleBias = x; }

  // During early iterations of a cycled MS Clean in mosaicing, it common
  // to come across an ocsilatory pattern going between positive and
  // negative in the large scale.  If this is set, we stop at the first
  // negative in the largest scale.
  void stopAtLargeScaleNegative() {itsStopAtLargeScaleNegative = True; }

  // Some algorithms require that the cycles be terminated when the image
  // is dominated by point sources; if we get nStopPointMode of the
  // smallest scale components in a row, we terminate the cycles
  void stopPointMode(Int nStopPointMode) {itsStopPointMode = nStopPointMode; }

  // After completion of cycle, querry this to find out if we stopped because
  // of stopPointMode
  Bool queryStopPointMode() const {return itsDidStopPointMode; }

  // speedup() will speed the clean iteration by raising the
  // threshold.  This may be required if the threshold is
  // accidentally set too low (ie, lower than can be achieved
  // given errors in the approximate PSF).
  //
  // threshold(iteration) = threshold(0) 
  //                        * ( exp( (iteration - startingiteration)/Ndouble )/ 2.718 )
  // If speedup() is NOT invoked, no effect on threshold
  void speedup(const Float Ndouble);

  // Look at what WE think the residuals look like
  // Assumes the first scale is zero-sized
  Matrix<Float>  residual() { return itsDirtyConvScales[0]; }

  // Method to return threshold, including any speedup factors
  Float threshold() const;

  // Method to return the strength optimum achieved at the last clean iteration
  // The output of this method makes sense only if it is called after clean
  Float strengthOptimum() const { return itsStrengthOptimum; }

  // Helper function to optimize adding
  //static void addTo(Matrix<Float>& to, const Matrix<Float> & add);

protected:
  // Make sure that the peak of the Psf is within the image
  Bool validatePsf(const Matrix<Float> & psf);

  // Make an array of the specified scale
  void makeScale(Matrix<Float>& scale, const Float& scaleSize);
  
  // Make Spheroidal function for scale images
  Float spheroidal(Float nu);
  
  // Find the Peak of the matrix
  static Bool findMaxAbs(const Matrix<Float>& lattice,
                         Float& maxAbs, IPosition& posMax);

  // Find the Peak of the lattice, applying a mask
  Bool findMaxAbsMask(const Matrix<Float>& lattice, const Matrix<Float>& mask,
                             Float& maxAbs, IPosition& posMax);

  // Helper function to reduce the box sizes until the have the same   
  // size keeping the centers intact  
  static void makeBoxesSameSize(IPosition& blc1, IPosition& trc1,                               
     IPosition &blc2, IPosition& trc2);


  CleanEnums::CleanType itsCleanType;
  Float itsGain;
  Int itsMaxNiter;      // maximum possible number of iterations
  Quantum<Double> itsThreshold;
  CountedPtr<Matrix<Float> > itsMask;
  IPosition itsPositionPeakPsf;
private:

  //# The following functions are used in various places in the code and are
  //# documented in the .cc file. Static functions are used when the functions
  //# do not modify the object state. They ensure that implicit assumptions
  //# about the current state and implicit side-effects are not possible
  //# because all information must be supplied in the input arguments


  CountedPtr<Matrix<Float> > itsDirty;
  CountedPtr<Matrix<Complex> >itsXfr;

  Int itsNscales;
  Vector<Float> itsScaleSizes;

  Block<Matrix<Float> > itsScales;
  Block<Matrix<Complex> > itsScaleXfrs;
  Block<Matrix<Float> > itsPsfConvScales;
  Block<Matrix<Float> > itsDirtyConvScales;
  Block<Matrix<Float> > itsScaleMasks;

  Bool itsScalesValid;

  Int itsIteration;	// what iteration did we get to?
  Int itsStartingIter;	// what iteration did we get to?
  Quantum<Double> itsFracThreshold;

  Float itsMaximumResidual;
  Float itsStrengthOptimum;


  Vector<Float> itsTotalFluxScale;
  Float itsTotalFlux;

  // Memory to be allocated per TempLattice
  Double itsMemoryMB;

  // Let the user choose whether to stop
  Bool itsChoose;

  // Threshold speedup factors:
  Bool  itsDoSpeedup;  // if false, threshold does not change with iteration
  Float itsNDouble;

  //# Stop now?
  //#//  Bool stopnow();   Removed on 8-Apr-2004 by GvD

  // Calculate index into PsfConvScales
  Int index(const Int scale, const Int otherscale);
  
  Bool destroyScales();
  Bool destroyMasks();


  
  Bool itsIgnoreCenterBox;
  Float itsSmallScaleBias;
  Bool itsStopAtLargeScaleNegative;
  Int itsStopPointMode;
  Bool itsDidStopPointMode;
  Bool itsJustStarting;

  // threshold for masks. If negative, mask values are used as weights and no pixels are
  // discarded (although effectively they would be discarded if the mask value is 0.)
  Float itsMaskThreshold;
  IPosition psfShape_p;

};

} //# NAMESPACE CASA - END

#endif

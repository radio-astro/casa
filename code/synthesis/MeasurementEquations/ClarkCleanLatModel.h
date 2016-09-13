//# ClarkCleanLatModel.h: this defines ClarkCleanLatModel
//# Copyright (C) 1996,1997,1998,1999,2000,2003
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
//# $Id$

#ifndef SYNTHESIS_CLARKCLEANLATMODEL_H
#define SYNTHESIS_CLARKCLEANLATMODEL_H

#include <casa/aips.h>
#include <synthesis/MeasurementEquations/LinearModel.h>
#include <synthesis/MeasurementEquations/LinearEquation.h>
#include <lattices/Lattices/Lattice.h>
#include <casa/Arrays/IPosition.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <casa/Logging/LogIO.h>

namespace casa { //# NAMESPACE CASA - BEGIN

template <class T> class Matrix;
template <class T> class Vector;
class ClarkCleanProgress;
class LatConvEquation;
class CCList;

// <summary>
// A Class for performing the Clark Clean Algorithm on Arrays
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite> 
// <li> ResidualEquation/LatConvEquation 
// <li> LinearModel/LinearEquation Paradigm 
// </prerequisite>
//
// <etymology>
// This class is called ClarkCleanLatModel because thats the algorithm it uses
// deconvolve the lattice-based model. 
// </etymology>
//
// <synopsis>
// This class is used to perform the Clark Clean Algorithm on an
// Array. Only the deconvolved model of the sky are directly stored by this
// class. The point spread function (psf) and convolved (dirty) image are
// stored in a companion class which is must be derived from
// ResidualEquation. 
// 
// The cleaning works like this. The user constructs a ClarkCleanLatModel by
// specifying an initial model of the sky. This can by be
// one,two,three... dimensional depending on the dimension of the psf (see
// below). The user then constructs a class which implements the forward
// equation between the model and the dirty image. Typically this will be
// the ConvolutionEquation class, although any class which has a
// ResidualEquation interface will be work (but perhaps very slowly, as the
// ConvolutionEquation class has member functions optimised for cleaning)
//
// The user then calls the solve() function (with the appropriate equation
// class as an arguement), and this class will perform the Clark clean.
// The various clean parameters are set (prior to calling solve) using the
// functions derived from the Iterate class, in particular setGain(),
// setNumberIterations() & setThreshold() (to set a flux limit). 
// 
// The solve() function does not return either the deconvolved model or the
// residuals. The solved model can be obtained using the getModel() function
// (derived from ArrayModel()) and the residual can be obtained using the
// residual() member function of the Convolution/Residual Equation Class.
// 
// The size and shape of the model used in this class MUST be the same as
// the convolved data (Dirty Image), stored in the companion
// ResidualEquation Class. However the model (and convolved data) can have
// more dimensions than the psf, as well as a different size (either larger
// or smaller). When the dimensionality is different the cleaning is done
// independendtly in each "plane" of the model. (Note this has not
// been implemented yet but is relatively simple to do if necessary). 
//
// This multi-dimensionalty is exploited when cleaning arrays of
// StokesVectors. Here the Array of StokesVectors is decomposed into a stack
// of 4 Floating point arrays and the cleaning is done on all the the arrays
// simultaneosly. The criterion for choosing the brightest pixel has been
// generalised by using the "length" of the Stokesvector in 4 dimensional
// space. 
//
// A companion class to this one is MaskedClarkCleanLatModel. This provides
// the same functionality but is used with MaskedArrays which indicate which
// regions of the model to search for clean components. 
//
// </synopsis>
//
// <example>
// <srcblock>
// Matrix<Float> psf(12,12), dirty(10,10), initialModel(10,10);
// ...put appropriate values into psf, dirty, & initialModel....
// ClarkCleanLatModel<Float> deconvolvedModel(initialModel); 
// ConvolutionEquation convEqn(psf, dirty);
// deconvolvedModel.setGain(0.2); 
// deconvolvedModel.setNumberIterations(1000);
// Bool convWorked = deconvolvedModel.solve(convEqn);
// if (convWorked)
//   ConvEqn.residual(deconvolvedModel, finalResidual);
// </srcblock> 
// </example>
//
// <motivation>
// This class is needed to deconvolve images.
// </motivation>
//
// <templating arg=T>
// I have tested this class with Arrays of
//    <li> Float
//    <li> StokesVector
// </templating>
//
// <todo asof="1996/05/02">
//   <li> Make changes so that multidimensions work as advertised
//   <li> compare timing with other clean implementations (ie, Mark's
//   CleanTools, SDE, AIPS & miriad) 
// </todo>

class ClarkCleanLatModel: 
  public LinearModel< Lattice<Float> >,
  public Iterate
{
public:
  // The default constructor does nothing more than initialise a zero length
  // array to hold the deconvolved model. If this constructor is used then 
  // the actual model must be set using the setModel() function of the
  // LatticeModel class.
  ClarkCleanLatModel();

  // Construct the ClarkCleanLatModel object and initialise the model.
  ClarkCleanLatModel(Lattice<Float> & model);

  // Construct the ClarkCleanLatModel object and initialise the model ans mask
  ClarkCleanLatModel(Lattice<Float> & model, Lattice<Float> & mask);

  // Construct the ClarkCleanLatModel object and initialise the model ans mask
  ClarkCleanLatModel(Lattice<Float> & model, Lattice<Float> & residual, 
		     Lattice<Float> & mask);
  // Destroy!
  virtual ~ClarkCleanLatModel();

  virtual const Lattice<Float> & getModel() const { return *itsModelPtr; }
  virtual void setModel(const Lattice<Float> & model);
  virtual void setModel(Lattice<Float> & model);

  const Lattice<Float> & getMask() const;
  void setMask(const Lattice<Float> & mask);

  void setResidual( Lattice<Float> & residual);
  virtual const Lattice<Float> & getResidual() const { return *itsResidualPtr; }

  Int getNumberIterations(){ return numberIterations(); }

  Float getMaxResidual() { return itsMaxRes;};
  // Using a Clark clean deconvolution proceedure solve for an improved
  // estimate of the deconvolved object. The convolution/residual equation
  // contains the psf and dirty image. When called with a ResidualEquation
  // arguement a quite general interface is used that is slow. The
  // convolution equation contains functions that speed things up. The
  // functions return False if the deconvolution could not be done.
  // <group>
  Bool solve(LatConvEquation & eqn);
  Bool singleSolve(LatConvEquation & eqn, Lattice<Float> & residual);
  // </group>

  // The user can be asked whether to stop after every minor cycle
  // <group>
  virtual void setChoose(const Bool askForChoice);
  virtual Bool getChoose();
  // </group>

  // These remaining functions set various "knobs" that the user can tweak and
  // are specific to the Clark clean algorithm. The more generic parameters
  // ie. clean gain, and maximum residual fluxlimit, are set using functions in
  // the Iterate base class. The get functions return the value that was
  // actually used after the cleaning was done.

  // set the size of the PSF used in the minor iterations. If not set it
  // defaults to the largest useful Psf (ie. min(modelSize*2, psfSize))
  // <group>
  virtual void setPsfPatchSize(const IPosition & psfPatchSize); 
  virtual IPosition getPsfPatchSize(); 
  // </group>

  // Set the size of the histogram used to determine how many pixels are
  // "active" in a minor iteration. Default value of 1000 is OK for
  // everything except very small cleans.
  // <group>
  virtual void setHistLength(const uInt histBins); 
  virtual uInt getHistLength();
  // </group>
 
  // Set the maximum number of minor iterations to perform for each major
  // cycle. 
  // <group>
  virtual void setMaxNumberMinorIterations(const uInt maxNumMinorIterations); 
  virtual uInt getMaxNumberMinorIterations();
  // </group>

  // Set and get the initial number of iterations
  // <group>
  virtual void setInitialNumberIterations(const uInt initialNumberIterations); 
  virtual uInt getInitialNumberIterations();
  // </group>

  // Set the maximum number of major cycles to perform
  // <group>
  virtual void setMaxNumberMajorCycles(const uInt maxNumMajorCycles); 
  virtual uInt getMaxNumberMajorCycles();
  // </group>

  // Set the maximum number of active pixels to use in the minor
  // iterations. The specified number can be exceeded if the topmost bin of
  // the histogram contains more pixels than specified here. The default is
  // 10,000 which is suitable for images of 512by512 pixels. Reduce this for
  // smaller images and increase it for larger ones. 
  // <group>
  virtual void setMaxNumPix(const uInt maxNumPix ); 
  virtual uInt getMaxNumPix(); 
  // </group>


  // Set the maximum exterior psf value. This is used to determine when to
  // stop the minor itartions. This is normally determined from the psf and
  // the number set here is only used if this cannot be determined. The
  // default is zero.
  // <group>
  virtual void setMaxExtPsf(const Float maxExtPsf ); 
  virtual Float getMaxExtPsf(); 
  // </group>

  // The total flux density in the model.
  Float modelFlux();

  // An exponent on the F(m,n) factor (see Clark[1980]) which influences how
  // quickly active pixels are treated as unreliable. Larger values mean
  // more major iterations. The default is zero. I have no experience on
  // when to use this factor.
  // <group>
  virtual void setSpeedup(const Float speedup); 
  virtual Float getSpeedup();
  // </group>
  //Set the cycle factor....the larger this is the shallower is the minor
  //cycle
  virtual void setCycleFactor(const Float factor);


  // Set/get the progress display 
  // <group>
  virtual void setProgress(ClarkCleanProgress& ccp) { itsProgressPtr = &ccp; }
  virtual ClarkCleanProgress& getProgress() { return *itsProgressPtr; }
  // </group>

private:
// Do all the minor iterations for one major cycle. Cleaning stops
// when the flux or iteration limit is reached.
  void doMinorIterations(CCList & activePixels,
			 Matrix<Float> & psfPatch,
			 Float fluxLimit, 
			 uInt & numberIterations, 
			 Float Fmn, 
			 const uInt totalIterations,
			 Float& totalFlux);

  void cacheActivePixels(CCList & activePixels, 
			const Lattice<Float> & residual, Float fluxLimit);

  // make histogram of absolute values in array
  void absHistogram(Vector<Int> & hist, Float & minVal, 
		    Float & maxVal, const Lattice<Float> & data);

  // Determine the flux limit if we only select the maxNumPix biggest
  // residuals. Flux limit is not exact due to quantising by the histogram
  Float biggestResiduals(Float & maxRes, const uInt maxNumPix, 
			 const Float fluxLimit, 
			 const Lattice<Float> & residual);

// Work out the size of the Psf patch to use. 
  Float getPsfPatch(Matrix<Float> & psfPatch, LatConvEquation & eqn);

// The maximum residual is the absolute maximum.
  Float maxResidual(const Lattice<Float> & residual);
  void maxVect(Block<Float> & maxVal, Float & absVal, Int & offset,
	       const CCList & activePixels);
  void subtractComponent(CCList & activePixels, const Block<Float> & maxVal,
			 const Block<Int> & maxPos, const Matrix<Float> & psf);
  Float absMaxBeyondDist(const IPosition & maxDist, const IPosition & centre,
			 const Lattice<Float> & psf);
  Bool stopnow();
  Int  getbig(Float const * pixValPtr, Int const * pixPosPtr, const Int nPix, 
	      const Float fluxLimit, 
	      const Float * const residualPtr, const Float * const maskPtr, 
	      const uInt npol, const Int nx, const Int ny);

  void updateModel(CCList & cleanComponents);

  Lattice<Float> * itsModelPtr;
  Lattice<Float> * itsResidualPtr;
  const Lattice<Float> * itsSoftMaskPtr;
  uInt itsMaxNumPix;
  uInt itsHistBins;
  uInt itsMaxNumberMinorIterations;
  uInt itsInitialNumberIterations;
  Int itsMaxNumberMajorCycles;
  Float itsMaxExtPsf;
  Float itsMaxRes;
  IPosition itsPsfPatchSize;
  Bool itsChoose;
  Float itsSpeedup;
  Float itsCycleFactor;
  LogIO itsLog;
  ClarkCleanProgress* itsProgressPtr;
  Bool itsJustStarting;
  Bool itsWarnFlag;
};


} //# NAMESPACE CASA - END

#endif

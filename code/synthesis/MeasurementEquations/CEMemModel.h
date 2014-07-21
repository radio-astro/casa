//# CEMemModel.h: this defines CEMemModel
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef SYNTHESIS_CEMEMMODEL_H
#define SYNTHESIS_CEMEMMODEL_H


#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <lattices/Lattices/Lattice.h>
#include <images/Images/PagedImage.h>
#include <synthesis/MeasurementEquations/Entropy.h>
#include <synthesis/MeasurementEquations/LinearEquation.h>
#include <synthesis/MeasurementEquations/LinearModel.h>
#include <casa/Logging/LogIO.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declaration
class LatConvEquation;
class CEMemProgress;


// <summary> Implements the Cornwell & Evans MEM Algorithm on Lattices </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite> 
// <li> ResidualEquation/ConvolutionEquation 
// <li> LinearModel/LinearEquation Paradigm 
// </prerequisite>
//
// <etymology>
// This class is called CEMemModel because it uses the Cornwell and
// Evans MEM algorithm to deconvolve the model. 
// </etymology>
//
// <synopsis>
// This class is used to perform the  Cornwell and Evans MEM Algorithm on an
// Array. Only the deconvolved model of the sky are directly stored by this
// class. The point spread function (psf) and convolved (dirty) image are
// stored in a companion class which is must be derived from
// ResidualEquation. 
// 
// The deconvolution works like this. The user constructs a CEMemModel by
// specifying an initial model of the sky. This can by be
// one,two,three... dimensional depending on the dimension of the psf (see
// below). The user then constructs a class which implements the forward
// equation between the model and the dirty image. Typically this will be
// the ConvolutionEquation class, although any class which has a
// ResidualEquation interface will work (but perhaps very slowly, as the
// ConvolutionEquation class has member functions optimised for CLEAN and MEM)
//
// The user then calls the solve() function (with the appropriate equation
// class as an arguement), and this class will perform the MEM deconvolution.
// The various MEM parameters are set (prior to calling solve) using the
// functions derived from the Iterate class, in particular
// setNumberIterations().
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
// or smaller). When the dimensionality is different the deconvolution is done
// independendtly in each "plane" of the model. (Note this has not
// been implemented yet but is relatively simple to do if necessary). 
//
// StokesVectors are not yet implemented.
//
// A companion class to this one is MaskedCEMemModel. This provides
// the same functionality but is used with MaskedArrays which indicate which
// regions of the NewtonRaphson (residual) image to apply when forming the
// step image (MaskedCEMemModel is not yet implemented).
//
// </synopsis>
//
// <example>
// <srcblock>
// Matrix<Float> psf(12,12), dirty(10,10), initialModel(10,10);
// ...put appropriate values into psf, dirty, & initialModel....
// CEMemModel<Float> deconvolvedModel(initialModel); 
// ConvolutionEquation convEqn(psf, dirty);
// deconvolvedModel.setSigma(0.001); 
// deconvolvedModel.setTargetFlux(-2.500); 
// deconvolvedModel.setNumberIterations(30);
// Bool convWorked = deconvolvedModel.solve(convEqn);
// Array<Float> finalModel, residuals;
// if (convWorked){
//   finalModel = deconvolvedModel.getModel();
//   ConvEqn.residual(deconvolvedModel, finalResidual);
// }
// </srcblock> 
// </example>
//
// <motivation>
// This class is needed to deconvolve extended images.  
// In time, the MEM algorithm will be a principle player in the 
// mosaicing stuff.
// </motivation>
//
// <templating arg=T>
//    For testing:
//    <li> Float: lets try it first
//    <li> StokesVector: will require lots more work
// </templating>
//
// <todo asof="1998/12/02">
//   <li> We need to implement soft Masking
// </todo>

class CEMemModel: public LinearModel< Lattice<Float> > 
{

  // Any new entropies derived from Entropy must sign the friend list:
friend class Entropy;
friend class EntropyI;
friend class EntropyEmptiness;


public:

  // Construct the CEMemModel object and initialise the model.
  CEMemModel(Entropy &ent, 
	     Lattice<Float> & model,
	     uInt nIntegrations = 10,
	     Float sigma = 0.001,
	     Float targetFlux = 1.0,
	     Bool useFluxConstraint = False,
	     Bool initializeModel = True,
	     Bool imagePlane = False);

  // Construct the CEMemModel object, initialise the model and Prior images.
  CEMemModel(Entropy &ent, 
	     Lattice<Float> & model,
	     Lattice<Float> & prior,
	     uInt nIntegrations = 10,
	     Float sigma = 0.001,
	     Float targetFlux = 1.0,
	     Bool useFluxConstraint = False,
	     Bool initializeModel = True,
	     Bool imagePlane = False);

  // Construct the CEMemModel object, initialise the model, Prior,
  // and mask images.
  CEMemModel(Entropy &ent, 
	     Lattice<Float> & model, 
	     Lattice<Float> & prior,
	     Lattice<Float> & mask,
	     uInt nIntegrations = 10,
	     Float sigma = 0.001,
	     Float targetFlux = 1.0,
	     Bool useFluxConstraint = False,
	     Bool initializeModel = True,
	     Bool imagePlane = False);


  // destructor
  virtual ~CEMemModel();

  
  // solve the convolution equation
  // returns True if converged

  // Gives information about the state of the CEMem
  // (ie, using mask image, using prior image; more work here!)
  void state();


  //  This needs to be "ResidualEquation", using LatConvEquation as
  //  polymorphism is broken
  Bool solve(ResidualEquation<Lattice<Float> >  & eqn);

  // Set and get various state images and classes
  // <group>
  // set or get the Entropy class
  void setEntropy(Entropy &myEntropy ) {itsEntropy_ptr = &myEntropy;}
  void getEntropy(Entropy &myEntropy ) {myEntropy = *itsEntropy_ptr;}

  // set or get the Model image
  Lattice<Float>& getModel() const 
    { return (*(itsModel_ptr->clone())); }
  void setModel(const Lattice<Float> & model)
    { itsModel_ptr = model.clone(); }

  // set or get the Prior image
  Lattice<Float>& getPrior() const 
    { return (*(itsPrior_ptr->clone())); }
  void setPrior(Lattice<Float> & prior);

  // set or get the Mask image
  Lattice<Float>& getMask() const 
    { return (*(itsMask_ptr->clone())); }
  void setMask(Lattice<Float> & mask);

  // get the Residual image
  Lattice<Float>& getResidual() const 
    { return (*(itsResidual_ptr->clone())); }

  // </group>


  // set and get alpha and beta
  // <group>
  Float getAlpha() const { return itsAlpha; }
  Float getBeta() const { return itsBeta; }
  void setAlpha(Float alpha) {itsAlpha = alpha; }
  void setBeta(Float beta) {itsBeta = beta; }
  // </group>

  // Set various controlling parameters
  // (The most popular controlling parameters are 
  // set in the constructor)
  // <group>
  Float getTolerance() {return itsTolerance; }
  void  setTolerance(Float x) { itsTolerance = x; }
  Float getQ() {return itsQ; }
  void  setQ(Float x) { itsQ= x; }
  Float getGain() {return itsGain; }
  void  setGain(Float x) { itsGain = x; }
  Float getMaxNormGrad() {return itsMaxNormGrad; }
  void  setMaxNormGrad(Float x) { itsMaxNormGrad = x; }
  Int getInitialNumberIterations() {return itsFirstIteration; }
  void  setInitialNumberIterations(Int x) { itsFirstIteration = x; }
  // </group>

  // The convergence can also be in terms of the maximum residual
  // (ie, for use in stopping in a major cycle).
  void setThreshold (const Float x ) { itsThreshold0 = x; }
  // Thresh doubles in iter iterations
  void setThresholdSpeedup (const Float iter) {itsThresholdSpeedup = iter; }
  Float getThreshold();

  // Set/get the progress display 
  // <group>
  virtual void setProgress(CEMemProgress& ccp) { itsProgressPtr = &ccp; }
  virtual CEMemProgress& getProgress() { return *itsProgressPtr; }
  // </group>

  // return the number of iterations completed
  Int numberIterations () { return itsIteration; }

  // if this MEMModel is constructed in a MF loop, we may need to
  // increment the flux by the last iterations flux
  void setCycleFlux(Float x) { itsCycleFlux = x; }
  Float getCycleFlux() { return itsCycleFlux; }

protected:


  // Perform One Iteration
  void oneIteration();

  // apply mask to a lattice; returns True if mask is available, 
  // False if not
  Bool applyMask( Lattice<Float> & lat );

  // Helper functions that interface with Entropy routines
  // Access to the entropy should be through this interface; the
  // points at which the Entropy is mentioned is then limited to
  // these lines right here, and to the constructors which set the
  // Entropy.  The entropy should not ever change the private
  // 
  // <group>
  void formEntropy() { itsEntropy = itsEntropy_ptr->formEntropy(); }

  void  formGDG() { itsEntropy_ptr->formGDG( itsGDG ); }

  void  formGDGStep() { itsEntropy_ptr->formGDGStep( itsGDG ); }

  void formGDS() { itsGradDotStep1=itsEntropy_ptr->formGDS(); }

  void entropyType(String &str) { itsEntropy_ptr->entropyType(str); }

  void relaxMin() { itsRequiredModelMin = itsEntropy_ptr->relaxMin(); }

  Bool testConvergence() { return itsEntropy_ptr->testConvergence(); }

  // </group>


  // protected generic constrcutor: DON'T USE IT!
  CEMemModel();

  // Set entropy pointer to zero: called by Entropy's destructor
  void letEntropyDie() { itsEntropy_ptr = 0; }

  // initialize itsStep and itsResidual and other stuff
  Bool initStuff();


  // controls how to change Alpha and Beta
  // <group>
  void changeAlphaBeta ();
  // initialize Alpha-Beta (called by changeAlphaBeta)
  void initializeAlphaBeta();
  // update Alpha-Beta (called by changeAlphaBeta)
  void updateAlphaBeta();
  // </group>




  // Generic utility functions
  // <group>
  // check that a single image is onf plausible shape
  Bool checkImage(const Lattice<Float> *);
  // check that the lattices and the underlying tile sizes are consistent
  Bool checkImages(const Lattice<Float> *one, const Lattice<Float> *other);
  // check that all is well in Denmark:
  //     ensure all images are the same size,
  //     ensure we have an entropy,
  //     ensure state variables have reasonable values
  Bool ok();
  // </group>
  



  // Helper functions for oneIteration:
  // <group>
  // calculate size of step
  void calculateStep();

  // take one step: clipped addition of
  // wt1*itsModel + wt2*itsStep
  void takeStep(Float wt1, Float wt2);

  // Calculate the flux, itsModMin, and itsModMax
  Float formFlux();
  // </group>

  // Determine if the peak residual is less than the getThreshold()
  Bool testConvergenceThreshold();

  // ------------Private Member Data---------------------
  // functional form of the entropy
  Entropy  *itsEntropy_ptr;   

  // form of the Residual method
  ResidualEquation< Lattice<Float> > * itsResidualEquation_ptr;

  // Images:
  Lattice<Float> * itsModel_ptr;
  Lattice<Float> * itsPrior_ptr;
  Lattice<Float> * itsMask_ptr;
  // Our OWN internal temp images; delete these upon destruction
  Lattice<Float> * itsStep_ptr;
  Lattice<Float> * itsResidual_ptr;


  // Controlling parameters
  // <group>
  Bool itsInitializeModel;
  uInt itsNumberIterations;
  Bool  itsDoInit;
  Float itsSigma;
  Float itsTargetFlux;  
  Float itsDefaultLevel;
  Float itsTargetChisq;
  // tolerance for convergence
  Float itsTolerance;	
  // N points per beam
  Float itsQ;		
  // gain for adding step image
  Float itsGain;	
  Float itsMaxNormGrad;
  // constrain flux or not?
  Bool  itsUseFluxConstraint;  
  // is this an image plane problem (like Single Dish or Optical?)
  Bool  itsDoImagePlane;
  Float itsThreshold0;
  Float itsThresholdSpeedup;
  Float itsCycleFlux; // flux from previous cycles
  // </group>

  // State variables
  // <group>  
  Float itsAlpha;
  Float itsBeta;
  Float itsNormGrad;
  Float itsFlux;
  Float itsTotalFlux;      // itsCycleFlux + itsFlux
  Float itsChisq;
  // sqrt( chi^2/target_chi^2 )
  Float itsFit;	       
  // sqrt( chi^2/ Npixels )
  Float itsAFit;       
  // numerical value of entropy
  Float itsEntropy;    
  // Model is constrained to be >= itsNewMin
  Float itsRequiredModelMin; 
  // maximum pixel value in model
  Float itsModelMax;   
  // minimum pixel value n model
  Float itsModelMin;   
  Float itsLength;
  Double itsGradDotStep0;
  Double itsGradDotStep1;
  uInt   itsIteration;
  uInt   itsFirstIteration;
  // matrix of gradient dot products
  Matrix<Double> itsGDG;  
  Float itsCurrentPeakResidual;
  // </group>
  Float itsNumberPixels;


  // Accesories
  // <group>  
  Bool itsChoose;
  LogIO itsLog;
  // </group>  

  // Enumerate the different Gradient subscript types
  enum GradientType {
    // Entropy
    H=0,
    // Chi_sq
    C=1,
    // Flux
    F=2,
    // Objective function J
    J=3
  };

  CEMemProgress* itsProgressPtr;
 

};



} //# NAMESPACE CASA - END

#endif


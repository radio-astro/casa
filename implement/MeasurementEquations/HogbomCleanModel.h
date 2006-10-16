//# HogbomCleanModel.h: this defines HogbomCleanModel
//# Copyright (C) 1996,1999
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

#ifndef SYNTHESIS_HOGBOMCLEANMODEL_H
#define SYNTHESIS_HOGBOMCLEANMODEL_H


#include <casa/aips.h>
#include <synthesis/MeasurementEquations/ArrayModel.h>
#include <synthesis/MeasurementEquations/Iterate.h>
#include <synthesis/MeasurementEquations/ResidualEquation.h>
#include <synthesis/MeasurementEquations/ConvolutionEquation.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A Class for performing Hogbom Clean's of Arrays
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite> 
// <li> ResidualEquation/ConvolutionEquation 
// <li> LinearModel/LinearEquation Paradigm 
// </prerequisite>
//
// <etymology>
// This class is called HogbomCleanModel because thats the algorithm it uses
// deconvolve the model. 
// </etymology>
//
// <synopsis>
// This class is used to perform the Hogbom Clean Algorithm on an
// Array. Only the deconvolved model of the sky are directly stored by this
// class. The point spread function (psf) and convolved (dirty) image are
// stored in a companion class which is must be derived from
// ResidualEquation. 
// 
// The cleaning works like this. The user constructs a HogbomCleanModel by
// specifying an initial model of the sky. This can by be
// one,two,three... dimensional depending on the dimension of the psf (see
// below). The user then constructs a class which implements the forward
// equation between the model and the dirty image. Typically this will be
// the ConvolutionEquation class, although any class which has a
// ResidualEquation interface will be work (but perhaps very slowly, as the
// ConvolutionEquation class has member functions optimised for cleaning)
//
// The user then calls the solve() function (with the appropriate equation
// class as an arguement), and this class will perform the Hogbom clean.
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
// A companion class to this one is MaskedHogbomCleanModel. This provides
// the same functionality but is used with MaskedArrays which indicate which
// regions of the model to search for clean components. 
//
// </synopsis>
//
// <example>
// <srcblock>
// Matrix<Float> psf(12,12), dirty(10,10), initialModel(10,10);
// ...put appropriate values into psf, dirty, & initialModel....
// HogbomCleanModel<Float> deconvolvedModel(initialModel); 
// ConvolutionEquation convEqn(psf, dirty);
// deconvolvedModel.setGain(0.2); 
// deconvolvedModel.setNumberIterations(1000);
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

template<class T> class HogbomCleanModel: 
  public ArrayModel<T>,
  public Iterate
{
public:
  // The default constructor does nothing more than initialise a zero length
  // array to hold the deconvolved model. If this constructor is used then 
  // the actual model must be set using the setModel() function of the
  // ArrayModel class.
  HogbomCleanModel():ArrayModel<T>(){};
  // Construct the HogbomCleanModel object and initialise the model.
  HogbomCleanModel(const Array<T> & model):ArrayModel<T>(model){};
  // Using a Hogbom clean deconvolution proceedure solve for an improved
  // estimate of the deconvolved object. The convolution/residual equation
  // contains the psf and dirty image. When called with a ResidualEquation
  // arguement a quite general interface is used that is slow. The
  // convolution equation contains functions that speed things up. The
  // functions return False if the deconvolution could not be done.
  // <group>
  Bool solve(ResidualEquation<Array<T> > & eqn);
  Bool solve(ConvolutionEquation & eqn);
  // </group>

  //# Make parent members known.
protected:
  using ArrayModel<T>::theModel;
};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/MeasurementEquations/HogbomCleanModel.cc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif

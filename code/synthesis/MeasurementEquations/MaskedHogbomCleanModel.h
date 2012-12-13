//# MaskedHogbomCleanModel.h: this defines MaskedHogbomCleanModel
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

#ifndef SYNTHESIS_MASKEDHOGBOMCLEANMODEL_H
#define SYNTHESIS_MASKEDHOGBOMCLEANMODEL_H


#include <casa/aips.h>
#include <synthesis/MeasurementEquations/MaskedArrayModel.h>
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

// <prerequisite> <li> ResidualEquation/ConvolutionEquation <li>
// LinearModel/LinearEquation Paradigm </prerequisite>
//
// <etymology>
// This class is called HogbomCleanModel because thats the algorithm it uses
// deconvolve the model. 
// </etymology>
//
// <synopsis>
// This class is used to perform the Hogbom Clean Algorithm on a
// MaskedArray. It is functionally equivalent to the HogbomCleanModel class and
// all the documention for that class will not be repreated here. The only
// difference is that the model used to represent the sky in this class is a
// MaskedArray, and hence this class can implement the concept of clean
// boxes.
// 
// Masking is used to deliniate the search region when clean is determing
// the maximum residual as well as the region that is subtracted in the
// when calculating the residual image (if the ConvolutionEquation class is
// used in conjunction with this class). However Because this user
// calculates the residual using the  ConvolutionEquation class (which ignores
// the mask) this will always return a residual that is accurate even in the
// masked regions of the image. 
// </synopsis>
//
// <example>
// <srcblock>
// Matrix<Float> psf(12,12); // The psf cannot be masked!
// MaskedMatrix dirty(10,10), initialModel(10,10);
// ...put appropriate values into psf, dirty, & initialModel....
// ConvolutionEquation convEqn(psf, dirty);
// MaskedHogbomCleanModel<Float> deconvolvedModel(initialModel); 
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
// </templating>
//
//
// <todo asof="1996/05/02">
//   <li> Check that Arrays of StokesVectors work as advertised
//   <li> compare timing with other clean implementations (ie, Mark's
//   CleanTools, SDE, AIPS & miriad) 
// </todo>

template<class T> class MaskedHogbomCleanModel: 
  public MaskedArrayModel<T>,
  public Iterate
{
public:
  // The default constructor does nothing more than initialise a zero length
  // array to hold the deconvolved model. If this constructor is used then 
  // the actual model must be set using the setModel() function of the
  // ArrayModel class.
  MaskedHogbomCleanModel():MaskedArrayModel<T>(){};
  // Construct the HogbomCleanModel object and initialise the model.
  MaskedHogbomCleanModel(const MaskedArray<T> & model)
    :MaskedArrayModel<T>(model){};
  // Using a Hogbom clean deconvolution proceedure solve for an improved
  // estimate of the deconvolved object. The convolution/residual equation
  // contains the psf and dirty image. When called with a ResidualEquation
  // arguement a quite general interface is used that is slow. The
  // convolution equation contains functions that speed things up. The
  // functions return False if the deconvolution could not be done.
  // <group>
  Bool solve(ResidualEquation<MaskedArray<T> > & eqn);
  Bool solve(ConvolutionEquation & eqn);
  // </group>

  //# Make parent members known.
protected:
  using MaskedArrayModel<T>::theModel;
};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <synthesis/MeasurementEquations/MaskedHogbomCleanModel.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif

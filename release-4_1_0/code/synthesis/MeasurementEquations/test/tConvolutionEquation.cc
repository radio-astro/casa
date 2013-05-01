//# tConvolutionEquation.cc:  this defines tConvolutionEquation
//# Copyright (C) 1996,1997,1999,2000,2001
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

#include <casa/aips.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicMath/Random.h>
#include <synthesis/MeasurementEquations/ConvolutionEquation.h>
#include <synthesis/MeasurementEquations/ArrayModel.h>
#include <scimath/Mathematics/Convolver.h>
#include <casa/OS/Time.h>

#include <casa/iostream.h>

#include <casa/namespace.h>
int main()
{
  Bool anyFailures = False;
  {
    Bool failed = False;
    const uInt moddims = 1, modSize = 6; 
    const uInt dirtydims = moddims, dirtySize = modSize;
    const uInt psfdims = 1, psfSize = 4;
    // Create a psf 
    Array<Float> psf(IPosition(psfdims, psfSize));
    psf = 0.01;
    IPosition psfOrigin(psfdims,psfSize/2);
    psf(psfOrigin) = 1;
    for (uInt i = 0; i < psfdims; i++){
      psfOrigin(i)++; 
      psf(psfOrigin) = 0.5; 
      psfOrigin(i)--; psfOrigin(i)--; 
      psf(psfOrigin) = 0.4;
      psfOrigin(i)++; 
    }
    // cout << "Psf:" << psf << endl; 
    // Create a model Image (using randomly place point sources)
    Array<Float> model(IPosition(moddims, modSize));
    model = 0;
    {
      Time now;
      MLCG gen(now.seconds(), now.seconds()); 
      DiscreteUniform rnd(&gen, 0, modSize-1);
      IPosition pos(moddims, 0); 
      for (Int j = 0; j < pow((int) modSize, (int) moddims)/5; j++){
	for (uInt k = 0; k < moddims; k++) {
	  pos(k) = rnd.asInt();
	}
	model(pos) = rnd();
      }
    }
    // cout << "Model:" << model << endl; 
    // Create the dirty image (by convolving the psf with the model)
    Array<Float> dirty(IPosition(dirtydims, dirtySize));
    {
      Convolver<Float> conv(psf, model.shape());
      conv.linearConv(dirty, model);
    }
    // cout << "Dirty:" << dirty << endl;
    // Create a convolution equation
    ConvolutionEquation ConvEqn(psf, dirty);
    // Now create an array model using the already existing model
    ArrayModel<Float> myModel(model);
    // let it do some evaluating
    Array<Float> result;
    ConvEqn.evaluate(result, myModel);
    // cout << "Evaluate:" << result << endl;
    if (allNearAbs(result, dirty, Double(1E-5)) == False){
      cout << "Failed";
      failed = True;
    }
    else
      cout << "Passed";
    cout << " the 1D test (evaluate)" << endl;
    // lets it calculate the residual (it should be exactly zero)
    if (!failed){
      Array<Float> residual;
      ConvEqn.residual(residual, myModel);
      //      cout << "Residual:" << residual << endl;
      Array<Float> zero(residual.shape());
      zero = 0;
      if (allNearAbs(residual, zero, Double(1E-5)) == False){
	cout << "Failed";
	failed = True;
      }
      else
	cout << "Passed";
      cout << " the 1D test (residual)" << endl;
    }
    // test out residual with chisq
    if (!failed){
      Float chisq;
      Array<Float> residual;
      ConvEqn.residual(residual, chisq, myModel);
      //      cout << "Residual:" << residual << endl;
      Array<Float> zero(residual.shape());
      zero = 0;
      if (abs(chisq) > 1e-5){
	cout << "Failed on residual chisq";
	failed = True;
      }
      if (allNearAbs(residual, zero, Double(1E-5)) == False){
	cout << "Failed";
	failed = True;
      }
      if (!failed) {
	cout << "Passed";
	cout << " the 1D test (residual with chis2)" << endl;
      }
    }
    if (failed) anyFailures = True;
  }
  if (anyFailures == False) {
    Bool failed = False;
    const uInt moddims = 2, modSize = 6; 
    const uInt dirtydims = moddims, dirtySize = modSize;
    const uInt psfdims = 1, psfSize = 12;
    // Create a psf 
    Array<Float> psf(IPosition(psfdims, psfSize));
    psf = 0.01;
    IPosition psfOrigin(psfdims,psfSize/2);
    psf(psfOrigin) = 1;
    for (uInt i = 0; i < psfdims; i++){
      psfOrigin(i)++; 
      psf(psfOrigin) = 0.5; 
      psfOrigin(i)--; psfOrigin(i)--; 
      psf(psfOrigin) = 0.4;
      psfOrigin(i)++; 
    }
    // cout << "Psf:" << psf << endl; 
    // Create a model Image (using randomly place point sources)
    Array<Float> model(IPosition(moddims, modSize));
    model = 0;
    {
      Time now;
      MLCG gen(now.seconds(), now.seconds()); 
      DiscreteUniform rnd(&gen, 0, modSize-1);
      IPosition pos(moddims, 0);
      for (Int j = 0; j < pow((int) modSize, (int) moddims)/5; j++){
	for (uInt k = 0; k < moddims; k++){
	  pos(k) = rnd.asInt();
	}
	model(pos) = rnd();
      }
    }
    //    cout << "Model:" << model << endl; 
    // Create the dirty image (by convolving the psf with the model)
    Array<Float> dirty(IPosition(dirtydims, dirtySize));
    {
      Convolver<Float> conv(psf, model.shape());
      conv.linearConv(dirty, model);
    }
    //    cout << "Dirty:" << dirty << endl;
    // Create a convolution equation
    ConvolutionEquation ConvEqn(psf, dirty);
    // Now create an array model using the already existing model
    ArrayModel<Float> myModel(model);
    // let it do some evaluating
    Array<Float> result;
    ConvEqn.evaluate(result, myModel);
    //    cout << "Evaluate:" << result << endl;
    if (allNearAbs(result, dirty, Double(1E-5)) == False){
      cout << "Failed";
      failed = True;
    }
    else
      cout << "Passed";
    cout <<  " the multiple 1D test (evaluate)" << endl;
    // lets it calculate the residual (it should be exactly zero)
    if (!failed){
      Array<Float> residual;
      ConvEqn.residual(residual, myModel);
      //      cout << "Residual:" << residual << endl;
      Array<Float> zero(residual.shape());
      zero = 0;
      if (allNearAbs(residual, zero, Double(1E-5)) == False){
	cout << "Failed";
	failed = True;
      }
      else
	cout << "Passed";
      cout << " the multiple 1D test (residual)" << endl;
    }
    if (failed) anyFailures = True;
  }
  if (anyFailures == False) {
    Bool failed = False;
    const uInt moddims = 2, modSize = 6; 
    const uInt dirtydims = moddims, dirtySize = modSize;
    const uInt psfdims = 2, psfSize = 6;
    // Create a psf 
    Array<Float> psf(IPosition(psfdims, psfSize));
    psf = 0.01;
    IPosition psfOrigin(psfdims,psfSize/2);
    psf(psfOrigin) = 1;
    for (uInt i = 0; i < psfdims; i++){
      psfOrigin(i)++; 
      psf(psfOrigin) = 0.5; 
      psfOrigin(i)--; psfOrigin(i)--; 
      psf(psfOrigin) = 0.4;
      psfOrigin(i)++; 
    }
    //    cout << "Psf:" << psf << endl; 
    // Create a model Image (using randomly place point sources)
    Array<Float> model(IPosition(moddims, modSize));
    model = 0;
    {
      Time now;
      MLCG gen(now.seconds(), now.seconds()); 
      DiscreteUniform rnd(&gen, 0, modSize-1);
      IPosition pos(moddims, 0);
      for (Int j = 0; j < pow((int) modSize, (int) moddims)/5; j++){
	for (uInt k = 0; k < moddims; k++){
	  pos(k) = rnd.asInt();
	}
	model(pos) = rnd();
      }
    }
    //    cout << "Model:" << model << endl; 
    // Create the dirty image (by convolving the psf with the model)
    Array<Float> dirty(IPosition(dirtydims, dirtySize));
    {
      Convolver<Float> conv(psf, model.shape());
      conv.linearConv(dirty, model);
    }
    //    cout << "Dirty:" << dirty << endl;
    // Create a convolution equation
    ConvolutionEquation ConvEqn(psf, dirty);
    // Now create an array model using the already existing model
    ArrayModel<Float> myModel(model);
    // let it do some evaluating
    Array<Float> result;
    ConvEqn.evaluate( result, myModel);
    //    cout << "Evaluate:" << result << endl;
    if (allNearAbs(result, dirty, Double(1E-5)) == False){
      cout << "Failed";
      failed = True;
    }
    else
      cout << "Passed";
    cout << " the 2D test (evaluate)" << endl;
    // lets it calculate the residual (it should be exactly zero)
    if (!failed){
      Array<Float> residual;
      ConvEqn.residual(residual, myModel);
      //      cout << "Residual:" << residual << endl;
      Array<Float> zero(residual.shape());
      zero = 0;
      if (allNearAbs(residual, zero, Double(1E-5)) == False){
	cout << "Failed";
	failed = True;
      }
      else
	cout << "Passed";
      cout << " the 2D test (residual)" << endl;
    }
    if (failed) anyFailures = True;
  }
  if (anyFailures) 
    return 1;
  else
    return 0;
}

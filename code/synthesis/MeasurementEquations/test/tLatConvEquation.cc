//# tLatConvEquation.cc:  this defines tLatConvEquation
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
#include <lattices/Lattices/TempLattice.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicMath/Random.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <synthesis/MeasurementEquations/LatticeModel.h>
#include <lattices/LatticeMath/LatticeConvolver.h>
#include <lattices/Lattices/PagedArray.h>
#include <casa/OS/Time.h>
#include <casa/BasicSL/String.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>

#include <casa/namespace.h>
void plotLattice(Lattice<Float>* latp, const String& title);

int main()
{
  Bool anyFailures = False;
  {
    Bool failed = False;
    const uInt moddims = 1, modSize = 6; 
    const uInt dirtydims = moddims, dirtySize = modSize;
    const uInt psfdims = 1, psfSize = 4;
    // Create a psf 
    TempLattice<Float> psf(IPosition(psfdims, psfSize));
    psf.set(0.01f);
    IPosition psfOrigin(psfdims,psfSize/2);
    psf.putAt(1.0f, psfOrigin);
    for (uInt i = 0; i < psfdims; i++){
      psfOrigin(i)++; 
      psf.putAt(0.5f, psfOrigin); 
      psfOrigin(i)--; psfOrigin(i)--; 
      psf.putAt(0.4f, psfOrigin);
      psfOrigin(i)++; 
    }
    // cout << "Psf:" << psf << endl; 
    // Create a model Image (using randomly place point sources)
    TempLattice<Float> model(IPosition(moddims, modSize));
    model.set(0.0f);
    {
      Time now;
      MLCG gen(now.seconds(), now.seconds()); 
      DiscreteUniform rnd(&gen, 0, modSize-1);
      IPosition pos(moddims, 0);
      for (uInt j = 0; j < (uInt)(pow((int) modSize, (int) moddims)/5); j++){
	for (uInt k = 0; k < moddims; k++){
	  pos(k) = rnd.asInt();
	}
	Float flux = rnd();
	model.putAt(flux, pos);
      }
    }
    // cout << "Model:" << model << endl; 
    // Create the dirty image (by convolving the psf with the model)
    TempLattice<Float> dirty(IPosition(dirtydims, dirtySize));
    {
      LatticeConvolver<Float> conv(psf, model.shape());
      conv.linear(dirty, model);
    }
    // cout << "Dirty:" << dirty << endl;
    // Create a convolution equation
    LatConvEquation ConvEqn(psf, dirty);
    // Now create a Lattice model using the already existing model
    LatticeModel  myModel(model);
    // let it do some evaluating
    TempLattice<Float> result(IPosition(dirtydims, dirtySize));

    cout << " result shape = " << result.shape() << endl;
    cout << " model shape = " << model.shape() << endl;
    const Lattice<Float> & modelLattice = myModel.getModel();
    cout << " myModel shape = " << modelLattice.shape() << endl;

    ConvEqn.evaluate(result, myModel);
    // cout << "Evaluate:" << result << endl;
    if (allNearAbs(result.get(), dirty.get(), Double(1E-5)) == False){
      cout << "Failed";
      failed = True;
    }
    else
      cout << "Passed";
    cout << " the 1D test (evaluate)" << endl;
    // lets it calculate the residual (it should be exactly zero)
    if (!failed){
      TempLattice<Float> residual(model.shape());
      ConvEqn.residual(residual, myModel);
      //      cout << "Residual:" << residual << endl;
      TempLattice<Float> zero(residual.shape());
      zero.set(0.0f);
      if (allNearAbs(residual.get(), zero.get(), Double(1E-5)) == False){
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
      TempLattice<Float> residual(model.shape());
      ConvEqn.residual(residual, chisq, myModel);
      //      cout << "Residual:" << residual << endl;
      TempLattice<Float> zero(residual.shape());
      zero.set(0.0f);
      if (abs(chisq) > 1e-5){
	cout << "Failed on residual chisq";
	failed = True;
      }
      if (allNearAbs(residual.get(), zero.get(), Double(1E-5)) == False){
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
    const uInt moddims = 2, modSize = 12; 
    const uInt dirtydims = moddims, dirtySize = modSize;
    const uInt psfdims = 2, psfSize = 12;
    // Create a psf 
    TempLattice<Float> psf(IPosition(psfdims, psfSize));
    psf.set(0.01f);
    IPosition psfOrigin(psfdims,psfSize/2);
    psf.putAt(1.0, psfOrigin);
    for (uInt i = 0; i < psfdims; i++){
      psfOrigin(i)++; 
      psf.putAt(0.5, psfOrigin); 
      psfOrigin(i)--; psfOrigin(i)--; 
      psf.putAt(0.4, psfOrigin); 
      psfOrigin(i)++; 
    }
    // cout << "Psf:" << psf << endl; 
    // Create a model Image (using randomly place point sources)
    TempLattice<Float> model(IPosition(moddims, modSize));
    model.set(0.0f);
    {
      Time now;
      MLCG gen(now.seconds(), now.seconds()); 
      DiscreteUniform rnd(& gen, 0, modSize-1);
      IPosition pos(moddims, 0);
      for (uInt j = 0; j < uInt(pow((int) modSize, (int) moddims)/5); j++){
	for (uInt k = 0; k < moddims; k++){
	  pos(k) = rnd.asInt();
	}
	Float flux = rnd();
	model.putAt(flux, pos);
      }
    }
    //    cout << "Model:" << model << endl; 
    // Create the dirty image (by convolving the psf with the model)
    TempLattice<Float> dirty(IPosition(dirtydims, dirtySize));
    {
      cout << "psf shape: " << psf.shape() << endl;
      cout << "model shape: " << model.shape () << endl;
      LatticeConvolver<Float> conv(psf, model.shape());
      conv.linear(dirty, model);
    }
    //    cout << "Dirty:" << dirty << endl;
    // Create a convolution equation
    LatConvEquation ConvEqn(psf, dirty);
    // Now create an lattice model using the already existing model
    LatticeModel myModel(model);
    // let it do some evaluating
    TempLattice<Float> result(model.shape());
    ConvEqn.evaluate(result, myModel);
    //    cout << "Evaluate:" << result << endl;
    if (allNearAbs(result.get(), dirty.get(), Double(1E-5)) == False){
      cout << "Failed";
      failed = True;
    }
    else
      cout << "Passed";
    cout <<  " the multiple 1D test (evaluate)" << endl;
    // lets it calculate the residual (it should be exactly zero)
    if (!failed){
      TempLattice<Float> residual(model.shape());
      ConvEqn.residual(residual, myModel);
      //      cout << "Residual:" << residual << endl;
      TempLattice<Float> zero(residual.shape());
      zero.set(0.0);
      if (allNearAbs(residual.get(), zero.get(), Double(1E-5)) == False){
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
    const uInt moddims = 2, modSize = 12; 
    const uInt dirtydims = moddims, dirtySize = modSize;
    const uInt psfdims = 2, psfSize = 12;
    // Create a psf 
    TempLattice<Float> psf(IPosition(psfdims, psfSize));
    psf.set(0.01f);
    IPosition psfOrigin(psfdims,psfSize/2);
    psf.putAt(1.0, psfOrigin);
    for (uInt i = 0; i < psfdims; i++){
      psfOrigin(i)++; 
      psf.putAt(0.5f, psfOrigin);
      psfOrigin(i)--; psfOrigin(i)--; 
      psf.putAt(0.4f, psfOrigin);
      psfOrigin(i)++; 
    }
    //    cout << "Psf:" << psf << endl; 
    // Create a model Image (using randomly place point sources)
    TempLattice<Float> model(IPosition(moddims, modSize));
    model.set(0.0f);
    {
      Time now;
      MLCG gen(now.seconds(), now.seconds()); 
      DiscreteUniform rnd(&gen, 0, modSize-1);
      IPosition pos(moddims, 0);
      for (uInt j = 0; j < (uInt)(pow((int) modSize, (int) moddims)/5); j++){
	for (uInt k = 0; k < moddims; k++){
	  pos(k) = rnd.asInt();
	}
	Float flux = rnd();
	model.putAt(flux, pos);
      }
    }
    //    cout << "Model:" << model << endl; 
    // Create the dirty image (by convolving the psf with the model)
    TempLattice<Float> dirty(IPosition(dirtydims, dirtySize));
    {
      LatticeConvolver<Float> conv(psf, model.shape());
      conv.linear(dirty, model);
    }
    //    cout << "Dirty:" << dirty << endl;
    // Create a convolution equation
    LatConvEquation ConvEqn(psf, dirty);
    // Now create an lattice model using the already existing model
    LatticeModel  myModel(model);
    // let it do some evaluating
    TempLattice<Float> result(model.shape());
    ConvEqn.evaluate( result, myModel);
    //    cout << "Evaluate:" << result << endl;
    if (allNearAbs(result.get(), dirty.get(), Double(1E-5)) == False){
      cout << "Failed";
      failed = True;
    }
    else
      cout << "Passed";
    cout << " the 2D test (evaluate)" << endl;
    // lets it calculate the residual (it should be exactly zero)
    if (!failed){
      TempLattice<Float> residual(model.shape());
      ConvEqn.residual(residual, myModel);
      //      cout << "Residual:" << residual << endl;
      TempLattice<Float> zero(residual.shape());
      zero.set(0.0);
      if (allNearAbs(residual.get(), zero.get(), Double(1E-5)) == False){
	cout << "Failed";
	failed = True;
      }
      else
	cout << "Passed";
      cout << " the 2D test (residual)" << endl;
    }
    if (failed) anyFailures = True;


    // Verify shift evaluate with Lattices
    Lattice<Float> * newpsf = ConvEqn.evaluate(psf.shape()/4, Float(1.0), psf.shape()/2);
    plotLattice(newpsf, "Center one");
    delete newpsf;
    newpsf = ConvEqn.evaluate(psf.shape()/2, Float(2.0), psf.shape()/2);
    plotLattice(newpsf, "Center one scaled");
    delete newpsf;


    // Verify evaluate with Larger Lattices
    newpsf = ConvEqn.evaluate(psf.shape(), Float(1.0), 2*psf.shape());
    plotLattice(newpsf, "Center one, padded");
    delete newpsf;
    // Verify shift evaluate with Larger Lattices
    newpsf = ConvEqn.evaluate(psf.shape()/2, Float(1.0), 2*psf.shape());
    plotLattice(newpsf, "shifted and padded");
    delete newpsf;
    newpsf = ConvEqn.evaluate(psf.shape()/2, Float(2.0), 2*psf.shape());
    plotLattice(newpsf, "shifted and padded and scaled");
    delete newpsf;


    // Verify shift evaluate with Arrays
    Array<Float> psfPatch;
    ConvEqn.evaluate(psfPatch, psf.shape()/2, Float(1.0), psf.shape()/2);
    ConvEqn.evaluate(psfPatch, psf.shape()/2, Float(2.0), psf.shape()/2);

    if (failed) anyFailures = True;

  }
  if (anyFailures) 
    return 1;
  else
    return 0;
}


void plotLattice(Lattice<Float>* latp, const String & title) {
  cout << title << endl;
  uInt i;
  uInt j;    
  IPosition sh = latp->shape();
  uInt ni = sh(0);
  uInt nj = sh(1);
  cout << setprecision(2);
  for (j=0;j<nj;j++) {
    for(i=0;i<ni;i++) {
      sh(0) = i;
      sh(1) = j;
      cout << " " << latp->getAt(sh);
    }
    cout << endl;
  }
}

//# tClarkCleanModel.cc:  this defines tClarkCleanModel
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

#include <casa/iostream.h>
#include <casa/aips.h>
#include <images/Images/PagedImage.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <synthesis/MeasurementEquations/ConvolutionEquation.h>
#include <synthesis/MeasurementEquations/ArrayModel.h>
#include <synthesis/MeasurementEquations/ClarkCleanModel.h>
#include <scimath/Mathematics/Convolver.h>
#include <casa/OS/Time.h>
#include <casa/OS/Timer.h>
#include <casa/BasicMath/Random.h>
#include <coordinates/Coordinates/CoordinateUtil.h>

#include <casa/namespace.h>
int main()
{
  Bool Failed = False;
  // Setup the sizes for the model,psf and dirty image.
  const uInt moddims = 2, modSize = 256; 
  const uInt dirtydims = moddims, dirtySize = modSize;
  const uInt psfdims = 2, psfSize = 4;
  const uInt maxIterations=10000;
  const Float fluxLimit=1E-2;
  // Create a psf 
  Array<Float> psf(IPosition(psfdims, psfSize));
  psf = Float(0);
  IPosition psfOrigin(psfdims,psfSize/2);
  psf(psfOrigin) = Float(1);
  for (uInt i = 0; i < psfdims; i++){
    psfOrigin(i)++; 
    psf(psfOrigin) = Float(0.3); 
    psfOrigin(i)--; psfOrigin(i)--; 
    psf(psfOrigin) = Float(0.35);
    psfOrigin(i)++; 
  }

  // Create a model Image
  Array<Float> model(IPosition(moddims, modSize));
  model = Float(0);
  {
    Time now;
    MLCG gen(now.seconds(), now.seconds()); 
    DiscreteUniform rnd(& gen, 0, modSize-1);
    IPosition pos(moddims, 0);
    for (Int j = 0; j < pow((int) modSize, (int) moddims)/400; j++){
      for (uInt k = 0; k < moddims; k++){
	pos(k) = rnd.asInt();
      }
      model(pos) = rnd()/(modSize-1);
    }
  }

  // Create the dirty image (by convolving the psf with the model)
  Array<Float> dirty(IPosition(dirtydims, dirtySize));
  {
    Convolver<Float> conv(psf, model.shape());
    conv.linearConv(dirty, model);
  }

  // Now create a clean model 
  ClarkCleanModel  myModel;
  // Create a convolution equation
  ConvolutionEquation ConvEqn(psf, dirty);
  // Now start testing the Clean model
  // Firstly Set the initial model to all zeros
  myModel.setModel(model*Float(0.0));
  // check that the model it is set correctly
  {
    // This checks that model and myModel are not references of each other
    IPosition zeroIndex =IPosition(model.ndim(),0);
    Float firstElement = model(zeroIndex);
    model(zeroIndex) = Float(-10); 
    Array<Float> initialModel;
    myModel.getModel(initialModel);
    if (allNearAbs(initialModel, Float(0), 1E-6) == False){
      cout << "Initial model is not set correctly" << endl;
      Failed = True;
    }
    model(zeroIndex) = firstElement;
  }
  // Now set up the clean parameters
  myModel.setGain(0.10);
  myModel.setNumberIterations(maxIterations);// clean for at most maxIterations
  myModel.setThreshold(fluxLimit);           // Or until the flux limit
                                             // is reached
  myModel.setPsfPatchSize(IPosition(psfdims,15)); 
  myModel.setHistLength(500);
  myModel.setMaxNumPix(1000);

  // Now do the clean
  cout << "Clean using a maximum of " << myModel.numberIterations() 
       << " iterations" 
       << " or max residual of " << myModel.threshold() << endl;
  cout << "Timing the clean"
       << " (psf size:" << psf.shape()
       << "  model size:" << model.shape() 
       << " )" << endl;
  Timer timer;
  myModel.solve(ConvEqn);
  timer.show();
  cout << "Clean used " << myModel.numberIterations() << " iterations" 
       << " to get to a max residual of " << myModel.threshold() 
       << endl;
  cout << myModel.getMaxNumPix() << " active pixels were used"
       << " and the histogram had " << myModel.getHistLength() << " bins"
       << endl; 
  cout << "The Psf patch size used was " << myModel.getPsfPatchSize()
       << " and the maximum exterior value was " << myModel.getMaxExtPsf()
       << endl;
  cout << "The maximum number of minor iterations in a major cycle was "
       << myModel.getMaxNumberMinorIterations() << endl;
  // Now get the results
  Array<Float> solvedModel;
  myModel.getModel(solvedModel);

  Array<Float> solvedResidual;
  ConvEqn.residual(solvedResidual, myModel);

  if (myModel.numberIterations() < (Int)maxIterations) {
  // if less than the maximum number of Residuals is used then
  // check the Residual is less than the fluxlimit
    if (max(abs(solvedResidual)) > 1.0001*fluxLimit) {
      cout << "Failed ";
      Failed = True;
    }
    else
      cout << "Passed";
    cout << " the residual fluxlimit test" << endl;
  // The sum of the residual and deconvolved model should be close to the
  // actual model used (however there are always pathological cases) 
    solvedModel+=solvedResidual;
    if (allNearAbs(solvedModel, model, 2*fluxLimit) != True) {
      cout << "Failed ";
      Failed = True;
    }
    else
      cout << "Passed";
    cout << " the model comparison test" << endl;
  }
  else {
    cout << "Cannot complete the test. Too few iterations used" << endl;
    Failed = True;
  }
  if (Failed) {
    cout << "FAIL" << endl;
    return 1;
  }
  else {
    cout << "OK" << endl;
    return 0;
  }
}

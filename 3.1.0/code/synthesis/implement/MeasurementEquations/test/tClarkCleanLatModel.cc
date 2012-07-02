//# tClarkCleanLatModel.cc:  this tests tClarkCleanLatModel
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
#include <lattices/Lattices/TempLattice.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Slicer.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <synthesis/MeasurementEquations/LatticeModel.h>
#include <synthesis/MeasurementEquations/ClarkCleanLatModel.h>
#include <synthesis/MeasurementEquations/ClarkCleanProgress.h>
#include <lattices/Lattices/LatticeConvolver.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <casa/System/PGPlotter.h>

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
  const uInt dirtySize = modSize;
  const uInt psfdims = 2, psfSize = 256;
  const uInt maxIterations=50000;
  const Float fluxLimit=1E-2;
  // Create a psf 
  TempLattice<Float> psf(IPosition(4, psfSize, psfSize, 1, 1));
  psf.set(0);
  IPosition psfOrigin(4,128,128,0,0);
  psf.putAt(1.0f, psfOrigin);
  for (uInt i = 0; i < psfdims; i++){
    psfOrigin(i)++; 
    psf.putAt(0.3f, psfOrigin);
    psfOrigin(i)--; psfOrigin(i)--; 
    psf.putAt(0.35f, psfOrigin);
    psfOrigin(i)++; 
  }
  // sprinkle some exterior sidelobes
  {
    IPosition psfExterior(4,200,184,0,0);
    psf.putAt(0.1, psfExterior);
  }
  {
    IPosition psfExterior(4,250,250,0,0);
    psf.putAt(0.15, psfExterior);
  }


  // Create a model Image
  TempLattice<Float> model(IPosition(4, modSize, modSize, 1, 1));
  model.set(0.0);
  {
    Time now;
    MLCG gen(now.seconds(), now.seconds()); 
    DiscreteUniform rnd(&gen, 0, modSize-1);
    IPosition pos(4, 0);
    for (uInt j = 0; j < uInt(pow((int) modSize, (int) moddims)/400); j++){
      for (uInt k = 0; k < moddims; k++){
	pos(k) = rnd.asInt();
	if(pos(k) < 64)
	  pos(k) +=64;
	if(pos(k) > 192)
	  pos(k) -= 64;
	   
      }
      model.putAt(rnd()/(modSize-1),  pos);
    }
  }
  // Create the dirty image (by convolving the psf with the model)
  TempLattice<Float> dirty(IPosition(4, dirtySize, dirtySize, 1, 1));
  {
    LatticeConvolver<Float> conv(psf, model.shape());
    conv.linear(dirty, model);
  }

  //  Now create a clean model 
  ClarkCleanLatModel myModel;
  //  Create a convolution equation
  LatConvEquation convEqn(psf, dirty);
  //  Now start testing the Clean model
  //  Firstly Set the initial model to all zeros

  TempLattice<Float> cleanModel(IPosition(4, modSize, modSize, 1, 1));
  cleanModel.set(0.0);

  myModel.setModel(cleanModel);

  // Now set up the clean parameters
  myModel.setGain(0.10);
  myModel.setNumberIterations(maxIterations);// clean for at most maxIterations
  myModel.setThreshold(fluxLimit);           // Or until the flux limit
                                             // is reached
  myModel.setPsfPatchSize(IPosition(2,51,51)); 
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

  PGPlotter *pgplotter_p = new PGPlotter("/xs");
  ClarkCleanProgress *cleanerProgress=0;
  //myModel.setProgress(*cleanerProgress);
  //pgplotter_p->ask(False);

  Timer timer;
  Bool ans = myModel.solve(convEqn);
  if (!ans) {
    cout << "ClarkCleanLatModel claims failure" << endl;
  }
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
  TempLattice<Float> solvedModel(myModel.getModel().shape());
  {
    LatticeExprNode  myLEN = myModel.getModel();
    solvedModel.copyData((LatticeExpr<Float>) myLEN );
  }

  TempLattice<Float> solvedResidual(solvedModel.shape());
  convEqn.residual(solvedResidual, myModel);

  if (myModel.numberIterations() < (Int)maxIterations) {
  // if less than the maximum number of Residuals is used then
  // check the Residual is less than the fluxlimit

    Float maxResidual;
    {
      LatticeExprNode myLEN;
      myLEN = max(abs( solvedResidual ) );
      maxResidual = myLEN.getFloat();
    }

    cout << "maxResidual " << maxResidual << " fluxLimit " << fluxLimit << endl;
    if (maxResidual > 2,0*fluxLimit) {
      cout << "Failed ";
      Failed = True;
    }
    else {
      cout << "Passed";
    }
    cout << " the residual fluxlimit test" << endl;
  // The sum of the residual and deconvolved model should be close to the
  // actual model used (however there are always pathological cases) 

    solvedModel.copyData((LatticeExpr<Float>) (solvedModel + solvedResidual));
    
    Float maxError;
    {
      LatticeExprNode myLEN;
      myLEN = max(abs( solvedModel - model ) );
      maxError = myLEN.getFloat();
    }
    if (maxError > 2.0*fluxLimit) {
      cout << "Failed the model comparison test" << endl;
      Failed = True;
    }   else {
      cout << "Passed the model comparison test" << endl;
    }
  } else {
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

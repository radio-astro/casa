//# PSkyEquation.cc: Implementation of Sky Equation classes
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
//# $Id$

#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <measures/Measures/MeasConvert.h>
#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/SkyJones.h>
#include <synthesis/MeasurementComponents/FTMachine.h>
#include <components/ComponentModels/Flux.h>
#include <synthesis/MeasurementComponents/ComponentFTMachine.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <msvis/MSVis/VisSet.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/String.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <unistd.h>
#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/SubLattice.h>   
#include <lattices/Lattices/TiledShape.h>  
#include <casa/System/ProgressMeter.h>
#include <synthesis/MeasurementEquations/PSkyEquation.h>
#include <synthesis/MeasurementComponents/PredictAlgorithm.h>
#include <synthesis/MeasurementComponents/ResidualAlgorithm.h>
#include <synthesis/Parallel/Applicator.h>

namespace casa { //# NAMESPACE CASA - BEGIN

extern Applicator applicator;

PSkyEquation::PSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft): SkyEquation(sm, vs, ft, cft){ };

PSkyEquation::PSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, FTMachine& ift, ComponentFTMachine& cft): SkyEquation(sm, vs, ft, ift, cft){ };

Bool PSkyEquation::solveSkyModel(){
 Bool result=sm_->solve(*this);
 return result;
}

void PSkyEquation::predict(Bool incremental) {
  // Parallelized model coherence prediction

  LogIO os(LogOrigin("PSkyEquation", "parallelPredict", WHERE));

  AlwaysAssert(ok(),AipsError);
  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  AlwaysAssert(vs_, AipsError);
  PredictAlgorithm  predictVis;
  String errorString; 
  Bool allDone, assigned; 
  Int rank;
  
  for (Int model=0;model<sm_->numberOfModels();model++) {     
      if(incremental){
        StokesImageUtil::From(sm_->cImage(model), sm_->deltaImage(model));
      }
      else{
        StokesImageUtil::From(sm_->cImage(model), sm_->image(model));
      }
  }  

  // Now do the images
  for (Int model=0;model<sm_->numberOfModels();model++) {      

    // Don't bother with empty images except for the first one
    if((model==0)||!sm_->isEmpty(model)) {
        assigned = applicator.nextAvailProcess(predictVis, rank);
     while (!assigned) {
      rank = applicator.nextProcessDone(predictVis, allDone);
      Int modelDone;
      applicator.get(modelDone);
      Record container_received; 
      applicator.get(container_received);
        
      // Assign the next available process
      assigned = applicator.nextAvailProcess(predictVis, rank);
      unScaleImage(modelDone, incremental);
     };

     scaleImage(model, incremental);
     // Send data to worker
     applicator.put(model);
     applicator.put(vs_->msName());
     applicator.put(incremental);

     // Send model image
     ImageInterface<Complex>& tempimage(sm_->cImage(model));
     Record container_image;
     tempimage.toRecord(errorString, container_image);
     applicator.put(container_image);

     // Send FT Machine
     Record container_ft;
     ft_->toRecord(errorString, container_ft);
     applicator.put(container_ft);
     
     // For serial transport
     applicator.apply(predictVis); 

     // Stagger child processes accessing the MS
     sleep(5);
    }
  }

  rank = applicator.nextProcessDone(predictVis, allDone);
  while (!allDone) {
    Int modelDone;
    applicator.get(modelDone);
    Record container_received; 
    applicator.get(container_received);
    unScaleImage(modelDone, incremental);
    // Wait for the next process to complete
    rank = applicator.nextProcessDone(predictVis, allDone);
  };
}

void PSkyEquation::gradientsChiSquared(Bool incremental) {

  AlwaysAssert(ok(),AipsError);

  // Predict the visibilities
  unlock();
  predict(incremental);
  lock();
  sumwt = 0.0;
  chisq = 0.0;
  
  // Initialize the gradients
  sm_->initializeGradients();

  //ROVisIter& vi(vs_->iter());
  unlock();

  for (Int model=0;model<sm_->numberOfModels();model++) {   
    scaleImage(model, incremental);
  }

  String errorString;
  ResidualAlgorithm  residualVis;
  Bool allDone, assigned;
  Int rank;

  for (Int model=0;model<sm_->numberOfModels();model++) {      
   if(sm_->isSolveable(model)) {
     assigned = applicator.nextAvailProcess(residualVis, rank);
     while (!assigned) {
       rank = applicator.nextProcessDone(residualVis, allDone);
       Int modelDone;
       applicator.get(modelDone);
       Record image_container;
       applicator.get(image_container);
       TempImage<Complex> tempoImage;
       tempoImage=*(new TempImage<Complex>());
       tempoImage.fromRecord(errorString, image_container);
       sm_->cImage(modelDone).copyData(tempoImage);
       Record container_received; 
       applicator.get(container_received);
       ift_->fromRecord(errorString, container_received);
       applicator.get(sm_->weight(modelDone));
     
       applySkyJonesInv(vb_p, -1, sm_->cImage(modelDone), sm_->work(modelDone),
		   sm_->gS(modelDone));

       applySkyJonesSquare(vb_p, -1, sm_->weight(modelDone), 
			   sm_->work(modelDone),
			   sm_->ggS(modelDone));

       sm_->addStatistics(sumwt, chisq);
       unScaleImage(modelDone, incremental);
       unlock();

      // Assign the next available process
      assigned = applicator.nextAvailProcess(residualVis, rank);
     };

     // Send data to worker
     applicator.put(model);
     applicator.put(vs_->msName());
     applicator.put(incremental);

     // Send image info 
     ImageInterface<Complex>& tempimage(sm_->cImage(model));
     Record container_image;
     tempimage.toRecord(errorString, container_image);
     applicator.put(container_image);
     // Send FT Machine info
     Bool withImage=False;
     Record container_ft;
     ift_->toRecord(errorString, container_ft, withImage);
     applicator.put(container_ft);
     applicator.put(sm_->weight(model));

     // For serial transport
     applicator.apply(residualVis); 

     // Stagger worker processes slightly
     sleep(5);     
   }
  }

  rank = applicator.nextProcessDone(residualVis, allDone);

  while (!allDone) {
    Int modelDone;
    applicator.get(modelDone);

    Record image_container;
    applicator.get(image_container);
  
    TempImage<Complex> tempoImage;
    tempoImage=*(new TempImage<Complex>());
    tempoImage.fromRecord(errorString, image_container);
    Record container_received; 
    applicator.get(container_received);
    
    // Receiving FT machine 
    ift_->fromRecord(errorString,container_received);
    applicator.get(sm_->weight(modelDone));
      
    sm_->cImage(modelDone).copyData(tempoImage);
      
    applySkyJonesInv(vb_p, -1, sm_->cImage(modelDone), sm_->work(modelDone),
		     sm_->gS(modelDone));

    applySkyJonesSquare(vb_p, -1, sm_->weight(modelDone), 
			sm_->work(modelDone), sm_->ggS(modelDone));

    sm_->addStatistics(sumwt, chisq);
    unScaleImage(modelDone, incremental);
    unlock();

    // Wait for the next process to complete
    rank = applicator.nextProcessDone(residualVis, allDone);
    
  };

  lock();
  fixImageScale();

  // Finish off any calculations needed internal to SkyModel
  sm_->finalizeGradients();

  lock();
}

void PSkyEquation::lock(){
vs_->lock();
};

void PSkyEquation::unlock(){
vs_->unlock();
};

String PSkyEquation::associatedMSName(){
return vs_->msName();
}


} //# NAMESPACE CASA - END


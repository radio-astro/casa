//# WFSkyEquation.cc: Implementation of Multi Field Sky Equation classes
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
#include <synthesis/MeasurementEquations/WFSkyEquation.h>
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
#include <synthesis/MeasurementComponents/ComponentFTMachine.h>
#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/iostream.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/SubLattice.h>   

#include <casa/System/ProgressMeter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

WFSkyEquation::WFSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft)
  : SkyEquation(sm, vs, ft, cft, False)
{
  itsSM=(WFCleanImageSkyModel *) sm_;
  itsFT=(WFGridFT *) ft_;
  itsIFT=(WFGridFT *) ift_;
};


void WFSkyEquation::makeMultiApproxPSF(PtrBlock<TempImage<Float> * >& psfMulti, Int nmodels) {
  //  AlwaysAssert(ok(),AipsError);
  for (Int thismodel=0; thismodel < nmodels ; thismodel++){
    sm_->cImage(thismodel);
    sm_->weight(thismodel);
  }
  itsIFT->makeMultiImage(FTMachine::PSF, *vs_, itsSM->cimageBlkPtr(),
		  itsSM->weightBlkPtr(), nmodels);
  for (Int thismodel=0; thismodel < nmodels ; thismodel++){
    StokesImageUtil::To(*psfMulti[thismodel], itsSM->cImage(thismodel));
  }
}


void WFSkyEquation::gradientsChiSquared(Bool incremental, Bool hasModel) {

  AlwaysAssert(ok(),AipsError);
  // Predict the visibilities
  multiPredict(incremental);
  //  predict(incremental);
  sumwt = 0.0;
  chisq = 0.0;

  // Initialize the gradients
  sm_->initializeGradients();

  ROVisIter& vi(vs_->iter());
   VisBuffer vb(vi);
  //  Int numberOfRows= vs_->numberCoh();
  //  vi.setRowBlocking(numberOfRows);

  // Loop over all models in SkyModel
    for (Int model=0;model<sm_->numberOfModels();model++) {
          scaleImage(model, incremental);

      // Change the model polarization frame
      if(vb.polFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::LINEAR);
      }
      else {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::CIRCULAR);
      }

    }
    Int nmodels=sm_->numberOfModels();
    // Reset the various SkyJones
    resetSkyJones();

    // Loop over all visibilities and pixels
    //    VisBuffer vb(vi);
      
    //    if(sm_->isSolveable(model)) {

      vi.originChunks();
      vi.origin();



      initializeMultiPut(vb, nmodels);
      Int cohDone=0;
      
      ostringstream modelName;modelName<<"WF "
				    <<" : transforming residuals";
      ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		       modelName, "", "", "", True);
      // Loop over the visibilities, putting VisBuffers
      
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin(); vi.more(); vi++) {
	  
	  vb.modelVisCube()-=vb.correctedVisCube();
	  vb.setVisCube(vb.modelVisCube());
	  multiPut(vb, nmodels);
	  cohDone+=vb.nRow();
	  pm.update(Double(cohDone));
	}
      }
      // Do the transform, apply the SkyJones transformation
      // and sum the statistics for this model
      finalizeMultiPut(vb_p, nmodels);
      //}

    for(Int model=0; model < nmodels; model++){
        unScaleImage(model, incremental);
    }
  fixImageScale();

  // Finish off any calculations needed internal to SkyModel
  sm_->finalizeGradients();


}
//----------------------------------------------------------------------

void WFSkyEquation::initializeMultiGet(const VisBuffer& vb, Int row, 
				       Int nmodels,  Bool incremental) {

  //  AlwaysAssert(ok(),AipsError);
  for (Int model=0; model < nmodels; model++){
    if(incremental) {
      applySkyJones(vb, row, sm_->deltaImage(model), sm_->cImage(model));
    }
    else {
      applySkyJones(vb, row, sm_->image(model), sm_->cImage(model));
    }
  }
  itsFT->initializeToMultiVis(itsSM->cimageBlkPtr(), vb, nmodels);
  
}

void WFSkyEquation::initializeMultiPut(const VisBuffer& vb, Int nmodels) {
  AlwaysAssert(ok(),AipsError);
  for (Int k=0; k< nmodels; k++){
    sm_->cImage(k);
    sm_->weight(k);
  }
  itsIFT->initializeToMultiSky(itsSM->cimageBlkPtr(),itsSM->weightBlkPtr(),
			     vb, nmodels);
  assertSkyJones(vb, -1);
  vb_p.assign(vb, False);
  vb_p.updateCoordInfo();
}

void WFSkyEquation::multiPut(const VisBuffer & vb, Int nmodels, Bool dopsf) {

  AlwaysAssert(ok(),AipsError);

  // We'll have to implemement Check to see if the SkyJones are constant
 
    itsIFT->multiPut(vb, -1, dopsf, nmodels);
 
}


void WFSkyEquation::finalizeMultiPut(const VisBuffer& vb, Int nmodels) {
  // Actually do the transform. Update weights as we do so.
   
   itsIFT->assignMapNumber(nmodels);
  for(Int model=0; model < nmodels; model++){
    //   ift_->initializeToSky(sm_->cImage(model), sm_->weight(model), vb);
    //  ift_->finalizeToSky();

    // 1. Now get the (unnormalized) image and add the 
    // weight to the summed weight

    Matrix<Float> delta;
    itsIFT->changeMapNumber(model);
    sm_->cImage(model).copyData(ift_->getImage(delta, False));

    // 2. Apply the SkyJones and add to grad chisquared

 
    applySkyJonesInv(vb, -1, sm_->cImage(model), sm_->work(model),
		     sm_->gS(model));


    // 3. If we need it, apply the square of the SkyJones and add
    // this to gradgrad chisquared

    // Just passing a dummy weight as it is already updated in this case

    applySkyJonesSquare(vb, -1, delta, sm_->work(model),
			sm_->ggS(model));
   
    // 4. Finally, we add the statistics
       sm_->addStatistics(sumwt, chisq);
  }
}


void WFSkyEquation::multiPredict(Bool incremental) {

  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  AlwaysAssert(vs_, AipsError);
  //  if(sm_->numberOfModels()!= 0)  
  //   AlwaysAssert(ok(),AipsError);
  // Initialize 
  VisIter& vi=vs_->iter();
  //Int numberOfRows= vs_->numberCoh();
  //  vi.setRowBlocking(numberOfRows);

  VisBuffer vb(vi);


  // Reset the visibilities only if this is not an incremental
  // change to the model
  Bool initialized=False;

  // Do the component model only if this is not an incremental update;
  if(sm_->hasComponentList() &&  !incremental ) {

    // Reset the various SkyJones
    resetSkyJones();

    // Loop over all visibilities


      vi.originChunks();
      vi.origin();



    Int cohDone=0;
    ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		     "Predicting component coherences",
		     "", "", "", True);

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
        if(!incremental&&!initialized) {
	  vb.setModelVisCube(Complex(0.0,0.0));
	  vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	}

	get(vb, sm_->componentList() );

	// and write it to the model MS
	vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	cohDone+=vb.nRow();
	pm.update(Double(cohDone));
      }
    }
    if(!incremental&&!initialized) initialized=True;
  }


  // Now do the images
  for (Int model=0;model<sm_->numberOfModels();model++) {      

    // Don't bother with empty images except for the first one
    if((model==0)||!sm_->isEmpty(model)) {
      
      // Change the model polarization frame
      if(vb.polFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::LINEAR);
      }
      else {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::CIRCULAR);
      }
      
      scaleImage(model, incremental);
    }
  }
      // Reset the various SkyJones
      resetSkyJones();
      

      // Initialize get (i.e. Transform from Sky)
      vi.originChunks();
      vi.origin();      


      initializeMultiGet(vb, 0, sm_->numberOfModels(), incremental);
      Int cohDone=0;
      
      ostringstream modelName;modelName
				    <<" WF : predicting coherences";
      ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		       modelName, "", "", "", True);
      // Loop over all visibilities
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin(); vi.more(); vi++) {
	  if(!incremental&&!initialized) {
	    vb.setModelVisCube(Complex(0.0,0.0));
	    vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	  }
	  // get the model visibility and write it to the model MS
	  multiGet(vb,  sm_->numberOfModels(), incremental);
	  vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	  cohDone+=vb.nRow();
	  pm.update(Double(cohDone));
	}
      }
      finalizeGet(); //This is a dummy for now.
      //      unScaleImage(model, incremental);
      if(!incremental&&!initialized) initialized=True;
    
  
}

VisBuffer& WFSkyEquation::multiGet(VisBuffer& result,  Int nmodels,
				   Bool incremental) {
  //  AlwaysAssert(ok(),AipsError);
  result.modelVisCube(); // get the visibility so vb will have it

  //  Cube<Complex> data(result.modelVisCube());
  // data=Complex(0.0, 0.0);

  //  VisBuffer vb(result);
  // we might need to recompute the "sky" for every single row, but we
  // avoid this if possible.
  //  if (constantSkyJones()) {
    // get all rows
    itsFT->multiGet(result, -1, nmodels);
 /*
  } else {
    // The SkyJones are not constant. We need to find out if there are
    // any changes in this buffer
    Bool internalChanges=False;  // Does this VB change inside itself?
    Bool firstOneChanges=False;  // Has this VB changed from the previous one?
    changedSkyJonesLogic(result, firstOneChanges, internalChanges);
    if(internalChanges) {
      // Yes there are changes within this buffer: go row by row.
      for (Int row=0; row<nRow; row++) {
	finalizeGet();
	initializeMultiGet(result, row, nmodels, incremental);
	ft_->multiGet(vb, data, row, nmodels);
      }
    } else if (firstOneChanges) {
      // This buffer has changed wrt the previous buffer, but
      // this buffer has no changes within it
      finalizeGet();
      initializeMultiGet(result, 0, nmodels, incremental);
      ft_->multiGet(vb, data, -1, nmodels);
    } else {
      // Although the SkyJones can change, it did not do so
      // in this buffer, so we just get it.
      ft_->multiGet(vb, data, -1, nmodels);
    }
 

  }
 */
  //  result.modelVisCube()+=vb.modelVisCube();
    //  result.modelVisCube()=vb.modelViscube();
  return result;
}

} //# NAMESPACE CASA - END


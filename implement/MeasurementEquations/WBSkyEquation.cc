//# WBSkyEquation.cc: Implementation of Multi Frequency Synthesis
//#                   Sky Equation classes
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

#include <synthesis/MeasurementEquations/WBSkyEquation.h>
#include <synthesis/MeasurementComponents/WideBandFT.h>

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
#include <measures/Measures/UVWMachine.h>
#include <synthesis/MeasurementComponents/ComponentFTMachine.h>
#include <casa/Exceptions/Error.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/iostream.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/SubLattice.h>   

#include <casa/System/ProgressMeter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

WBSkyEquation::WBSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft)
  : SkyEquation(sm, vs, ft, cft, False)
{
	itsSM = (WBCleanImageSkyModel *) sm_;
	os = LogIO(LogOrigin("WBSkyEquation","everything"));
	adbg = 0;
};

//----------------------------------------------------------------------
// We make an approximate PSF for each plane.
// ************* Note that this overwrites the model! ******************
void WBSkyEquation::makeApproxPSF(Int model, ImageInterface<Float>& psf) 
{

  AlwaysAssert(ok(), AipsError);
  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  AlwaysAssert(vs_, AipsError);
 
  ntaylor_p = ((WBCleanImageSkyModel*)sm_)->ntaylor_p;
  
  ft_->setNoPadding(noModelCol_p);

  isPSFWork_p= True; // avoid PB correction etc for PSF estimation
  Bool doPSF=True;
  if(ft_->name() == "MosaicFT") {
    // Reset the various SkyJones
    doPSF=False;
    resetSkyJones();
    
    VisIter& vi(vs_->iter());
    checkVisIterNumRows(vi);
    // Loop over all visibilities and pixels
    VisBuffer vb(vi);
    
    vi.originChunks();
    vi.origin();
    
    // Change the model polarization frame
    if(vb.polFrame()==MSIter::Linear) {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					SkyModel::LINEAR);
    }
    else {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					SkyModel::CIRCULAR);
    }
    
    IPosition start(4, sm_->image(model).shape()(0)/2,
		    sm_->image(model).shape()(1)/2, 0, 0);
    IPosition shape(4, 1, 1, sm_->image(model).shape()(2), sm_->image(model).shape()(3));
    Array<Float> line(shape);
    TempImage<Float> savedModel(sm_->image(model).shape(),
				sm_->image(model).coordinates());
    savedModel.copyData(sm_->image(model));
    sm_->image(model).set(0.0);
    line=1.0;
    sm_->image(model).putSlice(line, start);
    //initializeGet(vb, -1, model, False);
    StokesImageUtil::From(sm_->cImage(model), sm_->image(model));
    ft_->initializeToVis(sm_->cImage(model),vb);
    // Loop over all visibilities
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
       	vb.setModelVisCube(Complex(0.0,0.0));
	//	get(vb, model, False);
	ft_->get(vb);
	vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
      }
    }
    finalizeGet();
    
    sm_->image(model).copyData(savedModel);
  }
  
  // Initialize the gradients
  sm_->initializeGradients();
  

  ROVisIter& vi(vs_->iter());
  
  // Reset the various SkyJones
  resetSkyJones();
  
  checkVisIterNumRows(vi);
  // Loop over all visibilities and pixels
  VisBuffer vb(vi);


  vi.originChunks();
  vi.origin();
 
  // Change the model polarization frame
  if(vb.polFrame()==MSIter::Linear) {
    StokesImageUtil::changeCStokesRep(sm_->cImage(model),SkyModel::LINEAR);
    for(Int taylor=1;taylor<ntaylor_p;taylor++)
	    StokesImageUtil::changeCStokesRep(((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor),SkyModel::LINEAR);
  }
  else {
    StokesImageUtil::changeCStokesRep(sm_->cImage(model),SkyModel::CIRCULAR);
    for(Int taylor=1;taylor<ntaylor_p;taylor++)
	    StokesImageUtil::changeCStokesRep(((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor),SkyModel::CIRCULAR);
  }


  // Set up the variables into which the resulting PSF will appear
  // Calling local version -- with cImage and cImage1.
  /* (1) */
  if(adbg)os << "(1) Calling se.initializePut" << LogIO::POST;
  initializePut(vb, model, doPSF); //// send doPSF in here....
  if(adbg)os << "(1) Done with se.initializePut" << LogIO::POST;
  
  if(adbg)os << "(2) Start putting" << LogIO::POST;
  // Loop over the visibilities, putting VisBuffers
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin(); vi.more(); vi++) {
      // vb.setVisCube(vb.modelVisCube());
	    // This "put" must do both PSFs...
	    /* (2) */
      put(vb, model, doPSF, FTMachine::MODEL);
    }
  }
  
  // Do the transform, apply the SkyJones transformation
  /* (3) */
  if(adbg)os << "(3) Calling finalizePut" << LogIO::POST;
  finalizePut(vb_p, model, doPSF); //// send doPSF in here....
  if(adbg)os << " Done with finalizePut. Now to finalizeGradients" << LogIO::POST;
  sm_->finalizeGradients();
  if(adbg)os << "Finalized gradients - now fixing image scale " << LogIO::POST;
  fixImageScale();

  /* URV */
  /* (4) */
  ////LatticeExpr<Float> le(iif(sm_->ggS(model)>(0.0), (sm_->gS(model)/sm_->ggS(model)), 0.0));
  	
  LatticeExpr<Float> le(((WBCleanImageSkyModel*)sm_)->gS(model));
  (((WBCleanImageSkyModel*)sm_)->PSF(model)).copyData(le);
  
  ////LatticeExpr<Float> le1(iif(sm_->ggS(model)>(0.0), (((WBCleanImageSkyModel*)sm_)->gS1(model)/sm_->ggS(model)), 0.0));
  
    for(Int taylor=1;taylor<ntaylor_p;taylor++)
    {
	    LatticeExpr<Float> le1(((WBCleanImageSkyModel*)sm_)->gSspec(model,taylor));
	    (((WBCleanImageSkyModel*)sm_)->PSFspec(model,taylor)).copyData(le1);
	    if(adbg)os << "NOT dividing by weights or normalizing psfs to peak 1.0" << LogIO::POST;
    }

#if 0
  {
  	LatticeExpr<Float> le(iif(sm_->ggS(model)>(0.0), (sm_->gS(model)/sm_->ggS(model)), 0.0));
  	psf.copyData(le);
  	
  	LatticeExprNode maxPSF=max(psf);
  	Float maxpsf=maxPSF.getFloat();
	
	  if(abs(maxpsf-1.0) > 1e-3) {
	    os << "Maximum of approximate PSF for field " << model+1 << " = "
	       << maxpsf << " : renormalizing to unity" <<  LogIO::POST;
	  }
	  LatticeExpr<Float> len(psf/maxpsf);
	  psf.copyData(len);
  }
#endif

  isPSFWork_p=False; // resseting this flag so that subsequent calculation uses
  // the right SkyJones correction;
  
  
}

//----------------------------------------------------------------------
void WBSkyEquation::gradientsChiSquared(Bool incremental, Bool commitModel) 
{
  AlwaysAssert(ok(),AipsError);

  Bool forceFull=False;
  //Bool forceFull=True;
  // for these 2 gridders force incremental
  if((ft_->name() == "MosaicFT") || (ft_->name() == "WProjectFT") )
     forceFull=True;

   if( (sm_->numberOfModels() != 1) || !ft_->isFourier() || !incremental || forceFull)
   {
     
     if(commitModel || !noModelCol_p)
     {
       ft_->setNoPadding(False);
       //fullGradientsChiSquared(incremental);
       wbfullGradientsChiSquared(incremental);
       //os << "Returned from wbfullGradC" << LogIO::POST;
     }
     else
     {
       // For now use corrected_data...
       ft_->setNoPadding(True);
       //fullGradientsChiSquared(incremental, True);
       cout << "Still using wbfullGradientsChiSquared !! " << endl;
       wbfullGradientsChiSquared(incremental);
     }
   }
   else 
   {
     incrementGradientsChiSquared();
   }
}

//----------------------------------------------------------------------

void WBSkyEquation::wbfullGradientsChiSquared(Bool incremental) 
{

  AlwaysAssert(ok(),AipsError);

  //os << "Inside wbfullGradientsChiSquared"  << LogIO::POST;
  
  // Predict the visibilities
  predict(incremental);

  sumwt = 0.0;
  chisq = 0.0;

  // Initialize the gradients
  sm_->initializeGradients();
  //os << " Initialized Gradients" << LogIO::POST;

  ROVisIter& vi(vs_->iter());

  // Loop over all models in SkyModel
  for (Int model=0;model<sm_->numberOfModels();model++) 
  {

    // Reset the various SkyJones
    resetSkyJones();

    // Loop over all visibilities and pixels
    checkVisIterNumRows(vi);
    VisBuffer vb(vi);
      
    if(sm_->isSolveable(model)) 
    {

      // Initialize 
      scaleImage(model, incremental);
      //os << "Finished scaleImage" << LogIO::POST;

      vi.originChunks();
      vi.origin();

      // Change the model polarization frame
      if(vb.polFrame()==MSIter::Linear) {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::LINEAR);
      }
      else {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					  SkyModel::CIRCULAR);
      }

      //os << "Calling initializePut" << LogIO::POST;
      initializePut(vb, model, False);
      //os << "Finished initializePut" << LogIO::POST;
      Int cohDone=0;
      
      ostringstream modelName;modelName<<"Model "<<model+1
				    <<" : transforming residuals";
      ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		       modelName, "", "", "", True);
      // Loop over the visibilities, putting VisBuffers
      
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	{
	  vb.modelVisCube()-=vb.correctedVisCube();
	  //	  vb.setVisCube(vb.modelVisCube());
	  put(vb, model, False, FTMachine::MODEL);
	  cohDone+=vb.nRow();
	  pm.update(Double(cohDone));
	}
      }
      // Do the transform, apply the SkyJones transformation
      // and sum the statistics for this model
      //os << "Calling finalizePut" << LogIO::POST;
      finalizePut(vb_p, model, False);
      //os << "Finished finalizePut" << LogIO::POST;
      //os << "Starting unscaleimage" << LogIO::POST;
      unScaleImage(model, incremental);
      //os << "Finished unscaleimage" << LogIO::POST;
    }
  }

  fixImageScale();
  //os << "Finished fixImageScale" << LogIO::POST;

  // Finish off any calculations needed internal to SkyModel
  sm_->finalizeGradients();
  //os << "Finished finalizeGradients" << LogIO::POST;

}


//----------------------------------------------------------------------

//----------------------------------------------------------------------
void WBSkyEquation::initializePut(const VisBuffer& vb, Int model, Bool doPSF) 
{
	AlwaysAssert(ok(),AipsError);
	if(!doPSF)
	{
		//os << "Calling ft.initializeToSky()" << LogIO::POST;
		ift_->initializeToSky(sm_->cImage(model),sm_->weight(model),vb);
		//os << "Done with ft.initializetoSky()" << LogIO::POST;
	}
	else
	{
		//os << "Calling ft.wbinitializeToSky()" << LogIO::POST;
		
		/*** MAKE A PtrBlock OF SIZE EQUAL TO THE NUMBER OF TAYLOR
		  TERMS AND MAKE IT POINT TO THE RIGHT SECTION OF
		  CIMAGESPEC_P AND THEN SEND THAT IN, SO THAT THE
		  FT-MACHINE NEEDS ONLY TO LOOK AT THE LENGTH TO FIGURE
		  OUT THE NUMBER OF TAYLOR TERMS TO USE. PHEW. ***/

		//PtrBlock<ImageInterface<Complex> * > tempcimagespec;
		//tempcimagespec.resize(ntaylor_p-1);
		//for(Int taylor=1;taylor<ntaylor_p;taylor++)
		//	(*tempcimagespec[taylor-1]) = ImageInterface<Complex>(((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor));
		
		((WideBandFT*)ift_)->wbinitializeToSky(((WBCleanImageSkyModel*)sm_)->cImage(model),((WBCleanImageSkyModel*)sm_)->cimagespec_p,sm_->weight(model),vb,model,ntaylor_p-1);
			////((WideBandFT*)ift_)->wbinitializeToSky(((WBCleanImageSkyModel*)sm_)->cImage(model),((WBCleanImageSkyModel*)sm_)->cImagespec(model,1),sm_->weight(model),vb);
		//os << "Done with ft.wbinitializetoSky()" << LogIO::POST;

		//for(Int taylor=1;taylor<ntaylor_p;taylor++)
		//	(((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor)).copyData(*tempcimagespec[taylor-1]);
		
	}
	assertSkyJones(vb, -1);
	vb_p.assign(vb, False);
	vb_p.updateCoordInfo();
}

//----------------------------------------------------------------------
void WBSkyEquation::finalizePut(const VisBuffer& vb, Int model, Bool doPSF) {

  // Actually do the transform. Update weights as we do so.
  //os << "Calling ft_finalizeToSky" << LogIO::POST;
  ift_->finalizeToSky();
  //os << "Done with ft_finalizeToSky" << LogIO::POST;
  // 1. Now get the (unnormalized) image and add the 
  // weight to the summed weight
  Matrix<Float> delta;
  
  if(!doPSF)
  {
  	//os << "Calling getImage" << LogIO::POST;
  	sm_->cImage(model).copyData(ift_->getImage(delta, False));
  	//os << "Finished getImage" << LogIO::POST;
  }
  else
  {
	//PtrBlock<ImageInterface<Complex> * > tempcimagespec;
	//tempcimagespec.resize(ntaylor_p-1);
	
	//for(Int taylor=1;taylor<ntaylor_p;taylor++)
	//	(*tempcimagespec[taylor-1]) = ((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor);
	
	if(adbg)os << "Calling wbgetImage" << LogIO::POST;
	sm_->cImage(model).copyData(((WideBandFT*)ift_)->wbgetImage(((WBCleanImageSkyModel*)sm_)->cimagespec_p,delta, False,model,ntaylor_p-1));
  	if(adbg)os << "Finished wbgetImage" << LogIO::POST;
  }
  // if(doPSF) Clean Up.....
  
  sm_->weight(model)+=delta;
  
  //os << "Apply SkyJonesInv" << LogIO::POST;
  // 2. Apply the SkyJones and add to grad chisquared
  if(!doPSF)
  {
	  applySkyJonesInv(vb, -1, sm_->cImage(model), sm_->work(model), sm_->gS(model));
  }
  else
  {
  	applySkyJonesInv(vb, -1, ((WBCleanImageSkyModel*)sm_)->cImage(model), sm_->work(model), ((WBCleanImageSkyModel*)sm_)->gS(model));
  	//applySkyJonesInv(vb, -1, ((WBCleanImageSkyModel*)sm_)->cImage1(model), sm_->work(model), ((WBCleanImageSkyModel*)sm_)->gS1(model));
	for(Int taylor=1;taylor<ntaylor_p;taylor++)
		applySkyJonesInv(vb, -1, ((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor), sm_->work(model), ((WBCleanImageSkyModel*)sm_)->gSspec(model,taylor));
  }
  
  //os << "Apply SkyJonesSquare" << LogIO::POST;
  // 3. Apply the square of the SkyJones and add this to gradgrad chisquared
  applySkyJonesSquare(vb, -1, sm_->weight(model), sm_->work(model),sm_->ggS(model));

  // 4. Finally, we add the statistics
  sm_->addStatistics(sumwt, chisq);
}
//----------------------------------------------------------------------
// Predict the Sky coherence
void WBSkyEquation::predict(Bool incremental) 
{
	//os << "Inside se.predict()" << LogIO::POST;

  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  AlwaysAssert(vs_, AipsError);
  if(sm_->numberOfModels()!= 0)  AlwaysAssert(ok(),AipsError);
  // Initialize 
  VisIter& vi=vs_->iter();
  checkVisIterNumRows(vi);
  VisBuffer vb(vi);
  

  // Reset the visibilities only if this is not an incremental
  // change to the model
  Bool initialized=False;

  // Do the component model only if this is not an incremental update;
  if(sm_->hasComponentList() &&  !incremental ) 
  {

    // Reset the various SkyJones
    resetSkyJones();

    // Loop over all visibilities

    Int cohDone=0;
    ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		     "Predicting component coherences",
		     "", "", "", True);

    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
        if(!incremental&&!initialized) {
	  vb.setModelVisCube(Complex(0.0,0.0));
	  //	  vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
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
  for (Int model=0;model<sm_->numberOfModels();model++) 
  {      
    
    if( (sm_->isEmpty(model)) && (model==0) && !initialized && !incremental)
    { 
      // We are at the begining with an empty model start
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	{
	  vb.setModelVisCube(Complex(0.0,0.0));
	  vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	}
      }


    }
    // Don't bother with empty images
    if(adbg)os << "IsEmpty(model) : " << sm_->isEmpty(model) << LogIO::POST;
    if(!sm_->isEmpty(model)) 
    {
      
      // Change the model polarization frame
      if(vb.polFrame()==MSIter::Linear) 
      {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),SkyModel::LINEAR);
	//StokesImageUtil::changeCStokesRep(((WBCleanImageSkyModel*)sm_)->cImage1(model),SkyModel::LINEAR);
	for(Int taylor=1;taylor<ntaylor_p;taylor++)
		StokesImageUtil::changeCStokesRep(((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor),SkyModel::LINEAR);
      }
      else 
      {
	StokesImageUtil::changeCStokesRep(sm_->cImage(model),SkyModel::CIRCULAR);
	//StokesImageUtil::changeCStokesRep(((WBCleanImageSkyModel*)sm_)->cImage1(model),SkyModel::CIRCULAR);
	for(Int taylor=1;taylor<ntaylor_p;taylor++)
		StokesImageUtil::changeCStokesRep(((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor),SkyModel::CIRCULAR);
      }
      
      scaleImage(model, incremental);

      // Reset the various SkyJones
      resetSkyJones();
      

      // Initialize get (i.e. Transform from Sky)
      vi.originChunks();
      vi.origin();      

      //os << "Calling initializeGet()" << LogIO::POST;
      initializeGet(vb, 0, model, incremental);
      //os << "Finished initializeGet()" << LogIO::POST;
      Int cohDone=0;
      
      ostringstream modelName;modelName<<"Model "<<model+1
				    <<" : predicting coherences";
      ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		       modelName, "", "", "", True);
      // Loop over all visibilities
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
      {
	for (vi.origin(); vi.more(); vi++) 
	{
	  if(!incremental&&!initialized) 
	  {
	    vb.setModelVisCube(Complex(0.0,0.0));
	    //	    vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	  }
	  // get the model visibility and write it to the model MS
	  get(vb,model,incremental);
	  vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	  cohDone+=vb.nRow();
	  pm.update(Double(cohDone));
	}
      }
      finalizeGet();
      unScaleImage(model, incremental);
      if(!incremental&&!initialized) initialized=True;
    }
  }
}
//----------------------------------------------------------------------
// Initialize
void WBSkyEquation::initializeGet(const VisBuffer& vb, Int row, Int model,
				Bool incremental) 
{

  AlwaysAssert(ok(),AipsError);
  if(incremental) 
  {
	  applySkyJones(vb, row, sm_->deltaImage(model), sm_->cImage(model));
	  //applySkyJones(vb, row, ((WBCleanImageSkyModel*)sm_)->deltaImage1(model), ((WBCleanImageSkyModel*)sm_)->cImage1(model));
	  for(Int taylor=1;taylor<ntaylor_p;taylor++)
		  applySkyJones(vb, row, ((WBCleanImageSkyModel*)sm_)->deltaImagespec(model,taylor), ((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor));
  }
  else 
  {
	  applySkyJones(vb, row, sm_->image(model), sm_->cImage(model));
	  //applySkyJones(vb, row, ((WBCleanImageSkyModel*)sm_)->image1(model), ((WBCleanImageSkyModel*)sm_)->cImage1(model));
	  for(Int taylor=1;taylor<ntaylor_p;taylor++)
		  applySkyJones(vb, row, ((WBCleanImageSkyModel*)sm_)->imagespec(model,taylor), ((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor));
  }
  //ft_->initializeToVis(sm_->cImage(model),vb);
  //((WideBandFT*)ft_)->wbinitializeToVis(((WBCleanImageSkyModel*)sm_)->cImage(model),((WBCleanImageSkyModel*)sm_)->cImage1(model),vb);
  
  //PtrBlock<ImageInterface<Complex> * > tempcimagespec;
  //tempcimagespec.resize(ntaylor_p-1);
  //for(Int taylor=1;taylor<ntaylor_p;taylor++)
//	  (*tempcimagespec[taylor-1]) = ((WBCleanImageSkyModel*)sm_)->cImagespec(model,taylor);
  
  ((WideBandFT*)ft_)->wbinitializeToVis(((WBCleanImageSkyModel*)sm_)->cImage(model),((WBCleanImageSkyModel*)sm_)->cimagespec_p,vb, model, ntaylor_p-1);
}
//----------------------------------------------------------------------

// Add the sky visibility for this coherence sample
VisBuffer& WBSkyEquation::get(VisBuffer& result, Int model,Bool incremental) 
{
  AlwaysAssert(ok(),AipsError);
  Int nRow=result.nRow();

  result.modelVisCube(); // get the visibility so vb will have it
  VisBuffer vb(result);

  Bool FTChanged=changedFTMachine(vb);

  // we might need to recompute the "sky" for every single row, but we
  // avoid this if possible.
  Bool internalChanges=False;  // Does this VB change inside itself?
  Bool firstOneChanges=False;  // Has this VB changed from the previous one?
  if(ft_->name()!="MosaicFT")
  {
    changedSkyJonesLogic(result, firstOneChanges, internalChanges);
  } 
  if(internalChanges) 
  {
    // Yes there are changes within this buffer: go row by row.
    // This will automatically catch a change in the FTMachine so
    // we don't have to check for that.
    for (Int row=0; row<nRow; row++) 
    {
      finalizeGet();
      initializeGet(result, row, model, incremental);
      ft_->get(vb,row);
    }
  }
  else if (FTChanged||firstOneChanges) 
  {
    // This buffer has changed wrt the previous buffer, but
    // this buffer has no changes within it. Again we don't need to
    // check for the FTMachine changing.
    finalizeGet();
    initializeGet(result, 0, model, incremental);
    ft_->get(vb);
  }
  else 
  {
    ft_->get(vb);
  }
  result.modelVisCube()+=vb.modelVisCube();
  return result;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

} //# NAMESPACE CASA - END

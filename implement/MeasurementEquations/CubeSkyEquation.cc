//# CubeSkyEquation.cc: Implementation of Cube Optimized Sky Equation classes
//# Copyright (C) 2007
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
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/OS/HostInfo.h>
#include <casa/System/ProgressMeter.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <lattices/Lattices/LatticeExpr.h>

#include <synthesis/MeasurementComponents/SkyModel.h>
#include <synthesis/MeasurementEquations/CubeSkyEquation.h>
#include <synthesis/MeasurementComponents/SkyJones.h>
#include <synthesis/MeasurementComponents/FTMachine.h>
#include <synthesis/MeasurementComponents/GridFT.h>
#include <synthesis/MeasurementComponents/MosaicFT.h>
#include <synthesis/MeasurementComponents/GridBoth.h>
#include <synthesis/MeasurementComponents/WProjectFT.h>
#include <synthesis/MeasurementComponents/ComponentFTMachine.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>

#include <msvis/MSVis/StokesVector.h>
#include <msvis/MSVis/VisBufferUtil.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>


namespace casa { //# NAMESPACE CASA - BEGIN

CubeSkyEquation::CubeSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft, Bool noModelCol)
  : SkyEquation(sm, vs, ft, cft, noModelCol), internalChangesPut_p(False), 
    internalChangesGet_p(False), firstOneChangesPut_p(False), 
    firstOneChangesGet_p(False)
{

  //make a distinct ift_ as gridding and degridding can occur simultaneously
  if(ft_->name() == "MosaicFT"){
    ift_=new MosaicFT(static_cast<MosaicFT &>(ft));
  }
  else if(ft_->name()== "WProjectFT")
    ift_=new WProjectFT(static_cast<WProjectFT &>(ft));
  else if(ft_->name()== "GridBoth")
    ift_=new GridBoth(static_cast<GridBoth &>(ft));
  else 
    ift_=new GridFT(static_cast<GridFT &>(ft));


  imGetSlice_p=0;
  imPutSlice_p=0;

}


CubeSkyEquation::~CubeSkyEquation(){
  //As we  make an explicit ift_ in the constructor we need 
  //to take care of it here...
  if(ift_ && (ift_ != ft_))
    delete ift_;

}

void  CubeSkyEquation::predict(Bool incremental) {

  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  AlwaysAssert(vs_, AipsError);
  if(sm_->numberOfModels()!= 0)  AlwaysAssert(ok(),AipsError);
  // Initialize 
  VisIter& vi=vs_->iter();
  //Lets get the channel selection for later use
  vi.getChannelSelection(blockNumChanGroup_p, blockChanStart_p,
			 blockChanWidth_p, blockChanInc_p, blockSpw_p);
  checkVisIterNumRows(vi);
  VisBuffer vb(vi);
  Bool changedVI=False;
  // Reset the visibilities only if this is not an incremental
  // change to the model
  Bool initialized=False;
  predictComponents(incremental, initialized);
  //set to zero then loop over model...check for size...subimage then loop over  subimages

  if( (sm_->numberOfModels() >0) && (sm_->isEmpty(0))  && !initialized && !incremental){ 
      // We are at the begining with an empty model as starting point
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin(); vi.more(); vi++) {
	  vb.setModelVisCube(Complex(0.0,0.0));
	  vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	}
      }
  }


  // Now do the images
  for (Int model=0;model<sm_->numberOfModels();model++) {  
    // Don't bother with empty images
    if(!sm_->isEmpty(model)) {
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
      // Reset the various SkyJones
      resetSkyJones();
      Int nCubeSlice=1;
      isLargeCube(sm_->cImage(model), nCubeSlice);
      for (Int cubeSlice=0; cubeSlice< nCubeSlice; ++cubeSlice){
	changedVI= getFreqRange(vi, sm_->cImage(model).coordinates(),
				  cubeSlice, nCubeSlice) || changedVI;
	vi.originChunks();
	vi.origin();
	vb.invalidate();    
	initializeGetSlice(vb, 0, model, incremental, cubeSlice, nCubeSlice);
	for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	  for (vi.origin(); vi.more(); vi++) {
	    if(!incremental&&!initialized) {
	      vb.setModelVisCube(Complex(0.0,0.0));
	    }
	    // get the model visibility and write it to the model MS
	    getSlice(vb,model,incremental, cubeSlice, nCubeSlice);
	    vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	  }
	}
	finalizeGetSlice();
	if(!incremental&&!initialized) initialized=True;
      }
      unScaleImage(model, incremental);
    }
  }

  //lets return original selection back to iterator
  if(changedVI)
    vi.selectChannel(blockNumChanGroup_p, blockChanStart_p, 
		     blockChanWidth_p, blockChanInc_p, blockSpw_p); 

}

void CubeSkyEquation::makeApproxPSF(Int model, ImageInterface<Float>& psf) {
  
  LogIO os(LogOrigin("SkyEquation", "makeApproxPSF"));
  ft_->setNoPadding(noModelCol_p);
  isPSFWork_p= True; // avoid PB correction etc for PSF estimation
  Bool doPSF=True;
  Bool changedVI=False; 
 // Initialize the gradients
  sm_->initializeGradients();
  ROVisIter& vi(vs_->iter());
  //Lets get the channel selection for later use
  vi.getChannelSelection(blockNumChanGroup_p, blockChanStart_p,
			 blockChanWidth_p, blockChanInc_p, blockSpw_p);
  // Reset the various SkyJones
  resetSkyJones();
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

  Int nCubeSlice=1;
  isLargeCube(sm_->cImage(model), nCubeSlice);
  for (Int cubeSlice=0; cubeSlice< nCubeSlice; ++cubeSlice){
    changedVI= getFreqRange(vi, sm_->cImage(model).coordinates(),
				  cubeSlice, nCubeSlice) || changedVI;
    vi.originChunks();
    vi.origin();
    vb.invalidate();
    Int cohDone=0;
    ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		     "Gridding weights for PSF",
		     "", "", "", True);
    initializePutSlice(vb, model, cubeSlice, nCubeSlice);
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
	putSlice(vb, model, doPSF, FTMachine::MODEL, cubeSlice, nCubeSlice);
	cohDone+=vb.nRow();
	pm.update(Double(cohDone));
	
      }
    }
    finalizePutSlice(vb, model, cubeSlice, nCubeSlice);
  }

  //lets return original selection back to iterator
  if(changedVI)
    vi.selectChannel(blockNumChanGroup_p, blockChanStart_p, 
		     blockChanWidth_p, blockChanInc_p, blockSpw_p); 
  sm_->finalizeGradients();
  fixImageScale();
  LatticeExpr<Float> le(iif(sm_->ggS(model)>(0.0),
  			    (sm_->gS(model)/sm_->ggS(model)), 0.0));
  psf.copyData(le);
  LatticeExprNode maxPSF=max(psf);
  Float maxpsf=maxPSF.getFloat();
  if(abs(maxpsf-1.0) > 1e-3) {
    os << "Maximum of approximate PSF for field " << model << " = "
       << maxpsf << " : renormalizing to unity" <<  LogIO::POST;
  }
  if(maxpsf > 0.0 ){
    LatticeExpr<Float> len(psf/maxpsf);
    psf.copyData(len);
    
  }
  else{
    throw(AipsError("SkyEquation:: PSF calculation resulted in a PSF with a peak  being 0 or lesser !"));

  }
 


  isPSFWork_p=False; // resseting this flag so that subsequent calculation uses
  // the right SkyJones correction;
  

}
void CubeSkyEquation::gradientsChiSquared(Bool incremental, Bool commitModel){
  AlwaysAssert(cft_, AipsError);
  AlwaysAssert(sm_, AipsError);
  AlwaysAssert(vs_, AipsError);
  Bool initialized=False;
  Bool changedVI=False;
  predictComponents(incremental, initialized);
  Bool predictedComp=initialized;
  sm_->initializeGradients();
  // Initialize 
  VisIter& vi=vs_->iter();
  //Lets get the channel selection for later use
  vi.getChannelSelection(blockNumChanGroup_p, blockChanStart_p,
			 blockChanWidth_p, blockChanInc_p, blockSpw_p);
  checkVisIterNumRows(vi);
  VisBuffer vb(vi);
  /**** Do we need to do this
  if( (sm_->isEmpty(0))  && !initialized && !incremental){ 
    // We are at the begining with an empty model as starting point
    for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
      for (vi.origin(); vi.more(); vi++) {
	vb.setModelVisCube(Complex(0.0,0.0));
	vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
      }
    }
  }
  */
  // Now do the images
  for (Int model=0;model<sm_->numberOfModels();model++) {  
    Bool isEmpty=sm_->isEmpty(model);
    // Don't bother with empty images
    // Change the model polarization frame
    if(vb.polFrame()==MSIter::Linear) {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					SkyModel::LINEAR);
    }
    else {
      StokesImageUtil::changeCStokesRep(sm_->cImage(model),
					SkyModel::CIRCULAR);
    }
    //scaleImage(model, incremental);
    scaleImage(model);
    // Reset the various SkyJones
    resetSkyJones();
    firstOneChangesPut_p=False;
    firstOneChangesGet_p=False;
    
    Int nCubeSlice=1;
   
    isLargeCube(sm_->cImage(model), nCubeSlice);
    
    
    for (Int cubeSlice=0; cubeSlice< nCubeSlice; ++cubeSlice){
      //      vi.originChunks();
      //      vi.origin();      
      //sliceCube(imGetSlice_p, model, cubeSlice, nCubeSlice, 1);
      cout << "**** cubeslice " << cubeSlice << endl; 
      changedVI= getFreqRange(vi, sm_->cImage(model).coordinates(),
				  cubeSlice, nCubeSlice) || changedVI;
      
      vi.originChunks();
      vi.origin();
      vb.invalidate();
      if(!isEmpty){
	initializeGetSlice(vb, 0, model, False, cubeSlice, nCubeSlice);
      }
      initializePutSlice(vb, model, cubeSlice, nCubeSlice);
      Int cohDone=0;
      ProgressMeter pm(1.0, Double(vs_->numberCoh()),
		       "Gridding residual",
		       "", "", "", True);
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin(); vi.more(); vi++) {
	  if(!incremental&&!initialized) {
	    vb.setModelVisCube(Complex(0.0,0.0));
	  }
	  // get the model visibility and write it to the model MS
	  if(!isEmpty)
	    getSlice(vb,model,predictedComp, cubeSlice, nCubeSlice);
	  //this need to be done only when saving the model
	  if(commitModel)
	    vi.setVis(vb.modelVisCube(),VisibilityIterator::Model);
	  // Now lets grid the -ve of residual
	  // Here we need to use visCube if there is no correctedData
	  vb.modelVisCube()-=vb.correctedVisCube();
	  putSlice(vb, model, False, FTMachine::MODEL, cubeSlice, nCubeSlice);
	  cohDone+=vb.nRow();
	  pm.update(Double(cohDone));
	  
	}
      }
      finalizeGetSlice();
      if(!incremental&&!initialized) initialized=True;
      finalizePutSlice(vb, model, cubeSlice, nCubeSlice);
    }
    //unScaleImage(model, incremental);
    unScaleImage(model);
  }

  
  fixImageScale();
  //lets return original selection back to iterator
  if(changedVI)
    vi.selectChannel(blockNumChanGroup_p, blockChanStart_p, 
		     blockChanWidth_p, blockChanInc_p, blockSpw_p); 


}


void  CubeSkyEquation::isLargeCube(ImageInterface<Complex>& theIm, 
				   Int& nslice) {

  //non-cube
  if(theIm.shape()[3]==1){
    nslice=1;
  }
  else{
    Long npix=theIm.shape().product();
    Long pixInMem=(HostInfo::memoryTotal()/8)*1024;
    nslice=1;
    if(npix > (pixInMem/2)){
      //Lets slice it so grid is at most 1/4th of memory
      pixInMem=pixInMem/4;
      //One plane is
      npix=theIm.shape()(0)*theIm.shape()(1)*theIm.shape()(2);
      nchanPerSlice_p=Int(floor(pixInMem/npix));
      if (nchanPerSlice_p==0)
	nchanPerSlice_p=1;
      nslice=theIm.shape()(3)/nchanPerSlice_p;
      if( (theIm.shape()(3) % nchanPerSlice_p) > 0)
	++nslice;
    }
  }
}

void CubeSkyEquation::initializePutSlice(const VisBuffer& vb, Int model, 
					 Int cubeSlice, Int nCubeSlice) {
  AlwaysAssert(ok(),AipsError);
  sliceCube(imPutSlice_p, model, cubeSlice, nCubeSlice, 0);
  weightSlice_p.resize();
  ift_->initializeToSky(*imPutSlice_p,weightSlice_p,vb);
  assertSkyJones(vb, -1);
  //vb_p is used to finalize things if vb has changed propoerties
  vb_p.assign(vb, False);
  vb_p.updateCoordInfo();
}


void CubeSkyEquation::putSlice(const VisBuffer & vb, Int model, Bool dopsf, FTMachine::Type col, Int cubeSlice, Int nCubeSlice) {

  AlwaysAssert(ok(),AipsError);

  Bool IFTChanged=changedIFTMachine(vb);

  // we might need to recompute the "sky" for every single row, but we
  // avoid this if possible.

  Int nRow=vb.nRow();
  internalChangesPut_p=False;  // Does this VB change inside itself?
  firstOneChangesPut_p=False;  // Has this VB changed from the previous one?
  if(ft_->name() != "MosaicFT"){
    changedSkyJonesLogic(vb, firstOneChangesPut_p, internalChangesPut_p);
  }
  if(internalChangesPut_p || internalChangesGet_p) {
    if(internalChangesGet_p)
      internalChangesGet_p=False;
    // Yes there are changes: go row by row. 
    for (Int row=0; row<nRow; row++) {
      if(IFTChanged||changedSkyJones(vb,row)) {
	// Need to apply the SkyJones from the previous row
	// and finish off before starting with this row
	finalizePutSlice(vb_p, model, cubeSlice, nCubeSlice);  
	initializePutSlice(vb, model, cubeSlice, nCubeSlice);
      }
      ift_->put(vb, row, dopsf, col);
    }
  }
  else if (IFTChanged || firstOneChangesPut_p || firstOneChangesGet_p) {


    if(firstOneChangesGet_p)
      firstOneChangesGet_p=False;
     

    if(!isBeginingOfSkyJonesCache_p){
      finalizePutSlice(vb_p, model, cubeSlice, nCubeSlice);      
    }
    initializePutSlice(vb, model, cubeSlice, nCubeSlice);
    isBeginingOfSkyJonesCache_p=False;
    ift_->put(vb, -1, dopsf,col);
  }
  else {
    ift_->put(vb, -1, dopsf, col);
  }
  isBeginingOfSkyJonesCache_p=False;

}



void CubeSkyEquation::finalizePutSlice(const VisBuffer& vb, Int model, 
				       Int cubeSlice, Int nCubeSlice) {

  // Actually do the transform. Update weights as we do so.
  ift_->finalizeToSky();
  // 1. Now get the (unnormalized) image and add the 
  // weight to the summed weight
  Matrix<Float> delta;
  imPutSlice_p->copyData(ift_->getImage(delta, False));
  
  weightSlice_p+=delta;

  // 2. Apply the SkyJones and add to grad chisquared
  SubImage<Float> *workSlice;
  SubImage<Float> *gSSlice;
  sliceCube(workSlice, sm_->work(model), cubeSlice, nCubeSlice);
  sliceCube(gSSlice, sm_->gS(model), cubeSlice, nCubeSlice);


  applySkyJonesInv(vb, -1, *imPutSlice_p, *workSlice,
		   *gSSlice);
  SubImage<Float> *ggSSlice;
  sliceCube(ggSSlice, sm_->ggS(model), cubeSlice, nCubeSlice);
  
  // 3. Apply the square of the SkyJones and add this to gradgrad chisquared
  applySkyJonesSquare(vb, -1, weightSlice_p, *workSlice,
		      *ggSSlice);
  

  delete workSlice;
  delete gSSlice;
  delete ggSSlice;

  // 4. Finally, we add the statistics
  sm_->addStatistics(sumwt, chisq);
}


void CubeSkyEquation::initializeGetSlice(const VisBuffer& vb, 
					   Int row, Int model,
					   Bool incremental, Int cubeSlice, 
					   Int nCubeSlice){
  if(cubeSlice==0){
    if(incremental) {
      applySkyJones(vb, row, sm_->deltaImage(model), sm_->cImage(model));
    }
    else {
      applySkyJones(vb, row, sm_->image(model), sm_->cImage(model));
    }
  }
  sliceCube(imGetSlice_p, model, cubeSlice, nCubeSlice, 1);
  ft_->initializeToVis(*imGetSlice_p, vb);


}

void CubeSkyEquation::sliceCube(CountedPtr<ImageInterface<Complex> >& slice,Int model, Int cubeSlice, 
				Int nCubeSlice, Int typeOfSlice){

  IPosition blc(4,0,0,0,0);
  IPosition trc(4,sm_->image(model).shape()(0)-1,
		 sm_->image(model).shape()(1)-1,sm_->image(model).shape()(2)-1,
		 0);
  Int beginChannel=cubeSlice*nchanPerSlice_p;
  Int endChannel=beginChannel+nchanPerSlice_p-1;
  if(cubeSlice==(nCubeSlice-1))
    endChannel=sm_->image(model).shape()(3)-1;
  blc(3)=beginChannel;
  trc(3)=endChannel;
  sl_p=Slicer (blc, trc, Slicer::endIsLast);
  SubImage<Complex>* sliceIm= new SubImage<Complex>(sm_->cImage(model), sl_p, True);
  //if(slice) delete slice;
  //slice=0;

  if(typeOfSlice==0){
    slice=new TempImage<Complex> (sliceIm->shape(), sliceIm->coordinates());
    //slice.copyData(sliceIm);
    slice->set(Complex(0.0, 0.0));
    //slice->setCoordinateInfo(sm_->image(model).coordinates());
    delete sliceIm;
  }
  else{
    slice=sliceIm;
  }

}


void CubeSkyEquation::sliceCube(SubImage<Float>*& slice,
				  ImageInterface<Float>& image, Int cubeSlice, 
				  Int nCubeSlice){

  IPosition blc(4,0,0,0,0);
  IPosition trc(4,image.shape()(0)-1,
		image.shape()(1)-1,image.shape()(2)-1,
		 0);
  Int beginChannel=cubeSlice*nchanPerSlice_p;
  Int endChannel=beginChannel+nchanPerSlice_p-1;
  if(cubeSlice==(nCubeSlice-1))
    endChannel=image.shape()(3)-1;
  blc(3)=beginChannel;
  trc(3)=endChannel;
  sl_p=Slicer(blc, trc, Slicer::endIsLast);
  //writeable if possible
  slice=  new SubImage<Float> (image, sl_p, True);
}


VisBuffer& CubeSkyEquation::getSlice(VisBuffer& result, Int model, 
				     Bool incremental,
				     Int cubeSlice, Int nCubeSlice) {

  Int nRow=result.nRow();

  result.modelVisCube(); // get the visibility so vb will have it
  VisBuffer vb(result);

  Bool FTChanged=changedFTMachine(vb);

  // we might need to recompute the "sky" for every single row, but we
  // avoid this if possible.
  internalChangesGet_p=False;  // Does this VB change inside itself?
  firstOneChangesGet_p=False;  // Has this VB changed from the previous one?
  if(ft_->name()!="MosaicFT"){
    changedSkyJonesLogic(result, firstOneChangesGet_p, internalChangesGet_p);
  } 
  if(internalChangesGet_p || internalChangesPut_p) {
    if(internalChangesPut_p)
      internalChangesPut_p=False;
    // Yes there are changes within this buffer: go row by row.
    // This will automatically catch a change in the FTMachine so
    // we don't have to check for that.
    for (Int row=0; row<nRow; row++) {
      finalizeGetSlice();
      initializeGetSlice(result, row, model, False, cubeSlice, 
			 nCubeSlice);
      if(incremental)
	ft_->get(vb,row);
      else
	ft_->get(result, row);
    }
  }
  else if (FTChanged || firstOneChangesGet_p || firstOneChangesPut_p) {
    if(firstOneChangesPut_p)
      firstOneChangesPut_p=False;
    // This buffer has changed wrt the previous buffer, but
    // this buffer has no changes within it. Again we don't need to
    // check for the FTMachine changing.

    finalizeGetSlice();
    
    initializeGetSlice(result, 0, model, False, cubeSlice, nCubeSlice);
    if(incremental)
      ft_->get(vb);
    else
      ft_->get(result);
  }
  else {
    if(incremental)
      ft_->get(vb);
    else
      ft_->get(result);
  }
  if(incremental)
    result.modelVisCube()+=vb.modelVisCube();
  return result;

}

  void CubeSkyEquation::finalizeGetSlice(){


  }

  Bool CubeSkyEquation::getFreqRange(ROVisibilityIterator& vi, 
				     const CoordinateSystem& coords, 
				     Int slice, Int nslice){
    // Only one slice lets keep what the user selected
    if(nslice==1)
      return False;

    Double start=0.0; 
    Double end=0.0;
    Double chanwidth=1.0;
    Int specIndex=coords.findCoordinate(Coordinate::SPECTRAL);
    SpectralCoordinate specCoord=coords.spectralCoordinate(specIndex);
    Vector<Int>spectralPixelAxis=coords.pixelAxes(specIndex);
    if(nchanPerSlice_p>0){
      specCoord.toWorld(start,Double(slice*nchanPerSlice_p)-0.5);
      specCoord.toWorld(end, Double(nchanPerSlice_p*(slice+1))-0.5);
      chanwidth=fabs(end-start)/Double(nchanPerSlice_p);
    }

    Block<Vector<Int> > spwb;
    Block<Vector<Int> > startb;
    Block<Vector<Int> > nchanb;
    Block<Vector<Int> > incrb=blockChanInc_p;
    cout << "freq start " << start << " freqend " << end << " width " << chanwidth << endl;
    vi.getSpwInFreqRange(spwb, startb, nchanb, start, end, chanwidth);
    if(spwb.nelements()==0)
      return False;
    cout << "start " << startb[0][0] << " nchan " << nchanb[0][0] << " spw " 
	 << spwb[0][0] << endl;
    //vi.selectChannel(1, startb[0][0], nchanb[0][0], 1, spwb[0][0]); 
    vi.selectChannel(blockNumChanGroup_p, startb, nchanb, incrb, spwb); 

    return True;
    
  }



} //# NAMESPACE CASA - END

//# MFCleanImageSkyModel.cc: Implementation of MFCleanImageSkyModel class
//# Copyrighogt (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <synthesis/MeasurementComponents/MFCleanImageSkyModel.h>
#include <images/Images/PagedImage.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>

#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/ConvolutionEquation.h>
#include <synthesis/MeasurementEquations/ClarkCleanModel.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>


namespace casa {

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define hclean hclean_
#define maximg maximg_
#endif
extern "C" {
  void hclean(Float*, Float*, Float*, int*, Float*, int*, int*, int*,
              int*, int*, int*, int*, int*, int*, int*, Float*, Float*,
              Float*, void *, void *);
};
extern "C" {
  void maximg(Float*, int*, Float*, int*, int*, int*, Float*, Float*);
};

// This is defined in HogbomCleanImageSkyModel.cc
void HogbomCleanImageSkyModelstopnow (Int *yes);

// This is defined in HogbomCleanImageSkyModel.cc
void HogbomCleanImageSkyModelmsgput(Int* npol, Int* pol, Int* iter, Int* px, Int* py, Float* fMaxVal);


Int MFCleanImageSkyModel::add(ImageInterface<Float>& image,
			      const Int maxNumXfr)
{
  return CleanImageSkyModel::add(image, maxNumXfr);
};

Bool MFCleanImageSkyModel::addMask(Int image,
				   ImageInterface<Float>& mask)
{
  return CleanImageSkyModel::addMask(image, mask);
};

Bool MFCleanImageSkyModel::addResidual(Int image,
				       ImageInterface<Float>& residual) 
{
  return ImageSkyModel::addResidual(image, residual);
};

// Clean solver
Bool MFCleanImageSkyModel::solve(SkyEquation& se) {

  LogIO os(LogOrigin("MFCleanImageSkyModel","solve"));
  Bool converged=True;
  //Make the PSFs, one per field
  if(!donePSF_p){
    makeApproxPSFs(se);
  }
  // Validate PSFs for each field
  Vector<Float> psfmax(numberOfModels()); psfmax=0.0;
  Vector<Float> psfmaxouter(numberOfModels()); psfmaxouter=0.0;
  Vector<Float> psfmin(numberOfModels()); psfmin=1.0;
  Vector<Float> resmax(numberOfModels());
  Vector<Float> resmin(numberOfModels());

  Float maxSidelobe=0.0;
  Int model;
  for (model=0;model<numberOfModels();model++) {
    if(isSolveable(model)) {

      IPosition blc(PSF(model).shape().nelements(), 0);
      IPosition trc(PSF(model).shape()-1);
      blc(2) = 0;  trc(2) = 0;
      blc(3) = 0;  trc(3) = 0;
      
      SubLattice<Float> subPSF;
      Int k =0;
      Int numchan= PSF(model).shape()(3);
      //PSF of first non zero plane
      while(psfmax(model) < 0.1 && k< numchan){
        blc(3)= k;
	trc(3)=k;
	LCBox onePlane(blc, trc, PSF(model).shape());
	subPSF=SubLattice<Float> ( PSF(model), onePlane, True);
	{
	  LatticeExprNode node = max(subPSF);
	  psfmax(model) = node.getFloat();
	}
	++k;
      }
      {
	LatticeExprNode node = min(subPSF);
	psfmin(model) = node.getFloat();
      }
      // 4 pixels:  pretty arbitrary, but only look for sidelobes
      // outside the inner (2n+1) * (2n+1) square
      psfmaxouter(model) = maxOuter(subPSF, 4);  
      os << "Model " << model+1 << ": max, min, maxOuter PSF = "
	 << psfmax(model) << ", " << psfmin(model) << ", " <<
	psfmaxouter(model) << endl;
      if(abs(psfmin(model))>maxSidelobe) maxSidelobe=abs(psfmin(model));
      if(psfmaxouter(model) > maxSidelobe) maxSidelobe= psfmaxouter(model );
    }
  }
  os << LogIO::POST;
	
  Float absmax=threshold();
  Float cycleThreshold=0.0;
  Block< Vector<Int> > iterations(numberOfModels());
  Int maxIterations=0;
  Int oldMaxIterations=0;
    
  // Loop over major cycles
  Int cycle=0;
  Bool stop=False;

  if (displayProgress_p) {
    progress_p = new ClarkCleanProgress( pgplotter_p );
  } else {
    progress_p = 0;
  }
  
  //Making the mask matrices
  Int nmodels=numberOfModels();
  Block< Vector<Int> > xbeg(nmodels), ybeg(nmodels), xend(nmodels), 
                       yend(nmodels);
  Vector<Bool> isCubeMask(nmodels);
  isCubeMask.set(False);
  Int nx, ny, npol;
  Int nchan=image(0).shape()(3);
  PtrBlock< Matrix<Float>* > lmaskCube(nmodels*nchan);

  for (model=0;model<numberOfModels();model++) {
    
    nx=image(model).shape()(0);
    ny=image(model).shape()(1);
    npol=image(model).shape()(2);
    nchan=image(model).shape()(3);
    xbeg[model].resize(nchan);
    xend[model].resize(nchan);
    ybeg[model].resize(nchan);
    yend[model].resize(nchan);
    for (Int chan=0; chan < nchan; ++chan){
      xbeg[model][chan]=0;
      ybeg[model][chan]=0;
      xend[model][chan]=nx-1;
      yend[model][chan]=ny-1;
      lmaskCube[chan*nmodels+model]=0;
    }
    if(hasMask(model)){
      Int mx=mask(model).shape()(0);
      Int my=mask(model).shape()(1);
      Int mpol=mask(model).shape()(2);
      AlwaysAssert(mx==nx, AipsError);
      AlwaysAssert(my==ny, AipsError);
      AlwaysAssert(mpol==npol, AipsError);
      LatticeStepper mls(mask(model).shape(),
		       IPosition(4, nx, ny, 1, 1),
		       IPosition(4, 0, 1, 2, 3));
      RO_LatticeIterator<Float>maskli(mask(model), mls);
      maskli.reset();
      if(nchan > 1){
	if(nchan==mask(model).shape()(3)){
	  isCubeMask[model]=True;
	  for (Int chan=0; chan < nchan; ++chan){
	    lmaskCube[chan*nmodels+model]=makeMaskMatrix(nx,ny, maskli, 
							   xbeg[model][chan],
							   xend[model][chan],
							   ybeg[model][chan],
							   yend[model][chan]);
	    maskli++;
	  }
	}
	else{
	  //Using first plane mask
	  lmaskCube[model]=makeMaskMatrix(nx,ny, maskli, 
					  xbeg[model][0],
					  xend[model][0],
					  ybeg[model][0],
					  yend[model][0]);
	  for (Int chan=1; chan < nchan; ++chan){
	    xbeg[model][chan]=xbeg[model][0];
	    ybeg[model][chan]=ybeg[model][0];
	    xend[model][chan]=xend[model][0];
	    yend[model][chan]=yend[model][0];
	  }

	}
      }
      else{
	lmaskCube[model]=makeMaskMatrix(nx,ny, maskli, 
					xbeg[model][0],
					xend[model][0],
					ybeg[model][0],
					yend[model][0]);


      }
    }



  }

    
  Float maxggS=0.0;

  while(absmax>=threshold()&&maxIterations<numberIterations()&&!stop) {

    os << "*** Starting major cycle " << cycle+1 << LogIO::POST;
    cycle++;

    // Make the residual images. We do an incremental update
    // for cycles after the first one. If we have only one
    // model then we use convolutions to speed the processing
    os << "Making residual images for all fields" << LogIO::POST;
    if(modified_p) 
      makeNewtonRaphsonStep(se, (cycle>1));

    if(0) {
      ostringstream name;
      name << "Residual" << cycle;
      PagedImage<Float> thisCycleImage(residual(0).shape(),
				       residual(0).coordinates(),
				       name.str());
      thisCycleImage.copyData(residual(0));
    }

    if(cycle==1) {
      for (Int model=0;model<numberOfModels();model++) {
	LatticeExprNode LEN = max(LatticeExpr<Float>(ggS(model)));
	Float thisMax = LEN.getFloat();
	if(thisMax>maxggS) maxggS=thisMax;
	if(0) {
	  ostringstream name;
	  name << "ggS";
	  PagedImage<Float> thisImage(residual(0).shape(),
				      residual(0).coordinates(),
				      name.str());
	  thisImage.copyData(ggS(0));
	}
      }
      os << "Maximum sensitivity = " << 1.0/sqrt(maxggS)
	 << " Jy/beam" << LogIO::POST;
    }

    if(0) {
      ostringstream name;
      name << "ggSxResidual" << cycle;
      PagedImage<Float> thisCycleImage(residual(0).shape(),
				       residual(0).coordinates(),
				       name.str());
      thisCycleImage.copyData(LatticeExpr<Float>(sqrt(ggS(0)/maxggS)*residual(0)));
    }

    absmax=maxField(resmax, resmin);

    for (model=0;model<numberOfModels();model++) {
      os << "Model " << model+1 << ": max, min (weighted) residuals = "
	 << resmax(model) << ", " << resmin(model) << endl;
    }
    os << LogIO::POST;

    // Can we stop?
    if(absmax<threshold()) {
      os << "Reached stopping peak residual = " << absmax << LogIO::POST;
      stop=True;
    }
    else {
    
      // Calculate the threshold for this cycle. Add a safety factor
      // This will be fixed someday by an option for an increasing threshold
      Float fudge = cycleFactor_p * maxSidelobe;
      if (fudge > 0.8) fudge = 0.8;   // painfully slow!

      cycleThreshold=max(0.95*threshold(), fudge * absmax);
      os << "Maximum residual = " << absmax << ", cleaning down to "
	 << cycleThreshold << LogIO::POST;
      
      for (model=0;model<numberOfModels();model++) {
	
	nx=image(model).shape()(0);
	ny=image(model).shape()(1);
	npol=image(model).shape()(2);
	nchan=image(model).shape()(3);
	
	if(cycle==1) {
	  iterations[model].resize(nchan);
	  iterations[model]=0;
	}
	  
	// Initialize delta image
        deltaImage(model).set(0.0);

	// Only process solveable models
	if(isSolveable(model)) {
	  

          if((abs(resmax(model))>cycleThreshold)||
	     (abs(resmin(model))>cycleThreshold)) {

	    os << "Processing model " << model << LogIO::POST;

	    IPosition onePlane(4, nx, ny, 1, 1);
	    IPosition oneCube(4, nx, ny, npol, 1);
	    
	    AlwaysAssert((npol==1)||(npol==2)||(npol==4), AipsError);
	    
	    // Loop over all channels. We only want the PSF for the first
	    // polarization so we iterate over polarization LAST

	    LatticeStepper psfls(PSF(model).shape(), onePlane,
				 IPosition(4,0,1,3,2));
	    RO_LatticeIterator<Float> psfli(PSF(model),psfls);

	    LatticeStepper fsls(ggS(model).shape(), oneCube,
				IPosition(4,0,1,2,3));
	    RO_LatticeIterator<Float> ggSli(ggS(model),fsls);

	    LatticeStepper ls(image(model).shape(), oneCube,
			      IPosition(4, 0, 1, 2, 3));
	    LatticeIterator<Float> imageStepli(residual(model), ls);
	    LatticeIterator<Float> imageli(image(model), ls);
	    LatticeIterator<Float> deltaimageli(deltaImage(model), ls);
	    
	      // If we are using a mask: assign the first ones
	    Matrix<Float> maskArray;
	    Bool domask=False;
	    if(hasMask(model)) {
		maskArray.resize(lmaskCube[model]->shape());
		maskArray= *(lmaskCube[model]);
		domask=True;
	    }

	    // Now clean each channel
	    Int chan=0;
	    for (imageStepli.reset(),imageli.reset(),psfli.reset(),
		   deltaimageli.reset(),ggSli.reset();
		 !imageStepli.atEnd()&&(chan<nchan);
		 imageStepli++,imageli++,psfli++,deltaimageli++,
		   ggSli++,chan++) {

	      if(hasMask(model) && isCubeMask[model] && chan >0) {
		maskArray= *(lmaskCube[chan*nmodels+model]); 
	      }
	      
	      if(psfmax(model)>0.0) {

		if(nchan>1) {
		  os<<"Processing channel "<<chan+1<<" of "<<nchan<<LogIO::POST;
		}

		// Renormalize by the weights 
		Matrix<Float> wmask(IPosition(2, nx, ny));
		wmask=0.0;
		//Trouble..
		Array<Float>  residu(imageStepli.cursor());
		Cube<Float> resid(residu.nonDegenerate(3));
		Cube<Float> weight(sqrt(ggSli.cursor().nonDegenerate(3)/maxggS));

		for (Int iy=0;iy<ny;iy++) {
		  for (Int ix=0;ix<nx;ix++) {
		    for (Int pol=0;pol<npol;pol++) {
		      resid(ix,iy,pol)*=weight(ix,iy,pol);
		      if(weight(ix,iy,pol)>0.0001) {
			wmask(ix,iy)=1.0;
		      }
		    }
		  }
		}
		if (domask) {
		  wmask*=maskArray;
		}
		Bool delete_its;
		Float* limageStep_data=resid.getStorage(delete_its);

		if (itsSubAlgorithm == "hogbom") {
		  Bool delete_itdi, delete_itp, delete_itm;
		  Float *ldeltaimage_data;
		  ldeltaimage_data=deltaimageli.rwCursor().getStorage(delete_itdi);
		  const Float *lpsf_data(0), *lmask_data(0);
		  lmask_data=wmask.getStorage(delete_itm);
		  lpsf_data=psfli.cursor().getStorage(delete_itp);
		  
		  Int niter=numberIterations();
		  Int starting_iteration = iterations[model](chan);
		  Int ending_iteration;

		  Float g=gain();
		  Float thres=cycleThreshold;
		  Int fxbeg=xbeg[model][chan]+1;
		  Int fxend=xend[model][chan]+1;
		  Int fybeg=ybeg[model][chan]+1;
		  Int fyend=yend[model][chan]+1;

		  Int domaskI=1;
		  hclean(ldeltaimage_data, limageStep_data, (Float*)lpsf_data,
			 &domaskI, (Float*)lmask_data,
			 &nx, &ny, &npol,
			 &fxbeg, &fxend, &fybeg, &fyend, &niter,
			 &starting_iteration, &ending_iteration,
			 &g, &thres, &cycleSpeedup_p,
			 (void*) &HogbomCleanImageSkyModelmsgput,
			 (void*) &HogbomCleanImageSkyModelstopnow);

		  os<<"Finished Hogbom clean inner cycle " << LogIO::POST;
		  
		  imageStepli.rwCursor().putStorage (limageStep_data, delete_its);
		  deltaimageli.rwCursor().putStorage (ldeltaimage_data, delete_itdi);
		  Cube<Float> step(deltaimageli.rwCursor().nonDegenerate(3));
		  for (Int iy=0;iy<ny;iy++) {
		    for (Int ix=0;ix<nx;ix++) {
		      for (Int pol=0;pol<npol;pol++) {
			if(weight(ix,iy,pol)>0.0) {
			  step(ix,iy,pol)/=weight(ix,iy,pol);
			}
		      }
		    }
		  }
		  deltaimageli.rwCursor()=step.addDegenerate(1);     	  
		  imageli.rwCursor()+=deltaimageli.cursor();
		  psfli.cursor().freeStorage (lpsf_data, delete_itp);
		  if (domask) {
		    maskArray.freeStorage (lmask_data, delete_itm);
		  }		  
		  iterations[model](chan) = ending_iteration;
		  maxIterations=(iterations[model](chan)>maxIterations) ?
		    iterations[model](chan) : maxIterations;
		  modified_p=True;
		  
		} else {  // clark is the default for now
		  
		  // Now make a convolution equation for this
		  // residual image and psf and then clean it
		  ConvolutionEquation eqn(psfli.matrixCursor(),
					  residu);
		  deltaimageli.rwCursor()=Float(0.0);
		  ClarkCleanModel cleaner(deltaimageli.rwCursor());
		  cleaner.setMask(wmask);
		  cleaner.setGain(gain());
		  cleaner.setNumberIterations(numberIterations());
		  cleaner.setInitialNumberIterations(iterations[model](chan));
		  cleaner.setThreshold(cycleThreshold);
		  cleaner.setPsfPatchSize(IPosition(2,51)); 
		  cleaner.setMaxNumberMajorCycles(10);
		  cleaner.setHistLength(1024);
		  cleaner.setMaxNumPix(32*1024);
		  cleaner.setChoose(False);
		  cleaner.setCycleSpeedup(cycleSpeedup_p);
		  cleaner.setSpeedup(0.0);
		  if ( displayProgress_p ) {
		    cleaner.setProgress( *progress_p );
		  }
		  cleaner.solve(eqn);
		  iterations[model](chan)=cleaner.numberIterations();
		  maxIterations=(iterations[model](chan)>maxIterations) ?
		    iterations[model](chan) : maxIterations;
		  modified_p=True;
		  cleaner.getModel(deltaimageli.rwCursor());
		  Cube<Float> step(deltaimageli.rwCursor().nonDegenerate(3));
		  for (Int iy=0;iy<ny;iy++) {
		    for (Int ix=0;ix<nx;ix++) {
		      for (Int pol=0;pol<npol;pol++) {
			if(weight(ix,iy,pol)>0.0) {
			  step(ix,iy,pol)/=weight(ix,iy,pol);
			}
		      }
		    }
		  }
		  deltaimageli.rwCursor()=step.addDegenerate(1); 
		  imageli.rwCursor()+=deltaimageli.cursor();
		  eqn.residual(imageStepli.rwCursor(), cleaner);
		  
		  os<<"Finished Clark clean inner cycle " << LogIO::POST;
		}
		if(maxIterations==0) {
		  stop=True;
		}
		else{
		  stop=False;
		}
		os << "Clean used " << iterations[model](chan) << " iterations" 
		   << " to approach a threshold of " << cycleThreshold
		   << LogIO::POST; 
	      }
	    }
	    os << LatticeExprNode(sum(image(model))).getFloat() 
	       << " Jy is the sum of clean components of model " 
	       << model << LogIO::POST;
	  }
	  else {
	    os<<"No need to clean model "<<model+1<<" :peak residual below threshold"
	      <<LogIO::POST;
	  }
	}
      }

      if(maxIterations != oldMaxIterations)
	oldMaxIterations=maxIterations;
      else {
	os << "No more clean occured in this major cycle - stopping now" << LogIO::POST;
	stop=True;
	converged=True;
      }
    }
    if(0) {
      ostringstream name;
      name << "Delta" << cycle;
      PagedImage<Float> thisCycleDeltaImage(residual(1).shape(),
					    residual(1).coordinates(),
					    name.str());
      thisCycleDeltaImage.copyData(deltaImage(1));
    }
  }
  if (progress_p) delete progress_p;
  for(model=0; model < nmodels; ++model){
    for(Int chan=0; chan < nchan; ++chan){
      if(lmaskCube[chan*nmodels+model] !=0) 
	delete lmaskCube[chan*nmodels+model];
    }
  }
  if(modified_p) {
    os << "Finalizing residual images for all fields" << LogIO::POST;
    makeNewtonRaphsonStep(se, False, True); //committing model to MS
    Float finalabsmax=maxField(resmax, resmin);
    
    os << "Final maximum residual = " << finalabsmax << LogIO::POST;
    converged=(finalabsmax < 1.05 * threshold());
    for (model=0;model<numberOfModels();model++) {
      os << "Model " << model+1 << ": max, min residuals = "
	 << resmax(model) << ", " << resmin(model) << endl;
    }
  }
  else {
    os << "Residual images for all fields are up-to-date" << LogIO::POST;
  }

  os << LogIO::POST;
  if(stop)
    converged=True;

  return(converged);
};
  
  
// Find maximum residual
Float MFCleanImageSkyModel::maxField(Vector<Float>& imagemax,
				     Vector<Float>& imagemin) {

  LogIO os(LogOrigin("ImageSkyModel","maxField"));
  
  Float absmax=0.0;
  imagemax=-1e20;
  imagemin=1e20;

  // Find maximum of ggS for scaling in maximg
  Float maxggS=0.0;
  for (Int model=0;model<numberOfModels();model++) {
    LatticeExprNode LEN = max(LatticeExpr<Float>(ggS(model)));
    Float thisMax = LEN.getFloat();
    if(thisMax>maxggS) maxggS=thisMax;
  }

  // Loop over all models
  for (Int model=0;model<numberOfModels();model++) {

    // Remember that the residual image can be either as specified
    // or created specially.
    ImageInterface<Float>* imagePtr=0;
    if(residual_p[model]) {
      imagePtr=residual_p[model];
    }
    else {
      imagePtr=(ImageInterface<Float> *)residualImage_p[model];
    }
    AlwaysAssert(imagePtr, AipsError);
    AlwaysAssert(imagePtr->shape().nelements()==4, AipsError);
    Int nx=imagePtr->shape()(0);
    Int ny=imagePtr->shape()(1);
    Int npol=imagePtr->shape()(2);
    Int nchan=imagePtr->shape()(3);

    AlwaysAssert((npol==1)||(npol==2)||(npol==4), AipsError);
    
    // Loop over all channels
    IPosition oneSlab(4, nx, ny, npol, 1);
    LatticeStepper ls(imagePtr->shape(), oneSlab, IPosition(4, 0, 1, 2, 3));
    IPosition onePlane(4, nx, ny, 1, 1);
    LatticeStepper fsls(ggS(model).shape(), oneSlab,
			IPosition(4,0,1,2,3));
    RO_LatticeIterator<Float> ggSli(ggS(model),fsls);
    LatticeIterator<Float> imageli(*imagePtr, ls);
    
    // If we are using a mask then reset the region to be
    // cleaned
    Array<Float> maskArray;
    RO_LatticeIterator<Float> maskIter;
    Bool cubeMask=False;
    
    Int domask=0;
    if(hasMask(model)) {
      Int mx=mask(model).shape()(0);
      Int my=mask(model).shape()(1);
      Int mpol=mask(model).shape()(2);
      Int nMaskChan=0;
      if(mask(model).shape().nelements()==4){
	nMaskChan=mask(model).shape()(3);
      }
      if( (nchan >1) && nMaskChan==nchan)
	cubeMask=True;
      AlwaysAssert(mx==nx, AipsError);
      AlwaysAssert(my==ny, AipsError);
      AlwaysAssert(mpol==npol, AipsError);
      LatticeStepper mls(mask(model).shape(), oneSlab,
			 IPosition(4, 0, 1, 2, 3));
      
      RO_LatticeIterator<Float> maskli(mask(model), mls);
      maskli.reset();
      maskIter=maskli;
      if (maskli.cursor().shape().nelements() > 1) {
	domask=1;
	maskArray=maskli.cursor();
      }
    }
    
    Int chan=0;
    Float imax, imin;
    imax=-1E20; imagemax(model)=imax;
    imin=+1E20; imagemin(model)=imin;

    for (imageli.reset(),ggSli.reset();!imageli.atEnd();imageli++,ggSli++,chan++) {
      Float fmax, fmin;
      Bool delete_its;

      // Renormalize by the weights
      if(cubeMask){
	if(maskIter.cursor().shape().nelements() > 1){
	  domask=1;
	  maskArray=maskIter.cursor();
	}
	maskIter++;
	
      }
      
      Cube<Float> weight(sqrt(ggSli.cursor().nonDegenerate(3)/maxggS));
      Cube<Float>resid(imageli.cursor().nonDegenerate(3));
      for (Int pol=0;pol<npol;pol++) {
	for (Int iy=0;iy<ny;iy++) {
	  for (Int ix=0;ix<nx;ix++) {
	    resid(ix,iy,pol)*=weight(ix,iy,pol);
	  }
	}
      }
      Float* limage_data=resid.getStorage(delete_its);
      Float* lmask_data=maskArray.getStorage(delete_its);
      maximg((Float*)limage_data, &domask, (Float*)lmask_data,
	     &nx, &ny, &npol, &fmin, &fmax);

      if(fmax<0.99*imax) fmax=0.0;
      if(fmin>0.99*imin) fmin=0.0;
      if(abs(fmax)>absmax) absmax=abs(fmax);
      if(abs(fmin)>absmax) absmax=abs(fmin);
      if(fmin<imagemin(model)) imagemin(model)=fmin;
      if(fmax>imagemax(model)) imagemax(model)=fmax;
    }
  }
  return absmax;
};
    

Float MFCleanImageSkyModel::maxOuter(Lattice<Float> & lat, const uInt nCenter ) 
{
  Float myMax=0.0;
  Float myMin=0.0;

  /*
  TempLattice<Float>  mask(lat.shape());
  mask.set(1.0);

  IPosition pos(4,0,0,0,0 );
  uInt nxc = lat.shape()(0)/2;
  uInt nyc = lat.shape()(1)/2;
  for (uInt ix = -nCenter; ix < nCenter; ix++) {
    for (uInt iy = -nCenter; iy < nCenter; iy++) {
      pos(0) = nxc + ix;
      pos(1) = nyc + iy;
      mask.putAt( 0.0f, pos );       //   mask out the inner section
    }
  }
  {
    LatticeExpr<Float> exp = (lat * mask);
    LatticeExprNode LEN = max( exp );
    myMax = LEN.getFloat();
  }
  {
    LatticeExpr<Float> exp = (lat * mask);
    LatticeExprNode LEN = min( exp );
    myMin = LEN.getFloat();
  }

  */

  Array<Float> arr = lat.get();
  IPosition pos( arr.shape() );
  uInt nx = lat.shape()(0);
  uInt ny = lat.shape()(1);
  uInt nxc = 0;
  uInt nyc = 0;
  Float amax = 0.0;
  for (uInt ix = 0; ix < nx; ix++) {
    for (uInt iy = 0; iy < ny; iy++) {
      if (arr(IPosition(4, ix, iy, 0, 0)) > amax) {
	nxc = ix;
	nyc = iy;
	amax = arr(IPosition(4, ix, iy, 0, 0));
      }
    }
  }

  uInt nxL = nxc - nCenter;
  uInt nxH = nxc + nCenter;
  uInt nyL = nyc - nCenter;
  uInt nyH = nyc + nCenter;
    
  for (uInt ix = 0; ix < nx; ix++) {
    for (uInt iy = 0; iy < ny; iy++) {
      if ( !(ix >= nxL && ix <= nxH &&  iy >= nyL && iy <= nyH) ) {
	if (arr(IPosition(4, ix, iy, 0, 0)) > myMax) 
	  myMax = arr(IPosition(4, ix, iy, 0, 0));
	if (arr(IPosition(4, ix, iy, 0, 0)) < myMin)
	  myMin = arr(IPosition(4, ix, iy, 0, 0));
      }
    }
  }

  Float absMax = max( abs(myMin), myMax );
  return absMax;
};

Matrix<Float>* MFCleanImageSkyModel::makeMaskMatrix(const Int& nx, 
							const Int& ny, 
							RO_LatticeIterator<Float>& maskIter,
							Int& xbeg,
							Int& xend,
							Int& ybeg,
							Int& yend) {

  LogIO os(LogOrigin("MFCleanImageSkyModel","makeMaskMatrix",WHERE)); 

    xbeg=0;
    ybeg=0;
    xend=nx-1;
    yend=ny-1;
  
  Matrix<Float>* mask= new Matrix<Float>(maskIter.matrixCursor().shape());
  (*mask)=maskIter.matrixCursor();
  // ignore mask if none exists

  if(max(*mask) < 0.5) {
    //If empty mask respect it for MF.... till we break WF from it
    return mask;
  }
  // Now read the mask and determine the bounding box
  xbeg=xend;
  ybeg=yend;
  yend=0;
  xend=0;
  
  for (Int iy=0;iy<ny;iy++) {
    for (Int ix=0;ix<nx;ix++) {
      if((*mask)(ix,iy)>0.5) {
	xbeg=min(xbeg,ix);
	ybeg=min(ybeg,iy);
	xend=max(xend,ix);
	yend=max(yend,iy);
      }
    }
  }

  return mask;
}

} //#End casa namespace

//# CSCleanImageSkyModel.cc: Implementation of CSCleanImageSkyModel class
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

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <synthesis/MeasurementComponents/CSCleanImageSkyModel.h>
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
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <synthesis/MeasurementEquations/ClarkCleanLatModel.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>

namespace casa {

Int CSCleanImageSkyModel::add(ImageInterface<Float>& image,
			      const Int maxNumXfr)
{
  return CleanImageSkyModel::add(image, maxNumXfr);
};

Bool CSCleanImageSkyModel::addMask(Int image,
				   ImageInterface<Float>& mask)
{
  return CleanImageSkyModel::addMask(image, mask);
};

Bool CSCleanImageSkyModel::addResidual(Int image,
				       ImageInterface<Float>& residual) 
{
  return ImageSkyModel::addResidual(image, residual);
};

// Clean solver
Bool CSCleanImageSkyModel::solve(SkyEquation& se) {

  LogIO os(LogOrigin("CSCleanImageSkyModel","solve"));
  Bool converged=True;
  //Make the PSFs, one per field

  os << "Making approximate Point Spread Functions" << LogIO::POST;
  if(!donePSF_p)
    makeApproxPSFs(se);

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
      // Changed the algorithm now..so that 4 is not used
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
  Float oldmax=absmax;
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

  while((absmax>=threshold())&& (maxIterations<numberIterations()) &&!stop) {

    os << "*** Starting major cycle " << cycle+1 << LogIO::POST;
    cycle++;

    // Make the residual images. We do an incremental update
    // for cycles after the first one. If we have only one
    // model then we use convolutions to speed the processing
    os << "Starting major cycle : making residual images for all fields"
       << LogIO::POST;
    if(modified_p) {
      Bool incremental(cycle>1);
      if (incremental&&(itsSubAlgorithm == "fast")) {
	os << "Using XFR-based shortcut for residual calculation"
	   << LogIO::POST;
	makeNewtonRaphsonStep(se, False);
      }
      else {
	os << "Using visibility-subtraction for residual calculation"
	   << LogIO::POST;
	makeNewtonRaphsonStep(se, incremental);
      }
      os << "Finished update of residuals"
	 << LogIO::POST;
    }

    oldmax=absmax;
    absmax=maxField(resmax, resmin);
    if(cycle==1) oldmax=absmax;

    for (model=0;model<numberOfModels();model++) {
      os << "Model " << model+1 << ": max, min residuals = "
	 << resmax(model) << ", " << resmin(model) << endl;
    }
    os << LogIO::POST;

    // Can we stop?
    if(absmax<threshold()) {
      os << "Reached stopping peak residual = " << absmax << LogIO::POST;
      stop=True;
    }
    else {
      if(oldmax < absmax){
	//Diverging ? lets increase the cyclefactor 
	cycleFactor_p=1.5*cycleFactor_p;
	oldmax=absmax;
      }
      // Calculate the threshold for this cycle. Add a safety factor
      // This will be fixed someday by an option for an increasing threshold
      Float fudge = cycleFactor_p * maxSidelobe;
      if (fudge > 0.8) fudge = 0.8;   // painfully slow!

      cycleThreshold=max(threshold(), fudge * absmax);
      os << "Maximum residual = " << absmax << ", cleaning down to "
	 << cycleThreshold << LogIO::POST;
      
      for (model=0;model<numberOfModels();model++) {
	
	Int nx=image(model).shape()(0);
	Int ny=image(model).shape()(1);
	Int npol=image(model).shape()(2);
	Int nchan=image(model).shape()(3);
	
	AlwaysAssert((npol==1)||(npol==2)||(npol==4), AipsError);
	    
	if(cycle==1) {
	  iterations[model].resize(nchan);
	  iterations[model]=0;
	}
	  
	// Initialize delta image
        deltaImage(model).set(0.0);

	// Only process solveable models
	if(isSolveable(model)&&psfmax(model)>0.0) {
	  
          if((abs(resmax(model))>cycleThreshold)||
	     (abs(resmin(model))>cycleThreshold)) {
	    
	    os << "Processing model " << model+1 << LogIO::POST;
	    
	    IPosition onePlane(4, nx, ny, 1, 1);
	    
	    IPosition oneCube(4, nx, ny, npol, 1);
	    
	    // Loop over all channels. We only want the PSF for the first
	    // polarization so we iterate over polarization LAST
	    
	    // Now clean each channel
	    for (Int chan=0;chan<nchan;++chan) {
	      if(nchan>1) {
		os<<"Processing channel "<<chan+1<<" of "<<nchan<<LogIO::POST;
	      }
	      LCBox psfbox(IPosition(4, 0, 0, 0, chan), 
			   IPosition(4, nx-1, ny-1, 0, chan),
			   PSF(model).shape());
	      SubLattice<Float>  psf_sl (PSF(model), psfbox, False);

	      LCBox imagebox(IPosition(4, 0, 0, 0, chan), 
			     IPosition(4, nx-1, ny-1, npol-1, chan), 
			     residual(model).shape());
	      

	      SubLattice<Float>  residual_sl (residual(model), imagebox, True);
	      SubLattice<Float>  image_sl (image(model), imagebox, True);
	      SubLattice<Float>  deltaimage_sl (deltaImage(model), imagebox, True);
	      // Now make a convolution equation for this
	      // residual image and psf and then clean it
	      {
		LatConvEquation eqn(psf_sl, residual_sl);
		ClarkCleanLatModel cleaner(deltaimage_sl);
		cleaner.setResidual(residual_sl);
		cleaner.setGain(gain());
		cleaner.setNumberIterations(numberIterations());
		cleaner.setInitialNumberIterations(iterations[model](chan));
		cleaner.setThreshold(cycleThreshold);
		cleaner.setPsfPatchSize(IPosition(2,51)); 
		cleaner.setMaxNumberMajorCycles(1);
	       	cleaner.setMaxNumberMinorIterations(100000);
		cleaner.setHistLength(1024);
		cleaner.setCycleFactor(cycleFactor_p);
		cleaner.setMaxNumPix(32*1024);
		cleaner.setChoose(False);
		//cleaner.setCycleSpeedup(cycleSpeedup_p);
		//Be a bit more conservative with pathologically bad PSFs
		if(maxSidelobe > 0.5)
		  cleaner.setMaxNumberMinorIterations(5);
		else if(maxSidelobe > 0.35)
		  cleaner.setMaxNumberMinorIterations(50);
		
		cleaner.setSpeedup(0.0);
		if ( displayProgress_p ) {
		  cleaner.setProgress( *progress_p );
		}
		
		os << "Starting minor cycle of Clean" << LogIO::POST;
		if(hasMask(model)) {
		  SubLattice<Float>  mask_sl (mask(model), psfbox, True);
		  cleaner.setMask(mask_sl);
		  cleaner.singleSolve(eqn, residual_sl);
		}
		else {
		  cleaner.singleSolve(eqn, residual_sl);
		}
		
		os << "Finished minor cycle of Clean"
		   << LogIO::POST;
		
		os << "Clean used " << cleaner.numberIterations()
		   << " iterations to approach a threshold of "
		   << cycleThreshold << LogIO::POST;
		modified_p=True;
		
		iterations[model](chan)=cleaner.numberIterations();
		maxIterations=(iterations[model](chan)>maxIterations) ?
		  iterations[model](chan) : maxIterations;
		
		os << "Adding increment to existing model" << LogIO::POST;
		LatticeExpr<Float> expr=image_sl+deltaimage_sl;
		image_sl.copyData(expr);
	      }
	    }
	    if(maxIterations==0) {
	      stop=True;
	    }
	    else{
	      stop=False;
	    }
	    os << LatticeExprNode(sum(image(model))).getFloat() 
	       << " Jy is the sum of clean components of model " 
	       << model << LogIO::POST; 
	  }
	  else {
	    os<<"Skipping model "<<model+1<<" :peak residual below threshold"
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
  }
  if (progress_p) delete progress_p;
  
  
  if(modified_p) {

    os << LatticeExprNode(sum(image(0))).getFloat() 
       << " Jy is the sum of clean components " << LogIO::POST;
    os << "Finalizing residual images for all fields" << LogIO::POST;
    makeNewtonRaphsonStep(se, False);
    Float finalabsmax=maxField(resmax, resmin);

    os << "Final maximum residual = " << finalabsmax << LogIO::POST;
    converged=(finalabsmax < threshold());
    for (model=0;model<numberOfModels();model++) {
      os << "Model " << model+1 << ": max, min residuals = "
	 << resmax(model) << ", " << resmin(model) << endl;
    }
  }
  else {
    os << "Residual images for all fields are up-to-date" << LogIO::POST;
  }

  os << LogIO::POST;

  return(converged);
};
  
  
// Find maximum residual
Float CSCleanImageSkyModel::maxField(Vector<Float>& imagemax,
				     Vector<Float>& imagemin) {

  LogIO os(LogOrigin("ImageSkyModel","maxField"));
  
  Float absmax=0.0;
  imagemax=-1e20;
  imagemin=1e20;

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
    
    AlwaysAssert((npol==1)||(npol==2)||(npol==4), AipsError);
    
    // Loop over all channels
    IPosition onePlane(4, nx, ny, 1, 1);
    LatticeStepper ls(imagePtr->shape(), onePlane, IPosition(4, 0, 1, 2, 3));
    RO_LatticeIterator<Float> imageli(*imagePtr, ls);
    
    // If we are using a mask then reset the region to be
    // cleaned
    Array<Float> maskArray;
    
    if(hasMask(model)) {
      Int mx=mask(model).shape()(0);
      Int my=mask(model).shape()(1);
      Int mpol=mask(model).shape()(2);
      AlwaysAssert(mx==nx, AipsError);
      AlwaysAssert(my==ny, AipsError);
      AlwaysAssert(mpol==npol, AipsError);
      LatticeStepper mls(mask(model).shape(), onePlane,
			 IPosition(4, 0, 1, 2, 3));
      
      RO_LatticeIterator<Float> maskli(mask(model), mls);
      maskli.reset();
      if (maskli.cursor().shape().nelements() > 1) maskArray=maskli.cursor();
    }
    
    Int chan=0;
    Float imax, imin;
    imax=-1E20; imagemax(model)=imax;
    imin=+1E20; imagemin(model)=imin;
    imageli.reset();

    for (imageli.reset();!imageli.atEnd();imageli++,chan++) {
      
      IPosition imageposmax(imageli.cursor().ndim());
      IPosition imageposmin(imageli.cursor().ndim());
      
      // If we are using a mask then multiply by it
      if (hasMask(model)) {
	Array<Float> limage=imageli.cursor();
	limage*=maskArray;
	minMax(imin, imax, imageposmin, imageposmax, limage);
      }
      else {
	minMax(imin, imax, imageposmin, imageposmax, imageli.cursor());
      }
      if(abs(imax)>absmax) absmax=abs(imax);
      if(abs(imin)>absmax) absmax=abs(imin);
      if(imin<imagemin(model)) imagemin(model)=imin;
      if(imax>imagemax(model)) imagemax(model)=imax;
    }
  }
  return absmax;
};
    

Float CSCleanImageSkyModel::maxOuter(Lattice<Float> & lat, const uInt nCenter ) 
{
  //Float myMax=0.0;
  //Float myMin=0.0;

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
  Float amax2=0.0;
  Float amin=1e9;
  Float amin2=1e9;
  Bool toggle=False;
  for (uInt ix = 0; ix < nx; ix++) {
    for (uInt iy = 0; iy < ny; iy++) {
      if(arr(IPosition(4, ix, iy, 0, 0)) < amin){
	amin2=amin;
	amin=arr(IPosition(4, ix, iy, 0, 0));
	toggle=True;
	  
      }
      if (arr(IPosition(4, ix, iy, 0, 0)) > amax) {
	nxc = ix;
	nyc = iy;
	if(toggle){
	  amax2=amax;
	  toggle=False;
	}
	amax = arr(IPosition(4, ix, iy, 0, 0));
      }
    }
  }

  //uInt nxL = nxc - nCenter;
  //uInt nxH = nxc + nCenter;
  //uInt nyL = nyc - nCenter;
  //uInt nyH = nyc + nCenter;
  /*
    
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
  */
  Float absMax=max(amax2, abs(amin));
  
  return absMax;

};

} //#End casa namespace

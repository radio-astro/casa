//# HogbomCleanImageSkyModel.cc: Implementation of HogbomCleanImageSkyModel class
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

#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayPosIter.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementComponents/HogbomCleanImageSkyModel.h>
#include <images/Images/PagedImage.h>
#include <casa/OS/File.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include <casa/System/Choice.h>


namespace casa { //# NAMESPACE CASA - BEGIN

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define hclean hclean_
#endif
extern "C" {
  void hclean(Float*, Float*, Float*, int*, Float*, int*, int*, int*,
              int*, int*, int*, int*, int*, int*, int*, Float*, Float*,
              Float*, void *, void *);
   };

void HogbomCleanImageSkyModelstopnow (Int *yes) {
  Vector<String> choices(2);
  choices(0)="Continue";
  choices(1)="Stop Now";
  LogMessage message(LogOrigin("HogbomCleanImageSkyModel","solve"));
  LogSink logSink;
  *yes=0;
  return;
  String choice=Choice::choice("Clean iteration: do you want to continue or stop?", choices);
  if (choice==choices(0)) {
    *yes=0;
  }
  else {
    message.message("Stopping");
    logSink.post(message);
    *yes=1;
  }
}

void HogbomCleanImageSkyModelmsgput(Int *npol, Int* pol, Int* iter, Int* px, Int* py,
				    Float* fMaxVal) {
  LogMessage message(LogOrigin("HogbomCleanImageSkyModel","solve"));
  ostringstream o; 
  LogSink logSink;

  String stokes("Unknown");

  if(*npol==1) {
    stokes="I";
  }
  else if(*npol==2) {
    if(*pol==1) {
      stokes="I";
    }
    else if(*pol==2) {
      stokes="V";
    }
  }
  else if(*npol==3) {
    if(*pol==1) {
      stokes="I";
    }
    else if(*pol==2) {
      stokes="Q";
    }
    else if(*pol==3) {
      stokes="U";
    }
  }
  else if(*npol==4) {
    if(*pol==1) {
      stokes="I";
    }
    else if(*pol==2) {
      stokes="Q";
    }
    else if(*pol==3) {
      stokes="U";
    }
    else if(*pol==4) {
      stokes="V";
    }
  }
  if(*pol==-1) {
    stokes="I";
  }
  else if(*pol==-2) {
    stokes="I,V";
  }
  else if(*pol==-3) {
    stokes="I,Q,U";
  }
  else if(*pol==-4) {
    stokes="I,Q,U,V";
  }
  
  if(*npol<0) {
    StokesVector maxVal(fMaxVal[0], fMaxVal[1], fMaxVal[2], fMaxVal[3]);
    if(*iter==0) {
      o<<stokes<<": Before iteration, peak is "<<maxVal<<" at "<<*px-1<<","<<*py-1;
      message.message(o);
      logSink.post(message);
    }
    else if(*iter>-1) {
      o<<stokes<<": Iteration "<<*iter<<" peak is "<<maxVal<<" at "<<*px-1<<","<<*py-1;
      message.message(o);
      logSink.post(message);
    }
    else {
      o<<stokes<<": Final iteration "<<abs(*iter)<<" peak is "<<maxVal<<" at "<<*px-1<<","<<*py-1;
      message.message(o);
      logSink.post(message);
    }
  }
  else {
    Float maxVal(fMaxVal[0]);
    if(*iter==0) {
      o<<stokes<<": Before iteration, peak is "<<maxVal<<" at "<<*px-1<<","<<*py-1;
      message.message(o);
      logSink.post(message);
    }
    else if(*iter>-1) {
      o<<stokes<<": Iteration "<<*iter<<" peak is "<<maxVal<<" at "<<*px-1<<","<<*py-1;
      message.message(o);
      logSink.post(message);
    }
    else {
      o<<stokes<<": Final iteration "<<abs(*iter)<<" peak is "<<maxVal<<" at "<<*px-1<<","<<*py-1;
      message.message(o);
      logSink.post(message);
    }
  }
  
}

// Clean solver
Bool HogbomCleanImageSkyModel::solve(SkyEquation& se) {
  
  LogIO os(LogOrigin("HogbomCleanImageSkyModel","solve"));
  
  Bool converged=True;
  if(numberOfModels()>1) {
    os << "Cannot process more than one field" << LogIO::EXCEPTION;
  }
  
  // Make the residual image
  if(modified_p)
    makeNewtonRaphsonStep(se);
  
  //Make the PSF
  makeApproxPSFs(se);
  
  if(!isSolveable(0)) {
    os << "Model 1 is not solveable!" << LogIO::EXCEPTION;
  }
  
  Int nx=image(0).shape()(0);
  Int ny=image(0).shape()(1);
  Int npol=image(0).shape()(2);
  Int nchan=image(0).shape()(3);
  Bool isCubeMask=False; 
  AlwaysAssert((npol==1)||(npol==2)||(npol==3)||(npol==4), AipsError);
  
  // Loop over all channels
  LatticeStepper psfls(PSF(0).shape(), IPosition(4,nx,ny,1,1),
		       IPosition(4,0,1,2,3));
  RO_LatticeIterator<Float> psfli(PSF(0),psfls);
  
  // Read the entire image for each spectral channel
  LatticeStepper ls(image(0).shape(),
		    IPosition(4, nx, ny, npol, 1),
		    IPosition(4, 0, 1, 2, 3));
  LatticeIterator<Float> imageStepli(residual(0), ls);
  LatticeIterator<Float> imageli(image(0), ls);
  
  // Now set up the mask.
  RO_LatticeIterator<Float>* maskli = 0;
  Matrix<Float>* lmask= new Matrix<Float>(1,1); 
  lmask->set(1.0);
  Int xbeg, xend;
  Int ybeg, yend; 
  //default clean support
  xbeg=nx/4; 
  xend=3*nx/4-1;
  ybeg=ny/4; 
  yend=3*ny/4-1;
  Bool domask=True;
  if(hasMask(0)) {
    domask=True;
    AlwaysAssert(mask(0).shape()(0)==nx, AipsError);
    AlwaysAssert(mask(0).shape()(1)==ny, AipsError);
    if(nchan >1){
      if(mask(0).shape()(3)==nchan){
	isCubeMask=True;
	os << "Using multichannel mask" << LogIO::POST;
      }
      else{
	os << "Image cube and mask donot match in number of channels" 
	   << LogIO::WARN;
	os << "Will use first plane of the mask for all channels" 
	   << LogIO::WARN;
      }
    }
    LatticeStepper mls(mask(0).shape(),
		       IPosition(4, nx, ny, 1, 1),
		       IPosition(4, 0, 1, 2, 3));
    maskli= new RO_LatticeIterator<Float>(mask(0), mls);
    maskli->reset();
    lmask=makeMaskMatrix(nx, ny, *maskli, xbeg, 
			 xend, ybeg, yend); 
  }
  else {
    domask=False;
  }
  
 
  
  Int chan=0;
  for (imageStepli.reset(),imageli.reset(),psfli.reset();
       !imageStepli.atEnd();
       imageStepli++,imageli++,psfli++,chan++) {
    //Deal with cube mask
    if(hasMask(0) && isCubeMask && chan >0) {
      (*maskli)++;
      lmask=makeMaskMatrix(nx, ny, *maskli, xbeg, 
			   xend, ybeg, yend);       
    }

    
    // Make IPositions and find position of peak of PSF
    IPosition psfposmax(psfli.cursor().ndim());
    IPosition psfposmin(psfli.cursor().ndim());
    Float psfmax;
    Float psfmin;
    minMax(psfmin, psfmax, psfposmin, psfposmax, psfli.cursor()); 
    
    if(nchan>1) {
      os<<"Processing channel "<<chan+1<<" of "<<nchan<<LogIO::POST;
    }
    if(psfmax==0.0) {
      os<<"No data for this channel: skipping"<<LogIO::POST;
    }
    else {
      Bool delete_iti, delete_its, delete_itp, delete_itm;
      const Float *lpsf_data, *lmask_data;
      Float *limage_data, *limageStep_data;
      limage_data=imageli.rwCursor().getStorage(delete_iti);
      limageStep_data=imageStepli.rwCursor().getStorage(delete_its);
      lmask_data=lmask->getStorage(delete_itm);
      lpsf_data=psfli.cursor().getStorage(delete_itp);
      
      Int niter=numberIterations();
      Float g=gain();
      Float thres=threshold();
      Int fxbeg=xbeg+1;
      Int fxend=xend;
      Int fybeg=ybeg+1;
      Int fyend=yend;
      Int domaskI;
      if (domask) {
	domaskI = 1;
      } else {
	domaskI = 0;
      }
      Int starting_iteration = 0;   // unutilized
      Int ending_iteration;         // unutilized
      //# The const of lpsf and mask has to be cast away for the
      //# fortran function hclean.
      Float cycleSpeedup = -1; // ie, ignore it
      hclean(limage_data, limageStep_data,
	     (Float*)lpsf_data, &domaskI, (Float*)lmask_data,
	     &nx, &ny, &npol,
	     &fxbeg, &fxend, &fybeg, &fyend, &niter,
	     &starting_iteration, &ending_iteration,
	     &g, &thres, &cycleSpeedup,
	     (void*) &HogbomCleanImageSkyModelmsgput,
	     (void*) &HogbomCleanImageSkyModelstopnow);
      imageli.rwCursor().putStorage (limage_data, delete_iti);
      imageStepli.rwCursor().putStorage (limageStep_data, delete_its);
      lmask->freeStorage (lmask_data, delete_itm);
      psfli.cursor().freeStorage (lpsf_data, delete_itp);
      Float residualmax, residualmin;
      minMax(residualmin, residualmax, imageStepli.cursor());
      residualmax=max(residualmax, abs(residualmin));
      converged = (residualmax < threshold());
    }
    if (lmask != 0 && isCubeMask)  {
      lmask->resize(1,1);
    }

  }
  modified_p=True;
  return(converged);
};

Matrix<Float>* HogbomCleanImageSkyModel::makeMaskMatrix(const Int& nx, 
						       const Int& ny, 
						       RO_LatticeIterator<Float>& maskIter,
						       Int& xbeg,
						       Int& xend,
						       Int& ybeg,
						       Int& yend) {

  LogIO os(LogOrigin("HogbomCleanImageSkyModel","makeMaskMatrix",WHERE)); 

  xbeg=nx/4;
  ybeg=ny/4;
  
  xend=xbeg+nx/2-1;
  yend=ybeg+ny/2-1;  
  Matrix<Float>* mask= new Matrix<Float>(maskIter.matrixCursor().shape());
  (*mask)=maskIter.matrixCursor();
  // ignore mask if none exists
  if(max(*mask) < 0.000001) {
    os << "Mask seems to be empty; will CLEAN inner quarter" 
       << LogIO::WARN;
    mask->resize(1,1);
    mask->set(1.0);
    return mask;
  }
  // Now read the mask and determine the bounding box
  xbeg=nx-1;
  ybeg=ny-1;
  xend=0;
  yend=0;

  for (Int iy=0;iy<ny;iy++) {
    for (Int ix=0;ix<nx;ix++) {
      if((*mask)(ix,iy)>0.000001) {
	xbeg=min(xbeg,ix);
	ybeg=min(ybeg,iy);
	xend=max(xend,ix);
	yend=max(yend,iy);

      }
    }
  }
  // Now have possible BLC. Make sure that we don't go over the
  // edge later
  if((xend - xbeg)>nx/2) {
    xbeg=nx/4-1; //if larger than quarter take inner of mask
    os << LogIO::WARN << "Mask span over more than half the x-axis: Considering inner half of the x-axis"  << LogIO::POST;
  } 
  if((yend - ybeg)>ny/2) { 
    ybeg=ny/4-1;
    os << LogIO::WARN << "Mask span over more than half the y-axis: Considering inner half of the y-axis" << LogIO::POST;
  }  
  xend=min(xend,xbeg+nx/2-1);
  yend=min(yend,ybeg+ny/2-1);   
  

  return mask;
}


} //# NAMESPACE CASA - END


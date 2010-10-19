//# Copyright (C) 1997-2010
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
//# $Id:  $

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/BasicMath/Math.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Logging/LogIO.h>
#include <casa/OS/File.h>
#include <casa/Containers/Record.h>

#include <lattices/Lattices/LCBox.h>
#include <casa/Arrays/Slicer.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/OS/HostInfo.h>
#include <casa/Arrays/ArrayError.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Arrays/VectorIter.h>


#include <casa/Utilities/GenSort.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/Fallible.h>

#include <casa/BasicSL/Constants.h>

#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <synthesis/MeasurementEquations/MatrixCleaner.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
namespace casa { //# NAMESPACE CASA - BEGIN

 
Bool MatrixCleaner::validatePsf(const Matrix<Float> & psf)
{
  LogIO os(LogOrigin("MatrixCleaner", "validatePsf()", WHERE));
  
  // Find the peak of the raw Psf
  AlwaysAssert(psf.shape().product() != 0, AipsError);
  Float maxPsf=0;
  itsPositionPeakPsf=IPosition(psf.shape().nelements(), 0);
  findMaxAbs(psf, maxPsf, itsPositionPeakPsf);
  os << "Peak of PSF = " << maxPsf << " at " << itsPositionPeakPsf
     << LogIO::POST;
  return True;
}
  
 
MatrixCleaner::MatrixCleaner():
  itsMask(0),
  itsDirty(0),
  itsXfr(0),
  itsScaleSizes(0),
  itsMaximumResidual(0.0),
  itsStrengthOptimum(0.0),
  itsChoose(True),
  itsDoSpeedup(False),
  itsIgnoreCenterBox(False),
  itsSmallScaleBias(0.6),
  itsStopAtLargeScaleNegative(False),
  itsStopPointMode(-1),
  itsDidStopPointMode(False),
  itsJustStarting(True),
  itsMaskThreshold(0.9),
  psfShape_p(0)
{
  itsMemoryMB=Double(HostInfo::memoryTotal()/1024)/16.0;
  itsScales.resize(0);
  itsScaleXfrs.resize(0);
  itsDirtyConvScales.resize(0);
  itsPsfConvScales.resize(0);
  itsScaleMasks.resize(0);
  itsScalesValid = False;
  itsStartingIter = 0;
  itsFracThreshold=Quantity(0.0, "%");
}

 

MatrixCleaner::MatrixCleaner(const Matrix<Float> & psf,
				  const Matrix<Float> &dirty):
  itsMask(0),
  itsScaleSizes(0),
  itsMaximumResidual(0.0),
  itsStrengthOptimum(0.),
  itsChoose(True),
  itsDoSpeedup(False),
  itsIgnoreCenterBox(False),
  itsSmallScaleBias(0.6),
  itsStopAtLargeScaleNegative(False),
  itsStopPointMode(-1),
  itsDidStopPointMode(False),
  itsJustStarting(True)
{
  AlwaysAssert(validatePsf(psf), AipsError);
  psfShape_p.resize(0, False);
  psfShape_p=psf.shape();
  // Check that everything is the same dimension and that none of the
  // dimensions is zero length.
  AlwaysAssert(psf.shape().nelements() == dirty.shape().nelements(),
	       AipsError);
  AlwaysAssert(dirty.shape().product() != 0, AipsError);
  // looks OK so make the convolver
  
  // We need to guess the memory use. For the moment, we'll assume
  // that about 4 scales will be used, giving about 32 TempLattices
  // in all. Also we'll try not to take more that half of the memory

  // Ah, but when we are doing a mosaic, its actually worse than this!
  // So, we pass it in
  itsMemoryMB=Double(HostInfo::memoryTotal()/1024)/16.0;

  itsDirty = new Matrix<Float>(dirty.shape());
  itsDirty->assign(dirty);
  setPsf(psf);
  itsScales.resize(0);
  itsScaleXfrs.resize(0);
  itsDirtyConvScales.resize(0);
  itsPsfConvScales.resize(0);
  itsScaleMasks.resize(0);
  itsScalesValid = False;
  itsStartingIter = 0;
  itsFracThreshold=Quantity(0.0, "%");
}


void MatrixCleaner::setPsf(const Matrix<Float>& psf){
  itsXfr=new Matrix<Complex>();
  AlwaysAssert(validatePsf(psf), AipsError);
  psfShape_p.resize(0, False);
  psfShape_p=psf.shape();
  FFTServer<Float,Complex> fft(psf.shape()); 
  fft.fft0(*itsXfr, psf);
  //cout << "shapes " << itsXfr->shape() << " psf " << psf.shape() << endl;
}

MatrixCleaner::MatrixCleaner(const MatrixCleaner & other)
   
{
  operator=(other);
}

MatrixCleaner & MatrixCleaner::operator=(const MatrixCleaner & other) {
  if (this != &other) {
    itsCleanType = other.itsCleanType;
    itsXfr = other.itsXfr;
    itsMask = other.itsMask;
    itsDirty = other.itsDirty;
    itsScales = other.itsScales;
    itsScaleXfrs = other.itsScaleXfrs;
    itsPsfConvScales = other.itsPsfConvScales;
    itsDirtyConvScales = other.itsDirtyConvScales;
    itsScaleMasks = other.itsScaleMasks;
    itsStartingIter = other.itsStartingIter;
    itsMaximumResidual = other.itsMaximumResidual;
    itsIgnoreCenterBox = other.itsIgnoreCenterBox;
    itsSmallScaleBias = other.itsSmallScaleBias;
    itsStopAtLargeScaleNegative = other.itsStopAtLargeScaleNegative;
    itsStopPointMode = other.itsStopPointMode;
    itsDidStopPointMode = other.itsDidStopPointMode;
    itsJustStarting = other.itsJustStarting;
    itsStrengthOptimum = other.itsStrengthOptimum;
    itsMaskThreshold = other.itsMaskThreshold;
    psfShape_p.resize(0, False);
    psfShape_p=other.psfShape_p;
  }
  return *this;
}

MatrixCleaner::~MatrixCleaner()
{
  destroyScales();
  if(!itsMask.null()) itsMask=0;
}


void MatrixCleaner::makeDirtyScales(){

  if(!itsScalesValid || itsNscales < 1 || itsDirty.null() || (itsNscales != Int(itsScaleXfrs.nelements())) )
    return;

  if( (psfShape_p) != (itsDirty->shape()))
    throw(AipsError("PSF and Dirty array are not of the same shape"));
  Matrix<Complex> dirtyFT;
  FFTServer<Float,Complex> fft(itsDirty->shape());
  fft.fft0(dirtyFT, *itsDirty);
  itsDirtyConvScales.resize(itsNscales, True);
  Int scale=0;
  //Having problem with fftw and omp
  // #pragma omp parallel default(shared) private(scale) firstprivate(fft)
  {
    //#pragma omp  for 
    // Dirty*scale
    for (scale=0; scale<itsNscales; scale++) {
      Matrix<Complex> cWork;
      // Dirty * scale
      //      cout << "scale " << scale << " itsScaleptr " << &(itsScaleXfrs[scale]) << "\n"<< endl;
      
      itsDirtyConvScales[scale]=Matrix<Float>(itsDirty->shape());
      cWork=((dirtyFT)*(itsScaleXfrs[scale]));
      fft.fft0((itsDirtyConvScales[scale]), cWork, False);
      fft.flip((itsDirtyConvScales[scale]), False, False);
    }
  }
  
} 
void MatrixCleaner::update(const Matrix<Float> &dirty)
{
  itsDirty->assign(dirty);

  LogIO os(LogOrigin("MatrixCleaner", "update()", WHERE));

  

  // Now we can redo the relevant convolutions
  makeDirtyScales();  
}



// add a mask image
void MatrixCleaner::setMask(Matrix<Float> & mask, const Float& maskThreshold) 
{
  itsMaskThreshold = maskThreshold;
  IPosition maskShape = mask.shape();

  //cerr << "Mask Shape " << mask.shape() << endl;
  // This is not needed after the first steps
  itsMask = new Matrix<Float>(mask.shape());
  itsMask->assign(mask);



  if (itsScalesValid) {
    makeScaleMasks();
  }

}

Bool MatrixCleaner::setcontrol(CleanEnums::CleanType cleanType,
				   const Int niter,
				   const Float gain,
				   const Quantity& threshold)
{
  return setcontrol(cleanType, niter, gain, threshold, Quantity(0.0, "%"));
}

// Set up the control parameters
Bool MatrixCleaner::setcontrol(CleanEnums::CleanType cleanType,
				   const Int niter,
				   const Float gain,
				   const Quantity& aThreshold,
				   const Quantity& fThreshold)
{
  itsCleanType=cleanType;
  itsMaxNiter=niter;
  itsGain=gain;
  itsThreshold=aThreshold;
  itsFracThreshold=fThreshold;
  return True;
}

// Set up speedup parameters
void MatrixCleaner::speedup(const Float nDouble)
{
  itsDoSpeedup=True;
  itsNDouble = nDouble;
};



// Do the clean as set up
Int MatrixCleaner::clean(Matrix<Float>& model,
			      Bool showProgress)
{
  AlwaysAssert(model.shape()==itsDirty->shape(), AipsError);

  LogIO os(LogOrigin("MatrixCleaner", "clean()", WHERE));

  Float tmpMaximumResidual=0.0;

  Int nScalesToClean=itsNscales;
  if (itsCleanType==CleanEnums::HOGBOM) {
    os << LogIO::NORMAL1 << "Hogbom clean algorithm" << LogIO::POST;
    nScalesToClean=1;
  }
  else if (itsCleanType==CleanEnums::MULTISCALE) {
    if (nScalesToClean==1) {
      os << LogIO::NORMAL1 << "Multi-scale clean with only one scale" << LogIO::POST;
    }
    else {
      os << LogIO::NORMAL1 << "Multi-scale clean algorithm" << LogIO::POST;
    }
  }

  Int scale;
  Vector<Float> scaleBias(nScalesToClean);
  if (nScalesToClean > 1) {
    os << LogIO::NORMAL1 << "Scale biases =";
    for (scale=0;scale<nScalesToClean;scale++) {
      scaleBias(scale) = 1 - itsSmallScaleBias *
	itsScaleSizes(scale)/itsScaleSizes(nScalesToClean-1);
      if(scale) os << ",";
      os << " " << scaleBias(scale);
    }
    os << LogIO::POST;
  } else {
    scaleBias(0) = 1.0;
  }

  AlwaysAssert(itsScalesValid, AipsError);

  // Find the peaks of the convolved Psfs
  Vector<Float> maxPsfConvScales(nScalesToClean);
  Int naxes=model.shape().nelements();
  #pragma omp parallel default(shared) private(scale)
  { 
    #pragma omp for 
    for (scale=0;scale<nScalesToClean;scale++) {
      IPosition positionPeakPsfConvScales(naxes, 0);
      
      findMaxAbs(itsPsfConvScales[scale], maxPsfConvScales(scale),
			positionPeakPsfConvScales);
 
      //   cout  << "MAX  " << scale << "    " << positionPeakPsfConvScales
      //	    << "  " << maxPsfConvScales(scale) << "\n"
      //	    << endl;
     }
  } //End pragma parallel
  for (scale=0;scale<nScalesToClean;scale++) {
     if ( maxPsfConvScales(scale) < 0.0) {
      os << "As Peak of PSF is negative, you should setscales again with a smaller scale size" 
	 << LogIO::SEVERE;
      return -1;
     }
  }

  // Define a subregion for the inner quarter
  IPosition blcDirty(model.shape().nelements(), 0);
  IPosition trcDirty(model.shape()-1);

  if(!itsMask.null()){
    os << "Cleaning using given mask" << LogIO::POST;
    if (itsMaskThreshold<0) {
        os << LogIO::NORMAL
           << "Mask thresholding is not used, values are interpreted as weights"
           <<LogIO::POST;
    } else {
        os << LogIO::NORMAL
           << "Cleaning pixels with mask values above " << itsMaskThreshold
           << LogIO::POST;
    }

    
    Int nx=model.shape()(0);
    Int ny=model.shape()(1);
    
    
    AlwaysAssert(itsMask->shape()(0)==nx, AipsError);
    AlwaysAssert(itsMask->shape()(1)==ny, AipsError);    
    Int xbeg=nx-1;
    Int ybeg=ny-1;
    Int xend=0;
    Int yend=0;
    for (Int iy=0;iy<ny;iy++) {
      for (Int ix=0;ix<nx;ix++) {
	if((*itsMask)(ix,iy)>0.000001) {
	  xbeg=min(xbeg,ix);
	  ybeg=min(ybeg,iy);
	  xend=max(xend,ix);
	  yend=max(yend,iy);
	}
      }
    }
    
    if (!itsIgnoreCenterBox) {
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
    }

    blcDirty(0)=xbeg;
    blcDirty(1)=ybeg;
    trcDirty(0)=xend;
    trcDirty(1)=yend;
  }
  else {
    if (itsIgnoreCenterBox) {
      os << LogIO::NORMAL << "Cleaning entire image" << LogIO::POST;
      os << LogIO::NORMAL1 << "as per MF/WF" << LogIO::POST; // ???
    }
    else {
      os << "Cleaning inner quarter of the image" << LogIO::POST;
      for (Int i=0;i<Int(model.shape().nelements());i++) {
	blcDirty(i)=model.shape()(i)/4;
	trcDirty(i)=blcDirty(i)+model.shape()(i)/2-1;
	if(trcDirty(i)<0) trcDirty(i)=1;
      }
    }
  }
  LCBox centerBox(blcDirty, trcDirty, model.shape());

  Block<Matrix<Float> > scaleMaskSubs;
  if (!itsMask.null())  {
    scaleMaskSubs.resize(itsNscales);
    for (Int is=0; is < itsNscales; is++) {
      scaleMaskSubs[is] = ((itsScaleMasks[is]))(blcDirty, trcDirty);
    }
  }

  // Start the iteration
  Vector<Float> maxima(nScalesToClean);
  Block<IPosition> posMaximum(nScalesToClean);
  Vector<Float> totalFluxScale(nScalesToClean); totalFluxScale=0.0;
  Float totalFlux=0.0;
  Int converged=0;
  Int stopPointModeCounter = 0;
  Int optimumScale=0;
  itsStrengthOptimum=0.0;
  IPosition positionOptimum(model.shape().nelements(), 0);
  os << "Starting iteration"<< LogIO::POST;

  itsIteration = itsStartingIter;
  for (Int ii=itsStartingIter; ii < itsMaxNiter; ii++) {
    itsIteration++;
    // Find the peak residual
    itsStrengthOptimum = 0.0;
    optimumScale = 0;

    #pragma omp parallel default(shared) private(scale)
    {
      #pragma omp  for 
      for (scale=0; scale<nScalesToClean; scale++) {
	// Find absolute maximum for the dirty image
	Matrix<Float> dirtySub=(itsDirtyConvScales[scale])(blcDirty,trcDirty);
	maxima(scale)=0;
	posMaximum[scale]=IPosition(model.shape().nelements(), 0);
	
	
	if (!itsMask.null()) {
	  findMaxAbsMask(dirtySub, (scaleMaskSubs[scale]),
				maxima(scale), posMaximum[scale]);
	} else {
	  findMaxAbs(dirtySub, maxima(scale), posMaximum[scale]);
	}
	
	// Remember to adjust the position for the window and for 
	// the flux scale
	//cout << "scale " << scale << " maxPsfconvscale " << maxPsfConvScales(scale) << endl;
	//cout << "posmax " << posMaximum[scale] << " blcdir " << blcDirty << endl;
	maxima(scale)/=maxPsfConvScales(scale);
	maxima(scale) *= scaleBias(scale);
	posMaximum[scale]+=blcDirty;
	
      }
    }//End parallel section
    for (scale=0; scale<nScalesToClean; scale++) {
      if(abs(maxima(scale))>abs(itsStrengthOptimum)) {
        optimumScale=scale;
        itsStrengthOptimum=maxima(scale);
        positionOptimum=posMaximum[scale];
      }
    }

    AlwaysAssert(optimumScale<nScalesToClean, AipsError);

    // Now add to the total flux
    totalFlux += (itsStrengthOptimum*itsGain);
    totalFluxScale(optimumScale) += (itsStrengthOptimum*itsGain);

    if(ii==itsStartingIter ) {
      itsMaximumResidual=abs(itsStrengthOptimum);
      tmpMaximumResidual=itsMaximumResidual;
      os << "Initial maximum residual is " << itsMaximumResidual
	 << LogIO::POST;
    }

    // Various ways of stopping:
    //    1. stop if below threshold
    if(abs(itsStrengthOptimum)<threshold() ) {
      os << "Reached stopping threshold " << threshold() << " at iteration "<<
            ii << LogIO::POST;
      os << "Optimum flux is " << abs(itsStrengthOptimum) << LogIO::POST;
      converged = 1;
      break;
    }
    //    2. negatives on largest scale?
    if ((nScalesToClean > 1) && itsStopAtLargeScaleNegative  && 
	optimumScale == (nScalesToClean-1) && 
	itsStrengthOptimum < 0.0) {
      os << "Reached negative on largest scale" << LogIO::POST;
      converged = -2;
      break;
    }
    //  3. stop point mode at work
    if (itsStopPointMode > 0) {
      if (optimumScale == 0) {
	stopPointModeCounter++;
      } else {
	stopPointModeCounter = 0;
      }
      if (stopPointModeCounter >= itsStopPointMode) {
	os << "Cleaned " << stopPointModeCounter << 
	  " consecutive components from the smallest scale, stopping prematurely"
	   << LogIO::POST;
	itsDidStopPointMode = True;
	converged = -1;
	break;
      }
    }
    //4. Diverging large scale
    //If actual value is 50% above the maximum residual. ..good chance it will not recover at this stage
    if(((abs(itsStrengthOptimum)-abs(tmpMaximumResidual)) > (abs(tmpMaximumResidual)/2.0)) 
       && !(itsStopAtLargeScaleNegative)){
      os << "Diverging due to large scale?"
	 << LogIO::POST;
       //clean is diverging most probably due to the large scale 
      converged=-2;
      break;
    }
    //5. Diverging for some other reason; may just need another CS-style reconciling
    if((abs(itsStrengthOptimum)-abs(tmpMaximumResidual)) > (abs(tmpMaximumResidual)/2.0)){
      os << "Diverging due to unknown reason"
       << LogIO::POST;
      converged=-3;
      break;
    }


    /*
    if(progress) {
      progress->info(False, itsIteration, itsMaxNiter, maxima,
		     posMaximum, itsStrengthOptimum,
		     optimumScale, positionOptimum,
		     totalFlux, totalFluxScale,
		     itsJustStarting );
      itsJustStarting = False;
      } else*/ {
      if (itsIteration == itsStartingIter + 1) {
          os << "iteration    MaximumResidual   CleanedFlux" << LogIO::POST;
      }
      if ((itsIteration % (itsMaxNiter/10 > 0 ? itsMaxNiter/10 : 1)) == 0) {
	//Good place to re-up the fiducial maximum residual
	//tmpMaximumResidual=abs(itsStrengthOptimum);
	os << itsIteration <<"      "<<itsStrengthOptimum<<"      "
	   << totalFlux <<LogIO::POST;
      }
    }

    Float scaleFactor;
    scaleFactor=itsGain*itsStrengthOptimum;

    // Continuing: subtract the peak that we found from all dirty images
    // Define a subregion so that that the peak is centered
    IPosition support(model.shape());
    support(0)=max(Int(itsScaleSizes(itsNscales-1)+0.5), support(0));
    support(1)=max(Int(itsScaleSizes(itsNscales-1)+0.5), support(1));

    IPosition inc(model.shape().nelements(), 1);
    //cout << "support " << support.asVector()  << endl;
    //support(0)=1024;
    //support(1)=1024;
    //support(0)=min(Int(support(0)), Int(trcDirty(0)-blcDirty(0)));
    //support(1)=min(Int(support(1)), Int(trcDirty(1)-blcDirty(1)));
    // support(0)=min(Int(support(0)), (trcDirty(0)-blcDirty(0)+
    //				Int(2*abs(positionOptimum(0)-blcDirty(0)/2.0-trcDirty(0)/2.0))));
    //support(1)=min(Int(support(1)), (trcDirty(1)-blcDirty(1)+
    //				Int(2*abs(positionOptimum(1)-blcDirty(1)/2.0-trcDirty(1)/2.0))));

    IPosition blc(positionOptimum-support/2);
    IPosition trc(positionOptimum+support/2-1);
    LCBox::verify(blc, trc, inc, model.shape());
    
    //cout << "blc " << blc.asVector() << " trc " << trc.asVector() << endl;

    IPosition blcPsf(blc+itsPositionPeakPsf-positionOptimum);
    IPosition trcPsf(trc+itsPositionPeakPsf-positionOptimum);
    LCBox::verify(blcPsf, trcPsf, inc, model.shape());
    makeBoxesSameSize(blc,trc,blcPsf,trcPsf);
    // cout << "blcPsf " << blcPsf.asVector() << " trcPsf " << trcPsf.asVector() << endl;
    //cout << "blc " << blc.asVector() << " trc " << trc.asVector() << endl;
    //    LCBox subRegion(blc, trc, model.shape());
    //  LCBox subRegionPsf(blcPsf, trcPsf, model.shape());
    
    Matrix<Float> modelSub=model(blc, trc);
    Matrix<Float> scaleSub=(itsScales[optimumScale])(blcPsf,trcPsf);
    
 
    // Now do the addition of this scale to the model image....
    modelSub += scaleFactor*scaleSub;

    #pragma omp parallel default(shared) private(scale)
    {
      #pragma omp  for 			
      for (scale=0;scale<nScalesToClean;scale++) {
      
	Matrix<Float> dirtySub=(itsDirtyConvScales[scale])(blc,trc);
	//AlwaysAssert(itsPsfConvScales[index(scale,optimumScale)], AipsError);
	Matrix<Float> psfSub=(itsPsfConvScales[index(scale,optimumScale)])(blcPsf, trcPsf);
	dirtySub -= scaleFactor*psfSub;
	    
      }
    }//End parallel
  }
  // End of iteration

  for (scale=0;scale<nScalesToClean;scale++) {
    os << LogIO::NORMAL
       << "  " << scale << "    " << totalFluxScale(scale)
       << LogIO::POST;
  }
  // Finish off the plot, etc.
  /*
  if(progress) {
    progress->info(True, itsIteration, itsMaxNiter, maxima, posMaximum,
		   itsStrengthOptimum,
		   optimumScale, positionOptimum,
		   totalFlux, totalFluxScale);
  }
  */

  if(!converged) {
    os << "Failed to reach stopping threshold" << LogIO::POST;
  }

  return converged;
}



Bool MatrixCleaner::findMaxAbs(const Matrix<Float>& lattice,
					  Float& maxAbs,
					  IPosition& posMaxAbs)
{

  posMaxAbs = IPosition(lattice.shape().nelements(), 0);
  maxAbs=0.0;

  Float minVal;
  IPosition posmin(lattice.shape().nelements(), 0);
  minMax(minVal, maxAbs, posmin, posMaxAbs, lattice);
  //cout << "min " << minVal << "  " << maxAbs << "   " << max(lattice) << endl;
  if(abs(minVal) > abs(maxAbs)){
    maxAbs=minVal;
    posMaxAbs=posmin;
  }
  return True;
}





Bool MatrixCleaner::findMaxAbsMask(const Matrix<Float>& lattice,
					      const Matrix<Float>& mask,
					      Float& maxAbs,
					      IPosition& posMaxAbs)
{

  posMaxAbs = IPosition(lattice.shape().nelements(), 0);
  maxAbs=0.0;
  Float minVal;
  IPosition posmin(lattice.shape().nelements(), 0);
  minMaxMasked(minVal, maxAbs, posmin, posMaxAbs, lattice, mask);
  if(abs(minVal) > abs(maxAbs)){
    maxAbs=minVal;
    posMaxAbs=posmin;
  }
 
  return True;
}



Bool MatrixCleaner::setscales(const Int nscales, const Float scaleInc)
{
  LogIO os(LogOrigin("deconvolver", "setscales()", WHERE));


  itsNscales=nscales;
  if(itsNscales<1) {
    os << "Using default of 5 scales" << LogIO::POST;
    itsNscales=5;
  }
  
  Vector<Float> scaleSizes(itsNscales);
  
  // Validate scales
  os << "Creating " << itsNscales << " scales" << LogIO::POST;
  scaleSizes(0) = 0.00001 * scaleInc;
  os << "scale 1 = 0.0 arcsec" << LogIO::POST;
  for (Int scale=1; scale<itsNscales;scale++) {
    scaleSizes(scale) =
      scaleInc * pow(10.0, (Float(scale)-2.0)/2.0);
    os << "scale " << scale+1 << " = " << scaleSizes(scale)
       << " arcsec" << LogIO::POST;
  }

  return setscales(scaleSizes);

}


void MatrixCleaner::setDirty(const Matrix<Float>& dirty){

  itsDirty=new Matrix<Float>(dirty.shape());
  itsDirty->assign(dirty);
  
  

} 

//Define the scales without doing anything else
// user will call make makePsfScales and makeDirtyScales like an adult in the know

void MatrixCleaner::defineScales(const Vector<Float>& scaleSizes){
  if(itsScales.nelements()>0) {
    destroyScales();
  }

  destroyMasks();
  itsNscales=scaleSizes.nelements();
  itsScaleSizes.resize(itsNscales);
  itsScaleSizes=scaleSizes;  // make a copy that we can call our own
  GenSort<Float>::sort(itsScaleSizes);
  itsScalesValid=False;
}

void MatrixCleaner::makePsfScales(){
  LogIO os(LogOrigin("MatrixCleaner", "mkePsfScales()", WHERE));
  if(itsNscales < 1)
    throw(AipsError("Scales have to be set"));
  if(itsXfr.null())
    throw(AipsError("Psf is not defined"));
  destroyScales();
  itsScales.resize(itsNscales, True);
  itsScaleXfrs.resize(itsNscales, True);
  itsPsfConvScales.resize((itsNscales+1)*(itsNscales+1), True);
  FFTServer<Float,Complex> fft(psfShape_p);
  Int scale=0;
  for(scale=0; scale<itsNscales;scale++) {
    itsScales[scale] = Matrix<Float>(psfShape_p);
    makeScale(itsScales[scale], itsScaleSizes(scale));
    itsScaleXfrs[scale] = Matrix<Complex> ();
    fft.fft0(itsScaleXfrs[scale], itsScales[scale]);
  }
  Matrix<Complex> cWork;
  
  for (scale=0; scale<itsNscales;scale++) {
    os << "Calculating convolutions for scale " << scale << LogIO::POST;
    //PSF * scale
    itsPsfConvScales[scale] = Matrix<Float>(psfShape_p);
    cWork=((*itsXfr)*(itsScaleXfrs[scale]));
    //cout << "shape "  << cWork.shape() << "   " << itsPsfConvScales[scale].shape() << endl;

    fft.fft0((itsPsfConvScales[scale]), cWork, False);
    fft.flip(itsPsfConvScales[scale], False, False);
    
    //cout << "psf scale " << scale << " " << max(itsPsfConvScales[scale]) << " " << min(itsPsfConvScales[scale]) << endl;

    for (Int otherscale=scale;otherscale<itsNscales;otherscale++) {
      
      AlwaysAssert(index(scale, otherscale)<Int(itsPsfConvScales.nelements()),
		   AipsError);
      
      // PSF *  scale * otherscale
      itsPsfConvScales[index(scale,otherscale)] =Matrix<Float>(psfShape_p);
      cWork=((*itsXfr)*conj(itsScaleXfrs[scale])*(itsScaleXfrs[otherscale]));
      fft.fft0(itsPsfConvScales[index(scale,otherscale)], cWork, False);
      //For some reason this complex->real fft  does not need a flip ...may be because conj(a)*a is real
      //fft.flip(*itsPsfConvScales[index(scale,otherscale)], False, False);
    }
  }
  
  itsScalesValid=True;

}

// We calculate all the scales and the corresponding convolutions
// and cross convolutions.

Bool MatrixCleaner::setscales(const Vector<Float>& scaleSizes)
{
  LogIO os(LogOrigin("MatrixCleaner", "setscales()", WHERE));

  Int scale;

  defineScales(scaleSizes);

  // Residual, psf, and mask, plus cross terms
  // e.g. for 5 scales this is 45. for 6 it is 60.
  Int nImages=3*itsNscales+itsNscales*(itsNscales+1);
  os << "Expect to use "  << nImages << " scratch images" << LogIO::POST;

  // Now we can update the size of memory allocated
  itsMemoryMB=0.5*Double(HostInfo::memoryTotal()/1024)/Double(nImages);
  os << "Maximum memory allocated per image "  << itsMemoryMB << "MB" << LogIO::POST;

  itsDirtyConvScales.resize(itsNscales);
  itsScaleMasks.resize(itsNscales);
  itsScaleXfrs.resize(itsNscales);
  itsPsfConvScales.resize((itsNscales+1)*(itsNscales+1));
  for(scale=0; scale<itsNscales;scale++) {
    itsDirtyConvScales[scale].resize();
    itsScaleMasks[scale].resize();
    itsScaleXfrs[scale].resize();
  }
  for(scale=0; scale<((itsNscales+1)*(itsNscales+1));scale++) {
    itsPsfConvScales[scale].resize();
  }

  AlwaysAssert(!itsDirty.null(), AipsError);

  FFTServer<Float,Complex> fft(itsDirty->shape());

  Matrix<Complex> dirtyFT;
  fft.fft0(dirtyFT, *itsDirty);

  ///Having problem with fftw with openmp
  //#pragma parallel default(shared) private(scale) firstprivate(fft)
  {
    //#pragma omp for 
    for (scale=0; scale<itsNscales;scale++) {
      os << "Calculating scale image and Fourier transform for scale " << scale << LogIO::POST;
      //cout << "Calculating scale image and Fourier transform for scale " << scale << endl;
      itsScales[scale] = Matrix<Float>(itsDirty->shape());
      //AlwaysAssert(itsScales[scale], AipsError);
      // First make the scale
      makeScale(itsScales[scale], scaleSizes(scale));
      itsScaleXfrs[scale] = Matrix<Complex> ();
       fft.fft0(itsScaleXfrs[scale], itsScales[scale]);
    }
  }
  
  // Now we can do all the convolutions
  Matrix<Complex> cWork;
  for (scale=0; scale<itsNscales;scale++) {
    os << "Calculating convolutions for scale " << scale << LogIO::POST;
    
    // PSF * scale
     itsPsfConvScales[scale] = Matrix<Float>(itsDirty->shape());

    cWork=((*itsXfr)*(itsScaleXfrs[scale]));

    fft.fft0((itsPsfConvScales[scale]), cWork, False);
    fft.flip(itsPsfConvScales[scale], False, False);
    
    itsDirtyConvScales[scale] = Matrix<Float>(itsDirty->shape());
    cWork=((dirtyFT)*(itsScaleXfrs[scale]));
    fft.fft0(itsDirtyConvScales[scale], cWork, False);
    fft.flip(itsDirtyConvScales[scale], False, False);
    ///////////
    /*
    {
      String axisName = "TabularDoggies1";
      String axisUnit = "km";
      Double crval = 10.12;
      Double crpix = -128.32;
      Double cdelt = 3.145;
      TabularCoordinate tab1(crval, cdelt, crpix, axisUnit, axisName);
      TabularCoordinate tab2(crval, cdelt, crpix, axisUnit, "Dogma");
      CoordinateSystem csys;
      csys.addCoordinate(tab1);
      csys.addCoordinate(tab2);
      PagedImage<Float> limage(itsPsfConvScales[scale].shape(), csys, "psfconvscale_"+String::toString(scale));
      limage.put(itsPsfConvScales[scale]);
    }
    */
      ///////////

    for (Int otherscale=scale;otherscale<itsNscales;otherscale++) {
      
      AlwaysAssert(index(scale, otherscale)<Int(itsPsfConvScales.nelements()),
		   AipsError);
      
      // PSF *  scale * otherscale
       itsPsfConvScales[index(scale,otherscale)] =Matrix<Float>(itsDirty->shape());
      cWork=((*itsXfr)*conj(itsScaleXfrs[scale])*(itsScaleXfrs[otherscale]));
      fft.fft0(itsPsfConvScales[index(scale,otherscale)], cWork, False);
      //fft.flip(*itsPsfConvScales[index(scale,otherscale)], False, False);
    }
  }

  itsScalesValid=True;

  if (!itsMask.null()) {
    makeScaleMasks();
  }

  return True;
}
  
// Make a single scale size image
void MatrixCleaner::makeScale(Matrix<Float>& iscale, const Float& scaleSize) 
{
  
  Int nx=iscale.shape()(0);
  Int ny=iscale.shape()(1);
  //Matrix<Float> iscale(nx, ny);
  iscale=0.0;
  
  Double refi=nx/2;
  Double refj=ny/2;
  
  if(scaleSize==0.0) {
    iscale(Int(refi), Int(refj)) = 1.0;
  }
  else {
    AlwaysAssert(scaleSize>0.0,AipsError);

    Int mini = max( 0, (Int)(refi-scaleSize));
    Int maxi = min(nx-1, (Int)(refi+scaleSize));
    Int minj = max( 0, (Int)(refj-scaleSize));
    Int maxj = min(ny-1, (Int)(refj+scaleSize));

    Float ypart=0.0;
    Float volume=0.0;
    Float rad2=0.0;
    Float rad=0.0;

    for (Int j=minj;j<=maxj;j++) {
      ypart = square( (refj - (Double)(j)) / scaleSize );
      for (Int i=mini;i<=maxi;i++) {
	rad2 =  ypart + square( (refi - (Double)(i)) / scaleSize );
	if (rad2 < 1.0) {
	  if (rad2 <= 0.0) {
	    rad = 0.0;
	  } else {
	    rad = sqrt(rad2);
	  }
	  iscale(i,j) = (1.0 - rad2) * spheroidal(rad);
	  volume += iscale(i,j);
	} else {
	  iscale(i,j) = 0.0;
	}
      }
    }
    iscale/=volume;
  }
  //scale.putSlice(iscale, IPosition(scale.ndim(),0), IPosition(scale.ndim(),1));
}

// Calculate the spheroidal function
Float MatrixCleaner::spheroidal(Float nu) {
  
  if (nu <= 0) {
    return 1.0;
  } else if (nu >= 1.0) {
    return 0.0;
  } else {
    uInt np = 5;
    uInt nq = 3;
    Matrix<Float> p(np, 2);
    Matrix<Float> q(nq, 2);
    p(0,0) = 8.203343e-2;
    p(1,0) = -3.644705e-1;
    p(2,0) =  6.278660e-1;
    p(3,0) = -5.335581e-1; 
    p(4,0) =  2.312756e-1;
    p(0,1) =  4.028559e-3;
    p(1,1) = -3.697768e-2; 
    p(2,1) = 1.021332e-1;
    p(3,1) = -1.201436e-1;
    p(4,1) = 6.412774e-2;
    q(0,0) = 1.0000000e0;
    q(1,0) = 8.212018e-1;
    q(2,0) = 2.078043e-1;
    q(0,1) = 1.0000000e0;
    q(1,1) = 9.599102e-1;
    q(2,1) = 2.918724e-1;
    uInt part = 0;
    Float nuend = 0.0;
    if (nu >= 0.0 && nu < 0.75) {
      part = 0;
      nuend = 0.75;
    } else if (nu >= 0.75 && nu <= 1.00) {
      part = 1;
      nuend = 1.0;
    }

    Float top = p(0,part);
    Float delnusq = pow(nu,2.0) - pow(nuend,2.0);
    uInt k;
    for (k=1; k<np; k++) {
      top += p(k, part) * pow(delnusq, (Float)k);
    }
    Float bot = q(0, part);
    for (k=1; k<nq; k++) {
      bot += q(k,part) * pow(delnusq, (Float)k);
    }
    
    if (bot != 0.0) {
      return (top/bot);
    } else {
      return 0.0;
    }
  }
}


// Calculate index into PsfConvScales
Int MatrixCleaner::index(const Int scale, const Int otherscale) {
  if(otherscale>scale) {
    return scale + itsNscales*(otherscale+1);
  }
  else {
    return otherscale + itsNscales*(scale+1);
  }
}
  

Bool MatrixCleaner::destroyScales()
{
  if(!itsScalesValid) return True;
  for(uInt scale=0; scale<itsScales.nelements();scale++) {
    //if(itsScales[scale]) delete itsScales[scale];
    itsScales[scale].resize();
  }
  for(uInt scale=0; scale<itsScaleXfrs.nelements();scale++) {
    //if(itsScaleXfrs[scale]) delete itsScaleXfrs[scale];
    itsScaleXfrs[scale].resize();
  }
  for(uInt scale=0; scale<itsDirtyConvScales.nelements();scale++) {
    //if(itsDirtyConvScales[scale]) delete itsDirtyConvScales[scale];
    itsDirtyConvScales[scale].resize();
  }
  for(uInt scale=0; scale<itsPsfConvScales.nelements();scale++) {
    //if(itsPsfConvScales[scale]) delete itsPsfConvScales[scale];
    itsPsfConvScales[scale].resize();
  }
  destroyMasks();
  itsScales.resize(0, True);
  itsDirtyConvScales.resize(0,True);
  itsPsfConvScales.resize(0, True);
  itsScalesValid=False;
  return True;
}



Bool MatrixCleaner::destroyMasks()
{
  for(uInt scale=0; scale<itsScaleMasks.nelements();scale++) {
    //if(itsScaleMasks[scale]) delete itsScaleMasks[scale];
    itsScaleMasks[scale].resize();
  }
  itsScaleMasks.resize(0);
  return True;
};

void MatrixCleaner::unsetMask()
{
  destroyMasks();
  if(!itsMask.null())
    itsMask=0;
}



// Set up the masks for the various scales
// This really only works for well behaved (ie, non-concave) masks.
// with only 1.0 or 0.0 values, and assuming the Scale images have
// a finite extent equal to +/- itsScaleSizes(scale)

Bool MatrixCleaner::makeScaleMasks()
{
  LogIO os(LogOrigin("deconvolver", "makeScaleMasks()", WHERE));
  Int scale;

  if(!itsScalesValid) {
    os << "Scales are not yet set - cannot set scale masks"
       << LogIO::EXCEPTION;
  }


  destroyMasks();

  if(itsMask.null())
    return False;

  AlwaysAssert((itsMask->shape() == psfShape_p), AipsError);
  //cout << "itsmask " << itsMask->shape() << endl;
  FFTServer<Float,Complex> fft(itsMask->shape());

  Matrix<Complex> maskFT;
  fft.fft0(maskFT, *itsMask);
  itsScaleMasks.resize(itsNscales, True);
  // Now we can do all the convolutions
  Matrix<Complex> cWork;
  for (scale=0; scale<itsNscales;scale++) {
    //AlwaysAssert(itsScaleXfrs[scale], AipsError);
    os << "Calculating mask convolution for scale " << scale+1 << LogIO::POST;
    
    // Mask * scale
     // Allow only 10% overlap by default, hence 0.9 is a default mask threshold
    // if thresholding is not used, just extract the real part of the complex mask
    itsScaleMasks[scale] = Matrix<Float>(itsMask->shape());
    cWork=((maskFT)*(itsScaleXfrs[scale]));
    fft.fft0(itsScaleMasks[scale], cWork, False);
    fft.flip(itsScaleMasks[scale], False, False);
    for (Int j=0 ; j < (itsMask->shape())(1); ++j){
      for (Int k =0 ; k < (itsMask->shape())(0); ++k){
	if(itsMaskThreshold > 0){
	  (itsScaleMasks[scale])(k,j) =  (itsScaleMasks[scale])(k,j) > itsMaskThreshold ? 1.0 : 0.0;
	}
      }
    }
    Float mysum = sum(itsScaleMasks[scale] );
    if (mysum <= 0.1) {
      os << LogIO::WARN << "Ignoring scale " << scale << 
	" since it is too large to fit within the mask" << LogIO::POST;
    }
    
  }

  return True;
}





Float MatrixCleaner::threshold() const
{
  if (! itsDoSpeedup) {
    return max(itsFracThreshold.get("%").getValue() * itsMaximumResidual /100.0,
	       itsThreshold.get("Jy").getValue());
  } else {
    const Float factor = exp( (Float)( itsIteration - itsStartingIter )/ itsNDouble )
      / 2.7182818;
    return factor * max(itsFracThreshold.get("%").getValue() * itsMaximumResidual /100.0,
		       itsThreshold.get("Jy").getValue());
  }
}



void MatrixCleaner::makeBoxesSameSize(IPosition& blc1, IPosition& trc1, 
                  IPosition &blc2, IPosition& trc2)
{
  const IPosition shape1 = trc1 - blc1;
  const IPosition shape2 = trc2 - blc2;

  AlwaysAssert(shape1.nelements() == shape2.nelements(), AipsError);
  
  if (shape1 == shape2) {
      return;
  }
  for (uInt i=0;i<shape1.nelements();++i) {
       Int minLength = shape1[i];
       if (shape2[i]<minLength) {
           minLength = shape2[i];
       }
       AlwaysAssert(minLength>=0, AipsError);
       //if (minLength % 2 != 0) {
           // if the number of pixels is odd, ensure that the centre stays 
           // the same by making this number even
           //--minLength; // this code is a mistake and should be removed
       //}
       const Int increment1 = shape1[i] - minLength;
       const Int increment2 = shape2[i] - minLength;
       blc1[i] += increment1/2;
       trc1[i] -= increment1/2 + (increment1 % 2 != 0 ? 1 : 0);
       blc2[i] += increment2/2;
       trc2[i] -= increment2/2 + (increment2 % 2 != 0 ? 1 : 0);
  }
}


} //# NAMESPACE CASA - END


//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
//# $Id: MultiTermMatrixCleaner.cc 13656 2010-12-04 02:08:02 UrvashiRV$

// Same include list as in MatrixCleaner.cc
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

// Additional include files
#include <lattices/Lattices/SubLattice.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <images/Images/PagedImage.h>

#include<synthesis/MeasurementEquations/MultiTermMatrixCleaner.h>

namespace casa { //# NAMESPACE CASA - BEGIN

#define MIN(a,b) ((a)<=(b) ? (a) : (b))
#define MAX(a,b) ((a)>=(b) ? (a) : (b))
	
  MultiTermMatrixCleaner::MultiTermMatrixCleaner():
    MatrixCleaner(),
    ntaylor_p(0),psfntaylor_p(0),nscales_p(0),nx_p(0),ny_p(0),totalIters_p(0),
    globalmaxpos_p(IPosition(0)),maxscaleindex_p(0),
    memoryMB_p(0),donePSF_p(False),donePSP_p(False),doneCONV_p(False),
    adbg(False)
  { }

  /*
MultiTermMatrixCleaner::MultiTermMatrixCleaner(const MultiTermMatrixCleaner & other):
  MatrixCleaner(),ntaylor_p(other.ntaylor_p)  //And others... minus some...
    {
    }

MultiTermMatrixCleaner &MultiTermMatrixCleaner:: operator=(const MultiTermMatrixCleaner & other) 
{
  operator=(other);
  return *this;
}
  */

 MultiTermMatrixCleaner::~MultiTermMatrixCleaner(){}


Bool MultiTermMatrixCleaner::setscales(const Vector<Float> & scales)
{
   nscales_p = scales.nelements();
   scaleSizes_p.resize();
   scaleSizes_p = scales;
   totalScaleFlux_p.resize(nscales_p);
   totalScaleFlux_p.set(0.0);
   maxScaleVal_p.resize(nscales_p);
   maxScaleVal_p.set(0.0);
   maxScalePos_p.resize(nscales_p);
   globalmaxval_p=-1e+10;
   globalmaxpos_p=IPosition(2,0);
   maxscaleindex_p=0;
   return True;
}


Bool MultiTermMatrixCleaner::setntaylorterms(const int & nterms)
{
   ntaylor_p = nterms;
   psfntaylor_p = 2*nterms-1;
   totalTaylorFlux_p.resize(ntaylor_p);
   totalTaylorFlux_p.set(0.0);
   return True;
}

// Allocate memory, based on nscales and ntaylor
Bool MultiTermMatrixCleaner::initialise(Int nx, Int ny)
{
  LogIO os(LogOrigin("MultiTermMatrixCleaner", "initialise()", WHERE));
  
  /* Verify Image Shapes */
  //  nx_p = model.shape(0);
  nx_p = nx;
  ny_p = ny;
  
  if(adbg) os << "Checking shapes" << LogIO::POST;
  
  /* Verify nscales_p and ntaylor_p */
  AlwaysAssert(nscales_p>0, AipsError);
  AlwaysAssert(ntaylor_p>0, AipsError);
  AlwaysAssert(scaleSizes_p.nelements()==nscales_p, AipsError);

  if(adbg) os << "Verify Scale sizes" << LogIO::POST;
  verifyScaleSizes();
  
  if(adbg) os << "Start allocating mem" << LogIO::POST;

  /* Allocate memory for many many TempMatrices. */
  /* Check on a return value of -1 - for insuffucient memory */
  if( allocateMemory() == -1 ) return False;

  /* FFTServer */
  fftcomplex = FFTServer<Float,Complex>(IPosition(2,nx_p,ny_p));
  
  /* Create the scale functions and their FTs */
  setupScaleFunctions();

  totalIters_p=0;
  prev_max_p = 1e+10;
  min_max_p = 1e+10;

  if(adbg) os << "Finished initializing MultiTermMatrixCleaner" << LogIO::POST;
  return True;
}


Bool MultiTermMatrixCleaner::setpsf(int order, Matrix<Float> & psf)
{
	AlwaysAssert((order>=(int)0 && order<(int)vecPsfFT_p.nelements()), AipsError);
	if(order==0) AlwaysAssert(validatePsf(psf), AipsError);
        // Directly store the FFT of the PSFs.
        // The FT'd matrix is reshaped automatically.
	fftcomplex.fft0(vecPsfFT_p[order],psf,False);
        //fftcomplex.flip(vecPsfFT_p[order],False,False);
	//        Matrix<Float> temp(real(vecPsfFT_p[order]));
	//        writeMatrixToDisk("psfFT_"+String::toString(order), temp);
        return True;
}

/* Input : Dirty Images */
Bool MultiTermMatrixCleaner::setresidual(int order, Matrix<Float> & dirty)
{
	AlwaysAssert((order>=(int)0 && order<(int)vecDirty_p.nelements()), AipsError);
	vecDirty_p[order].reference(dirty);
  return True;
}

/* Input : Model Component Image */

Bool MultiTermMatrixCleaner::setmodel(int order, Matrix<Float> & model)
{
	AlwaysAssert((order>=(int)0 && order<(int)vecModel_p.nelements()), AipsError);
	vecModel_p[order].assign(model);
	totalTaylorFlux_p[order] = (sum(vecModel_p[order]));
  return True;
}

/* Input : Mask */

Bool MultiTermMatrixCleaner::setmask(Matrix<Float> & mask)
{
  if(itsMask.null()) 
       itsMask = new Matrix<Float>(mask);  // it's a counted ptr
  else
    {
      AlwaysAssert(itsMask->shape()==mask.shape(), AipsError);
      (*itsMask).assign(mask);
    }
  return True;
}

/* Output : Model Component Image */

Bool MultiTermMatrixCleaner::getmodel(int order, Matrix<Float> & model)
{
	AlwaysAssert((order>=(int)0 && order<(int)vecModel_p.nelements()), AipsError);
	model.assign(vecModel_p[order]);
  return True;
}

/* Output Residual Image */
/*
Bool MultiTermMatrixCleaner::getresidual(int order, Matrix<Float> & residual)
{
	AlwaysAssert((order>=(int)0 && order<(int)vecDirty_p.nelements()), AipsError);
	//AlwaysAssert(residual, AipsError);
	residual.assign(vecDirty_p[order]);
  return True;
}
*/


 /* Output Hessian matrix */
Bool MultiTermMatrixCleaner::getinvhessian(Matrix<Double> & invhessian)
{
        invhessian.resize((invMatA_p[0]).shape());
	invhessian = (invMatA_p[0]); 
  return True;
}

/* Do the deconvolution */
Int MultiTermMatrixCleaner::mtclean(Int maxniter, Float stopfraction, Float inputgain, Float userthreshold)
{
  LogIO os(LogOrigin("MultiTermMatrixCleaner", "mtclean()", WHERE));
  if(adbg)os << "SOLVER for Multi-Frequency Synthesis deconvolution" << LogIO::POST;

  /* Initialize some variables */  

 maxniter_p = maxniter;
 stopfraction_p=  stopfraction;
 inputgain_p=inputgain;
 userthreshold_p=userthreshold;


  Int convergedflag = 0;
  Float fluxlimit = -1;
  Float loopgain = 0.5; 
  if(inputgain>(Float)0.0) loopgain=inputgain;
  Int criterion=5; // This chooses among a list of 'penalty functions'
  itercount_p=0;
 
  /* Compute all convolutions and the matrix A */
  /* If matrix is not invertible, return ! */
  /* This is done only once - for the first major cycle. Null operation otherwise */
  if( computeHessianPeak() == -2 )
	  return -2;

  /* Set up the Mask image */
  /* This must be after the Hessian is computed.. */
  setupUserMask();
   
  /* Compute the convolutions of the current residual with all PSFs and scales */
  os << "Calculating convolutions of residual images with scales " << LogIO::POST;
  computeRHS();

  /* Check if peak RHS is already less than the threshold */
  /* This step computes the flux limit also - when called here... */
  convergedflag = checkConvergence(criterion,fluxlimit,loopgain);
  if(convergedflag==1) return 0;
  
  /* Compute the flux limits that determine the depth of the minor cycles. */
  //  computeFluxLimit(fluxlimit,thresh);
  
 /********************** START MINOR CYCLE ITERATIONS ***********************/
  os << "Doing deconvolution iterations..." << LogIO::POST;
  for(itercount_p=0;itercount_p<maxniter_p;itercount_p++)
  {
      globalmaxval_p=-1e+10;
      Int scale=0;
      Int ntaylor=ntaylor_p;
      #pragma omp parallel default(shared) private(scale) firstprivate(ntaylor,criterion)
       { 
	 #pragma omp for 
          for(scale=0;scale<nscales_p;scale++)
          {
            /* Solve the matrix eqn for all pixels */
            solveMatrixEqn(ntaylor,scale);
            /* Choose a component from the list of solutions. Record the location for each scale.*/
            chooseComponent(ntaylor, scale,criterion);
	  }
       }//end pragma omp

       /* Find the best component over all scales */
       for(Int scale=0;scale<nscales_p;scale++)
       {
          if((maxScaleVal_p[scale]*scaleBias_p[scale]) > globalmaxval_p)
          {
            globalmaxval_p = maxScaleVal_p[scale];
            globalmaxpos_p = maxScalePos_p[scale];
           maxscaleindex_p = scale;
          }
       }

        /* Update the current solution by this chosen step */
        updateModelAndRHS(loopgain);
      
        /* Increment iteration count */
        totalIters_p++;
  
        /* Check for convergence */
        convergedflag = checkConvergence(criterion,fluxlimit,loopgain);

	/* Reached stopping threshold for this major cycle */
        /* Break out of minor-cycle loop, but signal that major cycles should continue */
       if(convergedflag)
        {
          break;
        }

  }//end minor cycle
  if(convergedflag==0)
    {
      os << "Reached max number of iterations for this minor cycle " << LogIO::POST;
    }

  /* returning because of non-convergence */
  if(convergedflag == -1)
    {
      os << "Stopping minor cycle iterations because of possible divergence." << LogIO::POST;
      //return(-1);
    }
  
  /********************** END MINOR CYCLE ITERATIONS ***********************/		
  
  /* Print out flux counts so far */
  //if(adbg)
  {
    os << "Total flux by scale :"; 
    for(Int scale=0;scale<nscales_p;scale++) os << "  [" << scaleSizes_p[scale] << "]: " << totalScaleFlux_p[scale] ;
     os << " (in this run) " << LogIO::POST;
     os << "Total flux by Taylor coefficient :";
     for(Int taylor=0;taylor<ntaylor_p;taylor++) os << "  [" << taylor << "]: " << totalTaylorFlux_p[taylor] ;
     os << LogIO::POST;
     //     for(Int taylor=0;taylor<ntaylor_p;taylor++) os << "Taylor " << taylor << " has total flux = " << totalTaylorFlux_p[taylor] << LogIO::POST;
  }

  /* Write out the multiscale model images - to disk */ 
  //  for(Int scale=0;scale<nscales_p;scale++)
  //  writeMatrixToDisk("modelimage_scale_"+String::toString(scale) , vecScaleModel_p[scale]);

  /* Return the number of minor-cycle iterations completed in this run */
  return(itercount_p+1);
}

/* Indexing Rules... */
Int MultiTermMatrixCleaner::IND2(Int taylor, Int scale)
{
	return  taylor * nscales_p + scale;
}

Int MultiTermMatrixCleaner::IND4(Int taylor1, Int taylor2, Int scale1, Int scale2)
{
	Int tt1=taylor1;
	Int tt2=taylor2;
	Int ts1=scale1;
	Int ts2=scale2;
	scale1 = MAX(ts1,ts2);
	scale2 = MIN(ts1,ts2);
	taylor1 = MAX(tt1,tt2);
	taylor2 = MIN(tt1,tt2);
	Int totscale = nscales_p*(nscales_p+1)/2;
	return ((taylor1*(taylor1+1)/2)+taylor2)*totscale + ((scale1*(scale1+1)/2)+scale2);
}


 /* Check if scale sizes are appropriate to the image size 
     If some scales are too big, ignore them.
     Reset nscales_p BEFORE all arrays are allocated */
Int MultiTermMatrixCleaner::verifyScaleSizes()
{
  LogIO os(LogOrigin("MultiTermMatrixCleaner", "verifyScaleSizes()", WHERE));
  Vector<Int> scaleflags(nscales_p); scaleflags=0;
  for(Int scale=0;scale<nscales_p;scale++)
   {
     if(scaleSizes_p[scale] > nx_p/2  || scaleSizes_p[scale] > ny_p/2)
       {
         scaleflags[scale]=1;
         os << LogIO::WARN << "Scale size of " << scaleSizes_p[scale] << " pixels is too big for an image size of " << nx_p << " x " << ny_p << " pixels. This scale size will be ignored.  " << LogIO::POST;
       }
   }
  if(sum(scaleflags)>0) // prune the scalevector and change nscales_p
    {
      Vector<Float> newscalesizes(nscales_p - sum(scaleflags));
      Int i=0;
      for(Int scale=0;scale<nscales_p;scale++)
	{
	  if(!scaleflags[scale])
          {
             AlwaysAssert(i<newscalesizes.nelements(),AipsError);
             newscalesizes[i]=scaleSizes_p[scale];
             i++;
          }
	}
          AlwaysAssert(i==newscalesizes.nelements(), AipsError);
          scaleSizes_p.assign(newscalesizes);
          nscales_p = newscalesizes.nelements();
          totalScaleFlux_p.resize(nscales_p,True);
          maxScaleVal_p.resize(nscales_p,True);
          maxScalePos_p.resize(nscales_p,True);

    }
  os << "Scale sizes to be used for deconvolution : " << scaleSizes_p << LogIO::POST;
}

/*************************************
 *          Allocate Memory
 *************************************/

Int MultiTermMatrixCleaner::allocateMemory()
{
       LogIO os(LogOrigin("MultiTermMatrixCleaner", "allocateMemory()", WHERE));
	// Define max memory usage. (half of available);
	
	Int ntotal4d = (nscales_p*(nscales_p+1)/2) * (ntaylor_p*(ntaylor_p+1)/2);

        Int ntempfull = ntotal4d                    // cubeA
                               + 1                          // scratch
	                       + nscales_p * 3      // vecScales, vecScaleMasks, vecWork_p, ////vecScaleModel_p
                               + ntaylor_p   // vecModel (vecDirty is a ref)
                               + 2 * nscales_p * ntaylor_p; // matR and matCoeff

        Int ntemphalf = 2  // scratch
	                        + nscales_p  // vecSCalesFT
                                + psfntaylor_p; // vecPsfFT

	Int numMB = nx_p*ny_p*4*(ntempfull + ntemphalf/2.0)/(1024*1024);
        memoryMB_p = Double(HostInfo::memoryTotal()/1024);

        if(adbg) os << "This algorithm needs to allocate " << numMB << " MBytes for " << ntempfull + ntemphalf << " Matrices. " << LogIO::POST;
        if (numMB > 0.75*memoryMB_p) 
       { 
           os << "This algorithm needs to allocate " << numMB << " MBytes for " << ntempfull + ntemphalf << " Matrices. " << LogIO::POST;
           os << LogIO::WARN << "Available memory for this process is  " << memoryMB_p << " MB. Please reduce imsize/nscales/nterms and try again " << LogIO::POST;
           return -1;
       }

        // shape of all matrices
	gip = IPosition(2,nx_p,ny_p);  
	IPosition tgip(2,ntaylor_p,ntaylor_p);
	
	// Small HessianPeak matrix to be inverted for each point..
	matA_p.resize(nscales_p); invMatA_p.resize(nscales_p);
	for(Int i=0;i<nscales_p;i++)
	{
	  matA_p[i].resize(tgip); 
	  invMatA_p[i].resize(tgip);
	}
	
	// I_D 
        dirtyFT_p.resize(); 
	
	// Temporary work-holder
        cWork_p.resize(); 
	//	tWork_p.resize(gip);
	
	// Scales 
	vecScales_p.resize(nscales_p);
	vecScalesFT_p.resize(nscales_p);
        vecScaleMasks_p.resize(nscales_p);
	vecWork_p.resize(nscales_p);
	//        vecScaleModel_p.resize(nscales_p);
	for(Int i=0;i<nscales_p;i++) 
	{
	  vecScales_p[i].resize(gip);
	  vecScalesFT_p[i].resize();
          vecScaleMasks_p[i].resize(gip);
	  vecWork_p[i].resize(gip);
	  //          vecScaleModel_p[i].resize(gip);
	  //          vecScaleModel_p[i] = 0.0;
	}
	
	// Psfs and Models 
	vecPsfFT_p.resize(psfntaylor_p);
	for(Int i=0;i<psfntaylor_p;i++) 
	{
                  vecPsfFT_p[i].resize();
	}
	
	// Dirty/Residual Images
	vecDirty_p.resize(ntaylor_p);
	vecModel_p.resize(ntaylor_p);
	for(Int i=0;i<ntaylor_p;i++) 
	{
	  vecDirty_p[i].resize();
	  vecModel_p[i].resize(gip);
	}
	
	// (Psf * Scales) * (Psf * Scales)
	cubeA_p.resize(ntotal4d);

	for(Int i=0;i<ntotal4d;i++) 
	{
		  cubeA_p[i].resize(gip); 
	}
	
	// I_D * (Psf * Scales)
	matR_p.resize(ntaylor_p*nscales_p);
	// Coefficients to be solved for.
	matCoeffs_p.resize(ntaylor_p*nscales_p);
	
	for(Int i=0;i<ntaylor_p*nscales_p;i++) 
	{
		  matR_p[i].resize(gip); 
		  matCoeffs_p[i].resize(gip);
                  matCoeffs_p[i] = 0.0;
	}
  
	return 0;
}

/* Make masks for each scale size */
/* At the end, force a scale-dependent border */
Int MultiTermMatrixCleaner::setupUserMask()
{
    if(itsMask.null())
    {
           /* Make a mask the full size, for all scales */
           for(Int scale=0;scale<nscales_p;scale++)
	    {
              vecScaleMasks_p[scale] = 1.0;
	    }
    }
    else
    {
      /* Compute the convolutions of the input mask with each scale size */
      if(adbg) os << "Convolving user mask with scales, and using 0.1 as a threshold." << LogIO::POST;
      Matrix<Complex> maskft;
      fftcomplex.fft0(maskft , *itsMask , False);

      for(Int scale=0;scale<nscales_p;scale++)
      {
        AlwaysAssert(maskft.shape() == vecScalesFT_p[scale].shape(),AipsError);
        cWork_p.assign(maskft * vecScalesFT_p[scale]);
       	fftcomplex.fft0( vecScaleMasks_p[scale] , cWork_p , False  );
        fftcomplex.flip( vecScaleMasks_p[scale] , False , False);
	
        for (Int j=0 ; j < (itsMask->shape())(1); ++j)
        for (Int k =0 ; k < (itsMask->shape())(0); ++k)
        {
	  if(itsMaskThreshold > (Float)0.0)  // if negative, use the continuous mask
                {
		  (vecScaleMasks_p[scale])(k,j) =  (vecScaleMasks_p[scale])(k,j) > (Float)0.1 ? 1.0 : 0.0;
	        }
        }
	
	//	       writeMatrixToDisk("scalemask."+String::toString(scale), vecScaleMasks_p[scale]);
      }// end of for scale

          /* TO DO... maybe :
             Map some local variables to those in the parant MatrixCleaner class 
             so that its functions can be used. Later, clean this up by directly using
             the MatrixCleaner variables.  Currently not possible because the 
             function that creates scales and populates itsScaleXfrs (setscales), 
             does a lot of extra stuff that we don't need here. */

          /* Must reuse itsNscales, itsScaleSizes, itsScales, itsScaleXfrs, itsScaleMasks, itsScalesValid 
              and eliminate : vecScales_p, scaleSizes_p, vecScalesFT_p, vecScaleMasks_p */
          /*
          itsScaleXfrs.resize(nscales_p);
          itsNscales = nscales_p;
          for(Int scale=0;scale<nscales_p;scale++)
	    {
	      itsScaleXfrs[scale].reference(vecScalesFT_p[scale]);
	    }
          itsScalesValid=True;

	  makeScaleMasks();
          AlwaysAssert(nscales_p==itsScaleMasks.nelements(),AipsError);

          for(Int scale=0;scale<nscales_p;scale++)
	    {
              cout << "Shapes for scale : " << scale << itsScaleMasks[scale] << endl;
              vecScaleMasks_p[scale].reference(itsScaleMasks[scale]);
	    }
	  */

    }// end of if else


 /* Set the edges of the masks according to the scale size */
 // Set the values OUTSIDE the box to zero....
  for(Int scale=0;scale<nscales_p;scale++)
  {
      Int border = (Int)(scaleSizes_p[scale]*1.5);
      // bottom
      IPosition blc1(2, 0 , 0 );
      IPosition trc1(2,nx_p-1, border );
      IPosition inc1(2, 1);
      LCBox::verify(blc1,trc1,inc1,vecScaleMasks_p[scale].shape());
      (vecScaleMasks_p[scale])(blc1,trc1) = 0.0;
      // top
      blc1[0]=0; blc1[1]=ny_p-border-1;
      trc1[0]=nx_p-1; trc1[1]=ny_p-1;
      LCBox::verify(blc1,trc1,inc1,vecScaleMasks_p[scale].shape());
      (vecScaleMasks_p[scale])(blc1,trc1) = 0.0;
      // left
      blc1[0]=0; blc1[1]=border;
      trc1[0]=border; trc1[1]=ny_p-border-1;
      LCBox::verify(blc1,trc1,inc1,vecScaleMasks_p[scale].shape());
      (vecScaleMasks_p[scale])(blc1,trc1) = 0.0;
      // right
      blc1[0]=nx_p-border-1; blc1[1]=border;
      trc1[0]=nx_p; trc1[1]=ny_p-border-1;
      LCBox::verify(blc1,trc1,inc1,vecScaleMasks_p[scale].shape());
      (vecScaleMasks_p[scale])(blc1,trc1) = 0.0;

      //      writeMatrixToDisk("maskforscale_"+String::toString(scale),vecScaleMasks_p[scale]);
  }

   return 0;
}/* end of setupUserMask() */


/***************************************
 *  Set up the Blobs of various scales.
 ****************************************/

Int MultiTermMatrixCleaner::setupScaleFunctions()
{
  LogIO os(LogOrigin("MultiTermMatrixCleaner", "setupScaleFunctions", WHERE));
	// Set the scale sizes
	if(scaleSizes_p.nelements()==0)
	{
		scaleSizes_p.resize(nscales_p);
		Float scaleInc = 2.0;
		scaleSizes_p[0] = 0.0;
		for (Int scale=1; scale<nscales_p;scale++) 
		{
			scaleSizes_p[scale] = scaleInc * pow(10.0, (Float(scale)-2.0)/2.0) ;
		}  
	}

        /* SCALE BIAS : Can get rid of this entirely. Leaving it in, just in case scalebiases are needed later */	
	scaleBias_p.resize(nscales_p);
	totalScaleFlux_p.resize(nscales_p);
	//Float prefScale=2.0;
	//Float fac=6.0;
	if(nscales_p>1)
	{
		for(Int scale=0;scale<nscales_p;scale++) 
		{
		  //scaleBias_p[scale] = 1 - itsSmallScaleBias * scaleSizes_p[scale]/scaleSizes_p(nscales_p-1);
			//scaleBias_p[scale] = 1 - 0.4 * scaleSizes_p[scale]/scaleSizes_p(nscales_p-1);
	 	  scaleBias_p[scale] = 1.0;
			//////scaleBias_p[scale] = pow((Float)scale/fac,prefScale)*exp(-1.0*scale/fac)/(pow(prefScale/fac,prefScale)*exp(-1.0*prefScale/fac));
			//scaleBias_p[scale] = pow((Float)(scale+1)/fac,prefScale)*exp(-1.0*(scale+1)/fac);
		  //			os << "scale " << scale+1 << " = " << scaleSizes_p(scale) << " pixels with bias = " << scaleBias_p[scale] << LogIO::POST;
			totalScaleFlux_p[scale]=0.0;
		}
	}
	else scaleBias_p[0]=1.0;
 
	// Compute the scaled blobs - prolate spheroids with tapering and truncation
	// vecScales_p, scaleSizes_p, vecScalesFT_p
	if(!donePSP_p) 
	{
		// Compute h(s1), h(s2),... depending on the number of scales chosen.
		// NSCALES = 1;
		if(adbg) os << "Calculating scales and their FTs " << LogIO::POST;
			
		for (Int scale=0; scale<nscales_p;scale++) 
		{
			// First make the scale
			makeScale(vecScales_p[scale], scaleSizes_p(scale));
			// Now store the XFR (shape of FT is set automatically)
                        fftcomplex.fft0(vecScalesFT_p[scale] , vecScales_p[scale] , False);
                        //fftcomplex.flip(vecScalesFT_p[scale] , False , False);
		}
		donePSP_p=True;
	}
	
	return 0;
}/* end of setupBlobs() */



/***************************************
 *  Compute convolutions and the A matrix.
 ****************************************/

Int MultiTermMatrixCleaner::computeHessianPeak()
{
  LogIO os(LogOrigin("MultiTermMatrixCleaner", "computeHessianPeak", WHERE));
   gip = IPosition(2,nx_p,ny_p);  
   
   if(!doneCONV_p)
   {  
      // Compute the convolutions of the smoothed psfs with each other.
      // Compute Assxx
      // Compute A100, A101, A102
      //         A110, A111, A112
      //         A120, A121, A122   for h(s1)
      // Compute A200, A201, A202
      //         A210, A211, A212
      //         A220, A221, A222   for h(s2)
      //... depending on the number of scales chosen

      // (PSF * scale) * (PSF * scale) -> cubeA_p [nx_p,ny_p,ntaylor,ntaylor,nscales]
      os << "Calculating PSF and Scale convolutions " << LogIO::POST;
      for (Int taylor1=0; taylor1<ntaylor_p;taylor1++) 
      for (Int taylor2=0; taylor2<=taylor1;taylor2++) 
      for (Int scale1=0; scale1<nscales_p;scale1++) 
      for (Int scale2=0; scale2<=scale1;scale2++) 
      {
	Int ttay1 = taylor1+taylor2;
        
        // CALC Hess : Calculate  PSF_(t1+t2)  * scale_1 * scale 2

	///        cWork_p.assign( ((vecPsfFT_p[ttay1]) *(vecPsfFT_p[0]))*(vecScalesFT_p[scale1])*(vecScalesFT_p[scale2]) );
        cWork_p.assign( (vecPsfFT_p[ttay1]) *(vecScalesFT_p[scale1])*(vecScalesFT_p[scale2]) );
       	fftcomplex.fft0( cubeA_p[IND4(taylor1,taylor2,scale1,scale2)]  , cWork_p , False  );
	//        fftcomplex.flip( cubeA_p[IND4(taylor1,taylor2,scale1,scale2)] , False , False);

	//writeMatrixToDisk("psfconv_t_"+String::toString(taylor1)+"-"+String::toString(taylor2)+"_s_"+String::toString(scale1)+"-"+String::toString(scale2)+".im", cubeA_p[IND4(taylor1,taylor2,scale1,scale2)] );
      }	  

      // Construct A, invA for each scale.
      if(itsPositionPeakPsf != IPosition(2,(nx_p/2),(ny_p/2)))
	os << LogIO::WARN << "The PSF peak is not at the center of the image..." << LogIO::POST;

      Int stopnow=False;
      for (Int scale=0; scale<nscales_p;scale++) 
      {
	      // Fill up A
	      for (Int taylor1=0; taylor1<ntaylor_p;taylor1++) 
	      for (Int taylor2=0; taylor2<ntaylor_p;taylor2++) 
	      {
                (matA_p[scale])(taylor1,taylor2) = (cubeA_p[IND4(taylor1,taylor2,scale,scale)])(itsPositionPeakPsf);
		/* Check for exact zeros. Usually indicative of error */
		if( fabs( (matA_p[scale])(taylor1,taylor2) )  == 0.0 ) stopnow = True;
	      }
	      
	      if(stopnow)
	      {
                os << "Multi-Term Hessian has exact zeros. Not proceeding further." << LogIO::WARN << endl;
                os << "The Matrix [A] is : " << (matA_p[scale]) << LogIO::POST;
	        return -2;
	      }

	      /* If all elements are non-zero, check by brute-force if the rows/cols 
		 are nearly linearly dependant, making the matrix nearly singular. */
	       Vector<Float> ratios(ntaylor_p);
	       Float tsum=0.0;
	       for(Int taylor1=0; taylor1<ntaylor_p-1; taylor1++)
	       {
	           for(Int taylor2=0; taylor2<ntaylor_p; taylor2++)
		     ratios[taylor2] = (matA_p[scale])(taylor1,taylor2) / (matA_p[scale])(taylor1+1,taylor2);
                   tsum=0.0;
		   for(Int taylor2=0; taylor2<ntaylor_p-1; taylor2++)
		      tsum += fabs(ratios[taylor2] - ratios[taylor2+1]);
		   tsum /= ntaylor_p-1;
		   if(tsum < 1e-04) stopnow=True;

		   //cout << "Ratios for row " << taylor1 << " are " << ratios << endl;
		   //cout << "tsum : " << tsum << endl;
	       }

	      if(stopnow)
	      {
                os << "Multi-Term Hessian has linearly-dependent rows/cols. Not proceeding further." << LogIO::WARN << endl;
                os << "The Matrix [A] is : " << (matA_p[scale]) << LogIO::POST;
	        return -2;
	      }
	          
	      /* Try to invert the matrix. If it fails, return. 
	         The invertSymPosDef will check if it's positive definite or not. 
	         By construction, it should be pos-def. */
	      // Compute inv(A) 
	      // Use MatrixMathLA::invert
	      // or Use invertSymPosDef...
	      //
	      try
	      {
	             Double deter=0.0;
	             //invert((*invMatA_p[scale]),deter,(*matA_p[scale]));
	             //os << "Matrix Inverse : inv(A) : " << (*invMatA_p[scale]) << LogIO::POST;
	      
                     if(adbg) os << "The Matrix [A] is : " << (matA_p[scale]) << LogIO::POST;
	             invertSymPosDef((invMatA_p[scale]),deter,(matA_p[scale]));
	             if(adbg) os << "Lapack Cholesky Decomposition Inverse of [A] is : " << (invMatA_p[scale]) << LogIO::POST;
	             //if(adbg)os << "A matrix determinant : " << deter << LogIO::POST;
	             //if(fabs(deter) < 1.0e-08) os << "SINGULAR MATRIX !! STOP!! " << LogIO::EXCEPTION;
	      }
	      catch(AipsError &x)
	      {
                      os << "The Matrix [A] is : " << (matA_p[scale]) << LogIO::POST;
		      os << "Cannot Invert matrix : " << x.getMesg() << LogIO::WARN;
		      return -2;
	      }

              /* Add more checks based on the condition number of these matrices */
	      
      }// end for scales
      
      doneCONV_p=True;
   } 
   
   return 0;
}/* end of computeHessianPeak() */


/***************************************
 *  Compute convolutions of the residual images ( all specs ) with scales.
 *  --> the Right-Hand-Side of the matrix equation.
 ****************************************/

Int MultiTermMatrixCleaner::computeRHS()
{
  //  LogIO os(LogOrigin("MultiTermMatrixCleaner", "computeRHS()", WHERE));
	IPosition blc1(2,0,0);
	IPosition trc1(2,nx_p,ny_p);
	IPosition inc1(2, 1);
	
	/* Compute R10 = I_D*B10, R11 = I_D*B11, R12 = I_D*B12
	 * Compute R20 = I_D*B20, R21 = I_D*B21, R22 = I_D*B22
	 * ... depending on the number of scales chosen.
	 */

	/* I_D * (PSF * scale) -> matR_p [nx_p,ny_p,ntaylor,nscales] */
	for (Int taylor=0; taylor<ntaylor_p;taylor++) 
	{
	   /* Compute FT of dirty image */
	  fftcomplex.fft0( dirtyFT_p , vecDirty_p[taylor] , False );
	   
	   for (Int scale=0; scale<nscales_p;scale++) 
	   {

	     // CALC RHS :  Calculate   Dirty_t  * scale_s

                // Let cWork get resized if needed. Force matR to have the right shape
		////                cWork_p.assign( (dirtyFT_p)*(vecPsfFT_p[0])*(vecScalesFT_p[scale]) );
                cWork_p.assign( (dirtyFT_p)*(vecScalesFT_p[scale]) );
                fftcomplex.fft0( matR_p[IND2(taylor,scale)] , cWork_p , False );
                fftcomplex.flip(  matR_p[IND2(taylor,scale)] , False , False );
	   }
	   //	   writeMatrixToDisk("resid_"+String::toString(taylor)+".im", matR_p[IND2(taylor,0)] );
	}
	
	return 0;
}/* end of computeRHS() */

/***************************************
 *  Compute  flux limit for minor cycles
 ****************************************/

Int MultiTermMatrixCleaner::computeFluxLimit(Float &fluxlimit, Float threshold)
{
  // LogIO os(LogOrigin("MultiTermMatrixCleaner", "computeFluxLimit", WHERE));

  // For now, since this calculation is being done outside.....
    fluxlimit = threshold;

    /// fluxlimit = MAX(threshold , itsGain * max....

  // Later, implement the equivalent of an iteration-based cycle-speedup - if required.

	return 0;
}/* end of computeFluxLimit() */



/***************************************
 *  Solve the matrix eqn for each point in the lattice.
Note : This function is called within the 'scale' omp/pragma loop. Needs to be thread-safe
 ****************************************/
Int MultiTermMatrixCleaner::solveMatrixEqn(Int ntaylor, Int scale)
{
         /* Solve for the coefficients, one scale at at time*/
	for(Int taylor1=0;taylor1<ntaylor;taylor1++)
	{
	     (matCoeffs_p[IND2(taylor1,scale)]) = 0.0; 
             for(Int taylor2=0;taylor2<ntaylor;taylor2++)
	     {
		  matCoeffs_p[IND2(taylor1,scale)] = matCoeffs_p[IND2(taylor1,scale)] + ((Float)(invMatA_p[scale])(taylor1,taylor2))*(matR_p[IND2(taylor2,scale)]);
	     }
	}
	return 0;
}/* end of solveMatrixEqn() */
	
/***************************************
 * Find the 'peak' and its location. Fill this into maxScaleVal_p, maxScalePos_p  
 *  Compute the update direction
 *  Options are
 *  (1) Original implementation : something related to the derivative of Chi-Sq. Not sure why it ever worked.
 *  (2) Peak residual in the 00 image
 *  (3) Peak t_0 component (for each scale)
 *  (4) Derivative of chi-square
 *  (5) Similar to (1), but with the PSF replaced by the peak of the PSF only.
Note : This function is called within the 'scale' omp/pragma loop. Needs to be thread-safe
 ****************************************/

Int MultiTermMatrixCleaner::chooseComponent(Int ntaylor, Int scale, Int criterion)
{
    switch(criterion)
     {
     case 1 : /* For each scale, find the maximum chi-square derivative (maybe) */
       {
	 /*
               /// Code block using a private matrix
	       Matrix<Float> ttWork_p((matR_p[IND2(0,0)]).shape());
               ttWork_p = 0.0;
                for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
                {
                      ttWork_p = ttWork_p + (Float)2.0  * (  (matCoeffs_p[IND2(taylor1,scale)])  *  (matR_p[IND2(taylor1,scale)])  );
	              for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
	                     ttWork_p = ttWork_p -  matCoeffs_p[IND2(taylor1,scale)] * matCoeffs_p[IND2(taylor2,scale)] * cubeA_p[IND4(taylor1,taylor2,scale,scale)];
                }
                findMaxAbsMask(ttWork_p,vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
	 */
	       /// Code block using pre-allocated matrices
               vecWork_p[scale] = 0.0;
                for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
                {
                      vecWork_p[scale] = vecWork_p[scale] + (Float)2.0  * (  (matCoeffs_p[IND2(taylor1,scale)])  *  (matR_p[IND2(taylor1,scale)])  );
	              for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
	                     vecWork_p[scale] = vecWork_p[scale] -  matCoeffs_p[IND2(taylor1,scale)] * matCoeffs_p[IND2(taylor2,scale)] * cubeA_p[IND4(taylor1,taylor2,scale,scale)];
                }
                findMaxAbsMask(vecWork_p[scale],vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);

       }
       break;
     case 2 : /* For each scale, find the peak residual */
       {
               Float norm = sqrt((1.0/(matA_p[scale])(0,0)));
               findMaxAbsMask( norm*matR_p[IND2(0,scale)] , vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
       }
       break;
     case 3: /* For each scale, find the peak t_0 coefficnent */
       {
               findMaxAbsMask( matCoeffs_p[IND2(0,scale)] , vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
       }
       break;
     case 4: /* For each scale find the max chisq derivative (diag approx for all hessians) - BAD*/
       {
 	       Matrix<Float> ttWork_p((matR_p[IND2(0,0)]).shape());
                ttWork_p = 0.0;
                for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
                {
		    ttWork_p = ttWork_p + (Float)(2.0  * (matA_p[scale])(taylor1,taylor1) ) *  (matR_p[IND2(taylor1,scale)])  ;
	            for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
			ttWork_p = ttWork_p - (Float)(2.0*( (matA_p[scale])(taylor1,taylor1) * (matA_p[scale])(taylor1,taylor2))) * matCoeffs_p[IND2(taylor2,scale)] ; 
                }
                findMaxAbsMask(ttWork_p,vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
       }
       break;
     case 5 : /* For each scale, same as 1, but use only psf peaks */
       {
	 /*    
               /// Code block using a private matrix
	       Matrix<Float> ttWork_p((matR_p[IND2(0,0)]).shape());
               ttWork_p = 0.0;
                for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
                {
                      ttWork_p = ttWork_p + (Float)2.0  * (  (matCoeffs_p[IND2(taylor1,scale)])  *  (matR_p[IND2(taylor1,scale)])  );
	              for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
			ttWork_p = ttWork_p - (Float)((matA_p[scale])(taylor1,taylor2)) * matCoeffs_p[IND2(taylor1,scale)] * matCoeffs_p[IND2(taylor2,scale)] ;
                }
                findMaxAbsMask(ttWork_p,vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
	 */
		
	 /// Code block using pre-allocated matrices
	        vecWork_p[scale] = 0.0;
                for(Int taylor1=0;taylor1<ntaylor;taylor1++)
                {
		  vecWork_p[scale] = vecWork_p[scale] + (Float)2.0  * (  (matCoeffs_p[IND2(taylor1,scale)])  *  (matR_p[IND2(taylor1,scale)])  );
	          for(Int taylor2=0;taylor2<ntaylor;taylor2++)
			vecWork_p[scale] = vecWork_p[scale] - (Float)((matA_p[scale])(taylor1,taylor2)) * matCoeffs_p[IND2(taylor1,scale)] * matCoeffs_p[IND2(taylor2,scale)] ;
                }
                findMaxAbsMask(vecWork_p[scale],vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
		

       }
       break;
     case 6 : /* chi-square for this scale = sum of abs of residual images for taylor terms */
       {

	 /// Code block using pre-allocated matrices
	        vecWork_p[scale] = 0.0;
                for(Int taylor1=0;taylor1<ntaylor;taylor1++)
                {
		  vecWork_p[scale] = vecWork_p[scale] + (  abs(matR_p[IND2(taylor1,scale)])  );
                }
                findMaxAbsMask(vecWork_p[scale],vecScaleMasks_p[scale],maxScaleVal_p[scale],maxScalePos_p[scale]);
		

       }
       break;
     default:
       os << LogIO::SEVERE << "Internal error : Unknown option for type of update direction" << LogIO::POST;
     }
 

	return 0;
}/* end of chooseComponent() */

/* Update the RHS vector - Called from 'updateModelandRHS'.
Note : This function is called within the 'scale' omp/pragma loop. Needs to be thread-safe for scales.
 */
Int MultiTermMatrixCleaner::updateRHS(Int ntaylor, Int scale, Float loopgain, Vector<Float> coeffs, IPosition blc, IPosition trc, IPosition blcPsf, IPosition trcPsf)
{
    for(Int taylor1=0;taylor1<ntaylor;taylor1++)
    {
      Matrix<Float> residSub = (matR_p[IND2(taylor1,scale)])(blc,trc);  
	   for(Int taylor2=0;taylor2<ntaylor;taylor2++)
	   {
	     Matrix<Float> smoothSub = (cubeA_p[IND4(taylor1,taylor2,scale,maxscaleindex_p)])(blcPsf,trcPsf);
             residSub -= smoothSub * loopgain * coeffs[taylor2];
	     //	     residSub = residSub - smoothSub * loopgain * (matCoeffs_p[IND2(taylor2,maxscaleindex_p)])(globalmaxpos_p);
	   }
    }
}/* end of updateRHS */


/***************************************
 *  Update the model images and the convolved residuals
TODO : The current assumption is that only one scale is chosen at a time,
               However the chi-sq derivative can be computed across scales too
               and the update could be all scales at once for the 'best' location.
 ****************************************/
Int MultiTermMatrixCleaner::updateModelAndRHS(Float loopgain)
{

   /* Max support size for all updates is the full image size */
   // blc, trc :model image -> needs to be centred on the component location
   // blcPsf : psf or scale-blob -> centred on the psf image center (peak).
   gip = IPosition(2,nx_p,ny_p);  
   IPosition support(2,nx_p,ny_p);
   IPosition psfpeak(itsPositionPeakPsf);
   /* The update region. */
   IPosition inc(2,1,1);
   IPosition blc(globalmaxpos_p - support/2);
   IPosition trc(globalmaxpos_p + support/2 - IPosition(2,1,1));
   LCBox::verify(blc, trc, inc, gip);
   /* Shifted region, with the psf at the globalmaxpos_p. */
   IPosition blcPsf(blc + psfpeak - globalmaxpos_p);
   IPosition trcPsf(trc + psfpeak - globalmaxpos_p);
   LCBox::verify(blcPsf, trcPsf, inc, gip);

   /* Reconcile box sizes/locations with the image size */
   makeBoxesSameSize(blc,trc,blcPsf,trcPsf);

   //   cout << "region around peak residual : " << blc << trc << endl;
   //   cout << "around the PSF peak : " << blcPsf << trcPsf << endl;

   /*
   // Code to generate smaller support size for the model image updates.... 
   // No need to update all pixels when a delta-function is being added !!
   Int sz = scaleSizes_p[maxscaleindex_p]*3;
   IPosition xsupport(2,sz,sz);
   IPosition xblc(globalmaxpos_p - xsupport/2);
   IPosition xtrc(globalmaxpos_p + xsupport/2 - IPosition(2,1,1));
   LCBox::verify(xblc, xtrc, inc, gip);
   IPosition xblcPsf(xblc + psfpeak - globalmaxpos_p);
   IPosition xtrcPsf(xtrc + psfpeak - globalmaxpos_p);
   LCBox::verify(xblcPsf, xtrcPsf, inc, gip);

   makeBoxesSameSize(xblc,xtrc,xblcPsf,xtrcPsf);
   */


   /* Update the model images */
   Matrix<Float> scaleSub = (vecScales_p[maxscaleindex_p])(blcPsf,trcPsf); 
   for(Int taylor=0;taylor<ntaylor_p;taylor++)
   {
     Matrix<Float> modelSub = (vecModel_p[taylor])(blc,trc); 
     modelSub += scaleSub * loopgain * (matCoeffs_p[IND2(taylor,maxscaleindex_p)])(globalmaxpos_p);
   }

   /* Save the taylor0 images for each scale size */
   //   {
   //      Matrix<Float> modelSub = (vecScaleModel_p[maxscaleindex_p])(blc,trc); 
   //      modelSub += scaleSub * loopgain * (matCoeffs_p[IND2(0,maxscaleindex_p)])(globalmaxpos_p);
   //    }
   
   /* Update the convolved residuals */
   Vector<Float> coeffs(ntaylor_p);
   for(Int taylor=0;taylor<ntaylor_p;taylor++) 
          coeffs[taylor] = (matCoeffs_p[IND2(taylor,maxscaleindex_p)])(globalmaxpos_p);
   
   Int scale;
   Int ntaylor=ntaylor_p;
   #pragma omp parallel default(shared) private(scale) firstprivate(ntaylor,loopgain,coeffs,blc,trc,blcPsf,trcPsf)
  { 
    #pragma omp for 
    for(scale=0;scale<nscales_p;scale++)
   {
     updateRHS(ntaylor,scale, loopgain, coeffs, blc, trc, blcPsf, trcPsf);
   }
  }//End pragma parallel  

   /* Update flux counters */
   for(Int taylor=0;taylor<ntaylor_p;taylor++)
   {
	   totalTaylorFlux_p[taylor] += loopgain*(matCoeffs_p[IND2(taylor,maxscaleindex_p)])(globalmaxpos_p);
   }
   totalScaleFlux_p[maxscaleindex_p] += loopgain*(matCoeffs_p[IND2(0,maxscaleindex_p)])(globalmaxpos_p);
   
   return 0;
}/* end of updateModelAndRHS() */

/* ................ */
Int MultiTermMatrixCleaner::checkConvergence(Int criterion, Float &fluxlimit, Float &loopgain)
{
    Float rmaxval=0.0;
    
    /* Use the maximum residual (current), to compare against the convergence threshold */
    Float maxres=0.0;
    IPosition maxrespos;
    findMaxAbsMask((matR_p[IND2(0,0)]),vecScaleMasks_p[0],maxres,maxrespos);
    Float norma = (1.0/(matA_p[0])(0,0));
    //rmaxval = MAX(rmaxval, maxres*norma/5.0);
    rmaxval = abs(maxres*norma);
    
    /* Check for convergence */
    Int convergedflag = 0;
    if( fabs(rmaxval) < MAX(userthreshold_p,fluxlimit) ) 
    {
      os << "Reached stopping threshold at iteration " << totalIters_p << ". Peak residual " << fabs(rmaxval) << LogIO::POST;
       convergedflag=1; 
    }

    /* Levenberg-Macquart-like change in step size */
    if(itercount_p>1 && inputgain_p<=(Float)0.0)
   { 
     
	if (globalmaxval_p < prev_max_p) 
            loopgain = loopgain * 1.5;
        else 
            loopgain = loopgain / 1.5;

        loopgain = MIN((1-stopfraction_p),loopgain);
        loopgain = MIN((Float)0.6,loopgain);
        
        /* detect divergence by approximately 10 consecutive increases in maxval */
        if(loopgain < (Float)0.01)
   	{
	  os << "Not converging any more. May be diverging. Stopping minor cycles at iteration " << totalIters_p << ". Peak residual " << fabs(rmaxval) << LogIO::POST;
          convergedflag=-1; 
	 }
     
    /* Stop if there is divergence : 200% increase from the current minimum value */
	if( fabs(  (min_max_p-globalmaxval_p)/min_max_p ) > (Float)2.0 )
      {
	os << "Diverging.... Stopping minor cycles at iteration " << totalIters_p << ". Peak residual " << fabs(rmaxval) << " Max: " << globalmaxval_p << LogIO::POST;
        convergedflag=-1;
      }

    // Stop if the maxval has changed by less than 5% in 5 iterations 
    // --- this is similar to saying less than 1% change per iteration.... same as a loopgain < 0.01 
	
	//	if( abs(prev5_max - globalmaxval_p) < 0.01*abs(prev5_max) )
	//  { 
        //    os << "Not converging any more. Flattening out. Stopping minor cycles at iteration " << totalIters_p << ". Peak residual " << fabs(rmaxval) << LogIO::POST;
        //    convergedflag=-1;
	//  }


	
    }// end of if(itercount_p>1)

    /* Store current max value - to use in the next iteration */    
    prev_max_p = globalmaxval_p;
    //    if(itercount_p%5 == 0) 
    //     prev5_max=globalmaxval_p;
    min_max_p = MIN(min_max_p,abs(globalmaxval_p));

    /* Stop, if there are negatives on the largest scale in the Io image */
    //if(nscales_p>1 && maxscaleindex_p == nscales_p-2)
    //	if((*matCoeffs_p[IND2(0,maxscaleindex_p)]).getAt(globalmaxpos_p) < 0.0)
    //	{converged = False;break;}

    /* Detect divergence, and signal it.... */
    // TODO

    /* Print out coefficients for a few iterations */
    if(fluxlimit==-1) 
    {
         fluxlimit = rmaxval * stopfraction_p;
	 //        os << "Peak convolved residual : " << rmaxval << "    Minor cycle stopping threshold : " << itsThreshold.getValue("Jy")  << LogIO::POST;
        os << "Peak convolved residual : " << rmaxval << "    Minor cycle stopping threshold : " << fluxlimit  << LogIO::POST;
    }
    else
    {
      if( totalIters_p==maxniter_p || (adbg==(Bool)True) || maxniter_p < (int)5 || (totalIters_p%(Int)20==0) )
       {
	 
	    os << "[" << totalIters_p << "] Res: " << rmaxval << " Max: " << globalmaxval_p;
            os << " Gain: " << loopgain;
	     ////        os << "[" << totalIters_p << "] Res: " << rmaxval;
	    //os << "[" << totalIters_p << "] Max: " << globalmaxval_p;
            os << " Pos: " <<  globalmaxpos_p << " Scale: " << scaleSizes_p[maxscaleindex_p];
            os << " Coeffs: ";
            for(Int taylor=0;taylor<ntaylor_p;taylor++)
               os << (matCoeffs_p[IND2(taylor,maxscaleindex_p)])(globalmaxpos_p) << "  ";
            if(adbg)
	      {
              os << " OrigRes: ";
              for(Int taylor=0;taylor<ntaylor_p;taylor++)
                   os << (matR_p[IND2(taylor,maxscaleindex_p)])(globalmaxpos_p) << "  ";
	      }
            os << LogIO::POST;
        }
    }

    return convergedflag;

}/* end of checkConvergence */



/* Save a matrix to disk */
Int MultiTermMatrixCleaner::writeMatrixToDisk(String imagename, Matrix<Float>& themat)
{
  TabularCoordinate tab1(0, 1.0, 0, String("deg"), String("axis1"));
  TabularCoordinate tab2(0, 1.0, 0, String("deg"), String("axis2"));
      CoordinateSystem csys;
      csys.addCoordinate(tab1);
      csys.addCoordinate(tab2);
      PagedImage<Float> limage(themat.shape(), csys, imagename);
      limage.put(themat);
}



} //# NAMESPACE CASA - END

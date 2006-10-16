//# WBCleanImageSkyModel.cc: Implementation of WBCleanImageSkyModel class
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
#include <synthesis/MeasurementComponents/WBCleanImageSkyModel.h>
#include <synthesis/MeasurementEquations/WBSkyEquation.h>
#include <casa/OS/File.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <synthesis/MeasurementEquations/LatticeModel.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <lattices/Lattices/LatticeCleaner.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/TempLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/TiledLineStepper.h> 
#include <lattices/Lattices/LatticeStepper.h> 
#include <lattices/Lattices/LatticeNavigator.h> 
#include <lattices/Lattices/LatticeIterator.h>

#include <scimath/Mathematics/MatrixMathLA.h>

#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <ms/MeasurementSets/MSColumns.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include <casa/OS/HostInfo.h>

namespace casa { //# NAMESPACE CASA - BEGIN
#define TMR(a) "[User: " << a.user() << "] [System: " << a.system() << "] [Real: " << a.real() << "]"

#define MEMFACTOR (4.0)
	
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
#define MAX(a,b) ((a)>=(b) ? (a) : (b))
//#define IND2(ntay,nsca) ((ntay)*(nscales_p)+(nsca))
//#define IND4(nt1,nt2,ns1,ns2) (((((nt1)*((nt1)+1)/2)+(nt2))*nscales_p*(nscales_p+1)/2)+(((ns1)*((ns1)+1)/2)+(ns2)))

// IND4 for square storage
//#define IND4(nt1,nt2,ns1,ns2) ((ns1)*(nscales_p*ntaylor_p*ntaylor_p)+(nt1)*(nscales_p*ntaylor_p)+(nt2)*(nscales_p)+(ns2))

/*************************************
 *          Constructor
 *************************************/
WBCleanImageSkyModel::WBCleanImageSkyModel()
{
  initVars();
  if(adbg) cout << "CONSTRUCTOR - no args!!" << endl;
  nscales_p=4;
  ntaylor_p=2;
  totalTaylorFlux_p.resize(2,0.0);
  scaleSizes_p.resize(0);
};
WBCleanImageSkyModel::WBCleanImageSkyModel(const Int ntaylor,const Int nscales)
{
  initVars();
  if(adbg) cout << "CONSTRUCTOR - with nscales !!" << endl;
  nscales_p=nscales;
  ntaylor_p=ntaylor;
  totalTaylorFlux_p.resize(ntaylor_p,0.0);
  scaleSizes_p.resize(0);
};
WBCleanImageSkyModel::WBCleanImageSkyModel(const Int ntaylor,const Vector<Float>& userScaleSizes)
{
  initVars();
  if(adbg) cout << "CONSTRUCTOR with userscalevector !!" << endl;
  nscales_p=userScaleSizes.nelements();
  scaleSizes_p.resize(nscales_p);
  for(Int i=0;i<nscales_p;i++) scaleSizes_p[i] = userScaleSizes[i];
  ntaylor_p=ntaylor;
  totalTaylorFlux_p.resize(ntaylor_p,0.0);
};

void WBCleanImageSkyModel::initVars()
{
  adbg=0; 
  ddbg=0; // output per iteration
  tdbg=0;
  
  modified_p=True;
  memoryMB_p=256;
  donePSF_p=False;
  donePSP_p=False;
  doneCONV_p=False;
  matA_p.resize(0);
  invMatA_p.resize(0);
  vecScales_p.resize(0);
  cubeA_p.resize(0);
  matR_p.resize(0);
  matCoeffs_p.resize(0);
  scaleBias_p.resize(0);
  totalScaleFlux_p.resize(0);
  weightScaleFactor_p=1.0;
  maxPsf_p=1.0;
  useRefFrequency_p=1.42e+09;

  lambda_p = 0.001;
  
  os = LogIO(LogOrigin("WBCleanImageSkyModel","solve"));
  
}

/*************************************
 *          Destructor
 *************************************/
WBCleanImageSkyModel::~WBCleanImageSkyModel()
{
  if(adbg) cout << "DESTRUCTOR !!" << endl;
  manageMemory(False);

  // Do this for ALL... */
  for(Int i=0;i<(Int)imagespec_p.nelements();i++) 
  {
	  delete imagespec_p[i];
	  delete cimagespec_p[i];
	  delete gSspec_p[i];
	  delete psfspec_p[i];
  }

};

/*************************************
 *          Solver
 *************************************/
// Clean solver
Bool WBCleanImageSkyModel::solve(SkyEquation& se) 
{
	if(ddbg)os << "SOLVER for Multi-Frequency Synthesis deconvolution" << LogIO::POST;
	Bool converged = False;
	
	/* Initialize timers */
	/* 0 : computeRHS
	   1 : computeFluxLimit
	   2 : solveMatrixEqn
	   3 : computePenaltyFunction
	   4 : find max
	   5 : updateSolution
	   6 : solveResiduals
	   7 : minor cycles total
	   8 : Allocate Memory
	   9 : Set up masks
	   10: Make PSFs
	   11: Make Blobs
	   12: 4-way convolutions
	   13: Initial residuals
	   14: Write results to disk
	*/
	Vector<Float> tims(15);
	tims.set(0.0);

	/* Gather shape information */
	if(numberOfModels()>1) os << "Cannot process more than one field" << LogIO::EXCEPTION;
			
	nx = image(0).shape()(0);
	ny = image(0).shape()(1);
	npol_p=image(0).shape()(2);
	nchan=image(0).shape()(3);

	if(nchan > 1) os << "Cannot process more than one output channel" << LogIO::EXCEPTION;
	if(npol_p > 1) os << "Cannot process more than one output polarization" << LogIO::EXCEPTION;
	
	//AlwaysAssert((npol==1)||(npol==2)||(npol==3)||(npol==4), AipsError);  
	AlwaysAssert((npol_p==1), AipsError);  
	
	/* Allocate memory for all the TempLattices. */
	if(tdbg) tmr1.mark();
	manageMemory(True);
	if(tdbg) tims[8] += tmr1.real();
	
	/* Set up the Mask image */
	if(tdbg) tmr1.mark();
	setupMasks();
	if(tdbg) tims[9] += tmr1.real();
	
	/* Create the Point Spread Functions */
	if(tdbg) tmr1.mark();
	if(!donePSF_p)
	{
		makeSpectralPSFs(se);
		donePSF_p=True;
	}
	if(tdbg) tims[10] += tmr1.real();
	
	/* Create the scaled blobs and their FTs */
	if(tdbg) tmr1.mark();
	setupBlobs();
	if(tdbg) tims[11] += tmr1.real();
	
	/* Compute all convolutions and the matrix A */
	if(tdbg) tmr1.mark();
	computeMatrixA();
	if(tdbg) tims[12] += tmr1.real();
	
	// Calculate scale factor - to compensate for scaling the weights to
	// make the ramp have max and min as +1 and -1.
	calcWeightScaleFactor(se);
	
	/* Read in the model images */
	readInModels();
	
	// Calculate the initial residual image.
	if(tdbg) tmr1.mark();
	Float initmaxres=0.0;
	IPosition maxposition;
	solveResiduals(se, initmaxres, maxposition);
	if(tdbg) tims[13] += tmr1.real();
	os << "Initial Max Residual : " << initmaxres << "  at " << maxposition << LogIO::POST;

	/******************* START MAJOR CYCLE LOOP *****************/
	os << "Starting the solver major cycles" << LogIO::POST;
	Int totalIters=0;
	Bool choosespec = True;
	for(Int itercountmaj=0;itercountmaj<500;itercountmaj++)
	{
		
		/* Compute the convolutions of the current residual with all
		   PSFs and scales */

		if(tdbg) tmr2.mark();
		computeRHS();
		if(tdbg) tims[0] += tmr2.real();
		
		/* Compute the flux limits that determine the depth of the minor cycles. */
		
		Float fluxlimit =0.0;
		Float loopgain = gain();
		Float thresh = threshold();
		
		if(tdbg) tmr2.mark();
		computeFluxLimit(fluxlimit,thresh);
		if(tdbg) tims[1] += tmr2.real();
		
		/* Initialize persistent variables */
		
		gip = IPosition(4,nx,ny,1,1);  
		Float maxval,globalmaxval=-1e+10;
		IPosition maxpos(4,0),globalmaxpos(4,0);
		Int maxscaleindex=0;
		
		
		/********************** START MINOR CYCLE ITERATIONS ***********************/
		if(tdbg) tmr1.mark();
		
		for(Int itercount=0;itercount<MIN(20,numberIterations()-totalIters);itercount++)
		{
			globalmaxval=-1e+10;
			
			/* Find the best component over all scales */
			for(Int scale=0;scale<nscales_p;scale++)
			{
				/* Solve the matrix eqn for all points in the lattice */
				if(tdbg) tmr2.mark();
				solveMatrixEqn(scale);
				if(tdbg) tims[2] += tmr2.real();
				
				/* Now use the solved-for sets of coefficients and compute a penalty function */
				if(tdbg) tmr2.mark();
				computePenaltyFunction(scale, totalIters,loopgain,choosespec);
				if(tdbg) tims[3] += tmr2.real();
				
				/* Find the peak of the penalty function to choose the update direction */
				// Find the location and TAYLOR term of Max Eps sq  from twork.
				if(tdbg) tmr2.mark();
				findMaxAbsLattice((*mask_p),*tWork_p,maxval,maxpos);
				if(tdbg) tims[4] += tmr2.real();
				
				/* Record the maximum penalty-function value and chosen scale */
				//if(maxval > globalmaxval)
				if((maxval*scaleBias_p[scale]) > globalmaxval)
				{
					globalmaxval = maxval;
					globalmaxpos = maxpos;
					maxscaleindex = scale;
				}
				
			}// end of for scale
			
			/* Print out coefficients at each iteration */
			if(ddbg)
			//if(1)	
			{
				os << "[" << totalIters+1 << "] MaxVal: " << globalmaxval << "\tPos: " <<  globalmaxpos << "\tScale: " << scaleSizes_p[maxscaleindex];// << LogIO::POST;
				
				os << "\tCoeffs: ";
				for(Int taylor=0;taylor<ntaylor_p;taylor++)
					os <<  (*matCoeffs_p[IND2(taylor,maxscaleindex)]).getAt(globalmaxpos) << "\t";
				os << LogIO::POST;
			}
			
			/* Update the current solution by this chosen step */
			if(tdbg) tmr2.mark();
			updateSolution(globalmaxpos,maxscaleindex,loopgain);
			if(tdbg) tims[5] += tmr2.real();
			
			
			/* Calculate convergence thresholds..... */
			Float rmaxval=0.0;

			/* Use the strongest I0 component, to compare against the convergence threshold */
			Float compval = fabs((*matCoeffs_p[IND2(0,maxscaleindex)]).getAt(globalmaxpos));
			rmaxval = MAX( rmaxval , compval );

			// somehow - get the off-source rms estimate - and use that.
			// after it converges - then hunt for peaks only from max res in Io!

#if 0	
			/* Use the maximum residual (current), to compare against the convergence threshold */
			Float maxres=0.0;
			IPosition maxrespos;
			findMaxAbsLattice((*mask_p),(*matR_p[IND2(0,0)]),maxres,maxrespos);
			//Float norma = sqrt((1.0/(*matA_p[0])(0,0)));
			Float norma = ((1.0/(*matA_p[0])(0,0)));

			rmaxval = MAX(rmaxval, maxres*norma/5.0);
			os << "max val : " << rmaxval << LogIO::POST;
#endif
			
			
			modified_p=True;
			totalIters++;
			
			
			/* Check for convergence */
		        /* Switch between penalty functions, after a I0 component lower than the threshold
			   is picked. Until then, pick components that minimize chi-sq. After switching, 
			   pick components that correspond to the peak I0 residual */

			/*
			if(fabs(rmaxval) < thresh*1.5 && !choosespec){converged=True; break;}
			if(fabs(rmaxval) < thresh && choosespec)
			{converged=False; choosespec=False; if(ddbg)os << "Switching stopping criterion" << LogIO::POST; break;}
			else if(fabs(rmaxval) < fluxlimit){converged=False;break;}
			*/
			
			if(fabs(rmaxval) < thresh){converged=True; break;}
			else if(fabs(rmaxval) < fluxlimit){converged=False;break;}

			/* Stop, if there are negatives on the largest scale in the Io image */
			//if(nscales_p>1 && maxscaleindex == nscales_p-2)
			//	if((*matCoeffs_p[IND2(0,maxscaleindex)]).getAt(globalmaxpos) < 0.0)
			//	{converged = False;break;}
			
		}
		if(tdbg) tims[7] += tmr1.real();
		/********************** END MINOR CYCLE ITERATIONS ***********************/
		
		/* Print out flux counts so far */
		if(ddbg)
		{
			for(Int scale=0;scale<nscales_p;scale++) os << "Scale " << scale+1 << " with " << scaleSizes_p[scale] << " pixels has total flux = " << totalScaleFlux_p[scale] << LogIO::POST;
			for(Int taylor=0;taylor<ntaylor_p;taylor++) os << "Taylor " << taylor << " has total flux = " << totalTaylorFlux_p[taylor] << LogIO::POST;
		}
		
		/* Do the prediction and residual computation. */
		if(tdbg) tmr2.mark();
		Float maxres=0.0;
		solveResiduals(se,maxres, maxposition);
		if(tdbg) tims[6] += tmr2.real();
		os << "Max Residual after " << totalIters << " iterations : " << maxres << "  at " << maxposition << LogIO::POST;
		
		if(totalIters >= numberIterations() || converged==True) break;
		
	} 
	/******************* END MAJOR CYCLE LOOP *****************/
	
	/* Write results to disk */
	if(tdbg) tmr1.mark();
	writeResultsToDisk();
	if(tdbg) tims[14] += tmr1.real();
	
	/* Print out timing info */
	if(tdbg)
	{
		os << " ************************ Time to Allocate Memory : " << tims[8] << LogIO::POST;
		os << " ************************ Time to Set up Masks : " << tims[9] << LogIO::POST;
		os << " ************************ Time to Make PSFs : " << tims[10] << LogIO::POST;
		os << " ************************ Time to Make Blobs : " << tims[11] << LogIO::POST;
		os << " ************************ Time to do 4-way convolutions : " << tims[12] << LogIO::POST;
		os << " ************************ Time to make Initial Residual : " << tims[13] << LogIO::POST;
		os << " ************************ Start major cycle " << LogIO::POST;
		os << " ********************************** Time to compute RHS : " << tims[0] << LogIO::POST;
		os << " ********************************** Time to compute Flux Limit : " << tims[1] << LogIO::POST;
		os << " ********************************** Start minor cycle " << LogIO::POST;
		os << " ******************************************** Time to solve MatrixEqn : " << tims[2] << LogIO::POST;
		os << " ******************************************** Time to compute Penalty Function : " << tims[3] << LogIO::POST;
		os << " ******************************************** Time to find Max : " << tims[4] << LogIO::POST;
		os << " ******************************************** Time to update Solution : " << tims[5] << LogIO::POST;
		os << " ********************************** End minor cycle " << LogIO::POST;
		os << " ********************************** Total Time for Minor Cycles : " << tims[7] << LogIO::POST;
		os << " ********************************** Time to solve Residuals : " << tims[6] << LogIO::POST;
		os << " ************************ End major cycles " << LogIO::POST;
		os << " ************************ Time to Write to Disk : " << tims[14] << LogIO::POST;
	}
	
	return(converged);
} // END OF SOLVE



/***********************************************************************/
Int WBCleanImageSkyModel::calcWeightScaleFactor(SkyEquation& se)
{
	// Compute the weight Scale factor - to correct for the "factor" in the ftmachine.

	VisSet& wvs = se.visSet();
	VisIter& vi(wvs.iter());
	VisBuffer vb(vi);
	vi.origin();
	
	Vector<Double> selfreqlist(vb.frequency());
	Double refFreq = (selfreqlist(selfreqlist.endPosition()) + selfreqlist(IPosition(selfreqlist.ndim(),0)))/2.0;
	weightScaleFactor_p = refFreq/(selfreqlist(selfreqlist.endPosition())-refFreq);

	return 0;
}
	
/***********************************************************************/
Int WBCleanImageSkyModel::writeResultsToDisk()
{
	Quantity bmaj=0.0,bmin=0.0,bpa=0.0;
	StokesImageUtil::FitGaussianPSF(PSF(0), bmaj, bmin, bpa);

	// loop over model too 
	for(Int taylor=1;taylor<ntaylor_p;taylor++)
	{
		
		/* Write model images to disk - with correction */
		
		imagespec(0,taylor).copyData(LatticeExpr<Float> (imagespec(0,taylor)*(pow(weightScaleFactor_p,1))));
		
		/* Create new restored images for the higher order spectral terms */
		
		String restoredtaylor("");
		restoredtaylor = imageNames[0] + String(".spec_") + String::toString(taylor);
		
		{
			PagedImage<Float> resttay(image(0).shape(),image(0).coordinates(),restoredtaylor); 
			resttay.set(0.0);
			
			/* Restore with the clean beam, and add in the residuals */
			resttay.copyData(imagespec(0,taylor));
			StokesImageUtil::Convolve(resttay, bmaj, bmin, bpa);
			
			LatticeExpr<Float> le(resttay+(residual(0))); 
			resttay.copyData(le);
			
			ImageInfo ii = resttay.imageInfo();
			ii.setRestoringBeam(bmaj, bmin, bpa); 
			resttay.setImageInfo(ii);
			resttay.setUnits(Unit("Jy/beam"));
			resttay.table().unmarkForDelete();
		}
	}
	return 0;
}



/***********************************************************************/
Bool WBCleanImageSkyModel::solveResiduals(SkyEquation& se, Float &maxres, IPosition &maxip) 
{
	if(adbg)os << "Into solveResiduals(se)..." << LogIO::POST;
        makeNewtonRaphsonStep(se,False);
	
	{
	IPosition blc2(4,0,0,0,0);
  	IPosition trc2(4,nx,ny,0,0);
  	IPosition inc2(4, 1);
  	LCBox::verify(blc2,trc2,inc2,residual(0).shape());
  	LCBox lor(blc2,trc2,residual(0).shape());

  	Float zmaxval=0.0;
  	IPosition zmaxpos;
  	findMaxAbsLattice((*mask_p),(SubLattice<Float>(residual(0),lor,True)),zmaxval,zmaxpos);
	//os << "Max Residual : " << zmaxval << "  at " << zmaxpos << LogIO::POST;

	maxres = zmaxval;
	maxip = zmaxpos;
	}
	
	
	return True;
}
/***********************************************************************/

/*************************************
 *          INDICES !!!!!!! 
 *************************************/
//#define IND2(ntay,nsca) ((ntay)*(nscales_p)+(nsca))
//#define IND4(nt1,nt2,ns1,ns2) (((((nt1)*((nt1)+1)/2)+(nt2))*nscales_p*(nscales_p+1)/2)+(((ns1)*((ns1)+1)/2)+(ns2)))
Int WBCleanImageSkyModel::IND2(Int taylor, Int scale)
{
	return  taylor * nscales_p + scale;
}
Int WBCleanImageSkyModel::IND4(Int taylor1, Int taylor2, Int scale1, Int scale2)
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


/*************************************
 *          Number of TempLattices 
 *************************************/
Int WBCleanImageSkyModel::numberOfTempLattices(Int nscales, Int ntaylor)
{
  Int ntotal4d = (nscales*(nscales+1)/2) * (ntaylor*(ntaylor+1)/2);
  return ntotal4d + 6 + 2 + (2+1)*nscales + (1+1)*ntaylor + 2*nscales*ntaylor;
}

/*************************************
 *          Allocate Memory
 *************************************/
Int WBCleanImageSkyModel::manageMemory(Bool direction)
{
	if(direction)
	{
		// Define max memory usage for all TempLattices. (half of available);
		memoryMB_p = Double(HostInfo::memoryTotal()/1024)/(2.0); // ? /(16.0) ?
		Int ntemp = numberOfTempLattices(nscales_p,ntaylor_p);
		Int numMB = nx*ny*4*ntemp/(1024*1024);
		os << "This algorithm needs " << numMB << " MBytes for " << ntemp << " TempLattices " << LogIO::POST;
		memoryMB_p = MIN(memoryMB_p, numMB);
		os << "Allocating " << memoryMB_p << " MBytes." << LogIO::POST;
		
	}
	if(adbg && direction)os << "Allocating mem ... " ;
	if(adbg && !direction)os << "Freeing mem ... " ;
	
	Int ntotal4d = (nscales_p*(nscales_p+1)/2) * (ntaylor_p*(ntaylor_p+1)/2);
	
	//gip = IPosition(2,ntaylor_p,ntaylor_p);  
	IPosition tgip(2,ntaylor_p,ntaylor_p);
	
	// Small A matrix to be inverted for each point..
	matA_p.resize(nscales_p); invMatA_p.resize(nscales_p);
	for(Int i=0;i<nscales_p;i++)
	{
		if(direction)
		{ 
			matA_p[i] = new Matrix<Double>(tgip);
			invMatA_p[i] = new Matrix<Double>(tgip);
		}
		else
		{
			delete matA_p[i] ;
			delete invMatA_p[i] ;
		}
	}
	
	/// Make this read from model.shape() or image.shape()
	gip = IPosition(4,nx,ny,1,1);  
	
	// I_D and mask
	if(direction)
	{
		dirty_p = new TempLattice<Float>(gip, memoryMB_p);
		dirtyFT_p = new TempLattice<Complex>(gip, memoryMB_p);
		mask_p = new TempLattice<Float>(gip, memoryMB_p);
		fftmask_p = new TempLattice<Float>(gip, memoryMB_p);
		// Temporary work-holder
		cWork_p = new TempLattice<Complex>(gip,memoryMB_p);
		tWork_p = new TempLattice<Float>(gip,memoryMB_p);
	
	}
	else
	{
		delete dirty_p;
		delete dirtyFT_p;
		delete fftmask_p;
		delete mask_p;
		delete cWork_p;
		delete tWork_p;
	}
	
	// Scales
	vecScales_p.resize(nscales_p);
	vecScalesFT_p.resize(nscales_p);
	for(Int i=0;i<nscales_p;i++) 
	{
		if(direction)
		{
			vecScales_p[i] = new TempLattice<Float>(gip,memoryMB_p);
			vecScalesFT_p[i] = new TempLattice<Complex>(gip,memoryMB_p);
		}
		else
		{
			delete vecScales_p[i];
			delete vecScalesFT_p[i];
		}
	}
	
	// Psfs and Models
	// Try to remove the PSF and the Models and use 
	// PSF(model), PSF1(model) and image(model),image1(model)...
	vecPsfFT_p.resize(ntaylor_p);
	for(Int i=0;i<ntaylor_p;i++) 
	{
		if(direction)
		{
			vecPsfFT_p[i] = new TempLattice<Complex>(gip,memoryMB_p);
			
		}
		else
		{
			delete vecPsfFT_p[i];
		}
	}
	
	// Psf * Scales
	//  matPsfConvScales_p.resize(ntaylor_p*nscales_p);
	//  for(Int i=0;i<nscales_p*ntaylor_p;i++) matPsfConvScales_p = new TempLattice<Float>(gip,memoryMB_p);
	
	// Set up the latticeiterators also
	IPosition shapeOut;
	IPosition cursorShape;
	
	if(direction)
	{
		AlwaysAssert (tWork_p->isWritable(), AipsError);
		shapeOut = IPosition(tWork_p->shape());
		cursorShape = IPosition(tWork_p->niceCursorShape());
	}
	else
	{
		shapeOut = gip;
		cursorShape = gip;
	}
	
	LatticeStepper stepper(shapeOut, cursorShape, LatticeStepper::RESIZE);
	
	if(direction)itertWork_p = new LatticeIterator<Float>((*tWork_p), stepper);
	else delete itertWork_p;
	
	// (Psf * Scales) * (Psf * Scales)
	cubeA_p.resize(ntotal4d);
	itercubeA_p.resize(ntotal4d);
	for(Int i=0;i<ntotal4d;i++) 
	{
		if(direction) 
		{
			cubeA_p[i] = new TempLattice<Float>(gip,memoryMB_p);
			itercubeA_p[i] = new LatticeIterator<Float>((*cubeA_p[i]),stepper);
		}
		else 
		{
			delete cubeA_p[i];
			delete itercubeA_p[i];
		}
	}
	
	// I_D * (Psf * Scales)
	matR_p.resize(ntaylor_p*nscales_p);
	itermatR_p.resize(ntaylor_p*nscales_p);
	// Coefficients to be solved for.
	matCoeffs_p.resize(ntaylor_p*nscales_p);
	itermatCoeffs_p.resize(ntaylor_p*nscales_p);
	
	for(Int i=0;i<ntaylor_p*nscales_p;i++) 
	{
		if(direction)
		{	
			matR_p[i] = new TempLattice<Float>(gip,memoryMB_p);
			itermatR_p[i] = new LatticeIterator<Float>((*matR_p[i]),stepper);
			matCoeffs_p[i] = new TempLattice<Float>(gip,memoryMB_p);
			itermatCoeffs_p[i] = new LatticeIterator<Float>((*matCoeffs_p[i]),stepper);
		}
		else
		{
			delete matR_p[i];
			delete itermatR_p[i];
			delete matCoeffs_p[i];
			delete itermatCoeffs_p[i];
		}
	}
  
	if(adbg) os << "done" << LogIO::POST;
	
	return 0;
}

/*************************************
 *          Store a Templattice as an image
 *************************************/
Int WBCleanImageSkyModel::storeAsImg(String fileName, ImageInterface<Float>& theImg)
{
  PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), fileName);
  LatticeExpr<Float> le(theImg);
  tmp.copyData(le);
  return 0;
}

Int WBCleanImageSkyModel::storeTLAsImg(String fileName, TempLattice<Float> &TL, ImageInterface<Float>& theImg)
{
  PagedImage<Float> tmp(TL.shape(), theImg.coordinates(), fileName);
  LatticeExpr<Float> le(TL);
  tmp.copyData(le);
  return 0;
}

/*************************************
 *          Make Approx PSFs. 
 *************************************/

Int WBCleanImageSkyModel::makeSpectralPSFs(SkyEquation& se) 
{
  LogIO os(LogOrigin("WBCleanImageSkyModel", "makeSpectralPSFs"));

  ///// DO SOMETHING ABOUT MODELS - FOR ALL PRIVATE DATA VARS.
  for (Int thismodel=0;thismodel<nmodels_p;thismodel++) 
  {
      Float maxpsf=1.0;
      IPosition maxpsfpos;

      se.makeApproxPSF(thismodel, PSF(thismodel));  
      if(adbg) os << "Made both PSF0 and PSF1.....(un-normalized)" << LogIO::POST;
      
      findMaxAbsLattice((*mask_p),(PSF(0)),maxpsf,maxpsfpos);
      
      if(adbg)os << "Normalizing the PSFs..." << LogIO::POST;
      /* normalize PSF0 by the peak */
      (PSF(0)).copyData(LatticeExpr<Float> (PSF(0)/maxpsf) );
      /* normalize other PSFs by the peak from PSF(0) */
      for(Int taylor=1;taylor<ntaylor_p;taylor++)
	      (PSFspec(0,taylor)).copyData(LatticeExpr<Float> ((PSFspec(0,taylor))/maxpsf) );
      
      /* Normalize PSF1 by the peak from PSF0 */
      //if(ntaylor_p>1)(PSF1(0)).copyData(LatticeExpr<Float> ((PSF1(0))/maxpsf) );
      
      maxPsf_p = maxpsf;
      if(adbg) os << "Max PSF0 : " << maxpsf << LogIO::POST;
      
      beam(thismodel)=0.0;
      if(!StokesImageUtil::FitGaussianPSF(PSF(thismodel),beam(thismodel))) 
	os << "Beam fit failed: using default" << LogIO::POST;
      
      os << "Made spectral PSFs." << LogIO::POST;
      
      /* Compute Fourier Transforms of PSFs */
      for(Int taylor=0;taylor<ntaylor_p;taylor++)
      {
	      if(taylor==0) vecPsfFT_p[taylor]->copyData(LatticeExpr<Complex>(toComplex((*fftmask_p)*(PSF(0)))));
	      else vecPsfFT_p[taylor]->copyData(LatticeExpr<Complex>(toComplex((*fftmask_p)*(PSFspec(0,taylor)))));
	      //if(taylor==1) vecPsfFT_p[taylor]->copyData(LatticeExpr<Complex>(toComplex((*fftmask_p)*(PSF1(0)))));
	      LatticeFFT::cfft2d(*vecPsfFT_p[taylor], True);
      }
  
      // Write PSFs to disk..
      if(adbg)
      {
	      for(Int taylor=0;taylor<ntaylor_p;taylor++)
	      {
		      String plab("");
		      plab = String::toString(taylor);
		      if(taylor==0)
		      {
			      storeAsImg(String("PSF")+plab+String(".im"),PSF(0));
			      cout << "psf" << plab << " dims : " << (PSF(0)).shape() << endl;
		      }
		      else
		      {
			      storeAsImg(String("PSF")+plab+String(".im"),PSFspec(0,taylor));
			      cout << "psf" << plab << " dims : " << (PSFspec(0,taylor)).shape() << endl;
		      }
	      }
	      
      }
  }
  //donePSF_p=True;
  return 0;
}


/*******************************************************
 *  The following fns have been adapted from .....lattices/implement/Lattices/LatticeCleaner.cc
 *  ***************************************************************************/

#if 1

/*************************************
 *          make a blob
 *************************************/
// Make a single scale size image
Int WBCleanImageSkyModel::makeScale(Lattice<Float>& scale, const Float& scaleSize) 
{
  
  Int nx=scale.shape()(0);
  Int ny=scale.shape()(1);
  Matrix<Float> iscale(nx, ny);
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
  scale.putSlice(iscale, IPosition(scale.ndim(),0), IPosition(scale.ndim(),1));
  return 0;
}




/*************************************
 *          make a prolate spheroid
 *************************************/
Float WBCleanImageSkyModel::spheroidal(Float nu) 
{
  
  if (nu <= 0) {
    return 1.0;
  } else if (nu >= 1.0) {
    return 0.0;
  } else {
    uInt np = 5;
    uInt nq = 3;
    Matrix<float> p(np, 2);
    Matrix<float> q(nq, 2);
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

#endif
/*************************************
 *         Find the max and position 
 *         - restrict this to within the inner quarter.
 *************************************/
Bool WBCleanImageSkyModel::findMaxAbsLattice(const TempLattice<Float>& masklat,const Lattice<Float>& lattice,Float& maxAbs,IPosition& posMaxAbs, Bool flip)
{

  AlwaysAssert(masklat.shape()==lattice.shape(), AipsError);

  Array<Float> msk;
  
  posMaxAbs = IPosition(lattice.shape().nelements(), 0);
  maxAbs=0.0;
  //maxAbs=-1.0e+10;
  const IPosition tileShape = lattice.niceCursorShape();
  TiledLineStepper ls(lattice.shape(), tileShape, 0);
  TiledLineStepper lsm(masklat.shape(), tileShape, 0);
  {
    RO_LatticeIterator<Float> li(lattice, ls);
    RO_LatticeIterator<Float> lim(masklat, lsm);
    for(li.reset(),lim.reset();!li.atEnd();li++,lim++) 
    {
      IPosition posMax=li.position();
      IPosition posMin=li.position();
      Float maxVal=0.0;
      Float minVal=0.0;
      
      msk = lim.cursor();
      if(flip) msk = (Float)1.0 - msk;
      
      //minMaxMasked(minVal, maxVal, posMin, posMax, li.cursor(),lim.cursor());
      minMaxMasked(minVal, maxVal, posMin, posMax, li.cursor(),msk);
      
      
      if((maxVal)>(maxAbs)) 
      {
        maxAbs=maxVal;
	posMaxAbs=li.position();
	posMaxAbs(0)=posMax(0);
      }
     
      /*
      if(abs(minVal)>abs(maxAbs)) {
        maxAbs=minVal;
	posMaxAbs=li.position();
	posMaxAbs(0)=posMin(0);
      }
      if(abs(maxVal)>abs(maxAbs)) {
        maxAbs=maxVal;
	posMaxAbs=li.position();
	posMaxAbs(0)=posMax(0);
      }
      */
    }
  }

  return True;
}

/*************************************
 *    Add two subLattices..      -- same code as in copyData.   
 *************************************/
Int WBCleanImageSkyModel::addTo(Lattice<Float>& to, const Lattice<Float>& add, Float multiplier)
{
  // Check the lattice is writable.
  // Check the shape conformance.
  AlwaysAssert (to.isWritable(), AipsError);
  const IPosition shapeIn  = add.shape();
  const IPosition shapeOut = to.shape();
  AlwaysAssert (shapeIn.isEqual (shapeOut), AipsError);
  IPosition cursorShape = to.niceCursorShape();
  LatticeStepper stepper (shapeOut, cursorShape, LatticeStepper::RESIZE);
  LatticeIterator<Float> toIter(to, stepper);
  RO_LatticeIterator<Float> addIter(add, stepper);
  for (addIter.reset(), toIter.reset(); !addIter.atEnd();addIter++, toIter++) 
  {
    toIter.rwCursor()+=addIter.cursor()*multiplier;
  }
  return 0;
}

/***************************************
 * copy a templattice to an imageinterface
 ****************************************/

Bool WBCleanImageSkyModel::copyLatToImInt(TempLattice<Float>& lat, ImageInterface<Float>& im)
{
  	IPosition blc2(4,0,0,0,0);
  	IPosition trc2(4,nx,ny,0,0);
  	IPosition inc2(4, 1);
  	
  	LCBox::verify(blc2,trc2,inc2,im.shape());
  	LCBox copybox(blc2,trc2,im.shape());
  	(SubLattice<Float>(im,copybox,True)).copyData(LatticeExpr<Float> (lat));
	
	return True;
}

/***************************************
 * copy an imageinterface to a lattice
 ****************************************/

Bool WBCleanImageSkyModel::copyImIntToLat(TempLattice<Float>& lat, ImageInterface<Float>& im)
{
  	IPosition blc2(4,0,0,0,0);
  	IPosition trc2(4,nx,ny,0,0);
  	IPosition inc2(4, 1);
  	
  	LCBox::verify(blc2,trc2,inc2,im.shape());
  	LCBox copybox(blc2,trc2,im.shape());
	lat.copyData(SubLattice<Float>(im,copybox,True));
  
	return True;
}
/***************************************
 * Copy of cImage from ImageSkyModel.cc - for cImagespec.
 ****************************************/

ImageInterface<Complex>& WBCleanImageSkyModel::cImagespec(Int model,Int taylor) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert(ntaylor_p>1, AipsError);
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);

  Int index = model * (ntaylor_p-1) + (taylor-1);
  
  if(index>0&&(cimagespec_p[index-1])) cimagespec_p[index-1]->tempClose();

  Double memoryMB=HostInfo::memoryFree()/1024/(MEMFACTOR*maxnmodels_p*ntaylor_p);
  if(cimagespec_p[index]==0) {
    Vector<Int> whichStokes(0);
    IPosition cimageShape;
    cimageShape=image_p[model]->shape();
    
    Int npol=cimageShape(2);
    if(npol==3) cimageShape(2)=4;
    
    CoordinateSystem cimageCoord =
      StokesImageUtil::CStokesCoord(cimageShape,
				    image_p[model]->coordinates(),
				    whichStokes,
				    SkyModel::CIRCULAR);
    
    // Now set up the tile size, here we guess only
    IPosition tileShape(4, min(32, cimageShape(0)), min(32, cimageShape(1)),
			min(4, cimageShape(2)), min(32, cimageShape(3)));
    
    TempImage<Complex>* cimagePtr = 
      new TempImage<Complex> (IPosition(image_p[model]->ndim(),
					image_p[model]->shape()(0),
					image_p[model]->shape()(1),
					image_p[model]->shape()(2),
					image_p[model]->shape()(3)),
			      image_p[model]->coordinates(),
			      memoryMB);
    AlwaysAssert(cimagePtr, AipsError);
    cimagespec_p[index] = cimagePtr;
    cimagespec_p[index]->setMiscInfo(image_p[model]->miscInfo());
  }
  return *cimagespec_p[index];
};

/***************************************
 * Copy of image from ImageSkyModel.cc - for image1.
 ****************************************/
ImageInterface<Float>& WBCleanImageSkyModel::imagespec(Int model,Int taylor) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert(ntaylor_p>1, AipsError);

  Int index = model * (ntaylor_p-1) + (taylor-1);
  
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);
  AlwaysAssert(imagespec_p[index], AipsError);
  return *imagespec_p[index];
};


/***************************************
 * Copy of deltaimage from ImageSkyModel.cc - for deltaimagespec.
 ****************************************/
ImageInterface<Float>& WBCleanImageSkyModel::deltaImagespec(Int model, Int taylor) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert(ntaylor_p>1, AipsError);
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);

  Int index = model * (ntaylor_p-1) + (taylor-1);
  
  if(index>0&&(deltaimagespec_p[index-1])) deltaimagespec_p[index-1]->tempClose();

  if(deltaimagespec_p[index]==0) {
    Double memoryMB=HostInfo::memoryFree()/1024/(MEMFACTOR*maxnmodels_p*ntaylor_p);
    TempImage<Float>* deltaimagePtr = 
      new TempImage<Float> (image_p[model]->shape(),
			    image_p[model]->coordinates(),
			    memoryMB);
    AlwaysAssert(deltaimagePtr, AipsError);
    deltaimagespec_p[index] = deltaimagePtr;
  }
  return *deltaimagespec_p[index];
};


/***************************************
 * Copy of gS from ImageSkyModel.cc - for gSspec_p.
 ****************************************/

ImageInterface<Float>& WBCleanImageSkyModel::gSspec(Int model, Int taylor) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert(ntaylor_p>1, AipsError);
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);

  Int index = model * (ntaylor_p-1) + (taylor-1);
  
  if(index>0&&(gSspec_p[index-1])) gSspec_p[index-1]->tempClose();

  if(gSspec_p[index]==0) {
    Double memoryMB=HostInfo::memoryFree()/1024/(MEMFACTOR*maxnmodels_p*ntaylor_p);
    TempImage<Float>* gSPtr = 
      new TempImage<Float> (image_p[model]->shape(),
			     image_p[model]->coordinates(), memoryMB);
    AlwaysAssert(gSPtr, AipsError);
    gSspec_p[index] = gSPtr;
  }
  return *gSspec_p[index];
};

/***************************************
 * PSFspec --- to return psfspec_p
 ****************************************/

ImageInterface<Float>& WBCleanImageSkyModel::PSFspec(Int model,Int taylor) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert(ntaylor_p>1, AipsError);
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);

  Int index = model * (ntaylor_p-1) + (taylor-1);
  
  //if(model>0&&(psfspec_p[model-1])) psf1_p[model-1]->tempClose();
  if(index>0&&(psfspec_p[index-1])) psfspec_p[index-1]->tempClose();

  Double memoryMB=HostInfo::memoryFree()/1024/(MEMFACTOR*maxnmodels_p*ntaylor_p);
  if(psfspec_p[index]==0) {
    TempImage<Float>* psfPtr = 
      new TempImage<Float> (IPosition(image_p[model]->ndim(),
				      image_p[model]->shape()(0),
				      image_p[model]->shape()(1),
				      image_p[model]->shape()(2),
				      image_p[model]->shape()(3)),
			    image_p[model]->coordinates(),
			    memoryMB);
    AlwaysAssert(psfPtr, AipsError);
    psfspec_p[index] = psfPtr;
  }
  return *psfspec_p[index];
};


/***************************************
 *  add
 ****************************************/

//Int WBCleanImageSkyModel::add(ImageInterface<Float>& image,ImageInterface<Float>& image1, const Int maxNumXfr)
Int WBCleanImageSkyModel::add(ImageInterface<Float>& image, const Int maxNumXfr)
{
  Int thismodel=nmodels_p;
  nmodels_p++;

  if(nmodels_p>maxnmodels_p) maxnmodels_p=nmodels_p;

  maxNumXFR_p=maxNumXfr;

  image_p.resize(nmodels_p); 
  imagespec_p.resize(nmodels_p*(ntaylor_p-1));
  for(Int i=0;i<ntaylor_p-1;i++)
	  imagespec_p[thismodel*(ntaylor_p-1)+i]=0;
  
  cimage_p.resize(nmodels_p);
  cimagespec_p.resize(nmodels_p*(ntaylor_p-1));
  for(Int i=0;i<ntaylor_p-1;i++)
	  cimagespec_p[thismodel*(ntaylor_p-1)+i]=0;
  
  cxfr_p.resize(nmodels_p*maxNumXFR_p);
  residual_p.resize(nmodels_p);
  residualImage_p.resize(nmodels_p);
  
  gS_p.resize(nmodels_p);
  gSspec_p.resize(nmodels_p*(ntaylor_p-1));
  for(Int i=0;i<ntaylor_p-1;i++)
	  gSspec_p[thismodel*(ntaylor_p-1)+i]=0;
  
  psf_p.resize(nmodels_p);
  psfspec_p.resize(nmodels_p*(ntaylor_p-1));
  for(Int i=0;i<ntaylor_p-1;i++)
	  psfspec_p[thismodel*(ntaylor_p-1)+i]=0;
  
  ggS_p.resize(nmodels_p);
  fluxScale_p.resize(nmodels_p);
  work_p.resize(nmodels_p);
  
  deltaimage_p.resize(nmodels_p);
  deltaimagespec_p.resize(nmodels_p*(ntaylor_p-1));
  for(Int i=0;i<ntaylor_p-1;i++)
	  deltaimagespec_p[thismodel*(ntaylor_p-1)+i]=0;
  
  solve_p.resize(nmodels_p);
  doFluxScale_p.resize(nmodels_p);
  weight_p.resize(nmodels_p);
  beam_p.resize(nmodels_p);
  
  image_p[thismodel]=0;
  
  cimage_p[thismodel]=0;
  
  residual_p[thismodel]=0;

  for (Int numXFR=0;numXFR<maxNumXFR_p;numXFR++) {
    cxfr_p[thismodel*maxNumXFR_p+numXFR]=0;
  }
  residualImage_p[thismodel]=0;
  
  gS_p[thismodel]=0;
  
  psf_p[thismodel]=0;
  
  ggS_p[thismodel]=0;
  fluxScale_p[thismodel]=0;
  work_p[thismodel]=0;
  deltaimage_p[thismodel]=0;
  solve_p[thismodel]=True;
  doFluxScale_p[thismodel]=False;
  weight_p[thismodel]=0;
  beam_p[thismodel]=0;
  
  // Initialize image
  
  image_p[thismodel]=&image;
  AlwaysAssert(image_p[thismodel], AipsError);
  image_p[thismodel]->setUnits(Unit("Jy/pixel"));
  
  String modeltaylor("");
  Int index=0;
  for(Int taylor=1;taylor<ntaylor_p;taylor++)
  {
	  modeltaylor = image.name(False) + String(".spec_") + String::toString(taylor);
	  index = thismodel * (ntaylor_p-1) + (taylor-1);
	  
	  if(Table::isWritable(modeltaylor))
	  {
		  os << "Model exists for taylor" << taylor << LogIO::POST;
		  imagespec_p[index] = new PagedImage<Float>(modeltaylor);
		  AlwaysAssert(imagespec_p[index], AipsError);
	  }
	  else
	  {
		  os << "Model does not exist for taylor " << taylor << LogIO::POST;
		  imagespec_p[index]= new PagedImage<Float>(image.shape(),image.coordinates(),modeltaylor); 
		  AlwaysAssert(imagespec_p[index], AipsError);
		  imagespec(thismodel,taylor).set(0.0);
	  }
	  imagespec_p[index]->setUnits(Unit("Jy/pixel"));
  }
  
  donePSF_p=False;
  return thismodel;
}



/***************************************
 *  initializeGradients
 ****************************************/
void WBCleanImageSkyModel::initializeGradients()
{
	sumwt_p=0.0;
	chisq_p=0.0;
	for (Int thismodel=0;thismodel<nmodels_p;thismodel++) 
	{
		cImage(thismodel).set(Complex(0.0));
		gS(thismodel).set(0.0);
		ggS(thismodel).set(0.0);

		/* something wrong in indexing. */
		for(Int taylor=1;taylor<ntaylor_p;taylor++)
		{
			gSspec(thismodel,taylor).set(0.0);
			cImagespec(thismodel,taylor).set(Complex(0.0));
		}
	}
}


/***************************************
 *  Set up the Masks.
 ****************************************/
Int WBCleanImageSkyModel::setupMasks()
{

	// Check the validity of the mask.
	if(hasMask(0)) 
	{
		Int masknpol=mask(0).shape()(2);
		if(masknpol>1) 
		{
			if(masknpol!=npol_p) 
				os << "Mask has more than one polarization but not the same as the image" << LogIO::EXCEPTION;
			else 
				os << "Mask is a cube in polarization - will use appropriate plane for each polarization" << LogIO::POST;
		}
		Int masknchan=mask(0).shape()(3);
		if(masknchan>1) 
			os << "Mask cannot be specified per channel. Will use only first plane." << LogIO::POST;
	}
	
	/* Set up fftmask - inner quarter */
	
	(*fftmask_p).set(0.0);
	IPosition mblc(4,nx/4,ny/4,0,0);
	IPosition mtrc(4,3*nx/4,3*ny/4,0,0);
	IPosition minc(4, 1);
	LCBox::verify(mblc,mtrc,minc,(*fftmask_p).shape());
	LCBox regmask(mblc,mtrc,(*fftmask_p).shape());
	SubLattice<Float> smask((*fftmask_p),regmask,True);
	smask.set(True);
	//if(adbg)storeTLAsImg(String("maskfft")+String(".im"),(*fftmask_p), mask(0));
	
	Int pol=0;
	IPosition blc1(4,0,0,pol,0);
	IPosition trc1(4,nx,ny,pol,0);
	IPosition inc1(4, 1);
	
	/* Copy the input mask */
	LCBox::verify(blc1,trc1,inc1,mask(0).shape());
	LCBox singlepolmask(blc1,trc1,mask(0).shape());
	(mask_p)->copyData(SubLattice<Float>(mask(0),singlepolmask,True));
	//if(adbg)storeTLAsImg(String("mask")+String(".im"),(*mask_p), mask(0));
	
	/* Reconcile the two masks */
	(*mask_p).copyData(LatticeExpr<Float>((*mask_p)*(*fftmask_p)));
	
	return 0;
}/* end of setupMasks() */




/***************************************
 *  Set up the Blobs of various scales.
 ****************************************/
Int WBCleanImageSkyModel::setupBlobs()
{
	// Set the scale sizes
	if(scaleSizes_p.nelements()==0)
	{
		scaleSizes_p.resize(nscales_p);
		Float scaleInc = 2.0;
		scaleSizes_p[0] = 0.0;
		//os << "scale 1 = " << scaleSizes_p(0) << " pixels" << LogIO::POST;
		for (Int scale=1; scale<nscales_p;scale++) 
		{
			scaleSizes_p[scale] = scaleInc * pow(10.0, (Float(scale)-2.0)/2.0) ;
			//os << "scale " << scale+1 << " = " << scaleSizes_p(scale) << " pixels" << LogIO::POST;
			
		}  
	}
	
	scaleBias_p.resize(nscales_p);
	totalScaleFlux_p.resize(nscales_p);
	//Float prefScale=2.0;
	//Float fac=6.0;
	if(nscales_p>1)
	{
		for(Int scale=0;scale<nscales_p;scale++) 
		{
			//scaleBias_p[scale] = 1 - 0.4 * scaleSizes_p[scale]/scaleSizes_p(nscales_p-1);
			scaleBias_p[scale] = 1.0;
			//////scaleBias_p[scale] = pow((Float)scale/fac,prefScale)*exp(-1.0*scale/fac)/(pow(prefScale/fac,prefScale)*exp(-1.0*prefScale/fac));
			//scaleBias_p[scale] = pow((Float)(scale+1)/fac,prefScale)*exp(-1.0*(scale+1)/fac);
			os << "scale " << scale+1 << " = " << scaleSizes_p(scale) << " pixels with bias = " << scaleBias_p[scale] << LogIO::POST;
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
		os << "Calculating scales and their FTs " << LogIO::POST;
			
		for (Int scale=0; scale<nscales_p;scale++) 
		{
			AlwaysAssert(vecScales_p[scale], AipsError);
			AlwaysAssert(vecScalesFT_p[scale], AipsError);
			
			// First make the scale
			makeScale(*vecScales_p[scale], scaleSizes_p(scale));
			// Now store the XFR
			vecScalesFT_p[scale]->copyData(LatticeExpr<Complex>(toComplex((*fftmask_p)*(*vecScales_p[scale]))));
			// Now FFT
			LatticeFFT::cfft2d(*vecScalesFT_p[scale], True);
			if(0)//(adbg)
			{
				String llab("blob_"+String::toString((Int)scaleSizes_p(scale))+".im");
				storeTLAsImg(llab,(*vecScales_p[scale]), image(0));
				gip = IPosition(4,nx,ny,1,1);  
				TempLattice<Float> store(gip,memoryMB_p);
				store.copyData(LatticeExpr<Float>(real(*vecScalesFT_p[scale])));
				String fllab("blobft_"+String::toString((Int)scaleSizes_p(scale))+".im");
				storeTLAsImg(fllab,(store), image(0));
			}
		}
		donePSP_p=True;
	}
	
	return 0;
}/* end of setupBlobs() */



/***************************************
 *  Compute convolutions and the A matrix.
 ****************************************/
Int WBCleanImageSkyModel::computeMatrixA()
{
	gip = IPosition(4,nx,ny,1,1);  
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
		
		// (PSF * scale) * (PSF * scale) -> cubeA_p [nx,ny,ntaylor,ntaylor,nscales]
		os << "Calculating PSF and Scale convolutions " << LogIO::POST;
		IPosition wip(4,0,0,0,0);
		wip[0]=(nx/2); wip[1]=(ny/2);
		for (Int taylor1=0; taylor1<ntaylor_p;taylor1++) 
		for (Int taylor2=0; taylor2<=taylor1;taylor2++) 
		for (Int scale1=0; scale1<nscales_p;scale1++) 
		for (Int scale2=0; scale2<=scale1;scale2++) 
		{
			if(ddbg)os << "Calculating (PSF_"<< taylor1+1 << " * Scale_" << scale1+1 << ") * (PSF_"<< taylor2+1 << " * Scale_" << scale2+1 << ")"<< LogIO::POST;
			
			LatticeExpr<Complex> dpsExpr((*vecPsfFT_p[taylor1])*(*vecScalesFT_p[scale1]) *(*vecPsfFT_p[taylor2])*(*vecScalesFT_p[scale2]));
			cWork_p->copyData(dpsExpr);
			LatticeFFT::cfft2d(*cWork_p, False);
			AlwaysAssert(cubeA_p[IND4(taylor1,taylor2,scale1,scale2)], AipsError);
			LatticeExpr<Float> realWork2(real(*cWork_p));
			cubeA_p[IND4(taylor1,taylor2,scale1,scale2)]->copyData(realWork2);
			//String llab("convo_"+String::toString(taylor1)+String::toString(taylor2)+"_"+String::toString(scale1)+String::toString(scale2)+".im");
			//if(adbg)storeTLAsImg(llab,(*cubeA_p[IND4(taylor1,taylor2,scale1,scale2)]), image(0));
				
			if(0)
			{
				os << "ind : [" << taylor1 << "," << taylor2 << ","<< scale1 << ","<< scale2 << "] " << IND4(taylor1,taylor2,scale1,scale2) << " with value : " << (*cubeA_p[IND4(taylor1,taylor2,scale1,scale2)])(wip) << LogIO::POST;
				os << "xind : [" << taylor1 << "," << taylor2 << ","<< scale1 << ","<< scale2 << "] " << IND4(taylor2,taylor1,scale2,scale1) << " with value : " << (*cubeA_p[IND4(taylor2,taylor1,scale2,scale1)])(wip) << LogIO::POST;
				
			}
			
		}	  
		
		// Construct A, invA for each scale.
		
		//IPosition wip(4,0,0,0,0);
		//wip[0]=(nx/2); wip[1]=(ny/2);
		for (Int scale=0; scale<nscales_p;scale++) 
		{
			// Fill up A
			//Float maxv=0.0;
			//IPosition maxp(4,0,0,0,0);
			for (Int taylor1=0; taylor1<ntaylor_p;taylor1++) 
			for (Int taylor2=0; taylor2<ntaylor_p;taylor2++) 
			{
				(*matA_p[scale])(taylor1,taylor2) = (*cubeA_p[IND4(taylor1,taylor2,scale,scale)])(wip);
			}
			
			if(ddbg)os << "The Matrix A is : " << (*matA_p[scale]) << LogIO::POST;
			
			// Compute inv(A) 
			// Use MatrixMathLA::invert
			// of Use invertSymPosDef...
			//
			// Float deter=0.0;
			//MatrixMathLA::invert((*invMatA_p[scale]),deter,(*matA_p[scale]));
			//cout << "A matrix determinant : " << deter << endl;
			//if(fabs(deter) < 1.0e-08) os << "SINGULAR MATRIX !! STOP!! " << LogIO::EXCEPTION;
			
			Float det=0.0;
			switch(ntaylor_p)
			{
				case 1:
					if(fabs((*matA_p[scale])(0,0))< 1.0e-12) os << "SINGULAR MATRIX !! STOP!! " << LogIO::EXCEPTION;
					(*invMatA_p[scale])(0,0) = 1.0/(*matA_p[scale])(0,0);
					break;
				case 2:
					det = (*matA_p[scale])(0,0)*(*matA_p[scale])(1,1) - (*matA_p[scale])(1,0)*(*matA_p[scale])(0,1);
					if(fabs(det)< 1.0e-12) os << "SINGULAR MATRIX !! STOP!! " << LogIO::EXCEPTION;
					det = 1.0/det;
					(*invMatA_p[scale])(0,0) = det * (*matA_p[scale])(1,1);
					(*invMatA_p[scale])(1,1) = det * (*matA_p[scale])(0,0);
					(*invMatA_p[scale])(0,1) = -1* det * (*matA_p[scale])(0,1);
					(*invMatA_p[scale])(1,0) = -1* det * (*matA_p[scale])(1,0);
					break;
				case 3:
					det = (*matA_p[scale])(0,0)
						*( ((*matA_p[scale])(1,1)*(*matA_p[scale])(2,2))
						-((*matA_p[scale])(1,2)*(*matA_p[scale])(2,1)) )
						- (*matA_p[scale])(0,1)*( ((*matA_p[scale])(1,0)*(*matA_p[scale])(2,2))
						-((*matA_p[scale])(1,2)*(*matA_p[scale])(2,0)) )
						+ (*matA_p[scale])(0,2)*( ((*matA_p[scale])(1,0)*(*matA_p[scale])(2,1))
						-((*matA_p[scale])(1,1)*(*matA_p[scale])(2,0)) );
					if(fabs(det)< 1.0e-12) os << "SINGULAR MATRIX !! STOP!! " << LogIO::EXCEPTION;
					det = 1.0/det;
					(*invMatA_p[scale])(0,0) = det * ( ((*matA_p[scale])(1,1)*(*matA_p[scale])(2,2))
									   - ((*matA_p[scale])(1,2)*(*matA_p[scale])(2,1)) );
					(*invMatA_p[scale])(0,1) = -1*det * ( ((*matA_p[scale])(0,1)*(*matA_p[scale])(2,2))
									      - ((*matA_p[scale])(2,1)*(*matA_p[scale])(0,2)) );
					(*invMatA_p[scale])(0,2) = det * ( ((*matA_p[scale])(0,1)*(*matA_p[scale])(1,2))
									   - ((*matA_p[scale])(1,1)*(*matA_p[scale])(0,2)) );
					(*invMatA_p[scale])(1,0) = -1*det * ( ((*matA_p[scale])(1,0)*(*matA_p[scale])(2,2))
									      - ((*matA_p[scale])(2,0)*(*matA_p[scale])(1,2)) );
					(*invMatA_p[scale])(1,1) = det * ( ((*matA_p[scale])(0,0)*(*matA_p[scale])(2,2))
									   - ((*matA_p[scale])(2,0)*(*matA_p[scale])(0,2)) );
					(*invMatA_p[scale])(1,2) = -1*det * ( ((*matA_p[scale])(0,0)*(*matA_p[scale])(1,2))
									      - ((*matA_p[scale])(1,0)*(*matA_p[scale])(0,2)) );
					(*invMatA_p[scale])(2,0) = det * ( ((*matA_p[scale])(1,0)*(*matA_p[scale])(2,1))
									   - ((*matA_p[scale])(2,0)*(*matA_p[scale])(1,1)) );
					(*invMatA_p[scale])(2,1) = -1*det * ( ((*matA_p[scale])(0,0)*(*matA_p[scale])(2,1))
									      - ((*matA_p[scale])(0,1)*(*matA_p[scale])(2,0)) );
					(*invMatA_p[scale])(2,2) = det * ( ((*matA_p[scale])(0,0)*(*matA_p[scale])(1,1))
									   - ((*matA_p[scale])(1,0)*(*matA_p[scale])(0,1)) );
					break;
				default:
					os << "Can only handle up to 3 Taylor terms so far....." << LogIO::EXCEPTION ;
					// Need MatrixMathLA.h for this...
			}//end of switch
			if(ddbg)os << "Matrix inv(A) is : " << (*invMatA_p[scale]) << LogIO::POST;
		}
		
		doneCONV_p=True;
	} 
	
	return 0;
}/* end of computeMatrixA() */



/***************************************
 *  Read in the model images
 ****************************************/
Int WBCleanImageSkyModel::readInModels()
{
	// Read in the model images - in case there is a starting point
	
	if(ntaylor_p>1)
	{
		IPosition blc2(4,0,0,0,0);
		IPosition trc2(4,nx,ny,0,0);
		IPosition inc2(4, 1);
		LCBox::verify(blc2,trc2,inc2,image(0).shape());
		LCBox onetaylor(blc2,trc2,image(0).shape());
		
		if(fabs(weightScaleFactor_p)>1e-8) 
		{
			for(Int taylor=1;taylor<ntaylor_p;taylor++)
				(SubLattice<Float>(imagespec(0,taylor),onetaylor,True)).copyData(LatticeExpr<Float> (imagespec(0,taylor)/(pow(weightScaleFactor_p,taylor))));
		}
	}
	
	return 0;
}/* end of readInModels() */


/***************************************
 *  Compute convolutions of the residual image with everything else
 *  --> the Right-Hand-Side of the matrix equation.
 ****************************************/
Int WBCleanImageSkyModel::computeRHS()
{
	IPosition blc1(4,0,0,0,0);
	IPosition trc1(4,nx,ny,0,0);
	IPosition inc1(4, 1);
	
	/* Copy residual into I_D */
	LCBox::verify(blc1,trc1,inc1,residual(0).shape());
	LCBox singlepolres(blc1,trc1,residual(0).shape());
	(dirty_p)->copyData(SubLattice<Float>(residual(0),singlepolres,True));
	
	/* Compute FT of dirty image */
	dirtyFT_p->copyData(LatticeExpr<Complex>(toComplex((*fftmask_p)*(*dirty_p))));
	LatticeFFT::cfft2d(*dirtyFT_p, True);
	
	/* Compute R10 = I_D*B10, R11 = I_D*B11, R12 = I_D*B12
	 * Compute R20 = I_D*B20, R21 = I_D*B21, R22 = I_D*B22
	 * ... depending on the number of scales chosen.
	 */
	
	/* I_D * (PSF * scale) -> matR_p [nx,ny,ntaylor,nscales] */
	//os << "Calculating I_D * PSF_i * Scale_j " << LogIO::POST;
	for (Int taylor=0; taylor<ntaylor_p;taylor++) 
	for (Int scale=0; scale<nscales_p;scale++) 
	{
		if(ddbg)os << "Calculating I_D * (PSF_"<< taylor+1 << " * Scale_" << scale+1 << ")"<< LogIO::POST;
		LatticeExpr<Complex> dpsExpr( (*dirtyFT_p)*(*vecPsfFT_p[taylor])*(*vecScalesFT_p[scale]));
		cWork_p->copyData(dpsExpr);
		LatticeFFT::cfft2d(*cWork_p, False);
		AlwaysAssert(matR_p[IND2(taylor,scale)], AipsError);
		LatticeExpr<Float> realWork2(real(*cWork_p));
		matR_p[IND2(taylor,scale)]->copyData(realWork2);
		//String lab("_"+String::toString(taylor)+"_"+String::toString(scale));
		//if(adbg)storeTLAsImg(String("smoothdirty")+lab+String(".im"),(*matR_p[IND2(taylor,scale)]), image(0));
	}	  
		
	return 0;
}/* end of computeRHS() */



/***************************************
 *  Compute  flux limit for minor cycles
 ****************************************/
Int WBCleanImageSkyModel::computeFluxLimit(Float &fluxlimit, Float threshold)
{

	// Find max residual ( from all scale and taylor convos of the residual image )
	// Find max ext PSF value ( from all scale convos of all the PSFs )
	// factor = 0.5;
	// fluxlimit = maxRes * maxExtPsf * factor;
	
	Float maxRes=0.0;
	Float maxExtPsf=0.0;
	Float tmax=0.0;
	IPosition tmaxpos;
	Float ffactor=0.01;
	Int maxscale=0;
	
	for(Int taylor=0;taylor<ntaylor_p;taylor++)
	for(Int scale=0; scale<nscales_p;scale++)
	{
		findMaxAbsLattice((*mask_p),(*matR_p[IND2(taylor,scale)]),tmax,tmaxpos);
		if(tmax > maxRes) maxscale = scale;
		maxRes = MAX(maxRes,tmax);
	}
	for (Int taylor1=0; taylor1<ntaylor_p;taylor1++) 
	for (Int taylor2=0; taylor2<=taylor1;taylor2++) 
	for (Int scale1=0; scale1<nscales_p;scale1++) 
	for (Int scale2=0; scale2<=scale1;scale2++) 
	{
		findMaxAbsLattice((*mask_p),(*cubeA_p[IND4(taylor1,taylor2,scale1,scale2)]),tmax,tmaxpos, True);
		maxExtPsf = MAX(maxExtPsf,tmax);
	}
	
	Float norma = sqrt((1.0/(*matA_p[maxscale])(0,0)));
	
	fluxlimit = max(threshold, (maxRes*norma) * maxExtPsf * ffactor);
	
	if(ddbg)os << "Max Res : " << maxRes << "   Max Ext PSF : " << maxExtPsf << "   FluxLimit : " << fluxlimit << LogIO::POST;
	
	return 0;
}/* end of computeFluxLimit() */



/***************************************
 *  Solve the matrix eqn for each point in the lattice.
 ****************************************/
Int WBCleanImageSkyModel::solveMatrixEqn(Int scale)
{
#if 1
	/* Solve for the coefficients */
	for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
	{
		len_p = LatticeExprNode(0.0);
		for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
		{
			len_p = len_p + LatticeExprNode((Float)(*invMatA_p[scale])(taylor1,taylor2)*(*matR_p[IND2(taylor2,scale)]));
		}
		(*matCoeffs_p[IND2(taylor1,scale)]).copyData(LatticeExpr<Float>(len_p));
	}
#else

	for(Int i=0;i<(Int)itermatCoeffs_p.nelements();i++) itermatCoeffs_p[i]->reset();
	for(Int i=0;i<(Int)itercubeA_p.nelements();i++) itercubeA_p[i]->reset();
	for(Int i=0;i<(Int)itermatR_p.nelements();i++) itermatR_p[i]->reset();
	
	for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
	{
		//matCoeffs_p[IND2(taylor1,scale)]->set(0.0);
		for(Int taylor2=0;taylor2<ntaylor_p;taylor2++) itermatR_p[IND2(taylor2,scale)]->reset();
		
		for((itermatCoeffs_p[IND2(taylor1,scale)])->reset(); !((itermatCoeffs_p[IND2(taylor1,scale)])->atEnd()); (*itermatCoeffs_p[IND2(taylor1,scale)])++)
		{
			(itermatCoeffs_p[IND2(taylor1,scale)])->rwCursor() = 0.0;
			for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
			{
				(itermatCoeffs_p[IND2(taylor1,scale)])->rwCursor() = (itermatCoeffs_p[IND2(taylor1,scale)])->rwCursor() + (Float)(*invMatA_p[scale])(taylor1,taylor2) * (itermatR_p[IND2(taylor2,scale)])->rwCursor();
				(*itermatR_p[IND2(taylor2,scale)])++;
			}
		}
	}
#endif
	
	return 0;
}/* end of solveMatrixEqn() */
	
/***************************************
 *  Compute the penalty function
 ****************************************/
Int WBCleanImageSkyModel::computePenaltyFunction(Int scale, Int totaliters, Float &loopgain, Bool choosespec)
{
	tWork_p->set(0.0);
	
	for(Int i=0;i<(Int)itermatCoeffs_p.nelements();i++) itermatCoeffs_p[i]->reset();
	for(Int i=0;i<(Int)itercubeA_p.nelements();i++) itercubeA_p[i]->reset();
	for(Int i=0;i<(Int)itermatR_p.nelements();i++) itermatR_p[i]->reset();
	
	for(itertWork_p->reset(); !(itertWork_p->atEnd()); (*itertWork_p)++)
	{
		if(choosespec)
		{
			for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
			{
				itertWork_p->rwCursor() += (Float)2.0*((itermatCoeffs_p[IND2(taylor1,scale)])->rwCursor())*((itermatR_p[IND2(taylor1,scale)])->rwCursor());
				
				for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
					itertWork_p->rwCursor() -= ((itermatCoeffs_p[IND2(taylor1,scale)])->rwCursor())*((itermatCoeffs_p[IND2(taylor2,scale)])->rwCursor())*((itercubeA_p[IND4(taylor1,taylor2,scale,scale)])->rwCursor());
			}
		}
		else
		{
			if(loopgain > 0.5) loopgain*=0.5;
			Float norm = sqrt((1.0/(*matA_p[scale])(0,0)));
			itertWork_p->rwCursor() += norm*((itermatR_p[IND2(0,scale)])->rwCursor());
		}
		for(Int i=0;i<(Int)itermatCoeffs_p.nelements();i++) (*itermatCoeffs_p[i])++;
		for(Int i=0;i<(Int)itercubeA_p.nelements();i++) (*itercubeA_p[i])++;
		for(Int i=0;i<(Int)itermatR_p.nelements();i++) (*itermatR_p[i])++;
	}
	
	return 0;
}/* end of computePenaltyFunction() */

#if 0

/***************************************
 *  Compute the penalty function
 ****************************************/
Int WBCleanImageSkyModel::computePenaltyFunction(Int scale, Int totaliters, Float &loopgain)
{

	tWork_p->set(0.0);
	len_p = LatticeExprNode(0.0);
	
	if(totaliters>0)
	//if(totaliters>20)
	{
		/* Method 1 : Choose the set of coeffs that minimizes ChiSq */
		// Note that this is subject to the ambiguity between scale and spindex
		// Compute Eps sq for every point in the Lattice
		// Eps = 2*\sum_k a_{kj} R_{k}(j) - \sum_k \sum_l a_{kj} a_{lj} A_{kl}(0)
		// Initialize Eps sq in twork.
		
		// Fill in Eps sq into twork
		
		for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
		{
			len_p = len_p + 2.0*(*matCoeffs_p[IND2(taylor1,scale)])*(*matR_p[IND2(taylor1,scale)]);

			for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
				len_p = len_p - (*matCoeffs_p[IND2(taylor1,scale)])*(*matCoeffs_p[IND2(taylor2,scale)])*(*cubeA_p[IND4(taylor1,taylor2,scale,scale)]);
		}
		
		// Try a bias on alpha
		//len_p = len_p - abs((*matCoeffs_p[IND2(1,scale)])/(*matCoeffs_p[IND2(0,scale)]));
			
	}
	else
	{
		if(loopgain > 0.5) loopgain*=0.5;
		
		/* Method 2 : Choose the set of coeffs corresponding to the
		 * location and scale of the global Max residual */
		
		// Find the max from matR_p directly only for B0.
		// attempt to normalize by the area under the psf...
		Float norm = sqrt((1.0/(*matA_p[scale])(0,0)));
		//os << "Normalizing scale " << scale << " by " << norm << LogIO::POST;
		len_p = len_p + norm*(*matR_p[IND2(0,scale)]);
		
	}

	/* modify the penalty function with the lagrange multiplier */
	/// there could be a "divide by zero" problem here :-(
	///len_p = len_p - lambda_p * pow(((*matCoeffs_p[IND2(1,scale)])/(*matCoeffs_p[IND2(0,scale)])),2);
	
	/* Evaluate the penalty function */
	tWork_p->copyData(LatticeExpr<Float>(len_p));
	
	return 0;
}/* end of computePenaltyFunction() */
#endif


/***************************************
 *  Update the model images and the convolved residuals
 ****************************************/
Int WBCleanImageSkyModel::updateSolution(IPosition globalmaxpos, Int maxscaleindex, Float loopgain)
{
	gip = IPosition(4,nx,ny,1,1);  
	
	IPosition support(4,nx/2,ny/2,0,0);
	
	IPosition mid(support);
	globalmaxpos[2]=0;
	globalmaxpos[3]=0;
	
	/* Region for the inner quarter..... the update region. */
	IPosition inc(4,1,1,0,0);
	IPosition blc(mid-support/2);
	IPosition trc(mid+support/2-IPosition(4,1,1,0,0));
	LCBox::verify(blc, trc, inc, gip);
	
	/* Shifted region, with the psf at the globalmaxpos. */
	IPosition blcPsf(2*mid-support/2-globalmaxpos);
	IPosition trcPsf(2*mid+support/2-globalmaxpos-IPosition(4,1,1,0,0));
	LCBox::verify(blcPsf, trcPsf, inc, gip);
	
	LCBox subRegion(blc,trc,gip);
	LCBox subRegionPsf(blcPsf,trcPsf,gip);
	
	/* Update the model image */
	//if(maxscaleindex > 1)
	for(Int taylor=0;taylor<ntaylor_p;taylor++)
	{
		if(taylor==0)
		{
			SubLattice<Float> modelSub((image(0)),subRegion,True);
			SubLattice<Float> scaleSub((*vecScales_p[maxscaleindex]),subRegionPsf,True);
			//LatticeExpr<Float> add(loopgain*(*matCoeffs_p[IND2(taylor,maxscaleindex)]).getAt(globalmaxpos) * scaleSub);
			//addTo(modelSub,add);
			addTo(modelSub,scaleSub,loopgain*(*matCoeffs_p[IND2(taylor,maxscaleindex)]).getAt(globalmaxpos));
		}
		else
		{
			SubLattice<Float> modelSub((imagespec(0,taylor)),subRegion,True);
			SubLattice<Float> scaleSub((*vecScales_p[maxscaleindex]),subRegionPsf,True);
			//LatticeExpr<Float> add(loopgain*(*matCoeffs_p[IND2(taylor,maxscaleindex)]).getAt(globalmaxpos) * scaleSub);
			//addTo(modelSub,add);
			addTo(modelSub,scaleSub,loopgain*(*matCoeffs_p[IND2(taylor,maxscaleindex)]).getAt(globalmaxpos));
		}
	}
	
	/* Update the convolved residuals */
	for(Int scale=0;scale<nscales_p;scale++)
	for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
	{
		SubLattice<Float> residSub((*matR_p[IND2(taylor1,scale)]),subRegion,True);
		for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
		{
			SubLattice<Float> smoothSub((*cubeA_p[IND4(taylor1,taylor2,scale,maxscaleindex)]),subRegionPsf,True);
			//LatticeExpr<Float> subtract(-1*loopgain*(*matCoeffs_p[IND2(taylor2,maxscaleindex)]).getAt(globalmaxpos)* smoothSub);
			//addTo(residSub,subtract);
			addTo(residSub,smoothSub,-1*loopgain*(*matCoeffs_p[IND2(taylor2,maxscaleindex)]).getAt(globalmaxpos));
		}
	}
		  
	/* Update flux counters */
	for(Int taylor=0;taylor<ntaylor_p;taylor++)
	{
		totalTaylorFlux_p[taylor] += loopgain*(*matCoeffs_p[IND2(taylor,maxscaleindex)]).getAt(globalmaxpos);
	}
	totalScaleFlux_p[maxscaleindex] += loopgain*(*matCoeffs_p[IND2(0,maxscaleindex)]).getAt(globalmaxpos);

	
	
	return 0;
}/* end of updateSolution() */




} //# NAMESPACE CASA - END


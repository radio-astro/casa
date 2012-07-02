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
//# $Id: WBCleanImageSkyModel.cc 13615 2010-12-20 14:04:00 UrvashiRV$


#include <casa/Arrays/ArrayMath.h>
#include <synthesis/MeasurementComponents/WBCleanImageSkyModel.h>
#include <synthesis/MeasurementEquations/CubeSkyEquation.h>
#include <casa/OS/File.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <synthesis/MeasurementEquations/LatticeModel.h>
#include <synthesis/MeasurementEquations/LatConvEquation.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

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
	
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
#define MAX(a,b) ((a)>=(b) ? (a) : (b))

/*************************************
 *          Constructor
 *************************************/
WBCleanImageSkyModel::WBCleanImageSkyModel()
{
  initVars();
  nscales_p=4;
  ntaylor_p=2;
  refFrequency_p=1.42e+09;
  scaleSizes_p.resize(0);
};
  WBCleanImageSkyModel::WBCleanImageSkyModel(const Int ntaylor,const Int nscales,const Double reffreq)
{
  initVars();
  nscales_p=nscales;
  ntaylor_p=ntaylor;
  refFrequency_p=reffreq;
  scaleSizes_p.resize(0);
};
  WBCleanImageSkyModel::WBCleanImageSkyModel(const Int ntaylor,const Vector<Float>& userScaleSizes,const Double reffreq)
{
  initVars();
  if(userScaleSizes.nelements()==0)  os << "No scales set !" << LogIO::WARN;
  if(userScaleSizes[0]!=0.0)
  {
    nscales_p=userScaleSizes.nelements()+1;
    scaleSizes_p.resize(nscales_p);
    for(Int i=1;i<nscales_p;i++) scaleSizes_p[i] = userScaleSizes[i-1];
    scaleSizes_p[0]=0.0;
  }
  else
  {
    nscales_p=userScaleSizes.nelements();
    scaleSizes_p.resize(nscales_p);
    for(Int i=0;i<nscales_p;i++) scaleSizes_p[i] = userScaleSizes[i];
  }
  ntaylor_p=ntaylor;
  refFrequency_p=reffreq;
};

void WBCleanImageSkyModel::initVars()
{
  adbg=0; 
  ddbg=1; // output per iteration
  tdbg=0;
  
  modified_p=True;
  memoryMB_p = Double(HostInfo::memoryTotal(true)/1024)/(2.0);
  donePSF_p=False;

  numbermajorcycles_p=0;
  nfields_p=1;
  lc_p.resize(0);

  os = LogIO(LogOrigin("WBCleanImageSkyModel","solve"));

  setAlgorithm("MSMFS");
  
}

/*************************************
 *          Destructor
 *************************************/
WBCleanImageSkyModel::~WBCleanImageSkyModel()
{
  lc_p.resize(0);
};

/*************************************
 *          Solver
 *************************************/
Bool WBCleanImageSkyModel::solve(SkyEquation& se) 
{
	os << "MSMFS algorithm (v2.3) with " << ntaylor_p << " Taylor coefficients and Reference Frequency of " << refFrequency_p  << " Hz" << LogIO::POST;
	Int stopflag=0;
	Int nchan=0,npol=0;

	/* Gather shape information */
	nmodels_p = numberOfModels();

	if(nmodels_p % ntaylor_p != 0)
	{
	  os << "Incorrect number of input models " << LogIO::EXCEPTION;
	  os << "NModels = N_fields x N_taylor" << LogIO::EXCEPTION;
	  AlwaysAssert((nmodels_p % ntaylor_p == 0), AipsError);
	}

	/* Check supplied bandwidth-ratio and print warnings if needed */
        checkParameters();

	/* Calc the number of fields */
	nfields_p = nmodels_p/ntaylor_p;
        ///// NOTE : Code is written with loops on 'fields' to support a future implementation
        /////             Disable it for now, since it has not been tested.
        AlwaysAssert(nfields_p==1, AipsError);

        /* Current restriction : one pol and one chan-plane */
	for(Int model=0;model<nmodels_p;model++)
	{
	  nx = image(model).shape()(0);
	  ny = image(model).shape()(1);
	  npol=image(model).shape()(2);
	  nchan=image(model).shape()(3);
	  if(nchan > 1) os << "Cannot process more than one output channel" << LogIO::EXCEPTION;
	  if(npol > 1) os << "Cannot process more than one output polarization" << LogIO::EXCEPTION;
	  AlwaysAssert((nchan==1), AipsError);  
	  AlwaysAssert((npol==1), AipsError);  
	}

        /* Check if each field is solveable (not sure what this means) */
	for(Int field=0;field<=nfields_p;field++)
	  if( !isSolveable(getModelIndex(field,0)) ) 
	  {
		  os << "No more processing on this field" << LogIO::POST;
		  return True;
	  }
	
	/* Calculate the initial residual image for all models. */
	if(!donePSF_p)
	{
	   os << "Calculating initial residual images..." << LogIO::POST;
	   solveResiduals(se);
	}

	/* Initialize the MultiTermMatrixCleaners */
	if(adbg) cout << "Shape of lc_p : " << lc_p.nelements() << endl;
	if(lc_p.nelements()==0)
	{
		lc_p.resize(nfields_p);
                Bool state=True;
		for(Int thismodel=0;thismodel<nfields_p;thismodel++)
		{
			lc_p[thismodel].setscales(scaleSizes_p);
			lc_p[thismodel].setntaylorterms(ntaylor_p);
			nx = image(thismodel).shape()(0);
			ny = image(thismodel).shape()(1);
                        state &= lc_p[thismodel].initialise(nx,ny); // allocates memory once....
                }
                if( !state ) // initialise will return False if there is any internal inconsistency with settings so far.
		{
		  lc_p.resize(0);
		  //                  os << "Could not initialize MS-MFS minor cycle" << LogIO::EXCEPTION;
                  os << LogIO::SEVERE << "Could not initialize MS-MFS minor cycle" << LogIO::POST;
                  return False;
		}
	}

	/* Create the Point Spread Functions */
	if(!donePSF_p)
	{
  	    /* Resize the work arrays to calculate extra PSFs */
	    Int original_nmodels = nmodels_p;
	    nmodels_p = original_nmodels + nfields_p * (ntaylor_p - 1);
	    resizeWorkArrays(nmodels_p);

	    try
	    {
	      /* Make the 2N-1 PSFs */
	      os << "Calculating spectral PSFs..." << LogIO::POST;
              makeSpectralPSFs(se);
	    }
	    catch(AipsError &x)
	    {
	      /* Resize the work arrays to normal size - the destructors use 'nmodels_p' on other lists */
	      nmodels_p = original_nmodels;
	      resizeWorkArrays(nmodels_p);
	      os << "Could not make PSFs. Please check image co-ordinate system : " << x.getMesg() << LogIO::EXCEPTION;
	    }

	    /* Send all 2N-1 PSFs into the MultiTermLatticeCleaner */
	    for(Int thismodel=0;thismodel<nfields_p;thismodel++)
	    {
	      for (Int order=0;order<2*ntaylor_p-1;order++)
	      {
	        // This should be doing a reference only. Make sure of this.
	        Matrix<Float> tempMat;
	        Array<Float> tempArr;
                (PSF(getModelIndex(thismodel,order))).get(tempArr,True);
                tempMat.reference(tempArr);
  
	        lc_p[thismodel].setpsf( order , tempMat ); 
	      }
	    }
	  
	    /* Resize the work arrays to normal size - for residual comps, etc. */
	    nmodels_p = original_nmodels;
	    resizeWorkArrays(nmodels_p);
            ///// TODO : Make sure this releases memory as it is supposed to.	   

	    donePSF_p=True;
	}
	
	/* Return if niter=0 */
	/* Check if this is an interactive-clean run, or if niter=0 */
	if(adbg) cout << "NumberIterations - before any cycles: " << numberIterations() << endl;
	if(numberIterations() < 1)
	{
		return True;
	}

	/* Set up the Mask image */
	for(Int thismodel=0;thismodel<nfields_p;thismodel++)
	{
	  if(hasMask(getModelIndex(thismodel,0))) 
	  {
	    os << "Sending in the mask" << LogIO::POST;

            Matrix<Float> tempMat;
            Array<Float> tempArr;
            (mask(getModelIndex(thismodel,0))).get(tempArr,True);
            tempMat.reference(tempArr);

	    lc_p[thismodel].setmask( tempMat );
	  }
	}

	/******************* START MAJOR CYCLE LOOP *****************/
        /* Logic for number of iterations in the minor-cycle vs major cycle vs total,
           how they relate to convergence thresholds, and how they change with 
           interactive vs non-interactive use (user specified total niter, vs niters per 
           major cycle...) is a bit of a mess. Needs cleanup. Right now, hard-coded for
           nfields=1 */
        previous_maxresidual_p=1e+10;
	Int index=0;
        Float fractionOfPsf=0.1;
        Int iterationcount=0;
        Int moreiterations=0;
        Int thiscycleniter=0;
	for(Int itercountmaj=0;itercountmaj<1000;itercountmaj++)
	{
          numbermajorcycles_p ++;
          os << "**** Major Cycle " << numbermajorcycles_p << LogIO::POST;
          thiscycleniter=0;
	  for(Int thismodel=0;thismodel<nfields_p;thismodel++) // For now, nfields_p=1 always.
	    {
             /* Compute stopping threshold for this major cycle */
             stopflag = computeFluxLimit(thismodel,fractionOfPsf);
	     /* If the peak residual is already less than the user-threshold, stop */
             if(stopflag==1) break;
             /* If we detect divergence across major cycles, stop */
             if(stopflag==-1) break;
             /* Number of iterations left to do */ 
             moreiterations = numberIterations() - iterationcount;
             /* If all iterations are done for this run, stop */
             if(moreiterations <=0 ) {stopflag=-1;break;}
             
             /* Send in control parameters */
	     ///////          lc_p[thismodel].setcontrol(CleanEnums::MULTISCALE, moreiterations, gain(), Quantity(cyclethreshold,"Jy"));

             // cyclethreshold = fractionOfPsf... use this ro get fluxlimit internally, and set max loop gain to 1-this...
             // ignoring user-specified loop gain.
             //lc_p[thismodel].setcontrol(CleanEnums::MULTISCALE, moreiterations, cyclethreshold, Quantity(threshold(),"Jy"));
		
	     /* Send in the current model and residual */
	     for (Int order=0;order<ntaylor_p;order++)
	     {
	        index = getModelIndex(thismodel,order);

                Matrix<Float> tempMat;
                Array<Float> tempArr;
        
                (residual(index)).get(tempArr,True);
                tempMat.reference(tempArr);
		lc_p[thismodel].setresidual(order,tempMat); 
		
                (image(index)).get(tempArr,True);
                tempMat.reference(tempArr);
                lc_p[thismodel].setmodel(order,tempMat); 
	     }

	     /* Deconvolve */
             /* Return value is the number of minor-cycle iterations done */
	     thiscycleniter = lc_p[thismodel].mtclean(moreiterations, fractionOfPsf, gain(), threshold());

             /* A signal for a singular Hessian : stop */
             if(thiscycleniter==-2) { stopflag=-2; break;}
             /* A signal for convergence with no iterations  */
             if(thiscycleniter==0) { stopflag=1; break; }
	     ///* A signal for divergence. Save current model and stop (force a major cycle). */
             //if(thiscycleniter==-1) { stopflag=1; }

             /* Increment the minor-cycle iteration counter */
             iterationcount += thiscycleniter;

	     /* Get out the updated model */
	     for (Int order=0;order<ntaylor_p;order++)
	     {
	        index = getModelIndex(thismodel,order);
                Matrix<Float> tempMod;
                lc_p[thismodel].getmodel(order,tempMod);
                image(index).put(tempMod);
	     }           

	    }// end of model loop

            if(adbg) cout << "end of major cycle : " << itercountmaj << " -> iterationcount : " << iterationcount << "  thiscycleniter : " << thiscycleniter << endl;

	   /* Exit without further ado if MTMC cannot invert matrices */
	   if(stopflag == -2)
	   {
	      os << "Cannot invert Multi-Term Hessian matrix. Please check the reference-frequency and ensure that the number of frequency-channels in the selected data >= nterms" << LogIO::WARN << LogIO::POST;
	      break;
	   }

	   /* If reached 1000 major cycles - assume something is wrong */
	   if(itercountmaj==999) os << " Reached the allowed maximum of 1000 major cycles " << LogIO::POST;

	   /* Do the prediction and residual computation for all models. */
           /* Exit if we're already at the user-specified flux threshold, or reached numberIterations() */
	   if( abs(stopflag)==1 || iterationcount >= numberIterations() || itercountmaj==999) 
	   {
	       os << "Calculating final residual images..." << LogIO::POST;
	      /* Call 'solveResiduals' with modelToMS = True to write the model to the MS */
	       solveResiduals(se,True);
               break;
	   }
	   else 
	   {
	       os << "Calculating new residual images..." << LogIO::POST;
	       solveResiduals(se);
	   }
	   
	} 
	/******************* END MAJOR CYCLE LOOP *****************/
	
	/* Compute and write alpha,beta results to disk */
	writeResultsToDisk();
	
	/* stopflag=1 -> stopped because the user-threshold was reached */
        /* stopflag=-1 -> stopped because number of iterations was reached */
        /* stopflag=-2 -> stopped because of singular Hessian */
        /* stopflag=0 -> reached maximum number of major-cycle iterations !! */
	if(stopflag>0) return(True);
	else return(False);
} // END OF SOLVE


/* Calculate stopping threshold for the current set of minor-cycle iterations */
/* Note : The stopping threshold is computed from the residual image.
              However, MTMC checks on the peak of (residual * psf * scale_0).
              These values can differ slightly. 
              TODO : Perhaps move this function back into MTMC. 
                          (It was brought out to conform to the other devonvolvers..)
*/
Float WBCleanImageSkyModel::computeFluxLimit(Int model,Float &fractionOfPsf)
{
  LogIO os(LogOrigin("WBCleanImageSkyModel", "computeFluxLimit", WHERE));
  Float maxres=0.0,maxval=0.0,minval=0.0;

   /* Measure the peak residual */
  Float index = getModelIndex(model,0);
  Array<Float> tempArr;
  (residual(index)).get(tempArr,True);
  IPosition maxpos(tempArr.shape()),minpos(tempArr.shape());
  minMax(minval,maxval, minpos, maxpos,tempArr);
  maxres = maxval;

  /* If we've already converged, return */
  if(maxres < threshold()) 
  {
    os << "Peak residual : " << maxres << " is lower than the user-specified stopping threshold : " << threshold() << LogIO::POST;
      return 1;
  }

  /* If we detect divergence across major cycles, STOP */
  if(fabs( (maxres - previous_maxresidual_p)/previous_maxresidual_p ) > 2.0 )
  {
    os << "Peak residual : " << maxres << " has increased by more than a factor of 2 across major cycles. Could be diverging. Stopping" << LogIO::POST;
    return -1;
  }
  previous_maxresidual_p = maxres;

  /* Find PSF sidelobe level */
  /* abs(min of the PSF) will be treated as the max sidelobe level */
  (PSF(index)).get(tempArr,True);
  minMax(minval,maxval, minpos, maxpos,tempArr);

  fractionOfPsf = min(cycleMaxPsfFraction_p, cycleFactor_p * abs(minval));
  if(fractionOfPsf>0.8) fractionOfPsf=0.8;
  // cyclethreshold = max(threshold(), fractionOfPsf * maxres);

  // if(adbg) 
    {
       os << "Peak Residual : " << maxres  << "  User Threshold : " << threshold() << "  Max PSF Sidelobe : " << abs(minval) <<  " User maxPsfFraction : " << cycleMaxPsfFraction_p  << "  User cyclefactor : " << cycleFactor_p << "  fractionOfPsf = max(maxPsfFraction, PSFsidelobe x cyclefactor) : " << fractionOfPsf << LogIO::POST;
       //    os << "Stopping threshold for this major cycle min(user threshold , fractionOfPsf x Max Residual) : " <<  cyclethreshold  << endl;
    }

  return 0;
}

/***********************************************************************/
/***********************************************************************/
///// Write alpha and beta to disk. Calculate from smoothed model + residuals.
Int WBCleanImageSkyModel::writeResultsToDisk()
{
  if(ntaylor_p<=1) return 0;

  os << "Output Taylor-coefficient images : " << imageNames << LogIO::POST;
//  if(ntaylor_p==2) os << "Calculating Spectral Index" << LogIO::POST;
//  if(ntaylor_p>2) os << "Calculating Spectral Index and Curvature" << LogIO::POST;
  
  PtrBlock<TempLattice<Float>* > smoothed;
  if(ntaylor_p>2) smoothed.resize(3);
  else if(ntaylor_p==2) smoothed.resize(2);

  Quantity bmaj=0.0,bmin=0.0,bpa=0.0;
  Int index=0;
  
  for(Int model=0;model<nfields_p;model++)
  {

    String alphaname,betaname;
    if(  ( (imageNames[0]).substr( (imageNames[0]).length()-3 , 3 ) ).matches("tt0") )
    {
	    alphaname = (imageNames[0]).substr(0,(imageNames[0]).length()-3) + "alpha";
	    betaname = (imageNames[0]).substr(0,(imageNames[0]).length()-3) + "beta";
    }
    else
    {
            alphaname = (imageNames[0]) +  String(".alpha");
            betaname = (imageNames[0]) +  String(".beta");
    }
  
    StokesImageUtil::FitGaussianPSF(PSF(model), bmaj, bmin, bpa);

    /* Create empty alpha image */
    PagedImage<Float> imalpha(image(model).shape(),image(model).coordinates(),alphaname); 
    imalpha.set(0.0);
    

    /* Apply Inverse Hessian to the residuals */
    IPosition gip(4,image(model).shape()[0],image(model).shape()[1],1,1);
    Matrix<Double> invhessian;
    lc_p[model].getinvhessian(invhessian);
    //cout << "Inverse Hessian : " << invhessian << endl;

    Int tindex;
    LatticeExprNode len_p;
    PtrBlock<TempLattice<Float>* > coeffresiduals(ntaylor_p); //,smoothresiduals(ntaylor_p);
    for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
    {
	coeffresiduals[taylor1] = new TempLattice<Float>(gip,memoryMB_p);
    }
    
    /* Apply the inverse Hessian to the residuals */
    for(Int taylor1=0;taylor1<ntaylor_p;taylor1++)
    {
	    len_p = LatticeExprNode(0.0);
	    for(Int taylor2=0;taylor2<ntaylor_p;taylor2++)
	    {
                    tindex = getModelIndex(model,taylor2);
		    len_p = len_p + LatticeExprNode((Float)(invhessian)(taylor1,taylor2)*(residual(tindex)));
	    }
	    (*coeffresiduals[taylor1]).copyData(LatticeExpr<Float>(len_p));
    }
    
    /* Smooth the model images and add the above coefficient residuals */
    for(uInt i=0;i<smoothed.nelements();i++)
    {
	    smoothed[i] = new TempLattice<Float>(gip,memoryMB_p);
  
	    index = getModelIndex(model,i);
	    LatticeExpr<Float> cop(image(index));
	    imalpha.copyData(cop);
	    StokesImageUtil::Convolve(imalpha, bmaj, bmin, bpa);
	    //LatticeExpr<Float> le(imalpha); 
	    LatticeExpr<Float> le(imalpha+( *coeffresiduals[i] )); 
	    (*smoothed[i]).copyData(le);
    }


    /* Create a mask - make this adapt to the signal-to-noise */
    LatticeExprNode leMaxRes=max(residual( getModelIndex(model,0) ));
    Float maxres = leMaxRes.getFloat();
    // Threshold is either 10% of the peak residual (psf sidelobe level) or 
    // user threshold, if deconvolution has gone that deep.
    Float specthreshold = MAX( threshold()*5 , maxres/5.0 );
    os << "Calculating spectral parameters for  Intensity > MAX(threshold*5,peakresidual/5) = " << specthreshold << " Jy/beam" << LogIO::POST;
    LatticeExpr<Float> mask1(iif((*smoothed[0])>(specthreshold),1.0,0.0));
    LatticeExpr<Float> mask0(iif((*smoothed[0])>(specthreshold),0.0,1.0));

    /* Calculate alpha and beta */
    LatticeExpr<Float> alphacalc( ((*smoothed[1])*mask1)/((*smoothed[0])+(mask0)) );
    imalpha.copyData(alphacalc);
    
    ImageInfo ii = imalpha.imageInfo();
    ii.setRestoringBeam(bmaj, bmin, bpa);
    
    imalpha.setImageInfo(ii);
    //imalpha.setUnits(Unit("Spectral Index"));
    imalpha.table().unmarkForDelete();
    os << "Written Spectral Index Image : " << alphaname << LogIO::POST;

    if(ntaylor_p>2)
    {
      PagedImage<Float> imbeta(image(model).shape(),image(model).coordinates(),betaname); 
      imbeta.set(0.0);

      LatticeExpr<Float> betacalc( ((*smoothed[2])*mask1)/((*smoothed[0])+(mask0))-0.5*(imalpha)*(imalpha-1.0) );
      imbeta.copyData(betacalc);
      
      imbeta.setImageInfo(ii);
      //imbeta.setUnits(Unit("Spectral Curvature"));
      imbeta.table().unmarkForDelete();
      os << "Written Spectral Curvature Image : " << betaname << LogIO::POST;
    }

    /* Print out debugging info for center pixel */
    /*
    IPosition cgip(4,512,512,0,0);
    IPosition cgip2(4,490,542,0,0);
    for(Int i=0;i<ntaylor_p;i++)
    {
	    cout << "Extended : " << endl;
	    cout << "Original residual : " << i << " : " << residual( getModelIndex(model,i) ).getAt(cgip) << endl;
	    cout << "Smoothed residual : " << i << " : " << (*smoothresiduals[i]).getAt(cgip) << endl;
	    cout << "Coeff residual : " << i << " : " << (*coeffresiduals[i]).getAt(cgip) << endl;
	    cout << "Point : " << endl;
	    cout << "Original residual : " << i << " : " << residual( getModelIndex(model,i) ).getAt(cgip2) << endl;
	    cout << "Smoothed residual : " << i << " : " << (*smoothresiduals[i]).getAt(cgip2) << endl;
	    cout << "Coeff residual : " << i << " : " << (*coeffresiduals[i]).getAt(cgip2) << endl;
    }
    */
    

    /* Clean up temp arrays */
    for(uInt i=0;i<smoothed.nelements();i++) if(smoothed[i]) delete smoothed[i];
    for(uInt i=0;i<coeffresiduals.nelements();i++) 
    {
	    if(coeffresiduals[i]) delete coeffresiduals[i];
    }

  }// model loop
return 0;
}

/***********************************************************************/
/*************************************
 *          Make Residuals and compute the current peak  
 *************************************/
Bool WBCleanImageSkyModel::solveResiduals(SkyEquation& se, Bool modelToMS) 
{
        makeNewtonRaphsonStep(se,False,modelToMS);
	
	return True;
}
/***********************************************************************/

/*************************************
 *          Make Residuals 
 *************************************/
// Currently - all normalized by ggS from taylor0.
// SAME WITH MAKE_SPECTRAL_PSFS

Bool WBCleanImageSkyModel::makeNewtonRaphsonStep(SkyEquation& se, Bool incremental, Bool modelToMS) 
{
  LogIO os(LogOrigin("WBCleanImageSkyModel", "makeNewtonRaphsonStep"));
  se.gradientsChiSquared(incremental, modelToMS);

  Int index=0;
  for(Int thismodel=0;thismodel<nfields_p;thismodel++) 
  {
    for(Int taylor=0;taylor<ntaylor_p;taylor++)
    {
      /* Normalize by the Taylor 0 weight image */
      index = getModelIndex(thismodel,taylor);
      //LatticeExpr<Float> le(iif(ggS(index)>(0.0), -gS(index)/ggS(index), 0.0));
      LatticeExpr<Float> le(iif(ggS(thismodel)>(0.0), -gS(index)/ggS(thismodel), 0.0));
      residual(index).copyData(le);
      
      //storeAsImg(String("TstRes.")+String::toString(thismodel)+String(".")+String::toString(taylor),residual(index));
    }
  }
  modified_p=False;
  return True;
}

/*************************************
 *          Make Approx PSFs. 
 *************************************/
// The normalization ignores that done in makeSimplePSFs in the Sky Eqn
// and recalculates it from gS and ggS.
Int WBCleanImageSkyModel::makeSpectralPSFs(SkyEquation& se) 
{
  LogIO os(LogOrigin("WBCleanImageSkyModel", "makeSpectralPSFs"));
  if(!donePSF_p)
  {
    //make sure the psf images are made
    for (Int thismodel=0;thismodel<nmodels_p;thismodel++) 
    {
      PSF(thismodel);
    }
  }

  // All 2N-1 psfs will get made here....
  se.makeApproxPSF(psf_p);  

  // Normalize
  Float normfactor=1.0;
  Int index=0;
  for (Int thismodel=0;thismodel<nfields_p;thismodel++) 
  {
     normfactor=1.0;
     for(Int taylor=0;taylor<2*ntaylor_p-1;taylor++)
     {
	/* Normalize by the Taylor 0 weight image */
 	index = getModelIndex(thismodel,taylor);
	//LatticeExpr<Float> le(iif(ggS(index)>(0.0), gS(index)/ggS(index), 0.0));
	LatticeExpr<Float> le(iif(ggS(thismodel)>(0.0), gS(index)/ggS(thismodel), 0.0));
	PSF(index).copyData(le);
	if(taylor==0)
	{ 
	  LatticeExprNode maxPSF=max(PSF(index));
	  normfactor = maxPSF.getFloat();
	  if(adbg) os << "Normalize PSFs for field " << thismodel << " by " << normfactor << LogIO::POST;
	}
	LatticeExpr<Float> lenorm(PSF(index)/normfactor);
	PSF(index).copyData(lenorm);
	LatticeExprNode maxPSF2=max(PSF(index));
        Float maxpsf=maxPSF2.getFloat();
	if(adbg) os << "Psf for Model " << thismodel << " and Taylor " << taylor << " has peak " << maxpsf << LogIO::POST;

	//storeAsImg(String("TstPsf.")+String::toString(thismodel)+String(".")+String::toString(taylor),PSF(index));
     }
     
     index = getModelIndex(thismodel,0);
     beam(thismodel)=0.0;
     if(!StokesImageUtil::FitGaussianPSF(PSF(index),beam(thismodel))) 
        os << "Beam fit failed: using default" << LogIO::POST;
  }
  return 0;
}



/***********************************************************************/

/*************************************
 *          Store a Templattice as an image
 *************************************/
/*
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

Int WBCleanImageSkyModel::storeTLAsImg(String fileName, TempLattice<Complex> &TL, ImageInterface<Float>& theImg)
{
  PagedImage<Complex> tmp(TL.shape(), theImg.coordinates(), fileName);
  LatticeExpr<Complex> le(TL);
  tmp.copyData(le);
  return 0;
}
*/

/**************************
  Resize Work Arrays to calculate extra PSFs.
  (Resizing back to a shorter list must free memory correctly).
  *************************/
Bool WBCleanImageSkyModel::resizeWorkArrays(Int length)
{
   Int originallength = gS_p.nelements();
  
   if(length < originallength) // Clean up extra arrays
   {
     for(Int i = length; i < originallength; ++i)
     {
	if(psf_p[i]){delete psf_p[i]; psf_p[i]=0;}
	if(image_p[i]){delete image_p[i]; image_p[i]=0;}
	if(cimage_p[i]){delete cimage_p[i]; cimage_p[i]=0;}
	if(gS_p[i]){delete gS_p[i]; gS_p[i]=0;}
	if(ggS_p[i]){delete ggS_p[i]; ggS_p[i]=0;}
	if(work_p[i]){delete work_p[i]; work_p[i]=0;}
	if(fluxScale_p[i]){delete fluxScale_p[i]; fluxScale_p[i]=0;}
     }
   }

   psf_p.resize(length,True);
   image_p.resize(length,True);
   cimage_p.resize(length,True);
   gS_p.resize(length,True);
   ggS_p.resize(length,True);
   work_p.resize(length,True);
   fluxScale_p.resize(length,True);
   
   if(length > originallength) // Add extra arrays
   {
     for(Int i = originallength; i < length; ++i)
     {
       psf_p[i]=0;gS_p[i]=0;ggS_p[i]=0;cimage_p[i]=0;work_p[i]=0;fluxScale_p[i]=0;

       Int ind = getFieldIndex(i);
       TempImage<Float>* imptr = 
	 new TempImage<Float> (IPosition(image_p[ind]->ndim(),
				 image_p[ind]->shape()(0),
				 image_p[ind]->shape()(1),
				 image_p[ind]->shape()(2),
				 image_p[ind]->shape()(3)),
			         image_p[ind]->coordinates(), memoryMB_p);
       AlwaysAssert(imptr, AipsError);
       image_p[i] = imptr;
     }
   }
   return True;
}

/************************************************************************************
Check some input parameters and print warnings for the user  
	 fbw = (fmax-fmin)/fref.  
	   if(fbw < 0.1) and nterms>2 
	       => lever-arm may be insufficient for more than alpha.
	       => polynomial fit will work but alpha interpretation may not be ok.
	   if(ref < fmin or ref > fmax) 
	       => polynomial fit will work, but alpha interpretation will not be right.
	   if(nchan==1) or fbw = 0, then ask to use only nterms=1, or choose more than one chan 
	
***********************************************************************************/
Bool WBCleanImageSkyModel::checkParameters()
{
   /* Check ntaylor_p, nrefFrequency_p with min and max freq from the image-coords */
   
   for(Int i=0; i<image(0).coordinates().nCoordinates(); i++)
   {
       if( image(0).coordinates().type(i) == Coordinate::SPECTRAL )
       {
            SpectralCoordinate speccoord(image(0).coordinates().spectralCoordinate(i));
	    Double startfreq=0.0,startpixel=-0.5;
	    Double endfreq=0.0,endpixel=+0.5;
	    speccoord.toWorld(startfreq,startpixel);
	    speccoord.toWorld(endfreq,endpixel);
	    Float fbw = (endfreq - startfreq)/refFrequency_p;
	    //cout << "Freq range of the mfs channel : " << startfreq << " -> " << endfreq << endl;
	    //cout << "Fractional bandwidth : " << fbw << endl;
	     
	    os << "Fractional Bandwidth : " << fbw*100 << " %." << endl;

	    if(fbw < 0.1 && ntaylor_p == 2 )
	        os << "Fractional Bandwidth is " << fbw*100 << " %. Please check that the flux variation across the chosen frequency range (" << startfreq << " Hz to " << endfreq << " Hz) is at least twice the single-channel noise-level. If not, please use nterms=1." << LogIO::WARN << LogIO::POST; 
	    
	    if(fbw < 0.1 && ntaylor_p > 2)
	        os << "Fractional Bandwidth is " << fbw*100 << " %. Please check that (a) the flux variation across the chosen frequency range (" << startfreq << " Hz to " << endfreq << " Hz) is at least twice the single-channel noise-level, and (b) a " << ntaylor_p << "-term Taylor-polynomial fit across this frequency range is appropriate. " << LogIO::WARN << LogIO::POST; 

	    if(refFrequency_p < startfreq || refFrequency_p > endfreq)
	        os << "A Reference frequency of " << refFrequency_p << "Hz is outside the frequency range of the selected data (" << startfreq << " Hz to " << endfreq << " Hz). A power-law interpretation of the resulting Taylor-coefficients may not be accurate." << LogIO::WARN << LogIO::POST;

       }
   }

   
   return True;
}


} //# NAMESPACE CASA - END


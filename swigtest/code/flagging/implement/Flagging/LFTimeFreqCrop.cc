///# LFTimeFreqCrop.cc: this defines a light autoflagger 
//# Copyright (C) 2000,2001,2002
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
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/BasicSL/Complex.h>
#include <casa/OS/HostInfo.h>
#include <casa/stdio.h>
#include <casa/math.h>
#include <stdarg.h>

#include <flagging/Flagging/LFTimeFreqCrop.h>

#include <sstream>

namespace casa {
  
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
  
  
  // -----------------------------------------------------------------------
  // Default Constructor
  // -----------------------------------------------------------------------
  LFTimeFreqCrop::LFTimeFreqCrop () : LFBase()
  {
     dbg=True;

     // Set default parameters
     Record defrec;
     setParameters(defrec);
  }
  
  LFTimeFreqCrop::~LFTimeFreqCrop ()
  {
    //    cout << "TFC destructor" << endl;  
  }
  
 
  Bool LFTimeFreqCrop::setParameters(Record &parameters)
  {
    /* Create an agent record */
    Record fullrec;
    fullrec = getParameters();
    fullrec.merge(parameters,Record::OverwriteDuplicates);

    //    cout << fullrec << endl;    

    /* Read values into local variables */
    T_TOL = fullrec.asDouble("time_amp_cutoff");
    F_TOL = fullrec.asDouble("freq_amp_cutoff");
    MaxNPieces = fullrec.asInt("maxnpieces");
    intSelCorr = fullrec.asArrayInt("corrs");
    
    timeFitType_p = fullrec.asString("timefit");
    freqFitType_p = fullrec.asString("freqfit");
    flagDimension_p = fullrec.asString("flagdimension");
    halfWin_p = fullrec.asInt("halfwin");
    winStats_p = fullrec.asString("usewindowstats");

    MaxDeg=4;
    
    return True;
  }
  
  
  Record LFTimeFreqCrop::getParameters()
  {
    Record rec;
    if( !rec.nfields() )
      {
        rec.define("algorithm","tfcrop");
	rec.define("time_amp_cutoff",3);
	rec.define("freq_amp_cutoff",3);
	rec.define("maxnpieces",6);
	
        rec.define("timefit","line");
        rec.define("freqfit","poly");
        rec.define("flagdimension","freqtime");
        rec.define("halfwin",1);
	rec.define("usewindowstats","none");

        rec.define("corrs",Array<Int>());
      }
    return rec;
  }
  

  /********************************* TFCROP *****************************/
  
  void LFTimeFreqCrop :: AllocateMemory()
  {
    IPosition shpVis = visc.shape();
    //cout << "Shape of visc : " << shpVis << endl;
    
    IPosition shpFlag = flagc.shape();
    //cout << "Shape of flagc : " << shpFlag << endl;
    
    /* Cube to hold CLEAN bandpasses : POLZN x IFR x CHAN */
    cleanBP.resize(NumP,NumB,NumC);
    cleanTS.resize(NumP,NumB,NumT);
    cleanBP=0; cleanTS=0;
    
    /* Temporary workspace vectors */	
    tempBP.resize(NumC);tempTS.resize(NumT);
    flagBP.resize(NumC);flagTS.resize(NumT);
    fitBP.resize(NumC);fitTS.resize(NumT);
    tempBP=0;tempTS=0;flagBP=False;flagTS=False;fitBP=0;fitTS=0;
  }
  
  
  /* Run the TFCROP algorithm */
  /* Openmp on baselines... */
  Bool LFTimeFreqCrop :: runMethod(const VisBuffer &inVb, Cube<Float> &inVisc, 
				   Cube<Bool> &inFlagc, Cube<Bool> &inPreFlagc, 
				   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP)
  {
    // Initialize all the shape information, and make a references for visc and flagc.
    LFBase::runMethod(inVb, inVisc, inFlagc, inPreFlagc, numT, numAnt, numB, numC, numP);

    // Allocate some temp buffers (all 1D)
    AllocateMemory();

    // Setup the correlation selection
    if(intSelCorr.nelements()==0 || intSelCorr.nelements()!=NumP) 
      {
	intSelCorr.resize(NumP); intSelCorr=1; 
	//cout << "Selecting All Correlation Pairs" << endl; 
      }

    uInt a1,a2;
    //    meanBP = 0;
    cleanBP = 0;
    
    for(uInt pl=0;pl<NumP;pl++)
      {
        // If this pol is not to be flagged-on, skip it.
	if( !intSelCorr[pl] ) continue;

	uInt bs;
	
	for(bs=0;bs<NumB;bs++)
	  {
	    Ants(bs,&a1,&a2);
	    if( (a1 != a2) &&  baselineFlag[bs]==True)  
	      {
		/*		
                if(flagDimension_p == String("time") || flagDimension_p == String("both") )
		  {
		    //		    FlagTimeSeries(pl,bs,"line");    
		    //	    FitBaseAndFlag(pl,bs,String("line"),String("time"),cleanTS);
		  }
		*/

		if(flagDimension_p == String("time"))
		  {
		    FitBaseAndFlag(pl,bs,timeFitType_p,String("time"),cleanTS);
		  }
		else if( flagDimension_p == String("freq") )
		  {
		    FitBaseAndFlag(pl,bs,freqFitType_p,String("freq"),cleanBP);
		  }
		else if( flagDimension_p == String("timefreq") )
		  {
		    FitBaseAndFlag(pl,bs,timeFitType_p,String("time"),cleanTS);
		    FitBaseAndFlag(pl,bs,freqFitType_p,String("freq"),cleanBP);
		  }
		else // freqtime
		  {
		    FitBaseAndFlag(pl,bs,freqFitType_p,String("freq"),cleanBP);
		    FitBaseAndFlag(pl,bs,timeFitType_p,String("time"),cleanTS);
		  }
	      }// if baseline exists
	  }//for bs
      }//for pl
    
    return True;   
  }// End of RunTFCrop()
  
  
  
  // FitBaseAndFlag
  // Read data and flags, and fill in cleanBP(pl,bs,ch)  or cleanTS(pl,bs,tm)
  void LFTimeFreqCrop :: FitBaseAndFlag(uInt pl, uInt bs, String fittype, String direction, Cube<Float> &cleanArr)
  {    
    
    Float mn=0,sd=0,tpsd=0.0,tpsum=0.0;
    uInt a1,a2;
    Bool flg=True;
    Float mval=0.0;
    Int mcnt=0;
    
    IPosition cubeshp = cleanArr.shape();
    Vector<Float> tempDat,tempFit;
    Vector<Bool> tempFlag;
    Vector<Int> mind(2);
    Float tol=F_TOL;

    // Decide which direction to fit the base polynomial
    if(direction==String("freq"))
      { 
	AlwaysAssert(cubeshp==IPosition(3,NumP,NumB,NumC), AipsError);
	mind[0]=NumC;  mind[1]=NumT;
      }
    else if(direction==String("time") )
      { 
	AlwaysAssert(cubeshp==IPosition(3,NumP,NumB,NumT), AipsError);
	mind[0]=NumT;  mind[1]=NumC;
	tol=T_TOL;
      }

    // Resize temp arrays
    tempDat.resize(mind[0]); 
    tempFit.resize(mind[0]); 
    tempFlag.resize(mind[0]);
    tempDat=0.0;
    tempFit=0.0;
    tempFlag=False;

    // Get a1,a2 indices from the baseline count
    Ants(bs,&a1,&a2);

    // For each element in the dominant direction (axis0)
    //   calculate the mean across the other direction (axis1).
    for(int i0=0;i0<mind[0];i0++)	
      {
	// Calc the mean across axes1 (with flags)
	mval=0.0;mcnt=0;
	for(uInt i1=0;i1<(uInt) mind[1];i1++)
	  {
	    if(mind[0]==(Int) NumC)// if i0 is channel, and i1 is time
	      {
		if(!flagc(pl,i0,((i1*NumB)+bs)))
		  {
		    mval += visc(pl,i0,((i1*NumB)+bs));
		    mcnt++;
		  }
	      }
	    else // if i1 is channel, and i0 is time
	      {
		if(!flagc(pl,i1,((i0*NumB)+bs)))
		  {
		    mval += visc(pl,i1,((i0*NumB)+bs));
		    mcnt++;
		  }
	      }
	  }//for i1
	// Fill in the mean value across axis1 for i0
	tempDat[i0] = mcnt ? mval/mcnt : 0.0;
	tempFlag[i0] = mcnt ? False : True;
	if(! tempFlag[i0] ) flg=False;
      }// for i0

    // Fit a piece-wise polynomial across axis0 (or do a line)
    if(flg==False)
      {
	/* Piecewise Poly Fit*/
	if(fittype == String("poly"))
	  {
	    FitPiecewisePoly(tempDat,tempFlag,tempFit);	
	  }
	else
	  {
	    /* LineFit to flag off a line fit in frequency */
	    //flagBP=False;
	    for(uInt i0=0;i0<(uInt) mind[0];i0++)
	      if(tempDat[i0]==0) tempFlag[i0]=True;
	    Int maxnp = MaxNPieces;
	    MaxNPieces=1;
	    MaxDeg=1;
	    FitPiecewisePoly(tempDat,tempFlag,tempFit);	
	    MaxNPieces = maxnp;
	    MaxDeg=4;
	    //LineFit(tempDat,tempFlag,tempFit,0,mind[0]-1);	
	  }
      }// fit poly

    // Store the fit for later use.
    for(int i0=0;i0<mind[0];i0++)
      {
	if(flg==False) cleanArr(pl,bs,i0)= tempFit[i0];
	else cleanArr(pl,bs,i0)=0;
      }

    //      Float avgsd = sqrt( calcVar(tempDat,tempFlag,tempFit) );
    //	    if(mind[0]==NumC) cout << "  Stddev : "  << avgsd << endl;



    // Now, iterate through the data again and flag.
    // This time, iterate in reverse order : for each i1, get all i0
    for(uInt i1=0;i1<(uInt) mind[1];i1++)
      {
	/* Divide (or subtract) out the clean bandpass */
	tempDat=0,tempFlag=0;
	for(int i0=0;i0<mind[0];i0++)
	  {
	    if(mind[0]==(Int) NumC)// if i0 is channel, and i1 is time
	      {
		tempFlag[i0] = flagc(pl,i0,((i1*NumB)+bs));
		if(tempFlag[i0]==False) tempDat[i0] = visc(pl,i0,((i1*NumB)+bs))/cleanArr(pl,bs,i0);
		//if(tempFlag[i0]==False) tempDat[i0] = ( visc(pl,i0,((i1*NumB)+bs)) - cleanArr(pl,bs,i0) );
		//if(tempFlag[i0]==False) tempDat[i0] = ( visc(pl,i0,((i1*NumB)+bs)) );
	      }
	    else // if i1 is channel, i0 is time
	      {
		tempFlag[i0] = flagc(pl,i1,((i0*NumB)+bs));
		if(tempFlag[i0]==False) tempDat[i0] = visc(pl,i1,((i0*NumB)+bs))/cleanArr(pl,bs,i0);
		//if(tempFlag[i0]==False) tempDat[i0] = ( visc(pl,i1,((i0*NumB)+bs)) -cleanArr(pl,bs,i0) );
		//if(tempFlag[i0]==False) tempDat[i0] = ( visc(pl,i1,((i0*NumB)+bs)) );
	      }
	  }//for i0
	

	//Flag outliers based on absolute deviation from the model
	// Do this as a robust fit
	Float temp=0;
	for(Int loop=0;loop<5;loop++)
	  {
	    mn=1;
	    //mn=0.0;

	    sd = UStd(tempDat,tempFlag,mn);
	    // sd = UStd(tempDat,tempFlag,tempFit);
	    
	    // Flag if the data differs from 1 by N sd
	    for(Int i0=0;i0<mind[0];i0++)
	      {
		if(tempFlag[i0]==False && fabs(tempDat[i0]-mn) > tol*sd) tempFlag[i0]=True ;
	      }


	if(halfWin_p>0 && (winStats_p != "none") )
	  {
	    for(Int i0=halfWin_p;i0<mind[0]-halfWin_p;i0++)
	      {
		tpsum=0.0;tpsd=0.0;
		// Flag if sum of N points crosses a lower threshold
		if(winStats_p=="sum" || winStats_p=="both")
		  {
		    for(Int i=i0-halfWin_p; i<i0+halfWin_p+1; i++) tpsum += fabs(tempDat[i]-mn);
		    if(tpsum/(2*halfWin_p+1.0) > tol*sd/1.5 ) tempFlag[i0]=True;
		  }
		
		
		// Flag if the N point std is larger then N sd
		if(winStats_p=="std" || winStats_p=="both")
		  {
		    for(Int i=i0-halfWin_p; i<i0+halfWin_p+1; i++) tpsd += (tempDat[i]-mn) * (tempDat[i]-mn) ;
		    if(sqrt( tpsd / (2*halfWin_p+1.0) ) > tol*sd/1.5)  tempFlag[i0]=True ;
		  }
		
	      }//for i0
	  }// if winStatr += none
	

	    if(fabs(temp-sd) < 0.1)break;
	    temp=sd;
	  }//for loop
	
	
	/* Fill the flags into the visbuffer array */
	for(Int i0=0;i0<mind[0];i0++)
	  {
	    if(mind[0]==(Int) NumC)// if i0 is channel, and i1 is time
	      {
		flagc(pl,i0,((i1*NumB)+bs))=tempFlag[i0];
	      }
	    else //if i1 is channel, and i0 is time
	      {
		flagc(pl,i1,((i0*NumB)+bs))=tempFlag[i0];
	      }
	  }
	
      }//for i1
    
    
    
  }// end FitBaseAndFlag
  
  
  
  /* Calculate the MEAN of 'vect' ignoring values flagged in 'flag' */
  Float LFTimeFreqCrop :: UMean(Vector<Float> vect, Vector<Bool> flag)
  {
    Float mean=0;
    int cnt=0;
    for(int i=0;i<(int)vect.nelements();i++)
      if(flag[i]==False)
	{
	  mean += vect[i];
	  cnt++;
	}
    if(cnt==0) cnt=1;
    return mean/cnt;
  }
  
  
  /* Calculate the variance of 'vect' w.r.to a given 'fit' 
   * ignoring values flagged in 'flag' 
   * Use  median ( data - median(data) )   as the estimator of variance
   */
  Float LFTimeFreqCrop :: calcVar(Vector<Float> vect, Vector<Bool> flag, Vector<Float> fit)
  {
    int n=0,cnt=0;
    n = vect.nelements() < fit.nelements() ? vect.nelements() : fit.nelements();
    for(int i=0;i<n;i++)
      {
	if(flag[i]==False)cnt++;
      }
    
    Vector<Float> validvals(cnt);
    cnt=0;
    for(int i=0;i<n;i++)
      {
	if(flag[i]==False)
	  {
	    validvals[cnt] = fit[i] < 1e-6 ? 0.0 : fabs( (vect[i] - fit[i])/fit[i] );
	    cnt++;
	  }
      }
    
    Float med=0.0;
    
    if(validvals.nelements())
      {
	med = median(validvals,False);
	
	//cout << "validvals : " << validvals << endl;
	//cout << "median : " << med << endl;
	
	for(int i=0;i<(int) validvals.nelements();i++)
	  validvals[i] = fabs( validvals[i]-med );
	
	med = median(validvals,False);
	//cout << "median(data-median(data)) : " << med << endl;
      }
    
    return med;
  }
  
  
  
  /* Calculate the STANDARD DEVN. of 'vect' w.r.to a given 'fit' 
   * ignoring values flagged in 'flag' */
  Float LFTimeFreqCrop :: UStd(Vector<Float> vect, Vector<Bool> flag, Vector<Float> fit)
  {
    Float std=0;
    int n=0,cnt=0;
    n = vect.nelements() < fit.nelements() ? vect.nelements() : fit.nelements();
    for(int i=0;i<n;i++)
      if(flag[i]==False)
	{
	  cnt++;
	  std += (vect[i]-fit[i])*(vect[i]-fit[i]);
	}
    if(cnt==0) cnt=1;
    return sqrt(std/cnt);
  }
  
  
  /* Calculate the STANDARD DEVN. of 'vect' w.r.to a given mean 
   * ignoring values flagged in 'flag' */
  Float LFTimeFreqCrop :: UStd(Vector<Float> vect, Vector<Bool> flag, Float mean)
  {
    Float std=0;
    int cnt=0;
    for(int i=0;i<(int)vect.nelements();i++)
      if(flag[i]==False)
	{
	  cnt++;
	  std += (vect[i]-mean)*(vect[i]-mean);
	}
    return sqrt(std/cnt);
  }
  
  /* Fit Piecewise polynomials to 'data' and get the 'fit' */
  void LFTimeFreqCrop :: FitPiecewisePoly(Vector<Float> data, Vector<Bool> flag, Vector<Float> fit)
  {
    //    Int step=0,ind=0;
    Int deg=0;
    Int left=0,right=0;
    //  Int le=0,ri=0;
    Float sd,TOL=3;
    Vector<Float> tdata;
    //Vector<Bool> tfband;
    
    //tfband.resize(flag.nelements());
    tdata.resize(data.nelements());
    tdata = data;
    
    AlwaysAssert(data.nelements()==flag.nelements(), AipsError);    
    
    /* replace empty data values by adjacent values */
    for(uInt i=0;i<tdata.nelements();i++)
      {
	if(tdata[i]==0)
	  {
	    if(i==0)// find first non-zero value and set to that.
	      {
		Int ind=0;
		for(uInt j=1;j<tdata.nelements();j++)
		  if(tdata[j]!=0){ind=j;break;}
		if(ind==0) tdata[i]=0;
		else tdata[i]=tdata[ind];
	      }
	    else// find next non-zero value and interpolate.
	      {
		Int indr=0;
		for(uInt j=i+1;j<tdata.nelements();j++)
		  if(tdata[j]!=0){indr=j;break;}
		Int indl=-1;
		for(int j = i ; j >= 0 ; j--)
		  if(tdata[j]!=0){indl=j;break;}
		
		if(indl==-1 && indr==0) tdata[i]=0;
		if(indl>-1 && indr==0) tdata[i]=tdata[indl];
		if(indl==-1 && indr>0) tdata[i]=tdata[indr];
		if(indl>-1 && indr>0) tdata[i]=(tdata[indl]+tdata[indr])/2.0;
	      }
	  }
      }
    
    
    /* If there still are empty points (entire spectrum is flagged) flag it. */
    for(uInt i=0;i<tdata.nelements();i++)
      if(tdata[i]==0) 
	{
	  //cout << "chan " << i << " is blank" << endl;
	  flag[i]=True;
	}
    
    fit = tdata;
    
    Int psize=1;
    Int leftover=1,leftover_back=0,leftover_front=0,npieces=1;
    
    deg=1;
    npieces=1;
    
    for(uInt j=0;j<=4;j++)
      {
	//     if(j==0) {deg = 1;npieces=1;}
	//     if(j==1) {deg = 1;npieces=5;}
	//     if(j==2) {deg = 2;npieces=6;}
	//     if(j==3) {deg = 3;npieces=7;}
	//     if(j==4) {deg = 3;npieces=8;}
	
	npieces = MIN(2*j+1, (uInt) MaxNPieces);
	if(j>1) {deg=2;}
	if(j>2) {deg=3;}
	deg = MIN(deg,MaxDeg);
	
	psize = (int)(tdata.nelements()/npieces);
	//     cout << "Iter : " << j << " with Deg : " << deg << " and Piece-size : " << psize << endl;
	
	leftover = (int)(tdata.nelements() % npieces);
	
	leftover_front = (int)(leftover/2.0);
	leftover_back = leftover - leftover_front;
	
	left=0; right=tdata.nelements()-1;
	for(Int p=0;p<npieces;p++)
	  {
	    if(npieces>1)
	      {
		left = leftover_front + p*psize;
		right = left + psize; 
		
		if(p==0) {left = 0; right = leftover_front + psize;}
		if(p==npieces-1) {right = tdata.nelements()-1;} 
	      }
	    if(deg==1) 
	      LineFit(tdata,flag,fit,left,right);
	    else 
	      PolyFit(tdata,flag,fit,left,right,deg);
	  }
	
	/* Now, smooth the fit - make this nicer later */
	
	int winstart=0,winend=0;
	float winsum=0.0;
	int offset=2;
	for(uInt i=offset;i<tdata.nelements()-offset;i++)
	  {
	    winstart = i-offset;
	    winend = i+offset;
	    if(winstart<0)winstart=0;
	    if(winend>=(uInt) tdata.nelements())winend=tdata.nelements()-1;
	    if(winend <= winstart) break;
	    winsum=0.0;
	    for(uInt wi=winstart;wi<=(uInt) winend;++wi)
	      winsum += fit[wi];
	    fit[i] = winsum/(winend-winstart+1);
	  }
	
	
	/* Calculate the STD of the fit */
	sd = UStd(tdata,flag,fit);
	if(j>=2)  TOL=2;
	else TOL=3;
	
	
	/* Detect outliers */
	for(uInt i=0;i<tdata.nelements();i++)
	  {
	    if(tdata[i]-fit[i] > TOL*sd) 
	      flag[i]=True;
	  }
	
      } // for j
    
  } // end of FitPiecewisePoly
  
  
  
    /* Fit a polynomial to 'data' from lim1 to lim2, of given degree 'deg', 
     * taking care of flags in 'flag', and returning the fitted values in 'fit' */
  void LFTimeFreqCrop :: PolyFit(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2,uInt deg)
  {
    static Vector<Double> x;
    static Vector<Double> y;
    static Vector<Double> sig;
    static Vector<Double> solution;
    
    uInt cnt=0;
    for(uInt i=lim1;i<=lim2;i++)
      if(flag[i]==False) cnt++;
    
    if(cnt <= deg)
      {
	LineFit(data,flag,fit,lim1,lim2);
	return;
      }
    
    
    LinearFit<Double> fitter;
    Polynomial<AutoDiff<Double> > combination(deg);
    
    
    combination.setCoefficient(0,0.0);
    if (deg >= 1) combination.setCoefficient(1, 0.0);
    if (deg >= 2) combination.setCoefficient(2, 0.0);
    if (deg >= 3) combination.setCoefficient(3, 0.0);
    if (deg >= 4) combination.setCoefficient(4, 0.0);
    
    x.resize(lim2-lim1+1);
    y.resize(lim2-lim1+1);
    sig.resize(lim2-lim1+1);
    solution.resize(deg+1);
    
    for(uInt i=lim1;i<=lim2;i++)
      {
	x[i-lim1] = i+1;
	y[i-lim1] = data[i];
	sig[i-lim1] = (flag[i]==True)?0:1;
      }
    
    fitter.asWeight(True);
    
    fitter.setFunction(combination);
    solution = fitter.fit(x,y,sig);
    
    for(uInt i=lim1;i<=lim2;i++)
      {
	fit[i]=0;
	for(uInt j=0;j<deg+1;j++)
	  fit[i] += solution[j]*pow((double)(x[i-lim1]),(double)j);
      }
    
  }
  
  
  
  /* Fit a LINE to 'data' from lim1 to lim2, taking care of flags in 
   * 'flag', and returning the fitted values in 'fit' */
  void LFTimeFreqCrop :: LineFit(Vector<Float> data, Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2)
  {
    float Sx = 0, Sy = 0, Sxx = 0, Sxy = 0, S = 0, a, b, sd, mn;
    
    mn = UMean(data, flag);
    sd = UStd (data, flag, mn);
    
    for (uInt i = lim1; i <= lim2; i++)
      {
	if (flag[i] == False) // if unflagged
	  {
	    S += 1 / (sd * sd);
	    Sx += i / (sd * sd);
	    Sy += data[i] / (sd * sd);
	    Sxx += (i * i) / (sd * sd);
	    Sxy += (i * data[i]) / (sd * sd);
	  }
      }
    a = (Sxx * Sy - Sx * Sxy) / (S * Sxx - Sx * Sx);
    b = (S * Sxy - Sx * Sy) / (S * Sxx - Sx * Sx);
    
    for (uInt i = lim1; i <= lim2; i++)
      fit[i] = a + b * i;
    
  }
  
  Bool LFTimeFreqCrop::getMonitorSpectrum(Vector<Float> &monspec, uInt pl, uInt bs)
  {
    AlwaysAssert(monspec.nelements()==NumC, AipsError);
    Float sd;
    sd=0.0; 
    
    /*
      for(uInt tm=0;tm<NumT;tm++)
      {
      tempBP=0.0;
      for(int ch=0;ch<NumC;ch++)
      {
      flagBP[ch] = flagc(pl,ch,((tm*NumB)+bs));
      if(flagBP[ch]==False )
      {
      tempBP[ch] = visc(pl,ch,((tm*NumB)+bs))/cleanBP(pl,bs,ch);
      //tempBP[ch] = fabs( visc(pl,ch,((tm*NumB)+bs)) - cleanBP(pl,bs,ch) );
      }
      
      }
      sd += UStd(tempBP,flagBP,1);
      }
      
      sd /= NumT;
    */
    
    for(int ch=0;ch<(int) NumC;ch++)
      monspec[ch] = cleanBP(pl,bs,ch);;
    
    return True;
  }
  
} //#end casa namespace

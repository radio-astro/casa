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
    cout << "TFC destructor" << endl;  
  }
  
 
  Bool LFTimeFreqCrop::setParameters(Record &parameters)
  {
    /* Create an agent record */
    Record fullrec;
    fullrec = getParameters();
    fullrec.merge(parameters,Record::OverwriteDuplicates);

    cout << fullrec << endl;    

    /* Read values into local variables */
    ANT_TOL = fullrec.asDouble("ant_cutoff");
    BASELN_TOL = fullrec.asDouble("baseline_cutoff");
    T_TOL = fullrec.asDouble("time_amp_cutoff");
    F_TOL = fullrec.asDouble("freq_amp_cutoff");
    FlagLevel = fullrec.asInt("flag_level");
    NumTime = fullrec.asInt("ntime_sec");
    ShowPlots = fullrec.asBool("showplots");
    FreqLineFit = fullrec.asBool("freqlinefit");
    MaxNPieces = fullrec.asInt("maxnpieces");
    WriteFlagsToMS = fullrec.asBool("writeflags");
    Expr = fullrec.asString("expr");
    Column = fullrec.asString("datacolumn");
    intSelCorr = fullrec.asArrayInt("corrs");
    
    timeFitType_p = fullrec.asString("timefit");
    freqFitType_p = fullrec.asString("freqfit");
    flagDimension_p = fullrec.asString("flagdimension");
    
    return True;
  }
  
  
  Record LFTimeFreqCrop::getParameters()
  {
    Record rec;
    if( !rec.nfields() )
      {
        rec.define("algorithm","tfcrop");
	rec.define("datacolumn","DATA");
	rec.define("ant_cutoff",0);
	rec.define("baseline_cutoff",0);
	rec.define("time_amp_cutoff",3);
	rec.define("freq_amp_cutoff",3);
	rec.define("flag_level",1);
	rec.define("auto_cross",1);
	rec.define("ntime_sec",90);
	rec.define("column","DATA");
	rec.define("expr","ABS I");
	rec.define("fignore",False);
	rec.define("showplots",False);
	rec.define("freqlinefit",False);
	rec.define("maxnpieces",6);
	
        rec.define("writeflags",False);

        rec.define("timefit","line");
        rec.define("freqfit","poly");
        rec.define("flagdimension","both");

        rec.define("corrs",Array<Int>());
      }
    return rec;
  }
  

  /********************************* TFCROP *****************************/
  
  void LFTimeFreqCrop :: AllocateMemory()
  {
    ///// Only check that all shapes are consistent.

    IPosition shpVis = visc.shape();
    //cout << "Shape of visc : " << shpVis << endl;
    
    IPosition shpFlag = flagc.shape();
    //cout << "Shape of flagc : " << shpFlag << endl;
    
    /* Cube to hold visibility amplitudes : POLZN x CHAN x (IFR*TIME) */
    //visc.resize(NumP,NumC,NumB*NumT);
    //visc=0;
    
    /* Cube to hold visibility flags : POLZN x CHAN x (IFR*TIME) */
    //flagc.resize(NumP,NumC,NumB*NumT);
    //flagc=False;
    // 	cout << " CubeShape = " << cubepos << endl;
    
    /* Cube to hold MEAN bandpasses : POLZN x IFR x CHAN */
    /* Cube to hold CLEAN bandpasses : POLZN x IFR x CHAN */
    //	meanBP.resize(NumP,NumB,NumC);
	cleanBP.resize(NumP,NumB,NumC);
	cleanTS.resize(NumP,NumB,NumT);
    
	//    meanBP=0;
    cleanBP=0;
    cleanTS=0;
    
    /* Matrix to hold Row Flags : POLZX x (IFR*TIME) */
    // UUU : check - this should also be NumT
    //	RowFlags.resize((uInt)NumP,(uInt)(NumB*NumT));
    //	RowFlags=False;
    //cout << "RowFlags = " << RowFlags.shape() << endl;
    
    
    /* Temporary workspace vectors */	
    tempBP.resize(NumC);tempTS.resize(NumT);
    flagBP.resize(NumC);flagTS.resize(NumT);
    fitBP.resize(NumC);fitTS.resize(NumT);
    
    tempBP=0;tempTS=0;flagBP=False;flagTS=False;fitBP=0;fitTS=0;
    
    
  }
  
  
  /* Run the TFCROP algorithm */
  /* Openmp on baselines... */
  Bool LFTimeFreqCrop :: runMethod(Cube<Float> &inVisc, Cube<Bool> &inFlagc, 
				   uInt numT, uInt numAnt, uInt numB, uInt numC, uInt numP)
  {
    // Initialize all the shape information, and make a references for visc and flagc.
    LFBase::runMethod(inVisc, inFlagc, numT, numAnt, numB, numC, numP);

    // Allocate some temp buffers (all 1D)
    AllocateMemory();

    // Setup the correlation selection
    if(intSelCorr.nelements()==0 || intSelCorr.nelements()!=NumP) 
      {intSelCorr.resize(NumP); intSelCorr=1; cout << "Selecting All Correlation Pairs" << endl; }

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
	    if(a1 != a2) 
	      {
		
                if(flagDimension_p == String("time") || flagDimension_p == String("both") )
		  {
		    FlagTimeSeries(pl,bs,"line");    
		  }
		
   		if(flagDimension_p == String("freq") || flagDimension_p == String("both") )
		  {
		    FitCleanBandPass(pl,bs,freqFitType_p);
		    FlagBandPass(pl,bs);
		  }

		if(flagDimension_p == String("time") || flagDimension_p == String("both") )
		  {
		    FitCleanTimeSeries(pl,bs,timeFitType_p);
		    FlagTimeSeriesAgain(pl,bs);    
		  }

		
	      }// corrchoice
	  }//for bs
      }//for pl
    
    return True;   
  }// End of RunTFCrop()
  
  
  
  
  
  /* Flag in time, and build the average bandpass */
  /* Grow flags by one timestep, check for complete flagged baselines. */
  void LFTimeFreqCrop :: FlagTimeSeries(uInt pl, uInt bs, String fittype)
  {
    Float mn=0,sd=0,temp=0,tol=0;
    uInt a1,a2;
    Bool flg=False;

    // Create an average time-series. Average across channels    


/* For each Channel - fit lines to 1-D data in time - flag according 
     * to them and build up the mean bandpass */
    
    Float rmean=0;
    //			cout << " Antennas : " << a1 << " & " << a2 << endl;
    for(int ch=0;ch<NumC;ch++)
      {
	tempTS=0;flagTS=False;
	for(uInt tm=0;tm<NumT;tm++)
	  {
	    tempTS[tm] = visc(pl,ch,((tm*NumB)+bs));
	    flagTS[tm] = flagc(pl,ch,((tm*NumB)+bs));
	  }//for tm
	
	
	rmean += UMean(tempTS,flagTS);
	
	temp=0;
	for(int loop=0;loop<5;loop++)
	  {
	    
            if(fittype==String("poly")) FitPiecewisePoly(tempTS,flagTS,fitTS);
	    else LineFit(tempTS,flagTS,fitTS,0,tempTS.nelements()-1);	
	    
	    sd = UStd(tempTS,flagTS,fitTS);
	    
	    for(uInt i=0;i<NumT;i++)
	      if(flagTS[i]==False && fabs(tempTS[i]-fitTS[i]) > T_TOL*sd)
		{
		  flagTS[i]=True ;
		}
	    if(fabs(temp-sd) < 0.1)break;
	    temp=sd;
	  }
	
	// If sum of 2 adjacent flags also crosses threshold, flag 
	/*
	  for(uInt i=1;i<NumT-1;i++)
	  {
	  if(flagTS[i])
	  {
	  if( ( fabs(tempTS[i-1]-fitTS[i-1]) + fabs(tempTS[i+1]-fitTS[i+1]) ) > T_TOL*sd )
	  {flagTS[i-1]=True; flagTS[i+1]=True;}
	  }
	  }
	*/
	
	//	meanBP(pl,bs,ch) = UMean(tempTS,flagTS) ;
	
	/* write flags to local flag cube */
	for(uInt tm=0;tm<NumT;tm++)
	  flagc(pl,ch,((tm*NumB)+bs))=flagTS[tm];
	
      }//for ch
    
    /* Check for completely flagged ants/bs */
    // UUU : DOES NOT WORK. rmean is often ZERO for no reason.
    if(0)
      {
	if(a1 != a2) 
	  {
	    tol=BASELN_TOL;
	    if(fabs(rmean/float(NumC)) < tol)
	      {
		for(int ch=0;ch<NumC;ch++)
		  for(uInt tm=0;tm<NumT;tm++)
		    flagc(pl,ch,((tm*NumB)+bs))=True;
		cout << "Mean : " << rmean/NumC << " : Baseline Flagged : " << a1 << ":" << a2 << endl;
	      }
	    
	  }
      }///if(0);		
    
  }    
  
  /* Fit a smooth bandpass to the mean bandpass and store it 
   *  one for each baseline */
  
  // matpos  :  NumP. NumB. NumC
  
  void LFTimeFreqCrop :: FitCleanBandPass(uInt pl, uInt bs, String fittype)
  {    
    
    Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
    uInt a1,a2;
    Bool flg=False;
    
    Ants(bs,&a1,&a2);
    /* Fit a smooth bandpass */
    flg=True;
    for(int ch=0;ch<NumC;ch++)	
      {
	//tempBP[ch] = meanBP(pl,bs,ch);

	// Calc the mean across time (with flags), and assign to tempBP
	tempTS=0;flagTS=False;
	for(uInt tm=0;tm<NumT;tm++)
	  {
	    tempTS[tm] = visc(pl,ch,((tm*NumB)+bs));
	    flagTS[tm] = flagc(pl,ch,((tm*NumB)+bs));
	  }//for tm
	tempBP[ch] = UMean(tempTS,flagTS) ;

	if(tempBP[ch] != 0) flg=False;
      }
    
    if(flg==False)
      {
	/* Piecewise Poly Fit to the meanBP */
	//	if(!FreqLineFit)
	if(fittype == String("poly"))
	  {
	    FitPiecewisePoly(tempBP,flagBP,fitBP);	
	  }
	else
	  {
	    /* LineFit to flag off a line fit in frequency */
	    flagBP=False;
	    for(uInt ch=0;ch<tempBP.nelements();ch++)
	      if(tempBP[ch]==0) flagBP[ch]=True;
	    LineFit(tempBP,flagBP,fitBP,0,tempBP.nelements()-1);	
	  }
      }
    for(int ch=0;ch<NumC;ch++)
      {
	if(flg==False) cleanBP(pl,bs,ch)= fitBP[ch];
	else cleanBP(pl,bs,ch)=0;
      }
    
  }// end FitCleanBandPass    
  
  
  
  /* FLAGGING IN FREQUENCY */
  void LFTimeFreqCrop :: FlagBandPass(uInt pl, uInt bs)
  {
    
    Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
    uInt a1,a2;
    Bool flg=False;
    
    
    Ants(bs,&a1,&a2);
    
    for(uInt tm=0;tm<NumT;tm++)
      {
	/* Divide (or subtract) out the clean bandpass */
	tempBP=0,flagBP=0;
	for(int ch=0;ch<NumC;ch++)
	  {
	    flagBP[ch] = flagc(pl,ch,((tm*NumB)+bs));
	    if(flagBP[ch]==False)
	      tempBP[ch] = visc(pl,ch,((tm*NumB)+bs))/cleanBP(pl,bs,ch);
	  }//for ch
	
	/* Flag outliers */
	temp=0;
	for(Int loop=0;loop<5;loop++)
	  {
	    mn=1;
	    sd = UStd(tempBP,flagBP,mn);
	    
	    for(Int ch=0;ch<NumC;ch++)
	      {
		if(flagBP[ch]==False && fabs(tempBP[ch]-mn) > F_TOL*sd)
		  {
		    flagBP[ch]=True ;flagcnt++;
		  }
	      }
	    if(fabs(temp-sd) < 0.1)break;
	    temp=sd;
	  }
	
	/* If sum of power in two adjacent channels is more than thresh, flag both side chans */
	/*
	  if(FlagLevel>0)
	  {
	  for(int ch=1;ch<NumC-1;ch++)
	  {
	  if(flagBP[ch])
	  {
	  if( ( fabs(tempBP[ch-1]-mn) + fabs(tempBP[ch+1]-mn) ) > F_TOL*sd )
	  {flagBP[ch-1]=True; flagBP[ch+1]=True;}
	  }
	  }
	  }
	*/
	
	/* Fill the flags into the visbuffer array */
	for(Int ch=0;ch<NumC;ch++)
	  flagc(pl,ch,((tm*NumB)+bs))=flagBP[ch];
	
	
      }//for tm
    
    
  }// end FlagBandPass    


  void LFTimeFreqCrop :: FitCleanTimeSeries(uInt pl, uInt bs, String fittype)
  {    
    
    Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
    uInt a1,a2;
    Bool flg=False;
    
    Ants(bs,&a1,&a2);
    /* Fit a smooth bandpass */
    flg=True;
    for(int tm=0;tm<NumT;tm++)	
      {
	// Calc the mean across frequency (with flags), and assign to tempTS
	tempBP=0;flagBP=False;
	for(uInt ch=0;ch<NumC;ch++)
	  {
	    tempBP[ch] = visc(pl,ch,((tm*NumB)+bs));
	    flagBP[ch] = flagc(pl,ch,((tm*NumB)+bs));
	  }//for tm
	tempTS[tm] = UMean(tempBP,flagBP) ;

	if(tempTS[tm] != 0) flg=False;
      }

    if(flg==False)// if any one datapoint is unflagged..
      {
	/* Piecewise Poly Fit to the meanTS */
	if(fittype == String("poly"))
	  {
	    FitPiecewisePoly(tempTS,flagTS,fitTS);	
	  }
	else
	  {
	    /* LineFit to flag off a line fit in time */
	    flagTS=False;
	    for(uInt tm=0;tm<tempTS.nelements();tm++)
	      if(tempTS[tm]==0) flagTS[tm]=True;
	    LineFit(tempTS,flagTS,fitTS,0,tempTS.nelements()-1);	
	  }
      }
    for(int tm=0;tm<NumT;tm++)
      {
	if(flg==False) cleanTS(pl,bs,tm)= fitTS[tm];
	else cleanTS(pl,bs,tm)=0;
      }
    
  }// end FitCleanTimeSeries
  
  
  
  /* FLAGGING IN FREQUENCY */
  void LFTimeFreqCrop :: FlagTimeSeriesAgain(uInt pl, uInt bs)
  {
    
    Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
    uInt a1,a2;
    Bool flg=False;
    
    
    Ants(bs,&a1,&a2);
    
    for(uInt ch=0;ch<NumC;ch++)
      {
	/* Divide (or subtract) out the clean bandpass */
	tempTS=0,flagTS=0;
	for(int tm=0;tm<NumT;tm++)
	  {
	    flagTS[tm] = flagc(pl,ch,((tm*NumB)+bs));
	    if(flagTS[tm]==False)
	      tempTS[tm] = visc(pl,ch,((tm*NumB)+bs))/cleanTS(pl,bs,tm);
	  }//for tm
	
	/* Flag outliers */
	temp=0;
	for(Int loop=0;loop<5;loop++)
	  {
	    mn=1;
	    sd = UStd(tempTS,flagTS,mn);
	    
	    for(Int tm=0;tm<NumT;tm++)
	      {
		if(flagTS[tm]==False && fabs(tempTS[tm]-mn) > F_TOL*sd)
		  {
		    flagTS[tm]=True ;flagcnt++;
		  }
	      }
	    if(fabs(temp-sd) < 0.1)break;
	    temp=sd;
	  }
	
	/* If sum of power in two adjacent channels is more than thresh, flag both side chans */
	/*
	  if(FlagLevel>0)
	  {
	  for(int ch=1;ch<NumC-1;ch++)
	  {
	  if(flagBP[ch])
	  {
	  if( ( fabs(tempBP[ch-1]-mn) + fabs(tempBP[ch+1]-mn) ) > F_TOL*sd )
	  {flagBP[ch-1]=True; flagBP[ch+1]=True;}
	  }
	  }
	  }
	*/
	
	/* Fill the flags into the visbuffer array */
	for(Int tm=0;tm<NumT;tm++)
	  flagc(pl,ch,((tm*NumB)+bs))=flagTS[tm];
	
	
      }//for ch
    
    
  }// end FlagTimeSeriesAgain
  
  
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
    Int deg=0,start=0;
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
	
	npieces = MIN(2*j+1, MaxNPieces);
	if(j>1) {deg=2;}
	if(j>2) {deg=3;}
	
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
	    if(winend>=tdata.nelements())winend=tdata.nelements()-1;
	    if(winend <= winstart) break;
	    winsum=0.0;
	    for(uInt wi=winstart;wi<=winend;++wi)
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
  
  
} //#end casa namespace

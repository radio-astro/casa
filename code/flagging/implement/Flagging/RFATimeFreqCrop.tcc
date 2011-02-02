//# RFATimeFreqCrop.cc: this defines RFATimeFreqCrop
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

#include <flagging/Flagging/RFATimeFreqCrop.h>

namespace casa { //# NAMESPACE CASA - BEGIN

#define baselinecnt ( (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-ant1[bs])*((NumAnt)-ant1[bs]+1)/2 + (ant2[bs] - ant1[bs]) )
#define self(ant) ( (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-ant)*((NumAnt)-ant+1)/2 )
#define MIN(a,b) ((a)<=(b) ? (a) : (b))

//#define PLOT  // to activate the mean and clean bandpass plots
//#define UPLOT // to activate bandpass plots of each fit iteration

//#define DOPLOT  // to activate ds9 and bandpass-fit plots

/* Constructor for 'RFATimeFreqCrop' */
RFATimeFreqCrop :: RFATimeFreqCrop( RFChunkStats &ch,const RecordInterface &parm ):
	                    RFAFlagCubeBase(ch,parm) ,
	                    RFDataMapper(parm.asArrayString(RF_EXPR),parm.asString(RF_COLUMN)),
	                    vi(ch.visIter()),
	                    vb(ch.visBuf())
{
	cubepos.resize(3);
	matpos.resize(3);
	ANT_TOL = parm.asDouble("ant_cutoff");
	BASELN_TOL = parm.asDouble("baseline_cutoff");
	T_TOL = parm.asDouble("time_amp_cutoff");
	F_TOL = parm.asDouble("freq_amp_cutoff");
	FlagLevel = parm.asInt("flag_level");
	CorrChoice = parm.asInt("auto_cross");
	NumTime = parm.asInt("num_time");
	ShowPlots = parm.asBool("showplots");
	FreqLineFit = parm.asBool("freqlinefit");
	MaxNPieces = parm.asInt("maxnpieces");
        DryRun = parm.asBool("dryrun");
        Expr = parm.asArrayString(RF_EXPR);
        Column = parm.asString(RF_COLUMN);
	//	cout << "Flagging on " << parm.asArrayString(RF_EXPR) << " for column : " << parm.asString(RF_COLUMN) << endl;
        StopAndExit=False;
	/*
        if(ShowPlots)
        {
           cmd = "xpaset -p ds9 plot new name flagger \n";
           system(cmd.data());
        }
	*/

}


/* Sets default values to parameters */
const RecordInterface & RFATimeFreqCrop::getDefaults ()
{
 static Record rec;
if( !rec.nfields() )
       {
	rec = RFAFlagCubeBase::getDefaults();
	rec.define(RF_NAME,"RFATimeFreqCrop");
	rec.define(RF_EXPR,"ABS I");
	rec.define(RF_COLUMN,"DATA");
	rec.define("ant_cutoff",0);
	rec.define("baseline_cutoff",0);
	rec.define("time_amp_cutoff",3);
	rec.define("freq_amp_cutoff",3);
	rec.define("flag_level",1);
	rec.define("auto_cross",1);
	rec.define("num_time",50);
	rec.define("column","DATA");
	rec.define("expr","ABS I");
	rec.define("fignore",False);
	rec.define("showplots",False);
	rec.define("freqlinefit",False);
	rec.define("maxnpieces",6);
        rec.define("dryrun",False);
//	rec.setcomment("ant_cutoff","Total autocorrelation amplitude threshold for a functional antenna");
//	rec.setcomment("time_amp_cutoff","Multiple/fraction of standard deviation, to set the threshold, while flagging across time");
//	rec.setcomment("freq_amp_cutoff","Multiple/fraction of standard deviation, to set the threshold, while flagging across frequency");
//	rec.setcomment("flag_level","Levels of Flagging");
       }
     return rec;
}



/* Called at the beginning of each chunk of data */
Bool RFATimeFreqCrop :: newChunk (Int &i)
{
  LogIO os(LogOrigin("tfcrop","newChunk","WHERE"));

  if(StopAndExit) 
    {
      //      cout << "newChunk :: NOT Working with data chunk : " << chunk.msName() << endl;
      return False;
    }

 
 //	cout << "newChunk :: Working with data chunk : " << chunk.msName() << endl;
 //	cout << "TimeSteps = " << num(TIME) << ", Baselines = " << num(IFR) << ", Chans = " << num(CHAN) << ", Polns = " << num(POLZN) << ", Ants = " << num(ANT) << endl;
 //	cout << "Parameters : " << " Antenna_tol=" << ANT_TOL << ", Baseline_tol=" << BASELN_TOL << ", Time_tol=" << T_TOL << "sigma, Freq_tol=" << F_TOL << "sigma, FlagLevel=" << FlagLevel << ", Flag_corr=" << String(CorrChoice?"Cross":"Auto") << endl;

	/* Initialize NumT - number of timestamps to work with in one go */
	if(NumTime==0) NumTime=50;
	if(num(TIME) >= NumTime) NumT = NumTime;
	else NumT = num(TIME);
	
	/* Assume that all baselines are present in this chunk */
	// TODO : check this.
       NumAnt = num(ANT);
       NumB = ((NumAnt)*((NumAnt)-1)/2 + (NumAnt));

       /* Number of polarizations */
       NumP = num(POLZN);
       //UUU : FORCE it to be 1 for now.....
       NumP=1;

       /* Number of channels */
        NumC = num(CHAN);

        /* Polarizations/correlations */

        corrmask = RFDataMapper::corrMask(chunk.visIter());
        Vector<Int> corrlist(num(POLZN));
        for(Int corr=0; corr<num(POLZN); corr++)
	  corrlist[corr] = (Int) ( (corrmask >> corr) & 1 );
	
        /* Check that the above makes sense */
        if(NumC<1 || NumP<1 || NumB <1 || NumAnt<1 || NumT<1)
	  {
	    cout << "Invalid chunk shapes" << endl;
            return False;
	  }

	//	os << "Chunk=" << chunk.nchunk() << ", Field=" << chunk.visIter().fieldName() << ", FieldID=" << chunk.visIter().fieldId() << ", Spw=" << chunk.visIter().spectralWindow() << ", nTime=" << num(TIME) << " (" << NumT << " at a time), nBaseline=" << num(IFR) << ", nChan=" << num(CHAN) << ", nCorrs=" << num(POLZN) << " [" << chunk.getCorrString() << "]" <<  LogIO::POST;
	//cout << "Chunk=" << chunk.nchunk() << ", Field=" << chunk.visIter().fieldName() << ", FieldID=" << chunk.visIter().fieldId() << ", Spw=" << chunk.visIter().spectralWindow() << ", nTime=" << num(TIME) << " (" << NumT << " at a time), nBaseline=" << num(IFR) << ", nChan=" << num(CHAN) << ", nCorrs=" << num(POLZN) << " [" << chunk.getCorrString() << "]" <<  endl; // ". Flagging on " << Expr << endl; //" ->  correlations : " << corrlist << endl;
	//        cout << "Working with " << NumC << " x " << NumT << " subsets (nchan x ntime),  "<< Expr << " on the " << Column << " column, and applying flags to correlations : " << corrlist << endl;


	/* UUU : What is this ? */
	//       RFAFlagCubeBase::newChunk(i-=1);
       
       /* Allocate memory for one set of NumT timesteps. */
        AllocateMemory();

	return RFAFlagCubeBase::newChunk(i);
}


/* Called at the beginning of each PASS */
void RFATimeFreqCrop :: startData (bool verbose) 
{
  //  cout  << "StartData - reset time-counter" << endl;

	RFAFlagCubeBase::startData(verbose);

	iterTimecnt=0; // running count of visbuffers gone by.
	timecnt=0;
  
	///	(chunk.visIter()).setRowBlocking(0); 

}

void RFATimeFreqCrop :: AllocateMemory()
{
	
	/* Cube to hold visibility amplitudes : POLZN x CHAN x (IFR*TIME) */
        visc.resize(NumP,NumC,NumB*NumT);
	visc=0;
	cubepos = visc.shape();
	
	/* Cube to hold visibility flags : POLZN x CHAN x (IFR*TIME) */
	flagc.resize(cubepos);
	flagc=False;
	// 	cout << " CubeShape = " << cubepos << endl;
	
	/* Cube to hold MEAN bandpasses : POLZN x IFR x CHAN */
	/* Cube to hold CLEAN bandpasses : POLZN x IFR x CHAN */
	if(CorrChoice == 0)
	  {
		meanBP.resize(NumP,NumAnt,NumC);
		cleanBP.resize(NumP,NumAnt,NumC);
	  }
	else
	  {
		meanBP.resize(NumP,NumB,NumC);
		cleanBP.resize(NumP,NumB,NumC);
	  }

	matpos = meanBP.shape();
	meanBP=0;
	cleanBP=0;

	//	cout << " BP Shape = " << matpos << endl;

	/* Cube to hold flags for the entire Chunk (channel subset, but all times) */
	chunkflags.resize(NumP,NumC,NumB*num(TIME));

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



/* Called once for every TIMESTAMP - for each VisBuf */
RFA::IterMode RFATimeFreqCrop :: iterTime (uInt itime) 
{
  
  //    cout << "iterTime :: " << itime << endl;
  //    RFAFlagCubeBase::iterTime(itime);
    RFDataMapper::setVisBuffer(vb);
   flag.advance(itime,True);
    
    //    vv = &vb.visCube(); // extract a viscube - one timestamp - one VisBuf
    //    vi.flag(ff); // extract the corresponding flags
    
    if(ant1.shape() != (vb.antenna1()).shape())
	ant1.resize((vb.antenna1()).shape());
    ant1 = vb.antenna1();
    
    if(ant2.shape() != (vb.antenna2()).shape())
	ant2.resize((vb.antenna2()).shape());
    ant2 = vb.antenna2();
    const Vector<Int> &ifrs( chunk.ifrNums() );
    
    uInt nBaselinesInData = ant1.nelements();
    //    cout << "ant1 nelements  : " << nBaselinesInData << " timecnt : " << timecnt << " itertimecnt : " << iterTimecnt << endl;

      /* Polarizations/correlations */
      corrmask = RFDataMapper::corrMask(chunk.visIter());
      Vector<Int> corrlist(num(POLZN));
      for(Int corr=0; corr<num(POLZN); corr++)
	corrlist[corr] = (Int) ( (corrmask >> corr) & 1 );

      //      if(nBaselinesInData != num(IFR)) cout << "nbaselines is not consistent !" << endl; 
        
    // Read in the data AND any existing flags into the flagCube - accumulate 
    Bool tfl;
    for(uInt pl=0;pl<NumP;pl++)
      {
    	for(uInt bs=0;bs<nBaselinesInData;bs++)
	  {
            Int countflags=0, countpnts=0;
	    for(uInt ch=0;ch<NumC;ch++)
	      {
		visc(pl,ch,(timecnt*NumB)+baselinecnt) = mapValue(ch,bs);
		tfl = chunk.npass() ? flag.anyFlagged(ch,chunk.ifrNum(bs)) : flag.preFlagged(ch,chunk.ifrNum(bs));
		////	tfl = chunk.npass() ? flag.anyFlagged(ch,ifrs(bs)) : flag.preFlagged(ch,ifrs(bs));
	   		//tfl = flag.preFlagger(ch,ifrs(bs));
		//tfl = False;
                //tfl = (Bool) (flag.getFlag(ch,chunk.ifrNum(bs))   &  corrmask);
		flagc(pl,ch,(timecnt*NumB)+baselinecnt) = tfl; //flag.anyFlagged(ch,ifrs(bs));
		chunkflags(pl,ch,(itime*NumB)+baselinecnt) = tfl; //flag.anyFlagged(ch,ifrs(bs));
		countpnts++;
		countflags += Int(tfl);
	      }
	    //	    if(countflags>0) cout << "Time : " << itime << " Preflags for baseline : " << bs << " (" << ant1(bs) << "," << ant2(bs) << ") : " << countflags << " out of " << countpnts << " " << corrlist << endl;
	  }
      }
    
    timecnt++;
    iterTimecnt++; // running count of visbuffers going by.
    
    
    /* BEGIN TIME-FLAGGING ALGORITHM HERE */
    
    /* After accumulating NumT timestamps, start processing this block */
    /////if(iterTimecnt > 0 && (timecnt==NumT || iterTimecnt == (vi.nRowChunk()/NumB)))
    if(iterTimecnt > 0 && (timecnt==NumT || itime==(num(TIME)-1) ))
      {
	RunTFCrop();
	
	if(ShowFlagPlots() == RFA::STOP)
	  return RFA::STOP;
	
	ExtendFlags();
	
	CountAndFillFlags();    
	
	// reset the NumT time counter !!!
	timecnt=0;
      }
    //    flag.advance(itime,True);
    return RFA::CONT; 
    ////RFAFlagCubeBase::iterTime(itime);
}


/* Called for each ROW - each baseline in a VisBuf */
// Fill in the visibilities and flags here.
// flag.advance() has to get called in iterTime, for iterRow to see the correct values.
RFA::IterMode RFATimeFreqCrop :: iterRow  (uInt irow ) 
{
  /* DUMMY CALL */
  return RFAFlagCubeBase::iterRow(irow);
}




void RFATimeFreqCrop :: RunTFCrop()
{
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  
  //// Later, take the pl and bs loops outside these functions.... and openmp over bs.
  //    make private :  All temp arrays.....
  
  meanBP = 0;
  cleanBP = 0;
  flagcnt = 0;
  antcnt = 0;
  
  // start OMP
  //    FlagZeros();		
  
  FlagTimeSeries();    
  
  FitCleanBandPass();
  
  FlagBandPass();
  
  GrowFlags();
  // stop OMP
  
}



/* If any data points are exactly zero, make sure corresponding flags
   are set. For the baseline mapper - this is an indicator of which baselines
   are missing (RowFlags */
void RFATimeFreqCrop :: FlagZeros()
{
  Float temp=0;
  Bool flg=False;
  
  //		RowFlags=False;
  
  /* Check if the data in all channels are filled with zeros. 
     If so, set the flags to zero  */    
  
  for(int pl=0;pl<NumP;pl++)
    {
      for(uInt tm=0;tm<NumT;tm++)
	{
	  for(uInt bs=0;bs<NumB;bs++)
	    {
	      temp=0;
	      flg=True;
	      for(int ch=0;ch<NumC;ch++)
		{
		  temp += visc(pl,ch,tm*NumB+bs);
		  flg &= flagc(pl,ch,tm*NumB+bs);
		}
	      //Update RowFlags - Flags to indicate which baselines are missing in
	      //    each integration timestamp 
	      //			RowFlags(pl,tm*NumB+bs)=flg;
	      if(temp==0) 
		{
		  //	RowFlags(pl,tm*NumB+bs)=True;
		  for(int ch=0;ch<NumC;ch++)
		    flagc(pl,ch,tm*NumB+bs)=True;
		}
	    }// for bs
	}// for tm
    }// for pl
}


/* Flag in time, and build the average bandpass */
/* Grow flags by one timestep, check for complete flagged baselines. */
void RFATimeFreqCrop :: FlagTimeSeries()
{
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  /* For each Channel - fit lines to 1-D data in time - flag according 
   * to them and build up the mean bandpass */
  
  //    cout << endl << " Flag across " << timecnt << " timesteps and create a time-averaged bandpass " << endl;
  
  Float rmean=0;
  for(int pl=0;pl<NumP;pl++)
    {
      antcnt=0;
      for(uInt bs=0;bs<NumB;bs++)
	{
	  Ants(bs,&a1,&a2);
	  if(CorrChoice==0)
	    {if(a1 != a2) continue;} // choose auto correlations
	  else
	    {if(a1==a2) continue;} // choose cross correlations
	  
	  
	  rmean=0;
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
		  // UUU : HERE - give choice of PolyFit in time...
		  LineFit(tempTS,flagTS,fitTS,0,tempTS.nelements()-1);	
		  sd = UStd(tempTS,flagTS,fitTS);
		  
		  for(uInt i=0;i<NumT;i++)
		    if(flagTS[i]==False && fabs(tempTS[i]-fitTS[i]) > T_TOL*sd)
		      {
			flagTS[i]=True ;flagcnt++;
		      }
		  if(fabs(temp-sd) < 0.1)break;
		  temp=sd;
		}
	      
	      // If sum of 2 adjacent flags also crosses threshold, flag 
	      for(uInt i=1;i<NumT-1;i++)
		{
		  if(flagTS[i])
		    {
		      if( ( fabs(tempTS[i-1]-fitTS[i-1]) + fabs(tempTS[i+1]-fitTS[i+1]) ) > T_TOL*sd )
			{flagTS[i-1]=True; flagTS[i+1]=True;}
		    }
		}
	      
	      
	      meanBP(pl,bs,ch) = UMean(tempTS,flagTS) ;
	      
	      /* write flags to local flag cube */
	      for(uInt tm=0;tm<NumT;tm++)
		flagc(pl,ch,((tm*NumB)+bs))=flagTS[tm];
	      
	    }//for ch
	  
	  antcnt++; 
	  
	  /* Check for completely flagged ants/bs */
	  // UUU : DOES NOT WORK. rmean is often ZERO for no reason.
	  if(1)
	    {
	      if((CorrChoice==0 && a1 == a2)||(CorrChoice!=0 && a1 != a2)) 
		{
		  if(CorrChoice==0)tol=ANT_TOL;
		  else tol=BASELN_TOL;
		  if(fabs(rmean/float(NumC)) < tol)
		    {
		      for(int ch=0;ch<NumC;ch++)
			for(uInt tm=0;tm<NumT;tm++)
			  flagc(pl,ch,((tm*NumB)+bs))=True;
		      if(CorrChoice==0) 
			cout << "Antenna Flagged : " << a1 << endl;
		      else
			cout << "Mean : " << rmean/NumC << " : Baseline Flagged : " << a1 << ":" << a2 << endl;
		    }
		  
		}
	    }///if(0);		
	}//for bs
    }//for pl
  
}    

/* Fit a smooth bandpass to the mean bandpass and store it 
 *  one for each baseline */

// matpos  :  NumP. NumB. NumC

void RFATimeFreqCrop :: FitCleanBandPass()
{    
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  
  //cout << " Clean up the bandpasses ... " << endl;

  //  String emptylist="";
  
  for(int pl=0;pl<NumP;pl++)
    {
      for(int ant=0;ant<NumB;ant++)
	{
	  Ants(ant,&a1,&a2);
	  if(CorrChoice==0)
	    {if(a1 != a2) continue;} // choose auto correlations
	  else
	    {if(a1==a2) continue;} // choose cross correlations
	  
	  
	  /* Fit a smooth bandpass */
	  flg=True;
	  for(int ch=0;ch<NumC;ch++)	
	    {
	      tempBP[ch] = meanBP(pl,ant,ch);
	      if(tempBP[ch] != 0) flg=False;
	    }
	  
	  if(flg==False)
	    {
	      /* Piecewise Poly Fit to the meanBP */
	      if(!FreqLineFit)
		{
		  CleanBand(tempBP,fitBP);	
		}
	      else
		{
		  /* LineFit to flag off a line fit in frequency */
		  flagBP=False;
		  for(uInt ch=0;ch<tempBP.nelements();ch++)
		    if(tempBP[ch]==0) flagBP[ch]=True;
		  LineFit(tempBP,flagBP,fitBP,0,tempBP.nelements()-1);	
		}
	      
#ifdef PLOT 
	      if(CorrChoice==0)
		cout<<" Antenna : "<<ant<<" Polzn : "<<pl<<endl;
	      else
		{
		  Ants(ant,&a1,&a2);
		  cout << " Baseline : " << a1 << ":" << a2 << " Polzn : " << pl << endl;
		}
	      Plot_ds9(tempBP.nelements(), tempBP,fitBP);  // Plot the band
#endif
	    }
	  /*	  else
	    {
		  Ants(ant,&a1,&a2);
                  emptylist += String::toString(a1)+"-"+String::toString(a2)+" ";
		  // 	         cout << "meanBP is filled with zeros : baseline : " << a1 << "-" << a2 << endl;
	    }
	  */
	  for(int ch=0;ch<NumC;ch++)
	    {
	      if(flg==False) cleanBP(pl,ant,ch)= fitBP[ch];
	      else cleanBP(pl,ant,ch)=0;
	    }
	  
	}//for bs
    }//for pl

  //  cout << "Baselines with no usable data : " << emptylist << endl;

}// end FitCleanBandPass    

/* FLAGGING IN FREQUENCY */
void RFATimeFreqCrop :: FlagBandPass()
{
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  
  for(int pl=0;pl<NumP;pl++)
    {
      antcnt=0;
      for(uInt bs=0;bs<NumB;bs++)
	{
	  Ants(bs,&a1,&a2);
	  if(CorrChoice==0)
	    {if(a1 != a2) continue;} // choose auto correlations
	  else
	    {if(a1==a2)continue;} // choose cross correlations
	  
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
	      for(int ch=1;ch<NumC-1;ch++)
		{
		  if(flagBP[ch])
		    {
		      if( ( fabs(tempBP[ch-1]-mn) + fabs(tempBP[ch+1]-mn) ) > F_TOL*sd )
			{flagBP[ch-1]=True; flagBP[ch+1]=True;}
		    }
		}
	      
	      /* Fill the flags into the visbuffer array */
	      for(Int ch=0;ch<NumC;ch++)
		flagc(pl,ch,((tm*NumB)+bs))=flagBP[ch];
	      
	      
	    }//for tm
	  
	  if((CorrChoice==0 && a1 == a2) || (CorrChoice!=0 && a1 != a2)) 
	    antcnt++;
	}//for bs
    }//for pl	
  
}// end FlagBandPass    

/* APPLY FLAG HEURISTICS ON THE FLAGS FOR ALL AUTOCORRELATIONS */
void RFATimeFreqCrop :: GrowFlags()
{
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  //  cout << " GrowFlag :: Apply Flag Heuristics for flaglevel " << FlagLevel << endl;
  
  /// UUU : ADD NEW FUNCTION TO COUNT FLAGS. Do this before and after growing..... to check. Print this info out.
  
  if(FlagLevel > 0)
    {
      for(int pl=0;pl<NumP;pl++)
	{
	  for(uInt bs=0;bs<NumB;bs++)
	    {
	      Ants(bs,&a1,&a2);
	      if(CorrChoice==0)
		{if(a1 != a2) continue;} // choose autocorrelations
	      else
		{if(a1==a2) continue;} // choose cross correlations
	      
	      for(int ch=0;ch<NumC;ch++)
		{ 
		  // uInt fsum=0;
		  for(uInt tm=0;tm<NumT;tm++)
		    {       
		      if(FlagLevel>1) // flag level 2 and above...
			{
			  // flagging one timestamp before and after
			  if(tm>0)
			    if(flagc(pl,ch,((tm*NumB+bs)))==True)
			      flagc(pl,ch,(((tm-1)*NumB+bs)))=True;
			  
			  if((NumT-tm)<NumT-1)
			    if(flagc(pl,ch,(((NumT-tm)*NumB+bs)))==True)
			      flagc(pl,ch,(((NumT-tm+1)*NumB+bs)))=True;
			}
		      
		      if(FlagLevel>1) // flag level 2 and above
			{
			  // flagging one channel before and after
			  if(ch>0)
			    if(flagc(pl,ch,((tm*NumB+bs)))==True)
			      flagc(pl,ch-1,((tm*NumB+bs)))=True;
			  
			  if((NumC-ch)<NumC-1)
			    if(flagc(pl,(NumC-ch),(tm*NumB+bs))==True)
			      flagc(pl,(NumC-ch+1),(tm*NumB+bs))=True;
			}
		      
		      if(FlagLevel>0) // flag level 1 and above
			{
			  /* If previous and next channel are flagged, flag it */
			  if(ch>0 && ch < NumC-1)
			    {
			      if( flagc(pl,ch-1,(tm*NumB+bs) ) == True 
				  && flagc(pl,ch+1,(tm*NumB+bs) ) == True  )
				flagc(pl,ch,(tm*NumB+bs) ) = True;
			    }
			  /* If previous and next timestamp are flagged, flag it */
			  if(tm>0 && tm < NumT-1)
			    {
			      if( flagc(pl,ch,((tm-1)*NumB+bs) ) == True 
				  && flagc(pl,ch,((tm+1)*NumB+bs) ) == True  )
				flagc(pl,ch,(tm*NumB+bs) ) = True;
			    }
			}
		      
		      if(FlagLevel>1) // flag level 2 and above
			{
			  /* If next two channels are flagged, flag it */
			  if(ch < NumC-2)
			    if( flagc(pl,ch+1,(tm*NumB+bs)) == True 
				&& flagc(pl,ch+2,(tm*NumB+bs) ) == True  )
			      flagc(pl,ch,(tm*NumB+bs) ) = True;
			}
		      
		    }//for tm
		  
		  // if more than 60% of the timetange flagged - flag whole timerange for that channel
		  //if(fsum < 0.4*cubepos[2]/NumB && FlagLevel > 1) // flag level 2 
		  //	for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
		  //		flagc(pl,ch,((tm*NumB+bs)))=True;
		}//for ch
	      
	      if(FlagLevel>0) // flag level 1 and above
		{
		  /* If more than 4 surrounding points are flagged, flag it */
		  uInt fsum2=0;
		  for(int ch=1;ch<NumC-1;ch++)
		    {
		      for(uInt tm=1;tm<NumT-1;tm++)
			{
			  fsum2 = (uInt)(flagc(pl,ch-1,(((tm-1)*NumB+bs)))) + (uInt)(flagc(pl,ch-1,((tm*NumB+bs)))) + 
			    (uInt)(flagc(pl,ch-1,(((tm+1)*NumB+bs)))) + (uInt)(flagc(pl,ch,(((tm-1)*NumB+bs)))) + 
			    (uInt)(flagc(pl,ch,(((tm+1)*NumB+bs)))) + (uInt)(flagc(pl,ch+1,(((tm-1)*NumB+bs)))) + 
			    (uInt)(flagc(pl,ch+1,((tm*NumB+bs)))) + (uInt)(flagc(pl,ch+1,(((tm+1)*NumB+bs))));
			  if(fsum2 > 4) flagc(pl,ch,((tm*NumB+bs))) = True;
			} // for tm
		    }// for ch
		}// if FlagLevel>0
	      
	      if(FlagLevel>0) // flaglevel = 1 and above
		{
		  uInt fsum2=0;
		  /* Grow flags in time */
		  for(int ch=0;ch<NumC;ch++)
		    { 
		      fsum2=0;
		      /* count unflagged points for this channel (all times) */
		      for(uInt tm=0;tm<NumT;tm++)
			fsum2 += (flagc(pl,ch,((tm*NumB+bs)))==True)?0:1 ; 
		      /*if more than 50% of the timetange flagged - flag whole timerange for that channel */
		      if(fsum2 < 0.5*NumT)
			for(uInt tm=0;tm<NumT;tm++)
			  flagc(pl,ch,((tm*NumB+bs)))=True;
		    }// for ch
		}// if flaglevel>0
	      
	    }//for bs
	}//for pl
      
      
    }//if flag_level
  
  // Count flags
  /*
    Float runningcount=0, runningflag=0;
    runningcount=0;
    runningflag=0;
    for(int pl=0;pl<NumP;pl++)
    {
    for(uInt bs=0;bs<NumB;bs++)
    {
    Ants(bs,&a1,&a2);
    if(CorrChoice==0)
    {if(a1 != a2) continue;} // choose autocorrelations
    else
    {if(a1==a2) continue;} // choose cross correlations
    
    for(int ch=0;ch<NumC;ch++)
    {
    for(uInt tm=0;tm<NumT;tm++)
    {
    runningflag += Float(flagc(pl,ch,(tm*NumB)+bs));
    runningcount++ ;
    }// for tm
    }//for ch
    }// for bs
    }// for pl
    
    
    cout << " Flagged : " << 100 * runningflag/runningcount << " % for timesteps " << iterTimecnt-NumT << " - " << iterTimecnt << endl;
  */
}// end of GrowFlags

/* GRAY SCALE DISPLAYS ON ds9 */
RFA::IterMode RFATimeFreqCrop :: ShowFlagPlots()
{    
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  if(ShowPlots)
    {
      
      //	cout << "About to display : allocating cubes" << endl;	
      /*      
	      Float **dispdat=NULL,**flagdat=NULL;
	      dispdat = (Float **) malloc(sizeof(Float *) * NumT + sizeof(Float)*NumC*NumT);
	      for(uInt i=0;i<NumT;i++)
	      dispdat[i] = (Float *) (dispdat + NumT) + i * NumC;
	      
	      flagdat = (Float **) malloc(sizeof(Float *) * NumT + sizeof(Float)*NumC*NumT);
	      for(uInt i=0;i<NumT;i++)
	      flagdat[i] = (Float *) (flagdat + NumT) + i * NumC;
      */
      
      IPosition shp(2),tshp(2); shp(0)=NumC; shp(1)=NumT;
      Array<Float> dispdat(shp), flagdat(shp);
      
      //	cout << "About to display : allocated. "  << endl;
      
      char choice = 'a';
      
      Float runningsum=0, runningflag=0;
      for(int pl=0;pl<NumP;pl++)
	{
	  if(choice == 's') continue;
	  for(uInt bs=0;bs<NumB;bs++)
	    {
	      if(choice == 's') continue;
	      
	      runningsum=0;
	      runningflag=0;
	      Ants(bs,&a1,&a2);
	      if(CorrChoice==0)
		{if(a1 != a2) continue;} // choose autocorrelations
	      else
		{if(a1==a2) continue;} // choose cross correlations
	      
	      for(int ch=0;ch<NumC;ch++)	
		{
		  tempBP[ch] = meanBP(pl,bs,ch);
		  fitBP[ch] = cleanBP(pl,bs,ch);
		}
	      
	      for(int ch=0;ch<NumC;ch++)
		{ 
		  for(uInt tm=0;tm<NumT;tm++)
		    {       
		      //		      dispdat[tm][ch] = visc(pl,ch,(((tm*NumB)+bs)));
		      //		      flagdat[tm][ch] = dispdat[tm][ch]*(!flagc(pl,ch,(tm*NumB)+bs));
		      //		      runningsum += dispdat[tm][ch];
		      //		      runningflag += Float(flagc(pl,ch,(tm*NumB)+bs));
		      tshp[0]=ch; tshp[1]=tm;
		      dispdat(tshp) = visc(pl,ch,(((tm*NumB)+bs)));
		      flagdat(tshp) = dispdat(tshp)*(!flagc(pl,ch,(tm*NumB)+bs));
		      runningsum += dispdat(tshp);
		      runningflag += Float(flagc(pl,ch,(tm*NumB)+bs));
		    }//for tm
		}//for ch
	      
	      //Plot on ds9 !!
	      
	      //		cout << "Antenna1 : " << a1 << "  Antenna2 : " << a2 << "  Polarization : " << pl << endl;
	      //		cout << "Vis sum : " << runningsum << " Flag % : " << 100 * runningflag/(NumC*NumT) << endl;
	      //	      cout << " Flagged : " << 100 * runningflag/(NumC*NumT) << " %" << endl;
	      cout << " Flagged : " << 100 * runningflag/(NumC*NumT) << " %  on " << Expr << " for timesteps " << iterTimecnt-NumT << " - " << iterTimecnt << " on baseline " << a1 << "-" << a2;
	      
	      if(!runningsum)
		{
		  cout << " : No non-zero data !" << endl;
		}
	      else
		{
                  cout << endl;

		  Display_ds9(NumC,NumT,dispdat,1);
		  Display_ds9(NumC,NumT,flagdat,2);
		  Plot_ds9(tempBP.nelements(), tempBP, fitBP);
		  //UPlot(tempBP,fitBP,0,tempBP.nelements()-1);  // Plot the band
		  
		  cout << "Press <c> to continue display, <s> to stop display but continue flagging, <q> to quit." << endl;
		  //getchar();
		  cin >> choice;
		  //		    cout << " Choice : " << choice << endl;
		  switch(choice)
		    {
		    case 'q': 
		      ShowPlots = False; 
		      StopAndExit = True;
		      cout << "Exiting flagger" << endl;
		      return RFA::STOP;
		    case 's': 
		      ShowPlots = False;
		      cout << "Stopping display. Continuing flagging." << endl;
		      break;
		    default:
		      break;
		    }
		}
	    }//for bs
	}//for pl
      
    }// end of if ShowPlots
  
}// end ShowFlagPlots

/* Extend Flags
   (1) If flagging on self-correlations, extend to cross-corrs.
   (2) If requested, extend across polarization (fiddle with corrmask)
   (3) If requested, extend across baseline/antenna
*/
void RFATimeFreqCrop :: ExtendFlags()
{    
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  
  /* FLAG BASELINES FROM THE SELF FLAGS */
  if(CorrChoice ==0)
    {	
      cout << " Flagging Cross correlations from self correlation flags " << endl;
      for(int pl=0;pl<NumP;pl++)
	{
	  for(uInt bs=0;bs<NumB;bs++)
	    {
	      Ants(bs,&a1,&a2);
	      if(a1 == a2) continue; // choose cross correlations
	      for(int ch=0;ch<NumC;ch++)
		{
		  for(uInt tm=0;tm<cubepos[2]/NumB;tm++)
		    {
		      flagc(pl,ch,((tm*NumB+bs))) = flagc(pl,ch,((tm*NumB)+self(a1))) | flagc(pl,ch,((tm*NumB)+self(a1))); 
		    }//for tm
		}//for ch
	    }//for bs
	}//for pl
    }
  
}// end Extend Flags    

void RFATimeFreqCrop :: CountAndFillFlags()
{
  Float mn=0,sd=0,temp=0,flagcnt=0,tol=0;
  uInt a1,a2,antcnt;
  Bool flg=False;
  //cout << " Diagnostics on flag cube " << endl;
  
  for(int pl=0;pl<NumP;pl++)
    {
      for(uInt bs=0;bs<NumB;bs++)
	{
	  //if(RowFlags(pl,(tm*NumB)+bs)==True)
	  //	continue;
	  for(int ch=0;ch<NumC;ch++)
	    {
	      for(uInt tm=0;tm<NumT;tm++)
		{
		  if(flagc(pl,ch,((tm*NumB)+bs))==True )  
		    {
		      chunkflags(pl,ch,((tm+iterTimecnt-NumT)*NumB)+bs) = True;
		    }
		}// for tm
	    }//for ch
	}// for bs
    }// for pl
  
  // what is this ??  
  timecnt=0;
  
}// end of CountAndFillFlags



/*RedFlagger::run - ends loop for all agents with endData 
 * Calls endData once at the end of each PASS */
RFA::IterMode RFATimeFreqCrop :: endData  () 
{
  //  cout << " In End Data. Ending timecnt :  "  << timecnt << endl;
  
  RFAFlagCubeBase::endData();
  return RFA::STOP;
}


/* Called at the beginning of each PASS */
void RFATimeFreqCrop :: startFlag (bool verbose) 
{
  //  corrmask = RFDataMapper::corrMask(chunk.visIter());
  //  cout  << "StartFlag : corrmask : " << corrmask << endl;
  
  RFAFlagCubeBase::startFlag(verbose);
  
}


/* Write Flags to MS */
void RFATimeFreqCrop :: iterFlag(uInt itime)
{
  //  cout << "iterFlag :: Set flags for time : " << itime << endl;
    
  // FLAG DATA
  
  if(!DryRun)
    {
      flag.advance(itime);
      corrmask = RFDataMapper::corrMask(chunk.visIter());
      
      const Vector<Int> &ifrs( chunk.ifrNums() );
      if(ant1.shape() != (vb.antenna1()).shape())
	ant1.resize((vb.antenna1()).shape());
      ant1 = vb.antenna1();
      
      if(ant2.shape() != (vb.antenna2()).shape())
	ant2.resize((vb.antenna2()).shape());
      ant2 = vb.antenna2();
      
      uInt nbs = ant1.nelements();
      for(uInt pl=0;pl<NumP;pl++)
	{
	  for(uInt ch=0;ch<NumC;ch++)
	    {
	      for(uInt bs=0;bs<nbs;bs++)
		{
		  if(chunkflags(pl,ch,(itime*NumB)+baselinecnt))
		    flag.setFlag(ch,ifrs(bs));
		}
	    }
	}
      
      flag.setMSFlags(itime);
      
    }// if not dry-run
  else
    {
      RFAFlagCubeBase::iterFlag(itime);
    }
  
  /// FLAG ROWS  
  /*
    ant1 = vb.antenna1();
    ant2 = vb.antenna2();
    uInt npols = (chunkflags.shape())[0];
    uInt nbs = ant1.nelements();
    
    vi.flag(ff);
    vi.flagRow(fr);
    
    for(uInt bs=0;bs<nbs;bs++)
    {
    Bool rowflag=True;
    for(uInt pl=0;pl<npols;pl++)
    {
    for(uInt ch=StartChan;ch<=EndChan;ch++)
    {
    ff(pl,ch,bs) = chunkflags(pl,ch-StartChan,(itime*NumB)+baselinecnt);
    rowflag &= ff(pl,ch,bs);
    }
    }
    if(rowflag) fr[bs]=True;
    }
    
    //vi.setFlag(ff);
    //vi.setFlagRow(fr); 
    
    chunk.visIter().setFlag(ff);
    chunk.visIter().setFlagRow(fr);
    
  */
}

/*RedFlagger::run - calls 'endChunk()' on all agents. */

void RFATimeFreqCrop :: endChunk () 
{
  LogIO os(LogOrigin("tfcrop","endChunk","WHERE"));
  //     cout << "endChunk : counting flags" << endl;
  // Count flags
  if(!StopAndExit)
    {
      Float runningcount=0, runningflag=0;
      
      const Vector<Int> &ifrs( chunk.ifrNums() );
      if(ant1.shape() != (vb.antenna1()).shape())
	ant1.resize((vb.antenna1()).shape());
      ant1 = vb.antenna1();
      
      if(ant2.shape() != (vb.antenna2()).shape())
	ant2.resize((vb.antenna2()).shape());
      ant2 = vb.antenna2();
      
      uInt nbs = ant1.nelements();
      for(uInt pl=0;pl<NumP;pl++)
	{
	  for(uInt ch=0;ch<NumC;ch++)
	    {
	      for(uInt tm=0;tm<num(TIME);tm++)
		{
		  for(uInt bs=0;bs<nbs;bs++)
		    {
		      runningflag += Float(chunkflags(pl,ch,((tm)*NumB)+baselinecnt));
		      runningcount++;
		    }
		}
	    }
	}
      
      /* Polarizations/correlations */
      corrmask = RFDataMapper::corrMask(chunk.visIter());
      Vector<Int> corrlist(num(POLZN));
      for(Int corr=0; corr<num(POLZN); corr++)
	corrlist[corr] = (Int) ( (corrmask >> corr) & 1 );
      
      //      cout << "--> Flagged " << 100 * runningflag/runningcount << " % on " << Expr << ". Applying to correlations : " << corrlist << endl;
      os << "TFCROP : Flagged " << 100 * runningflag/runningcount << " % on " << Expr << ". Applying to corrs : " << corrlist;
      if(DryRun) os << " (Not writing flags to MS)" << LogIO::POST;
      else os << " (Writing flags to MS)" << LogIO::POST;
    }
  RFAFlagCubeBase::endChunk();
  ///  (chunk.visIter()).setRowBlocking(0); //reset to default
  //      	cout << " End of endChunk !!" << endl;
}


/* Destructor for RFATimeFreqCrop */


RFATimeFreqCrop :: ~RFATimeFreqCrop () 
{
  //cout << "destructor for RFATimeFreqCrop" << endl;
}




/* Calculate the MEAN of 'vect' ignoring values flagged in 'flag' */
Float RFATimeFreqCrop :: UMean(Vector<Float> vect, Vector<Bool> flag)
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
Float RFATimeFreqCrop :: UStd(Vector<Float> vect, Vector<Bool> flag, Vector<Float> fit)
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
Float RFATimeFreqCrop :: UStd(Vector<Float> vect, Vector<Bool> flag, Float mean)
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
void RFATimeFreqCrop :: CleanBand(Vector<Float> data,Vector<Float> fit)
{
  //    Int step=0,ind=0;
  Int deg=0,start=0;
  Int left=0,right=0;
  //  Int le=0,ri=0;
  Float sd,TOL=3;
  Vector<Float> tdata;
  Vector<Bool> tfband;
  
  tfband.resize(data.nelements());
  tdata.resize(data.nelements());
  
  tfband = False;
  tdata = data;
  
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
	tfband[i]=True;
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
	    LineFit(tdata,tfband,fit,left,right);
	  else 
	    PolyFit(tdata,tfband,fit,left,right,deg);
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
      sd = UStd(tdata,tfband,fit);
      if(j>=2)  TOL=2;
      else TOL=3;
      
      /* Display the Fit and the data */
#ifdef UPLOT 
      Plot_ds9(data.nelements(),data,fit1);	 
#endif
      
      /* Detect outliers */
      for(uInt i=0;i<tdata.nelements();i++)
	{
	  if(tdata[i]-fit[i] > TOL*sd) 
	    tfband[i]=True;
	}
      
    } // for j
  
} // end of CleanBand



  /* Fit a polynomial to 'data' from lim1 to lim2, of given degree 'deg', 
   * taking care of flags in 'flag', and returning the fitted values in 'fit' */
void RFATimeFreqCrop :: PolyFit(Vector<Float> data,Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2,uInt deg)
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
void RFATimeFreqCrop :: LineFit(Vector<Float> data, Vector<Bool> flag, Vector<Float> fit, uInt lim1, uInt lim2)
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

/* Return antenna numbers from baseline number - upper triangle storage */
void RFATimeFreqCrop :: Ants(uInt bs, uInt *a1, uInt *a2)
{
  uInt sum=0,cnt=0;
  for(uInt i=(NumAnt);i>1;i--)
    {
      sum += i;
      if(sum<=bs) cnt++;
      else break;
    }
  *a1 = cnt;
  
  sum = (NumAnt)*((NumAnt)+1)/2 - ((NumAnt)-(*a1))*((NumAnt)-(*a1)+1)/2; 
  
  *a2 = bs - sum + (*a1);
}


/* Display a 2D data set on DS9 in gray scale */
void RFATimeFreqCrop :: Display_ds9(Int xdim, Int ydim, Array<Float> &data, Int frame)
{
  
  FILE *SAOout = NULL;
  char tmp[100];
  char server[100] = "ds9";	
  int bitpix = -32, i;
  char xpa[100] = "";
  
  strcpy (xpa, "xpaset");
  
  sprintf (tmp, "%s %s \"array [xdim=%d,ydim=%d,bitpix=%d]\"\n",
	   xpa, server, xdim, ydim, bitpix);
  SAOout = (FILE *) popen (tmp, "w");
  
  if (frame > 0)
    {
      sprintf (tmp, "echo \"frame %d \" | %s %s \n", frame, xpa, server);
      system (tmp);
    }
  
  if (SAOout != NULL)
    {
      //      for (i = 0; i < ydim; i++)
      //	fwrite (data[i], sizeof (float) * xdim, 1, SAOout);
      Bool deleteit=False;
      Float *dataptr = data.getStorage(deleteit);
      fwrite(dataptr, sizeof(Float)*xdim*ydim, 1, SAOout);
      pclose (SAOout);
      //if(deleteit) data.freeStorage(dataptr,True);
    }
  else
    {
      perror ("Error in opening SAO - ds9 \n");
    }
  
  if (frame > 0)
    {
      system ("xpaset -p ds9 zoom to fit");
    }
  
  //cout << " Press enter to continue... " << endl;
  //getchar();
  
  
}

/* Display a line plot in DS9 !!! */
void RFATimeFreqCrop :: Plot_ds9(Int dim, Vector<Float> data1, Vector<Float> data2)
{
  
  FILE *SAOout = NULL;
  char tmp[100];
  char server[100] = "ds9";	
  int bitpix = -32, i;
  char xpa[100] = "";
  
  //static Bool firstentry=True;
  strcpy (xpa, "xpaset");
  
  String cmd("");
  
  {
    cmd = "xpaset -p ds9 plot flagger clear \n";
    system(cmd.data());
    //        SAOout = (FILE *) popen (tmp, "w");
  }
  
  
  cmd = "echo '";
  for(i=0;i<dim;i++)
    {
      cmd +=  String::toString(i) + " " + String::toString(data2[i]) + " " ;
    }
  cmd += "\n' | xpaset ds9 plot flagger data xy\n";
  cmd += "xpaset -p ds9 plot flagger color linear blue\n";
  cmd += "xpaset -p ds9 plot flagger line linear width 2.0\n";
  system(cmd.data());
  
  cmd = "echo '";
  for(i=0;i<dim;i++)
    {
      cmd +=  String::toString(i) + " " + String::toString(data1[i]) + " " ;
    }
  cmd += "\n' | xpaset ds9 plot flagger data xy\n";
  cmd += "xpaset -p ds9 plot flagger color linear red\n";
  cmd += "xpaset -p ds9 plot flagger line linear width 2.0\n";
  system(cmd.data());
  
}// end of plot_ds9


} //# NAMESPACE CASA - END

